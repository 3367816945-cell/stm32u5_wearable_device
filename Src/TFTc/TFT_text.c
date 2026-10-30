#include "TFTh/TFT_text.h"
#include "TFTh/TFT_io.h"
#include "TFTh/font.h"

static void _TFT_Draw_Glyph(TFT_HandleTypeDef *htft, uint16_t x, uint16_t y, const uint8_t *glyph_data,
                            uint8_t width, uint8_t height,
                            uint16_t color, uint16_t back_color, uint8_t mode)
{
    uint16_t col, row;
    uint8_t byte, bit;
    uint8_t bytes_per_column = (height + 7) / 8;

    TFT_Set_Address(htft, x, y, x + width - 1, y + height - 1);
    TFT_Reset_Buffer(htft);

    for (col = 0; col < width; col++)
    {
        for (uint8_t byte_idx = 0; byte_idx < bytes_per_column; byte_idx++)
        {
            byte = glyph_data[col * bytes_per_column + byte_idx];
            for (row = 0; row < 8; row++)
            {
                uint16_t pixel_row = byte_idx * 8 + row;
                if (pixel_row >= height)
                    break;

                bit = (byte >> row) & 0x01;

                if (bit)
                {
                    TFT_Buffer_Write16(htft, color);
                }
                else if (mode == 0)
                {
                    TFT_Buffer_Write16(htft, back_color);
                }
            }
        }
    }
    TFT_Flush_Buffer(htft, 1);
}

void TFT_Show_String(TFT_HandleTypeDef *htft, uint16_t x, uint16_t y, const uint8_t *str, uint16_t color, uint16_t back_color, uint8_t size, uint8_t mode)
{
    uint16_t current_x = x;
    uint8_t char_width = 0;

    while (*str)
    {
        TFT_Show_Char(htft, current_x, y, *str, color, back_color, size, mode);

        if (size == 16)
            char_width = afont16x8.w;
        else if (size == 12)
            char_width = afont12x6.w;
        else
            char_width = afont8x6.w;

        current_x += char_width;
        str++;
    }
}

void TFT_Show_Char(TFT_HandleTypeDef *htft, uint16_t x, uint16_t y, uint8_t chr, uint16_t color, uint16_t back_color, uint8_t size, uint8_t mode)
{
    const ASCIIFont *ascii_font;
    const unsigned char *glyph_data;
    uint8_t char_width, char_height;
    uint16_t char_index;
    uint16_t bytes_per_char;

    if (size == 16)
    {
        ascii_font = &afont16x8;
        bytes_per_char = 16;
    }
    else if (size == 12)
    {
        ascii_font = &afont12x6;
        bytes_per_char = 12;
    }
    else
    {
        ascii_font = &afont8x6;
        size = 8;
        bytes_per_char = 6;
    }

    char_width = ascii_font->w;
    char_height = ascii_font->h;

    if (chr < ' ' || chr > '~')
    {
        chr = ' ';
    }

    char_index = chr - ' ';
    glyph_data = ascii_font->chars + char_index * bytes_per_char;

    _TFT_Draw_Glyph(htft, x, y, glyph_data, char_width, char_height, color, back_color, mode);
}
