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



/*****************************************************************************************************
 * Include Files
 *****************************************************************************************************/

#include "TGRHelpers.h"
#include "msg.h"




/*****************************************************************************************************
 * USER BUTTON (BLUE BUTTON)
 *****************************************************************************************************/

typedef struct {
	uint16_t ticks;
	uint16_t state;
	uint32_t count;
	Psw_Callback cbkPushed;
	Psw_Callback cbkReleased;
	Psw_Callback cbkHolding;
}helper_psw_t;

helper_psw_t helper_psw = {0, 0, 0, 0, 0, 0};



static uint8_t psw_cbk_flags = 0x00;

void Psw_SetPushedCallback(Psw_Callback callback) {
	helper_psw.cbkPushed = callback;
}

void Psw_SetReleasedCallback(Psw_Callback callback) {
	helper_psw.cbkReleased = callback;
}

void Psw_SetHoldingCallback(Psw_Callback callback) {
	helper_psw.cbkHolding = callback;
}


void Psw_Executor(bool isOn) {

	if(isOn) {
		++helper_psw.ticks;
		if(helper_psw.state == 0) {
			if(helper_psw.ticks >= 50) {
				helper_psw.count++;
				helper_psw.ticks = 0;
				helper_psw.state = 1;
				psw_cbk_flags |= 1<<0;
			}
		}
		else if(helper_psw.state == 1) {
			if(helper_psw.ticks >= 1000) {
				helper_psw.count++;
				helper_psw.ticks = 0;
				helper_psw.state = 2;
				psw_cbk_flags |= 1<<1;
			}
		}
		else if(helper_psw.state == 2) {

		}
	}
	else if(helper_psw.state == 2) {
		helper_psw.ticks = 0;
		helper_psw.state = 0;
		psw_cbk_flags |= 1<<2;
	}
	else {
		helper_psw.ticks = 0;
		helper_psw.state = 0;
	}
}



/*****************************************************************************************************
 * CLOCK
 *****************************************************************************************************/

typedef struct{
	uint16_t hh;
	uint16_t mm;
	uint16_t ss;
	uint16_t xx;
}helper_clk_t;

helper_clk_t helper_clock;

static bool helper_clock_rtc_updated = false;
static uint32_t helper_system_ticks = 0;

static Tck_Callback tck_tick_cbk    = NULL;
static Tck_Callback tck_onesec_cbk  = NULL;
static Tck_Callback tck_halfsec_cbk = NULL;

void Clk_Executor(void) {

	helper_system_ticks++;

	if(++helper_clock.xx >= 1000) {
		helper_clock.xx = 0;
		if(++helper_clock.ss >= 60) {
			helper_clock.ss = 0;
			if(++helper_clock.mm >= 60) {
				helper_clock.mm = 0;
				if(helper_clock.hh >= 24) {
					helper_clock.hh = 0;
				}
			}
		}
	}

	if(tck_tick_cbk != NULL) {
		tck_tick_cbk(helper_system_ticks);
	}
}



/**
 * This function is also called by timedate.c, @line 185
 */
void Clk_SetTime(uint16_t hour, uint16_t min, uint16_t sec, uint16_t GTM) {
	helper_clock.ss = sec;
	helper_clock.mm = min;
	helper_clock.hh = (hour+GTM)%24;
	helper_clock.xx = 0;
	if(GTM == 0x0007) { // GTM is 7, if this function is called by RCT.
		helper_clock_rtc_updated = true;
	}
}

bool Clk_IsRtcUpdated(void) {
	return(helper_clock_rtc_updated);
}


char *Clk_GetString(uint16_t mode) {
	static char clkstr[32];
	if(mode==0){
		sprintf(clkstr, "%.2d:%.2d:%.2d:%.3d", helper_clock.hh, helper_clock.mm, helper_clock.ss, (helper_clock.xx)%1000);
	}
	else if(mode == 1) {
		sprintf(clkstr, "%.2d:%.2d:%.2d", helper_clock.hh, helper_clock.mm, helper_clock.ss);
	}
	return clkstr;
}


uint16_t Clk_GetHours(void) {
	return helper_clock.hh;
}

uint16_t Clk_GetMinutes(void) {
	return helper_clock.mm;
}

