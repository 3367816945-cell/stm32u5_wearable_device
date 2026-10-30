##########################################################################################################################
# Makefile for MultiSensor_FatigueDetect on STM32U545RETxQ
# Target: ARM Cortex-M33 (STM32U545RETxQ)
# Toolchain: GNU Tools for STM32 (arm-none-eabi-gcc)
# Build: make all
# Clean: make clean
# Flash: make flash
##########################################################################################################################

# ===== Project Name =====
PROJECT = MultiSensor_FatigueDetect

# ===== Toolchain =====
TOOLCHAIN_PATH ?= C:/ST/STM32CubeIDE_1.19.0/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.13.3.rel1.win32_1.0.0.202411081344/tools/bin
PROGRAMMER_PATH ?= C:/ST/STM32CubeIDE_1.19.0/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.externaltools.cubeprogrammer.win32_2.2.200.202503041107/tools/bin

CC  = $(TOOLCHAIN_PATH)/arm-none-eabi-gcc
CXX = $(TOOLCHAIN_PATH)/arm-none-eabi-g++
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
CXXFLAGS = $(MCU_SPEC) $(FPU_FLAGS) $(MACRO_DEF) $(OPT) -Wall -Wno-unused-function -fdata-sections -ffunction-sections -g -gdwarf-2 -fno-exceptions -fno-rtti
CXXFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

# ===== Assembler Flags =====
ASFLAGS = $(MCU_SPEC) $(FPU_FLAGS) $(OPT) -Wall -fdata-sections -ffunction-sections -g -gdwarf-2 -x assembler-with-cpp

# ===== Linker Flags =====
LDFLAGS = $(MCU_SPEC) $(FPU_FLAGS) -T$(LDSCRIPT) -specs=nano.specs -u _printf_float -Wl,-Map=$(BUILD_DIR)/$(PROJECT).map -Wl,--gc-sections -lstdc++_nano -lm
LDFLAGS += Middlewares/ST/AI/Lib/NetworkRuntime1020_CM33_GCC.a
LDFLAGS += Middlewares/ST/touchgfx/lib/core/cortex_m33/gcc/libtouchgfx-float-abi-hard.a

# ===== Project Root Directory (Hardcoded for Windows) =====
TOPDIR = E:/stm32u545/STM32CubeU5/Projects/NUCLEO-U545RE-Q/Examples/MultiSensor_FatigueDetectAll

# ===== STM32Cube Firmware Package Path =====
STM32CUBE_FW = C:/Users/Lenovo/STM32Cube/Repository/STM32Cube_FW_U5_V1.9.0

# ===== TouchGFX Framework Path =====
TOUCHGFX_FW = Middlewares/ST/touchgfx/framework

# ===== Include Paths (Absolute Windows Paths) =====
INCLUDES  = -I$(TOPDIR)/Inc
INCLUDES += -I$(TOPDIR)/Inc/TFTh
INCLUDES += -I$(TOPDIR)/Src
INCLUDES += -I$(TOPDIR)/Src/gui/include
INCLUDES += -I$(TOPDIR)/Src/generated/gui_generated/include
INCLUDES += -I$(TOPDIR)/Src/generated/images/include
INCLUDES += -I$(TOPDIR)/Src/generated/fonts/include
INCLUDES += -I$(TOPDIR)/Src/generated/texts/include
INCLUDES += -I$(STM32CUBE_FW)/Drivers/STM32U5xx_HAL_Driver/Inc
INCLUDES += -I$(STM32CUBE_FW)/Drivers/STM32U5xx_HAL_Driver/Inc/Legacy
INCLUDES += -I$(STM32CUBE_FW)/Drivers/CMSIS/Device/ST/STM32U5xx/Include
INCLUDES += -I$(STM32CUBE_FW)/Drivers/CMSIS/Include
INCLUDES += -I$(TOPDIR)/Middlewares/ST/touchgfx/framework/include
INCLUDES += -I$(TOPDIR)/Middlewares/ST/AI/Inc

# ===== Source Files (User) =====
CORE_SOURCES = \
  Src/main.c \
  Src/stm32u5xx_it.c \
  Src/stm32u5xx_hal_msp.c \
  Src/system_stm32u5xx.c \
  Src/pulse_sensor.c \
  Src/mpu9250_driver.c \
  Src/mpu9250_debug.c \
  Src/kalman_filter.c \
  Src/sensor_fusion.c \
  Src/gait_detect.c \
  Src/imu_features.c \
  Src/hrv_analyzer.c \
  Src/syscalls.c \
  Src/sysmem.c \
  Src/TFTc/TFT_CAD.c \
  Src/TFTc/TFT_init.c \
  Src/TFTc/TFT_io.c \
  Src/imu_features_8d.c \
  Src/fatigue_predict.c \
  Src/fatigue_analysis.c \
  Src/fusion_rules.c \
  Src/networka.c \
  Src/networka_data.c \
  Src/networka_data_params.c \
  Src/app_touchgfx.c

