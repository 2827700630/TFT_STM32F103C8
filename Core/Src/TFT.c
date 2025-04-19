/*
 * @file    tft.c
 * @brief   TFT绘图及显示函数
 */
#include "TFT/TFT.h"
#include "TFT/TFT_init.h"
#include "TFT/TFTfont.h"

/*
 * @brief  在指定区域填充单色
 * @param  x_start 起始列坐标
 * @param  y_start 起始行坐标
 * @param  x_end   结束列坐标
 * @param  y_end   结束行坐标
 * @param  color   要填充的颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Fill_Area(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t color)
{
	uint16_t i, j;
	TFT_Set_Address(x_start, y_start, x_end - 1, y_end - 1); // 设置显示范围 (结束坐标是包含的，所以减1)
	for (i = y_start; i < y_end; i++)
	{
		for (j = x_start; j < x_end; j++)
		{
			TFT_Write_Data16(color); // 写入颜色数据
		}
	}
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
	TFT_Write_Data16(color);
}

/*
 * @brief  绘制一条直线 (Bresenham算法)
 * @param  x1    起点列坐标
 * @param  y1    起点行坐标
 * @param  x2    终点列坐标
 * @param  y2    终点行坐标
 * @param  color 线的颜色 (RGB565格式)
 * @retval 无
 */
