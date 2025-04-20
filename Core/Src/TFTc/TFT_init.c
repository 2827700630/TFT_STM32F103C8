/**
 * @file    TFT_init.c
 * @brief   TFT屏幕初始化函数
 * @details 支持ST7735S、ST7735R等型号屏幕的初始化。
 */
#include "TFTh/TFT_init.h"
#include "TFTh/TFT_io.h"
#include <stdint.h>

// 设置屏幕方向
static void TFT_Set_Direction(TFT_HandleTypeDef *htft, uint8_t direction);

/**
 * @brief  ST7735S初始化，支持多实例
 * @param  htft TFT句柄指针
 * @retval 无
 * @note   适用于多数红绿蓝背景的0.96/1.44/1.8寸TFT，此函数简化了初始化流程
 */
void TFT_Init_ST7735S(TFT_HandleTypeDef *htft)
{
	// 初始化IO层
	TFT_IO_Init(htft);

	// 硬复位TFT
	TFT_Pin_RES_Set(htft, 0);
	HAL_Delay(100);
	TFT_Pin_RES_Set(htft, 1);
	HAL_Delay(100);

	// 开始发送初始化命令序列
	TFT_Write_Command(htft, 0x11); // Sleep Exit
	HAL_Delay(120);

	// Memory Data Access Control (MADCTL)
	TFT_Write_Command(htft, 0x36);
	TFT_Write_Data8(htft, 0xC0); // 行扫描顺序(MH,ML):从下到上,从左到右，RGB顺序

	// Interface Pixel Format (COLMOD)
	TFT_Write_Command(htft, 0x3A);
	TFT_Write_Data8(htft, 0x05); // 16-bit RGB565 格式

	// Frame Rate Control (In normal mode/ Full colors)
	TFT_Write_Command(htft, 0xB1);
	TFT_Write_Data8(htft, 0x01); // Frame rate = fosc/(1*2+40) * (LINE+2C+2D)
	TFT_Write_Data8(htft, 0x2C);
	TFT_Write_Data8(htft, 0x2D);

	// Frame Rate Control (In Idle mode/ 8-colors)
	TFT_Write_Command(htft, 0xB2);
	TFT_Write_Data8(htft, 0x01);
	TFT_Write_Data8(htft, 0x2C);
	TFT_Write_Data8(htft, 0x2D);

	// Frame Rate Control (In Partial mode/ full colors)
	TFT_Write_Command(htft, 0xB3);
	TFT_Write_Data8(htft, 0x01);
	TFT_Write_Data8(htft, 0x2C);
	TFT_Write_Data8(htft, 0x2D);
	TFT_Write_Data8(htft, 0x01);
	TFT_Write_Data8(htft, 0x2C);
	TFT_Write_Data8(htft, 0x2D);

	// Display Inversion Control (INVCTR)
	TFT_Write_Command(htft, 0xB4);
	TFT_Write_Data8(htft, 0x07); // 列倒装

	// Power Control 1 (PWCTR1)
	TFT_Write_Command(htft, 0xC0);
	TFT_Write_Data8(htft, 0xA2);
	TFT_Write_Data8(htft, 0x02);
	TFT_Write_Data8(htft, 0x84);

	// Power Control 2 (PWCTR2)
	TFT_Write_Command(htft, 0xC1);
	TFT_Write_Data8(htft, 0xC5);

	// Power Control 3 (PWCTR3) in Normal mode
	TFT_Write_Command(htft, 0xC2);
	TFT_Write_Data8(htft, 0x0A);
	TFT_Write_Data8(htft, 0x00);

	// Power Control 4 (PWCTR4) in Idle mode
	TFT_Write_Command(htft, 0xC3);
	TFT_Write_Data8(htft, 0x8A);
	TFT_Write_Data8(htft, 0x2A);

	// Power Control 5 (PWCTR5) in Partial mode
	TFT_Write_Command(htft, 0xC4);
	TFT_Write_Data8(htft, 0x8A);
	TFT_Write_Data8(htft, 0xEE);

	// VCOM Control 1 (VMCTR1)
	TFT_Write_Command(htft, 0xC5);
	TFT_Write_Data8(htft, 0x0E);

	// 显示反转关闭
	TFT_Write_Command(htft, 0x20);

	// 伽马校准
	TFT_Write_Command(htft, 0xE0); // Gamma (positive polarity)
	TFT_Write_Data8(htft, 0x0F);
	TFT_Write_Data8(htft, 0x1A);
	TFT_Write_Data8(htft, 0x0F);
	TFT_Write_Data8(htft, 0x18);
	TFT_Write_Data8(htft, 0x2F);
	TFT_Write_Data8(htft, 0x28);
	TFT_Write_Data8(htft, 0x20);
	TFT_Write_Data8(htft, 0x22);
	TFT_Write_Data8(htft, 0x1F);
	TFT_Write_Data8(htft, 0x1B);
	TFT_Write_Data8(htft, 0x23);
	TFT_Write_Data8(htft, 0x37);
	TFT_Write_Data8(htft, 0x00);
	TFT_Write_Data8(htft, 0x07);
	TFT_Write_Data8(htft, 0x02);
	TFT_Write_Data8(htft, 0x10);

	TFT_Write_Command(htft, 0xE1); // Gamma (negative polarity)
	TFT_Write_Data8(htft, 0x0F);
	TFT_Write_Data8(htft, 0x1B);
	TFT_Write_Data8(htft, 0x0F);
	TFT_Write_Data8(htft, 0x17);
	TFT_Write_Data8(htft, 0x33);
	TFT_Write_Data8(htft, 0x2C);
	TFT_Write_Data8(htft, 0x29);
	TFT_Write_Data8(htft, 0x2E);
	TFT_Write_Data8(htft, 0x30);
	TFT_Write_Data8(htft, 0x30);
	TFT_Write_Data8(htft, 0x39);
	TFT_Write_Data8(htft, 0x3F);
	TFT_Write_Data8(htft, 0x00);
	TFT_Write_Data8(htft, 0x07);
	TFT_Write_Data8(htft, 0x03);
	TFT_Write_Data8(htft, 0x10);

	// 设置屏幕旋转方向
	TFT_Set_Direction(htft, htft->display_direction);

	// 打开显示
	TFT_Write_Command(htft, 0x29); // Display ON
	TFT_Pin_BLK_Set(htft, 1);	   // 打开背光
	// HAL_Delay(20);
}

