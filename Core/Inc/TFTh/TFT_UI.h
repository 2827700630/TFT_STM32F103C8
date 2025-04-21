/*
 * @file    TFT_UI.h
 * @brief   TFT用户界面组件库头文件
 * @details 定义了TFT屏幕上常用的UI组件，如按钮、进度条、开关等
 */
#ifndef __TFT_UI_H
#define __TFT_UI_H

#include "TFTh/TFT_CAD.h"
#include "TFTh/TFT_text.h"
#include "TFTh/TFT_io.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief UI组件基础结构体
 */
typedef struct {
    uint16_t x;         // X坐标（左上角）
    uint16_t y;         // Y坐标（左上角）
    uint16_t width;     // 组件宽度
    uint16_t height;    // 组件高度
    bool visible;       // 是否可见
    bool enabled;       // 是否启用
} TFT_UI_Component;

/**
 * @brief 按钮状态枚举
 */
typedef enum {
    BUTTON_NORMAL,     // 正常状态
    BUTTON_PRESSED,    // 按下状态
    BUTTON_DISABLED    // 禁用状态
} TFT_Button_State;

/**
 * @brief 按钮结构体
 */
typedef struct {
    TFT_UI_Component base;       // 基础组件属性
    char text[20];               // 按钮文本
    uint16_t text_color;         // 文本颜色
    uint16_t bg_color;           // 背景颜色
    uint16_t border_color;       // 边框颜色
    uint16_t pressed_color;      // 按下时的背景颜色
    uint16_t disabled_color;     // 禁用时的背景颜色
    uint8_t corner_radius;       // 圆角半径
    uint8_t text_size;           // 文本大小
    TFT_Button_State state;      // 按钮状态
} TFT_Button;

/**
 * @brief 进度条结构体
 */
typedef struct {
    TFT_UI_Component base;       // 基础组件属性
    uint8_t progress;            // 进度值(0-100)
    uint16_t bg_color;           // 背景颜色
    uint16_t progress_color;     // 进度条颜色
    uint16_t border_color;       // 边框颜色
    uint8_t corner_radius;       // 圆角半径
    bool show_percentage;        // 是否显示百分比文本
    uint16_t text_color;         // 文本颜色
} TFT_ProgressBar;

/**
 * @brief 开关结构体
 */
typedef struct {
    TFT_UI_Component base;       // 基础组件属性
    bool state;                  // 开关状态(true=开,false=关)
    uint16_t on_color;           // 开启状态颜色
    uint16_t off_color;          // 关闭状态颜色
    uint16_t thumb_color;        // 滑块颜色
    uint16_t border_color;       // 边框颜色
    uint8_t corner_radius;       // 圆角半径
} TFT_Switch;

/**
 * @brief 图标按钮结构体
 */
typedef struct {
    TFT_UI_Component base;       // 基础组件属性
    const uint8_t* icon;         // 图标数据指针
    uint16_t icon_width;         // 图标宽度
    uint16_t icon_height;        // 图标高度
    uint16_t bg_color;           // 背景颜色
    uint16_t icon_color;         // 图标颜色
    uint16_t pressed_color;      // 按下时的背景颜色
    uint16_t disabled_color;     // 禁用时的背景颜色
    uint8_t corner_radius;       // 圆角半径
    TFT_Button_State state;      // 按钮状态
} TFT_IconButton;

/**
 * @brief 标签结构体
 */
typedef struct {
    TFT_UI_Component base;       // 基础组件属性
    char text[50];               // 标签文本
    uint16_t text_color;         // 文本颜色
    uint16_t bg_color;           // 背景颜色
    uint8_t text_size;           // 文本大小
    bool transparent_bg;         // 背景是否透明
    uint8_t alignment;           // 对齐方式(0=左,1=中,2=右)
} TFT_Label;

/**
 * @brief 复选框结构体
 */
typedef struct {
    TFT_UI_Component base;       // 基础组件属性
    bool checked;                // 是否选中
    char text[30];               // 复选框文本
    uint16_t text_color;         // 文本颜色
    uint16_t bg_color;           // 背景颜色
    uint16_t check_color;        // 选中标记颜色
    uint16_t border_color;       // 边框颜色
    uint8_t text_size;           // 文本大小
} TFT_Checkbox;

/**
 * @brief 简单消息框结构体
 */
typedef struct {
    TFT_UI_Component base;       // 基础组件属性
    char title[30];              // 标题
    char message[100];           // 消息内容
    uint16_t title_color;        // 标题颜色
    uint16_t text_color;         // 文本颜色
    uint16_t bg_color;           // 背景颜色
    uint16_t border_color;       // 边框颜色
    uint8_t title_size;          // 标题字体大小
    uint8_t text_size;           // 消息字体大小
    uint8_t corner_radius;       // 圆角半径
} TFT_MessageBox;

//----------------- 按钮相关函数声明 -----------------

