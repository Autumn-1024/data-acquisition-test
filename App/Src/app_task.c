/**
 ****************************************************************************************************
 * @file        app_task.c
 * @author      Autumn
 * @version     V2.0
 * @date        2026-06-16
 * @brief       应用层 - 菜单驱动主循环
 ****************************************************************************************************
 * @attention
 *
 * 按键映射 (菜单模式):
 *   KEY0(PB10) -> 确认 / 执行
 *   KEY1(PB11) -> 下移 / 减少
 *   KEY2(PA11) -> 上移 / 增加
 *   KEY3(PA12) -> 返回
 *
 ****************************************************************************************************
 */

#include "app_task.h"
#include "bsp_key.h"
#include "bsp_led.h"
#include "bsp_oled.h"
#include "bsp_rs485.h"
#include "bsp_curtain.h"
#include "app_menu.h"
#include <stdio.h>

/**
 * @brief       应用入口
 * @param       无
 * @retval      无
 */
void app_start(void)
{
    uint8_t key;
    uint8_t i;
    uint8_t rx_buf[16];
    uint8_t rx_len;
    uint8_t t = 0;

    /* 初始化菜单 */
    app_menu_init();

    while (1)
    {
        /* 扫描按键 */
        key = bsp_key_scan(0);

        /* 菜单处理 */
        app_menu_process(key);

        /* 接收RS485数据 */
        bsp_rs485_receive_data(rx_buf, &rx_len);

        if (rx_len)
        {
            if (rx_len > 16) rx_len = 16;

            printf("[RX] RS485 RECV: ");
            for (i = 0; i < rx_len; i++)
            {
                printf("%02X ", rx_buf[i]);
            }
            printf("\r\n");
        }

        t++;
        HAL_Delay(10);

        if (t == 50)
        {
            LED_TOGGLE();
            t = 0;
        }
    }
}
