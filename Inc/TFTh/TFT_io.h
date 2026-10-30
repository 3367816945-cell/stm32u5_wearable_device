/*
 * @file    tft_io.h
 * @brief   TFT底层IO驱动头文件
 * @details 定义了与硬件交互的底层函数接口，包括 GPIO 控制、SPI 通信和缓冲区管理。
 */
#ifndef __TFT_IO_H
#define __TFT_IO_H

#include "main.h"
#include <stdint.h>
#include "TFT_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        SPI_HandleTypeDef *spi_handle;
        GPIO_TypeDef *cs_port;
        uint16_t cs_pin;
        GPIO_TypeDef *dc_port;
        uint16_t dc_pin;
        GPIO_TypeDef *res_port;
        uint16_t res_pin;
        GPIO_TypeDef *bl_port;
        uint16_t bl_pin;

        uint8_t *tx_buffer;
        uint16_t buffer_size;
        uint16_t buffer_write_index;

        uint8_t is_dma_enabled;
        volatile uint8_t is_dma_transfer_active;

        uint8_t display_direction;
        uint8_t x_offset;
        uint8_t y_offset;
    } TFT_HandleTypeDef;

    void TFT_Pin_RES_Set(TFT_HandleTypeDef *htft, uint8_t level);
    void TFT_Pin_DC_Set(TFT_HandleTypeDef *htft, uint8_t level);
    void TFT_Pin_CS_Set(TFT_HandleTypeDef *htft, uint8_t level);
    void TFT_Pin_BLK_Set(TFT_HandleTypeDef *htft, uint8_t level);

    void TFT_Init_Instance(TFT_HandleTypeDef *htft, SPI_HandleTypeDef *hspi,
                           GPIO_TypeDef *cs_port, uint16_t cs_pin);
    void TFT_Config_Pins(TFT_HandleTypeDef *htft,
                         GPIO_TypeDef *dc_port, uint16_t dc_pin,
                         GPIO_TypeDef *res_port, uint16_t res_pin,
                         GPIO_TypeDef *bl_port, uint16_t bl_pin);
    void TFT_Config_Display(TFT_HandleTypeDef *htft,
                            uint8_t display_direction,
                            uint8_t x_offset, uint8_t y_offset);
    void TFT_IO_Init(TFT_HandleTypeDef *htft);
    void TFT_SPI_Send(TFT_HandleTypeDef *htft, uint8_t *data_buffer, uint16_t length, uint8_t wait_completion);
    void TFT_Buffer_Write16(TFT_HandleTypeDef *htft, uint16_t data);
    void TFT_Flush_Buffer(TFT_HandleTypeDef *htft, uint8_t wait_completion);
    void TFT_Reset_Buffer(TFT_HandleTypeDef *htft);
    void TFT_Write_Data8(TFT_HandleTypeDef *htft, uint8_t data);
    void TFT_Write_Data16(TFT_HandleTypeDef *htft, uint16_t data);
    void TFT_Write_Command(TFT_HandleTypeDef *htft, uint8_t command);
    void TFT_Set_Address(TFT_HandleTypeDef *htft, uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end);

    int TFT_Platform_SPI_Transmit_Blocking(SPI_HandleTypeDef *spi_handle, uint8_t *pData, uint16_t Size, uint32_t Timeout);
    int TFT_Platform_SPI_Transmit_DMA_Start(SPI_HandleTypeDef *spi_handle, uint8_t *pData, uint16_t Size);

    uint16_t TFT_RGB(uint8_t r, uint8_t g, uint8_t b);

#ifdef __cplusplus
}
#endif

#endif