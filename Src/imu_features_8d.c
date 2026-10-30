/**
  ******************************************************************************
  * @file    imu_features_8d.c
  * @brief   8维IMU特征提取实现（PAMAP2 Network A）
  ******************************************************************************
  */

#include "imu_features_8d.h"
#include "pamap2_norm.h"
#include <math.h>
#include <string.h>

/* ========== 初始化 ========== */
void IMU8D_Init(IMU8DBuffer_t *buf)
{
    if (!buf) return;

    memset(buf, 0, sizeof(IMU8DBuffer_t));
    buf->buf_idx = 0;
    buf->sample_count = 0;
    buf->peak_count = 0;
    buf->features.features_ready = 0;
}

/* ========== 填充样本 ========== */
void IMU8D_Feed(IMU8DBuffer_t *buf, float *accel, float *gyro)
{
    if (!buf || !accel || !gyro) return;

    // 填充环形缓冲区（500样本）
    buf->ax_buf[buf->buf_idx] = accel[0];
    buf->ay_buf[buf->buf_idx] = accel[1];
    buf->az_buf[buf->buf_idx] = accel[2];
    buf->gx_buf[buf->buf_idx] = gyro[0];
    buf->gy_buf[buf->buf_idx] = gyro[1];
    buf->gz_buf[buf->buf_idx] = gyro[2];

    buf->buf_idx++;
    if (buf->buf_idx >= IMU8D_WINDOW_SIZE) {
        buf->buf_idx = 0;  // 环形缓冲区循环
    }

    buf->sample_count++;
    if (buf->sample_count > IMU8D_WINDOW_SIZE) {
        buf->sample_count = IMU8D_WINDOW_SIZE;
    }
}

/* ========== 步态检测（Z轴峰值法） ========== */
static void detect_gait_peaks(IMU8DBuffer_t *buf)
{
    // 1. 低通滤波az（5Hz cutoff，简化版均值滤波）
    for (uint16_t i = 0; i < buf->sample_count; i++) {
        float sum = 0.0f;
        uint8_t win = 5;  // 5样本窗口（简化滤波）
        uint16_t start = (i >= win/2) ? (i - win/2) : 0;
        uint16_t end = (i + win/2 < buf->sample_count) ? (i + win/2) : (buf->sample_count - 1);
        for (uint16_t j = start; j <= end; j++) {
            sum += buf->az_buf[j];
        }
        buf->az_filt[i] = sum / (float)(end - start + 1);
    }

    // 2. 计算阈值：mean + 0.5 * std
    float mean_az = 0.0f, var_az = 0.0f;
    for (uint16_t i = 0; i < buf->sample_count; i++) {
        mean_az += buf->az_filt[i];
    }
    mean_az /= (float)buf->sample_count;

    for (uint16_t i = 0; i < buf->sample_count; i++) {
        float diff = buf->az_filt[i] - mean_az;
        var_az += diff * diff;
    }
    var_az /= (float)buf->sample_count;
    float std_az = sqrtf(var_az);
    float threshold = mean_az + 0.5f * std_az;

    // 3. 峰值检测（相邻峰值间距 >= 30样本，即0.3s @ 100Hz）
    buf->peak_count = 0;
    for (uint16_t i = 1; i < buf->sample_count - 1; i++) {
        if (buf->az_filt[i] > buf->az_filt[i-1] &&
            buf->az_filt[i] > buf->az_filt[i+1] &&
            buf->az_filt[i] > threshold) {
            
            // 检查相邻峰值间距
            if (buf->peak_count == 0 ||
                (i - buf->peak_times[buf->peak_count - 1]) >= 30) {
                buf->peak_times[buf->peak_count++] = i;
                if (buf->peak_count >= IMU8D_MAX_PEAKS) break;
            }
        }
    }
}

