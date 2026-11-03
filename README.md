# 多传感器疲劳检测系统（STM32U545）

一套运行在 STM32U545（Cortex-M33）上的穿戴式疲劳检测设备固件。系统有两条独立的分析通道：以光电心率构建的 **HRV 生理信号通道**，以及基于 IMU 数据的 **端侧神经网络通道**，两者按规则融合后给出疲劳等级，并通过 TouchGFX 三屏界面呈现。

> 本项目为团队作品：硬件 PCB 设计与焊接由队友完成，本仓库只包含软件部分。

---

## 功能特性

| 模块 | 说明 |
| --- | --- |
| HRV 时域分析 | 300 拍 IBI 环形缓冲，累计 50 拍起算；输出 MeanRR、SDNN、RMSSD、pNN50 |
| 副交感活性预警 | 两级判据：L1 为 RMSSD < 25 ms 且 SDNN < 50 ms；L2 在 L1 基础上叠加 pNN50 < 0.10 |
| IMU 特征工程 | MPU9250 100 Hz 采集，重力自动校准、零速修正、Z 轴峰值步态检测、三轴 Kalman 姿态解算 |
| 端侧推理 | 5 秒窗口 8 维特征 → 全连接网络三分类（Low / Medium / High），每 5 秒推理一次 |
| 双通道融合 | 模型结果与 HRV 预警按规则融合，抑制单通道误判 |
| 心率恢复速率 | 30 秒 HRR 状态机（IDLE → COUNTDOWN → RESULT），作为运动后评估结论 |
| 人机界面 | TouchGFX MVP 架构，ST7735S 128×128 SPI 屏，静息 / 运动 / 结果三屏循环 |

---

## 硬件组成

| 部件 | 型号 | 接口 |
| --- | --- | --- |
| 主控 | STM32U545RETxQ（Cortex-M33，带 FPU） | — |
| 光电心率传感器 | 模拟输出 | ADC + TIM1 定时采样 |
| 六轴 IMU | MPU9250 | I2C |
| 显示屏 | ST7735S 128×128 | SPI + DMA |
| 操作按键 | 机械按键 | EXTI |

---

## 软件架构

裸机前后台 + 中断驱动，无 RTOS。

```
中断层
  TIM1  500 Hz   ADC 采样，斜率法双沿确认心跳（100~300 ms 有效 IBI）
  TIM16  60 Hz   驱动 TouchGFX VSync
  EXTI           按键切换界面

主循环
  心跳确认 → IBI 入环形缓冲 → HRV 指标计算 → 疲劳预警
  IMU 采样 → 校准 → 步态检测 → 8 维特征提取 → Z-score 归一化 → 神经网络推理
  两路结果 → 规则融合 → UI 数据模型 → TouchGFX 渲染
```

中断内只做轻量状态更新，特征提取与推理全部下沉到主循环，保证高优先级采样不被阻塞。

### 目录结构

```
Inc/                            自研头文件
  hrv_analyzer.h                HRV 窗口与指标接口
  imu_features_8d.h             8 维特征定义
  fatigue_predict.h             模型推理封装
  fusion_rules.h                双通道融合规则
  gait_detect.h / kalman_filter.h / sensor_fusion.h / mpu9250_driver.h
  networka*.h                   X-CUBE-AI 生成代码的接口

Src/                            自研源文件
  main.c                        初始化与主循环
  pulse_sensor.c                ADC 心率采集与心跳确认
  hrv_analyzer.c                HRV 时域指标与环形缓冲
  mpu9250_driver.c / sensor_fusion.c / kalman_filter.c / gait_detect.c
  imu_features_8d.c             5 秒窗口 8 维特征提取
  fatigue_predict.c             激活缓冲、模型句柄与推理 API
  fusion_rules.c                预警判据与融合矩阵
  fatigue_analysis.c            HRR 状态机
  networka.c / networka_data.c / networka_data_params.c
                                X-CUBE-AI 生成的模型 C 代码
  app_touchgfx.c / TouchGFX*.cpp
  TFTc/                         ST7735S 底层驱动
  gui/ , generated/             TouchGFX 视图与生成代码

Python/                         训练侧数据生成脚本与示例数据
networka_generate_report.txt    ST Edge AI 转换报告（含逐层资源明细）
SPI_FullDuplex_ComIT_Master.ioc STM32CubeMX 工程文件
STM32U545RETXQ_FLASH.ld         链接脚本
Makefile                        命令行构建与烧录
```

