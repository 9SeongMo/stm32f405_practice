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
#include <string.h>

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


//-------------------------------전역변수-----------------------------------------------------------
GPIO_InitTypeDef GP;   // uart용 gpio
UART_HandleTypeDef UartHandle;
GPIO_InitTypeDef LED;
GPIO_InitTypeDef JOG;
GPIO_InitTypeDef PIEZO;
GPIO_InitTypeDef LCD;
GPIO_InitTypeDef Vibration;
TIM_HandleTypeDef TimHandle2;
TIM_OC_InitTypeDef TIM_OCInit;

uint8_t TxBuffer[] = "Hello World! \n\r" ;
uint8_t RxBuffer[RxBufferSize];

int star[]= {0,0,4,4,5,5,4,3,3,2,2,1,1,0};
int timer_cnt=10000,click=1;
int cnt=0;
//int t_cnt=0;  // 타이머인터럽트 방생시 첫번째 인터럽트 소리만 다르게하려고

void ms_delay_int_count(volatile unsigned int nTime);
void us_delay_int_count(volatile unsigned int n);
void EXTILine_JOG_Config(void);
void UART_Config(void);
void LED_Config(void);
void LCD_Config(void);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void PIEZO_Config(void);
void play_note(int note);
void LCD_write(unsigned char rs, char data);
void lcd_put_string(char* str );
void LCD_init();


 // ----------------------------------------------- Config---------------------------------------------------


void Vibration_Config(void){
   __HAL_RCC_GPIOA_CLK_ENABLE();

   Vibration.Pin = GPIO_PIN_1;
   Vibration.Mode = GPIO_MODE_OUTPUT_PP;
   Vibration.Pull = GPIO_NOPULL;
   Vibration.Speed = GPIO_SPEED_LOW;
   HAL_GPIO_Init(GPIOA, &Vibration);
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

   LCD_init();
}

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
   UartHandle.Init.BaudRate = 115200;    // 윈도우 pc랑은 9600
   UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
   UartHandle.Init.StopBits = UART_STOPBITS_1;
   UartHandle.Init.Parity = UART_PARITY_NONE;
   UartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE; //하드웨어 컨트롤X
   UartHandle.Init.Mode = UART_MODE_TX_RX;
   UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
   HAL_UART_Init(&UartHandle);

   HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
   HAL_NVIC_EnableIRQ(USART1_IRQn);
}


void TIMER2_Config(int period){

   __HAL_RCC_TIM2_CLK_ENABLE();

   TimHandle2.Instance = TIM2;  // timer2 는 APB1 42MHz 에 연결되어있다 42000000Hz
   TimHandle2.Init.Period = period -1;  // 0~9999(ARR) -> 1만번 떨굼 ->  4200Hz  ///  100000-1  -> 10초
   //TimHandle2.Init.Period = 100000 -1;  // 0~9999(ARR) -> 1만번 떨굼 ->  4200Hz  ///  100000-1  -> 10초
   TimHandle2.Init.Prescaler = 8400 -1; // 8400-1  = 1초
   TimHandle2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;  // 디비전 설정 x
   TimHandle2.Init.CounterMode = TIM_COUNTERMODE_UP;   // 0부터 ARR 설정값 까지 올라가면서 카운트
   HAL_TIM_Base_Init(&TimHandle2);
   HAL_TIM_Base_Start_IT(&TimHandle2);

   HAL_NVIC_SetPriority(TIM2_IRQn,3,0);
   HAL_NVIC_EnableIRQ(TIM2_IRQn);

}

//------------------------------------Callback-------------------------------------------------------------------------

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

    if(GPIO_Pin == GPIO_PIN_0){ // key up
       uint8_t arr[] = "o";
       HAL_UART_Transmit(&UartHandle, (uint8_t*)arr, arrSize,0xFFFF);
       for(int i=0; i<6; i++){
          HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_2 |GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 );
          ms_delay_int_count(100);
       }


    }
    else if(GPIO_Pin == GPIO_PIN_1){  // key down
       uint8_t arr[] = "f";
       HAL_UART_Transmit(&UartHandle, (uint8_t*)arr, arrSize,0xFFFF);
       for(int i=0; i<6; i++){
          HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_2 |GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 );
          ms_delay_int_count(100);
       }

    }

    else if(GPIO_Pin == GPIO_PIN_2){  // key center
       timer_cnt = 100000*click;
       //TIMER2_Config(timer_cnt);  // 10000이 들어가면 1초
       click +=1;
       LCD_init();
       char* s1 = "set alarm: ";
       char s2[2];
       sprintf(s2, "%d", click);  // click 문자열로 변환
       lcd_put_string(s1);
       lcd_put_string(s2);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){


//     if(HAL_UART_Receive(&UartHandle, (uint8_t*) RxBuffer,1,0xFFFF) == HAL_OK)
//     {
//    		LCD_init();
//    		LCD_write(1,RxBuffer[0]);


        if(RxBuffer[0] == 'h')
        {   // 바르지 못한 자세일때
           unsigned int period,buf;

           HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, 1);
           ms_delay_int_count(1000);
           HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, 0);

           for(int i=0; i<4; i++)
           {
              for(period = 0x1000; period >=1; period--)
              {
                 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
                 buf = period;
                 while(buf--);

                 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 0);

                 buf = period;
                 while(buf--);
              }
           }
        }

        else if (RxBuffer[0] == 'p')
        {  // 사람이 앉았을때
        	if(cnt == 0)
        	{
        		TIMER2_Config(timer_cnt);  // 10000이 들어가면 1초
        	}

        }
        else if(RxBuffer[0] == 'x')
        {
        	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, 0);
        	__HAL_RCC_TIM2_CLK_DISABLE();
        	cnt=0;
        }

     //}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim){

   if(cnt == 0)
   {
	   play_note(0);
	   cnt=1;
   }
   else
   {
	   play_note(0);play_note(1);play_note(2);
   }


//    for(int i=0; i<sizeof(test)/sizeof(int); i++){
//       play_note(test[i]);
//    }
}

//-----------------------------------------Function----------------------------------------------------

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

void play_note(int note){

   int T;

   if(note == 0) T = 950;  // 도
   else if(note == 1) T = 850; // 레
   else if(note == 2) T = 750; // 미
   else if(note == 3) T = 715; // 파
   else if(note == 4) T =  630; // 솔
   else if(note == 5) T =  550; // 라
   else if (note == -1) T = 1010; // 낮은 시
   else if(note == -2) T = 1100; // 낮은 라
   else if(note == -3) T = 1250;//낮은 솔


   for(int i=0; i<200; i++){
       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
      us_delay_int_count(T);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 0);
      us_delay_int_count(T);
   }

   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 0);
   ms_delay_int_count(100);

}

void ms_delay_int_count(volatile unsigned int nTime){
   nTime = nTime*14000;
   for(;nTime>0; nTime--);
}

 void us_delay_int_count(volatile unsigned int n){
    for(n *=12; n>0; n--);
 }

//---------------------------------------------MAIN-----------------------------------------------

int main(int argc, char* argv[])
{
   UART_Config();
   LED_Config();
   EXTILine_JOG_Config();
   PIEZO_Config();
   click = 1;
   LCD_Config();
   Vibration_Config();
   timer_cnt = 100000*click;

   //HAL_UART_Transmit(&UartHandle, (uint8_t*)TxBuffer, TxBufferSize, 0xFFFF);

   while (1)
    {
     HAL_UART_Receive_IT(&UartHandle, (uint8_t*)RxBuffer, 1); //타임아웃이없기때문에 밑에 할일이있으면 한다
    }

}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
