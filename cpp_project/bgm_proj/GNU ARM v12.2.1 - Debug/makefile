################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include simplicity_sdk_2024.12.2/platform/service/udelay/src/subdir.mk
-include simplicity_sdk_2024.12.2/platform/service/system/src/subdir.mk
-include simplicity_sdk_2024.12.2/platform/service/sleeptimer/src/subdir.mk
-include simplicity_sdk_2024.12.2/platform/service/memory_manager/src/subdir.mk
-include simplicity_sdk_2024.12.2/platform/service/interrupt_manager/src/subdir.mk
-include simplicity_sdk_2024.12.2/platform/service/device_manager/src/subdir.mk
-include simplicity_sdk_2024.12.2/platform/service/device_manager/devices/subdir.mk
-include simplicity_sdk_2024.12.2/platform/service/device_manager/clocks/subdir.mk
-include simplicity_sdk_2024.12.2/platform/service/device_init/src/subdir.mk
-include simplicity_sdk_2024.12.2/platform/service/clock_manager/src/subdir.mk
-include simplicity_sdk_2024.12.2/platform/peripheral/src/subdir.mk
-include simplicity_sdk_2024.12.2/platform/emlib/src/subdir.mk
-include simplicity_sdk_2024.12.2/platform/driver/leddrv/src/subdir.mk
-include simplicity_sdk_2024.12.2/platform/driver/i2cspm/src/subdir.mk
-include simplicity_sdk_2024.12.2/platform/driver/gpio/src/subdir.mk
-include simplicity_sdk_2024.12.2/platform/common/src/subdir.mk
-include simplicity_sdk_2024.12.2/platform/Device/SiliconLabs/BGM22/Source/subdir.mk
-include simplicity_sdk_2024.12.2/hardware/board/src/subdir.mk
-include autogen/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := bgm_proj
BUILD_ARTIFACT_EXTENSION := axf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: main-build

# Main-build Target
main-build: bgm_proj.axf

