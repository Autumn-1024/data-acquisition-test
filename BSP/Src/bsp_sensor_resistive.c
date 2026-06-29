/**
 ****************************************************************************************************
 * @file        bsp_sensor_resistive.c
 * @author      Autumn
 * @version     V1.0
 * @date        2026-06-29
 * @brief       1号传感器 - 电阻式传感器（米朗KTR裂缝计）— HAL 库版本
 ****************************************************************************************************
 * @attention
 *
 * PA4 (ADC1_CH4) 模拟输入
 * 输出电压 V，7次采样去极值取平均
 *
 ****************************************************************************************************
 */

#include "bsp_sensor_resistive.h"

/******************************************************************************************/
/* 配置 */

#define KTR_ADC_CHANNEL     ADC_CHANNEL_4
#define KTR_ADC_GPIO_PORT   GPIOA
#define KTR_ADC_GPIO_PIN    GPIO_PIN_4

#define KTR_REF_VOLTAGE     3.3f
#define SAMPLE_COUNT        7
#define SKIP_EDGE_NUM       1

/******************************************************************************************/
/* ADC 句柄 */

static ADC_HandleTypeDef hadc;

/******************************************************************************************/
/* 短延时（采样间隔用） */

static void delay_short(uint32_t n)
{
    while (n--);
}

/******************************************************************************************/
/* 初始化 */

void bsp_resistive_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_ADC_CONFIG(RCC_ADCPCLK2_DIV6);   /* ADC时钟=72MHz/6=12MHz，与原始工程一致 */

    /* PA4: 模拟输入 */
    gpio.Pin  = KTR_ADC_GPIO_PIN;
    gpio.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(KTR_ADC_GPIO_PORT, &gpio);

    /* ADC1 配置 */
    hadc.Instance                   = ADC1;
    hadc.Init.ScanConvMode          = ADC_SCAN_DISABLE;
    hadc.Init.ContinuousConvMode    = DISABLE;
    hadc.Init.DiscontinuousConvMode = DISABLE;
    hadc.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    hadc.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    hadc.Init.NbrOfConversion       = 1;
    HAL_ADC_Init(&hadc);

    /* 校准 */
    HAL_ADCEx_Calibration_Start(&hadc);
}

/******************************************************************************************/
/* 单次 ADC 读取 */

static uint16_t adc_read_single(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel      = KTR_ADC_CHANNEL;
    sConfig.Rank         = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc, &sConfig);

    HAL_ADC_Start(&hadc);
    HAL_ADC_PollForConversion(&hadc, 10);
    return (uint16_t)HAL_ADC_GetValue(&hadc);
}

/******************************************************************************************/
/* 读取电压 (V) - 7次采样去极值取平均 */

float bsp_resistive_read(void)
{
    uint16_t adc_buf[SAMPLE_COUNT];
    float volt_buf[SAMPLE_COUNT];
    uint32_t adc_sum = 0;
    float volt_sum = 0.0f;
    uint8_t valid_num = 0;
    uint8_t i, j;

    /* 1. 连续采集 7 次 */
    for (i = 0; i < SAMPLE_COUNT; i++)
    {
        adc_buf[i] = adc_read_single();
        volt_buf[i] = (float)adc_buf[i] * KTR_REF_VOLTAGE / 4096.0f;
        delay_short(10000);
    }

    /* 2. 冒泡排序（从小到大） */
    for (i = 0; i < SAMPLE_COUNT - 1; i++)
    {
        for (j = 0; j < SAMPLE_COUNT - 1 - i; j++)
        {
            if (adc_buf[j] > adc_buf[j + 1])
            {
                uint16_t tmp_adc = adc_buf[j];
                adc_buf[j] = adc_buf[j + 1];
                adc_buf[j + 1] = tmp_adc;

                float tmp_volt = volt_buf[j];
                volt_buf[j] = volt_buf[j + 1];
                volt_buf[j + 1] = tmp_volt;
            }
        }
    }

    /* 3. 取中间 5 个值（跳过首尾各 1 个） */
    for (i = SKIP_EDGE_NUM; i < SAMPLE_COUNT - SKIP_EDGE_NUM; i++)
    {
        adc_sum += adc_buf[i];
        volt_sum += volt_buf[i];
        valid_num++;
    }

    if (valid_num == 0) return 0.0f;
    return volt_sum / valid_num;
}
