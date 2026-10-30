/*
 * hrv_analyzer.h
 * HRV分析器头文件 - 时域特征提取（移植自Python版）
 *
 * 医学依据：
 * - SDNN < 50ms：猝死高风险预测因子
 * - RMSSD < 25ms：副交感神经活性下降
 * - pNN50：反映心率变异性的稳定性
 *
 * 技术参数：
 * - 滑动窗口：300次心跳（医学标准5分钟）
 * - 输出：4维时域HRV特征向量
 */

#ifndef HRV_ANALYZER_H
#define HRV_ANALYZER_H

#include <stdint.h>
#include <stdbool.h>

/* 滑动窗口大小（医学标准5分钟≈300次心跳） */
#define HRV_WINDOW_SIZE     300
#define HRV_MIN_BEATS       50   /* 最少心跳数才计算HRV */

/* HRV特征结构体 */
typedef struct {
    float MeanRR;      /* 平均心跳间期（ms） */
    float SDNN;        /* IBI标准差（ms） */
    float RMSSD;       /* 相邻差值均方根（ms） */
    float pNN50;       /* 差值>50ms的比例（0~1） */
    uint16_t n_beats;  /* 窗口内心跳数 */
    uint32_t timestamp;/* 时间戳（ms） */
} HRVFeatures_t;

/* HRV分析器结构体 */
typedef struct {
    uint16_t ibi_buffer[HRV_WINDOW_SIZE];  /* IBI序列缓存 */
    uint32_t ts_buffer[HRV_WINDOW_SIZE];   /* 时间戳缓存 */
    uint16_t buf_idx;                      /* 当前索引 */
    uint16_t buf_count;                    /* 缓存计数 */

    HRVFeatures_t features;                /* 最新HRV特征 */
    bool features_ready;                   /* 特征就绪标志 */
} HRVAnalyzer_t;

/* 初始化HRV分析器 */
void HRV_Init(HRVAnalyzer_t *hrv);

/* 添加IBI样本 */
void HRV_AddSample(HRVAnalyzer_t *hrv, uint32_t timestamp, uint16_t ibi);

/* 计算时域HRV特征 */
void HRV_Compute(HRVAnalyzer_t *hrv);

/* 获取最新HRV特征 */
HRVFeatures_t* HRV_GetFeatures(HRVAnalyzer_t *hrv);

/* 检查是否达到最小窗口 */
bool HRV_IsWindowFull(HRVAnalyzer_t *hrv);

#endif /* HRV_ANALYZER_H */