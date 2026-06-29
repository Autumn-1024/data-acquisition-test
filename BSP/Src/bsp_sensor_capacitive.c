/**
 ****************************************************************************************************
 * @file        bsp_sensor_capacitive.c
 * @author      Autumn
 * @version     V1.0
 * @date        2026-06-29
 * @brief       3号传感器 - 电容式传感器（接近开关）— HAL 库版本
 ****************************************************************************************************
 * @attention
 *
 * PA5 (GPIO 输入，内部上拉，NPN 模式)
 * 低电平=检测到物体，高电平=无物体
 * 输出: 1.0=有物体, 0.0=无物体
 *
 ****************************************************************************************************
 */

#include "bsp_sensor_capacitive.h"

#define CAP_PORT    GPIOA
#define CAP_PIN     GPIO_PIN_5

void bsp_capacitive_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    gpio.Pin  = CAP_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;        /* NPN 模式用上拉 */
    HAL_GPIO_Init(CAP_PORT, &gpio);
}

/**
 * @brief  读取电容式传感器状态
 * @retval 1.0=检测到物体, 0.0=无物体
 */
float bsp_capacitive_read(void)
{
    if (HAL_GPIO_ReadPin(CAP_PORT, CAP_PIN) == GPIO_PIN_RESET)
        return 1.0f;    /* 低电平 = 检测到物体 */
    else
        return 0.0f;    /* 高电平 = 无物体 */
}
