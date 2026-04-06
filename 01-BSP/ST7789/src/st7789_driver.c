//
// Created by redmiX on 2026/4/5.
//

//******************************** Includes *********************************//
#include "st7789_driver.h"
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
#ifdef  LOG_TAG
#undef  LOG_TAG
#define LOG_TAG       "ST7789"
#else
#define LOG_TAG       "ST7789"
#endif
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Macros ***********************************//
// #define ST7789_DEBUG
#if defined(ST7789_DEBUG) && defined(MYDEBUG)
#define LOG_DEBUG(x,...)  log_d(x, ##__VA_ARGS__)
#define LOG_ERROR(x,...)  log_e(x, ##__VA_ARGS__)
#else
#define LOG_DEBUG(fmt, ...)   ((void)0)
#define LOG_ERROR(fmt, ...)   ((void)0)
#endif
//******************************** Macros ***********************************//
//---------------------------------------------------------------------------//
//******************************** 函数声明   *********************************//
static uint8_t st7789_write_command(bsp_st7789_driver_t *drv, uint8_t cmd);
static uint8_t st7789_write_data(bsp_st7789_driver_t *drv,
                            const uint8_t *data, uint32_t len);
static uint8_t st7789_write_simple_data(bsp_st7789_driver_t *drv, uint8_t data);
//******************************** 函数声明   *********************************//
//---------------------------------------------------------------------------//
//******************************** Variables ********************************//
//******************************** Variables ********************************//
//---------------------------------------------------------------------------//
//******************************** Functions ********************************//
static uint8_t st7789_write_command(bsp_st7789_driver_t *drv, uint8_t cmd)
{
    drv->p_basic_operation->pf_write_cs_pin(0);   // CS low
    drv->p_basic_operation->pf_write_dc_pin(0);   // DC low (command)
    drv->p_basic_operation->pf_spi_transmit(&cmd, 1);
    drv->p_basic_operation->pf_write_cs_pin(1);   // CS high
    return 0;
}

static uint8_t st7789_write_data(bsp_st7789_driver_t *drv,
                            const uint8_t *data, uint32_t len)
{
    drv->p_basic_operation->pf_write_cs_pin(0);   // CS low
    drv->p_basic_operation->pf_write_dc_pin(1);   // DC high (data)

    // 分块发送（避免单次超 65535，同时优先使用 DMA）
    while (len > 0) {
        uint32_t chunk = (len > 65535) ? 65535 : len;
        if (chunk >= 16) {
            drv->p_basic_operation->pf_spi_transmit_dma(data, chunk);
        } else {
            drv->p_basic_operation->pf_spi_transmit(data, chunk);
        }
        data += chunk;
        len  -= chunk;
    }
    drv->p_basic_operation->pf_write_cs_pin(1);   // CS high
    return 0;
}

static uint8_t st7789_write_simple_data(bsp_st7789_driver_t *drv, uint8_t data)
{
    drv->p_basic_operation->pf_write_cs_pin(0);
    drv->p_basic_operation->pf_write_dc_pin(1);
    drv->p_basic_operation->pf_spi_transmit(&data, 1);
    drv->p_basic_operation->pf_write_cs_pin(1);
    return 0;
}