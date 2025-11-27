//
// Created by redmiX on 2025/11/25.
//
//******************************** Includes *********************************//
#include "imu_handler.h"
#include "circular_buffer.h"
#include "elog.h"
#include "FreeRTOS.h"
#include "task.h"
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
#ifdef  LOG_TAG
#undef  LOG_TAG
#define LOG_TAG       "imu_handler"
#else // else of LOG_TAG
#define LOG_TAG       "imu_handler"
#endif // end of LOG_TAG

#define HANDLER_UNINITIALIZED   0
#define HANDLER_INITIALIZED     1
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Macros ***********************************//

#define IMU_DEBUG
#ifdef  IMU_DEBUG
#define LOG_DEBUG(x,...)  log_d(x, ##__VA_ARGS__)
#define LOG_ERROR(x,...)  log_e(x, ##__VA_ARGS__)
#else
#define LOG_DEBUG(x,...)   ((void)0)
#define LOG_ERROR(x,...)   ((void)0)
#endif

#define NULL_CHECK(x, tag)                              do{\
if(NULL == x)                                              \
{                                                          \
LOG_ERROR(#x" is null ptr");                               \
goto tag;}                                                 \
}while (0)

#define ERROR_CHECK(EERO_NUM, NUM_EXPECT_VAL, LOG, tag) do{\
if((EERO_NUM) != (NUM_EXPECT_VAL))                         \
{                                                          \
LOG_ERROR((LOG));                                          \
goto tag;                                                  \
}}while (0)
//******************************** Macros ***********************************//
//---------------------------------------------------------------------------//
//******************************** Variables ********************************//
static uint8_t imu_handler_init_flag = HANDLER_UNINITIALIZED;
//******************************** Variables ********************************//
//---------------------------------------------------------------------------//
//******************************** Functions ********************************//
extern void (*pf_pin_interrupt_callback)(void*, void*);
extern void (*pf_DMA_interrupt_callback)(void*, void*);

//把参数传入的函数注册进pf_pin_interrupt_callback
void register_callback_pin(void (*callback)(void*,void*))
{
    pf_pin_interrupt_callback = callback;
}
//把参数传入的函数注册进pf_DMA_interrupt_callback
void register_callback_dma(void (*callback)(void*,void*))
{
    pf_DMA_interrupt_callback = callback;
}

/**
 * @brief 初始化IMU处理器
 * 此函数用于初始化IMU处理器，包括创建队列以及实例化MPU6050驱动程序。
 * 如果已经初始化过，则返回错误。
 * @param[in] pHandler 指向IMU处理器结构体的指针
 * @return 执行状态
 */
mpu6050_status_t imu_handler_init(bsp_imu_handler_t* pHandler)
{
    /****************************** 检查参数 ********************************/
    NULL_CHECK(pHandler,         NULLPTR_init_error);
    NULL_CHECK(pHandler->pDriver, NULLPTR_init_error);
    if (HANDLER_INITIALIZED== imu_handler_init_flag)
    {
        LOG_ERROR("imu not need init again");
        return MPU6050_ERRORRESOURCE;
    }

    mpu6050_status_t ret = MPU6050_OK;
    ret = pHandler->pOS->os_queue_create(pHandler->Queue_length,
                                         pHandler->Queue_item_size,
                                         pHandler->pUnpack_queue_handle);
    ERROR_CHECK(ret, MPU6050_OK, "os_queue_create error", RETURN_init_error);
    ret = bsp_mpu6050_driver_inst(pHandler->pDriver,
                                  pHandler->pIIC_driver,
                                  pHandler->pYield,
                                  pHandler->pOS,
                                  pHandler->pDelay,
                                  pHandler->pTimebase,
                                  register_callback_pin,
                                  register_callback_dma,
                                  pHandler->Queue_handle,
                                  pHandler->semaphore_binary_handle,
                                  pHandler->notify_handle);
    ERROR_CHECK(ret, MPU6050_OK, "bsp_mpu6050_driver_inst error",
                                              RETURN_init_error);
    return ret;

NULLPTR_init_error:
    {
        LOG_ERROR("the driver is null");
        return MPU6050_ERRORPARAMETER;
    }
RETURN_init_error:
    {
        LOG_ERROR("function is return a error");
        return MPU6050_ERROR;
    }
}

/**
 * @brief 实例化IMU处理器并完成初始化
 * 此函数用于设置IMU处理器所需的外部依赖，并调用初始化函数。
 * 包括IIC驱动、延时函数、时间基准、调度让出函数以及操作系统抽象层等。
 * @param[in] pHandler 指向IMU处理器结构体的指针
 * @param[in] pInput_api 指向输入API配置结构体的指针
 * @return 执行状态
 */
