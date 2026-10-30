#ifndef IMU_FEATURES_H
#define IMU_FEATURES_H

#include <stdint.h>

#define IMU_FEATURE_DEBUG  1
#define IMU_WINDOW_SIZE    20
#define IMU_FEATURE_COUNT  7
#define IMU_EVERSION_BUF_SIZE 20

/* 三域特征结构体 */
typedef struct {
    /* Impact域 */
    float peak_acc_z;
    float peak_acc_mag;

    /* Stability域（最高权重） */
    float cv_stride_time;
    float cv_eversion_range;

    /* Gait策略域 */
    float stride_time;
    float stance_time;
    float cadence;

    /* 基线参数（7维，与特征顺序一致） */
    float baseline_mean[IMU_FEATURE_COUNT];
    float baseline_std[IMU_FEATURE_COUNT];

    uint32_t timestamp;
} IMUFeatures_t;

/* 三域得分结构体（论文可解释疲劳综合分） */
typedef struct {
    float impact_score;     /* I = peak_acc_z + peak_acc_mag */
    float stability_score;  /* S = cv_stride_time + cv_eversion_range */
    float gait_score;       /* G = stride_time + stance_time + cadence */
    float fatigue_score;    /* F = w1*I + w2*S + w3*G */
} IMU_DomainScore_t;

typedef struct {
    float ax_buf[IMU_WINDOW_SIZE];
    float ay_buf[IMU_WINDOW_SIZE];
    float az_buf[IMU_WINDOW_SIZE];
    float gx_buf[IMU_WINDOW_SIZE];
    float gy_buf[IMU_WINDOW_SIZE];
    float gz_buf[IMU_WINDOW_SIZE];
    uint16_t buf_idx;
    uint16_t sample_count;

    float peak_acc_z_val;
    float peak_acc_mag_val;

    IMUFeatures_t features;
    uint8_t features_ready;

    uint8_t baseline_calibrated;

    uint16_t step_count_in_window;
    uint16_t prev_step_count;
    uint32_t window_start_tick;  /* hybrid窗口：时间戳起点 */

    /* eversion_range跟踪 */
    float eversion_buf[IMU_EVERSION_BUF_SIZE];
    uint16_t eversion_idx;
    uint16_t eversion_count;
} IMUFeatureBuffer_t;

void IMUFeatures_Init(IMUFeatureBuffer_t *buf);
void IMUFeatures_Feed(IMUFeatureBuffer_t *buf, float *accel, float *gyro);
void IMUFeatures_FeedGait(IMUFeatureBuffer_t *buf, float stride_time, float cv_stride, float cadence, float eversion);
void IMUFeatures_CalibrateBaseline(IMUFeatureBuffer_t *buf);
IMUFeatures_t* IMUFeatures_GetVector(IMUFeatureBuffer_t *buf);
IMU_DomainScore_t IMUFeatures_GetDomainScores(IMUFeatureBuffer_t *buf);

#endif /* IMU_FEATURES_H */