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

	TFT_Pin_BLK_Set(htft, 1); // 打开背光 (高)
	HAL_Delay(100);

	// 1. 软件复位 (Software Reset)
	TFT_Write_Command(htft, 0x01);
	HAL_Delay(150);

	// 2. 退出睡眠模式 (Sleep out)
	TFT_Write_Command(htft, 0x11); // 退出睡眠模式
	HAL_Delay(255);

	// 3. 设置帧率控制 (Frame Rate Control)
	TFT_Write_Command(htft, 0xB1); // FRMCTR1 (In normal mode/ Full colors)
	TFT_Write_Data8(htft, 0x01);   // Frame rate = fosc/(1*2+40) * (LINE+2C+2D)
	TFT_Write_Data8(htft, 0x2C);
	TFT_Write_Data8(htft, 0x2D);

	// 4. 设置帧率控制 (空闲模式) (Frame Rate Control 2)
	TFT_Write_Command(htft, 0xB2); // FRMCTR2 (In Idle mode/ 8-colors)
	TFT_Write_Data8(htft, 0x01);
	TFT_Write_Data8(htft, 0x2C);
	TFT_Write_Data8(htft, 0x2D);

	// 5. 设置帧率控制 (部分模式) (Frame Rate control 3)
	TFT_Write_Command(htft, 0xB3); // FRMCTR3 (In Partial mode/ full colors)
	TFT_Write_Data8(htft, 0x01);
	TFT_Write_Data8(htft, 0x2C);
	TFT_Write_Data8(htft, 0x2D);
	TFT_Write_Data8(htft, 0x01);
	TFT_Write_Data8(htft, 0x2C);
	TFT_Write_Data8(htft, 0x2D);

	// 6. 设置显示反转控制 (Display Inversion Control)
	TFT_Write_Command(htft, 0xB4); // INVCTR
	TFT_Write_Data8(htft, 0x07);   // 列倒装

	// 7. 设置电源控制1 (Power Control 1)
	TFT_Write_Command(htft, 0xC0); // PWCTR1
	TFT_Write_Data8(htft, 0xA2);   // -4.6V
	TFT_Write_Data8(htft, 0x02);   // AVCC=VCIx2, VGH=VCIx7, VGL=-VCIx4
	TFT_Write_Data8(htft, 0x84);   // Opamp current small, Boost frequency

	// 8. 设置电源控制2 (Power Control 2)
	TFT_Write_Command(htft, 0xC1); // PWCTR2
	TFT_Write_Data8(htft, 0xC5);   // VGH = VCI * 2.5, VGL = -VCI * 2.5

	// 9. 设置电源控制3 (Power Control 3)
	TFT_Write_Command(htft, 0xC2); // PWCTR3 (In Normal mode/ Full colors)
	TFT_Write_Data8(htft, 0x0A);   // Opamp current small, Boost frequency
	TFT_Write_Data8(htft, 0x00);   // Boost frequency

	// 10. 设置电源控制4 (Power Control 4)
	TFT_Write_Command(htft, 0xC3); // PWCTR4 (In Idle mode/ 8-colors)
	TFT_Write_Data8(htft, 0x8A);   // Opamp current small, Boost frequency
	TFT_Write_Data8(htft, 0x2A);   // Boost frequency

	// 11. 设置电源控制5 (Power Control 5)
	TFT_Write_Command(htft, 0xC4); // PWCTR5 (In Partial mode/ full colors)
	TFT_Write_Data8(htft, 0x8A);   // Opamp current small, Boost frequency
	TFT_Write_Data8(htft, 0xEE);   // Boost frequency

	// 12. 设置VCOM控制 (VCOM Control 1)
	TFT_Write_Command(htft, 0xC5); // VMCTR1
	TFT_Write_Data8(htft, 0x0E);   // VCOMH = 4.025V, VCOML = -1.5V

	// 13. 设置屏幕旋转方向
	TFT_Set_Direction(htft, htft->display_direction);

	// 14. 设置像素格式 (Pixel Format Set)
	TFT_Write_Command(htft, 0x3A); // COLMOD
	TFT_Write_Data8(htft, 0x05);   // 16位像素格式 (RGB565)

	// 15. 伽马校准
	// Gamma (positive polarity)
	TFT_Write_Command(htft, 0xE0); // GMCTRP1
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
	// Negative Gamma Correction
	TFT_Write_Command(htft, 0xE1); // GMCTRN1
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

	// 16. 开启正常显示模式 (Normal Display Mode ON)
	TFT_Write_Command(htft, 0x13); // NORON
	HAL_Delay(10);

	// 17. 打开显示
	TFT_Write_Command(htft, 0x29); // Display ON
	 HAL_Delay(20);
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
	// MADCTL 位标志: MY MX MV ML RGB MH - -
	// MY: 行地址顺序 (0=从上到下, 1=从下到上)
	// MX: 列地址顺序 (0=从左到右, 1=从右到左)
	// MV: 行/列交换 (0=正常, 1=交换)
	// ML: 垂直刷新顺序 (0=从上到下, 1=从下到上)
	// RGB: 颜色顺序 (0=RGB, 1=BGR)
	// MH: 水平刷新顺序 (0=从左到右, 1=从右到左)
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


