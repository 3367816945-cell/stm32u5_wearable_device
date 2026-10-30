/**
 * @file    TFT_init.c
 * @brief   TFT屏幕初始化函数
 * @details 支持ST7735S、ST7735R等型号屏幕的初始化。
 */
#include "TFTh/TFT_init.h"
#include "TFTh/TFT_io.h"
#include <stdint.h>

static void TFT_Set_Direction(TFT_HandleTypeDef *htft, uint8_t direction);

void TFT_Init_ST7735S(TFT_HandleTypeDef *htft)
{
	TFT_IO_Init(htft);

	TFT_Pin_RES_Set(htft, 0);
	HAL_Delay(100);
	TFT_Pin_RES_Set(htft, 1);
	HAL_Delay(100);

	TFT_Pin_BLK_Set(htft, 1);
	HAL_Delay(100);

	TFT_Write_Command(htft, 0x01);
	HAL_Delay(150);

	TFT_Write_Command(htft, 0x11);
	HAL_Delay(255);

	TFT_Write_Command(htft, 0xB1);
	TFT_Write_Data8(htft, 0x01);
	TFT_Write_Data8(htft, 0x2C);
	TFT_Write_Data8(htft, 0x2D);

	TFT_Write_Command(htft, 0xB2);
	TFT_Write_Data8(htft, 0x01);
	TFT_Write_Data8(htft, 0x2C);
	TFT_Write_Data8(htft, 0x2D);

	TFT_Write_Command(htft, 0xB3);
	TFT_Write_Data8(htft, 0x01);
	TFT_Write_Data8(htft, 0x2C);
	TFT_Write_Data8(htft, 0x2D);
	TFT_Write_Data8(htft, 0x01);
	TFT_Write_Data8(htft, 0x2C);
	TFT_Write_Data8(htft, 0x2D);

	TFT_Write_Command(htft, 0xB4);
	TFT_Write_Data8(htft, 0x07);

	TFT_Write_Command(htft, 0xC0);
	TFT_Write_Data8(htft, 0xA2);
	TFT_Write_Data8(htft, 0x02);
	TFT_Write_Data8(htft, 0x84);

	TFT_Write_Command(htft, 0xC1);
	TFT_Write_Data8(htft, 0xC5);

	TFT_Write_Command(htft, 0xC2);
	TFT_Write_Data8(htft, 0x0A);
	TFT_Write_Data8(htft, 0x00);

	TFT_Write_Command(htft, 0xC3);
	TFT_Write_Data8(htft, 0x8A);
	TFT_Write_Data8(htft, 0x2A);

	TFT_Write_Command(htft, 0xC4);
	TFT_Write_Data8(htft, 0x8A);
	TFT_Write_Data8(htft, 0xEE);

	TFT_Write_Command(htft, 0xC5);
	TFT_Write_Data8(htft, 0x0E);

	TFT_Set_Direction(htft, htft->display_direction);

	TFT_Write_Command(htft, 0x3A);
	TFT_Write_Data8(htft, 0x05);

	TFT_Write_Command(htft, 0xE0);
	TFT_Write_Data8(htft, 0x0F);
	TFT_Write_Data8(htft, 0x1A);
	TFT_Write_Data8(htft, 0x0F);
	TFT_Write_Data8(htft, 0x18);
	TFT_Write_Data8(htft, 0x2F);
	TFT_Write_Data8(htft, 0x28);
	TFT_Write_Data8(htft, 0x20);
	TFT_Write_Data8(htft, 0x22);
	TFT_Write_Data8(htft, 0x1F);
	TFT_Write_Data8(htft, 0x1B);
	TFT_Write_Data8(htft, 0x23);
	TFT_Write_Data8(htft, 0x37);
	TFT_Write_Data8(htft, 0x00);
	TFT_Write_Data8(htft, 0x07);
	TFT_Write_Data8(htft, 0x02);
	TFT_Write_Data8(htft, 0x10);

	TFT_Write_Command(htft, 0xE1);
	TFT_Write_Data8(htft, 0x0F);
	TFT_Write_Data8(htft, 0x1B);
	TFT_Write_Data8(htft, 0x0F);
	TFT_Write_Data8(htft, 0x17);
	TFT_Write_Data8(htft, 0x33);
	TFT_Write_Data8(htft, 0x2C);
	TFT_Write_Data8(htft, 0x29);
	TFT_Write_Data8(htft, 0x2E);
	TFT_Write_Data8(htft, 0x30);
	TFT_Write_Data8(htft, 0x30);
	TFT_Write_Data8(htft, 0x39);
	TFT_Write_Data8(htft, 0x3F);
	TFT_Write_Data8(htft, 0x00);
	TFT_Write_Data8(htft, 0x07);
	TFT_Write_Data8(htft, 0x03);
	TFT_Write_Data8(htft, 0x10);

	TFT_Write_Command(htft, 0x13);
	HAL_Delay(10);

	TFT_Write_Command(htft, 0x29);
	HAL_Delay(20);
}