uint16_t Clk_GetSeconds(void) {
	return helper_clock.ss;
}

uint32_t Clk_GetTicks(void) {
	return helper_system_ticks;
}
void Clk_SetTicks(void) {
	helper_system_ticks = 0;
}

uint32_t Clk_GetMilliseconds(void) {
	return helper_system_ticks;
}

void Clk_SetTickCallback(Tck_Callback callback) {
	tck_tick_cbk = callback;
}

void Clk_SetOneSecCallback(Tck_Callback callback) {
	tck_onesec_cbk = callback;
}

void Clk_SetHalfSecCallback(Tck_Callback callback) {
	tck_halfsec_cbk = callback;
}



/*****************************************************************************************************
 * MQTT MAIN LOOP CONTROL
 *****************************************************************************************************/

bool g_periodic_publish_flag  = false;

bool g_single_publish_flag    = false;

bool g_actuators_publish_flag = true;


extern uint32_t g_publishing_counnter;

bool Mqtt_SensorsPublish(void) {
	if(g_publishing_counnter > 0) {
		msg_error("\n** MQTT Publishing Error. The publisher is in progress **\n");
		return false;
	}

	g_single_publish_flag = true;
	return true;

}

bool Mqtt_SensorsPublishContinuous(uint16_t sct) {
	if(g_publishing_counnter > 0) {
		msg_error("\n** MQTT Publishing Error. The publisher is in progress **\n");
		return false;
	}

	if(sct == 0) {
		g_periodic_publish_flag = false;
	}
	else if(sct == 1) {
		g_periodic_publish_flag = true;
	}
	else {
		g_periodic_publish_flag = !g_periodic_publish_flag;
	}
	return true;
}


void Mqtt_ActuatorsPublish(void) {
	g_actuators_publish_flag = true;
}


/*****************************************************************************************************
 * CALLBACKS
 *****************************************************************************************************/

void TGRHelpers_PerformCallback(void) {

	if(tck_halfsec_cbk!=NULL && helper_system_ticks % 500 == 0) {
		tck_halfsec_cbk(helper_system_ticks);
	}
	if(tck_onesec_cbk!=NULL && helper_system_ticks % 1000 == 0) {
		tck_onesec_cbk(helper_system_ticks);
	}


	if( psw_cbk_flags&(1<<0) ) {
		psw_cbk_flags &= ~(1<<0);
		if( helper_psw.cbkPushed!=NULL ) {
			helper_psw.cbkPushed(helper_psw.count);
		}
	}
	if( psw_cbk_flags&(1<<1) ) {
		psw_cbk_flags &= ~(1<<1);
		if( helper_psw.cbkHolding!=NULL ) {
			helper_psw.cbkHolding(helper_psw.count);
		}
	}
	if( psw_cbk_flags&(1<<2) ) {
		psw_cbk_flags &= ~(1<<2);
		if( helper_psw.cbkReleased!=NULL ) {
			helper_psw.cbkReleased(helper_psw.count);
		}
	}
}




/*****************************************************************************************************
 * SYSTEM TICK ISR
 *****************************************************************************************************/
void HAL_SYSTICK_Callback(void) {
	Clk_Executor();
	Psw_Executor((HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET));
}






/*****************************************************************************************************
 * ACTUATORS (DIGITAL OUTPUTS) CONTROL SECTION
 *****************************************************************************************************/

/**
 * Digital Outputs
 *   [0]: A0		PC5
 *   [1]: A1		PC4
 *   [2]: A2		PC3
 *   [3]: A3		PC2
 *   [4]: A4		PC1
 *   [5]: A5		PC0
 *   [6]: LED1		PA5
 *   [7]: LED2		PB14
 *   [8]: LED3		PC9		LED3&LED4 (WiFi/Bluetooth)
 */


#define NUM_DIGITAL_OUTPUTS		9


static bool  g_digital_outputs[NUM_DIGITAL_OUTPUTS] = {0, 0, 0, 0, 0, 0, 0, 0, 0};