# ===== Source Files (TouchGFX HAL C++) =====
CPP_SOURCES = \
  Src/TouchGFXGPIO.cpp \
  Src/OSWrappers.cpp \
  Src/STM32DMA.cpp \
  Src/STM32TouchController.cpp \
  Src/TouchGFXHAL.cpp \
  Src/TouchGFXGeneratedHAL.cpp \
  Src/TouchGFXConfiguration.cpp \
  Src/TouchGFX_DisplayDriver.cpp

# ===== TouchGFX Generated Files =====
GENERATED_SOURCES = \
  Src/generated/fonts/src/ApplicationFontProvider.cpp \
  Src/generated/fonts/src/CachedFont.cpp \
  Src/generated/fonts/src/CompressedFontCache.cpp \
  Src/generated/fonts/src/CompressedUnmappedFontCache.cpp \
  Src/generated/fonts/src/FontCache.cpp \
  Src/generated/fonts/src/GeneratedFont.cpp \
  Src/generated/fonts/src/UnmappedDataFont.cpp \
  Src/generated/fonts/src/VectorFontRendererBuffers.cpp \
  Src/generated/fonts/src/Font_verdana_10_4bpp_0.cpp \
  Src/generated/fonts/src/Font_verdana_20_4bpp_0.cpp \
  Src/generated/fonts/src/Font_timesbi_14_4bpp_0.cpp \
  Src/generated/fonts/src/Font_STKAITI_TTF_16_4bpp_0.cpp \
  Src/generated/fonts/src/Kerning_verdana_10_4bpp.cpp \
  Src/generated/fonts/src/Kerning_verdana_20_4bpp.cpp \
  Src/generated/fonts/src/Kerning_timesbi_14_4bpp.cpp \
  Src/generated/fonts/src/Kerning_STKAITI_TTF_16_4bpp.cpp \
  Src/generated/fonts/src/Table_verdana_10_4bpp.cpp \
  Src/generated/fonts/src/Table_verdana_20_4bpp.cpp \
  Src/generated/fonts/src/Table_timesbi_14_4bpp.cpp \
  Src/generated/fonts/src/Table_STKAITI_TTF_16_4bpp.cpp \
  Src/generated/images/src/BitmapDatabase.cpp \
  Src/generated/images/src/SVGDatabase.cpp \
  Src/generated/images/src/Icons/image_PrtSc20260708.cpp \
  Src/generated/images/src/__generated/image_alternate_theme_images_widgets_circleprogress_backgrounds_small.cpp \
  Src/generated/texts/src/LanguageGb.cpp \
  Src/generated/texts/src/Texts.cpp \
  Src/generated/texts/src/TypedTextDatabase.cpp \
  Src/generated/gui_generated/src/common/FrontendApplicationBase.cpp \
  Src/generated/gui_generated/src/containers/CustomContainer1Base.cpp \
  Src/generated/gui_generated/src/main_screen/MainViewBase.cpp \
  Src/generated/gui_generated/src/screen0_screen/screen0ViewBase.cpp \
  Src/generated/gui_generated/src/screen1_screen/Screen1ViewBase.cpp

# ===== TouchGFX GUI Files =====
GUI_SOURCES = \
  Src/gui/src/common/FrontendApplication.cpp \
  Src/gui/src/model/Model.cpp \
  Src/gui/src/main_screen/MainPresenter.cpp \
  Src/gui/src/main_screen/MainView.cpp \
  Src/gui/src/screen0_screen/screen0Presenter.cpp \
  Src/gui/src/screen0_screen/screen0View.cpp \
  Src/gui/src/screen1_screen/Screen1Presenter.cpp \
  Src/gui/src/screen1_screen/Screen1View.cpp \
  Src/gui/src/containers/CustomContainer1.cpp

# ===== TouchGFX Framework Source Files =====
TOUCHGFX_SOURCES = \
  $(TOUCHGFX_FW)/source/touchgfx/containers/Container.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/containers/CacheableContainer.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/containers/ListLayout.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/containers/ModalWindow.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/containers/ScrollableContainer.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/containers/SlideMenu.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/containers/Slider.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/containers/SwipeContainer.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/containers/ZoomAnimationImage.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/Box.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/BoxWithBorder.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/Button.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/ButtonWithLabel.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/ButtonWithIcon.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/Image.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/TextArea.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/TextAreaWithWildcard.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/TouchArea.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/AbstractButton.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/AnimatedImage.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/AnimationTextureMapper.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/Gauge.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/Keyboard.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/PixelDataWidget.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/QRCode.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/RadioButton.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/RepeatButton.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/ScalableImage.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/SnapshotWidget.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/TiledImage.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/ToggleButton.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/TextureMapper.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/SVGImage.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/canvas/Canvas.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/canvas/CanvasWidget.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/canvas/Circle.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/canvas/Line.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/canvas/PainterRGB565.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/canvas/PainterRGB565Bitmap.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/canvas/CWRVectorRenderer.cpp \
  $(TOUCHGFX_FW)/source/touchgfx/widgets/utils/qrcodegen.cpp

