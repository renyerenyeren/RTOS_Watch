//
// Created by redmiX on 2026/4/5.
//

//******************************** Includes *********************************//
#include "st7789_driver.h"

#include <stddef.h>
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
static uint16_t disp_buf[ST7789_WIDTH * HOR_LEN];
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

static uint8_t st7789_init(void *instance)
{
    bsp_st7789_driver_t *drv = (bsp_st7789_driver_t *)instance;

    drv->p_timebase->pf_delay_no_os(10);
    drv->p_basic_operation->pf_write_reset_pin(0);
    drv->p_timebase->pf_delay_no_os(10);
    drv->p_basic_operation->pf_write_reset_pin(1);
    drv->p_timebase->pf_delay_no_os(20);

    // 退出睡眠
    st7789_write_command(drv, ST7789_SLPOUT);
    drv->p_timebase->pf_delay_no_os(120);

    // 颜色模式：16bit
    st7789_write_command(drv, ST7789_COLMOD);
    st7789_write_simple_data(drv, ST7789_COLOR_MODE_16bit);
    drv->p_timebase->pf_delay_no_os(10);

    // 其他初始化序列（参考原驱动）
    st7789_write_command(drv, 0xB2);
    {
        uint8_t data[] = {0x0C, 0x0C, 0x00, 0x33, 0x33};
        st7789_write_data(drv, data, sizeof(data));
    }
    st7789_write_command(drv, 0xB7);
    st7789_write_simple_data(drv, 0x35);
    st7789_write_command(drv, 0xBB);
    st7789_write_simple_data(drv, 0x19);
    st7789_write_command(drv, 0xC0);
    st7789_write_simple_data(drv, 0x2C);
    st7789_write_command(drv, 0xC2);
    st7789_write_simple_data(drv, 0x01);
    st7789_write_command(drv, 0xC3);
    st7789_write_simple_data(drv, 0x12);
    st7789_write_command(drv, 0xC4);
    st7789_write_simple_data(drv, 0x20);
    st7789_write_command(drv, 0xC6);
    st7789_write_simple_data(drv, 0x0F);
    st7789_write_command(drv, 0xD0);
    st7789_write_simple_data(drv, 0xA4);
    st7789_write_simple_data(drv, 0xA1);

    st7789_write_command(drv, 0xE0);
    {
        uint8_t data[] = {0xD0,0x04,0x0D,0x11,0x13,0x2B,0x3F,0x54,0x4C,0x18,0x0D,0x0B,0x1F,0x23};
        st7789_write_data(drv, data, sizeof(data));
    }
    st7789_write_command(drv, 0xE1);
    {
        uint8_t data[] = {0xD0,0x04,0x0C,0x11,0x13,0x2C,0x3F,0x44,0x51,0x2F,0x1F,0x1F,0x20,0x23};
        st7789_write_data(drv, data, sizeof(data));
    }

    st7789_write_command(drv, ST7789_INVON);
    st7789_write_command(drv, ST7789_NORON);
    st7789_write_command(drv, ST7789_DISPON);
    drv->p_timebase->pf_delay_no_os(50);

    // 设置默认方向（由用户后续调用 pf_set_direction 修改）
    drv->pf_set_direction(instance, 0);
    drv->pf_fill_color(instance, 0x0000);   // 清屏黑色
    return 0;
}

static uint8_t st7789_set_direction(void *instance, uint8_t dir)
{
    bsp_st7789_driver_t *drv = (bsp_st7789_driver_t *)instance;
    (void)dir;  // 本例固定为 MADCTL_MX | MY | RGB，可扩展
    st7789_write_command(drv, ST7789_MADCTL);
    st7789_write_simple_data(drv, ST7789_MADCTL_MX | ST7789_MADCTL_MY | ST7789_MADCTL_RGB);
    return 0;
}

static uint8_t st7789_set_addr_window(void *instance, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    bsp_st7789_driver_t *drv = (bsp_st7789_driver_t *)instance;
    uint16_t xs = x0 + X_SHIFT;
    uint16_t xe = x1 + X_SHIFT;
    uint16_t ys = y0 + Y_SHIFT;
    uint16_t ye = y1 + Y_SHIFT;

    st7789_write_command(drv, ST7789_CASET);
    {
        uint8_t data[] = {xs >> 8, xs & 0xFF, xe >> 8, xe & 0xFF};
        st7789_write_data(drv, data, sizeof(data));
    }
    st7789_write_command(drv, ST7789_RASET);
    {
        uint8_t data[] = {ys >> 8, ys & 0xFF, ye >> 8, ye & 0xFF};
        st7789_write_data(drv, data, sizeof(data));
    }
    st7789_write_command(drv, ST7789_RAMWR);
    return 0;
}

