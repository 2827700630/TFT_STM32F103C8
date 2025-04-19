# TFT_STM32F103C8

本项目是一个基于 STM32F103C8 微控制器的 TFT 液晶屏幕驱动库，使用 HAL 库和 SPI 通信。

## 特性

*   **支持驱动芯片**:
    *   ST7735S
    *   ST7735R
    *   (包含一个通用初始化序列，可能适用于其他 ST7735 变种)
*   **基本绘图**:
    *   填充区域 (`TFT_Fill_Area`)
    *   绘制点 (`TFT_Draw_Point`)
    *   绘制直线 (`TFT_Draw_Line`)
    *   绘制矩形 (`TFT_Draw_Rectangle`)
    *   绘制圆 (`TFT_Draw_Circle`)
*   **文本显示**:
    *   显示 ASCII 字符 (`TFT_Show_Char`)
    *   显示 ASCII 字符串 (`TFT_Show_String`)
    *   显示中文字符串 (UTF-8 编码) (`TFT_Show_Chinese_String`)
    *   支持多种字号 (16x16, 24x24, 32x32 for Chinese; 8x16, 16x32 for ASCII)
    *   支持背景透明/不透明模式
*   **数字显示**:
    *   显示无符号整数 (`TFT_Show_Int_Num`)，支持指定长度和前导空格
    *   显示浮点数 (`TFT_Show_Float_Num`)，固定两位小数
*   **图片显示**:
    *   显示存储在 Flash 中的图片 (`TFT_Show_Picture`)，支持 RGB565 格式

## 硬件

*   **MCU**: STM32F103C8Tx
*   **屏幕**: 1.8寸 TFT 液晶屏 (或其他尺寸，只要驱动是 ST7735S/R)
*   **接口**: SPI (本项目使用 SPI1)
*   **引脚连接**: (请根据 `Core/Inc/TFT/TFT_init.h` 和 CubeMX 配置确认)
    *   SPI SCK, MOSI
    *   TFT_RES (复位)
    *   TFT_DC (数据/命令)
    *   TFT_CS (片选)
    *   TFT_BLK (背光控制)

## 软件与环境

*   **开发环境**: STM32CubeIDE / PlatformIO (根据 `platformio.ini` 文件推断可能支持 PlatformIO)
*   **库**: STM32 HAL 库
*   **字体**: 包含 ASCII (8x16, 16x32) 和中文 (16x16, 24x24, 32x32) 字模数据 (`Core/Inc/TFT/TFTfont.h`)
*   **图片**: 包含示例图片数据 (`Core/Inc/TFT/pic.h`)

## 文件结构

```
Core/
├── Inc/
│   ├── TFT/
│   │   ├── TFT.h         # 绘图及显示函数声明
│   │   ├── TFT_init.h    # 底层驱动、引脚、颜色、初始化函数声明
│   │   ├── TFTfont.h     # 字体数据
│   │   └── pic.h         # 图片数据
│   ├── main.h
│   ├── gpio.h
│   ├── spi.h
│   └── ...             # 其他 HAL 配置头文件
└── Src/
    ├── TFT/            # (建议将 lcd.c 和 lcd_init.c 移入此目录并重命名)
    │   ├── TFT.c         # 绘图及显示函数实现 (原 lcd.c)
    │   └── TFT_init.c    # 底层驱动及初始化函数实现 (原 lcd_init.c)
    ├── main.c          # 主程序
    ├── gpio.c
    ├── spi.c
    └── ...             # 其他 HAL 驱动和系统文件
Drivers/                # HAL 库和 CMSIS 文件
...                     # 其他项目文件 (编译输出, 配置等)
```

## 如何使用

1.  **配置**:
    *   使用 STM32CubeMX 配置 SPI1 (或你选择的 SPI 接口) 为全双工 Master 模式。
    *   配置 RES, DC, CS, BLK 引脚为 GPIO 输出模式。确保 CubeMX 中的引脚名称与 `Core/Inc/TFT/TFT_init.h` 中的 `TFT_XXX_GPIO_Port` 和 `TFT_XXX_Pin` 宏定义一致。
    *   在 `Core/Inc/TFT/TFT_init.h` 中，根据你的屏幕驱动芯片和期望的显示方向，修改 `DISPLAY_DIRECTION` 宏的值。
2.  **初始化**:
    *   在 `main.c` 中包含必要的头文件: `#include "TFT/TFT_init.h"` 和 `#include "TFT/TFT.h"`。
    *   在 `main` 函数的初始化部分，调用适合你屏幕的初始化函数，例如 `TFT_Init_ST7735R();` 或 `TFT_Init_ST7735S();`。
3.  **调用函数**:
    *   调用 `TFT.h` 中声明的绘图、显示函数来操作屏幕。例如:
        ```c
        // 清屏为白色
        TFT_Fill_Area(0, 0, LCD_WIDTH, LCD_HEIGHT, WHITE);

        // 在 (10, 10) 处显示红色 "Hello"
        TFT_Show_String(10, 10, (uint8_t*)"Hello", RED, WHITE, 16, 0);

        // 绘制一个蓝色圆心在 (64, 64)，半径为 20 的圆
        TFT_Draw_Circle(64, 64, 20, BLUE);
        ```

## 注意事项

*   **中文字符串**: `TFT_Show_Chinese_String` 函数期望接收 UTF-8 编码的字符串。确保你的源文件保存为 UTF-8 格式，或者在传入前进行编码转换。字库文件 (`TFTfont.h`) 也需要包含对应汉字的字模数据。
*   **坐标系统**: 所有坐标均从左上角 (0, 0) 开始。
*   **颜色格式**: 颜色使用 RGB565 格式。
*   **图片格式**: `TFT_Show_Picture` 函数期望图片数据为 RGB565 格式，并且高字节在前。请使用合适的取模软件生成。
*   **文件重命名**: 建议将 `Core/Src/lcd.c` 重命名为 `TFT.c`，`Core/Src/lcd_init.c` 重命名为 `TFT_init.c`，并将它们移动到 `Core/Src/TFT/` 目录下，以保持与头文件结构一致。同时，更新 `Core/Inc/TFT/lcd.h` 为 `TFT.h`，`Core/Inc/TFT/lcd_init.h` 为 `TFT_init.h`，并修改相应的 `#include` 语句。

## 更新日志

*   **2025/4/19**:
    *   增加SPI缓冲区大幅提升DMA效率。
    *   完善画图算法，画图现在也可以应用DMA了

*   **2025/4/18**:
    *   开始重新整理工程。
    *   优化注释风格。
    *   将 `u8/u16/u32` 类型替换为标准的 `uint8_t/uint16_t/uint32_t`。
    *   将函数名前缀从 `LCD_` 修改为 `TFT_`。
    *   更新 README 文件。