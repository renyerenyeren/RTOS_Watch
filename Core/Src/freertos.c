/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "../../03-Adapter/03_2-MPU6050/imu_system_adaption.h"
#include "elog.h"
#include "log_task.h"
#include "unpack_task.h"
#include "../../03-Adapter/03_X-Debug/user_debug.h"
#include "DWT_delay.h"
#include "st7789_system_adaption.h"
#include "image_rgb565.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#ifdef  LOG_TAG
#undef  LOG_TAG
#define LOG_TAG       "FreeRTOS"
#else // else of LOG_TAG
#define LOG_TAG       "FreeRTOS"
#endif // end of LOG_TAG

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define FREERTOS_DEBUG
#ifdef  FREERTOS_DEBUG
#define LOG_DEBUG(x,...)  log_d(x, ##__VA_ARGS__)
#define LOG_ERROR(x,...)  log_e(x, ##__VA_ARGS__)
#else
#define LOG_DEBUG(x,...)   ((void)0)
#define LOG_ERROR(x,...)   ((void)0)
#endif

#define ERROR_CHECK(EERO_NUM, NUM_EXPECT_VAL, LOG) do{\
if((EERO_NUM) != (NUM_EXPECT_VAL))                    \
{                                                     \
LOG_ERROR((LOG));                                     \
}}while (0)
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
TaskHandle_t g_log_task_handler;
TaskHandle_t g_unpack_task_handler;


/* USER CODE END Variables */
/* Definitions for main_task */
osThreadId_t main_taskHandle;
const osThreadAttr_t main_task_attributes = {
  .name = "main_task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void MainTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  user_debug_init();
  DWT_Delay_Init();
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of main_task */
  main_taskHandle = osThreadNew(MainTask, NULL, &main_task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  st7789_system_adaption();
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_MainTask */
void temp_humi_callback(float* temp,float* humi)
{
  log_i("get the temp:%d and humi:%d", (uint32_t)(*temp), (uint32_t)(*humi));
}
/**
  * @brief  Function implementing the main_task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_MainTask */
void MainTask(void *argument)
{
  /* USER CODE BEGIN MainTask */
  st7789_driver_instance.pf_init(&st7789_driver_instance);
  /* Infinite loop */
  for(;;)
  {
    // st7789_driver_instance.pf_fill_color(&st7789_driver_instance, 0x07E0);
    // st7789_driver_instance.pf_fill(&st7789_driver_instance, 0, 0, ST7789_WIDTH-1, ST7789_HEIGHT-1, MAGENTA);
    st7789_driver_instance.pf_flush_color_buffer(&st7789_driver_instance, 0, 0, ST7789_WIDTH-1, ST7789_HEIGHT-1, image_rgb565);
  }
  /* USER CODE END MainTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/* USER CODE END Application */

