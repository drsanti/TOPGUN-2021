/*****************************************************************************************************
 * File Name:	TGRHelpers.c
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

/*******************************************************************************
 * Include Files
 *******************************************************************************/
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

#include <TGRHelpers.h>
#include <TGRMqtt.h>


extern void mqtt_connected_to_server(void *params);
extern void mqtt_on_message_received(const char* topic, const char *message);

/** The init/deinit netif functions are called from cloud.c.
 * However, the application needs to reinit whenever the connectivity seems to be broken.
 */
extern int net_if_reinit(void* if_ctxt);


/*******************************************************************************
 * Private Define
 *******************************************************************************/
#define MODEL_MAC_SIZE                    13
#define MODEL_DEFAULT_MAC                 "0102030405"
#define MODEL_DEFAULT_LEDON               false
#define MODEL_DEFAULT_TELEMETRYINTERVAL   5 /* 15 */
#define MQTT_SEND_BUFFER_SIZE             600
#define MQTT_READ_BUFFER_SIZE             600
#define MQTT_CMD_TIMEOUT                  5000
#define MAX_SOCKET_ERRORS_BEFORE_NETIF_RESET  3
#define MQTT_TOPIC_BUFFER_SIZE            100  					// Maximum length of the application-defined topic names
#define MQTT_MSG_BUFFER_SIZE              MQTT_SEND_BUFFER_SIZE // Maximum length of the application-defined MQTT messages

/*******************************************************************************
 * Private Typedef
 *******************************************************************************/
typedef struct {
    char      mac[MODEL_MAC_SIZE];      // To be read from the netif
    uint32_t  ts;           			// Tick count since MCU boot
#ifdef SENSOR
    int16_t   ACC_Value[3]; 			//Accelerometer
    float     GYR_Value[3]; 			//Gyroscope
    int16_t   MAG_Value[3]; 			//Magnetometer
    float     temperature;
    float     humidity;
    float     pressure;
    int32_t   proximity;
#endif // SENSOR
} pub_data_t;


typedef struct {
    char* HostName;
    char* HostPort;
    char* ConnSecurity;
    char* MQClientId;
    char* MQUserName;
    char* MQUserPwd;
} device_config_t;

/*******************************************************************************
 * Private Macro
 *******************************************************************************/
#define MIN(a,b)  (((a) < (b)) ? (a) : (b))


/*******************************************************************************
 * Private Variables
 *******************************************************************************/
static bool g_continueRunning;
static bool g_reboot;
static int g_connection_needed_score;

static uint32_t g_telemetry_interval = MODEL_DEFAULT_TELEMETRYINTERVAL;
static char mac_addr[MODEL_MAC_SIZE];

pub_data_t pub_data = { MODEL_DEFAULT_MAC, 0 };

/**
 * Publishing counter. Increased when MQTTPublish() is called.
 */
uint32_t g_publishing_counnter = 0;

/**
 * Warning: The subscribed topics names strings must be allocated separately,
 * because Paho does not copy them and uses references to dispatch the incoming message.
 */
static char mqtt_subtopic[MQTT_TOPIC_BUFFER_SIZE]; 	// SUB
static char mqtt_pubtopic[MQTT_TOPIC_BUFFER_SIZE];	// PUB
static char mqtt_msg[MQTT_MSG_BUFFER_SIZE];			// MSG


/*******************************************************************************
 * Private Function Prototypes
 *******************************************************************************/
void GenericMQTT_Client_Run(void *pvParameters);

int stiot_publish(void* mqtt_ctxt, const char* topic, const char* msg);

int32_t comp_left_ms(uint32_t init, uint32_t now, uint32_t timeout);

int network_read(Network* n, unsigned char* buffer, int len, int timeout_ms);

int network_write(Network* n, unsigned char* buffer, int len, int timeout_ms);

void allpurposeMessageHandler(MessageData* data);

int parse_and_fill_device_config(device_config_t** pConfig, const char* string);

void free_device_config(device_config_t* config);

