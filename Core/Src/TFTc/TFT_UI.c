/**
 * @file    TFT_UI.c
 * @brief   TFT用户界面组件库源文件
 * @details 实现了TFT屏幕上常用的UI组件功能，如按钮、进度条、开关等
 */
#include "TFTh/TFT_UI.h"
#include "TFTh/TFT_CAD.h"
#include "TFTh/TFT_text.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

//----------------- 内部辅助函数定义 -----------------

/**
 * @brief  检查点是否在矩形区域内（碰撞检测）
 * @param  x 点的X坐标
 * @param  y 点的Y坐标
 * @param  rect_x 矩形左上角X坐标
 * @param  rect_y 矩形左上角Y坐标
 * @param  rect_w 矩形宽度
 * @param  rect_h 矩形高度
 * @retval bool 点是否在矩形内
 */
static bool _TFT_PointInRect(uint16_t x, uint16_t y, uint16_t rect_x, uint16_t rect_y, uint16_t rect_w, uint16_t rect_h)
{
    return (x >= rect_x && x < rect_x + rect_w && 
            y >= rect_y && y < rect_y + rect_h);
}

/**
 * @brief  计算居中文本的X坐标
 * @param  text 文本内容
 * @param  text_size 文本大小
 * @param  area_x 区域左上角X坐标
 * @param  area_width 区域宽度
 * @retval uint16_t 居中后的文本X坐标
 */
static uint16_t _TFT_CenterTextX(const char* text, uint8_t text_size, uint16_t area_x, uint16_t area_width)
{
    uint16_t text_width = strlen(text) * 6 * (text_size ? text_size : 1); // 估算文本宽度
    if (text_width < area_width)
    {
        return area_x + (area_width - text_width) / 2;
    }
    return area_x; // 如果文本宽度大于区域宽度，则左对齐
}

//----------------- 按钮相关函数实现 -----------------

/**
 * @brief  初始化按钮
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
                    const char* text, uint16_t text_color, uint16_t bg_color, uint16_t border_color)
{
    if (btn == NULL) return;
    
    // 初始化基础属性
    btn->base.x = x;
    btn->base.y = y;
    btn->base.width = width;
    btn->base.height = height;
    btn->base.visible = true;
    btn->base.enabled = true;
    
    // 初始化按钮特有属性
    strncpy(btn->text, text, sizeof(btn->text) - 1);
    btn->text[sizeof(btn->text) - 1] = '\0';  // 确保字符串以'\0'结尾
    
    btn->text_color = text_color;
    btn->bg_color = bg_color;
    btn->border_color = border_color;
    btn->pressed_color = (bg_color & 0xF7DE) >> 1; // 较暗的背景色
    btn->disabled_color = 0x7BEF; // 淡灰色
    btn->corner_radius = 4;       // 默认圆角半径
    btn->text_size = 12;          // 默认文本大小
    btn->state = BUTTON_NORMAL;   // 默认为正常状态
}

/**
 * @brief  绘制按钮
 * @param  htft TFT句柄指针
 * @param  btn 按钮指针
 * @retval 无
 */
void TFT_Button_Draw(TFT_HandleTypeDef* htft, TFT_Button* btn)
{
    if (htft == NULL || btn == NULL || !btn->base.visible) return;
    
    uint16_t bg_color;
    
    // 根据状态确定背景颜色
    switch (btn->state)
    {
        case BUTTON_PRESSED:
            bg_color = btn->pressed_color;
            break;
        case BUTTON_DISABLED:
            bg_color = btn->disabled_color;
            break;
        default:
            bg_color = btn->bg_color;
            break;
    }
    
    // 绘制圆角矩形按钮
    if (btn->corner_radius > 0)
    {
        TFT_Fill_Rounded_Rectangle(htft, btn->base.x, btn->base.y, 
                                   btn->base.width, btn->base.height, 
                                   btn->corner_radius, bg_color);
                                   
        TFT_Draw_Rounded_Rectangle(htft, btn->base.x, btn->base.y, 
                                   btn->base.width, btn->base.height, 
                                   btn->corner_radius, btn->border_color);
    }
    else
    {
        TFT_Fill_Rectangle(htft, btn->base.x, btn->base.y, 
                           btn->base.x + btn->base.width - 1, 
                           btn->base.y + btn->base.height - 1, bg_color);
                           
        TFT_Draw_Rectangle(htft, btn->base.x, btn->base.y, 
                           btn->base.x + btn->base.width - 1, 
                           btn->base.y + btn->base.height - 1, btn->border_color);
    }
    
    // 绘制文本（居中）
    if (btn->text[0] != '\0')
    {
        uint16_t text_x = _TFT_CenterTextX(btn->text, btn->text_size == 0 ? 1 : btn->text_size, 
                                        btn->base.x, btn->base.width);
        uint16_t text_y = btn->base.y + (btn->base.height - 8 * (btn->text_size == 0 ? 1 : btn->text_size)) / 2;
        
        uint16_t text_color = (btn->state == BUTTON_DISABLED) ? 
                             (btn->text_color & 0x7BEF) : btn->text_color;
        
        // 使用TFT_Show_String函数显示文本
        TFT_Show_String(htft, text_x, text_y, (uint8_t*)btn->text, text_color, bg_color, btn->text_size, 0);
    }
}

