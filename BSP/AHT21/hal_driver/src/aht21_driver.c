//
// Created by redmiX on 2025/11/13.
//******************************** Includes *********************************//
#include "aht21_driver.h"
#include "aht21_reg.h"
#include "elog.h"
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
#define ERROR_CHECK(EERO_NUM, NUM_EXPECT_VAL, LOG)  do{\
if((EERO_NUM) != (NUM_EXPECT_VAL))                     \
{                                                      \
    log_e((LOG));                                      \
}}while (0)

#ifdef  LOG_TAG
#undef  LOG_TAG
#define LOG_TAG       "aht21_driver"
#else // else of LOG_TAG
#define LOG_TAG       "aht21_driver"
#endif // end of LOG_TAG

#define IS_INITED                  (1 == inited)
#define BIT(x)                     (0x01 << x  )

#define AHT21_MEASURE_WAITING_TIME (80         )
#define AHT21_ID                   (0X18       )
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Variables *********************************//

static uint8_t inited;

//******************************** Variables ********************************//
//---------------------------------------------------------------------------//
//******************************** 函数声明   *********************************//
static aht21_status_t read_ID(bsp_aht21_driver_t * const p_aht21,
                             uint8_t * const state_num);

/**
 * 用于读取AHT21的运行状态
 * @param[in] p_aht21  指向AHT21对象的结构体指针
 * @return  表示状态的一字节数据
 */
static uint8_t aht21_read_status(bsp_aht21_driver_t * const p_aht21)
{
    if (!IS_INITED) return AHT21_ERRORRESOURCE;
    uint8_t rx_data = 0;
    /**********************************临界区*************************************/
    p_aht21->p_i2c_driver_interface->pf_critical_enter();
    {
        /** 发送起始信号,发送从机读取地址*/
        p_aht21->p_i2c_driver_interface->pf_i2c_start    (NULL);
        p_aht21->p_i2c_driver_interface->pf_i2c_send_byte(NULL,
                                                          AHT21_REG_READ_ADDR);
        uint8_t ret = p_aht21->p_i2c_driver_interface->pf_i2c_wait_ack(NULL);
        /** 超时检测*/
        ERROR_CHECK(ret, AHT21_OK, "aht21 time out");
        /** 读取从机一字节数据*/
        p_aht21->p_i2c_driver_interface->pf_i2c_receive_byte(NULL,&rx_data);
        p_aht21->p_i2c_driver_interface->pf_i2c_send_nack   (NULL);
        p_aht21->p_i2c_driver_interface->pf_i2c_stop        (NULL);
    }
    p_aht21->p_i2c_driver_interface->pf_critical_exit();
    /********************************临界区结束************************************/
    log_d("rx_data = %#x",rx_data);

    return rx_data;
}

/**
 * 初始化AHT21对象
 * @param[in] p_aht21  指向AHT21对象结构体的指针
 * @return  返回初始化结果
 */
static aht21_status_t aht21_init(bsp_aht21_driver_t * const p_aht21)
{
    log_d("aht21_init start");
    if (IS_INITED) return AHT21_ERRORRESOURCE;
    inited = 1; // 标记初始化成功
    p_aht21->p_yield_interface->pf_rtos_yield(300);

    if (NULL == p_aht21                         ||
        NULL == p_aht21->p_i2c_driver_interface ||
        NULL == p_aht21->p_i2c_driver_interface->pf_i2c_init)
    {
        log_e("p_iic_inst is NULL");
        return AHT21_ERRORPARAMETER;
    }

    p_aht21->p_i2c_driver_interface->pf_i2c_init(NULL);
    log_d("aht21_init iic_driver_init------");
    uint8_t state_num = 0;
    read_ID(p_aht21, &state_num); // 读取状态ID

    log_d("aht21_is_init,state is [%#X]",state_num);
    return AHT21_OK;
}

/**
 * 用于AHT21的反初始化
 * @param[in] p_aht21  指向AHT21对象结构体的指针
 * @return  返回释放结果
 */
