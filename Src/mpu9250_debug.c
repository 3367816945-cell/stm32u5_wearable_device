/*
 * mpu9250_debug.c
 * MPU9250调试/诊断工具实现
 */

#include "mpu9250_debug.h"
#include "mpu9250_driver.h"
#include <stdio.h>
#include <string.h>

extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart1;

static void Debug_Print(const char *msg) {
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);
}

void I2C_ScanBus(void)
{
    uint8_t i;
    HAL_StatusTypeDef status;
    char debug_msg[100];
    uint8_t devices_found = 0;

    Debug_Print("\r\n=== I2C Bus Scan Start ===\r\n");
    Debug_Print("Scanning addresses 0x00-0xFE (8-bit)...\r\n\r\n");

    for (i = 0; i < 255; i++) {
        if (i % 2 == 0) {  // 只扫描写地址（偶数）
            status = HAL_I2C_IsDeviceReady(&hi2c1, i, 1, 10);
            if (status == HAL_OK) {
                sprintf(debug_msg, "Device found at address: 0x%02X (8-bit write)\r\n", i);
                Debug_Print(debug_msg);
                devices_found++;
            }
        }
    }

    if (devices_found == 0) {
        Debug_Print("\r\nERROR: No I2C devices found!\r\n");
        Debug_Print("Possible causes:\r\n");
        Debug_Print("1. I2C wiring incorrect (PB3/PB6)\r\n");
        Debug_Print("2. Devices not powered\r\n");
        Debug_Print("3. I2C bus stuck (check pull-up resistors)\r\n");
        Debug_Print("4. I2C peripheral not initialized\r\n");
    } else {
        sprintf(debug_msg, "\r\nTotal devices found: %d\r\n", devices_found);
        Debug_Print(debug_msg);

        Debug_Print("\r\nChecking MPU9250 at 0xD0 (8-bit)...\r\n");
    status = HAL_I2C_IsDeviceReady(&hi2c1, 0xD0, 1, 10);
    if (status == HAL_OK) {
        Debug_Print("  MPU9250 detected at 0xD0 (AD0=GND)\r\n");
    } else {
        Debug_Print("  MPU9250 NOT found at 0xD0\r\n");
        Debug_Print("  Checking alternate address 0xD2...\r\n");
        status = HAL_I2C_IsDeviceReady(&hi2c1, 0xD2, 1, 10);
        if (status == HAL_OK) {
            Debug_Print("  MPU9250 detected at 0xD2 (AD0=VCC)\r\n");
            } else {
                Debug_Print("  MPU9250 NOT found at 0x69\r\n");
            }
        }
    }

    Debug_Print("\r\n=== I2C Bus Scan End ===\r\n\r\n");
}

