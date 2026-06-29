/**
 ****************************************************************************************************
 * @file        bsp_sensor_pressure.c
 * @author      Autumn
 * @version     V1.0
 * @date        2026-06-29
 * @brief       10号传感器 - 压力传感器（风压变送器）— HAL 库版本
 ****************************************************************************************************
 * @attention
 *
 * PA4 (ADC1_CH4) 模拟输入
 * 4-20mA 模拟量，16次采样剔除极值平均
 * 输出风压 kPa
 *
 ****************************************************************************************************
 */

#include "bsp_sensor_pressure.h"

#define PRESSURE_ADC_CHANNEL    ADC_CHANNEL_4
#define PRESSURE_GPIO_PORT      GPIOA
#define PRESSURE_GPIO_PIN       GPIO_PIN_4

#define ADC_REF_VOLTAGE         3.3f
#define MEASURE_RANGE           10.0f
#define CURRENT_4MA_V           0.66f
#define CURRENT_20MA_V          3.3f
#define PRESSURE_OFFSET         2.0f
#define ATMOSPHERIC_PRESSURE    1.0f
#define ADC_SAMPLE_TIMES        16

static ADC_HandleTypeDef hadc;

void bsp_pressure_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_ADC_CONFIG(RCC_ADCPCLK2_DIV6);

    gpio.Pin  = PRESSURE_GPIO_PIN;
    gpio.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(PRESSURE_GPIO_PORT, &gpio);

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

static uint16_t adc_read_raw(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel      = PRESSURE_ADC_CHANNEL;
    sConfig.Rank         = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc, &sConfig);

    HAL_ADC_Start(&hadc);
    HAL_ADC_PollForConversion(&hadc, 10);
    return (uint16_t)HAL_ADC_GetValue(&hadc);
}

static float adc_read_voltage(void)
{
    float buf[ADC_SAMPLE_TIMES];
    float max_val, min_val, voltage = 0.0f;
    uint8_t i;
    uint32_t tmp;

    for (i = 0; i < ADC_SAMPLE_TIMES; i++)
    {
        tmp = adc_read_raw();
        buf[i] = (float)tmp * ADC_REF_VOLTAGE / 4096.0f;
    }

    max_val = buf[0];
    min_val = buf[0];
    for (i = 0; i < ADC_SAMPLE_TIMES; i++)
    {
        if (buf[i] > max_val) max_val = buf[i];
        if (buf[i] < min_val) min_val = buf[i];
        voltage += buf[i];
    }
    voltage = (voltage - max_val - min_val) / (ADC_SAMPLE_TIMES - 2);

    return voltage;
}

/**
 * @brief  读取风压 (kPa)
 */
float bsp_pressure_read(void)
{
    float voltage = adc_read_voltage();
    float wind_pressure, actual_pressure;

    wind_pressure = (voltage - CURRENT_4MA_V) / (CURRENT_20MA_V - CURRENT_4MA_V) * MEASURE_RANGE;
    wind_pressure -= PRESSURE_OFFSET;
    if (wind_pressure < 0.0f) wind_pressure = 0.0f;
    if (wind_pressure > MEASURE_RANGE) wind_pressure = MEASURE_RANGE;

    actual_pressure = wind_pressure - ATMOSPHERIC_PRESSURE;
    if (actual_pressure < 0.0f) actual_pressure = 0.0f;

    return actual_pressure;
}
