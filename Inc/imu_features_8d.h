/**
  ******************************************************************************
  * @file    imu_features_8d.h
  * @brief   8维IMU特征提取模块（PAMAP2 Network A）
  ******************************************************************************
  * @attention
  *
  * 特征定义（embedded_ai_prompt.md 1.2节）：
  * 特征 0: peak_acc_z       = max(acc_z)                              // 峰值Z轴冲击
  * 特征 1: rms_acc_mag      = sqrt(mean(acc_x² + acc_y² + acc_z²))    // 合加速度RMS
  * 特征 2: impact_per_stride= max(acc_mag) / mean(stride_time)        // 单位步幅冲击
  * 特征 3: stride_time      = mean(stride_time)                        // 平均步态周期 (s)
  * 特征 4: cadence          = 60.0 / stride_time                      // 步频 (步/分钟)
  * 特征 5: cv_stride_time   = std(stride_time) / mean(stride_time)    // 步幅变异系数
  * 特征 6: cv_acc_mag       = std(acc_mag) / mean(acc_mag)            // 加速度变异系数
  * 特征 7: gyro_x_variance  = var(gyro_x)                              // 外翻代理
  *
  * 窗口：5秒 @ 100Hz（500样本）
  *
  ******************************************************************************
  */

#ifndef IMU_FEATURES_8D_H
#define IMU_FEATURES_8D_H

#include <stdint.h>
#include <stdbool.h>

#define IMU8D_WINDOW_SIZE      500    // 5秒 @ 100Hz
#define IMU8D_FEATURE_COUNT    8
#define IMU8D_MAX_PEAKS        50     // 最大峰值数量

typedef struct {
    float features[8];               // 8维特征向量（已归一化）
    uint8_t features_ready;          // 特征就绪标志
    uint32_t timestamp;              // 时间戳
} IMU8DFeatures_t;

typedef struct {
    // 500样本环形缓冲区（5秒 @ 100Hz）
    float ax_buf[IMU8D_WINDOW_SIZE];
    float ay_buf[IMU8D_WINDOW_SIZE];
    float az_buf[IMU8D_WINDOW_SIZE];
    float gx_buf[IMU8D_WINDOW_SIZE];
    float gy_buf[IMU8D_WINDOW_SIZE];
    float gz_buf[IMU8D_WINDOW_SIZE];
    uint16_t buf_idx;                // 当前索引
    uint16_t sample_count;           // 样本计数

    // 步态峰值检测（Z轴峰值法）
    float az_filt[IMU8D_WINDOW_SIZE];// 低通滤波后的az
    uint16_t peak_times[IMU8D_MAX_PEAKS]; // 峰值时间戳（样本索引）
    uint8_t peak_count;              // 峰值计数

    IMU8DFeatures_t features;        // 提取的特征
} IMU8DBuffer_t;

/* 初始化 */
void IMU8D_Init(IMU8DBuffer_t *buf);

/* 填充样本（每100ms调用） */
void IMU8D_Feed(IMU8DBuffer_t *buf, float *accel, float *gyro);

/* 特征提取（5秒窗口满时调用） */
void IMU8D_Extract(IMU8DBuffer_t *buf);

/* Z-score归一化（使用pamap2_norm.h参数） */
void IMU8D_Normalize(IMU8DBuffer_t *buf);

/* 获取特征向量指针 */
IMU8DFeatures_t* IMU8D_GetFeatures(IMU8DBuffer_t *buf);

#endif /* IMU_FEATURES_8D_H */