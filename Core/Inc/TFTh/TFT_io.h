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

    //----------------- TFT 控制引脚函数声明 (硬件抽象) -----------------

    /**
     * @brief  控制复位引脚 (RES/RST)
     * @param  level: 0=拉低 (复位激活), 1=拉高 (复位释放)
     * @retval 无
     */
    void TFT_Pin_RES_Set(uint8_t level);

    /**
     * @brief  控制数据/命令选择引脚 (DC/RS)
     * @param  level: 0=命令模式 (低), 1=数据模式 (高)
     * @retval 无
     */
    void TFT_Pin_DC_Set(uint8_t level);

    /**
     * @brief  控制片选引脚 (CS)
     * @param  level: 0=选中 (低), 1=取消选中 (高)
     * @retval 无
     */
    void TFT_Pin_CS_Set(uint8_t level);

    /**
     * @brief  控制背光引脚 (BLK/BL)
     * @param  level: 0=关闭 (低), 1=打开 (高) (注意: 可能需要根据硬件反转)
     * @retval 无
     */
    void TFT_Pin_BLK_Set(uint8_t level);

    //----------------- TFT IO 函数声明 -----------------

    /**
     * @brief  初始化 TFT IO 层
     * @param  hspi_ptr 指向已初始化的 SPI_HandleTypeDef 结构的指针 (或其他平台的 SPI 句柄)
     * @retval 无
     * @note   必须在使用其他 IO 函数之前调用。会保存 SPI 句柄并检查 DMA 配置。
     */
    void TFT_IO_Init(SPI_HandleTypeDef *hspi_ptr); // 注意：参数类型可能需要根据平台调整

    /**
     * @brief  通过 SPI 发送指定缓冲区的数据到 TFT (使用缓冲区和 DMA/阻塞)
     * @param  data_buffer 要发送的数据缓冲区指针
     * @param  length      要发送的数据长度（字节数）
     * @param  wait_completion 是否等待传输完成 (1=等待, 0=不等待，仅 DMA 模式有效)
     * @retval 无
     */
    void TFT_SPI_Send(uint8_t *data_buffer, uint16_t length, uint8_t wait_completion);

    /**
     * @brief  向发送缓冲区写入 16 位数据 (通常是颜色值)
     * @param  data 要写入的 16 位数据
     * @retval 无
     * @note   数据以大端模式写入。若缓冲区满则自动刷新 (非阻塞)。
     */
    void TFT_Buffer_Write16(uint16_t data);

    /**
     * @brief  将发送缓冲区中剩余的数据发送到 TFT
     * @param  wait_completion 是否等待传输完成 (1=等待, 0=不等待，仅 DMA 模式有效)
     * @retval 无
     */
    void TFT_Flush_Buffer(uint8_t wait_completion);

    /**
     * @brief  重置发送缓冲区（清空索引，不发送数据）
     * @retval 无
     */
    void TFT_Reset_Buffer(void);

    /**
     * @brief  向 TFT 写入 8 位数据 (阻塞方式)
     * @param  data 要写入的 8 位数据
     * @retval 无
     * @note   主要用于发送命令参数。
     */
    void TFT_Write_Data8(uint8_t data);

    /**
     * @brief  向 TFT 写入 16 位数据 (阻塞方式)
     * @param  data 要写入的 16 位数据
     * @retval 无
     * @note   不经过发送缓冲区，效率较低。
     */
    void TFT_Write_Data16(uint16_t data);

    /**
     * @brief  向 TFT 发送命令 (阻塞方式)
     * @param  command 要发送的命令字节
     * @retval 无
     * @note   发送命令前会阻塞等待缓冲区刷新完成。
     */
    void TFT_Write_Command(uint8_t command);

    /**
     * @brief  设置 TFT 显示窗口区域 (GRAM 访问窗口)
     * @param  x_start 列起始坐标 (0-based)
     * @param  y_start 行起始坐标 (0-based)
     * @param  x_end   列结束坐标 (0-based, inclusive)
     * @param  y_end   行结束坐标 (0-based, inclusive)
     * @retval 无
     * @note   设置地址前会阻塞等待缓冲区刷新完成。坐标会根据配置自动偏移。
     */
    void TFT_Set_Address(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end);

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