void MPU9250_Debug_Init(void)
{
    uint8_t data;
    HAL_StatusTypeDef status;
    char debug_msg[100];

    Debug_Print("\r\n=== MPU9250 Debug Init Start ===\r\n");

    sprintf(debug_msg, "I2C Address: 0x%02X (8-bit write)\r\n", MPU9250_ADDRESS_WRITE);
    Debug_Print(debug_msg);

    Debug_Print("Step 1: Reset MPU9250...\r\n");
    data = 0x80;
    status = HAL_I2C_Mem_Write(&hi2c1, MPU9250_ADDRESS_WRITE, PWR_MGMT_1, 1, &data, 1, 1000);
    sprintf(debug_msg, "  HAL_I2C_Mem_Write Status: %d\r\n", status);
    Debug_Print(debug_msg);

    if (status == HAL_OK) {
        Debug_Print("  Reset command sent OK\r\n");
        HAL_Delay(100);

        Debug_Print("Step 2: Wake up and set clock...\r\n");
        data = 0x01;
        status = HAL_I2C_Mem_Write(&hi2c1, MPU9250_ADDRESS_WRITE, PWR_MGMT_1, 1, &data, 1, 1000);
        sprintf(debug_msg, "  HAL_I2C_Mem_Write Status: %d\r\n", status);
        Debug_Print(debug_msg);

        if (status == HAL_OK) {
            Debug_Print("  Wake up OK\r\n");
            HAL_Delay(100);

            Debug_Print("Step 3: Read WHO_AM_I register...\r\n");
            uint8_t who_am_i = 0;
            status = HAL_I2C_Mem_Read(&hi2c1, MPU9250_ADDRESS_WRITE, WHO_AM_I_REG, 1, &who_am_i, 1, 1000);
            sprintf(debug_msg, "  HAL_I2C_Mem_Read Status: %d\r\n", status);
            Debug_Print(debug_msg);
            sprintf(debug_msg, "  WHO_AM_I Value: 0x%02X\r\n", who_am_i);
            Debug_Print(debug_msg);

            if (who_am_i == 0x71 || who_am_i == 0x73) {
                Debug_Print("  WHO_AM_I Check PASSED!\r\n");

                Debug_Print("Step 4: Configure Accel +/-16g...\r\n");
                data = MPU9250_ACCEL_FS_16G;
                status = HAL_I2C_Mem_Write(&hi2c1, MPU9250_ADDRESS_WRITE, ACCEL_CONFIG, 1, &data, 1, 1000);
                sprintf(debug_msg, "  HAL_I2C_Mem_Write Status: %d\r\n", status);
                Debug_Print(debug_msg);

                if (status == HAL_OK) {
                    Debug_Print("  Accel config OK\r\n");

                    Debug_Print("Step 5: Configure Gyro +/-2000dps...\r\n");
                    data = MPU9250_GYRO_FS_2000DPS;
                    status = HAL_I2C_Mem_Write(&hi2c1, MPU9250_ADDRESS_WRITE, GYRO_CONFIG, 1, &data, 1, 1000);
                    sprintf(debug_msg, "  HAL_I2C_Mem_Write Status: %d\r\n", status);
                    Debug_Print(debug_msg);

                    if (status == HAL_OK) {
                        Debug_Print("  Gyro config OK\r\n");
                        Debug_Print("\r\n=== MPU9250 Init SUCCESS ===\r\n");
                    } else {
                        Debug_Print("  ERROR: Gyro config failed\r\n");
                    }
                } else {
                    Debug_Print("  ERROR: Accel config failed\r\n");
                }
            } else {
                Debug_Print("  ERROR: WHO_AM_I value incorrect\r\n");
                Debug_Print("  Expected: 0x71 or 0x73\r\n");
            }
        } else {
            Debug_Print("  ERROR: Wake up failed\r\n");
        }
    } else {
        Debug_Print("  ERROR: Reset failed\r\n");
        Debug_Print("  Possible causes:\r\n");
        Debug_Print("  1. I2C wiring incorrect (PB3/PB6)\r\n");
        Debug_Print("  2. MPU9250 not powered (3.3V/GND)\r\n");
        Debug_Print("  3. AD0 pin not grounded (address should be 0x68)\r\n");
        Debug_Print("  4. MPU9250 module damaged\r\n");
    }

    Debug_Print("\r\n=== Debug Init End ===\r\n\r\n");
}

