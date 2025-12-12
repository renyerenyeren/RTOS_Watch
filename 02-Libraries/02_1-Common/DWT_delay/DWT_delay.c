//
// Created by redmiX on 2025/12/2.
//

// 基于STM32F411芯片的DWT（Data Watchpoint and Trace）延时库
// 利用ARM Cortex-M4内核的硬件计数器实现高精度微秒/毫秒级延时
// 核心优势：无中断干扰、延时精度高（依赖CPU主频）、占用资源少
#include "DWT_delay.h"
#include "stm32f407xx.h"  // STM32F411系列芯片寄存器定义头文件


void DWT_Delay_Init(void) {
    // 1. 启用DWT跟踪单元（DEMCR寄存器：Debug Exception and Monitor Control Register）
    // TRCENA位（bit24）：置1时启用内核跟踪功能，DWT等跟踪单元才能工作
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    // 2. 清零CYCCNT计数器（DWT->CYCCNT：32位循环计数器，记录CPU时钟周期数）
    // 初始化后从0开始计数，计数溢出后自动归零（32位计数器，溢出周期=2^32 / CPU主频）
    DWT->CYCCNT = 0x00000000;

    // 3. 启用CYCCNT计数器（DWT->CTRL：DWT控制寄存器）
    // CYCCNTENA位（bit0）：置1时启动CYCCNT计数，CPU每执行1个时钟周期，计数器+1
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}


void DWT_Delay_us(uint32_t us)
{
    uint32_t start_cnt = DWT->CYCCNT;  // 记录延时开始时的计数器值
    // 计算目标计数器值：开始值 + 微秒数 × CPU主频（MHz）
    // 注：CPU_FREQ_MHZ = CPU主频（Hz）/ 1e6，1us内CPU执行的时钟周期数 = 主频（MHz）
    uint32_t target_cnt = start_cnt + (us * CPU_FREQ_MHZ);

    // 处理计数器溢出情况（32位计数器最大值为0xFFFFFFFF，超过后会归零）
    if (target_cnt < start_cnt)
    {
        // 溢出场景：先等待计数器从start_cnt计数到0xFFFFFFFF（溢出），再等待从0计数到target_cnt
        while (DWT->CYCCNT >= start_cnt && DWT->CYCCNT < target_cnt);
    }
    else
    {
        // 无溢出场景：直接等待计数器从start_cnt计数到target_cnt
        while (DWT->CYCCNT < target_cnt);
    }
}


void DWT_Delay_ms(uint32_t ms)
{
    // 转换为微秒延时（1ms = 1000us），使用UL确保无符号长整型运算，避免溢出
    DWT_Delay_us(ms * 1000UL);
}