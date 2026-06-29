/**
 ****************************************************************************************************
 * @file        bsp_sensor_piezo.c
 * @author      Autumn
 * @version     V1.0
 * @date        2026-06-29
 * @brief       9号传感器 - 压电式振动传感器 — HAL 库版本
 ****************************************************************************************************
 * @attention
 *
 * PA4 (ADC1_CH4) 模拟输入
 * 5次采样平均，输出电压 V
 *
 ****************************************************************************************************
 */

#include "bsp_sensor_piezo.h"

#define PIEZO_ADC_CHANNEL   ADC_CHANNEL_4
#define PIEZO_GPIO_PORT     GPIOA
#define PIEZO_GPIO_PIN      GPIO_PIN_4

#define PIEZO_REF_VOLTAGE   3.3f
#define SAMPLE_AVG_COUNT    5

static ADC_HandleTypeDef hadc;

static void delay_short(uint32_t n)
{
    while (n--);
}

void bsp_piezo_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_ADC_CONFIG(RCC_ADCPCLK2_DIV6);

    gpio.Pin  = PIEZO_GPIO_PIN;
    gpio.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(PIEZO_GPIO_PORT, &gpio);

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

static uint16_t adc_read_single(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel      = PIEZO_ADC_CHANNEL;
    sConfig.Rank         = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc, &sConfig);

    HAL_ADC_Start(&hadc);
    HAL_ADC_PollForConversion(&hadc, 10);
    return (uint16_t)HAL_ADC_GetValue(&hadc);
}

/**
 * @brief  读取电压 V（5次采样平均）
 */
float bsp_piezo_read(void)
{
    uint32_t adc_sum = 0;
    uint8_t i;

    for (i = 0; i < SAMPLE_AVG_COUNT; i++)
    {
        adc_sum += adc_read_single();
        delay_short(1000);
    }

    return (float)(adc_sum / SAMPLE_AVG_COUNT) * PIEZO_REF_VOLTAGE / 4096.0f;
}
