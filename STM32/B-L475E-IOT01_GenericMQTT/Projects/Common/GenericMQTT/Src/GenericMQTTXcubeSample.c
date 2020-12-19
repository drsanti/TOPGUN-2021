/**
  ******************************************************************************
  * @file    GenericMQTTXcubeSample.c
  * @author  MCD Application Team
  * @brief   Generic MQTT IoT connection example.
  *          Basic telemetry on sensor-equipped boards.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V.
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other
  *    contributors to this software may be used to endorse or promote products
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under
  *    this license is void and will automatically terminate your rights under
  *    this license.
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "iot_flash_config.h"
#include "msg.h"
#include "timingSystem.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "main.h"
#include "MQTTClient.h"
#include "cJSON.h"
#include "GenericMQTTXCubeSample.h"


extern void mqtt_connected_to_server(void *params);
extern void mqtt_on_message_received(const char* topic, const char *message);

/* The init/deinit netif functions are called from cloud.c.
 * However, the application needs to reinit whenever the connectivity seems to be broken. */
extern int net_if_reinit(void* if_ctxt);


/* Private define ------------------------------------------------------------*/
#define MODEL_MAC_SIZE                    13
#define MODEL_DEFAULT_MAC                 "0102030405"
#define MODEL_DEFAULT_LEDON               false
#define MODEL_DEFAULT_TELEMETRYINTERVAL   5 /* 15 */
/* Uncomment one of the below defines to use an alternative MQTT service */
/* #define LITMUS_LOOP */
/* #define UBIDOTS_MQTT */

#ifdef LITMUS_LOOP
#define MQTT_SEND_BUFFER_SIZE             1500  /* Has to be large enough for the message size plus the topic name size */
#else
#define MQTT_SEND_BUFFER_SIZE             600
#endif /* LITMUS_LOOP */
#define MQTT_READ_BUFFER_SIZE             600
#define MQTT_CMD_TIMEOUT                  5000
#define MAX_SOCKET_ERRORS_BEFORE_NETIF_RESET  3

#define MQTT_TOPIC_BUFFER_SIZE            100  /**< Maximum length of the application-defined topic names. */
#define MQTT_MSG_BUFFER_SIZE              MQTT_SEND_BUFFER_SIZE /**< Maximum length of the application-defined MQTT messages. */

/* Private typedef -----------------------------------------------------------*/
typedef struct {
    char      mac[MODEL_MAC_SIZE];      /*< To be read from the netif */
    uint32_t  ts;           /*< Tick count since MCU boot. */
#ifdef SENSOR
    int16_t   ACC_Value[3]; /*< Accelerometer */
    float     GYR_Value[3]; /*< Gyroscope */
    int16_t   MAG_Value[3]; /*< Magnetometer */
    float     temperature;
    float     humidity;
    float     pressure;
    int32_t   proximity;
#endif /* SENSOR */
} pub_data_t;

typedef struct {
    char      mac[MODEL_MAC_SIZE];      /*< To be read from the netif */
    bool      LedOn;
    uint32_t  TelemetryInterval;
} status_data_t;

typedef struct {
    char* HostName;
    char* HostPort;
    char* ConnSecurity;
    char* MQClientId;
    char* MQUserName;
    char* MQUserPwd;
#ifdef LITMUS_LOOP
    char* LoopTopicId;
#endif
} device_config_t;

/* Private macro -------------------------------------------------------------*/
#define MIN(a,b)  (((a) < (b)) ? (a) : (b))

/* Private variables ---------------------------------------------------------*/
static bool g_continueRunning;
static bool g_publishData;
static bool g_statusChanged;
static bool g_reboot;
static int g_connection_needed_score;

pub_data_t pub_data = { MODEL_DEFAULT_MAC, 0 };
status_data_t status_data = { MODEL_DEFAULT_MAC, MODEL_DEFAULT_LEDON, MODEL_DEFAULT_TELEMETRYINTERVAL };

/* Warning: The subscribed topics names strings must be allocated separately,
 * because Paho does not copy them and uses references to dispatch the incoming message. */
static char mqtt_subtopic[MQTT_TOPIC_BUFFER_SIZE];
static char mqtt_pubtopic[MQTT_TOPIC_BUFFER_SIZE];
static char mqtt_msg[MQTT_MSG_BUFFER_SIZE];

/* Private function prototypes -----------------------------------------------*/
void genericmqtt_client_XCube_sample_run(void);
int stiot_publish(void* mqtt_ctxt, const char* topic, const char* msg);
int32_t comp_left_ms(uint32_t init, uint32_t now, uint32_t timeout);
int network_read(Network* n, unsigned char* buffer, int len, int timeout_ms);
int network_write(Network* n, unsigned char* buffer, int len, int timeout_ms);
void allpurposeMessageHandler(MessageData* data);