/**
 * @brief  初始化按钮
 * @param  htft TFT句柄指针
 * @param  btn 按钮指针
 * @param  x 左上角X坐标
 * @param  y 左上角Y坐标
 * @param  width 按钮宽度
 * @param  height 按钮高度
 * @param  text 按钮文本
 * @param  text_color 文本颜色
 * @param  bg_color 背景颜色
 * @param  border_color 边框颜色
 * @retval 无
 */
void TFT_Button_Init(TFT_Button* btn, uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                    const char* text, uint16_t text_color, uint16_t bg_color, uint16_t border_color);

/**
 * @brief  绘制按钮
 * @param  htft TFT句柄指针
 * @param  btn 按钮指针
 * @retval 无
 */
void TFT_Button_Draw(TFT_HandleTypeDef* htft, TFT_Button* btn);

/**
 * @brief  设置按钮状态
 * @param  btn 按钮指针
 * @param  state 按钮状态
 * @retval 无
 */
void TFT_Button_SetState(TFT_Button* btn, TFT_Button_State state);

/**
 * @brief  检查是否点击了按钮
 * @param  btn 按钮指针
 * @param  touch_x 触摸X坐标
 * @param  touch_y 触摸Y坐标
 * @retval bool 是否点击了按钮
 */
bool TFT_Button_IsPressed(TFT_Button* btn, uint16_t touch_x, uint16_t touch_y);

//----------------- 进度条相关函数声明 -----------------

/**
 * @brief  初始化进度条
 * @param  bar 进度条指针
 * @param  x 左上角X坐标
 * @param  y 左上角Y坐标
 * @param  width 进度条宽度
 * @param  height 进度条高度
 * @param  bg_color 背景颜色
 * @param  progress_color 进度条颜色
 * @param  border_color 边框颜色
 * @retval 无
 */
void TFT_ProgressBar_Init(TFT_ProgressBar* bar, uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                        uint16_t bg_color, uint16_t progress_color, uint16_t border_color);

/**
 * @brief  绘制进度条
 * @param  htft TFT句柄指针
 * @param  bar 进度条指针
 * @retval 无
 */
void TFT_ProgressBar_Draw(TFT_HandleTypeDef* htft, TFT_ProgressBar* bar);

/**
 * @brief  设置进度条进度
 * @param  bar 进度条指针
 * @param  progress 进度值(0-100)
 * @retval 无
 */
void TFT_ProgressBar_SetProgress(TFT_ProgressBar* bar, uint8_t progress);

//----------------- 开关相关函数声明 -----------------

/**
 * @brief  初始化开关
 * @param  sw 开关指针
 * @param  x 左上角X坐标
 * @param  y 左上角Y坐标
 * @param  width 开关宽度
 * @param  height 开关高度
 * @param  on_color 开启状态颜色
 * @param  off_color 关闭状态颜色
 * @param  thumb_color 滑块颜色
 * @retval 无
 */
void TFT_Switch_Init(TFT_Switch* sw, uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                   uint16_t on_color, uint16_t off_color, uint16_t thumb_color);

/**
 * @brief  绘制开关
 * @param  htft TFT句柄指针
 * @param  sw 开关指针
 * @retval 无
 */
void TFT_Switch_Draw(TFT_HandleTypeDef* htft, TFT_Switch* sw);

/**
 * @brief  设置开关状态
 * @param  sw 开关指针
 * @param  state 开关状态(true=开,false=关)
 * @retval 无
 */
void TFT_Switch_SetState(TFT_Switch* sw, bool state);

/**
 * @brief  切换开关状态
 * @param  sw 开关指针
 * @retval 无
 */
void TFT_Switch_Toggle(TFT_Switch* sw);

/**
 * @brief  检查是否点击了开关
 * @param  sw 开关指针
 * @param  touch_x 触摸X坐标
 * @param  touch_y 触摸Y坐标
 * @retval bool 是否点击了开关
 */
bool TFT_Switch_IsPressed(TFT_Switch* sw, uint16_t touch_x, uint16_t touch_y);

//----------------- 图标按钮相关函数声明 -----------------

/**
 * @brief  初始化图标按钮
 * @param  btn 图标按钮指针
 * @param  x 左上角X坐标
 * @param  y 左上角Y坐标
 * @param  width 按钮宽度
 * @param  height 按钮高度
 * @param  icon 图标数据指针
 * @param  icon_width 图标宽度
 * @param  icon_height 图标高度
 * @param  icon_color 图标颜色
 * @param  bg_color 背景颜色
 * @retval 无
 */
void TFT_IconButton_Init(TFT_IconButton* btn, uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                        const uint8_t* icon, uint16_t icon_width, uint16_t icon_height,
                        uint16_t icon_color, uint16_t bg_color);

/**
 * @brief  绘制图标按钮
 * @param  htft TFT句柄指针
 * @param  btn 图标按钮指针
 * @retval 无
 */
void TFT_IconButton_Draw(TFT_HandleTypeDef* htft, TFT_IconButton* btn);

