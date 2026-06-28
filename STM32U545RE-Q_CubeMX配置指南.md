# STM32U545RE-Q PulseSensor项目CubeMX配置指南

## 📋 配置概述

本文档详细说明如何使用STM32CubeMX配置NUCLEO-U545RE-Q开发板的PulseSensor项目，参考STM32F103C8T6的配置流程，并针对U545RE-Q的硬件特性进行调整。

---

## 🔧 硬件差异对比

### 关键硬件差异

| 项目 | STM32F103C8T6 | STM32U545RE-Q | 说明 |
|------|---------------|---------------|------|
| **时钟源** | 8MHz HSE（外部晶振） | **无HSE**（使用MSI/HSI内部时钟） | ⚠️ 重要差异！U545RE-Q无板载HSE |
| **系统时钟** | 72MHz | 160MHz | U545性能更强 |
| **ADC引脚** | PA0 (ADC1_IN0) | PA4 (ADC1_IN9) | Arduino A2接口 |
| **调试接口** | SWD | ST-Link V3 | U545板载ST-Link V3 |
| **功耗模式** | 标准功耗 | **SMPS供电**（低功耗） | U545特色功能 |

---

## 🎯 CubeMX配置详细步骤

### 步骤1：启动CubeMX并打开.ioc文件

1. **启动STM32CubeMX**
   - 打开STM32CubeIDE或独立版STM32CubeMX

2. **打开.ioc配置文件**
   ```
   文件路径：e:\stm32u545\STM32CubeU5\Projects\NUCLEO-U545RE-Q\Examples\PulseSensor\PulseSensor.ioc
   ```
   - File → Open File → 选择PulseSensor.ioc
   - 或直接双击PulseSensor.ioc文件

3. **验证基本配置**
   - MCU型号：STM32U545RETxQ（自动加载）
   - 封装：LQFP64
   - 工具链：STM32CubeIDE（已配置）

---

### 步骤2：时钟系统配置（关键！）

#### ⚠️ 重要：U545RE-Q无HSE晶振，必须使用内部时钟

1. **打开时钟配置界面**
   - 点击"Clock Configuration"标签页

2. **时钟源选择**
   ```
   ✅ 正确配置：
   - PLL Source：MSI (4MHz) ← 必选！U545RE-Q无HSE
   
   ❌ 错误配置：
   - PLL Source：HSE (16MHz) ← U545RE-Q无板载HSE，不要选择！
   ```

3. **PLL配置参数**
   ```
   Input clock (MSI)：4 MHz
   PLLM：1（不分频）
   PLLN：80（倍频因子）
   PLLP：2（系统时钟分频）
   PLLQ：2（USB时钟分频）
   PLLR：2（备用分频）
   
   结果：
   - VCO输出：320 MHz (4MHz × 80)
   - 系统时钟：160 MHz (320MHz ÷ 2)
   - USB时钟：48 MHz (HSI48专用时钟)
   ```

4. **时钟树配置图**
   ```
   MSI (4MHz)
     ↓
   PLL (×80 → ÷2)
     ↓
   SYSCLK = 160MHz
     ↓
   ┌─────────────┬─────────────┬─────────────┐
   │   AHB       │   APB1      │   APB2      │
   │  160MHz     │  160MHz     │  160MHz     │
   └─────────────┴─────────────┴─────────────┘
   ```

5. **验证时钟配置**
   - 系统时钟（SYSCLK）：160 MHz ✅
   - HCLK（AHB总线）：160 MHz ✅
   - APB1/APB2时钟：160 MHz ✅
   - ADC时钟：16 MHz ✅
   - USB时钟：48 MHz（HSI48）✅

---

### 步骤3：ADC配置（脉搏信号采集）

#### 3.1 ADC引脚配置

1. **打开Pinout视图**
   - 点击"Pinout & Configuration"标签页

