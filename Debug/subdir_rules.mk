################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"/Applications/ti/ccs1281/ccs/tools/compiler/ti-cgt-armllvm_3.2.1.LTS/bin/tiarmclang" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"/Users/jiantangli/workspace_v12/sensor_and_lcd" -I"/Users/jiantangli/workspace_v12/sensor_and_lcd/Debug" -I"/Users/jiantangli/ti/mspm0_sdk_2_03_00_07/source/third_party/CMSIS/Core/Include" -I"/Users/jiantangli/ti/mspm0_sdk_2_03_00_07/source" -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-1599572399: ../i2c_controller_rw_multibyte_fifo_interrupts.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"/Users/jiantangli/ti/sysconfig_1.22.0/sysconfig_cli.sh" --script "/Users/jiantangli/workspace_v12/sensor_and_lcd/i2c_controller_rw_multibyte_fifo_interrupts.syscfg" -o "." -s "/Users/jiantangli/ti/mspm0_sdk_2_03_00_07/.metadata/product.json" --compiler ticlang
	@echo 'Finished building: "$<"'
	@echo ' '

device_linker.cmd: build-1599572399 ../i2c_controller_rw_multibyte_fifo_interrupts.syscfg
device.opt: build-1599572399
device.cmd.genlibs: build-1599572399
ti_msp_dl_config.c: build-1599572399
ti_msp_dl_config.h: build-1599572399
Event.dot: build-1599572399

%.o: ./%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"/Applications/ti/ccs1281/ccs/tools/compiler/ti-cgt-armllvm_3.2.1.LTS/bin/tiarmclang" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"/Users/jiantangli/workspace_v12/sensor_and_lcd" -I"/Users/jiantangli/workspace_v12/sensor_and_lcd/Debug" -I"/Users/jiantangli/ti/mspm0_sdk_2_03_00_07/source/third_party/CMSIS/Core/Include" -I"/Users/jiantangli/ti/mspm0_sdk_2_03_00_07/source" -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

startup_mspm0g350x_ticlang.o: /Users/jiantangli/ti/mspm0_sdk_2_03_00_07/source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_mspm0g350x_ticlang.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"/Applications/ti/ccs1281/ccs/tools/compiler/ti-cgt-armllvm_3.2.1.LTS/bin/tiarmclang" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"/Users/jiantangli/workspace_v12/sensor_and_lcd" -I"/Users/jiantangli/workspace_v12/sensor_and_lcd/Debug" -I"/Users/jiantangli/ti/mspm0_sdk_2_03_00_07/source/third_party/CMSIS/Core/Include" -I"/Users/jiantangli/ti/mspm0_sdk_2_03_00_07/source" -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


