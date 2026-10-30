/*
 * @file    tft.c
 * @brief   TFT绘图函数。这里的画图是以左上角为原点
 */
#include "TFTh/TFT_CAD.h"
#include "TFTh/TFT_io.h"
#include <stdlib.h>
#include <math.h>

#define SWAP_INT16(a, b) \
	{                    \
		int16_t t = a;   \
		a = b;           \
		b = t;           \
	}

void TFT_Draw_Point(TFT_HandleTypeDef *htft, uint16_t x, uint16_t y, uint16_t color)
{
	TFT_Set_Address(htft, x, y, x, y);
	TFT_Write_Data16(htft, color);
}

void TFT_Draw_MultiPoint(TFT_HandleTypeDef *htft, const TFT_Point points[], uint16_t count, uint16_t color)
{
	if (points == NULL || count == 0)
		return;

	TFT_Reset_Buffer(htft);

	for (uint16_t i = 0; i < count; i++)
	{
		TFT_Set_Address(htft, points[i].x, points[i].y, points[i].x, points[i].y);
		TFT_Buffer_Write16(htft, color);
	}

	TFT_Flush_Buffer(htft, 1);
}

void TFT_Fill_Area(TFT_HandleTypeDef *htft, uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t color)
{
	if (x_end <= x_start || y_end <= y_start)
		return;

	uint32_t total_pixels = (uint32_t)(x_end - x_start) * (y_end - y_start);
	if (total_pixels == 0)
		return;

	TFT_Set_Address(htft, x_start, y_start, x_end - 1, y_end - 1);

	TFT_Reset_Buffer(htft);

	for (uint32_t i = 0; i < total_pixels; i++)
	{
		TFT_Buffer_Write16(htft, color);
	}

	TFT_Flush_Buffer(htft, 1);
}

void TFT_Draw_Fast_HLine(TFT_HandleTypeDef *htft, uint16_t x, uint16_t y, uint16_t width, uint16_t color)
{
	if (width == 0)
		return;

	TFT_Set_Address(htft, x, y, x + width - 1, y);
	TFT_Reset_Buffer(htft);

	for (uint16_t i = 0; i < width; i++)
	{
		TFT_Buffer_Write16(htft, color);
	}

	TFT_Flush_Buffer(htft, 1);
}

void TFT_Draw_Fast_VLine(TFT_HandleTypeDef *htft, uint16_t x, uint16_t y, uint16_t height, uint16_t color)
{
	if (height == 0)
		return;

	TFT_Set_Address(htft, x, y, x, y + height - 1);
	TFT_Reset_Buffer(htft);

	for (uint16_t i = 0; i < height; i++)
	{
		TFT_Buffer_Write16(htft, color);
	}

	TFT_Flush_Buffer(htft, 1);
}

void TFT_Draw_Line(TFT_HandleTypeDef *htft, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	if (y1 == y2)
	{
		if (x1 > x2)
		{
			SWAP_INT16(x1, x2);
		}
		TFT_Draw_Fast_HLine(htft, x1, y1, x2 - x1 + 1, color);
		return;
	}

	if (x1 == x2)
	{
		if (y1 > y2)
		{
			SWAP_INT16(y1, y2);
		}
		TFT_Draw_Fast_VLine(htft, x1, y1, y2 - y1 + 1, color);
		return;
	}

	int16_t deltaX = abs(x2 - x1);
	int16_t deltaY = abs(y2 - y1);
	int16_t stepX = (x1 < x2) ? 1 : -1;
	int16_t stepY = (y1 < y2) ? 1 : -1;
	int16_t currentX = x1;
	int16_t currentY = y1;
	int16_t errorTerm;

	if (deltaX > deltaY)
	{
		errorTerm = deltaX / 2;
		while (currentX != x2)
		{
			TFT_Draw_Point(htft, currentX, currentY, color);
			errorTerm -= deltaY;
			if (errorTerm < 0)
			{
				currentY += stepY;
				errorTerm += deltaX;
			}
			currentX += stepX;
		}
	}
	else
	{
		errorTerm = deltaY / 2;
		while (currentY != y2)
		{
			TFT_Draw_Point(htft, currentX, currentY, color);
			errorTerm -= deltaX;
			if (errorTerm < 0)
			{
				currentX += stepX;
				errorTerm += deltaY;
			}
			currentY += stepY;
		}
	}
	TFT_Draw_Point(htft, currentX, currentY, color);
	TFT_Flush_Buffer(htft, 1);
}

