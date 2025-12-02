//
// Created by redmiX on 2025/11/16.
//
//******************************** Includes *********************************//
#include "aht21_system_adaption.h"
#include "i2c_bus.h"
#include "aht21_driver.h"
#include "temp_humi_handler.h"
#include "elog.h"
#include "FreeRTOS.h"
#include "portmacro.h"
#include "queue.h"
#include "task.h"


//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
#define NULL_CHECK(x, title)                   do{\
if(NULL == x)                                     \
{                                                 \
log_e("PRT IS NULL");                             \
goto title;                                       \
}}while (0)

#define ERROR_CHECK(EERO_NUM,EXPECTED_VAL,LOG) do{\
if ((EERO_NUM) != (EXPECTED_VAL))                 \
{                                                 \
log_e((LOG));                                     \
}                                                 \
}while (0)

#ifdef  LOG_TAG
#undef  LOG_TAG
#define LOG_TAG       "temp_humi_adaption"
#else // else of LOG_TAG
#define LOG_TAG       "temp_humi_adaption"
#endif // end of LOG_TAG
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** 函数声明   *********************************//
// i2c函数实例
static aht21_status_t i2c_init_myown(void* bus);
static aht21_status_t i2c_deinit_myown(void* bus);
static aht21_status_t i2c_start_myown(void* bus);
static aht21_status_t i2c_stop_myown(void* bus);
static aht21_status_t i2c_wait_ack_myown(void* bus);
static aht21_status_t i2c_send_ack_myown(void* bus);
static aht21_status_t i2c_send_no_ack_myown(void* bus);
static aht21_status_t i2c_send_byte_myown(void* bus,
                                         const uint8_t send_data);
static aht21_status_t i2c_receive_byte_myown(void* bus,
                                         uint8_t* const rx_data);
static aht21_status_t critical_enter_myown(void);
static aht21_status_t critical_exit_myown(void);
// 获取时基函数实例
static uint32_t get_tick_count_myown(void);
// os延时函数实例
static void os_Delay_myown(const uint32_t time_ms);

static temp_humi_status_t os_delay_ms_myown(uint32_t ms);
static temp_humi_status_t os_queue_creat_myown(uint32_t num,
                                               uint32_t size,
                                               void** p_queue_handler);
static temp_humi_status_t os_queue_put_myown(void* queue_handler,
                                             void* item,
                                             uint32_t timeout);
static temp_humi_status_t os_queue_get_myown(void* queue_handler,
                                             void* item,
                                             uint32_t timeout);
//******************************** 函数声明   *********************************//
//---------------------------------------------------------------------------//
//******************************** Variables ********************************//

// 1. 初始化i2c总线结构体
const i2c_bus_t aht21_i2c_bus = {
    .I2C_SCL_PORT = GPIOB,
    .I2C_SCL_PIN  = GPIO_PIN_8,
    .I2C_SDA_PORT = GPIOB,
    .I2C_SDA_PIN  = GPIO_PIN_9
};
// 2. 初始化i2c驱动接口结构体
aht_i2c_driver_interface_t i2c_driver_interface = {
    .pf_i2c_init         = i2c_init_myown,
    .pf_i2c_deinit       = i2c_deinit_myown,
    .pf_i2c_start        = i2c_start_myown,
    .pf_i2c_stop         = i2c_stop_myown,
    .pf_i2c_wait_ack     = i2c_wait_ack_myown,
    .pf_i2c_send_ack     = i2c_send_ack_myown,
    .pf_i2c_send_nack    = i2c_send_no_ack_myown,
    .pf_i2c_send_byte    = i2c_send_byte_myown,
    .pf_i2c_receive_byte = i2c_receive_byte_myown,
    .pf_critical_enter   = critical_enter_myown,
    .pf_critical_exit    = critical_exit_myown
};
// 3. 初始化获取时基接口结构体
timebase_interface_t timebase_interface = {
    .pf_get_tick_count = get_tick_count_myown
};
// 4. 初始化OS延时函数接口结构体
yield_interface_t yeiled_interface = {
    .pf_rtos_yield = os_Delay_myown
};
// 5， 初始化os操作接口结构体
temp_humi_handler_os_api_t os_api= {
    .os_delay = os_delay_ms_myown,
    .os_queue_creat = os_queue_creat_myown,
    .os_queue_put = os_queue_put_myown,
    .os_queue_get = os_queue_get_myown
};
// 6. 初始化
temp_humi_handler_input_api_t input_api = {
    .i2c_driver_interface = &i2c_driver_interface,
    .timebase_interface = &timebase_interface,
    .yield_interface = &yeiled_interface,
    .os_interface = &os_api
};

