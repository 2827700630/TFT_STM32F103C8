#include "TFTh/TFT_text.h"
#include "TFTh/TFT_io.h" // 包含绘图函数和 IO 函数
#include "TFTh/font.h"

//----------------- 内部辅助函数 -----------------

/**
 * @brief 绘制字模数据到 TFT 屏幕 (支持列行式字库)
 * @param htft TFT句柄指针
 * @param x          起始列坐标
 * @param y          起始行坐标
 * @param glyph_data 指向字模数据的指针 (列行式)
 * @param width      字符宽度
 * @param height     字符高度
 * @param color      字符颜色
 * @param back_color 背景颜色
 * @param mode       模式 (0: 背景不透明, 1: 背景透明)
 */
static void _TFT_Draw_Glyph(TFT_HandleTypeDef *htft, uint16_t x, uint16_t y, const uint8_t *glyph_data,
                            uint8_t width, uint8_t height,
                            uint16_t color, uint16_t back_color, uint8_t mode)
{
    uint16_t col, row;
    uint8_t byte, bit;
    uint8_t bytes_per_column = (height + 7) / 8; // 每列字节数（8行=1, 12行=2）

    TFT_Set_Address(htft, x, y, x + width - 1, y + height - 1);
    TFT_Reset_Buffer(htft);

    // 按列优先处理（匹配逐行式字库）
    for (col = 0; col < width; col++)
    { // 先遍历列
        for (uint8_t byte_idx = 0; byte_idx < bytes_per_column; byte_idx++)
        { // 每列的字节
            byte = glyph_data[col * bytes_per_column + byte_idx];
            for (row = 0; row < 8; row++)
            {                                            // 遍历当前字节的8位
                uint16_t pixel_row = byte_idx * 8 + row; // 计算实际行号
                if (pixel_row >= height)
                    break; // 防止越界

                // 关键修改：选择正确的位顺序（尝试两种方式）左右翻转请修改它
                // bit = (byte >> (7 - row)) & 0x01; // 方式1：MSB在上（常见）
                bit = (byte >> row) & 0x01; // 方式2：LSB在上

                if (bit)
                {
                    TFT_Buffer_Write16(htft, color); // 前景色
                }
                else if (mode == 0)
                {
                    TFT_Buffer_Write16(htft, back_color); // 背景色
                }
                else
                {
                    // 透明模式（需硬件支持）
                }
            }
        }
    }
    TFT_Flush_Buffer(htft, 1);
}

//----------------- 字符/字符串显示函数 -----------------

/**
 * @brief  在指定位置显示 ASCII 字符串
 * @param  htft TFT句柄指针
 * @param  x          起始列坐标
 * @param  y          起始行坐标
 * @param  str        要显示的 ASCII 字符串
 * @param  color      字符颜色
 * @param  back_color 背景颜色
 * @param  size       字体大小 (支持 8, 12, 16)
 * @param  mode       模式 (0: 背景不透明, 1: 背景透明)
 */
void TFT_Show_String(TFT_HandleTypeDef *htft, uint16_t x, uint16_t y, const uint8_t *str, uint16_t color, uint16_t back_color, uint8_t size, uint8_t mode)
{
    uint16_t current_x = x;
    uint8_t char_width = 0;

    while (*str) // 遍历字符串直到遇到 null 终止符
    {
        // 调用 TFT_Show_Char 显示当前 ASCII 字符
        TFT_Show_Char(htft, current_x, y, *str, color, back_color, size, mode);

        // 根据字体大小更新 X 坐标
        if (size == 16)
            char_width = afont16x8.w;
        else if (size == 12)
            char_width = afont12x6.w;
        else // 默认或 size=8
            char_width = afont8x6.w;

        current_x += char_width; // 移动到下一个字符的位置
        str++;                   // 指向下一个字符
    }
}

/**
 * @brief  在指定位置显示一个 ASCII 字符
 * @param  htft TFT句柄指针
 * @param  x          起始列坐标
 * @param  y          起始行坐标
 * @param  chr        要显示的 ASCII 字符
 * @param  color      字符颜色
 * @param  back_color 背景颜色
 * @param  size       字体大小 (支持 8, 12, 16)
 * @param  mode       模式 (0: 背景不透明, 1: 背景透明)
 */
void TFT_Show_Char(TFT_HandleTypeDef *htft, uint16_t x, uint16_t y, uint8_t chr, uint16_t color, uint16_t back_color, uint8_t size, uint8_t mode)
{
    const ASCIIFont *ascii_font;
    const unsigned char *glyph_data;
    uint8_t char_width, char_height;
    uint16_t char_index;
    uint16_t bytes_per_char;

    // 根据字体大小选择对应字库
    if (size == 16)
    {
        ascii_font = &afont16x8;
        bytes_per_char = 16; // 8列×16行，每列2字节，共16字节
    }
    else if (size == 12)
    {
        ascii_font = &afont12x6;
        bytes_per_char = 12; // 6列×12行，每列2字节，共12字节
    }
    else
    {
        ascii_font = &afont8x6;
        size = 8;
        bytes_per_char = 6; // 6列×8行，每列1字节，共6字节
    }

    char_width = ascii_font->w;
    char_height = ascii_font->h;

    // 检查字符是否在可显示范围内
    if (chr < ' ' || chr > '~')
    {
        chr = ' '; // 不可显示字符显示为空格
    }

    char_index = chr - ' ';                                       // 计算字符在字库中的索引位置
    glyph_data = ascii_font->chars + char_index * bytes_per_char; // 指向对应字模数据

    // 调用绘制函数
    _TFT_Draw_Glyph(htft, x, y, glyph_data, char_width, char_height, color, back_color, mode);
}
