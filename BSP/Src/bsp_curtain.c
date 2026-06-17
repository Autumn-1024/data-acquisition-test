/**
 ****************************************************************************************************
 * @file        bsp_curtain.c
 * @author      Autumn
 * @version     V2.0
 * @date        2026-06-16
 * @brief       杜亚窗帘电机控制 (RS485协议)
 ****************************************************************************************************
 * @attention
 *
 * 协议帧格式:
 *   55 + 设备地址(2) + 功能码(1) + 数据地址(1) + 数据长度(1) + 数据(N) + CRC16(2)
 *
 * 预计算帧 (地址 0xFEFE, 参考F429工程验证):
 *   打开: 55 FE FE 03 01 B9 24
 *   关闭: 55 FE FE 03 02 F9 25
 *   停止: 55 FE FE 03 03 38 E5
 *   查询: 55 FE FE 01 00 79 84
 *
 ****************************************************************************************************
 */

#include "bsp_curtain.h"
#include "bsp_rs485.h"
#include <stdio.h>

/* 设备地址 */
static const uint8_t g_addr_h = CURTAIN_ADDR_H;
static const uint8_t g_addr_l = CURTAIN_ADDR_L;

/* 最近一次发送的命令 (供OLED显示) */
uint8_t g_curtain_last_cmd[16];
uint8_t g_curtain_last_cmd_len = 0;

/******************************************************************************************/
/* CRC16 计算 (CRC-16/MODBUS) */

/**
 * @brief       计算CRC16 (多项式 0xA001, 初始值 0xFFFF)
 * @param       data: 数据指针
 * @param       len : 数据长度
 * @retval      CRC16值
 */