/**
 * @brief  设置按钮状态
 * @param  btn 按钮指针
 * @param  state 按钮状态
 * @retval 无
 */
void TFT_Button_SetState(TFT_Button* btn, TFT_Button_State state)
{
    if (btn == NULL) return;
    btn->state = state;
    // 注意：调用此函数后需要手动调用TFT_Button_Draw重绘按钮
}

/**
 * @brief  检查是否点击了按钮
 * @param  btn 按钮指针
 * @param  touch_x 触摸X坐标
 * @param  touch_y 触摸Y坐标
 * @retval bool 是否点击了按钮
 */
bool TFT_Button_IsPressed(TFT_Button* btn, uint16_t touch_x, uint16_t touch_y)
{
    if (btn == NULL || !btn->base.enabled) return false;
    
    return _TFT_PointInRect(touch_x, touch_y, btn->base.x, btn->base.y, 
                          btn->base.width, btn->base.height);
}

//----------------- 进度条相关函数实现 -----------------

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
                        uint16_t bg_color, uint16_t progress_color, uint16_t border_color)
{
    if (bar == NULL) return;
    
    // 初始化基础属性
    bar->base.x = x;
    bar->base.y = y;
    bar->base.width = width;
    bar->base.height = height;
    bar->base.visible = true;
    bar->base.enabled = true;
    
    // 初始化进度条特有属性
    bar->progress = 0;               // 初始进度为0
    bar->bg_color = bg_color;
    bar->progress_color = progress_color;
    bar->border_color = border_color;
    bar->corner_radius = 3;          // 默认圆角半径
    bar->show_percentage = true;     // 默认显示百分比
    bar->text_color = 0xFFFF;        // 默认文本颜色为白色
}

/**
 * @brief  绘制进度条
 * @param  htft TFT句柄指针
 * @param  bar 进度条指针
 * @retval 无
 */
void TFT_ProgressBar_Draw(TFT_HandleTypeDef* htft, TFT_ProgressBar* bar)
{
    if (htft == NULL || bar == NULL || !bar->base.visible) return;
    
    uint16_t progress_width;
    
    // 计算进度条实际宽度
    progress_width = (bar->progress * (bar->base.width - 4)) / 100;
    if (progress_width > bar->base.width - 4) progress_width = bar->base.width - 4;
    
    // 绘制背景和边框
    if (bar->corner_radius > 0)
    {
        // 绘制圆角矩形背景
        TFT_Fill_Rounded_Rectangle(htft, bar->base.x, bar->base.y, 
                                 bar->base.width, bar->base.height, 
                                 bar->corner_radius, bar->bg_color);
        
        // 绘制边框
        TFT_Draw_Rounded_Rectangle(htft, bar->base.x, bar->base.y, 
                                 bar->base.width, bar->base.height, 
                                 bar->corner_radius, bar->border_color);
        
        // 绘制进度填充（内部偏移2像素）
        if (progress_width > 0 && bar->progress > 0)
        {
            uint8_t inner_radius = (bar->corner_radius > 2) ? bar->corner_radius - 2 : 1;
            
            if (bar->progress >= 100)
            {
                // 如果是满进度，绘制完整的圆角矩形
                TFT_Fill_Rounded_Rectangle(htft, bar->base.x + 2, bar->base.y + 2, 
                                         bar->base.width - 4, bar->base.height - 4, 
                                         inner_radius, bar->progress_color);
            }
            else
            {
                // 部分进度，填充矩形
                TFT_Fill_Rectangle(htft, bar->base.x + 2, bar->base.y + 2, 
                                 bar->base.x + 2 + progress_width - 1, 
                                 bar->base.y + bar->base.height - 3, 
                                 bar->progress_color);
            }
        }
    }
    else
    {
        // 无圆角版本
        // 绘制背景
        TFT_Fill_Rectangle(htft, bar->base.x, bar->base.y, 
                         bar->base.x + bar->base.width - 1, 
                         bar->base.y + bar->base.height - 1, 
                         bar->bg_color);
        
        // 绘制边框
        TFT_Draw_Rectangle(htft, bar->base.x, bar->base.y, 
                         bar->base.x + bar->base.width - 1, 
                         bar->base.y + bar->base.height - 1, 
                         bar->border_color);
        
        // 绘制进度填充（内部偏移2像素）
        if (progress_width > 0 && bar->progress > 0)
        {
            TFT_Fill_Rectangle(htft, bar->base.x + 2, bar->base.y + 2, 
                             bar->base.x + 2 + progress_width - 1, 
                             bar->base.y + bar->base.height - 3, 
                             bar->progress_color);
        }
    }
    
    // 显示百分比文本（如果启用）
    if (bar->show_percentage)
    {
        char percent_text[5];
        sprintf(percent_text, "%d%%", bar->progress);
        
        uint16_t text_x = _TFT_CenterTextX(percent_text, 1, bar->base.x, bar->base.width);
        uint16_t text_y = bar->base.y + (bar->base.height - 8) / 2;
        
        // 根据进度确定文本颜色和背景色
        uint16_t text_color, text_bg_color;
        if (bar->progress > 50)
        {
            text_color = bar->text_color;
            text_bg_color = bar->progress_color;
        }
        else
        {
            text_color = bar->text_color;
            text_bg_color = bar->bg_color;
        }
        
        TFT_Show_String(htft, text_x, text_y, (uint8_t*)percent_text, text_color, text_bg_color, 16, 0);
    }
}