/**
 * @brief  设置屏幕方向 (根据MADCTL寄存器设置)
 * @param  htft TFT句柄指针
 * @param  direction 方向 (0-3)
 *         0: 0度旋转
 *         1: 顺时针90度
 *         2: 顺时针180度
 *         3: 顺时针270度
 * @retval 无
 * @note   各种TFT屏幕的MADCTL设置可能不同，请根据数据手册调整
 *         ST7735S红板和ST7735R黑板的MADCTL设置和颜色顺序不同
 */
static void TFT_Set_Direction(TFT_HandleTypeDef *htft, uint8_t direction)
{
	TFT_Write_Command(htft, 0x36); // MADCTL - Memory Data Access Control

	// 注意: 设置取决于屏幕型号，以下设置适用于普通ST7735S
	switch (direction)
	{
	case 0:							 // 0度旋转
		TFT_Write_Data8(htft, 0xC0); // MY=1, MX=1, MV=0, RGB
		break;
	case 1:							 // 90度旋转
		TFT_Write_Data8(htft, 0xA0); // MY=1, MX=0, MV=1, RGB
		break;
	case 2:							 // 180度旋转
		TFT_Write_Data8(htft, 0x00); // MY=0, MX=0, MV=0, RGB
		break;
	case 3:							 // 270度旋转
		TFT_Write_Data8(htft, 0x60); // MY=0, MX=1, MV=1, RGB
		break;
	default:						 // 默认0度旋转
		TFT_Write_Data8(htft, 0xC0); // MY=1, MX=1, MV=0, RGB
		break;
	}
}
