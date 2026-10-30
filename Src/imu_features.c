#include "imu_features.h"
#include "main.h"
#include "gait_detect.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

void IMUFeatures_Init(IMUFeatureBuffer_t *buf)
{
    memset(buf, 0, sizeof(IMUFeatureBuffer_t));
    buf->peak_acc_z_val = -100.0f;
    buf->peak_acc_mag_val = 0.0f;
    buf->window_start_tick = HAL_GetTick();  /* 初始化窗口起始时间 */
}

void IMUFeatures_Feed(IMUFeatureBuffer_t *buf, float *accel, float *gyro)
{
    uint16_t idx = buf->buf_idx;
    
    buf->ax_buf[idx] = accel[0];
    buf->ay_buf[idx] = accel[1];
    buf->az_buf[idx] = accel[2];
    buf->gx_buf[idx] = gyro[0];
    buf->gy_buf[idx] = gyro[1];
    buf->gz_buf[idx] = gyro[2];
    
    float mag = sqrtf(accel[0]*accel[0] + accel[1]*accel[1] + accel[2]*accel[2]);
    
    if (accel[2] > buf->peak_acc_z_val) {
        buf->peak_acc_z_val = accel[2];
    }
    if (mag > buf->peak_acc_mag_val) {
        buf->peak_acc_mag_val = mag;
    }
    
    buf->buf_idx = (buf->buf_idx + 1) % IMU_WINDOW_SIZE;
    if (buf->sample_count < IMU_WINDOW_SIZE) {
        buf->sample_count++;
    }
}

