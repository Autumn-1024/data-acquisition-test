# 采集卡测试工程 项目记忆

## 项目信息
- **MCU**: STM32F103C8T6 (Cortex-M3, 64KB Flash, 20KB RAM)
- **GitHub**: https://github.com/Autumn-1024/data-acquisition-test
- **本地路径**: C:\Users\Autumn\Desktop\阜阳幼专采集卡\采集卡测试程序\data-acquisition-test
- **创建日期**: 2026-06-16 (原窗帘控制工程, 2026-06-25 改为采集卡测试)

## 硬件配置
- **LED**: PC13 (低电平点亮)
- **按键**: KEY1=PB10, KEY2=PB11, KEY3=PA11, KEY4=PA12 (均低电平有效)
- **OLED**: SSD1306 128x64, 软件I2C (PB8=SCL, PB9=SDA, 地址0x78)
- **调试串口**: USART1 PA9(TX)/PA10(RX), 115200bps, printf重定向

## 功能说明
- 14个传感器列表 (Sensor 1~14)，上下键选择，KEY4进入详情
- 传感器详情页：名称 + 大字数据值(cm) + 实时波形图，每秒刷新
- 数据范围 1.0~20.0，一位小数，模拟波动 ±1.0
- 串口命令跳转传感器、控制LED、自动输出数据

## 按键映射
| 按键 | 引脚 | 功能 |
|------|------|------|
| KEY1 | PB10 | 返回 |
| KEY2 | PB11 | 下移 |
| KEY3 | PA11 | 上移 |
| KEY4 | PA12 | 确认/进入 |

## 串口协议
- 传感器跳转: `1`~`9`→Sensor 1~9, `10`~`14`→Sensor 10~14
- LED控制: `a`=亮, `b`=灭, `c`=翻转, `d`=查询状态+传感器
- 数据输出: `序号@值` (如 `1@9.6`), 每秒发送

## 工程结构
```
data-acquisition-test/
├── Core/Inc/          ← main.h, hal_conf.h, stm32f1xx_it.h
├── Core/Src/          ← main.c, stm32f1xx_it.c, system_stm32f1xx.c
├── Core/Startup/      ← startup_stm32f103xe.s
├── Drivers/           ← CMSIS + HAL_Driver (零修改)
├── BSP/Inc/           ← bsp_led.h, bsp_key.h, bsp_uart.h, bsp_oled.h
├── BSP/Src/           ← bsp_led.c, bsp_key.c, bsp_uart.c, bsp_oled.c
├── App/Inc/           ← app_task.h, app_menu.h
├── App/Src/           ← app_task.c, app_menu.c
├── MDK-ARM/           ← Keil工程
├── Output/            ← 编译产物
├── claw.md            ← 本文件
└── README.md
```

## Keil 配置
- **启动文件**: startup_stm32f103xe.s
- **宏定义**: STM32F103xE,USE_HAL_DRIVER
- **烧录器**: ST-Link III KEIL SWO (pMon: STLink\ST-LINKIII-KEIL_SWO.dll)
- **编译器**: V5.06 ARMCC

## HAL模块开关
- GPIO, RCC, FLASH, PWR, CORTEX, DMA, UART

## 开发注意事项
1. OLED使用软件I2C模式 (PB8=SCL, PB9=SDA)
2. 按键编号 1~4 (不是 0~3)
3. 烧录器必须是 ST-Link III KEIL SWO，不是 ULINK
4. 波形图区域 y=34~63，数据区域 y=0~33

## 常用命令
```powershell
# 编译
& "C:\Program Files (x86)\MDK\core\UV4\uVision.com" -r "C:\Users\Autumn\Desktop\阜阳幼专采集卡\采集卡测试程序\data-acquisition-test\MDK-ARM\atk_f103c8t6.uvprojx" -o "C:\Users\Autumn\Desktop\阜阳幼专采集卡\采集卡测试程序\data-acquisition-test\Output\build_log.txt" -j0

# 烧录
& "C:\Program Files (x86)\MDK\core\UV4\uVision.com" -f "C:\Users\Autumn\Desktop\阜阳幼专采集卡\采集卡测试程序\data-acquisition-test\MDK-ARM\atk_f103c8t6.uvprojx" -o "C:\Users\Autumn\Desktop\阜阳幼专采集卡\采集卡测试程序\data-acquisition-test\Output\flash_log.txt" -j0
```

## 更新日志
- 2026-06-16: 创建工程 (原窗帘控制工程)
- 2026-06-25: 改为采集卡测试工程
  - 按键引脚改为 PB10/PB11/PA11/PA12，编号改为 KEY1~4
  - OLED 引脚改为 PB8(SCL)/PB9(SDA)
  - 去掉窗帘/RS485/WiFi 模块
  - 新增传感器列表 + 详情页 + 波形图
  - 新增串口命令控制 (传感器跳转、LED控制、数据输出)