void TFT_Draw_Line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	int dx = x2 - x1;
	int dy = y2 - y1;
	int ux = ((dx > 0) << 1) - 1; // x步进方向: 1或-1
	int uy = ((dy > 0) << 1) - 1; // y步进方向: 1或-1
	int x = x1, y = y1, eps;

	dx = (dx > 0 ? dx : -dx); // 取绝对值
	dy = (dy > 0 ? dy : -dy);

	if (dx > dy)
	{
		eps = dx >> 1;
		while (x != x2)
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
		while (y != y2)
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
	TFT_Draw_Point(x, y, color); // 绘制终点
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
	TFT_Draw_Line(x1, y1, x2, y1, color); // 上边
	TFT_Draw_Line(x1, y1, x1, y2, color); // 左边
	TFT_Draw_Line(x1, y2, x2, y2, color); // 下边
	TFT_Draw_Line(x2, y1, x2, y2, color); // 右边
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
 * @brief  显示一个中文字符串 (自动根据字号调用相应函数)
 * @param  x      起始列坐标
 * @param  y      起始行坐标
 * @param  str    要显示的UTF-8编码的中文字符串指针 (每个汉字占3字节)
 * @param  fg_color 字体颜色
 * @param  bg_color 背景颜色
 * @param  font_size 字号 (支持 16, 24, 32)
 * @param  mode   显示模式: 0=背景不透明, 1=背景透明 (叠加显示)
 * @retval 无
 * @note   确保传入的字符串是UTF-8编码，并且字库文件包含对应的汉字。
 *         GBK/GB2312编码的汉字需要先转换为UTF-8。
 *         此函数假设每个汉字在字库中占用的字节数与字号相关。
 */
void TFT_Show_Chinese_String(uint16_t x, uint16_t y, uint8_t *str, uint16_t fg_color, uint16_t bg_color, uint8_t font_size, uint8_t mode)
{
	while (*str != 0)
	{
		// 检查是否为ASCII字符 (简单判断)
		if (*str < 0x80)
		{ // 假设是ASCII
			TFT_Show_Char(x, y, *str, fg_color, bg_color, font_size, mode);
			x += font_size / 2; // ASCII字符宽度通常为字号一半
			str++;
		}
		else
		{ // 假设是多字节字符 (UTF-8汉字通常3字节)
			if (font_size == 16)
				TFT_Show_Chinese_16x16(x, y, str, fg_color, bg_color, font_size, mode);
			else if (font_size == 24)
				TFT_Show_Chinese_24x24(x, y, str, fg_color, bg_color, font_size, mode);
			else if (font_size == 32)
				TFT_Show_Chinese_32x32(x, y, str, fg_color, bg_color, font_size, mode);
			else
				return;		// 不支持的字号
			str += 3;		// UTF-8 汉字通常占3字节
			x += font_size; // 移动到下一个字符位置
		}
	}
}

/*
 * @brief  显示一个16x16的汉字
 * @param  x      起始列坐标
 * @param  y      起始行坐标
 * @param  hz_ptr 指向要显示的汉字内码(UTF-8编码，通常3字节)的指针
 * @param  fg_color 字体颜色
 * @param  bg_color 背景颜色
 * @param  font_size 字号 (固定为16)
 * @param  mode   显示模式: 0=背景不透明, 1=背景透明 (叠加显示)
 * @retval 无
 * @note   字库 tfont16 必须包含对应的汉字点阵数据。
 *         字库索引 Index[] 存储的是汉字的内码。
 */
void TFT_Show_Chinese_16x16(uint16_t x, uint16_t y, uint8_t *hz_ptr, uint16_t fg_color, uint16_t bg_color, uint8_t font_size, uint8_t mode)
{
	uint8_t i, j;
	uint16_t k;
	uint16_t hz_count;		 // 汉字库中的汉字数量
	uint16_t bytes_per_char; // 每个汉字点阵占用的字节数
	uint16_t x0 = x;

	bytes_per_char = (font_size / 8) * font_size;	  // 计算点阵字节数 (16x16 -> 32 bytes)
	hz_count = sizeof(tfont16) / sizeof(typFNT_GB16); // 统计汉字库中的汉字数量

	for (k = 0; k < hz_count; k++)
	{
		// 比较字库中的索引和传入的汉字内码 (假设字库索引是GBK/GB2312的2字节)
		// 注意：这里需要根据字库实际存储的编码来匹配！
		// 如果字库是GBK/GB2312，而传入的是UTF-8，需要转换或修改匹配逻辑。
		// 假设 tfont16 的 Index 存储的是 GBK/GB2312 内码 (2字节)
		// 这是一个潜在的问题点，需要确认字库格式和输入编码一致性
		if ((tfont16[k].Index[0] == *(hz_ptr)) && (tfont16[k].Index[1] == *(hz_ptr + 1))) // 假设匹配GBK/GB2312前两字节
		{
			TFT_Set_Address(x, y, x + font_size - 1, y + font_size - 1);
			for (i = 0; i < bytes_per_char; i++)
			{
				uint8_t font_byte = tfont16[k].Msk[i]; // 获取点阵数据字节
				for (j = 0; j < 8; j++)
				{
					if (!mode) // 非叠加模式 (绘制背景)
					{
						if (font_byte & (0x80 >> j)) // 从高位到低位检查像素点
						{
							TFT_Write_Data16(fg_color);
						}
						else
						{
							TFT_Write_Data16(bg_color);
						}
					}
					else // 叠加模式 (透明背景)
					{
						if (font_byte & (0x80 >> j)) // 只绘制字体本身的像素点
						{
							TFT_Draw_Point(x, y, fg_color);
						}
						x++;					   // 移动到下一个像素位置
						if ((x - x0) == font_size) // 当前行绘制完成
						{
							x = x0; // 回到行首
							y++;	// 移动到下一行
							break;	// 当前字节处理完毕，跳到下一个字节
						}
					}
				}
			}
			return; // 找到并显示汉字后退出函数
		}
	}
	// 如果字库中没有找到该汉字，可以选择显示一个替代字符或不显示
}

/*
 * @brief  显示一个24x24的汉字
 * @param  x      起始列坐标
 * @param  y      起始行坐标
 * @param  hz_ptr 指向要显示的汉字内码(UTF-8编码，通常3字节)的指针
 * @param  fg_color 字体颜色
 * @param  bg_color 背景颜色
 * @param  font_size 字号 (固定为24)
 * @param  mode   显示模式: 0=背景不透明, 1=背景透明 (叠加显示)
 * @retval 无
 * @note   字库 tfont24 必须包含对应的汉字点阵数据。
 *         同样需要注意字库索引与输入编码的匹配问题。
 */
void TFT_Show_Chinese_24x24(uint16_t x, uint16_t y, uint8_t *hz_ptr, uint16_t fg_color, uint16_t bg_color, uint8_t font_size, uint8_t mode)
{
	uint8_t i, j;
	uint16_t k;
	uint16_t hz_count;		 // 汉字库中的汉字数量
	uint16_t bytes_per_char; // 每个汉字点阵占用的字节数
	uint16_t x0 = x;

	bytes_per_char = (font_size / 8) * font_size;	  // 计算点阵字节数 (24x24 -> 72 bytes)
	hz_count = sizeof(tfont24) / sizeof(typFNT_GB24); // 统计汉字库中的汉字数量

	for (k = 0; k < hz_count; k++)
	{
		// 假设 tfont24 的 Index 存储的是 GBK/GB2312 内码 (2字节)
		if ((tfont24[k].Index[0] == *(hz_ptr)) && (tfont24[k].Index[1] == *(hz_ptr + 1))) // 假设匹配GBK/GB2312前两字节
		{
			TFT_Set_Address(x, y, x + font_size - 1, y + font_size - 1);
			for (i = 0; i < bytes_per_char; i++)
			{
				uint8_t font_byte = tfont24[k].Msk[i];
				for (j = 0; j < 8; j++)
				{
					if (!mode)
					{
						if (font_byte & (0x80 >> j))
						{
							TFT_Write_Data16(fg_color);
						}
						else
						{
							TFT_Write_Data16(bg_color);
						}
					}
					else
					{
						if (font_byte & (0x80 >> j))
						{
							TFT_Draw_Point(x, y, fg_color);
						}
						x++;
						if ((x - x0) == font_size)
						{
							x = x0;
							y++;
							break;
						}
					}
				}
			}
			return;
		}
	}
}

/*
 * @brief  显示一个32x32的汉字
 * @param  x      起始列坐标
 * @param  y      起始行坐标
 * @param  hz_ptr 指向要显示的汉字内码(UTF-8编码，通常3字节)的指针
 * @param  fg_color 字体颜色
 * @param  bg_color 背景颜色
 * @param  font_size 字号 (固定为32)
 * @param  mode   显示模式: 0=背景不透明, 1=背景透明 (叠加显示)
 * @retval 无
 * @note   字库 tfont32 必须包含对应的汉字点阵数据。
 *         同样需要注意字库索引与输入编码的匹配问题。
 */
void TFT_Show_Chinese_32x32(uint16_t x, uint16_t y, uint8_t *hz_ptr, uint16_t fg_color, uint16_t bg_color, uint8_t font_size, uint8_t mode)
{
	uint8_t i, j;
	uint16_t k;
	uint16_t hz_count;		 // 汉字库中的汉字数量
	uint16_t bytes_per_char; // 每个汉字点阵占用的字节数
	uint16_t x0 = x;

	bytes_per_char = (font_size / 8) * font_size;	  // 计算点阵字节数 (32x32 -> 128 bytes)
	hz_count = sizeof(tfont32) / sizeof(typFNT_GB32); // 统计汉字库中的汉字数量

	for (k = 0; k < hz_count; k++)
	{
		// 假设 tfont32 的 Index 存储的是 GBK/GB2312 内码 (2字节)
		if ((tfont32[k].Index[0] == *(hz_ptr)) && (tfont32[k].Index[1] == *(hz_ptr + 1))) // 假设匹配GBK/GB2312前两字节
		{
			TFT_Set_Address(x, y, x + font_size - 1, y + font_size - 1);
			for (i = 0; i < bytes_per_char; i++)
			{
				uint8_t font_byte = tfont32[k].Msk[i];
				for (j = 0; j < 8; j++)
				{
					if (!mode)
					{
						if (font_byte & (0x80 >> j))
						{
							TFT_Write_Data16(fg_color);
						}
						else
						{
							TFT_Write_Data16(bg_color);
						}
					}
					else
					{
						if (font_byte & (0x80 >> j))
						{
							TFT_Draw_Point(x, y, fg_color);
						}
						x++;
						if ((x - x0) == font_size)
						{
							x = x0;
							y++;
							break;
						}
					}
				}
			}
			return;
		}
	}
}

/*
 * @brief  显示一个ASCII字符
 * @param  x      起始列坐标
 * @param  y      起始行坐标
 * @param  ascii_char 要显示的ASCII字符
 * @param  fg_color 字体颜色
 * @param  bg_color 背景颜色
 * @param  font_size 字号 (支持 16, 32)
 * @param  mode   显示模式: 0=背景不透明, 1=背景透明 (叠加显示)
 * @retval 无
 * @note   ASCII字符宽度为字号的一半。
 *         字库 ascii_1608 和 ascii_3216 必须包含对应的字符点阵数据。
 */
void TFT_Show_Char(uint16_t x, uint16_t y, uint8_t ascii_char, uint16_t fg_color, uint16_t bg_color, uint8_t font_size, uint8_t mode)
{
	uint8_t temp, t;
	uint16_t i, bytes_per_char; // 每个字符点阵占用的字节数
	uint16_t x0 = x;
	uint8_t char_width = font_size / 2; // ASCII字符宽度

	// 计算点阵字节数
	if (font_size == 16)
		bytes_per_char = 16; // 8x16字体，16字节
	else if (font_size == 32)
		bytes_per_char = 32; // 16x32字体，32字节
	else
		return; // 不支持的字号

	uint8_t char_index = ascii_char - ' '; // 计算字符在字库数组中的索引 (假设字库从空格开始)

	TFT_Set_Address(x, y, x + char_width - 1, y + font_size - 1); // 设置字符显示区域

	for (i = 0; i < bytes_per_char; i++)
	{
		if (font_size == 16)
			temp = ascii_1608[char_index][i]; // 调用8x16字体数据
		else
			temp = ascii_3216[char_index][i]; // 调用16x32字体数据

		for (t = 0; t < 8; t++)
		{
			if (!mode) // 非叠加模式
			{
				if (temp & (0x01 << t)) // 从低位到高位检查像素点 (注意：ASCII字库取模方式可能不同)
				{
					TFT_Write_Data16(fg_color);
				}
				else
				{
					TFT_Write_Data16(bg_color);
				}
			}
			else // 叠加模式
			{
				if (temp & (0x01 << t))
				{
					TFT_Draw_Point(x, y, fg_color); // 画一个点
				}
				x++;						// 移动到下一个像素位置
				if ((x - x0) == char_width) // 当前行绘制完成
				{
					x = x0; // 回到行首
					y++;	// 移动到下一行
					break;	// 当前字节处理完毕，跳到下一个字节
				}
			}
		}
	}
}

/*
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
void TFT_Show_String(uint16_t x, uint16_t y, const uint8_t *str, uint16_t fg_color, uint16_t bg_color, uint8_t font_size, uint8_t mode)
{
	while (*str != '\0')
	{
		TFT_Show_Char(x, y, *str, fg_color, bg_color, font_size, mode);
		x += font_size / 2; // ASCII字符宽度为字号一半
		str++;
	}
}

/*
 * @brief  计算m的n次方 (整数次幂)
 * @param  m 底数
 * @param  n 指数 (非负整数)
 * @retval uint32_t 计算结果
 */
uint32_t TFT_Pow(uint8_t m, uint8_t n)
{
	uint32_t result = 1;
	while (n--)
		result *= m;
	return result;
}

/*
 * @brief  显示一个无符号整型数字
 * @param  x      起始列坐标
 * @param  y      起始行坐标
 * @param  num    要显示的无符号整数
 * @param  len    要显示的数字位数 (如果实际位数小于len，前面会补空格)
 * @param  fg_color 字体颜色
 * @param  bg_color 背景颜色
 * @param  font_size 字号 (支持 16, 32)
 * @param  mode   显示模式: 0=背景不透明, 1=背景透明 (叠加显示)
 * @retval 无
 */
void TFT_Show_Int_Num(uint16_t x, uint16_t y, uint16_t num, uint8_t len, uint16_t fg_color, uint16_t bg_color, uint8_t font_size, uint8_t mode)
{
	uint8_t t, temp;
	uint8_t enshow = 0; // 用于标记是否开始显示有效数字 (跳过前导零)
	uint8_t char_width = font_size / 2;

	for (t = 0; t < len; t++)
	{
		temp = (num / TFT_Pow(10, len - t - 1)) % 10; // 提取当前位的数字
		if (enshow == 0 && t < (len - 1))
		{
			if (temp == 0)
			{
				// 如果是前导零，且不是最后一位，则显示空格
				TFT_Show_Char(x + t * char_width, y, ' ', fg_color, bg_color, font_size, mode);
				continue;
			}
			else
			{
				enshow = 1; // 遇到第一个非零数字，开始正常显示
			}
		}
		// 显示数字 (ASCII码 = 数字 + '0')
		TFT_Show_Char(x + t * char_width, y, temp + '0', fg_color, bg_color, font_size, mode);
	}
}

/*
 * @brief  显示一个浮点数 (保留两位小数)
 * @param  x      起始列坐标
 * @param  y      起始行坐标
 * @param  num    要显示的浮点数
 * @param  int_len 整数部分要显示的位数 (不足会补空格)
 * @param  fg_color 字体颜色
 * @param  bg_color 背景颜色
 * @param  font_size 字号 (支持 16, 32)
 * @param  mode   显示模式: 0=背景不透明, 1=背景透明 (叠加显示)
 * @retval 无
 */
void TFT_Show_Float_Num(uint16_t x, uint16_t y, float num, uint8_t int_len, uint16_t fg_color, uint16_t bg_color, uint8_t font_size, uint8_t mode)
{
	uint8_t t, char_width;
	uint32_t integer_part, decimal_part;

	char_width = font_size / 2;

	// 处理负数
	if (num < 0)
	{
		TFT_Show_Char(x, y, '-', fg_color, bg_color, font_size, mode);
		x += char_width;
		num = -num;
	}

	integer_part = (uint32_t)num;						   // 获取整数部分
	decimal_part = (uint32_t)((num - integer_part) * 100); // 获取两位小数部分

	// 显示整数部分 (使用 TFT_Show_Int_Num 处理前导空格)
	TFT_Show_Int_Num(x, y, integer_part, int_len, fg_color, bg_color, font_size, mode);

	// 显示小数点
	TFT_Show_Char(x + int_len * char_width, y, '.', fg_color, bg_color, font_size, mode);

	// 显示小数部分 (两位，不足补零)
	TFT_Show_Char(x + (int_len + 1) * char_width, y, (decimal_part / 10) + '0', fg_color, bg_color, font_size, mode);
	TFT_Show_Char(x + (int_len + 2) * char_width, y, (decimal_part % 10) + '0', fg_color, bg_color, font_size, mode);
}

/*
 * @brief  显示一幅存储在Flash中的图片
 * @param  x      图片左上角列坐标
 * @param  y      图片左上角行坐标
 * @param  width  图片宽度 (像素)
 * @param  height 图片高度 (像素)
 * @param  pic_ptr 指向图片数据数组 (RGB565格式, 高字节在前, 低字节在后)
 * @retval 无
 * @note   图片数据需要预先使用取模软件生成，格式为16位真彩色，高位在前。
 *         确保 pic_ptr 指向的数据大小至少为 width * height * 2 字节。
 */
void TFT_Show_Picture(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t pic_ptr[])
{
	uint32_t i, total_pixels;
	const uint8_t *pdata = pic_ptr;

	TFT_Set_Address(x, y, x + width - 1, y + height - 1); // 设置图片显示区域

	total_pixels = (uint32_t)width * height;

	for (i = 0; i < total_pixels; i++)
	{
		// 假设图片数据是高字节在前，低字节在后
		TFT_Write_Data8(*pdata++); // 发送高字节
		TFT_Write_Data8(*pdata++); // 发送低字节
		// 如果图片数据是低字节在前，高字节在后，则交换顺序：
		// uint8_t low_byte = *pdata++;
		// uint8_t high_byte = *pdata++;
		// TFT_Write_Data8(high_byte);
		// TFT_Write_Data8(low_byte);
	}
}
