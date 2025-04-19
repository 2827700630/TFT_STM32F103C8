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
#include "dma.h"
#include "spi.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "TFTh/TFT.h"
#include "TFTh/TFT_init.h"
#include <stdio.h> // 添加 stdio.h 用于 sprintf
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
uint32_t total_frames = 0;    // 总帧数
uint32_t test_start_tick = 0; // 测试开始时间
uint8_t test_running = 1;     // 测试运行状态标志 (1: running, 0: stopped)
float fps = 0.0f;
char fps_str[30]; // 用于存储最终 FPS 字符串 (可能需要更长)
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_DMA_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  // TFT_Init_ST7735R(&hspi1); // LCD初始化，请看此函数注释 (示例：如果使用 ST7735R)
  TFT_Init_ST7735(&hspi1);              // TFT初始化, 传入 SPI1 句柄
  TFT_Fill_Area(0, 0, 128, 160, BLACK); // 黑色背景 (调整为常用尺寸)

  test_start_tick = HAL_GetTick();         // 记录测试开始时间
  test_running = 1;                        // 开始测试
  TFT_Fill_Circle(64, 80, 30, BLUE);       // 绘制一个蓝色圆形，圆心在(64, 80)，半径为30
  TFT_Draw_Rectangle(10, 10, 50, 50, RED); // 绘制一个红色矩形，左上角在(10, 10)，右下角在(50, 50)
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    TFT_Fill_Circle(64, 80, 30, BLUE);           // 绘制一个蓝色圆形，圆心在(64, 80)，半径为30
    HAL_Delay(1000);                             // 延时 1000ms
    TFT_Fill_Rectangle(60, 20, 128, 160, GREEN); // 填充绿色矩形，覆盖整个屏幕
    HAL_Delay(1000);                             // 延时 1000ms
    // if (test_running)
    // {
    //   HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin); // 翻转 LED 引脚的状态

    //   // 三原色刷屏测试 (作为帧率测试负载)
    //   TFT_Fill_Area(0, 0, 128, 160, BLUE);
    //   TFT_Fill_Area(0, 0, 128, 160, GREEN);
    //   TFT_Fill_Area(0, 0, 128, 160, RED);

    //   total_frames += 3; // 每次循环绘制了 3 帧 (蓝、绿、红)

    //   // 检查测试时间是否达到 10 秒
    //   if (HAL_GetTick() - test_start_tick >= 10000)
    //   {
    //     test_running = 0; // 停止测试

    //     // 计算最终平均 FPS
    //     uint32_t elapsed_time = HAL_GetTick() - test_start_tick; // 获取精确的经过时间
    //     fps = (float)total_frames * 1000.0f / (float)elapsed_time; // 使用实际经过时间计算，更精确

    //     // 格式化最终 FPS 字符串并显示
    //     sprintf(fps_str, "FPS: %.1f", fps);
    //     TFT_Fill_Area(0, 0, 128, 160, BLACK); // 清屏为黑色背景
    //    // TFT_Show_String(5, 5, fps_str, WHITE, BLACK, 16, 0); // 在 (5, 5) 位置显示白色最终 FPS 字符串
    //    // TFT_Show_String(5, 25, "Test Finished!", YELLOW, BLACK, 16, 0); // 显示测试完成信息
    //   }
    // }
    // else
    // {
    //   // 测试已停止，可以进入低功耗模式或执行其他操作
    //   HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET); // 关闭 LED (假设低电平点亮)
    // }
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

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
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
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
