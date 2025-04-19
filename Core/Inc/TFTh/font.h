#ifndef __FONT_H__
#define __FONT_H__

#include <stdint.h>

// ASCII 字体结构体
typedef struct
{
    const uint8_t h;    // 字符高度
    const uint8_t w;    // 字符宽度
    const uint8_t *chars; // 指向字模数据的指针
} ASCIIFont;

// 通用字体结构体 (包含 ASCII 和中文)
typedef struct
{
    const uint8_t h;             // 字符高度
    const uint8_t w;             // 字符宽度 (通常指中文宽度)
    const uint8_t (*font_data)[]; // 指向字库数据的指针 (例如 zh16x16)
    const uint8_t num_chars;     // 字库中的字符数量
    const ASCIIFont *ascii_font; // 指向关联的 ASCII 字体
} Font;

// 图像结构体
typedef struct
{
    const uint16_t w; // 图像宽度
    const uint16_t h; // 图像高度
    const uint8_t *data; // 指向图像数据的指针
} Image;


// --- 外部字体数据声明 ---
// ASCII
extern const unsigned char ascii_8x6[][6];
extern const unsigned char ascii_12x6[][12];
extern const unsigned char ascii_16x8[][16];
extern const ASCIIFont afont8x6;
extern const ASCIIFont afont12x6;
extern const ASCIIFont afont16x8;

// 中文 (16x16)
extern const uint8_t zh16x16[][36]; // 假设每个条目最多 36 字节 (4 UTF-8 + 32 data)
extern const Font font16x16;

// --- 外部图像数据声明 ---
extern const uint8_t bilibiliData[];
extern const Image bilibiliImg;
extern const uint8_t A001Data[];
extern const Image A001Img;


#endif // __FONT_H__
