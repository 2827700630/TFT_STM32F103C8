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
#include <stdio.h>  // 添加 stdio.h 用于 sprintf
#include <stdlib.h> // 添加 stdlib.h 用于 rand()
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
uint32_t frame_count = 0;
uint32_t start_tick = 0;
float fps = 0.0f;
char fps_str[20]; // 用于显示 FPS
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
  TFT_Init_ST7735(&hspi1);              // TFT初始化, 传入 SPI1 句柄
  TFT_Fill_Area(0, 0, 128, 160, BLACK); // 清屏为黑色背景

  start_tick = HAL_GetTick(); // 获取开始时间
  frame_count = 0;

  // 动画变量初始化
  int16_t rect_x = 10;
  int16_t rect_y = 10;
  int16_t rect_w = 30;
  int16_t rect_h = 20;
  int16_t rect_dx = 2; // X方向速度
  int16_t rect_dy = 1; // Y方向速度

  uint8_t circle_r = 10;
  uint8_t circle_r_dir = 1; // 半径变化方向

  uint16_t current_color = RED;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // 1. 清屏 (或只清除需要更新的区域以提高效率)
    TFT_Fill_Area(0, 0, 128, 160, BLACK);

    // 2. 更新动画状态
    // 移动矩形
    rect_x += rect_dx;
    rect_y += rect_dy;

    // 边界碰撞检测
    if (rect_x <= 0 || rect_x + rect_w >= 128)
    {
<<<<<<< Updated upstream
      rect_dx = -rect_dx; // X方向反向
      rect_x += rect_dx;  // 调整位置防止卡住
=======
      uint32_t current_tick = HAL_GetTick();
      uint32_t elapsed_ms = current_tick - start_tick;

      // 检查测试时间是否结束
      if (elapsed_ms >= test_duration_ms)
      {
        test_running = false; // 停止测试
        // 计算平均帧率
        avg_fps = (float)frame_count * 1000.0f / elapsed_ms;

        // 清屏并显示最终结果
        TFT_Fill_Area(0, 0, 128, 160, BLACK);
        TFT_Show_String(5, 5, (uint8_t *)"Test Finished!", GREEN, BLACK, 16, 0);
        sprintf(fps_str, "Avg FPS: %.1f", avg_fps);
        TFT_Show_String(5, 25, (uint8_t *)fps_str, YELLOW, BLACK, 16, 0);
        sprintf(fps_str, "Frames: %lu", frame_count);
        TFT_Show_String(5, 45, (uint8_t *)fps_str, WHITE, BLACK, 16, 0);
        sprintf(fps_str, "Time: %lu ms", elapsed_ms);
        TFT_Show_String(5, 65, (uint8_t *)fps_str, WHITE, BLACK, 16, 0);

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
        TFT_Fill_Area(pos_x, pos_y, pos_x + rect_size, pos_y + rect_size, BLACK);

        // 2. 更新位置和颜色
        pos_x = (pos_x + 3) % (128 - rect_size); // 在屏幕宽度内移动，留出方块宽度
        if (frame_count % 30 == 0)
        {                               // 每 30 帧改变一次颜色
          rect_color = rand() % 0xFFFF; // 随机颜色
        }

        // 3. 绘制新的大方块
        TFT_Fill_Area(pos_x, pos_y, pos_x + rect_size, pos_y + rect_size, rect_color);

        // 4. 绘制一个简单的进度条指示测试时间
        uint16_t progress_width = (uint16_t)(((float)elapsed_ms / test_duration_ms) * 128);
        TFT_Fill_Area(0, 150, progress_width, 159, BLUE);   // 底部蓝色进度条
        TFT_Fill_Area(progress_width, 150, 128, 159, GRAY); // 剩余部分灰色

        // 5. 显示实时帧数 (会稍微影响性能，但有助于观察)
        sprintf(frame_str, "Frame: %lu", frame_count);
        // 在一个固定区域显示帧数，使用背景色覆盖旧的数字
        TFT_Show_String(5, 130, (uint8_t *)frame_str, MAGENTA, BLACK, 16, 0);

        // --- 动态图形绘制结束 ---

        frame_count++; // 帧计数增加

        // LED闪烁表示程序运行
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
      }
>>>>>>> Stashed changes
    }
    if (rect_y <= 0 || rect_y + rect_h >= 160)
    {
      rect_dy = -rect_dy; // Y方向反向
      rect_y += rect_dy;  // 调整位置防止卡住
    }

    // 变化圆半径
    if (circle_r_dir)
    {
      circle_r++;
      if (circle_r >= 30)
        circle_r_dir = 0;
    }
    else
    {
      circle_r--;
      if (circle_r <= 5)
        circle_r_dir = 1;
    }

    // 变化颜色 (简单示例：在几种颜色间切换)
    current_color = (uint16_t)rand(); // 随机颜色，或者使用更平滑的过渡

    // 3. 绘制图形
    TFT_Fill_Rectangle(rect_x, rect_y, rect_x + rect_w - 1, rect_y + rect_h - 1, BLUE);
    TFT_Fill_Circle(64, 80, circle_r, current_color);

    // 4. 帧率计算与显示 (可选，需要显示函数支持)
    frame_count++;
    uint32_t current_tick = HAL_GetTick();
    uint32_t elapsed_time = current_tick - start_tick;
    if (elapsed_time >= 1000) // 每秒更新一次 FPS
    {
      fps = (float)frame_count * 1000.0f / (float)elapsed_time;
      sprintf(fps_str, "FPS:%.1f", fps);
      // TFT_Show_String(5, 5, (uint8_t*)fps_str, WHITE, BLACK, 16, 0); // 需要 TFT_Show_String 函数
      start_tick = current_tick;
      frame_count = 0;
    }

    // 5. LED 闪烁与延时
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    // HAL_Delay(10); // 可以添加少量延时来控制帧率，或移除以测试最大性能

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
