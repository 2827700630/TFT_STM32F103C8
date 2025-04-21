/**
 * @file    tft_io.c
 * @brief   TFT底层IO驱动实现，硬件抽象层 (HAL)
 * @details 封装了与硬件相关的操作，如 GPIO 控制和 SPI 通信 (阻塞/DMA)。
 *          提供了一个发送缓冲区以提高连续数据传输的效率。
 */
#include "TFTh/TFT_io.h"
#include <stdint.h>
#include <stdlib.h> // 用于malloc/free

/**
内存限制说明：
全屏帧缓冲对于资源有限的 MCU (如 STM32F103C8T6 只有 20KB SRAM) 通常是不可行的。
例如：
- 240x320 屏幕 @ 16位色 (RGB565) 需要 240 * 320 * 2 = 153,600 字节 (150 KB)
- 128x160 屏幕 @ 16位色 (RGB565) 需要 128 * 160 * 2 = 40,960 字节 (40 KB)
因此，本驱动采用较小的发送缓冲区结合 DMA (如果可用) 来优化性能。
*/


static TFT_HandleTypeDef *g_tft_handles[MAX_TFT_DEVICES] = {NULL}; // TFT设备句柄数组

// --- 内部辅助函数声明 ---
static void TFT_Wait_DMA_Transfer_Complete(TFT_HandleTypeDef *htft); // 等待 DMA 传输完成
static void TFT_Register_Device(TFT_HandleTypeDef *htft);			 // 注册TFT设备

//----------------- TFT 初始化与配置函数实现 -----------------

/**
 * @brief  TFT实例初始化
 * @param  htft: TFT屏幕句柄指针
 * @param  hspi: SPI句柄指针
 * @param  cs_port: CS引脚端口
 * @param  cs_pin: CS引脚号
 * @retval 无
 */
void TFT_Init_Instance(TFT_HandleTypeDef *htft, SPI_HandleTypeDef *hspi,
					   GPIO_TypeDef *cs_port, uint16_t cs_pin)
{
	// 初始化基本参数
	htft->spi_handle = hspi;
	htft->cs_port = cs_port;
	htft->cs_pin = cs_pin;

	// 设置默认缓冲区大小
	htft->buffer_size = TFT_BUFFER_SIZE;
	htft->buffer_write_index = 0;
	htft->tx_buffer = NULL; // 后续会分配内存

	// 设置默认显示参数
	htft->display_direction = DISPLAY_DIRECTION;
	htft->x_offset = TFT_X_OFFSET;
	htft->y_offset = TFT_Y_OFFSET;
}

/**
 * @brief  配置TFT控制引脚
 * @param  htft: TFT屏幕句柄指针
 * @param  dc_port: DC引脚端口
 * @param  dc_pin: DC引脚号
 * @param  res_port: RES引脚端口
 * @param  res_pin: RES引脚号
 * @param  bl_port: BL引脚端口
 * @param  bl_pin: BL引脚号
 * @note   必须手动配置 GPIO 引脚模式和速度。否则无法显示。
 */
void TFT_Config_Pins(TFT_HandleTypeDef *htft,
					 GPIO_TypeDef *dc_port, uint16_t dc_pin,
					 GPIO_TypeDef *res_port, uint16_t res_pin,
					 GPIO_TypeDef *bl_port, uint16_t bl_pin)
{
	htft->dc_port = dc_port;
	htft->dc_pin = dc_pin;
	htft->res_port = res_port;
	htft->res_pin = res_pin;
	htft->bl_port = bl_port;
	htft->bl_pin = bl_pin;
}

/**
 * @brief  配置TFT显示参数
 * @param  htft: TFT屏幕句柄指针
 * @param  display_direction: 显示方向：0正置，1顺时针90度，2顺时针180度，3顺时针270度
 * @param  x_offset: X偏移量
 * @param  y_offset: Y偏移量
 * @retval 无
 */
void TFT_Config_Display(TFT_HandleTypeDef *htft,
						uint8_t display_direction,
						uint8_t x_offset, uint8_t y_offset)
{
	htft->display_direction = display_direction;
	htft->x_offset = x_offset;
	htft->y_offset = y_offset;
}

