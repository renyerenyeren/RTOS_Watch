//
// Created by redmiX on 2025/11/25.
//

#ifndef RTOS_PROJECT_IMU_HANDLER_H
#define RTOS_PROJECT_IMU_HANDLER_H

//******************************** Includes *********************************//
#include <stdint.h>

#include "mpu6050_driver.h"
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Typedefs *********************************//
//******************************** Typedefs *********************************//
//---------------------------------------------------------------------------//
//**************************** Interface Structs ****************************//
typedef struct
{
    mpu_i2c_driver_interface_t* pIIC_driver;
    mpu_delay_interface_t*      pDelay;
    mpu_timebase_interface_t*   pTimebase;
    mpu_yield_interface_t*      pYield;
    os_interface_t*             pOS;
}imu_handler_input_api_t;
//**************************** Interface Structs ****************************//
//---------------------------------------------------------------------------//
//******************************** Classes **********************************//
typedef struct
{
    /** 传递给驱动层的接口 */
    mpu_i2c_driver_interface_t* pIIC_driver;
    mpu_delay_interface_t*      pDelay;
    mpu_timebase_interface_t*   pTimebase;
    mpu_yield_interface_t*      pYield;
    os_interface_t*             pOS;

    /** 底层驱动的具体实例对象 */
    bsp_mpu6050_driver_t*       pDriver;

    void*                       Queue_handle;            /** 队列句柄      */
    void*                       pUnpack_queue_handle;    /** 解包队列句柄   */
    uint32_t                    Queue_length;            /** 队列长度       */
    uint32_t                    Queue_item_size;         /** 队列项大小     */

    void*                       semaphore_binary_handle; /** 二进制信号量句柄 */
    void*                       notify_handle;           /** 通知机制句柄    */
}bsp_imu_handler_t;
//******************************** Classes **********************************//
//---------------------------------------------------------------------------//
//******************************** 函数声明 ***********************************//
mpu6050_status_t imu_handler_inst(bsp_imu_handler_t* pHandler,
                            imu_handler_input_api_t* pInput_api);
void imu_handler_thread(void* argument);
mpu6050_status_t imu_unpack_data(mpu6050_data_t* mpu6050_data);
//******************************** 函数声明 ***********************************//

#endif //RTOS_PROJECT_IMU_HANDLER_H