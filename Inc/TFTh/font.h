#ifndef __FONT_H
#define __FONT_H
#include "stdint.h"
#include "string.h"
typedef struct ASCIIFont
{
  uint8_t h;
  uint8_t w;
  uint8_t *chars;
} ASCIIFont;

extern const ASCIIFont afont8x6;
extern const ASCIIFont afont12x6;
extern const ASCIIFont afont16x8;
extern const ASCIIFont afont24x12;

typedef struct Font
{
  uint8_t h;
  uint8_t w;
  const uint8_t *chars;
  uint8_t len;
  const ASCIIFont *ascii;
} Font;

extern const Font font16x16;

typedef struct Image
{
  uint8_t w;
  uint8_t h;
  const uint8_t *data;
} Image;

extern const Image bilibiliImg;
extern const Image A001Img;

#endif