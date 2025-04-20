#ifndef __TFT_TEXT_H
#define __TFT_TEXT_H

#include "stdint.h"
#include "TFTh/font.h" // 需要字体结构定义和字体数据声明

#ifdef __cplusplus
extern "C"
{
#endif
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
     * @brief  在指定位置显示字符串 (支持 ASCII 和 UTF-8 中文)
     * @param  x          起始列坐标
     * @param  y          起始行坐标
     * @param  str        要显示的字符串 (UTF-8 编码)
     * @param  color      字符颜色
     * @param  back_color 背景颜色
     * @param  size       字体大小 (ASCII 支持 8, 12, 16; 中文目前支持 16)
     * @param  mode       模式 (0: 背景不透明, 1: 背景透明)
     * @retval 无
     * @note   中文显示依赖 zh16x16 字库 (size=16)。
     *         请确保源文件和编译器使用 UTF-8 编码。
     */
    void TFT_Show_String(uint16_t x, uint16_t y, const uint8_t *str, uint16_t color, uint16_t back_color, uint8_t size, uint8_t mode);

#ifdef __cplusplus
}
#endif

#endif // __TFT_TEXT_H