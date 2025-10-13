/* init.c
 * author: ttoomat
 * date: October, 13, 2025.
 */

#include "init.h"

#define BAUDRATE 19200
#define APBx_FREQ 16000000

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
	// 4. Config PLL
	// 4.1. Set PLL clock source
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSE;
	// 4.2. Set PLL M, N and P prescaler
	// M = 8 => 001000
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLM_3;
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM_0;
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM_1;
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM_2;
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM_4;
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM_5;
	// N = 100 =>  001100100
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN_8s;
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN_7;
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLN_6;
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLN_5;
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN_4;
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN_3;
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLN_2;
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN_1;
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN_0;
	// P = 2 => 00
	RCC->PLLCFGR &= ~ RCC_PLLCFGR_PLLP_1;
	RCC->PLLCFGR &= ~ RCC_PLLCFGR_PLLP_0;
	// 4.3. Enable PLL. Wait until PLL start
	RCC->CR |= RCC_CR_PLLON;
	while(!(RCC->CR & RCC_CR_PLLRDY)) {}
	// 5. Set Main clock switch to PLL source
	// PLL_P => 10 (System clock switch)
	RCC->CFGR |= RCC_CFGR_SW_1;
	RCC->CFGR &= ~RCC_CFGR_SW_0;
	// check that the switch moves to correct source - wait
	while (!(RCC->CFGR & (RCC_CFGR_SWS_1 & (~RCC_CFGR_SWS_0)))) {}
}

void DAC_MODER_Init() {
	// PA4
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	// PA4 mode: Analog
	GPIOA->MODER |= (1U << 8);
	GPIOA->MODER |= (1U << 9);
}

void DAC_Init() {
	DAC_MODER_Init();
	RCC->APB1ENR |= RCC_APB1ENR_DACEN;
	// channel 1
	DAC->CR |= DAC_CR_EN1;
	// upload data to DAC
	DAC->DHR12R1 = 0xFFF;
}

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

// I chose TIM2 for interrupts. APB1
// если хотим синус частотой 15Гц, то таймер должен быть ещё намного быстрее
void TIM2_Init() {
	// 1. Включение тактирования таймера
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	// 2. Настройка Prescaler
	TIM2->PSC = 10000-1; // 100 000 000 / 10 000 = 10 000
	// 3. Настройка значения ARR (auto reload register)
	TIM2->ARR = 10; // 10 000 / 10 = 1kHz
	// 4. Разрешить прерывание по Update Event
	TIM2->DIER |= (1U << 0); // Update interrupt enable
	NVIC_EnableIRQ(TIM2_IRQn);
	// 5. Запуск счетчика
	TIM2->CR1 |= (1U << 0); // Counter enabled
	TIM2->CNT = 0;
}
