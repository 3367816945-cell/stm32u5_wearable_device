#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
仿真疲劳数据生成器 - 第四阶段Cube.AI模型部署验证

医学依据：
- HRV特征：SDNN↓RMSSD↓为副交感神经活性下降（疲劳标志）
- IMU特征：基于"666.md"论文三域生理机制
  - Impact域↑（缓冲肌群疲劳 → 冲击增强）
  - Stability域↑（神经控制衰退 → 波动增大）
  - Gait域↑（节能代偿 → 步态调整）

生成策略：
- 疲劳前期：HRV正常范围 + IMU低冲击/高稳定性
- 疲劳中期：HRV中度下降 + IMU中冲击/中稳定性
- 疲劳后期：HRV显著下降 + IMU高冲击/低稳定性
"""

import random
import csv

# 生成仿真数据集
random.seed(42)
n_samples = 200  # 每阶段200样本

def generate_fatigue_data(phase):
    """
    phase: 0=疲劳前期, 1=疲劳中期, 2=疲劳后期
    
    返回：
    - HRV特征（4维）：MeanRR, SDNN, RMSSD, pNN50
    - IMU特征（7维）：peak_acc_z, peak_acc_mag, cv_stride_time, cv_eversion_range, stride_time, stance_time, cadence
    """
    # 生成正态分布数据（简化版）
    def normal(mean, std, n):
        return [random.gauss(mean, std) for _ in range(n)]
    
    if phase == 0:
        # ===== 疲劳前期（正常状态）=====
        # HRV特征（医学依据：正常心率变异性范围）
        hrv = {
            'MeanRR': normal(750, 50, n_samples),   # 平均心跳间期750ms（80 BPM）
            'SDNN': normal(50, 10, n_samples),      # 标准差50ms（正常范围）
            'RMSSD': normal(30, 5, n_samples),      # RMSSD 30ms（副交感神经活性正常）
            'pNN50': normal(0.20, 0.05, n_samples)  # pNN50 20%（心搏变异正常）
        }
        # IMU特征（Impact↓Stability↑Gait正常）
        imu = {
            'peak_acc_z': normal(0.8, 0.2, n_samples),      # 垂直冲击峰值0.8g（低冲击）
            'peak_acc_mag': normal(1.0, 0.3, n_samples),    # 合加速度峰值1.0g
            'cv_stride_time': normal(0.05, 0.02, n_samples),# 步长变异系数0.05（高稳定性）
            'cv_eversion_range': normal(0.10, 0.03, n_samples), # 踝外翻波动0.10
            'stride_time': normal(800, 100, n_samples),     # 单步时长800ms
            'stance_time': normal(250, 50, n_samples),      # 支撑时间250ms
            'cadence': normal(150, 10, n_samples)           # 步频150spm
        }
        
    elif phase == 1:
        # ===== 疲劳中期（轻度疲劳）=====
        # HRV特征（医学依据：副交感神经活性下降）
        hrv = {
            'MeanRR': normal(850, 60, n_samples),   # 平均心跳间期850ms（70 BPM，心率下降）
            'SDNN': normal(40, 12, n_samples),      # 标准差40ms（↓下降）
            'RMSSD': normal(25, 6, n_samples),      # RMSSD 25ms（↓副交感活性下降）
            'pNN50': normal(0.15, 0.04, n_samples)  # pNN50 15%（↓心搏变异下降）
        }
        # IMU特征（Impact↑Stability↑Gait↑）
        imu = {
            'peak_acc_z': normal(1.2, 0.3, n_samples),      # 垂直冲击峰值1.2g（↑缓冲肌群疲劳）
            'peak_acc_mag': normal(1.5, 0.4, n_samples),    # 合加速度峰值1.5g
            'cv_stride_time': normal(0.10, 0.03, n_samples),# 步长变异系数0.10（↑神经控制衰退）
            'cv_eversion_range': normal(0.15, 0.04, n_samples), # 踝外翻波动0.15（↑）
            'stride_time': normal(900, 120, n_samples),     # 单步时长900ms（↑步长缩短）
            'stance_time': normal(300, 60, n_samples),      # 支撑时间300ms（↑节能代偿）
            'cadence': normal(140, 12, n_samples)           # 步频140spm（↓下降）
        }
        
    else:
        # ===== 疲劳后期（重度疲劳）=====
        # HRV特征（医学依据：副交感神经活性显著下降，猝死风险因子）
        hrv = {
            'MeanRR': normal(950, 70, n_samples),   # 平均心跳间期950ms（63 BPM）
            'SDNN': normal(30, 15, n_samples),      # 标准差30ms（↓显著下降，<50ms为高风险）
            'RMSSD': normal(20, 7, n_samples),      # RMSSD 20ms（↓<25ms为副交感活性下降）
            'pNN50': normal(0.10, 0.03, n_samples)  # pNN50 10%（↓显著下降）
        }
        # IMU特征（Impact显著↑Stability显著↑Gait显著↑）
        imu = {
            'peak_acc_z': normal(1.6, 0.4, n_samples),      # 垂直冲击峰值1.6g（↑显著增强）
            'peak_acc_mag': normal(2.0, 0.5, n_samples),    # 合加速度峰值2.0g
            'cv_stride_time': normal(0.15, 0.05, n_samples),# 步长变异系数0.15（↑波动显著增大）
            'cv_eversion_range': normal(0.20, 0.05, n_samples), # 踝外翻波动0.20（↑）
            'stride_time': normal(1000, 150, n_samples),    # 单步时长1000ms（↑步长显著缩短）
            'stance_time': normal(350, 70, n_samples),      # 支撑时间350ms（↑代偿明显）
            'cadence': normal(130, 15, n_samples)           # 步频130spm（↓显著下降）
        }
    
    return hrv, imu

# ===== 生成三阶段数据 =====
print("=== 疲劳仿真数据生成器 ===")
print(f"生成策略：每阶段{n_samples}样本 × 3阶段 = {n_samples * 3}总样本")
print("HRV特征（4维）：MeanRR, SDNN, RMSSD, pNN50")
print("IMU特征（7维）：peak_acc_z, peak_acc_mag, cv_stride_time, cv_eversion_range, stride_time, stance_time, cadence")
print("")

data_list = []
for phase in range(3):
    phase_name = ["疲劳前期", "疲劳中期", "疲劳后期"][phase]
    print(f"生成 {phase_name} 数据...")
    
    hrv, imu = generate_fatigue_data(phase)
    
    for i in range(n_samples):
        row = {
            'timestamp': i * 100 + phase * 10000,
            'MeanRR': hrv['MeanRR'][i],
            'SDNN': hrv['SDNN'][i],
            'RMSSD': hrv['RMSSD'][i],
            'pNN50': hrv['pNN50'][i],
            'peak_acc_z': imu['peak_acc_z'][i],
            'peak_acc_mag': imu['peak_acc_mag'][i],
            'cv_stride_time': imu['cv_stride_time'][i],
            'cv_eversion_range': imu['cv_eversion_range'][i],
            'stride_time': imu['stride_time'][i],
            'stance_time': imu['stance_time'][i],
            'cadence': imu['cadence'][i],
            'fatigue_label': phase + 1  # 标签1/2/3
        }
        data_list.append(row)

# ===== 保存为CSV文件 =====
output_file = 'simulation_fatigue_data.csv'
fieldnames = ['timestamp', 'MeanRR', 'SDNN', 'RMSSD', 'pNN50', 
              'peak_acc_z', 'peak_acc_mag', 'cv_stride_time', 
              'cv_eversion_range', 'stride_time', 'stance_time', 'cadence', 'fatigue_label']

with open(output_file, 'w', newline='') as csvfile:
    writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
    writer.writeheader()
    writer.writerows(data_list)

print("")
print(f"✅ 仿真数据集生成完成！")
print(f"   文件位置：{output_file}")
print(f"   总样本数：{len(data_list)}")
print(f"   特征维度：{len(fieldnames) - 2}维（HRV 4维 + IMU 7维）")
print("")

# ===== 显示前5行数据 =====
print("=== 前5行数据预览 ===")
with open(output_file, 'r') as csvfile:
    reader = csv.DictReader(csvfile)
    for i, row in enumerate(reader):
        if i < 5:
            print(f"Row {i+1}: timestamp={row['timestamp']}, MeanRR={row['MeanRR']:.1f}, SDNN={row['SDNN']:.1f}, fatigue_label={row['fatigue_label']}")