//
// Created by redmiX on 2025/11/20.
//

#ifndef RTOS_PROJECT_MPU6050_DRIVER_H
#define RTOS_PROJECT_MPU6050_DRIVER_H

//******************************** Includes *********************************//
#include <stdint.h>

//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
#define OS_SUPPORTING 1            /* OS supporting                          */
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Typedefs *********************************//
/*   函数返回值   */
typedef enum
{
    MPU6050_OK             = 0,          //* 操作执行成功
    MPU6050_ERROR          = 1,          //* 运行时错误：无匹配场景
    MPU6050_ERRORTIMEOUT   = 2,          //* 操作失败：超时
    MPU6050_ERRORRESOURCE  = 3,          //* 资源不可用
    MPU6050_ERRORPARAMETER = 4,          //* 参数错误
    MPU6050_ERRORNOMEMORY  = 5,          //* 内存不足
    MPU6050_ERRORISR       = 6,          //* ISR上下文不允许此操作
    MPU6050_RESERVED       = 0x7FFFFFFF  //* 保留
}mpu6050_status_t;

/** mpu6050 数据格式 */
typedef struct
{
    /* 来自传感器的原始加速度计数据 */
    int16_t accel_x_raw;
    int16_t accel_y_raw;
    int16_t accel_z_raw;

    /* 已处理的加速度计数据，单位为g */
    double ax;
    double ay;
    double az;

    /* 来自传感器的原始角速度计数据 */
    int16_t gyro_x_raw;
    int16_t gyro_y_raw;
    int16_t gyro_z_raw;

    /* 已处理的角速度计数据，单位为度/秒 */
    double gx;
    double gy;
    double gz;

    /* 温度读数，单位为摄氏度 */
    float temperature;

    /* 卡尔曼滤波处理后的角度 */
    double kalman_angle_x;
    double kalman_angle_y;
}mpu6050_data_t;
//******************************** Typedefs *********************************//
//---------------------------------------------------------------------------//
//**************************** Interface Structs ****************************//
/** From HAL Layer :       HAL_IIC    */
typedef struct
{
    void* hi2c;       /* hi2c：指向 I2C_HandleTypeDef 结构体的指针 */
    mpu6050_status_t (*pf_i2c_init)        (void*); /* IIC init    interface */
    mpu6050_status_t (*pf_i2c_deinit)      (void*); /* IIC deinit  interface */
    mpu6050_status_t (*pf_i2c_mem_write)   (void* hi2c,
                                            uint16_t dst_address,
                                            uint16_t mem_addr,
                                            uint16_t mem_size,
                                            uint8_t* p_data,
                                            uint16_t size,
                                            uint32_t timeout);
    mpu6050_status_t (*pf_i2c_mem_read)    (void* hi2c,
                                             uint16_t dst_address,
                                             uint16_t mem_addr,
                                             uint16_t mem_size,
                                             uint8_t* p_data,
                                             uint16_t size,
                                             uint32_t timeout);
    //* 使用DMA异步读取I2C设备寄存器的数据
    mpu6050_status_t (*pf_i2c_mem_read_dma)(void* hi2c,
                                            uint16_t dst_address,
                                            uint16_t mem_addr,
                                            uint16_t mem_size,
                                            uint8_t* p_data,
                                            uint16_t size);
}mpu_i2c_driver_interface_t;

/** 硬件中断 */
typedef struct
{
    mpu6050_status_t (*pf_init)             (void);
    mpu6050_status_t (*pf_deinit)           (void);
    mpu6050_status_t (*pf_enable_interrupt) (void);
    mpu6050_status_t (*pf_disable_interrupt)(void);
    mpu6050_status_t (*pf_enable_clock)     (void);
    mpu6050_status_t (*pf_disable_clock)    (void);
}mpu_hardware_interrupt_interface_t;

typedef struct
{
    void (*pf_delay_init)(void);             /* Delay init interface    */
    void (*pf_delay_us)  (const uint32_t us);/* Delay us interface      */
    void (*pf_delay_ms)  (const uint32_t ms);/* Delay ms interface      */
}mpu_delay_interface_t;

/** Form Core Layer :    TimeBase     */
typedef struct
{
    uint32_t (*pf_get_tick_count) (void); /* Get tick count interface */
}mpu_timebase_interface_t;

/** 存储来自 mpu6050 驱动的数据 */
typedef struct
{
    uint8_t *(*pf_buffer_init)     (uint8_t size);
    uint8_t *(*pf_get_rbuffer_addr)(void);
    uint8_t *(*pf_get_wbuffer_addr)(void);
}mpu_buffer_interface_t;

