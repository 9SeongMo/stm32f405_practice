#include "stm32f4xx_hal.h"
#include "stm32f4xx_it.h"

extern TIM_HandleTypeDef TimHandle2,TimHandle3;
extern TIM_OC_InitTypeDef TIM_OCInit;
extern UART_HandleTypeDef UartHandle;

void TIM2_IRQHandler(){
	HAL_TIM_IRQHandler(&TimHandle2);
}

//void TIM3_IRQHandler(){
//	HAL_TIM_IRQHandler(&TimHandle3);
//}

void EXTI0_IRQHandler(){
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void EXTI1_IRQHandler(){
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}

void EXTI2_IRQHandler(){
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}

void USART1_IRQHandler(void){
	HAL_UART_IRQHandler(&UartHandle);
}
