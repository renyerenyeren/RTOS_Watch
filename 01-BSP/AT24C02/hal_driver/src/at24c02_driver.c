//
// Created by redmiX on 2025/11/25.
//
//******************************** Includes *********************************//
#include <stdint.h>

#include "at24c02_driver.h"
#include "at24c02_reg.h"
#include "elog.h"
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
#ifdef  LOG_TAG
#undef  LOG_TAG
#define LOG_TAG       "at24c02_driver"
#else // else of LOG_TAG
#define LOG_TAG       "at24c02_driver"
#endif // end of LOG_TAG

#define IS_INITED                 ( 1==inited )

//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Macros ***********************************//
#define AT24C02_DEBUG
#ifdef  AT24C02_DEBUG
#define LOG_DEBUG(x,...)  log_d(x, ##__VA_ARGS__)
#define LOG_ERROR(x,...)  log_e(x, ##__VA_ARGS__)
#else
#define LOG_DEBUG(x,...)   ((void)0)
#define LOG_ERROR(x,...)   ((void)0)
#endif

#define ERROR_CHECK(EERO_NUM, NUM_EXPECT_VAL, LOG)  do{\
if((EERO_NUM) != (NUM_EXPECT_VAL))                     \
{                                                      \
LOG_ERROR((LOG));                                      \
}}while (0)

#define NULL_CHECK(x, tag)                          do{\
if(NULL == x)                                          \
{                                                      \
LOG_ERROR(#x" is null ptr");                           \
goto tag;}                                             \
}while (0)
//******************************** Macros ***********************************//
//---------------------------------------------------------------------------//
//******************************* Variables *********************************//
static uint8_t inited;
//******************************** Variables ********************************//
//---------------------------------------------------------------------------//
//******************************** Functions ********************************//

/**
 * 计算实际写入长度（处理页边界截断）
 * @param start_addr 起始地址
 * @param len 请求写入长度
 * @param actual_len 返回的实际可写入长度
 * @return 操作状态
 */
static void at24c_get_actual_write_length(uint8_t start_addr,
                                          uint8_t len,
                                          uint8_t *actual_len)
{
    // 计算当前页剩余空间
    uint8_t remaining_in_page = AT24C02_PAGE_BYTES -
                                (start_addr % AT24C02_PAGE_BYTES);

    // 实际写入长度取最小值
    *actual_len = (len > remaining_in_page) ? remaining_in_page : len;

    // 只在截断时记录警告
    if (*actual_len < len)
    {
        LOG_DEBUG("Write truncated: %d->%d bytes (page boundary at addr %d)",
                   len, *actual_len, start_addr + remaining_in_page);
    }
}

/**
 * @brief 初始化 AT24C02 EEPROM 驱动
 * @param[in] p_at24c_driver 指向 AT24C02 驱动实例的指针
 * @return 操作状态
 */
static at24c02_status_t at24c_init(bsp_at24c02_driver_t * const p_at24c_driver)
{
    LOG_DEBUG("at24c02_init start");
    NULL_CHECK(p_at24c_driver, init_error);
    NULL_CHECK(p_at24c_driver->p_i2c_driver_interface, init_error);
    NULL_CHECK(p_at24c_driver->p_i2c_driver_interface->pf_i2c_init,
                                                       init_error);
    inited = 1; //* 标记初始化完成
    p_at24c_driver->p_i2c_driver_interface->pf_i2c_init(NULL);
    LOG_DEBUG("at24c02_init iic_driver_init------");
    return AT24C02_OK;

init_error:
    {
        return AT24C02_ERRORPARAMETER;
    }
}

/**
 * @brief 反初始化 AT24C02 EEPROM 驱动
 * @param[in] p_at24c_driver 指向 AT24C02 驱动实例的指针
 * @return 操作状态
 */
static at24c02_status_t at24c_deinit(
                          bsp_at24c02_driver_t * const p_at24c_driver)
{
    if (!IS_INITED)
    {
        LOG_ERROR("at24c02 is deinit, do not need do agian");
    }
    NULL_CHECK(p_at24c_driver, deinit_error);
    inited = 0;
    return AT24C02_OK;

deinit_error:
    {
        return AT24C02_ERRORPARAMETER;
    }
}

/**
 * @brief 向 AT24C02 指定地址写入一个字节数据
 * @param[in] p_at24c_driver 指向 AT24C02 驱动实例的指针
 * @param[in] addr 内存地址
 * @param[in] data 要写入的数据
 * @return 操作状态
 */