void IMUFeatures_FeedGait(IMUFeatureBuffer_t *buf, float stride_time, float cv_stride, float cadence, float eversion)
{
    uint16_t current_steps = MPU9250_GetStrideCount();
    
    if (current_steps > buf->prev_step_count) {
        buf->step_count_in_window += (current_steps - buf->prev_step_count);
        buf->prev_step_count = current_steps;
    }
    
    buf->features.stride_time = stride_time;
    buf->features.cv_stride_time = cv_stride;
    buf->features.cadence = cadence;

    /* 记录eversion到滑窗 */
    buf->eversion_buf[buf->eversion_idx++] = eversion;
    if (buf->eversion_idx >= IMU_EVERSION_BUF_SIZE) buf->eversion_idx = 0;
    if (buf->eversion_count < IMU_EVERSION_BUF_SIZE) buf->eversion_count++;

    /* 计算cv_eversion_range */
    if (buf->eversion_count >= 2) {
        float sum = 0.0f, sum_sq = 0.0f;
        uint16_t n = (buf->eversion_count < IMU_EVERSION_BUF_SIZE) ? buf->eversion_count : IMU_EVERSION_BUF_SIZE;
        uint16_t i;
        for (i = 0; i < n; i++) {
            sum += buf->eversion_buf[i];
        }
        float mean = sum / (float)n;
        for (i = 0; i < n; i++) {
            float diff = buf->eversion_buf[i] - mean;
            sum_sq += diff * diff;
        }
        float std = sqrtf(sum_sq / (float)(n - 1));
        buf->features.cv_eversion_range = (mean > 0.0001f) ? (std / mean) : 0.0f;
    }
    
    /* hybrid窗口策略：20步 OR 10秒超时，先到为准 */
    uint32_t now = HAL_GetTick();
    uint8_t window_expired = (buf->step_count_in_window >= GAIT_STRIDE_BUF_SIZE);
    uint8_t timeout_expired = (buf->window_start_tick > 0 && (now - buf->window_start_tick) >= 10000);
    
    if (window_expired || timeout_expired) {
        buf->features.peak_acc_z = buf->peak_acc_z_val;
        buf->features.peak_acc_mag = buf->peak_acc_mag_val;
        
        buf->peak_acc_z_val = -100.0f;
        buf->peak_acc_mag_val = 0.0f;
        
        buf->features.timestamp = now;
        
        if (buf->baseline_calibrated) {
            if (buf->features.baseline_std[0] > 0.0001f)
                buf->features.peak_acc_z = (buf->features.peak_acc_z - buf->features.baseline_mean[0]) / buf->features.baseline_std[0];
            if (buf->features.baseline_std[1] > 0.0001f)
                buf->features.peak_acc_mag = (buf->features.peak_acc_mag - buf->features.baseline_mean[1]) / buf->features.baseline_std[1];
            if (buf->features.baseline_std[2] > 0.0001f)
                buf->features.cv_stride_time = (buf->features.cv_stride_time - buf->features.baseline_mean[2]) / buf->features.baseline_std[2];
            if (buf->features.baseline_std[3] > 0.0001f)
                buf->features.cv_eversion_range = (buf->features.cv_eversion_range - buf->features.baseline_mean[3]) / buf->features.baseline_std[3];
            if (buf->features.baseline_std[4] > 0.0001f)
                buf->features.stride_time = (buf->features.stride_time - buf->features.baseline_mean[4]) / buf->features.baseline_std[4];
            if (buf->features.baseline_std[5] > 0.0001f)
                buf->features.stance_time = (buf->features.stance_time - buf->features.baseline_mean[5]) / buf->features.baseline_std[5];
            if (buf->features.baseline_std[6] > 0.0001f)
                buf->features.cadence = (buf->features.cadence - buf->features.baseline_mean[6]) / buf->features.baseline_std[6];
        }
        
        buf->features_ready = 1;
        
        buf->step_count_in_window = 0;
        buf->window_start_tick = now;
        
#if IMU_FEATURE_DEBUG
        /* STM32 nano.specs不支持%f，改用整数输出 */
        int16_t paz = (int16_t)(buf->features.peak_acc_z * 1000);
        int16_t pam = (int16_t)(buf->features.peak_acc_mag * 1000);
        int16_t cvst = (int16_t)(buf->features.cv_stride_time * 10000);
        int16_t cver = (int16_t)(buf->features.cv_eversion_range * 10000);
        int16_t st = (int16_t)(buf->features.stride_time * 10);
        int16_t stt = (int16_t)(buf->features.stance_time * 10);
        int16_t cad = (int16_t)(buf->features.cadence * 10);
        printf("F:I=%d.%03d,%d.%03d;S=%d.%04d,%d.%04d;G=%d.%01d,%d.%01d,%d.%01d\r\n",
               paz/1000, abs(paz%1000),
               pam/1000, abs(pam%1000),
               cvst/10000, abs(cvst%10000),
               cver/10000, abs(cver%10000),
               st/10, abs(st%10),
               stt/10, abs(stt%10),
               cad/10, abs(cad%10));
#endif
    }
}

IMU_DomainScore_t IMUFeatures_GetDomainScores(IMUFeatureBuffer_t *buf)
{
    IMU_DomainScore_t scores = {0, 0, 0, 0};
    
    scores.impact_score = buf->features.peak_acc_z + buf->features.peak_acc_mag;
    scores.stability_score = buf->features.cv_stride_time + buf->features.cv_eversion_range;
    scores.gait_score = buf->features.stride_time + buf->features.stance_time + buf->features.cadence;
    
    /* 默认权重：Stability最高，Impact和Gait各占0.25 */
    scores.fatigue_score = 0.25f * scores.impact_score
                         + 0.50f * scores.stability_score
                         + 0.25f * scores.gait_score;
    
    return scores;
}

void IMUFeatures_CalibrateBaseline(IMUFeatureBuffer_t *buf)
{
    buf->baseline_calibrated = 1;
}

IMUFeatures_t* IMUFeatures_GetVector(IMUFeatureBuffer_t *buf)
{
    if (buf->features_ready) {
        buf->features_ready = 0;
        return &buf->features;
    }
    return NULL;
}