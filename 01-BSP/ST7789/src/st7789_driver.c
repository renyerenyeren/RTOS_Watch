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
// 外部定义的显示缓冲区，用于加速全屏填充
static uint16_t disp_buf[ST7789_WIDTH * HOR_LEN];
//******************************** Variables ********************************//
//---------------------------------------------------------------------------//
//******************************** Functions ********************************//
/**
 * @brief 向 ST7789 发送命令字节
 * @param drv 驱动实例指针
 * @param cmd 要发送的命令字节
 * @return 0: 成功, 1: 失败
 * @note 发送命令时 DC 引脚需拉低，CS 引脚需拉低
 */
static uint8_t st7789_write_command(bsp_st7789_driver_t *drv, uint8_t cmd)
{
    // 1. 拉低片选信号，选中设备
    drv->p_basic_operation->pf_write_cs_pin(0);   // CS low
    // 2. 拉低 DC 信号，指示接下来传输的是命令
    drv->p_basic_operation->pf_write_dc_pin(0);   // DC low (command)
    // 3. 通过 SPI 发送命令字节
    drv->p_basic_operation->pf_spi_transmit(&cmd, 1);
    // 4. 拉高片选信号，结束传输
    drv->p_basic_operation->pf_write_cs_pin(1);   // CS high
    return 0;
}

/**
 * @brief 向 ST7789 发送数据流
 * @param drv 驱动实例指针
 * @param data 数据缓冲区指针
 * @param len 要发送的数据长度（字节）
 * @return 0: 成功, 1: 失败
 * @note 发送数据时 DC 引脚需拉高。支持分块传输以适应 DMA 限制。
 */
static uint8_t st7789_write_data(bsp_st7789_driver_t *drv,
                            const uint8_t *data, uint32_t len)
{
    // 1. 拉低片选信号，选中设备
    drv->p_basic_operation->pf_write_cs_pin(0);   // CS low
    // 2. 拉高 DC 信号，指示接下来传输的是数据
    drv->p_basic_operation->pf_write_dc_pin(1);   // DC high (data)

    // 3. 循环发送数据块
    // 分块发送（避免单次超 65535，同时优先使用 DMA）
    while (len > 0)
    {
        // 3.1 计算当前块的大小，不超过 65535
        uint32_t chunk = (len > 65535) ? 65535 : len;
        // 3.2 如果块大小 >= 16，使用 DMA 传输以提高效率
        if (chunk >= 16)
        {
            drv->p_basic_operation->pf_spi_transmit_dma(data, chunk);
        }
        // 3.3 否则使用普通中断/轮询传输
        else
        {
            drv->p_basic_operation->pf_spi_transmit(data, chunk);
        }
        // 3.4 更新指针和剩余长度
        data += chunk;
        len  -= chunk;
    }
    // 4. 拉高片选信号，结束传输
    drv->p_basic_operation->pf_write_cs_pin(1);   // CS high
    return 0;
}

/**
 * @brief 向 ST7789 发送单个字节数据
 * @param drv 驱动实例指针
 * @param data 要发送的数据字节
 * @return 0: 成功, 1: 失败
 */
static uint8_t st7789_write_simple_data(bsp_st7789_driver_t *drv, uint8_t data)
{
    // 1. 拉低片选信号
    drv->p_basic_operation->pf_write_cs_pin(0);
    // 2. 拉高 DC 信号
    drv->p_basic_operation->pf_write_dc_pin(1);
    // 3. 发送单字节数据
    drv->p_basic_operation->pf_spi_transmit(&data, 1);
    // 4. 拉高片选信号
    drv->p_basic_operation->pf_write_cs_pin(1);
    return 0;
}

/**
 * @brief 初始化 ST7789 显示屏
 * @param instance 驱动实例指针
 * @return 0: 成功, 1: 失败
 */