static uint8_t st7789_fill_color(void *instance, uint16_t color)
{
    bsp_st7789_driver_t *drv = (bsp_st7789_driver_t *)instance;
    drv->pf_set_addr_window(instance, 0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1);

    uint32_t total_pixels = ST7789_WIDTH * ST7789_HEIGHT;
    uint32_t pixels_sent = 0;
    uint32_t buf_pixels = sizeof(disp_buf) / sizeof(uint16_t);
    uint16_t i;

    // 填充缓冲区
    for (i = 0; i < buf_pixels; i++)
        disp_buf[i] = color;

    while (pixels_sent < total_pixels) {
        uint32_t send = total_pixels - pixels_sent;
        if (send > buf_pixels) send = buf_pixels;
        st7789_write_data(drv, (uint8_t*)disp_buf, send * 2);
        pixels_sent += send;
    }
    return 0;
}

static uint8_t st7789_draw_pixel(void *instance, uint16_t x, uint16_t y, uint16_t color)
{
    bsp_st7789_driver_t *drv = (bsp_st7789_driver_t *)instance;
    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT)
        return 1;
    drv->pf_set_addr_window(instance, x, y, x, y);
    uint8_t data[] = {color >> 8, color & 0xFF};
    st7789_write_data(drv, data, sizeof(data));
    return 0;
}

static uint8_t st7789_fill(void *instance, uint16_t xSta, uint16_t ySta, uint16_t xEnd, uint16_t yEnd, uint16_t color)
{
    bsp_st7789_driver_t *drv = (bsp_st7789_driver_t *)instance;
    if (xEnd >= ST7789_WIDTH || yEnd >= ST7789_HEIGHT)
        return 1;
    drv->pf_set_addr_window(instance, xSta, ySta, xEnd, yEnd);

    uint32_t width  = xEnd - xSta + 1;
    uint32_t height = yEnd - ySta + 1;
    uint32_t total  = width * height;
    uint32_t sent = 0;

    // 准备一个临时行缓冲区（避免逐个像素发送）
    uint16_t line_buf[width];
    for (uint32_t i = 0; i < width; i++)
        line_buf[i] = color;

    while (sent < total) {
        uint32_t rows = total - sent;
        if (rows > 1) rows = 1;          // 每次发送一行
        st7789_write_data(drv, (uint8_t*)line_buf, width * 2);
        sent += width;
    }
    return 0;
}

static uint8_t st7789_flush_color_buffer(void *instance, uint16_t x1, uint16_t y1,
                                         uint16_t x2, uint16_t y2, const uint16_t *color_buf)
{
    bsp_st7789_driver_t *drv = (bsp_st7789_driver_t *)instance;
    uint32_t w = x2 - x1 + 1;
    uint32_t h = y2 - y1 + 1;
    drv->pf_set_addr_window(instance, x1, y1, x2, y2);
    // 直接发送颜色缓冲区（注意 color_buf 是 16bit 像素数组，需转为字节流）
    st7789_write_data(drv, (const uint8_t*)color_buf, w * h * 2);
    return 0;
}

uint8_t st7789_driver_inst(bsp_st7789_driver_t * const driver_instance,
                 st7789_timebase_interface_t   * const timebase_instance,
                 basic_oper_driver_interface_t * const basic_operation_instance)
{
    if (NULL == driver_instance          ||
        NULL == basic_operation_instance ||
        NULL == timebase_instance)
        return 1;
    /** 实例化底层需要的接口 */
    driver_instance->p_basic_operation = basic_operation_instance;
    driver_instance->p_timebase        = timebase_instance;
    /** 实例化对外提供的接口 */
    driver_instance->pf_init               = st7789_init;
    driver_instance->pf_set_direction      = st7789_set_direction;
    driver_instance->pf_fill_color         = st7789_fill_color;
    driver_instance->pf_draw_pixel         = st7789_draw_pixel;
    driver_instance->pf_fill               = st7789_fill;
    driver_instance->pf_set_addr_window    = st7789_set_addr_window;
    driver_instance->pf_flush_color_buffer = st7789_flush_color_buffer;

    return 0;
}