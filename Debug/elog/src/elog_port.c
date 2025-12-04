/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2015, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for each platform.
 * Created on: 2015-04-28
 */
 
#include <elog.h>
#include <stdio.h>

//添加FreeRTOS支持
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "portmacro.h"
#include "task.h"
//添加RTT输出
#include "SEGGER_RTT.h"

/**
 * EasyLogger port initialize
 *
 * @return result
 */
ElogErrCode elog_port_init(void) {
    ElogErrCode result = ELOG_NO_ERR;

    /* add your code here */
    
    return result;
}

/**
 * output log port interface
 *
 * @param log output of log
 * @param size log size
 */
void elog_port_output(const char *log, size_t size) {
    
    /* add your code here */
    SEGGER_RTT_Write(0,log,size);
}

/**
 * output lock
 */
void elog_port_output_lock(void) {
    
    /* add your code here */
  // taskENTER_CRITICAL();
  if (!xPortIsInsideInterrupt()) { // 仅任务上下文加锁
        taskENTER_CRITICAL();
  }
}

/**
 * output unlock
 */
void elog_port_output_unlock(void) {
    
    /* add your code here */
  // taskEXIT_CRITICAL();
    if (!xPortIsInsideInterrupt()) { // 仅任务上下文解锁
        taskEXIT_CRITICAL();
    }

}

/**
 * get current time interface
 *
 * @return current time
 */
const char *elog_port_get_time(void) {
  static char cur_system_time[16] = { 0 };

#if (INCLUDE_xTaskGetSchedulerState == 1)
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
#endif
    TickType_t tick = xTaskGetTickCount();
    snprintf(cur_system_time, sizeof(cur_system_time), "%ld.%.3ld",
             tick / configTICK_RATE_HZ, tick % configTICK_RATE_HZ);
    return cur_system_time;
#if (INCLUDE_xTaskGetSchedulerState == 1)
  }
#endif
  return "";
}

/**
 * get current process name interface
 *
 * @return current process name
 */
const char *elog_port_get_p_info(void) {
    
    /* add your code here */
    return "";
}

/**
 * get current thread name interface
 *
 * @return current thread name
 */
const char *elog_port_get_t_info(void) {
    
    /* add your code here */
#if (INCLUDE_xTaskGetSchedulerState == 1)
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        return pcTaskGetName(NULL);
    }
    else return "";
#endif
}