static uint8_t st7789_init(void *instance)
{
    bsp_st7789_driver_t *drv = (bsp_st7789_driver_t *)instance;

    // 1. 硬件复位
    drv->p_timebase->pf_delay_no_os(10);
    drv->p_basic_operation->pf_write_reset_pin(0); // Reset Low
    drv->p_timebase->pf_delay_no_os(10);
    drv->p_basic_operation->pf_write_reset_pin(1); // Reset High
    drv->p_timebase->pf_delay_no_os(20);          // 等待复位完成

    // 2. 软件复位 (0x01)
    st7789_write_command(drv, 0x01);
    drv->p_timebase->pf_delay_no_os(120);         // 延时 120ms

    // 3. 退出睡眠模式 (0x11)
    st7789_write_command(drv, ST7789_SLPOUT);
    drv->p_timebase->pf_delay_no_os(120);         // 延时 120ms

    // 4. 设置颜色模式为 16bit RGB565 (0x3A, 0x55)
    st7789_write_command(drv, ST7789_COLMOD);
    st7789_write_simple_data(drv, ST7789_COLOR_MODE_16bit);
    drv->p_timebase->pf_delay_no_os(10);

    // 5. 设置内存访问控制 (0x36, 0x00) - 根据需要可改为你原来的方向设置
    st7789_write_command(drv, ST7789_MADCTL);
    st7789_write_simple_data(drv, 0x00);
    drv->p_timebase->pf_delay_no_os(10);

    // 6. 设置列地址范围 (CASET) 全屏
    st7789_write_command(drv, ST7789_CASET);
    {
        uint16_t x_start = 0;
        uint16_t x_end = ST7789_WIDTH - 1;
        uint8_t data[] = {
            x_start >> 8, x_start & 0xFF,
            x_end >> 8, x_end & 0xFF
        };
        st7789_write_data(drv, data, sizeof(data));
    }

    // 7. 设置行地址范围 (RASET) 全屏
    st7789_write_command(drv, ST7789_RASET);
    {
        uint16_t y_start = 0;
        uint16_t y_end = ST7789_HEIGHT - 1;
        uint8_t data[] = {
            y_start >> 8, y_start & 0xFF,
            y_end >> 8, y_end & 0xFF
        };
        st7789_write_data(drv, data, sizeof(data));
    }

    // 8. 开启显示反转 (0x21)
    st7789_write_command(drv, ST7789_INVON);
    drv->p_timebase->pf_delay_no_os(10);

    // 9. 正常显示模式 (0x13)
    st7789_write_command(drv, ST7789_NORON);
    drv->p_timebase->pf_delay_no_os(10);

    // 10. 开启显示 (0x29)
    st7789_write_command(drv, ST7789_DISPON);
    drv->p_timebase->pf_delay_no_os(50);

    // 11. 清屏为黑色 (注意你的 pf_fill_color 可能使用了 Y_SHIFT，请确保 X_SHIFT/Y_SHIFT 为 0)
    // 如果你保留了 Y_SHIFT，建议先设为0
    drv->pf_fill_color(instance, 0x0000);   // 黑色

    return 0;
}

/**
 * @brief 设置显示屏扫描方向
 * @param instance 驱动实例指针
 * @param dir 方向参数（本例中未使用，固定为一种模式）
 * @return 0: 成功, 1: 失败
 */
static uint8_t st7789_set_direction(void *instance, uint8_t dir)
{
    bsp_st7789_driver_t *drv = (bsp_st7789_driver_t *)instance;
    (void)dir;  // 本例固定为 MADCTL_MX | MY | RGB，可扩展

    // 1. 发送内存访问控制命令
    st7789_write_command(drv, ST7789_MADCTL);
    // 2. 设置参数：MX(列地址顺序), MY(行地址顺序), RGB(颜色顺序)
    st7789_write_simple_data(drv, ST7789_MADCTL_MX | ST7789_MADCTL_MY | ST7789_MADCTL_RGB);
    return 0;
}

/**
 * @brief 设置显示窗口区域
 * @param instance 驱动实例指针
 * @param x0 起始列坐标
 * @param y0 起始行坐标
 * @param x1 结束列坐标
 * @param y1 结束行坐标
 * @return 0: 成功, 1: 失败
 */
