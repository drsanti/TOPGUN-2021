################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Middlewares/Third_Party/cJSON/subdir.mk
-include Middlewares/Third_Party/MbedTLS/subdir.mk
-include Middlewares/Third_Party/MQTTPacket/subdir.mk
-include Middlewares/Third_Party/MQTTClient-C/subdir.mk
-include Drivers/STM32L4xx_HAL_Driver/subdir.mk
-include Drivers/CMSIS/subdir.mk
-include Drivers/BSP/Components/vl53l0x/subdir.mk
-include Drivers/BSP/Components/LSM6DSL/subdir.mk
-include Drivers/BSP/Components/LPS22HB/subdir.mk
-include Drivers/BSP/Components/LIS3MDL/subdir.mk
-include Drivers/BSP/Components/HTS221/subdir.mk
-include Drivers/BSP/B-L475E-IOT01/subdir.mk
-include Drivers/BSP/subdir.mk
-include Application/Wifi/subdir.mk
-include Application/User/subdir.mk
-include Application/SW4STM32/subdir.mk
-include Application/GenericMQTT/subdir.mk
-include Application/Common/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
B-L475E-IOT01_GenericMQTT.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
B-L475E-IOT01_GenericMQTT.list \

OBJCOPY_BIN += \
B-L475E-IOT01_GenericMQTT.bin \


# All Target
all: B-L475E-IOT01_GenericMQTT.elf secondary-outputs

# Tool invocations
B-L475E-IOT01_GenericMQTT.elf: $(OBJS) $(USER_OBJS) ../STM32L475VGTx_FLASH.ld
	arm-none-eabi-gcc -o "B-L475E-IOT01_GenericMQTT.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"../STM32L475VGTx_FLASH.ld" --specs=nosys.specs -Wl,-Map="B-L475E-IOT01_GenericMQTT.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -u _printf_float -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

B-L475E-IOT01_GenericMQTT.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "B-L475E-IOT01_GenericMQTT.list"
	@echo 'Finished building: $@'
	@echo ' '

B-L475E-IOT01_GenericMQTT.bin: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "B-L475E-IOT01_GenericMQTT.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_BIN)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
