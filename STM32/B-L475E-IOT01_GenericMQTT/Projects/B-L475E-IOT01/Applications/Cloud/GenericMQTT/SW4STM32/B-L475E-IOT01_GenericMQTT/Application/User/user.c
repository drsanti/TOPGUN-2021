
#include "main.h"
#include "MQTTClient.h"

void mcu_initialized(void *params) {
	int16_t cnt = 0;
	while(++cnt<10) {

//		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);			// LED1 ON
//		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);		// LED2 ON
//		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);			// LED3 ON
//		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5, GPIO_PIN_SET); // SET A<5:0>
//		HAL_Delay(50);
//
//		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);		// LED1 OFF
//		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);		// LED2 OFF
//		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);		// LED3 OFF
//		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5, GPIO_PIN_RESET); // CLEAR A<5:0>
//		HAL_Delay(200);
	}
}


void iot_got_ip_address(void *params) {
	msg_info("\niot_got_ip_address()\n");
}

void sensors_initialized(void *params) {
	msg_info("\nsensors_initialized()\n");
}

void mqtt_connected_to_server(void *params) {
	MQTTClient* c = (MQTTClient*)params;
	(void)c;
	msg_info("\nmqtt_connected_to_server()\n");
}

//void mqtt_on_message_received(const char* topic, const char *message) {
//	printf("Topic: %.*s, message: %s\r\n", topic, message);
//}