static uint8_t st7789_set_addr_window(void *instance, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    bsp_st7789_driver_t *drv = (bsp_st7789_driver_t *)instance;

    // 1. 加上偏移量，适配实际物理屏幕
    uint16_t xs = x0 + X_SHIFT;
    uint16_t xe = x1 + X_SHIFT;
    uint16_t ys = y0 + Y_SHIFT;
    uint16_t ye = y1 + Y_SHIFT;

    // 2. 设置列地址范围 (CASET)
    st7789_write_command(drv, ST7789_CASET);
    {
        uint8_t data[] = {xs >> 8, xs & 0xFF, xe >> 8, xe & 0xFF};
        st7789_write_data(drv, data, sizeof(data));
    }
    // 3. 设置行地址范围 (RASET)
    st7789_write_command(drv, ST7789_RASET);
    {
        uint8_t data[] = {ys >> 8, ys & 0xFF, ye >> 8, ye & 0xFF};
        st7789_write_data(drv, data, sizeof(data));
    }
    // 4. 发送写内存命令 (RAMWR)，准备写入像素数据
    st7789_write_command(drv, ST7789_RAMWR);
    return 0;
}

/**
 * @brief 全屏填充指定颜色
 * @param instance 驱动实例指针
 * @param color RGB565 格式的颜色值
 * @return 0: 成功, 1: 失败
 */
static uint8_t st7789_fill_color(void *instance, uint16_t color)
{
    bsp_st7789_driver_t *drv = (bsp_st7789_driver_t *)instance;

    // 1. 设置窗口为全屏
    drv->pf_set_addr_window(instance, 0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1);

    // 2. 交换字节序：ST7789 要求先发送高字节，而小端MCU内存中低字节在前，故提前交换
    uint16_t swapped_color = (color >> 8) | (color << 8);

    uint32_t total_pixels = ST7789_WIDTH * ST7789_HEIGHT;
    uint32_t pixels_sent = 0;
    uint32_t buf_pixels = sizeof(disp_buf) / sizeof(uint16_t);
    uint16_t i;

    // 3. 使用交换后的颜色填充缓冲区
    for (i = 0; i < buf_pixels; i++)
        disp_buf[i] = swapped_color;

    // 4. 循环发送缓冲区数据直到填满屏幕
    while (pixels_sent < total_pixels)
    {
        uint32_t send = total_pixels - pixels_sent;
        if (send > buf_pixels) send = buf_pixels;
        // 发送数据（按字节流，此时缓冲区中已是高字节在前内存布局）
        st7789_write_data(drv, (uint8_t*)disp_buf, send * 2);
        pixels_sent += send;
    }
    return 0;
}

/**
 * @brief 绘制单个像素点
 * @param instance 驱动实例指针
 * @param x 列坐标
 * @param y 行坐标
 * @param color RGB565 格式的颜色值
 * @return 0: 成功, 1: 失败（坐标越界）
 */
static uint8_t st7789_draw_pixel(void *instance, uint16_t x, uint16_t y, uint16_t color)
{
    bsp_st7789_driver_t *drv = (bsp_st7789_driver_t *)instance;

    // 1. 检查坐标是否越界
    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT)
        return 1;

    // 2. 设置当前像素点的地址窗口
    drv->pf_set_addr_window(instance, x, y, x, y);

    // 3. 发送颜色数据（高字节在前）
    uint8_t data[] = {color >> 8, color & 0xFF};
    st7789_write_data(drv, data, sizeof(data));
    return 0;
}

/**
 * @brief 填充指定矩形区域
 * @param instance 驱动实例指针
 * @param xSta 起始列坐标
 * @param ySta 起始行坐标
 * @param xEnd 结束列坐标
 * @param yEnd 结束行坐标
 * @param color RGB565 格式的颜色值
 * @return 0: 成功, 1: 失败（坐标越界）
 */
