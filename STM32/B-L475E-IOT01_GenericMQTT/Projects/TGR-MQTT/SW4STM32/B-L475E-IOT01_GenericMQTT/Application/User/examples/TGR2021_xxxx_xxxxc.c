



























































	void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
	{
		//...CODE...
	}

	void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
	{
		if(GPIO_Pin == GPIO_PIN_13) {
			//...CODE...
		}
	}


	void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
	{
		//...CODE...
	}


	void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
	{
		if(htim->Instance == TIM2) {
			//...CODE...
		}
	}



	void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
	{
		//...CODE...
	}


	void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
	{
		if(huart->Instance == USART1) {
			//....CODE...
		}
	}





















	uint8_t rxBuffer[4];
	uint8_t rxData [8];
	uint8_t rxCounter = 0;

	void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
	{
		HAL_UART_Receive_IT(&huart1, rxBuffer, 1);

		if(rxBuffer[0] == '\n') {

			if(rxCounter == 2 && rxData[0] == 'O' && rxData[1] == 'N') {
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET); 	// ON
			}
			else if(rxCounter == 3 && rxData[0] == 'O' && rxData[1] == 'F' && rxData[2] == 'F') {
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); 	// OFF
			}
			else {}

			rxCounter = 0;
		}
		else {
			rxData[rxCounter] = rxBuffer[0];
			rxCounter++;
			if(rxCounter > 3) {
				rxCounter = 0;
			}
		}
	}

	int main(void) {
		//...

		HAL_UART_Receive_IT(&huart1, rxBuffer, 1);

		while (1) {
			//...
		}
	}








	uint8_t  led_status = 0; // OFF
	uint16_t t_on   = 0;
	uint16_t t_off  = 0;


	uint16_t timCounter = 0;

	void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
	{	// 0.01 == 10 mS
		timCounter++;
	}

	int main(void) {
		//...
		HAL_TIM_Base_Start_IT(&htim2);
		while (1) {
			if(timCounter >= 1) {
				timCounter = 0;
				if(led_status == GPIO_PIN_RESET) {
					if(++t_off == 90) {
						t_off = 0;
						led_status = GPIO_PIN_SET;		// ON
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, led_status);
					}
				}
				else if(led_status == GPIO_PIN_SET) {
					if(++t_on == 10) {
						t_on = 0;
						led_status = GPIO_PIN_RESET;	// OFF
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, led_status);
					}
				}
			}
		}
	}














	uint16_t counter = 0;

	void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
	{
		counter++;
	}

	int main(void)
	{
		//...
		while (1)
		{
			if(counter >= 5) {
				counter = 0;
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
				HAL_Delay(500);
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
			}
		}
	}





































	char rxBuffer[32];
	while (1)
	{
		HAL_StatusTypeDef status = HAL_UART_Receive(&huart1, (uint8_t *)rxBuffer, 1, 10);

		if(status == HAL_OK) {
			if( rxBuffer[0] == '0' ) {
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); 	// LED1 OFF
			}
			else if( rxBuffer[0] == '1' ) {
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);		// LED1 ON
			}
			else {
				// Error
			}
		}
	}


	uint16_t counter = 0;
	while (1)
	{
		char buffer[32];
		sprintf(buffer, "Counter: %d\n", ++counter);
		HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 20);
		HAL_Delay(500);
	}




	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin  = GPIO_PIN_9;
	while (1)
	{

		// Output Mode (Yellow or Blue LEDs)
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		// Yellow LED ON, Blue LED OFF
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
		HAL_Delay(900);

		// Blue LED ON, Yellow LED OFF
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
		HAL_Delay(100);


		// Input Mode (Yellow & Blue LEDs OFF)
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
		HAL_Delay(1000);
	}


	while (1)
	{
		// LED1 ON
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
		HAL_Delay(100);

		// LED1 OFF
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
		HAL_Delay(900);

		// Get switch status
		GPIO_PinState blueSW = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);

		// Write switch status to LED2
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, blueSW);
	}











if(rxCounter >= 3) {
		if(rxData[0] == 'O' && rxData[1] == 'N') {
			rxCounter = 0;
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET); // ON
		}
		else if(rxData[0] == 'O' && rxData[1] == 'F' && rxData[2] == 'F' ) {
			rxCounter = 0;
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); // OFF
		}
	}
}
if(rxCounter >= 4) {
	rxCounter = 0;
	// Error
}


	/*** Interrupt Function ***/

	void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
	{
		counter++;
	}



	/*** Main Function ***/

	int main(void)
	{
		while(1)
		{
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
			HAL_Delay(500);

			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
			HAL_Delay(500);
		}
	}




/*** TGR2021 SECTION1 START ******************************************************************/

/*** YOUR CODE HERE
int x = 0;

/*** TGR2021 SECTION1 END ********************************************************************/


