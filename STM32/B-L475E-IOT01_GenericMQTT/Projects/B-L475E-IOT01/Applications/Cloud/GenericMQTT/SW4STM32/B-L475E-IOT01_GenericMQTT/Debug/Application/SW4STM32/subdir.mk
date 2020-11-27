################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
C:/Users/drsanti/Desktop/TOPGUN-2021/STM32/B-L475E-IOT01_GenericMQTT/Projects/B-L475E-IOT01/Applications/Cloud/GenericMQTT/SW4STM32/startup_stm32l475xx.s 

C_SRCS += \
C:/Users/drsanti/Desktop/TOPGUN-2021/STM32/B-L475E-IOT01_GenericMQTT/Projects/B-L475E-IOT01/Applications/Cloud/GenericMQTT/SW4STM32/syscalls.c 

OBJS += \
./Application/SW4STM32/startup_stm32l475xx.o \
./Application/SW4STM32/syscalls.o 

S_DEPS += \
./Application/SW4STM32/startup_stm32l475xx.d 

C_DEPS += \
./Application/SW4STM32/syscalls.d 


# Each subdirectory must supply rules for building sources it contributes
Application/SW4STM32/startup_stm32l475xx.o: C:/Users/drsanti/Desktop/TOPGUN-2021/STM32/B-L475E-IOT01_GenericMQTT/Projects/B-L475E-IOT01/Applications/Cloud/GenericMQTT/SW4STM32/startup_stm32l475xx.s
	arm-none-eabi-gcc -mcpu=cortex-m4 -g3 -c -x assembler-with-cpp -MMD -MP -MF"Application/SW4STM32/startup_stm32l475xx.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"
Application/SW4STM32/syscalls.o: C:/Users/drsanti/Desktop/TOPGUN-2021/STM32/B-L475E-IOT01_GenericMQTT/Projects/B-L475E-IOT01/Applications/Cloud/GenericMQTT/SW4STM32/syscalls.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32L475xx -DUSE_HAL_DRIVER -DSENSOR '-DMBEDTLS_CONFIG_FILE=<genmqtt_mbedtls_config.h>' '-DMQTTCLIENT_PLATFORM_HEADER=paho_mqtt_platform.h' -DENABLE_IOT_INFO -DENABLE_IOT_ERROR -DENABLE_IOT_WARNING -DGENERICMQTT -DUSE_MBED_TLS -DUSE_WIFI -c -I../../../../../../../../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../../../../../../../../Drivers/STM32L4xx_HAL_Driver/Inc -I../../../../../../../../Drivers/BSP/B-L475E-IOT01 -I../../../Inc -I../../../../../../../Common/GenericMQTT/Inc -I../../../../../../../Common/Shared/Inc -I../../../../../../../../Middlewares/Third_Party/paho.mqtt.embedded-c/MQTTClient-C/src -I../../../../../../../../Middlewares/Third_Party/paho.mqtt.embedded-c/MQTTPacket/src -I../../../../../../../../Middlewares/Third_Party/mbedTLS/include -I../../../../../../../../Middlewares/Third_Party/FreeRTOS/Source/include -I../../../../../../../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../../../../../../../../Middlewares/Third_Party/cJSON -I../../../../../../../../Drivers/BSP/Components/es_wifi -I../../../../../../../../Drivers/BSP/Components/hts221 -I../../../../../../../../Drivers/BSP/Components/lis3mdl -I../../../../../../../../Drivers/BSP/Components/lps22hb -I../../../../../../../../Drivers/BSP/Components/lsm6dsl -I../../../../../../../../Drivers/BSP/Components/vl53l0x -I../../../../../../../../Drivers/CMSIS/Include -Os -ffunction-sections -Wall -fstack-usage -MMD -MP -MF"Application/SW4STM32/syscalls.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

