/**
  ******************************************************************************
  * @file    fatigue_predict.c
  * @brief   Cube.AI推理封装实现（Network A）
  ******************************************************************************
  */

#include "fatigue_predict.h"
#include "networka.h"
#include <stdio.h>
#include <math.h>

/* 全局变量 */
static ai_handle g_network = AI_HANDLE_NULL;
static ai_buffer *g_input = NULL;
static ai_buffer *g_output = NULL;
static FatiguePrediction_t g_pred;

/* Activations buffer（必须手动分配，96字节，4字节对齐） */
AI_ALIGNED(4)
static ai_u8 g_activations[96];

/* ========== 初始化 ========== */
void Fatigue_Init(void)
{
    ai_error err;
    ai_handle activations[1] = { AI_HANDLE_PTR(g_activations) };

    // 创建并初始化模型（传入activations地址）
    err = ai_networka_create_and_init(&g_network, activations, NULL);
    if (err.type != AI_ERROR_NONE) {
        printf("Cube.AI init failed: type=%d code=%d\r\n", err.type, err.code);
        return;
    }

    // 获取输入/输出buffer指针
    g_input = ai_networka_inputs_get(g_network, NULL);
    g_output = ai_networka_outputs_get(g_network, NULL);

    if (!g_input || !g_output) {
        printf("Cube.AI buffer get failed\r\n");
        return;
    }

    printf("Cube.AI networka Ready (8D)\r\n");
}

/* ========== 设置输入特征 ========== */
void Fatigue_SetIMUFeatures(float features[8])
{
    if (!g_input || !features) return;

    // 填充输入buffer（8维float）
    float *input_data = (float *)g_input->data;
    for (uint8_t i = 0; i < 8; i++) {
        input_data[i] = features[i];
    }
}

/* ========== 执行推理 ========== */
FatiguePrediction_t* Fatigue_Predict(void)
{
    if (!g_network || !g_input || !g_output) {
        printf("Cube.AI not initialized\r\n");
        return NULL;
    }

    // 执行推理
    ai_i32 batch = ai_networka_run(g_network, g_input, g_output);
    if (batch <= 0) {
        ai_error err = ai_networka_get_error(g_network);
        printf("ai_networka_run failed: type=%d code=%d\r\n", err.type, err.code);
        return NULL;
    }

    // 读取输出数据（3维float）
    float *output_data = (float *)g_output->data;
    g_pred.probs[0] = output_data[0];  // P_Low
    g_pred.probs[1] = output_data[1];  // P_Med
    g_pred.probs[2] = output_data[2];  // P_High

    // argmax → 疲劳等级
    float max_prob = g_pred.probs[0];
    int max_idx = 0;
    for (int i = 1; i < 3; i++) {
        if (g_pred.probs[i] > max_prob) {
            max_prob = g_pred.probs[i];
            max_idx = i;
        }
    }

    g_pred.level = (FatigueLevel_t)max_idx;
    g_pred.confidence = max_prob;

    return &g_pred;
}

/* ========== 获取最后一次预测结果 ========== */
FatiguePrediction_t* Fatigue_GetLastPrediction(void)
{
    return &g_pred;
}