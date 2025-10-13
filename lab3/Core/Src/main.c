/* main.c
 * author: ttoomat
 * date: October, 7, 2025.
 */
#include "init.h"

#define CP_FREQ 100000000
#define SIN_FREQ 15
#define SIN_AMPLITUDE 1.5

void USART2_IRQHandler() {}

void RCC_Init() {
	// 1. Set FLASH latency
	// flash latency One wait state - idk what latency we want
	FLASH->ACR |= (1U << 0);
	// 2. Enable HSE (external clock), wait until HSE is ready
	RCC->CR |= RCC_CR_HSEON;
	while(!(RCC->CR & RCC_CR_HSERDY)) {}
	// 3. Set AHB, APB1, APB2 Prescaler
	// AHB Prescaler = 1
	RCC->CFGR &= ~RCC_CFGR_HPRE_3;
	// APB1 Presc = 4 (101)
	RCC->CFGR |= RCC_CFGR_PPRE1_2;
	RCC->CFGR &= ~RCC_CFGR_PPRE1_1;
	RCC->CFGR |= RCC_CFGR_PPRE1_0;
	// APB2 Presc = 2 (100)
	RCC->CFGR |= RCC_CFGR_PPRE1_2;
	RCC->CFGR &= ~RCC_CFGR_PPRE1_1;
	RCC->CFGR &= ~RCC_CFGR_PPRE1_0;
	//RCC->CR |= RCC_CR_PLLON;
}

int main() {
	RCC_Init();
	// 2. FPU on
	SCB->CPACR |= (3UL << 10 * 2) | (3UL << 11 * 2);
	// 3-6 - GPIO for UART + UART init + same for DAC
	USART2_Init();
	DAC_Init();

	while (1) {}
}
