/**
 * @file    tft_io.c
 * @brief   TFT底层IO驱动实现，硬件抽象层 (HAL)
 * @details 封装了与硬件相关的操作，如 GPIO 控制和 SPI 通信 (阻塞/DMA)。
 *          提供了一个发送缓冲区以提高连续数据传输的效率。
 */
#include "TFTh/TFT_io.h"
#include <stdint.h>
#include <stdlib.h>
#include <cmsis_compiler.h>  // CMSIS头文件，包含SCB_CleanDCache等Cache管理函数

static TFT_HandleTypeDef *g_tft_handles[MAX_TFT_DEVICES] = {NULL};

static void TFT_Wait_DMA_Transfer_Complete(TFT_HandleTypeDef *htft);
static void TFT_Register_Device(TFT_HandleTypeDef *htft);

void TFT_Init_Instance(TFT_HandleTypeDef *htft, SPI_HandleTypeDef *hspi,
					   GPIO_TypeDef *cs_port, uint16_t cs_pin)
{
	htft->spi_handle = hspi;
	htft->cs_port = cs_port;
	htft->cs_pin = cs_pin;
	htft->buffer_size = TFT_BUFFER_SIZE;
	htft->buffer_write_index = 0;
	htft->tx_buffer = NULL;
	htft->display_direction = DISPLAY_DIRECTION;
	htft->x_offset = TFT_X_OFFSET;
	htft->y_offset = TFT_Y_OFFSET;
}

void TFT_Config_Pins(TFT_HandleTypeDef *htft,
					 GPIO_TypeDef *dc_port, uint16_t dc_pin,
					 GPIO_TypeDef *res_port, uint16_t res_pin,
					 GPIO_TypeDef *bl_port, uint16_t bl_pin)
{
	htft->dc_port = dc_port;
	htft->dc_pin = dc_pin;
	htft->res_port = res_port;
	htft->res_pin = res_pin;
	htft->bl_port = bl_port;
	htft->bl_pin = bl_pin;
}

void TFT_Config_Display(TFT_HandleTypeDef *htft,
						uint8_t display_direction,
						uint8_t x_offset, uint8_t y_offset)
{
	htft->display_direction = display_direction;
	htft->x_offset = x_offset;
	htft->y_offset = y_offset;
}

static void TFT_Register_Device(TFT_HandleTypeDef *htft)
{
	for (int i = 0; i < MAX_TFT_DEVICES; i++)
	{
		if (g_tft_handles[i] == NULL || g_tft_handles[i]->spi_handle == htft->spi_handle)
		{
			g_tft_handles[i] = htft;
			break;
		}
	}
}

void TFT_Pin_RES_Set(TFT_HandleTypeDef *htft, uint8_t level)
{
#ifdef STM32HAL
	HAL_GPIO_WritePin(htft->res_port, htft->res_pin, (GPIO_PinState)level);
#endif
}

void TFT_Pin_DC_Set(TFT_HandleTypeDef *htft, uint8_t level)
{
#ifdef STM32HAL
	HAL_GPIO_WritePin(htft->dc_port, htft->dc_pin, (GPIO_PinState)level);
#endif
}

void TFT_Pin_CS_Set(TFT_HandleTypeDef *htft, uint8_t level)
{
#ifdef STM32HAL
	HAL_GPIO_WritePin(htft->cs_port, htft->cs_pin, (GPIO_PinState)level);
#endif
}

void TFT_Pin_BLK_Set(TFT_HandleTypeDef *htft, uint8_t level)
{
#ifdef STM32HAL
	HAL_GPIO_WritePin(htft->bl_port, htft->bl_pin, (GPIO_PinState)level);
#endif
}

int TFT_Platform_SPI_Transmit_Blocking(SPI_HandleTypeDef *spi_handle, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
#ifdef STM32HAL
	return HAL_SPI_Transmit(spi_handle, pData, Size, Timeout);
#endif
	return -1;
}

int TFT_Platform_SPI_Transmit_DMA_Start(SPI_HandleTypeDef *spi_handle, uint8_t *pData, uint16_t Size)
{
#ifdef STM32HAL
	return HAL_SPI_Transmit_DMA(spi_handle, pData, Size);
#endif
	return -1;
}

void TFT_SPI_Send(TFT_HandleTypeDef *htft, uint8_t *data_buffer, uint16_t length, uint8_t wait_completion)
{
	if (htft == NULL || htft->spi_handle == NULL || length == 0 || data_buffer == NULL)
		return;

	TFT_Wait_DMA_Transfer_Complete(htft);

	TFT_Pin_DC_Set(htft, 1);
	TFT_Pin_CS_Set(htft, 0);

	if (htft->is_dma_enabled)
	{
		// STM32U5 D-Cache管理暂时移除，后续可通过MPU配置解决
		// DMA传输前确保数据在物理内存中
		
		htft->is_dma_transfer_active = 1;
		TFT_Platform_SPI_Transmit_DMA_Start(htft->spi_handle, data_buffer, length);
		if (wait_completion)
		{
			TFT_Wait_DMA_Transfer_Complete(htft);
			TFT_Pin_CS_Set(htft, 1);
		}
	}
	else
	{
		TFT_Platform_SPI_Transmit_Blocking(htft->spi_handle, data_buffer, length, HAL_MAX_DELAY);
		TFT_Pin_CS_Set(htft, 1);
	}
}

