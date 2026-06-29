/**
 ****************************************************************************************************
 * @file        bsp_sensor_linear_hall.h
 * @author      Autumn
 * @version     V1.0
 * @date        2026-06-29
 * @brief       8号传感器 - 磁电式传感器（线性霍尔）
 ****************************************************************************************************
 * @attention
 *
 * PA5 (ADC1_CH5) 模拟输入
 * 输出磁场强度 G（高斯）
 *
 ****************************************************************************************************
 */

#ifndef __BSP_SENSOR_LINEAR_HALL_H
#define __BSP_SENSOR_LINEAR_HALL_H

#include "stm32f1xx_hal.h"

void bsp_linear_hall_init(void);
float bsp_linear_hall_read(void);

#endif
