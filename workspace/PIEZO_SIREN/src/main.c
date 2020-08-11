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
void play_note(int note);

// 도 : 262Hz  레:293Hz   미: 329  파: 349  솔:392  라:440  시: 493

// on - off 200회 반복 -> 소리나옴 (0.8초)     off는 (0.2초)
void play_note(int note){
	int T = 0;

	if(note == 0) T = 1900;
	else if(note == 1) T = 1700;
	else if(note == 2) T = 1500;
	else if(note == 3) T = 1400;
	else if(note == 4) T =  1250;
	else if(note == 5) T =  1100;
	else if (note == -1) T = 2050;


	for(int i=0; i<200; i++){
		GPIOB->ODR = 0x8000;
		us_delay_int_count(T);
		GPIOB->ODR = 0x0000;
		us_delay_int_count(T);
	}
	GPIOB->ODR = 0x0000;
	ms_delay_int_count(150);

}

void ms_delay_int_count(volatile unsigned int nTime){
	nTime = nTime*14000;
	for(;nTime>0; nTime--);
}

 void us_delay_int_count(volatile unsigned int n){
	 for(n *=14; n>0; n--);
 }

int main(int argc, char* argv[])
{
	RCC->AHB1ENR = 0x00000002; // gpio b에 clock 공급
	GPIOB->MODER = 0x40000000; // gpio-b 15번 핀 output mode
	GPIOB->OTYPER = 0; // gpio-a 15번핀 push-pull
	GPIOB->PUPDR = 0; // no pull up -  pull down
	GPIOB->OSPEEDR = 0;

	unsigned int period,buf;
	int bani[] = {0,0,1,2,0,2,1,0,0,1,2,0,-1,0,0,1,2,3,2,1,0,-1};
  // Infinite loop
  while (1){

	 for(int i=0; i<sizeof(bani); i++){
		 play_note(bani[i]);
	 }
//	  for(period = 0x1000; period >=1; period--){
//		  GPIOB->ODR = 0x8000;
//		  buf = period;
//		  while(buf--);
//
//		  GPIOB->ODR = 0x0000;
//		  buf = period;
//		  while(buf--);
//	  }

    }
  // Infinite loop, never return.
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
