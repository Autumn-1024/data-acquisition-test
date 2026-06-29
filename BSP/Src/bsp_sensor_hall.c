/**
 ****************************************************************************************************
 * @file        bsp_sensor_hall.c
 * @author      Autumn
 * @version     V1.0
 * @date        2026-06-29
 * @brief       2号传感器 - 霍尔传感器（开关型）— HAL 库版本
 ****************************************************************************************************
 * @attention
 *
 * PA5 (GPIO 输入，内部上拉)
 * 低电平=检测到磁铁，高电平=无磁铁
 * 输出: 1.0=有磁铁, 0.0=无磁铁
 *
 ****************************************************************************************************
 */

#include "bsp_sensor_hall.h"

#define HALL_PORT   GPIOA
#define HALL_PIN    GPIO_PIN_5

void bsp_hall_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    gpio.Pin  = HALL_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(HALL_PORT, &gpio);
}

/**
 * @brief  读取霍尔传感器状态
 * @retval 1.0=检测到磁铁, 0.0=无磁铁
 */
float bsp_hall_read(void)
{
    if (HAL_GPIO_ReadPin(HALL_PORT, HALL_PIN) == GPIO_PIN_RESET)
        return 1.0f;    /* 低电平 = 检测到磁铁 */
    else
        return 0.0f;    /* 高电平 = 无磁铁 */
}