/**
 * @brief  设置图标按钮状态
 * @param  btn 图标按钮指针
 * @param  state 按钮状态
 * @retval 无
 */
void TFT_IconButton_SetState(TFT_IconButton* btn, TFT_Button_State state);

/**
 * @brief  检查是否点击了图标按钮
 * @param  btn 图标按钮指针
 * @param  touch_x 触摸X坐标
 * @param  touch_y 触摸Y坐标
 * @retval bool 是否点击了图标按钮
 */
bool TFT_IconButton_IsPressed(TFT_IconButton* btn, uint16_t touch_x, uint16_t touch_y);

//----------------- 标签相关函数声明 -----------------

/**
 * @brief  初始化标签
 * @param  label 标签指针
 * @param  x 左上角X坐标
 * @param  y 左上角Y坐标
 * @param  width 标签宽度
 * @param  height 标签高度
 * @param  text 标签文本
 * @param  text_color 文本颜色
 * @param  bg_color 背景颜色
 * @param  text_size 文本大小
 * @retval 无
 */
void TFT_Label_Init(TFT_Label* label, uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                  const char* text, uint16_t text_color, uint16_t bg_color, uint8_t text_size);

/**
 * @brief  绘制标签
 * @param  htft TFT句柄指针
 * @param  label 标签指针
 * @retval 无
 */
void TFT_Label_Draw(TFT_HandleTypeDef* htft, TFT_Label* label);

/**
 * @brief  设置标签文本
 * @param  label 标签指针
 * @param  text 标签文本
 * @retval 无
 */
void TFT_Label_SetText(TFT_Label* label, const char* text);

/**
 * @brief  设置标签对齐方式
 * @param  label 标签指针
 * @param  alignment 对齐方式(0=左,1=中,2=右)
 * @retval 无
 */
void TFT_Label_SetAlignment(TFT_Label* label, uint8_t alignment);

//----------------- 复选框相关函数声明 -----------------

/**
 * @brief  初始化复选框
 * @param  checkbox 复选框指针
 * @param  x 左上角X坐标
 * @param  y 左上角Y坐标
 * @param  size 复选框大小
 * @param  text 复选框文本
 * @param  text_color 文本颜色
 * @param  bg_color 背景颜色
 * @param  check_color 选中标记颜色
 * @retval 无
 */
void TFT_Checkbox_Init(TFT_Checkbox* checkbox, uint16_t x, uint16_t y, uint16_t size,
                     const char* text, uint16_t text_color, uint16_t bg_color, uint16_t check_color);

/**
 * @brief  绘制复选框
 * @param  htft TFT句柄指针
 * @param  checkbox 复选框指针
 * @retval 无
 */
void TFT_Checkbox_Draw(TFT_HandleTypeDef* htft, TFT_Checkbox* checkbox);

/**
 * @brief  设置复选框状态
 * @param  checkbox 复选框指针
 * @param  checked 是否选中
 * @retval 无
 */
void TFT_Checkbox_SetChecked(TFT_Checkbox* checkbox, bool checked);

/**
 * @brief  切换复选框状态
 * @param  checkbox 复选框指针
 * @retval 无
 */
void TFT_Checkbox_Toggle(TFT_Checkbox* checkbox);

/**
 * @brief  检查是否点击了复选框
 * @param  checkbox 复选框指针
 * @param  touch_x 触摸X坐标
 * @param  touch_y 触摸Y坐标
 * @retval bool 是否点击了复选框
 */
bool TFT_Checkbox_IsPressed(TFT_Checkbox* checkbox, uint16_t touch_x, uint16_t touch_y);

//----------------- 消息框相关函数声明 -----------------

/**
 * @brief  初始化消息框
 * @param  msgbox 消息框指针
 * @param  x 左上角X坐标
 * @param  y 左上角Y坐标
 * @param  width 消息框宽度
 * @param  height 消息框高度
 * @param  title 标题
 * @param  message 消息内容
 * @param  title_color 标题颜色
 * @param  text_color 文本颜色
 * @param  bg_color 背景颜色
 * @retval 无
 */
void TFT_MessageBox_Init(TFT_MessageBox* msgbox, uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                       const char* title, const char* message, uint16_t title_color,
                       uint16_t text_color, uint16_t bg_color);

/**
 * @brief  绘制消息框
 * @param  htft TFT句柄指针
 * @param  msgbox 消息框指针
 * @retval 无
 */
void TFT_MessageBox_Draw(TFT_HandleTypeDef* htft, TFT_MessageBox* msgbox);

/**
 * @brief  设置消息框内容
 * @param  msgbox 消息框指针
 * @param  title 标题
 * @param  message 消息内容
 * @retval 无
 */
void TFT_MessageBox_SetContent(TFT_MessageBox* msgbox, const char* title, const char* message);

#ifdef __cplusplus
}
#endif

#endif /* __TFT_UI_H */