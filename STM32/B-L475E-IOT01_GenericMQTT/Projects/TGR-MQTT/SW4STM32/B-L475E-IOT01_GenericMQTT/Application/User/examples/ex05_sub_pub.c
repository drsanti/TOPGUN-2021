
#include <main.h>


/*** TGR2021 SECTION1 START ******************************************************************/
/*** YOUR CODE HERE */

MQTTClient *ptr_Client;

void mqtt_message_callback(MessageData *data)
{
	char buffer[64];
	memcpy(buffer, data->message->payload, data->message->payloadlen);
	buffer[data->message->payloadlen] = 0;
	printf("Received Payload: %s\n", buffer);
}

void mqtt_connected_callback(MQTTClient *client, const char *deviceId)
{
	ptr_Client = client;
	printf("%s connected to mqtt server.\n", deviceId);
	Mqtt_Subscribe(ptr_Client, "tgr_test", mqtt_message_callback);
}

void key_down(uint32_t count)
{
	static int32_t counter = 0;
	char buffer[32];
	sprintf(buffer, "counter: %ld\n", ++counter);

	if(Mqtt_CanPublish()){
		Mqtt_Publish(ptr_Client, "tgr_test", buffer);
	}
	else {
		printf("Cannot publish!\n");
	}
}

/*** TGR2021 SECTION1 END ********************************************************************/



void TGR_Main(void)
{
	/*** TGR2021 SECTION2 START ******************************************************************/
	/*** YOUR CODE HERE */

	Mqtt_SetConnectedCallback(mqtt_connected_callback);
	Psw_SetPushedCallback(key_down);
	/*** TGR2021 SECTION1 END ********************************************************************/


	GenericMQTT_Client_Run(0);
	while(1);
}
