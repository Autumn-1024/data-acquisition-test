/**
 ****************************************************************************************************
 * @file        bsp_sensor_photoelec.h
 * @author      Autumn
 * @version     V1.0
 * @date        2026-06-29
 * @brief       5号传感器 - 光电式传感器
 ****************************************************************************************************
 * @attention
 *
 * PA5 (GPIO 输入，内部上拉)
 * 低电平=检测到物体，高电平=未检测到
 * 输出: 1.0=有物体, 0.0=无物体
 *
 ****************************************************************************************************
 */

#ifndef __BSP_SENSOR_PHOTOELEC_H
#define __BSP_SENSOR_PHOTOELEC_H

#include "stm32f1xx_hal.h"

void bsp_photoelec_init(void);
float bsp_photoelec_read(void);

#endif