/**
 * @brief  注册TFT设备到全局设备列表
 * @param  htft TFT句柄指针
 * @retval 无
 * @note   内部函数，用于DMA回调
 */
static void TFT_Register_Device(TFT_HandleTypeDef *htft)
{
	for (int i = 0; i < MAX_TFT_DEVICES; i++)
	{
		if (g_tft_handles[i] == NULL || g_tft_handles[i]->spi_handle == htft->spi_handle)
		{
			g_tft_handles[i] = htft;
			break;
		}
	}
}

//----------------- TFT 控制引脚函数实现 (依赖于具体硬件平台 HAL) -----------------
// 这些函数通过调用 HAL 库函数来控制 TFT 的 GPIO 引脚。
// 如果更换硬件平台，需要修改这些函数的实现以适配新的 GPIO 控制方式。

/**
 * @brief  控制复位引脚 (RES/RST)
 * @param  htft: TFT屏幕句柄指针
 * @param  level: 0=拉低 (复位激活), 1=拉高 (复位释放)
 */
void TFT_Pin_RES_Set(TFT_HandleTypeDef *htft, uint8_t level)
{
#ifdef STM32HAL
	HAL_GPIO_WritePin(htft->res_port, htft->res_pin, (GPIO_PinState)level);
#elif defined(SOME_OTHER_PLATFORM)
	// 在此添加其他平台的 GPIO 控制代码
	// 例如: OtherPlatform_GPIOWrite(htft->res_pin, level);
#else
#error "No platform defined for GPIO control in TFT_config.h"
#endif
}

/**
 * @brief  控制数据/命令选择引脚 (DC/RS)
 * @param  htft: TFT屏幕句柄指针
 * @param  level: 0=命令模式 (低), 1=数据模式 (高)
 */
void TFT_Pin_DC_Set(TFT_HandleTypeDef *htft, uint8_t level)
{
#ifdef STM32HAL
	HAL_GPIO_WritePin(htft->dc_port, htft->dc_pin, (GPIO_PinState)level);
#elif defined(SOME_OTHER_PLATFORM)
	// 在此添加其他平台的 GPIO 控制代码
	// 例如: OtherPlatform_GPIOWrite(htft->dc_pin, level);
#else
#error "No platform defined for GPIO control in TFT_config.h"
#endif
}

/**
 * @brief  控制片选引脚 (CS)
 * @param  htft: TFT屏幕句柄指针
 * @param  level: 0=选中 (低), 1=取消选中 (高)
 */
void TFT_Pin_CS_Set(TFT_HandleTypeDef *htft, uint8_t level)
{
#ifdef STM32HAL
	HAL_GPIO_WritePin(htft->cs_port, htft->cs_pin, (GPIO_PinState)level);
#elif defined(SOME_OTHER_PLATFORM)
	// 在此添加其他平台的 GPIO 控制代码
	// 例如: OtherPlatform_GPIOWrite(htft->cs_pin, level);
#else
#error "No platform defined for GPIO control in TFT_config.h"
#endif
}

/**
 * @brief  控制背光引脚 (BLK/BL)
 * @param  htft: TFT屏幕句柄指针
 * @param  level: 0=关闭 (低), 1=打开 (高)
 */
void TFT_Pin_BLK_Set(TFT_HandleTypeDef *htft, uint8_t level)
{
	// 注意：某些屏幕背光可能是低电平点亮，需根据实际硬件调整
#ifdef STM32HAL
	HAL_GPIO_WritePin(htft->bl_port, htft->bl_pin, (GPIO_PinState)level);
#elif defined(SOME_OTHER_PLATFORM)
	// 在此添加其他平台的 GPIO 控制代码
	// 例如: OtherPlatform_GPIOWrite(htft->bl_pin, level);
#else
#error "No platform defined for GPIO control in TFT_config.h"
#endif
}
//--------------------------------------------------------------------------