/*** TGR2021 SECTION2 START ******************************************************************/
/*** YOUR CODE HERE
 *
int y = 1;

/*** TGR2021 SECTION2 END ********************************************************************/


/*** TGR2021 SECTION1 START ******************************************************************/


/*** TGR2021 SECTION1 END ********************************************************************/



//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
//HAL_Delay(200);

//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
//HAL_Delay(200);

HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
HAL_Delay(100);


//*** TGR2021 SECTION1 END ********************************************************************


if(state == 0) {

	if(blueSW == GPIO_PIN_SET) {
		state = 1;
	}
}
else if(state == 1) {

	if(blueSW == GPIO_PIN_RESET) {
		state = 0;
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
	}
}

//HAL_Delay(10);







blueSW = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);


/*********************************************************************************************
 * File Name:	TGR2021_xxxx_xxxxc.c
 *
 * Description: Provides API functions for accessing IOs, timing, and MQTT client
 *
 * Author:		Asst.Prof.Dr.Santi Nuratch
 * 				Embedded Computing and Control Laboratory		: ECC-Lab
 * 				Control System and Instrumentation Engineering 	: INC
 *				King Mongkut's University of Technology Thonburi: KMUTT
 *
 * Update:		06 December 2020 (Initial version)
 *
 *********************************************************************************************/

/*********************************************************************************************
####### ####### ######   #####  #     # #     #    ######     #    #       #       #     #
   #    #     # #     # #     # #     # ##    #    #     #   # #   #       #        #   #
   #    #     # #     # #       #     # # #   #    #     #  #   #  #       #         # #
   #    #     # ######  #  #### #     # #  #  #    ######  #     # #       #          #
   #    #     # #       #     # #     # #   # #    #   #   ####### #       #          #
   #    #     # #       #     # #     # #    ##    #    #  #     # #       #          #
   #    ####### #        #####   #####  #     #    #     # #     # ####### #######    #
**********************************************************************************************/



/*********************************************************************************************
 * Include File                                                                              *
 *********************************************************************************************/
#include <main.h>




/*********************************************************************************************
 * APPLICATION BEGIN                                                                         *
 *********************************************************************************************/

/*
 *********************************************************************************************
 *                                 OUTPUT/INPUT DESCRIPTION                                  *
 *********************************************************************************************
 * DIGITAL OUTPUT PORTS                                                                      *
 *********************************************************************************************
 * +----------+----------+--------+---------------------+------------------------------------+
 * |   NAME   |   GPIO   |   PIN  |  GPIOx, GPIO_PIN_x  |             Description            |
 * +----------+----------+--------+---------------------+------------------------------------+
 * |    A0    |   GPIOC  |    5   |  GPIOC, GPIO_PIN_5  | CN4 PIN1 - Digital Output          |
 * +----------+----------+--------+---------------------+------------------------------------+
 * |    A1    |   GPIOC  |    4   |  GPIOC, GPIO_PIN_4  | CN4 PIN2 - Digital Output          |
 * +----------+----------+--------+---------------------+------------------------------------+
 * |    A2    |   GPIOC  |    5   |  GPIOC, GPIO_PIN_3  | CN4 PIN3 - Digital Output          |
 * +----------+----------+--------+---------------------+------------------------------------+
 * |    A3    |   GPIOC  |    2   |  GPIOC, GPIO_PIN_2  | CN4 PIN4 - Digital Output          |
 * +----------+----------+--------+---------------------+------------------------------------+
 * |    A4    |   GPIOC  |    1   |  GPIOC, GPIO_PIN_1  | CN4 PIN5 - Digital Output          |
 * +----------+----------+--------+---------------------+------------------------------------+
 * |    A5    |   GPIOC  |    0   |  GPIOC, GPIO_PIN_0  | CN4 PIN6 - Digital Output          |
 * +----------+----------+--------+---------------------+------------------------------------+
 * |   LED1   |   GPIOA  |    5   |  GPIOA, GPIO_PIN_5  | LD1 - Built-in GREEN LED           |
 * +----------+----------+--------+---------------------+------------------------------------+
 * |   LED2   |   GPIOB  |   14   |  GPIOB, GPIO_PIN_14 | LD2 - Built-in GREEN LED           |
 * +----------+----------+--------+---------------------+------------------------------------+
 * |   LED3   |   GPIOC  |    9   |  GPIOC, GPIO_PIN_9  | LD2&LD4 - Built-in BLUE/ORANGE LED |
 * +----------+----------+--------+---------------------+------------------------------------+
 *
 * +-----------------------------------------------------------------------------------------+
 * | Examples:                                                                               |
 * +-----------------------------------------------------------------------------------------+
 * |    HAL_GPIO_WritePin(GPIOx, GPIO_PIN_x, GPIO_PIN_RESET);                                |
 * +-----------------------------------------------------------------------------------------+
 * +    HAL_GPIO_WritePin(GPIOx, GPIO_PIN_x, GPIO_PIN_SET);                                  |
 * +-----------------------------------------------------------------------------------------+
 * +    HAL_GPIO_TogglePin(GPIOx, GPIO_PIN_x);                                               |
 * +-----------------------------------------------------------------------------------------+
 *
 *
 *********************************************************************************************
 * DIGITAL INPUT PORT                                                                        *
 *********************************************************************************************
 * +----------+----------+--------+---------------------+------------------------------------+
 * |   NAME   |   GPIO   |   PIN  |  GPIOx, GPIO_PIN_x  |             Description            |
 * +----------+----------+--------+---------------------+------------------------------------+
 * | BLUE SW  |   GPIOC  |   13   |  GPIOC, GPIO_PIN_13 | BLUE SWITCH - Built-in Push Button |
 * +----------+----------+--------+---------------------+------------------------------------+
 *
 * +-----------------------------------------------------------------------------------------+
 * | Examples:                                                                               |
 * +-----------------------------------------------------------------------------------------+
 * |    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET) {...}                    |
 * +-----------------------------------------------------------------------------------------+
 * |    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET) {...}                    |
 * +-----------------------------------------------------------------------------------------+
 */


