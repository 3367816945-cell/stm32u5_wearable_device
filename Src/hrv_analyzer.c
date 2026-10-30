/*
 * hrv_analyzer.c
 * HRV分析器实现 - 时域特征提取（移植自Python版hrv_analyzer.py）
 *
 * 计算指标：
 * - MeanRR：平均心跳间期（ms）
 * - SDNN：IBI标准差（ms）
 * - RMSSD：相邻差值均方根（ms）
 * - pNN50：差值>50ms的比例（0~1）
 *
 * 参考：PulseSensor/python/hrv_analyzer.py compute_time_domain()
 */

#include "hrv_analyzer.h"
#include <math.h>
#include <string.h>

/* 初始化HRV分析器 */
void HRV_Init(HRVAnalyzer_t *hrv)
{
    if (!hrv) return;

    memset(hrv->ibi_buffer, 0, sizeof(hrv->ibi_buffer));
    memset(hrv->ts_buffer, 0, sizeof(hrv->ts_buffer));
    hrv->buf_idx = 0;
    hrv->buf_count = 0;

    hrv->features.MeanRR = 0.0f;
    hrv->features.SDNN = 0.0f;
    hrv->features.RMSSD = 0.0f;
    hrv->features.pNN50 = 0.0f;
    hrv->features.n_beats = 0;
    hrv->features.timestamp = 0;
    hrv->features_ready = false;
}

/* 添加IBI样本到环形缓冲区 */
void HRV_AddSample(HRVAnalyzer_t *hrv, uint32_t timestamp, uint16_t ibi)
{
    if (!hrv) return;

    hrv->ibi_buffer[hrv->buf_idx] = ibi;
    hrv->ts_buffer[hrv->buf_idx] = timestamp;

    hrv->buf_idx = (hrv->buf_idx + 1) % HRV_WINDOW_SIZE;

    if (hrv->buf_count < HRV_WINDOW_SIZE) {
        hrv->buf_count++;
    }
}

/* 计算时域HRV特征 */
void HRV_Compute(HRVAnalyzer_t *hrv)
{
    if (!hrv || hrv->buf_count < HRV_MIN_BEATS) return;

    uint16_t n = hrv->buf_count;

    /* ===== Step 1: 计算MeanRR ===== */
    float sum = 0.0f;
    for (uint16_t i = 0; i < n; i++) {
        sum += (float)hrv->ibi_buffer[i];
    }
    hrv->features.MeanRR = sum / (float)n;

    /* ===== Step 2: 计算SDNN ===== */
    float var_sum = 0.0f;
    for (uint16_t i = 0; i < n; i++) {
        float diff = (float)hrv->ibi_buffer[i] - hrv->features.MeanRR;
        var_sum += diff * diff;
    }
    hrv->features.SDNN = sqrtf(var_sum / (float)n);

    /* ===== Step 3: 计算RMSSD和pNN50 ===== */
    float rmssd_sum = 0.0f;
    uint16_t nn50_count = 0;

    for (uint16_t i = 0; i < n - 1; i++) {
        float diff = (float)hrv->ibi_buffer[i + 1] - (float)hrv->ibi_buffer[i];
        float abs_diff = fabsf(diff);

        rmssd_sum += diff * diff;

        if (abs_diff > 50.0f) {
            nn50_count++;
        }
    }

    hrv->features.RMSSD = sqrtf(rmssd_sum / (float)(n - 1));
    hrv->features.pNN50 = (float)nn50_count / (float)(n - 1);

    /* ===== Step 4: 记录元数据 ===== */
    hrv->features.n_beats = n;
    hrv->features.timestamp = hrv->ts_buffer[(hrv->buf_idx == 0) ? HRV_WINDOW_SIZE - 1 : hrv->buf_idx - 1];
    hrv->features_ready = true;
}

/* 获取最新HRV特征 */
HRVFeatures_t* HRV_GetFeatures(HRVAnalyzer_t *hrv)
{
    if (!hrv || !hrv->features_ready) return NULL;
    return &hrv->features;
}

/* 检查是否达到最小窗口 */
bool HRV_IsWindowFull(HRVAnalyzer_t *hrv)
{
    if (!hrv) return false;
    return (hrv->buf_count >= HRV_MIN_BEATS);
}