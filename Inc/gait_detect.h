/*
 * gait_detect.h
 * 步态周期分割 - 阈值法（论文第4.3-4.4节）
 * 增强版：支持步长时间记录和变异系数计算
 */

#ifndef GAIT_DETECT_H
#define GAIT_DETECT_H

#include <stdint.h>

#define GAIT_STRIDE_BUF_SIZE 20

/* 步态检测状态机 */
typedef enum {
    GAIT_IDLE,       /* 静止 */
    GAIT_RISING,     /* 加速度上升 */
    GAIT_PEAK,       /* 峰值附近 */
    GAIT_FALLING     /* 加速度下降 */
} GaitState;

void MPU9250_GaitDetect(float vertical_accel, uint32_t timestamp);
uint16_t MPU9250_GetStepCount(void);
float MPU9250_GetLastPeak(void);

/* 增强功能：步长时间记录 */
uint32_t MPU9250_GetLastStepTime(void);
float MPU9250_GetStrideTime(void);
float MPU9250_GetCVStrideTime(void);
float MPU9250_GetCadence(void);
uint16_t MPU9250_GetStrideCount(void);
float MPU9250_GetAdaptiveThreshold(void);

#endif /* GAIT_DETECT_H */