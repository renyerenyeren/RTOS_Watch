//
// Created by redmiX on 2025/12/1.
//
//******************************** Includes *********************************//
#include "aht21_system_adaption.h"
#include "mpu6050_driver.h"
#include "imu_handler.h"
#include "stm32f4xx_hal.h"
#include "i2c.h"
#include "DWT_delay.h"
#include "elog.h"
#include "FreeRTOS.h"
#include "portmacro.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
#ifdef  LOG_TAG
#undef  LOG_TAG
#define LOG_TAG       "imu_adaption"
#else // else of LOG_TAG
#define LOG_TAG       "imu_adaption"
#endif // end of LOG_TAG
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Macros ***********************************//

#define IMU_ADAPTION_DEBUG
#ifdef  IMU_ADAPTION_DEBUG
#define LOG_DEBUG(x,...)  log_d(x, ##__VA_ARGS__)
#define LOG_ERROR(x,...)  log_e(x, ##__VA_ARGS__)
#else
#define LOG_DEBUG(x,...)   ((void)0)
#define LOG_ERROR(x,...)   ((void)0)
#endif

#define NULL_CHECK(x, title)                   do{\
if(NULL == x)                                     \
{                                                 \
log_e("PRT IS NULL");                             \
goto title;                                       \
}}while (0)

#define ERROR_CHECK(EERO_NUM, NUM_EXPECT_VAL, LOG, tag) do{\
if((EERO_NUM) != (NUM_EXPECT_VAL))                         \
{                                                          \
LOG_ERROR((LOG));                                          \
goto tag;                                                  \
}}while (0)
//******************************** Macros ***********************************//
//---------------------------------------------------------------------------//
//******************************** 函数声明   *********************************//
static mpu6050_status_t i2c_init_myown(void* pIIC);
static mpu6050_status_t i2c_deinit_myown(void* pIIC);
static mpu6050_status_t i2c_mem_write_myown(void *hi2c,
                                     uint16_t slave_addr_8bit,
                                     uint16_t mem_addr,
                                     uint16_t mem_size,
                                     uint8_t *pData,
                                     uint16_t size,
                                     uint32_t timeout);
static mpu6050_status_t i2c_mem_read_myown(void* hi2c,
                                    uint16_t slave_addr_8bit,
                                    uint16_t mem_addr,
                                    uint16_t mem_size,
                                    uint8_t* pData,
                                    uint16_t size,
                                    uint32_t timeout);
static mpu6050_status_t i2c_mem_read_dma_myown(void* hi2c,
                                        uint16_t slave_addr_8bit,
                                        uint16_t mem_addr,
                                        uint16_t mem_size,
                                        uint8_t* pData,
                                        uint16_t size);

static void os_Delay_myown(const uint32_t time_ms);

static mpu6050_status_t os_queue_create_myown(uint32_t const queue_size,
                                       uint32_t const item_size,
                                       void** queue_handle);
static mpu6050_status_t os_queue_put_myown(void*   const queue_handle,
                                    void*   const item,
                                    uint32_t const timeout);
static mpu6050_status_t os_queue_put_isr_myown(void* const queue_handle,
                                        void* const item,
                                        long* const HigherPriorityTaskWoken);
static mpu6050_status_t os_queue_get_myown(void*    const queue_handle,
                                    void*    const item,
                                    uint32_t const timeout);
static mpu6050_status_t os_queue_delete_myown(void* const queue_handle);

static mpu6050_status_t os_semaphore_create_mutex_myown(void** mutex_handle);
static mpu6050_status_t os_semaphore_delete_mutex_myown(void* mutex_handle);
static mpu6050_status_t os_semaphore_lock_mutex_myown(void* mutex_handle);
static mpu6050_status_t os_semaphore_unlock_mutex_myown(void* mutex_handle);

static mpu6050_status_t os_semaphore_create_binary_myown(void** binary_handle);
static mpu6050_status_t os_semaphore_delete_binary_myown(void* binary_handle);
static mpu6050_status_t os_semaphore_wait_binary_myown(void* binary_handle);
static mpu6050_status_t os_semaphore_signal_binary_myown(void* binary_handle);
static mpu6050_status_t os_semaphore_signal_binary_isr_myown(void* binary_handle,
                                            long* HigherPriorityTaskWoken);
static mpu6050_status_t os_semaphore_signal_notify_isr_myown(
                                    void * const notify_handle,
                                    uint32_t ulValue,
                                    uint32_t eAction,
                                    long * const HigherPriorityTaskWoken);
static mpu6050_status_t os_semaphore_wait_notify_myown(uint32_t ulBitsToClearOnEntry,
                                         uint32_t ulBitsToClearOnExit,
                                         uint32_t *pulNotificationValue,
                                         uint32_t timeout);