static void __Ctl_UpdateDigitalOutputs(void) {

	for(int i=0; i<=5; i++) {
		HAL_GPIO_WritePin(GPIOC, (uint16_t)(1<<(5-i)), (int)g_digital_outputs[i]);	// A<5:0>
	}

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5,  (int)g_digital_outputs[DIGITAL_OUTPUT_BIT_LED1]);	// LED1
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, (int)g_digital_outputs[DIGITAL_OUTPUT_BIT_LED2]);	// LED2
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9,  (int)g_digital_outputs[DIGITAL_OUTPUT_BIT_LED3]);	// LED3/LED4

}


bool *Ctl_GetDigitalOutputValues(void) {
	return g_digital_outputs;
}


uint16_t Ctl_GetDigitalOutputChannels(void) {
	return NUM_DIGITAL_OUTPUTS;
}

bool Ctl_DigitalOutputWriteBit(int16_t BitId, bool BitData) {

	bool isChanged = true;

	if( BitId>=0 && BitId<=5 ) {
		g_digital_outputs[BitId] = BitData;
		//HAL_GPIO_WritePin(GPIOC, (uint16_t)(1<<(5-BitId)), (int)g_digital_outputs[BitId]);	// A<5:0>
	}
	else if( BitId == DIGITAL_OUTPUT_BIT_LED1 ) {
		g_digital_outputs[DIGITAL_OUTPUT_BIT_LED1] = BitData;
		//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5,  (int)g_digital_outputs[BIT_LED1]);	// LED1
	}
	else if( BitId == DIGITAL_OUTPUT_BIT_LED2 ) {
		g_digital_outputs[DIGITAL_OUTPUT_BIT_LED2] = BitData;
		//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, (int)g_digital_outputs[BIT_LED2]);	// LED2
	}
	else if( BitId == DIGITAL_OUTPUT_BIT_LED3 ) {
		g_digital_outputs[DIGITAL_OUTPUT_BIT_LED3] = BitData;
		//HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9,  (int)g_digital_outputs[BIT_LED3]);	// LED3/LED4
	}
	else {
		isChanged = false;
	}

	if(isChanged == true) {
		__Ctl_UpdateDigitalOutputs();
	}
	return isChanged;
}

bool Ctl_DigitalOutputSetBit(int16_t BitId) {
	return Ctl_DigitalOutputWriteBit(BitId, true);
}

bool Ctl_DigitalOutputClrBit(int16_t BitId) {
	return Ctl_DigitalOutputWriteBit(BitId, false);
}

bool Ctl_DigitalOutputInvBit(int16_t BitId) {
	if(BitId>=0 && BitId<=8) {
		return Ctl_DigitalOutputWriteBit(BitId, !g_digital_outputs[BitId]);
	}
	return false;
}
bool Ctl_DigitalOutputGetBit(int16_t BitId) {
	if(BitId>=0 && BitId<=8) {
		return g_digital_outputs[BitId];
	}
	return false;
}


void Ctl_LedBlueOrangeBlink(int period, int duty, int count)
{
    if ((duty > 0) && (period >= duty)) {
        do {
        	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9,  GPIO_PIN_SET); 	// LED3/LED4
            HAL_Delay(duty);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9,  GPIO_PIN_RESET); 	// LED3/LED4
            HAL_Delay(period - duty);
        } while (count--);
    }
    if ((duty < 0) && (period >= -duty))  {
        do  {
        	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9,  GPIO_PIN_RESET); 	// LED3/LED4
            HAL_Delay(period + duty);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9,  GPIO_PIN_SET); 	// LED3/LED4
            HAL_Delay(-duty);
        } while (count--);
    }

    // Restore
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9,  (int)g_digital_outputs[DIGITAL_OUTPUT_BIT_LED3]);	// LED3/LED4
}


char *Ctl_GetBinaryString(char *buffer) {
	for(int i=0; i<NUM_DIGITAL_OUTPUTS; i++) {
		buffer[NUM_DIGITAL_OUTPUTS-i-1] = (g_digital_outputs[i] == true) ? '1' : '0';
	}
	buffer[NUM_DIGITAL_OUTPUTS] = 0;
	return buffer;
}

uint16_t Ctl_GetDecimalValue(void) {
	int  value = 0;
	for(int i=0; i<NUM_DIGITAL_OUTPUTS; i++) {
		value<<=1;
		value |= (int)g_digital_outputs[NUM_DIGITAL_OUTPUTS-i-1];
	}
	return value;
}


