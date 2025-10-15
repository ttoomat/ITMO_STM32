/* main.c
 * author: ttoomat
 * date: October, 13, 2025.
 */
#include <math.h> // sin
#include "init.h"

#define CP_FREQ 100000000

double SIN_FREQ = 15;
double SIN_AMPLITUDE = 1.2;
uint8_t timerUpdate = 0; // if timer updated, this flag will change
uint32_t time = 0; // time counter
uint16_t maxDACData = 4095;

uint8_t isDataRead = 0;
// we can choose 1 or 2 or 3rd operating mode: 0.1, 1, 10, default: 15Hz
// and for amplitude 1.2, 1.3, 1.6, def: 1.5
uint8_t dataLen = 1; // frequency, amplitude. What is amplitude if we have 0-3.3V interval? 0-3 for amplitude 1.5?
uint8_t dataCounter = 0;
uint8_t buf[] = {'a', 'b'};
uint8_t isReceived = 0;
void USART2_IRQHandler() {
	// receive
	if (USART2->SR & USART_SR_RXNE) {
		buf[dataCounter] = (uint8_t)(USART2->DR & 0xFF);
		// снять RXNE
		USART2->SR &= ~USART_SR_RXNE_Msk;
		dataCounter += 1; // считали ещё один бит
	}
	if (dataCounter >= dataLen) {
		isDataRead = 1;
		// command_handler(buf, n);
		dataCounter=0;
	}
}

void USART_change_freq() {
	// frequency
	switch (buf[0]) {
		case '1': {
			SIN_FREQ = 0.15;
			break;
		}
		case '2': {
			SIN_FREQ = 1;
			break;
		}
		case '3': {
			SIN_FREQ = 10;
			break;
		}
		default: {
			SIN_FREQ = 15;
			break;
		}
	}
	switch (buf[0]) {
		case '1': {
			SIN_AMPLITUDE = 1.2;
			break;
		}
		case '2': {
			SIN_AMPLITUDE = 1.3;
			break;
		}
		case '3': {
			SIN_AMPLITUDE = 1.4;
			break;
		}
		default: {
			SIN_AMPLITUDE = 1.5;
			break;
		}
	}
}

// transmit an array
void transmit(const uint8_t* data, uint8_t n) {
	for (uint8_t i = 0; i < n; i++) {
		// пока TXE = 0 будет задержка (TXE = 1 means USART_DR is empty, we can write)
		while (!(USART2->SR & USART_SR_TXE)) {};
		USART2->DR = data[i];
	}
}

void SysTick_Handler() {
	// отсчитал 1kHz частоту
	time = (time + 1 % 10000);
	timerUpdate=1;
}

/*
 * Todo: what HSE frequency? Remake PLL prescalers.
 */
int main() {
	// 1. RCC, Prescalers
	RCC_Init();
	// 2. FPU on
	SCB->CPACR |= (3UL << 10 * 2) | (3UL << 11 * 2);
	// 3-6 - GPIO for UART + UART init + same for DAC
	USART2_Init();
	DAC_Init();
	SysTick_Init();

	while (1) {
		if (timerUpdate) {
			// upload to DAC
			double sin_value = 0.5 * (sin(M_PI / 180.0 * time * SIN_FREQ) + 1.0);
			uint16_t DAC_value = (uint16_t)(2 * SIN_AMPLITUDE * sin_value * maxDACData / 3.3);
			DAC->DHR12R1 = DAC_value;
			timerUpdate = 0;
		}
		// if UART received data
		if (isReceived) {
			USART_change_freq();
			isReceived=0; // снимаем флаг, чтоб потом опять его ставить
		}
	}
}
