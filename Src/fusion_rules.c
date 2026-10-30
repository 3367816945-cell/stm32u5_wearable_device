/**
  ******************************************************************************
  * @file    fusion_rules.c
  * @brief   双通道疲劳融合规则实现
  ******************************************************************************
  */

#include "fusion_rules.h"
#include "sensor_data.h"
#include <stdio.h>

/* ========== 全局传感器数据 ========== */
SensorData_t sensorData = {0};

/* ========== HRV阈值判定 ========== */
bool HRV_CheckWarning(HRVFeatures_t *hrv)
{
    if (!hrv) return false;

    /* 
     * 一级预警（建议休息）：
     *   if (RMSSD < 25ms) && (SDNN < 50ms) → warning=true
     * 
     * 二级预警（强制休息）：
     *   if (RMSSD < 25ms) && (SDNN < 50ms) && (pNN50 < 0.10) → warning=true
     */

    // 一级预警条件
    bool level1_warning = (hrv->RMSSD < 25.0f) && (hrv->SDNN < 50.0f);

    // 二级预警条件
    bool level2_warning = level1_warning && (hrv->pNN50 < 0.10f);

    if (level2_warning) {
        printf("HRV Level2 Warning: RMSSD=%.1f SDNN=%.1f pNN50=%.2f\r\n",
               hrv->RMSSD, hrv->SDNN, hrv->pNN50);
        return true;
    }

    if (level1_warning) {
        printf("HRV Level1 Warning: RMSSD=%.1f SDNN=%.1f\r\n",
               hrv->RMSSD, hrv->SDNN);
        return true;
    }

    return false;
}

/* ========== 双通道融合 ========== */
FuseLevel_t Fatigue_Fuse(FatigueLevel_t imu_level, bool hrv_warning)
{
    /*
     * 融合矩阵：
     *   IMU=High   → 强制High（运动学疲劳已确认）
     *   IMU=Low    → HRV预警触发? Medium : Low
     *   IMU=Medium → HRV预警触发? High : Medium
     */

    switch (imu_level) {
        case FATIGUE_HIGH:
            // IMU=High → 强制High（运动学疲劳已确认）
            return FUSE_HIGH;

        case FATIGUE_LOW:
            // IMU=Low → HRV预警触发? Medium : Low
            if (hrv_warning) {
                printf("Fusion: IMU=Low + HRV_warn → Medium\r\n");
                return FUSE_MEDIUM;
            } else {
                return FUSE_LOW;
            }

        case FATIGUE_MEDIUM:
            // IMU=Medium → HRV预警触发? High : Medium
            if (hrv_warning) {
                printf("Fusion: IMU=Medium + HRV_warn → High\r\n");
                return FUSE_HIGH;
            } else {
                return FUSE_MEDIUM;
            }

        default:
            return FUSE_LOW;
    }
}