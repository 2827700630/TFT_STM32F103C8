/*
 * @file    tft_init.c
 * @brief   TFT底层驱动初始化及通信函数
 */
#include "TFT/TFT_init.h" // Keep include name for now, or rename file later
#include "spi.h"
#include <stdint.h>

/******************************************************************************
 * @brief  通过SPI总线发送一个字节的数据
 * @param  data 要发送的字节
 * @retval 无
 ******************************************************************************/
void TFT_Write_Bus(uint8_t data)
{
	TFT_CS_LOW(); // 片选选中
	HAL_SPI_Transmit(&hspi1, &data, 1, 1000); // 使用HAL库发送SPI数据，超时时间1000ms
	// 注意：DMA方式在此处可能效率不高，因为它适用于大量数据的连续传输。
	// 若要使用DMA，需要配置SPI的DMA通道，并可能需要缓冲区管理。
	// HAL_SPI_Transmit_DMA(&hspi1, &data, 1);
	TFT_CS_HIGH(); // 片选取消
}

/******************************************************************************
 * @brief  向TFT写入8位数据
 * @param  data 要写入的8位数据
 * @retval 无
 ******************************************************************************/
void TFT_Write_Data8(uint8_t data)
{
	TFT_DC_HIGH(); // 选择数据模式
	TFT_Write_Bus(data);
}

/******************************************************************************
 * @brief  向TFT写入16位数据
 * @param  data 要写入的16位数据
 * @retval 无
 * @note   高字节先发送
 ******************************************************************************/
void TFT_Write_Data16(uint16_t data)
{
	TFT_DC_HIGH(); // 选择数据模式
	TFT_Write_Bus(data >> 8); // 发送高8位
	TFT_Write_Bus(data);      // 发送低8位
}

/******************************************************************************
 * @brief  向TFT写入命令
 * @param  command 要写入的命令字节
 * @retval 无
 ******************************************************************************/
void TFT_Write_Command(uint8_t command)
{
	TFT_DC_LOW(); // 选择命令模式
	TFT_Write_Bus(command);
	// 通常在发送命令后，需要切换回数据模式以发送参数或像素数据
	// 但为了函数单一职责，这里不自动切换，由调用者根据需要切换
	// TFT_DC_HIGH();
}

/******************************************************************************
 * @brief  设置TFT显示区域 (窗口)
 * @param  x_start 列起始坐标 (0-based)
 * @param  y_start 行起始坐标 (0-based)
 * @param  x_end   列结束坐标 (0-based, inclusive)
 * @param  y_end   行结束坐标 (0-based, inclusive)
 * @retval 无
 * @note   此函数根据 lcd_init.h 中定义的 DISPLAY_DIRECTION 宏调整坐标偏移。
 *         不同的TFT控制器或屏幕可能需要不同的偏移量。
 ******************************************************************************/
