/**
  ******************************************************************************
  * @file    fatigue_predict.h
  * @brief   Cube.AI推理封装模块（Network A）
  ******************************************************************************
  * @attention
  *
  * 基于实际生成的networka API：
  * - ai_networka_create_and_init()
  * - ai_networka_inputs_get()
  * - ai_networka_outputs_get()
  * - ai_networka_run()
  *
  * 输入：8维IMU特征（归一化后）
  * 输出：3类疲劳等级概率 [P_Low, P_Med, P_High]
  *
  ******************************************************************************
  */

#ifndef FATIGUE_PREDICT_H
#define FATIGUE_PREDICT_H

#include <stdint.h>
#include <stdbool.h>

/* 疲劳等级枚举 */
typedef enum {
    FATIGUE_LOW = 0,      // 疲劳前期（正常）
    FATIGUE_MEDIUM = 1,   // 疲劳中期（轻度疲劳）
    FATIGUE_HIGH = 2      // 疲劳后期（重度疲劳）
} FatigueLevel_t;

/* 预测结果结构体 */
typedef struct {
    FatigueLevel_t level;  // 疲劳等级（argmax）
    float confidence;      // 置信度（Softmax最大值）
    float probs[3];        // 三类概率 [P_Low, P_Med, P_High]
} FatiguePrediction_t;

/* 初始化Cube.AI模型 */
void Fatigue_Init(void);

/* 设置输入特征（8维，归一化后） */
void Fatigue_SetIMUFeatures(float features[8]);

/* 执行推理并返回结果 */
FatiguePrediction_t* Fatigue_Predict(void);

/* 获取最后一次预测结果 */
FatiguePrediction_t* Fatigue_GetLastPrediction(void);

#endif /* FATIGUE_PREDICT_H */