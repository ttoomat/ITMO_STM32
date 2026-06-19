/* File: main.c
 * Date: 19.06.2026
 * MCU: STM32G474
 * Purpose: MOSFET driver control.
 * MOSFETs: IRF3710.
 * Drivers: NSi6651A.
 */
#include <inttypes.h>
#include "stm32g4xx.h"

// PC6 led on
void LED_Setup() {
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
	// MODE13 = 01, General purpose output
	GPIOC->MODER &= ~(GPIO_MODER_MODE6);
	GPIOC->MODER |= (GPIO_MODER_MODE6_0);
}

void LED_on() {
	GPIOC->ODR |= (GPIO_ODR_OD6);
}

void LED_off() {
	GPIOC->ODR &= ~(GPIO_ODR_OD6);
}

int main() {
	LED_Setup();
	LED_on();
	return 0;
}
