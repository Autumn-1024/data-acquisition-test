/**
 ****************************************************************************************************
 * @file        app_menu.c
 * @author      Autumn
 * @version     V3.0
 * @date        2026-06-25
 * @brief       传感器采集显示系统
 ****************************************************************************************************
 * @attention
 *
 * 0.96寸 OLED 128x64
 * 欢迎界面 -> 传感器列表(14个) -> 传感器详情(数据+折线图)
 * 按键: KEY1=返回, KEY2=下移, KEY3=上移, KEY4=确认
 *
 ****************************************************************************************************
 */

#include "app_menu.h"
#include "bsp_key.h"
#include "bsp_oled.h"
#include "bsp_sensor_ultrasonic.h"
#include "bsp_sensor_resistive.h"
#include "bsp_sensor_hall.h"
#include "bsp_sensor_capacitive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/******************************************************************************************/
/* 传感器数据 */

static sensor_data_t g_sensors[SENSOR_COUNT];

/* 页面状态 */
static menu_page_t cur_page = PAGE_WELCOME;
static uint8_t cur_index = 0;           /* 列表光标 / 当前传感器索引 */
static uint8_t need_refresh = 1;

/* 传感器详情页刷新计时(由主循环10ms调用) */
static uint16_t detail_tick = 0;
#define DETAIL_REFRESH_MS   1000        /* 1秒刷新一次 */

/******************************************************************************************/
/* 模拟随机传感器数据 (跳变平缓) */

/**
 * @brief       初始化传感器数据
 */
static void sensor_data_init(void)
{
    uint8_t i, n;

    for (i = 0; i < SENSOR_COUNT; i++)
    {
        snprintf(g_sensors[i].name, SENSOR_NAME_LEN, "Sensor %d", i + 1);
        /* 初始值 8.0 ~ 12.0 */
        g_sensors[i].cur_value = 8.0f + (float)(rand() % 40) / 10.0f;
        g_sensors[i].history_cnt = 0;
        for (n = 0; n < CHART_WIDTH; n++)
        {
            g_sensors[i].history[n] = 0;
        }
    }
}

/**
 * @brief       更新一个传感器的模拟数据(缓慢波动)
 * @param       idx: 传感器索引
 */
static void sensor_simulate(uint8_t idx)
{
    float new_val;

    /* 1号电阻式传感器: 读取真实电压 */
    if (idx == 0)
    {
        new_val = bsp_resistive_read();
    }
    /* 2号霍尔传感器: 读取真实状态 */
    else if (idx == 1)
    {
        new_val = bsp_hall_read();
    }
    /* 3号电容式传感器: 读取真实状态 */
    else if (idx == 2)
    {
        new_val = bsp_capacitive_read();
    }
    /* 12号超声波传感器: 读取真实数据 */
    else if (idx == 11)
    {
        float dist = bsp_ultrasonic_read();
        if (dist >= 0.0f && dist <= 300.0f)
        {
            new_val = dist;
        }
        else
        {
            /* 读取失败: -1=超时(无ECHO), -2=超出范围 */
            new_val = 0.0f;
        }
    }
    else
    {
        /* 其他传感器: 模拟数据 */
        float delta;
        delta = ((float)(rand() % 201) - 100.0f) / 100.0f;
        new_val = g_sensors[idx].cur_value + delta;
    }

    /* 钳位 */
    if (new_val < SENSOR_VAL_MIN) new_val = SENSOR_VAL_MIN;
    if (new_val > SENSOR_VAL_MAX) new_val = SENSOR_VAL_MAX;

    g_sensors[idx].cur_value = new_val;
}

/**
 * @brief       将传感器值映射到折线图Y轴像素偏移(0~CHART_HEIGHT)
 * @param       val: 传感器值
 * @retval      像素偏移(0=顶部=最大值, CHART_HEIGHT=底部=最小值)
 */
static uint8_t sensor_val_to_pixel(float val)
{
    float ratio;

    if (val < SENSOR_VAL_MIN) val = SENSOR_VAL_MIN;
    if (val > SENSOR_VAL_MAX) val = SENSOR_VAL_MAX;

    ratio = (val - SENSOR_VAL_MIN) / (SENSOR_VAL_MAX - SENSOR_VAL_MIN);
    return (uint8_t)((1.0f - ratio) * CHART_HEIGHT);
}

