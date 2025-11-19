//
// Created by redmiX on 2025/11/14.

//******************************** Includes *********************************//
#include <stdint.h>

#include "temp_humi_handler.h"
#include "elog.h"
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
#define ERROR_CHECK(EERO_NUM, NUM_EXPECT_VAL, LOG)  do{\
if((EERO_NUM) != (NUM_EXPECT_VAL))                     \
{                                                      \
log_e((LOG));                                          \
}}while (0)

#ifdef  LOG_TAG
#undef  LOG_TAG
#define LOG_TAG       "temp_humi_handler"
#else // else of LOG_TAG
#define LOG_TAG       "temp_humi_handler"
#endif // end of LOG_TAG

#define TEMP_HUMI_IS_INST   1
#define TEMP_HUMI_NOT_INST  0
#define MY_MAX_DELAY        0xffffffffUL
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Variables ********************************//

static bsp_temp_humi_handler_t *gp_temp_humi_instance = NULL;
static temp_humi_handler_private_data_t temp_humi_handler_private_data = {
    .init_flag = TEMP_HUMI_NOT_INST
};

//******************************** Variables ********************************//
//---------------------------------------------------------------------------//
//******************************** Functions ********************************//

/**
 * 将handler实例挂载为全局变量，供其他函数使用
 * @param[in] p_handler
 */
void mount_handler(bsp_temp_humi_handler_t* p_handler)
{
    gp_temp_humi_instance = p_handler;
}

/**
 * 该函数用于初始化温湿度传感器处理器句柄，包括创建事件队列和初始化AHT21传感器。
 * 函数会检查输入参数的有效性，然后依次初始化操作系统队列和AHT21硬件接口。
 * @param[in,out] p_handler
 */
static temp_humi_status_t bsp_temp_humi_handler_init(
                                        bsp_temp_humi_handler_t* const p_handler)
{
    temp_humi_status_t ret = TEMP_HUMI_OK;
    log_d("bsp_temp_humi_handler init is start");
    if (NULL == p_handler ||
        NULL == p_handler->os_interface->os_queue_creat)
    {
        return TEMP_HUMI_ERRORRESOURCE;
    }
    ret = p_handler->os_interface->os_queue_creat(10,
                            sizeof(temp_humi_event_t),
                            &(p_handler->event_queue_handler));
    if (ret) {log_e("queue is ng");return ret;}
    ret = (temp_humi_status_t)aht21_inst(p_handler->p_aht21_instance,
                                         p_handler->i2c_driver_interface,
                                         p_handler->timebase_interface,
                                         p_handler->yield_interface);
    if (ret) {log_e("aht21_init is ng");return TEMP_HUMI_ERRORRESOURCE;}

    return TEMP_HUMI_OK;
}

/**
 * 该函数用于初始化温湿度传感器处理器实例，包括参数检查、接口赋值、
 * 内部初始化和设置初始化完成标志等步骤。
 * @param[in,out]  p_handler 指向温湿度处理器句柄的指针
 * @param[in]  p_input_api 指向输入API接口的指针
 */
temp_humi_status_t bsp_temp_humi_handler_inst(
                            bsp_temp_humi_handler_t*             p_handler,
                      const temp_humi_handler_input_api_t* const p_input_api)
{
    log_d("temp humi instance is start");

    // 1.检查输入参数的有效性
    if (NULL == p_handler)
    {
        log_e("handler is null");
        return TEMP_HUMI_ERRORRESOURCE;
    }
    if (NULL == p_input_api)
    {
        log_e("p_input_api is null");
        return TEMP_HUMI_ERRORRESOURCE;
    }
    if (NULL == p_input_api->i2c_driver_interface)
    {
        log_e("iic is null");
        return TEMP_HUMI_ERRORRESOURCE;
    }
    if (NULL == p_input_api->timebase_interface)
    {
        log_e("timebase is null");
        return TEMP_HUMI_ERRORRESOURCE;
    }
    if (NULL == p_input_api->yield_interface)
    {
        log_e("yield is null");
        return TEMP_HUMI_ERRORRESOURCE;
    }
    if (NULL == p_input_api->os_interface)
    {
        log_e("os is null");
        return TEMP_HUMI_ERRORRESOURCE;
    }

    // 2. 将外部接口赋值给处理器结构体
    p_handler->i2c_driver_interface = p_input_api->i2c_driver_interface;
    p_handler->timebase_interface = p_input_api->timebase_interface;
    p_handler->yield_interface = p_input_api->yield_interface;
    p_handler->os_interface = p_input_api->os_interface;

    // 3. 调用内部初始化函数
    temp_humi_status_t ret = bsp_temp_humi_handler_init(p_handler);
    if (TEMP_HUMI_OK != ret)
    {
        log_e("TEMP_HUMI init is ng");
        return ret;
    }

    // 4. 设置初始化完成标志
    p_handler->p_private_data->init_flag = TEMP_HUMI_IS_INST;
    log_d("bsp_temp_humi_handler_inst end");

    return TEMP_HUMI_OK;
}

