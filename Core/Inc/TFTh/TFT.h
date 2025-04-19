#ifndef __TFT_H__
#define __TFT_H__

#include "main.h"
#include "TFTh/TFT_io.h"   // 包含 TFT_io.h 以使用底层函数和颜色定义
#include <stdint.h>

//----------------- TFT 绘图函数声明 -----------------

/**
 * @brief  在指定区域填充单色
 * @param  x_start 起始列坐标
 * @param  y_start 起始行坐标
 * @param  x_end   结束列坐标
 * @param  y_end   结束行坐标
 * @param  color   要填充的颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Fill_Area(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t color);

/**
 * @brief  在指定坐标绘制一个点
 * @param  x     点的列坐标
 * @param  y     点的行坐标
 * @param  color 点的颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Draw_Point(uint16_t x, uint16_t y, uint16_t color);

/**
 * @brief  绘制一条直线
 * @param  x1    起点列坐标
 * @param  y1    起点行坐标
 * @param  x2    终点列坐标
 * @param  y2    终点行坐标
 * @param  color 线的颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Draw_Line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

/**
 * @brief  绘制一个空心矩形
 * @param  x1    左上角列坐标
 * @param  y1    左上角行坐标
 * @param  x2    右下角列坐标
 * @param  y2    右下角行坐标
 * @param  color 矩形边框颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Draw_Rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

/**
 * @brief  绘制一个圆
 * @param  x0    圆心列坐标
 * @param  y0    圆心行坐标
 * @param  r     圆的半径
 * @param  color 圆的颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Draw_Circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color);

//----------------- TFT 字符/字符串/数字 显示函数声明 -----------------

/**
 * @brief  显示一个中文字符串 (自动根据字号调用相应函数)
 * @param  x      起始列坐标
 * @param  y      起始行坐标
 * @param  str    要显示的UTF-8编码的中文字符串指针
 * @param  fg_color 字体颜色
 * @param  bg_color 背景颜色
 * @param  font_size 字号 (支持 16, 24, 32)
 * @param  mode   显示模式: 0=背景不透明, 1=背景透明 (叠加显示)
 * @retval 无
 */
void TFT_Show_Chinese_String(uint16_t x, uint16_t y, uint8_t *str, uint16_t fg_color, uint16_t bg_color, uint8_t font_size, uint8_t mode);

/**
 * @brief  显示一个16x16的汉字
 * @param  x      起始列坐标
 * @param  y      起始行坐标
 * @param  hz_ptr 指向要显示的汉字内码(2字节)的指针
 * @param  fg_color 字体颜色
 * @param  bg_color 背景颜色
 * @param  font_size 字号 (固定为16)
 * @param  mode   显示模式: 0=背景不透明, 1=背景透明 (叠加显示)
 * @retval 无
 */
void TFT_Show_Chinese_16x16(uint16_t x, uint16_t y, uint8_t *hz_ptr, uint16_t fg_color, uint16_t bg_color, uint8_t font_size, uint8_t mode);

/**
 * @brief  显示一个24x24的汉字
 * @param  x      起始列坐标
 * @param  y      起始行坐标
 * @param  hz_ptr 指向要显示的汉字内码(2字节)的指针
 * @param  fg_color 字体颜色
 * @param  bg_color 背景颜色
 * @param  font_size 字号 (固定为24)
 * @param  mode   显示模式: 0=背景不透明, 1=背景透明 (叠加显示)
 * @retval 无
 */
void TFT_Show_Chinese_24x24(uint16_t x, uint16_t y, uint8_t *hz_ptr, uint16_t fg_color, uint16_t bg_color, uint8_t font_size, uint8_t mode);

/**
 * @brief  显示一个32x32的汉字
 * @param  x      起始列坐标
 * @param  y      起始行坐标
 * @param  hz_ptr 指向要显示的汉字内码(2字节)的指针
 * @param  fg_color 字体颜色
 * @param  bg_color 背景颜色
 * @param  font_size 字号 (固定为32)
 * @param  mode   显示模式: 0=背景不透明, 1=背景透明 (叠加显示)
 * @retval 无
 */
void TFT_Show_Chinese_32x32(uint16_t x, uint16_t y, uint8_t *hz_ptr, uint16_t fg_color, uint16_t bg_color, uint8_t font_size, uint8_t mode);

/**
 * @brief  显示一个ASCII字符
 * @param  x      起始列坐标
 * @param  y      起始行坐标
 * @param  ascii_char 要显示的ASCII字符
 * @param  fg_color 字体颜色
 * @param  bg_color 背景颜色
 * @param  font_size 字号 (支持 16, 32)
 * @param  mode   显示模式: 0=背景不透明, 1=背景透明 (叠加显示)
 * @retval 无
 */
void TFT_Show_Char(uint16_t x, uint16_t y, uint8_t ascii_char, uint16_t fg_color, uint16_t bg_color, uint8_t font_size, uint8_t mode);

/**
 * @brief  显示一个ASCII字符串
 * @param  x      起始列坐标
 * @param  y      起始行坐标
 * @param  str    要显示的ASCII字符串指针
 * @param  fg_color 字体颜色
 * @param  bg_color 背景颜色
 * @param  font_size 字号 (支持 16, 32)
 * @param  mode   显示模式: 0=背景不透明, 1=背景透明 (叠加显示)
 * @retval 无
 */
void TFT_Show_String(uint16_t x, uint16_t y, const uint8_t *str, uint16_t fg_color, uint16_t bg_color, uint8_t font_size, uint8_t mode);

/**
 * @brief  计算m的n次方
 * @param  m 底数
 * @param  n 指数
 * @retval uint32_t 计算结果
 */
uint32_t TFT_Pow(uint8_t m, uint8_t n);

/**
 * @brief  显示一个无符号整型数字
 * @param  x      起始列坐标
 * @param  y      起始行坐标
 * @param  num    要显示的无符号整数
 * @param  len    要显示的数字位数 (不足位会补空格或0，取决于实现)
 * @param  fg_color 字体颜色
 * @param  bg_color 背景颜色
 * @param  font_size 字号 (支持 16, 32)
 * @param  mode   显示模式: 0=背景不透明, 1=背景透明 (叠加显示)
 * @retval 无
 */
void TFT_Show_Int_Num(uint16_t x, uint16_t y, uint16_t num, uint8_t len, uint16_t fg_color, uint16_t bg_color, uint8_t font_size, uint8_t mode);

/**
 * @brief  显示一个浮点数 (保留两位小数)
 * @param  x      起始列坐标
 * @param  y      起始行坐标
 * @param  num    要显示的浮点数
 * @param  len    整数部分要显示的位数
 * @param  fg_color 字体颜色
 * @param  bg_color 背景颜色
 * @param  font_size 字号 (支持 16, 32)
 * @param  mode   显示模式: 0=背景不透明, 1=背景透明 (叠加显示)
 * @retval 无
 */
void TFT_Show_Float_Num(uint16_t x, uint16_t y, float num, uint8_t len, uint16_t fg_color, uint16_t bg_color, uint8_t font_size, uint8_t mode);

//----------------- TFT 图片显示函数声明 -----------------

/**
 * @brief  显示一幅存储在Flash中的图片
 * @param  x      图片左上角列坐标
 * @param  y      图片左上角行坐标
 * @param  width  图片宽度 (像素)
 * @param  height 图片高度 (像素)
 * @param  pic_ptr 指向图片数据数组 (RGB565格式, 高位在前)
 * @retval 无
 * @note   图片数据需要预先使用取模软件生成
 */
void TFT_Show_Picture(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t pic_ptr[]);

#endif
