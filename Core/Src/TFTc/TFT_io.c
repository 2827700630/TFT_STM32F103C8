/*
 * @file    tft_io.c
 * @brief   TFT底层IO驱动实现，硬件抽象层
 */
#include "TFTh/TFT_io.h"
#include <stdint.h>

// 全局变量存储 SPI 句柄和 DMA 使用标志 (非 static)
SPI_HandleTypeDef *TFT_spi = NULL;
static uint8_t tft_use_dma = 0;				  // DMA 使用标志设为 static，仅在此文件内部访问
static volatile uint8_t tft_spi_dma_busy = 0; // DMA 传输忙标志设为 static

//----------------- TFT 控制引脚函数实现 (TFT HAL specific) -----------------
// 这些函数实现了硬件抽象层，将平台相关的GPIO操作封装起来，以便于在不同平台上使用相同的TFT驱动代码。
// 目前只实现了 STM32 系列的 GPIO 操作，其他平台可以根据需要自行实现。
void TFT_Pin_RES_Set(uint8_t state)
{
	HAL_GPIO_WritePin(TFT_RES_GPIO_Port, TFT_RES_Pin, (GPIO_PinState)state);
}

void TFT_Pin_DC_Set(uint8_t state)
{
	HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, (GPIO_PinState)state);
}

void TFT_Pin_CS_Set(uint8_t state)
{
	HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, (GPIO_PinState)state);
}

void TFT_Pin_BLK_Set(uint8_t state)
{
	HAL_GPIO_WritePin(TFT_BL_GPIO_Port, TFT_BL_Pin, (GPIO_PinState)state);
}
//--------------------------------------------------------------------------

/*
 * @brief  初始化 TFT IO 层
 * @param  hspi 指向 SPI_HandleTypeDef 结构的指针
 * @retval 无
 */
void TFT_IO_Init(SPI_HandleTypeDef *hspi)
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
	tft_spi_dma_busy = 0; // 初始化 DMA 忙标志
}

/*
 * @brief  等待上一次 SPI DMA 传输完成
 * @param  无
 * @retval 无
 */
static void TFT_Wait_DMA_Done(void) // 设为 static，仅在此文件内使用
{
	// 只有在 DMA 模式下才需要等待
	if (tft_use_dma)
	{
		while (tft_spi_dma_busy)
		{
			// 可以添加超时退出机制或让出 CPU (如 __WFI())
			// 简单的忙等待，适用于多数场景
		}
	}
}

/*
 * @brief  向TFT写入8位数据
 * @param  data 要写入的8位数据
 * @retval 无
 */
void TFT_Write_Data8(uint8_t data)
{
	if (TFT_spi == NULL)
		return;			 // 检查句柄
	TFT_Wait_DMA_Done(); // 等待上一次 DMA 完成
	TFT_Pin_DC_Set(1);	 // 选择数据模式 (高)
	TFT_Pin_CS_Set(0);	 // 片选选中 (低)

	if (tft_use_dma)
	{
		tft_spi_dma_busy = 1; // 设置忙标志
		HAL_SPI_Transmit_DMA(TFT_spi, &data, 1);
		// 注意：CS 在回调中拉高
	}
	else
	{
		HAL_SPI_Transmit(TFT_spi, &data, 1, 100); // 增加超时时间
		TFT_Pin_CS_Set(1);						  // 非DMA模式下，传输完成后立即拉高CS (高)
	}
}

/*
 * @brief  向TFT写入16位数据
 * @param  data 要写入的16位数据
 * @retval 无
 */