/**
 * @brief  设置进度条进度
 * @param  bar 进度条指针
 * @param  progress 进度值(0-100)
 * @retval 无
 */
void TFT_ProgressBar_SetProgress(TFT_ProgressBar* bar, uint8_t progress)
{
    if (bar == NULL) return;
    
    // 限制进度在0-100范围内
    if (progress > 100) progress = 100;
    
    bar->progress = progress;
    // 注意：调用此函数后需要手动调用TFT_ProgressBar_Draw重绘进度条
}

//----------------- 开关相关函数实现 -----------------

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
                   uint16_t on_color, uint16_t off_color, uint16_t thumb_color)
{
    if (sw == NULL) return;
    
    // 初始化基础属性
    sw->base.x = x;
    sw->base.y = y;
    sw->base.width = width;
    sw->base.height = height;
    sw->base.visible = true;
    sw->base.enabled = true;
    
    // 初始化开关特有属性
    sw->state = false;            // 默认关闭状态
    sw->on_color = on_color;
    sw->off_color = off_color;
    sw->thumb_color = thumb_color;
    sw->border_color = 0x8410;    // 默认边框颜色
    sw->corner_radius = height/2; // 设置圆角为高度的一半，实现椭圆效果
}

/**
 * @brief  绘制开关
 * @param  htft TFT句柄指针
 * @param  sw 开关指针
 * @retval 无
 */
void TFT_Switch_Draw(TFT_HandleTypeDef* htft, TFT_Switch* sw)
{
    if (htft == NULL || sw == NULL || !sw->base.visible) return;
    
    uint16_t bg_color = sw->state ? sw->on_color : sw->off_color;
    
    // 绘制开关背景
    TFT_Fill_Rounded_Rectangle(htft, sw->base.x, sw->base.y, 
                             sw->base.width, sw->base.height, 
                             sw->corner_radius, bg_color);
    
    // 绘制边框
    TFT_Draw_Rounded_Rectangle(htft, sw->base.x, sw->base.y, 
                             sw->base.width, sw->base.height, 
                             sw->corner_radius, sw->border_color);
    
    // 计算滑块位置
    uint16_t thumb_size = sw->base.height - 4;
    uint16_t thumb_x = sw->state ? 
                      (sw->base.x + sw->base.width - thumb_size - 2) : 
                      (sw->base.x + 2);
                      
    uint16_t thumb_y = sw->base.y + 2;
    
    // 绘制滑块
    TFT_Fill_Circle(htft, thumb_x + thumb_size/2, thumb_y + thumb_size/2, 
                  thumb_size/2, sw->thumb_color);
}

/**
 * @brief  设置开关状态
 * @param  sw 开关指针
 * @param  state 开关状态(true=开,false=关)
 * @retval 无
 */