/** From OS Layer :       OS_Delay    */
#ifdef OS_SUPPORTING
typedef struct
{
    void (*pf_rtos_yield)(const uint32_t);/* OS No-Blocking delay  */
}mpu_yield_interface_t;

/** os操作接口 */
typedef struct
{
    mpu6050_status_t (*os_queue_create) (uint32_t const item_num,
                                         uint32_t const item_size,
                                         void** queue_handle);
    mpu6050_status_t (*os_queue_put)    (void* const queue_handle,
                                         void* const item,
                                         uint32_t const timeout);
    mpu6050_status_t (*os_queue_put_isr)(void* const queue_handle,
                                         void* const item,
                                         long* const HigherPriorityTaskWoken);
    mpu6050_status_t (*os_queue_get)    (void* const queue_handle,
                                         void* const item,
                                         uint32_t const timeout);
    mpu6050_status_t (*os_queue_delete) (void * const queue_handle);

    mpu6050_status_t (*os_semaphore_create_mutex)(void** mutex_handle);
    mpu6050_status_t (*os_semaphore_delete_mutex)(void* const mutex_handle);
    mpu6050_status_t (*os_semaphore_lock_mutex)  (void* const mutex_handle);
    mpu6050_status_t (*os_semaphore_unlock_mutex)(void* const mutex_handle);

    mpu6050_status_t (*os_semaphore_create_binary)(void** binary_handle);
    mpu6050_status_t (*os_semaphore_delete_binary)(void* const binary_handle);
    mpu6050_status_t (*os_semaphore_wait_binary)  (void* const binary_handle);
    mpu6050_status_t (*os_semaphore_signal_binary)(void* const binary_handle);
    mpu6050_status_t (*os_semaphore_signal_binary_isr)(
                                void* const binary_handle,
                                long* const HigherPriorityTaskWoken);

    mpu6050_status_t (*os_semaphore_signal_notify_isr)(
                                void * const notify_handle,
                                uint32_t ulValue,
                                uint32_t eAction,
                                long * const HigherPriorityTaskWoken);
    mpu6050_status_t (*os_semaphore_wait_notify)(
                                uint32_t ulBitsToClearOnEntry,
                                uint32_t ulBitsToClearOnExit,
                                uint32_t *pulNotificationValue,
                                uint32_t timeout);
}os_interface_t;
#endif /* End of OS_SUPPORTING       */
//**************************** Interface Structs ****************************//
//---------------------------------------------------------------------------//
//******************************** Classes **********************************//
typedef struct bsp_mpu6050_driver bsp_mpu6050_driver_t;
typedef struct bsp_mpu6050_driver
{
    /** 底层需要的接口 */
    mpu_i2c_driver_interface_t         *p_i2c_driver_interface;
    mpu_hardware_interrupt_interface_t *p_interrupt_interface;
    mpu_delay_interface_t              *p_delay_interface;
    mpu_timebase_interface_t           *p_timebase_interface;

    /** os操作接口 */
#ifdef OS_SUPPORTING
    mpu_buffer_interface_t *p_buffer_interface;
    mpu_yield_interface_t  *p_yield_interface;
    os_interface_t         *p_os_interface;

    void *queue_handle;            /*  消息队列句柄  */
    void *semaphore_mutex_handle;  /*  互斥锁句柄   */
    void *semaphore_binary_handle; /* 二值信号量句柄 */
    void *notify_handle;           /*  任务通知句柄  */

    /** 回调函数 */
    void (*pf_dma_completed_callback)(void);
    void (*pf_int_interrupt_callback)(void);
#endif /* End of OS_SUPPORTING */

    /** MPU6050 传感器驱动的对外的接口 */
    /** 反初始化MPU6050传感器 */
    mpu6050_status_t (*pf_deinit)(bsp_mpu6050_driver_t *);
    /** 将MPU6050传感器置于睡眠模式 */
    mpu6050_status_t (*pf_sleep)(bsp_mpu6050_driver_t *);
    /** 将MPU6050传感器从睡眠模式唤醒 */
    mpu6050_status_t (*pf_wakeup)(bsp_mpu6050_driver_t *);
    /** 设置陀螺仪的满量程范围 (Full Scale Range) */
    mpu6050_status_t (*pf_set_gyro_fsr)        (bsp_mpu6050_driver_t *, uint8_t);
    /** 设置加速度计的满量程范围 (Full Scale Range) */
    mpu6050_status_t (*pf_set_accel_fsr)       (bsp_mpu6050_driver_t *, uint8_t);
    /** 设置低通滤波器 (Low Pass Filter) 的截止频率 */
    mpu6050_status_t (*pf_set_lpf)             (bsp_mpu6050_driver_t *, uint8_t);
    /** 设置传感器的数据输出速率 (Sample Rate) */
    mpu6050_status_t (*pf_set_rate)            (bsp_mpu6050_driver_t *, uint8_t);
    /** 使能或禁用MPU6050的中断 */
    mpu6050_status_t (*pf_set_interrupt_enable)(bsp_mpu6050_driver_t *, uint8_t);
    /** 设置运动检测的加速度阈值 */
    mpu6050_status_t (*pf_set_motion_threshold)(bsp_mpu6050_driver_t *, uint8_t);
    /** 设置INT引脚的有效电平 (高电平或低电平有效) */
    mpu6050_status_t (*pf_set_INT_level)       (bsp_mpu6050_driver_t *, uint8_t);
    /** 设置用户控制寄存器 (例如：I2C主模式控制) */
    mpu6050_status_t (*pf_set_user_ctrl)       (bsp_mpu6050_driver_t *, uint8_t);
    /** 设置电源管理1寄存器 (例如：唤醒、睡眠、复位) */
    mpu6050_status_t (*pf_set_pwr_mgmt1_reg)   (bsp_mpu6050_driver_t *, uint8_t);
    /** 设置电源管理2寄存器 (例如：禁用加速度计/陀螺仪轴) */
    mpu6050_status_t (*pf_set_pwr_mgmt2_reg)   (bsp_mpu6050_driver_t *, uint8_t);
    /** 设置FIFO使能寄存器 (选择哪些数据写入FIFO) */
    mpu6050_status_t (*pf_set_fifo_en_reg)     (bsp_mpu6050_driver_t *, uint8_t);
    /** 读取MPU6050的温度数据 */
    mpu6050_status_t (*pf_get_temperature)     (bsp_mpu6050_driver_t *,
                                                mpu6050_data_t *);
    /** 读取MPU6050的加速度计数据 */
    mpu6050_status_t (*pf_get_accel)           (bsp_mpu6050_driver_t *,
                                                mpu6050_data_t *);
    /** 读取MPU6050的陀螺仪数据 */
    mpu6050_status_t (*pf_get_gyro)            (bsp_mpu6050_driver_t *,
                                                mpu6050_data_t *);
    /** 一次性读取MPU6050的所有传感器数据 (加速度计、陀螺仪、温度) */
    mpu6050_status_t (*pf_get_all_data)        (bsp_mpu6050_driver_t *,
                                                mpu6050_data_t *);
    /** 读取中断状态寄存器的值 */
    mpu6050_status_t (*pf_get_interrupt_status_reg)(bsp_mpu6050_driver_t *,
                                                    uint8_t *);
    /** 从FIFO缓冲区中读取一个数据包 */
    mpu6050_status_t (*pf_read_fifo_packet)(
                bsp_mpu6050_driver_t *p_mpu_driver,
                mpu6050_data_t *p_data);
    /** 在中断发生时，从FIFO缓冲区中读取数据 */
    mpu6050_status_t (*pf_read_fifo_isr_occur)(
                bsp_mpu6050_driver_t *p_mpu_driver,
                mpu6050_data_t *p_data);
} bsp_mpu6050_driver_t;
//******************************** Classes **********************************//
//---------------------------------------------------------------------------//
//******************************** 函数声明 ***********************************//
mpu6050_status_t bsp_mpu6050_driver_inst(
    bsp_mpu6050_driver_t       *p_mpu6050_driver,
    mpu_i2c_driver_interface_t *p_i2c_driver_interface,
#ifdef OS_SUPPORTING
    mpu_yield_interface_t      *p_yield_interface,
    os_interface_t             *p_os_interfece,
#endif /* End of OS_SUPPORTING */
    mpu_delay_interface_t      *p_delay_interface,
    mpu_timebase_interface_t   *p_timebase_interface,
    void (*callback_register)    (void (*callback)(void *, void *)),
    void (*callback_register_dma)(void (*callback)(void *, void *))
#ifdef OS_SUPPORTING
    ,void *queue_handle,
    void *semaphore_handle,
    void *notify_handle
#endif /* End of OS_SUPPORTING */
                                 );
uint32_t mpu6050_flag_read(void);
void mpu6050_flag_set(uint8_t flag);

//******************************** 函数声明 ***********************************//

#endif //RTOS_PROJECT_MPU6050_DRIVER_H