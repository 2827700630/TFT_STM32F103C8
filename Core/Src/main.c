/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body - TFT 多屏显示功能测试程序
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
#include <math.h>        // 添加 math.h 用于三角函数

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SCREEN_WIDTH  128 // TFT屏幕宽度
#define SCREEN_HEIGHT 160 // TFT屏幕高度
#define PI 3.14159265358979323846f

// 定义第二个屏幕的CS引脚 (仅用于演示，应根据实际硬件配置)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
TFT_HandleTypeDef htft1; // 第一个TFT屏幕句柄
TFT_HandleTypeDef htft2; // 第二个TFT屏幕句柄

char textBuf[32]; // 文本缓冲区
uint16_t angle = 0; // 动画角度
uint32_t frameCount = 0; // 帧计数器
uint32_t lastTick = 0;   // 上次计时点
float fps = 0;          // 帧率
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void TFT_Demo_Init(void);
void TFT_Demo_Screen1(void);
void TFT_Demo_Screen2(void);
void TFT_Draw_Clock(TFT_HandleTypeDef *htft, uint16_t centerX, uint16_t centerY, uint16_t radius, uint16_t color);
void TFT_Draw_Bar_Chart(TFT_HandleTypeDef *htft, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t values[], uint8_t valueCount, uint16_t colors[]);
void TFT_Draw_Animation(TFT_HandleTypeDef *htft, uint16_t x, uint16_t y, uint16_t size, uint16_t color);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
 * @brief 初始化TFT屏幕
 * @retval 无
 */
void TFT_Demo_Init(void) 
{
    // 初始化第一个TFT屏幕
    TFT_Init_Instance(&htft1, &hspi1, TFT_CS_GPIO_Port, TFT_CS_Pin);
    TFT_Config_Pins(&htft1, TFT_DC_GPIO_Port, TFT_DC_Pin,
                    TFT_RES_GPIO_Port, TFT_RES_Pin,
                    TFT_BL_GPIO_Port, TFT_BL_Pin);
    TFT_Config_Display(&htft1, 0, 0, 0); // 设置方向、X/Y偏移
    TFT_IO_Init(&htft1); // 初始化IO层
    TFT_Init_ST7789v3(&htft1); // ST7789屏幕初始化
    
    // 初始化第二个TFT屏幕
    // 注: 在实际使用时，可能需要配置第二个SPI接口，或使用同一SPI但不同CS
    TFT_Init_Instance(&htft2, &hspi2, CS2_GPIO_Port, CS2_Pin);
    TFT_Config_Pins(&htft2, DC2_GPIO_Port, DC2_Pin,
                    RES2_GPIO_Port, RES2_Pin,
                    BL2_GPIO_Port, BL2_Pin);
    TFT_Config_Display(&htft2, 2, 2, 1); // 设置方向、X/Y偏移
    TFT_IO_Init(&htft2); // 初始化IO层
    TFT_Init_ST7735S(&htft2); // ST7735S屏幕初始化
    
    // 设置不同的缓冲区大小以测试内存管理
    
    // 初始化帧率计时
    lastTick = HAL_GetTick();
}

/**
 * @brief 在第一个屏幕上绘制模拟时钟和条形图
 * @retval 无
 */
void TFT_Demo_Screen1(void)
{
    // 清屏为黑色
    TFT_Fill_Area(&htft1, 0, 0, 240, 320, BLACK);
    
    // 绘制标题
    TFT_Show_String(&htft1, 20, 35, (uint8_t *)"Screen #1", WHITE, BLACK, 16, 0);
    
    // 绘制圆形边框
    TFT_Draw_Circle(&htft1, 64, 85, 30, CYAN);
    
    // 绘制模拟时钟
    TFT_Draw_Clock(&htft1, 64, 85, 25, WHITE);
    
    // 绘制帧率文本
    sprintf(textBuf, "FPS: %.1f", fps);
    TFT_Show_String(&htft1, 20, 150, (uint8_t *)textBuf, GREEN, BLACK, 16, 0);
    
    // 绘制帧计数器
    sprintf(textBuf, "Frame: %lu", frameCount);
    TFT_Show_String(&htft1, 20, 170, (uint8_t *)textBuf, GREEN, BLACK, 16, 0);
    
    // 绘制条形图数据
    uint16_t values[5] = {10 + (frameCount % 30), 20 + (frameCount % 20), 
                          35 + (frameCount % 10), 15 + (frameCount % 25), 
                          25 + (frameCount % 15)};
    uint16_t colors[5] = {RED, GREEN, BLUE, YELLOW, MAGENTA};
    
    // 绘制条形图
    TFT_Draw_Bar_Chart(&htft1, 20, 205, 90, 40, values, 5, colors);
}

