#include "stm32F4xx.h"
#include "stm32f4xx_hal.h"

extern TIM_HandleTypeDef TimHandle2,TimHandle3;
extern TIM_OC_InitTypeDef TIM_OCInit;

void TIM2_IRQHandler(){
	HAL_TIM_IRQHandler(&TimHandle2);
}
void TIM3_IRQHandler(){
	HAL_TIM_IRQHandler(&TimHandle3);
}
