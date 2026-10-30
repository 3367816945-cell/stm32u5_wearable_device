/**
 * @file    fatigue_analysis.c
 * @brief   疲劳分析状态机实现
 * @details 实现运动结束后心率恢复分析和疲劳评估
 */

#include "fatigue_analysis.h"
#include "sensor_data.h"
#include <stdio.h>

/* 全局实例 */
FatigueAnalysis_t g_fatigueAnalysis;

/**
 * @brief 初始化疲劳分析模块
 */
void FatigueAnalysis_Init(void)
{
    g_fatigueAnalysis.state = FA_STATE_IDLE;
    g_fatigueAnalysis.countdown_sec = 0;
    g_fatigueAnalysis.progress = 0;
    g_fatigueAnalysis.start_bpm = 0;
    g_fatigueAnalysis.end_bpm = 0;
    g_fatigueAnalysis.history_index = 0;
    g_fatigueAnalysis.fatigue_result = 0;
    g_fatigueAnalysis.confidence = 0;
    g_fatigueAnalysis.tick_counter = 0;

    /* 清空BPM历史 */
    for (int i = 0; i < 30; i++) {
        g_fatigueAnalysis.bpm_history[i] = 0;
    }
}

/**
 * @brief 启动疲劳分析
 */
void FatigueAnalysis_Start(void)
{
    g_fatigueAnalysis.state = FA_STATE_COUNTDOWN;
    g_fatigueAnalysis.countdown_sec = 30;
    g_fatigueAnalysis.progress = 0;
    g_fatigueAnalysis.start_bpm = sensorData.bpm;
    g_fatigueAnalysis.history_index = 0;
    g_fatigueAnalysis.tick_counter = 0;

    printf("Fatigue Analysis Started: StartBPM=%u\r\n", g_fatigueAnalysis.start_bpm);
}

/**
 * @brief 疲劳分析Tick函数（每帧调用）
 * @details 每60帧（约1秒）更新一次倒计时
 */
void FatigueAnalysis_Tick(void)
{
    if (g_fatigueAnalysis.state != FA_STATE_COUNTDOWN) {
        return;
    }

    /* 记录当前BPM */
    if (g_fatigueAnalysis.history_index < 30) {
        g_fatigueAnalysis.bpm_history[g_fatigueAnalysis.history_index++] = sensorData.bpm;
    }

    /* 更新倒计时（Model::tick()已做1秒节流） */
    g_fatigueAnalysis.countdown_sec--;
    g_fatigueAnalysis.progress = ((30 - g_fatigueAnalysis.countdown_sec) * 100) / 30;

    printf("Countdown: %us, Progress: %u%%, BPM=%u\r\n",
           g_fatigueAnalysis.countdown_sec, g_fatigueAnalysis.progress, sensorData.bpm);

    /* 倒计时结束，开始分析 */
    if (g_fatigueAnalysis.countdown_sec == 0) {
        g_fatigueAnalysis.end_bpm = sensorData.bpm;
        g_fatigueAnalysis.state = FA_STATE_ANALYZING;

        /* 计算心率恢复速度 */
        uint16_t bpm_drop = g_fatigueAnalysis.start_bpm - g_fatigueAnalysis.end_bpm;
        float recovery_rate = (float)bpm_drop / 30.0f;  /* BPM/秒 */

        printf("Analysis: StartBPM=%u, EndBPM=%u, Drop=%u, Rate=%.2f BPM/s\r\n",
               g_fatigueAnalysis.start_bpm, g_fatigueAnalysis.end_bpm,
               bpm_drop, recovery_rate);

        /* 根据恢复速率判断疲劳等级 */
        if (recovery_rate >= 2.0f) {
            g_fatigueAnalysis.fatigue_result = 0;  /* Low */
            g_fatigueAnalysis.confidence = 90;
        } else if (recovery_rate >= 1.0f) {
            g_fatigueAnalysis.fatigue_result = 1;  /* Med */
            g_fatigueAnalysis.confidence = 85;
        } else {
            g_fatigueAnalysis.fatigue_result = 2;  /* High */
            g_fatigueAnalysis.confidence = 95;
        }

        g_fatigueAnalysis.state = FA_STATE_RESULT;
        printf("Fatigue Result: %d, Confidence: %u%%\r\n",
               g_fatigueAnalysis.fatigue_result, g_fatigueAnalysis.confidence);
    }
}

/**
 * @brief 获取当前进度值
 */
uint8_t FatigueAnalysis_GetProgress(void)
{
    return g_fatigueAnalysis.progress;
}

/**
 * @brief 检查疲劳分析是否激活
 */
uint8_t FatigueAnalysis_IsActive(void)
{
    return (g_fatigueAnalysis.state != FA_STATE_IDLE) ? 1 : 0;
}