//----------------- 平台相关的 SPI 传输函数实现 -----------------

/**
 * @brief  平台相关的阻塞式 SPI 发送函数
 * @param  spi_handle 平台相关的 SPI 句柄指针
 * @param  pData      要发送的数据缓冲区指针
 * @param  Size       要发送的数据大小 (字节)
 * @param  Timeout    超时时间 (平台相关定义)
 * @retval 平台相关的状态码 (例如 HAL_StatusTypeDef)
 */
int TFT_Platform_SPI_Transmit_Blocking(SPI_HandleTypeDef *spi_handle, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
#ifdef STM32HAL
	return HAL_SPI_Transmit(spi_handle, pData, Size, Timeout);
#elif defined(SOME_OTHER_PLATFORM)
	// 在此添加其他平台的阻塞式 SPI 发送代码
	// return OtherPlatform_SPISendBlocking(spi_handle, pData, Size, Timeout);
	return -1; // Placeholder error
#else
#error "No platform defined for SPI blocking transmit in TFT_config.h"
	return -1; // Return error code
#endif
}

/**
 * @brief  平台相关的启动 SPI DMA 发送函数
 * @param  spi_handle 平台相关的 SPI 句柄指针
 * @param  pData      要发送的数据缓冲区指针
 * @param  Size       要发送的数据大小 (字节)
 * @retval 平台相关的状态码 (例如 HAL_StatusTypeDef)
 * @note   此函数应启动传输但不等待完成。完成由回调处理。
 */
int TFT_Platform_SPI_Transmit_DMA_Start(SPI_HandleTypeDef *spi_handle, uint8_t *pData, uint16_t Size)
{
#ifdef STM32HAL
	return HAL_SPI_Transmit_DMA(spi_handle, pData, Size);
#elif defined(SOME_OTHER_PLATFORM)
	// 在此添加其他平台的启动 DMA SPI 发送代码
	// return OtherPlatform_SPISendDMAStart(spi_handle, pData, Size);
	return -1; // Placeholder error
#else
#error "No platform defined for SPI DMA transmit in TFT_config.h"
	return -1; // Return error code
#endif
}

//----------------- TFT SPI 通信与缓冲区管理函数实现 -----------------

/**
 * @brief  通过 SPI 发送指定缓冲区的数据到 TFT
 * @param  htft TFT句柄指针
 * @param  data_buffer 要发送的数据缓冲区指针
 * @param  length      要发送的数据长度（字节数）
 * @param  wait_completion 是否等待传输完成 (1=等待, 0=不等待，仅 DMA 模式有效)
 * @retval 无
 */
void TFT_SPI_Send(TFT_HandleTypeDef *htft, uint8_t *data_buffer, uint16_t length, uint8_t wait_completion)
{
	if (htft == NULL || htft->spi_handle == NULL || length == 0 || data_buffer == NULL)
		return; // 参数检查

	TFT_Wait_DMA_Transfer_Complete(htft); // 确保上一次 DMA 传输 (如果有) 已完成

	TFT_Pin_DC_Set(htft, 1); // 设置为数据模式
	TFT_Pin_CS_Set(htft, 0); // 拉低片选，开始传输

	if (htft->is_dma_enabled) // 如果启用了 DMA
	{
		htft->is_dma_transfer_active = 1; // 设置 DMA 忙标志
		// 启动 SPI DMA 传输 (使用平台抽象函数)
		TFT_Platform_SPI_Transmit_DMA_Start(htft->spi_handle, data_buffer, length);
		// 如果需要等待完成，则在此处等待
		if (wait_completion)
		{
			TFT_Wait_DMA_Transfer_Complete(htft); // 等待 DMA 完成
			TFT_Pin_CS_Set(htft, 1);			  // DMA 完成后手动拉高片选
		}
		// 如果不需要等待 (wait_completion = 0)，CS 将在 DMA 完成回调函数 HAL_SPI_TxCpltCallback 中拉高
	}
	else // 如果未使用 DMA，使用阻塞式 SPI 传输
	{
		// 使用平台抽象的阻塞式发送函数
		TFT_Platform_SPI_Transmit_Blocking(htft->spi_handle, data_buffer, length, HAL_MAX_DELAY); // 使用最大超时时间
		TFT_Pin_CS_Set(htft, 1);																  // 阻塞传输完成后立即拉高片选
	}
}

