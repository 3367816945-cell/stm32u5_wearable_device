/*
   PulseSensor 心跳检测算法实现 - 移植自 PulseSensor Playground
   原始库: https://github.com/WorldFamousElectronics/PulseSensorPlayground
   Copyright World Famous Electronics LLC - see LICENSE
   
   移植说明:
   - 从 C++ 类方法改为 C 函数, 通过 PulseSensor_t 指针操作
   - 移除 analogRead() 调用, ADC 值由外部写入 ps->Signal
   - sampleCounter 每次固定 +2 (500Hz 采样)
   - 移除了 pinMode/digitalWrite/analogWrite 的 LED 操作
   - constrain() 替换为 CLAMP() 宏
*/

#include "pulse_sensor.h"

/* ========== 辅助宏 ========== */

/* 约束值在 [lo, hi] 范围内 */
#define CLAMP(x, lo, hi) (((x) < (lo)) ? (lo) : (((x) > (hi)) ? (hi) : (x)))

/* ========== 函数实现 ========== */

void PulseSensor_Init(PulseSensor_t *ps)
{
    if (!ps) return;

    /* 默认配置 */
    ps->Signal = 0;
    ps->threshSetting = 550;     /* 默认阈值 */
    ps->FadeLevel = 0;

    PulseSensor_Reset(ps);
}

void PulseSensor_Reset(PulseSensor_t *ps)
{
    if (!ps) return;

    for (int i = 0; i < 10; ++i) {
        ps->rate[i] = 0;
    }

    ps->QS          = false;
    ps->BPM         = 0;
    ps->IBI         = 750;       /* 750ms/beat ≈ 80 BPM */
    ps->Pulse       = false;
    ps->sampleCounter = 0;
    ps->lastBeatTime = 0;
    ps->P           = 512;       /* 峰值初始为量程中点 (0~1023) */
    ps->T           = 512;       /* 谷值初始为量程中点 */
    ps->thresh      = ps->threshSetting;
    ps->amp         = 100;       /* 幅值初始为量程 1/10 */
    ps->firstBeat   = true;
    ps->secondBeat  = false;
    ps->FadeLevel   = 0;
    ps->N           = 0;
}

void PulseSensor_SetThreshold(PulseSensor_t *ps, int threshold)
{
    if (!ps) return;

    /* 关中断保护 (STM32 单核无竞争, 保留语义) */
    ps->threshSetting = threshold;
    ps->thresh        = threshold;  /* 官方算法:直接使用用户设定的阈值 */
}

/*
   核心处理函数: 每次采样后调用
   必须在 500Hz (2ms 间隔) 下调用, 否则时间相关逻辑会出错
*/
void PulseSensor_ProcessSample(PulseSensor_t *ps)
{
    if (!ps) return;

    /* ---- 时间跟踪 ---- */
    ps->sampleCounter += PULSE_SENSOR_SAMPLE_INTERVAL_MS;
    ps->N = (int)(ps->sampleCounter - ps->lastBeatTime);

    /* ---- LED 淡出 ---- */
    ps->FadeLevel -= PULSE_SENSOR_FADE_PER_SAMPLE;
    ps->FadeLevel = CLAMP(ps->FadeLevel, 0, PULSE_SENSOR_MAX_FADE_LEVEL);

    /* ---- 寻峰寻谷 ---- */
    /*
       在安全区间 (距上次心跳 > IBI*3/5) 内寻找谷值,
       避免双峰噪声干扰
    */
    if (ps->Signal < ps->thresh && ps->N > (ps->IBI / 5) * 3) {
        if (ps->Signal < ps->T) {
            ps->T = ps->Signal;      /* 更新谷值 */
        }
    }

    /* 信号 > 阈值时, 跟踪峰值 */
    if (ps->Signal > ps->thresh && ps->Signal > ps->P) {
        ps->P = ps->Signal;          /* 更新峰值 */
    }

    /* ---- 心跳检测（官方算法） ---- */
    /* 信号超过阈值, 且不在脉冲内, 且距上次心跳足够远 */
    if (ps->N > 250) {               /* 过滤 250ms 内的高频噪声 */
        if ((ps->Signal > ps->thresh) &&
            (ps->Pulse == false) &&
            (ps->N > (ps->IBI / 5) * 3))
        {
            ps->Pulse = true;        /* 置位脉冲标志 */
            ps->IBI = (int)(ps->sampleCounter - ps->lastBeatTime);
            ps->lastBeatTime = ps->sampleCounter;

            if (ps->secondBeat) {
                /* 第二次心跳: 用当前 IBI 填充整个 rate 数组 */
                ps->secondBeat = false;
                for (int i = 0; i <= 9; i++) {
                    ps->rate[i] = ps->IBI;
                }
            }

            if (ps->firstBeat) {
                /* 首次心跳: IBI 不可靠, 跳过 */
                ps->firstBeat = false;
                ps->secondBeat = true;
                return;
            }

            /* 滑动平均: 更新最近 10 次 IBI */
            uint16_t runningTotal = 0;
            for (int i = 0; i <= 8; i++) {
                ps->rate[i] = ps->rate[i + 1];
                runningTotal += ps->rate[i];
            }
            ps->rate[9] = ps->IBI;
            runningTotal += ps->rate[9];
            runningTotal /= 10;

            ps->BPM = 60000 / (int)runningTotal;
            ps->QS = true;           /* 置位心跳标志 */
            ps->FadeLevel = PULSE_SENSOR_MAX_FADE_LEVEL;
            
            /* 注意：官方算法不在心跳检测时更新阈值 */
            /* 阈值更新在脉冲结束时进行（见第148-154行） */
        }
    }

    /* ---- 脉冲结束（官方算法核心） ---- */
    /* 信号回落到阈值以下, 本次心跳脉冲结束 */
    if (ps->Signal < ps->thresh && ps->Pulse == true) {
        ps->Pulse = false;           /* 重置脉冲标志 */
        ps->amp = ps->P - ps->T;     /* 计算本次脉搏幅值 */
        ps->thresh = ps->amp / 2 + ps->T;  /* 更新阈值为幅值的一半（官方算法） */
        /* 注意：官方算法没有限制阈值范围！ */
        ps->P = ps->thresh;          /* 重置峰值跟踪起点 */
        ps->T = ps->thresh;          /* 重置谷值跟踪起点 */
    }

    /* ---- 超时复位 ---- */
    /* 2.5 秒未检测到心跳, 重置算法状态 */
    if (ps->N > 2500) {
        ps->thresh      = ps->threshSetting;
        ps->P           = 512;
        ps->T           = 512;
        ps->lastBeatTime = ps->sampleCounter;
        ps->firstBeat   = true;
        ps->secondBeat  = false;
        ps->QS          = false;
        ps->BPM         = 0;
        ps->IBI         = 600;       /* 600ms/beat ≈ 100 BPM */
        ps->Pulse       = false;
        ps->amp         = 100;
    }
}

/*
   读取并清除心跳起始标志 (线程安全, 单核无竞争)
   返回 true 表示检测到新的心跳起始
*/
bool PulseSensor_SawStartOfBeat(PulseSensor_t *ps)
{
    if (!ps) return false;

    bool started = ps->QS;
    ps->QS = false;
    return started;
}