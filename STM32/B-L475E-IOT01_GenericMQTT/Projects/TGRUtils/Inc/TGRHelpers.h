/*****************************************************************************************************
 * File Name:	TGRHelpers.h
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


#ifndef __ECCLAB_TGR_HELPER_H__
#define __ECCLAB_TGR_HELPER_H__

/*****************************************************************************************************
 * Include Files
 *****************************************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stm32l4xx_hal.h>
#include <cJSON.h>


/*****************************************************************************************************
 * USER BUTTON (BLUE BUTTON)
 *****************************************************************************************************/

/**
 * Typedef of function pointer of user push button switch
 */
typedef void (*Psw_Callback)(uint32_t cnt);

/**
 * Sets callback function of switch pushed
 */
void Psw_SetPushedCallback(Psw_Callback callback);

/**
 * Sets callback function of switch released
 */
void Psw_SetReleasedCallback(Psw_Callback callback);

/**
 * Sets callback function of switch holding
 */
void Psw_SetHoldingCallback(Psw_Callback callback);




/*****************************************************************************************************
 * TICK/CLOCK/TIMING
 *****************************************************************************************************/

/**
 * Typedef of function pointer of ticker
 */
typedef void (*Tck_Callback)(uint32_t ticks);


/**
 * Sets system time of ticker
 */
void Clk_SetTime(uint16_t hour, uint16_t min, uint16_t sec, uint16_t GTM);


/**
 * Returns true if the system time is update by the RTC
 */
bool Clk_IsRtcUpdated(void);


/**
 * Return string of current time
 * mode 0: hh:mm:ss:xxx
 * mode 1: hh:mm:ss
 */
char *Clk_GetString(uint16_t mode);


/**
 * Returns hours
 */
uint16_t Clk_GetHours(void);


/**
 * Returns munites
 */
uint16_t Clk_GetMinutes(void);


/**
 * Returns seconds
 */
uint16_t Clk_GetSeconds(void);


/**
 * Returns mulliseconds of the system time
 */
uint32_t Clk_GetMilliseconds(void);

/**
 * Returns ticks (1 ticks = 1 mS)
 */
uint32_t Clk_GetTicks(void);


/**
 * Sets ticks to a specified value
 */
void Clk_SetTicks(void);



/**
 * Sets on-tick callback function
 */
void Clk_SetTickCallback(Tck_Callback callback);


/**
 * Sets on-1sec callback function
 */
void Clk_SetOneSecCallback(Tck_Callback callback);


/**
 * Sets on-halfsec callback function
 */
void Clk_SetHalfSecCallback(Tck_Callback callback);




/*****************************************************************************************************
 * MQTT MAIN LOOP CONTROL
 *****************************************************************************************************/

/**
 * Requset to publish sensors
 */
bool Mqtt_SensorsPublish(void);


/**
 * Requset to publish sensors continuously
 */
bool Mqtt_SensorsPublishContinuous(uint16_t sct);


/**
 * Requset to publish actuators
 */
void Mqtt_ActuatorsPublish(void);


/**
 * Performs callbacks
 * PRIVATE: Called by MQTT main loop
 */
void TGRHelpers_PerformCallback(void);



/*****************************************************************************************************
 * DIGITAL OUTPUT CONTROL FUNCTIONS
 *****************************************************************************************************/


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

#define DIGITAL_OUTPUT_BIT_A0			0	// Index of digital output bit #0
#define DIGITAL_OUTPUT_BIT_A1			1	// Index of digital output bit #1
#define DIGITAL_OUTPUT_BIT_A2			2	// Index of digital output bit #2
#define DIGITAL_OUTPUT_BIT_A3			3	// Index of digital output bit #3
#define DIGITAL_OUTPUT_BIT_A4			4	// Index of digital output bit #4
#define DIGITAL_OUTPUT_BIT_A5			5	// Index of digital output bit #5
#define DIGITAL_OUTPUT_BIT_LED1			6	// Index of digital output bit #6
#define DIGITAL_OUTPUT_BIT_LED2			7	// Index of digital output bit #7
#define DIGITAL_OUTPUT_BIT_LED3			8	// Index of digital output bit #8


/**
 * Writes bit-data to digitaloutput
 */
bool Ctl_DigitalOutputWriteBit(int16_t BitId, bool BitData);


/**
 * Blinks the Blue and Orange leds using PWM
 */
void Ctl_LedBlueOrangeBlink(int period, int duty, int count);


/**
 * Processes MQTT message
 * Returns true if the received message is controlling message
 */
bool Ctl_ProcessMessage(const char* mqtt_msg);


/**
 * Regurns all bits of digital output
 */
bool *Ctl_GetDigitalOutputValues(void);


/**
 * Returns number of digital output channels
 */
uint16_t Ctl_GetDigitalOutputChannels(void);


/**
 * Returns digital output data in for mat of binary-string
 */
char *Ctl_GetBinaryString(char *buffer);


/**
 * Returns digital output data in format of decimal value
 */
uint16_t Ctl_GetDecimalValue(void);


/**
 * Sets digital output specified by BitId
 */
bool Ctl_DigitalOutputSetBit(int16_t BitId);


/**
 * Clears/Resets digital output specified by BitId
 */
bool Ctl_DigitalOutputClrBit(int16_t BitId);


/**
 * Toggles/Inverts digital output specified by BitId
 */
bool Ctl_DigitalOutputInvBit(int16_t BitId);


/**
 * Returns digital output specified by BitId
 */
bool Ctl_DigitalOutputGetBit(int16_t BitId);


#endif // __ECCLAB_TGR_HELPER_H__
