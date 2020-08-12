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

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

void ms_delay_int_count(volatile unsigned int nTime);
void us_delay_int_count(volatile unsigned int n);
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim);

GPIO_InitTypeDef LED;
TIM_HandleTypeDef TimHandle2,TimHandle3;
TIM_OC_InitTypeDef TIM_OCInit;


void ms_delay_int_count(volatile unsigned int nTime){
	nTime = nTime*14000;
	for(;nTime>0; nTime--);
}

 void us_delay_int_count(volatile unsigned int n){
	 for(n *=14; n>0; n--);
 }



void TIMER2_Config(void){
	__HAL_RCC_TIM2_CLK_ENABLE();

	TimHandle2.Instance = TIM2;  // timer2 는 APB1 42MHz 에 연결되어있다 42000000Hz
	TimHandle2.Init.Period = 10000 -1;  // 0~9999(ARR) -> 1만번 떨굼 ->  4200Hz
	TimHandle2.Init.Prescaler = 8400 -1; //
	TimHandle2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;  // 디비전 설정 x
	TimHandle2.Init.CounterMode = TIM_COUNTERMODE_UP;   // 0부터 ARR 설정값 까지 올라가면서 카운트
	HAL_TIM_Base_Init(&TimHandle2);
	HAL_TIM_Base_Start_IT(&TimHandle2);

	 TIM_OCInit.OCMode = TIM_OCMODE_TIMING;
	 TIM_OCInit.Pulse = 8000 -1;
	 TIM_OCInit.OCPolarity =TIM_OCPOLARITY_LOW;
	 TIM_OCInit.OCFastMode = TIM_OCFAST_DISABLE;
	 HAL_TIM_OC_Init(&TimHandle2);

	 HAL_TIM_OC_ConfigChannel(&TimHandle2, &TIM_OCInit, TIM_CHANNEL_1);
	 HAL_TIM_OC_Start_IT(&TimHandle2, TIM_CHANNEL_1);

	HAL_NVIC_SetPriority(TIM2_IRQn,0,0);
	HAL_NVIC_EnableIRQ(TIM2_IRQn);

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

	LED_Config();
	TIMER2_Config();

	 while (1);

 }


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim){

	 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2,1);
	 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3,1);

}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef* htim){
	 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2,0);
	 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3,0);
}
  // Infinite loop, never return.


#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