/**
 * @brief       记录传感器历史数据并左移
 * @param       idx: 传感器索引
 */
static void sensor_record_history(uint8_t idx)
{
    uint16_t i;
    uint8_t pixel;

    /* 左移历史数据 */
    for (i = 0; i < CHART_WIDTH - 1; i++)
    {
        g_sensors[idx].history[i] = g_sensors[idx].history[i + 1];
    }

    /* 新数据放到最右边 */
    pixel = sensor_val_to_pixel(g_sensors[idx].cur_value);
    g_sensors[idx].history[CHART_WIDTH - 1] = pixel;

    if (g_sensors[idx].history_cnt < CHART_WIDTH)
    {
        g_sensors[idx].history_cnt++;
    }
}

/******************************************************************************************/
/* 页面绘制 */

/**
 * @brief       绘制欢迎界面
 */
static void draw_welcome(void)
{
    oled_clear();
    oled_show_string(12, 8,  "Data Acq.", 16);
    oled_show_string(6,  28, "Test Board", 16);
    oled_show_string(0,  52, "Press any key...", 12);
    oled_refresh_gram();
}

/**
 * @brief       绘制传感器列表
 */
static void draw_sensor_list(void)
{
    uint8_t i;
    uint8_t start;
    char buf[22];

    oled_clear();
    oled_show_string(0, 0, "== Sensors ==", 12);

    /* 显示传感器列表，一屏最多显示4个(每行14像素，从y=14开始) */
    /* 根据光标位置滚动 */
    if (cur_index < 4)
        start = 0;
    else
        start = cur_index - 3;

    for (i = 0; i < 4 && (start + i) < SENSOR_COUNT; i++)
    {
        if (start + i == cur_index)
        {
            snprintf(buf, sizeof(buf), "> %s", g_sensors[start + i].name);
        }
        else
        {
            snprintf(buf, sizeof(buf), "  %s", g_sensors[start + i].name);
        }
        oled_show_string(0, 14 + i * 12, buf, 12);
    }

    /* 显示滚动指示 */
    if (start > 0)
    {
        oled_show_string(116, 14, "^", 12);
    }
    if (start + 4 < SENSOR_COUNT)
    {
        oled_show_string(116, 50, "v", 12);
    }

    oled_refresh_gram();
}

/**
 * @brief       绘制传感器详情页
 */
static void draw_sensor_detail(void)
{
    sensor_data_t *s = &g_sensors[cur_index];
    char buf[22];
    uint16_t i;
    uint8_t x, y;
    uint16_t valid_cnt;

    oled_clear();

    /* 第1行: 小字传感器名称 */
    snprintf(buf, sizeof(buf), "%s", s->name);
    oled_show_string(0, 0, buf, 12);

    /* 第2行: 大字数据值 + 单位 */
    snprintf(buf, sizeof(buf), "%.1f", s->cur_value);
    {
        uint8_t len = strlen(buf);
        uint8_t char_w = 12;
        uint8_t total_w = len * char_w + 8;  /* +8 给 'cm' 留空间 */
        uint8_t x_pos = (128 - total_w) / 2;
        oled_show_string(x_pos, 12, buf, 24);
        oled_show_string(x_pos + total_w - 6, 16, "cm", 12);
    }

    /* 分隔线 y=33 */
    for (x = 0; x < 128; x++)
    {
        oled_draw_point(x, 33, 1);
    }

    /* 折线图区域: y=32 ~ y=63 (32像素高) */
    valid_cnt = s->history_cnt;
    if (valid_cnt > CHART_WIDTH) valid_cnt = CHART_WIDTH;

    for (i = 0; i < valid_cnt; i++)
    {
        x = (uint8_t)(CHART_WIDTH - valid_cnt + i);
        y = s->history[i];

        if (y >= CHART_HEIGHT) y = CHART_HEIGHT - 1;

        oled_draw_point(x, CHART_Y_START + y, 1);

        /* 连线 */
        if (i > 0)
        {
            uint8_t prev_y = s->history[i - 1];
            uint8_t cur_y = y;
            uint8_t y_min, y_max, ty;

            if (prev_y > cur_y) { y_min = cur_y; y_max = prev_y; }
            else                { y_min = prev_y; y_max = cur_y; }

            for (ty = y_min; ty <= y_max; ty++)
            {
                oled_draw_point(x - 1, CHART_Y_START + ty, 1);
            }
        }
    }

    oled_refresh_gram();
}

