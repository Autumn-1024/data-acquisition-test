/**
 ****************************************************************************************************
 * @file        bsp_sensor_ds18b20.c
 * @author      Autumn
 * @version     V1.0
 * @date        2026-06-29
 * @brief       7号传感器 - DS18B20 温度传感器 — HAL 库版本
 ****************************************************************************************************
 * @attention
 *
 * PA5 (单总线，推挽输出+浮空输入切换)
 * 测量范围: -55°C ~ +125°C
 * 输出: 温度值 °C
 *
 ****************************************************************************************************
 */

#include "bsp_sensor_ds18b20.h"

/******************************************************************************************/
/* 配置 */

#define DS18B20_PORT    GPIOA
#define DS18B20_PIN     GPIO_PIN_5

#define CMD_SKIP_ROM    0xCC
#define CMD_CONVERT     0x44
#define CMD_READ        0xBE

/******************************************************************************************/
/* 微秒延时 (72MHz，循环延时，与原始工程一致) */

static void delay_us(uint32_t us)
{
    uint32_t i;
    while (us--)
    {
        i = 72;
        while (i--);
    }
}

/******************************************************************************************/
/* GPIO 模式切换 */

static void ds18b20_set_output(void)
{
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin   = DS18B20_PIN;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DS18B20_PORT, &gpio);
}

static void ds18b20_set_input(void)
{
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin  = DS18B20_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DS18B20_PORT, &gpio);
}

/******************************************************************************************/
/* 复位 */

static uint8_t ds18b20_reset(void)
{
    uint8_t ack = 0;

    ds18b20_set_output();
    HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
    delay_us(500);

    HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_SET);
    delay_us(60);

    ds18b20_set_input();
    if (HAL_GPIO_ReadPin(DS18B20_PORT, DS18B20_PIN) == GPIO_PIN_RESET)
    {
        ack = 1;
        while (HAL_GPIO_ReadPin(DS18B20_PORT, DS18B20_PIN) == GPIO_PIN_RESET);
    }

    ds18b20_set_output();
    HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_SET);
    delay_us(200);

    return ack;
}

/******************************************************************************************/
/* 写一位 */

static void ds18b20_write_bit(uint8_t bit)
{
    ds18b20_set_output();
    HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
    delay_us(2);

    if (bit)
        HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_SET);
    delay_us(60);

    HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_SET);
    delay_us(2);
}

/******************************************************************************************/
/* 写一个字节 */

static void ds18b20_write_byte(uint8_t byte)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        ds18b20_write_bit(byte & 0x01);
        byte >>= 1;
    }
}

/******************************************************************************************/
/* 读一位 */

static uint8_t ds18b20_read_bit(void)
{
    uint8_t bit = 0;

    ds18b20_set_output();
    HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
    delay_us(2);

    HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_SET);
    delay_us(1);
    ds18b20_set_input();

    if (HAL_GPIO_ReadPin(DS18B20_PORT, DS18B20_PIN) == GPIO_PIN_SET)
        bit = 1;
    delay_us(60);

    return bit;
}

/******************************************************************************************/
/* 读一个字节 */

static uint8_t ds18b20_read_byte(void)
{
    uint8_t i, byte = 0;
    for (i = 0; i < 8; i++)
    {
        byte >>= 1;
        if (ds18b20_read_bit())
            byte |= 0x80;
    }
    return byte;
}

/******************************************************************************************/
/* 初始化 */

void bsp_ds18b20_init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /* DS18B20 无需特殊初始化，复位已包含在读取流程中 */
}

/******************************************************************************************/
/* 读取温度 (°C) */

float bsp_ds18b20_read(void)
{
    uint8_t temp_low, temp_high;
    uint16_t temp_raw;
    float temp;

    /* 1. 复位 */
    if (ds18b20_reset() == 0)
        return -999.0f;

    /* 2. Skip ROM → 启动转换 */
    ds18b20_write_byte(CMD_SKIP_ROM);
    ds18b20_write_byte(CMD_CONVERT);
    HAL_Delay(750);

    /* 3. 复位 → Skip ROM → 读温度 */
    ds18b20_reset();
    ds18b20_write_byte(CMD_SKIP_ROM);
    ds18b20_write_byte(CMD_READ);

    /* 4. 读温度寄存器 */
    temp_low  = ds18b20_read_byte();
    temp_high = ds18b20_read_byte();

    /* 5. 转换 (12位精度) */
    temp_raw = (temp_high << 8) | temp_low;
    if ((temp_raw & 0xF800) == 0xF800)
    {
        temp_raw = ~temp_raw + 1;
        temp = -((float)temp_raw / 16.0f);
    }
    else
    {
        temp = (float)temp_raw / 16.0f;
    }

    return temp;
}