/*********************************************************************************************
 * Global Variables                                                                          *
 *********************************************************************************************/
static MQTTClient *ptr_Client;


/*********************************************************************************************
 * Callback Functions                                                                        *
 *********************************************************************************************/
void mqtt_message_received(MessageData* data) {
	printf("\n\n**mqtt_message_received()\n\n");
	char topicBuffer[64]={0};
	int len = data->topicName->lenstring.len;
	memcpy(topicBuffer, data->topicName->lenstring.data,  len);

	char payload[64]={0};
	memcpy(payload, data->message->payload, data->message->payloadlen);

	printf("Topic:  %s\r\nPayload: %s\r\n", topicBuffer, payload);
}


void mqtt_connected(MQTTClient *client, const char *deviceId) {
	printf("\n\n**mqtt_connected() %s\n\n", deviceId);
	ptr_Client = client;
	Mqtt_Subscribe(client, "/hello/world", mqtt_message_received);
}


void psw_pushed(uint32_t count) {
	printf("\nbtn_pushed_callback()\n");
	if(Mqtt_CanPublish()){
		Mqtt_SensorsPublish();
	}
	else if(Mqtt_CanPublish()){
		Mqtt_Publish(ptr_Client, "/hello/world", "pushed callback!") ;
	}
}


void psw_holding(uint32_t count) {
	printf("\nbtn_holding_callback()\n");
	if(Mqtt_CanPublish()){
		Mqtt_SensorsPublishContinuous(2);
	}
	else if(Mqtt_CanPublish()){
		Mqtt_Publish(ptr_Client, "/hello/world", "holding callback!");
	}

}


void ticked_callback(uint32_t ticks) {
	if(ticks % 1000 == 0 && Clk_IsRtcUpdated()) {
		printf("time: %s\r\n", Clk_GetString(1));
	}
}


/*** TGR2021 SECTION1 START ******************************************************************/
/*** YOUR CODE HERE

/*** TGR2021 SECTION1 END ********************************************************************/



/*********************************************************************************************
 ****                                                                                     ****
 **** TGR2021 MAIN FUNCTION                                                               ****
 ****                                                                                     ****
 *********************************************************************************************/

void TGR_Main(void) {


	/*** TGR2021 SECTION2 START **************************************************************/
	/*** YOUR CODE HERE

	/*** TGR2021 SECTION2 END ****************************************************************/


	Psw_SetPushedCallback(psw_pushed);
	Psw_SetHoldingCallback(psw_holding);
	Mqtt_SetConnectedCallback(mqtt_connected);
	Clk_SetTickCallback(ticked_callback);
	GenericMQTT_Client_Run(0);
	while(1);
}

/*********************************************************************************************
 ****                                                                                     ****
 **** END OF MAIN FUNCTION                                                                ****
 ****                                                                                     ****
 *********************************************************************************************/


/*
	####### ####### ######   #####  #     # #     #    ######     #    #       #       #     #
	   #    #     # #     # #     # #     # ##    #    #     #   # #   #       #        #   #
	   #    #     # #     # #       #     # # #   #    #     #  #   #  #       #         # #
	   #    #     # ######  #  #### #     # #  #  #    ######  #     # #       #          #
	   #    #     # #       #     # #     # #   # #    #   #   ####### #       #          #
	   #    #     # #       #     # #     # #    ##    #    #  #     # #       #          #
	   #    ####### #        #####   #####  #     #    #     # #     # ####### #######    #
*/




/*** TGR2021 SECTION1 START ******************************************************************/

/*** YOUR CODE HERE
int x = 0;

/*** TGR2021 SECTION1 END ********************************************************************/


/*** TGR2021 SECTION2 START ******************************************************************/
/*** YOUR CODE HERE
 *
int y = 1;

/*** TGR2021 SECTION2 END ********************************************************************/
