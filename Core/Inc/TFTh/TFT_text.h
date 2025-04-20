/*
 * @file    TFT_text.h
 * @brief   TFT屏幕文本显示函数头文件
 * @details 定义了TFT屏幕字符和字符串显示函数接口
 */
#ifndef __TFT_TEXT_H
#define __TFT_TEXT_H

#include "main.h"
#include "TFTh/TFT_io.h" // 包含TFT_io.h以获取TFT_HandleTypeDef结构体定义
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief  在指定位置显示一个 ASCII 字符
     * @param  htft TFT句柄指针
     * @param  x          起始列坐标
     * @param  y          起始行坐标
     * @param  chr        要显示的 ASCII 字符
     * @param  color      字符颜色
     * @param  back_color 背景颜色
     * @param  size       字体大小 (支持 8, 12, 16)
     * @param  mode       模式 (0: 背景不透明, 1: 背景透明)
     * @retval 无
     */
    void TFT_Show_Char(TFT_HandleTypeDef *htft, uint16_t x, uint16_t y, uint8_t chr, uint16_t color, uint16_t back_color, uint8_t size, uint8_t mode);

    /**
     * @brief  在指定位置显示 ASCII 字符串
     * @param  htft TFT句柄指针
     * @param  x          起始列坐标
     * @param  y          起始行坐标
     * @param  str        要显示的 ASCII 字符串
     * @param  color      字符颜色
     * @param  back_color 背景颜色
     * @param  size       字体大小 (支持 8, 12, 16)
     * @param  mode       模式 (0: 背景不透明, 1: 背景透明)
     * @retval 无
     */
    void TFT_Show_String(TFT_HandleTypeDef *htft, uint16_t x, uint16_t y, const uint8_t *str, uint16_t color, uint16_t back_color, uint8_t size, uint8_t mode);

#ifdef __cplusplus
}
#endif

#endif