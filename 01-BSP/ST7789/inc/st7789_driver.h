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
//**************************** Interface Structs ****************************//
//---------------------------------------------------------------------------//
//******************************** Classes **********************************//
typedef struct
{
    /** 底层需要的接口 */
    basic_oper_driver_interface_t *p_basic_operation;

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
                 basic_oper_driver_interface_t * const basic_operation_instance);
//******************************** 函数声明 ***********************************//

#endif //RTOS_PROJECT_ST7789_DRIVER_H