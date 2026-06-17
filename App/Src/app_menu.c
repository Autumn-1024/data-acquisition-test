/**
 ****************************************************************************************************
 * @file        app_menu.c
 * @author      Autumn
 * @version     V1.0
 * @date        2026-06-16
 * @brief       OLED菜单系统
 ****************************************************************************************************
 * @attention
 *
 * 0.96�?OLED 128x64, 12号字�?(每行�?1字符, �?�?
 * 按键: KEY0=返回, KEY1=下移, KEY2=上移, KEY_UP=确认
 *
 ****************************************************************************************************
 */

#include "app_menu.h"
#include "bsp_key.h"
#include "bsp_oled.h"
#include "bsp_curtain.h"
#include <stdio.h>

/* 菜单项文�?*/
static const char *menu_items[MENU_MAX] = {
    "1.Open",
    "2.Close",
    "3.Stop",
    "4.Percent",
    "5.Query",
};

/* 当前状�?*/
static menu_page_t cur_page = PAGE_MAIN_MENU;
static uint8_t cur_index = 0;          /* 菜单光标位置 */
static uint8_t percent_val = 50;       /* 百分比�?(默认50%) */
static uint8_t need_refresh = 1;       /* 刷新标志 */

/**
 * @brief       菜单初始�? */
void app_menu_init(void)
{
    cur_page = PAGE_MAIN_MENU;
    cur_index = 0;
    percent_val = 50;
    need_refresh = 1;
}

/**
 * @brief       绘制主菜�? */
static void menu_draw_main(void)
{
    uint8_t i;

    oled_clear();

    for (i = 0; i < MENU_MAX; i++)
    {
        if (i == cur_index)
        {
            /* 选中项前面加 > */
            oled_show_string(0, i * 12, ">", 12);
        }
        oled_show_string(8, i * 12, menu_items[i], 12);
    }

    oled_refresh_gram();
}

/**
 * @brief       绘制百分比选择页面
 */
static void menu_draw_percent(void)
{
    char buf[22];

    oled_clear();
    oled_show_string(0, 0, "-- Percent --", 12);

    /* 百分比�?*/
    oled_show_string(0, 16, "Value:", 12);
    oled_show_num(42, 16, percent_val, 3, 12);
    oled_show_string(66, 16, "%", 12);

    /* 进度�?(16字符�? */
    {
        uint8_t bar_len = (uint8_t)((uint16_t)percent_val * 16 / 100);
        uint8_t i;

        for (i = 0; i < 16; i++)
        {
            buf[i] = (i < bar_len) ? '#' : '-';
        }
        buf[16] = '\0';
        oled_show_string(0, 30, buf, 12);
    }

    /* 操作提示 */
    oled_show_string(0, 48, "K2:UP K1:DN KUP:OK", 12);

    oled_refresh_gram();
}

/**
 * @brief       绘制状态显示页�? */
static void menu_draw_status(void)
{
    oled_clear();
    oled_show_string(0, 0, "-- Status --", 12);
    oled_show_string(0, 16, "Querying...", 12);
    oled_show_string(0, 48, "K0:Back", 12);

    oled_refresh_gram();

    /* 发送查询命�?*/
    bsp_curtain_query_status();
}

/**
 * @brief       处理主菜单按�? * @param       key: 按键�? */
