#ifndef __TFT_INIT_H
#define __TFT_INIT_H
#include "main.h"
#include <stdint.h>

//----------- 显示方向设置 -----------
// 控制LCD显示方向 (DISPLAY_DIRECTION)
// ST7735S: 0=上下颠倒, 1=正常, 2=逆时针旋转90度, 3=顺时针旋转90度
// ST7735R: 4=正常, 5=上下颠倒
// 请根据实际使用的LCD型号和期望的显示方向选择合适的宏值
#define DISPLAY_DIRECTION 4
//------------------------------------

// 根据显示方向定义LCD逻辑宽高
#if DISPLAY_DIRECTION == 0 || DISPLAY_DIRECTION == 1 || DISPLAY_DIRECTION == 4 || DISPLAY_DIRECTION == 5
#define LCD_WIDTH 128  // 逻辑宽度
#define LCD_HEIGHT 128 // 逻辑高度 (注意：此宏定义仅为逻辑尺寸，实际驱动中可能根据方向调整)
#else                  // 方向 2 或 3 (旋转)
#define LCD_WIDTH 128  // 逻辑宽度
#define LCD_HEIGHT 128 // 逻辑高度
#endif

//----------------- TFT 控制引脚宏定义 -----------------
// 使用HAL库函数简化引脚操作

// 复位引脚 (RES/RST)
#define TFT_RES_LOW() HAL_GPIO_WritePin(TFT_RES_GPIO_Port, TFT_RES_Pin, GPIO_PIN_RESET) // 拉低RES引脚
#define TFT_RES_HIGH() HAL_GPIO_WritePin(TFT_RES_GPIO_Port, TFT_RES_Pin, GPIO_PIN_SET)  // 拉高RES引脚

// 数据/命令选择引脚 (DC/RS)
#define TFT_DC_LOW() HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_RESET) // 选择命令模式
#define TFT_DC_HIGH() HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_SET)  // 选择数据模式

// 片选引脚 (CS)
#define TFT_CS_LOW() HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, GPIO_PIN_RESET) // 片选选中
#define TFT_CS_HIGH() HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, GPIO_PIN_SET)  // 片选取消

// 背光引脚 (BLK/BL)
#define TFT_BLK_LOW() HAL_GPIO_WritePin(TFT_BL_GPIO_Port, TFT_BL_Pin, GPIO_PIN_RESET) // 关闭背光
#define TFT_BLK_HIGH() HAL_GPIO_WritePin(TFT_BL_GPIO_Port, TFT_BL_Pin, GPIO_PIN_SET)  // 打开背光

//----------------- 常用颜色定义 (RGB565格式) -----------------
#define WHITE 0xFFFF   // 白色
#define BLACK 0x0000   // 黑色
#define BLUE 0x001F    // 蓝色
#define BRED 0XF81F    // 蓝红色 (洋红)
#define GRED 0XFFE0    // 绿红色 (黄色)
#define GBLUE 0X07FF   // 绿蓝色 (青色)
#define RED 0xF800     // 红色
#define MAGENTA 0xF81F // 品红色 (同 BRED)
#define GREEN 0x07E0   // 绿色
#define CYAN 0x7FFF    // 青色 (同 GBLUE)
#define YELLOW 0xFFE0  // 黄色 (同 GRED)
#define BROWN 0XBC40   // 棕色
#define BRRED 0XFC07   // 棕红色
#define GRAY 0X8430    // 灰色

//----------------- TFT驱动函数声明 -----------------

/**
 * @brief  通过SPI总线发送一个字节的数据
 * @param  data 要发送的字节
 * @retval 无
 */
void TFT_Write_Bus(uint8_t data);

/**
 * @brief  向TFT写入8位数据
 * @param  data 要写入的8位数据
 * @retval 无
 */
void TFT_Write_Data8(uint8_t data);

/**
 * @brief  向TFT写入16位数据
 * @param  data 要写入的16位数据
 * @retval 无
 */
void TFT_Write_Data16(uint16_t data);

/**
 * @brief  向TFT写入命令
 * @param  command 要写入的命令字节
 * @retval 无
 */
void TFT_Write_Command(uint8_t command);

/**
 * @brief  设置TFT显示区域 (窗口)
 * @param  x_start 列起始坐标
 * @param  y_start 行起始坐标
 * @param  x_end   列结束坐标
 * @param  y_end   行结束坐标
 * @retval 无
 * @note   坐标从0开始
 */
void TFT_Set_Address(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end);

/**
 * @brief  初始化ST7735S驱动芯片的TFT
 * @param  无
 * @retval 无
 * @note   需要在本文件中配置好 DISPLAY_DIRECTION
 */
void TFT_Init_ST7735S(void);

/**
 * @brief  初始化ST7735R驱动芯片的TFT
 * @param  无
 * @retval 无
 * @note   需要在本文件中配置好 DISPLAY_DIRECTION
 */
void TFT_Init_ST7735R(void);

/**
 * @brief  TFT通用初始化 (基于原TFT.c中的序列)
 * @param  无
 * @retval 无
 * @note   这是一个备用的初始化序列，可能适用于特定的ST7735变种或配置
 */
void TFT_Init_Generic(void);

#endif