static uint8_t st7789_fill(void *instance, uint16_t xSta, uint16_t ySta, uint16_t xEnd, uint16_t yEnd, uint16_t color)
{
    bsp_st7789_driver_t *drv = (bsp_st7789_driver_t *)instance;

    if (xEnd >= ST7789_WIDTH || yEnd >= ST7789_HEIGHT)
        return 1;

    drv->pf_set_addr_window(instance, xSta, ySta, xEnd, yEnd);

    uint32_t width  = xEnd - xSta + 1;
    uint32_t height = yEnd - ySta + 1;

    // 交换字节序：ST7789要求高字节在前
    uint16_t swapped = (color >> 8) | (color << 8);

    // 用交换后的颜色填充全局缓冲区的前 width 个像素
    for (uint32_t i = 0; i < width; i++) {
        disp_buf[i] = swapped;
    }

    // 逐行发送：每行都是 width 个像素（每个像素2字节）
    for (uint32_t row = 0; row < height; row++) {
        st7789_write_data(drv, (uint8_t*)disp_buf, width * 2);
    }
    return 0;
}

/**
 * @brief 将外部颜色缓冲区刷新到屏幕指定区域
 * @param instance 驱动实例指针
 * @param x1 区域左上角列坐标
 * @param y1 区域左上角行坐标
 * @param x2 区域右下角列坐标
 * @param y2 区域右下角行坐标
 * @param color_buf 指向像素数据缓冲区的指针 (RGB565 格式)
 * @return 0: 成功, 1: 失败
 */
static uint8_t st7789_flush_color_buffer(void *instance, uint16_t x1, uint16_t y1,
                                         uint16_t x2, uint16_t y2, const uint16_t *color_buf)
{
    bsp_st7789_driver_t *drv = (bsp_st7789_driver_t *)instance;

    uint32_t w = x2 - x1 + 1;
    uint32_t h = y2 - y1 + 1;
    uint32_t total_pixels = w * h;
    uint32_t pixels_done = 0;

    // 每次最多处理缓冲区大小的像素（240*40=9600）
    uint32_t buf_pixels = sizeof(disp_buf) / sizeof(disp_buf[0]);

    // 设置窗口
    drv->pf_set_addr_window(instance, x1, y1, x2, y2);

    while (pixels_done < total_pixels) {
        uint32_t send = total_pixels - pixels_done;
        if (send > buf_pixels)
            send = buf_pixels;

        // 转换字节序（只转换本次发送的部分）
        for (uint32_t i = 0; i < send; i++) {
            uint16_t c = color_buf[pixels_done + i];
            disp_buf[i] = (c >> 8) | (c << 8);
        }

        // 发送
        st7789_write_data(drv, (const uint8_t*)disp_buf, send * 2);
        pixels_done += send;
    }

    return 0;
}

/**
 * @brief 实例化 ST7789 驱动程序
 * @param driver_instance 驱动结构体实例指针
 * @param timebase_instance 时基接口实例（延时功能）
 * @param basic_operation_instance 基础操作接口实例（SPI, GPIO）
 * @return 0: 成功, 1: 失败（输入指针为空）
 */
uint8_t st7789_driver_inst(bsp_st7789_driver_t * const driver_instance,
                 st7789_timebase_interface_t   * const timebase_instance,
                 basic_oper_driver_interface_t * const basic_operation_instance)
{
    // 1. 检查输入指针是否有效
    if (NULL == driver_instance          ||
        NULL == basic_operation_instance ||
        NULL == timebase_instance)
        return 1;

    /** 实例化底层需要的接口 */
    // 2. 将底层操作接口挂载到驱动实例中
    driver_instance->p_basic_operation = basic_operation_instance;
    driver_instance->p_timebase        = timebase_instance;

    /** 实例化对外提供的接口 */
    // 3. 将功能函数挂载到驱动实例的函数指针中
    driver_instance->pf_init               = st7789_init;
    driver_instance->pf_set_direction      = st7789_set_direction;
    driver_instance->pf_fill_color         = st7789_fill_color;
    driver_instance->pf_draw_pixel         = st7789_draw_pixel;
    driver_instance->pf_fill               = st7789_fill;
    driver_instance->pf_set_addr_window    = st7789_set_addr_window;
    driver_instance->pf_flush_color_buffer = st7789_flush_color_buffer;

    return 0;
}