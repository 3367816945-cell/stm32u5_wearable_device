/*
 * mpu9250_driver.h
 *
 * MPU9250驱动头文件 - 疲劳检测系统最小实现
 *
 * 参考：论文《基于多传感融合的人体疲劳检测系统》第3章
 * 移植来源：e:\mpu9250_stm32f103\stm32f103_i2c--multi-sensor-mpu9250\MPU9250\MPU9250.h
 */

#ifndef MPU9250_DRIVER_H
#define MPU9250_DRIVER_H

#include "main.h"  // HAL库定义

/* MPU9250 I2C地址（论文要求：AD0接地，地址0x68） */
#define MPU9250_ADDRESS          0x68    // 7-bit地址
#define MPU9250_ADDRESS_WRITE    (MPU9250_ADDRESS << 1)  // 0xD0
#define MPU9250_ADDRESS_READ     ((MPU9250_ADDRESS << 1) | 1)  // 0xD1

/* 关键寄存器地址（从现有代码移植） */
#define WHO_AM_I_REG            0x75    // WHO_AM_I寄存器地址
#define WHO_AM_I_MPU9250_VAL    0x71    // WHO_AM_I标准返回值（MPU9250）
#define WHO_AM_I_MPU9255_VAL    0x73    // WHO_AM_I返回值（MPU9255）
#define WHO_AM_I_GY91_VAL       0x70    // WHO_AM_I返回值（GY-91特殊芯片，实验发现）
#define PWR_MGMT_1              0x6B    // 电源管理1
#define PWR_MGMT_2              0x6C    // 电源管理2
#define SMPLRT_DIV               0x19    // 采样率分频器
#define CONFIG                   0x1A    // 配置寄存器
#define GYRO_CONFIG              0x1B    // 陀螺仪配置
#define ACCEL_CONFIG             0x1C    // 加速度计配置
#define ACCEL_CONFIG2            0x1D    // 加速度计配置2
#define INT_PIN_CFG              0x37    // 中断引脚配置（开启旁路模式，关键！）
#define ACCEL_XOUT_H             0x3B    // 加速度X轴高位
#define ACCEL_XOUT_L             0x3C    // 加速度X轴低位
#define ACCEL_YOUT_H             0x3D    // 加速度Y轴高位
#define ACCEL_YOUT_L             0x3E    // 加速度Y轴低位
#define ACCEL_ZOUT_H             0x3F    // 加速度Z轴高位
#define ACCEL_ZOUT_L             0x40    // 加速度Z轴低位
#define TEMP_OUT_H               0x41    // 温度高位
#define TEMP_OUT_L               0x42    // 温度低位
#define GYRO_XOUT_H              0x43    // 陀螺仪X轴高位
#define GYRO_XOUT_L              0x44    // 陀螺仪X轴低位
#define GYRO_YOUT_H              0x45    // 陀螺仪Y轴高位
#define GYRO_YOUT_L              0x46    // 陀螺仪Y轴低位
#define GYRO_ZOUT_H              0x47    // 陀螺仪Z轴高位
#define GYRO_ZOUT_L              0x48    // 陀螺仪Z轴低位

/* 量程配置（论文第3.1节要求） */
#define MPU9250_ACCEL_FS_2G      0x00    // ±2g
#define MPU9250_ACCEL_FS_4G      0x08    // ±4g
#define MPU9250_ACCEL_FS_8G      0x10    // ±8g
#define MPU9250_ACCEL_FS_16G     0x18    // ±16g（论文要求）

#define MPU9250_GYRO_FS_250DPS   0x00    // ±250°/s
#define MPU9250_GYRO_FS_500DPS   0x08    // ±500°/s
#define MPU9250_GYRO_FS_1000DPS  0x10    // ±1000°/s
#define MPU9250_GYRO_FS_2000DPS  0x18    // ±2000°/s（论文要求）

/* 加速度灵敏度（LSB/g） */
#define MPU9250_ACCEL_SENS_2G    16384.0f
#define MPU9250_ACCEL_SENS_4G    8192.0f
#define MPU9250_ACCEL_SENS_8G    4096.0f
#define MPU9250_ACCEL_SENS_16G   2048.0f

