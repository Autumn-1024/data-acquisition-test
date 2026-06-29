/**
 ****************************************************************************************************
 * @file        bsp_sensor_inductive.h
 * @author      Autumn
 * @version     V1.0
 * @date        2026-06-29
 * @brief       4号传感器 - 电感式传感器（漫反射光电开关 E3F-DS30）
 ****************************************************************************************************
 * @attention
 *
 * PA5 (GPIO 输入，内部上拉，NPN 常开)
 * 低电平=检测到遮挡，高电平=无遮挡
 * 输出: 1.0=有遮挡, 0.0=无遮挡
 *
 ****************************************************************************************************
 */

#ifndef __BSP_SENSOR_INDUCTIVE_H
#define __BSP_SENSOR_INDUCTIVE_H

#include "stm32f1xx_hal.h"

void bsp_inductive_init(void);
float bsp_inductive_read(void);

#endif
