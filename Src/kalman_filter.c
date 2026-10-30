/*
 * kalman_filter.c
 * 卡尔曼滤波器实现 - 从参考Kalman.c移植
 */

#include "kalman_filter.h"

void kalman_init(kalman *filter, float R_angle, float Q_gyro, float Q_angle)
{
    filter->x_bias = 0.0f;
    filter->x_rate = 0.0f;
    filter->x_angle = 0.0f;
    filter->R_angle = R_angle;
    filter->Q_gyro = Q_gyro;
    filter->Q_angle = Q_angle;
    filter->P_00 = 1.0f;
    filter->P_01 = 0.0f;
    filter->P_10 = 0.0f;
    filter->P_11 = 1.0f;
}

void kalman_predict(kalman *filter, float dot_angle, float dt)
{
    float gyro_rate_unbiased = dot_angle - filter->x_bias;
    filter->x_rate = gyro_rate_unbiased;
    filter->x_angle += (dot_angle - filter->x_bias) * dt;

    float Pdot_00 = filter->Q_angle - filter->P_01 - filter->P_10;
    float Pdot_01 = -filter->P_11;
    float Pdot_10 = -filter->P_11;
    float Pdot_11 = filter->Q_gyro;

    filter->P_00 += Pdot_00 * dt;
    filter->P_01 += Pdot_01 * dt;
    filter->P_10 += Pdot_10 * dt;
    filter->P_11 += Pdot_11 * dt;
}

void kalman_update(kalman *filter, float angle_measured)
{
    float y = angle_measured - filter->x_angle;
    float S = filter->R_angle + filter->P_00;
    float K_0 = filter->P_00 / S;
    float K_1 = filter->P_10 / S;

    filter->P_00 -= K_0 * filter->P_00;
    filter->P_01 -= K_0 * filter->P_01;
    filter->P_10 -= K_1 * filter->P_00;
    filter->P_11 -= K_1 * filter->P_01;

    filter->x_angle += K_0 * y;
    filter->x_bias += K_1 * y;
}

float kalman_get_angle(kalman *filter)
{
    return filter->x_angle;
}

float kalman_get_rate(kalman *filter)
{
    return filter->x_rate;
}

void kalman_set_angle(kalman *filter, float angle)
{
    filter->x_angle = angle;
}