void TFT_Draw_Rectangle(TFT_HandleTypeDef *htft, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	TFT_Draw_Fast_HLine(htft, x1, y1, x2 - x1 + 1, color);
	TFT_Draw_Fast_HLine(htft, x1, y2, x2 - x1 + 1, color);
	TFT_Draw_Fast_VLine(htft, x1, y1, y2 - y1 + 1, color);
	TFT_Draw_Fast_VLine(htft, x2, y1, y2 - y1 + 1, color);
}

void TFT_Fill_Rectangle(TFT_HandleTypeDef *htft, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	if (x1 > x2)
	{
		uint16_t temp = x1;
		x1 = x2;
		x2 = temp;
	}

	if (y1 > y2)
	{
		uint16_t temp = y1;
		y1 = y2;
		y2 = temp;
	}

	TFT_Fill_Area(htft, x1, y1, x2 + 1, y2 + 1, color);
}

void TFT_Draw_Circle(TFT_HandleTypeDef *htft, uint16_t x0, uint16_t y0, uint8_t r, uint16_t color)
{
	int16_t plotX = 0;
	int16_t plotY = r;
	int16_t decisionParam = 3 - (r << 1);
	TFT_Point circlePoints[8];

	if (r > 0)
	{
		TFT_Draw_Point(htft, x0, y0 + r, color);
		TFT_Draw_Point(htft, x0, y0 - r, color);
		TFT_Draw_Point(htft, x0 + r, y0, color);
		TFT_Draw_Point(htft, x0 - r, y0, color);
		TFT_Flush_Buffer(htft, 1);
	}
	else
	{
		TFT_Draw_Point(htft, x0, y0, color);
		TFT_Flush_Buffer(htft, 1);
		return;
	}

	while (plotX < plotY)
	{
		plotX++;

		if (decisionParam < 0)
		{
			decisionParam += (plotX << 2) + 6;
		}
		else
		{
			plotY--;
			decisionParam += ((plotX - plotY) << 2) + 10;
		}

		if (plotX == plotY)
		{
			circlePoints[0] = (TFT_Point){x0 + plotX, y0 + plotY};
			circlePoints[1] = (TFT_Point){x0 - plotX, y0 + plotY};
			circlePoints[2] = (TFT_Point){x0 + plotX, y0 - plotY};
			circlePoints[3] = (TFT_Point){x0 - plotX, y0 - plotY};
			TFT_Draw_MultiPoint(htft, circlePoints, 4, color);
		}
		else
		{
			circlePoints[0] = (TFT_Point){x0 + plotX, y0 + plotY};
			circlePoints[1] = (TFT_Point){x0 - plotX, y0 + plotY};
			circlePoints[2] = (TFT_Point){x0 + plotX, y0 - plotY};
			circlePoints[3] = (TFT_Point){x0 - plotX, y0 - plotY};
			circlePoints[4] = (TFT_Point){x0 + plotY, y0 + plotX};
			circlePoints[5] = (TFT_Point){x0 - plotY, y0 + plotX};
			circlePoints[6] = (TFT_Point){x0 + plotY, y0 - plotX};
			circlePoints[7] = (TFT_Point){x0 - plotY, y0 - plotX};
			TFT_Draw_MultiPoint(htft, circlePoints, 8, color);
		}
	}
}

