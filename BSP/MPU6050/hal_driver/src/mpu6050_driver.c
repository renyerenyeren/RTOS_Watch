//
// Created by redmiX on 2025/11/21.
//
//******************************** Includes *********************************//
#include "mpu6050_driver.h"
#include "elog.h"
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//

#define MPU6050_NOT_INIT  0
#define MPU6050_INIT      1

#ifdef  LOG_TAG
#undef  LOG_TAG
#define LOG_TAG       "mpu6050_driver"
#else // else of LOG_TAG
#define LOG_TAG       "mpu6050_driver"
#endif // end of LOG_TAG
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Macros ***********************************//
#define MPU_DEBUG
#ifdef  MPU_DEBUG
#define LOG_DEBUG(x,...)  log_d(x, ##__VA_ARGS__)
#define LOG_ERROR(x,...)  log_e(x, ##__VA_ARGS__)
#else
#define LOG_DEBUG(x,...)   ((void)0)
#define LOG_ERROR(x,...)   ((void)0)
#endif

#define NULL_CHECK(x, tag)                          do{\
if(NULL == x)                                          \
{                                                      \
LOG_ERROR(#x" is null ptr");                           \
goto tag;}                                             \
}while (0)
//******************************** Macros ***********************************//
//---------------------------------------------------------------------------//
//******************************** Variables ********************************//

static uint8_t g_is_init_flag = MPU6050_NOT_INIT;
//******************************** Variables ********************************//
//---------------------------------------------------------------------------//
//******************************** Functions ********************************//

static mpu6050_status_t mpu_driver_sleep(bsp_mpu6050_driver_t *p_mpu6050)
{
    g_is_init_flag = MPU6050_NOT_INIT;
    return MPU6050_OK;
}

static mpu6050_status_t mpu_driver_wakeup(bsp_mpu6050_driver_t *p_mpu6050)
{
    mpu6050_status_t ret = MPU6050_OK;
    uint8_t data = 0x40; // 0b0100 0000
    ret =0;
}



mpu6050_status_t bsp_mpu6050_driver_init(bsp_mpu6050_driver_t *p_mpu6050)
{

}

