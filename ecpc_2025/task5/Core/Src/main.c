/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention = HAL_DMA_Trf_Cplt ;
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
#include "dma.h"
#include "gpio.h"
#include "tim.h"

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

/* USER CODE BEGIN PV */
#define DMA_STAR_STATE 0
#define DMA_PAUSE_STATE 1
extern DMA_HandleTypeDef hdma_tim1_up;
// uint16_t sequence[8] = {0x1, 0x3, 0x6, 0xE, 0xC, 0x8, 0x9, 0x0};
uint32_t sequence[8] = {
    (0x1 << 15), // 0x1000: BLUE (PD15)
    (0x3 << 14), // 0x3000: red (PD14) + blue (PD15)
    (0x6 << 12), // 0x6000: Orange (PD13) + Red (PD14)
    (0xE << 11), // 0xE000: Green (PD12) + Orange (PD13) + Red (PD14)
    (0xC << 10), // 0xC000: Green (PD12) + Orange (PD13)
    (0x8 << 9),  // 0x8000: Green (PD12)
    (0x9 << 12), // 0x9000: Green (PD12) + blue (PD15)
    (0x0 << 15)  // 0x0000: All off
};
volatile uint8_t transferIndex = 0;
volatile uint8_t systemState = 0;
volatile uint8_t dmaStopped = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
// {
//     // HAL_GPIO_TogglePin(GPIOD, GREEN_LED_Pin);
// }
void HAL_DMA_XferCpltCallback(DMA_HandleTypeDef* hdma)
{
    if (dmaStopped)
    {
        __HAL_DMA_DISABLE(&hdma_tim1_up);
        hdma_tim1_up.Instance->M0AR =(uint32_t)&sequence[0]; 
        hdma_tim1_up.Instance->PAR =(uint32_t)&GPIOD->ODR; 
        hdma_tim1_up.Instance->NDTR = 8; 
        __HAL_DMA_ENABLE(&hdma_tim1_up);
        dmaStopped = 0;
    }
}
// HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == BTN_Pin)
    {
        NVIC_DisableIRQ(EXTI0_IRQn);
        if (systemState == DMA_STAR_STATE)
        {
            transferIndex = 7 - hdma_tim1_up.Instance->NDTR;
            dmaStopped = 1;
            HAL_DMA_Abort_IT(&hdma_tim1_up);

            // __HAL_DMA_GET_COUNTER
            systemState = DMA_PAUSE_STATE;
        }
        else if (systemState == DMA_PAUSE_STATE)
        {

            __HAL_DMA_DISABLE(&hdma_tim1_up);
            hdma_tim1_up.Instance->M0AR = (uint32_t)&sequence[transferIndex]; 
            hdma_tim1_up.Instance->PAR = (uint32_t)&GPIOD->ODR; 
            hdma_tim1_up.Instance->NDTR = 8 - transferIndex; 
            __HAL_DMA_ENABLE(&hdma_tim1_up);


            systemState = DMA_STAR_STATE;
        }
        else
        {
            // nothing to do
        }
        NVIC_EnableIRQ(EXTI0_IRQn);
    }
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
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_TIM1_Init();

    /* Initialize interrupts */
    MX_NVIC_Init();
    /* USER CODE BEGIN 2 */
    __HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_UPDATE);
    hdma_tim1_up.XferCpltCallback = HAL_DMA_XferCpltCallback;
    HAL_DMA_Start_IT(&hdma_tim1_up, (uint32_t)&sequence[0],
                     (uint32_t)&GPIOD->ODR, 8);

    HAL_TIM_Base_Start(&htim1);

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
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
    /* DMA2_Stream5_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA2_Stream5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);
    /* EXTI0_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
    /* TIM1_UP_TIM10_IRQn interrupt configuration */
    // REMOVED
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
    /* User can add his own implementation to report the HAL error return
     * state
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