# Tool invocations
bgm_proj.axf: $(OBJS) $(USER_OBJS) makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM C++ Linker'
	arm-none-eabi-g++ -g -gdwarf-2 -mcpu=cortex-m33 -mthumb -T "D:\01-University\00-Github\Proprioception_stimulation\cpp_project\bgm_proj\autogen\linkerfile.ld" -fno-lto -Wl,--no-warn-rwx-segments --specs=nosys.specs -Xlinker --gc-sections -Xlinker -Map="bgm_proj.map" -mfpu=fpv5-sp-d16 -mfloat-abi=hard --specs=nano.specs -o bgm_proj.axf -Wl,--start-group "./app.o" "./driver.o" "./main.o" "./autogen/sl_board_default_init.o" "./autogen/sl_event_handler.o" "./autogen/sl_i2cspm_init.o" "./simplicity_sdk_2024.12.2/hardware/board/src/sl_board_control_gpio.o" "./simplicity_sdk_2024.12.2/hardware/board/src/sl_board_init.o" "./simplicity_sdk_2024.12.2/platform/Device/SiliconLabs/BGM22/Source/startup_bgm22.o" "./simplicity_sdk_2024.12.2/platform/Device/SiliconLabs/BGM22/Source/system_bgm22.o" "./simplicity_sdk_2024.12.2/platform/common/src/sl_assert.o" "./simplicity_sdk_2024.12.2/platform/common/src/sl_core_cortexm.o" "./simplicity_sdk_2024.12.2/platform/common/src/sl_syscalls.o" "./simplicity_sdk_2024.12.2/platform/driver/gpio/src/sl_gpio.o" "./simplicity_sdk_2024.12.2/platform/driver/i2cspm/src/sl_i2cspm.o" "./simplicity_sdk_2024.12.2/platform/driver/leddrv/src/sl_led.o" "./simplicity_sdk_2024.12.2/platform/emlib/src/em_burtc.o" "./simplicity_sdk_2024.12.2/platform/emlib/src/em_cmu.o" "./simplicity_sdk_2024.12.2/platform/emlib/src/em_core.o" "./simplicity_sdk_2024.12.2/platform/emlib/src/em_emu.o" "./simplicity_sdk_2024.12.2/platform/emlib/src/em_gpio.o" "./simplicity_sdk_2024.12.2/platform/emlib/src/em_i2c.o" "./simplicity_sdk_2024.12.2/platform/emlib/src/em_msc.o" "./simplicity_sdk_2024.12.2/platform/emlib/src/em_rtcc.o" "./simplicity_sdk_2024.12.2/platform/emlib/src/em_system.o" "./simplicity_sdk_2024.12.2/platform/emlib/src/em_timer.o" "./simplicity_sdk_2024.12.2/platform/peripheral/src/sl_hal_gpio.o" "./simplicity_sdk_2024.12.2/platform/service/clock_manager/src/sl_clock_manager.o" "./simplicity_sdk_2024.12.2/platform/service/clock_manager/src/sl_clock_manager_hal_s2.o" "./simplicity_sdk_2024.12.2/platform/service/clock_manager/src/sl_clock_manager_init.o" "./simplicity_sdk_2024.12.2/platform/service/clock_manager/src/sl_clock_manager_init_hal_s2.o" "./simplicity_sdk_2024.12.2/platform/service/device_init/src/sl_device_init_dcdc_s2.o" "./simplicity_sdk_2024.12.2/platform/service/device_init/src/sl_device_init_emu_s2.o" "./simplicity_sdk_2024.12.2/platform/service/device_manager/clocks/sl_device_clock_efr32xg22.o" "./simplicity_sdk_2024.12.2/platform/service/device_manager/devices/sl_device_peripheral_hal_efr32xg22.o" "./simplicity_sdk_2024.12.2/platform/service/device_manager/src/sl_device_clock.o" "./simplicity_sdk_2024.12.2/platform/service/device_manager/src/sl_device_gpio.o" "./simplicity_sdk_2024.12.2/platform/service/device_manager/src/sl_device_peripheral.o" "./simplicity_sdk_2024.12.2/platform/service/interrupt_manager/src/sl_interrupt_manager_cortexm.o" "./simplicity_sdk_2024.12.2/platform/service/memory_manager/src/sl_memory_manager_region.o" "./simplicity_sdk_2024.12.2/platform/service/sleeptimer/src/sl_sleeptimer.o" "./simplicity_sdk_2024.12.2/platform/service/sleeptimer/src/sl_sleeptimer_hal_burtc.o" "./simplicity_sdk_2024.12.2/platform/service/sleeptimer/src/sl_sleeptimer_hal_prortc.o" "./simplicity_sdk_2024.12.2/platform/service/sleeptimer/src/sl_sleeptimer_hal_rtcc.o" "./simplicity_sdk_2024.12.2/platform/service/sleeptimer/src/sl_sleeptimer_hal_timer.o" "./simplicity_sdk_2024.12.2/platform/service/system/src/sl_system_init.o" "./simplicity_sdk_2024.12.2/platform/service/system/src/sl_system_process_action.o" "./simplicity_sdk_2024.12.2/platform/service/udelay/src/sl_udelay.o" "./simplicity_sdk_2024.12.2/platform/service/udelay/src/sl_udelay_armv6m_gcc.o" -lstdc++ -lgcc -lc -lm -lnosys -Wl,--end-group -Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

	@echo 'Building out file: bgm_proj.out'
	arm-none-eabi-objcopy "bgm_proj.axf" "bgm_proj.out"
	@echo ' '

	@echo 'Building hex file: bgm_proj.hex'
	arm-none-eabi-objcopy -O ihex "bgm_proj.axf" "bgm_proj.hex"
	@echo ' '

	@echo 'Building bin file: bgm_proj.bin'
	arm-none-eabi-objcopy -O binary "bgm_proj.axf" "bgm_proj.bin"
	@echo ' '

	@echo 'Building s37 file: bgm_proj.s37'
	arm-none-eabi-objcopy -O srec "bgm_proj.axf" "bgm_proj.s37"
	@echo ' '

	@echo 'Running size tool'
	arm-none-eabi-size "bgm_proj.axf" -A
	@echo ' '

# Other Targets
clean:
	-@$(foreach FILE, $(CC_DEPS), $(shell $(RM) $(FILE)) )
	-@$(foreach FILE, $(C++_DEPS), $(shell $(RM) $(FILE)) )
	-@$(foreach FILE, $(EXECUTABLES), $(shell $(RM) $(FILE)) )
	-@$(foreach FILE, $(OBJS), $(shell $(RM) $(FILE)) )
	-@$(foreach FILE, $(C_UPPER_DEPS), $(shell $(RM) $(FILE)) )
	-@$(foreach FILE, $(CXX_DEPS), $(shell $(RM) $(FILE)) )
	-@$(foreach FILE, $(C_DEPS), $(shell $(RM) $(FILE)) )
	-@$(foreach FILE, $(CPP_DEPS), $(shell $(RM) $(FILE)) )
	-$(RM) bgm_proj.axf
	-@echo ' '

.PHONY: all clean dependents main-build

-include ../makefile.targets
