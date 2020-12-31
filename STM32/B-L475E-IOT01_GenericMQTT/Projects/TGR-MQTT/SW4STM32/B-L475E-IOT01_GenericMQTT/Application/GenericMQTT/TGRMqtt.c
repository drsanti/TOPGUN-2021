/*****************************************************************************************************
 * File Name:	TGRMqtt.c
 *
 * Description: Provides functions for performing with MQTT client
 *
 * Author:		Asst.Prof.Dr.Santi Nuratch
 * 				Embedded Computing and Control Laboratory		: ECC-Lab
 * 				Control System and Instrumentation Engineering 	: INC
 *				King Mongkut's University of Technology Thonburi: KMUTT
 *
 * Update:		06 December 2020 (Initial version)
 *
 *****************************************************************************************************/

/*********************************************************************************************
####### ####### ######   #####  #     # #     #    ######     #    #       #       #     #
   #    #     # #     # #     # #     # ##    #    #     #   # #   #       #        #   #
   #    #     # #     # #       #     # # #   #    #     #  #   #  #       #         # #
   #    #     # ######  #  #### #     # #  #  #    ######  #     # #       #          #
   #    #     # #       #     # #     # #   # #    #   #   ####### #       #          #
   #    #     # #       #     # #     # #    ##    #    #  #     # #       #          #
   #    ####### #        #####   #####  #     #    #     # #     # ####### #######    #
**********************************************************************************************/

#ifndef APPLICATION_GENERICMQTT_TGRMQTT_C_
#define APPLICATION_GENERICMQTT_TGRMQTT_C_

#include <TGRMqtt.h>

/*****************************************************************************************************
 * MQTT
 *****************************************************************************************************/


/**
 * Function pointer of MQTT Connected callback
 */
static Mqtt_ConnectedCallback mqtt_connected_cbk = NULL;

/**
 * Function pointer of MQTT Idle callback
 */
static Mqtt_IdleCallback      mqtt_idle_cbk      = NULL;


/**
 * Defined in TGRMqttClient.c
 */
extern uint32_t g_publishing_counnter;

/**
 * Sets mqtt connected callback function
 */
void Mqtt_SetConnectedCallback(Mqtt_ConnectedCallback callback) {
	mqtt_connected_cbk = callback;
}


/**
 * Sets mqtt idle callback function
 */
void Mqtt_SetIdleCallback(Mqtt_IdleCallback callback) {
	mqtt_idle_cbk = callback;
}


/**
 * Performs mqtt connected callback
 * PRIVATE: Called by the TGRMqttClient.c
 */
void Mqtt_PerformConnectedCallback(MQTTClient *client, const char *clientId) {
	if(mqtt_connected_cbk != NULL) {
		mqtt_connected_cbk(client, clientId);
	}
}

/**
 * Performs mqtt idle callback
 * PRIVATE: Called by the TGRMqttClient.c
 */
void Mqtt_PerformIdleCallback(MQTTClient *client) {
	if(mqtt_idle_cbk != NULL) {
		mqtt_idle_cbk(client);
	}
}


/**
 * Subscribes to a desired topic and adds received callback
 */
int Mqtt_Subscribe(MQTTClient *client, const char* topic, Mqtt_ReceivedCallback callback) {
	return MQTTSubscribe(client, topic, QOS0, (callback));
}


/**
 * Unscribes from desired topic
 */
int Mqtt_Unsubscribe(MQTTClient *client, const char* topic) {
	return MQTTUnsubscribe(client, topic);
}


/**
 * Returns true if the mqtt publisher is ready.
 * This function must be callbed to check the publisher status before peforming mqtt publishing
 */
bool Mqtt_CanPublish(void) {
	return g_publishing_counnter <= 0;
}


/**
 * Publishes to target topic
 */
int Mqtt_Publish(MQTTClient *client, const char* topic, const char *message) {

	if(g_publishing_counnter > 0) {
		msg_error("\n** MQTT Publishing Error. The publisher is in progress **\n");
		return FAILURE;
	}

    int rc;
    MQTTMessage mqmsg;
    memset(&mqmsg, 0, sizeof(MQTTMessage));
    mqmsg.qos = QOS0;
    mqmsg.payload = (char*)message;
    mqmsg.payloadlen = strlen(message);

    rc = MQTTPublish(client, topic, &mqmsg);
    if (rc != MQSUCCESS)
    {
        msg_error("Failed publishing %s on %s\n", (char*)(mqmsg.payload), topic);
    }

    int ret = MQTTYield(client, 50);
	if (ret != MQSUCCESS) {
		msg_error("Mqtt_Publish() Yield failed\n");
	}

	g_publishing_counnter++;

    return rc;
}



#endif /* APPLICATION_GENERICMQTT_TGRMQTT_C_ */
