/**
 ****************************************************************************************************
 * @file        bsp_sensor_ultrasonic.h
 * @author      Autumn
 * @version     V1.0
 * @date        2026-06-29
 * @brief       超声波传感器驱动 (HC-SR04)
 ****************************************************************************************************
 * @attention
 *
 * TRIG = PA3 (推挽输出)
 * ECHO = PA2 (浮空输入)
 * TIM2 计时，分频 35 → 2MHz → 0.5μs 精度
 * 测量范围: 2~300cm
 *
 ****************************************************************************************************
 */

#ifndef __BSP_SENSOR_ULTRASONIC_H
#define __BSP_SENSOR_ULTRASONIC_H

#include "stm32f1xx_hal.h"

void bsp_ultrasonic_init(void);
float bsp_ultrasonic_read(void);

#endif