void TFT_Buffer_Write16(TFT_HandleTypeDef *htft, uint16_t data)
{
	if (htft == NULL || htft->tx_buffer == NULL)
		return;

	if (htft->buffer_write_index >= htft->buffer_size - 1)
	{
		TFT_Flush_Buffer(htft, 0);
	}

	htft->tx_buffer[htft->buffer_write_index++] = (data >> 8) & 0xFF;
	htft->tx_buffer[htft->buffer_write_index++] = data & 0xFF;
}

void TFT_Flush_Buffer(TFT_HandleTypeDef *htft, uint8_t wait_completion)
{
	if (htft == NULL || htft->tx_buffer == NULL || htft->buffer_write_index == 0)
		return;

	TFT_SPI_Send(htft, htft->tx_buffer, htft->buffer_write_index, wait_completion);
	htft->buffer_write_index = 0;
}

void TFT_Reset_Buffer(TFT_HandleTypeDef *htft)
{
	if (htft == NULL)
		return;
	htft->buffer_write_index = 0;
}

void TFT_IO_Init(TFT_HandleTypeDef *htft)
{
	if (htft == NULL || htft->spi_handle == NULL)
		return;

	if (htft->tx_buffer == NULL)
	{
		htft->tx_buffer = (uint8_t *)malloc(htft->buffer_size);
		if (htft->tx_buffer == NULL)
			return;
	}

	htft->buffer_write_index = 0;

#ifdef STM32HAL
	if (htft->spi_handle->hdmatx != NULL)
	{
		htft->is_dma_enabled = 1;
	}
	else
	{
		htft->is_dma_enabled = 0;
	}
#endif

	htft->is_dma_transfer_active = 0;
	TFT_Register_Device(htft);
}

static void TFT_Wait_DMA_Transfer_Complete(TFT_HandleTypeDef *htft)
{
	if (htft == NULL)
		return;

	if (htft->is_dma_enabled)
	{
		while (htft->is_dma_transfer_active)
		{
		}
	}
}

void TFT_Write_Data8(TFT_HandleTypeDef *htft, uint8_t data)
{
	if (htft == NULL || htft->spi_handle == NULL)
		return;

	TFT_Wait_DMA_Transfer_Complete(htft);
	TFT_Pin_DC_Set(htft, 1);
	TFT_Pin_CS_Set(htft, 0);
	TFT_Platform_SPI_Transmit_Blocking(htft->spi_handle, &data, 1, HAL_MAX_DELAY);
	TFT_Pin_CS_Set(htft, 1);
}

void TFT_Write_Data16(TFT_HandleTypeDef *htft, uint16_t data)
{
	if (htft == NULL || htft->spi_handle == NULL)
		return;

	uint8_t spi_data[2];
	spi_data[0] = (data >> 8) & 0xFF;
	spi_data[1] = data & 0xFF;

	TFT_Wait_DMA_Transfer_Complete(htft);
	TFT_Pin_DC_Set(htft, 1);
	TFT_Pin_CS_Set(htft, 0);
	TFT_Platform_SPI_Transmit_Blocking(htft->spi_handle, spi_data, 2, HAL_MAX_DELAY);
	TFT_Pin_CS_Set(htft, 1);
}

void TFT_Write_Command(TFT_HandleTypeDef *htft, uint8_t command)
{
	if (htft == NULL || htft->spi_handle == NULL)
		return;

	TFT_Flush_Buffer(htft, 1);

	TFT_Pin_DC_Set(htft, 0);
	TFT_Pin_CS_Set(htft, 0);
	TFT_Platform_SPI_Transmit_Blocking(htft->spi_handle, &command, 1, HAL_MAX_DELAY);
	TFT_Pin_CS_Set(htft, 1);
}

void TFT_Set_Address(TFT_HandleTypeDef *htft, uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end)
{
	if (htft == NULL)
		return;

	TFT_Flush_Buffer(htft, 1);

	TFT_Write_Command(htft, 0x2A);

	if (htft->display_direction == 0 || htft->display_direction == 2)
	{
		TFT_Write_Data16(htft, x_start + htft->x_offset);
		TFT_Write_Data16(htft, x_end + htft->x_offset);
	}
	else
	{
		TFT_Write_Data16(htft, x_start + htft->y_offset);
		TFT_Write_Data16(htft, x_end + htft->y_offset);
	}

	TFT_Write_Command(htft, 0x2B);

	if (htft->display_direction == 0 || htft->display_direction == 2)
	{
		TFT_Write_Data16(htft, y_start + htft->y_offset);
		TFT_Write_Data16(htft, y_end + htft->y_offset);
	}
	else
	{
		TFT_Write_Data16(htft, y_start + htft->x_offset);
		TFT_Write_Data16(htft, y_end + htft->x_offset);
	}

	TFT_Write_Command(htft, 0x2C);
}

uint16_t TFT_RGB(uint8_t r, uint8_t g, uint8_t b)
{
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

#ifdef STM32HAL
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	for (int i = 0; i < MAX_TFT_DEVICES; i++)
	{
		if (g_tft_handles[i] != NULL && g_tft_handles[i]->spi_handle == hspi)
		{
			TFT_HandleTypeDef *htft = g_tft_handles[i];
			if (htft->is_dma_enabled)
			{
				TFT_Pin_CS_Set(htft, 1);
				htft->is_dma_transfer_active = 0;
			}
			break;
		}
	}
}
#endif
