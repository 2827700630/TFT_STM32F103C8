/*
 * @file    tft_io.h
 * @brief   TFT底层IO驱动头文件
 */
#ifndef __TFT_IO_H
#define __TFT_IO_H

#include "main.h"
#include "spi.h" // 包含 spi.h 以获取 SPI_HandleTypeDef 类型
#include <stdint.h>
#include "TFT_config.h" // 包含配置文件，获取颜色定义等

//----------------- TFT 控制引脚函数声明 -----------------

/**
 * @brief  控制复位引脚 (RES/RST)
 * @param  state: 0=拉低, 1=拉高
 * @retval 无
 */
void TFT_Pin_RES_Set(uint8_t state);

/**
 * @brief  控制数据/命令选择引脚 (DC/RS)
 * @param  state: 0=命令模式 (低), 1=数据模式 (高)
 * @retval 无
 */
void TFT_Pin_DC_Set(uint8_t state);

/**
 * @brief  控制片选引脚 (CS)
 * @param  state: 0=选中 (低), 1=取消选中 (高)
 * @retval 无
 */
void TFT_Pin_CS_Set(uint8_t state);

/**
 * @brief  控制背光引脚 (BLK/BL)
 * @param  state: 0=关闭 (低), 1=打开 (高)
 * @retval 无
 */
void TFT_Pin_BLK_Set(uint8_t state);

//----------------- TFT IO 函数声明 -----------------

/**
 * @brief  初始化 TFT IO 层
 * @param  hspi 指向 SPI_HandleTypeDef 结构的指针
 * @retval 无
 */
void TFT_IO_Init(SPI_HandleTypeDef *hspi);

/**
 * @brief  使用SPI发送缓冲区数据到TFT
 * @param  buffer 要发送的数据缓冲区
 * @param  length 要发送的数据长度（字节数）
 * @param  wait   是否等待传输完成
 * @retval 无
 */
void TFT_SPI_Send(uint8_t* buffer, uint16_t length, uint8_t wait);

/**
 * @brief  向缓冲区添加16位数据
 * @param  data 要添加的16位数据
 * @retval 无
 * @note   如果缓冲区已满，会自动发送并清空缓冲区
 */
void TFT_Buffer_Write16(uint16_t data);

/**
 * @brief  刷新缓冲区内容到TFT
 * @param  wait 是否等待传输完成 (1=等待, 0=不等待)
 * @retval 无
 */
void TFT_Flush_Buffer(uint8_t wait);

/**
 * @brief  重置缓冲区（清空但不发送）
 * @retval 无
 */
void TFT_Reset_Buffer(void);

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
 * @note   坐标从0开始, 会根据 DISPLAY_DIRECTION 自动调整偏移。
 */
void TFT_Set_Address(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end);

// HAL库回调函数声明 (如果需要在其他地方调用，否则可以只在 .c 文件中)
// void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi);

#endif