2. **配置PA4引脚（Arduino A2接口）**
   ```
   引脚：PA4
   位置：Arduino连接器 A2接口（易于连接传感器）
   
   ⚠️ 重要：PA4对应ADC1_IN9，不是IN4！
   - Mode：ADC1_IN9 (Single-Ended)
   - Signal：ADC1_IN9
   ```

3. **配置步骤**
   - 在Pinout视图中找到PA4引脚
   - 右键点击PA4 → 选择"ADC1_IN9"
   - 或在左侧"Analog" → "ADC1"中配置

#### 3.2 ADC参数配置

1. **打开ADC1配置界面**
   - 左侧"Analog" → "ADC1"
   - 点击"Parameter Settings"

2. **ADC基本参数**
   ```
   ADC_Settings:
   - Clock Prescaler：Asynchronous clock mode divided by 2 (ADC_CLOCK_ASYNC_DIV2)
   - Resolution：12 bits (ADC_RESOLUTION_12B)
   - Data Alignment：Right alignment (ADC_DATAALIGN_RIGHT)
   - Scan Conversion Mode：Disabled（单通道模式）
   - Continuous Conversion Mode：Disabled（单次转换）
   - Discontinuous Conversion Mode：Disabled
   - Low Power Auto Wait：Disabled
   - End Of Conversion Selection：Single conversion (EOC flag at end of single conversion)
   
   ADC_Regular_ConversionMode:
   - Number Of Conversion：1
   - External Trigger Conversion Source：Software Trigger (ADC_SOFTWARE_START)
   - External Trigger Conversion Edge：none
   - Enable Regular Conversion：Enable
   
   Channel_0 Settings:
   - Channel：ADC_CHANNEL_9 ← PA4对应通道9
   - Sampling Time：391.5 ADC clock cycles（采样时间）
   - Offset Number：ADC_OFFSET_NONE
   - Rank：1（转换序列第一位）
   ```

3. **采样时间说明**
   ```
   采样时间：391.5 cycles
   - ADC时钟：16 MHz
   - 采样时间：391.5 cycles × (1/16MHz) ≈ 24.47 μs
   - 转换时间：12.5 cycles + 391.5 cycles = 404 cycles ≈ 25.25 μs
   - 采样率：约39.6 kHz（远高于脉搏信号频率）
   
   ✅ 足够采样脉搏信号（约1Hz）！
   ```

#### 3.3 ADC中断配置（可选）

1. **启用ADC中断**
   - 左侧"Core" → "NVIC"
   - NVIC Settings：
     ```
     ADC1 interrupt：Enabled ✅
     Priority：0（最高优先级）
     SubPriority：0
     ```

2. **中断回调函数**
   ```c
   // 在stm32u5xx_it.c中自动生成
   void ADC1_IRQHandler(void)
   {
     HAL_ADC_IRQHandler(&hadc1);
   }
   
   // 在main.c中添加回调函数
   void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
   {
     if(hadc->Instance == ADC1)
     {
       uint16_t adc_value = HAL_ADC_GetValue(&hadc1);
       // 处理ADC数据
     }
   }
   ```

---

### 步骤4：USART配置（串口数据传输）

#### 4.1 USART引脚配置

.ioc文件已预配置USART1引脚，验证配置：

1. **验证USART1引脚**
   ```
   PA9：USART1_TX
   PA10：USART1_RX
   Mode：Asynchronous（异步模式）
   GPIO Pull-up/Pull-down：Pull-down（下拉）
   GPIO Speed：Very High（高速）
   ```

2. **连接方式**
   ```
   NUCLEO-U545RE-Q板载ST-Link V3提供虚拟串口：
   - PA9 → ST-Link VCOM_TX
   - PA10 → ST-Link VCOM_RX
   - 通过USB连接PC，自动识别为虚拟串口
   
   ✅ 无需外部连接！直接通过USB使用串口
   ```

#### 4.2 USART参数配置

.ioc文件已预配置USART参数：

```
USART1 Configuration:
- Mode：Asynchronous
- Baud Rate：115200 bit/s（与F103一致）
- Word Length：8 bits
- Parity：None
- Stop Bits：1
- Data Direction：Receive and Transmit
- Over Sampling：16 Samples
```

