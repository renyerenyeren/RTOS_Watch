//
// Created by redmiX on 2026/4/6.
//

#ifndef RTOS_PROJECT_ST7789_DRIVER_H
#define RTOS_PROJECT_ST7789_DRIVER_H

//******************************** Includes *********************************//
#include <stdint.h>
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
#define ST7789_WIDTH   240
#define ST7789_HEIGHT  280
#define X_SHIFT        0
#define Y_SHIFT        20
#define HOR_LEN        40          // 分段刷新高度（用于 fill_color）

/* 基本命令 */
#define ST7789_SLPOUT    0x11   // 退出睡眠
#define ST7789_NORON     0x13   // 正常显示模式
#define ST7789_INVOFF    0x20   // 关闭反色
#define ST7789_INVON     0x21   // 开启反色
#define ST7789_DISPON    0x29   // 主屏开启
#define ST7789_CASET     0x2A   // 列地址设置
#define ST7789_RASET     0x2B   // 行地址设置
#define ST7789_RAMWR     0x2C   // 写显存
#define ST7789_TEOFF     0x34   // 关闭撕裂效应
#define ST7789_TEON      0x35   // 开启撕裂效应
#define ST7789_MADCTL    0x36   // 存储器访问控制
#define ST7789_COLMOD    0x3A   // 接口像素格式

/* MADCTL 参数 (RGB/BGR, 扫描方向等) */
#define ST7789_MADCTL_MY  0x80   // 行扫描顺序反转
#define ST7789_MADCTL_MX  0x40   // 列扫描顺序反转
#define ST7789_MADCTL_MV  0x20   // 行列交换（垂直刷新）
#define ST7789_MADCTL_ML  0x10   // 垂直扫描顺序反转
#define ST7789_MADCTL_BGR 0x08   // BGR 像素格式
#define ST7789_MADCTL_MH  0x04   // 水平扫描顺序反转
#define ST7789_MADCTL_RGB 0x00   // RGB 像素格式

/* COLMOD 参数 */
#define ST7789_COLOR_MODE_16bit  0x55  // 16位色 (65K) RGB565
#define ST7789_COLOR_MODE_18bit  0x66  // 18位色 (262K)
#define ST7789_COLOR_MODE_24bit  0x77  // 24位色 (16.7M)

/* 其他常用命令（初始化序列中使用） */
#define ST7789_PORCTRL   0xB2   //  porch 控制
#define ST7789_GCTRL     0xB7   //  gate 控制
#define ST7789_VCOMS     0xBB   //  VCOM 设置
#define ST7789_LCMCTRL   0xC0   //  LCM 控制
#define ST7789_VDVVRHEN  0xC2   //  VDV/VRH 使能
#define ST7789_VRHS      0xC3   //  VRH 设置
#define ST7789_VDVS      0xC4   //  VDV 设置
#define ST7789_FRCTRL2   0xC6   // 帧率控制
#define ST7789_PWCTRL1   0xD0   // 电源控制1
#define ST7789_PVGAMCTRL 0xE0   // 正电压伽马控制
#define ST7789_NVGAMCTRL 0xE1   // 负电压伽马控制
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Typedefs *********************************//
//******************************** Typedefs *********************************//
//---------------------------------------------------------------------------//
//**************************** Interface Structs ****************************//
typedef struct
{
    uint8_t (*pf_spi_transmit)(const uint8_t *pData, uint32_t dataLength);
    uint8_t (*pf_spi_transmit_dma)(const uint8_t *pData, uint32_t dataLength);
    uint8_t (*pf_write_reset_pin)(uint8_t pinState);
    uint8_t (*pf_write_cs_pin)(uint8_t pinState);
    uint8_t (*pf_write_dc_pin)(uint8_t pinState);
} basic_oper_driver_interface_t;

typedef struct
{
    void (*pf_delay_no_os) (uint32_t delay_ms);
} st7789_timebase_interface_t;
//**************************** Interface Structs ****************************//
//---------------------------------------------------------------------------//
//******************************** Classes **********************************//
typedef struct
{
    /** 底层需要的接口 */
    basic_oper_driver_interface_t *p_basic_operation;
    st7789_timebase_interface_t *p_timebase;

    /** 对外提供的接口 */
    uint8_t (*pf_init)(void *instance);
    uint8_t (*pf_set_direction)(void *instance, uint8_t direction);
    uint8_t (*pf_fill_color)(void *instance, uint16_t color);
    uint8_t (*pf_draw_pixel)(void *instance, uint16_t x, uint16_t y, uint16_t color);
    uint8_t (*pf_fill)(void *instance, uint16_t xSta, uint16_t ySta, uint16_t xEnd, uint16_t yEnd, uint16_t color);
    uint8_t (*pf_set_addr_window)(void *instance, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    uint8_t (*pf_flush_color_buffer)(void *instance, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, const uint16_t *color_buf);
} bsp_st7789_driver_t;
//******************************** Classes **********************************//
//---------------------------------------------------------------------------//
//**************************** Extern Variables *****************************//
//**************************** Extern Variables *****************************//
//---------------------------------------------------------------------------//
//******************************** 函数声明 ***********************************//
uint8_t st7789_driver_inst(bsp_st7789_driver_t * const driver_instance,
                 st7789_timebase_interface_t   * const timebase_instance,
                 basic_oper_driver_interface_t * const basic_operation_instance);
//******************************** 函数声明 ***********************************//

#endif //RTOS_PROJECT_ST7789_DRIVER_H