/**
 * 该函数用于从温湿度传感器中读取数据，并返回给调用者。
 * 函数会检查输入参数的有效性，然后从队列中获取数据并返回给调用者。
 * @param[in] event
 */
temp_humi_status_t bsp_temp_humi_read(temp_humi_event_t* event)
{
    log_d("bsp_temp_humi_read start");
    if (NULL == event                 ||
        NULL == gp_temp_humi_instance ||
        TEMP_HUMI_NOT_INST ==
        gp_temp_humi_instance->p_private_data->init_flag)
    {
        log_e("temp_humi_handler is not inst");
        return TEMP_HUMI_ERRORRESOURCE;
    }
    temp_humi_status_t ret =
        gp_temp_humi_instance->os_interface->os_queue_put(
            gp_temp_humi_instance->event_queue_handler,
            event, MY_MAX_DELAY);
    if (ret)
    {
        log_e("os_queue_put event failed");
        return TEMP_HUMI_ERRORRESOURCE;
    }
    return TEMP_HUMI_OK;
}

/**
 * 获取温度和湿度值。
 * @param[in] handler_instance 指向处理程序实例的指针。
 * @param[in] event 事件数据，指定要获取的数据类型及其生命周期。
 * @param[out] temp 用于存储获取到的温度值的指针。
 * @param[out] humi 用于存储获取到的湿度值的指针。
 * @return
 */
temp_humi_status_t bsp_temp_humi_get_data(
    bsp_temp_humi_handler_t* handler_instance,
    temp_humi_event_t  event,
    float* temp,
    float* humi)
{
    temp_humi_status_t ret = TEMP_HUMI_OK;
    uint32_t tick_time = handler_instance->timebase_interface->pf_get_tick_count();
    switch (event.type)
    {
        case TEMP_HUMI_EVENT_TEMP:
            if (tick_time - handler_instance->last_temp_tick > event.lifetime)
            {
                handler_instance->p_aht21_instance->pf_read_temp_humi(
                     handler_instance->p_aht21_instance,
                     temp,
                     humi);
                handler_instance->last_temp_tick = tick_time;
            }
            break;
        case TEMP_HUMI_EVENT_HUMI:
            if (tick_time - handler_instance->last_humi_tick > event.lifetime)
            {
                handler_instance->p_aht21_instance->pf_read_humi(
                     handler_instance->p_aht21_instance,
                     humi);
                handler_instance->last_humi_tick = tick_time;
            }
            break;
        case TEMP_HUMI_EVENT_BOTH:
            if (tick_time - handler_instance->last_humi_tick > event.lifetime)
            {
                handler_instance->p_aht21_instance->pf_read_humi(
                     handler_instance->p_aht21_instance,
                     humi);
                handler_instance->last_humi_tick = tick_time;
            }
            if (tick_time - handler_instance->last_temp_tick > event.lifetime)
            {
                handler_instance->p_aht21_instance->pf_read_temp_humi(
                     handler_instance->p_aht21_instance,
                     temp,
                     humi);
                handler_instance->last_temp_tick = tick_time;
            }
            break;
        default:
            *temp = 0;
            *humi = 0;
            ret = TEMP_HUMI_ERROR;
    }
    return ret;
}

void temp_humi_handler_thread(void* argument)
{
    log_d("temp humi handler thread start");
    temp_humi_handler_input_api_t* input_api = NULL;
    temp_humi_event_t event;
    temp_humi_status_t ret = TEMP_HUMI_OK;
    float temp = 0;
    float humi = 0;
    if (NULL == argument)
    {
        log_e("argument is NULL");
        return;
    }
    input_api = (temp_humi_handler_input_api_t*)argument;

    // AHT21 实例
    bsp_aht21_driver_t bsp_aht21_driver;
    // 温湿度Handler实例结构体
    bsp_temp_humi_handler_t temp_humi_handler_instance = {0};

    temp_humi_handler_instance.p_aht21_instance = &bsp_aht21_driver;
    temp_humi_handler_instance.p_private_data = &temp_humi_handler_private_data;

    ret = bsp_temp_humi_handler_inst(&temp_humi_handler_instance, input_api);
    if (TEMP_HUMI_OK == ret)
    {
        // 将处理句柄挂载在全局变量上，方便外部调用bsp_temp_humi_read_data
        mount_handler(&temp_humi_handler_instance);
    }

    for (;;)
    {
        ret = temp_humi_handler_instance.os_interface->os_queue_get(
            temp_humi_handler_instance.event_queue_handler,
            &event,MY_MAX_DELAY);
        ERROR_CHECK(ret,TEMP_HUMI_OK,"get queue is fuck");
        if (TEMP_HUMI_OK ==
            bsp_temp_humi_get_data(&temp_humi_handler_instance,
                                   event,
                                   &temp,
                                   &humi))
        {
            //预留，获取到正确的事情之后要做什么
        }
        else
        {
            //预留，获取到错误数据之后要干什么
            log_e("get data is error");
        }

        event.pf_callback(&temp, &humi);
    }
}