void I2C_Bus_Diagnosis(void)
{
    char debug_msg[100];

    Debug_Print("\r\n=== I2C Bus Deep Diagnosis ===\r\n");

    if (__HAL_I2C_GET_FLAG(&hi2c1, I2C_FLAG_BUSY)) {
        Debug_Print("ERROR: I2C bus is BUSY (SCL or SDA held low)\r\n");
        Debug_Print("  This indicates a device is holding the bus\r\n");
        Debug_Print("  Solution: Power cycle all devices or send I2C bus reset\r\n");
    } else {
        Debug_Print("OK: I2C bus is FREE (ready for communication)\r\n");
    }

    GPIO_PinState scl_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6);
    GPIO_PinState sda_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7);

    sprintf(debug_msg, "  SCL (PB6) state: %s\r\n", scl_state == GPIO_PIN_SET ? "HIGH" : "LOW");
    Debug_Print(debug_msg);
    sprintf(debug_msg, "  SDA (PB7) state: %s\r\n", sda_state == GPIO_PIN_SET ? "HIGH" : "LOW");
    Debug_Print(debug_msg);

    if (scl_state == GPIO_PIN_RESET || sda_state == GPIO_PIN_RESET) {
        Debug_Print("  WARNING: Pin held LOW (need pull-up resistor)\r\n");
        Debug_Print("  Expected: Both SCL and SDA should be HIGH when idle\r\n");
        Debug_Print("  Check: Pull-up resistor 4.7kohm from each pin to VCC\r\n");
    } else {
        Debug_Print("  OK: Pull-up resistors working (both pins HIGH)\r\n");
    }

    Debug_Print("I2C Speed: 400 kHz (Fast Mode)\r\n");

    Debug_Print("\r\n=== I2C Bus Diagnosis End ===\r\n\r\n");
}

void MPU9250_Register_Dump(void)
{
    uint8_t reg_value;
    char debug_msg[100];

    Debug_Print("\r\n=== MPU9250 Register Dump ===\r\n");

    HAL_I2C_Mem_Read(&hi2c1, MPU9250_ADDRESS_WRITE, WHO_AM_I_REG, 1, &reg_value, 1, 1000);
    sprintf(debug_msg, "WHO_AM_I (0x75): 0x%02X (expect 0x71 or 0x73)\r\n", reg_value);
    Debug_Print(debug_msg);
    if (reg_value == 0x71) Debug_Print("  -> MPU9250 detected\r\n");
    else if (reg_value == 0x73) Debug_Print("  -> MPU9255 detected\r\n");
    else Debug_Print("  -> ERROR: Unknown device\r\n");

    HAL_I2C_Mem_Read(&hi2c1, MPU9250_ADDRESS_WRITE, PWR_MGMT_1, 1, &reg_value, 1, 1000);
    sprintf(debug_msg, "PWR_MGMT_1 (0x6B): 0x%02X\r\n", reg_value);
    Debug_Print(debug_msg);
    if (reg_value & 0x80) Debug_Print("  -> Device in RESET state\r\n");
    else if (reg_value & 0x40) Debug_Print("  -> Device in SLEEP mode\r\n");
    else Debug_Print("  -> Device ACTIVE\r\n");

    HAL_I2C_Mem_Read(&hi2c1, MPU9250_ADDRESS_WRITE, PWR_MGMT_2, 1, &reg_value, 1, 1000);
    sprintf(debug_msg, "PWR_MGMT_2 (0x6C): 0x%02X\r\n", reg_value);
    Debug_Print(debug_msg);
    if (reg_value == 0x00) Debug_Print("  -> All sensors enabled\r\n");
    else Debug_Print("  -> Some sensors disabled\r\n");

    HAL_I2C_Mem_Read(&hi2c1, MPU9250_ADDRESS_WRITE, INT_PIN_CFG, 1, &reg_value, 1, 1000);
    sprintf(debug_msg, "INT_PIN_CFG (0x37): 0x%02X\r\n", reg_value);
    Debug_Print(debug_msg);
    if (reg_value & 0x02) Debug_Print("  -> Bypass mode enabled (good)\r\n");
    else Debug_Print("  -> Bypass mode disabled (may cause I2C conflict)\r\n");

    HAL_I2C_Mem_Read(&hi2c1, MPU9250_ADDRESS_WRITE, ACCEL_CONFIG, 1, &reg_value, 1, 1000);
    sprintf(debug_msg, "ACCEL_CONFIG (0x1C): 0x%02X\r\n", reg_value);
    Debug_Print(debug_msg);
    uint8_t accel_range = (reg_value >> 3) & 0x03;
    sprintf(debug_msg, "  -> Accel range: +/-%d g\r\n", 2 << accel_range);
    Debug_Print(debug_msg);

    HAL_I2C_Mem_Read(&hi2c1, MPU9250_ADDRESS_WRITE, GYRO_CONFIG, 1, &reg_value, 1, 1000);
    sprintf(debug_msg, "GYRO_CONFIG (0x1B): 0x%02X\r\n", reg_value);
    Debug_Print(debug_msg);
    uint8_t gyro_range = (reg_value >> 3) & 0x03;
    sprintf(debug_msg, "  -> Gyro range: +/-%d deg/s\r\n", 250 << gyro_range);
    Debug_Print(debug_msg);

    Debug_Print("\r\n=== Register Dump End ===\r\n\r\n");
}

