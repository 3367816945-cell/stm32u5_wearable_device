/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "app_touchgfx.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include "pulse_sensor.h"
#include "mpu9250_driver.h"
#include "imu_features.h"
#include "gait_detect.h"
#include "sensor_fusion.h"
#include "TFTh/TFT_init.h"
#include "TFTh/TFT_io.h"
#include "TFTh/TFT_CAD.h"
#include "hrv_analyzer.h"
#include "imu_features_8d.h"
#include "fatigue_predict.h"
#include "fusion_rules.h"
#include "sensor_data.h"
#include "fatigue_analysis.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

CRC_HandleTypeDef hcrc;

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef handle_GPDMA1_Channel15;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim16;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
PulseSensor_t g_ps;  /* 改为全局变量,供中断访问 */
volatile uint16_t g_adc_value = 0;
volatile uint8_t g_sample_ready = 0;
volatile uint8_t g_heartbeat_ready = 0;  /* 心跳检测标志 */
volatile uint32_t g_tick_ms = 0;
extern volatile uint32_t tim1_irq_count;
extern volatile uint8_t adc_debug_ready;
extern volatile uint16_t adc_debug_value;
char uart_buf[100];
TFT_HandleTypeDef htft;  /* TFT显示屏句柄（TouchGFX显示驱动需要） */
static IMUFeatureBuffer_t imu_buf;
static AHRS_t ahrs;
static HRVAnalyzer_t g_hrv;
static IMU8DBuffer_t imu8d_buf;
// static FatiguePrediction_t g_pred;  /* 暂未使用 */
static FuseLevel_t g_fuse_level;
volatile uint32_t g_exti_dbg = 0;  /* EXTI中断调试计数器 */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void SystemPower_Config(void);
static void MX_GPIO_Init(void);
static void MX_GPDMA1_Init(void);
static void MX_ICACHE_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_CRC_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM16_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// 按键事件标志
volatile uint8_t g_buttonPressed = 0;
// 屏幕状态: 0=Main, 1=screen0, 2=Screen1
volatile uint8_t g_scrId = 0;
volatile uint8_t g_scrReq = 0;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the System Power */
  SystemPower_Config();

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_GPDMA1_Init();
  MX_ICACHE_Init();
  MX_USART1_UART_Init();
  MX_CRC_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_SPI1_Init();
  MX_TIM16_Init();
  MX_TouchGFX_Init();
  /* USER CODE BEGIN 2 */
  /* PulseSensor 初始化 */
  PulseSensor_Init(&g_ps);
  PulseSensor_SetThreshold(&g_ps, 450);  // 降低阈值匹配Signal范围400-620
  printf("PulseSensor Init OK\r\n");
  
  /* 输出PulseSensor初始值 */
  printf("=== PulseSensor Initial Values ===\r\n");
  printf("Threshold: %d\r\n", g_ps.thresh);
  printf("P (Peak): %d\r\n", g_ps.P);
  printf("T (Trough): %d\r\n", g_ps.T);
  printf("BPM: %d\r\n", g_ps.BPM);
  printf("IBI: %d\r\n", g_ps.IBI);
  printf("Amp: %d\r\n", g_ps.amp);
  printf("FirstBeat: %d, SecondBeat: %d\r\n", g_ps.firstBeat, g_ps.secondBeat);
  printf("===============================\r\n\r\n");
  
  /* MPU9250 初始化 */
  if (MPU9250_Init() == 0) {
    MPU9250_SetAccelRange(MPU9250_ACCEL_16G);
    MPU9250_SetGyroRange(MPU9250_GYRO_2000);
    MPU9250_InitBMP280();
    printf("MPU9250+BMP280 Ready\r\n");
  } else {
    printf("MPU9250 Init Failed\r\n");
    Error_Handler();
  }
  
  /* TFT屏幕初始化（TouchGFX显示需要） */
  printf("TFT Init...\r\n");
  TFT_Init_Instance(&htft, &hspi1, TFT_CS_GPIO_Port, TFT_CS_Pin);
  TFT_Config_Pins(&htft, TFT_DC_GPIO_Port, TFT_DC_Pin, TFT_RES_GPIO_Port, TFT_RES_Pin, TFT_BL_GPIO_Port, TFT_BL_Pin);
  TFT_IO_Init(&htft);
  TFT_Init_ST7735S(&htft);
  TFT_Fill_Area(&htft, 0, 0, 127, 127, BLACK);
  printf("TFT Init OK\r\n");

  /* 启动 TIM16 (VSync for TouchGFX) */
  HAL_TIM_Base_Start_IT(&htim16);
  printf("TIM16 VSync Started\r\n");

  /* 启动 TIM1 (500Hz 采样) */
  HAL_TIM_Base_Start_IT(&htim1);
  
  /* 启动 ADC */
  HAL_ADC_Start(&hadc1);
  
  /* IMU特征提取初始化 */
  IMUFeatures_Init(&imu_buf);
  printf("IMU Features Init OK\r\n");

  /* 3轴Kalman姿态初始化 */
  AHRS_Init(&ahrs);
  printf("AHRS Init OK\r\n");
  
  /* HRV分析器初始化 */
  HRV_Init(&g_hrv);
  printf("HRV Analyzer Init OK\r\n");
  
  /* 8维IMU特征提取初始化 */
  IMU8D_Init(&imu8d_buf);
  printf("IMU8D Features Init OK\r\n");
  
  /* Cube.AI推理初始化 */
  Fatigue_Init();

  /* 疲劳分析模块初始化 */
  FatigueAnalysis_Init();

  printf("=== All Init Complete ===\r\n");

