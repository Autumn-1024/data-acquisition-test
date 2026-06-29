/**
 ****************************************************************************************************
 * @file        bsp_sensor_pressure.h
 * @author      Autumn
 * @version     V1.0
 * @date        2026-06-29
 * @brief       10号传感器 - 压力传感器（风压变送器）
 ****************************************************************************************************
 * @attention
 *
 * PA4 (ADC1_CH4) 模拟输入
 * 4-20mA 模拟量，16次采样剔除极值平均
 * 输出风压 kPa
 *
 ****************************************************************************************************
 */

#ifndef __BSP_SENSOR_PRESSURE_H
#define __BSP_SENSOR_PRESSURE_H

#include "stm32f1xx_hal.h"

void bsp_pressure_init(void);
float bsp_pressure_read(void);

#endif