static at24c02_status_t at24c_write_byte(
                            bsp_at24c02_driver_t * const p_at24c_driver,
                            uint8_t                const addr,
                            uint8_t                const data)
{
    if (!IS_INITED) return AT24C02_ERRORRESOURCE;
    NULL_CHECK(p_at24c_driver, write_byte_error);
    at24c02_status_t ret = AT24C02_OK;

    /**********************************临界区************************************/
    p_at24c_driver->p_i2c_driver_interface->pf_critical_enter();
    {
        /** 发送起始信号,发送从机写入地址*/
        p_at24c_driver->p_i2c_driver_interface->pf_i2c_start       (NULL);
        p_at24c_driver->p_i2c_driver_interface->pf_i2c_send_byte   (NULL ,
                                                AT24C02_SLAVE_ADDR_WRITE);
        ret =
            p_at24c_driver->p_i2c_driver_interface->pf_i2c_wait_ack(NULL);
        /** 超时检测*/
        ERROR_CHECK(ret, AT24C02_OK, "at24c02 time out (slave write addr ACK)");

        /** 发送内存地址 */
        p_at24c_driver->p_i2c_driver_interface->pf_i2c_send_byte   (NULL ,
                                                                    addr);
        ret =
            p_at24c_driver->p_i2c_driver_interface->pf_i2c_wait_ack(NULL);
        /** 超时检测*/
        ERROR_CHECK(ret, AT24C02_OK, "at24c02 time out (mem addr ACK)");

        /** 写入一字节数据 */
        p_at24c_driver->p_i2c_driver_interface->pf_i2c_send_byte   (NULL ,
                                                                    data);
        ret =
            p_at24c_driver->p_i2c_driver_interface->pf_i2c_wait_ack(NULL);
        /** 超时检测*/
        ERROR_CHECK(ret, AT24C02_OK, "at24c02 time out (data ACK)");

        p_at24c_driver->p_i2c_driver_interface->pf_i2c_stop        (NULL);
    }
    p_at24c_driver->p_i2c_driver_interface->pf_critical_exit();
    /********************************临界区结束***********************************/

    LOG_DEBUG("Write 1 byte (%d) to address %#x", data, addr);

    return ret;

write_byte_error:
    {
        LOG_ERROR("Write byte failed: Invalid pointer");
        return AT24C02_ERRORPARAMETER;
    }
}


/**
 * @brief 向 AT24C02 指定地址写入多个字节数据（页写入）
 * @param[in] p_at24c_driver 指向 AT24C02 驱动实例的指针
 * @param[in] start_addr 起始内存地址
 * @param[in] p_data 指向要写入数据的指针
 * @param[in] len 要写入的数据长度
 * @return 操作状态
 */
static at24c02_status_t at24c_write_page(
                            bsp_at24c02_driver_t * const p_at24c_driver,
                            uint8_t                const start_addr,
                      const uint8_t              *       p_data,
                            uint8_t                const len)
{
    if (!IS_INITED) return AT24C02_ERRORRESOURCE;
    NULL_CHECK(p_at24c_driver, write_page_error);
    NULL_CHECK(p_data,         write_page_error);
    at24c02_status_t ret = AT24C02_OK;

    if (len == 0)
    {
        LOG_ERROR("Write page failed: length is 0");
        return AT24C02_ERRORPARAMETER;
    }

    uint8_t actual_len;
    at24c_get_actual_write_length(start_addr, len, &actual_len);

    /**********************************临界区************************************/
    p_at24c_driver->p_i2c_driver_interface->pf_critical_enter();
    {
        // I2C通信部分（完全不变）
        p_at24c_driver->p_i2c_driver_interface->pf_i2c_start         (NULL);
        p_at24c_driver->p_i2c_driver_interface->pf_i2c_send_byte     (NULL ,
                                                  AT24C02_SLAVE_ADDR_WRITE);
        ret = p_at24c_driver->p_i2c_driver_interface->pf_i2c_wait_ack(NULL);
        ERROR_CHECK(ret, AT24C02_OK, "at24c02 time out (slave write addr ACK)");

        p_at24c_driver->p_i2c_driver_interface->pf_i2c_send_byte     (NULL ,
                                                                start_addr);
        ret = p_at24c_driver->p_i2c_driver_interface->pf_i2c_wait_ack(NULL);
        ERROR_CHECK(ret, AT24C02_OK, "at24c02 time out (start addr ACK)");

        for (uint8_t i = 0; i < actual_len; i++)
        {
            p_at24c_driver->p_i2c_driver_interface->pf_i2c_send_byte     (NULL ,
                                                                 *(p_data + i));
            ret = p_at24c_driver->p_i2c_driver_interface->pf_i2c_wait_ack(NULL);
            ERROR_CHECK(ret, AT24C02_OK, "at24c02 time out (data ACK)");
        }
        p_at24c_driver->p_i2c_driver_interface->pf_i2c_stop          (NULL);
    }
    p_at24c_driver->p_i2c_driver_interface->pf_critical_exit();
    /********************************临界区结束***********************************/

    LOG_DEBUG("Write %d bytes to start address %#x (truncated from %d)",
              actual_len, start_addr, len);

    return ret;

write_page_error:
    LOG_ERROR("Write page failed: Invalid pointer");
    return AT24C02_ERRORPARAMETER;
}

