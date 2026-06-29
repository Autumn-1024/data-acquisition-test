/**
 ****************************************************************************************************
 * @file        bsp_sensor_linear_hall.c
 * @author      Autumn
 * @version     V1.0
 * @date        2026-06-29
 * @brief       8号传感器 - 磁电式传感器（线性霍尔）— HAL 库版本
 ****************************************************************************************************
 * @attention
 *
 * PA5 (ADC1_CH5) 模拟输入
 * 输出磁场强度 G（高斯）
 *
 ****************************************************************************************************
 */

#include "bsp_sensor_linear_hall.h"

/******************************************************************************************/
/* 配置 */

#define HALL_ADC_CHANNEL    ADC_CHANNEL_5
#define HALL_ADC_GPIO_PORT  GPIOA
#define HALL_ADC_GPIO_PIN   GPIO_PIN_5

#define HALL_POWER_VOLTAGE  3.3f
#define HALL_MID_VOLTAGE    1.65f
#define HALL_SENSITIVITY    0.0014f

/******************************************************************************************/
/* ADC 句柄 */

static ADC_HandleTypeDef hadc;

/******************************************************************************************/
/* 初始化 */

void bsp_linear_hall_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_ADC_CONFIG(RCC_ADCPCLK2_DIV6);

    /* PA5: 模拟输入 */
    gpio.Pin  = HALL_ADC_GPIO_PIN;
    gpio.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(HALL_ADC_GPIO_PORT, &gpio);

    /* ADC1 配置 */
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

/******************************************************************************************/
/* 读取磁场强度 (G) */

float bsp_linear_hall_read(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    uint16_t adc_val;
    float voltage, gauss;

    sConfig.Channel      = HALL_ADC_CHANNEL;
    sConfig.Rank         = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc, &sConfig);

    HAL_ADC_Start(&hadc);
    HAL_ADC_PollForConversion(&hadc, 10);
    adc_val = (uint16_t)HAL_ADC_GetValue(&hadc);

    voltage = (float)adc_val * HALL_POWER_VOLTAGE / 4096.0f;
    gauss = (voltage - HALL_MID_VOLTAGE) / HALL_SENSITIVITY;

    return gauss;
}