//******************************** 函数声明   *********************************//
//---------------------------------------------------------------------------//
//******************************** Variables ********************************//
// 1. 初始化i2c驱动接口结构体
static mpu_i2c_driver_interface_t i2c_driver_interface = {
    .hi2c = &hi2c1,
    .pf_i2c_init = i2c_init_myown,
    .pf_i2c_deinit = i2c_deinit_myown,
    .pf_i2c_mem_write = i2c_mem_write_myown,
    .pf_i2c_mem_read = i2c_mem_read_myown,
    .pf_i2c_mem_read_dma = i2c_mem_read_dma_myown
};
// 2. 初始化延时函数接口结构体
static mpu_delay_interface_t delay_interface = {
    .pf_delay_init = DWT_Delay_Init,
    .pf_delay_us = DWT_Delay_us,
    .pf_delay_ms = DWT_Delay_ms
};
// 3. 初始化获取时基接口结构体
static mpu_timebase_interface_t timebase_interface = {
    .pf_get_tick_count = HAL_GetTick
};
// 4. 初始化OS延时函数接口结构体
static mpu_yield_interface_t yield_interface = {
    .pf_rtos_yield = os_Delay_myown
};
// 5， 初始化os操作接口结构体
static os_interface_t os_interface = {
    .os_queue_create = os_queue_create_myown,
    .os_queue_put = os_queue_put_myown,
    .os_queue_put_isr = os_queue_put_isr_myown,
    .os_queue_get = os_queue_get_myown,
    .os_queue_delete = os_queue_delete_myown,
    .os_semaphore_create_mutex = os_semaphore_create_mutex_myown,
    .os_semaphore_delete_mutex = os_semaphore_delete_mutex_myown,
    .os_semaphore_lock_mutex = os_semaphore_lock_mutex_myown,
    .os_semaphore_unlock_mutex = os_semaphore_unlock_mutex_myown,
    .os_semaphore_create_binary = os_semaphore_create_binary_myown,
    .os_semaphore_delete_binary = os_semaphore_delete_binary_myown,
    .os_semaphore_wait_binary = os_semaphore_wait_binary_myown,
    .os_semaphore_signal_binary = os_semaphore_signal_binary_myown,
    .os_semaphore_signal_binary_isr = os_semaphore_signal_binary_isr_myown,
    .os_semaphore_signal_notify_isr = os_semaphore_signal_notify_isr_myown,
    .os_semaphore_wait_notify = os_semaphore_wait_notify_myown
};
// 6. 创建imu系统接口结构体
static imu_handler_input_api_t imu_api = {
    .pIIC_driver = &i2c_driver_interface,
    .pDelay = &delay_interface,
    .pTimebase = &timebase_interface,
    .pYield = &yield_interface,
    .pOS = &os_interface
};
//******************************** Variables ********************************//
//---------------------------------------------------------------------------//
//******************************** Functions ********************************//
static mpu6050_status_t i2c_init_myown(void* pIIC)
{
    return MPU6050_OK;
}
static mpu6050_status_t i2c_deinit_myown(void* pIIC)
{
    __HAL_RCC_I2C2_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7);
    return MPU6050_OK;
}
static mpu6050_status_t i2c_mem_write_myown(void *hi2c,
                                     uint16_t slave_addr_8bit,
                                     uint16_t mem_addr,
                                     uint16_t mem_size,
                                     uint8_t *pData,
                                     uint16_t size,
                                     uint32_t timeout)
{
    HAL_StatusTypeDef ret = HAL_OK;
    ret = HAL_I2C_Mem_Write(hi2c,
                 slave_addr_8bit,
                 mem_addr,
                 mem_size,
                 pData, size, timeout);
    if (ret != HAL_OK)
    {
        return MPU6050_ERROR;
    }
    return MPU6050_OK;
}
static mpu6050_status_t i2c_mem_read_myown(void* hi2c,
                                    uint16_t slave_addr_8bit,
                                    uint16_t mem_addr,
                                    uint16_t mem_size,
                                    uint8_t* pData,
                                    uint16_t size,
                                    uint32_t timeout)
{
    HAL_StatusTypeDef ret = HAL_OK;
    ret = HAL_I2C_Mem_Read(hi2c,
                 slave_addr_8bit,
                 mem_addr,
                 mem_size,
                 pData, size, timeout);
    if (ret != HAL_OK)
    {
        return MPU6050_ERROR;
    }
    return MPU6050_OK;
}
static mpu6050_status_t i2c_mem_read_dma_myown(void* hi2c,
                                        uint16_t slave_addr_8bit,
                                        uint16_t mem_addr,
                                        uint16_t mem_size,
                                        uint8_t* pData,
                                        uint16_t size)
{
    HAL_StatusTypeDef ret = HAL_OK;
    ret = HAL_I2C_Mem_Read_DMA(hi2c,
                 slave_addr_8bit,
                 mem_addr,
                 mem_size,
                 pData, size);
    if (ret != HAL_OK)
    {
        return MPU6050_ERROR;
    }
    return MPU6050_OK;
}

