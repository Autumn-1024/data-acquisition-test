/**
 ****************************************************************************************************
 * @file        app_task.c
 * @author      Autumn
 * @version     V3.0
 * @date        2026-06-25
 * @brief       应用层 - 主循环
 ****************************************************************************************************
 * @attention
 *
 * 按键映射:
 *   KEY1(PB10) -> 返回
 *   KEY2(PB11) -> 下移
 *   KEY3(PA11) -> 上移
 *   KEY4(PA12) -> 确认
 *
 ****************************************************************************************************
 */

#include "app_task.h"
#include "bsp_key.h"
#include "bsp_led.h"
#include "bsp_uart.h"
#include "app_menu.h"
#include <stdio.h>

/**
 * @brief       应用入口
 */
void app_start(void)
{
    uint8_t key;
    uint8_t t = 0;

    /* 初始化菜单(含传感器数据) */
    app_menu_init();

    while (1)
    {
        /* 串口命令处理: '1'~'10' 跳转传感器 */
        if (bsp_uart_rx_ready())
        {
            static uint8_t last_ch = 0;
            uint8_t ch = bsp_uart_read_char();

            if (last_ch == '1' && ch == '0')
            {
                app_menu_goto_sensor(9);           /* "10" -> Sensor10 */
                last_ch = 0;
            }
            else if (ch >= '1' && ch <= '9')
            {
                app_menu_goto_sensor(ch - '1');    /* '1'->Sensor1 ... '9'->Sensor9 */
                last_ch = ch;
            }
            else
            {
                last_ch = ch;
            }

            /* LED控制命令 */
            if (ch == 'a') { LED(0); printf("LED ON\r\n"); }
            else if (ch == 'b') { LED(1); printf("LED OFF\r\n"); }
            else if (ch == 'c') { LED_TOGGLE(); printf("LED TOGGLE\r\n"); }
            else if (ch == 'd') { printf("LED:%s,Sensor:%d\r\n", HAL_GPIO_ReadPin(LED_GPIO_PORT, LED_GPIO_PIN) ? "OFF" : "ON", app_menu_get_sensor_index() + 1); }
        }

        /* 扫描按键 */
        key = bsp_key_scan(0);

        /* 菜单处理 */
        app_menu_process(key);

        /* LED 心跳 */
        t++;
        HAL_Delay(10);
        if (t == 50)
        {
            LED_TOGGLE();
            t = 0;
        }
    }
}