/******************************************************************************************/
/* 页面处理 */

/**
 * @brief       处理欢迎界面按键
 */
static void handle_welcome(uint8_t key)
{
    if (key != 0)
    {
        printf("[KEY] -> Sensor List\r\n");
        cur_page = PAGE_SENSOR_LIST;
        cur_index = 0;
        need_refresh = 1;
    }
}

/**
 * @brief       处理传感器列表按键
 */
static void handle_sensor_list(uint8_t key)
{
    if (key == KEY3_PRES)       /* 上移 */
    {
        if (cur_index > 0)
        {
            cur_index--;
            need_refresh = 1;
        }
    }
    else if (key == KEY2_PRES)  /* 下移 */
    {
        if (cur_index < SENSOR_COUNT - 1)
        {
            cur_index++;
            need_refresh = 1;
        }
    }
    else if (key == KEY4_PRES)  /* 确认 - 进入详情 */
    {
        printf("[KEY3] -> Sensor %d detail\r\n", cur_index + 1);
        cur_page = PAGE_SENSOR_DETAIL;
        detail_tick = 0;
        need_refresh = 1;
    }
    else if (key == KEY1_PRES)  /* 返回 */
    {
        printf("[KEY1] -> Welcome\r\n");
        cur_page = PAGE_WELCOME;
        need_refresh = 1;
    }
}

/**
 * @brief       处理传感器详情按键
 */
static void handle_sensor_detail(uint8_t key)
{
    if (key == KEY1_PRES)       /* 返回列表 */
    {
        printf("[KEY1] -> Sensor List\r\n");
        cur_page = PAGE_SENSOR_LIST;
        need_refresh = 1;
    }
}

/******************************************************************************************/
/* 初始化 */

void app_menu_init(void)
{
    sensor_data_init();
    bsp_resistive_init();     /* 初始化1号电阻式传感器 */
    bsp_hall_init();           /* 初始化2号霍尔传感器 */
    bsp_capacitive_init();    /* 初始化3号电容式传感器 */
    bsp_ultrasonic_init();    /* 初始化12号超声波传感器 */
    cur_page = PAGE_WELCOME;
    cur_index = 0;
    need_refresh = 1;
    detail_tick = 0;
}

/******************************************************************************************/
/* 获取当前传感器索引 */

uint8_t app_menu_get_sensor_index(void)
{
    return cur_index;
}

/******************************************************************************************/
/* 外部跳转: 直接进入指定传感器详情 */

void app_menu_goto_sensor(uint8_t idx)
{
    if (idx >= SENSOR_COUNT) return;
    cur_index = idx;
    cur_page = PAGE_SENSOR_DETAIL;
    detail_tick = 0;
    need_refresh = 1;
    printf("[UART] -> Sensor %d\r\n", idx + 1);
}

/******************************************************************************************/
/* 主处理函数(主循环10ms调用一次) */

void app_menu_process(uint8_t key)
{
    /* 传感器详情页定时刷新 */
    if (cur_page == PAGE_SENSOR_DETAIL)
    {
        detail_tick += 10;
        if (detail_tick >= DETAIL_REFRESH_MS)
        {
            detail_tick = 0;

            /* 模拟数据更新 */
            sensor_simulate(cur_index);
            /* 记录历史 */
            sensor_record_history(cur_index);
            /* 串口发送数据: 传感器序号@数据值 */
            printf("%d@%.1f\r\n", cur_index + 1, g_sensors[cur_index].cur_value);
            /* 标记需要刷新 */
            need_refresh = 1;
        }
    }

    /* 刷新显示 */
    if (need_refresh)
    {
        need_refresh = 0;

        switch (cur_page)
        {
            case PAGE_WELCOME:       draw_welcome();       break;
            case PAGE_SENSOR_LIST:   draw_sensor_list();   break;
            case PAGE_SENSOR_DETAIL: draw_sensor_detail(); break;
            default: break;
        }
    }

    /* 处理按键 */
    switch (cur_page)
    {
        case PAGE_WELCOME:       handle_welcome(key);       break;
        case PAGE_SENSOR_LIST:   handle_sensor_list(key);   break;
        case PAGE_SENSOR_DETAIL: handle_sensor_detail(key); break;
        default: break;
    }
}
