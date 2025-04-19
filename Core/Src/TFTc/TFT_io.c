/*
 * @file    tft_io.c
 * @brief   TFT底层IO驱动实现，硬件抽象层 (HAL)
 * @details 封装了与硬件相关的操作，如 GPIO 控制和 SPI 通信 (阻塞/DMA)。
 *          提供了一个发送缓冲区以提高连续数据传输的效率。
 */
#include "TFTh/TFT_io.h"
#include <stdint.h>

/*
内存限制说明：
全屏帧缓冲对于资源有限的 MCU (如 STM32F103C8T6 只有 20KB SRAM) 通常是不可行的。
例如：
- 240x320 屏幕 @ 16位色 (RGB565) 需要 240 * 320 * 2 = 153,600 字节 (150 KB)
- 128x160 屏幕 @ 16位色 (RGB565) 需要 128 * 160 * 2 = 40,960 字节 (40 KB)
因此，本驱动采用较小的发送缓冲区结合 DMA (如果可用) 来优化性能。
*/

// --- 内部变量 ---
static uint8_t tft_tx_buffer[TFT_BUFFER_SIZE]; // SPI 发送缓冲区 (Transmit Buffer)
static uint16_t buffer_write_index = 0;		   // 当前缓冲区写入位置索引

// 指向 SPI 句柄的指针，由 TFT_IO_Init 初始化
static SPI_HandleTypeDef *tft_spi_handle = NULL;
// DMA 使用标志，由 TFT_IO_Init 根据 SPI 配置确定
static uint8_t is_dma_enabled = 0;
// DMA 传输忙标志 (volatile 因为可能在中断中被修改)
static volatile uint8_t is_dma_transfer_active = 0;

// --- 内部辅助函数声明 ---
static void TFT_Wait_DMA_Transfer_Complete(void); // 等待 DMA 传输完成

//----------------- TFT 控制引脚函数实现 (依赖于具体硬件平台 HAL) -----------------
// 这些函数通过调用 HAL 库函数来控制 TFT 的 GPIO 引脚。
// 如果更换硬件平台，需要修改这些函数的实现以适配新的 GPIO 控制方式。

/**
 * @brief  控制复位引脚 (RES/RST)
 * @param  level: 0=拉低 (复位激活), 1=拉高 (复位释放)
 */
void TFT_Pin_RES_Set(uint8_t level)
{
	HAL_GPIO_WritePin(TFT_RES_GPIO_Port, TFT_RES_Pin, (GPIO_PinState)level);
}

/**
 * @brief  控制数据/命令选择引脚 (DC/RS)
 * @param  level: 0=命令模式 (低), 1=数据模式 (高)
 */
void TFT_Pin_DC_Set(uint8_t level)
{
	HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, (GPIO_PinState)level);
}

/**
 * @brief  控制片选引脚 (CS)
 * @param  level: 0=选中 (低), 1=取消选中 (高)
 */
void TFT_Pin_CS_Set(uint8_t level)
{
	HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, (GPIO_PinState)level);
}

/**
 * @brief  控制背光引脚 (BLK/BL)
 * @param  level: 0=关闭 (低), 1=打开 (高)
 */
void TFT_Pin_BLK_Set(uint8_t level)
{
	// 注意：某些屏幕背光可能是低电平点亮，需根据实际硬件调整
	HAL_GPIO_WritePin(TFT_BL_GPIO_Port, TFT_BL_Pin, (GPIO_PinState)level);
}
//--------------------------------------------------------------------------

//----------------- TFT SPI 通信与缓冲区管理函数实现 -----------------

/**
 * @brief  通过 SPI 发送指定缓冲区的数据到 TFT
 * @param  data_buffer 要发送的数据缓冲区指针
 * @param  length      要发送的数据长度（字节数）
 * @param  wait_completion 是否等待传输完成 (1=等待, 0=不等待，仅 DMA 模式有效)
 * @retval 无
 */
