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

#include "stm32F4xx.h"
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


GPIO_InitTypeDef LED;
GPIO_InitTypeDef JOG;

void ms_delay_int_count(volatile unsigned int nTime){
	nTime = nTime*14000;
	for(;nTime>0; nTime--);
}

 void us_delay_int_count(volatile unsigned int n){
	 for(n *=14; n>0; n--);
 }


 void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

	 if(GPIO_Pin == GPIO_PIN_0){ // key up
		 HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_2);
	 }
	 else if(GPIO_Pin == GPIO_PIN_1){  // key down
		 HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_3);
	 }

	 else if(GPIO_Pin == GPIO_PIN_2){  // key center
	 		 HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_2);
	 		 HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_3);
	 	 }
	 //HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_3);
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


// void EXTILine2_Config(void){
//	 RCC->AHB1ENR |= 0x02;   // 기존설정 그대로 두면서 gpio b도 쓰겠다
//	 GPIOB->MODER = 0x00000000; // gpio-b 2번 핀 input mode
//	 GPIOB->PUPDR = 0; // no pull up -  pull down
//	 GPIOB->OSPEEDR = 0x00000030;
//
//	 RCC->AHB2ENR = 0x00004000;  // syscfg enable
//
//	 SYSCFG->EXTICR[0] |= 0x00000100;
//
//	 EXTI->RTSR |= 0x00000004;
//	 EXTI->IMR |= 0x00000004;
//
//	 NVIC_SetPriority(EXTI2_IRQn, 2);
//	 NVIC_EnableIRQ(EXTI2_IRQn);
// }

// void EXTI2_IRQHandler(void){
//
//	 if(EXTI->PR == 0x4){
//		 EXTI->PR |= (1<< 2);
//
//		 GPIOC->ODR ^= 0x0004;
//
//	 }
// }

int main(int argc, char* argv[])
{

	__HAL_RCC_GPIOC_CLK_ENABLE();

	LED.Pin = GPIO_PIN_2 | GPIO_PIN_3 ;
	LED.Mode = GPIO_MODE_OUTPUT_PP;
	LED.Pull = GPIO_NOPULL;
	LED.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOC, &LED);

//	EXTILine1_Config();
	EXTILine_JOG_Config();

	 while (1);


 }
  // Infinite loop, never return.


#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
