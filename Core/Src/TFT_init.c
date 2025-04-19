/*
 * @file    tft_init.c
 * @brief   TFT底层驱动初始化及通信函数
 */
#include "TFT/TFT_init.h"
#include <stdint.h>

// 静态全局变量存储 SPI 句柄和 DMA 使用标志
static SPI_HandleTypeDef *TFT_spi = NULL;
static uint8_t tft_use_dma = 0;

/*
 * @brief  向TFT写入8位数据 (优化CS控制, 支持DMA)
 * @param  data 要写入的8位数据
 * @retval 无
 */
void TFT_Write_Data8(uint8_t data)
{
	TFT_DC_HIGH(); // 选择数据模式
	TFT_CS_LOW();  // 片选选中

	if (tft_use_dma)
	{
		HAL_SPI_Transmit_DMA(TFT_spi, &data, 1);
	}
	else
	{
		HAL_SPI_Transmit(TFT_spi, &data, 1, 10); // 使用较短的超时时间
		TFT_CS_HIGH();							 // 非DMA模式下，传输完成后立即拉高CS
	}
}

/*
 * @brief  向TFT写入16位数据 (优化CS控制和合并传输, 支持DMA)
 * @param  data 要写入的16位数据
 * @retval 无
 * @note   高字节先发送
 */
void TFT_Write_Data16(uint16_t data)
{
	static uint8_t buffer[2]; // 使用静态buffer避免栈问题，但引入非重入性
	buffer[0] = data >> 8;	  // 高字节
	buffer[1] = data;		  // 低字节

	TFT_DC_HIGH(); // 选择数据模式
	TFT_CS_LOW();  // 片选选中

	if (tft_use_dma)
	{
		HAL_SPI_Transmit_DMA(TFT_spi, buffer, 2);
	}
	else
	{
		HAL_SPI_Transmit(TFT_spi, buffer, 2, 10); // 使用较短的超时时间
		TFT_CS_HIGH();							  // 非DMA模式下，传输完成后立即拉高CS
	}
}

/*
 * @brief  向TFT写入命令 (优化CS控制, 支持DMA)
 * @param  command 要写入的命令字节
 * @retval 无
 */
void TFT_Write_Command(uint8_t command)
{
	TFT_DC_LOW(); // 选择命令模式
	TFT_CS_LOW(); // 片选选中

	HAL_SPI_Transmit(TFT_spi, &command, 1, 10); // 使用较短的超时时间
	TFT_CS_HIGH();								// 命令发送完成后立即拉高CS
}

/*
 * @brief  设置TFT显示区域 (窗口)
 * @param  x_start 列起始坐标 (0-based)
 * @param  y_start 行起始坐标 (0-based)
 * @param  x_end   列结束坐标 (0-based, inclusive)
 * @param  y_end   行结束坐标 (0-based, inclusive)
 * @retval 无
 * @note   此函数根据 TFT_init.h 中定义的 DISPLAY_DIRECTION 宏调整坐标偏移。
 *         不同的TFT控制器或屏幕可能需要不同的偏移量。请根据实际显示效果微调。
 */
