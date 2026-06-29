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

    /* 初始化菜单(含传感器数据) */
    app_menu_init();

    while (1)
    {
        /* 串口命令处理: 从缓冲区读取，分析是单位数还是两位数 */
        if (bsp_uart_rx_ready())
        {
            uint8_t buf[4] = {0};
            uint16_t len = bsp_uart_read_buf(buf, sizeof(buf));
            uint16_t i = 0;

            while (i < len)
            {
                uint8_t ch = buf[i];

                /* 两位数: "10"~"14" -> Sensor 10~14 */
                if (ch == '1' && (i + 1) < len && buf[i + 1] >= '0' && buf[i + 1] <= '4')
                {
                    app_menu_goto_sensor(9 + (buf[i + 1] - '0'));
                    i += 2;
                }
                /* 单位数: '1'~'9' -> Sensor 1~9 */
                else if (ch >= '1' && ch <= '9')
                {
                    app_menu_goto_sensor(ch - '1');
                    i++;
                }
                /* LED控制命令 */
                else if (ch == 'a') { LED(0); printf("LED ON\r\n"); i++; }
                else if (ch == 'b') { LED(1); printf("LED OFF\r\n"); i++; }
                else if (ch == 'c') { LED_TOGGLE(); printf("LED TOGGLE\r\n"); i++; }
                else if (ch == 'd') { printf("LED:%s,Sensor:%d\r\n", HAL_GPIO_ReadPin(LED_GPIO_PORT, LED_GPIO_PIN) ? "OFF" : "ON", app_menu_get_sensor_index() + 1); i++; }
                else { i++; }
            }
        }

        /* 扫描按键 */
        key = bsp_key_scan(0);

        /* 菜单处理 */
        app_menu_process(key);

        HAL_Delay(10);
    }
}
