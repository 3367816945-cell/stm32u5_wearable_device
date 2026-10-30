/*
   PulseSensor 心跳检测算法 - 移植自 PulseSensor Playground
   原始库: https://github.com/WorldFamousElectronics/PulseSensorPlayground
   Copyright World Famous Electronics LLC - see LICENSE
   
   移植说明：
   - 将 C++ 类封装改为 C 风格结构体 + 函数
   - 移除 Arduino 平台依赖 (analogRead, pinMode 等)
   - ADC 值由外部传入 (通过 g_ps.Signal 赋值)
   - 采样率固定 500Hz (2ms), sampleCounter 每次 +2
*/

#ifndef PULSE_SENSOR_H
#define PULSE_SENSOR_H

#include <stdint.h>
#include <stdbool.h>

/* ========== 常量定义 ========== */

/* 采样相关 */
#define PULSE_SENSOR_SAMPLE_INTERVAL_MS  2   // 2ms 采样间隔 (500Hz)

/* LED 淡出效果参数 (保留供参考, 实际 LED 在主逻辑控制) */
#define PULSE_SENSOR_FADE_SCALE          10
#define PULSE_SENSOR_FADE_PER_SAMPLE     12
#define PULSE_SENSOR_MAX_FADE_LEVEL      (255 * PULSE_SENSOR_FADE_SCALE)

/* ========== 数据结构 ========== */

typedef struct {
    /* ---- 输入 ---- */
    int Signal;                    /* 最新 ADC 原始信号值 (0~1023, 外部赋值) */

    /* ---- 输出 ---- */
    volatile int BPM;              /* 每分钟心跳数 */
    volatile int IBI;              /* 心跳间期 (ms) */
    volatile bool Pulse;           /* 当前是否在心跳脉冲内 */
    volatile bool QS;              /* 新心跳标志 (主循环读取后需清除) */
    volatile int amp;              /* 脉搏波幅值 */
    volatile unsigned long lastBeatTime; /* 上次心跳的采样计数值 */

    /* ---- 算法内部 ---- */
    unsigned long sampleCounter;   /* 累计采样时间 (ms) */
    int rate[10];                  /* 最近 10 次 IBI 记录 */
    int N;                         /* 距上次心跳的采样时间 (ms) */
    int P;                         /* 波形峰值跟踪 */
    int T;                         /* 波形谷值跟踪 */
    int thresh;                    /* 动态阈值 */
    int threshSetting;             /* 用户设定的初始阈值 (后备) */
    bool firstBeat;                /* 首次心跳标志 */
    bool secondBeat;               /* 第二次心跳标志 */

    /* ---- LED 淡出 (保留, 当前未使用) ---- */
    int FadeLevel;
} PulseSensor_t;

/* ========== 函数声明 ========== */

/* 初始化传感器结构体 */
void PulseSensor_Init(PulseSensor_t *ps);

/* 重置所有算法变量 (回到初始状态) */
void PulseSensor_Reset(PulseSensor_t *ps);

/* 设置初始阈值 (通常 550) */
void PulseSensor_SetThreshold(PulseSensor_t *ps, int threshold);

/* 核心处理函数: 每次采样后调用一次 */
void PulseSensor_ProcessSample(PulseSensor_t *ps);

/* ---- 查询函数 ---- */
static inline int  PulseSensor_GetBPM(PulseSensor_t *ps)    { return ps->BPM; }
static inline int  PulseSensor_GetIBI(PulseSensor_t *ps)    { return ps->IBI; }
static inline int  PulseSensor_GetSignal(PulseSensor_t *ps) { return ps->Signal; }
static inline int  PulseSensor_GetAmplitude(PulseSensor_t *ps) { return ps->amp; }

/* 读取并清除心跳起始标志 */
bool PulseSensor_SawStartOfBeat(PulseSensor_t *ps);

/* 查询是否在心跳脉冲内 */
static inline bool PulseSensor_IsInsideBeat(PulseSensor_t *ps) { return ps->Pulse; }

#endif /* PULSE_SENSOR_H */