void TFT_SPI_Send(uint8_t *data_buffer, uint16_t length, uint8_t wait_completion)
{
	if (tft_spi_handle == NULL || length == 0 || data_buffer == NULL)
		return; // 参数检查

	TFT_Wait_DMA_Transfer_Complete(); // 确保上一次 DMA 传输 (如果有) 已完成

	TFT_Pin_DC_Set(1); // 设置为数据模式
	TFT_Pin_CS_Set(0); // 拉低片选，开始传输

	if (is_dma_enabled) // 如果启用了 DMA
	{
		is_dma_transfer_active = 1; // 设置 DMA 忙标志
		// 启动 SPI DMA 传输
		HAL_SPI_Transmit_DMA(tft_spi_handle, data_buffer, length);
		// 如果需要等待完成，则在此处等待
		if (wait_completion)
		{
			TFT_Wait_DMA_Transfer_Complete(); // 等待 DMA 完成
			TFT_Pin_CS_Set(1); // DMA 完成后手动拉高片选
		}
		// 如果不需要等待 (wait_completion = 0)，CS 将在 DMA 完成回调函数 HAL_SPI_TxCpltCallback 中拉高
	}
	else // 如果未使用 DMA，使用阻塞式 SPI 传输
	{
		HAL_SPI_Transmit(tft_spi_handle, data_buffer, length, HAL_MAX_DELAY); // 使用最大超时时间
		TFT_Pin_CS_Set(1); // 阻塞传输完成后立即拉高片选
	}
}

/**
 * @brief  向发送缓冲区写入 16 位数据 (通常是颜色值)
 * @param  data 要写入的 16 位数据
 * @retval 无
 * @note   数据以大端模式 (高字节在前) 写入缓冲区。
 *         如果缓冲区空间不足以写入 2 字节，会自动刷新缓冲区 (非阻塞)。
 */
void TFT_Buffer_Write16(uint16_t data)
{
	// 检查缓冲区剩余空间是否足够存放 16 位数据 (2字节)
	if (buffer_write_index >= TFT_BUFFER_SIZE - 1)
	{
		TFT_Flush_Buffer(0); // 缓冲区满，刷新缓冲区，不等待完成
	}

	// 将 16 位数据按大端序写入缓冲区
	tft_tx_buffer[buffer_write_index++] = (data >> 8) & 0xFF; // 高字节
	tft_tx_buffer[buffer_write_index++] = data & 0xFF;		 // 低字节
}

/**
 * @brief  将发送缓冲区中剩余的数据发送到 TFT
 * @param  wait_completion 是否等待传输完成 (1=等待, 0=不等待，仅 DMA 模式有效)
 * @retval 无
 */
void TFT_Flush_Buffer(uint8_t wait_completion)
{
	if (buffer_write_index == 0)
		return; // 缓冲区为空，无需刷新

	// 调用 TFT_SPI_Send 发送缓冲区中的数据
	TFT_SPI_Send(tft_tx_buffer, buffer_write_index, wait_completion);

	buffer_write_index = 0; // 发送后重置缓冲区索引
}

/**
 * @brief  重置发送缓冲区（清空索引，不发送数据）
 * @retval 无
 */
void TFT_Reset_Buffer(void)
{
	buffer_write_index = 0;
}

/**
 * @brief  初始化 TFT IO 层，配置 SPI 句柄和 DMA 使用状态
 * @param  hspi_ptr 指向 SPI_HandleTypeDef 结构的指针
 * @retval 无
 */
void TFT_IO_Init(SPI_HandleTypeDef *hspi_ptr)
{
	tft_spi_handle = hspi_ptr; // 保存 SPI 句柄
	if (tft_spi_handle == NULL)
	{
		// 可以在这里添加错误处理，例如断言或日志记录
		return;
	}

	// 检查关联的 SPI 句柄是否配置了 DMA 发送通道
	if (tft_spi_handle->hdmatx != NULL)
	{
		is_dma_enabled = 1; // SPI 已配置 DMA 发送
	}
	else
	{
		is_dma_enabled = 0; // SPI 未配置 DMA 发送
	}
	is_dma_transfer_active = 0; // 初始化 DMA 传输状态标志
	buffer_write_index = 0;		// 初始化缓冲区索引
}

/**
 * @brief  等待上一次 SPI DMA 传输完成 (内部辅助函数)
 * @param  无
 * @retval 无
 * @note   仅在 DMA 模式下且 DMA 传输正在进行时阻塞。
 */