void TFT_Fill_Circle(TFT_HandleTypeDef *htft, uint16_t x0, uint16_t y0, uint8_t r, uint16_t color)
{
	if (r == 0)
	{
		TFT_Draw_Point(htft, x0, y0, color);
		TFT_Flush_Buffer(htft, 1);
		return;
	}

	int16_t plotX = 0;
	int16_t plotY = r;
	int16_t decisionParam = 3 - (r << 1);

	TFT_Draw_Fast_HLine(htft, x0 - r, y0, 2 * r + 1, color);

	while (plotX < plotY)
	{
		plotX++;

		if (decisionParam < 0)
		{
			decisionParam += (plotX << 2) + 6;
		}
		else
		{
			TFT_Draw_Fast_HLine(htft, x0 - plotX, y0 + plotY, 2 * plotX + 1, color);
			TFT_Draw_Fast_HLine(htft, x0 - plotX, y0 - plotY, 2 * plotX + 1, color);
			plotY--;
			decisionParam += ((plotX - plotY) << 2) + 10;
		}

		TFT_Draw_Fast_HLine(htft, x0 - plotY, y0 + plotX, 2 * plotY + 1, color);
		TFT_Draw_Fast_HLine(htft, x0 - plotY, y0 - plotX, 2 * plotY + 1, color);
	}
}

void TFT_Draw_Triangle(TFT_HandleTypeDef *htft, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color)
{
	TFT_Draw_Line(htft, x1, y1, x2, y2, color);
	TFT_Draw_Line(htft, x2, y2, x3, y3, color);
	TFT_Draw_Line(htft, x3, y3, x1, y1, color);
}

void TFT_Fill_Triangle(TFT_HandleTypeDef *htft, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color)
{
	int16_t scanlineStartX, scanlineEndX;
	int16_t currentY, lastY;
	int16_t deltaX1, deltaY1, deltaX2, deltaY2, deltaX3, deltaY3;
	int32_t edge1Accumulator, edge2Accumulator;

	if (y1 > y2)
	{
		SWAP_INT16(y1, y2);
		SWAP_INT16(x1, x2);
	}
	if (y2 > y3)
	{
		SWAP_INT16(y3, y2);
		SWAP_INT16(x3, x2);
	}
	if (y1 > y2)
	{
		SWAP_INT16(y1, y2);
		SWAP_INT16(x1, x2);
	}

	if (y1 == y3)
	{
		scanlineStartX = scanlineEndX = x1;
		if (x2 < scanlineStartX)
			scanlineStartX = x2;
		else if (x2 > scanlineEndX)
			scanlineEndX = x2;
		if (x3 < scanlineStartX)
			scanlineStartX = x3;
		else if (x3 > scanlineEndX)
			scanlineEndX = x3;
		TFT_Draw_Fast_HLine(htft, scanlineStartX, y1, scanlineEndX - scanlineStartX + 1, color);
		return;
	}

	deltaX1 = x2 - x1;
	deltaY1 = y2 - y1;
	deltaX2 = x3 - x1;
	deltaY2 = y3 - y1;
	deltaX3 = x3 - x2;
	deltaY3 = y3 - y2;

	edge1Accumulator = 0;
	edge2Accumulator = 0;

	if (y1 != y2)
	{
		lastY = y2 - 1;
		for (currentY = y1; currentY <= lastY; currentY++)
		{
			scanlineStartX = x1 + (edge1Accumulator / deltaY1);
			scanlineEndX = x1 + (edge2Accumulator / deltaY2);
			edge1Accumulator += deltaX1;
			edge2Accumulator += deltaX2;
			if (scanlineStartX > scanlineEndX)
				SWAP_INT16(scanlineStartX, scanlineEndX);
			TFT_Draw_Fast_HLine(htft, scanlineStartX, currentY, scanlineEndX - scanlineStartX + 1, color);
		}
	}

	edge1Accumulator = (int32_t)deltaX3 * (currentY - y2);

	if (y2 != y3)
	{
		lastY = y3;
		for (; currentY <= lastY; currentY++)
		{
			scanlineStartX = x2 + (edge1Accumulator / deltaY3);
			scanlineEndX = x1 + (edge2Accumulator / deltaY2);
			edge1Accumulator += deltaX3;
			edge2Accumulator += deltaX2;
			if (scanlineStartX > scanlineEndX)
				SWAP_INT16(scanlineStartX, scanlineEndX);
			TFT_Draw_Fast_HLine(htft, scanlineStartX, currentY, scanlineEndX - scanlineStartX + 1, color);
		}
	}
}

