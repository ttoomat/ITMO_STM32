/* main.c
 * author: ttoomat
 * date: October, 13, 2025.
 */
#include "init.h"

#define CP_FREQ 100000000
#define SIN_FREQ 15
#define SIN_AMPLITUDE 1.5

void USART2_IRQHandler() {}

int main() {
	// 1. RCC, Prescalers
	RCC_Init();
	// 2. FPU on
	SCB->CPACR |= (3UL << 10 * 2) | (3UL << 11 * 2);
	// 3-6 - GPIO for UART + UART init + same for DAC
	USART2_Init();
	DAC_Init();

	while (1) {}
}
