#ifndef __LCD_H
#define __LCD_H

#include "main.h"
#include "lcd_init.h" // 包含lcd_init.h以使用u8/u16/u32类型定义和颜色定义

//----------------- LCD 绘图函数声明 -----------------

/**
 * @brief  在指定区域填充单色
 * @param  x_start 起始列坐标
 * @param  y_start 起始行坐标
 * @param  x_end   结束列坐标
 * @param  y_end   结束行坐标
 * @param  color   要填充的颜色 (RGB565格式)
 * @retval 无
 */
void LCD_Fill_Area(u16 x_start, u16 y_start, u16 x_end, u16 y_end, u16 color);

/**
 * @brief  在指定坐标绘制一个点
 * @param  x     点的列坐标
 * @param  y     点的行坐标
 * @param  color 点的颜色 (RGB565格式)
 * @retval 无
 */
void LCD_Draw_Point(u16 x, u16 y, u16 color);

/**
 * @brief  绘制一条直线
 * @param  x1    起点列坐标
 * @param  y1    起点行坐标
 * @param  x2    终点列坐标
 * @param  y2    终点行坐标
 * @param  color 线的颜色 (RGB565格式)
 * @retval 无
 */
void LCD_Draw_Line(u16 x1, u16 y1, u16 x2, u16 y2, u16 color);

/**
 * @brief  绘制一个空心矩形
 * @param  x1    左上角列坐标
 * @param  y1    左上角行坐标
 * @param  x2    右下角列坐标
 * @param  y2    右下角行坐标
 * @param  color 矩形边框颜色 (RGB565格式)
 * @retval 无
 */
void LCD_Draw_Rectangle(u16 x1, u16 y1, u16 x2, u16 y2, u16 color);

/**
 * @brief  绘制一个圆
 * @param  x0    圆心列坐标
 * @param  y0    圆心行坐标
 * @param  r     圆的半径
 * @param  color 圆的颜色 (RGB565格式)
 * @retval 无
 */
void LCD_Draw_Circle(u16 x0, u16 y0, u8 r, u16 color);

//----------------- LCD 字符/字符串/数字 显示函数声明 -----------------

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
void LCD_Show_Chinese_String(u16 x, u16 y, u8 *str, u16 fg_color, u16 bg_color, u8 font_size, u8 mode);

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
void LCD_Show_Chinese_16x16(u16 x, u16 y, u8 *hz_ptr, u16 fg_color, u16 bg_color, u8 font_size, u8 mode);

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
void LCD_Show_Chinese_24x24(u16 x, u16 y, u8 *hz_ptr, u16 fg_color, u16 bg_color, u8 font_size, u8 mode);

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
void LCD_Show_Chinese_32x32(u16 x, u16 y, u8 *hz_ptr, u16 fg_color, u16 bg_color, u8 font_size, u8 mode);

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
void LCD_Show_Char(u16 x, u16 y, u8 ascii_char, u16 fg_color, u16 bg_color, u8 font_size, u8 mode);

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
void LCD_Show_String(u16 x, u16 y, const u8 *str, u16 fg_color, u16 bg_color, u8 font_size, u8 mode);

/**
 * @brief  计算m的n次方
 * @param  m 底数
 * @param  n 指数
 * @retval u32 计算结果
 */
u32 LCD_Pow(u8 m, u8 n);

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
void LCD_Show_Int_Num(u16 x, u16 y, u16 num, u8 len, u16 fg_color, u16 bg_color, u8 font_size, u8 mode);

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
void LCD_Show_Float_Num(u16 x, u16 y, float num, u8 len, u16 fg_color, u16 bg_color, u8 font_size, u8 mode);

//----------------- LCD 图片显示函数声明 -----------------

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
void LCD_Show_Picture(u16 x, u16 y, u16 width, u16 height, const u8 pic_ptr[]);

#endif
