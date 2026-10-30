/*
 * mpu9250_debug.h
 * MPU9250调试/诊断工具（I2C扫描、寄存器dump、总线诊断）
 */

#ifndef MPU9250_DEBUG_H
#define MPU9250_DEBUG_H

#include "main.h"

void I2C_ScanBus(void);
void MPU9250_Debug_Init(void);
void I2C_Bus_Diagnosis(void);
void MPU9250_Register_Dump(void);
const char* HAL_Status_To_String(HAL_StatusTypeDef status);
void Print_I2C_Error_Analysis(HAL_StatusTypeDef status);

#endif /* MPU9250_DEBUG_H */