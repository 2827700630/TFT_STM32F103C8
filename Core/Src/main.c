/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body - TFT CAD功能测试程序
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
#include "TFTh/TFT_io.h" // 包含RGB转换函数
#include <stdio.h>       // 添加 stdio.h 用于 sprintf
#include <stdlib.h>      // 添加 stdlib.h 用于 rand()
#include <stdbool.h>     // 添加 stdbool.h 用于 bool 类型
#include <math.h>        // 添加 math.h 用于三角函数
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum
{
  PAGE_BASIC_SHAPES,
  PAGE_ADVANCED_SHAPES,
  PAGE_POLYGONS,
  PAGE_CURVES,
  PAGE_COMBINED,
  PAGE_COUNT // 总页数
} DemoPage;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 160
#define DEMO_DELAY 3000 // 每页展示时间(毫秒)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
char text_buffer[32]; // 用于格式化字符串
DemoPage current_page = 0;
uint32_t page_start_time = 0;
bool auto_switch_pages = true; // 是否自动切换页面
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void Draw_Basic_Shapes(void);
void Draw_Advanced_Shapes(void);
void Draw_Polygons(void);
void Draw_Curves(void);
void Draw_Combined_Demo(void);
void Draw_Page_Title(const char *title, uint16_t color);
void Draw_Page_Footer(uint8_t current, uint8_t total);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
 * @brief 绘制页面标题
 */
void Draw_Page_Title(const char *title, uint16_t color)
{
  TFT_Fill_Rectangle(0, 0, SCREEN_WIDTH - 1, 16, BLUE);
  TFT_Show_String(5, 0, (uint8_t *)title, color, BLUE, 16, 0);
}

/**
 * @brief 绘制页面底部信息
 */
void Draw_Page_Footer(uint8_t current, uint8_t total)
{
  char footer[20];
  sprintf(footer, "Page %d/%d", current + 1, total);
  TFT_Fill_Rectangle(0, SCREEN_HEIGHT - 16, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, GRAY);
  TFT_Show_String(5, SCREEN_HEIGHT - 16, (uint8_t *)footer, WHITE, GRAY, 16, 0);
}

/**
 * @brief 绘制基本图形测试页
 */
