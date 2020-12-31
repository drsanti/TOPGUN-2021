/*****************************************************************************************************
 * File Name:	TGRMqtt.h
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

#ifndef __ECCLAB_TGR_MQTT__H__
#define __ECCLAB_TGR_MQTT__H__

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <msg.h>
#include <MQTTClient.h>

/*****************************************************************************************************
 * MQTT
 *****************************************************************************************************/


/**
 * Function pointer of mqtt connected
 */
typedef void (*Mqtt_ConnectedCallback)(MQTTClient *client, const char *deviceId);

/**
 * Function pointer of mqtt data received
 */
typedef void (*Mqtt_ReceivedCallback)(MessageData* data);

/**
 * Function pointer of mqtt idle
 */
typedef void (*Mqtt_IdleCallback)(MQTTClient *client);


/**
 * Sets mqtt connected callback function
 */
void Mqtt_SetConnectedCallback(Mqtt_ConnectedCallback callback);


/**
 * Sets mqtt idle callback function
 */
void Mqtt_SetIdleCallback(Mqtt_IdleCallback callback);


/**
 * Performs mqtt connected callback
 * PRIVATE: Called by the TGRMqttClient.c
 */
void Mqtt_PerformConnectedCallback(MQTTClient *client, const char *clientId);


/**
 * Performs mqtt idle callback
 * PRIVATE: Called by the TGRMqttClient.c
 */
void Mqtt_PerformIdleCallback(MQTTClient *client);


/**
 * Subscribes to a desired topic and adds received callback
 */
int Mqtt_Subscribe(MQTTClient *client, const char* topic, Mqtt_ReceivedCallback callback);


/**
 * Unscribes from desired topic
 */
int Mqtt_Unsubscribe(MQTTClient *client, const char* topic);


/**
 * Returns true if the mqtt publisher is ready.
 * This function must be callbed to check the publisher status before peforming mqtt publishing
 */
bool Mqtt_CanPublish(void);


/**
 * Publishes to target topic
 */
int Mqtt_Publish(MQTTClient *client, const char* topic, const char *message);



#endif /* __ECCLAB_TGR_MQTT__H__ */