static aht21_status_t aht21_deinit(bsp_aht21_driver_t * const p_aht21)
{
    if (NULL == p_aht21)
    {
        log_e("p_iic_inst is NULL");
        return AHT21_ERRORPARAMETER;
    }
    inited = 0;
    return AHT21_OK;
}
/**
 * 用于读取AHT21的状态
 * @param[in]  p_aht21   指向AHT21对象结构体的指针
 * @param[out] state_num 读取状态
 * @return  返回初始化结果
 */
static aht21_status_t read_ID(bsp_aht21_driver_t * const p_aht21  ,
                             uint8_t             * const state_num)
{
    /*****************************临界区*******************************************/
    p_aht21->p_i2c_driver_interface->pf_critical_enter();
    {
        p_aht21->p_i2c_driver_interface->pf_i2c_start    (NULL);
        p_aht21->p_i2c_driver_interface->pf_i2c_send_byte(NULL, AHT21_REG_READ_ADDR);
        aht21_status_t ret = p_aht21->p_i2c_driver_interface->pf_i2c_wait_ack(NULL);
        ERROR_CHECK(ret, AHT21_OK, "aht21 init is fuck");
        p_aht21->p_i2c_driver_interface->pf_i2c_receive_byte(NULL, state_num);
        p_aht21->p_i2c_driver_interface->pf_i2c_send_nack   (NULL);
        p_aht21->p_i2c_driver_interface->pf_i2c_stop        (NULL);
    }
    p_aht21->p_i2c_driver_interface->pf_critical_exit();
    /********************************临界区结束*************************************/
    return AHT21_OK;
}

/**
 * 用于AHT21的测温和湿度数据读取
 * @param[in]  p_aht21   指向AHT21对象结构体的指针
 * @param[out] temp      温度数据
 * @param[out] humi      湿度数据
 * @return  返回初始化结果
 */
static aht21_status_t aht21_read_temp_humi(bsp_aht21_driver_t * const p_aht21,
                                           float              * const temp   ,
                                           float              * const humi   )
{
    if (!IS_INITED) return AHT21_ERRORRESOURCE;
    uint8_t cnt = 5;
    uint8_t byte[6] = {0};
    uint32_t retu_data = 0;
    // 1.发送测温命令
    /*****************************临界区*******************************************/
    p_aht21->p_i2c_driver_interface->pf_critical_enter();
    {
        p_aht21->p_i2c_driver_interface->pf_i2c_start    (NULL);
        p_aht21->p_i2c_driver_interface->pf_i2c_send_byte(NULL,
                                                          AHT21_REG_WRITE_ADDR);
        p_aht21->p_i2c_driver_interface->pf_i2c_wait_ack (NULL);
        p_aht21->p_i2c_driver_interface->pf_i2c_send_byte(NULL,
                                                          AHT21_REG_MEASURE_CMD);
        p_aht21->p_i2c_driver_interface->pf_i2c_wait_ack (NULL);
        p_aht21->p_i2c_driver_interface->pf_i2c_send_byte(NULL,
                                                   AHT21_REG_MEASURE_CMD_ARFS1);
        p_aht21->p_i2c_driver_interface->pf_i2c_wait_ack (NULL);
        p_aht21->p_i2c_driver_interface->pf_i2c_send_byte(NULL,
                                                   AHT21_REG_MEASURE_CMD_ARFS2);
        p_aht21->p_i2c_driver_interface->pf_i2c_wait_ack (NULL);
        p_aht21->p_i2c_driver_interface->pf_i2c_stop     (NULL);
    }
    p_aht21->p_i2c_driver_interface->pf_critical_exit();
    /********************************临界区结束*************************************/
    // 2.等待测温完成
    p_aht21->p_yield_interface->pf_rtos_yield(AHT21_MEASURE_WAITING_TIME);

    while (0x80 == (0x80 & aht21_read_status(p_aht21)) && cnt)
    {
        p_aht21->p_yield_interface->pf_rtos_yield(5);
        cnt--;
        if (0 == cnt) return AHT21_ERRORTIMEOUT;
    }
    log_d("read temp start ......");
    // 3.读取测量结果
    /*****************************临界区*******************************************/
    p_aht21->p_i2c_driver_interface->pf_critical_enter();
    {
        p_aht21->p_i2c_driver_interface->pf_i2c_start    (NULL);
        p_aht21->p_i2c_driver_interface->pf_i2c_send_byte(NULL,
                                                          AHT21_REG_READ_ADDR);
        p_aht21->p_i2c_driver_interface->pf_i2c_wait_ack (NULL);
        for (uint8_t i = 0; i < 5; i++)
        {
            p_aht21->p_i2c_driver_interface->pf_i2c_receive_byte(NULL,
                                                                 &byte[i]);
            p_aht21->p_i2c_driver_interface->pf_i2c_send_ack(NULL);
        }
        p_aht21->p_i2c_driver_interface->pf_i2c_receive_byte(NULL,
                                                             &byte[5]);
        p_aht21->p_i2c_driver_interface->pf_i2c_send_nack   (NULL);
        p_aht21->p_i2c_driver_interface->pf_i2c_stop        (NULL);
    }
    p_aht21->p_i2c_driver_interface->pf_critical_exit();
    /********************************临界区结束*************************************/
    retu_data = (retu_data | byte[1]) <<8;
    retu_data = (retu_data | byte[2]) <<8;
    retu_data = (retu_data | byte[3]) >>4;
    retu_data = retu_data & 0xFFFFF;
    *humi = (float)(retu_data * 1000 >> 20);
    *humi /= 10;

    retu_data = 0;
    retu_data = (retu_data | (byte[3] & 0x0f)) <<8;
    retu_data = (retu_data | byte[4]) <<8;
    retu_data = (retu_data | byte[5]);
    retu_data = retu_data & 0xFFFFF;
    *temp = (float)((retu_data* 2000 >> 20) - 500);
    *temp /= 10;

    return AHT21_OK;
}