static void TFT_Wait_DMA_Transfer_Complete(void)
{
	// 仅当 DMA 被启用且当前有活动的 DMA 传输时才需要等待
	if (is_dma_enabled)
	{
		while (is_dma_transfer_active)
		{
			// 忙等待。在 RTOS 环境下，可以考虑使用信号量或事件标志来避免忙等，提高 CPU 效率。
			// 例如: osSemaphoreWait(spiDmaSemaphore, osWaitForever);
			// 或者使用 __WFI() 指令让 CPU 进入低功耗模式等待中断。
		}
	}
	// 如果 DMA 未启用或没有活动的传输，此函数立即返回。
}

/**
 * @brief  向 TFT 写入 8 位数据 (主要用于初始化序列中的参数)
 * @param  data 要写入的 8 位数据
 * @retval 无
 * @note   此函数总是使用阻塞式 SPI 传输，并假定在发送前 DC 引脚已设为数据模式。
 *         通常在发送命令后调用，用于发送命令参数。
 */
void TFT_Write_Data8(uint8_t data)
{
	if (tft_spi_handle == NULL) return;

	TFT_Wait_DMA_Transfer_Complete(); // 确保之前的 DMA 操作完成
	TFT_Pin_DC_Set(1);	 // 确保是数据模式
	TFT_Pin_CS_Set(0);	 // 片选选中

	// 使用阻塞式发送单个字节
	HAL_SPI_Transmit(tft_spi_handle, &data, 1, HAL_MAX_DELAY);

	TFT_Pin_CS_Set(1);	 // 传输完成后拉高 CS
}

/**
 * @brief  向 TFT 写入 16 位数据 (阻塞方式)
 * @param  data 要写入的 16 位数据
 * @retval 无
 * @note   此函数总是使用阻塞式 SPI 传输。
 *         主要用于绘制单个点或少量数据，不经过发送缓冲区。
 */
void TFT_Write_Data16(uint16_t data)
{
	if (tft_spi_handle == NULL) return;

	uint8_t spi_data[2];
	spi_data[0] = (data >> 8) & 0xFF; // 高字节 (大端)
	spi_data[1] = data & 0xFF;		// 低字节

	TFT_Wait_DMA_Transfer_Complete(); // 确保之前的 DMA 操作完成
	TFT_Pin_DC_Set(1);	 // 数据模式
	TFT_Pin_CS_Set(0);	 // 片选选中

	// 使用阻塞式发送 2 个字节
	HAL_SPI_Transmit(tft_spi_handle, spi_data, 2, HAL_MAX_DELAY);

	TFT_Pin_CS_Set(1);	 // 传输完成后拉高 CS
}

/**
 * @brief  向 TFT 发送命令
 * @param  command 要发送的命令字节
 * @retval 无
 * @note   发送命令前会先刷新缓冲区 (阻塞等待)。
 *         命令本身使用阻塞式 SPI 传输。
 */
void TFT_Write_Command(uint8_t command)
{
	if (tft_spi_handle == NULL) return;

	// 发送命令前，确保缓冲区中的所有数据已发送完成
	TFT_Flush_Buffer(1); // 等待缓冲区刷新完成

	// 不需要再次调用 TFT_Wait_DMA_Transfer_Complete()，因为 Flush_Buffer(1) 已经等待了

	TFT_Pin_DC_Set(0);	 // 设置为命令模式
	TFT_Pin_CS_Set(0);	 // 片选选中

	// 使用阻塞式发送命令字节
	HAL_SPI_Transmit(tft_spi_handle, &command, 1, HAL_MAX_DELAY);

	TFT_Pin_CS_Set(1);	 // 命令发送完成后立即拉高 CS
}

/**
 * @brief  设置 TFT 显示窗口区域 (GRAM 访问窗口)
 * @param  x_start 列起始坐标 (0-based)
 * @param  y_start 行起始坐标 (0-based)
 * @param  x_end   列结束坐标 (0-based, inclusive)
 * @param  y_end   行结束坐标 (0-based, inclusive)
 * @retval 无
 * @note   设置地址前会先刷新缓冲区 (阻塞等待)。
 *         坐标会根据 `TFT_config.h` 中定义的 `DISPLAY_DIRECTION` 自动添加偏移量，
 *         以适应不同屏幕型号和旋转方向下的物理像素地址。
 */
