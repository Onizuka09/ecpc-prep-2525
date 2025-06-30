/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define STRING_MODEx_SIZE 20
const char  STRING_MODE1[STRING_MODEx_SIZE]= "Mode chnage: 1 \n\r";
const char  STRING_MODE2[STRING_MODEx_SIZE]= "Mode chnage: 2 \n\r";
const char  STRING_MODE3[STRING_MODEx_SIZE]= "Mode chnage: 3 \n\r";

uint8_t rx_buff[2];
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define MCU_FREQ (16000000U)
#define BAUDRATE_1 (4800U)
#define BAUDRATE_2 (9600U)
#define BAUDRATE_3 (19200U)
volatile uint8_t state = -1;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {

		HAL_GPIO_TogglePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin);

}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
//  HAL_TIM_Base_Stop_IT(&htim2) ;

  while (1) {
		HAL_UART_Receive(&huart2, rx_buff, 2, HAL_MAX_DELAY);
		if (rx_buff[0] == '1') {

			HAL_UART_Transmit(&huart2, (uint8_t*) STRING_MODE1,STRING_MODEx_SIZE, HAL_MAX_DELAY);

			HAL_TIM_Base_Start_IT(&htim2) ;
			huart2.Instance->BRR = ((MCU_FREQ + (BAUDRATE_1 / 2)) / BAUDRATE_1);
			// set period of 2s
			HAL_TIM_Base_Stop_IT(&htim2) ;
			htim2.Instance->PSC = 16000 - 1;
			htim2.Instance->CNT = 2000 - 1;
			HAL_TIM_Base_Start_IT(&htim2) ;
		} else if (rx_buff[0] == '2') {

			HAL_UART_Transmit(&huart2, (uint8_t*) STRING_MODE2,STRING_MODEx_SIZE, HAL_MAX_DELAY);
			HAL_TIM_Base_Stop_IT(&htim2) ;
			// set period of 1s
			huart2.Instance->BRR = ((MCU_FREQ + (BAUDRATE_2 / 2)) / BAUDRATE_2);

			htim2.Instance->PSC = 16000 - 1;
			htim2.Instance->CNT = 1000 - 1;
			HAL_TIM_Base_Start_IT(&htim2) ;
		} else if (rx_buff[0] == '3') {
			HAL_UART_Transmit(&huart2, (uint8_t*) STRING_MODE3,STRING_MODEx_SIZE, HAL_MAX_DELAY);
			HAL_TIM_Base_Stop_IT(&htim2) ;
			huart2.Instance->BRR = ((MCU_FREQ + (BAUDRATE_3 / 2)) / BAUDRATE_3);
			// set period of 200 s
			htim2.Instance->PSC = 1600 - 1;
			htim2.Instance->CNT = 2000 - 1;
			HAL_TIM_Base_Start_IT(&htim2) ;

		} else {
			HAL_TIM_Base_Stop_IT(&htim2) ;

		}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* TIM2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
