//
// Created by redmiX on 2025/11/28.
//
//******************************** Includes *********************************//
#include "unpack_task.h"
#include "imu_handler.h"
#include "elog.h"
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

#define ERROR_CHECK(EERO_NUM, NUM_EXPECT_VAL, LOG, tag) do{\
if((EERO_NUM) != (NUM_EXPECT_VAL))                         \
{                                                          \
LOG_ERROR((LOG));                                          \
goto tag;                                                  \
}}while (0)
//******************************** Macros ***********************************//
//---------------------------------------------------------------------------//
//******************************** Variables ********************************//
//******************************** Variables ********************************//
//---------------------------------------------------------------------------//
//******************************** Functions ********************************//

void unpack_task(void* argument)
{
    LOG_INFO("unpack_task start");
    mpu6050_status_t ret = MPU6050_OK;
    uint8_t data = 0;


}