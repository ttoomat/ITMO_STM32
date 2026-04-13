/* Date: 07.04.2026
 * Purpose: DAC sine wave generation.
 */
#include "stm32f1xx.h"

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

int main() {
	MODE_R_Setup();
	while(1) {
		LED_off();
	}
	return 0;
}
