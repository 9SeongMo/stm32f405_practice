/*
 * This file is part of the 쨉OS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2014 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"

#include "Timer.h"
#include "BlinkLed.h"

#include "stm32f4xx_hal.h"
#include "stm32f4xx_it.h"
#include "stm32F4xx.h"

// ----------------------------------------------------------------------------
//
// Standalone STM32F4 led blink sample (trace via ITM).
//
// In debug configurations, demonstrate how to print a greeting message
// on the trace device. In release configurations the message is
// simply discarded.
//
// Then demonstrates how to blink a led with 1 Hz, using a
// continuous loop and SysTick delays.
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the ITM output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// ----- Timing definitions -------------------------------------------------

// Keep the LED on for 2/3 of a second.
#define BLINK_ON_TICKS  (TIMER_FREQUENCY_HZ * 3 / 4)
#define BLINK_OFF_TICKS (TIMER_FREQUENCY_HZ - BLINK_ON_TICKS)


#define countof(a)      (sizeof(a) / sizeof(*(a))) //데이터사이즈
#define TxBufferSize   (countof(TxBuffer) - 1)
#define RxBufferSize   0xFF
#define arrSize (countof(arr) - 1)

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"


//--------------------------------------------------------------------------------
GPIO_InitTypeDef GP;   // uart용 gpio
UART_HandleTypeDef UartHandle;
GPIO_InitTypeDef LED;
GPIO_InitTypeDef JOG;
GPIO_InitTypeDef PIEZO;

uint8_t TxBuffer[] = "Hello World! \n\r" ;
uint8_t RxBuffer[RxBufferSize];

void ms_delay_int_count(volatile unsigned int nTime);
void us_delay_int_count(volatile unsigned int n);
void EXTILine_JOG_Config(void);
void UART_Config(void);
void LED_Config(void);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void PIEZO_Config(void);


void ms_delay_int_count(volatile unsigned int nTime){
	nTime = nTime*14000;
	for(;nTime>0; nTime--);
}

 void us_delay_int_count(volatile unsigned int n){
	 for(n *=12; n>0; n--);
 }

 // --------------------------- Config----------------------------

 void PIEZO_Config(void){
	 __HAL_RCC_GPIOB_CLK_ENABLE();

	 PIEZO.Pin = GPIO_PIN_15;
	 PIEZO.Mode = GPIO_MODE_OUTPUT_PP;
	 PIEZO.Pull = GPIO_NOPULL;
	 PIEZO.Speed = GPIO_SPEED_LOW;
	 HAL_GPIO_Init(GPIOB, &PIEZO);

 }

 void LED_Config(void){
		__HAL_RCC_GPIOC_CLK_ENABLE();

		LED.Pin = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5  ;
		LED.Mode = GPIO_MODE_OUTPUT_PP;
		LED.Pull = GPIO_NOPULL;
		LED.Speed = GPIO_SPEED_LOW;
		HAL_GPIO_Init(GPIOC, &LED);
 }

 void EXTILine_JOG_Config(void){
 		__HAL_RCC_GPIOB_CLK_ENABLE();

 		JOG.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
 		JOG.Mode = GPIO_MODE_IT_RISING;
 		JOG.Pull = GPIO_NOPULL;
 		JOG.Speed = GPIO_SPEED_HIGH;
 		HAL_GPIO_Init(GPIOB, &JOG);

 		HAL_NVIC_SetPriority(EXTI0_IRQn,2,0);
 		HAL_NVIC_EnableIRQ(EXTI0_IRQn);

 		HAL_NVIC_SetPriority(EXTI1_IRQn,2,0);
 		HAL_NVIC_EnableIRQ(EXTI1_IRQn);

 		HAL_NVIC_SetPriority(EXTI2_IRQn,2,0);
 		HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  }

void UART_Config(void){
   //UART CLOCK 활성화
   __HAL_RCC_GPIOA_CLK_ENABLE();
   __HAL_RCC_USART1_CLK_ENABLE(); //UART1 : PA9,10(됨) UART2 : PA2,3(안됨)

   //GPIO세팅.
   GP.Pin = GPIO_PIN_9 | GPIO_PIN_10;
   GP.Mode = GPIO_MODE_AF_PP;
   GP.Pull = GPIO_NOPULL;
   GP.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
   GP.Alternate = GPIO_AF7_USART1; //p.371 표
   HAL_GPIO_Init(GPIOA, &GP);

   //UART 세팅
   UartHandle.Instance = USART1;
   UartHandle.Init.BaudRate = 9600;
   UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
   UartHandle.Init.StopBits = UART_STOPBITS_1;
   UartHandle.Init.Parity = UART_PARITY_NONE;
   UartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE; //하드웨어 컨트롤X
   UartHandle.Init.Mode = UART_MODE_TX_RX;
   UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
   HAL_UART_Init(&UartHandle);

   HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
   HAL_NVIC_EnableIRQ(USART1_IRQn);
}

//-----------------Callback---------------------------

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

	 if(GPIO_Pin == GPIO_PIN_0){ // key up
		 uint8_t arr[] = "camera on\n\r";
		 HAL_UART_Transmit(&UartHandle, (uint8_t*)arr, arrSize,0xFFFF);

		 for(int i=0; i<6; i++){
			 HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_2 |GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 );
			 ms_delay_int_count(100);
		 }
	 }
	 else if(GPIO_Pin == GPIO_PIN_1){  // key down
		 uint8_t arr[] = "camera off\n\r";
		 HAL_UART_Transmit(&UartHandle, (uint8_t*)arr, arrSize,0xFFFF);

		 for(int i=0; i<6; i++){
			 HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_2 |GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 );
			 ms_delay_int_count(100);
		 }
	 }

	 else if(GPIO_Pin == GPIO_PIN_2){  // key center
		 //HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_2 |GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 );
	 	 }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
     //RxBuffer[0] += 1;
     HAL_UART_Transmit(huart, (uint8_t*)RxBuffer, 1, 0xFFFF);

     unsigned int period,buf;

     for(int i=0; i<10; i++){
         for(period = 0x1000; period >=1; period--){
        	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
        	 buf = period;
        	 while(buf--);

        	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 0);
        	 buf = period;
        	 while(buf--);
         }
     }

}


int main(int argc, char* argv[])
{
   UART_Config();
   LED_Config();
   EXTILine_JOG_Config();
   PIEZO_Config();

   HAL_UART_Transmit(&UartHandle, (uint8_t*)TxBuffer, TxBufferSize, 0xFFFF);

   while (1)
    {
     HAL_UART_Receive_IT(&UartHandle, (uint8_t*)RxBuffer, 1); //타임아웃이없기때문에 밑에 할일이있으면 한다
    }

}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