static void TFT_Set_Direction(TFT_HandleTypeDef *htft, uint8_t direction)
{
	TFT_Write_Command(htft, 0x36);

	// ST7735 需要 BGR 位 (0x08) 才能正确显示颜色
	// 0x36 寄存器 bit3 (BGR) = 1 表示 RGB 数据以 BGR 顺序解释
	switch (direction)
	{
	case 0:  // 正常方向
		TFT_Write_Data8(htft, 0x08);  // BGR
		break;
	case 1:  // 旋转90度
		TFT_Write_Data8(htft, 0xA8);  // MX+MV+BGR
		break;
	case 2:  // 旋转180度 (竖屏)
		TFT_Write_Data8(htft, 0xC8);  // MY+BGR
		break;
	case 3:  // 旋转270度
		TFT_Write_Data8(htft, 0x68);  // MX+MY+MV+BGR
		break;
	default:
		TFT_Write_Data8(htft, 0x08);
		break;
	}
}

void TFT_Init_ST7789v3(TFT_HandleTypeDef *htft)
{
	TFT_IO_Init(htft);

	TFT_Pin_RES_Set(htft, 0);
	HAL_Delay(100);
	TFT_Pin_RES_Set(htft, 1);
	HAL_Delay(100);

	TFT_Pin_BLK_Set(htft, 1);
	HAL_Delay(100);

	TFT_Write_Command(htft, 0x01);
	HAL_Delay(120);

	TFT_Write_Command(htft, 0x11);
	HAL_Delay(120);

	TFT_Set_Direction(htft, htft->display_direction);

	TFT_Write_Command(htft, 0x3A);
	TFT_Write_Data8(htft, 0x05);

	TFT_Write_Command(htft, 0xB2);
	TFT_Write_Data8(htft, 0x0C);
	TFT_Write_Data8(htft, 0x0C);
	TFT_Write_Data8(htft, 0x00);
	TFT_Write_Data8(htft, 0x33);
	TFT_Write_Data8(htft, 0x33);

	TFT_Write_Command(htft, 0xB7);
	TFT_Write_Data8(htft, 0x72);

	TFT_Write_Command(htft, 0xBB);
	TFT_Write_Data8(htft, 0x3D);

	TFT_Write_Command(htft, 0xC0);
	TFT_Write_Data8(htft, 0x2C);

	TFT_Write_Command(htft, 0xC2);
	TFT_Write_Data8(htft, 0x01);

	TFT_Write_Command(htft, 0xC3);
	TFT_Write_Data8(htft, 0x19);

	TFT_Write_Command(htft, 0xC4);
	TFT_Write_Data8(htft, 0x20);

	TFT_Write_Command(htft, 0xC6);
	TFT_Write_Data8(htft, 0x05);

	TFT_Write_Command(htft, 0xD0);
	TFT_Write_Data8(htft, 0xA4);
	TFT_Write_Data8(htft, 0xA1);

	TFT_Write_Command(htft, 0xD6);
	TFT_Write_Data8(htft, 0xA1);

	TFT_Write_Command(htft, 0xE0);
	TFT_Write_Data8(htft, 0xD0);
	TFT_Write_Data8(htft, 0x04);
	TFT_Write_Data8(htft, 0x0D);
	TFT_Write_Data8(htft, 0x11);
	TFT_Write_Data8(htft, 0x13);
	TFT_Write_Data8(htft, 0x2B);
	TFT_Write_Data8(htft, 0x3F);
	TFT_Write_Data8(htft, 0x54);
	TFT_Write_Data8(htft, 0x4C);
	TFT_Write_Data8(htft, 0x18);
	TFT_Write_Data8(htft, 0x0D);
	TFT_Write_Data8(htft, 0x0B);
	TFT_Write_Data8(htft, 0x1F);
	TFT_Write_Data8(htft, 0x23);

	TFT_Write_Command(htft, 0xE1);
	TFT_Write_Data8(htft, 0xD0);
	TFT_Write_Data8(htft, 0x04);
	TFT_Write_Data8(htft, 0x0C);
	TFT_Write_Data8(htft, 0x11);
	TFT_Write_Data8(htft, 0x13);
	TFT_Write_Data8(htft, 0x2C);
	TFT_Write_Data8(htft, 0x3F);
	TFT_Write_Data8(htft, 0x44);
	TFT_Write_Data8(htft, 0x51);
	TFT_Write_Data8(htft, 0x2F);
	TFT_Write_Data8(htft, 0x1F);
	TFT_Write_Data8(htft, 0x1F);
	TFT_Write_Data8(htft, 0x20);
	TFT_Write_Data8(htft, 0x23);

	TFT_Write_Command(htft, 0x21);

	TFT_Write_Command(htft, 0x29);
	HAL_Delay(20);
}