static void menu_handle_main(uint8_t key)
{
    if (key == KEY1_PRES)       /* 上移 */
    {
        if (cur_index > 0)
        {
            cur_index--;
            need_refresh = 1;
            printf("[KEY1] Cursor -> %s\r\n", menu_items[cur_index]);
        }
        else
        {
            printf("[KEY1] Already at top\r\n");
        }
    }
    else if (key == KEY2_PRES)  /* 下移 */
    {
        if (cur_index < MENU_MAX - 1)
        {
            cur_index++;
            need_refresh = 1;
            printf("[KEY2] Cursor -> %s\r\n", menu_items[cur_index]);
        }
        else
        {
            printf("[KEY2] Already at bottom\r\n");
        }
    }
    else if (key == KEY3_PRES)  /* 确认 */
    {
        switch (cur_index)
        {
            case MENU_OPEN:
                printf("[KEY3] Confirm: Open curtain\r\n");
                bsp_curtain_open();
                break;

            case MENU_CLOSE:
                printf("[KEY3] Confirm: Close curtain\r\n");
                bsp_curtain_close();
                break;

            case MENU_STOP:
                printf("[KEY3] Confirm: Stop curtain\r\n");
                bsp_curtain_stop();
                break;

            case MENU_PERCENT:
                printf("[KEY3] -> Percent page (current: %d%%)\r\n", percent_val);
                cur_page = PAGE_PERCENT;
                need_refresh = 1;
                return;

            case MENU_QUERY:
                printf("[KEY3] -> Status page\r\n");
                cur_page = PAGE_STATUS;
                need_refresh = 1;
                return;

            default:
                break;
        }

        /* 执行完命令后短暂显示提示 */
        oled_show_string(0, 52, "Sent!           ", 12);
        oled_refresh_gram();
        HAL_Delay(500);
        need_refresh = 1;
    }
}

/**
 * @brief       处理百分比页面按�? * @param       key: 按键�? */
static void menu_handle_percent(uint8_t key)
{
    if (key == KEY2_PRES)       /* 上移 = 增加10% */
    {
        if (percent_val <= 90)
        {
            percent_val += 10;
            need_refresh = 1;
            printf("[KEY2] Percent -> %d%%\r\n", percent_val);
        }
        else
        {
            printf("[KEY2] Percent already max (100%%)\r\n");
        }
    }
    else if (key == KEY1_PRES)  /* 下移 = 减少10% */
    {
        if (percent_val >= 10)
        {
            percent_val -= 10;
            need_refresh = 1;
            printf("[KEY1] Percent -> %d%%\r\n", percent_val);
        }
        else
        {
            printf("[KEY1] Percent already min (0%%)\r\n");
        }
    }
    else if (key == KEY3_PRES)  /* 确认 = 发送 */
    {
        printf("[KEY3] Send percent: %d%%\r\n", percent_val);
        bsp_curtain_set_percent(percent_val);

        oled_show_string(0, 52, "Sent!           ", 12);
        oled_refresh_gram();
        HAL_Delay(500);
        need_refresh = 1;
    }
    else if (key == KEY0_PRES)  /* 返回 */
    {
        printf("[KEY0] Back to main menu\r\n");
        cur_page = PAGE_MAIN_MENU;
        need_refresh = 1;
    }
}

/**
 * @brief       处理状态页面按键
 * @param       key: 按键值
 */
static void menu_handle_status(uint8_t key)
{
    if (key == KEY0_PRES)       /* 返回 */
    {
        printf("[KEY0] Back to main menu\r\n");
        cur_page = PAGE_MAIN_MENU;
        need_refresh = 1;
    }
}

/**
 * @brief       菜单处理(主循环调�?
 * @param       key: 当前按键�?(0=无按�?
 */
void app_menu_process(uint8_t key)
{
    /* 刷新显示 */
    if (need_refresh)
    {
        need_refresh = 0;

        switch (cur_page)
        {
            case PAGE_MAIN_MENU:
                menu_draw_main();
                break;
            case PAGE_PERCENT:
                menu_draw_percent();
                break;
            case PAGE_STATUS:
                menu_draw_status();
                break;
            default:
                break;
        }
    }

    /* 处理按键 */
    switch (cur_page)
    {
        case PAGE_MAIN_MENU:
            menu_handle_main(key);
            break;
        case PAGE_PERCENT:
            menu_handle_percent(key);
            break;
        case PAGE_STATUS:
            menu_handle_status(key);
            break;
        default:
            break;
    }
}