/**
 * @brief 在第二个屏幕上绘制动画和几何图形
 * @retval 无
 */
void TFT_Demo_Screen2(void)
{
    // 清屏为深蓝色
    TFT_Fill_Area(&htft2, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, BLUE);
    
    // 绘制标题
    TFT_Show_String(&htft2, 20, 5, (uint8_t *)"Screen #2", WHITE, BLUE, 16, 0);
    
    // 绘制旋转动画
    TFT_Draw_Animation(&htft2, 64, 45, 30, YELLOW);
    
    // 绘制三角形，随角度变化
    uint16_t x1 = 64 + 40 * cos(angle * PI / 180.0f);
    uint16_t y1 = 100 + 20 * sin(angle * PI / 180.0f);
    uint16_t x2 = 64 + 40 * cos((angle + 120) * PI / 180.0f);
    uint16_t y2 = 100 + 20 * sin((angle + 120) * PI / 180.0f);
    uint16_t x3 = 64 + 40 * cos((angle + 240) * PI / 180.0f);
    uint16_t y3 = 100 + 20 * sin((angle + 240) * PI / 180.0f);
    
    TFT_Draw_Triangle(&htft2, x1, y1, x2, y2, x3, y3, GREEN);
    
    // 绘制椭圆并填充
    TFT_Fill_Ellipse(&htft2, 64, 130, 50, 20, RED);
    TFT_Draw_Ellipse(&htft2, 64, 130, 50, 20, WHITE);
}

/**
 * @brief 绘制模拟时钟
 * @param htft TFT句柄指针
 * @param centerX 时钟中心X坐标
 * @param centerY 时钟中心Y坐标
 * @param radius 时钟半径
 * @param color 时钟颜色
 * @retval 无
 */
void TFT_Draw_Clock(TFT_HandleTypeDef *htft, uint16_t centerX, uint16_t centerY, uint16_t radius, uint16_t color)
{
    int i;
    
    // 绘制时钟刻度
    for (i = 0; i < 12; i++) {
        float angle = i * 30.0f * PI / 180.0f;
        uint16_t x1 = centerX + (radius - 5) * cos(angle);
        uint16_t y1 = centerY + (radius - 5) * sin(angle);
        uint16_t x2 = centerX + radius * cos(angle);
        uint16_t y2 = centerY + radius * sin(angle);
        TFT_Draw_Line(htft, x1, y1, x2, y2, color);
    }
    
    // 绘制时针（基于angle值旋转）
    float hourAngle = (angle / 30.0f) * PI / 180.0f; // 每12秒转一圈
    uint16_t hourX = centerX + (radius - 10) * cos(hourAngle);
    uint16_t hourY = centerY + (radius - 10) * sin(hourAngle);
    TFT_Draw_Line(htft, centerX, centerY, hourX, hourY, RED);
    
    // 绘制分针（基于angle值旋转，比时针快）
    float minAngle = (angle * 5 / 30.0f) * PI / 180.0f; // 每2.4秒转一圈
    uint16_t minX = centerX + (radius - 5) * cos(minAngle);
    uint16_t minY = centerY + (radius - 5) * sin(minAngle);
    TFT_Draw_Line(htft, centerX, centerY, minX, minY, GREEN);
    
    // 绘制秒针（基于angle值旋转，最快）
    float secAngle = (angle * 12 / 30.0f) * PI / 180.0f; // 每秒转一圈
    uint16_t secX = centerX + radius * cos(secAngle);
    uint16_t secY = centerY + radius * sin(secAngle);
    TFT_Draw_Line(htft, centerX, centerY, secX, secY, YELLOW);
    
    // 绘制时钟中心点
    TFT_Fill_Circle(htft, centerX, centerY, 2, WHITE);
}