int parse_and_fill_device_config(device_config_t** pConfig, const char* string);
void free_device_config(device_config_t* config);
int string_allocate_from_token(char** pDestString, char* tokenName, const char* sourceString);

/* Exported functions --------------------------------------------------------*/

int cloud_device_enter_credentials(void)
{
    iot_config_t iot_config;
    int ret = 0;

    memset(&iot_config, 0, sizeof(iot_config_t));

    printf("\nEnter the connection string of your device:\n"
#ifdef LITMUS_LOOP
        "template for MQTT plain TCP connection:                                          HostName=xxx;HostPort=xxx;ConnSecurity=0;MQClientId=xxx;MQUserName=xxx;MQUserPwd=xxx;LoopTopicId=xxx;\n"
        "template for MQTT TLS/SSL connection, without Litmus Loop server authentication: HostName=xxx;HostPort=xxx;ConnSecurity=1;MQClientId=xxx;MQUserName=xxx;MQUserPwd=xxx;LoopTopicId=xxx;\n"
        "template for MQTT TLS/SSL connection, with Litmus Loop server authentication:    HostName=xxx;HostPort=xxx;ConnSecurity=2;MQClientId=xxx;MQUserName=xxx;MQUserPwd=xxx;LoopTopicId=xxx;\n");
#else
        "template with MQTT authentication:    HostName=xxx;HostPort=xxx;ConnSecurity=x;MQClientId=xxx;MQUserName=xxx;MQUserPwd=xxx;\n"
        "template without MQTT authentication: HostName=xxx;HostPort=xxx;ConnSecurity=x;MQClientId=xxx;\n");
#endif /* LITMUS_LOOP */

    getInputString(iot_config.device_name, USER_CONF_DEVICE_NAME_LENGTH);
    msg_info("read: --->\n%s\n<---\n", iot_config.device_name);

    if (setIoTDeviceConfig(&iot_config) != 0)
    {
        ret = -1;
        msg_error("Failed programming the IoT device configuration to Flash.\n");
    }

    return ret;
}

bool app_needs_device_keypair()
{
    const char* config_string = NULL;
    device_config_t* device_config = NULL;
    conn_sec_t security = CONN_SEC_UNDEFINED;

    if (getIoTDeviceConfig(&config_string) != 0)
    {
        msg_error("Failed retrieving the device configuration string.\n");
    }
    else
    {
        if (parse_and_fill_device_config(&device_config, config_string) == 0)
        {
            security = (conn_sec_t)atoi(device_config->ConnSecurity);
            free_device_config(device_config);
        }
        else
        {
            msg_error("Could not parse the connection security settings from the configuration string.\n");
        }
    }

    return (security == CONN_SEC_MUTUALAUTH) ? true : false;
}

/** Function to read data from the socket opened into provided buffer
 * @param - Address of Network Structure
 *        - Buffer to store the data read from socket
 *        - Expected number of bytes to read from socket
 *        - Timeout in milliseconds
 * @return - Number of Bytes read on SUCCESS
 *         - -1 on FAILURE
 **/
int network_read(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
    int bytes;

    bytes = net_sock_recv((net_sockhnd_t)n->my_socket, buffer, len);
    if (bytes < 0)
    {
        msg_error("net_sock_recv failed - %d\n", bytes);
        bytes = -1;
    }

    return bytes;
}

/** Function to write data to the socket opened present in provided buffer
 * @param - Address of Network Structure
 *        - Buffer storing the data to write to socket
 *        - Number of bytes of data to write to socket
 *        - Timeout in milliseconds
 * @return - Number of Bytes written on SUCCESS
 *         - -1 on FAILURE
 **/
int network_write(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
    int rc;

    rc = net_sock_send((net_sockhnd_t)n->my_socket, buffer, len);
    if (rc < 0)
    {
        msg_error("net_sock_send failed - %d\n", rc);
        rc = -1;
    }

    return rc;
}

/** Message callback
 *
 *  Note: No context handle is passed by the callback. Must rely on static variables.
 *        TODO: Maybe store couples of hander/contextHanders so that the context could
 *              be retrieved from the handler address. */
void allpurposeMessageHandler(MessageData* data)
{

	/**
	 * Print
	 */
	snprintf(mqtt_msg, MIN(MQTT_MSG_BUFFER_SIZE, data->message->payloadlen + 1), "%s", (char *)data->message->payload);
	msg_info("\nReceived message: topic: %.*s content: %s.\n", data->topicName->lenstring.len, data->topicName->lenstring.data, mqtt_msg);



	cJSON *json = NULL;
	cJSON *root = cJSON_Parse(mqtt_msg);

	json = cJSON_GetObjectItemCaseSensitive(root, "TelemetryInterval");
	if (json != NULL) {
		if (cJSON_IsNumber(json) == true)  {
			status_data.TelemetryInterval = json->valueint;
			g_statusChanged = true;
		}
		else  {
			msg_error("JSON parsing error of TelemetryInterval value.\n");
		}
	}

	json = cJSON_GetObjectItemCaseSensitive(root, "Reboot");
	if (json != NULL) {
		if (cJSON_IsBool(json) == true) {
			g_reboot = (cJSON_IsTrue(json) == true);
		}
		else {
			msg_error("JSON parsing error of Reboot value.\n");
		}
	}

	cJSON_Delete(root);
}