mpu6050_status_t bsp_mpu6050_driver_inst(
    bsp_mpu6050_driver_t       *p_mpu6050_driver,
    mpu_i2c_driver_interface_t *p_i2c_driver_interface,
#ifdef OS_SUPPORTING
    yield_interface_t          *p_yield_interface,
    os_interface_t             *p_os_interfece,
#endif /* End of OS_SUPPORTING */
    delay_interface_t          *p_delay_interface,
    timebase_interface_t       *p_timebase_interface,
    void (*callback_register)    (void (*callback)(void *, void *)),
    void (*callback_register_dma)(void (*callback)(void *, void *))
#ifdef OS_SUPPORTING
    ,void *queue_handle,
    void *semaphore_handle,
    void *notify_handle
#endif /* End of OS_SUPPORTING */
                                 )
{
    mpu6050_status_t ret = MPU6050_OK;
    /****************************** 检查参数 ********************************/
    NULL_CHECK(p_mpu6050_driver,       mpu_driver_inst_null);
    NULL_CHECK(p_i2c_driver_interface, mpu_driver_inst_null);
#ifdef OS_SUPPORTING
    NULL_CHECK(p_yield_interface, mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece,    mpu_driver_inst_null);
#endif /* End of OS_SUPPORTING */
    NULL_CHECK(p_delay_interface,     mpu_driver_inst_null);
    NULL_CHECK(p_timebase_interface,  mpu_driver_inst_null);
    NULL_CHECK(callback_register,     mpu_driver_inst_null);
    NULL_CHECK(callback_register_dma, mpu_driver_inst_null);
#ifdef OS_SUPPORTING
    NULL_CHECK(queue_handle,     mpu_driver_inst_null);
    NULL_CHECK(semaphore_handle, mpu_driver_inst_null);
    NULL_CHECK(notify_handle,    mpu_driver_inst_null);
#endif /* End of OS_SUPPORTING */
    /****************************** 检查IIC参数 *****************************/
    NULL_CHECK(p_i2c_driver_interface->pf_i2c_init,      mpu_driver_inst_null);
    NULL_CHECK(p_i2c_driver_interface->pf_i2c_deinit,    mpu_driver_inst_null);
    NULL_CHECK(p_i2c_driver_interface->pf_i2c_mem_write, mpu_driver_inst_null);
    NULL_CHECK(p_i2c_driver_interface->pf_i2c_mem_read,  mpu_driver_inst_null);
    NULL_CHECK(p_i2c_driver_interface->pf_i2c_mem_read_dma,
                                                         mpu_driver_inst_null);
    /** 实例化i2c接口 */
    p_mpu6050_driver->p_i2c_driver_interface = p_i2c_driver_interface;
#ifdef OS_SUPPORTING
    //*************************** 检查OS延时参数 *******************************//
    NULL_CHECK(p_yield_interface->pf_rtos_yield, mpu_driver_inst_null);
    /** 实例化os延时接口 */
    p_mpu6050_driver->p_yield_interface = p_yield_interface;
    /**************************** 检查OS参数 ********************************/
    NULL_CHECK(p_os_interfece->os_queue_create,          mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_queue_put,             mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_queue_put_isr,         mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_queue_get,             mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_queue_delete,          mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_semaphore_create_mutex,mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_semaphore_delete_mutex,mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_semaphore_lock_mutex,  mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_semaphore_unlock_mutex,mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_semaphore_create_binary,
                                                         mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_semaphore_delete_binary,
                                                         mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_semaphore_wait_binary, mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_semaphore_signal_binary,
                                                         mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_semaphore_signal_binary_isr,
                                                         mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_semaphore_signal_notify_isr,
                                                         mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_semaphore_wait_notify, mpu_driver_inst_null);
    /** 实例化os操作接口接口 */
    p_mpu6050_driver->p_os_interface = p_os_interfece;
#endif /* End of OS_SUPPORTING */
    /**************************** 检查阻塞延时 ********************************/
    NULL_CHECK(p_delay_interface->pf_delay_init,         mpu_driver_inst_null);
    NULL_CHECK(p_delay_interface->pf_delay_us,           mpu_driver_inst_null);
    NULL_CHECK(p_delay_interface->pf_delay_ms,           mpu_driver_inst_null);
    /** 实例化阻塞延时接口 */
    p_mpu6050_driver->p_delay_interface = p_delay_interface;
    /*************************** 时基获取接口 *********************************/
    NULL_CHECK(p_timebase_interface->pf_get_tick_count,  mpu_driver_inst_null);
    /** 实例化获取时基接口 */
    p_mpu6050_driver->p_timebase_interface = p_timebase_interface;

    /** 实例化对外提供的接口 */
    p_mpu6050_driver->pf_deinit =                             mpu_driver_deinit;
    p_mpu6050_driver->pf_sleep =                               mpu_driver_sleep;
    p_mpu6050_driver->pf_wakeup =                             mpu_driver_wakeup;
    p_mpu6050_driver->pf_set_gyro_fsr =                 mpu_driver_set_gyro_fsr;
    p_mpu6050_driver->pf_set_accel_fsr =               mpu_driver_set_accel_fsr;
    p_mpu6050_driver->pf_set_lpf =                           mpu_driver_set_lpf;
    p_mpu6050_driver->pf_set_rate =                         mpu_driver_set_rate;
    p_mpu6050_driver->pf_set_interrupt_enable = mpu_driver_set_interrupt_enable;
    p_mpu6050_driver->pf_set_motion_threshold = mpu_driver_set_motion_threshold;
    p_mpu6050_driver->pf_set_INT_level =               mpu_driver_set_INT_level;
    p_mpu6050_driver->pf_set_user_ctrl =               mpu_driver_set_user_ctrl;
    p_mpu6050_driver->pf_set_pwr_mgmt1_reg =       mpu_driver_set_pwr_mgmt1_reg;
    p_mpu6050_driver->pf_set_pwr_mgmt2_reg =       mpu_driver_set_pwr_mgmt2_reg;
    p_mpu6050_driver->pf_set_fifo_en_reg =           mpu_driver_set_fifo_en_reg;
    p_mpu6050_driver->pf_get_temperature =           mpu_driver_get_temperature;
    p_mpu6050_driver->pf_get_accel =                       mpu_driver_get_accel;
    p_mpu6050_driver->pf_get_gyro =                         mpu_driver_get_gyro;
    p_mpu6050_driver->pf_get_all_data =                 mpu_driver_get_all_data;
    p_mpu6050_driver->pf_get_interrupt_status_reg =
                                            mpu_driver_get_interrupt_status_reg;
    p_mpu6050_driver->pf_read_fifo_packet =         mpu_driver_read_fifo_packet;
    p_mpu6050_driver->pf_read_fifo_isr_occur =   mpu_driver_read_fifo_isr_occur;

    p_mpu6050_driver->queue_handle = queue_handle;
    p_mpu6050_driver->semaphore_binary_handle = semaphore_handle;
    p_mpu6050_driver->notify_handle = notify_handle;

    ret = bsp_mpu6050_driver_init(p_mpu6050_driver);
    if (MPU6050_OK != ret)
    {
        LOG_ERROR("mpu init is ng");
        return MPU6050_ERROR;
    }

mpu_driver_inst_null:
    {
        LOG_ERROR("bsp_mpu6050_driver inst");
        return MPU6050_ERRORPARAMETER;
    }
}