int string_allocate_from_token(char** pDestString, char* tokenName, const char* sourceString);


/*******************************************************************************
 * Exported Functions
 *******************************************************************************/

int cloud_device_enter_credentials(void)
{
    iot_config_t iot_config;
    int ret = 0;

    memset(&iot_config, 0, sizeof(iot_config_t));

    printf("\nEnter the connection string of your device:\n"
        "template with MQTT authentication:    HostName=xxx;HostPort=xxx;ConnSecurity=x;MQClientId=xxx;MQUserName=xxx;MQUserPwd=xxx;\n"
        "template without MQTT authentication: HostName=xxx;HostPort=xxx;ConnSecurity=x;MQClientId=xxx;\n");


    getInputString(iot_config.device_name, USER_CONF_DEVICE_NAME_LENGTH);
    msg_info("read: --->\n%s\n<---\n", iot_config.device_name);

    if (setIoTDeviceConfig(&iot_config) != 0)
    {
        ret = -1;
        msg_error("Failed programming the IoT device configuration to Flash.\n");
    }

    return ret;
}

bool app_needs_device_keypair(void)
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


/*******************************************************************************
 * Private Functions
 *******************************************************************************/

/**
 * Function to read data from the socket opened into provided buffer
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

/**
 * Function to write data to the socket opened present in provided buffer
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






/*****************************************************************************************************
 * MQTT DEFAULT TOPICS
 *****************************************************************************************************/
#define MQTT_ACTUATORS_TOPIC 	"/actuators"
#define MQTT_SENSORS_TOPIC 		"/sensors"


/*****************************************************************************************************
 * MQTT MAIN LOOP CONTROL FLAGS (DEFINED IN TGRHELPER.C)
 *****************************************************************************************************/
extern bool  g_periodic_publish_flag;
extern bool  g_single_publish_flag;
extern bool  g_actuators_publish_flag;




/**
 * Default MQTT Message Received callback
 *
 *  Note: No context handle is passed by the callback. Must rely on static variables.
 *        TODO: Maybe store couples of hander/contextHanders so that the context could
 *              be retrieved from the handler address.
 */
void allpurposeMessageHandler(MessageData* data)
{

	/**
	 * Print
	 */
	snprintf(mqtt_msg, MIN(MQTT_MSG_BUFFER_SIZE, data->message->payloadlen + 1), "%s", (char *)data->message->payload);
	msg_info("\nReceived message: topic: %.*s content: %s.\n", data->topicName->lenstring.len, data->topicName->lenstring.data, mqtt_msg);


	/**
	 * Process received message
	 */
	g_actuators_publish_flag = Ctl_ProcessMessage(mqtt_msg);


	/**
	 * JSON
	 */
	cJSON *json = NULL;
	cJSON *root = cJSON_Parse(mqtt_msg);



	/**
	 * Key: TelemetryInterval
	 * Val: int
	 */
	json = cJSON_GetObjectItemCaseSensitive(root, "TelemetryInterval");
	if (json != NULL) {
		if (cJSON_IsNumber(json) == true)  {
			g_telemetry_interval = json->valueint;
			g_actuators_publish_flag = true;
		}
		else  {
			msg_error("JSON parsing error of TelemetryInterval value.\n");
		}
	}


	/**
	 * Key: Reboot
	 * Val: true
	 */
	json = cJSON_GetObjectItemCaseSensitive(root, "Reboot");
	if (json != NULL) {
		if (cJSON_IsBool(json) == true) {
			g_reboot = (cJSON_IsTrue(json) == true);
		}
		else {
			msg_error("JSON parsing error of Reboot value.\n");
		}
	}

	/**
	 * Cleanup
	 */
	cJSON_Delete(root);

}


/**
 * Creates sensors data
 */