static void os_Delay_myown(const uint32_t time_ms)
{
    vTaskDelay(pdMS_TO_TICKS(time_ms));
}

static mpu6050_status_t os_queue_create_myown(uint32_t const queue_size,
                                       uint32_t const item_size,
                                       void** queue_handle)
{
    *queue_handle = xQueueCreate(queue_size, item_size);
    if (NULL == *queue_handle) return MPU6050_ERRORRESOURCE;
    return MPU6050_OK;
}
static mpu6050_status_t os_queue_put_myown(void*   const queue_handle,
                                    void*   const item,
                                    uint32_t const timeout)
{
    BaseType_t ret = xQueueSend(queue_handle, item, timeout);
    ERROR_CHECK(ret, pdPASS, "queue put error", os_queue_put_error);
    return MPU6050_OK;

os_queue_put_error:
    {
        return MPU6050_ERRORRESOURCE;
    }
}
static mpu6050_status_t os_queue_put_isr_myown(void* const queue_handle,
                                        void* const item,
                                        long* const HigherPriorityTaskWoken)
{
    BaseType_t ret = xQueueSendFromISR(queue_handle, item,
                                       HigherPriorityTaskWoken);
    ERROR_CHECK(ret, pdPASS, "queueisr put error", os_queue_put_error);
    return MPU6050_OK;

os_queue_put_error:
    {
        return MPU6050_ERRORRESOURCE;
    }
}
static mpu6050_status_t os_queue_get_myown(void*    const queue_handle,
                                    void*    const item,
                                    uint32_t const timeout)
{
    BaseType_t ret = xQueueReceive(queue_handle, item, timeout);
    ERROR_CHECK(ret, pdPASS, "queue get error", os_queue_get_error);
    return MPU6050_OK;

os_queue_get_error:
    {
        return MPU6050_ERRORRESOURCE;
    }
}
static mpu6050_status_t os_queue_delete_myown(void* const queue_handle)
{
    vQueueDelete(queue_handle);
    return MPU6050_OK;
}

static mpu6050_status_t os_semaphore_create_mutex_myown(void** mutex_handle)
{
    *mutex_handle = xSemaphoreCreateMutex();
    NULL_CHECK(*mutex_handle, os_semaphore_create_mutex_error);
    return MPU6050_OK;

os_semaphore_create_mutex_error:
    {
        return MPU6050_ERRORRESOURCE;
    }
}
static mpu6050_status_t os_semaphore_delete_mutex_myown(void* mutex_handle)
{
    //vQueueDelete(mutex_handle);
    return MPU6050_OK;
}
static mpu6050_status_t os_semaphore_lock_mutex_myown(void* mutex_handle)
{
    xSemaphoreTake(mutex_handle, portMAX_DELAY);
    return MPU6050_OK;
}
static mpu6050_status_t os_semaphore_unlock_mutex_myown(void* mutex_handle)
{
    xSemaphoreGive(mutex_handle);
    return MPU6050_OK;
}

static mpu6050_status_t os_semaphore_create_binary_myown(void** binary_handle)
{
    *binary_handle = xSemaphoreCreateBinary();
    return MPU6050_OK;
}
static mpu6050_status_t os_semaphore_delete_binary_myown(void* binary_handle)
{
    //vQueueDelete(binary_handle);
    return MPU6050_OK;
}
static mpu6050_status_t os_semaphore_wait_binary_myown(void* binary_handle)
{
    xSemaphoreGive(binary_handle);
    return MPU6050_OK;
}
static mpu6050_status_t os_semaphore_signal_binary_myown(void* binary_handle)
{
    xSemaphoreTake(binary_handle, portMAX_DELAY);
    return MPU6050_OK;
}
static mpu6050_status_t os_semaphore_signal_binary_isr_myown(void* binary_handle,
                                            long* HigherPriorityTaskWoken)
{
    return MPU6050_OK;
}
static mpu6050_status_t os_semaphore_signal_notify_isr_myown(
                                    void * const notify_handle,
                                    uint32_t ulValue,
                                    uint32_t eAction,
                                    long * const HigherPriorityTaskWoken)
{
    return MPU6050_OK;
}
static mpu6050_status_t os_semaphore_wait_notify_myown(uint32_t ulBitsToClearOnEntry,
                                         uint32_t ulBitsToClearOnExit,
                                         uint32_t *pulNotificationValue,
                                         uint32_t timeout)
{
    return MPU6050_OK;
}

TaskHandle_t imu_handler = NULL;
void imu_system_adaption(void)
{
    BaseType_t ret = xTaskCreate(imu_handler_thread,
                                 "imu_handler",
                                 128*12,
                                 &imu_api,
                                 25,
                                 &imu_handler);
    if (pdPASS != ret)
    {
        LOG_ERROR("task create error");
        vTaskDelete(imu_handler);
    }
}
