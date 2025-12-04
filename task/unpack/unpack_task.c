//
// Created by redmiX on 2025/11/28.
//
//******************************** Includes *********************************//
#include "unpack_task.h"

#include "bsp_mpu6050_reg_bit.h"
#include "mpu6050_driver.h"
#include "imu_handler.h"
#include "elog.h"
#include "FreeRTOS.h"
#include "i2c.h"
#include "task.h"
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
#ifdef  LOG_TAG
#undef  LOG_TAG
#define LOG_TAG       "unpack_task"
#else // else of LOG_TAG
#define LOG_TAG       "unpack_task"
#endif // end of LOG_TAG
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Macros ***********************************//

#define UNPACK_DEBUG
#ifdef  UNPACK_DEBUG
#define LOG_DEBUG(x,...)  log_d(x, ##__VA_ARGS__)
#define LOG_ERROR(x,...)  log_e(x, ##__VA_ARGS__)
#define LOG_INFO(x,...)   log_i(x, ##__VA_ARGS__)
#else
#define LOG_DEBUG(x,...)   ((void)0)
#define LOG_ERROR(x,...)   ((void)0)
#define LOG_INFO(x,...)    ((void)0)
#endif

#define NULL_CHECK(x, tag)                              do{\
if(NULL == x)                                              \
{                                                          \
LOG_ERROR(#x" is null ptr");                               \
goto tag;}                                                 \
}while (0)

#define ERROR_CHECK(EERO_NUM, NUM_EXPECT_VAL, LOG)      do{\
if((EERO_NUM) != (NUM_EXPECT_VAL))                         \
{                                                          \
LOG_ERROR((LOG));                                          \
}}while (0)
//******************************** Macros ***********************************//
//---------------------------------------------------------------------------//
//******************************** Variables ********************************//

mpu6050_status_t i2c_init_test(void* pIIC)
{
    return MPU6050_OK;
}
mpu6050_status_t i2c_deinit_test(void* pIIC)
{
    __HAL_RCC_I2C2_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7);
    return MPU6050_OK;
}
mpu6050_status_t i2c_mem_write_test(void *hi2c,
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
mpu6050_status_t i2c_mem_read_test(void* hi2c,
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
mpu6050_status_t i2c_mem_read_dma_test(void* hi2c,
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

static mpu_i2c_driver_interface_t i2c_driver_interface = {
    .hi2c = &hi2c1,
    .pf_i2c_init = i2c_init_test,
    .pf_i2c_deinit = i2c_deinit_test,
    .pf_i2c_mem_write = i2c_mem_write_test,
    .pf_i2c_mem_read = i2c_mem_read_test,
    .pf_i2c_mem_read_dma = i2c_mem_read_dma_test
};

bsp_mpu6050_driver_t mpu6050 = {
    // .p_buffer_interface = &buffer_interface,
    // .p_delay_interface = &delay_interface,
    .p_i2c_driver_interface = &i2c_driver_interface,
    // .p_interrupt_interface = &interrupt_interface,
    // .p_os_interface = &os_interface,
    // .p_timebase_interface = &timebase_interface,
    // .p_yield_interface = &yield_interface
};
//******************************** Variables ********************************//
//---------------------------------------------------------------------------//
//******************************** Functions ********************************//

void unpack_task(void* argument)
{
    LOG_INFO("unpack_task start");
    mpu6050_status_t ret = MPU6050_OK;
    mpu6050_data_t mpu6050_data = {0};

    for (;;)
    {
        ret = imu_unpack_data(&mpu6050_data);
        ERROR_CHECK(ret, MPU6050_OK, "unpack get data error");
        if (MPU6050_OK == ret)
        {
            LOG_INFO("UnpackThread temp=%d", (uint32_t)mpu6050_data.temperature);
            LOG_INFO("UnpackThread ax=%d", (uint32_t)mpu6050_data.ax);
            LOG_INFO("UnpackThread ay=%d", (uint32_t)mpu6050_data.ay);
            LOG_INFO("UnpackThread az=%d", (uint32_t)mpu6050_data.az);
            LOG_INFO("UnpackThread gx=%d", (uint32_t)mpu6050_data.gx);
            LOG_INFO("UnpackThread gy=%d", (uint32_t)mpu6050_data.gy);
            LOG_INFO("UnpackThread gz=%d", (uint32_t)mpu6050_data.gz);


            // mpu_driver_set_interrupt_enable(&mpu6050, DATA_RDY_EN_BIT(1));
        }
    }
}