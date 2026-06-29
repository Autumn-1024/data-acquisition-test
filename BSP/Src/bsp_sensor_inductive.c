/**
 ****************************************************************************************************
 * @file        bsp_sensor_inductive.c
 * @author      Autumn
 * @version     V1.0
 * @date        2026-06-29
 * @brief       4号传感器 - 电感式传感器（漫反射光电开关 E3F-DS30）— HAL 库版本
 ****************************************************************************************************
 * @attention
 *
 * PA5 (GPIO 输入，内部上拉，NPN 常开)
 * 低电平=检测到遮挡，高电平=无遮挡
 * 输出: 1.0=有遮挡, 0.0=无遮挡
 *
 ****************************************************************************************************
 */

#include "bsp_sensor_inductive.h"

#define SENSOR_PORT GPIOA
#define SENSOR_PIN  GPIO_PIN_5

void bsp_inductive_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    gpio.Pin  = SENSOR_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;        /* NPN 常开用上拉 */
    HAL_GPIO_Init(SENSOR_PORT, &gpio);
}

/**
 * @brief  读取电感式传感器状态
 * @retval 1.0=检测到遮挡, 0.0=无遮挡
 */
float bsp_inductive_read(void)
{
    if (HAL_GPIO_ReadPin(SENSOR_PORT, SENSOR_PIN) == GPIO_PIN_RESET)
        return 1.0f;    /* 低电平 = 检测到遮挡 */
    else
        return 0.0f;    /* 高电平 = 无遮挡 */
}
