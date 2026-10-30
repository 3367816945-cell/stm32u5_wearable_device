/*
 * sensor_fusion.c
 * 传感器融合实现 - 低通滤波 + 垂直加速度提取 + 3轴Kalman姿态 + 踝外翻角
 */

#include "sensor_fusion.h"
#include <math.h>

void MPU9250_LowPassFilter3D(float *filtered, float *raw)
{
    filtered[0] = raw[0] * LPF_ALPHA + filtered[0] * (1.0f - LPF_ALPHA);
    filtered[1] = raw[1] * LPF_ALPHA + filtered[1] * (1.0f - LPF_ALPHA);
    filtered[2] = raw[2] * LPF_ALPHA + filtered[2] * (1.0f - LPF_ALPHA);
}

void MPU9250_GetVerticalAccel(float *accel, float pitch, float roll, float *vertical)
{
    float pr = pitch * 0.0174533f;  /* 度转弧度 */
    float rr = roll * 0.0174533f;
    float sp = sinf(pr), cp = cosf(pr);
    float sr = sinf(rr), cr = cosf(rr);

    /* 旋转到世界坐标系，提取垂直分量，减去重力(1g) */
    *vertical = -accel[0] * sp + accel[1] * sr * cp + accel[2] * cr * cp - 1.0f;
}

/* ===== 3轴Kalman姿态解算 ===== */

void AHRS_Init(AHRS_t *ahrs)
{
    /* roll: 对加速度计角度信任较高，陀螺仪漂移信任较低 */
    kalman_init(&ahrs->roll, 0.01f, 0.001f, 0.003f);
    /* pitch: 同roll */
    kalman_init(&ahrs->pitch, 0.01f, 0.001f, 0.003f);
    /* yaw: 无加速度计参考，纯陀螺仪积分，对漂移信任更低 */
    kalman_init(&ahrs->yaw, 100.0f, 0.01f, 0.03f);
}

void AHRS_Update(AHRS_t *ahrs, float *accel, float *gyro, float dt)
{
    /* 从加速度计计算roll和pitch初值 */
    float accel_roll = atan2f(accel[1], accel[2]) * 57.29578f;   /* 弧度转度 */
    float accel_pitch = atan2f(-accel[0], 
        sqrtf(accel[1]*accel[1] + accel[2]*accel[2])) * 57.29578f;

    /* Roll轴Kalman */
    kalman_predict(&ahrs->roll, gyro[0], dt);
    kalman_update(&ahrs->roll, accel_roll);

    /* Pitch轴Kalman */
    kalman_predict(&ahrs->pitch, gyro[1], dt);
    kalman_update(&ahrs->pitch, accel_pitch);

    /* Yaw轴Kalman（纯陀螺仪，无加速度计参考） */
    kalman_predict(&ahrs->yaw, gyro[2], dt);
}

float AHRS_GetRoll(AHRS_t *ahrs)
{
    return kalman_get_angle(&ahrs->roll);
}

float AHRS_GetPitch(AHRS_t *ahrs)
{
    return kalman_get_angle(&ahrs->pitch);
}

float AHRS_GetYaw(AHRS_t *ahrs)
{
    return kalman_get_angle(&ahrs->yaw);
}

/* 踝外翻角：绕前进轴的旋转角度 = roll角 */
float AHRS_GetEversion(AHRS_t *ahrs)
{
    return AHRS_GetRoll(ahrs);
}