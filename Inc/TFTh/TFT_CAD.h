#ifndef __TFT_CAD_H__
#define __TFT_CAD_H__

#include <stdint.h>
#include "TFTh/TFT_io.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        uint16_t x;
        uint16_t y;
    } TFT_Point;

    void TFT_Fill_Area(TFT_HandleTypeDef *htft, uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t color);
    void TFT_Draw_Point(TFT_HandleTypeDef *htft, uint16_t x, uint16_t y, uint16_t color);
    void TFT_Draw_MultiPoint(TFT_HandleTypeDef *htft, const TFT_Point points[], uint16_t count, uint16_t color);
    void TFT_Draw_Fast_HLine(TFT_HandleTypeDef *htft, uint16_t x, uint16_t y, uint16_t width, uint16_t color);
    void TFT_Draw_Fast_VLine(TFT_HandleTypeDef *htft, uint16_t x, uint16_t y, uint16_t height, uint16_t color);
    void TFT_Draw_Line(TFT_HandleTypeDef *htft, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
    void TFT_Draw_Rectangle(TFT_HandleTypeDef *htft, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
    void TFT_Fill_Rectangle(TFT_HandleTypeDef *htft, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
    void TFT_Draw_Quarter_Circle(TFT_HandleTypeDef *htft, uint16_t centerX, uint16_t centerY, uint8_t radius, uint8_t cornerMask, uint16_t color);
    void TFT_Fill_Quarter_Circle(TFT_HandleTypeDef *htft, uint16_t centerX, uint16_t centerY, uint8_t radius, uint8_t cornerMask, uint16_t color);
    void TFT_Draw_Circle(TFT_HandleTypeDef *htft, uint16_t x0, uint16_t y0, uint8_t r, uint16_t color);
    void TFT_Fill_Circle(TFT_HandleTypeDef *htft, uint16_t x0, uint16_t y0, uint8_t r, uint16_t color);
    void TFT_Draw_Triangle(TFT_HandleTypeDef *htft, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color);
    void TFT_Fill_Triangle(TFT_HandleTypeDef *htft, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color);
    void TFT_Draw_Rounded_Rectangle(TFT_HandleTypeDef *htft, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t radius, uint16_t color);
    void TFT_Fill_Rounded_Rectangle(TFT_HandleTypeDef *htft, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t radius, uint16_t color);
    void TFT_Draw_Ellipse(TFT_HandleTypeDef *htft, uint16_t centerX, uint16_t centerY, uint16_t radiusX, uint16_t radiusY, uint16_t color);
    void TFT_Fill_Ellipse(TFT_HandleTypeDef *htft, uint16_t centerX, uint16_t centerY, uint16_t radiusX, uint16_t radiusY, uint16_t color);
    void TFT_Draw_Bezier2(TFT_HandleTypeDef *htft, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                          uint16_t x2, uint16_t y2, uint8_t segments, uint16_t color);
    void TFT_Draw_Polygon(TFT_HandleTypeDef *htft, const TFT_Point points[], uint16_t numPoints, uint16_t color);
    void TFT_Fill_Polygon(TFT_HandleTypeDef *htft, const TFT_Point points[], uint16_t numPoints, uint16_t color);
    void TFT_Draw_Arc(TFT_HandleTypeDef *htft, uint16_t centerX, uint16_t centerY, uint8_t radius,
                      uint16_t startAngle, uint16_t endAngle, uint16_t color);

#ifdef __cplusplus
}
#endif

#endif