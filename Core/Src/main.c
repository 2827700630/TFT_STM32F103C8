/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : UIDemo.c
 * @brief          : TFT UI组件演示程序
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
#include "TFTh/TFT_CAD.h"  // 包含绘图函数
#include "TFTh/TFT_init.h" // 包含初始化函数
#include "TFTh/TFT_text.h" // 包含文本显示函数
#include "TFTh/TFT_io.h"   // 包含IO函数
#include "TFTh/TFT_UI.h"   // 包含UI组件函数
#include <stdio.h>         // 用于sprintf
#include <math.h>          // 用于sin/cos
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SCREEN_WIDTH 240  // TFT屏幕宽度
#define SCREEN_HEIGHT 320 // TFT屏幕高度
#define PI 3.14159265358979323846f

// 模拟按键GPIO (如果有触摸屏，可以替换为触摸屏处理)
#define KEY_UP_PIN GPIO_PIN_0
#define KEY_DOWN_PIN GPIO_PIN_1
#define KEY_LEFT_PIN GPIO_PIN_2
#define KEY_RIGHT_PIN GPIO_PIN_3
#define KEY_OK_PIN GPIO_PIN_4
#define KEY_GPIO_PORT GPIOA

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
TFT_HandleTypeDef htft1; // TFT屏幕句柄

// UI组件声明
TFT_Button btn_ok;            // 确定按钮
TFT_Button btn_cancel;        // 取消按钮
TFT_ProgressBar progress_bar; // 进度条
TFT_Switch toggle_switch;     // 开关
TFT_Label label_title;        // 标题标签
TFT_Label label_status;       // 状态标签

// 状态变量
uint8_t current_progress = 0;  // 当前进度
bool auto_increase = true;     // 自动增加进度
uint32_t last_update_time = 0; // 上次更新时间
uint8_t active_component = 0;  // 当前激活的组件 (0=btn_ok, 1=btn_cancel, 2=progress_bar, 3=toggle_switch)
char status_text[50];          // 状态文本
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void TFT_UI_Demo_Init(void);
void TFT_UI_Process_Input(void);
void TFT_UI_Update(void);
bool Is_Key_Pressed(uint16_t key_pin);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
 * @brief 初始化UI演示
 * @retval 无
 */
void TFT_UI_Demo_Init(void)
{
  // 初始化TFT屏幕
  TFT_Init_Instance(&htft1, &hspi1, TFT_CS_GPIO_Port, TFT_CS_Pin);
  TFT_Config_Pins(&htft1, TFT_DC_GPIO_Port, TFT_DC_Pin,
                  TFT_RES_GPIO_Port, TFT_RES_Pin,
                  TFT_BL_GPIO_Port, TFT_BL_Pin);
  TFT_Config_Display(&htft1, 0, 0, 0); // 设置方向、X/Y偏移
  TFT_IO_Init(&htft1);                 // 初始化IO层
  TFT_Init_ST7789v3(&htft1);            // ST7735屏幕初始化

  // 清屏为黑色背景
  TFT_Fill_Area(&htft1, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);

  // 初始化标题标签
  TFT_Label_Init(&label_title, 0, 10, SCREEN_WIDTH, 20,
                 "UI DEMO", WHITE, BLACK, 16); // <--- Changed text_size to 16
  label_title.alignment = 1;                   // 设置居中对齐
  TFT_Label_Draw(&htft1, &label_title);

  // 初始化"确定"按钮
  TFT_Button_Init(&btn_ok, 10, 40, 50, 30, "OK", WHITE, GREEN, 0x8410);
  btn_ok.corner_radius = 5;
  btn_ok.text_size = 16; // <--- Changed text_size to 16
  TFT_Button_Draw(&htft1, &btn_ok);

  // 初始化"取消"按钮
  TFT_Button_Init(&btn_cancel, SCREEN_WIDTH - 60, 40, 50, 30, "Cancel", WHITE, RED, 0x8410);
  btn_cancel.corner_radius = 5;
  btn_cancel.text_size = 16; // <--- Changed text_size to 16
  TFT_Button_Draw(&htft1, &btn_cancel);

  // 初始化进度条
  TFT_ProgressBar_Init(&progress_bar, 10, 80, SCREEN_WIDTH - 20, 20,
                       0x4208, BLUE, 0x8410);
  progress_bar.show_percentage = true;
  TFT_ProgressBar_SetProgress(&progress_bar, current_progress);
  TFT_ProgressBar_Draw(&htft1, &progress_bar);

  // 初始化开关
  TFT_Switch_Init(&toggle_switch, SCREEN_WIDTH / 2 - 20, 110, 40, 20,
                  0x07E0, 0xF800, WHITE);
  TFT_Switch_Draw(&htft1, &toggle_switch);

  // 初始化状态标签
  sprintf(status_text, "Active: OK button");
  TFT_Label_Init(&label_status, 5, 140, SCREEN_WIDTH - 10, 15,
                 status_text, YELLOW, BLACK, 16); // <--- Changed text_size to 16
  TFT_Label_Draw(&htft1, &label_status);

  // 高亮当前选中的组件
  TFT_Button_SetState(&btn_ok, BUTTON_PRESSED);
  TFT_Button_Draw(&htft1, &btn_ok);

  // 记录当前时间
  last_update_time = HAL_GetTick();
}

