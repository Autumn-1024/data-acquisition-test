/**
 ****************************************************************************************************
 * @file        bsp_sensor_resistive.h
 * @author      Autumn
 * @version     V1.0
 * @date        2026-06-29
 * @brief       1号传感器 - 电阻式传感器（米朗KTR裂缝计）
 ****************************************************************************************************
 * @attention
 *
 * PA4 (ADC1_CH4) 模拟输入
 * 输出电压 V，7次采样去极值取平均
 *
 ****************************************************************************************************
 */

#ifndef __BSP_SENSOR_RESISTIVE_H
#define __BSP_SENSOR_RESISTIVE_H

#include "stm32f1xx_hal.h"

void bsp_resistive_init(void);
float bsp_resistive_read(void);

#endif
