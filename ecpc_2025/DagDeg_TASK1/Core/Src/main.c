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
#include "adc.h"
#include "dma.h"
#include "gpio.h"
#include "tim.h"
#include <stm32f4xx_hal_flash.h>
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define AVG_SLOPE (2.5F)
#define V_AT_25C (0.76F)
#define V_REF_INT (3.0F)
#define ADC_MAX_RES (4095)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define START_FALSH_MEM_ADDR_W ((uint32_t*)0x080E0000)
#define TEMP_40 40
#define TEMP_30 30
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

volatile uint16_t adcVal[1];
volatile uint8_t cnvCplt = 0;
volatile double myTemp = 0.0;

double calcTemp(uint8_t adc_val)
{
    double vsense = (double)(adc_val * V_REF_INT) / ADC_MAX_RES;
    double temp = ((vsense - V_AT_25C) / AVG_SLOPE) + 25;
    return temp;
}
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* hadc)
{
    HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ORANGE_LED_GPIO_Port, ORANGE_LED_Pin, GPIO_PIN_RESET);
}
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    cnvCplt = 1;
    HAL_GPIO_TogglePin(debug_led_GPIO_Port, debug_led_Pin);
    HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ORANGE_LED_GPIO_Port, ORANGE_LED_Pin, GPIO_PIN_RESET);
}
// void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
// {
//     if (htim->Instance == TIM5)
//     {
//         HAL_GPIO_TogglePin(debug_led_GPIO_Port, debug_led_Pin);
//     }
// }
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU
     * Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the
     * Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_DMA_Init();
    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_TIM2_Init();

    /* Initialize interrupts */
    MX_NVIC_Init();

    HAL_TIM_Base_Start(&htim2);
    HAL_TIM_Base_Start_IT(&htim5);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adcVal, 2);
    // HAL_ADC_PollForConversion();
    while (1)
    {
        if (cnvCplt)
        {
            cnvCplt = 0;
            myTemp = calcTemp(adcVal[0]);
            if (myTemp < TEMP_30)
            {
                // FLASH_PageWrite(BANK_1, FLASH_ADDRESS(0x08000000));
                HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin,
                                  GPIO_PIN_SET);
                HAL_GPIO_WritePin(ORANGE_LED_GPIO_Port, ORANGE_LED_Pin,
                                  GPIO_PIN_RESET);
                HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin,
                                  GPIO_PIN_RESET);
            }
            else if (myTemp >= TEMP_30)
            {
                HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin,
                                  GPIO_PIN_RESET);
                HAL_GPIO_WritePin(ORANGE_LED_GPIO_Port, ORANGE_LED_Pin,
                                  GPIO_PIN_SET);
                HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin,
                                  GPIO_PIN_RESET);

                if (myTemp > TEMP_40)
                {
                    if (HAL_FLASH_Unlock() == HAL_OK)
                    {
                        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
                                              ((START_FALSH_MEM_ADDR_W)),
                                              myTemp) == HAL_OK)
                        {
                        }
                        HAL_FLASH_Lock();
                    }
                }
            }
            else
            {
            }
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
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
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
    /* DMA2_Stream0_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
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
    /* User can add his own implementation to report the HAL error return state
     */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line
       number, ex: printf("Wrong parameters value: file %s on line %d\r\n",
       file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