/**
 * 用于AHT21的只读湿度信息
 * @param[in]  p_aht21   指向AHT21对象结构体的指针
 * @param[out] humi      湿度数据
 * @return  返回初始化结果
 */
static aht21_status_t aht21_read_humi(bsp_aht21_driver_t * const p_aht21,
                                      float              * const humi   )
{
        if (!IS_INITED) return AHT21_ERRORRESOURCE;
    uint8_t cnt = 5;
    uint8_t byte[4] = {0};
    uint32_t retu_data = 0;
    // 1.发送测温命令
    /*****************************临界区*******************************************/
    p_aht21->p_i2c_driver_interface->pf_critical_enter();
    {
        p_aht21->p_i2c_driver_interface->pf_i2c_start    (NULL);
        p_aht21->p_i2c_driver_interface->pf_i2c_send_byte(NULL,
                                                          AHT21_REG_WRITE_ADDR);
        p_aht21->p_i2c_driver_interface->pf_i2c_wait_ack (NULL);
        p_aht21->p_i2c_driver_interface->pf_i2c_send_byte(NULL,
                                                          AHT21_REG_MEASURE_CMD);
        p_aht21->p_i2c_driver_interface->pf_i2c_wait_ack (NULL);
        p_aht21->p_i2c_driver_interface->pf_i2c_send_byte(NULL,
                                                   AHT21_REG_MEASURE_CMD_ARFS1);
        p_aht21->p_i2c_driver_interface->pf_i2c_wait_ack (NULL);
        p_aht21->p_i2c_driver_interface->pf_i2c_send_byte(NULL,
                                                   AHT21_REG_MEASURE_CMD_ARFS2);
        p_aht21->p_i2c_driver_interface->pf_i2c_wait_ack (NULL);
        p_aht21->p_i2c_driver_interface->pf_i2c_stop     (NULL);
    }
    p_aht21->p_i2c_driver_interface->pf_critical_exit();
    /********************************临界区结束*************************************/
    // 2.等待测温完成
    p_aht21->p_yield_interface->pf_rtos_yield(AHT21_MEASURE_WAITING_TIME);

    while (0x80 == (0x80 & aht21_read_status(p_aht21)) && cnt)
    {
        p_aht21->p_yield_interface->pf_rtos_yield(5);
        cnt--;
        if (0 == cnt) return AHT21_ERRORTIMEOUT;
    }
    log_d("read temp start ......");
    // 3.读取测量结果
    /*****************************临界区*******************************************/
    p_aht21->p_i2c_driver_interface->pf_critical_enter();
    {
        p_aht21->p_i2c_driver_interface->pf_i2c_start    (NULL);
        p_aht21->p_i2c_driver_interface->pf_i2c_send_byte(NULL,
                                                          AHT21_REG_READ_ADDR);
        p_aht21->p_i2c_driver_interface->pf_i2c_wait_ack (NULL);
        for (uint8_t i = 0; i < 3; i++)
        {
            p_aht21->p_i2c_driver_interface->pf_i2c_receive_byte(NULL,
                                                                 &byte[i]);
            p_aht21->p_i2c_driver_interface->pf_i2c_send_ack    (NULL);
        }
        p_aht21->p_i2c_driver_interface->pf_i2c_receive_byte(NULL,
                                                             &byte[3]);
        p_aht21->p_i2c_driver_interface->pf_i2c_send_nack   (NULL);
        p_aht21->p_i2c_driver_interface->pf_i2c_stop        (NULL);
    }
    p_aht21->p_i2c_driver_interface->pf_critical_exit();
    /********************************临界区结束*************************************/
    retu_data = (retu_data | byte[1]) <<8;
    retu_data = (retu_data | byte[2]) <<8;
    retu_data = (retu_data | byte[3]) >>4;
    retu_data = retu_data & 0xFFFFF;
    *humi = (float)(retu_data * 1000 >> 20);
    *humi /= 10;

    return AHT21_OK;
}