const char* HAL_Status_To_String(HAL_StatusTypeDef status)
{
    switch (status) {
        case HAL_OK:      return "HAL_OK (Success)";
        case HAL_ERROR:   return "HAL_ERROR (I2C Error)";
        case HAL_BUSY:    return "HAL_BUSY (Bus Stuck)";
        case HAL_TIMEOUT: return "HAL_TIMEOUT (No Response)";
        default:          return "UNKNOWN";
    }
}

void Print_I2C_Error_Analysis(HAL_StatusTypeDef status)
{
    char debug_msg[100];

    sprintf(debug_msg, "  HAL Status: %s\r\n", HAL_Status_To_String(status));
    Debug_Print(debug_msg);

    if (status == HAL_OK) {
        Debug_Print("  Action: Success, proceed to next step\r\n");
    } else if (status == HAL_BUSY) {
        Debug_Print("  ERROR: I2C bus is BUSY (SCL/SDA held low by device)\r\n");
        Debug_Print("  Possible causes:\r\n");
        Debug_Print("    1. Previous I2C transaction incomplete (device holding bus)\r\n");
        Debug_Print("    2. Pull-up resistors missing or too weak\r\n");
        Debug_Print("    3. I2C speed too high for wiring length\r\n");
        Debug_Print("  Solutions:\r\n");
        Debug_Print("    A. Power cycle MPU9250 module (disconnect VCC, wait 1s)\r\n");
        Debug_Print("    B. Check pull-up resistors (need 4.7kohm on SCL/SDA)\r\n");
        Debug_Print("    C. Reduce I2C speed from 400kHz to 100kHz\r\n");
        Debug_Print("    D. Add I2C bus reset code (toggle SCL 9 times)\r\n");
    } else if (status == HAL_ERROR) {
        Debug_Print("  ERROR: I2C communication error\r\n");
        Debug_Print("  Possible causes:\r\n");
        Debug_Print("    1. Device address incorrect (check AD0 pin)\r\n");
        Debug_Print("    2. I2C wiring incorrect (SCL/SDA swapped)\r\n");
        Debug_Print("    3. Device not powered (check VCC/GND)\r\n");
        Debug_Print("  Solutions:\r\n");
        Debug_Print("    A. Run I2C_Bus_Diagnosis() to check bus status\r\n");
        Debug_Print("    B. Check AD0 pin connection (should be GND for 0x68)\r\n");
        Debug_Print("    C. Verify VCC=3.3V and GND connected\r\n");
    } else if (status == HAL_TIMEOUT) {
        Debug_Print("  ERROR: I2C timeout (no ACK from device)\r\n");
        Debug_Print("  Possible causes:\r\n");
        Debug_Print("    1. Device not present on bus\r\n");
        Debug_Print("    2. Device address mismatch\r\n");
        Debug_Print("    3. Device in sleep/deep sleep mode\r\n");
        Debug_Print("  Solutions:\r\n");
        Debug_Print("    A. Run I2C_ScanBus() to find device address\r\n");
        Debug_Print("    B. Send wake-up command (PWR_MGMT_1 = 0x00)\r\n");
        Debug_Print("    C. Increase timeout value (try 2000ms instead of 1000ms)\r\n");
    }
}