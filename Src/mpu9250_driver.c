/*
 * mpu9250_driver.c
 *
 * MPU9250驱动实现 - 疲劳检测系统最小实现
 *
 * 参考：论文《基于多传感融合的人体疲劳检测系统》第3章
 * 移植来源：e:\mpu9250_stm32f103\stm32f103_i2c--multi-sensor-mpu9250\MPU9250\MPU9250.cpp
 *
 * HAL库I2C函数对照：
 * - StdPeriph: I2C_Send7bitAddress() -> HAL: HAL_I2C_Mem_Write()
 * - StdPeriph: Read_data_buffer() -> HAL: HAL_I2C_Mem_Read()
 */

#include "mpu9250_driver.h"
#include "sensor_fusion.h"
#include <stdio.h>
#include <string.h>

/* 外部变量（STM32CubeMX生成的I2C句柄） */
extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart1;  // 用于调试输出

/* 陀螺仪零偏值（硬编码，来自静止实验测量） */
static float gyro_bias[3] = {2.14f, 2.50f, 1.25f};  // gx, gy, gz (°/s)

/* 辅助函数：输出调试信息到串口 */
static void Debug_Print(const char *msg) {
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);
}

/**
 * @brief MPU9250初始化（简化流程，参考GY91项目）
 * @return 0成功，1失败
 *
 * 初始化流程（参考GY91第19-29行）：
 * 1. 复位MPU9250
 * 2. 激活所有传感器
 * 3. 开启旁路模式（关键！）
 * 4. WHO_AM_I检测（兼容MPU9250和MPU9255）
 */
uint8_t MPU9250_Init(void)
{
    uint8_t data;
    HAL_StatusTypeDef status;

    // 复位MPU9250
    data = 0x80;
    status = HAL_I2C_Mem_Write(&hi2c1, MPU9250_ADDRESS_WRITE, PWR_MGMT_1, 1, &data, 1, 1000);
    if (status != HAL_OK) {
        Debug_Print("MPU9250: Reset failed\r\n");
        return 1;
    }
    HAL_Delay(100);

    // 激活所有传感器
    data = 0x00;
    status = HAL_I2C_Mem_Write(&hi2c1, MPU9250_ADDRESS_WRITE, PWR_MGMT_2, 1, &data, 1, 1000);
    if (status != HAL_OK) {
        Debug_Print("MPU9250: Activate failed\r\n");
        return 1;
    }

    // 开启旁路模式
    data = 0x02;
    status = HAL_I2C_Mem_Write(&hi2c1, MPU9250_ADDRESS_WRITE, INT_PIN_CFG, 1, &data, 1, 1000);
    if (status != HAL_OK) {
        Debug_Print("MPU9250: Bypass failed\r\n");
        return 1;
    }

    return MPU9250_CheckConnection();
}

/**
 * @brief MPU9250连接检测（WHO_AM_I，兼容MPU9250和MPU9255）
 * @return 0成功，1失败
 *
 * 读取WHO_AM_I寄存器（0x75）：
 * - MPU9250返回0x71
 * - MPU9255返回0x73（更常见）
 */
uint8_t MPU9250_CheckConnection(void)
{
    uint8_t who_am_i;
    HAL_StatusTypeDef status;

    status = HAL_I2C_Mem_Read(&hi2c1, MPU9250_ADDRESS_WRITE, WHO_AM_I_REG, 1, &who_am_i, 1, 1000);
    if (status != HAL_OK) {
        Debug_Print("WHO_AM_I read failed\r\n");
        return 1;
    }

    if (who_am_i == WHO_AM_I_MPU9250_VAL || who_am_i == WHO_AM_I_MPU9255_VAL || who_am_i == WHO_AM_I_GY91_VAL) {
        return 0;  // 成功
    } else {
        Debug_Print("Unknown device\r\n");
        return 1;
    }
}

/**

 * @brief 读取加速度数据

 * @param accel 加速度数组[3]，单位：g
 *
 * 读取加速度X/Y/Z三轴数据（6字节），转换为物理单位（g）
 * 使用±16g量程（论文要求）
 */