mpu6050_status_t imu_handler_inst(bsp_imu_handler_t* pHandler,
                            imu_handler_input_api_t* pInput_api)
{
    LOG_DEBUG("===imu inst is start===");
    /****************************** 检查参数 ********************************/
    NULL_CHECK(pHandler,                NULLPTR_inst_error);
    NULL_CHECK(pInput_api,              NULLPTR_inst_error);
    NULL_CHECK(pInput_api->pIIC_driver, NULLPTR_inst_error);
    NULL_CHECK(pInput_api->pDelay,      NULLPTR_inst_error);
    NULL_CHECK(pInput_api->pTimebase,   NULLPTR_inst_error);
    NULL_CHECK(pInput_api->pYield,      NULLPTR_inst_error);
    NULL_CHECK(pInput_api->pOS,         NULLPTR_inst_error);

    pHandler->pIIC_driver = pInput_api->pIIC_driver;
    pHandler->pDelay      = pInput_api->pDelay;
    pHandler->pTimebase   = pInput_api->pTimebase;
    pHandler->pYield      = pInput_api->pYield;
    pHandler->pOS         = pInput_api->pOS;

    mpu6050_status_t ret = MPU6050_OK;
    ret = imu_handler_init(pHandler);
    ERROR_CHECK(ret, MPU6050_OK, "imu_handler_init error", RETURN_inst_error);
    return ret;


NULLPTR_inst_error:
    {
        LOG_ERROR("check ptr");
        return MPU6050_ERRORPARAMETER;
    }
RETURN_inst_error:
    {
        LOG_ERROR("function is return a error");
        return MPU6050_ERROR;
    }
}

void imu_handler_thread(void* argument)
{
    LOG_DEBUG("imu_handler[mpu6050] is start");
    imu_handler_input_api_t* input_api = NULL;
    NULL_CHECK(argument, NULLPTR_thread_error);
    input_api = (imu_handler_input_api_t*)argument;
    mpu6050_status_t ret = MPU6050_OK;
    uint8_t data = 0;

    /** 初始化环形缓冲区，分配10*14个字节 */
    buffer_init(&mpu_circular_buffer, 10);

    // MPU6050 实例
    bsp_mpu6050_driver_t bsp_mpu6050_driver;
    // IMUHandler实例结构体
    bsp_imu_handler_t imu_handler_instance = {0};
    imu_handler_instance.pDriver                 = &bsp_mpu6050_driver;
    imu_handler_instance.Queue_handle            = NULL;
    imu_handler_instance.pUnpack_queue_handle    = NULL;
    imu_handler_instance.Queue_length            = 20;
    imu_handler_instance.Queue_item_size         = 1;
    imu_handler_instance.semaphore_binary_handle = NULL;
    ret = imu_handler_inst(&imu_handler_instance, input_api);
    ERROR_CHECK(ret, MPU6050_OK,"imu_handler_inst error",RETURN_thread_error);
    LOG_DEBUG("=====imu_handler_inst success=====");

    for (;;)
    {
/*********************************************************/
#if 0 // queue test
        ret = imu_handler_instance.pOS->os_queue_get(
                                        imu_handler_instance.Queue_handle,
                                        &data,
                                        0xffffffff);
        ERROR_CHECK(ret,MPU6050_OK,"os_queue_get error",RETURN_thread_error);
        LOG_DEBUG("imu_handler_thread: data = %d", data);
#endif// queue test

/*********************************************************/
#if 0 // binary test
        ret = imu_handler_instance.pOS->os_semaphore_wait_binary(
                        imu_handler_instance.semaphore_binary_handle);
        ERROR_CHECK(ret, MPU6050_OK, "os_semaphore_wait_binary error",
                                                 RETURN_thread_error);
#endif // binary test

/*********************************************************/
#if 0 // notify test
        ret = imu_handler_instance.pOS->os_semaphore_wait_notify(0,
                                                                 0,
                                                                 NULL,
                                                                 0xffffffff);
#endif // End of notify test

/*********************************************************/
#if 1 // global variable test
        if (1 == mpu6050_flag_read())
        {
            // Put the data into the unpack queue for unpacking
            ret = imu_handler_instance.pOS->os_queue_put(
                imu_handler_instance.pUnpack_queue_handle,
                                               &data,
                                               0);
            ERROR_CHECK(ret, MPU6050_OK, "os_queue_put error",
                                         RETURN_thread_error);
            mpu6050_flag_set(0);
        }
#endif // End of notify test
        vTaskDelay(1);
    }


NULLPTR_thread_error:
    {
        LOG_ERROR("input api is null");
        vTaskDelete(NULL);
    }
RETURN_thread_error:
    {
        LOG_ERROR("imu_handler_inst failed");
        vTaskDelete(NULL);
    }
}