################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/drsanti/Desktop/TOPGUN-2021/STM32/B-L475E-IOT01_GenericMQTT/Projects/B-L475E-IOT01/Applications/Cloud/GenericMQTT/Src/es_wifi_io.c \
C:/Users/drsanti/Desktop/TOPGUN-2021/STM32/B-L475E-IOT01_GenericMQTT/Projects/B-L475E-IOT01/Applications/Cloud/GenericMQTT/Src/wifi.c 

OBJS += \
./Application/Wifi/es_wifi_io.o \
./Application/Wifi/wifi.o 

C_DEPS += \
./Application/Wifi/es_wifi_io.d \
./Application/Wifi/wifi.d 


# Each subdirectory must supply rules for building sources it contributes
Application/Wifi/es_wifi_io.o: C:/Users/drsanti/Desktop/TOPGUN-2021/STM32/B-L475E-IOT01_GenericMQTT/Projects/B-L475E-IOT01/Applications/Cloud/GenericMQTT/Src/es_wifi_io.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32L475xx -DUSE_HAL_DRIVER -DSENSOR '-DMBEDTLS_CONFIG_FILE=<genmqtt_mbedtls_config.h>' '-DMQTTCLIENT_PLATFORM_HEADER=paho_mqtt_platform.h' -DENABLE_IOT_INFO -DENABLE_IOT_ERROR -DENABLE_IOT_WARNING -DGENERICMQTT -DUSE_MBED_TLS -DUSE_WIFI -c -I../../../../../../../../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../../../../../../../../Drivers/STM32L4xx_HAL_Driver/Inc -I../../../../../../../../Drivers/BSP/B-L475E-IOT01 -I../../../Inc -I../../../../../../../Common/GenericMQTT/Inc -I../../../../../../../Common/Shared/Inc -I../../../../../../../../Middlewares/Third_Party/paho.mqtt.embedded-c/MQTTClient-C/src -I../../../../../../../../Middlewares/Third_Party/paho.mqtt.embedded-c/MQTTPacket/src -I../../../../../../../../Middlewares/Third_Party/mbedTLS/include -I../../../../../../../../Middlewares/Third_Party/FreeRTOS/Source/include -I../../../../../../../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../../../../../../../../Middlewares/Third_Party/cJSON -I../../../../../../../../Drivers/BSP/Components/es_wifi -I../../../../../../../../Drivers/BSP/Components/hts221 -I../../../../../../../../Drivers/BSP/Components/lis3mdl -I../../../../../../../../Drivers/BSP/Components/lps22hb -I../../../../../../../../Drivers/BSP/Components/lsm6dsl -I../../../../../../../../Drivers/BSP/Components/vl53l0x -I../../../../../../../../Drivers/CMSIS/Include -Os -ffunction-sections -Wall -fstack-usage -MMD -MP -MF"Application/Wifi/es_wifi_io.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/Wifi/wifi.o: C:/Users/drsanti/Desktop/TOPGUN-2021/STM32/B-L475E-IOT01_GenericMQTT/Projects/B-L475E-IOT01/Applications/Cloud/GenericMQTT/Src/wifi.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32L475xx -DUSE_HAL_DRIVER -DSENSOR '-DMBEDTLS_CONFIG_FILE=<genmqtt_mbedtls_config.h>' '-DMQTTCLIENT_PLATFORM_HEADER=paho_mqtt_platform.h' -DENABLE_IOT_INFO -DENABLE_IOT_ERROR -DENABLE_IOT_WARNING -DGENERICMQTT -DUSE_MBED_TLS -DUSE_WIFI -c -I../../../../../../../../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../../../../../../../../Drivers/STM32L4xx_HAL_Driver/Inc -I../../../../../../../../Drivers/BSP/B-L475E-IOT01 -I../../../Inc -I../../../../../../../Common/GenericMQTT/Inc -I../../../../../../../Common/Shared/Inc -I../../../../../../../../Middlewares/Third_Party/paho.mqtt.embedded-c/MQTTClient-C/src -I../../../../../../../../Middlewares/Third_Party/paho.mqtt.embedded-c/MQTTPacket/src -I../../../../../../../../Middlewares/Third_Party/mbedTLS/include -I../../../../../../../../Middlewares/Third_Party/FreeRTOS/Source/include -I../../../../../../../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../../../../../../../../Middlewares/Third_Party/cJSON -I../../../../../../../../Drivers/BSP/Components/es_wifi -I../../../../../../../../Drivers/BSP/Components/hts221 -I../../../../../../../../Drivers/BSP/Components/lis3mdl -I../../../../../../../../Drivers/BSP/Components/lps22hb -I../../../../../../../../Drivers/BSP/Components/lsm6dsl -I../../../../../../../../Drivers/BSP/Components/vl53l0x -I../../../../../../../../Drivers/CMSIS/Include -Os -ffunction-sections -Wall -fstack-usage -MMD -MP -MF"Application/Wifi/wifi.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

