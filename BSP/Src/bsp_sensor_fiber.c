/**
 ****************************************************************************************************
 * @file        bsp_sensor_fiber.c
 * @author      Autumn
 * @version     V1.0
 * @date        2026-06-29
 * @brief       11号传感器 - 光纤传感器 — HAL 库版本
 ****************************************************************************************************
 * @attention
 *
 * PA5 (GPIO 输入，内部上拉，NPN 常开)
 * 低电平=检测到物体，高电平=无物体
 * 输出: 1.0=有物体, 0.0=无物体
 *
 ****************************************************************************************************
 */

#include "bsp_sensor_fiber.h"

#define FIBER_PORT  GPIOA
#define FIBER_PIN   GPIO_PIN_5

void bsp_fiber_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    gpio.Pin  = FIBER_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(FIBER_PORT, &gpio);
}

float bsp_fiber_read(void)
{
    if (HAL_GPIO_ReadPin(FIBER_PORT, FIBER_PIN) == GPIO_PIN_RESET)
        return 1.0f;
    else
        return 0.0f;
}