/**
 * @brief 从 AT24C02 指定地址读取一个字节数据
 * @param[in] p_at24c_driver 指向 AT24C02 驱动实例的指针
 * @param[in] addr 内存地址
 * @param[out] p_data 指向存储读取数据的指针
 * @return 操作状态
 */
static at24c02_status_t at24c_read_byte(
                                bsp_at24c02_driver_t * const p_at24c_driver,
                                uint8_t                const addr,
                                uint8_t              * const p_data)
{
    if (!IS_INITED) return AT24C02_ERRORRESOURCE;
    NULL_CHECK(p_at24c_driver, read_byte_error);
    NULL_CHECK(p_data        , read_byte_error);
    at24c02_status_t ret = AT24C02_OK;

    /**********************************临界区************************************/
    p_at24c_driver->p_i2c_driver_interface->pf_critical_enter();
    {
        /** 发送起始信号,发送从机写入地址*/
        p_at24c_driver->p_i2c_driver_interface->pf_i2c_start       (NULL);
        p_at24c_driver->p_i2c_driver_interface->pf_i2c_send_byte   (NULL ,
                                                AT24C02_SLAVE_ADDR_WRITE);
        ret =
            p_at24c_driver->p_i2c_driver_interface->pf_i2c_wait_ack(NULL);
        /** 超时检测*/
        ERROR_CHECK(ret, AT24C02_OK, "at24c02 time out (slave write addr ACK)");

        /** 发送内存地址 */
        p_at24c_driver->p_i2c_driver_interface->pf_i2c_send_byte   (NULL ,
                                                                    addr);
        ret =
            p_at24c_driver->p_i2c_driver_interface->pf_i2c_wait_ack(NULL);
        /** 超时检测*/
        ERROR_CHECK(ret, AT24C02_OK, "at24c02 time out (mem addr ACK)");

        /** 重新发送起始信号 */
        p_at24c_driver->p_i2c_driver_interface->pf_i2c_start       (NULL);
        p_at24c_driver->p_i2c_driver_interface->pf_i2c_send_byte   (NULL ,
                                                 AT24C02_SLAVE_ADDR_READ);
        ret =
            p_at24c_driver->p_i2c_driver_interface->pf_i2c_wait_ack(NULL);
        /** 超时检测*/
        ERROR_CHECK(ret, AT24C02_OK, "at24c02 time out (slave read addr ACK)");

        /** 读取一字节数据 */
        p_at24c_driver->p_i2c_driver_interface->pf_i2c_receive_byte(NULL ,
                                                                  p_data);
        p_at24c_driver->p_i2c_driver_interface->pf_i2c_send_nack   (NULL);
        p_at24c_driver->p_i2c_driver_interface->pf_i2c_stop        (NULL);
    }
    p_at24c_driver->p_i2c_driver_interface->pf_critical_exit();
    /********************************临界区结束***********************************/

    LOG_DEBUG("Read 1 byte (%d) from address %#X", *p_data, addr);

    return ret;

read_byte_error:
    {
        LOG_ERROR("read byte failed: Invalid pointer");
        return AT24C02_ERRORPARAMETER;
    }
}

/**
 * @brief 从 AT24C02 指定地址连续读取多个字节数据
 * @param[in] p_at24c_driver 指向 AT24C02 驱动实例的指针
 * @param[in] start_addr 起始内存地址
 * @param[out] p_data 指向存储读取数据的指针
 * @param[in] len 要读取的数据长度
 * @return 操作状态
 */
