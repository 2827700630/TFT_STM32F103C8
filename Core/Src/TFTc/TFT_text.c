#include "TFTh/TFT_text.h"
#include "TFTh/TFT.h" // 包含绘图函数和 IO 函数
#include "TFTh/font.h" 
#include <string.h>   // 包含 memcpy 和 memcmp

//----------------- 内部辅助函数 -----------------

/**
 * @brief 获取 UTF-8 字符的字节长度
 * @param ch 指向 UTF-8 字符首字节的指针
 * @return 字节长度 (1-4)，如果不是有效的 UTF-8 起始字节则返回 0
 */
static uint8_t _TFT_GetUTF8Len(const uint8_t *ch)
{
    if (*ch < 0x80)
        return 1; // ASCII
    else if ((*ch & 0xE0) == 0xC0)
        return 2; // 2字节 UTF-8
    else if ((*ch & 0xF0) == 0xE0)
        return 3; // 3字节 UTF-8
    else if ((*ch & 0xF8) == 0xF0)
        return 4; // 4字节 UTF-8
    return 0;     // 无效的 UTF-8 起始字节
}

/**
 * @brief 绘制字模数据到 TFT 屏幕
 * @param x          起始列坐标
 * @param y          起始行坐标
 * @param glyph_data 指向字模数据的指针 (不含 UTF-8 头)
 * @param width      字符宽度
 * @param height     字符高度
 * @param color      字符颜色
 * @param back_color 背景颜色
 * @param mode       模式 (0: 背景不透明, 1: 背景透明)
 */
static void _TFT_Draw_Glyph(uint16_t x, uint16_t y, const uint8_t *glyph_data,
                            uint8_t width, uint8_t height,
                            uint16_t color, uint16_t back_color, uint8_t mode)
{
    uint16_t i, j;
    uint8_t temp;
    uint8_t bytes_per_row = (width + 7) / 8; // 每行占用的字节数

    TFT_Set_Address(x, y, x + width - 1, y + height - 1); // 设置绘制区域
    TFT_Reset_Buffer();                                   // 重置缓冲区

    for (i = 0; i < height; i++)
    {
        for (j = 0; j < bytes_per_row; j++)
        {
            temp = glyph_data[i * bytes_per_row + j]; // 获取当前字节的字模数据
            for (uint8_t bit = 0; bit < 8; bit++)
            {
                // 检查是否超出字符实际宽度
                if (j * 8 + bit < width)
                {
                    if (temp & 0x80) // 从最高位开始检查
                    {
                        TFT_Buffer_Write16(color); // 字体点
                    }
                    else if (mode == 0) // 非透明模式下绘制背景色
                    {
                        TFT_Buffer_Write16(back_color); // 背景点
                    }
                    // 透明模式下跳过背景点写入
                    temp <<= 1; // 移到下一位
                }
            }
        }
    }
    TFT_Flush_Buffer(1); // 绘制完一个字符后刷新缓冲区
}

//----------------- 字符/字符串显示函数 -----------------

/**
 * @brief  在指定位置显示字符串 (支持 ASCII 和 UTF-8 中文)
 * @param  x          起始列坐标
 * @param  y          起始行坐标
 * @param  str        要显示的字符串 (UTF-8 编码)
 * @param  color      字符颜色
 * @param  back_color 背景颜色
 * @param  size       字体大小 (ASCII 支持 8, 12, 16; 中文目前支持 16)
 * @param  mode       模式 (0: 背景不透明, 1: 背景透明)
 */
