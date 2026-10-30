/*
 * gait_detect.c
 * 步态周期分割实现 - 自适应阈值法（论文第4.3-4.4节）
 * 增强版：自适应阈值 + 步长过滤 + 双峰抑制 + 步长时间记录（Stability域核心指标）
 */

#include "gait_detect.h"
#include <math.h>

/* 固定阈值仅在自适应阈值初始化完成前使用 */
#define GAIT_FIXED_THRESHOLD  0.15f
#define GAIT_MIN_STRIDE_MS    200.0f   /* 最小步长时间(ms) */
#define GAIT_MAX_STRIDE_MS    1500.0f  /* 最大步长时间(ms) */
#define GAIT_DOUBLE_PEAK_MS   200      /* 双峰抑制间隔(ms) */
#define GAIT_FALLING_FLOOR    0.05f    /* 回落判定阈值 */

static GaitState gait_state = GAIT_IDLE;
static uint16_t step_count = 0;
static float last_peak = 0.0f;
static uint32_t last_step_time = 0;
static uint32_t prev_step_time = 0;
static uint32_t last_peak_time = 0;

static float stride_buf[GAIT_STRIDE_BUF_SIZE];
static uint16_t stride_idx = 0;
static uint16_t stride_count = 0;

/* 自适应阈值：滑窗记录最近20步峰值 */
static float peak_buf[GAIT_STRIDE_BUF_SIZE];
static uint16_t peak_idx = 0;
static uint16_t peak_count = 0;
static float adaptive_threshold = GAIT_FIXED_THRESHOLD;

static float GetAdaptiveThreshold(void)
{
    if (peak_count == 0) return GAIT_FIXED_THRESHOLD;
    uint16_t n = (peak_count < GAIT_STRIDE_BUF_SIZE) ? peak_count : GAIT_STRIDE_BUF_SIZE;
    float sum = 0.0f;
    uint16_t i;
    for (i = 0; i < n; i++) {
        sum += peak_buf[i];
    }
    float avg = sum / (float)n;
    return (avg > GAIT_FIXED_THRESHOLD) ? (avg * 0.5f) : GAIT_FIXED_THRESHOLD;
}

void MPU9250_GaitDetect(float vertical_accel, uint32_t timestamp)
{
    float thresh = GetAdaptiveThreshold();

    switch (gait_state) {
    case GAIT_IDLE:
        if (vertical_accel > thresh) {
            gait_state = GAIT_RISING;
            last_peak = vertical_accel;
        }
        break;

    case GAIT_RISING:
        if (vertical_accel > last_peak) {
            last_peak = vertical_accel;
        }
        if (vertical_accel > thresh + 0.1f) {
            gait_state = GAIT_PEAK;
        }
        break;

    case GAIT_PEAK:
        if (vertical_accel > last_peak) {
            last_peak = vertical_accel;
        }
        if (vertical_accel < thresh) {
            /* 双峰抑制：距上次峰值 < 200ms 时跳过 */
            if (timestamp - last_peak_time < GAIT_DOUBLE_PEAK_MS) {
                gait_state = GAIT_FALLING;
                last_peak = 0.0f;
                break;
            }

            step_count++;
            prev_step_time = last_step_time;
            last_step_time = timestamp;

            if (prev_step_time > 0 && last_step_time > prev_step_time) {
                float stride_ms = (float)(last_step_time - prev_step_time);

                /* 步长时间有效性过滤 */
                if (stride_ms >= GAIT_MIN_STRIDE_MS && stride_ms <= GAIT_MAX_STRIDE_MS) {
                    stride_buf[stride_idx++] = stride_ms;
                    if (stride_idx >= GAIT_STRIDE_BUF_SIZE) stride_idx = 0;
                    if (stride_count < GAIT_STRIDE_BUF_SIZE) stride_count++;

                    /* 更新峰值滑窗 */
                    peak_buf[peak_idx++] = last_peak;
                    if (peak_idx >= GAIT_STRIDE_BUF_SIZE) peak_idx = 0;
                    if (peak_count < GAIT_STRIDE_BUF_SIZE) peak_count++;
                }
                /* 无效步长：不记录，但步数照常递增，仅丢弃buffer记录 */
            }

            last_peak_time = timestamp;
            last_peak = 0.0f;
            gait_state = GAIT_FALLING;
        }
        break;

    case GAIT_FALLING:
        if (vertical_accel < GAIT_FALLING_FLOOR) {
            gait_state = GAIT_IDLE;
        }
        break;
    }

    /* 更新自适应阈值（每次调用都刷新） */
    adaptive_threshold = GetAdaptiveThreshold();
}

uint16_t MPU9250_GetStepCount(void)
{
    return step_count;
}

float MPU9250_GetLastPeak(void)
{
    return last_peak;
}

uint32_t MPU9250_GetLastStepTime(void)
{
    return last_step_time;
}

float MPU9250_GetStrideTime(void)
{
    if (stride_count < 2) return 0.0f;

    float sum = 0.0f;
    uint16_t i;
    for (i = 0; i < stride_count; i++) {
        sum += stride_buf[i];
    }
    return sum / (float)stride_count;
}

float MPU9250_GetCVStrideTime(void)
{
    if (stride_count < 2) return 0.0f;

    float mean = MPU9250_GetStrideTime();
    float sum_sq = 0.0f;
    uint16_t i;
    for (i = 0; i < stride_count; i++) {
        float diff = stride_buf[i] - mean;
        sum_sq += diff * diff;
    }
    float std = sqrtf(sum_sq / (float)(stride_count - 1));
    return (mean > 0.0f) ? (std / mean) : 0.0f;
}

float MPU9250_GetCadence(void)
{
    float stride_ms = MPU9250_GetStrideTime();
    return (stride_ms > 0.0f) ? (60000.0f / stride_ms) : 0.0f;
}

uint16_t MPU9250_GetStrideCount(void)
{
    return stride_count;
}

float MPU9250_GetAdaptiveThreshold(void)
{
    return adaptive_threshold;
}