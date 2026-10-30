/**
 * @file    TouchGFX_DisplayDriver.cpp
 * @brief   TouchGFX Partial Frame Buffer 显示驱动实现
 * @details 实现 touchgfxDisplayDriverTransmitBlock 和 touchgfxDisplayDriverTransmitActive 函数
 *          通过 SPI 将像素数据传输到 ST7735 屏幕
 */

#include <touchgfx/hal/HAL.hpp>
#include <touchgfx/hal/OSWrappers.hpp>
#include "TFTh/TFT_io.h"
#include "TFTh/TFT_config.h"
#include "stm32u5xx_hal.h"
#include <stdio.h>

extern TFT_HandleTypeDef htft;  // 来自 main.c

// 传输状态标志
static volatile bool transmitActive = false;
static uint32_t blockCount = 0;      // 传输块计数
static uint32_t totalPixels = 0;     // 总像素计数

/**
 * @brief 检查是否正在进行 SPI 数据传输
 * @return 1 表示正在传输，0 表示空闲
 */
extern "C" int touchgfxDisplayDriverTransmitActive()
{
    return transmitActive ? 1 : 0;
}

/**
 * @brief 通过 SPI 传输像素块到 ST7735 屏幕
 * @param pixels 像素数据指针 (RGB565格式，每像素2字节)
 * @param x      区域起始 X 坐标
 * @param y      区域起始 Y 坐标
 * @param w      区域宽度
 * @param h      区域高度
 */
namespace touchgfx {
    void startNewTransfer();
}

extern "C" void touchgfxDisplayDriverTransmitBlock(const uint8_t* pixels, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    transmitActive = true;
    blockCount++;
    uint32_t pixelCount = w * h;
    totalPixels += pixelCount;

    // 1. 设置 ST7735 GRAM 地址窗口
    TFT_Set_Address(&htft, x, y, x + w - 1, y + h - 1);

    // 2. 发送写内存命令 (0x2C)
    TFT_Write_Command(&htft, 0x2C);

    // 3. 发送像素数据 (RGB565, 每像素2字节)
    const uint16_t* pixelData = (const uint16_t*)pixels;
    for (uint32_t i = 0; i < pixelCount; i++)
    {
        TFT_Write_Data16(&htft, pixelData[i]);
    }

    // 4. 传输完成，释放块并启动下一传输
    transmitActive = false;
    touchgfx::startNewTransfer();  // 关键：释放帧缓冲块，否则渲染会停止
}

/**
 * @brief 获取传输统计信息
 */
extern "C" void TouchGFX_PrintStats()
{
    printf("=== TouchGFX Display Stats ===\r\n");
    printf("Blocks transmitted: %lu\r\n", blockCount);
    printf("Total pixels: %lu\r\n", totalPixels);
    printf("Current state: %s\r\n", transmitActive ? "Active" : "Idle");
}