void TFT_Show_String(uint16_t x, uint16_t y, const uint8_t *str, uint16_t color, uint16_t back_color, uint8_t size, uint8_t mode)
{
    uint16_t current_x = x;
    uint16_t i = 0;
    uint8_t utf8_len;
    uint8_t found_chinese = 0;
    const uint8_t *glyph_data = NULL;
    uint8_t char_width = 0;
    uint8_t char_height = 0;

    // 中文字库信息 (目前只支持 16x16)
    // 这些常量现在依赖于 font.h 中定义的 zh16x16 和 font16x16
    const uint8_t chinese_char_bytes = 32;     // 16*16/8
    const uint8_t chinese_utf8_header_len = 4; // 字库中存储 UTF-8 编码占用的字节数
    // const uint8_t chinese_font_entry_len = chinese_utf8_header_len + chinese_char_bytes; // 不再需要
    const uint8_t num_chinese_chars = sizeof(zh16x16) / sizeof(zh16x16[0]); // 中文字库中的字符数

    while (str[i])
    {
        utf8_len = _TFT_GetUTF8Len(str + i);
        if (utf8_len == 0)
        { // 无效 UTF-8，跳过
            i++;
            continue;
        }

        found_chinese = 0;
        glyph_data = NULL;

        // 尝试匹配中文字符 (仅当 size 为 16 且 utf8_len > 1)
        if (size == 16 && utf8_len > 1)
        {
            for (uint8_t j = 0; j < num_chinese_chars; j++)
            {
                // 比较字库中的 UTF-8 头和当前字符
                // 假设字库条目结构为: {utf8_byte1, utf8_byte2, utf8_byte3, 0x00, glyph_data...}
                // 或 {utf8_byte1, utf8_byte2, 0x00, 0x00, glyph_data...} 等
                // 需要确保比较长度正确，并且字库中的编码以0结尾（如果字库是这样设计的话）
                // 这里的比较逻辑可能需要根据 zh16x16 的具体格式调整
                if (memcmp(str + i, zh16x16[j], utf8_len) == 0 && zh16x16[j][utf8_len] == 0) // 检查字库头长度是否匹配
                {
                    glyph_data = zh16x16[j] + chinese_utf8_header_len; // 获取字模数据指针
                    char_width = 16;                                   // font16x16.w;
                    char_height = 16;                                  // font16x16.h;
                    found_chinese = 1;
                    break;
                }
            }
        }

        // 如果找到中文字符
        if (found_chinese)
        {
            _TFT_Draw_Glyph(current_x, y, glyph_data, char_width, char_height, color, back_color, mode);
            current_x += char_width; // 更新 X 坐标
            i += utf8_len;           // 移动字符串索引
        }
        else // 未找到中文或为 ASCII 字符
        {
            // 强制使用 TFT_Show_Char 处理 ASCII 或未找到的中文字符（显示为 ASCII 或占位符）
            TFT_Show_Char(current_x, y, str[i], color, back_color, size, mode);

            // 更新 X 坐标 (根据 ASCII 字体宽度)
            if (size == 16)
                char_width = afont16x8.w;
            else if (size == 12)
                char_width = afont12x6.w;
            else
                char_width = afont8x6.w; // 默认或 size=8

            current_x += char_width;
            // 如果是未找到的多字节 UTF-8 字符，正确处理方式是跳过整个字符
            if (utf8_len > 1)
            {
                i += utf8_len;
            }
            else
            {
                i += 1; // ASCII 字符长度为 1
            }
        }
    }
}

/**
 * @brief  在指定位置显示一个 ASCII 字符
 * @param  x          起始列坐标
 * @param  y          起始行坐标
 * @param  chr        要显示的 ASCII 字符
 * @param  color      字符颜色
 * @param  back_color 背景颜色
 * @param  size       字体大小 (支持 8, 12, 16)
 * @param  mode       模式 (0: 背景不透明, 1: 背景透明)
 */
void TFT_Show_Char(uint16_t x, uint16_t y, uint8_t chr, uint16_t color, uint16_t back_color, uint8_t size, uint8_t mode)
{
    const ASCIIFont *ascii_font;
    const unsigned char *glyph_data;
    uint8_t char_width, char_height;
    uint16_t char_index;
    uint16_t bytes_per_char;

    // 根据 size 选择 ASCII 字体 (依赖 font.h 中的 extern 声明)
    if (size == 16)
    {
        ascii_font = &afont16x8;
    }
    else if (size == 12)
    {
        ascii_font = &afont12x6;
    }
    else
    { // 默认或 size == 8
        ascii_font = &afont8x6;
        size = 8; // 确保 size 值对应
    }

    char_width = ascii_font->w;
    char_height = ascii_font->h;

    // 计算字符在字模数组中的索引
    if (chr < ' ' || chr > '~')
    {
        chr = ' '; // 对于超出范围的字符，显示空格
    }
    char_index = chr - ' ';

    // 计算每个字符在数组中占用的字节数
    bytes_per_char = char_width * ((char_height + 7) / 8);

    // 获取字模数据指针
    // 注意：这里的指针运算依赖于 font.c 中定义的实际数组 ascii_8x6, ascii_12x6, ascii_16x8
    // 需要确保 ascii_font->chars 指向的是正确的二维数组首地址
    // C语言中直接用指针访问二维数组元素需要小心类型转换和计算
    // 假设 ASCIIFont 结构中的 chars 指针被正确初始化为指向对应二维数组的首元素地址
    // 例如: afont8x6 = {8, 6, (unsigned char *)ascii_8x6};
    glyph_data = ascii_font->chars + char_index * bytes_per_char;

    // 调用绘制函数
    _TFT_Draw_Glyph(x, y, glyph_data, char_width, char_height, color, back_color, mode);
}