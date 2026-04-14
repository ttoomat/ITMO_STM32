/* Date: 07.04.2026
 * Purpose: DAC sine wave generation.
 */

#include <inttypes.h>
#include "stm32f1xx.h"

#define APB2_FREQ (8000000UL)
#define TIM1_FREQ (1UL)

volatile uint32_t time_cnt = 0;

// PC13 led on
void MODE_R_Setup() {
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
	// MODE13 = 10 (Output mode, max speed 2 MHz)
	GPIOC->CRH &= ~(GPIO_CRH_MODE13);
	GPIOC->CRH |= (GPIO_CRH_MODE13_1);
}

// inverted LED logic
void LED_off() {
	GPIOC->ODR |= (GPIO_ODR_ODR13);
}

void LED_on() {
	GPIOC->ODR &= ~(GPIO_ODR_ODR13);
}

void LED_toggle() {
	GPIOC->ODR ^= (GPIO_ODR_ODR13);
}

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

int main() {
	MODE_R_Setup();
	TIM1_Setup();
	while (1) {
		if (time_cnt >= 3) {
			LED_toggle();
			time_cnt = 0;
		}
	}
	return 0;
}

