/**
 ****************************************************************************************************
 * @file        bsp_sensor_alcohol.h
 * @author      Autumn
 * @version     V1.0
 * @date        2026-06-29
 * @brief       13号传感器 - 气敏传感器（酒精 MQ-3）
 ****************************************************************************************************
 * @attention
 *
 * PA4 (ADC1_CH4) 模拟输入
 * 输出 ADC 原始值
 *
 ****************************************************************************************************
 */

#ifndef __BSP_SENSOR_ALCOHOL_H
#define __BSP_SENSOR_ALCOHOL_H

#include "stm32f1xx_hal.h"

void bsp_alcohol_init(void);
float bsp_alcohol_read(void);

#endif