/** Main loop */
void genericmqtt_client_XCube_sample_run(void)
{
    int ret = 0;
    bool b_mqtt_connected = false;
    const char* connectionString = NULL;
    device_config_t* device_config = NULL;
    conn_sec_t connection_security = CONN_SEC_UNDEFINED;
    const char* ca_cert = NULL;
    const char* device_cert = NULL;
    const char* device_key = NULL;

    MQTTClient client;
    Network network;
    static unsigned char mqtt_send_buffer[MQTT_SEND_BUFFER_SIZE];
    static unsigned char mqtt_read_buffer[MQTT_READ_BUFFER_SIZE];

    g_continueRunning = true;
    g_publishData = false;
    g_statusChanged = true;
    g_reboot = false;
    g_connection_needed_score = 1;

    memset(&pub_data, 0, sizeof(pub_data));

    ret = platform_init();

    if (ret != 0) {
        msg_error("Failed to initialize the platform.\n");
    }
    else {
        ret = ( getIoTDeviceConfig(&connectionString ) != 0);
        ret |= (parse_and_fill_device_config(&device_config, connectionString) != 0);

        connection_security = (conn_sec_t)atoi(device_config->ConnSecurity);
    }

    if (ret != 0) {
        msg_error("Cannot retrieve the connection string from the user configuration storage.\n");
    }
    else {


    	/**
    	 * Initialize the defaults of the published messages.
    	 */
        net_macaddr_t mac = { 0 };
        if (net_get_mac_address(hnet, &mac) == NET_OK) {
        	//** snprintf(status_data.mac, MODEL_MAC_SIZE - 1, "%02X%02X%02X%02X%02X%02X", mac.mac[0], mac.mac[1], mac.mac[2], mac.mac[3], mac.mac[4], mac.mac[5]);
        	sprintf(status_data.mac, "%02X%02X%02X%02X%02X%02X", mac.mac[0], mac.mac[1], mac.mac[2], mac.mac[3], mac.mac[4], mac.mac[5]);

        }
        else {
            msg_warning("Could not retrieve the MAC address to set the device ID.\n");
            //** snprintf(status_data.mac, MODEL_MAC_SIZE - 1, "MyDevice-UnknownMAC");
            sprintf(status_data.mac, "%s", "UnknownMAC");
        }
        strncpy(pub_data.mac, status_data.mac, MODEL_MAC_SIZE - 1);





        status_data.TelemetryInterval = MODEL_DEFAULT_TELEMETRYINTERVAL;


        /**
         * Re-connection loop: Socket level, with a netIf reconnect in case of repeated socket failures.
         */
        do
        {
            /* Init MQTT client */
            net_sockhnd_t socket;
            net_ipaddr_t ip;
            int ret = 0;


            /* If the socket connection failed MAX_SOCKET_ERRORS_BEFORE_NETIF_RESET times in a row,
             * even if the netif still has a valid IP address, we assume that the network link is down
             * and must be reset.
             */
            if ((net_get_ip_address(hnet, &ip) == NET_ERR) || (g_connection_needed_score > MAX_SOCKET_ERRORS_BEFORE_NETIF_RESET)) {
                msg_info("Network link %s down. Trying to reconnect.\n", (g_connection_needed_score > MAX_SOCKET_ERRORS_BEFORE_NETIF_RESET) ? "may be" : "");
                if (net_reinit(hnet, (net_if_reinit)) != 0) {
                    msg_error("Netif re-initialization failed.\n");
                    continue;
                }
                else  {
                    msg_info("Netif re-initialized successfully.\n");
                    HAL_Delay(1000);
                    g_connection_needed_score = 1;
                }
            }

            ret = net_sock_create(hnet, &socket, (connection_security == CONN_SEC_NONE) ? NET_PROTO_TCP : NET_PROTO_TLS);
            if (ret != NET_OK) {
                msg_error("Could not create the socket.\n");
            }
            else {
                switch (connection_security) {
                case CONN_SEC_MUTUALAUTH:
                    ret |= ((checkTLSRootCA() != 0) && (checkTLSDeviceConfig() != 0)) || (getTLSKeys(&ca_cert, &device_cert, &device_key) != 0);
                    ret |= net_sock_setopt(socket, "tls_server_name", (void*)device_config->HostName, strlen(device_config->HostName) + 1);
                    ret |= net_sock_setopt(socket, "tls_ca_certs", (void*)ca_cert, strlen(ca_cert) + 1);
                    ret |= net_sock_setopt(socket, "tls_dev_cert", (void*)device_cert, strlen(device_cert) + 1);
                    ret |= net_sock_setopt(socket, "tls_dev_key", (void*)device_key, strlen(device_key) + 1);
                    break;
                case CONN_SEC_SERVERNOAUTH:
                    ret |= net_sock_setopt(socket, "tls_server_noverification", NULL, 0);
                    ret |= (checkTLSRootCA() != 0)
                        || (getTLSKeys(&ca_cert, NULL, NULL) != 0);
                    ret |= net_sock_setopt(socket, "tls_server_name", (void*)device_config->HostName, strlen(device_config->HostName) + 1);
                    ret |= net_sock_setopt(socket, "tls_ca_certs", (void*)ca_cert, strlen(ca_cert) + 1);
                    break;
                case CONN_SEC_SERVERAUTH:
                    ret |= (checkTLSRootCA() != 0)
                        || (getTLSKeys(&ca_cert, NULL, NULL) != 0);
                    ret |= net_sock_setopt(socket, "tls_server_name", (void*)device_config->HostName, strlen(device_config->HostName) + 1);
                    ret |= net_sock_setopt(socket, "tls_ca_certs", (void*)ca_cert, strlen(ca_cert) + 1);
                    break;
                case CONN_SEC_NONE:
                    break;
                default:
                    msg_error("Invalid connection security mode. - %d\n", connection_security);
                }// switch

                ret |= net_sock_setopt(socket, "sock_noblocking", NULL, 0);
            }

            if (ret != NET_OK) {
                msg_error("Could not retrieve the security connection settings and set the socket options.\n");
            }
            else {
                ret = net_sock_open(socket, device_config->HostName, atoi(device_config->HostPort), 0);
            }

            if (ret != NET_OK) {
                msg_error("Could not open the socket at %s port %d.\n", device_config->HostName, atoi(device_config->HostPort));
                g_connection_needed_score++;
                HAL_Delay(1000);
            }
            else {


            	/**
				 * Init MQTT Client
				 */
                network.my_socket = socket;
                network.mqttread  = (network_read);
                network.mqttwrite = (network_write);
                MQTTClientInit(&client, &network, MQTT_CMD_TIMEOUT, mqtt_send_buffer, MQTT_SEND_BUFFER_SIZE, mqtt_read_buffer, MQTT_READ_BUFFER_SIZE);


                /**
                 * MQTT Connect
                 */
                MQTTPacket_connectData options = MQTTPacket_connectData_initializer;
                options.clientID.cstring = device_config->MQClientId;
                options.username.cstring = device_config->MQUserName;
                options.password.cstring = device_config->MQUserPwd;
                ret = MQTTConnect(&client, &options);
                if (ret != 0) {
                    msg_error("MQTTConnect() failed: %d\n", ret);
                }
                else {

                	/**
					 * Connected
					 */
                    g_connection_needed_score = 0;
                    b_mqtt_connected = true;

#ifdef LITMUS_LOOP
                    snprintf(mqtt_subtopic, MQTT_TOPIC_BUFFER_SIZE, "loop/req/%s/json", device_config->LoopTopicId);
#elif defined(UBIDOTS_MQTT)
                    snprintf(mqtt_subtopic, MQTT_TOPIC_BUFFER_SIZE, "/v1.6/devices/%s/ts/lv", device_config->MQClientId); /* Subscribe the timestamp latest value */
#else
                    snprintf(mqtt_subtopic, MQTT_TOPIC_BUFFER_SIZE, "/devices/%s/control", device_config->MQClientId);
#endif /* LITMUS_LOOP */
                    ret = MQTTSubscribe(&client, mqtt_subtopic, QOS0, (allpurposeMessageHandler));


                    /**
                     * MQTT_UserCallback -------------------------------------------------------------------------------------------
                     */
                    mqtt_connected_to_server(&client);
                }

                /* ret = MQTTSetMessageHandler(&client, "#", (allpurposeMessageHandler)); */

                if (ret != MQSUCCESS) {
                    msg_error("Failed subscribing to the %s topic.\n", mqtt_subtopic);
                }
                else {
                    msg_info("Subscribed to %s.\n", mqtt_subtopic);
                    ret = MQTTYield(&client, 500);
                }
                if (ret != MQSUCCESS) {
                    msg_error("Yield failed.\n");
                }
                else {

#ifdef LITMUS_LOOP
                    /* Device information data update */
                    ret = snprintf(mqtt_pubtopic, sizeof(mqtt_pubtopic), "loop/data/%s/json", device_config->LoopTopicId);

                    if (ret >= 0 && ret < sizeof(mqtt_pubtopic))
                    {
                        /* Updates of Manufacturer, Model Number, Serial Number, Firmware Version */
                        ret = snprintf(mqtt_msg, sizeof(mqtt_msg), "{ \"timestamp\":0, \"values\": [ \n"
                            "{\"objectId\":3,\"instanceId\":0,\"resourceId\":0,\"datatype\":\"String\",\"value\":\"STMicroelectronics\"}, \n"
                            "{\"objectId\":3,\"instanceId\":0,\"resourceId\":1,\"datatype\":\"String\",\"value\":\"Model 1.0\"}, \n"
                            "{\"objectId\":3,\"instanceId\":0,\"resourceId\":2,\"datatype\":\"String\",\"value\":\"Serial 1\"}, \n"
                            "{\"objectId\":3,\"instanceId\":0,\"resourceId\":3,\"datatype\":\"String\",\"value\":\"Firmware 1.0\"} \n"
                            "] }");

                        if (ret >= 0 && ret < sizeof(mqtt_msg))
                        {
                            ret = stiot_publish(&client, mqtt_pubtopic, mqtt_msg);  /* Wrapper for MQTTPublish() */

                            if (ret == MQSUCCESS)
                            {
                                msg_info("publication topic: %s \tpayload: %s\n", mqtt_pubtopic, mqtt_msg);
                            }
                            else
                            {
                                msg_error("Device information publication failed.\n");
                                g_connection_needed_score++;
                            }

                        }
                        else
                        {
                            msg_error("Error formatting device information message.\n");
                        }
                    }
                    else
                    {
                        msg_error("Error formatting device information topic.\n");
                    }
#endif /* LITMUS_LOOP */


                    /**
                     * Send the telemetry data, and send the device status if it was changed by a received message.
                     */
                    uint32_t last_telemetry_time_ms = HAL_GetTick();
                    do {

                        uint8_t command = Button_WaitForMultiPush(500);

                        bool b_sample_data = (command == BP_SINGLE_PUSH); 	/* If short button push, publish once. */

                        if (command == BP_MULTIPLE_PUSH){              		/* If long button push, toggle the telemetry publication. */

                            g_publishData = !g_publishData;
                            msg_info("%s the sensor values publication loop.\n", (g_publishData == true) ? "Enter" : "Exit");
                        }

                        int32_t left_ms = comp_left_ms(last_telemetry_time_ms, HAL_GetTick(), status_data.TelemetryInterval * 1000);

                        if (((g_publishData == true) && (left_ms <= 0)) || (b_sample_data == true))
                        {
                            last_telemetry_time_ms = HAL_GetTick();

#ifdef SENSOR
                            /* Read the Data from the sensors */
                            pub_data.temperature = BSP_TSENSOR_ReadTemp();
                            pub_data.humidity = BSP_HSENSOR_ReadHumidity();
                            pub_data.pressure = BSP_PSENSOR_ReadPressure();
                            pub_data.proximity = VL53L0X_PROXIMITY_GetDistance();
                            BSP_ACCELERO_AccGetXYZ(pub_data.ACC_Value);
                            BSP_GYRO_GetXYZ(pub_data.GYR_Value);
                            BSP_MAGNETO_GetXYZ(pub_data.MAG_Value);
#endif /* SENSOR */
                            pub_data.ts = time(NULL); /* last_telemetry_time_ms; */

                            /* Create and send the message. */
                            /* Note: The state.reported object hierarchy is used to help the inter-operability with 1st tier cloud providers. */
#ifdef LITMUS_LOOP
                            ret = snprintf(mqtt_pubtopic, sizeof(mqtt_pubtopic), "loop/data/%s/json", device_config->LoopTopicId);
                            if (ret >= 0 && ret < sizeof(mqtt_pubtopic))
                            {
                                ret = snprintf(mqtt_msg, sizeof(mqtt_msg), "{ \"timestamp\":0, \"values\": [ \n"
#ifdef SENSOR
                                    "{\"objectId\":3303,\"instanceId\":0,\"resourceId\":5700,\"datatype\":\"Float\",\"value\":%.2f}, \n"
                                    "{\"objectId\":3304,\"instanceId\":0,\"resourceId\":5700,\"datatype\":\"Float\",\"value\":%.2f}, \n"
                                    "{\"objectId\":3323,\"instanceId\":0,\"resourceId\":5700,\"datatype\":\"Float\",\"value\":%.2f}, \n"
                                    "{\"objectId\":3330,\"instanceId\":0,\"resourceId\":5700,\"datatype\":\"Integer\",\"value\":%ld}, \n"
                                    "{\"objectId\":3313,\"instanceId\":0,\"resourceId\":5702,\"datatype\":\"Integer\",\"value\":%d}, \n"
                                    "{\"objectId\":3313,\"instanceId\":0,\"resourceId\":5703,\"datatype\":\"Integer\",\"value\":%d}, \n"
                                    "{\"objectId\":3313,\"instanceId\":0,\"resourceId\":5704,\"datatype\":\"Integer\",\"value\":%d}, \n"
                                    "{\"objectId\":3334,\"instanceId\":0,\"resourceId\":5702,\"datatype\":\"Integer\",\"value\":%.0f}, \n"
                                    "{\"objectId\":3334,\"instanceId\":0,\"resourceId\":5703,\"datatype\":\"Integer\",\"value\":%.0f}, \n"
                                    "{\"objectId\":3334,\"instanceId\":0,\"resourceId\":5704,\"datatype\":\"Integer\",\"value\":%.0f}, \n"
                                    "{\"objectId\":3314,\"instanceId\":0,\"resourceId\":5702,\"datatype\":\"Integer\",\"value\":%d}, \n"
                                    "{\"objectId\":3314,\"instanceId\":0,\"resourceId\":5703,\"datatype\":\"Integer\",\"value\":%d}, \n"
                                    "{\"objectId\":3314,\"instanceId\":0,\"resourceId\":5704,\"datatype\":\"Integer\",\"value\":%d}, \n"
#endif /* SENSOR */
                                    "{\"objectId\":3333,\"instanceId\":0,\"resourceId\":5506,\"datatype\":\"Integer\",\"value\":%lu} \n"
                                    "] }",
#ifdef SENSOR
                                    pub_data.temperature, pub_data.humidity, pub_data.pressure, pub_data.proximity,
                                    pub_data.ACC_Value[0], pub_data.ACC_Value[1], pub_data.ACC_Value[2],
                                    pub_data.GYR_Value[0], pub_data.GYR_Value[1], pub_data.GYR_Value[2],
                                    pub_data.MAG_Value[0], pub_data.MAG_Value[1], pub_data.MAG_Value[2],
#endif /* SENSOR */
                                    pub_data.ts
                                );

                            }
                            else
                            {
                                msg_error("Error formatting device data topic.\n");
                            }
#elif defined(UBIDOTS_MQTT)
                            snprintf(mqtt_pubtopic, MQTT_TOPIC_BUFFER_SIZE, "/v1.6/devices/%s", device_config->MQClientId);
                            ret = snprintf(mqtt_msg, MQTT_MSG_BUFFER_SIZE, "{\n"
#ifdef SENSOR
                                "   \"temperature\": %.2f,\n   \"humidity\": %.2f,\n   \"pressure\": %.2f,\n   \"proximity\": %ld,\n"
                                "   \"acc_x\": %d, \"acc_y\": %d, \"acc_z\": %d,\n"
                                "   \"gyr_x\": %.0f, \"gyr_y\": %.0f, \"gyr_z\": %.0f,\n"
                                "   \"mag_x\": %d, \"mag_y\": %d, \"mag_z\": %d,\n"
#endif /* SENSOR */
                                "   \"ts\":  %lu\n"
                                "}"
#ifdef SENSOR
                                , pub_data.temperature, pub_data.humidity, pub_data.pressure, pub_data.proximity
                                , pub_data.ACC_Value[0], pub_data.ACC_Value[1], pub_data.ACC_Value[2]
                                , pub_data.GYR_Value[0], pub_data.GYR_Value[1], pub_data.GYR_Value[2]
                                , pub_data.MAG_Value[0], pub_data.MAG_Value[1], pub_data.MAG_Value[2]
#endif  /* SENSOR */
                                , pub_data.ts
                            );

#else /* ! LITMUS_LOOP */
                            snprintf(mqtt_pubtopic, MQTT_TOPIC_BUFFER_SIZE, "/sensors/%s", device_config->MQClientId);
                            ret = snprintf(mqtt_msg, MQTT_MSG_BUFFER_SIZE, "{\n \"state\": {\n  \"reported\": {\n"

#ifdef SENSOR
                                "   \"temperature\": %.2f,\n   \"humidity\": %.2f,\n   \"pressure\": %.2f,\n   \"proximity\": %ld,\n"
                                "   \"acc_x\": %d, \"acc_y\": %d, \"acc_z\": %d,\n"
                                "   \"gyr_x\": %.0f, \"gyr_y\": %.0f, \"gyr_z\": %.0f,\n"
                                "   \"mag_x\": %d, \"mag_y\": %d, \"mag_z\": %d,\n"
#endif /* SENSOR */
                                "   \"ts\": %lu, \"mac\": \"%s\", \"devId\": \"%s\"\n"
                                "  }\n }\n}",

#ifdef SENSOR
                                pub_data.temperature, pub_data.humidity, pub_data.pressure, pub_data.proximity,
                                pub_data.ACC_Value[0], pub_data.ACC_Value[1], pub_data.ACC_Value[2],
                                pub_data.GYR_Value[0], pub_data.GYR_Value[1], pub_data.GYR_Value[2],
                                pub_data.MAG_Value[0], pub_data.MAG_Value[1], pub_data.MAG_Value[2],
#endif /* SENSOR */
                                pub_data.ts, pub_data.mac, device_config->MQClientId);

#endif /* LITMUS_LOOP */

                            if ((ret < 0) || (ret >= MQTT_MSG_BUFFER_SIZE)) {
                                msg_error("Telemetry message formatting error.\n");
                            }
                            else {

                            	/**
                            	 * MQTTPublish()
                            	 */
                                ret = stiot_publish(&client, mqtt_pubtopic, mqtt_msg);
                                if (ret == MQSUCCESS)  {

                                    /* Visual notification of the telemetry publication: LED blink. */
                                	Led_Blink(50, 25, 5);

                                    msg_info("#\n");
                                    msg_info("publication topic: %s \npayload: %s\n", mqtt_pubtopic, mqtt_msg);
                                }
                                else  {
                                    msg_error("Telemetry publication failed.\n");
                                    g_connection_needed_score++;
                                }

                                ret = MQTTYield(&client, 500);
                                if (ret != MQSUCCESS)  {
                                    msg_error("Yield failed. Reconnection needed?.\n");
                                    /* g_connection_needed_score++; */
                                }
                            }
                        }

#ifndef UBIDOTS_MQTT
                        /* Publish the updated device status */
                        if (g_statusChanged)
                        {
#ifdef LITMUS_LOOP
                            snprintf(mqtt_pubtopic, MQTT_TOPIC_BUFFER_SIZE, "loop/resp/%s/json", device_config->LoopTopicId);
#else
                            snprintf(mqtt_pubtopic, MQTT_TOPIC_BUFFER_SIZE, "/devices/%s/status", device_config->MQClientId);
#endif

                            uint32_t ts = time(NULL); /* last_telemetry_time_ms; */

                            ret = snprintf(mqtt_msg, MQTT_MSG_BUFFER_SIZE, "{\n \"state\": {\n  \"reported\": {\n"
                                "   \"TelemetryInterval\": %d,\n"
                                "   \"ts\": %ld, \"mac\": \"%s\", \"devId\": \"%s\"\n"
                                "  }\n }\n}",
                                (int)status_data.TelemetryInterval,
                                ts,
                                pub_data.mac,
                                device_config->MQClientId);


                            if ((ret < 0) || (ret >= MQTT_MSG_BUFFER_SIZE)) {
                                msg_error("Telemetry message formatting error.\n");
                            }
                            else {
                            	/**
                            	 *   MQTTPublish()
                            	 */
                                ret = stiot_publish(&client, mqtt_pubtopic, mqtt_msg);
                                if (ret != MQSUCCESS)  {
                                    msg_error("Status publication failed.\n");
                                    g_connection_needed_score++;
                                }
                                else {
                                    msg_info("publication topic: %s \npayload: %s\n", mqtt_pubtopic, mqtt_msg);
                                    g_statusChanged = false;
                                }
                            }
                        }
#else /* ! UBIDOTS_MQTT */
                        (void)g_statusChanged;
#endif /* ! UBIDOTS_MQTT */

                        ret = MQTTYield(&client, 500);
                        if (ret != MQSUCCESS) {
                            msg_error("Yield failed. Reconnection needed.\n");
                            g_connection_needed_score++;
                        }
                        else {
                            msg_info(".");
                        }
                    } while (g_continueRunning && !g_reboot && (g_connection_needed_score == 0));

                }


                /* The publication loop is exited.
                   NB: No need to unsubscribe as we are disconnecting.
                   NB: MQTTDisconnect() will raise additional error logs if the network link is already broken,
                       but it is the only way to clean the MQTT session. */
                if (b_mqtt_connected == true) {
                    ret = MQTTDisconnect(&client);
                    if (ret != MQSUCCESS) {
                        msg_error("MQTTDisconnect() failed.\n");
                    }
                    b_mqtt_connected = false;
                }
                if (NET_OK != net_sock_close(socket)) {
                    msg_error("net_sock_close() failed.\n");
                }
            }

            if (NET_OK != net_sock_destroy(socket)){
                msg_error("net_sock_destroy() failed.\n");
            }
        } while (!g_reboot && (g_connection_needed_score > 0));
    }

    free_device_config(device_config);
    platform_deinit();

    if (g_reboot == true) {
        msg_info("Calling HAL_NVIC_SystemReset()\n");
        HAL_NVIC_SystemReset();
    }
}



