/**
 ****************************************************************************************************
 * @file        bsp_uart.c
 * @author      Autumn
 * @version     V1.0
 * @date        2026-06-16
 * @brief       USART1 调试串口驱动 (PA9/PA10, printf重定向)
 ****************************************************************************************************
 */

#include "bsp_uart.h"

UART_HandleTypeDef g_uart_handle;

/* 环形缓冲区 */
#define RX_BUF_SIZE     64
static uint8_t rx_buf[RX_BUF_SIZE];
static volatile uint16_t rx_head = 0;  /* 写入位置 (中断) */
static volatile uint16_t rx_tail = 0;  /* 读取位置 (主循环) */

/**
 * @brief       USART1初始化
 * @param       bound: 波特率
 * @retval      无
 */
void bsp_uart_init(uint32_t bound)
{
    GPIO_InitTypeDef gpio_init_struct;

    DEBUG_TX_GPIO_CLK_ENABLE();
    DEBUG_RX_GPIO_CLK_ENABLE();
    DEBUG_USART_CLK_ENABLE();

    /* TX - 复用推挽输出 */
    gpio_init_struct.Pin   = DEBUG_TX_GPIO_PIN;
    gpio_init_struct.Mode  = GPIO_MODE_AF_PP;
    gpio_init_struct.Pull  = GPIO_PULLUP;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DEBUG_TX_GPIO_PORT, &gpio_init_struct);

    /* RX - 浮空输入 */
    gpio_init_struct.Pin  = DEBUG_RX_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_INPUT;
    gpio_init_struct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DEBUG_RX_GPIO_PORT, &gpio_init_struct);

    /* USART配置 */
    g_uart_handle.Instance          = DEBUG_USART;
    g_uart_handle.Init.BaudRate     = bound;
    g_uart_handle.Init.WordLength   = UART_WORDLENGTH_8B;
    g_uart_handle.Init.StopBits     = UART_STOPBITS_1;
    g_uart_handle.Init.Parity       = UART_PARITY_NONE;
    g_uart_handle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    g_uart_handle.Init.Mode         = UART_MODE_TX_RX;
    HAL_UART_Init(&g_uart_handle);

    /* 启动中断接收 */
    HAL_NVIC_SetPriority(USART1_IRQn, 3, 3);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
    HAL_UART_Receive_IT(&g_uart_handle, &rx_buf[0], 1);
}

/**
 * @brief       UART接收中断回调
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        uint16_t next = (rx_head + 1) % RX_BUF_SIZE;
        if (next != rx_tail)
        {
            rx_head = next;
        }
        /* 继续接收下一个字节 */
        HAL_UART_Receive_IT(&g_uart_handle, &rx_buf[rx_head], 1);
    }
}

/**
 * @brief       USART1中断服务函数
 */
void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&g_uart_handle);
}

/**
 * @brief       重定向fputc到USART1 (支持printf)
 */
int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&g_uart_handle, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

/**
 * @brief       重定向fgetc到USART1 (支持scanf)
 */
int fgetc(FILE *f)
{
    uint8_t ch = 0;
    HAL_UART_Receive(&g_uart_handle, &ch, 1, HAL_MAX_DELAY);
    return (int)ch;
}

/**
 * @brief       检查串口是否有数据可读
 * @retval      1=有数据, 0=无数据
 */
uint8_t bsp_uart_rx_ready(void)
{
    return (rx_head != rx_tail) ? 1 : 0;
}

/**
 * @brief       读取一个字节(非阻塞)
 * @retval      接收到的字节
 */
uint8_t bsp_uart_read_char(void)
{
    uint8_t ch = 0;
    if (rx_head != rx_tail)
    {
        ch = rx_buf[rx_tail];
        rx_tail = (rx_tail + 1) % RX_BUF_SIZE;
    }
    return ch;
}

/**
 * @brief       读取缓冲区中的所有可用字节
 * @param       buf:    输出缓冲区
 * @param       max_len: 最大读取长度
 * @retval      实际读取的字节数
 */
uint16_t bsp_uart_read_buf(uint8_t *buf, uint16_t max_len)
{
    uint16_t len = 0;
    while (rx_head != rx_tail && len < max_len)
    {
        buf[len++] = rx_buf[rx_tail];
        rx_tail = (rx_tail + 1) % RX_BUF_SIZE;
    }
    return len;
}
