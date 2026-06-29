/**
 ****************************************************************************************************
 * @file        bsp_sensor_eddy.c
 * @author      Autumn
 * @version     V1.0
 * @date        2026-06-29
 * @brief       6号传感器 - 涡流式传感器（电感式接近开关）— HAL 库版本
 ****************************************************************************************************
 * @attention
 *
 * PA5 (GPIO 输入，内部上拉)
 * 低电平=检测到金属，高电平=无金属
 * 输出: 1.0=有金属, 0.0=无金属
 *
 ****************************************************************************************************
 */

#include "bsp_sensor_eddy.h"

#define SENSOR_PORT GPIOA
#define SENSOR_PIN  GPIO_PIN_5

void bsp_eddy_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    gpio.Pin  = SENSOR_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(SENSOR_PORT, &gpio);
}

/**
 * @brief  读取涡流式传感器状态
 * @retval 1.0=检测到金属, 0.0=无金属
 */
float bsp_eddy_read(void)
{
    if (HAL_GPIO_ReadPin(SENSOR_PORT, SENSOR_PIN) == GPIO_PIN_RESET)
        return 1.0f;
    else
        return 0.0f;
}
