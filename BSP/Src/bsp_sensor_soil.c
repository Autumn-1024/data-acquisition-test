/**
 ****************************************************************************************************
 * @file        bsp_sensor_soil.c
 * @author      Autumn
 * @version     V1.0
 * @date        2026-06-29
 * @brief       14号传感器 - 土壤湿度传感器 — HAL 库版本
 ****************************************************************************************************
 * @attention
 *
 * PA4 (ADC1_CH4) 模拟输入
 * 输出湿度百分比 (%)
 *
 ****************************************************************************************************
 */

#include "bsp_sensor_soil.h"

#define SOIL_ADC_CHANNEL    ADC_CHANNEL_4
#define SOIL_GPIO_PORT      GPIOA
#define SOIL_GPIO_PIN       GPIO_PIN_4

#define SOIL_REF_VOLTAGE    3.3f
#define DRY_VOLTAGE         3.2f
#define WET_VOLTAGE         0.8f

static ADC_HandleTypeDef hadc;

void bsp_soil_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_ADC_CONFIG(RCC_ADCPCLK2_DIV6);

    gpio.Pin  = SOIL_GPIO_PIN;
    gpio.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(SOIL_GPIO_PORT, &gpio);

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
 * @brief  读取土壤湿度 (%)
 * @retval 湿度百分比 0~100%
 */
float bsp_soil_read(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    uint16_t adc_val;
    float voltage, humidity;

    sConfig.Channel      = SOIL_ADC_CHANNEL;
    sConfig.Rank         = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc, &sConfig);

    HAL_ADC_Start(&hadc);
    HAL_ADC_PollForConversion(&hadc, 10);
    adc_val = (uint16_t)HAL_ADC_GetValue(&hadc);

    voltage = (float)adc_val * SOIL_REF_VOLTAGE / 4096.0f;
    humidity = (DRY_VOLTAGE - voltage) / (DRY_VOLTAGE - WET_VOLTAGE) * 100.0f;

    if (humidity < 0.0f) humidity = 0.0f;
    if (humidity > 100.0f) humidity = 100.0f;

    return humidity;
}