---

### 步骤5：TIM1配置（定时采样）

.ioc文件已预配置TIM1定时器：

#### 5.1 TIM1基本配置

```
TIM1 Configuration:
- Clock Source：Internal Clock（内部时钟）
- Prescaler (PSC)：8000 ← 自定义参数
- Counter Mode：Up（向上计数）
- Counter Period (ARR)：200 ← 自定义参数
- auto-reload preload：Enable
```

#### 5.2 采样频率计算

```
定时器频率计算：
- 定时器时钟：160 MHz（APB2时钟）
- 预分频后：160 MHz ÷ 8000 = 20 kHz
- 计数周期：20 kHz ÷ 200 = 100 Hz
- 采样间隔：10 ms

✅ 100 Hz采样率，适合脉搏信号采集（约1Hz信号）
```

#### 5.3 TIM1中断配置（重要！）

1. **启用TIM1中断**
   - .ioc文件已配置TIM1中断：
     ```
     TIM1 break interrupt：Enabled
     TIM1 update interrupt：Enabled ← 用于定时触发ADC
     Priority：0
     SubPriority：0
     ```

2. **定时器中断回调**
   ```c
   // 在main.c中添加
   void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
   {
     if(htim->Instance == TIM1)
     {
       // 定时触发ADC采样
       HAL_ADC_Start_IT(&hadc1);
       
       // 或直接读取ADC
       HAL_ADC_Start(&hadc1);
       HAL_ADC_PollForConversion(&hadc1, 10);
       uint16_t adc_value = HAL_ADC_GetValue(&hadc1);
       // 处理ADC数据
     }
   }
   ```

---

### 步骤6：GPIO配置（LED指示）

.ioc文件已预配置LED引脚：

```
PA5 Configuration:
- GPIO Mode：Output
- GPIO Pull-up/Pull-down：No pull-up, no pull-down
- GPIO Speed：Low
- GPIO Label：LED_STATUS ← 自定义标签
- Maximum Output Speed：Low
```

**LED功能**：用于指示脉搏检测状态或系统运行状态。

---

### 步骤7：代码生成

#### 7.1 生成代码

1. **生成项目代码**
   - Project → Generate Code
   - 或按Ctrl+S保存并生成

2. **生成位置**
   ```
   输出目录：e:\stm32u545\STM32CubeU5\Projects\NUCLEO-U545RE-Q\Examples\PulseSensor\
   
   生成的关键文件：
   - Core/Src/main.c：主程序
   - Core/Src/stm32u5xx_hal_msp.c：硬件初始化
   - Core/Src/stm32u5xx_it.c：中断处理
   - Core/Inc/main.h：主头文件
   - Core/Inc/stm32u5xx_hal_conf.h：HAL配置
   ```

#### 7.2 生成的初始化函数

.ioc文件已配置函数列表：

```c
// 自动生成的初始化函数顺序
1. SystemClock_Config()      // 时钟配置（RCC）
2. MX_GPIO_Init()            // GPIO初始化
3. MX_USART1_UART_Init()     // USART1初始化
4. MX_TIM1_Init()            // TIM1初始化
5. MX_ADC1_Init()            // ADC1初始化（您添加后）
6. MX_ICACHE_Init()          // 指令缓存初始化
7. MX_PWR_Init()             // 电源管理初始化
8. MX_CORTEX_M33_NS_Init()   // Cortex-M33配置
```

---

## 🔄 PulseSensor代码移植指南

### 从F103移植到U545RE-Q的关键修改

#### 1. HAL库头文件修改

```c
// ❌ F103使用的头文件
#include "stm32f1xx_hal.h"

// ✅ U545RE-Q使用的头文件
#include "stm32u5xx_hal.h"
```

#### 2. ADC通道修改

