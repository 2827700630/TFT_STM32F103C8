/*
 * @file    tft.c
 * @brief   TFT绘图函数。这里的画图是以左上角为原点
 */
#include "TFTh/TFT.h"
#include "TFTh/TFT_io.h" // 包含底层 IO 函数
#include <stdlib.h>		 // 用于 abs 函数

// 宏定义：交换两个 int16_t 变量的值
#define SWAP_INT16(a, b) \
	{                    \
		int16_t t = a;   \
		a = b;           \
		b = t;           \
	}

/*
 * @brief  在指定坐标绘制一个点
 * @param  x     点的列坐标
 * @param  y     点的行坐标
 * @param  color 点的颜色 (RGB565格式)
 * @retval 无
 * @note   优化：移除了不必要的 Flush。
 *         在连续绘制多个点时，调用者应在最后负责 Flush (如果需要)。
 */
void TFT_Draw_Point(uint16_t x, uint16_t y, uint16_t color)
{
	TFT_Set_Address(x, y, x, y); // 设置光标位置到单个点
	TFT_Write_Data16(color);	 // 对于单点，直接写入即可 (阻塞)
}

/*
 * @brief  绘制多个点 (利用缓冲区加速)
 * @param  points 点坐标数组
 * @param  count  点的数量
 * @param  color  点的颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Draw_MultiPoint(const TFT_Point points[], uint16_t count, uint16_t color)
{
	if (points == NULL || count == 0)
	{
		return;
	}

	// 重置缓冲区确保从空的缓冲区开始
	TFT_Reset_Buffer();

	for (uint16_t i = 0; i < count; i++)
	{
		TFT_Set_Address(points[i].x, points[i].y, points[i].x, points[i].y); // 设置单个点地址
		TFT_Buffer_Write16(color);											 // 将颜色写入缓冲区
	}

	// 确保所有缓冲的点都被发送
	TFT_Flush_Buffer(1);
}

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
 * @brief  快速绘制水平线
 * @param  x     起始列坐标
 * @param  y     行坐标
 * @param  width 线宽度
 * @param  color 线的颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Draw_Fast_HLine(uint16_t x, uint16_t y, uint16_t width, uint16_t color)
{
	if (width == 0)
		return;

	TFT_Set_Address(x, y, x + width - 1, y); // 设置地址窗口
	TFT_Reset_Buffer();						 // 重置缓冲区

	// 填充像素到缓冲区
	for (uint16_t i = 0; i < width; i++)
	{
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
	if (height == 0)
		return;

	TFT_Set_Address(x, y, x, y + height - 1); // 设置地址窗口
	TFT_Reset_Buffer();						  // 重置缓冲区

	// 填充像素到缓冲区
	for (uint16_t i = 0; i < height; i++)
	{
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
			SWAP_INT16(x1, x2); // 使用宏交换
		}
		TFT_Draw_Fast_HLine(x1, y1, x2 - x1 + 1, color); // 使用快速水平线函数
		return;
	}

	// 优化：处理垂直线
	if (x1 == x2)
	{
		if (y1 > y2) // 确保 y1 <= y2
		{
			SWAP_INT16(y1, y2); // 使用宏交换
		}
		TFT_Draw_Fast_VLine(x1, y1, y2 - y1 + 1, color); // 使用快速垂直线函数
		return;
	}

	// Bresenham 算法绘制斜线
	int16_t deltaX = abs(x2 - x1);		// X 轴距离绝对值
	int16_t deltaY = abs(y2 - y1);		// Y 轴距离绝对值
	int16_t stepX = (x1 < x2) ? 1 : -1; // X 轴步进方向
	int16_t stepY = (y1 < y2) ? 1 : -1; // Y 轴步进方向
	int16_t currentX = x1;
	int16_t currentY = y1;
	int16_t errorTerm; // 误差项

	// 注意：由于 TFT_Draw_Point 不再 Flush，这里的循环效率会提高
	if (deltaX > deltaY) // 以 X 轴为主轴 (斜率绝对值 < 1)
	{
		errorTerm = deltaX / 2; // 初始误差
		while (currentX != x2)	// 循环直到到达终点 X 坐标
		{
			TFT_Draw_Point(currentX, currentY, color); // 绘制当前点
			errorTerm -= deltaY;
			if (errorTerm < 0)
			{
				currentY += stepY; // Y 移动一步
				errorTerm += deltaX;
			}
			currentX += stepX; // X 移动一步
		}
	}
	else // 以 Y 轴为主轴 (斜率绝对值 >= 1)
	{
		errorTerm = deltaY / 2; // 初始误差
		while (currentY != y2)	// 循环直到到达终点 Y 坐标
		{
			TFT_Draw_Point(currentX, currentY, color); // 绘制当前点
			errorTerm -= deltaX;
			if (errorTerm < 0)
			{
				currentX += stepX; // X 移动一步
				errorTerm += deltaY;
			}
			currentY += stepY; // Y 移动一步
		}
	}
	TFT_Draw_Point(currentX, currentY, color); // 绘制终点 (Bresenham 循环不包含终点)

	// 如果 TFT_Write_Data16 未来改为非阻塞+缓冲，则可能需要在此处添加 Flush
	TFT_Flush_Buffer(1); // 绘制斜线后刷新缓冲区，因为 Draw_Point 不刷新
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
	if (x1 > x2)
	{
		uint16_t temp = x1;
		x1 = x2;
		x2 = temp;
	}

	if (y1 > y2)
	{
		uint16_t temp = y1;
		y1 = y2;
		y2 = temp;
	}

	// 直接调用填充区域函数
	TFT_Fill_Area(x1, y1, x2 + 1, y2 + 1, color);
}

/*
 * @brief  绘制一个空心圆 (中点画圆法)
 * @param  x0    圆心列坐标
 * @param  y0    圆心行坐标
 * @param  r     圆的半径
 * @param  color 圆的颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Draw_Circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color)
{
	int16_t plotX = 0;					  // 相对于圆心的 x 坐标
	int16_t plotY = r;					  // 相对于圆心的 y 坐标
	int16_t decisionParam = 3 - (r << 1); // 初始决策参数: 3 - 2*r
	TFT_Point circlePoints[8];			  // 用于存储对称点的数组

	// 绘制圆的初始四个点 (0, r), (0, -r), (r, 0), (-r, 0)
	// 这些点在循环中不会被绘制，需要单独处理
	if (r > 0)
	{
		TFT_Draw_Point(x0, y0 + r, color);
		TFT_Draw_Point(x0, y0 - r, color);
		TFT_Draw_Point(x0 + r, y0, color);
		TFT_Draw_Point(x0 - r, y0, color);
		TFT_Flush_Buffer(1); // 刷新初始点
	}
	else
	{
		TFT_Draw_Point(x0, y0, color); // 半径为0，只画一个点
		TFT_Flush_Buffer(1);
		return;
	}

	while (plotX < plotY) // 仅需计算八分之一圆弧 (第二象限从 y 轴到 y=x)
	{
		plotX++; // x 增加 1

		// 更新决策参数
		if (decisionParam < 0)
		{
			// 选择 E 点 (x+1, y)
			decisionParam += (plotX << 2) + 6; // decisionParam += 4*plotX + 6
		}
		else
		{
			// 选择 SE 点 (x+1, y-1)
			plotY--;									  // y 减小 1
			decisionParam += ((plotX - plotY) << 2) + 10; // decisionParam += 4*(plotX - plotY) + 10
		}

		// 如果 plotX == plotY，说明到达 y=x 线，只需绘制 4 个点
		if (plotX == plotY)
		{
			circlePoints[0] = (TFT_Point){x0 + plotX, y0 + plotY};
			circlePoints[1] = (TFT_Point){x0 - plotX, y0 + plotY};
			circlePoints[2] = (TFT_Point){x0 + plotX, y0 - plotY};
			circlePoints[3] = (TFT_Point){x0 - plotX, y0 - plotY};
			TFT_Draw_MultiPoint(circlePoints, 4, color); // 批量绘制 4 个点
		}
		else
		{
			// 计算 8 个对称点
			circlePoints[0] = (TFT_Point){x0 + plotX, y0 + plotY}; // 第 4 象限
			circlePoints[1] = (TFT_Point){x0 - plotX, y0 + plotY}; // 第 5 象限
			circlePoints[2] = (TFT_Point){x0 + plotX, y0 - plotY}; // 第 1 象限
			circlePoints[3] = (TFT_Point){x0 - plotX, y0 - plotY}; // 第 8 象限
			circlePoints[4] = (TFT_Point){x0 + plotY, y0 + plotX}; // 第 3 象限
			circlePoints[5] = (TFT_Point){x0 - plotY, y0 + plotX}; // 第 6 象限
			circlePoints[6] = (TFT_Point){x0 + plotY, y0 - plotX}; // 第 2 象限
			circlePoints[7] = (TFT_Point){x0 - plotY, y0 - plotX}; // 第 7 象限
			// 调用 MultiPoint 函数批量绘制这 8 个点
			TFT_Draw_MultiPoint(circlePoints, 8, color);
		}
		// 注意：不需要在这里 Flush，因为 TFT_Draw_MultiPoint 内部会 Flush
	}
}

/*
 * @brief  绘制一个实心圆 (水平线扫描法)
 * @param  x0    圆心列坐标
 * @param  y0    圆心行坐标
 * @param  r     圆的半径
 * @param  color 填充颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Fill_Circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color)
{
	if (r == 0)
	{
		// 处理半径为 0 的情况：绘制一个点
		TFT_Draw_Point(x0, y0, color);
		TFT_Flush_Buffer(1); // 刷新单个点
		return;
	}

	// 绘制实心圆方法：水平线扫描填充 (基于 Bresenham 画圆算法)
	int16_t plotX = 0;					  // 相对于圆心的 x 坐标
	int16_t plotY = r;					  // 相对于圆心的 y 坐标
	int16_t decisionParam = 3 - (r << 1); // 初始决策参数: 3 - 2*r

	// 1. 绘制中心水平线 (对应 x=0, y=r)
	TFT_Draw_Fast_HLine(x0 - r, y0, 2 * r + 1, color);

	// 2. 循环计算圆弧上的点，并绘制对称的水平线填充
	while (plotX < plotY)
	{
		plotX++; // x 增加 1

		// 更新决策参数
		if (decisionParam < 0)
		{
			// 选择 E 点 (x+1, y)
			decisionParam += (plotX << 2) + 6; // decisionParam += 4*plotX + 6
		}
		else
		{
			// 选择 SE 点 (x+1, y-1)
			// 在 y 变化时绘制水平线
			// 绘制基于 (x, y) 对称点的水平线 (较窄的线段)
			// 行坐标: y0 + y 和 y0 - y
			// 列范围: x0 - x 到 x0 + x (宽度 2*x + 1)
			TFT_Draw_Fast_HLine(x0 - plotX, y0 + plotY, 2 * plotX + 1, color);
			TFT_Draw_Fast_HLine(x0 - plotX, y0 - plotY, 2 * plotX + 1, color);
			plotY--;									  // y 减小 1
			decisionParam += ((plotX - plotY) << 2) + 10; // decisionParam += 4*(plotX - plotY) + 10
		}

		// 绘制基于 (y, x) 对称点的水平线 (较宽的线段)
		// 行坐标: y0 + x 和 y0 - x
		// 列范围: x0 - y 到 x0 + y (宽度 2*y + 1)
		TFT_Draw_Fast_HLine(x0 - plotY, y0 + plotX, 2 * plotY + 1, color);
		TFT_Draw_Fast_HLine(x0 - plotY, y0 - plotX, 2 * plotY + 1, color);
	}
	// 注意: Fast_HLine 内部会 Flush，这里不需要额外 Flush
}

/*
 * @brief  绘制一个空心三角形
 * @param  x1, y1 第一个顶点坐标
 * @param  x2, y2 第二个顶点坐标
 * @param  x3, y3 第三个顶点坐标
 * @param  color  三角形边框颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Draw_Triangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color)
{
	TFT_Draw_Line(x1, y1, x2, y2, color); // 绘制第一条边
	TFT_Draw_Line(x2, y2, x3, y3, color); // 绘制第二条边
	TFT_Draw_Line(x3, y3, x1, y1, color); // 绘制第三条边
}

/*
 * @brief  填充一个实心三角形 (水平扫描线算法)
 * @param  x1, y1 第一个顶点坐标
 * @param  x2, y2 第二个顶点坐标
 * @param  x3, y3 第三个顶点坐标
 * @param  color  三角形填充颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Fill_Triangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color)
{
	int16_t scanlineStartX, scanlineEndX;						  // 当前扫描线填充的起始和结束 X 坐标
	int16_t currentY, lastY;									  // 当前扫描线 Y 坐标, 结束 Y 坐标
	int16_t deltaX1, deltaY1, deltaX2, deltaY2, deltaX3, deltaY3; // 三条边的 X, Y 差值
	int32_t edge1Accumulator, edge2Accumulator;					  // 边斜率累加器 (使用 int32_t 防止溢出)

	// 1. 按 Y 坐标对顶点进行排序 (y1 <= y2 <= y3)
	if (y1 > y2)
	{
		SWAP_INT16(y1, y2);
		SWAP_INT16(x1, x2);
	}
	if (y2 > y3)
	{
		SWAP_INT16(y3, y2);
		SWAP_INT16(x3, x2);
	}
	if (y1 > y2)
	{
		SWAP_INT16(y1, y2);
		SWAP_INT16(x1, x2);
	}

	// 2. 处理特殊情况：水平线或单个点
	if (y1 == y3)
	{
		scanlineStartX = scanlineEndX = x1;
		if (x2 < scanlineStartX)
			scanlineStartX = x2;
		else if (x2 > scanlineEndX)
			scanlineEndX = x2;
		if (x3 < scanlineStartX)
			scanlineStartX = x3;
		else if (x3 > scanlineEndX)
			scanlineEndX = x3;
		TFT_Draw_Fast_HLine(scanlineStartX, y1, scanlineEndX - scanlineStartX + 1, color);
		return;
	}

	// 3. 计算边的差值
	deltaX1 = x2 - x1;
	deltaY1 = y2 - y1; // 边 1->2
	deltaX2 = x3 - x1;
	deltaY2 = y3 - y1; // 边 1->3
	deltaX3 = x3 - x2;
	deltaY3 = y3 - y2; // 边 2->3

	// 初始化斜率累加器
	edge1Accumulator = 0;
	edge2Accumulator = 0;

	// 4. 填充上部分三角形 (从 y1 到 y2-1)
	// 使用边 1->2 和 边 1->3
	if (y1 != y2)
	{					// 只有当 y1 和 y2 不同时才需要填充上部分
		lastY = y2 - 1; // 循环包含 y1, 不包含 y2
		for (currentY = y1; currentY <= lastY; currentY++)
		{
			// 计算当前扫描线与两条边的交点 X 坐标
			// 使用累加器避免浮点运算和重复除法
			scanlineStartX = x1 + (edge1Accumulator / deltaY1);
			scanlineEndX = x1 + (edge2Accumulator / deltaY2);
			// 更新累加器
			edge1Accumulator += deltaX1;
			edge2Accumulator += deltaX2;
			// 确保 scanlineStartX <= scanlineEndX
			if (scanlineStartX > scanlineEndX)
				SWAP_INT16(scanlineStartX, scanlineEndX);
			// 绘制水平扫描线
			TFT_Draw_Fast_HLine(scanlineStartX, currentY, scanlineEndX - scanlineStartX + 1, color);
		}
	}

	// 5. 填充下部分三角形 (从 y2 到 y3)
	// 使用边 2->3 和 边 1->3
	// 重置/调整累加器以匹配新的起始点和边
	edge1Accumulator = (int32_t)deltaX3 * (currentY - y2); // 边 2->3 的累加器，从 y2 开始
	// edge2Accumulator 继续使用边 1->3 的累加器

	if (y2 != y3)
	{				// 只有当 y2 和 y3 不同时才需要填充下部分
		lastY = y3; // 循环包含 y2 和 y3
		for (; currentY <= lastY; currentY++)
		{
			// 计算当前扫描线与两条边的交点 X 坐标
			scanlineStartX = x2 + (edge1Accumulator / deltaY3); // 基于顶点 2 和边 2->3
			scanlineEndX = x1 + (edge2Accumulator / deltaY2);	// 基于顶点 1 和边 1->3
			// 更新累加器
			edge1Accumulator += deltaX3;
			edge2Accumulator += deltaX2;
			// 确保 scanlineStartX <= scanlineEndX
			if (scanlineStartX > scanlineEndX)
				SWAP_INT16(scanlineStartX, scanlineEndX);
			// 绘制水平扫描线
			TFT_Draw_Fast_HLine(scanlineStartX, currentY, scanlineEndX - scanlineStartX + 1, color);
		}
	}
	// 注意: Fast_HLine 内部会 Flush，这里不需要额外 Flush
}

/*
 * @brief  绘制四分之一圆弧 (使用 Bresenham 算法)
 * @param  centerX, centerY 圆弧所在圆的圆心坐标
 * @param  radius   圆弧半径
 * @param  cornerMask 指定绘制哪个角落 (位掩码: 1=右上, 2=右下, 4=左下, 8=左上)
 * @param  color    颜色
 * @retval 无
 */
