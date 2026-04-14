/* File: LED.c
 * Date: 14.04.2026
 * Purpose: PC13 LED setup & control.
 */
#include "stm32f1xx.h"
#include "LED.h"

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

