# TFT_STM32F103C8

本项目是一个基于 STM32F103C8 微控制器的 TFT 液晶屏幕驱动库，使用 HAL 库、SPI 通信和 DMA 传输（可选）。

## 功能特性

*   **支持驱动芯片**:
    *   ST7735S (红板)
    *   ST7735R (黑板)
    *   包含通用初始化序列，可能适用于其他 ST7735 变种。
*   **底层接口**:
    *   SPI 通信，支持 DMA 传输以提高效率。
    *   可配置的绘图缓冲区 (`TFT_BUFFER_SIZE` in `TFT_config.h`)。
    *   GPIO 控制 (RES, DC, CS, BLK)。
*   **基本绘图**:
    *   填充区域 (`TFT_Fill_Area`) - 支持 DMA
    *   绘制点 (`TFT_Draw_Point`)
    *   绘制直线 (`TFT_Draw_Line`)
    *   绘制矩形 (`TFT_Draw_Rectangle`)
    *   填充矩形 (`TFT_Fill_Rectangle`) - 支持 DMA
    *   绘制圆 (`TFT_Draw_Circle`)
    *   填充圆 (`TFT_Fill_Circle`)
    *   绘制三角形 (`TFT_Draw_Triangle`)
    *   填充三角形 (`TFT_Fill_Triangle`)
    *   绘制圆角矩形 (`TFT_Draw_Rounded_Rectangle`)
    *   填充圆角矩形 (`TFT_Fill_Rounded_Rectangle`)
*   **文本显示**:
    *   显示 ASCII 字符 (`TFT_Show_Char`)
    *   显示 ASCII 字符串 (`TFT_Show_String`)
    *   支持多种字号 (如 8x16, 16x32，具体见 `font.h`)
    *   支持背景透明/不透明模式
    *   (注意: 中文字符显示功能可能需要额外字体支持和函数实现，当前版本主要关注 ASCII)
*   **图片显示**:
    *   (需要相应函数实现，如 `TFT_Show_Picture`，当前版本未包含)

## 硬件要求

*   **MCU**: STM32F103C8Tx (或其他兼容 HAL 库的 STM32 型号)
*   **屏幕**: 1.8寸 TFT 液晶屏 (或其他尺寸，只要驱动是 ST7735S/R)
*   **接口**: SPI (本项目默认使用 SPI1)
*   **引脚连接**: (请根据 `Core/Inc/TFTh/TFT_config.h` 和 CubeMX 配置确认)
    *   SPI SCK, MOSI
    *   TFT_RES (复位)
    *   TFT_DC (数据/命令)
    *   TFT_CS (片选)
    *   TFT_BLK (背光控制)

## 软件与环境

*   **开发环境**: STM32CubeIDE 或其他支持 Make / GCC ARM 的环境。
*   **库**: STM32 HAL 库
*   **字体**: 包含 ASCII 字模数据 (`Core/Src/TFTc/font.c`, `Core/Inc/TFTh/font.h`)

## 如何配置和使用

### 1. STM32CubeMX 配置

*   **SPI 配置**:
    *   启用你选择的 SPI 外设 (例如 SPI1)。
    *   **模式**: 设置为 `Full-Duplex Master`。
    *   **硬件 NSS 信号**: 设置为 `Disable` (我们将使用软件控制 CS 引脚)。
    *   **数据大小**: 设置为 `8 Bits`。
    *   **First Bit**: 设置为 `MSB First`。
    *   **时钟极性 (CPOL)**: 设置为 `Low`。
    *   **时钟相位 (CPHA)**: 设置为 `1 Edge` (或根据你的屏幕数据手册调整，通常是 `Low` 和 `1 Edge` 或 `Low` 和 `2 Edge`)。
    *   **波特率**: 根据你的系统时钟和屏幕规格设置一个合适的值 (例如 18 MHz 或 36 MHz)。
    *   **DMA 设置 (可选但推荐)**:
        *   在 `DMA Settings` 选项卡中，为 SPI 的 `TX` 添加一个 DMA 请求。
        *   选择一个 DMA 通道。
        *   **模式**: 设置为 `Normal`。
        *   **方向**: 设置为 `Memory to Peripheral`。
        *   **优先级**: 设置为 `Medium` 或 `High`。
        *   **数据宽度**: `Increment Address` 应勾选 `Memory`，不勾选 `Peripheral`。数据宽度设置为 `Byte`。