void TFT_Draw_Quarter_Circle(uint16_t centerX, uint16_t centerY, uint8_t radius, uint8_t cornerMask, uint16_t color)
{
	int16_t plotX = 0;						   // 相对于圆心的 x 坐标
	int16_t plotY = radius;					   // 相对于圆心的 y 坐标
	int16_t decisionParam = 3 - (radius << 1); // 初始决策参数: 3 - 2*r

	// 绘制圆弧的初始点 (坐标轴上的点)
	if (cornerMask == 1) // 右上角
	{
		TFT_Draw_Point(centerX + radius, centerY, color); // (x+r, y)
		TFT_Draw_Point(centerX, centerY - radius, color); // (x, y-r)
	}
	if (cornerMask == 2) // 右下角
	{
		TFT_Draw_Point(centerX + radius, centerY, color); // (x+r, y)
		TFT_Draw_Point(centerX, centerY + radius, color); // (x, y+r)
	}
	if (cornerMask == 4) // 左下角
	{
		TFT_Draw_Point(centerX - radius, centerY, color); // (x-r, y)
		TFT_Draw_Point(centerX, centerY + radius, color); // (x, y+r)
	}
	if (cornerMask == 8) // 左上角
	{
		TFT_Draw_Point(centerX - radius, centerY, color); // (x-r, y)
		TFT_Draw_Point(centerX, centerY - radius, color); // (x, y-r)
	}

	// 循环绘制圆弧上的其他点
	while (plotX < plotY)
	{
		plotX++; // x 增加 1
		if (decisionParam < 0)
		{
			decisionParam += (plotX << 2) + 6;
		}
		else
		{
			plotY--; // y 减小 1
			decisionParam += ((plotX - plotY) << 2) + 10;
		}

		// 根据 cornerMask 绘制对应的圆弧点
		// 检查 plotX != plotY 避免在对角线上重复绘制
		if (plotX != plotY)
		{
			if (cornerMask & 0x1) // 右上角 (第二象限部分: x>0, y<0)
			{
				TFT_Draw_Point(centerX + plotX, centerY - plotY, color); // (x+x, y-y)
				TFT_Draw_Point(centerX + plotY, centerY - plotX, color); // (x+y, y-x)
			}
			if (cornerMask & 0x2) // 右下角 (第三象限部分: x>0, y>0)
			{
				TFT_Draw_Point(centerX + plotY, centerY + plotX, color); // (x+y, y+x)
				TFT_Draw_Point(centerX + plotX, centerY + plotY, color); // (x+x, y+y)
			}
			if (cornerMask & 0x4) // 左下角 (第六象限部分: x<0, y>0)
			{
				TFT_Draw_Point(centerX - plotX, centerY + plotY, color); // (x-x, y+y)
				TFT_Draw_Point(centerX - plotY, centerY + plotX, color); // (x-y, y+x)
			}
			if (cornerMask & 0x8) // 左上角 (第七象限部分: x<0, y<0)
			{
				TFT_Draw_Point(centerX - plotY, centerY - plotX, color); // (x-y, y-x)
				TFT_Draw_Point(centerX - plotX, centerY - plotY, color); // (x-x, y-y)
			}
		}
		else
		{ // plotX == plotY (对角线上的点)
			if (cornerMask & 0x1)
				TFT_Draw_Point(centerX + plotX, centerY - plotY, color);
			if (cornerMask & 0x2)
				TFT_Draw_Point(centerX + plotX, centerY + plotY, color);
			if (cornerMask & 0x4)
				TFT_Draw_Point(centerX - plotX, centerY + plotY, color);
			if (cornerMask & 0x8)
				TFT_Draw_Point(centerX - plotX, centerY - plotY, color);
		}
	}
	// 需要刷新缓冲区，因为 Draw_Point 不刷新
	TFT_Flush_Buffer(1);
}