void MPU9250_GetAccel(float *accel)
{
    uint8_t data[6];
    int16_t raw_accel[3];
    static float lpf_accel[3] = {0, 0, 0};  // 低通滤波状态
    HAL_StatusTypeDef status;

    /* 读取加速度原始数据（6字节，从ACCEL_XOUT_H开始） */
    status = HAL_I2C_Mem_Read(&hi2c1, MPU9250_ADDRESS_WRITE, ACCEL_XOUT_H, 1, data, 6, 1000);
    if (status != HAL_OK) {
        return;
    }

    raw_accel[0] = (int16_t)((data[0] << 8) | data[1]);
    raw_accel[1] = (int16_t)((data[2] << 8) | data[3]);
    raw_accel[2] = (int16_t)((data[4] << 8) | data[5]);

    /* 转换为物理单位，然后低通滤波 */
    float raw[3];
    raw[0] = (float)raw_accel[0] / MPU9250_ACCEL_SENS_16G;
    raw[1] = (float)raw_accel[1] / MPU9250_ACCEL_SENS_16G;
    raw[2] = (float)raw_accel[2] / MPU9250_ACCEL_SENS_16G;

    MPU9250_LowPassFilter3D(lpf_accel, raw);
    accel[0] = lpf_accel[0];
    accel[1] = lpf_accel[1];
    accel[2] = lpf_accel[2];
}

/**
 * @brief 读取陀螺仪数据
 * @param gyro 陀螺仪数组[3]，单位：°/s
 *
 * 读取陀螺仪X/Y/Z三轴数据（6字节），转换为物理单位（°/s）
 * 使用±2000°/s量程（论文要求）
 */
void MPU9250_GetGyro(float *gyro)
{
    uint8_t data[6];
    int16_t raw_gyro[3];
    static float lpf_gyro[3] = {0, 0, 0};  // 低通滤波状态
    HAL_StatusTypeDef status;

    /* 读取陀螺仪原始数据（6字节，从GYRO_XOUT_H开始） */
    status = HAL_I2C_Mem_Read(&hi2c1, MPU9250_ADDRESS_WRITE, GYRO_XOUT_H, 1, data, 6, 1000);
    if (status != HAL_OK) {
        return;
    }

    raw_gyro[0] = (int16_t)((data[0] << 8) | data[1]);
    raw_gyro[1] = (int16_t)((data[2] << 8) | data[3]);
    raw_gyro[2] = (int16_t)((data[4] << 8) | data[5]);

    /* 转换为物理单位，然后低通滤波 */
    float raw[3];
    raw[0] = (float)raw_gyro[0] / MPU9250_GYRO_SENS_2000;
    raw[1] = (float)raw_gyro[1] / MPU9250_GYRO_SENS_2000;
    raw[2] = (float)raw_gyro[2] / MPU9250_GYRO_SENS_2000;

    MPU9250_LowPassFilter3D(lpf_gyro, raw);
    gyro[0] = lpf_gyro[0] - gyro_bias[0];
    gyro[1] = lpf_gyro[1] - gyro_bias[1];
    gyro[2] = lpf_gyro[2] - gyro_bias[2];
}

/**
 * @brief 读取温度数据
 * @param temp 温度，单位：°C
 *
 * 读取温度原始数据，转换为物理单位（°C）
 */
void MPU9250_GetTemp(float *temp)
{
    uint8_t data[2];
    int16_t raw_temp;
    HAL_StatusTypeDef status;

    /* 读取温度原始数据（2字节） */
    status = HAL_I2C_Mem_Read(&hi2c1, MPU9250_ADDRESS_WRITE, TEMP_OUT_H, 1, data, 2, 1000);
    if (status != HAL_OK) {
        *temp = 0.0f;
        return;
    }

    /* 转换为16位有符号整数 */
    raw_temp = (int16_t)((data[0] << 8) | data[1]);

    /* 转换为物理单位（°C） */
    *temp = ((float)raw_temp / 340.0f) + 36.53f;
}

/**
 * @brief 设置加速度量程（动态配置，参考GY91第96-99行）
 * @param range 量程值（MPU9250_ACCEL_2G/4G/8G/16G）
 */
void MPU9250_SetAccelRange(uint8_t range)
{
    uint8_t reg;
    HAL_StatusTypeDef status;

    /* 读取当前配置 */
    status = HAL_I2C_Mem_Read(&hi2c1, MPU9250_ADDRESS_WRITE, ACCEL_CONFIG, 1, &reg, 1, 1000);
    if (status != HAL_OK) {
        return;
    }

    /* 修改量程位（bit3-4），保留其他位 */
    reg = (reg & 0xE7) | (range << 3);

    /* 写入新配置 */
    HAL_I2C_Mem_Write(&hi2c1, MPU9250_ADDRESS_WRITE, ACCEL_CONFIG, 1, &reg, 1, 1000);
}

