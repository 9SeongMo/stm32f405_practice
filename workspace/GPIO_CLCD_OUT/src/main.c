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
#include <string.h>
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

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

void ms_delay_int_count(volatile unsigned int nTime);
void us_delay_int_count(volatile unsigned int n);

GPIO_InitTypeDef LCD;


void ms_delay_int_count(volatile unsigned int nTime){
	nTime = nTime*14000;
	for(;nTime>0; nTime--);
}

 void us_delay_int_count(volatile unsigned int n){
	 for(n *=12; n>0; n--);
 }



void LCD_Config(void){

	__HAL_RCC_GPIOC_CLK_ENABLE();

	LCD.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15 ;
	LCD.Mode = GPIO_MODE_OUTPUT_PP;
	LCD.Pull = GPIO_NOPULL;
	LCD.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOC, &LCD);

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);

}

void LCD_write(unsigned char rs, char data){  //rs는 0과 1을 가지는 옵션플래그
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, rs);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
	us_delay_int_count(2);

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, (data >> 4) & 0x1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, (data >> 5) & 0x1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, (data >> 6) & 0x1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, (data >> 7) & 0x1);

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
	us_delay_int_count(2);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
	us_delay_int_count(2);

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, (data >> 0) & 0x1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, (data >> 1) & 0x1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, (data >> 2) & 0x1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, (data >> 3) & 0x1);

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
	us_delay_int_count(2);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
	ms_delay_int_count(2);
}

void LCD_init(){
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
	LCD_write(0,0x33);
	LCD_write(0,0x32);
	LCD_write(0,0x28);
	LCD_write(0,0x0F);
	LCD_write(0,0x01);
	LCD_write(0,0x06);
	LCD_write(0,0x02);

}

void lcd_put_string(char* str ){
	char* s = str;
	int len = strlen(str);

	for (int i =0; i<len; i++){
		LCD_write(1,s[i]);
		if(i >2 && i%16 ==0){
			ms_delay_int_count(2000);
			LCD_init();
		}
	}
}

int main(int argc, char* argv[])
{

	LCD_Config();
	LCD_init();
	char* s = "AAAAAAAAAAAAAAAABBBBBBBBBB";

	lcd_put_string(s);

//	for(int i=0; i<sizeof(arr); i++){
//		LCD_write(1,arr[i]);
//	}

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
