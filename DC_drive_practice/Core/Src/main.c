#include <inttypes.h>
#include "stm32f4xx.h"

// PC13 led on
void LED_Setup() {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
	// MODE13 = 01, General purpose output
	GPIOC->MODER &= ~(GPIO_MODER_MODE13);
	GPIOC->MODER |= (GPIO_MODER_MODE13_0);
}

// inverted LED logic
void LED_off() {
	GPIOC->ODR |= (GPIO_ODR_ODR_13);
}

void LED_on() {
	GPIOC->ODR &= ~(GPIO_ODR_ODR_13);
}

int main() {
	LED_Setup();
	LED_on();
	return 0;
}