# ===== Source Files (HAL Drivers - only needed ones) =====
HAL_SOURCES = \
  $(STM32CUBE_FW)/Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal.c \
  $(STM32CUBE_FW)/Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_adc.c \
  $(STM32CUBE_FW)/Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_adc_ex.c \
  $(STM32CUBE_FW)/Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_cortex.c \
  $(STM32CUBE_FW)/Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_crc.c \
  $(STM32CUBE_FW)/Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_crc_ex.c \
  $(STM32CUBE_FW)/Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_dma.c \
  $(STM32CUBE_FW)/Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_dma_ex.c \
  $(STM32CUBE_FW)/Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_exti.c \
  $(STM32CUBE_FW)/Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_flash.c \
  $(STM32CUBE_FW)/Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_flash_ex.c \
  $(STM32CUBE_FW)/Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_gpio.c \
  $(STM32CUBE_FW)/Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_i2c.c \
  $(STM32CUBE_FW)/Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_i2c_ex.c \
  $(STM32CUBE_FW)/Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_pwr.c \
  $(STM32CUBE_FW)/Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_pwr_ex.c \
  $(STM32CUBE_FW)/Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_rcc.c \
  $(STM32CUBE_FW)/Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_rcc_ex.c \
  $(STM32CUBE_FW)/Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_spi.c \
  $(STM32CUBE_FW)/Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_spi_ex.c \
  $(STM32CUBE_FW)/Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_tim.c \
  $(STM32CUBE_FW)/Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_tim_ex.c \
  $(STM32CUBE_FW)/Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_uart.c \
  $(STM32CUBE_FW)/Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_uart_ex.c \
  $(STM32CUBE_FW)/Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_icache.c

# ===== Startup File =====
STARTUP_SOURCE = Startup/startup_stm32u545retxq.s

# ===== All Sources =====
SOURCES = $(CORE_SOURCES) $(HAL_SOURCES)
CPP_ALL = $(CPP_SOURCES) $(GENERATED_SOURCES) $(GUI_SOURCES) $(TOUCHGFX_SOURCES)
ASM_SOURCES = $(STARTUP_SOURCE)

# ===== Object Files =====
# Local C files - use basename with path prefix
OBJECTS_C_LOCAL = $(addprefix $(BUILD_DIR)/,$(notdir $(CORE_SOURCES:.c=.o)))
# HAL files - use hal_ prefix to avoid conflicts
OBJECTS_C_EXT = $(addprefix $(BUILD_DIR)/hal_,$(notdir $(HAL_SOURCES:.c=.o)))
OBJECTS_C = $(OBJECTS_C_LOCAL) $(OBJECTS_C_EXT)

# C++ files - use basename
OBJECTS_CPP = $(addprefix $(BUILD_DIR)/,$(notdir $(CPP_ALL:.cpp=.o)))
OBJECTS_ASM = $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
OBJECTS = $(OBJECTS_C) $(OBJECTS_CPP) $(OBJECTS_ASM)

# vpath for source files
vpath %.c $(sort $(dir $(SOURCES)))
vpath %.cpp $(sort $(dir $(CPP_ALL)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

# ===== Default Target =====
all: $(BUILD_DIR)/$(PROJECT).elf $(BUILD_DIR)/$(PROJECT).hex $(BUILD_DIR)/$(PROJECT).bin

# ===== Link =====
$(BUILD_DIR)/$(PROJECT).elf: $(OBJECTS) $(LDSCRIPT)
	@echo "=== Linking $@ ==="
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

# ===== Compile C =====
$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	@echo "  CC  $<"
	$(CC) -c $(CFLAGS) $(INCLUDES) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

# ===== Compile C (HAL with prefix) =====
$(BUILD_DIR)/hal_%.o: %.c Makefile | $(BUILD_DIR)
	@echo "  CC  $<"
	$(CC) -c $(CFLAGS) $(INCLUDES) -Wa,-a,-ad,-alms=$(BUILD_DIR)/hal_$(notdir $(<:.c=.lst)) $< -o $@

# ===== Compile C++ =====
$(BUILD_DIR)/%.o: %.cpp Makefile | $(BUILD_DIR)
	@echo "  CXX $<"
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) $< -o $@

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