*   **GPIO 配置**:
    *   **SPI 引脚**: CubeMX 会自动配置 SCK 和 MOSI 引脚为 `Alternate Function Push Pull`。
    *   **控制引脚**:
        *   配置 `TFT_RES`, `TFT_DC`, `TFT_CS`, `TFT_BLK` 对应的 GPIO 引脚为 `GPIO_Output` 模式。
        *   设置输出电平为 `High` (CS 和 BLK 通常初始为高电平)。
        *   设置输出速度为 `High` 或 `Very High`。
        *   **用户标签**: 强烈建议为这些引脚添加用户标签 (User Label)，例如 `TFT_RES`, `TFT_DC`, `TFT_CS`, `TFT_BLK`。这将生成相应的宏定义在 `main.h` 中，方便代码引用。
*   **时钟配置**: 确保 SPI 外设的时钟已启用，并且系统时钟配置正确。
*   **生成代码**: 生成 CubeMX 项目代码。

### 2. TFT 驱动库配置 (`Core/Inc/TFTh/TFT_config.h`)

打开 `TFT_config.h` 文件进行以下配置：

*   **`#define STM32HAL`**: 确保此行已定义，表示使用 STM32 HAL 库。
*   **`TFT_BUFFER_SIZE`**: 定义绘图缓冲区的大小（字节）。推荐使用 1024 到 4096 之间的值，以优化 DMA 传输性能。例如 `4096`。
*   **`DISPLAY_DIRECTION`**: **非常重要**。根据你的屏幕型号（ST7735S 红板或 ST7735R 黑板）和期望的显示方向（0°, 90°, 180°, 270°）选择合适的值。注释中有详细说明不同值的含义和对应的屏幕类型/颜色顺序 (BGR/RGB)。
    *   例如，对于 ST7735R 黑板，竖屏（0度）可能使用 `4`，横屏（90度）可能需要添加新的定义或调整现有定义。对于 ST7735S 红板，竖屏（0度）使用 `0`。
*   **`TFT_X_OFFSET`, `TFT_Y_OFFSET`**: 如果你的屏幕显示内容有偏移，调整这些值（像素单位）来校正。默认值通常适用于某些常见模块，但可能需要根据实际情况修改。
*   **颜色定义**: 文件末尾预定义了一些常用颜色 (RGB565 格式)，你可以根据需要添加更多颜色。

### 3. 引脚宏定义确认/修改 (`Core/Inc/TFTh/TFT_io.h`)

打开 `TFT_io.h` 文件，检查或修改以下宏定义，确保它们与你在 CubeMX 中设置的 GPIO 用户标签或引脚定义一致：

*   `TFT_RES_GPIO_Port`, `TFT_RES_Pin`
*   `TFT_DC_GPIO_Port`, `TFT_DC_Pin`
*   `TFT_CS_GPIO_Port`, `TFT_CS_Pin`
*   `TFT_BLK_GPIO_Port`, `TFT_BLK_Pin`

如果 CubeMX 生成的宏名称不同（例如，如果你没有使用用户标签），你需要将这里的宏定义修改为 CubeMX 生成的实际名称 (通常在 `main.h` 中可以找到)。

### 4. 集成到 `main.c`

*   **包含头文件**:
    ```c
    #include "TFTh/TFT_init.h" // 包含初始化函数
    #include "TFTh/TFT_CAD.h"  // 包含绘图函数
    #include "TFTh/TFT_text.h" // 包含文本显示函数
    ```
*   **初始化**: 在 `main` 函数的 `/* USER CODE BEGIN 2 */` 部分，调用 TFT 初始化函数，并传入配置好的 SPI 句柄指针：
    ```c
    // 假设你的 SPI 句柄是 hspi1
    extern SPI_HandleTypeDef hspi1;
    TFT_Init_ST7735(&hspi1); // 初始化 ST7735 驱动
    ```
