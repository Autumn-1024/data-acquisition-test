/**
 ****************************************************************************************************
 * @file        bsp_sensor_capacitive.h
 * @author      Autumn
 * @version     V1.0
 * @date        2026-06-29
 * @brief       3号传感器 - 电容式传感器（接近开关）
 ****************************************************************************************************
 * @attention
 *
 * PA5 (GPIO 输入，内部上拉)
 * NPN: 低电平=检测到物体，高电平=无物体
 * 输出: 1.0=有物体, 0.0=无物体
 *
 ****************************************************************************************************
 */

#ifndef __BSP_SENSOR_CAPACITIVE_H
#define __BSP_SENSOR_CAPACITIVE_H

#include "stm32f1xx_hal.h"

void bsp_capacitive_init(void);
float bsp_capacitive_read(void);

#endif
