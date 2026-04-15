/* File: main.c
 * Date: 15.04.2026
 * Purpose: DAC sine wave generation project.
 */
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

void LED_toggle() {
	GPIOC->ODR ^= (GPIO_ODR_ODR_13);
}

int main() {
	LED_Setup();
	while(1) {
		LED_on();
	}
}
