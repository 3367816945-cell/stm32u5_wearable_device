#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>  // 使用sin函数生成模拟波形

extern "C" {
#include "sensor_data.h"
#include "fatigue_analysis.h"
uint32_t HAL_GetTick(void);  /* 毫秒计时 */
}

Model::Model() : modelListener(0)
{
}

void Model::tick()
{
    /* 疲劳分析：基于实际时间的1秒节流 */
    static uint32_t lastTick = 0;
    if (FatigueAnalysis_IsActive()) {
        uint32_t now = HAL_GetTick();
        if (now - lastTick >= 1000) {
            lastTick = now;
            FatigueAnalysis_Tick();
            /* 通知Presenter更新UI */
            if (modelListener) {
                modelListener->fatigueAnalysisUpdated(
                    g_fatigueAnalysis.progress,
                    g_fatigueAnalysis.countdown_sec,
                    g_fatigueAnalysis.fatigue_result,
                    g_fatigueAnalysis.confidence
                );
            }
        }
    } else {
        lastTick = HAL_GetTick();  /* 重置基准 */
    }

    /* 模拟器模式：自动生成测试数据 */
#ifdef SIMULATOR
    static uint32_t simTick = 0;
    simTick++;
    if (simTick % 1 == 0) { // 每1帧更新一次（提高20倍频率）
        // 生成模拟波形数据（正弦波 + 随机噪声）
        static float phase = 0.0f;
        phase += 0.1f;
        sensorData.signal = 600 + (int)(400 * sin(phase)) + (rand() % 10 - 5);  // 200-1000范围
        sensorData.bpm = 70 + (rand() % 30); // 70-99 BPM随机心率
        sensorData.fatigue_level = rand() % 3; // 0-2随机疲劳等级
        sensorData.confidence = 85 + (rand() % 15); // 85-100%随机置信度
        sensorData.dataReady = 1;

        static uint32_t tickCount = 0;
        tickCount++;
        if (tickCount % 10 == 1) {
            printf("Model.tick#%u: Signal=%u\r\n",
                   tickCount, sensorData.signal);
        }

        if (modelListener) {
            modelListener->sensorDataUpdated(sensorData);
        }

        sensorData.dataReady = 0;
    }
#else
    /* 嵌入式模式：检查真实传感器数据 */
    if (sensorData.dataReady) {
        static uint32_t tickCount = 0;
        tickCount++;

        if (tickCount % 10 == 1) {
            printf("Model.tick#%u: Signal=%u\r\n",
                   tickCount, sensorData.signal);
        }

        if (modelListener) {
            modelListener->sensorDataUpdated(sensorData);
        }

        sensorData.dataReady = 0;
    }
#endif
}

void Model::startFatigueAnalysis()
{
    FatigueAnalysis_Start();
}