/*********************************************************************************************
 * File Name:	main.c
 *
 * Description: Provides settings, and other utility functions
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
 * External Function                                                                         *
 *********************************************************************************************/
extern void TGR_Main(void);


/*********************************************************************************************
 ****                                                                                     ****
 **** MAIN FUNCTION                                                                       ****
 ****                                                                                     ****
 *********************************************************************************************/
 int main(void) {
	System_Init();
	TGR_Main();
 }



 /*********************************************************************************************
  * Global Variables                                                                          *
  *********************************************************************************************/
  RTC_HandleTypeDef hrtc;
  RNG_HandleTypeDef hrng;
  net_hnd_t         hnet;// Is initialized by cloud_main()

 /*********************************************************************************************
  * Initialization Functions and ISRs                                                   	     *
  *********************************************************************************************/

 /**
  * Initializes sensors
  */
 static void Sensors_Init(void) {
 	int res = init_sensors();
 	if(0 != res) {
 		msg_error("init_sensors returned error : %d\n", res);
 	}
 }


 /**
   * @brief GPIO Initialization Function
   * @param None
   * @retval None
   */
 static void MX_GPIO_Init(void)
 {
     GPIO_InitTypeDef GPIO_InitStruct = { 0 };

     /* GPIO Ports Clock Enable */
     __HAL_RCC_GPIOC_CLK_ENABLE();
     __HAL_RCC_GPIOA_CLK_ENABLE();
     __HAL_RCC_GPIOB_CLK_ENABLE();

     /*Configure GPIO pin Output Level */
     HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_9, GPIO_PIN_RESET);

     /*Configure GPIO pin Output Level */
     HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4 | GPIO_PIN_5, GPIO_PIN_RESET);

     /*Configure GPIO pin Output Level */
     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2 | GPIO_PIN_14, GPIO_PIN_RESET);

     /*Configure GPIO pins : PC0 PC1 PC2 PC3 PC4 PC5 PC9 */
     GPIO_InitStruct.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_9;
     GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
     GPIO_InitStruct.Pull  = GPIO_NOPULL;
     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
     HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

     /*Configure GPIO pins : PA4 PA5 */
     GPIO_InitStruct.Pin   = GPIO_PIN_4 | GPIO_PIN_5;
     GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
     GPIO_InitStruct.Pull  = GPIO_NOPULL;
     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
     HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

     /*Configure GPIO pins : PB2 PB14 */
     GPIO_InitStruct.Pin   = GPIO_PIN_2 | GPIO_PIN_14;
     GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
     GPIO_InitStruct.Pull  = GPIO_NOPULL;
     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
     HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

 }



 /**
   * @brief  System Clock Configuration
   *         The system Clock is configured as follows :
   *            System Clock source            = PLL (MSI MSE)
   *            SYSCLK(Hz)                     = 80000000
   *            HCLK(Hz)                       = 80000000
   *            AHB Prescaler                  = 1
   *            APB1 Prescaler                 = 1
   *            APB2 Prescaler                 = 1
   *            MSI Frequency(Hz)              = 48000000
   *            PLL_M                          = 6
   *            PLL_N                          = 20
   *            PLL_R                          = 2
   *            PLL_P                          = 7
   *            PLL_Q                          = 2
   *            Flash Latency(WS)              = 4
   * @param  None
   * @retval None
   */
 static void SystemClock_Config(void)
 {
     RCC_OscInitTypeDef RCC_OscInitStruct;
     RCC_ClkInitTypeDef RCC_ClkInitStruct;

     RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE | RCC_OSCILLATORTYPE_MSI;
     RCC_OscInitStruct.LSEState = RCC_LSE_ON;
     RCC_OscInitStruct.MSIState = RCC_MSI_ON;
     RCC_OscInitStruct.MSICalibrationValue = 0;
     RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_11;
     RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
     RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
     RCC_OscInitStruct.PLL.PLLM = 6;
     RCC_OscInitStruct.PLL.PLLN = 20;
     RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
     RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
     RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
     if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
     {
         Error_Handler();
     }

     /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
        clocks dividers */
     RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
     RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
     RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
     RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
     RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
     if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
     {
         Error_Handler();
     }

     /* Enable MSI PLL mode */
     HAL_RCCEx_EnableMSIPLLMode();
 }


 /**
   * @brief Set LED state
   */
 void Led_SetState(bool on)
 {
     if (on == true) {
         BSP_LED_On(LED_GREEN);
     }
     else {
         BSP_LED_Off(LED_GREEN);
     }
 }


 /**
  * @brief Blink LED for 'count' cycles of 'period' period and 'duty' ON duration.
  * duty < 0 tells to start with an OFF state.
  */
 void Led_Blink(int period, int duty, int count)
 {
     if ((duty > 0) && (period >= duty)) {
         /*  Shape:   ____
                       on |_off__ */
         do {
             Led_SetState(true);
             HAL_Delay(duty);
             Led_SetState(false);
             HAL_Delay(period - duty);
         } while (count--);
     }
     if ((duty < 0) && (period >= -duty)) {
         /*  Shape:         ____
                     __off_| on   */
         do  {
             Led_SetState(false);
             HAL_Delay(period + duty);
             Led_SetState(true);
             HAL_Delay(-duty);
         } while (count--);
     }
 }


 static volatile uint8_t button_flags = 0;

 /**
   * @brief Update button ISR status
   */
 static void Button_ISR(void)
 {
     button_flags++;
 }


 /**
   * @brief Waiting for button to be pushed
   */
 uint8_t Button_WaitForPush(uint32_t delay)
 {
     uint32_t time_out = HAL_GetTick() + delay;
     do  {
         if (button_flags > 1)  {
             button_flags = 0;
             return BP_MULTIPLE_PUSH;
         }

         if (button_flags == 1)  {
             button_flags = 0;
             return BP_SINGLE_PUSH;
         }
     } while (HAL_GetTick() < time_out);
     return BP_NOT_PUSHED;
 }

 /**
   * @brief Waiting for button to be pushed
   */
 uint8_t Button_WaitForMultiPush(uint32_t delay)
 {
     HAL_Delay(delay);

     if (button_flags > 1) {
         button_flags = 0;
         return BP_MULTIPLE_PUSH;
     }

     if (button_flags == 1) {
         button_flags = 0;
         return BP_SINGLE_PUSH;
     }
     return BP_NOT_PUSHED;
 }


 static UART_HandleTypeDef console_uart;

 /**
   * @brief UART console init function
   */
 static void UART_Init(void)
 {
     console_uart.Instance = USART1;
     console_uart.Init.BaudRate = 115200;
     console_uart.Init.WordLength = UART_WORDLENGTH_8B;
     console_uart.Init.StopBits = UART_STOPBITS_1;
     console_uart.Init.Parity = UART_PARITY_NONE;
     console_uart.Init.Mode = UART_MODE_TX_RX;
     console_uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
     console_uart.Init.OverSampling = UART_OVERSAMPLING_16;
 #ifdef UART_ONE_BIT_SAMPLE_DISABLE
     console_uart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
     console_uart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
 #endif
     BSP_COM_Init(COM1, &console_uart);
 }

 #if (defined(__GNUC__) && !defined(__CC_ARM))
 #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
 #define GETCHAR_PROTOTYPE int __io_getchar(void)
 #else
 #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
 #define GETCHAR_PROTOTYPE int fgetc(FILE *f)
 #endif /* __GNUC__ */

 /**
   * @brief  Retargets the C library printf function to the USART.
   * @param  None
   * @retval None
   */
 PUTCHAR_PROTOTYPE
 {
     /* Place your implementation of fputc here */
     /* e.g. write a character to the USART2 and Loop until the end of transmission */
     while (HAL_OK != HAL_UART_Transmit(&console_uart, (uint8_t*)&ch, 1, 30000));
     return ch;
 }

 /**
   * @brief  Retargets the C library scanf function to the USART.
   * @param  None
   * @retval None
   */
 GETCHAR_PROTOTYPE
 {
     /* Place your implementation of fgetc here */
     /* e.g. read a character on USART and loop until the end of read */
     uint8_t ch = 0;
     while (HAL_OK != HAL_UART_Receive(&console_uart, (uint8_t*)&ch, 1, 30000));
     return ch;
 }

 /**
   * @brief RTC init function
   */
 static void RTC_Init(void)
 {
     /**Initialize RTC Only */
     hrtc.Instance = RTC;
     hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
     hrtc.Init.AsynchPrediv = 127;
     hrtc.Init.SynchPrediv = 255;
     hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
 #ifdef RTC_OUTPUT_REMAP_NONE
     hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
 #endif
     hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
     hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
     if (HAL_RTC_Init(&hrtc) != HAL_OK)  {
         Error_Handler();
     }
 }


 /**
   * @brief  EXTI line detection callback.
   * @param  GPIO_Pin: Specifies the port pin connected to corresponding EXTI line.
   * @retval None
   */
 void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
 {
     switch (GPIO_Pin) {
 		case (GPIO_PIN_13): {
 			Button_ISR();
 			break;
 		}
 		case (GPIO_PIN_1): {
 			SPI_WIFI_ISR();
 			break;
 		}
 		default: {
 			break;
 		}
     }
 }


 void SPI3_IRQHandler(void) {
     HAL_SPI_IRQHandler(&hspi);
 }


 void Error_Handler(void) {
     while (1)  {
         BSP_LED_Toggle(LED_GREEN);
         HAL_Delay(200);
     }
 }



 void System_Init(void) {

 	HAL_Init();
 	SystemClock_Config();
 	Periph_Config();

 	BSP_LED_Init(LED_GREEN);
 	BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);
 	hrng.Instance = RNG;
 	if (HAL_RNG_Init(&hrng) != HAL_OK)  {
 		Error_Handler();
 	}

 	RTC_Init();
 	UART_Init();
 	MX_GPIO_Init();
 	Sensors_Init();

 	#ifdef FIREWALL_MBEDLIB
 	    firewall_init();
 	#endif
 }

 /*********************************************************************************************
 ####### ####### ######   #####  #     # #     #    ######     #    #       #       #     #
    #    #     # #     # #     # #     # ##    #    #     #   # #   #       #        #   #
    #    #     # #     # #       #     # # #   #    #     #  #   #  #       #         # #
    #    #     # ######  #  #### #     # #  #  #    ######  #     # #       #          #
    #    #     # #       #     # #     # #   # #    #   #   ####### #       #          #
    #    #     # #       #     # #     # #    ##    #    #  #     # #       #          #
    #    ####### #        #####   #####  #     #    #     # #     # ####### #######    #
 **********************************************************************************************/

