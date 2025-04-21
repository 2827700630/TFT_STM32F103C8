/*
 * @file    tft_io.h
 * @brief   TFT底层IO驱动头文件
 * @details 定义了与硬件交互的底层函数接口，包括 GPIO 控制、SPI 通信和缓冲区管理。
 */
#ifndef __TFT_IO_H
#define __TFT_IO_H

#include "main.h"
#include "spi.h" // 包含 spi.h 以获取 SPI_HandleTypeDef 类型
#include <stdint.h>
#include "TFT_config.h" // 包含配置文件，获取缓冲区大小、颜色定义、引脚配置等

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief  TFT屏幕句柄结构体，用于多屏同时显示
     * @note   每个TFT屏幕实例都有一个独立的句柄
     */
    typedef struct
    {
        SPI_HandleTypeDef *spi_handle; // SPI句柄
        GPIO_TypeDef *cs_port;         // CS引脚端口
        uint16_t cs_pin;               // CS引脚号
        GPIO_TypeDef *dc_port;         // DC引脚端口
        uint16_t dc_pin;               // DC引脚号
        GPIO_TypeDef *res_port;        // RES引脚端口
        uint16_t res_pin;              // RES引脚号
        GPIO_TypeDef *bl_port;         // BL引脚端口
        uint16_t bl_pin;               // BL引脚号

        uint8_t *tx_buffer;          // 发送缓冲区
        uint16_t buffer_size;        // 缓冲区大小
        uint16_t buffer_write_index; // 当前缓冲区写入位置索引

        uint8_t is_dma_enabled;                  // DMA使能标志
        volatile uint8_t is_dma_transfer_active; // DMA传输忙标志

        uint8_t display_direction; // 显示方向
        uint8_t x_offset;          // X偏移量
        uint8_t y_offset;          // Y偏移量
    } TFT_HandleTypeDef;

    //----------------- TFT 控制引脚函数声明 (硬件抽象) -----------------

    /**
     * @brief  控制复位引脚 (RES/RST)
     * @param  htft: TFT屏幕句柄指针
     * @param  level: 0=拉低 (复位激活), 1=拉高 (复位释放)
     * @retval 无
     */
    void TFT_Pin_RES_Set(TFT_HandleTypeDef *htft, uint8_t level);

    /**
     * @brief  控制数据/命令选择引脚 (DC/RS)
     * @param  htft: TFT屏幕句柄指针
     * @param  level: 0=命令模式 (低), 1=数据模式 (高)
     * @retval 无
     */
    void TFT_Pin_DC_Set(TFT_HandleTypeDef *htft, uint8_t level);

    /**
     * @brief  控制片选引脚 (CS)
     * @param  htft: TFT屏幕句柄指针
     * @param  level: 0=选中 (低), 1=取消选中 (高)
     * @retval 无
     */
    void TFT_Pin_CS_Set(TFT_HandleTypeDef *htft, uint8_t level);

    /**
     * @brief  控制背光引脚 (BLK/BL)
     * @param  htft: TFT屏幕句柄指针
     * @param  level: 0=关闭 (低), 1=打开 (高) (注意: 可能需要根据硬件反转)
     * @retval 无
     */
    void TFT_Pin_BLK_Set(TFT_HandleTypeDef *htft, uint8_t level);

    //----------------- TFT IO 函数声明 -----------------

    /**
     * @brief  TFT实例初始化
     * @param  htft: TFT屏幕句柄指针
     * @param  hspi: SPI句柄指针
     * @param  cs_port: CS引脚端口
     * @param  cs_pin: CS引脚号
     * @retval 无
     */
    void TFT_Init_Instance(TFT_HandleTypeDef *htft, SPI_HandleTypeDef *hspi,
                           GPIO_TypeDef *cs_port, uint16_t cs_pin);

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
                         GPIO_TypeDef *bl_port, uint16_t bl_pin);

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
                            uint8_t x_offset, uint8_t y_offset);

    /**
     * @brief  初始化 TFT IO 层
     * @param  htft TFT句柄指针
     * @retval 无
     * @note   必须在使用其他 IO 函数之前调用。会保存 SPI 句柄并检查 DMA 配置。
     */
    void TFT_IO_Init(TFT_HandleTypeDef *htft);

    /**
     * @brief  通过 SPI 发送指定缓冲区的数据到 TFT (使用缓冲区和 DMA/阻塞)
     * @param  htft TFT句柄指针
     * @param  data_buffer 要发送的数据缓冲区指针
     * @param  length      要发送的数据长度（字节数）
     * @param  wait_completion 是否等待传输完成 (1=等待, 0=不等待，仅 DMA 模式有效)
     * @retval 无
     */
    void TFT_SPI_Send(TFT_HandleTypeDef *htft, uint8_t *data_buffer, uint16_t length, uint8_t wait_completion);

    /**
     * @brief  向发送缓冲区写入 16 位数据 (通常是颜色值)
     * @param  htft TFT句柄指针
     * @param  data 要写入的 16 位数据
     * @retval 无
     * @note   数据以大端模式写入。若缓冲区满则自动刷新 (非阻塞)。
     */
    void TFT_Buffer_Write16(TFT_HandleTypeDef *htft, uint16_t data);

    /**
     * @brief  将发送缓冲区中剩余的数据发送到 TFT
     * @param  htft TFT句柄指针
     * @param  wait_completion 是否等待传输完成 (1=等待, 0=不等待，仅 DMA 模式有效)
     * @retval 无
     */
    void TFT_Flush_Buffer(TFT_HandleTypeDef *htft, uint8_t wait_completion);

    /**
     * @brief  重置发送缓冲区（清空索引，不发送数据）
     * @param  htft TFT句柄指针
     * @retval 无
     */
    void TFT_Reset_Buffer(TFT_HandleTypeDef *htft);

    /**
     * @brief  向 TFT 写入 8 位数据 (阻塞方式)
     * @param  htft TFT句柄指针
     * @param  data 要写入的 8 位数据
     * @retval 无
     * @note   主要用于发送命令参数。
     */
    void TFT_Write_Data8(TFT_HandleTypeDef *htft, uint8_t data);

    /**
     * @brief  向 TFT 写入 16 位数据 (阻塞方式)
     * @param  htft TFT句柄指针
     * @param  data 要写入的 16 位数据
     * @retval 无
     * @note   不经过发送缓冲区，效率较低。
     */
    void TFT_Write_Data16(TFT_HandleTypeDef *htft, uint16_t data);

    /**
     * @brief  向 TFT 发送命令 (阻塞方式)
     * @param  htft TFT句柄指针
     * @param  command 要发送的命令字节
     * @retval 无
     * @note   发送命令前会阻塞等待缓冲区刷新完成。
     */
    void TFT_Write_Command(TFT_HandleTypeDef *htft, uint8_t command);

    /**
     * @brief  设置 TFT 显示窗口区域 (GRAM 访问窗口)
     * @param  htft TFT句柄指针
     * @param  x_start 列起始坐标 (0-based)
     * @param  y_start 行起始坐标 (0-based)
     * @param  x_end   列结束坐标 (0-based, inclusive)
     * @param  y_end   行结束坐标 (0-based, inclusive)
     * @retval 无
     * @note   设置地址前会阻塞等待缓冲区刷新完成。坐标会根据配置自动偏移。
     */
    void TFT_Set_Address(TFT_HandleTypeDef *htft, uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end);

    //----------------- 平台相关的 SPI 传输函数声明 (内部使用) -----------------

    /**
     * @brief  平台相关的阻塞式 SPI 发送函数
     * @param  spi_handle 平台相关的 SPI 句柄指针
     * @param  pData      要发送的数据缓冲区指针
     * @param  Size       要发送的数据大小 (字节)
     * @param  Timeout    超时时间 (平台相关定义)
     * @retval 平台相关的状态码 (例如 HAL_StatusTypeDef)
     */
    int TFT_Platform_SPI_Transmit_Blocking(SPI_HandleTypeDef *spi_handle, uint8_t *pData, uint16_t Size, uint32_t Timeout);

    /**
     * @brief  平台相关的启动 SPI DMA 发送函数
     * @param  spi_handle 平台相关的 SPI 句柄指针
     * @param  pData      要发送的数据缓冲区指针
     * @param  Size       要发送的数据大小 (字节)
     * @retval 平台相关的状态码 (例如 HAL_StatusTypeDef)
     * @note   此函数应启动传输但不等待完成。完成由回调处理。
     */
    int TFT_Platform_SPI_Transmit_DMA_Start(SPI_HandleTypeDef *spi_handle, uint8_t *pData, uint16_t Size);

    // HAL库回调函数声明 (如果需要在其他文件访问，通常不需要)
    // void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi);

    /**
     * @brief  将RGB颜色值转换为RGB565格式
     * @param  r  红色分量，范围0-255
     * @param  g  绿色分量，范围0-255
     * @param  b  蓝色分量，范围0-255
     * @retval RGB565格式的16位颜色值
     * @note   RGB888 (24bit) -> RGB565 (16bit)
     *         R: 5bit (0-31), G: 6bit (0-63), B: 5bit (0-31)
     */
    uint16_t TFT_RGB(uint8_t r, uint8_t g, uint8_t b);

#ifdef __cplusplus
}
#endif

#endif
