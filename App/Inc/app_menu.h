/**
 ****************************************************************************************************
 * @file        app_menu.h
 * @author      Autumn
 * @version     V3.0
 * @date        2026-06-25
 * @brief       传感器采集显示系统
 ****************************************************************************************************
 * @attention
 *
 * 0.96寸 OLED 128x64, 12号/24号字体
 * 按键: KEY1=返回, KEY2=下移, KEY3=上移, KEY4=确认
 *
 ****************************************************************************************************
 */

#ifndef __APP_MENU_H
#define __APP_MENU_H

#include "stm32f1xx_hal.h"

/******************************************************************************************/
/* 配置 */

#define SENSOR_COUNT        10          /* 传感器数量 */
#define SENSOR_NAME_LEN     12          /* 传感器名称最大长度 */
#define CHART_WIDTH         128         /* 折线图宽度(像素) = 屏幕宽度 */
#define CHART_HEIGHT        32          /* 折线图高度(像素) */
#define CHART_Y_START       34          /* 折线图起始Y坐标 */
#define SENSOR_VAL_MIN      1.0f        /* 传感器最小值 */
#define SENSOR_VAL_MAX      20.0f       /* 传感器最大值 */

/******************************************************************************************/
/* 页面枚举 */

typedef enum
{
    PAGE_WELCOME = 0,       /* 欢迎界面 */
    PAGE_SENSOR_LIST,       /* 传感器列表 */
    PAGE_SENSOR_DETAIL,     /* 传感器详情(数据+折线图) */
    PAGE_MAX
} menu_page_t;

/******************************************************************************************/
/* 传感器数据结构 */

typedef struct
{
    char     name[SENSOR_NAME_LEN];     /* 传感器名称 */
    float    cur_value;                 /* 当前值 */
    uint8_t  history[CHART_WIDTH];      /* 历史数据(映射到0~CHART_HEIGHT) */
    uint16_t history_cnt;               /* 已记录的历史数据数量 */
} sensor_data_t;

/******************************************************************************************/
/* 函数声明 */

void app_menu_init(void);
void app_menu_process(uint8_t key);
void app_menu_goto_sensor(uint8_t idx);

#endif