//******************************** Variables ********************************//
//---------------------------------------------------------------------------//
//******************************** Functions ********************************//
static aht21_status_t i2c_init_myown(void* bus)
{
    I2CInit(&aht21_i2c_bus);
    return AHT21_OK;
}
static aht21_status_t i2c_deinit_myown(void* bus)
{
    return AHT21_OK;
}
static aht21_status_t i2c_start_myown(void* bus)
{
    I2CStart(&aht21_i2c_bus);
    return AHT21_OK;
}
static aht21_status_t i2c_stop_myown(void* bus)
{
    I2CStop(&aht21_i2c_bus);
    return AHT21_OK;
}
static aht21_status_t i2c_wait_ack_myown(void* bus)
{
    ErrorStatus ret = I2CWaitAck(&aht21_i2c_bus);
    ERROR_CHECK(ret, SUCCESS, "i2c wait ack is timeout");
    if (1 == ret) return AHT21_ERRORTIMEOUT;
    return AHT21_OK;
}
static aht21_status_t i2c_send_ack_myown(void* bus)
{
    I2CSendAck(&aht21_i2c_bus);
    return AHT21_OK;
}
static aht21_status_t i2c_send_no_ack_myown(void* bus)
{
    I2CSendNotAck(&aht21_i2c_bus);
    return AHT21_OK;
}
static aht21_status_t i2c_send_byte_myown(void* bus,
                                         const uint8_t send_data)
{
    I2CSendByte(&aht21_i2c_bus, send_data);
    return AHT21_OK;
}
static aht21_status_t i2c_receive_byte_myown(void* bus,
                                         uint8_t* const rx_data)
{
    *rx_data = I2CReceiveByte(&aht21_i2c_bus);
    return AHT21_OK;
}
static aht21_status_t critical_enter_myown(void)
{
    portENTER_CRITICAL();
    return AHT21_OK;
}
static aht21_status_t critical_exit_myown(void)
{
    portEXIT_CRITICAL();
    return AHT21_OK;
}
static uint32_t get_tick_count_myown(void)
{
    return HAL_GetTick();
}
static void os_Delay_myown(const uint32_t time_ms)
{
    vTaskDelay(pdMS_TO_TICKS(time_ms));
}
static temp_humi_status_t os_delay_ms_myown(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
    return TEMP_HUMI_OK;
}
static temp_humi_status_t os_queue_creat_myown(uint32_t num,
                                               uint32_t size,
                                               void** p_queue_handler)
{
    NULL_CHECK(p_queue_handler, os_queue_creat_error);
    QueueHandle_t queue_handle = xQueueCreate(num, size);
    NULL_CHECK(queue_handle, os_queue_creat_error);
    *(QueueHandle_t *)p_queue_handler = queue_handle;
    return TEMP_HUMI_OK;

    os_queue_creat_error:
    {
        log_e("queue or p_queue_handler is NULL");
        return TEMP_HUMI_ERRORRESOURCE;
    }
}
static temp_humi_status_t os_queue_put_myown(void* queue_handler,
                                             void* item,
                                             uint32_t timeout)
{
    NULL_CHECK(queue_handler, os_queue_put_error);
    // xQueueSend(queue_handler, item, timeout);
    BaseType_t ret = xQueueSend(queue_handler, item, timeout);
    ERROR_CHECK(ret, pdPASS, "xqueue is fuck");
    return TEMP_HUMI_OK;

    os_queue_put_error:
    {
        log_e("queue_handler is NULL");
        return TEMP_HUMI_ERRORRESOURCE;
    }
}
static temp_humi_status_t os_queue_get_myown(void* queue_handler,
                                             void* item,
                                             uint32_t timeout)
{
    NULL_CHECK(queue_handler, os_queue_get_error);
    BaseType_t ret = xQueueReceive(queue_handler, item, timeout);
    ERROR_CHECK(ret, pdPASS, "queue get ng");
    return TEMP_HUMI_OK;

    os_queue_get_error:
    {
        log_e("queue_handler is NULL");
        return TEMP_HUMI_ERRORRESOURCE;
    }
}
TaskHandle_t temp_humi_handler = NULL;
void temp_humi_system_adaption(void)
{
    BaseType_t ret = xTaskCreate(temp_humi_handler_thread,
                                 "temp_humi_handler",
                                 128*6,
                                 &input_api,
                                 3,
                                 &temp_humi_handler);
    if (pdPASS != ret)
    {
        log_e("task create is ng");
        vTaskDelete(temp_humi_handler);
    }
}