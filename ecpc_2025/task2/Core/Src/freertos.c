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
#include "cmsis_os.h"
#include "main.h"
#include "task.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
    .name = "defaultTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for ClignoteTask */
osThreadId_t ClignoteTaskHandle;
const osThreadAttr_t ClignoteTask_attributes = {
    .name = "ClignoteTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityLow1,
};
/* Definitions for ButtonTask */
osThreadId_t ButtonTaskHandle;
const osThreadAttr_t ButtonTask_attributes = {
    .name = "ButtonTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityLow4,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void* argument);
void ClignoteTask_Cbk(void* argument);
void ButtonTask_Cbk(void* argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void)
{
    /* USER CODE BEGIN Init */

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
    /* creation of defaultTask */
    defaultTaskHandle =
        osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

    /* creation of ClignoteTask */
    ClignoteTaskHandle =
        osThreadNew(ClignoteTask_Cbk, NULL, &ClignoteTask_attributes);

    /* creation of ButtonTask */
    ButtonTaskHandle =
        osThreadNew(ButtonTask_Cbk, NULL, &ButtonTask_attributes);

    /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
    /* USER CODE END RTOS_THREADS */

    /* USER CODE BEGIN RTOS_EVENTS */
    /* add events, ... */
    /* USER CODE END RTOS_EVENTS */
}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void* argument)
{
    /* USER CODE BEGIN StartDefaultTask */
    /* Infinite loop */
    for (;;)
    {
        osDelay(1);
    }
    /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_ClignoteTask_Cbk */
/**
 * @brief Function implementing the ClignoteTask thread.
 * @param argument: Not used
 * @retval None
 */
volatile int btn_taskPeriod = 0 ; 
volatile uint8_t LedTaskState = 0 ; 
static TickType_t xLastWakeTime = 0;  // Make it static or global (NOT local)
volatile int i = 0 ; 
volatile sum = 0 ; 
volatile  TickType_t xStartTime = 0 ; 
/* USER CODE END Header_ClignoteTask_Cbk */
void ClignoteTask_Cbk(void* argument)
{
    /* USER CODE BEGIN ClignoteTask_Cbk */
    /* Infinite loop */


    for (;;)
    {

      
        if (LedTaskState == 1 ){ 
            GPIO_PinState tmp = HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_15); 
            HAL_GPIO_WritePin(GPIOD,GPIO_PIN_15,tmp); 
            LedTaskState = 0 ; 

        }else { 
            HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);

        }

        xStartTime = xTaskGetTickCount();
        while (xTaskGetTickCount() - xStartTime <= (1000))
        {
        xLastWakeTime = xTaskGetTickCount();
        }

        xStartTime = 0 ; 
    }
    /* USER CODE END ClignoteTask_Cbk */
}

/* USER CODE BEGIN Header_ButtonTask_Cbk */
uint8_t BtnState = 0;
/**
 * @brief Function implementing the ButtonTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_ButtonTask_Cbk */
void ButtonTask_Cbk(void* argument)
{
    /* USER CODE BEGIN ButtonTask_Cbk */
    /* Infinite loop */

    for (;;)
    {

        int start = 0 ;
        BtnState = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
        if (BtnState == 1)
        {
            osDelay(20);
            BtnState = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
            if (BtnState == 1)
            {
                osThreadSuspend(ClignoteTaskHandle);
                HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
                LedTaskState = 1 ; 
                start =  xTaskGetTickCount(); 
            }
            else
            {
                // HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
                // osThreadResume(ClignoteTaskHandle);
                // do nohing
            }
        }
        else
        {
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
            osThreadResume(ClignoteTaskHandle);
        }

        osDelay(80);
        int last = xTaskGetTickCount(); 
        btn_taskPeriod += (last - start); 
    }
    /* USER CODE END ButtonTask_Cbk */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
