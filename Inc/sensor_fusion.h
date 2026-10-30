/*
 * sensor_fusion.h
 * 传感器融合 - 低通滤波 + 垂直加速度提取 + 3轴Kalman姿态 + 踝外翻角（论文第3.5节）
 */

#ifndef SENSOR_FUSION_H
#define SENSOR_FUSION_H

#include "kalman_filter.h"

#define LPF_ALPHA 0.5f  /* 一阶低通滤波系数 */

/* 3轴Kalman姿态结构体 */
typedef struct {
    kalman roll;
    kalman pitch;
    kalman yaw;
} AHRS_t;

void MPU9250_LowPassFilter3D(float *filtered, float *raw);
void MPU9250_GetVerticalAccel(float *accel, float pitch, float roll, float *vertical);

/* 3轴Kalman姿态解算 */
void AHRS_Init(AHRS_t *ahrs);
void AHRS_Update(AHRS_t *ahrs, float *accel, float *gyro, float dt);
float AHRS_GetRoll(AHRS_t *ahrs);
float AHRS_GetPitch(AHRS_t *ahrs);
float AHRS_GetYaw(AHRS_t *ahrs);

/* 踝外翻角计算（绕前进轴旋转，基于roll角） */
float AHRS_GetEversion(AHRS_t *ahrs);

#endif /* SENSOR_FUSION_H */