/**
 * @file sensor_data.h
 * @brief 传感器数据结构定义
 * @note 此文件需与 Src/gui/include/sensor_data.h 保持同步
 */
#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <stdint.h>

typedef struct {
    uint8_t fatigue_level;      // 0=Low, 1=Med, 2=High
    uint8_t confidence;         // 0-100
    uint16_t bpm;               // Heart rate
    uint16_t signal;            // 原始心跳信号值（ADC 10bit, 0-1023）
    uint16_t stride_time;       // Stride time in ms
    uint16_t cadence;           // Steps per minute
    float peak_acc_z;           // Z-axis peak acceleration
    float rms_acc_mag;          // RMS acceleration magnitude
    volatile uint8_t dataReady; // New data flag
} SensorData_t;

#ifdef __cplusplus
extern "C" {
#endif

extern SensorData_t sensorData;

#ifdef __cplusplus
}
#endif

#endif /* SENSOR_DATA_H */