```c
// ❌ F103配置（PA0）
#define ADC_CHANNEL  ADC_CHANNEL_0  // PA0 = IN0

// ✅ U545RE-Q配置（PA4）
#define ADC_CHANNEL  ADC_CHANNEL_9  // PA4 = IN9
```

#### 3. 定时器配置调整

```c
// F103定时器配置（TIM2）
// ✅ U545RE-Q定时器配置（TIM1）
// 定时器频率从72MHz变为160MHz
// 预分频和周期参数需要重新计算

// F103 TIM2配置示例（72MHz）
TIM_HandleTypeDef htim2;
htim2.Init.Prescaler = 7200;   // 72MHz ÷ 7200 = 10kHz
htim2.Init.Period = 100;       // 10kHz ÷ 100 = 100Hz

// ✅ U545RE-Q TIM1配置（160MHz）
TIM_HandleTypeDef htim1;
htim1.Init.Prescaler = 8000;   // 160MHz ÷ 8000 = 20kHz
htim1.Init.Period = 200;       // 20kHz ÷ 200 = 100Hz
```

#### 4. 时钟系统差异

```c
// ❌ F103使用HSE（8MHz外部晶振）
RCC_OscInitTypeDef RCC_OscInitStruct = {0};
RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
RCC_OscInitStruct.HSEState = RCC_HSE_ON;
RCC_OscInitStruct.PLL.Source = RCC_PLLSOURCE_HSE;
RCC_OscInitStruct.PLL.PLLM = 1;  // 8MHz输入
RCC_OscInitStruct.PLL.PLLN = 9;  // 72MHz输出

// ✅ U545RE-Q使用MSI（4MHz内部时钟）
RCC_OscInitTypeDef RCC_OscInitStruct = {0};
RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
RCC_OscInitStruct.MSIState = RCC_MSI_ON;
RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_4;  // 4MHz
RCC_OscInitStruct.PLL.Source = RCC_PLLSOURCE_MSI;
RCC_OscInitStruct.PLL.PLLM = 1;  // 4MHz输入
RCC_OscInitStruct.PLL.PLLN = 80; // 160MHz输出（4MHz × 80 ÷ 2）
```

---

## 📝 PulseSensor代码文件移植

### 需要复制的文件

从F103项目复制以下文件到U545RE-Q项目：

```
源项目：e:\PulseSensorPlayground-master\PulseSensorPlayground-master\stm32f103_pulse\stm32f103_pulse\Core\
目标项目：e:\stm32u545\STM32CubeU5\Projects\NUCLEO-U545RE-Q\Examples\PulseSensor\STM32CubeIDE\...\Core\

复制文件：
1. Src/pulse_sensor.c → Core/Src/pulse_sensor.c
2. Inc/pulse_sensor.h → Core/Inc/pulse_sensor.h
```

### 需要修改的代码部分

#### 1. pulse_sensor.h修改

```c
// 修改头文件引用
#ifndef PULSE_SENSOR_H
#define PULSE_SENSOR_H

// ❌ F103
// #include "stm32f1xx_hal.h"

// ✅ U545RE-Q
#include "stm32u5xx_hal.h"

// ADC通道定义修改
// ❌ #define ADC_CHANNEL  ADC_CHANNEL_0
// ✅ #define ADC_CHANNEL  ADC_CHANNEL_9  // PA4

// 定时器修改
// ❌ extern TIM_HandleTypeDef htim2;
// ✅ extern TIM_HandleTypeDef htim1;

#endif
```

#### 2. pulse_sensor.c修改

```c
// ADC初始化函数修改
void PulseSensor_Init(void)
{
  // ADC配置
  ADC_ChannelConfTypeDef sConfig = {0};
  
  // ❌ F103通道0
  // sConfig.Channel = ADC_CHANNEL_0;
  
  // ✅ U545RE-Q通道9
  sConfig.Channel = ADC_CHANNEL_9;  // PA4
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_391CYCLES_5;
  
  HAL_ADC_ConfigChannel(&hadc1, &sConfig);
  
  // 启动定时器
  // ❌ HAL_TIM_Base_Start_IT(&htim2);
  // ✅ HAL_TIM_Base_Start_IT(&htim1);
  HAL_TIM_Base_Start_IT(&htim1);
}
```

