/**
 ****************************************************************************************************
 * @file        bsp_sensor_photoelec.c
 * @author      Autumn
 * @version     V1.0
 * @date        2026-06-29
 * @brief       5号传感器 - 光电式传感器 — HAL 库版本
 ****************************************************************************************************
 * @attention
 *
 * PA5 (GPIO 输入，内部上拉)
 * 低电平=检测到物体，高电平=未检测到
 * 输出: 1.0=有物体, 0.0=无物体
 *
 ****************************************************************************************************
 */

#include "bsp_sensor_photoelec.h"

#define SENSOR_PORT GPIOA
#define SENSOR_PIN  GPIO_PIN_5

void bsp_photoelec_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    gpio.Pin  = SENSOR_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(SENSOR_PORT, &gpio);
}

/**
 * @brief  读取光电传感器状态
 * @retval 1.0=检测到物体, 0.0=无物体
 */
float bsp_photoelec_read(void)
{
    if (HAL_GPIO_ReadPin(SENSOR_PORT, SENSOR_PIN) == GPIO_PIN_RESET)
        return 1.0f;
    else
        return 0.0f;
}