#if (SERIAL_OUTPUT_MODE == SERIAL_OUTPUT_MODE_RAW)
  /* Processing Visualizer 表头（原始模式） */
  printf("TS,Sig,BPM,IBI\r\n");
#else
  /* 特征模式提示 */
  printf("\r\n=== MultiSensor Fatigue Detect (Features Mode) ===\r\n");
  printf("Output: 8D IMU Features + HRV Features + Prediction\r\n\r\n");
#endif
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    MX_TouchGFX_Process();

    /* PulseSensor串口输出和TouchGFX更新 */
    static uint16_t sample_count = 0;  // 降采样计数器

    /* Signal输出（降采样到50Hz,避免串口阻塞） */
    if (g_sample_ready) {
      g_sample_ready = 0;

      /* 每10次采样输出一次Signal数据（50Hz） */
      sample_count++;
      if (sample_count >= 10) {
        sample_count = 0;
        printf("S%d\n", g_ps.Signal);  /* 输出Signal数据 */
        sensorData.signal = (uint16_t)g_ps.Signal;  /* 保存原始波形值 */
        sensorData.dataReady = 1;  /* 触发TouchGFX更新 */
      }
    }

    /* 心跳输出（检测到心跳时） */
    if (g_heartbeat_ready) {
      g_heartbeat_ready = 0;
      sensorData.bpm = (uint16_t)g_ps.BPM;

      /* 输出BPM和IBI数据（官方Processing Visualizer格式）*/
      printf("B%d\n", g_ps.BPM);  /* 发送BPM数据 */
      printf("Q%d\n", g_ps.IBI);  /* 发送IBI数据 */

      /* HRV特征提取（暂时注释掉输出）*/
      uint32_t ts = HAL_GetTick();
      uint16_t ibi = (uint16_t)g_ps.IBI;
      HRV_AddSample(&g_hrv, ts, ibi);

      /* 达到50次心跳时计算HRV特征（暂时注释掉输出）*/
      if (HRV_IsWindowFull(&g_hrv)) {
        HRV_Compute(&g_hrv);
        g_hrv.features_ready = false;
      }
    }
    
    /* MPU9250数据输出 (每10ms, 100Hz采样) */
    static uint32_t mpu_tm = 0;
    static float vz = 0.0f;  /* z轴速度积分 */
    static float az_prev = 0.0f;  /* 前一次加速度 */
    static float g_ref = 0.0f;  /* 重力参考值（自动校准） */
    static uint8_t calib_cnt = 0;  /* 校准计数 */
    if (HAL_GetTick() - mpu_tm >= 10) {  // 10ms周期 = 100Hz采样
      mpu_tm = HAL_GetTick();
      float ax[3], gx[3], tp;
      MPU9250_GetAccel(ax);
      MPU9250_GetGyro(gx);
      MPU9250_GetTemp(&tp);

      /* 前5秒自动校准重力参考值 */
      if (calib_cnt < 50) {
        g_ref += ax[2];  /* 累积静止加速度 */
        calib_cnt++;
      }
      /* 校准完成后计算平均值 */
      float g_display = (calib_cnt > 0) ? (g_ref / (float)calib_cnt) : 0.0f;

      /* 计算净加速度（去除实测重力） */
      float az_net = ax[2] - g_display;

      /* 零速检测：加速度变化<0.05g时认为静止，速度清零 */
      float az_delta = ax[2] - az_prev;
      if (fabsf(az_delta) < 0.05f && fabsf(az_net) < 0.05f) {
        vz = 0.0f;  /* 零速修正 */
      } else {
        vz += az_net * 0.1f;  /* dt=100ms=0.1s */
      }
      az_prev = ax[2];

      /* 步态检测 */
      MPU9250_GaitDetect(az_net, mpu_tm);

      /* 3轴Kalman姿态解算 */
      AHRS_Update(&ahrs, ax, gx, 0.1f);

      /* IMU特征提取 */
      IMUFeatures_Feed(&imu_buf, ax, gx);
      IMUFeatures_FeedGait(&imu_buf, MPU9250_GetStrideTime(),
                           MPU9250_GetCVStrideTime(),
                           MPU9250_GetCadence(),
                           AHRS_GetEversion(&ahrs));

      /* 8维IMU特征提取（Network A）*/
      IMU8D_Feed(&imu8d_buf, ax, gx);
      
      /* 每5秒提取特征并推理 */
      if (imu8d_buf.sample_count >= IMU8D_WINDOW_SIZE) {
        IMU8D_Extract(&imu8d_buf);
        IMU8D_Normalize(&imu8d_buf);
        
        /* Cube.AI推理 */
        Fatigue_SetIMUFeatures(imu8d_buf.features.features);
        FatiguePrediction_t *pred = Fatigue_Predict();
        
        if (pred) {
          /* 双通道融合（通道A=IMU,通道B=HRV）*/
          bool hrv_warn = HRV_CheckWarning(&g_hrv.features);
          g_fuse_level = Fatigue_Fuse(pred->level, hrv_warn);
          
          /* IMU特征输出已注释（调试心率传感器）*/
          // printf("\r\n=== 8D Features ===\r\n");
          // printf("F0_peak_acc_z: %.3f\r\n", imu8d_buf.features.features[0]);
          // ... 其他特征输出已注释

          /* 更新传感器数据结构 */
          if (g_hrv.features.MeanRR > 0) {
            sensorData.bpm = (uint16_t)(60000.0f / g_hrv.features.MeanRR);
          } else {
            sensorData.bpm = 0;
          }
          sensorData.fatigue_level = g_fuse_level;
          int conf = (int)(pred->probs[g_fuse_level] * 100.0f);
          sensorData.confidence = (uint8_t)((conf < 0) ? 0 : ((conf > 100) ? 100 : conf));
          sensorData.dataReady = 1;
        }

        /* 重置缓冲区 */
        imu8d_buf.sample_count = 0;
        imu8d_buf.buf_idx = 0;
      }

      /* IMU原始数据输出已注释（调试心率传感器）*/
      // int16_t az_int = (int16_t)(ax[2] * 100);
      // int16_t vz_int = (int16_t)(vz * 100);
      // int16_t g_int = (int16_t)(g_display * 100);
      // printf("A:%d.%02d V:%d.%02d G:%d.%02d\r\n", ...);
    }

    /* 按键处理：三屏循环切换 Main→screen0→Screen1→Main */
    if (g_buttonPressed) {
      g_buttonPressed = 0;
      g_scrId = (g_scrId + 1) % 3;
      g_scrReq = 1;
      if (g_scrId == 1) {
        /* 进入screen0: 运动开始，停止分析 */
        FatigueAnalysis_Init();
        printf("\r\n[BTN] -> screen0\r\n");
      } else if (g_scrId == 2) {
        /* 进入Screen1: 运动结束，开始30s疲劳分析 */
        FatigueAnalysis_Start();
        printf("\r\n[BTN] -> Screen1, Fatigue Start\r\n");
      } else {
        /* 进入Main: 停止分析 */
        FatigueAnalysis_Init();
        printf("\r\n[BTN] -> Main\r\n");
      }
    }

    /* 调试：每秒打印EXTI中断计数 */
    { static uint32_t dbg_tm = 0; static uint32_t last_exti = 0;
    if (HAL_GetTick() - dbg_tm >= 1000) {
      dbg_tm = HAL_GetTick();
      if (g_exti_dbg != last_exti) {
        printf("[DBG] EXTI=%lu btn=%u\r\n", g_exti_dbg, g_buttonPressed);
        last_exti = g_exti_dbg;
      }
    }}
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMBOOST = RCC_PLLMBOOST_DIV1;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 20;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLLVCIRANGE_1;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Power Configuration
  * @retval None
  */
