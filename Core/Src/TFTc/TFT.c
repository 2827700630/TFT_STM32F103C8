/*
 * @file    tft.c
 * @brief   TFT绘图及显示文字图片函数
 */
#include "TFTh/TFT.h"
#include "TFTh/TFT_io.h" // 包含底层 IO 函数

/*
 * @brief  在指定区域填充单色
 * @param  x_start 起始列坐标
 * @param  y_start 起始行坐标
 * @param  x_end   结束列坐标 (不包含)
 * @param  y_end   结束行坐标 (不包含)
 * @param  color   要填充的颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Fill_Area(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t color)
{
	// 检查坐标有效性，防止 x_end <= x_start 或 y_end <= y_start
	if (x_end <= x_start || y_end <= y_start)
		return;

	uint32_t total_pixels = (uint32_t)(x_end - x_start) * (y_end - y_start);
	if (total_pixels == 0)
		return;

	TFT_Set_Address(x_start, y_start, x_end - 1, y_end - 1); // 设置显示范围 (Set_Address 使用包含的坐标)
    
    // 重置缓冲区以确保从一个干净的缓冲区开始
    TFT_Reset_Buffer();

    // 批量填充像素，利用缓冲区提高性能
	for (uint32_t i = 0; i < total_pixels; i++)
	{
		TFT_Buffer_Write16(color); // 写入颜色数据到缓冲区，缓冲区满时会自动发送
	}
    
    // 确保所有剩余数据都被发送
    TFT_Flush_Buffer(1);
}

/*
 * @brief  在指定坐标绘制一个点
 * @param  x     点的列坐标
 * @param  y     点的行坐标
 * @param  color 点的颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Draw_Point(uint16_t x, uint16_t y, uint16_t color)
{
	TFT_Set_Address(x, y, x, y); // 设置光标位置到单个点
	TFT_Write_Data16(color);     // 对于单点，直接写入即可
    TFT_Flush_Buffer(1);         // 确保数据被发送
}

/*
 * @brief  快速绘制水平线
 * @param  x     起始列坐标
 * @param  y     行坐标
 * @param  width 线宽度
 * @param  color 线的颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Draw_Fast_HLine(uint16_t x, uint16_t y, uint16_t width, uint16_t color)
{
    if (width == 0) return;
    
    TFT_Set_Address(x, y, x + width - 1, y); // 设置地址窗口
    TFT_Reset_Buffer(); // 重置缓冲区
    
    // 填充像素到缓冲区
    for (uint16_t i = 0; i < width; i++) {
        TFT_Buffer_Write16(color);
    }
    
    TFT_Flush_Buffer(1); // 发送并等待完成
}

/*
 * @brief  快速绘制垂直线
 * @param  x     列坐标
 * @param  y     起始行坐标
 * @param  height 线高度
 * @param  color 线的颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Draw_Fast_VLine(uint16_t x, uint16_t y, uint16_t height, uint16_t color)
{
    if (height == 0) return;
    
    TFT_Set_Address(x, y, x, y + height - 1); // 设置地址窗口
    TFT_Reset_Buffer(); // 重置缓冲区
    
    // 填充像素到缓冲区
    for (uint16_t i = 0; i < height; i++) {
        TFT_Buffer_Write16(color);
    }
    
    TFT_Flush_Buffer(1); // 发送并等待完成
}

/*
 * @brief  绘制一条直线 (Bresenham算法, 优化水平/垂直线)
 * @param  x1    起点列坐标
 * @param  y1    起点行坐标
 * @param  x2    终点列坐标
 * @param  y2    终点行坐标
 * @param  color 线的颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Draw_Line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	// 优化：处理水平线
	if (y1 == y2)
	{
		if (x1 > x2) // 确保 x1 <= x2
		{
			uint16_t temp = x1;
			x1 = x2;
			x2 = temp;
		}
		TFT_Draw_Fast_HLine(x1, y1, x2 - x1 + 1, color); // 使用快速水平线函数
		return;
	}

	// 优化：处理垂直线
	if (x1 == x2)
	{
		if (y1 > y2) // 确保 y1 <= y2
		{
			uint16_t temp = y1;
			y1 = y2;
			y2 = temp;
		}
		TFT_Draw_Fast_VLine(x1, y1, y2 - y1 + 1, color); // 使用快速垂直线函数
		return;
	}

	// 对于斜线，我们需要优化Bresenham算法实现
	int dx = x2 - x1;
	int dy = y2 - y1;
	int ux = ((dx > 0) << 1) - 1; // x步进方向: 1或-1
	int uy = ((dy > 0) << 1) - 1; // y步进方向: 1或-1
	int x = x1, y = y1, eps;
	
	dx = (dx > 0 ? dx : -dx); // 取绝对值
	dy = (dy > 0 ? dy : -dy);

	// 使用缓冲区批量绘制点的方法
	// 对于线条，我们可能需要频繁设置地址，使用基于点的方法
	if (dx > dy)
	{
		eps = dx >> 1;
		while (x != x2) // 循环直到到达终点 x 坐标
		{
			TFT_Draw_Point(x, y, color);
			eps -= dy;
			if (eps < 0)
			{
				y += uy;
				eps += dx;
			}
			x += ux;
		}
	}
	else
	{
		eps = dy >> 1;
		while (y != y2) // 循环直到到达终点 y 坐标
		{
			TFT_Draw_Point(x, y, color);
			eps -= dx;
			if (eps < 0)
			{
				x += ux;
				eps += dy;
			}
			y += uy;
		}
	}
	TFT_Draw_Point(x, y, color); // 绘制终点 (Bresenham 循环不包含终点)
}

/*
 * @brief  绘制一个空心矩形
 * @param  x1    左上角列坐标
 * @param  y1    左上角行坐标
 * @param  x2    右下角列坐标
 * @param  y2    右下角行坐标
 * @param  color 矩形边框颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Draw_Rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	TFT_Draw_Fast_HLine(x1, y1, x2 - x1 + 1, color); // 上边
	TFT_Draw_Fast_HLine(x1, y2, x2 - x1 + 1, color); // 下边
	TFT_Draw_Fast_VLine(x1, y1, y2 - y1 + 1, color); // 左边
	TFT_Draw_Fast_VLine(x2, y1, y2 - y1 + 1, color); // 右边
}

/*
 * @brief  填充一个实心矩形
 * @param  x1    左上角列坐标
 * @param  y1    左上角行坐标
 * @param  x2    右下角列坐标
 * @param  y2    右下角行坐标
 * @param  color 矩形填充颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Fill_Rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    if (x1 > x2) {
        uint16_t temp = x1;
        x1 = x2;
        x2 = temp;
    }
    
    if (y1 > y2) {
        uint16_t temp = y1;
        y1 = y2;
        y2 = temp;
    }
    
    // 直接调用填充区域函数
    TFT_Fill_Area(x1, y1, x2 + 1, y2 + 1, color);
}

/*
 * @brief  绘制一个圆 (中点画圆法)
 * @param  x0    圆心列坐标
 * @param  y0    圆心行坐标
 * @param  r     圆的半径
 * @param  color 圆的颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Draw_Circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color)
{
	int a = 0, b = r;
	int di = 3 - (r << 1); // 初始决策值

	while (a <= b)
	{
		TFT_Draw_Point(x0 + a, y0 - b, color); // 5
		TFT_Draw_Point(x0 - a, y0 - b, color); // 4
		TFT_Draw_Point(x0 + b, y0 - a, color); // 6
		TFT_Draw_Point(x0 - b, y0 - a, color); // 3
		TFT_Draw_Point(x0 + b, y0 + a, color); // 1
		TFT_Draw_Point(x0 - b, y0 + a, color); // 2
		TFT_Draw_Point(x0 + a, y0 + b, color); // 0
		TFT_Draw_Point(x0 - a, y0 + b, color); // 7
		a++;
		// 使用决策参数 di 更新下一个点的位置
		if (di < 0)
		{
			di += (a << 2) + 6;
		}
		else
		{
			di += 10 + ((a - b) << 2);
			b--;
		}
	}
}

/*
 * @brief  填充一个圆
 * @param  x0    圆心列坐标
 * @param  y0    圆心行坐标
 * @param  r     圆的半径
 * @param  color 填充颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Fill_Circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color)
{
    // 绘制实心圆方法：水平线扫描填充
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;
    
    // 绘制垂直中心线
    TFT_Draw_Fast_VLine(x0, y0 - r, 2 * r + 1, color);
    
    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        
        // 用水平线填充两侧
        TFT_Draw_Fast_HLine(x0 - x, y0 + y, 2 * x + 1, color);
        TFT_Draw_Fast_HLine(x0 - x, y0 - y, 2 * x + 1, color);
        TFT_Draw_Fast_HLine(x0 - y, y0 + x, 2 * y + 1, color);
        TFT_Draw_Fast_HLine(x0 - y, y0 - x, 2 * y + 1, color);
    }
}