void TFT_Set_Address(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end)
{
	// 设置地址前，确保缓冲区中的所有数据已发送完成
	TFT_Flush_Buffer(1); // 等待缓冲区刷新完成

	// --- 设置列地址 (Column Address Set, CASET, 0x2A) ---
	TFT_Write_Command(0x2A);
	// 根据显示方向应用 X 轴偏移量
#if (DISPLAY_DIRECTION == 0) || (DISPLAY_DIRECTION == 2) // 0度或180度 (ST7735S 红板)
	TFT_Write_Data16(x_start + TFT_X_OFFSET);
	TFT_Write_Data16(x_end + TFT_X_OFFSET);
#elif (DISPLAY_DIRECTION == 1) || (DISPLAY_DIRECTION == 3) // 90度或270度 (ST7735S 红板)
	TFT_Write_Data16(x_start + TFT_Y_OFFSET); // 注意：旋转后 X/Y 偏移可能互换或不同
	TFT_Write_Data16(x_end + TFT_Y_OFFSET);
#elif (DISPLAY_DIRECTION == 4) || (DISPLAY_DIRECTION == 5) // ST7735R 黑板 (假设偏移与红板不同)
	TFT_Write_Data16(x_start + TFT_X_OFFSET); // 使用配置的偏移
	TFT_Write_Data16(x_end + TFT_X_OFFSET);
#else
	#error "Invalid DISPLAY_DIRECTION defined in TFT_config.h"
#endif

	// --- 设置行地址 (Row Address Set, RASET, 0x2B) ---
	TFT_Write_Command(0x2B);
	// 根据显示方向应用 Y 轴偏移量
#if (DISPLAY_DIRECTION == 0) || (DISPLAY_DIRECTION == 2) // 0度或180度 (ST7735S 红板)
	TFT_Write_Data16(y_start + TFT_Y_OFFSET);
	TFT_Write_Data16(y_end + TFT_Y_OFFSET);
#elif (DISPLAY_DIRECTION == 1) || (DISPLAY_DIRECTION == 3) // 90度或270度 (ST7735S 红板)
	TFT_Write_Data16(y_start + TFT_X_OFFSET); // 注意：旋转后 X/Y 偏移可能互换或不同
	TFT_Write_Data16(y_end + TFT_X_OFFSET);
#elif (DISPLAY_DIRECTION == 4) || (DISPLAY_DIRECTION == 5) // ST7735R 黑板
	TFT_Write_Data16(y_start + TFT_Y_OFFSET); // 使用配置的偏移
	TFT_Write_Data16(y_end + TFT_Y_OFFSET);
#endif

	// --- 发送写 GRAM 命令 (Memory Write, 0x2C) ---
	// 后续发送的数据将被写入由此窗口定义的 GRAM 区域
	TFT_Write_Command(0x2C);
}

//----------------- HAL SPI DMA 回调函数 -----------------

/**
 * @brief  SPI DMA 发送完成回调函数
 * @note   此函数由 STM32 HAL 库在 SPI DMA 发送完成后自动调用。
 *         用户通常不需要直接调用此函数。
 *         确保此函数定义唯一，没有在 stm32f1xx_it.c 等其他地方重复定义。
 * @param  hspi: 触发回调的 SPI 句柄指针
 * @retval None
 */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	// 检查是否是 TFT 使用的 SPI 实例触发的回调
	if (hspi == tft_spi_handle)
	{
		// 仅在 DMA 模式下，传输完成后需要处理
		if (is_dma_enabled)
		{
			// 1. 拉高片选引脚 (CS)，结束本次 SPI 通信
			TFT_Pin_CS_Set(1);
			// 2. 清除 DMA 传输忙标志
			is_dma_transfer_active = 0;
			// 3. (可选) 在 RTOS 环境下，可以在这里释放信号量或设置事件标志，
			//    以唤醒等待 DMA 完成的任务。
			//    例如: osSemaphoreRelease(spiDmaSemaphore);
		}
	}
	// 如果系统中有其他设备也使用 SPI DMA，需要添加对其他 SPI 句柄的判断和处理
	// else if (hspi == &other_spi_handle) { /* 处理其他设备的 DMA 完成事件 */ }
}
