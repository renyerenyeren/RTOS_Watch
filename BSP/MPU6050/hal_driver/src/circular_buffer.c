//
// Created by redmiX on 2025/11/25.
//
//******************************** Includes *********************************//
#include <stdlib.h>

#include "circular_buffer.h"
#include "elog.h"
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
#ifdef  LOG_TAG
#undef  LOG_TAG
#define LOG_TAG       "circular_buffer"
#else // else of LOG_TAG
#define LOG_TAG       "circular_buffer"
#endif // end of LOG_TAG
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Macros ***********************************//
#define BUFFER_DEBUG
#ifdef  BUFFER_DEBUG
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
circular_buffer_t mpu_circular_buffer;
//******************************** Variables ********************************//
//---------------------------------------------------------------------------//
//******************************** Functions ********************************//
/**
 * @brief 获取循环缓冲区中当前写入位置的地址
 * @param buffer 指向循环缓冲区结构体的指针
 * @return 返回写入位置的地址，如果参数为空则返回NULL
 */
uint8_t *get_wbuffer_addr(circular_buffer_t *buffer)
{
    NULL_CHECK(buffer, null_ptr);
    return buffer->buffer + buffer->wflag * MPU6050_DATA_PACKET_SIZE;

null_ptr:
    {
        LOG_ERROR("fuck you, get ptr addr!");
        return NULL;
    }
}

/**
 * @brief 获取循环缓冲区中当前读取位置的地址
 * @param buffer 指向循环缓冲区结构体的指针
 * @return 返回读取位置的地址，如果参数为空则返回NULL
 */
uint8_t *get_rbuffer_addr(circular_buffer_t *buffer)
{
    NULL_CHECK(buffer, null_ptr);
    return buffer->buffer + buffer->rflag * MPU6050_DATA_PACKET_SIZE;

null_ptr:
    {
        LOG_ERROR("fuck you, get ptr addr!");
        return NULL;
    }
}

/**
 * @brief 更新写入标志位，表示一个数据包已写入完成
 * @param buffer 指向循环缓冲区结构体的指针
 */
void data_writed(circular_buffer_t *buffer)
{
    NULL_CHECK(buffer, null_ptr);
    //* DMA写数据结束
    // todo:buffer已满
    buffer->wflag = (buffer->wflag + 1) % buffer->size;

null_ptr:
    {
        LOG_ERROR("fuck you, get ptr addr!");
    }
}

/**
 * @brief 更新读取标志位，表示一个数据包已读取完成
 * @param buffer 指向循环缓冲区结构体的指针
 */
void data_readed(circular_buffer_t *buffer)
{
    NULL_CHECK(buffer, null_ptr);
    //* DMA写数据结束
    // todo:buffer已满
    buffer->rflag = (buffer->rflag + 1) % buffer->size;

null_ptr:
    {
        LOG_ERROR("fuck you, get ptr addr!");
    }
}

/**
 * @brief 初始化循环缓冲区
 * @param buffer 指向循环缓冲区结构体的指针
 * @param size 缓冲区槽位数量
 */
void buffer_init(circular_buffer_t *buffer, uint8_t size)
{
    NULL_CHECK(buffer, null_ptr);

    buffer->size = size;  // 槽位数量
    buffer->rflag = 0;
    buffer->wflag = 0;
    /*buffer 分配空间: 槽位数量 × 每个槽位的数据包大小*/
    buffer->buffer = (uint8_t *)malloc(size * MPU6050_DATA_PACKET_SIZE);
    //* 绑定实例函数
    buffer->pf_get_rbuffer_addr = get_rbuffer_addr;
    buffer->pf_get_wbuffer_addr = get_wbuffer_addr;
    buffer->pf_data_readed = data_readed;
    buffer->pf_data_writed = data_writed;

null_ptr:
     {
         LOG_ERROR("fuck you, get ptr addr!");
     }
}