static void SystemPower_Config(void)
{

  /*
   * Switch to SMPS regulator instead of LDO
   */
  if (HAL_PWREx_ConfigSupply(PWR_SMPS_SUPPLY) != HAL_OK)
  {
    Error_Handler();
  }
/* USER CODE BEGIN PWR */
/* USER CODE END PWR */
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.GainCompensation = 0;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_5CYCLE;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief GPDMA1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPDMA1_Init(void)
{

  /* USER CODE BEGIN GPDMA1_Init 0 */

  /* USER CODE END GPDMA1_Init 0 */

  /* Peripheral clock enable */
  __HAL_RCC_GPDMA1_CLK_ENABLE();

  /* GPDMA1 interrupt Init */
    HAL_NVIC_SetPriority(GPDMA1_Channel15_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(GPDMA1_Channel15_IRQn);

  /* USER CODE BEGIN GPDMA1_Init 1 */

  /* USER CODE END GPDMA1_Init 1 */
  /* USER CODE BEGIN GPDMA1_Init 2 */

  /* USER CODE END GPDMA1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10F02A89;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief ICACHE Initialization Function
  * @param None
  * @retval None
  */
static void MX_ICACHE_Init(void)
{

  /* USER CODE BEGIN ICACHE_Init 0 */

  /* USER CODE END ICACHE_Init 0 */

  /* USER CODE BEGIN ICACHE_Init 1 */

  /* USER CODE END ICACHE_Init 1 */

  /** Enable instruction cache in 1-way (direct mapped cache)
  */
  if (HAL_ICACHE_ConfigAssociativityMode(ICACHE_1WAY) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_ICACHE_Enable() != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ICACHE_Init 2 */

  /* USER CODE END ICACHE_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  SPI_AutonomousModeConfTypeDef HAL_SPI_AutonomousMode_Cfg_Struct = {0};

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 0x7;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  hspi1.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi1.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi1.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi1.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi1.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi1.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi1.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  hspi1.Init.ReadyMasterManagement = SPI_RDY_MASTER_MANAGEMENT_INTERNALLY;
  hspi1.Init.ReadyPolarity = SPI_RDY_POLARITY_HIGH;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_SPI_AutonomousMode_Cfg_Struct.TriggerState = SPI_AUTO_MODE_DISABLE;
  HAL_SPI_AutonomousMode_Cfg_Struct.TriggerSelection = SPI_GRP1_GPDMA_CH0_TCF_TRG;
  HAL_SPI_AutonomousMode_Cfg_Struct.TriggerPolarity = SPI_TRIG_POLARITY_RISING;
  if (HAL_SPIEx_SetConfigAutonomousMode(&hspi1, &HAL_SPI_AutonomousMode_Cfg_Struct) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 3200;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 99;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */
  printf("TIM1 Init: PSC=%lu, Period=%lu\r\n", htim1.Init.Prescaler, htim1.Init.Period);
  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM16 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM16_Init(void)
{

  /* USER CODE BEGIN TIM16_Init 0 */

  /* USER CODE END TIM16_Init 0 */

  /* USER CODE BEGIN TIM16_Init 1 */

  /* USER CODE END TIM16_Init 1 */
  htim16.Instance = TIM16;
  htim16.Init.Prescaler = 16000-1;
  htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim16.Init.Period = 16;
  htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim16.Init.RepetitionCounter = 0;
  htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim16) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM16_Init 2 */

  /* USER CODE END TIM16_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, TFT_CS_Pin|TFT_DC_Pin|TFT_RES_Pin|TFT_BL_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : user_but_Pin */
  GPIO_InitStruct.Pin = user_but_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(user_but_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : TFT_CS_Pin TFT_DC_Pin TFT_RES_Pin TFT_BL_Pin */
  GPIO_InitStruct.Pin = TFT_CS_Pin|TFT_DC_Pin|TFT_RES_Pin|TFT_BL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI13_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI13_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
// TouchGFX VSync信号 - 由TIM16中断触发
extern void touchgfxSignalVSync(void);

/**
  * @brief  GPIO外部中断回调函数
  * @param  GPIO_Pin: 触发中断的GPIO引脚
  */
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
    g_exti_dbg++;  /* 调试：任何EXTI中断都会计数 */
    if (GPIO_Pin == GPIO_PIN_13) {  // User Button (PC13)
        g_buttonPressed = 1;  // 设置按键标志
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM16) {
        touchgfxSignalVSync();
    }
    // TIM1中断处理已在stm32u5xx_it.c的TIM1_UP_IRQHandler中实现
    // 包括ADC采样、PulseSensor_ProcessSample和g_sample_ready设置
}

/**
  * @brief  printf 重定向到 USART1
  * @param  ch: 字符
  * @retval 字符
  */
int __io_putchar(int ch)
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

int fputc(int ch, FILE *f)
{
  return __io_putchar(ch);
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param None
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