void TFT_Draw_Quarter_Circle(TFT_HandleTypeDef *htft, uint16_t centerX, uint16_t centerY, uint8_t radius, uint8_t cornerMask, uint16_t color)
{
	int16_t plotX = 0;
	int16_t plotY = radius;
	int16_t decisionParam = 3 - (radius << 1);

	if (cornerMask == 1)
	{
		TFT_Draw_Point(htft, centerX + radius, centerY, color);
		TFT_Draw_Point(htft, centerX, centerY - radius, color);
	}
	if (cornerMask == 2)
	{
		TFT_Draw_Point(htft, centerX + radius, centerY, color);
		TFT_Draw_Point(htft, centerX, centerY + radius, color);
	}
	if (cornerMask == 4)
	{
		TFT_Draw_Point(htft, centerX - radius, centerY, color);
		TFT_Draw_Point(htft, centerX, centerY + radius, color);
	}
	if (cornerMask == 8)
	{
		TFT_Draw_Point(htft, centerX - radius, centerY, color);
		TFT_Draw_Point(htft, centerX, centerY - radius, color);
	}

	while (plotX < plotY)
	{
		plotX++;
		if (decisionParam < 0)
		{
			decisionParam += (plotX << 2) + 6;
		}
		else
		{
			plotY--;
			decisionParam += ((plotX - plotY) << 2) + 10;
		}

		if (plotX != plotY)
		{
			if (cornerMask & 0x1)
			{
				TFT_Draw_Point(htft, centerX + plotX, centerY - plotY, color);
				TFT_Draw_Point(htft, centerX + plotY, centerY - plotX, color);
			}
			if (cornerMask & 0x2)
			{
				TFT_Draw_Point(htft, centerX + plotY, centerY + plotX, color);
				TFT_Draw_Point(htft, centerX + plotX, centerY + plotY, color);
			}
			if (cornerMask & 0x4)
			{
				TFT_Draw_Point(htft, centerX - plotX, centerY + plotY, color);
				TFT_Draw_Point(htft, centerX - plotY, centerY + plotX, color);
			}
			if (cornerMask & 0x8)
			{
				TFT_Draw_Point(htft, centerX - plotY, centerY - plotX, color);
				TFT_Draw_Point(htft, centerX - plotX, centerY - plotY, color);
			}
		}
		else
		{
			if (cornerMask & 0x1)
				TFT_Draw_Point(htft, centerX + plotX, centerY - plotY, color);
			if (cornerMask & 0x2)
				TFT_Draw_Point(htft, centerX + plotX, centerY + plotY, color);
			if (cornerMask & 0x4)
				TFT_Draw_Point(htft, centerX - plotX, centerY + plotY, color);
			if (cornerMask & 0x8)
				TFT_Draw_Point(htft, centerX - plotX, centerY - plotY, color);
		}
	}
	TFT_Flush_Buffer(htft, 1);
}

