/*
 * @file    TFT_config.h
 * @brief   TFT屏幕配置文件
 *
 * 本文件用于配置TFT屏幕的相关参数和颜色定义。
 * 请根据您的具体硬件和需求进行相应的修改。
 */

#ifndef __TFT_CONFIG_H
#define __TFT_CONFIG_H

/**
 * @brief 定义使用的硬件平台库
 * 取消注释或定义您正在使用的平台。
 * 这将用于在 TFT_io.c 中选择正确的底层 GPIO 和 SPI 函数。
 */
#define STM32HAL // 使用 STM32 HAL 库
// #define SOME_OTHER_PLATFORM // 示例：用于其他平台（还没做）

/**
 * @brief 定义绘图缓冲区的大小 (字节)，足够大的缓冲区在DMA传输时有明显优势
 *
 * 该缓冲区用于存储绘图数据，确保足够的空间以支持图形显示。
 * 目前测试发现 1024-4096 字节的缓冲区在 DMA 传输时效果最好
 */

#define TFT_BUFFER_SIZE 4096 // 2048 字节 (1024 像素, RGB565 格式)

/**
 * @brief 定义屏幕的显示方向 (重要配置)
 *
 * - 0: 正常方向 (0°旋转)
 * - 1: 顺时针旋转90度
 * - 2: 顺时针旋转180度
 * - 3: 顺时针旋转270度
 * 注意:
 * 1. ST7735S (红板) 和 ST7735R (黑板) 的 MADCTL 设置和颜色顺序 (BGR/RGB) 不同。
 * 2. 不同方向的地址偏移量 (X offset, Y offset) 可能需要根据实际屏幕调整。
 * 3. 请根据您使用的具体屏幕型号和期望的显示方向选择合适的值。
 * 4. 如果您的屏幕是其他型号 (如 ST7789, ILI9341), 您需要查阅其数据手册来确定正确的 MADCTL 值和偏移量，
 *    并在 TFT_init.c 和 TFT_io.c 中添加相应的初始化序列和地址设置逻辑。
 */
#define DISPLAY_DIRECTION 2

/*
 * @brief 定义屏幕的 X 和 Y 偏移量 (像素)，用于调整显示区域
 *
 * 这些偏移量用于在设置地址时调整实际的显示区域。如果你的屏幕有偏移，修改这些值。
 * 根据您的屏幕型号和连接方式进行相应的修改。
 */
#define TFT_X_OFFSET 2 // X轴偏移量
#define TFT_Y_OFFSET 1 // Y轴偏移量

/*
 * 常用颜色定义 (RGB565格式)
 */
#define WHITE 0xFFFF   // 白色
#define BLACK 0x0000   // 黑色
#define BLUE 0x001F    // 蓝色
#define BRED 0XF81F    // 蓝红色 (洋红)
#define GRED 0XFFE0    // 绿红色 (黄色)
#define GBLUE 0X07FF   // 绿蓝色 (青色)
#define RED 0xF800     // 红色
#define MAGENTA 0xF81F // 品红色 (同 BRED)
#define GREEN 0x07E0   // 绿色
#define CYAN 0x7FFF    // 青色 (同 GBLUE)
#define YELLOW 0xFFE0  // 黄色 (同 GRED)
#define BROWN 0XBC40   // 棕色
#define BRRED 0XFC07   // 棕红色
#define GRAY 0X8430    // 灰色
// 可以根据需要添加更多颜色定义
// #define ORANGE      0xFD20
// #define PINK        0xFE19

#endif
