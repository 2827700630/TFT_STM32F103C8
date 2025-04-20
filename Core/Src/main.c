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
#include "TFTh/TFT_CAD.h" // 包含绘图函数和 IO 函数
#include "TFTh/TFT_init.h"
#include "TFTh/TFT_text.h"
#include <stdio.h>   // 添加 stdio.h 用于 sprintf
#include <stdlib.h>  // 添加 stdlib.h 用于 rand()
#include <stdbool.h> // 添加 stdbool.h 用于 bool 类型
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
TFT_HandleTypeDef htft1; // 第一个TFT屏幕句柄
TFT_HandleTypeDef htft2; // 第二个TFT屏幕句柄

uint32_t frame_count = 0;
uint32_t start_tick = 0;
float avg_fps = 0.0f;
char fps_str[30]; // 用于显示 FPS
bool test_running = true;
const uint32_t test_duration_ms = 10000; // 测试持续时间 10 秒
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
  uint16_t rect_size = 60; // 增大矩形尺寸
  uint16_t pos_x = 0;
  uint16_t pos_y = 70; // 调整 Y 坐标以适应更大的矩形
  uint16_t rect_color = RED;
  char frame_str[20]; // 用于显示实时帧数
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
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */
  // 初始化第一个TFT屏幕
  TFT_Init_Instance(&htft1, &hspi1, TFT_CS_GPIO_Port, TFT_CS_Pin);
  TFT_Config_Pins(&htft1, TFT_DC_GPIO_Port, TFT_DC_Pin,
                  TFT_RES_GPIO_Port, TFT_RES_Pin,
                  TFT_BL_GPIO_Port, TFT_BL_Pin);
  TFT_Config_Display(&htft1, 0, 0, 0);          // 设置方向、X/Y偏移
  TFT_IO_Init(&htft1);                          // 初始化IO层
  TFT_Init_ST7789v3(&htft1);                    // ST7735S屏幕初始化
  TFT_Fill_Area(&htft1, 0, 0, 240, 320, BLACK); // 清屏为黑色背景

  TFT_Show_String(&htft1, 5, 25, (uint8_t *)"FPS Test Run", WHITE, BLACK, 16, 0);
  TFT_Show_String(&htft1, 5, 45, (uint8_t *)"Large Area", CYAN, BLACK, 16, 0);

  // 初始化第二个TFT屏幕
  TFT_Init_Instance(&htft2, &hspi1, CS2_GPIO_Port, CS2_Pin); // 第二个屏幕使用同一SPI接口
  TFT_Config_Pins(&htft2, DC2_GPIO_Port, DC2_Pin,RES2_GPIO_Port, RES2_Pin, BL2_GPIO_Port, BL2_Pin);
  TFT_Config_Display(&htft2, 2, 2, 1); // 设置方向、X/Y偏移
  TFT_IO_Init(&htft2);
  TFT_Init_ST7735S(&htft2);
  TFT_Fill_Area(&htft2, 0, 0, 128, 160, BLACK); // 清屏为黑色背景
  TFT_Show_String(&htft2, 5, 5, (uint8_t *)"FPS Test Run", WHITE, BLACK, 16, 0);
  TFT_Show_String(&htft2, 5, 25, (uint8_t *)"Large Area", CYAN, BLACK, 16, 0);

  start_tick = HAL_GetTick(); // 获取测试开始时间
  frame_count = 0;
  test_running = true;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if (test_running)
    {
      uint32_t current_tick = HAL_GetTick();
      uint32_t elapsed_ms = current_tick - start_tick;

      // 检查测试时间是否结束
      if (elapsed_ms >= test_duration_ms)
      {
        test_running = false; // 停止测试
        // 计算平均帧率
        avg_fps = (float)frame_count * 1000.0f / elapsed_ms;

        // 清屏并显示最终结果
        TFT_Fill_Area(&htft1, 0, 0, 128, 160, BLACK);
        TFT_Show_String(&htft1, 5, 5, (uint8_t *)"Test Finished!", GREEN, BLACK, 16, 0);
        sprintf(fps_str, "Avg FPS: %.1f", avg_fps);
        TFT_Show_String(&htft1, 5, 25, (uint8_t *)fps_str, YELLOW, BLACK, 16, 0);
        sprintf(fps_str, "Frames: %lu", frame_count);
        TFT_Show_String(&htft1, 5, 45, (uint8_t *)fps_str, WHITE, BLACK, 16, 0);
        sprintf(fps_str, "Time: %lu ms", elapsed_ms);
        TFT_Show_String(&htft1, 5, 65, (uint8_t *)fps_str, WHITE, BLACK, 16, 0);

        // 进入停止状态，只闪烁LED
        while (1)
        {
          HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
          HAL_Delay(500); // 慢速闪烁表示测试结束
        }
      }
      else
      {
        // --- 动态图形绘制 (大面积更新) ---
        // 1. 清除上一个大方块 (用背景色填充)
        TFT_Fill_Area(&htft1, pos_x, pos_y, pos_x + rect_size, pos_y + rect_size, BLACK);

        // 2. 更新位置和颜色
        pos_x = (pos_x + 3) % (128 - rect_size); // 在屏幕宽度内移动，留出方块宽度
        if (frame_count % 30 == 0)
        {                               // 每 30 帧改变一次颜色
          rect_color = rand() % 0xFFFF; // 随机颜色
        }

        // 3. 绘制新的大方块
        TFT_Fill_Area(&htft1, pos_x, pos_y, pos_x + rect_size, pos_y + rect_size, rect_color);

        // 4. 绘制一个简单的进度条指示测试时间
        uint16_t progress_width = (uint16_t)(((float)elapsed_ms / test_duration_ms) * 128);
        TFT_Fill_Area(&htft1, 0, 150, progress_width, 159, BLUE);   // 底部蓝色进度条
        TFT_Fill_Area(&htft1, progress_width, 150, 128, 159, GRAY); // 剩余部分灰色

        // 5. 显示实时帧数 (会稍微影响性能，但有助于观察)
        sprintf(frame_str, "Frame: %lu", frame_count);
        // 在一个固定区域显示帧数，使用背景色覆盖旧的数字
        TFT_Show_String(&htft1, 5, 130, (uint8_t *)frame_str, MAGENTA, BLACK, 16, 0);

        // --- 动态图形绘制结束 ---

        frame_count++; // 帧计数增加

        // LED闪烁表示程序运行
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
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