void TFT_Fill_Quarter_Circle(TFT_HandleTypeDef *htft, uint16_t centerX, uint16_t centerY, uint8_t radius, uint8_t cornerMask, uint16_t color)
{
	int16_t plotX = 0;
	int16_t plotY = radius;
	int16_t decisionParam = 3 - (radius << 1);
	uint16_t hlineWidth;

	if (cornerMask & 0x1)
	{
		TFT_Draw_Fast_VLine(htft, centerX, centerY - radius, radius + 1, color);
		TFT_Draw_Fast_HLine(htft, centerX, centerY, radius + 1, color);
	}
	if (cornerMask & 0x2)
	{
		TFT_Draw_Fast_VLine(htft, centerX, centerY, radius + 1, color);
		TFT_Draw_Fast_HLine(htft, centerX, centerY, radius + 1, color);
	}
	if (cornerMask & 0x4)
	{
		TFT_Draw_Fast_VLine(htft, centerX, centerY, radius + 1, color);
		TFT_Draw_Fast_HLine(htft, centerX - radius, centerY, radius + 1, color);
	}
	if (cornerMask & 0x8)
	{
		TFT_Draw_Fast_VLine(htft, centerX, centerY - radius, radius + 1, color);
		TFT_Draw_Fast_HLine(htft, centerX - radius, centerY, radius + 1, color);
	}

	while (plotX < plotY)
	{
		plotX++;
		if (decisionParam < 0)
		{
			decisionParam += (plotX << 2) + 6;
		}
		else
		{
			hlineWidth = plotX + 1;
			if (cornerMask & 0x1)
				TFT_Draw_Fast_HLine(htft, centerX, centerY - plotY, hlineWidth, color);
			if (cornerMask & 0x2)
				TFT_Draw_Fast_HLine(htft, centerX, centerY + plotY, hlineWidth, color);
			if (cornerMask & 0x4)
				TFT_Draw_Fast_HLine(htft, centerX - plotX, centerY + plotY, hlineWidth, color);
			if (cornerMask & 0x8)
				TFT_Draw_Fast_HLine(htft, centerX - plotX, centerY - plotY, hlineWidth, color);

			plotY--;
			decisionParam += ((plotX - plotY) << 2) + 10;
		}

		hlineWidth = plotY + 1;
		if (cornerMask & 0x1)
			TFT_Draw_Fast_HLine(htft, centerX, centerY - plotX, hlineWidth, color);
		if (cornerMask & 0x2)
			TFT_Draw_Fast_HLine(htft, centerX, centerY + plotX, hlineWidth, color);
		if (cornerMask & 0x4)
			TFT_Draw_Fast_HLine(htft, centerX - plotY, centerY + plotX, hlineWidth, color);
		if (cornerMask & 0x8)
			TFT_Draw_Fast_HLine(htft, centerX - plotY, centerY - plotX, hlineWidth, color);
	}
}

void TFT_Draw_Rounded_Rectangle(TFT_HandleTypeDef *htft, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t radius, uint16_t color)
{
	if (width == 0 || height == 0)
		return;
	if (radius > width / 2)
		radius = width / 2;
	if (radius > height / 2)
		radius = height / 2;

	TFT_Draw_Fast_HLine(htft, x + radius, y, width - 2 * radius, color);
	TFT_Draw_Fast_HLine(htft, x + radius, y + height - 1, width - 2 * radius, color);
	TFT_Draw_Fast_VLine(htft, x, y + radius, height - 2 * radius, color);
	TFT_Draw_Fast_VLine(htft, x + width - 1, y + radius, height - 2 * radius, color);

	TFT_Draw_Quarter_Circle(htft, x + radius, y + radius, radius, 8, color);
	TFT_Draw_Quarter_Circle(htft, x + width - radius - 1, y + radius, radius, 1, color);
	TFT_Draw_Quarter_Circle(htft, x + width - radius - 1, y + height - radius - 1, radius, 2, color);
	TFT_Draw_Quarter_Circle(htft, x + radius, y + height - radius - 1, radius, 4, color);
}

void TFT_Fill_Rounded_Rectangle(TFT_HandleTypeDef *htft, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t radius, uint16_t color)
{
	if (width == 0 || height == 0)
		return;
	if (radius > width / 2)
		radius = width / 2;
	if (radius > height / 2)
		radius = height / 2;

	TFT_Fill_Rectangle(htft, x + radius, y, x + width - radius - 1, y + height - 1, color);
	TFT_Fill_Rectangle(htft, x, y + radius, x + radius - 1, y + height - radius - 1, color);
	TFT_Fill_Rectangle(htft, x + width - radius, y + radius, x + width - 1, y + height - radius - 1, color);

	TFT_Fill_Quarter_Circle(htft, x + radius, y + radius, radius, 8, color);
	TFT_Fill_Quarter_Circle(htft, x + width - radius - 1, y + radius, radius, 1, color);
	TFT_Fill_Quarter_Circle(htft, x + width - radius - 1, y + height - radius - 1, radius, 2, color);
	TFT_Fill_Quarter_Circle(htft, x + radius, y + height - radius - 1, radius, 4, color);
}

