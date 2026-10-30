/**
  ******************************************************************************
  * @file    fusion_rules.h
  * @brief   双通道疲劳融合规则模块
  ******************************************************************************
  * @attention
  *
  * 双通道融合规则（embedded_ai_prompt.md简化版）：
  * - 通道A（IMU）: Cube.AI MLP → Low/Med/High
  * - 通道B（心率）: PulseSensor → HRV特征 → 阈值判定 → warning (bool)
  *
  * 融合矩阵：
  *   IMU=High   → 强制High（运动学疲劳已确认）
  *   IMU=Low    → HRV预警触发? Medium : Low
  *   IMU=Medium → HRV预警触发? High : Medium
  *
  ******************************************************************************
  */

#ifndef FUSION_RULES_H
#define FUSION_RULES_H

#include <stdbool.h>
#include "fatigue_predict.h"
#include "hrv_analyzer.h"

/* 融合等级枚举（与FatigueLevel_t一致） */
typedef enum {
    FUSE_LOW = 0,
    FUSE_MEDIUM = 1,
    FUSE_HIGH = 2
} FuseLevel_t;

/* HRV阈值判定（embedded_ai_prompt.md 2.4节） */
bool HRV_CheckWarning(HRVFeatures_t *hrv);

/* 双通道融合 */
FuseLevel_t Fatigue_Fuse(FatigueLevel_t imu_level, bool hrv_warning);

#endif /* FUSION_RULES_H */