void TFT_Set_Address(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end)
{
	// 根据不同的显示方向调整坐标偏移量
	// 这些偏移量是为了适配屏幕的物理像素排列和驱动IC的寻址方式
#if DISPLAY_DIRECTION == 0	 // ST7735S 正常 (0度, 红板)
	TFT_Write_Command(0x2a); // 列地址设置
	TFT_Write_Data16(x_start + 2);
	TFT_Write_Data16(x_end + 2);
	TFT_Write_Command(0x2b); // 行地址设置
	TFT_Write_Data16(y_start + 1);
	TFT_Write_Data16(y_end + 1);
#elif DISPLAY_DIRECTION == 1 // ST7735S 旋转90度 (红板)
	TFT_Write_Command(0x2a); // 列地址设置
	TFT_Write_Data16(x_start + 1);
	TFT_Write_Data16(x_end + 1);
	TFT_Write_Command(0x2b); // 行地址设置
	TFT_Write_Data16(y_start + 2);
	TFT_Write_Data16(y_end + 2);
#elif DISPLAY_DIRECTION == 2 // ST7735S 旋转180度 (红板)
	TFT_Write_Command(0x2a); // 列地址设置
	TFT_Write_Data16(x_start + 2);
	TFT_Write_Data16(x_end + 2);
	TFT_Write_Command(0x2b); // 行地址设置
	TFT_Write_Data16(y_start + 3); // 注意: 偏移量可能需调整
	TFT_Write_Data16(y_end + 3);
#elif DISPLAY_DIRECTION == 3 // ST7735S 旋转270度 (红板)
	TFT_Write_Command(0x2a); // 列地址设置
	TFT_Write_Data16(x_start + 3); // 注意: 偏移量可能需调整
	TFT_Write_Data16(x_end + 3);
	TFT_Write_Command(0x2b); // 行地址设置
	TFT_Write_Data16(y_start + 2);
	TFT_Write_Data16(y_end + 2);
#elif DISPLAY_DIRECTION == 4 // ST7735R 正常 (0度, 黑板)
	TFT_Write_Command(0x2a);	   // 列地址设置
	TFT_Write_Data16(x_start + 2); // R型黑板偏移示例
	TFT_Write_Data16(x_end + 2);
	TFT_Write_Command(0x2b); // 行地址设置
	TFT_Write_Data16(y_start + 1); // R型黑板偏移示例
	TFT_Write_Data16(y_end + 1);
#elif DISPLAY_DIRECTION == 5 // ST7735R 旋转180度 (黑板)
	TFT_Write_Command(0x2a);	   // 列地址设置
	TFT_Write_Data16(x_start + 2); // R型黑板偏移示例
	TFT_Write_Data16(x_end + 2);
	TFT_Write_Command(0x2b); // 行地址设置
	TFT_Write_Data16(y_start + 1); // R型黑板偏移示例
	TFT_Write_Data16(y_end + 1);
#else
#error "Invalid DISPLAY_DIRECTION defined in TFT_init.h"
#endif

	TFT_Write_Command(0x2c); // 准备写入显存 (Memory Write)
}

/*
 * @brief  通用 ST7735 初始化序列
 * @param  hspi 指向 SPI_HandleTypeDef 结构的指针
 * @retval 无
 * @note   此函数基于常见的 ST7735 初始化流程，并根据 DISPLAY_DIRECTION 调整 MADCTL。
 *         适用于 ST7735S 和 ST7735R 变种。
 *         伽马值等其他参数可能需要根据具体屏幕微调。
 */