/**
 * MQTT publish API abstraction called by the metering loop.
 */
int stiot_publish(void* mqtt_ctxt, const char* topic, const char* msg)
{
    int rc;
    MQTTMessage mqmsg;
    memset(&mqmsg, 0, sizeof(MQTTMessage));
    mqmsg.qos = QOS0;
    mqmsg.payload = (char*)msg;
    mqmsg.payloadlen = strlen(msg);

    rc = MQTTPublish(mqtt_ctxt, topic, &mqmsg);
    if (rc != MQSUCCESS)
    {
        msg_error("Failed publishing %s on %s\n", (char*)(mqmsg.payload), topic);
    }
    return rc;
}


/** Look for a 'key=value' pair in the passed configuration string, and return a new buffer
 *  holding the 'value' field.
 */
int string_allocate_from_token(char** pDestString, char* tokenName, const char* sourceString)
{
    int ret = 0;
    char* key = NULL;
    char* value = NULL;

    if ((key = strstr(sourceString, tokenName)) != NULL)
    {
        int size = 0;
        value = key + strlen(tokenName);    /* '=' key=value separator is part of tokenName. */
        if ((key = strstr(value, ";")) != NULL)
        {
            size = key - value;
        }
        *pDestString = malloc(size + 1);
        if (*pDestString != NULL)
        {
            memcpy(*pDestString, value, size);
            (*pDestString)[size] = '\0';
        }
        else
        {
            msg_error("Allocation failed\n");
        }
    }

    return ret;
}