*   **调用绘图/显示函数**: 在需要的地方调用 `TFT_CAD.h` 和 `TFT_text.h` 中声明的函数来操作屏幕。
    ```c
    // 清屏为白色
    TFT_Fill_Area(0, 0, TFT_WIDTH, TFT_HEIGHT, WHITE);

    // 在 (10, 10) 处显示红色 "Hello" (字体大小16, 不透明背景)
    TFT_Show_String(10, 10, (uint8_t*)"Hello", RED, WHITE, 16, 0);

    // 绘制一个蓝色圆心在 (64, 64)，半径为 20 的圆
    TFT_Draw_Circle(64, 64, 20, BLUE);

    // 使用 DMA 填充一个黄色矩形
    TFT_Fill_Rectangle(20, 80, 100, 120, YELLOW);
    ```
    *   注意: `TFT_WIDTH` 和 `TFT_HEIGHT` 宏在 `TFT_init.h` 中根据 `DISPLAY_DIRECTION` 自动定义。

## 文件结构 (建议)

```
Core/
├── Inc/
│   ├── TFTh/           # TFT 驱动头文件
│   │   ├── font.h
│   │   ├── TFT_CAD.h     # 绘图和显示函数声明
│   │   ├── TFT_config.h  # 配置 (引脚, 方向, 缓冲区, 颜色)
│   │   ├── TFT_init.h    # 初始化函数声明
│   │   └── TFT_io.h      # 底层 IO 函数声明 (SPI, GPIO)
│   ├── main.h
│   ├── gpio.h
│   ├── spi.h
│   ├── dma.h
│   └── ...             # 其他 HAL 配置头文件
└── Src/
    ├── TFTc/           # TFT 驱动源文件
    │   ├── font.c
    │   ├── TFT_CAD.c     # 绘图和显示函数实现
    │   ├── TFT_init.c    # 初始化函数实现
    │   └── TFT_io.c      # 底层 IO 函数实现
    ├── main.c          # 主程序
    ├── gpio.c
    ├── spi.c
    ├── dma.c
    └── ...             # 其他 HAL 驱动和系统文件
Drivers/                # HAL 库和 CMSIS 文件
...                     # 其他项目文件 (构建输出, 配置等)
```

## 注意事项

*   **坐标系统**: 所有坐标均从左上角 (0, 0) 开始。
*   **颜色格式**: 颜色使用 RGB565 格式 (16位)。
*   **DMA**: 使用 DMA 可以显著提高大面积填充（如 `TFT_Fill_Area`, `TFT_Fill_Rectangle`）的效率。确保 CubeMX 中正确配置了 SPI TX DMA。
*   **缓冲区**: `TFT_BUFFER_SIZE` 影响 DMA 传输效率和内存占用。如果内存紧张，可以适当减小此值，但可能会降低 DMA 性能。
*   **阻塞与非阻塞**: 当前的 SPI 传输函数 (`TFT_Write_Data`, `TFT_Write_Cmd`, `TFT_Buffer_Write16`) 可能是阻塞的（等待传输完成）。如果需要非阻塞操作，需要修改 `TFT_io.c` 中的 SPI/DMA 调用方式并处理完成中断。

## 更新日志

*   **2025/4/20**:
    *   更新 README，详细说明功能、使用方法、CubeMX 配置和 `TFT_config.h` 设置。
    *   完善硬件抽象层
*   **2025/4/19**:
    *   改进代码可读性：使用更清晰的变量名并添加中文注释 (TFT_CAD.c, TFT_io.c, TFT_CAD.h, TFT_io.h)。
    *   增加 SPI 缓冲区 (`TFT_BUFFER_SIZE`) 和 DMA 支持，大幅提升填充效率。
    *   完善绘图算法，部分绘图函数应用 DMA。
*   **2025/4/18**:
    *   开始重新整理工程。
    *   优化注释风格。
    *   将 `u8/u16/u32` 类型替换为标准的 `uint8_t/uint16_t/uint32_t`。
    *   将函数名前缀从 `LCD_` 修改为 `TFT_`。
    *   更新 README 文件。
