/*
 * @file    TFT_text.h
 * @brief   TFT屏幕文本显示函数头文件
 * @details 定义了TFT屏幕字符和字符串显示函数接口
 */
#ifndef __TFT_TEXT_H
#define __TFT_TEXT_H

#include "main.h"
#include "TFTh/TFT_io.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void TFT_Show_Char(TFT_HandleTypeDef *htft, uint16_t x, uint16_t y, uint8_t chr, uint16_t color, uint16_t back_color, uint8_t size, uint8_t mode);
    void TFT_Show_String(TFT_HandleTypeDef *htft, uint16_t x, uint16_t y, const uint8_t *str, uint16_t color, uint16_t back_color, uint8_t size, uint8_t mode);

#ifdef __cplusplus
}
#endif

#endif