void TFT_Set_Address(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end)
{
	// 根据不同的显示方向调整坐标偏移量
	// 这些偏移量通常是为了适配屏幕的物理像素排列和驱动IC的寻址方式
#if DISPLAY_DIRECTION == 0 // ST7735S 上下颠倒
	TFT_Write_Command(0x2a); // 列地址设置 (Column Address Set)
	TFT_Write_Data16(x_start + 2);
	TFT_Write_Data16(x_end + 2);
	TFT_Write_Command(0x2b); // 行地址设置 (Row Address Set)
	TFT_Write_Data16(y_start + 1);
	TFT_Write_Data16(y_end + 1);
#elif DISPLAY_DIRECTION == 1 // ST7735S 正常
	TFT_Write_Command(0x2a); // 列地址设置
	TFT_Write_Data16(x_start + 2);
	TFT_Write_Data16(x_end + 2);
	TFT_Write_Command(0x2b); // 行地址设置
	TFT_Write_Data16(y_start + 3); // 注意：这里的偏移量可能需要根据具体屏幕调整
	TFT_Write_Data16(y_end + 3);
#elif DISPLAY_DIRECTION == 2 // ST7735S 逆时针90度
	TFT_Write_Command(0x2a); // 列地址设置
	TFT_Write_Data16(x_start + 1);
	TFT_Write_Data16(x_end + 1);
	TFT_Write_Command(0x2b); // 行地址设置
	TFT_Write_Data16(y_start + 2);
	TFT_Write_Data16(y_end + 2);
#elif DISPLAY_DIRECTION == 3 // ST7735S 顺时针90度
	TFT_Write_Command(0x2a); // 列地址设置
	TFT_Write_Data16(x_start + 3);
	TFT_Write_Data16(x_end + 3);
	TFT_Write_Command(0x2b); // 行地址设置
	TFT_Write_Data16(y_start + 2);
	TFT_Write_Data16(y_end + 2);
#elif DISPLAY_DIRECTION == 4 // ST7735R 正常
	TFT_Write_Command(0x2a); // 列地址设置
	TFT_Write_Data16(x_start); // ST7735R 可能不需要偏移
	TFT_Write_Data16(x_end);
	TFT_Write_Command(0x2b); // 行地址设置
	TFT_Write_Data16(y_start);
	TFT_Write_Data16(y_end);
#elif DISPLAY_DIRECTION == 5 // ST7735R 上下颠倒
	TFT_Write_Command(0x2a); // 列地址设置
	TFT_Write_Data16(x_start); // ST7735R 可能不需要偏移
	TFT_Write_Data16(x_end);
	TFT_Write_Command(0x2b); // 行地址设置
	TFT_Write_Data16(y_start);
	TFT_Write_Data16(y_end);
#else
#error "Invalid DISPLAY_DIRECTION defined in lcd_init.h"
#endif

	TFT_Write_Command(0x2c); // 准备写入显存 (Memory Write)
	TFT_DC_HIGH(); // 切换到数据模式，准备接收像素数据
}

/******************************************************************************
 * @brief  初始化ST7735R驱动芯片的TFT
 * @param  无
 * @retval 无
 * @note   需要在 lcd_init.h 中配置好 DISPLAY_DIRECTION 为 4 或 5
 ******************************************************************************/
