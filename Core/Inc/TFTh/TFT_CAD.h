#ifndef __TFT_CAD_H__
#define __TFT_CAD_H__

#include <stdint.h>
#include "TFTh/TFT_io.h" // 已包含 main.h

#ifdef __cplusplus
extern "C"
{
#endif

    // 点结构体定义
    /**
     * @brief  结构体用于表示一个二维坐标点
     */
    typedef struct
    {
        uint16_t x; // 点的 X 坐标 (列)
        uint16_t y; // 点的 Y 坐标 (行)
    } TFT_Point;

    // TFT 绘图函数声明

    /**
     * @brief  在指定矩形区域内填充单色
     * @param  htft TFT句柄指针
     * @param  x_start 起始列坐标 (左上角 X)
     * @param  y_start 起始行坐标 (左上角 Y)
     * @param  x_end   结束列坐标 (右下角 X, 不包含)
     * @param  y_end   结束行坐标 (右下角 Y, 不包含)
     * @param  color   要填充的颜色 (RGB565格式)
     * @retval 无
     */
    void TFT_Fill_Area(TFT_HandleTypeDef *htft, uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t color);

    /**
     * @brief  在指定坐标绘制一个像素点 (使用阻塞方式)
     * @param  htft TFT句柄指针
     * @param  x     点的列坐标
     * @param  y     点的行坐标
     * @param  color 点的颜色 (RGB565格式)
     * @retval 无
     * @note   此函数效率较低，不适合大量绘制。绘制后不自动刷新缓冲区。
     */
    void TFT_Draw_Point(TFT_HandleTypeDef *htft, uint16_t x, uint16_t y, uint16_t color);

    /**
     * @brief  绘制多个离散的点 (利用缓冲区和 DMA 加速)
     * @param  htft TFT句柄指针
     * @param  points 点坐标数组 (TFT_Point 类型)
     * @param  count  点的数量
     * @param  color  所有点的颜色 (RGB565格式)
     * @retval 无
     */
    void TFT_Draw_MultiPoint(TFT_HandleTypeDef *htft, const TFT_Point points[], uint16_t count, uint16_t color);

    /**
     * @brief  快速绘制水平线 (利用缓冲区和 DMA 加速)
     * @param  htft TFT句柄指针
     * @param  x     起始列坐标
     * @param  y     行坐标
     * @param  width 线条宽度 (像素数)
     * @param  color 线的颜色 (RGB565格式)
     * @retval 无
     */
    void TFT_Draw_Fast_HLine(TFT_HandleTypeDef *htft, uint16_t x, uint16_t y, uint16_t width, uint16_t color);

    /**
     * @brief  快速绘制垂直线 (利用缓冲区和 DMA 加速)
     * @param  htft TFT句柄指针
     * @param  x      列坐标
     * @param  y      起始行坐标
     * @param  height 线条高度 (像素数)
     * @param  color  线的颜色 (RGB565格式)
     * @retval 无
     */
    void TFT_Draw_Fast_VLine(TFT_HandleTypeDef *htft, uint16_t x, uint16_t y, uint16_t height, uint16_t color);

    /**
     * @brief  绘制任意两点之间的直线 (Bresenham 算法)
     * @param  htft TFT句柄指针
     * @param  x1    起点列坐标
     * @param  y1    起点行坐标
     * @param  x2    终点列坐标
     * @param  y2    终点行坐标
     * @param  color 线的颜色 (RGB565格式)
     * @retval 无
     * @note   内部优化了水平和垂直线，斜线绘制效率相对较低。
     */
    void TFT_Draw_Line(TFT_HandleTypeDef *htft, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

    /**
     * @brief  绘制一个空心矩形框
     * @param  htft TFT句柄指针
     * @param  x1    左上角列坐标
     * @param  y1    左上角行坐标
     * @param  x2    右下角列坐标
     * @param  y2    右下角行坐标
     * @param  color 矩形边框颜色 (RGB565格式)
     * @retval 无
     */
    void TFT_Draw_Rectangle(TFT_HandleTypeDef *htft, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

    /**
     * @brief  填充一个实心矩形
     * @param  htft TFT句柄指针
     * @param  x1    左上角列坐标
     * @param  y1    左上角行坐标
     * @param  x2    右下角列坐标
     * @param  y2    右下角行坐标
     * @param  color 矩形填充颜色 (RGB565格式)
     * @retval 无
     * @note   内部调用 TFT_Fill_Area 实现。
     */
    void TFT_Fill_Rectangle(TFT_HandleTypeDef *htft, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

    /**
     * @brief  绘制四分之一圆弧 (使用 Bresenham 算法)
     * @param  htft TFT句柄指针
     * @param  centerX, centerY 圆弧所在圆的圆心坐标
     * @param  radius   圆弧半径
     * @param  cornerMask 指定绘制哪个角落 (位掩码: 1=右上, 2=右下, 4=左下, 8=左上)
     * @param  color    颜色
     * @retval 无
     */
    void TFT_Draw_Quarter_Circle(TFT_HandleTypeDef *htft, uint16_t centerX, uint16_t centerY, uint8_t radius, uint8_t cornerMask, uint16_t color);

    /**
     * @brief  填充实心四分之一圆弧区域 (使用水平线)
     * @param  htft TFT句柄指针
     * @param  centerX, centerY 圆弧所在圆的圆心坐标
     * @param  radius   圆弧半径
     * @param  cornerMask 指定填充哪个角落 (位掩码: 1=右上, 2=右下, 4=左下, 8=左上)
     * @param  color    颜色
     * @retval 无
     */
    void TFT_Fill_Quarter_Circle(TFT_HandleTypeDef *htft, uint16_t centerX, uint16_t centerY, uint8_t radius, uint8_t cornerMask, uint16_t color);

    /**
     * @brief  绘制一个空心圆 (中点画圆法 / Bresenham 画圆法)
     * @param  htft TFT句柄指针
     * @param  x0    圆心列坐标
     * @param  y0    圆心行坐标
     * @param  r     圆的半径
     * @param  color 圆的颜色 (RGB565格式)
     * @retval 无
     */
    void TFT_Draw_Circle(TFT_HandleTypeDef *htft, uint16_t x0, uint16_t y0, uint8_t r, uint16_t color);

    /**
     * @brief  填充一个实心圆 (水平线扫描法)
     * @param  htft TFT句柄指针
     * @param  x0    圆心列坐标
     * @param  y0    圆心行坐标
     * @param  r     圆的半径
     * @param  color 填充颜色 (RGB565格式)
     * @retval 无
     */
    void TFT_Fill_Circle(TFT_HandleTypeDef *htft, uint16_t x0, uint16_t y0, uint8_t r, uint16_t color);

    /**
     * @brief  绘制一个空心三角形
     * @param  htft TFT句柄指针
     * @param  x1, y1 第一个顶点坐标
     * @param  x2, y2 第二个顶点坐标
     * @param  x3, y3 第三个顶点坐标
     * @param  color  三角形边框颜色 (RGB565格式)
     * @retval 无
     */
    void TFT_Draw_Triangle(TFT_HandleTypeDef *htft, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color);

    /**
     * @brief  填充一个实心三角形 (水平扫描线算法)
     * @param  htft TFT句柄指针
     * @param  x1, y1 第一个顶点坐标
     * @param  x2, y2 第二个顶点坐标
     * @param  x3, y3 第三个顶点坐标
     * @param  color  三角形填充颜色 (RGB565格式)
     * @retval 无
     */
    void TFT_Fill_Triangle(TFT_HandleTypeDef *htft, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color);

    /**
     * @brief  绘制一个空心圆角矩形
     * @param  htft TFT句柄指针
     * @param  x      左上角列坐标
     * @param  y      左上角行坐标
     * @param  width  矩形宽度
     * @param  height 矩形高度
     * @param  radius 圆角半径
     * @param  color  边框颜色 (RGB565格式)
     * @retval 无
     */
    void TFT_Draw_Rounded_Rectangle(TFT_HandleTypeDef *htft, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t radius, uint16_t color);

    /**
     * @brief  填充一个实心圆角矩形
     * @param  htft TFT句柄指针
     * @param  x      左上角列坐标
     * @param  y      左上角行坐标
     * @param  width  矩形宽度
     * @param  height 矩形高度
     * @param  radius 圆角半径
     * @param  color  填充颜色 (RGB565格式)
     * @retval 无
     */
    void TFT_Fill_Rounded_Rectangle(TFT_HandleTypeDef *htft, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t radius, uint16_t color);

    //----------------- 新增高级CAD绘图函数声明 -----------------

    /**
     * @brief  绘制一个空心椭圆
     * @param  htft TFT句柄指针
     * @param  centerX, centerY 椭圆中心坐标
     * @param  radiusX 椭圆X轴半径
     * @param  radiusY 椭圆Y轴半径
     * @param  color   椭圆颜色 (RGB565格式)
     * @retval 无
     */
    void TFT_Draw_Ellipse(TFT_HandleTypeDef *htft, uint16_t centerX, uint16_t centerY, uint16_t radiusX, uint16_t radiusY, uint16_t color);

    /**
     * @brief  填充一个实心椭圆
     * @param  htft TFT句柄指针
     * @param  centerX, centerY 椭圆中心坐标
     * @param  radiusX 椭圆X轴半径
     * @param  radiusY 椭圆Y轴半径
     * @param  color   填充颜色 (RGB565格式)
     * @retval 无
     */
    void TFT_Fill_Ellipse(TFT_HandleTypeDef *htft, uint16_t centerX, uint16_t centerY, uint16_t radiusX, uint16_t radiusY, uint16_t color);

    /**
     * @brief  绘制二阶贝塞尔曲线
     * @param  htft TFT句柄指针
     * @param  x0, y0 起始点坐标
     * @param  x1, y1 控制点坐标
     * @param  x2, y2 终点坐标
     * @param  segments 曲线分段数(越大越平滑)
     * @param  color 曲线颜色 (RGB565格式)
     * @retval 无
     */
    void TFT_Draw_Bezier2(TFT_HandleTypeDef *htft, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                          uint16_t x2, uint16_t y2, uint8_t segments, uint16_t color);

    /**
     * @brief  绘制空心多边形
     * @param  htft TFT句柄指针
     * @param  points 多边形顶点坐标数组
     * @param  numPoints 顶点数量
     * @param  color 多边形边框颜色
     * @retval 无
     */
    void TFT_Draw_Polygon(TFT_HandleTypeDef *htft, const TFT_Point points[], uint16_t numPoints, uint16_t color);

    /**
     * @brief  填充简单多边形（不处理复杂情况如自交）
     * @param  htft TFT句柄指针
     * @param  points 多边形顶点坐标数组
     * @param  numPoints 顶点数量
     * @param  color 填充颜色
     * @retval 无
     * @note   使用水平扫描线算法，仅适用于简单多边形
     */
    void TFT_Fill_Polygon(TFT_HandleTypeDef *htft, const TFT_Point points[], uint16_t numPoints, uint16_t color);

    /**
     * @brief  绘制圆弧
     * @param  htft TFT句柄指针
     * @param  centerX, centerY 圆心坐标
     * @param  radius 圆弧半径
     * @param  startAngle 起始角度(0-360度)
     * @param  endAngle 结束角度(0-360度)
     * @param  color 圆弧颜色
     * @retval 无
     */
    void TFT_Draw_Arc(TFT_HandleTypeDef *htft, uint16_t centerX, uint16_t centerY, uint8_t radius,
                      uint16_t startAngle, uint16_t endAngle, uint16_t color);

#ifdef __cplusplus
}
#endif

#endif