void TFT_Init_ST7735(SPI_HandleTypeDef *hspi)
{
	TFT_spi = hspi; // 存储 SPI 句柄
	if (TFT_spi == NULL)
		return; // 如果句柄无效则返回

	// 检查 SPI 是否配置了 DMA
	if (TFT_spi->hdmatx != NULL)
	{
		tft_use_dma = 1; // SPI 已配置 DMA
	}
	else
	{
		tft_use_dma = 0; // SPI 未配置 DMA
	}

	TFT_RES_LOW(); // 硬复位TFT
	HAL_Delay(100);
	TFT_RES_HIGH();
	HAL_Delay(100);

	TFT_BLK_HIGH(); // 打开背光
	HAL_Delay(100);

	// 1. 软件复位 (Software Reset)
	TFT_Write_Command(0x01);
	HAL_Delay(150); // 等待复位完成

	// 2. 退出睡眠模式 (Sleep out)
	TFT_Write_Command(0x11);
	HAL_Delay(255); // 等待退出睡眠完成，数据手册建议 > 120ms

	// 3. 设置帧率控制 (Frame Rate Control)
	TFT_Write_Command(0xB1); // FRMCTR1 (In normal mode/ Full colors)
	TFT_Write_Data8(0x01);	 // Rate = fosc/(1x2+40) * (LINE+2C+2D)
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
	TFT_Write_Data8(0x07);	 // No inversion

	// 7. 设置电源控制1 (Power Control 1)
	TFT_Write_Command(0xC0); // PWCTR1
	TFT_Write_Data8(0xA2);	 // -4.6V
	TFT_Write_Data8(0x02);	 // AVCC=VCIx2, VGH=VCIx7, VGL=-VCIx4
	TFT_Write_Data8(0x84);	 // Opamp current small, Boost frequency

	// 8. 设置电源控制2 (Power Control 2)
	TFT_Write_Command(0xC1); // PWCTR2
	TFT_Write_Data8(0xC5);	 // VGH = VCI * 2.5, VGL = -VCI * 2.5

	// 9. 设置电源控制3 (Power Control 3)
	TFT_Write_Command(0xC2); // PWCTR3 (In Normal mode/ Full colors)
	TFT_Write_Data8(0x0A);	 // Opamp current small, Boost frequency
	TFT_Write_Data8(0x00);	 // Boost frequency

	// 10. 设置电源控制4 (Power Control 4)
	TFT_Write_Command(0xC3); // PWCTR4 (In Idle mode/ 8-colors)
	TFT_Write_Data8(0x8A);	 // Opamp current small, Boost frequency
	TFT_Write_Data8(0x2A);	 // Boost frequency

	// 11. 设置电源控制5 (Power Control 5)
	TFT_Write_Command(0xC4); // PWCTR5 (In Partial mode/ full colors)
	TFT_Write_Data8(0x8A);	 // Opamp current small, Boost frequency
	TFT_Write_Data8(0xEE);	 // Boost frequency

	// 12. 设置VCOM控制 (VCOM Control 1)
	TFT_Write_Command(0xC5); // VMCTR1
	TFT_Write_Data8(0x0E);	 // VCOMH = 4.025V, VCOML = -1.5V

	// 13. 设置内存访问控制 (Memory Access Control - MADCTL)
	TFT_Write_Command(0x36);
	// MADCTL Bits: MY MX MV ML RGB MH - -
	// MY: Row Address Order (0=T->B, 1=B->T)
	// MX: Col Address Order (0=L->R, 1=R->L)
	// MV: Row/Col Exchange (0=Normal, 1=Exchange)
	// ML: Vertical Refresh Order (0=T->B, 1=B->T)
	// RGB: Color Order (0=RGB, 1=BGR)
	// MH: Horizontal Refresh Order (0=L->R, 1=R->L)
#if DISPLAY_DIRECTION == 0	 // ST7735S 正常 (0度, 红板, BGR)
	TFT_Write_Data8(0x08);	 // MY=0, MX=0, MV=0, ML=0, RGB=1, MH=0
#elif DISPLAY_DIRECTION == 1 // ST7735S 旋转90度 (红板, BGR)
	TFT_Write_Data8(0x68); // MY=0, MX=1, MV=1, ML=0, RGB=1, MH=0
#elif DISPLAY_DIRECTION == 2 // ST7735S 旋转180度 (红板, BGR)
	TFT_Write_Data8(0xC8); // MY=1, MX=1, MV=0, ML=0, RGB=1, MH=0
#elif DISPLAY_DIRECTION == 3 // ST7735S 旋转270度 (红板, BGR)
	TFT_Write_Data8(0xA8); // MY=1, MX=0, MV=1, ML=0, RGB=1, MH=0
#elif DISPLAY_DIRECTION == 4 // ST7735R 正常 (0度, 黑板, RGB)
	TFT_Write_Data8(0x00); // MY=0, MX=0, MV=0, ML=0, RGB=0, MH=0
#elif DISPLAY_DIRECTION == 5 // ST7735R 旋转180度 (黑板, RGB)
	TFT_Write_Data8(0xC0); // MY=1, MX=1, MV=0, ML=0, RGB=0, MH=0
	// 注意: ST7735R 的 90/270 度旋转可能需要不同的 MADCTL 值，这里未列出
	// 例如 90度: 0x60 (MY=0,MX=1,MV=1,RGB=0)
	// 例如 270度: 0xA0 (MY=1,MX=0,MV=1,RGB=0)
#else
#error "Invalid DISPLAY_DIRECTION defined in TFT_init.h"
#endif

	// 14. 设置像素格式 (Pixel Format Set)
	TFT_Write_Command(0x3A); // COLMOD
	TFT_Write_Data8(0x05);	 // 16位像素格式 (RGB565)

	// 15. 设置伽马校正 (Gamma Correction) - 使用ST7735R的伽马值作为通用示例
	// Positive Gamma Correction
	TFT_Write_Command(0xE0); // GMCTRP1
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
	TFT_Write_Command(0xE1); // GMCTRN1
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

	// 16. 开启正常显示模式 (Normal Display Mode ON)
	TFT_Write_Command(0x13); // NORON
	HAL_Delay(10);

	// 17. 开启显示 (Display ON)
	TFT_Write_Command(0x29); // DISPON
	HAL_Delay(100);

	// 初始化完成后，可以设置一次全屏地址，但这通常由绘图函数处理
	// TFT_Set_Address(0, 0, TFT_WIDTH - 1, TFT_HEIGHT - 1); // 假设定义了 TFT_WIDTH 和 TFT_HEIGHT
}

/**
 * @brief  SPI DMA传输完成回调函数
 * @note   此函数会被HAL库自动调用，不需要用户手动调用。
 *         确保在 stm32f1xx_it.c 或其他地方没有重复定义。
 *         在DMA模式下，用于在传输完成后自动拉高CS。
 * @param  hspi: SPI句柄指针
 * @retval None
 */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	// 检查是否为TFT使用的SPI实例
	if (hspi == TFT_spi)
	{
		// 仅在DMA模式下，传输完成后拉高CS引脚
		if (tft_use_dma)
		{
			TFT_CS_HIGH(); // 拉高片选，表示数据传输结束
		}
	}
}
