/**
  ******************************************************************************
  * @file    pamap2_norm.h
  * @brief   PAMAP2 Network A 归一化参数（8维特征）
  ******************************************************************************
  * @attention
  *
  * 用于Cube.AI推理前的Z-score归一化：
  * X_norm[i] = (X_raw[i] - mean[i]) / std[i]
  *
  ******************************************************************************
  */

#ifndef PAMAP2_NORM_H
#define PAMAP2_NORM_H

#define NUM_FEATURES 8

static const float feature_mean[NUM_FEATURES] = {
    15.015757f, 15.013099f, 84.788146f, 0.682729f, 95.039858f, 0.363933f, 0.391259f, 55.074834f
};

static const float feature_std[NUM_FEATURES] = {
    19.640697f, 5.474431f, 78.305794f, 0.219956f, 24.256212f, 0.183393f, 0.292514f, 66.859225f
};

#endif /* PAMAP2_NORM_H */