void TFT_Write_Data16(uint16_t data)
{
	if (TFT_spi == NULL)
		return;			   // 检查句柄
	TFT_Wait_DMA_Done();   // 等待上一次 DMA 完成
	uint8_t buffer[2];	   // 局部变量
	buffer[0] = data >> 8; // 高字节
	buffer[1] = data;	   // 低字节

	TFT_Pin_DC_Set(1); // 选择数据模式 (高)
	TFT_Pin_CS_Set(0); // 片选选中 (低)

	if (tft_use_dma)
	{
		tft_spi_dma_busy = 1; // 设置忙标志
		HAL_SPI_Transmit_DMA(TFT_spi, buffer, 2);
		// 注意：CS 在回调中拉高
	}
	else
	{
		HAL_SPI_Transmit(TFT_spi, buffer, 2, 100); // 增加超时时间
		TFT_Pin_CS_Set(1);						   // 非DMA模式下，传输完成后立即拉高CS (高)
	}
}

/*
 * @brief  向TFT写入命令
 * @param  command 要写入的命令字节
 * @retval 无
 */
void TFT_Write_Command(uint8_t command)
{
	if (TFT_spi == NULL)
		return;			 // 检查句柄
	TFT_Wait_DMA_Done(); // 等待上一次 DMA 完成
	TFT_Pin_DC_Set(0);	 // 选择命令模式 (低)
	TFT_Pin_CS_Set(0);	 // 片选选中 (低)

	HAL_SPI_Transmit(TFT_spi, &command, 1, 100); // 增加超时时间
	TFT_Pin_CS_Set(1);							 // 命令发送完成后立即拉高CS (高)
}

/*
 * @brief  设置TFT显示区域 (窗口)
 * @param  x_start 列起始坐标 (0-based)
 * @param  y_start 行起始坐标 (0-based)
 * @param  x_end   列结束坐标 (0-based, inclusive)
 * @param  y_end   行结束坐标 (0-based, inclusive)
 * @retval 无
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
	TFT_Write_Command(0x2b);	   // 行地址设置
	TFT_Write_Data16(y_start + 3); // 注意: 偏移量可能需调整
	TFT_Write_Data16(y_end + 3);
#elif DISPLAY_DIRECTION == 3 // ST7735S 旋转270度 (红板)
	TFT_Write_Command(0x2a);	   // 列地址设置
	TFT_Write_Data16(x_start + 3); // 注意: 偏移量可能需调整
	TFT_Write_Data16(x_end + 3);
	TFT_Write_Command(0x2b); // 行地址设置
	TFT_Write_Data16(y_start + 2);
	TFT_Write_Data16(y_end + 2);
#elif DISPLAY_DIRECTION == 4 // ST7735R 正常 (0度, 黑板)
	TFT_Write_Command(0x2a);	   // 列地址设置
	TFT_Write_Data16(x_start + 2); // R型黑板偏移示例
	TFT_Write_Data16(x_end + 2);
	TFT_Write_Command(0x2b);	   // 行地址设置
	TFT_Write_Data16(y_start + 1); // R型黑板偏移示例
	TFT_Write_Data16(y_end + 1);
#elif DISPLAY_DIRECTION == 5 // ST7735R 旋转180度 (黑板)
	TFT_Write_Command(0x2a);	   // 列地址设置
	TFT_Write_Data16(x_start + 2); // R型黑板偏移示例
	TFT_Write_Data16(x_end + 2);
	TFT_Write_Command(0x2b);	   // 行地址设置
	TFT_Write_Data16(y_start + 1); // R型黑板偏移示例
	TFT_Write_Data16(y_end + 1);
#else
#error "Invalid DISPLAY_DIRECTION defined in TFT_io.h" // 引用 TFT_io.h
#endif

	TFT_Write_Command(0x2c); // 准备写入显存 (Memory Write)
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
		// 仅在DMA模式下，传输完成后拉高CS引脚并清除忙标志
		if (tft_use_dma)
		{
			TFT_Pin_CS_Set(1);	  // 拉高片选 (高)
			tft_spi_dma_busy = 0; // 清除忙标志
		}
	}
	// 如果系统中有其他 SPI 设备使用 DMA，需要添加相应的处理逻辑
	// else if (hspi == &other_spi_handle) { ... }
}