/**
 * @brief  向发送缓冲区写入 16 位数据 (通常是颜色值)
 * @param  htft TFT句柄指针
 * @param  data 要写入的 16 位数据
 * @retval 无
 * @note   数据以大端模式 (高字节在前) 写入缓冲区。
 *         如果缓冲区空间不足以写入 2 字节，会自动刷新缓冲区 (非阻塞)。
 */
void TFT_Buffer_Write16(TFT_HandleTypeDef *htft, uint16_t data)
{
	// 检查参数
	if (htft == NULL || htft->tx_buffer == NULL)
		return;

	// 检查缓冲区剩余空间是否足够存放 16 位数据 (2字节)
	if (htft->buffer_write_index >= htft->buffer_size - 1)
	{
		TFT_Flush_Buffer(htft, 0); // 缓冲区满，刷新缓冲区，不等待完成
	}

	// 将 16 位数据按大端序写入缓冲区
	htft->tx_buffer[htft->buffer_write_index++] = (data >> 8) & 0xFF; // 高字节
	htft->tx_buffer[htft->buffer_write_index++] = data & 0xFF;		  // 低字节
}

/**
 * @brief  将发送缓冲区中剩余的数据发送到 TFT
 * @param  htft TFT句柄指针
 * @param  wait_completion 是否等待传输完成 (1=等待, 0=不等待，仅 DMA 模式有效)
 * @retval 无
 */
void TFT_Flush_Buffer(TFT_HandleTypeDef *htft, uint8_t wait_completion)
{
	if (htft == NULL || htft->tx_buffer == NULL || htft->buffer_write_index == 0)
		return; // 缓冲区为空，无需刷新

	// 调用 TFT_SPI_Send 发送缓冲区中的数据
	TFT_SPI_Send(htft, htft->tx_buffer, htft->buffer_write_index, wait_completion);

	htft->buffer_write_index = 0; // 发送后重置缓冲区索引
}

/**
 * @brief  重置发送缓冲区（清空索引，不发送数据）
 * @param  htft TFT句柄指针
 * @retval 无
 */
void TFT_Reset_Buffer(TFT_HandleTypeDef *htft)
{
	if (htft == NULL)
		return;

	htft->buffer_write_index = 0;
}

/**
 * @brief  初始化 TFT IO 层，配置 SPI 句柄和 DMA 使用状态
 * @param  htft TFT句柄指针
 * @retval 无
 */
void TFT_IO_Init(TFT_HandleTypeDef *htft)
{
	if (htft == NULL || htft->spi_handle == NULL)
	{
		// 可以在这里添加错误处理，例如断言或日志记录
		return;
	}

	// 分配发送缓冲区内存
	if (htft->tx_buffer == NULL)
	{
		htft->tx_buffer = (uint8_t *)malloc(htft->buffer_size);
		if (htft->tx_buffer == NULL)
		{
			// 内存分配失败处理
			return;
		}
	}

	htft->buffer_write_index = 0; // 初始化缓冲区索引

#ifdef STM32HAL
	// 检查关联的 SPI 句柄是否配置了 DMA 发送通道
	if (htft->spi_handle->hdmatx != NULL)
	{
		htft->is_dma_enabled = 1; // SPI 已配置 DMA 发送
	}
	else
	{
		htft->is_dma_enabled = 0; // SPI 未配置 DMA 发送
	}
#elif defined(SOME_OTHER_PLATFORM)
	// 在此添加其他平台的 DMA 配置检查逻辑
	// htft->is_dma_enabled = OtherPlatform_IsDmaEnabled(htft->spi_handle);
	htft->is_dma_enabled = 0; // 假设默认禁用 DMA，需要具体实现
#else
#error "No platform defined for SPI/DMA initialization in TFT_config.h"
#endif

	htft->is_dma_transfer_active = 0; // 初始化 DMA 传输状态标志

	// 注册设备到全局设备列表，用于DMA回调
	TFT_Register_Device(htft);
}

