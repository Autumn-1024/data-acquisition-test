/**
 ****************************************************************************************************
 * @file        bsp_sensor_alcohol.c
 * @author      Autumn
 * @version     V1.0
 * @date        2026-06-29
 * @brief       13号传感器 - 气敏传感器（酒精 MQ-3）— HAL 库版本
 ****************************************************************************************************
 * @attention
 *
 * PA4 (ADC1_CH4) 模拟输入
 * 输出 ADC 原始值（0~4095）
 *
 ****************************************************************************************************
 */

#include "bsp_sensor_alcohol.h"

#define ALCOHOL_ADC_CHANNEL ADC_CHANNEL_4
#define ALCOHOL_GPIO_PORT   GPIOA
#define ALCOHOL_GPIO_PIN    GPIO_PIN_4

static ADC_HandleTypeDef hadc;

void bsp_alcohol_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_ADC_CONFIG(RCC_ADCPCLK2_DIV6);

    gpio.Pin  = ALCOHOL_GPIO_PIN;
    gpio.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(ALCOHOL_GPIO_PORT, &gpio);

    hadc.Instance                   = ADC1;
    hadc.Init.ScanConvMode          = ADC_SCAN_DISABLE;
    hadc.Init.ContinuousConvMode    = DISABLE;
    hadc.Init.DiscontinuousConvMode = DISABLE;
    hadc.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    hadc.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    hadc.Init.NbrOfConversion       = 1;
    HAL_ADC_Init(&hadc);

    HAL_ADCEx_Calibration_Start(&hadc);
}

/**
 * @brief  读取酒精传感器 ADC 值
 * @retval ADC 原始值 (0~4095)
 */
float bsp_alcohol_read(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel      = ALCOHOL_ADC_CHANNEL;
    sConfig.Rank         = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;   /* 与原始工程一致 */
    HAL_ADC_ConfigChannel(&hadc, &sConfig);

    HAL_ADC_Start(&hadc);
    HAL_ADC_PollForConversion(&hadc, 10);
    return (float)HAL_ADC_GetValue(&hadc);
}
