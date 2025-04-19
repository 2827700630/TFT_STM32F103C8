#ifndef __TFT_TEXT_H
#define __TFT_TEXT_H

#include "stdint.h"
#include "TFTh/font.h" // 需要字体结构定义和字体数据声明

//----------------- 函数声明 -----------------

/**
 * @brief  在指定位置显示一个 ASCII 字符
 * @param  x          起始列坐标
 * @param  y          起始行坐标
 * @param  chr        要显示的 ASCII 字符
 * @param  color      字符颜色
 * @param  back_color 背景颜色
 * @param  size       字体大小 (支持 8, 12, 16)
 * @param  mode       模式 (0: 背景不透明, 1: 背景透明)
 * @retval 无
 */
void TFT_Show_Char(uint16_t x, uint16_t y, uint8_t chr, uint16_t color, uint16_t back_color, uint8_t size, uint8_t mode);

/**
 * @brief  在指定位置显示字符串 (仅支持 ASCII)
 * @param  x          起始列坐标
 * @param  y          起始行坐标
 * @param  str        要显示的字符串 (ASCII 编码)
 * @param  color      字符颜色
 * @param  back_color 背景颜色
 * @param  size       字体大小 (支持 8, 12, 16)
 * @param  mode       模式 (0: 背景不透明, 1: 背景透明)
 * @retval 无
 */
void TFT_Show_String(uint16_t x, uint16_t y, const uint8_t *str, uint16_t color, uint16_t back_color, uint8_t size, uint8_t mode);

#endif // __TFT_TEXT_H