/**
 ****************************************************************************************************
 * @file        bsp_sensor_ultrasonic.c
 * @author      Autumn
 * @version     V1.0
 * @date        2026-06-29
 * @brief       超声波传感器驱动 (HC-SR04) — HAL 库版本
 ****************************************************************************************************
 * @attention
 *
 * TRIG = PA3 (推挽输出)
 * ECHO = PA2 (浮空输入)
 * TIM2 计时，分频 35 → 2MHz → 0.5μs 精度
 * 测量范围: 2~300cm
 *
 ****************************************************************************************************
 */

#include "bsp_sensor_ultrasonic.h"

/******************************************************************************************/
/* 引脚定义 */

#define TRIG_PORT       GPIOA
#define TRIG_PIN        GPIO_PIN_3
#define ECHO_PORT       GPIOA
#define ECHO_PIN        GPIO_PIN_2

#define US_TIM          TIM2
#define SOUND_VELOCITY  0.034f      /* 声速 cm/μs */

/******************************************************************************************/
/* 微秒延时 (72MHz 主频，循环延时，与原始工程一致) */

static void delay_us(uint32_t us)
{
    uint32_t i;
    while (us--)
    {
        i = 72;                     /* 72MHz → 约 72 次循环/μs */
        while (i--);
    }
}

/******************************************************************************************/
/* TIM 句柄 (全局，读取时需要访问) */

static TIM_HandleTypeDef htim;

/******************************************************************************************/
/* 初始化 */

void bsp_ultrasonic_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    /* 使能时钟 */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_TIM2_CLK_ENABLE();

    /* TRIG (PA3): 推挽输出，初始低电平 */
    gpio.Pin   = TRIG_PIN;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(TRIG_PORT, &gpio);
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);

    /* ECHO (PA2): 浮空输入 */
    gpio.Pin  = ECHO_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ECHO_PORT, &gpio);

    /* TIM2: 分频 35 → 72MHz/(35+1)=2MHz → 0.5μs/计数 */
    htim.Instance               = US_TIM;
    htim.Init.Prescaler         = 35;
    htim.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim.Init.Period            = 0xFFFF;
    htim.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&htim);
    HAL_TIM_Base_Start(&htim);
}

/******************************************************************************************/
/* 读取距离 (cm) */

float bsp_ultrasonic_read(void)
{
    uint32_t timeout;
    uint32_t time_us;

    /* 1. 发送 10μs TRIG 脉冲 */
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET);
    delay_us(10);
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);

    /* 2. 等待 ECHO 上升沿 */
    timeout = 0;
    while (HAL_GPIO_ReadPin(ECHO_PORT, ECHO_PIN) == GPIO_PIN_RESET)
    {
        if (++timeout > 100000) return -1.0f;
    }

    /* 3. 开始计时，等待 ECHO 下降沿 */
    __HAL_TIM_SET_COUNTER(&htim, 0);
    while (HAL_GPIO_ReadPin(ECHO_PORT, ECHO_PIN) == GPIO_PIN_SET)
    {
        if (__HAL_TIM_GET_COUNTER(&htim) > 30000) return -2.0f;
    }
    time_us = __HAL_TIM_GET_COUNTER(&htim);

    /* 4. 计算距离: 时间(μs) × 声速(cm/μs) / 2 */
    return (float)time_us * SOUND_VELOCITY / 2.0f;
}
