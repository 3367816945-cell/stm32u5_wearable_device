/*
 * @file    TFT_config.h
 * @brief   TFT屏幕配置文件
 *
 * 本文件用于配置TFT屏幕的相关参数和颜色定义。
 * 请根据您的具体硬件和需求进行相应的修改。
 */

#ifndef __TFT_CONFIG_H
#define __TFT_CONFIG_H

#define STM32HAL

// ST7735 128x128 屏幕分辨率
#define TFT_WIDTH  128
#define TFT_HEIGHT 128

#define DISPLAY_DIRECTION 2

// ST7735 128x128 偏移量（解决边缘花屏）
// 匹配成功项目配置
#define TFT_X_OFFSET 0
#define TFT_Y_OFFSET 0

#define TFT_BUFFER_SIZE 4096

#define MAX_TFT_DEVICES 4

#define WHITE 0xFFFF
#define BLACK 0x0000
#define BLUE 0x001F
#define BRED 0XF81F
#define GRED 0XFFE0
#define GBLUE 0X07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define GREEN 0x07E0
#define CYAN 0x7FFF
#define YELLOW 0xFFE0
#define BROWN 0XBC40
#define BRRED 0XFC07
#define GRAY 0X8430

#endif