/**
 * @brief 绘制条形图
 * @param htft TFT句柄指针
 * @param x 起始X坐标
 * @param y 起始Y坐标
 * @param width 图表宽度
 * @param height 图表高度
 * @param values 数据值数组
 * @param valueCount 数据值数量
 * @param colors 条形颜色数组
 * @retval 无
 */
void TFT_Draw_Bar_Chart(TFT_HandleTypeDef *htft, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t values[], uint8_t valueCount, uint16_t colors[])
{
    uint8_t i;
    uint16_t barWidth = width / valueCount;
    uint16_t maxValue = 0;
    
    // 查找最大值
    for (i = 0; i < valueCount; i++) {
        if (values[i] > maxValue)
            maxValue = values[i];
    }
    
    // 绘制坐标轴
    TFT_Draw_Line(htft, x, y + height, x + width, y + height, WHITE); // X轴
    TFT_Draw_Line(htft, x, y, x, y + height, WHITE);                 // Y轴
    
    // 绘制条形
    for (i = 0; i < valueCount; i++) {
        uint16_t barHeight = (values[i] * height) / (maxValue + 5); // +5避免高度为0
        uint16_t barX = x + i * barWidth + 1;
        uint16_t barY = y + height - barHeight;
        
        TFT_Fill_Rectangle(htft, barX, barY, barX + barWidth - 2, y + height - 1, colors[i]);
    }
}

/**
 * @brief 绘制动画效果
 * @param htft TFT句柄指针
 * @param x 中心X坐标
 * @param y 中心Y坐标
 * @param size 动画大小
 * @param color 动画颜色
 * @retval 无
 */
void TFT_Draw_Animation(TFT_HandleTypeDef *htft, uint16_t x, uint16_t y, uint16_t size, uint16_t color)
{
    int i;
    for (i = 0; i < 12; i++) {
        float ang = (i * 30.0f + angle) * PI / 180.0f;
        uint16_t x1 = x + (size - 10) * cos(ang);
        uint16_t y1 = y + (size - 10) * sin(ang);
        uint16_t x2 = x + size * cos(ang);
        uint16_t y2 = y + size * sin(ang);
        
        uint8_t intensity = ((i + angle / 30) % 12) * 255 / 12;
        uint16_t dotColor = TFT_RGB(intensity, intensity, color >> 8);
        
        TFT_Draw_Line(htft, x1, y1, x2, y2, dotColor);
        TFT_Fill_Circle(htft, x2, y2, 2, color);
    }
    
    // 绘制中心圆
    TFT_Fill_Circle(htft, x, y, 5, WHITE);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  uint32_t currentTick;
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
  
  // 初始化TFT屏幕
  TFT_Demo_Init();
  
  // 显示启动画面
  TFT_Fill_Area(&htft1, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, BLACK);
  TFT_Show_String(&htft1, 10, 60, (uint8_t *)"TFT Multi-Screen", WHITE, BLACK, 16, 0);
  TFT_Show_String(&htft1, 20, 80, (uint8_t *)"Demo Starting...", GREEN, BLACK, 16, 0);
  
  TFT_Fill_Area(&htft2, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, BLUE);
  TFT_Show_String(&htft2, 15, 60, (uint8_t *)"Screen 2 Ready", WHITE, BLUE, 16, 0);
  TFT_Show_String(&htft2, 20, 80, (uint8_t *)"Please Wait...", YELLOW, BLUE, 16, 0);
  
  // 启动延时
  HAL_Delay(1000);
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // 更新帧计数和动画角度
    frameCount++;
    angle = (angle + 3) % 360;
    
    // 计算帧率
    currentTick = HAL_GetTick();
    if (currentTick - lastTick >= 1000) { // 每秒更新一次帧率
        fps = frameCount * 1000.0f / (currentTick - lastTick);
        frameCount = 0;
        lastTick = currentTick;
    }
    
    // 更新两个屏幕显示
    TFT_Demo_Screen1();
    TFT_Demo_Screen2();
    
    // 控制刷新速度
    // HAL_Delay(10);
    
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
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
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
