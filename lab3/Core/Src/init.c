/* init.c
 * author: ttoomat
 * date: October, 7, 2025.
 */

#include "init.h"

#define BAUDRATE 19200
#define APBx_FREQ 16000000

void USART2_MODER_Init() {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

	// USART2
	// PA2 mode: AF (10)
	GPIOA->MODER |= (1U << 5);
	GPIOA->MODER &= ~(1U << 4);
	// PA3 mode: AF (10)
	GPIOA->MODER |= (1U << 7);
	GPIOA->MODER &= ~(1U << 6);

	// PA2 AFRL=AFR[0] 0111: AF07 (USART2_TX)
	GPIOA->AFR[0] &= ~(1U << 11);
	GPIOA->AFR[0] |= (1U << 10);
	GPIOA->AFR[0] |= (1U << 9);
	GPIOA->AFR[0] |= (1U << 8);
	// PA3 AF: AF07 (0111) (USART2_RX)
	GPIOA->AFR[0] &= ~(1U << 15);
	GPIOA->AFR[0] |= (1U << 14);
	GPIOA->AFR[0] |= (1U << 13);
	GPIOA->AFR[0] |= (1U << 12);
}

void USART2_Init() {
	USART2_MODER_Init();
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	// transmit enable, receive enable
	USART2->CR1 |= USART_CR1_TE;
	USART2->CR1 |= USART_CR1_RE;
	// baudrate
	USART2->BRR = APBx_FREQ / BAUDRATE;
	// RXNE interrupt enable
	USART2->CR1 |= USART_CR1_RXNEIE;
	// MVIC USART2 interrupt function
	NVIC_EnableIRQ(USART2_IRQn);
	// USART2 enable
	USART2->CR1 |= USART_CR1_UE;
}

void DAC_Init() {

}
