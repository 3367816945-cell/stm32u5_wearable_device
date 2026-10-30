/*
 * @file    TFT_init.h
 * @brief   TFT屏幕初始化头文件
 * @details 定义了TFT屏幕初始化和绘图函数接口
 */
#ifndef __TFT_INIT_H
#define __TFT_INIT_H

#include "main.h"
#include "TFTh/TFT_io.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void TFT_Init_ST7735S(TFT_HandleTypeDef *htft);
    void TFT_Init_ST7789v3(TFT_HandleTypeDef *htft);

#ifdef __cplusplus
}
#endif

#endif