void TFT_Init_ST7735R(void)
{
	TFT_RES_LOW(); // 硬复位TFT
	HAL_Delay(100);
	TFT_RES_HIGH();
	HAL_Delay(100);

	TFT_BLK_HIGH(); // 打开背光
	HAL_Delay(100);

	// 1. 退出睡眠模式 (Sleep out)
	TFT_Write_Command(0x11);
	HAL_Delay(120); // 需要至少120ms的延迟

	// 2. 帧率控制 (Frame Rate Control)
	// Normal mode
	TFT_Write_Command(0xB1);
	TFT_Write_Data8(0x01);
	TFT_Write_Data8(0x2C);
	TFT_Write_Data8(0x2D);
	// Idle mode
	TFT_Write_Command(0xB2);
	TFT_Write_Data8(0x01);
	TFT_Write_Data8(0x2C);
	TFT_Write_Data8(0x2D);
	// Partial mode
	TFT_Write_Command(0xB3);
	TFT_Write_Data8(0x01);
	TFT_Write_Data8(0x2C);
	TFT_Write_Data8(0x2D);
	TFT_Write_Data8(0x01);
	TFT_Write_Data8(0x2C);
	TFT_Write_Data8(0x2D);

	// 3. 列反转控制 (Column inversion)
	TFT_Write_Command(0xB4);
	TFT_Write_Data8(0x07);

	// 4. 电源控制序列 (Power Sequence)
	TFT_Write_Command(0xC0); // Power control 1
	TFT_Write_Data8(0xA2);
	TFT_Write_Data8(0x02);
	TFT_Write_Data8(0x84);
	TFT_Write_Command(0xC1); // Power control 2
	TFT_Write_Data8(0xC5);
	TFT_Write_Command(0xC2); // Power control 3
	TFT_Write_Data8(0x0A);
	TFT_Write_Data8(0x00);
	TFT_Write_Command(0xC3); // Power control 4
	TFT_Write_Data8(0x8A);
	TFT_Write_Data8(0x2A);
	TFT_Write_Command(0xC4); // Power control 5
	TFT_Write_Data8(0x8A);
	TFT_Write_Data8(0xEE);

	// 5. VCOM 控制 (VCOM Control 1)
	TFT_Write_Command(0xC5);
	TFT_Write_Data8(0x0E);

	// 6. 内存访问控制 (Memory Access Control)
	TFT_Write_Command(0x36);
#if DISPLAY_DIRECTION == 4 // ST7735R 正常
	TFT_Write_Data8(0xC0); // MY=1, MX=1, MV=0, ML=0, RGB=0, MH=0 -> C0
	// 解释: 行地址顺序(MY)=Bottom to Top, 列地址顺序(MX)=Right to Left, 行/列交换(MV)=Normal
#elif DISPLAY_DIRECTION == 5 // ST7735R 上下颠倒
	TFT_Write_Data8(0x00); // MY=0, MX=0, MV=0, ML=0, RGB=0, MH=0 -> 00
	// 解释: 行地址顺序(MY)=Top to Bottom, 列地址顺序(MX)=Left to Right, 行/列交换(MV)=Normal
#else
#warning "DISPLAY_DIRECTION in lcd_init.h is not set for ST7735R (should be 4 or 5)"
	TFT_Write_Data8(0xC0); // 默认使用正常方向
#endif

	// 7. 伽马校正序列 (Gamma Sequence)
	// Positive Gamma Correction
	TFT_Write_Command(0xe0);
	TFT_Write_Data8(0x0f);
	TFT_Write_Data8(0x1a);
	TFT_Write_Data8(0x0f);
	TFT_Write_Data8(0x18);
	TFT_Write_Data8(0x2f);
	TFT_Write_Data8(0x28);
	TFT_Write_Data8(0x20);
	TFT_Write_Data8(0x22);
	TFT_Write_Data8(0x1f);
	TFT_Write_Data8(0x1b);
	TFT_Write_Data8(0x23);
	TFT_Write_Data8(0x37);
	TFT_Write_Data8(0x00);
	TFT_Write_Data8(0x07);
	TFT_Write_Data8(0x02);
	TFT_Write_Data8(0x10);
	// Negative Gamma Correction
	TFT_Write_Command(0xe1);
	TFT_Write_Data8(0x0f);
	TFT_Write_Data8(0x1b);
	TFT_Write_Data8(0x0f);
	TFT_Write_Data8(0x17);
	TFT_Write_Data8(0x33);
	TFT_Write_Data8(0x2c);
	TFT_Write_Data8(0x29);
	TFT_Write_Data8(0x2e);
	TFT_Write_Data8(0x30);
	TFT_Write_Data8(0x30);
	TFT_Write_Data8(0x39);
	TFT_Write_Data8(0x3f);
	TFT_Write_Data8(0x00);
	TFT_Write_Data8(0x07);
	TFT_Write_Data8(0x03);
	TFT_Write_Data8(0x10);

	// 8. 设置列地址范围 (Column Address Set) - 可选，通常由 TFT_Set_Address 设置
	// TFT_Write_Command(0x2a);
	// TFT_Write_Data8(0x00);
	// TFT_Write_Data8(0x00);
	// TFT_Write_Data8(0x00);
	// TFT_Write_Data8(0x7f); // 128 columns

	// 9. 设置行地址范围 (Row Address Set) - 可选
	// TFT_Write_Command(0x2b);
	// TFT_Write_Data8(0x00);
	// TFT_Write_Data8(0x00);
	// TFT_Write_Data8(0x00);
	// TFT_Write_Data8(0x9f); // 160 rows (for 1.8 inch, 1.44 inch is 128)

	// 10. 使能测试命令 (Enable test command) - 通常不需要
	// TFT_Write_Command(0xF0);
	// TFT_Write_Data8(0x01);
	// 11. 禁用RAM省电模式 (Disable ram power save mode) - 通常不需要
	// TFT_Write_Command(0xF6);
	// TFT_Write_Data8(0x00);

	// 12. 设置像素格式 (Pixel Format Set)
	TFT_Write_Command(0x3A);
	TFT_Write_Data8(0x05); // 选择 16位/像素 (RGB565)

	// 13. 开启显示 (Display ON)
	TFT_Write_Command(0x29);
	HAL_Delay(10); // 短暂延时确保显示开启
}