/** Allocate and return a device_config_t structure initialized with the values defined by the passed
 *  configuration string.
 *  The buffers holding the structure and those fields are allocated dynamically by the callee, and
 *  must be freed after usage by free_device_config().
 */
int parse_and_fill_device_config(device_config_t** pConfig, const char* string)
{
    int ret = -1;
    device_config_t* config = NULL;

    if (strlen(string) > USER_CONF_DEVICE_NAME_LENGTH)
    {
        msg_error("Cannot parse the configuration string:  It is not null-terminated!\n");
    }
    else
    {
        if (pConfig == NULL)
        {
            msg_error("Null parameter\n");
        }
        else
        {
            config = malloc(sizeof(device_config_t));
            memset(config, 0, sizeof(device_config_t));

            ret = string_allocate_from_token(&config->HostName, "HostName=", string);
            ret |= string_allocate_from_token(&config->HostPort, "HostPort=", string);
            ret |= string_allocate_from_token(&config->ConnSecurity, "ConnSecurity=", string);
            ret |= string_allocate_from_token(&config->MQClientId, "MQClientId=", string);
            ret |= string_allocate_from_token(&config->MQUserName, "MQUserName=", string);
            ret |= string_allocate_from_token(&config->MQUserPwd, "MQUserPwd=", string);
#ifdef LITMUS_LOOP
            ret |= string_allocate_from_token(&config->LoopTopicId, "LoopTopicId=", string);
#endif

            if (ret != 0)
            {
                msg_error("Failed parsing the device configuration string.\n");
                free_device_config(config);
            }
            else
            {
                *pConfig = config;
                ret = 0;
            }
        }
    }

    return ret;
}