void TFT_Draw_Ellipse(TFT_HandleTypeDef *htft, uint16_t centerX, uint16_t centerY, uint16_t radiusX, uint16_t radiusY, uint16_t color)
{
	int32_t x = 0;
	int32_t y = radiusY;

	int32_t radiusX2 = radiusX * radiusX;
	int32_t radiusY2 = radiusY * radiusY;
	int32_t error = radiusY2 - (radiusX2 * radiusY) + (radiusX2 / 4);

	TFT_Point ellipsePoints[4];

	TFT_Reset_Buffer(htft);

	while (radiusX2 * y > radiusY2 * x)
	{
		ellipsePoints[0] = (TFT_Point){centerX + x, centerY + y};
		ellipsePoints[1] = (TFT_Point){centerX - x, centerY + y};
		ellipsePoints[2] = (TFT_Point){centerX + x, centerY - y};
		ellipsePoints[3] = (TFT_Point){centerX - x, centerY - y};

		for (uint8_t i = 0; i < 4; i++)
		{
			TFT_Set_Address(htft, ellipsePoints[i].x, ellipsePoints[i].y,
							ellipsePoints[i].x, ellipsePoints[i].y);
			TFT_Buffer_Write16(htft, color);
		}

		if (error >= 0)
		{
			y--;
			error -= 2 * radiusX2 * y;
		}
		x++;
		error += 2 * radiusY2 * x + radiusY2;
	}

	error = radiusY2 * x * x + radiusX2 * y * y - radiusX2 * radiusY2;

	while (y >= 0)
	{
		ellipsePoints[0] = (TFT_Point){centerX + x, centerY + y};
		ellipsePoints[1] = (TFT_Point){centerX - x, centerY + y};
		ellipsePoints[2] = (TFT_Point){centerX + x, centerY - y};
		ellipsePoints[3] = (TFT_Point){centerX - x, centerY - y};

		for (uint8_t i = 0; i < 4; i++)
		{
			TFT_Set_Address(htft, ellipsePoints[i].x, ellipsePoints[i].y,
							ellipsePoints[i].x, ellipsePoints[i].y);
			TFT_Buffer_Write16(htft, color);
		}

		if (error <= 0)
		{
			x++;
			error += 2 * radiusY2 * x;
		}
		y--;
		error += -2 * radiusX2 * y + radiusX2;
	}

	TFT_Flush_Buffer(htft, 1);
}

void TFT_Fill_Ellipse(TFT_HandleTypeDef *htft, uint16_t centerX, uint16_t centerY, uint16_t radiusX, uint16_t radiusY, uint16_t color)
{
	int32_t x = 0;
	int32_t y = radiusY;
	int32_t radiusX2 = radiusX * radiusX;
	int32_t radiusY2 = radiusY * radiusY;
	int32_t error = radiusY2 - (radiusX2 * radiusY) + (radiusX2 / 4);

	while (radiusX2 * y > radiusY2 * x)
	{
		TFT_Draw_Fast_HLine(htft, centerX - x, centerY + y, 2 * x + 1, color);
		TFT_Draw_Fast_HLine(htft, centerX - x, centerY - y, 2 * x + 1, color);

		if (error >= 0)
		{
			y--;
			error -= 2 * radiusX2 * y;
		}
		x++;
		error += 2 * radiusY2 * x + radiusY2;
	}

	error = radiusY2 * x * x + radiusX2 * y * y - radiusX2 * radiusY2;

	while (y >= 0)
	{
		TFT_Draw_Fast_HLine(htft, centerX - x, centerY + y, 2 * x + 1, color);
		TFT_Draw_Fast_HLine(htft, centerX - x, centerY - y, 2 * x + 1, color);

		if (error <= 0)
		{
			x++;
			error += 2 * radiusY2 * x;
		}
		y--;
		error += -2 * radiusX2 * y + radiusX2;
	}
}

void TFT_Draw_Bezier2(TFT_HandleTypeDef *htft, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
					  uint16_t x2, uint16_t y2, uint8_t segments, uint16_t color)
{
	float t, t2, t_1, t_12;
	uint16_t x, y, lx = x0, ly = y0;

	if (segments == 0)
	{
		segments = 1;
	}

	TFT_Draw_Point(htft, x0, y0, color);

	for (uint8_t i = 1; i <= segments; i++)
	{
		t = i / (float)segments;
		t2 = t * t;
		t_1 = 1.0f - t;
		t_12 = t_1 * t_1;

		x = t_12 * x0 + 2 * t_1 * t * x1 + t2 * x2;
		y = t_12 * y0 + 2 * t_1 * t * y1 + t2 * y2;

		TFT_Draw_Line(htft, lx, ly, x, y, color);
		lx = x;
		ly = y;
	}
}