bool Ctl_ProcessMessage(const char* mqtt_msg) {


	bool isChanged = false;

	/**
	 * JSON
	 */
	cJSON *json = NULL;
	cJSON *root = cJSON_Parse(mqtt_msg);


	for(int i=0; i<3; i++) {
			char key[64];
			sprintf(key, "Led%d", i+1);
			json = cJSON_GetObjectItemCaseSensitive(root, key);
			if(json != NULL) {
				if (cJSON_IsBool(json) == true) {
					g_digital_outputs[i+6] = (cJSON_IsTrue(json) == true);
					__Ctl_UpdateDigitalOutputs();
					isChanged = true;
					break;
				}
				else if( (cJSON_IsString(json) == true) ) {
					if( (strcmp(json->valuestring, "toggle") == 0) || (strcmp(json->valuestring, "inv") == 0)) {
						g_digital_outputs[i+6] = !g_digital_outputs[i+6];
						__Ctl_UpdateDigitalOutputs();
						isChanged = true;
						break;
					}
					else if( (strcmp(json->valuestring, "set") == 0) ) {
						g_digital_outputs[i+6] = true;
						__Ctl_UpdateDigitalOutputs();
						isChanged = true;
						break;
					}
					else if( (strcmp(json->valuestring, "reset") == 0) || (strcmp(json->valuestring, "clear") == 0) || (strcmp(json->valuestring, "clr") == 0)) {
						g_digital_outputs[i+6] = false;
						__Ctl_UpdateDigitalOutputs();
						isChanged = true;
						break;
					}
				}
				else {
					msg_error("JSON parsing error of Led value.\n");
				}
			}
		}


	/**
	 * DigitalOutput<5:NUM_DIGITAL_OUTPUTS-1>
	 */

	for(int i=0; i<NUM_DIGITAL_OUTPUTS; i++) {
		char key[64];
		sprintf(key, "DigitalOutput%d", i);
		json = cJSON_GetObjectItemCaseSensitive(root, key);
		if(json != NULL) {
			if (cJSON_IsBool(json) == true) {
				g_digital_outputs[i] = (cJSON_IsTrue(json) == true);
				__Ctl_UpdateDigitalOutputs();
				isChanged = true;
				break;
			}
			else if( (cJSON_IsString(json) == true)  ) {
				if( (strcmp(json->valuestring, "toggle") == 0 ) || (strcmp(json->valuestring, "inv") == 0 )) {
					g_digital_outputs[i] = !g_digital_outputs[i];
					__Ctl_UpdateDigitalOutputs();
					isChanged = true;
					break;
				}
				else if( (strcmp(json->valuestring, "set") == 0 ) ) {
					g_digital_outputs[i] = true;
					__Ctl_UpdateDigitalOutputs();
					isChanged = true;
					break;
				}
				else if( (strcmp(json->valuestring, "reset") == 0 ) || (strcmp(json->valuestring, "clear") == 0 ) || (strcmp(json->valuestring, "clr") == 0 )) {
					g_digital_outputs[i] = false;
					__Ctl_UpdateDigitalOutputs();
					isChanged = true;
					break;
				}
			}
			else {
				msg_error("JSON parsing error of DigitalOutputX value.\n");
			}
		}
	}

	json = cJSON_GetObjectItemCaseSensitive(root, "DigitalWrite");
	int d = 0;
	if (json != NULL) {
		if (cJSON_IsNumber(json) == true) {

			d = json->valueint;
			for(int j=0; j<NUM_DIGITAL_OUTPUTS; j++) {
				g_digital_outputs[j] = ((d&1) > 0);
				d>>=1;
				printf("DigitalOutput[%d]: %s\n", j, ((g_digital_outputs[j] == true) ? "true" : "false") );
			}
			__Ctl_UpdateDigitalOutputs();
			isChanged = true;
		}
		else {
			msg_error("JSON parsing error of DigitalWrite value.\n");
		}
	}




	/* Visual notification of the Received message: LED blink. */
	Ctl_LedBlueOrangeBlink(40, 10, 4);

	/**
	 * Cleanup
	 */
	cJSON_Delete(root);

	return isChanged;
}

