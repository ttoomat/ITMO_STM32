/* File: TIM1.c
 * Date: 14.04.2026
 * Purpose: TIM1 1 Hz setup with interrupts.
 */
#include "stm32f1xx.h"
#include "TIM1.h"

#define APB2_FREQ (8000000UL)
#define TIM1_FREQ (1UL)

volatile uint32_t time_cnt = 0;

void TIM1_Setup() {
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
	TIM1->ARR = 1001;
	TIM1->PSC = APB2_FREQ / TIM1_FREQ / 1000;
	TIM1->DIER |= TIM_DIER_UIE;
	TIM1->CR1 |= TIM_CR1_CEN;
	NVIC_EnableIRQ(TIM1_UP_IRQn);
}

void TIM1_UP_IRQHandler() {
	if (TIM1->SR & TIM_SR_UIF) {
		time_cnt++;
		TIM1->SR &= ~TIM_SR_UIF;
	}
}

