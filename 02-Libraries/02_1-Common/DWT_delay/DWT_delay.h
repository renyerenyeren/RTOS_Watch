//
// Created by redmiX on 2025/12/2.
//

#ifndef RTOS_PROJECT_DWT_DELAY_H
#define RTOS_PROJECT_DWT_DELAY_H

#include <stdint.h>

#define CPU_FREQ_MHZ     168UL   //CPU主频168MHz
#define CPU_FREQ_HZ      (CPU_FREQ_MHZ * 1000000UL)

void DWT_Delay_Init(void) ;

void DWT_Delay_us(uint32_t us);

void DWT_Delay_ms(uint32_t ms);

#endif //RTOS_PROJECT_DWT_DELAY_H