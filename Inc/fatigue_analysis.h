/**
 * @file    fatigue_analysis.h
 * @brief   疲劳分析状态机
 * @details 运动结束后通过心率恢复曲线评估疲劳程度
 */

#ifndef FATIGUE_ANALYSIS_H
#define FATIGUE_ANALYSIS_H

#include <stdint.h>
#include <stdbool.h>

/* 疲劳分析状态 */
typedef enum {
    FA_STATE_IDLE = 0,        // 空闲状态
    FA_STATE_COUNTDOWN,       // 倒计时中（30秒）
    FA_STATE_ANALYZING,       // 分析中
    FA_STATE_RESULT           // 显示结果
} FatigueAnalysisState_t;

/* 疲劳分析数据结构 */
typedef struct {
    FatigueAnalysisState_t state;       // 当前状态
    uint8_t countdown_sec;              // 倒计时秒数（30->0）
    uint8_t progress;                   // 进度百分比（100->0）
    uint16_t start_bpm;                 // 开始时的BPM
    uint16_t end_bpm;                   // 结束时的BPM
    uint16_t bpm_history[30];           // 30秒BPM历史记录
    uint8_t history_index;              // 历史记录索引
    uint8_t fatigue_result;             // 分析结果：0=Low, 1=Med, 2=High
    uint8_t confidence;                 // 置信度（百分比）
    uint32_t tick_counter;              // 帧计数器（用于1秒定时）
} FatigueAnalysis_t;

/* 全局实例 */
extern FatigueAnalysis_t g_fatigueAnalysis;

/* 屏幕切换请求标志（C→C++桥接） */
extern volatile uint8_t g_fatigueScreenSwitchRequested;

/* 接口函数 */
void FatigueAnalysis_Init(void);
void FatigueAnalysis_Start(void);
void FatigueAnalysis_Tick(void);
uint8_t FatigueAnalysis_GetProgress(void);
uint8_t FatigueAnalysis_IsActive(void);

#endif /* FATIGUE_ANALYSIS_H */