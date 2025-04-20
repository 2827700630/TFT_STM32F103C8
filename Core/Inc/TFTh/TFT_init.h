#ifndef __TFT_INIT_H
#define __TFT_INIT_H

#include "TFTh/TFT_io.h" // 包含新的 IO 头文件
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief  通用 ST7735 初始化序列
     * @param  hspi 指向 SPI_HandleTypeDef 结构的指针
     * @retval 无
     * @note   此函数基于常见的 ST7735 初始化流程，并根据 DISPLAY_DIRECTION 调整 MADCTL。
     *         适用于 ST7735S 和 ST7735R 变种。
     *         伽马值等其他参数可能需要根据具体屏幕微调。
     */
    void TFT_Init_ST7735(SPI_HandleTypeDef *hspi);

    // 可以添加其他屏幕的初始化函数声明
    // void TFT_Init_ILI9341(SPI_HandleTypeDef *hspi);

#ifdef __cplusplus
}
#endif

#endif
