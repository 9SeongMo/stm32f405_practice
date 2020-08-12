#include "stm32F4xx.h"
#include "stm32f4xx_hal.h"

void EXTI0_IRQHandler(){
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void EXTI1_IRQHandler(){
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}

void EXTI2_IRQHandler(){
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}
