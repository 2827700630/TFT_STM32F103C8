/*
 * @file    TFT_init.h
 * @brief   TFT屏幕初始化头文件
 * @details 定义了TFT屏幕初始化和绘图函数接口
 */
#ifndef __TFT_INIT_H
#define __TFT_INIT_H

#include "main.h"
#include "TFTh/TFT_io.h" // 包含TFT_io.h以获取TFT_HandleTypeDef结构体定义
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief  ST7735S初始化
     * @param  htft TFT句柄指针
     * @retval 无
     * @note   适用于常见的1.8寸红底128x160 TFT
     */
    void TFT_Init_ST7735S(TFT_HandleTypeDef *htft);

    /**
     * @brief  ST7789v3初始化函数，支持多实例
     * @param  htft TFT句柄指针
     * @retval 无
     * @note   适用于ST7789v3驱动的TFT屏幕，240x240分辨率
     */
    void TFT_Init_ST7789v3(TFT_HandleTypeDef *htft);

#ifdef __cplusplus
}
#endif

#endif