/*
 * @brief  填充实心四分之一圆弧区域 (使用水平线)
 * @param  centerX, centerY 圆弧所在圆的圆心坐标
 * @param  radius   圆弧半径
 * @param  cornerMask 指定填充哪个角落 (位掩码: 1=右上, 2=右下, 4=左下, 8=左上)
 * @param  color    颜色
 * @retval 无
 * @note   使用水平线填充指定象限的圆弧区域。
 */
void TFT_Fill_Quarter_Circle(uint16_t centerX, uint16_t centerY, uint8_t radius, uint8_t cornerMask, uint16_t color)
{
	int16_t plotX = 0;						   // 相对于圆心的 x 坐标
	int16_t plotY = radius;					   // 相对于圆心的 y 坐标
	int16_t decisionParam = 3 - (radius << 1); // 初始决策参数: 3 - 2*r
	uint16_t hlineWidth;					   // 水平线宽度

	// 填充从圆心开始的水平/垂直线段 (对应坐标轴上的部分)
	if (cornerMask & 0x1) // 右上角
	{
		TFT_Draw_Fast_VLine(centerX, centerY - radius, radius + 1, color); // 垂直线 (x, y-r) to (x, y)
		TFT_Draw_Fast_HLine(centerX, centerY, radius + 1, color);		   // 水平线 (x, y) to (x+r, y)
	}
	if (cornerMask & 0x2) // 右下角
	{
		TFT_Draw_Fast_VLine(centerX, centerY, radius + 1, color);		   // 垂直线 (x, y) to (x, y+r)
		TFT_Draw_Fast_HLine(centerX, centerY, radius + 1, color);		   // 水平线 (x, y) to (x+r, y)
	}
	if (cornerMask & 0x4) // 左下角
	{
		TFT_Draw_Fast_VLine(centerX, centerY, radius + 1, color);		   // 垂直线 (x, y) to (x, y+r)
		TFT_Draw_Fast_HLine(centerX - radius, centerY, radius + 1, color); // 水平线 (x-r, y) to (x, y)
	}
	if (cornerMask & 0x8) // 左上角
	{
		TFT_Draw_Fast_VLine(centerX, centerY - radius, radius + 1, color); // 垂直线 (x, y-r) to (x, y)
		TFT_Draw_Fast_HLine(centerX - radius, centerY, radius + 1, color); // 水平线 (x-r, y) to (x, y)
	}


	while (plotX < plotY)
	{
		plotX++; // x 增加 1
		if (decisionParam < 0)
		{
			decisionParam += (plotX << 2) + 6;
		}
		else
		{
			// 在 y 变化前绘制水平线段 (较窄的部分)
			hlineWidth = plotX + 1;
			if (cornerMask & 0x1) TFT_Draw_Fast_HLine(centerX, centerY - plotY, hlineWidth, color); // 右上
			if (cornerMask & 0x2) TFT_Draw_Fast_HLine(centerX, centerY + plotY, hlineWidth, color); // 右下
			if (cornerMask & 0x4) TFT_Draw_Fast_HLine(centerX - plotX, centerY + plotY, hlineWidth, color); // 左下
			if (cornerMask & 0x8) TFT_Draw_Fast_HLine(centerX - plotX, centerY - plotY, hlineWidth, color); // 左上

			plotY--; // y 减小 1
			decisionParam += ((plotX - plotY) << 2) + 10;
		}

		// 绘制水平线段 (较宽的部分)
		hlineWidth = plotY + 1;
		if (cornerMask & 0x1) TFT_Draw_Fast_HLine(centerX, centerY - plotX, hlineWidth, color); // 右上
		if (cornerMask & 0x2) TFT_Draw_Fast_HLine(centerX, centerY + plotX, hlineWidth, color); // 右下
		if (cornerMask & 0x4) TFT_Draw_Fast_HLine(centerX - plotY, centerY + plotX, hlineWidth, color); // 左下
		if (cornerMask & 0x8) TFT_Draw_Fast_HLine(centerX - plotY, centerY - plotX, hlineWidth, color); // 左上

	}
	// 注意: Fast_HLine/VLine 内部会 Flush，这里不需要额外 Flush
}