/** Free a device_config_t allocated by parse_and_fill_device_config().
 */
void free_device_config(device_config_t* config)
{
    if (config != NULL)
    {
        if (config->HostName != NULL) free(config->HostName);
        if (config->HostPort != NULL) free(config->HostPort);
        if (config->ConnSecurity != NULL) free(config->ConnSecurity);
        if (config->MQClientId != NULL) free(config->MQClientId);
        if (config->MQUserName != NULL) free(config->MQUserName);
        if (config->MQUserPwd != NULL) free(config->MQUserPwd);
#ifdef LITMUS_LOOP
        if (config->LoopTopicId != NULL) free(config->LoopTopicId);
#endif

        free(config);
    }
    else
    {
        msg_warning("Attemped to free a non-allocated config structure.\n");
    }
}


/**
 * @brief   Return the integer difference between 'init + timeout' and 'now'.
 *          The implementation is robust to uint32_t overflows.
 * @param   In:   init      Reference index.
 * @param   In:   now       Current index.
 * @param   In:   timeout   Target index.
 * @retval  Number of units from now to target.
 */
int32_t comp_left_ms(uint32_t init, uint32_t now, uint32_t timeout)
{
    int32_t ret = 0;
    uint32_t wrap_end = 0;

    if (now < init)
    { /* Timer wrap-around detected */
      /* printf("Timer: wrap-around detected from %d to %d\n", init, now); */
        wrap_end = UINT32_MAX - init;
    }
    ret = wrap_end - (now - init) + timeout;

    return ret;
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