/**
 * @brief 设置陀螺仪量程（动态配置，参考GY91第139-142行）
 * @param range 量程值（MPU9250_GYRO_250/500/1000/2000）
 */
void MPU9250_SetGyroRange(uint8_t range)
{
    uint8_t reg;
    HAL_StatusTypeDef status;

    /* 读取当前配置 */
    status = HAL_I2C_Mem_Read(&hi2c1, MPU9250_ADDRESS_WRITE, GYRO_CONFIG, 1, &reg, 1, 1000);
    if (status != HAL_OK) {
        return;
    }

    /* 修改量程位（bit3-4），保留其他位 */
    reg = (reg & 0xE7) | (range << 3);

    /* 写入新配置 */
    HAL_I2C_Mem_Write(&hi2c1, MPU9250_ADDRESS_WRITE, GYRO_CONFIG, 1, &reg, 1, 1000);
}

/**
 * @brief BMP280 气压计初始化
 * @return 0成功，1失败
 */
uint8_t MPU9250_InitBMP280(void)
{
    uint8_t data, status;
    
    // 1. 检测 CHIP_ID（应返回 0x58）
    status = HAL_I2C_Mem_Read(&hi2c1, BMP280_ADDRESS_WRITE, BMP280_CHIP_ID, 1, &data, 1, 1000);
    if (status != HAL_OK || data != 0x58) {
        Debug_Print("BMP280 CHIP_ID Failed\r\n");
        return 1;
    }
    
    // 2. 软复位
    HAL_I2C_Mem_Write(&hi2c1, BMP280_ADDRESS_WRITE, BMP280_RESET, 1, (uint8_t[]){0xB6}, 1, 1000);
    HAL_Delay(100);
    
    // 3. 配置采样率：正常模式，压力×16，温度×2
    // 0xB7 = 正常模式(mode=11) + 温度×16(osrs_t=101) + 气压×16(osrs_p=101)
    HAL_I2C_Mem_Write(&hi2c1, BMP280_ADDRESS_WRITE, BMP280_CTRL_MEAS, 1, (uint8_t[]){0xB7}, 1, 1000);
    
    // 4. 配置滤波：IIR×4
    HAL_I2C_Mem_Write(&hi2c1, BMP280_ADDRESS_WRITE, BMP280_CONFIG, 1, (uint8_t[]){0x24}, 1, 1000);
    
    Debug_Print("BMP280 Init OK\r\n");
    return 0;
}

/**
 * @brief 读取 BMP280 气压和温度（简化版，不包含校准补偿）
 * @param pressure 气压输出[Pa]（原始值，需校准补偿）
 * @param temp 温度输出[°C]（原始值）
 */
void MPU9250_GetBMP280(float *pressure, float *temp)
{
    uint8_t data[6];
    int32_t raw_press, raw_temp;
    HAL_StatusTypeDef status;
    // char dbg[100];  /* 调试用,已注释 */
    uint8_t meas_status;
    uint8_t timeout = 50;  // 最大等待50次
    
    // 等待测量完成（measuring位变为0）
    do {
        status = HAL_I2C_Mem_Read(&hi2c1, BMP280_ADDRESS_WRITE, 0xF3, 1, &meas_status, 1, 100);
        if ((meas_status & 0x08) == 0) {  // measuring=0，测量完成
            break;
        }
        HAL_Delay(1);  // 等待1ms
        timeout--;
    } while (timeout > 0);
    
    if (timeout == 0) {
        Debug_Print("BMP280: Measurement timeout\r\n");
        *pressure = 0.0f;
        *temp = 0.0f;
        return;
    }
    
    // 读取气压和温度数据（地址0xEC）
    status = HAL_I2C_Mem_Read(&hi2c1, BMP280_ADDRESS_WRITE, BMP280_PRESSURE_MSB, 1, data, 6, 100);
    
    if (status == HAL_OK) {
        raw_press = ((int32_t)data[0] << 16) | ((int32_t)data[1] << 8) | data[2];
        raw_press = raw_press >> 4;
        raw_temp = ((int32_t)data[3] << 16) | ((int32_t)data[4] << 8) | data[5];
        raw_temp = raw_temp >> 4;
        
        *pressure = (float)raw_press;
        *temp = (float)raw_temp / 512.0f;
    } else {
        Debug_Print("BMP280: Read failed\r\n");
        *pressure = 0.0f;
        *temp = 0.0f;
    }
}