static int __helper_create_sensors_data(device_config_t* device_config) {

	/* Read the Data from the sensors */
	pub_data.temperature = BSP_TSENSOR_ReadTemp();
	pub_data.humidity 	 = BSP_HSENSOR_ReadHumidity();
	pub_data.pressure 	 = BSP_PSENSOR_ReadPressure();
	pub_data.proximity 	 = VL53L0X_PROXIMITY_GetDistance();
	BSP_ACCELERO_AccGetXYZ(pub_data.ACC_Value);
	BSP_GYRO_GetXYZ(pub_data.GYR_Value);
	BSP_MAGNETO_GetXYZ(pub_data.MAG_Value);
	pub_data.ts = time(NULL);

	snprintf(mqtt_pubtopic, MQTT_TOPIC_BUFFER_SIZE, "%s/%s", MQTT_SENSORS_TOPIC, device_config->MQClientId);

	return snprintf(mqtt_msg, MQTT_MSG_BUFFER_SIZE, "{\n \"state\": {\n  \"reported\": {\n"
						"   \"temperature\": %.2f,\n   \"humidity\": %.2f,\n   \"pressure\": %.2f,\n   \"proximity\": %ld,\n"
						"   \"acc_x\": %d, \"acc_y\": %d, \"acc_z\": %d,\n"
						"   \"gyr_x\": %.0f, \"gyr_y\": %.0f, \"gyr_z\": %.0f,\n"
						"   \"mag_x\": %d, \"mag_y\": %d, \"mag_z\": %d,\n"
						"   \"ts\": %lu, \"mac\": \"%s\", \"devId\": \"%s\"\n"
						"  }\n }\n}",
						pub_data.temperature, pub_data.humidity, pub_data.pressure, pub_data.proximity,
						pub_data.ACC_Value[0], pub_data.ACC_Value[1], pub_data.ACC_Value[2],
						pub_data.GYR_Value[0], pub_data.GYR_Value[1], pub_data.GYR_Value[2],
						pub_data.MAG_Value[0], pub_data.MAG_Value[1], pub_data.MAG_Value[2],
						pub_data.ts, pub_data.mac, device_config->MQClientId);
}


/**
 * Creates actuators data
 */
static int __helper_create_actuators_data(device_config_t* device_config) {


    /**
     * Bits to binary (string format)
     */
    char bits[32];

    Ctl_GetBinaryString(bits);

    int value = Ctl_GetDecimalValue();

    uint32_t ts = time(NULL);

    snprintf(mqtt_pubtopic, MQTT_TOPIC_BUFFER_SIZE, "%s/%s/status", MQTT_ACTUATORS_TOPIC, device_config->MQClientId);

    return snprintf(mqtt_msg, MQTT_MSG_BUFFER_SIZE, "{\n \"state\": {\n  \"reported\": {\n"
						"   \"DigitalOutputs\": \"%s %d 0x%.3X\",\n"
						"   \"TelemetryInterval\": %d,\n"
						"   \"ts\": %ld, \"mac\": \"%s\", \"devId\": \"%s\"\n"
						"  }\n }\n}",
						bits, value, value, (int)g_telemetry_interval, ts, pub_data.mac, device_config->MQClientId);
}




/**
 * MQTT Main Loop
 * INFINITE LOOP:
 */