---

## 端侧模型

训练侧导出的是 **float32 TFLite 模型**（PAMAP2 数据集，网络结构 8 → 16(ReLU) → 8(ReLU) → 3）。该模型通过 **ST Edge AI Core / STM32Cube.AI** 做离线图分析与 BatchNorm 折叠，**AOT 编译为纯 C 代码**并链接 CM33 专用运行时静态库，MCU 上不运行解释器，也没有运行时动态内存分配。

资源占用（数据见 `networka_generate_report.txt`）：

| 指标 | 数值 |
| --- | --- |
| 参数量 | 307（权重 1,228 B） |
| 乘加次数 | 331 MACC |
| 激活缓冲 | 96 B（单段，4 字节对齐，输入/中间层/输出共用） |
| C 层 / C 数组 | 5 层 / 12 个 |
| AI 部分 Flash | 9,952 B（其中 CM33 运行时 6,424 B） |
| AI 部分 RAM | 1,908 B |

推理接口封装在 `Src/fatigue_predict.c`，对外只有三个调用：初始化、写入 8 维特征、取预测结果（argmax + 置信度）。

---

## 构建

### 依赖

- STM32CubeIDE 1.19.0（内含 GNU Tools for STM32 13.3）
- STM32Cube FW_U5 V1.9.0
- TouchGFX Designer
- X-CUBE-AI（提供 `NetworkRuntime1020_CM33_GCC.a`）

### 命令

```bash
make all      # 编译，产物在 Debug/ 下
make flash    # 通过 SWD 烧录 Debug/MultiSensor_FatigueDetect.bin 到 0x08000000
make clean
```

`make all` 的执行链：`arm-none-eabi-gcc/g++` 编译 `.c/.cpp/.s` → `g++` 链接 `.elf` 并调用 `size` 打印占用 → `arm-none-eabi-objcopy` 导出 `.hex` / `.bin`。链接时额外引入 X-CUBE-AI 运行时与 TouchGFX 两个静态库。

### 关于第三方依赖

本仓库**不包含** ST 官方驱动与中间件（见 `.gitignore`）：`Drivers/`、`Middlewares/`、`Src/assets/` 均已排除，以免仓库体积膨胀并规避字体版权问题。要让工程可编译，需要在本地补齐：

1. 安装 STM32Cube FW_U5 V1.9.0；
2. 用 STM32CubeMX 打开 `SPI_FullDuplex_ComIT_Master.ioc` 生成 HAL 驱动与中间件；
3. 用 TouchGFX Designer 打开 `Src/SPI_FullDuplex_ComIT_Master.touchgfx` 生成 `Src/generated/` 与 `Src/assets/`；
4. 通过 X-CUBE-AI 获取运行时库，或重新导入模型生成 `networka*` 系列文件。

模型生成的 C 代码（`Src/networka*.c`）已随仓库提供，无需重新训练即可复现推理逻辑。

### 路径配置

`Makefile` 顶部的工具链路径与固件包路径是按作者本机环境写的，换机器构建时请调整：

```make
TOOLCHAIN_PATH ?= <arm-none-eabi-gcc 所在目录>
PROGRAMMER_PATH ?= <STM32_Programmer_CLI 所在目录>
TOPDIR          = <本工程绝对路径>
STM32CUBE_FW    = <STM32Cube_FW_U5_V1.9.0 路径>
```

---

## 关键实现说明

- **心跳确认**：不依赖固定阈值，用信号斜率与上升/下降双沿时间窗（100~300 ms）联合判定，抑制运动伪影造成的误触发。
- **HRV 收敛策略**：50 拍即输出首份结果，随后随窗口填充逐步收敛，避免长时间空窗。
- **双阈值预警**：要求多个指标同时越界，而非单指标抖动，降低误报。
- **特征一致性**：端侧 8 维特征的提取顺序与归一化参数与训练侧严格对齐，避免分布漂移。
- **内存规划**：模型激活缓冲手工指定为 96 B 且 4 字节对齐，输入/隐藏层/输出全部落在这一段，杜绝堆分配。

---

## 许可

本项目代码采用 [MIT License](LICENSE)。

`LICENSE_X-CUBE-AI.txt` 为 ST X-CUBE-AI 运行时库的许可条款，使用时请遵守其约定。STM32 HAL 驱动、TouchGFX 框架遵循各自的 ST 许可。
