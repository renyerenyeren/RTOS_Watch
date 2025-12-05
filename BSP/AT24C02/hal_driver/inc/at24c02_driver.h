//
// Created by redmiX on 2025/11/25.
//

#ifndef RTOS_PROJECT_AT24C02_DRIVER_H
#define RTOS_PROJECT_AT24C02_DRIVER_H
//******************************** Includes *********************************//
#include <stdint.h>
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Typedefs *********************************//
/* 函数返回状态     */
/*  函数返回状态枚举                    */
typedef enum
{
    AT24C02_OK                = 0,         //* 操作成功完成
    AT24C02_ERROR             = 1,         //* 运行时错误，无匹配情况
    AT24C02_ERRORTIMEOUT      = 2,         //* 操作失败，超市
    AT24C02_ERRORRESOURCE     = 3,         //* 资源不可用
    AT24C02_ERRORPARAMETER    = 4,         //* 参数错误
    AT24C02_ERRORMEMORY       = 5,         //* 内存不足
    AT24C02_ERRORISR          = 6,         //* 中断服务程序（ISR）上下文不允许
    AT24C02_RESERVED          = 0x7FFFFFFF //* 保留
}at24c02_status_t;
//******************************** Typedefs *********************************//
//---------------------------------------------------------------------------//
//**************************** Interface Structs ****************************//
typedef struct
{
    at24c02_status_t (*pf_i2c_init)         (void*);  /*   IIC init    interf.*/
    at24c02_status_t (*pf_i2c_deinit)       (void*);  /*   IIC deinit  interf.*/
    at24c02_status_t (*pf_i2c_start)        (void*);  /*   IIC start   interf.*/
    at24c02_status_t (*pf_i2c_stop)         (void*);  /*   IIC stop    interf.*/
    at24c02_status_t (*pf_i2c_wait_ack)     (void*);  /*   IIC w-ack   interf.*/
    at24c02_status_t (*pf_i2c_send_ack)     (void*);  /*   IIC s-ack   interf.*/
    at24c02_status_t (*pf_i2c_send_nack)    (void*);  /*   IIC s-n-ack interf.*/
    at24c02_status_t (*pf_i2c_send_byte)    (void*,   /*   IIC s-byte  interf.*/
                                           const uint8_t);
    at24c02_status_t (*pf_i2c_receive_byte) (void*,   /*   IIC r-byte  interf.*/
                                            uint8_t * const );
    at24c02_status_t (*pf_critical_enter)   (void);   /* enter critical state.*/
    at24c02_status_t (*pf_critical_exit)    (void);   /* exit  critical state.*/
}at24c02_i2c_driver_interface_t;

/** 获取让出CPU使用的函数 */
typedef struct
{
    void (*pf_rtos_yield)(const uint32_t);          /*OS Not-Blocking Delay  */
}at24c02_yield_interface_t;
//**************************** Interface Structs ****************************//
//---------------------------------------------------------------------------//
//******************************** Classes **********************************//
typedef struct bsp_at24c02_driver_struct bsp_at24c02_driver_t ;
typedef struct bsp_at24c02_driver_struct
{
    /**需要提供的接口*/
    at24c02_i2c_driver_interface_t *p_i2c_driver_interface;
    at24c02_yield_interface_t      *p_yield_interface;

    /**对象函数指针*/
    /**对象实例化函数指针*/
    at24c02_status_t (*pf_inst)(
      bsp_at24c02_driver_t*           const p_aht21_instance,
      at24c02_i2c_driver_interface_t* const p_i2c_driver_interface,
      at24c02_yield_interface_t*      const p_yield_interface);
    /**初始化函数指针*/
    at24c02_status_t (*pf_init)         (bsp_at24c02_driver_t * const);
    /**反初始化函数指针*/
    at24c02_status_t (*pf_deinit)       (bsp_at24c02_driver_t * const);
    /**写字节函数指针*/
    at24c02_status_t (*pf_write_byte)(
    bsp_at24c02_driver_t * const p_driver,  // 设备实例（this指针）
    uint8_t                const addr,      // 目标内存地址（0x00~0xFF）
    uint8_t                const data);     // 待写入数据（0x00~0xFF）
    /**写页函数指针*/
    at24c02_status_t (*pf_write_page)(
    bsp_at24c02_driver_t * const p_driver,  // 设备实例
    uint8_t                const start_addr,// 页起始地址（0x00~0xFF）
    const uint8_t        *       p_data,    // 待写入数据缓冲区（非NULL）
    uint8_t                const len);      // 数据长度（1~8字节）
    /**读字节函数指针*/
    at24c02_status_t (*pf_read_byte)(
    bsp_at24c02_driver_t * const p_driver,  // 设备实例
    uint8_t                const addr,      // 目标内存地址（0x00~0xFF）
    uint8_t              * const p_data);   // 接收数据的缓冲区（非NULL）
    /**读连续字节函数指针*/
    at24c02_status_t (*pf_read_sequential)(
    bsp_at24c02_driver_t * const p_driver,  // 设备实例
    uint8_t                const start_addr,// 读取起始地址（0x00~0xFF）
    uint8_t              *       p_data,    // 接收数据的缓冲区（非NULL）
    uint16_t               const len);      // 读取长度（1~256字节）
}bsp_at24c02_driver_t;
//******************************** Classes **********************************//
//---------------------------------------------------------------------------//
//******************************** 函数声明 ***********************************//
at24c02_status_t bsp_at24c02_inst(
    bsp_at24c02_driver_t*           const p_driver_instance,
    at24c02_i2c_driver_interface_t* const p_i2c_if,
    at24c02_yield_interface_t*      const p_yield_if);
//******************************** 函数声明 ***********************************//

#endif //RTOS_PROJECT_AT24C02_DRIVER_H