void TFT_Switch_SetState(TFT_Switch* sw, bool state)
{
    if (sw == NULL) return;
    sw->state = state;
    // 注意：调用此函数后需要手动调用TFT_Switch_Draw重绘开关
}

/**
 * @brief  切换开关状态
 * @param  sw 开关指针
 * @retval 无
 */
void TFT_Switch_Toggle(TFT_Switch* sw)
{
    if (sw == NULL) return;
    sw->state = !sw->state;
    // 注意：调用此函数后需要手动调用TFT_Switch_Draw重绘开关
}

/**
 * @brief  检查是否点击了开关
 * @param  sw 开关指针
 * @param  touch_x 触摸X坐标
 * @param  touch_y 触摸Y坐标
 * @retval bool 是否点击了开关
 */
bool TFT_Switch_IsPressed(TFT_Switch* sw, uint16_t touch_x, uint16_t touch_y)
{
    if (sw == NULL || !sw->base.enabled) return false;
    
    return _TFT_PointInRect(touch_x, touch_y, sw->base.x, sw->base.y, 
                          sw->base.width, sw->base.height);
}

//----------------- 标签相关函数实现 -----------------

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
                  const char* text, uint16_t text_color, uint16_t bg_color, uint8_t text_size)
{
    if (label == NULL) return;
    
    // 初始化基础属性
    label->base.x = x;
    label->base.y = y;
    label->base.width = width;
    label->base.height = height;
    label->base.visible = true;
    label->base.enabled = true;
    
    // 初始化标签特有属性
    strncpy(label->text, text, sizeof(label->text) - 1);
    label->text[sizeof(label->text) - 1] = '\0';  // 确保字符串以'\0'结尾
    
    label->text_color = text_color;
    label->bg_color = bg_color;
    label->text_size = text_size;
    label->transparent_bg = false;   // 默认不透明
    label->alignment = 0;            // 默认左对齐
}

/**
 * @brief  绘制标签
 * @param  htft TFT句柄指针
 * @param  label 标签指针
 * @retval 无
 */
void TFT_Label_Draw(TFT_HandleTypeDef* htft, TFT_Label* label)
{
    if (htft == NULL || label == NULL || !label->base.visible) return;
    
    // 如果背景不透明，先绘制背景
    if (!label->transparent_bg)
    {
        TFT_Fill_Rectangle(htft, label->base.x, label->base.y, 
                         label->base.x + label->base.width - 1, 
                         label->base.y + label->base.height - 1, 
                         label->bg_color);
    }
    
    // 确定文本X坐标（根据对齐方式）
    uint16_t text_x;
    uint16_t char_width = 6 * (label->text_size ? label->text_size : 1); // 字符宽度
    uint16_t text_width = strlen(label->text) * char_width;
    
    switch (label->alignment)
    {
        case 1: // 居中对齐
            text_x = label->base.x + (label->base.width - text_width) / 2;
            break;
        case 2: // 右对齐
            text_x = label->base.x + label->base.width - text_width;
            break;
        default: // 左对齐
            text_x = label->base.x;
            break;
    }
    
    // 文本Y坐标（垂直居中）
    uint16_t char_height = 8 * (label->text_size ? label->text_size : 1);
    uint16_t text_y = label->base.y + (label->base.height - char_height) / 2;
    
    // 显示文本
    uint8_t mode = label->transparent_bg ? 1 : 0;
    TFT_Show_String(htft, text_x, text_y, (uint8_t*)label->text, 
                   label->text_color, label->bg_color, label->text_size, mode);
}

/**
 * @brief  设置标签文本
 * @param  label 标签指针
 * @param  text 标签文本
 * @retval 无
 */
void TFT_Label_SetText(TFT_Label* label, const char* text)
{
    if (label == NULL || text == NULL) return;
    
    strncpy(label->text, text, sizeof(label->text) - 1);
    label->text[sizeof(label->text) - 1] = '\0';  // 确保字符串以'\0'结尾
    
    // 注意：调用此函数后需要手动调用TFT_Label_Draw重绘标签
}

/**
 * @brief  设置标签对齐方式
 * @param  label 标签指针
 * @param  alignment 对齐方式(0=左,1=中,2=右)
 * @retval 无
 */
void TFT_Label_SetAlignment(TFT_Label* label, uint8_t alignment)
{
    if (label == NULL) return;
    
    // 确保对齐值有效
    if (alignment > 2) alignment = 0;
    
    label->alignment = alignment;
    
    // 注意：调用此函数后需要手动调用TFT_Label_Draw重绘标签
}

// 更多UI组件函数将在下一个版本中实现...