/* 陀螺仪灵敏度（LSB/°/s） */
#define MPU9250_GYRO_SENS_250    131.0f
#define MPU9250_GYRO_SENS_500    65.5f
#define MPU9250_GYRO_SENS_1000   32.8f
#define MPU9250_GYRO_SENS_2000   16.4f

/* 量程定义（参考GY91项目）*/
#define MPU9250_ACCEL_2G    0x00
#define MPU9250_ACCEL_4G    0x01
#define MPU9250_ACCEL_8G    0x02
#define MPU9250_ACCEL_16G   0x03  // 论文要求

#define MPU9250_GYRO_250    0x00
#define MPU9250_GYRO_500    0x01
#define MPU9250_GYRO_1000   0x02
#define MPU9250_GYRO_2000   0x03  // 论文要求

/* AK8963 磁力计地址和寄存器（MPU9250内部） */
#define AK8963_ADDRESS         0x0C  // 7-bit address
#define AK8963_ADDRESS_WRITE   0x18  // 8-bit write (0x0C << 1)
#define AK8963_ADDRESS_READ    0x19  // 8-bit read

#define AK8963_WHO_AM_I        0x00  // 应返回 0x48
#define AK8963_INFO            0x01
#define AK8963_ST1             0x02  // 数据就绪状态
#define AK8963_XOUT_L          0x03
#define AK8963_XOUT_H          0x04
#define AK8963_YOUT_L          0x05
#define AK8963_YOUT_H          0x06
#define AK8963_ZOUT_L          0x07
#define AK8963_ZOUT_H          0x08
#define AK8963_ST2             0x09  // 数据溢出和读取错误状态
#define AK8963_CNTL            0x0A  // 控制寄存器
#define AK8963_ASAX            0x10  // 工厂校准值 X
#define AK8963_ASAY            0x11  // 工厂校准值 Y
#define AK8963_ASAZ            0x12  // 工厂校准值 Z

/* BMP280 气压计地址和寄存器（GY-91独立芯片） */
#define BMP280_ADDRESS         0x76  // 默认地址（SDO=GND）
#define BMP280_ADDRESS_WRITE   0xEC  // 8-bit write
#define BMP280_ADDRESS_READ    0xED  // 8-bit read

#define BMP280_CHIP_ID         0xD0  // 应返回 0x58
#define BMP280_RESET           0xE0
#define BMP280_STATUS          0xF3
#define BMP280_CTRL_MEAS       0xF4
#define BMP280_CONFIG          0xF5
#define BMP280_PRESSURE_MSB    0xF7
#define BMP280_PRESSURE_LSB    0xF8
#define BMP280_PRESSURE_XLSB   0xF9
#define BMP280_TEMP_MSB        0xFA
#define BMP280_TEMP_LSB        0xFB
#define BMP280_TEMP_XLSB       0xFC

/* 函数声明 */
uint8_t MPU9250_Init(void);              // 初始化MPU9250，返回0成功
uint8_t MPU9250_CheckConnection(void);   // WHO_AM_I检测，返回0成功
void MPU9250_GetAccel(float *accel);     // 读取加速度（单位：g）
void MPU9250_GetGyro(float *gyro);       // 读取陀螺仪（单位：°/s）
void MPU9250_GetTemp(float *temp);       // 读取温度（单位：°C）

/* 动态配置函数（参考GY91项目）*/
void MPU9250_SetAccelRange(uint8_t range);  // 设置加速度量程
void MPU9250_SetGyroRange(uint8_t range);   // 设置陀螺仪量程

/* AK8963 磁力计函数 */
uint8_t MPU9250_InitAK8963(float *mag_calib);  // 初始化磁力计，返回0成功
void MPU9250_GetMag(float *mag, float *mag_calib);  // 读取磁力计（单位：μT）

/* BMP280 气压计函数 */
uint8_t MPU9250_InitBMP280(void);  // 初始化气压计，返回0成功
void MPU9250_GetBMP280(float *pressure, float *temp);  // 读取气压（Pa）和温度（°C）

#endif /* MPU9250_DRIVER_H */