/**
 * @brief 处理按键输入
 * @retval 无
 */
void TFT_UI_Process_Input(void)
{
  // 模拟处理按键输入 (根据实际硬件修改)

  // 移动焦点 (上下切换组件)
  if (Is_Key_Pressed(KEY_DOWN_PIN))
  {
    // 恢复当前组件的默认状态
    switch (active_component)
    {
    case 0: // OK按钮
      TFT_Button_SetState(&btn_ok, BUTTON_NORMAL);
      TFT_Button_Draw(&htft1, &btn_ok);
      break;
    case 1: // 取消按钮
      TFT_Button_SetState(&btn_cancel, BUTTON_NORMAL);
      TFT_Button_Draw(&htft1, &btn_cancel);
      break;
    case 2: // 进度条
      // 进度条无焦点状态，不需要处理
      break;
    case 3: // 开关
      // 开关无焦点状态，不需要处理
      break;
    }

    // 切换到下一个组件
    active_component = (active_component + 1) % 4;

    // 更新状态标签
    switch (active_component)
    {
    case 0:
      sprintf(status_text, "Active: OK button");
      TFT_Button_SetState(&btn_ok, BUTTON_PRESSED);
      TFT_Button_Draw(&htft1, &btn_ok);
      break;
    case 1:
      sprintf(status_text, "Active: Cancel button");
      TFT_Button_SetState(&btn_cancel, BUTTON_PRESSED);
      TFT_Button_Draw(&htft1, &btn_cancel);
      break;
    case 2:
      sprintf(status_text, "Active: Progress bar");
      break;
    case 3:
      sprintf(status_text, "Active: Switch");
      break;
    }

    TFT_Label_SetText(&label_status, status_text);
    TFT_Label_Draw(&htft1, &label_status);

    // 延迟以防止按键抖动
    HAL_Delay(200);
  }

  // 确认按键处理
  if (Is_Key_Pressed(KEY_OK_PIN))
  {
    switch (active_component)
    {
    case 0: // OK按钮
      sprintf(status_text, "OK button pressed!");
      break;
    case 1: // 取消按钮
      sprintf(status_text, "Cancel button pressed!");
      break;
    case 2: // 进度条
      // 切换进度条自动/手动模式
      auto_increase = !auto_increase;
      sprintf(status_text, "Progress: %s", auto_increase ? "Auto" : "Manual");
      break;
    case 3: // 开关
      // 切换开关状态
      TFT_Switch_Toggle(&toggle_switch);
      TFT_Switch_Draw(&htft1, &toggle_switch);
      sprintf(status_text, "Switch: %s", toggle_switch.state ? "ON" : "OFF");
      break;
    }

    TFT_Label_SetText(&label_status, status_text);
    TFT_Label_Draw(&htft1, &label_status);

    // 延迟以防止按键抖动
    HAL_Delay(200);
  }

  // 左右按键处理（用于手动调整进度条）
  if (active_component == 2 && !auto_increase)
  {
    if (Is_Key_Pressed(KEY_LEFT_PIN) && current_progress > 0)
    {
      current_progress -= 5;
      if (current_progress < 0)
        current_progress = 0;

      TFT_ProgressBar_SetProgress(&progress_bar, current_progress);
      TFT_ProgressBar_Draw(&htft1, &progress_bar);

      sprintf(status_text, "Progress: %d%%", current_progress);
      TFT_Label_SetText(&label_status, status_text);
      TFT_Label_Draw(&htft1, &label_status);

      // 延迟以防止按键抖动
      HAL_Delay(100);
    }
    else if (Is_Key_Pressed(KEY_RIGHT_PIN) && current_progress < 100)
    {
      current_progress += 5;
      if (current_progress > 100)
        current_progress = 100;

      TFT_ProgressBar_SetProgress(&progress_bar, current_progress);
      TFT_ProgressBar_Draw(&htft1, &progress_bar);

      sprintf(status_text, "Progress: %d%%", current_progress);
      TFT_Label_SetText(&label_status, status_text);
      TFT_Label_Draw(&htft1, &label_status);

      // 延迟以防止按键抖动
      HAL_Delay(100);
    }
  }
}

/**
 * @brief 更新UI状态
 * @retval 无
 */
void TFT_UI_Update(void)
{
  uint32_t current_time = HAL_GetTick();

  // 自动增加进度条
  if (auto_increase && (current_time - last_update_time) > 100)
  {
    current_progress = (current_progress + 1) % 101; // 0-100循环
    TFT_ProgressBar_SetProgress(&progress_bar, current_progress);
    TFT_ProgressBar_Draw(&htft1, &progress_bar);

    last_update_time = current_time;
  }

  // 可以添加更多UI更新逻辑，如动画效果等
}

/**
 * @brief 检查按键是否被按下
 * @param key_pin 按键引脚
 * @retval bool 是否被按下
 */
bool Is_Key_Pressed(uint16_t key_pin)
{
  // 注意：这里假设按键接地时为低电平（按下），实际应根据硬件连接方式修改
  return (HAL_GPIO_ReadPin(KEY_GPIO_PORT, key_pin) == GPIO_PIN_RESET);
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

  // 初始化UI演示
  TFT_UI_Demo_Init();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // 处理输入
    TFT_UI_Process_Input();

    // 更新UI状态
    TFT_UI_Update();
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