/*****************************************************************************************************
 * File Name:	main.h
 *
 * Description: Provides include files, function prototypes, types and contsnts for application
 *
 * Author:		Asst.Prof.Dr.Santi Nuratch
 * 				Embedded Computing and Control Laboratory		: ECC-Lab
 * 				Control System and Instrumentation Engineering 	: INC
 *				King Mongkut's University of Technology Thonburi: KMUTT
 *
 * Update:		06 December 2020 (Initial version)
 *
 *****************************************************************************************************/


#ifndef __MAIN_H
#define __MAIN_H
#define __main_h__
#ifdef __cplusplus
 extern "C" {
#endif


 /**
  * BEGIN TGR SECTION
  */

 /*********************************************************************************************
  * TGR::External Functions                                                                   *
  *********************************************************************************************/
 extern void GenericMQTT_Client_Run(void *pvParameters);
 extern void SPI_WIFI_ISR(void);



 /*********************************************************************************************
  * TGR::Private Functions                                                                    *
  *********************************************************************************************/
 void System_Init(void);


 /*********************************************************************************************
  * TGR::Include Files                                                                        *
  *********************************************************************************************/
 #include <TGRHelpers.h>
 #include <TGRMqtt.h>


/**
 * END TGR SECTION
 */




/**
 * DEFAULT SECTION BEGIN
 */
/*********************************************************************************************
 * Include Files                                                                             *
 *********************************************************************************************/
#include "stm32l4xx_hal.h"
#include "stm32l475e_iot01.h"
#include "stm32l4xx_hal_iwdg.h"
#include "version.h"
#ifdef RFU
#include "rfu.h"
#endif
#ifdef SENSOR
#include "stm32l475e_iot01_accelero.h"
#include "stm32l475e_iot01_psensor.h"
#include "stm32l475e_iot01_gyro.h"
#include "stm32l475e_iot01_hsensor.h"
#include "stm32l475e_iot01_tsensor.h"
#include "stm32l475e_iot01_magneto.h"
#include "vl53l0x_proximity.h"
#endif
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "timedate.h"
#include "flash.h"
#ifdef FIREWALL_MBEDLIB
#include "firewall_wrapper.h"
#include "firewall.h"
#endif /* FIREWALL_MBEDLIB */
#include "net.h"
#include "iot_flash_config.h"
#include "msg.h"
#include "cloud.h"
#include "sensors_data.h"


#ifdef USE_MBED_TLS
extern int mbedtls_hardware_poll( void *data, unsigned char *output, size_t len, size_t *olen );
#endif /* USE_MBED_TLS */


/*********************************************************************************************
 * Exported Types and Constants                                                              *
 *********************************************************************************************/
#if defined(USE_WIFI)
#define NET_IF  NET_IF_WLAN
#elif defined(USE_LWIP)
#define NET_IF  NET_IF_ETH
#elif defined(USE_C2C)
#define NET_IF  NET_IF_C2C
#endif

enum {BP_NOT_PUSHED=0, BP_SINGLE_PUSH, BP_MULTIPLE_PUSH};

/*********************************************************************************************
 * Exported Functions                                                                        *
 *********************************************************************************************/
void    Error_Handler(void);
uint8_t Button_WaitForPush(uint32_t timeout);
uint8_t Button_WaitForMultiPush(uint32_t timeout);
void    Led_SetState(bool on);
void    Led_Blink(int period, int duty, int count);
void    Periph_Config(void);
void 	SPI3_IRQHandler(void);

extern  SPI_HandleTypeDef hspi;
extern  RNG_HandleTypeDef hrng;
extern  RTC_HandleTypeDef hrtc;
extern  net_hnd_t         hnet;

extern const user_config_t *lUserConfigPtr;


#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */


