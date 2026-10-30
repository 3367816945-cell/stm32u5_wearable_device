很好，这一题已经是你整个项目里**最核心的“理论闭环推导”问题**了。

我给你做一件很重要的事：

> 把“传感器原始数据 → 疲劳预测结果”写成一条**完整可发表级数学链路（end-to-end mathematical pipeline）**

并且每一步都严格对应你现在的系统（PPG + IMU + MLP + Cube.AI），同时按照三域疲劳生物力学框架重构。

---

# 🧠 一、总体目标（你要在论文里写的系统函数）

你的系统本质是一个函数：

[
f: (PPG(t), IMU(t)) \rightarrow F
]

其中：

* (PPG(t))：光电容积脉搏波（连续时间信号）
* (IMU(t))：三轴加速度+角速度
* (F)：综合疲劳指数（0~100连续值，可解释）

---

# 🚨 二、完整数学链路（核心）— 三域框架版

我给你分成 **7层数学变换**（这是最标准论文写法）

---

# 🔷 Step 1：连续物理信号

## PPG信号

[
x_p(t) = s_p(t) + n_p(t)
]

* (s_p(t))：真实血容量变化
* (n_p(t))：噪声

---

## IMU信号

[
x_a(t) = [a_x(t), a_y(t), a_z(t)]
]

[
x_g(t) = [g_x(t), g_y(t), g_z(t)]
]

---

# 🔷 Step 2：采样（ADC离散化）

[
x_p[n] = x_p(nT_s)
]

[
x_a[n], x_g[n]
]

得到离散序列：

```text
PPG[n]
IMU[n]
```

---

# 🔷 Step 3：信号预处理（滤波）

## PPG滤波（带通滤波）

[
\tilde{x}_p[n] = H_{bp}(x_p[n])
]

典型频带：

[
0.5Hz \sim 4Hz
]

---

## IMU滤波（低通）

[
\tilde{x}_a[n] = H_{lp}(x_a[n])
]

---

# 🔷 Step 4：心跳检测（Peak Detection）

从PPG提取峰值：

[
t_i = \arg\max \tilde{x}_p[n]
]

得到心跳间隔：

[
RR_i = t_{i+1} - t_i
]

---

# 🔷 Step 5：HRV特征提取（关键）

---

## 1️⃣ MeanRR

[
\mu_{RR} = \frac{1}{N} \sum RR_i
]

---

## 2️⃣ SDNN

[
SDNN = \sqrt{\frac{1}{N-1} \sum (RR_i - \mu)^2}
]

---

## 3️⃣ RMSSD

[
RMSSD = \sqrt{\frac{1}{N-1} \sum (RR_{i+1} - RR_i)^2}
]

---

## 4️⃣ pNN50

[
pNN50 = \frac{1}{N} \sum I(|RR_{i+1}-RR_i| > 50ms)
]

---

# 🔷 Step 6：IMU特征提取（三域框架核心）

**⚠️ 已淘汰指标**（工程不稳定，论文不可复现）：
- Jerk（加加速度）：滤波参数敏感
- Sample Entropy（样本熵）：窗口/噪声敏感
- loading_rate（冲击加载率）：<200Hz失真

---

## 域1：冲击控制下降（Impact域）

### 1️⃣ 垂直冲击峰值 peak_acc_z

[
peak\_acc\_z = \max(|a_z[n]|)
]

**生理机制**：小腿、足部缓冲肌群疲劳 → 落地减震能力衰退 → 垂直冲击加速度升高

### 2️⃣ 合加速度峰值 peak_acc_mag

[
peak\_acc\_mag = \max(\sqrt{a_x[n]^2 + a_y[n]^2 + a_z[n]^2})
]

---

## 域2：神经肌肉稳定性衰退（Stability域 — 最高权重）

### 3️⃣ 变异系数 CV（最核心疲劳指标）

[
CV = \frac{\sigma}{\mu} \times 100\%
]

其中：

[
\mu = \frac{1}{N} \sum x_i
]

[
\sigma = \sqrt{\frac{1}{N-1} \sum (x_i - \mu)^2}
]

**应用**：`cv_stride_time`、`cv_eversion_range`

**生理机制**：踝周肌肉控制力下降 → 步间波动增大 → CV升高

---

## 域3：步态节能代偿（Gait策略域）

### 4️⃣ 单步时长 stride_time

[
stride\_time = \frac{1}{N} \sum (t_{i+1} - t_i)
]

其中 (t_i) 为第i步时间戳

### 5️⃣ 步频 cadence

[
cadence = \frac{60}{\text{stride\_time}}
]

### 6️⃣ 支撑时间 stance_time（高采样率需200Hz+）

[
stance\_time = \frac{1}{N} \sum (t_{off,i} - t_{on,i})
]

其中 (t_{on,i}) 为触地时刻，(t_{off,i}) 为离地时刻

---

# 🔷 Step 7：受试者内基线归一化（最关键预处理）

**公式**：

[
x' = \frac{x - \mu_{baseline}}{\sigma_{baseline}}
]

**定义**：
- (x)：当前特征值
- (\mu_{baseline})：该受试者未疲劳安静跑步时特征均值
- (\sigma_{baseline})：该受试者未疲劳安静跑步时特征标准差

