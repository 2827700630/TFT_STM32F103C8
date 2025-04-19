/*
 * @file    tft.c
 * @brief   TFT绘图及显示文字图片函数
 */
#include "TFTh/TFT.h"
#include "TFTh/TFT_io.h" // 包含底层 IO 函数

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
								 // TFT_Flush_Buffer(1);      // REMOVED: 移除不必要的刷新
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

	// Bresenham 算法绘制斜线
	int dx_abs = (x2 > x1) ? (x2 - x1) : (x1 - x2); // x 距离绝对值
	int dy_abs = (y2 > y1) ? (y2 - y1) : (y1 - y2); // y 距离绝对值
	int step_x = (x1 < x2) ? 1 : -1;				// x 步进方向
	int step_y = (y1 < y2) ? 1 : -1;				// y 步进方向
	int current_x = x1;
	int current_y = y1;
	int error_term; // 误差项

	// 注意：由于 TFT_Draw_Point 不再 Flush，这里的循环效率会提高
	if (dx_abs > dy_abs) // 以 x 为主轴
	{
		error_term = dx_abs >> 1; // 初始误差
		while (current_x != x2)	  // 循环直到到达终点 x 坐标
		{
			TFT_Draw_Point(current_x, current_y, color); // 绘制当前点
			error_term -= dy_abs;
			if (error_term < 0)
			{
				current_y += step_y; // y 移动一步
				error_term += dx_abs;
			}
			current_x += step_x; // x 移动一步
		}
	}
	else // 以 y 为主轴
	{
		error_term = dy_abs >> 1; // 初始误差
		while (current_y != y2)	  // 循环直到到达终点 y 坐标
		{
			TFT_Draw_Point(current_x, current_y, color); // 绘制当前点
			error_term -= dx_abs;
			if (error_term < 0)
			{
				current_x += step_x; // x 移动一步
				error_term += dy_abs;
			}
			current_y += step_y; // y 移动一步
		}
	}
	TFT_Draw_Point(current_x, current_y, color); // 绘制终点 (Bresenham 循环不包含终点)

	// 如果 TFT_Write_Data16 未来改为非阻塞+缓冲，则可能需要在此处添加 Flush
	// TFT_Flush_Buffer(1); // Optional: Flush if needed in the future
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
 * @brief  绘制一个圆 (中点画圆法)
 * @param  x0    圆心列坐标
 * @param  y0    圆心行坐标
 * @param  r     圆的半径
 * @param  color 圆的颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Draw_Circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color)
{
	int plot_x = 0;					   // 相对于圆心的 x 坐标
	int plot_y = r;					   // 相对于圆心的 y 坐标
	int decision_param = 3 - (r << 1); // 初始决策值: 3 - 2*r
	TFT_Point circle_points[8];		   // 用于存储对称点的数组

	while (plot_x <= plot_y)
	{
		// 计算 8 个对称点
		circle_points[0] = (TFT_Point){x0 + plot_x, y0 - plot_y}; // Octant 1
		circle_points[1] = (TFT_Point){x0 - plot_x, y0 - plot_y}; // Octant 8
		circle_points[2] = (TFT_Point){x0 + plot_y, y0 - plot_x}; // Octant 2
		circle_points[3] = (TFT_Point){x0 - plot_y, y0 - plot_x}; // Octant 7
		circle_points[4] = (TFT_Point){x0 + plot_y, y0 + plot_x}; // Octant 3
		circle_points[5] = (TFT_Point){x0 - plot_y, y0 + plot_x}; // Octant 6
		circle_points[6] = (TFT_Point){x0 + plot_x, y0 + plot_y}; // Octant 4
		circle_points[7] = (TFT_Point){x0 - plot_x, y0 + plot_y}; // Octant 5

		// 调用 MultiPoint 函数批量绘制这 8 个点
		TFT_Draw_MultiPoint(circle_points, 8, color);

		plot_x++;

		// 更新决策参数
		if (decision_param < 0)
		{
			decision_param += (plot_x << 2) + 6; // decision_param += 4*plot_x + 6
		}
		else
		{
			decision_param += ((plot_x - plot_y) << 2) + 10; // decision_param += 4*(plot_x - plot_y) + 10
			plot_y--;
		}
	}
	// 注意：不需要在这里 Flush，因为 TFT_Draw_MultiPoint 内部会 Flush
}

/*
 * @brief  填充一个圆 (修复中心线未填充问题)
 * @param  x0    圆心列坐标
 * @param  y0    圆心行坐标
 * @param  r     圆的半径
 * @param  color 填充颜色 (RGB565格式)
 * @retval 无
 * @note   修改了填充逻辑，先绘制中心水平线，然后循环绘制两侧的水平线。
 *         移除了中心垂直线绘制，避免潜在冲突。
 */
void TFT_Fill_Circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color)
{
	if (r == 0) {
		// 处理半径为 0 的情况：绘制一个点或宽度为 1 的水平线
		TFT_Draw_Fast_HLine(x0, y0, 1, color);
		return;
	}

	// 绘制实心圆方法：水平线扫描填充 (修改版)
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	// 1. 绘制中心水平线 (对应 x=0, y=r)
	TFT_Draw_Fast_HLine(x0 - r, y0, 2 * r + 1, color);

	// 2. 循环计算圆弧上的点，并绘制对称的水平线填充
	while (x < y)
	{
		// Bresenham 算法更新 x, y
		if (f >= 0)
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		// 绘制基于 (y, x) 对称点的水平线 (较宽的线段)
		// y 行坐标: y0 + x 和 y0 - x
		// x 范围: x0 - y 到 x0 + y (宽度 2*y + 1)
		TFT_Draw_Fast_HLine(x0 - y, y0 + x, 2 * y + 1, color);
		TFT_Draw_Fast_HLine(x0 - y, y0 - x, 2 * y + 1, color);

		// 绘制基于 (x, y) 对称点的水平线 (较窄的线段)
		// y 行坐标: y0 + y 和 y0 - y
		// x 范围: x0 - x 到 x0 + x (宽度 2*x + 1)
		TFT_Draw_Fast_HLine(x0 - x, y0 + y, 2 * x + 1, color);
		TFT_Draw_Fast_HLine(x0 - x, y0 - y, 2 * x + 1, color);
	}
}