static uint16_t curtain_crc16(const uint8_t *data, uint8_t len)
{
    uint16_t crc = 0xFFFF;
    uint8_t i, j;

    for (i = 0; i < len; i++)
    {
        crc ^= data[i];

        for (j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
            {
                crc = (crc >> 1) ^ 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return crc;
}

/******************************************************************************************/
/* 帧构建 */

/**
 * @brief       构建并发送控制帧 (功能码 0x03, 无数据)
 * @param       cmd: 控制命令字节 (0x01=开, 0x02=关, 0x03=停, 0x07=删行程, 0x08=恢复出厂)
 * @retval      帧长度
 */
static uint8_t curtain_build_ctrl_cmd(uint8_t cmd, uint8_t *buf)
{
    uint16_t crc;

    buf[0] = 0x55;
    buf[1] = g_addr_h;
    buf[2] = g_addr_l;
    buf[3] = CURTAIN_FUNC_CTRL;
    buf[4] = cmd;

    /* CRC校验: 从起始码到命令字节 (5字节) */
    crc = curtain_crc16(buf, 5);
    buf[5] = crc & 0xFF;        /* CRC低字节 */
    buf[6] = (crc >> 8) & 0xFF; /* CRC高字节 */

    return 7;
}

/**
 * @brief       构建并发送百分比控制帧 (功能码 0x03, 命令 0x04, 1字节数据)
 * @param       percent: 百分比 (0~100)
 * @retval      帧长度
 */
static uint8_t curtain_build_percent_cmd(uint8_t percent, uint8_t *buf)
{
    uint16_t crc;

    buf[0] = 0x55;
    buf[1] = g_addr_h;
    buf[2] = g_addr_l;
    buf[3] = CURTAIN_FUNC_CTRL;
    buf[4] = CURTAIN_CMD_PERCENT;
    buf[5] = percent;   /* 百分比值 */

    /* CRC校验: 从起始码到百分比值 (6字节) */
    crc = curtain_crc16(buf, 6);
    buf[6] = crc & 0xFF;
    buf[7] = (crc >> 8) & 0xFF;

    return 8;
}

/**
 * @brief       构建读寄存器帧 (功能码 0x01)
 * @param       reg_addr: 寄存器地址
 * @retval      帧长度
 */
static uint8_t curtain_build_read_cmd(uint8_t reg_addr, uint8_t *buf)
{
    uint16_t crc;

    buf[0] = 0x55;
    buf[1] = g_addr_h;
    buf[2] = g_addr_l;
    buf[3] = CURTAIN_FUNC_READ;
    buf[4] = reg_addr;
    buf[5] = 0x01;  /* 数据长度: 读1个寄存器 */

    /* CRC校验: 从起始码到数据长度 (6字节) */
    crc = curtain_crc16(buf, 6);
    buf[6] = crc & 0xFF;
    buf[7] = (crc >> 8) & 0xFF;

    return 8;
}

/******************************************************************************************/
/* 发送辅助 */

/**
 * @brief       保存命令到全局缓冲区并发送
 * @param       buf: 帧数据
 * @param       len: 帧长度
 * @retval      无
 */
static void curtain_send_frame(uint8_t *buf, uint8_t len)
{
    uint8_t i;

    /* 保存到全局缓冲区供显示 */
    for (i = 0; i < len && i < 16; i++)
    {
        g_curtain_last_cmd[i] = buf[i];
    }
    g_curtain_last_cmd_len = len;

    /* 调试打印 */
    printf("[TX] CURTAIN: ");
    for (i = 0; i < len; i++)
    {
        printf("%02X ", buf[i]);
    }
    printf("\r\n");

    /* 发送 */
    bsp_rs485_send_data(buf, len);
}

/******************************************************************************************/
/* 公开接口 */

/**
 * @brief       窗帘初始化
 */
void bsp_curtain_init(void)
{
    /* RS485已在bsp_rs485_init中初始化 */
}

/**
 * @brief       打开窗帘
 * @note        预计算帧: 55 FE FE 03 01 B9 24
 */
void bsp_curtain_open(void)
{
    uint8_t buf[8];
    uint8_t len = curtain_build_ctrl_cmd(CURTAIN_CMD_OPEN, buf);
    curtain_send_frame(buf, len);
}

/**
 * @brief       关闭窗帘
 * @note        预计算帧: 55 FE FE 03 02 F9 25
 */
void bsp_curtain_close(void)
{
    uint8_t buf[8];
    uint8_t len = curtain_build_ctrl_cmd(CURTAIN_CMD_CLOSE, buf);
    curtain_send_frame(buf, len);
}

/**
 * @brief       停止窗帘
 * @note        预计算帧: 55 FE FE 03 03 38 E5
 */
void bsp_curtain_stop(void)
{
    uint8_t buf[8];
    uint8_t len = curtain_build_ctrl_cmd(CURTAIN_CMD_STOP, buf);
    curtain_send_frame(buf, len);
}

/**
 * @brief       百分比控制
 * @param       percent: 0~100
 * @note        示例(30%): 55 FE FE 03 04 01 1E CRC_L CRC_H
 */
void bsp_curtain_set_percent(uint8_t percent)
{
    uint8_t buf[9];

    if (percent > 100) return;

    uint8_t len = curtain_build_percent_cmd(percent, buf);
    curtain_send_frame(buf, len);
}

/**
 * @brief       查询当前位置
 * @note        帧: 55 FE FE 01 02 01 CRC_L CRC_H
 */
void bsp_curtain_query_position(void)
{
    uint8_t buf[8];
    uint8_t len = curtain_build_read_cmd(CURTAIN_REG_POSITION, buf);
    curtain_send_frame(buf, len);
}

/**
 * @brief       查询电机运行状态
 * @note        帧: 55 FE FE 01 05 01 CRC_L CRC_H
 */
void bsp_curtain_query_status(void)
{
    uint8_t buf[8];
    uint8_t len = curtain_build_read_cmd(CURTAIN_REG_STATUS, buf);
    curtain_send_frame(buf, len);
}

/**
 * @brief       删除行程
 * @note        帧: 55 FE FE 03 07 00 CRC_L CRC_H
 */
void bsp_curtain_del_travel(void)
{
    uint8_t buf[8];
    uint8_t len = curtain_build_ctrl_cmd(CURTAIN_CMD_DEL_TRAVEL, buf);
    curtain_send_frame(buf, len);
}

/**
 * @brief       恢复出厂设置
 * @note        帧: 55 FE FE 03 08 00 CRC_L CRC_H
 *              ⚠️ 恢复后地址变为 0xFEFE，行程被删除
 */
void bsp_curtain_factory_reset(void)
{
    uint8_t buf[8];
    uint8_t len = curtain_build_ctrl_cmd(CURTAIN_CMD_RESET, buf);
    curtain_send_frame(buf, len);
}

/**
 * @brief       写设备地址 (广播方式)
 * @param       addr_h: 新地址高字节
 * @param       addr_l: 新地址低字节
 * @note        帧: 55 00 00 02 00 02 addr_h addr_l CRC
 *              需先按住电机设置键5秒进入分配模式
 */
void bsp_curtain_set_address(uint8_t addr_h, uint8_t addr_l)
{
    uint8_t buf[10];
    uint16_t crc;

    buf[0] = 0x55;
    buf[1] = 0x00;              /* 广播地址 */
    buf[2] = 0x00;
    buf[3] = CURTAIN_FUNC_WRITE; /* 功能码: 写 */
    buf[4] = 0x00;              /* 数据地址: 寄存器0x00 */
    buf[5] = 0x02;              /* 数据长度: 2字节 */
    buf[6] = addr_h;            /* 新地址高字节 */
    buf[7] = addr_l;            /* 新地址低字节 */

    crc = curtain_crc16(buf, 8);
    buf[8] = crc & 0xFF;
    buf[9] = (crc >> 8) & 0xFF;

    curtain_send_frame(buf, 10);
}