/*
 * @brief  绘制一个空心圆角矩形
 * @param  x     左上角列坐标
 * @param  y     左上角行坐标
 * @param  width 宽度
 * @param  height 高度
 * @param  radius 圆角半径
 * @param  color 边框颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Draw_Rounded_Rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t radius, uint16_t color)
{
	if (width == 0 || height == 0)
		return; // 无效尺寸
	// 限制圆角半径不超过宽度和高度的一半
	if (radius > width / 2)
		radius = width / 2;
	if (radius > height / 2)
		radius = height / 2;

	// 绘制直线部分
	// 上边 (从左圆角结束到右圆角开始)
	TFT_Draw_Fast_HLine(x + radius, y, width - 2 * radius, color);
	// 下边 (从左圆角结束到右圆角开始)
	TFT_Draw_Fast_HLine(x + radius, y + height - 1, width - 2 * radius, color);
	// 左边 (从上圆角结束到下圆角开始)
	TFT_Draw_Fast_VLine(x, y + radius, height - 2 * radius, color);
	// 右边 (从上圆角结束到下圆角开始)
	TFT_Draw_Fast_VLine(x + width - 1, y + radius, height - 2 * radius, color);

	// 绘制圆角部分 (调用辅助函数绘制四个角的圆弧)
	// 注意圆心坐标的计算
	TFT_Draw_Quarter_Circle(x + radius, y + radius, radius, 8, color);							// 左上角 (cornerMask=8)
	TFT_Draw_Quarter_Circle(x + width - radius - 1, y + radius, radius, 1, color);				// 右上角 (cornerMask=1)
	TFT_Draw_Quarter_Circle(x + width - radius - 1, y + height - radius - 1, radius, 2, color); // 右下角 (cornerMask=2)
	TFT_Draw_Quarter_Circle(x + radius, y + height - radius - 1, radius, 4, color);				// 左下角 (cornerMask=4)
}

/*
 * @brief  填充一个实心圆角矩形
 * @param  x     左上角列坐标
 * @param  y     左上角行坐标
 * @param  width 宽度
 * @param  height 高度
 * @param  radius 圆角半径
 * @param  color 填充颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Fill_Rounded_Rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t radius, uint16_t color)
{
	if (width == 0 || height == 0)
		return; // 无效尺寸
	// 限制圆角半径不超过宽度和高度的一半
	if (radius > width / 2)
		radius = width / 2;
	if (radius > height / 2)
		radius = height / 2;

	// 1. 填充中心矩形区域 (不包含圆角部分)
	TFT_Fill_Rectangle(x + radius, y, x + width - radius - 1, y + height - 1, color);

	// 2. 填充左右两侧，圆角上下的矩形区域
	// 左侧矩形 (x, y+radius) 到 (x+radius-1, y+height-radius-1)
	TFT_Fill_Rectangle(x, y + radius, x + radius - 1, y + height - radius - 1, color);
	// 右侧矩形 (x+width-radius, y+radius) 到 (x+width-1, y+height-radius-1)
	TFT_Fill_Rectangle(x + width - radius, y + radius, x + width - 1, y + height - radius - 1, color);

	// 3. 填充四个圆角区域 (调用辅助函数)
	// 注意圆心坐标和 cornerMask
	TFT_Fill_Quarter_Circle(x + radius, y + radius, radius, 8, color);						   // 左上角
	TFT_Fill_Quarter_Circle(x + width - radius - 1, y + radius, radius, 1, color);			   // 右上角
	TFT_Fill_Quarter_Circle(x + width - radius - 1, y + height - radius - 1, radius, 2, color); // 右下角
	TFT_Fill_Quarter_Circle(x + radius, y + height - radius - 1, radius, 4, color);			   // 左下角

	// 确保所有缓冲数据都被发送 (如果 Fill_Quarter_Circle 内部没有完全刷新)
	// TFT_Flush_Buffer(1); // Fill_Rectangle 和 Fast_HLine/VLine 内部会刷新，可能不需要
}