#### 3. main.c集成

```c
// main.c中添加PulseSensor初始化和回调

#include "pulse_sensor.h"

// 主函数中初始化
int main(void)
{
  // ... HAL初始化 ...
  MX_GPIO_Init();
  MX_ADC1_Init();       // ADC初始化
  MX_TIM1_Init();       // 定时器初始化
  MX_USART1_UART_Init(); // 串口初始化
  
  // ✅ PulseSensor初始化
  PulseSensor_Init();
  
  while (1)
  {
    // 主循环处理
  }
}

// 定时器中断回调
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if(htim->Instance == TIM1)  // ✅ TIM1而不是TIM2
  {
    // ✅ 触发ADC采样或调用PulseSensor处理函数
    PulseSensor_TimerCallback();
  }
}

// ADC转换完成回调
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  if(hadc->Instance == ADC1)
  {
    uint16_t adc_value = HAL_ADC_GetValue(&hadc1);
    // ✅ 调用PulseSignal处理函数
    PulseSensor_ADCCallback(adc_value);
  }
}
```

---

## 🎨 波形显示配置

### 串口输出波形数据

参考F103的波形显示方案，U545RE-Q使用USART1输出数据：

```c
// 波形数据输出函数
void SendWaveformData(uint16_t adc_value)
{
  char buffer[20];
  sprintf(buffer, "%d\n", adc_value);
  HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
}

// 在ADC回调中调用
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  if(hadc->Instance == ADC1)
  {
    uint16_t adc_value = HAL_ADC_GetValue(&hadc1);
    SendWaveformData(adc_value);  // 发送数据到串口
  }
}
```

### Python波形显示脚本（参考F103）

```python
# serial_plot.py
import serial
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# ✅ U545RE-Q虚拟串口（自动识别）
ser = serial.Serial('COM?', 115200, timeout=1)  # 替换COM号为实际串口

# 初始化波形
fig, ax = plt.subplots()
x_data, y_data = [], []
line, = ax.plot(x_data, y_data)

def update(frame):
    if ser.in_waiting:
        data = ser.readline().decode().strip()
        if data:
            y_data.append(int(data))
            x_data.append(len(y_data))
            line.set_data(x_data, y_data)
            ax.relim()
            ax.autoscale_view()
    return line,

ani = FuncAnimation(fig, update, blit=True, interval=10)
plt.show()
```

---

## ⚠️ 重要注意事项

### 1. 硬件连接差异

```
PulseSensor传感器连接：

✅ U545RE-Q连接方式：
- 信号输出 → Arduino A2接口（PA4）
- VCC → 3.3V或5V（根据传感器规格）
- GND → GND

⚠️ PA0不可用：
- PA0在NUCLEO-U545RE-Q上未焊接连接
- PA0预留用于唤醒功能
- 必须使用PA4（ADC1_IN9）
```

### 2. 时钟系统差异（关键！）

```
❌ 错误配置：
- 使用HSE作为PLL源
- U545RE-Q无板载HSE晶振
- CubeMX会显示"HSE not available"

✅ 正确配置：
- 使用MSI (4MHz) 作为PLL源
- MSI是内部时钟，无需外部晶振
- PLL配置：4MHz × 80 ÷ 2 = 160MHz
```

### 3. 采样频率调整

```
F103采样频率：100Hz（10ms间隔）
✅ U545RE-Q保持相同采样频率：
- TIM1配置：160MHz ÷ 8000 ÷ 200 = 100Hz
- 保持与F103相同的10ms采样间隔
- 确保脉搏信号处理算法兼容
```

### 4. ADC精度差异

```
F103 ADC：12位分辨率
✅ U545RE-Q ADC：12位分辨率（相同）
- 转换结果范围：0~4095
- 输入电压范围：0~3.3V
- 精度一致，无需修改算法
```