**作用**：消除身高、体重、跑步姿势的个体基线差异，只保留疲劳带来的相对变化。

---

# 🔷 Step 8：特征向量构建（三域融合核心）

## HRV向量：

[
x_{HRV} = [SDNN, RMSSD, MeanRR, pNN50]
]

---

## IMU向量（三域框架）：

[
x_{IMU} = [peak\_acc\_z, peak\_acc\_mag, cv\_stride\_time, cv\_eversion\_range, stride\_time, stance\_time, cadence]
]

**维度**：7维（Phase2A：4维可落地；Phase2B：7维完整）

---

## 三域特征加权得分：

[
I = w_{I1} \cdot peak\_acc\_z + w_{I2} \cdot peak\_acc\_mag
]

[
S = w_{S1} \cdot cv\_stride\_time + w_{S2} \cdot cv\_eversion\_range
]

[
G = w_{G1} \cdot stride\_time + w_{G2} \cdot stance\_time + w_{G3} \cdot cadence
]

---

## 可解释疲劳综合分（物理机制驱动）：

[
F = w_1 \cdot I + w_2 \cdot S + w_3 \cdot G
]

| 参数 | 含义 | 权重建议 |
|------|------|---------|
| F | 综合疲劳指数 | 最终输出 0~100 |
| I | 冲击域得分 | w₁：中等 |
| S | 稳定性域得分 | w₂：最大（神经控制衰退最敏感） |
| G | 步态代偿得分 | w₃：中等 |
| w₁/w₂/w₃ | 机制贡献权重 | 通过统计/训练确定 |

**答辩价值**：物理机制驱动的融合模型，完美规避"深度学习黑盒"质疑。

---

## 融合：

[
x = [x_{HRV}, x_{IMU}]
]

最终：

[
x \in \mathbb{R}^{11}
]

---

# 🔷 Step 9：MLP前向传播（Cube.AI核心）

## 第一层

[
h_1 = ReLU(W_1 x + b_1)
]

---

## 第二层

[
h_2 = ReLU(W_2 h_1 + b_2)
]

---

## 输出层

[
y = Softmax(W_3 h_2 + b_3)
]

---

# 🔷 Step 10：疲劳概率输出

[
y = [P_0, P_1, P_2]
]

例如：

* (P_0)：正常
* (P_1)：轻度疲劳
* (P_2)：严重疲劳

---

# 🔷 Step 11：决策函数（最终输出）

[
\hat{y} = \arg\max(y)
]

---

# 🔷 Step 12：疲劳指数连续化（加分项）

[
FatigueScore = \sum_{i=0}^2 P_i \cdot w_i
]

例如：

[
Score = 0\cdot P_0 + 0.5\cdot P_1 + 1.0\cdot P_2
]

---

# 📌 三、整条完整数学链（论文最重要）

你可以直接写这一条：

[
PPG(t), IMU(t)
]

↓

[
x_p[n], x_a[n]
]

↓

[
RR_i
]

↓

[
HRV = f(RR)
]

↓

[
IMU\_features = g(a,g) \quad (\text{三域框架})
]

↓

[
\text{基线归一化}: x' = \frac{x - \mu_b}{\sigma_b}
]

↓

[
F = w_1 I + w_2 S + w_3 G \quad (\text{可解释疲劳综合分})
]

↓

[
y = f_\theta(x) \quad (\text{MLP分类})
]

↓

[
Fatigue / Risk
]

---

# 🧠 四、这条链路为什么“是成立的”（学术依据）

你的模型对应三个成熟理论：

---

## 1️⃣ 信号处理理论

* sampling theorem
* bandpass filtering
* stochastic signal processing

---

## 2️⃣ 生理建模

* autonomic nervous system (HRV)

---

## 3️⃣ 生物力学（三域机制）

* **Impact域**：小腿缓冲肌群疲劳 → 冲击升高（Derrick 2002）
* **Stability域**：踝周肌肉控制衰退 → 步间波动增大（Muhovic 2024）
* **Gait域**：全身代谢疲劳 → 步态节能代偿（PLOS ONE）

---

## 4️⃣ 机器学习

* nonlinear function approximation (MLP)

---

# 🚀 五、最关键结论（评委最在意）

你的系统不是：

❌ 黑盒AI

而是：

> ✔ “信号处理 + 生理建模 + **三域生物力学机制** + 可解释加权融合 + 非线性映射”

---

# ⭐ 六、一句话总结（答辩杀手锏）

你可以直接说：

> 本系统通过对PPG与IMU信号进行时域与统计特征提取，基于冲击控制下降、神经肌肉稳定性衰退、步态节能代偿三大疲劳生物力学机制构建可解释融合模型，将生理状态与运动状态映射到统一特征空间，并利用轻量级多层感知机学习其非线性关系，从而实现对人体疲劳状态的实时估计与风险预测。

---

# 🚀 如果你下一步要继续升级（我建议）

我可以帮你再做三个“比赛冲奖级内容”：

### ✔ 1. 实验数据生成与验证方法（ROC + confusion matrix）

### ✔ 2. CubeAI模型结构图（可以直接画进PPT）

### ✔ 3. 整套论文 Method + Result + Discussion

只要说一句：

> **“继续完善论文结果部分”**

我可以帮你直接做到“可以投稿IEEE学生论文”的完整版本。