/**
 * @brief  等待上一次 SPI DMA 传输完成 (内部辅助函数)
 * @param  htft TFT句柄指针
 * @retval 无
 * @note   仅在 DMA 模式下且 DMA 传输正在进行时阻塞。
 */
static void TFT_Wait_DMA_Transfer_Complete(TFT_HandleTypeDef *htft)
{
	if (htft == NULL)
		return;

	// 仅当 DMA 被启用且当前有活动的 DMA 传输时才需要等待
	if (htft->is_dma_enabled)
	{
		while (htft->is_dma_transfer_active)
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
 * @param  htft TFT句柄指针
 * @param  data 要写入的 8 位数据
 * @retval 无
 * @note   此函数总是使用阻塞式 SPI 传输，并假定在发送前 DC 引脚已设为数据模式。
 *         通常在发送命令后调用，用于发送命令参数。
 */
void TFT_Write_Data8(TFT_HandleTypeDef *htft, uint8_t data)
{
	if (htft == NULL || htft->spi_handle == NULL)
		return;

	TFT_Wait_DMA_Transfer_Complete(htft); // 确保之前的 DMA 操作完成
	TFT_Pin_DC_Set(htft, 1);			  // 确保是数据模式
	TFT_Pin_CS_Set(htft, 0);			  // 片选选中

	// 使用平台抽象的阻塞式发送单个字节
	TFT_Platform_SPI_Transmit_Blocking(htft->spi_handle, &data, 1, HAL_MAX_DELAY);

	TFT_Pin_CS_Set(htft, 1); // 传输完成后拉高 CS
}

/**
 * @brief  向 TFT 写入 16 位数据 (阻塞方式)
 * @param  htft TFT句柄指针
 * @param  data 要写入的 16 位数据
 * @retval 无
 * @note   此函数总是使用阻塞式 SPI 传输。
 *         主要用于绘制单个点或少量数据，不经过发送缓冲区。
 */
void TFT_Write_Data16(TFT_HandleTypeDef *htft, uint16_t data)
{
	if (htft == NULL || htft->spi_handle == NULL)
		return;

	uint8_t spi_data[2];
	spi_data[0] = (data >> 8) & 0xFF; // 高字节 (大端)
	spi_data[1] = data & 0xFF;		  // 低字节

	TFT_Wait_DMA_Transfer_Complete(htft); // 确保之前的 DMA 操作完成
	TFT_Pin_DC_Set(htft, 1);			  // 数据模式
	TFT_Pin_CS_Set(htft, 0);			  // 片选选中

	// 使用平台抽象的阻塞式发送 2 个字节
	TFT_Platform_SPI_Transmit_Blocking(htft->spi_handle, spi_data, 2, HAL_MAX_DELAY);

	TFT_Pin_CS_Set(htft, 1); // 传输完成后拉高 CS
}

/**
 * @brief  向 TFT 发送命令
 * @param  htft TFT句柄指针
 * @param  command 要发送的命令字节
 * @retval 无
 * @note   发送命令前会先刷新缓冲区 (阻塞等待)。
 *         命令本身使用阻塞式 SPI 传输。
 */
void TFT_Write_Command(TFT_HandleTypeDef *htft, uint8_t command)
{
	if (htft == NULL || htft->spi_handle == NULL)
		return;

	// 发送命令前，确保缓冲区中的所有数据已发送完成
	TFT_Flush_Buffer(htft, 1); // 等待缓冲区刷新完成

	// 不需要再次调用 TFT_Wait_DMA_Transfer_Complete()，因为 Flush_Buffer(1) 已经等待了

	TFT_Pin_DC_Set(htft, 0); // 设置为命令模式
	TFT_Pin_CS_Set(htft, 0); // 片选选中

	// 使用平台抽象的阻塞式发送命令字节
	TFT_Platform_SPI_Transmit_Blocking(htft->spi_handle, &command, 1, HAL_MAX_DELAY);

	TFT_Pin_CS_Set(htft, 1); // 命令发送完成后立即拉高 CS
}

/**
 * @brief  设置显示区域的地址范围
 * @param  htft TFT句柄指针
 * @param  x_start 起始列坐标
 * @param  y_start 起始行坐标
 * @param  x_end   结束列坐标
 * @param  y_end   结束行坐标
 * @retval 无
 * @note   设置后，后续所有的数据传输都会写入此区域，窗口在不同屏幕方向下会自动适配
 */
void TFT_Set_Address(TFT_HandleTypeDef *htft, uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end)
{
	if (htft == NULL)
		return;

	// 设置地址前，确保缓冲区中的所有数据已发送完成
	TFT_Flush_Buffer(htft, 1); // 等待缓冲区刷新完成

	// --- 设置列地址 (Column Address Set, CASET, 0x2A) ---
	TFT_Write_Command(htft, 0x2A);

	// 根据屏幕方向和型号设置列地址
	if (htft->display_direction == 0 || htft->display_direction == 2) // 0°或180°
	{
		TFT_Write_Data16(htft, x_start + htft->x_offset);
		TFT_Write_Data16(htft, x_end + htft->x_offset);
	}
	else // 90°或270°
	{
		TFT_Write_Data16(htft, x_start + htft->y_offset);
		TFT_Write_Data16(htft, x_end + htft->y_offset);
	}

	// --- 设置行地址范围 (Set Row Address, 0x2B) ---
	TFT_Write_Command(htft, 0x2B);

	// 根据屏幕方向和型号设置行地址
	if (htft->display_direction == 0 || htft->display_direction == 2) // 0°或180°
	{
		TFT_Write_Data16(htft, y_start + htft->y_offset);
		TFT_Write_Data16(htft, y_end + htft->y_offset);
	}
	else // 90°或270°
	{
		TFT_Write_Data16(htft, y_start + htft->x_offset);
		TFT_Write_Data16(htft, y_end + htft->x_offset);
	}

	// --- 发送写 GRAM 命令 (Memory Write, 0x2C) ---
	// 后续发送的数据将被写入由此窗口定义的 GRAM 区域
	TFT_Write_Command(htft, 0x2C);
}

/**
 * @brief  将RGB颜色值转换为RGB565格式
 * @param  r  红色分量，范围0-255
 * @param  g  绿色分量，范围0-255
 * @param  b  蓝色分量，范围0-255
 * @retval RGB565格式的16位颜色值
 * @note   RGB888 (24bit) -> RGB565 (16bit)
 *         R: 5bit (0-31), G: 6bit (0-63), B: 5bit (0-31)
 */
uint16_t TFT_RGB(uint8_t r, uint8_t g, uint8_t b)
{
	// 转换RGB888到RGB565格式
	// R(5位) G(6位) B(5位)
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

//----------------- HAL SPI DMA 回调函数 -----------------

#ifdef STM32HAL // 仅当使用 STM32 HAL 时编译此回调函数
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
	// 检查是哪个TFT设备触发了回调
	for (int i = 0; i < MAX_TFT_DEVICES; i++)
	{
		if (g_tft_handles[i] != NULL && g_tft_handles[i]->spi_handle == hspi)
		{
			// 找到对应设备
			TFT_HandleTypeDef *htft = g_tft_handles[i];

			// 仅在 DMA 模式下，传输完成后需要处理
			if (htft->is_dma_enabled)
			{
				// 1. 拉高片选引脚 (CS)，结束本次 SPI 通信
				TFT_Pin_CS_Set(htft, 1);
				// 2. 清除 DMA 传输忙标志
				htft->is_dma_transfer_active = 0;
				// 3. (可选) 在 RTOS 环境下，可以在这里释放信号量或设置事件标志，
				//    以唤醒等待 DMA 完成的任务。
				//    例如: osSemaphoreRelease(htft->spiDmaSemaphore);
			}
			break;
		}
	}
}
#endif // STM32HAL