---

## 📊 配置验证清单

### CubeMX配置验证

✅ **时钟系统**
- MSI时钟源：4 MHz ✅
- PLL倍频：80 ✅
- 系统时钟：160 MHz ✅
- 无HSE依赖 ✅

✅ **ADC配置**
- ADC1启用 ✅
- PA4引脚配置为ADC1_IN9 ✅
- 12位分辨率 ✅
- 采样时间：391.5 cycles ✅
- ADC中断启用（可选）✅

✅ **USART配置**
- USART1启用 ✅
- PA9/PA10引脚配置正确 ✅
- 波特率：115200 ✅
- 8N1配置 ✅

✅ **TIM1配置**
- TIM1启用 ✅
- 内部时钟源 ✅
- 预分频：8000 ✅
- 周期：200 ✅
- TIM1中断启用 ✅

✅ **GPIO配置**
- PA5（LED）配置为输出 ✅
- GPIO标签：LED_STATUS ✅

### 硬件连接验证

✅ **传感器连接**
- PulseSensor信号 → PA4（Arduino A2）✅
- VCC连接正确（3.3V/5V）✅
- GND连接正确 ✅

✅ **串口连接**
- USB连接到PC ✅
- 虚拟串口识别（COM?）✅
- 波特率匹配（115200）✅

---

## 🔗 相关资源

### 参考文档
- F103移植指南：[STM32F103C8T6移植与波形显示.md](file:///e:/PulseSensorPlayground-master/PulseSensorPlayground-master/stm32f103_pulse/stm32f103_pulse/STM32F103C8T6移植与波形显示.md)
- U545RE-Q用户手册：UM2841
- STM32U5参考手册：RM0456

### .ioc文件位置
```
e:\stm32u545\STM32CubeU5\Projects\NUCLEO-U545RE-Q\Examples\PulseSensor\PulseSensor.ioc
```

### HAL驱动库位置
```
e:\stm32u545\STM32CubeU5\Drivers\STM32U5xx_HAL_Driver\
```

---

## 🎯 下一步操作

1. **完成CubeMX配置**
   - 按本指南完成ADC配置
   - 验证时钟配置正确（使用MSI）
   - 生成代码

2. **移植PulseSensor代码**
   - 复制pulse_sensor.c/h文件
   - 修改HAL库头文件引用
   - 修改ADC通道和定时器配置

3. **编译和调试**
   - 编译项目（STM32CubeIDE）
   - 下载到NUCLEO-U545RE-Q开发板
   - 连接PulseSensor传感器
   - 测试串口数据输出

4. **波形显示**
   - 使用Python脚本显示波形
   - 或使用SerialPlot工具
   - 验证脉搏信号正确采集

---

## 📌 配置总结

### U545RE-Q配置要点

| 配置项 | U545RE-Q配置 | F103配置 | 差异说明 |
|-------|-------------|---------|---------|
| **时钟源** | MSI (4MHz) | HSE (8MHz) | ⚠️ 无HSE晶振 |
| **ADC引脚** | PA4 (ADC1_IN9) | PA0 (ADC1_IN0) | ⚠️ PA0不可用 |
| **系统时钟** | 160MHz | 72MHz | 性能更强 |
| **定时器** | TIM1 | TIM2 | 定时器资源不同 |
| **采样率** | 100Hz | 100Hz | ✅ 保持一致 |
| **串口** | USART1 (115200) | USART1 (115200) | ✅ 配置相同 |

---

## ✅ 配置完成确认

按照本指南完成CubeMX配置后，您的项目将具备：

✅ 完整的时钟系统（160MHz，无HSE依赖）
✅ ADC采集系统（PA4通道，100Hz采样）
✅ 串口通信系统（115200波特率，虚拟串口）
✅ 定时采样系统（TIM1定时器）
✅ LED指示系统（PA5）
✅ PulseSensor代码移植准备

**配置完成后，可以开始移植PulseSensor代码并测试！**