/**
 * 用于AHT21的休眠
 * @param[in]  p_aht21   指向AHT21对象结构体的指针
 * @return  返回初始化结果
 */
static aht21_status_t aht21_sleep(bsp_aht21_driver_t * const p_aht21)
{
    if (! IS_INITED)
    {
        return AHT21_ERRORRESOURCE;
    }
    return AHT21_OK;
}

/**
 * 用于AHT21的唤醒
 * @param[in]  p_aht21   指向AHT21对象结构体的指针
 * @return  返回初始化结果
 */
static aht21_status_t aht21_weakup(bsp_aht21_driver_t * const p_aht21)
{
    if (! IS_INITED)
    {
        return AHT21_ERRORRESOURCE;
    }
    return AHT21_OK;
}

/**
 * 用于AHT21的初始化
 * @param[in]  p_aht21   指向AHT21对象结构体的指针
 * @return  返回初始化结果
 */
aht21_status_t aht21_inst(
                    bsp_aht21_driver_t*     const p_bsp_aht21_inst     ,
                    aht_i2c_driver_interface_t* const p_i2c_driver_inst,
                    timebase_interface_t*   const p_timebase_inst  ,
                    yield_interface_t*      const p_yield_inst     )
{
    log_d("aht21_inst start ");
    if (NULL == p_bsp_aht21_inst    ||
        NULL == p_i2c_driver_inst   ||
        NULL == p_yield_inst        ||
        NULL == p_timebase_inst
    ) { return AHT21_ERRORPARAMETER; }
    p_bsp_aht21_inst->p_i2c_driver_interface = p_i2c_driver_inst   ;
    p_bsp_aht21_inst->p_timebase_interface   = p_timebase_inst     ;
    p_bsp_aht21_inst->p_yield_interface      = p_yield_inst        ;

    p_bsp_aht21_inst->pf_inst                = aht21_inst          ;
    p_bsp_aht21_inst->pf_init                = aht21_init          ;
    p_bsp_aht21_inst->pf_deinit              = aht21_deinit        ;
    p_bsp_aht21_inst->pf_read_id             = read_ID             ;
    p_bsp_aht21_inst->pf_read_temp_humi      = aht21_read_temp_humi;
    p_bsp_aht21_inst->pf_read_humi           = aht21_read_humi     ;
    p_bsp_aht21_inst->pf_sleep               = aht21_sleep         ;
    p_bsp_aht21_inst->pf_weakup              = aht21_weakup        ;

    aht21_status_t ret = aht21_init(p_bsp_aht21_inst);
    ERROR_CHECK(ret, AHT21_OK, "aht21_init is unsuccessful");
    if (ret) return AHT21_ERRORRESOURCE;

    log_d("aht21_inst end");
    return AHT21_OK;
}