/* ========== 特征提取 ========== */
void IMU8D_Extract(IMU8DBuffer_t *buf)
{
    if (!buf || buf->sample_count < 100) return;  // 最少100样本

    // 1. 步态检测
    detect_gait_peaks(buf);

    uint16_t n = buf->sample_count;

    // 2. 特征0: peak_acc_z = max(az)
    float max_az = buf->az_buf[0];
    for (uint16_t i = 1; i < n; i++) {
        if (buf->az_buf[i] > max_az) {
            max_az = buf->az_buf[i];
        }
    }
    buf->features.features[0] = max_az;

    // 3. 特征1: rms_acc_mag = sqrt(mean(ax²+ay²+az²))
    float sum_mag_sq = 0.0f;
    for (uint16_t i = 0; i < n; i++) {
        float mag_sq = buf->ax_buf[i]*buf->ax_buf[i] +
                       buf->ay_buf[i]*buf->ay_buf[i] +
                       buf->az_buf[i]*buf->az_buf[i];
        sum_mag_sq += mag_sq;
    }
    buf->features.features[1] = sqrtf(sum_mag_sq / (float)n);

    // 4. 计算stride_time和stride_times数组
    float stride_times[IMU8D_MAX_PEAKS];
    uint8_t stride_count = 0;
    float mean_stride_time = 0.0f;

    if (buf->peak_count >= 2) {
        for (uint8_t i = 1; i < buf->peak_count; i++) {
            stride_times[stride_count] = (float)(buf->peak_times[i] - buf->peak_times[i-1]) / 100.0f;  // 单位：秒
            mean_stride_time += stride_times[stride_count];
            stride_count++;
        }
        mean_stride_time /= (float)stride_count;
    } else {
        // 无峰值时使用默认值
        mean_stride_time = 0.8f;  // 默认800ms
        stride_count = 1;
        stride_times[0] = 0.8f;
    }

    // 5. 特征3: stride_time = mean(stride_time)
    buf->features.features[3] = mean_stride_time;

    // 6. 特征4: cadence = 60.0 / stride_time
    buf->features.features[4] = 60.0f / mean_stride_time;

    // 7. 特征2: impact_per_stride = max(acc_mag) / mean(stride_time)
    float max_acc_mag = 0.0f;
    for (uint16_t i = 0; i < n; i++) {
        float mag = sqrtf(buf->ax_buf[i]*buf->ax_buf[i] +
                         buf->ay_buf[i]*buf->ay_buf[i] +
                         buf->az_buf[i]*buf->az_buf[i]);
        if (mag > max_acc_mag) {
            max_acc_mag = mag;
        }
    }
    buf->features.features[2] = max_acc_mag / mean_stride_time;

    // 8. 特征5: cv_stride_time = std(stride_time) / mean(stride_time)
    float std_stride_time = 0.0f;
    if (stride_count >= 2) {
        for (uint8_t i = 0; i < stride_count; i++) {
            float diff = stride_times[i] - mean_stride_time;
            std_stride_time += diff * diff;
        }
        std_stride_time = sqrtf(std_stride_time / (float)stride_count);
    } else {
        std_stride_time = 0.05f;  // 默认值
    }
    buf->features.features[5] = std_stride_time / mean_stride_time;

    // 9. 特征6: cv_acc_mag = std(acc_mag) / mean(acc_mag)
    float mean_acc_mag = 0.0f, std_acc_mag = 0.0f;
    float acc_mag_arr[IMU8D_WINDOW_SIZE];
    for (uint16_t i = 0; i < n; i++) {
        acc_mag_arr[i] = sqrtf(buf->ax_buf[i]*buf->ax_buf[i] +
                               buf->ay_buf[i]*buf->ay_buf[i] +
                               buf->az_buf[i]*buf->az_buf[i]);
        mean_acc_mag += acc_mag_arr[i];
    }
    mean_acc_mag /= (float)n;

    for (uint16_t i = 0; i < n; i++) {
        float diff = acc_mag_arr[i] - mean_acc_mag;
        std_acc_mag += diff * diff;
    }
    std_acc_mag = sqrtf(std_acc_mag / (float)n);
    buf->features.features[6] = std_acc_mag / mean_acc_mag;

    // 10. 特征7: gyro_x_variance = var(gyro_x)
    float mean_gx = 0.0f, var_gx = 0.0f;
    for (uint16_t i = 0; i < n; i++) {
        mean_gx += buf->gx_buf[i];
    }
    mean_gx /= (float)n;

    for (uint16_t i = 0; i < n; i++) {
        float diff = buf->gx_buf[i] - mean_gx;
        var_gx += diff * diff;
    }
    var_gx /= (float)n;
    buf->features.features[7] = var_gx;

    buf->features.features_ready = 1;
}

/* ========== Z-score归一化 ========== */
void IMU8D_Normalize(IMU8DBuffer_t *buf)
{
    if (!buf || !buf->features.features_ready) return;

    for (uint8_t i = 0; i < NUM_FEATURES; i++) {
        buf->features.features[i] = (buf->features.features[i] - feature_mean[i]) / feature_std[i];
    }
}

/* ========== 获取特征向量指针 ========== */
IMU8DFeatures_t* IMU8D_GetFeatures(IMU8DBuffer_t *buf)
{
    if (!buf || !buf->features.features_ready) return NULL;
    return &buf->features;
}