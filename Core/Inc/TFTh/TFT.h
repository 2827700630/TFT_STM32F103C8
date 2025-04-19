#ifndef __TFT_H__
#define __TFT_H__

#include "main.h"
#include "TFTh/TFT_io.h" // 包含 TFT_io.h 以使用底层函数和颜色定义
#include <stdint.h>

//----------------- 点结构体定义 -----------------
/**
 * @brief  结构体用于表示一个点的坐标
 */
typedef struct
{
    uint16_t x;
    uint16_t y;
} TFT_Point;

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
 * @brief  绘制多个点 (利用缓冲区加速)
 * @param  points 点坐标数组
 * @param  count  点的数量
 * @param  color  点的颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Draw_MultiPoint(const TFT_Point points[], uint16_t count, uint16_t color);

/**
 * @brief  快速绘制水平线
 * @param  x     起始列坐标
 * @param  y     行坐标
 * @param  width 线宽度
 * @param  color 线的颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Draw_Fast_HLine(uint16_t x, uint16_t y, uint16_t width, uint16_t color);

/**
 * @brief  快速绘制垂直线
 * @param  x     列坐标
 * @param  y     起始行坐标
 * @param  height 线高度
 * @param  color 线的颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Draw_Fast_VLine(uint16_t x, uint16_t y, uint16_t height, uint16_t color);

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
 * @brief  填充一个实心矩形
 * @param  x1    左上角列坐标
 * @param  y1    左上角行坐标
 * @param  x2    右下角列坐标
 * @param  y2    右下角行坐标
 * @param  color 矩形填充颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Fill_Rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

/**
 * @brief  绘制一个圆
 * @param  x0    圆心列坐标
 * @param  y0    圆心行坐标
 * @param  r     圆的半径
 * @param  color 圆的颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Draw_Circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color);

/**
 * @brief  填充一个圆
 * @param  x0    圆心列坐标
 * @param  y0    圆心行坐标
 * @param  r     圆的半径
 * @param  color 填充颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Fill_Circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color);

//----------------- TFT 字符/字符串/数字 显示函数声明 -----------------

#endif