void Draw_Basic_Shapes(void)
{
  // 清屏并绘制标题
  TFT_Fill_Area(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
  Draw_Page_Title("Basic Shapes", WHITE);

  // 绘制点阵
  for (uint8_t i = 0; i < 8; i++)
  {
    for (uint8_t j = 0; j < 8; j++)
    {
      TFT_Draw_Point(20 + i * 12, 25 + j * 5, TFT_RGB(255 - (i * 32), j * 32, 128));
    }
  }

  // 绘制线条
  TFT_Draw_Line(10, 70, 118, 70, RED);      // 水平线
  TFT_Draw_Line(10, 80, 118, 80, GREEN);    // 水平线
  TFT_Draw_Line(10, 70, 10, 100, BLUE);     // 垂直线
  TFT_Draw_Line(118, 70, 118, 100, BLUE);   // 垂直线
  TFT_Draw_Line(10, 100, 118, 100, YELLOW); // 水平线
  TFT_Draw_Line(10, 70, 118, 100, MAGENTA); // 对角线
  TFT_Draw_Line(118, 70, 10, 100, CYAN);    // 对角线

  // 绘制矩形
  TFT_Draw_Rectangle(15, 110, 55, 140, RED);    // 空心矩形
  TFT_Fill_Rectangle(65, 110, 105, 140, GREEN); // 实心矩形

  Draw_Page_Footer(PAGE_BASIC_SHAPES, PAGE_COUNT);
}

/**
 * @brief 绘制高级图形测试页
 */
void Draw_Advanced_Shapes(void)
{
  // 清屏并绘制标题
  TFT_Fill_Area(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
  Draw_Page_Title("Advanced Shapes", YELLOW);

  // 绘制圆
  TFT_Draw_Circle(32, 50, 20, RED);  // 空心圆
  TFT_Fill_Circle(96, 50, 20, BLUE); // 实心圆

  // 绘制椭圆
  TFT_Draw_Ellipse(32, 100, 25, 15, GREEN);   // 空心椭圆
  TFT_Fill_Ellipse(96, 100, 15, 25, MAGENTA); // 实心椭圆

  // 绘制圆角矩形
  TFT_Draw_Rounded_Rectangle(15, 130, 45, 25, 5, CYAN);    // 空心圆角矩形
  TFT_Fill_Rounded_Rectangle(70, 130, 45, 25, 10, YELLOW); // 实心圆角矩形

  Draw_Page_Footer(PAGE_ADVANCED_SHAPES, PAGE_COUNT);
}

/**
 * @brief 绘制多边形测试页
 */
void Draw_Polygons(void)
{
  // 清屏并绘制标题
  TFT_Fill_Area(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
  Draw_Page_Title("Polygons", GREEN);

  // 绘制三角形
  TFT_Draw_Triangle(30, 30, 10, 60, 50, 60, RED);   // 空心三角形
  TFT_Fill_Triangle(90, 30, 70, 60, 110, 60, BLUE); // 实心三角形

  // 绘制五边形
  TFT_Point pentagon[5] = {
      {30, 80},  // 顶点
      {10, 95},  // 左上
      {15, 120}, // 左下
      {45, 120}, // 右下
      {50, 95}   // 右上
  };
  TFT_Draw_Polygon(pentagon, 5, YELLOW);

  // 绘制六边形
  TFT_Point hexagon[6] = {
      {90, 80},   // 上
      {75, 95},   // 左上
      {75, 115},  // 左下
      {90, 130},  // 下
      {105, 115}, // 右下
      {105, 95}   // 右上
  };
  TFT_Fill_Polygon(hexagon, 6, MAGENTA);

  Draw_Page_Footer(PAGE_POLYGONS, PAGE_COUNT);
}

/**
 * @brief 绘制曲线测试页
 */
void Draw_Curves(void)
{
  // 清屏并绘制标题
  TFT_Fill_Area(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
  Draw_Page_Title("Curves", CYAN);

  // 绘制贝塞尔曲线
  TFT_Draw_Bezier2(10, 40, 64, 25, 118, 40, 20, RED);    // 二阶贝塞尔曲线
  TFT_Draw_Bezier2(10, 60, 64, 90, 118, 60, 20, GREEN);  // 二阶贝塞尔曲线
  TFT_Draw_Bezier2(10, 80, 118, 100, 64, 120, 20, BLUE); // 二阶贝塞尔曲线

  // 绘制圆弧
  TFT_Draw_Arc(32, 100, 20, 0, 90, YELLOW);     // 90度圆弧
  TFT_Draw_Arc(96, 100, 20, 180, 360, MAGENTA); // 180度圆弧

  // 绘制波浪线
  for (int16_t x = 0; x < SCREEN_WIDTH; x++)
  {
    int16_t y = 120 + (int16_t)(10 * sin(x * 0.2));
    TFT_Draw_Point(x, y, CYAN);
  }

  Draw_Page_Footer(PAGE_CURVES, PAGE_COUNT);
}

/**
 * @brief 绘制综合测试页
 */
void Draw_Combined_Demo(void)
{
  // 清屏并绘制标题
  TFT_Fill_Area(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
  Draw_Page_Title("Combined Demo", MAGENTA);

  // 绘制气泡聊天界面
  TFT_Fill_Rounded_Rectangle(10, 30, 108, 30, 5, BLUE); // 消息气泡1
  TFT_Show_String(15, 35, (uint8_t *)"Hello TFT!", WHITE, BLUE, 16, 0);

  TFT_Fill_Rounded_Rectangle(30, 70, 88, 30, 5, GREEN); // 消息气泡2
  TFT_Show_String(35, 75, (uint8_t *)"CAD Demo", BLACK, GREEN, 16, 0);

  // 绘制简单仪表盘
  TFT_Draw_Circle(64, 120, 25, WHITE); // 仪表盘外圈
  TFT_Draw_Circle(64, 120, 2, RED);    // 仪表盘中心点

  // 绘制指针
  float angle = 3.14159f * 0.75f; // 135度
  int16_t needle_x = 64 + (int16_t)(22 * cos(angle));
  int16_t needle_y = 120 + (int16_t)(22 * sin(angle));
  TFT_Draw_Line(64, 120, needle_x, needle_y, RED);

  // 绘制刻度
  for (uint8_t i = 0; i < 12; i++)
  {
    float mark_angle = 3.14159f * (0.5f + (i * 0.0833f)); // 从90度开始，每30度一个刻度
    int16_t mark_x1 = 64 + (int16_t)(25 * cos(mark_angle));
    int16_t mark_y1 = 120 + (int16_t)(25 * sin(mark_angle));
    int16_t mark_x2 = 64 + (int16_t)(22 * cos(mark_angle));
    int16_t mark_y2 = 120 + (int16_t)(22 * sin(mark_angle));
    TFT_Draw_Line(mark_x1, mark_y1, mark_x2, mark_y2, YELLOW);
  }

  Draw_Page_Footer(PAGE_COMBINED, PAGE_COUNT);
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
  MX_DMA_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  TFT_Init_ST7735(&hspi1);                                 // TFT初始化, 传入 SPI1 句柄
  TFT_Fill_Area(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK); // 清屏为黑色背景

  // 显示欢迎信息
  TFT_Show_String(5, 60, (uint8_t *)"TFT CAD Demo", WHITE, BLACK, 16, 0);
  TFT_Show_String(15, 80, (uint8_t *)"Starting...", GREEN, BLACK, 16, 0);
  HAL_Delay(1000); // 显示欢迎信息1秒

  page_start_time = HAL_GetTick(); // 获取开始时间
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // 根据当前页面显示不同的测试内容
    switch (current_page)
    {
    case PAGE_BASIC_SHAPES:
      Draw_Basic_Shapes();
      break;
    case PAGE_ADVANCED_SHAPES:
      Draw_Advanced_Shapes();
      break;
    case PAGE_POLYGONS:
      Draw_Polygons();
      break;
    case PAGE_CURVES:
      Draw_Curves();
      break;
    case PAGE_COMBINED:
      Draw_Combined_Demo();
      break;
    default:
      current_page = 0; // 防止越界
      continue;
    }

    // 等待一段时间后切换到下一个页面
    uint32_t current_time;
    do
    {
      HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin); // LED闪烁表示程序运行
      HAL_Delay(100);                             // 降低CPU负载，同时保持LED闪烁
      current_time = HAL_GetTick();
    } while (auto_switch_pages && (current_time - page_start_time < DEMO_DELAY));

    // 更新页面和时间
    current_page = (current_page + 1) % PAGE_COUNT;
    page_start_time = HAL_GetTick();

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