/******************************************************************************
 * @brief  初始化ST7735S驱动芯片的TFT
 * @param  无
 * @retval 无
 * @note   需要在 lcd_init.h 中配置好 DISPLAY_DIRECTION 为 0, 1, 2, 或 3
 ******************************************************************************/
void TFT_Init_ST7735S(void)
{
	TFT_RES_LOW(); // 硬复位TFT
	HAL_Delay(100);
	TFT_RES_HIGH();
	HAL_Delay(100);

	TFT_BLK_HIGH(); // 打开背光
	HAL_Delay(100);

	// 1. 退出睡眠模式 (Sleep out)
	TFT_Write_Command(0x11);
	HAL_Delay(120); // 需要至少120ms的延迟

	// 2. 帧率控制 (Frame Rate Control)
	// Normal mode
	TFT_Write_Command(0xB1);
	TFT_Write_Data8(0x02); // 根据数据手册或实际效果调整
	TFT_Write_Data8(0x35);
	TFT_Write_Data8(0x36);
	// Idle mode
	TFT_Write_Command(0xB2);
	TFT_Write_Data8(0x02);
	TFT_Write_Data8(0x35);
	TFT_Write_Data8(0x36);
	// Partial mode
	TFT_Write_Command(0xB3);
	TFT_Write_Data8(0x02);
	TFT_Write_Data8(0x35);
	TFT_Write_Data8(0x36);
	TFT_Write_Data8(0x02);
	TFT_Write_Data8(0x35);
	TFT_Write_Data8(0x36);

	// 3. 点反转控制 (Dot inversion)
	TFT_Write_Command(0xB4);
	TFT_Write_Data8(0x03); // 根据数据手册或实际效果调整

	// 4. 电源控制序列 (Power Sequence)
	TFT_Write_Command(0xC0);
	TFT_Write_Data8(0xA2);
	TFT_Write_Data8(0x02);
	TFT_Write_Data8(0x84);
	TFT_Write_Command(0xC1);
	TFT_Write_Data8(0xC5);
	TFT_Write_Command(0xC2);
	TFT_Write_Data8(0x0D);
	TFT_Write_Data8(0x00);
	TFT_Write_Command(0xC3);
	TFT_Write_Data8(0x8D);
	TFT_Write_Data8(0x2A);
	TFT_Write_Command(0xC4);
	TFT_Write_Data8(0x8D);
	TFT_Write_Data8(0xEE);

	// 5. VCOM 控制 (VCOM Control 1)
	TFT_Write_Command(0xC5);
	TFT_Write_Data8(0x0a); // 根据数据手册或实际效果调整

	// 6. 内存访问控制 (Memory Access Control)
	TFT_Write_Command(0x36);
#if DISPLAY_DIRECTION == 0 // ST7735S 上下颠倒
	TFT_Write_Data8(0x08); // MY=0, MX=0, MV=0, ML=0, RGB=1, MH=0 -> 08 (BGR color filter panel)
	// 解释: 行=Top to Bottom, 列=Left to Right, 行/列=Normal, 颜色=BGR
#elif DISPLAY_DIRECTION == 1 // ST7735S 正常
	TFT_Write_Data8(0xC8); // MY=1, MX=1, MV=0, ML=0, RGB=1, MH=0 -> C8 (BGR color filter panel)
	// 解释: 行=Bottom to Top, 列=Right to Left, 行/列=Normal, 颜色=BGR
#elif DISPLAY_DIRECTION == 2 // ST7735S 逆时针90度
	TFT_Write_Data8(0x78); // MY=0, MX=1, MV=1, ML=0, RGB=1, MH=0 -> 78 (BGR color filter panel)
	// 解释: 行=Top to Bottom, 列=Right to Left, 行/列=交换, 颜色=BGR
#elif DISPLAY_DIRECTION == 3 // ST7735S 顺时针90度
	TFT_Write_Data8(0xA8); // MY=1, MX=0, MV=1, ML=0, RGB=1, MH=0 -> A8 (BGR color filter panel)
	// 解释: 行=Bottom to Top, 列=Left to Right, 行/列=交换, 颜色=BGR
#else
#warning "Invalid DISPLAY_DIRECTION defined in lcd_init.h for ST7735S (should be 0, 1, 2, or 3)"
	TFT_Write_Data8(0xC8); // 默认使用正常方向
#endif

	// 7. 伽马校正序列 (Gamma Sequence)
	// Positive Gamma Correction
	TFT_Write_Command(0XE0);
	TFT_Write_Data8(0x12);
	TFT_Write_Data8(0x1C);
	TFT_Write_Data8(0x10);
	TFT_Write_Data8(0x18);
	TFT_Write_Data8(0x33);
	TFT_Write_Data8(0x2C);
	TFT_Write_Data8(0x25);
	TFT_Write_Data8(0x28);
	TFT_Write_Data8(0x28);
	TFT_Write_Data8(0x27);
	TFT_Write_Data8(0x2F);
	TFT_Write_Data8(0x3C);
	TFT_Write_Data8(0x00);
	TFT_Write_Data8(0x03);
	TFT_Write_Data8(0x03);
	TFT_Write_Data8(0x10);
	// Negative Gamma Correction
	TFT_Write_Command(0XE1);
	TFT_Write_Data8(0x12);
	TFT_Write_Data8(0x1C);
	TFT_Write_Data8(0x10);
	TFT_Write_Data8(0x18);
	TFT_Write_Data8(0x2D);
	TFT_Write_Data8(0x28);
	TFT_Write_Data8(0x23);
	TFT_Write_Data8(0x28);
	TFT_Write_Data8(0x28);
	TFT_Write_Data8(0x26);
	TFT_Write_Data8(0x2F);
	TFT_Write_Data8(0x3B);
	TFT_Write_Data8(0x00);
	TFT_Write_Data8(0x03);
	TFT_Write_Data8(0x03);
	TFT_Write_Data8(0x10);

	// 8. 设置像素格式 (Pixel Format Set)
	TFT_Write_Command(0x3A);
	TFT_Write_Data8(0x05); // 选择 16位/像素 (RGB565)

	// 9. 开启显示 (Display ON)
	TFT_Write_Command(0x29);
	HAL_Delay(10); // 短暂延时确保显示开启
}

