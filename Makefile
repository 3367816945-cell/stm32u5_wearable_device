##########################################################################################################################
# Makefile for PulseSensor on STM32U545RETxQ
# Target: ARM Cortex-M33 (STM32U545RETxQ)
# Toolchain: GNU Tools for STM32 (arm-none-eabi-gcc)
# Build: make all
# Clean: make clean
# Flash: make flash
##########################################################################################################################

# ===== Project Name =====
PROJECT = PulseSensor

# ===== Toolchain =====
TOOLCHAIN_PATH ?= C:/ST/STM32CubeIDE_1.19.0/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.13.3.rel1.win32_1.0.0.202411081344/tools/bin
PROGRAMMER_PATH ?= C:/ST/STM32CubeIDE_1.19.0/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.externaltools.cubeprogrammer.win32_2.2.200.202503041107/tools/bin

CC  = $(TOOLCHAIN_PATH)/arm-none-eabi-gcc
AS  = $(TOOLCHAIN_PATH)/arm-none-eabi-gcc -x assembler-with-cpp
CP  = $(TOOLCHAIN_PATH)/arm-none-eabi-objcopy
SZ  = $(TOOLCHAIN_PATH)/arm-none-eabi-size
HEX = $(CP) -O ihex
BIN = $(CP) -O binary
STM32_PROG = $(PROGRAMMER_PATH)/STM32_Programmer_CLI

# ===== Build Output Directory =====
BUILD_DIR = Debug

# ===== MCU Settings =====
MCU_SPEC  = -mcpu=cortex-m33 -mthumb
FPU_FLAGS = -mfloat-abi=hard -mfpu=fpv5-sp-d16
MACRO_DEF = -DUSE_HAL_DRIVER -DSTM32U545xx

# ===== Linker Script =====
LDSCRIPT = STM32U545RETXQ_FLASH.ld

# ===== Compiler Flags =====
OPT = -Og
CFLAGS  = $(MCU_SPEC) $(FPU_FLAGS) $(MACRO_DEF) $(OPT) -Wall -Wno-unused-function -fdata-sections -ffunction-sections -g -gdwarf-2
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

# ===== Assembler Flags =====
ASFLAGS = $(MCU_SPEC) $(FPU_FLAGS) $(OPT) -Wall -fdata-sections -ffunction-sections -g -gdwarf-2 -x assembler-with-cpp

# ===== Linker Flags =====
LDFLAGS = $(MCU_SPEC) $(FPU_FLAGS) -T$(LDSCRIPT) -specs=nano.specs -Wl,-Map=$(BUILD_DIR)/$(PROJECT).map -Wl,--gc-sections

# ===== Include Paths =====
INCLUDES  = -IInc
INCLUDES += -IDrivers/STM32U5xx_HAL_Driver/Inc
INCLUDES += -IDrivers/STM32U5xx_HAL_Driver/Inc/Legacy
INCLUDES += -IDrivers/CMSIS/Device/ST/STM32U5xx/Include
INCLUDES += -IDrivers/CMSIS/Include

# ===== Source Files (User) =====
CORE_SOURCES = \
  Src/main.c \
  Src/stm32u5xx_it.c \
  Src/stm32u5xx_hal_msp.c \
  Src/system_stm32u5xx.c \
  Src/pulse_sensor.c \
  Src/syscalls.c \
  Src/sysmem.c

# ===== Source Files (HAL Drivers - only needed ones) =====
HAL_SOURCES = \
  Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal.c \
  Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_adc.c \
  Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_adc_ex.c \
  Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_cortex.c \
  Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_dma.c \
  Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_dma_ex.c \
  Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_exti.c \
  Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_flash.c \
  Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_flash_ex.c \
  Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_gpio.c \
  Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_pwr.c \
  Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_pwr_ex.c \
  Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_rcc.c \
  Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_rcc_ex.c \
  Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_tim.c \
  Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_tim_ex.c \
  Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_uart.c \
  Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_uart_ex.c \
  Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_icache.c

# ===== Startup File =====
STARTUP_SOURCE = Startup/startup_stm32u545retxq.s

# ===== All Sources =====
SOURCES = $(CORE_SOURCES) $(HAL_SOURCES)
ASM_SOURCES = $(STARTUP_SOURCE)

# ===== Object Files =====
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.c $(sort $(dir $(SOURCES)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

# ===== Default Target =====
all: $(BUILD_DIR)/$(PROJECT).elf $(BUILD_DIR)/$(PROJECT).hex $(BUILD_DIR)/$(PROJECT).bin

# ===== Link =====
$(BUILD_DIR)/$(PROJECT).elf: $(OBJECTS) $(LDSCRIPT)
	@echo "=== Linking $@ ==="
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

# ===== Compile C =====
$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	@echo "  CC  $<"
	$(CC) -c $(CFLAGS) $(INCLUDES) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

# ===== Compile Assembly =====
$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	@echo "  AS  $<"
	$(AS) -c $(ASFLAGS) $< -o $@

# ===== Create Build Directory =====
$(BUILD_DIR):
	mkdir $(BUILD_DIR)

# ===== Generate HEX =====
$(BUILD_DIR)/$(PROJECT).hex: $(BUILD_DIR)/$(PROJECT).elf
	$(HEX) $< $@

# ===== Generate BIN =====
$(BUILD_DIR)/$(PROJECT).bin: $(BUILD_DIR)/$(PROJECT).elf
	$(BIN) $< $@

# ===== Clean =====
clean:
	@echo "=== Cleaning ==="
	-rm -fR $(BUILD_DIR)

# ===== Flash (STM32CubeProgrammer CLI) =====
flash: $(BUILD_DIR)/$(PROJECT).elf
	@echo "=== Flashing $< ==="
	$(STM32_PROG) -c port=SWD -w $< -v -rst

# ===== Show used size =====
size: $(BUILD_DIR)/$(PROJECT).elf
	$(SZ) $<

# ===== Include dependencies =====
-include $(wildcard $(BUILD_DIR)/*.d)

# ===== Phony Targets =====
.PHONY: all clean flash size