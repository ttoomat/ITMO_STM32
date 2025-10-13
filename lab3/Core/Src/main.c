/* main.c
 * author: ttoomat
 * date: October, 13, 2025.
 */
#include <math.h> // sin
#include "init.h"

#define CP_FREQ 100000000

uint16_t SIN_FREQ = 15;
float SIN_AMPLITUDE = 1.5;
uint8_t timerUpdate = 0;
uint8_t time = 0;
uint16_t maxDACData = 4095;

uint8_t isDataRead = 0;
// what would be data length?
uint8_t dataLen = 2; // frequency, amplitude
uint8_t dataCounter = 0;
void USART2_IRQHandler() {
	// receive
	if (USART2->SR & USART_SR_RXNE) {
		buf[dataCounter] = (uint8_t)(USART2->DR & 0xFF);
		// снять RXNE
		USART2->SR &= ~USART_SR_RXNE_Msk;
		dataCounter += 1; // считали ещё один бит
	}
	if (dataCounter >= n) {
		isDataRead = 1;
		// command_handler(buf, n);
		dataCounter=0;
	}
}

void TIM2_IRQHandler() {
	// если update event
	if (TIM2->SR & TIM_SR_UIF) {
		// отключили флаг
		TIM2->SR &= ~TIM_SR_UIF;
		// надо послать на DAC что-то или хотя бы поднять флаг что DAC должен что-то изменить
		time = (time + 1) % 15;
		timerUpdate = 1;
	}
}

int main() {
	// 1. RCC, Prescalers
	RCC_Init();
	// 2. FPU on
	SCB->CPACR |= (3UL << 10 * 2) | (3UL << 11 * 2);
	// 3-6 - GPIO for UART + UART init + same for DAC
	USART2_Init();
	DAC_Init();
	TIM2_Init();

	while (1) {
		if (timerUpdate) {
			// делаем что-то с DAC
			// частота и амплитуда синуса важны именно при подстановке в функцию sin
			// хотим от 0 до 1.5?
			// может понадобиться перевести аргумент в градусы (degrees * M_PI) / 180.0
			DAC->DHR12R1 = (SIN_AMPLITUDE/2) * (sin(time / SIN_FREQ) + 1);
			timerUpdate = 0;
		}
	}
}