/*
 * @brief  TFT初始化 (根据数据手册阅读，测试用)
 * @param  无
 * @retval 无
 * @note   这是一个备用的初始化序列，可能适用于特定的ST7735变种或配置。
 *         请根据实际测试效果选择合适的初始化函数。
 *         此函数未考虑 DISPLAY_DIRECTION 设置。
*/
void TFT_Init_Generic(void)
{
	// 1. 软件复位 (Software Reset)
	TFT_Write_Command(0x01);
	HAL_Delay(150); // 等待复位完成

	// 2. 退出睡眠模式 (Sleep out)
	TFT_Write_Command(0x11);
	HAL_Delay(255); // 等待退出睡眠完成，数据手册建议 > 120ms

	// 3. 设置帧率控制 (Frame Rate Control)
	TFT_Write_Command(0xB1); // FRMCTR1 (In normal mode/ Full colors)
	TFT_Write_Data8(0x01); // Rate = fosc/(1x2+40) * (LINE+2C+2D)
	TFT_Write_Data8(0x2C);
	TFT_Write_Data8(0x2D);

	// 4. 设置帧率控制 (空闲模式) (Frame Rate Control 2)
	TFT_Write_Command(0xB2); // FRMCTR2 (In Idle mode/ 8-colors)
	TFT_Write_Data8(0x01);
	TFT_Write_Data8(0x2C);
	TFT_Write_Data8(0x2D);

	// 5. 设置帧率控制 (部分模式) (Frame Rate control 3)
	TFT_Write_Command(0xB3); // FRMCTR3 (In Partial mode/ full colors)
	TFT_Write_Data8(0x01);
	TFT_Write_Data8(0x2C);
	TFT_Write_Data8(0x2D);
	TFT_Write_Data8(0x01);
	TFT_Write_Data8(0x2C);
	TFT_Write_Data8(0x2D);

	// 6. 设置显示反转控制 (Display Inversion Control)
	TFT_Write_Command(0xB4); // INVCTR
	TFT_Write_Data8(0x07); // No inversion

	// 7. 设置电源控制1 (Power Control 1)
	TFT_Write_Command(0xC0); // PWCTR1
	TFT_Write_Data8(0xA2); // -4.6V
	TFT_Write_Data8(0x02); // AVCC=VCIx2, VGH=VCIx7, VGL=-VCIx4
	TFT_Write_Data8(0x84); // Opamp current small, Boost frequency

	// 8. 设置电源控制2 (Power Control 2)
	TFT_Write_Command(0xC1); // PWCTR2
	TFT_Write_Data8(0xC5); // VGH = VCI * 2.5, VGL = -VCI * 2.5

	// 9. 设置电源控制3 (Power Control 3)
	TFT_Write_Command(0xC2); // PWCTR3 (In Normal mode/ Full colors)
	TFT_Write_Data8(0x0A); // Opamp current small, Boost frequency
	TFT_Write_Data8(0x00); // Boost frequency

	// 10. 设置电源控制4 (Power Control 4)
	TFT_Write_Command(0xC3); // PWCTR4 (In Idle mode/ 8-colors)
	TFT_Write_Data8(0x8A); // Opamp current small, Boost frequency
	TFT_Write_Data8(0x2A); // Boost frequency

	// 11. 设置电源控制5 (Power Control 5)
	TFT_Write_Command(0xC4); // PWCTR5 (In Partial mode/ full colors)
	TFT_Write_Data8(0x8A); // Opamp current small, Boost frequency
	TFT_Write_Data8(0xEE); // Boost frequency

	// 12. 设置VCOM控制 (VCOM Control 1)
	TFT_Write_Command(0xC5); // VMCTR1
	TFT_Write_Data8(0x0E); // VCOMH = 4.025V, VCOML = -1.5V

	// 13. 设置内存访问控制 (Memory Access Control)
	TFT_Write_Command(0x36); // MADCTL
	TFT_Write_Data8(0xC0); // 设置扫描方向等参数 (MY=1, MX=1, MV=0, ML=0, RGB=0, MH=0)
	                       // 行=Bottom to Top, 列=Right to Left, 行/列=Normal, 颜色=RGB

	// 14. 设置像素格式 (Pixel Format Set)
	TFT_Write_Command(0x3A); // COLMOD
	TFT_Write_Data8(0x05); // 16位像素格式 (RGB565)

	// 15. 设置伽马校正 (Gamma Correction)
	// Positive Gamma Correction
	TFT_Write_Command(0xE0); // GMCTRP1
	TFT_Write_Data8(0x02);
	TFT_Write_Data8(0x1C);
	TFT_Write_Data8(0x07);
	TFT_Write_Data8(0x12);
	TFT_Write_Data8(0x37);
	TFT_Write_Data8(0x32);
	TFT_Write_Data8(0x29);
	TFT_Write_Data8(0x2D);
	TFT_Write_Data8(0x29);
	TFT_Write_Data8(0x25);
	TFT_Write_Data8(0x2B);
	TFT_Write_Data8(0x39);
	TFT_Write_Data8(0x00);
	TFT_Write_Data8(0x01);
	TFT_Write_Data8(0x03);
	TFT_Write_Data8(0x10);
	// Negative Gamma Correction
	TFT_Write_Command(0xE1); // GMCTRN1
	TFT_Write_Data8(0x03);
	TFT_Write_Data8(0x1D);
	TFT_Write_Data8(0x07);
	TFT_Write_Data8(0x06);
	TFT_Write_Data8(0x2E);
	TFT_Write_Data8(0x2C);
	TFT_Write_Data8(0x29);
	TFT_Write_Data8(0x2D);
	TFT_Write_Data8(0x2E);
	TFT_Write_Data8(0x2E);
	TFT_Write_Data8(0x37);
	TFT_Write_Data8(0x3F);
	TFT_Write_Data8(0x00);
	TFT_Write_Data8(0x00);
	TFT_Write_Data8(0x02);
	TFT_Write_Data8(0x10);

	// 16. 开启正常显示模式 (Normal Display Mode ON)
	TFT_Write_Command(0x13); // NORON
	HAL_Delay(10);

	// 17. 开启显示 (Display ON)
	TFT_Write_Command(0x29); // DISPON
	HAL_Delay(100);
}
