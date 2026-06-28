# STM32U545RE-Q PulseSensor 项目

## 编译说明

由于STM32CubeU5固件包的特殊目录结构，本项目使用STM32CubeIDE GUI编译，而不是make命令。

### 方法一：STM32CubeIDE GUI编译（推荐）

```powershell
# 步骤1：打开STM32CubeIDE
C:\ST\STM32CubeIDE_1.19.0\STM32CubeIDE\stm32cubeide.exe

# 步骤2：导入项目
File → Import → Existing Projects into Workspace
选择目录：e:\stm32u545\STM32CubeU5\Projects\NUCLEO-U545RE-Q\Examples\PulseSensor

# 步骤3：编译项目
Project → Build Project (Ctrl+B)
```

### 方法二：命令行编译（STM32CubeIDE headless build）

```powershell
# 设置工作空间
$workspace = "e:\stm32u545\STM32CubeU5\Projects\NUCLEO-U545RE-Q\Examples"

# 编译
cd C:\ST\STM32CubeIDE_1.19.0\STM32CubeIDE
.\headless-build.bat -data $workspace -build PulseSensor
```

## 烧录说明

使用flash.ps1脚本烧录（通过ST-Link V3）：

```powershell
cd e:\stm32u545\STM32CubeU5\Projects\NUCLEO-U545RE-Q\Examples\PulseSensor
.\flash.ps1
```

## 文件结构

```
PulseSensor/
├── Src/
│   ├── main.c               # 主程序（已集成PulseSensor）
│   ├── pulse_sensor.c       # 心跳检测算法
│   ├── stm32u5xx_it.c       # 中断处理
│   └── stm32u5xx_hal_msp.c  # HAL MSP初始化
├── Inc/
│   ├── main.h               # 主程序头文件
│   ├── pulse_sensor.h       # 心跳检测算法头文件
│   └── stm32u5xx_hal_conf.h # HAL配置
├── PulseSensor.ioc          # CubeMX配置文件
├── build.ps1                # 构建脚本（参考）
├── flash.ps1                # 烧录脚本（实际使用）
└── STM32U545RETXQ_FLASH.ld  # 链接脚本
```

## 硬件连接

PulseSensor传感器连接：
- 信号输出 → Arduino A2接口（PA4）
- VCC → 3.3V
- GND → GND

## 串口数据格式

波特率：115200, 8N1

输出格式：
```
2048       # 波形数据（ADC值，每2ms）
2050
BPM: 72, IBI: 833ms  # 心跳数据
2055
...
```

## 参考

- F103项目：E:\PulseSensorPlayground-master\PulseSensorPlayground-master\stm32f103_pulse\stm32f103_pulse\README.md
- STM32CubeU5固件包：e:\stm32u545\STM32CubeU5