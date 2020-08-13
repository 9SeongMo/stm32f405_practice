/*
 * This file is part of the µOS++ distribution.
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

// UART
#define TxBufferSize (countof(TxBuffer) -1)
#define RxBufferSize 0xFF
#define countof(a) (sizeof(a)/sizeof(*(a)))

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

void ms_delay_int_count(volatile unsigned int nTime);
void us_delay_int_count(volatile unsigned int n);

GPIO_InitTypeDef LED;
GPIO_InitTypeDef GP;
UART_HandleTypeDef UartHandle;

uint8_t TxBuffer[] = "UART Example Translation Success!! \n\r";
uint8_t RxBuffer[RxBufferSize];


void ms_delay_int_count(volatile unsigned int nTime){
	nTime = nTime*14000;
	for(;nTime>0; nTime--);
}

 void us_delay_int_count(volatile unsigned int n){
	 for(n *=14; n>0; n--);
 }

 void UART_config(void){
	 __HAL_RCC_GPIOA_CLK_ENABLE();
	 __HAL_RCC_USART1_CLK_ENABLE();

		GP.Pin = GPIO_PIN_9 | GPIO_PIN_10 ;
		GP.Mode = GPIO_MODE_AF_PP;
		GP.Pull = GPIO_NOPULL;
		GP.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GP.Alternate = GPIO_AF7_USART1;
		HAL_GPIO_Init(GPIOA, &GP);

		UartHandle.Instance = USART1;
		UartHandle.Init.BaudRate = 9600;
		UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
		UartHandle.Init.StopBits = UART_STOPBITS_1;
		UartHandle.Init.Parity = UART_PARITY_NONE;
		UartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
		UartHandle.Init.Mode = UART_MODE_TX_RX;
		UartHandle.Init.OverSampling =UART_OVERSAMPLING_16;

		HAL_UART_Init(&UartHandle);

 }


void LED_Config(void){

	__HAL_RCC_GPIOC_CLK_ENABLE();

	LED.Pin = GPIO_PIN_2 | GPIO_PIN_3 ;
	LED.Mode = GPIO_MODE_OUTPUT_PP;
	LED.Pull = GPIO_NOPULL;
	LED.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOC, &LED);

}

int main(int argc, char* argv[])
{

	//LED_Config();
	UART_config();

	HAL_UART_Transmit(&UartHandle, (uint8_t*) TxBuffer, TxBufferSize, 0xFFFF);

	 while (1);
	  // Infinite loop, never return.

 }


#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