void LCD_ST7789V3_Init(void)
{

	LCD_RES_Clr(); // 复位
	HAL_Delay(100);
	LCD_RES_Set();
	HAL_Delay(100);

	LCD_BLK_Set(); // 打开背光
	HAL_Delay(100);

	LCD_WR_REG(0x01); // Software Reset
	HAL_Delay(120);

	//************* Start Initial Sequence **********//
	LCD_WR_REG(0x11); // Sleep out
	HAL_Delay(120);	  // Delay 120ms
	//************* Start Initial Sequence **********//
	LCD_WR_REG(0x36);
	if (USE_HORIZONTAL == 0)
		LCD_WR_DATA8(0x00);
	else if (USE_HORIZONTAL == 1)
		LCD_WR_DATA8(0xC0);
	else if (USE_HORIZONTAL == 2)
		LCD_WR_DATA8(0x70);
	else
		LCD_WR_DATA8(0xA0);

	LCD_WR_REG(0x3A);
	LCD_WR_DATA8(0x05);

	LCD_WR_REG(0xB2);
	LCD_WR_DATA8(0x0c);
	LCD_WR_DATA8(0x0c);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x33);
	LCD_WR_DATA8(0x33);

	LCD_WR_REG(0xB7);
	LCD_WR_DATA8(0x72);

	LCD_WR_REG(0xBB);
	LCD_WR_DATA8(0x3d); // 2b

	LCD_WR_REG(0xC0);
	LCD_WR_DATA8(0x2C);

	LCD_WR_REG(0xC2);
	LCD_WR_DATA8(0x01);

	LCD_WR_REG(0xC3);
	LCD_WR_DATA8(0x19);

	LCD_WR_REG(0xC4);
	LCD_WR_DATA8(0x20); // VDV, 0x20:0v

	LCD_WR_REG(0xC6);
	LCD_WR_DATA8(0x0f); // 0x13:60Hz

	LCD_WR_REG(0xD0);
	LCD_WR_DATA8(0xA4);
	LCD_WR_DATA8(0xA1);

	LCD_WR_REG(0xD6);
	LCD_WR_DATA8(0xA1); // sleep in后，gate输出为GND

	LCD_WR_REG(0xE0);
	LCD_WR_DATA8(0xD0);
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x0D);
	LCD_WR_DATA8(0x11);
	LCD_WR_DATA8(0x13);
	LCD_WR_DATA8(0x2B);
	LCD_WR_DATA8(0x3F);
	LCD_WR_DATA8(0x54);
	LCD_WR_DATA8(0x4C);
	LCD_WR_DATA8(0x18);
	LCD_WR_DATA8(0x0D);
	LCD_WR_DATA8(0x0B);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x23);
	/* 电压设置 */
	LCD_WR_REG(0xE1);
	LCD_WR_DATA8(0xD0);
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x11);
	LCD_WR_DATA8(0x13);
	LCD_WR_DATA8(0x2C);
	LCD_WR_DATA8(0x3F);
	LCD_WR_DATA8(0x44);
	LCD_WR_DATA8(0x51);
	LCD_WR_DATA8(0x2F);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x20);
	LCD_WR_DATA8(0x23);
	/* 显示开 */
	LCD_WR_REG(0x21);
	LCD_WR_REG(0x29);
}