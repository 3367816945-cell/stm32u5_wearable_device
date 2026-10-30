/*
 * kalman_filter.h
 * 卡尔曼滤波器 - 姿态解算（从参考Kalman.c移植）
 */

#ifndef KALMAN_FILTER_H
#define KALMAN_FILTER_H

/* 卡尔曼滤波结构体 */
typedef struct {
    float x_bias;
    float x_rate;
    float x_angle;
    float P_00;
    float P_01;
    float P_10;
    float P_11;
    float R_angle;
    float Q_gyro;
    float Q_angle;
} kalman;

void kalman_init(kalman *filter, float R_angle, float Q_gyro, float Q_angle);
void kalman_predict(kalman *filter, float dot_angle, float dt);
void kalman_update(kalman *filter, float angle_measured);
float kalman_get_angle(kalman *filter);
float kalman_get_rate(kalman *filter);
void kalman_set_angle(kalman *filter, float angle);

#endif /* KALMAN_FILTER_H */