static at24c02_status_t at24c_read_sequential(
                                bsp_at24c02_driver_t * const p_at24c_driver,
                                uint8_t                const start_addr,
                                uint8_t              *       p_data,
                                uint16_t               const len)
{
    if (!IS_INITED) return AT24C02_ERRORRESOURCE;
    NULL_CHECK(p_at24c_driver, read_page_error);
    NULL_CHECK(p_data        , read_page_error);

    // 补充1：判断len==0（原有）+ len超过258字节（用户要求）
    if (len == 0)
    {
        LOG_ERROR("Read sequential failed: length is 0");
        return AT24C02_ERRORPARAMETER;
    }
    if (len > AT24C02_TOTAL_BYTES) // 读取长度超258则报错
    {
        LOG_ERROR("Read sequential failed: length exceeds max read len");
        return AT24C02_ERRORPARAMETER;
    }

    at24c02_status_t ret = AT24C02_OK;

    /**********************************临界区************************************/
    p_at24c_driver->p_i2c_driver_interface->pf_critical_enter();
    {
        /** 发送起始信号,发送从机写入地址*/
        p_at24c_driver->p_i2c_driver_interface->pf_i2c_start       (NULL);
        p_at24c_driver->p_i2c_driver_interface->pf_i2c_send_byte   (NULL ,
                                                AT24C02_SLAVE_ADDR_WRITE);
        ret =
            p_at24c_driver->p_i2c_driver_interface->pf_i2c_wait_ack(NULL);
        /** 超时检测*/
        ERROR_CHECK(ret, AT24C02_OK, "at24c02 time out (slave write addr ACK)");

        /** 发送内存地址 */
        p_at24c_driver->p_i2c_driver_interface->pf_i2c_send_byte   (NULL ,
                                                              start_addr);
        ret =
            p_at24c_driver->p_i2c_driver_interface->pf_i2c_wait_ack(NULL);
        /** 超时检测*/
        ERROR_CHECK(ret, AT24C02_OK, "at24c02 time out (start addr ACK)");

        /** 重新发送起始信号，发送从机读取地址 */
        p_at24c_driver->p_i2c_driver_interface->pf_i2c_start       (NULL);
        p_at24c_driver->p_i2c_driver_interface->pf_i2c_send_byte   (NULL ,
                                                 AT24C02_SLAVE_ADDR_READ);
        ret =
            p_at24c_driver->p_i2c_driver_interface->pf_i2c_wait_ack(NULL);
        /** 超时检测*/
        ERROR_CHECK(ret, AT24C02_OK, "at24c02 time out (slave read addr ACK)");

        /** 连续读取数据 */
        for (uint16_t i = 0; i<len-1; i++)
        {
            p_at24c_driver->p_i2c_driver_interface->pf_i2c_receive_byte(NULL,
                                                               (p_data + i));
            p_at24c_driver->p_i2c_driver_interface->pf_i2c_send_ack   (NULL);
        }
        p_at24c_driver->p_i2c_driver_interface->pf_i2c_receive_byte    (NULL,
                                                         (p_data + len - 1));
        p_at24c_driver->p_i2c_driver_interface->pf_i2c_send_nack   (NULL);
        p_at24c_driver->p_i2c_driver_interface->pf_i2c_stop        (NULL);
    }
    p_at24c_driver->p_i2c_driver_interface->pf_critical_exit();
    /********************************临界区结束***********************************/

    LOG_DEBUG("Read %d bytes from start address %#x", len, start_addr);

    return ret;

read_page_error:
    {
        LOG_ERROR("read page failed: Invalid pointer");
        return AT24C02_ERRORPARAMETER;
    }
}

/**
 * @brief 实例化 AT24C02 EEPROM 驱动
 * @param[in] p_driver_instance 指向驱动实例的指针
 * @param[in] p_i2c_if 指向 I2C 接口的指针
 * @param[in] p_yield_if 指向调度接口的指针
 * @return 操作状态
 */
at24c02_status_t bsp_at24c02_inst(
    bsp_at24c02_driver_t*           const p_driver_instance,
    at24c02_i2c_driver_interface_t* const p_i2c_if,
    at24c02_yield_interface_t*         const p_yield_if)
{
    LOG_DEBUG("===at24c02_inst start===");
    NULL_CHECK(p_driver_instance, inst_error);
    NULL_CHECK(p_i2c_if,          inst_error);
    NULL_CHECK(p_yield_if,        inst_error);

    p_driver_instance->p_i2c_driver_interface = p_i2c_if;
    p_driver_instance->p_yield_interface      = p_yield_if;

    p_driver_instance->pf_inst                = bsp_at24c02_inst;
    p_driver_instance->pf_init                = at24c_init;
    p_driver_instance->pf_deinit              = at24c_deinit;
    p_driver_instance->pf_write_byte          = at24c_write_byte;
    p_driver_instance->pf_write_page          = at24c_write_page;
    p_driver_instance->pf_read_byte           = at24c_read_byte;
    p_driver_instance->pf_read_sequential     = at24c_read_sequential;


    at24c02_status_t ret = at24c_init(p_driver_instance);
    ERROR_CHECK(ret, AT24C02_OK, "at24c02_init is unsuccessful");
    if (ret) return AT24C02_ERRORRESOURCE;

    LOG_DEBUG("at24c02_inst end");
    return AT24C02_OK;

inst_error:
    {
        return AT24C02_ERRORPARAMETER;
    }
}