void GenericMQTT_Client_Run(void * pvParameters)
{

	#define MQTT_YIELD_MS	100

	static uint16_t loop_counter_100ms = 0;

	static unsigned char mqtt_send_buffer[MQTT_SEND_BUFFER_SIZE];
	static unsigned char mqtt_read_buffer[MQTT_READ_BUFFER_SIZE];

	static bool b_mqtt_connected = false;
	static const char* connectionString   = NULL;
	static device_config_t* device_config = NULL;
	static conn_sec_t connection_security = CONN_SEC_UNDEFINED;

	static const char* ca_cert     = NULL;
	static const char* device_cert = NULL;
	static const char* device_key  = NULL;

	static MQTTClient client;
	static Network    network;

    int ret = 0;

    g_periodic_publish_flag  = false;
	g_single_publish_flag    = false;
	g_actuators_publish_flag = true;

    g_continueRunning = true;
    g_reboot          = false;

    g_connection_needed_score = 1;

    memset(&pub_data, 0, sizeof(pub_data));

    /**
     *
     */
    printf("\n\n");
	printf("*************************************************************\n");
	printf("***                TESA TOPGUN RALLY 2021                 ***\n");
	printf("***                   TGR MQTT Client                     ***\n");
	printf("*************************************************************\n\n");
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
        	sprintf(mac_addr, "%02X%02X%02X%02X%02X%02X", mac.mac[0], mac.mac[1], mac.mac[2], mac.mac[3], mac.mac[4], mac.mac[5]);

        }
        else {
            msg_warning("Could not retrieve the MAC address to set the device ID.\n");
            sprintf(mac_addr, "%s", "UnknownMAC");
        }
        strncpy(pub_data.mac, mac_addr, MODEL_MAC_SIZE - 1);





        g_telemetry_interval = MODEL_DEFAULT_TELEMETRYINTERVAL;


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
					 * MQTT Connected
					 */
                    g_connection_needed_score = 0;
                    b_mqtt_connected = true;

                    snprintf(mqtt_subtopic, MQTT_TOPIC_BUFFER_SIZE, "%s/%s/control", MQTT_ACTUATORS_TOPIC, device_config->MQClientId);
                    ret = MQTTSubscribe(&client, mqtt_subtopic, QOS0, (allpurposeMessageHandler));



                    /**
                     * Perform Callback
                     */
                    Mqtt_PerformConnectedCallback(&client, device_config->MQClientId);
                }


                if (ret != MQSUCCESS) {
                    msg_error("Failed subscribing to the %s topic.\n", mqtt_subtopic);
                }
                else {
                    msg_info("Subscribed to %s.\n", mqtt_subtopic);
                    ret = MQTTYield(&client, MQTT_YIELD_MS);
                }
                if (ret != MQSUCCESS) {
                    msg_error("Yield failed.\n");
                }
                else {


                	/***************************************************************************************************
                     * Send the telemetry data, and send the device status if it was changed by a received message.
                     ***************************************************************************************************/

                    uint32_t last_telemetry_time_ms = HAL_GetTick();


                    do {


                        int32_t left_ms = comp_left_ms(last_telemetry_time_ms, HAL_GetTick(), g_telemetry_interval * 1000);


                        /***************************************************************************************************
                         * [1] Publish sensors data (activated by the BLUE button)
                         ***************************************************************************************************/

                        if (((g_periodic_publish_flag == true) && (left_ms <= 0)) || (g_single_publish_flag == true)) {

                        	g_single_publish_flag = false;



                            last_telemetry_time_ms = HAL_GetTick();

                            /**
                             * [1.1] Prepare sensor data for publishing
                             */
                            ret = __helper_create_sensors_data(device_config);


                            if ((ret < 0) || (ret >= MQTT_MSG_BUFFER_SIZE)) {
                                msg_error("Telemetry message formatting error.\n");
                            }
                            else {

                            	/********************************************************************************************
                            	 * [1.2] Publish sensors data (activated by the BLUE switch)
                            	 ********************************************************************************************/
                                ret = stiot_publish(&client, mqtt_pubtopic, mqtt_msg);
                                if (ret == MQSUCCESS)  {

                                    /* Visual notification of the telemetry publication: LED blink. */
                                	Ctl_LedBlueOrangeBlink(20, 10, 5);

                                    msg_info("#\n");
                                    msg_info("publication topic: %s \npayload: %s\n", mqtt_pubtopic, mqtt_msg);
                                }
                                else  {
                                    msg_error("Telemetry publication failed.\n");
                                    g_connection_needed_score++;
                                }

                                ret = MQTTYield(&client, MQTT_YIELD_MS);//ret = MQTTYield(&client, 500);
                                if (ret != MQSUCCESS)  {
                                    msg_error("Yield failed. Reconnection needed?.\n");
                                    /* g_connection_needed_score++; */
                                }
                            }
                        }// Publish Sensors data



                        /***************************************************************************************************
                         * [2] Publish the updated device status (activated when a controlled command is received)
                         ***************************************************************************************************/

                        if (g_actuators_publish_flag) {

                        	/**
							 * [2.1] Prepare actuators data for publishing
							 */
                            ret = __helper_create_actuators_data(device_config);


                            if ((ret < 0) || (ret >= MQTT_MSG_BUFFER_SIZE)) {
                                msg_error("Telemetry message formatting error.\n");
                            }
                            else {
                            	/********************************************************************************************
								 * [2.2] Publish actuators data
								 ********************************************************************************************/
                                ret = stiot_publish(&client, mqtt_pubtopic, mqtt_msg);
                                if (ret != MQSUCCESS)  {
                                    msg_error("Status publication failed.\n");
                                    g_connection_needed_score++;
                                }
                                else {
                                	msg_info("#\n");
                                    msg_info("publication topic: %s \npayload: %s\n", mqtt_pubtopic, mqtt_msg);
                                    g_actuators_publish_flag = false;
                                }
                            }
                        }


                        /***************************************************************************************************
						 * [3] Publish others if required
						 ***************************************************************************************************/




                        ret = MQTTYield(&client, MQTT_YIELD_MS);
                        if (ret != MQSUCCESS) {
                            msg_error("Yield failed. Reconnection needed.\n");
                            g_connection_needed_score++;
                        }
                        else {
                        	if(++loop_counter_100ms >= 3) {
                        		Ctl_LedBlueOrangeBlink(10, 5, 1);//msg_info(".");
								loop_counter_100ms = 0;

								/**
								 * Decrease publishing counter
								 */
								if(g_publishing_counnter > 0) {
									g_publishing_counnter--;
								}
							}
                        }

                        /**
                         * Idle Callback
                         */
                        if(g_publishing_counnter <= 0) {
                        	Mqtt_PerformIdleCallback(&client);
                        }

                        /**
                         * Loop callback
                         */
                        TGRHelpers_PerformCallback();






                    } while (g_continueRunning && !g_reboot && (g_connection_needed_score == 0));
                    // END of internal do{}while()

                } // END of external do{}while()




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

	if(g_publishing_counnter > 0) {
		msg_error("\n** MQTT Publishing Error. The publisher is in progress **\n");
		return FAILURE;
	}

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
    g_publishing_counnter++;
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

    if (strlen(string) > USER_CONF_DEVICE_NAME_LENGTH) {
        msg_error("Cannot parse the configuration string:  It is not null-terminated!\n");
    }
    else {
        if (pConfig == NULL) {
            msg_error("Null parameter\n");
        }
        else  {
            config = malloc(sizeof(device_config_t));
            memset(config, 0, sizeof(device_config_t));

            ret = string_allocate_from_token(&config->HostName, "HostName=", string);
            ret |= string_allocate_from_token(&config->HostPort, "HostPort=", string);
            ret |= string_allocate_from_token(&config->ConnSecurity, "ConnSecurity=", string);
            ret |= string_allocate_from_token(&config->MQClientId, "MQClientId=", string);
            ret |= string_allocate_from_token(&config->MQUserName, "MQUserName=", string);
            ret |= string_allocate_from_token(&config->MQUserPwd, "MQUserPwd=", string);

            if (ret != 0)  {
                msg_error("Failed parsing the device configuration string.\n");
                free_device_config(config);
            }
            else {
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
    if (config != NULL) {
        if (config->HostName != NULL) free(config->HostName);
        if (config->HostPort != NULL) free(config->HostPort);
        if (config->ConnSecurity != NULL) free(config->ConnSecurity);
        if (config->MQClientId != NULL) free(config->MQClientId);
        if (config->MQUserName != NULL) free(config->MQUserName);
        if (config->MQUserPwd != NULL) free(config->MQUserPwd);

        free(config);
    }
    else {
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

    if (now < init) {
    	/* Timer wrap-around detected */
    	/* printf("Timer: wrap-around detected from %d to %d\n", init, now); */
        wrap_end = UINT32_MAX - init;
    }
    ret = wrap_end - (now - init) + timeout;

    return ret;
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