void TFT_Draw_Polygon(TFT_HandleTypeDef *htft, const TFT_Point points[], uint16_t numPoints, uint16_t color)
{
	if (numPoints < 3 || points == NULL)
		return;

	for (uint16_t i = 0; i < numPoints - 1; i++)
	{
		TFT_Draw_Line(htft, points[i].x, points[i].y, points[i + 1].x, points[i + 1].y, color);
	}

	TFT_Draw_Line(htft, points[numPoints - 1].x, points[numPoints - 1].y, points[0].x, points[0].y, color);
}

void TFT_Fill_Polygon(TFT_HandleTypeDef *htft, const TFT_Point points[], uint16_t numPoints, uint16_t color)
{
	if (numPoints < 3 || points == NULL)
		return;

	int16_t minY = points[0].y;
	int16_t maxY = points[0].y;

	for (uint16_t i = 1; i < numPoints; i++)
	{
		if (points[i].y < minY)
			minY = points[i].y;
		if (points[i].y > maxY)
			maxY = points[i].y;
	}

	if (minY >= maxY)
		return;

	int16_t intersections[128];

	for (int16_t y = minY; y <= maxY; y++)
	{
		uint8_t intersectCount = 0;

		for (uint16_t i = 0; i < numPoints; i++)
		{
			uint16_t j = (i + 1) % numPoints;
			int16_t y1 = points[i].y;
			int16_t y2 = points[j].y;

			if ((y1 <= y && y2 > y) || (y2 <= y && y1 > y))
			{
				int16_t x1 = points[i].x;
				int16_t x2 = points[j].x;

				int32_t x;
				if (y1 == y2)
				{
					x = (x1 + x2) / 2;
				}
				else
				{
					x = x1 + (int32_t)(y - y1) * (x2 - x1) / (y2 - y1);
				}

				if (intersectCount < 128)
				{
					intersections[intersectCount++] = x;
				}
			}
		}

		for (uint8_t i = 1; i < intersectCount; i++)
		{
			int16_t temp = intersections[i];
			int16_t j = i;
			while (j > 0 && intersections[j - 1] > temp)
			{
				intersections[j] = intersections[j - 1];
				j--;
			}
			intersections[j] = temp;
		}

		for (uint8_t i = 0; i < intersectCount; i += 2)
		{
			if (i + 1 < intersectCount)
			{
				int16_t x1 = intersections[i];
				int16_t x2 = intersections[i + 1];
				TFT_Draw_Fast_HLine(htft, x1, y, x2 - x1 + 1, color);
			}
		}
	}
}

void TFT_Draw_Arc(TFT_HandleTypeDef *htft, uint16_t centerX, uint16_t centerY, uint8_t radius,
				  uint16_t startAngle, uint16_t endAngle, uint16_t color)
{
	startAngle = startAngle % 360;
	endAngle = endAngle % 360;

	if (endAngle < startAngle)
	{
		endAngle += 360;
	}

	uint16_t segments = (endAngle - startAngle) / 2;
	if (segments < 1)
		segments = 1;

	float angleStep = (endAngle - startAngle) * 0.0174532925f / segments;
	float currentAngle = startAngle * 0.0174532925f;

	uint16_t lastX = centerX + (uint16_t)(radius * cos(currentAngle));
	uint16_t lastY = centerY + (uint16_t)(radius * sin(currentAngle));
	TFT_Draw_Point(htft, lastX, lastY, color);

	for (uint16_t i = 1; i <= segments; i++)
	{
		currentAngle += angleStep;
		uint16_t x = centerX + (uint16_t)(radius * cos(currentAngle));
		uint16_t y = centerY + (uint16_t)(radius * sin(currentAngle));

		TFT_Draw_Line(htft, lastX, lastY, x, y, color);

		lastX = x;
		lastY = y;
	}
}
