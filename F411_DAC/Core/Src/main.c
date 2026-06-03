/* File: main.c
 * Date: 15.04.2026
 * Purpose: external DAC sawtooth wave generation.
 * External DAC: DAC8830
 * Vref (for Vpow=3.1V): 1.24 V
 */
#include "stm32f4xx.h"

#define GPIO_AFRL_AFRL4_AF5 (5U << 16U)
#define GPIO_AFRL_AFRL5_AF5 (5U << 20U)
#define GPIO_AFRL_AFRL7_AF5 (5U << 28U)
#define APB2_FREQ (16000000UL) // APB2 default frequency
volatile uint16_t data = 0x0000; // SPI data for external DAC
volatile uint8_t data_changed = 0;

#define CS_LOW()  (GPIOA->BSRR = GPIO_BSRR_BR_4)
#define CS_HIGH() (GPIOA->BSRR = GPIO_BSRR_BS_4)

void SPI1_Send(uint16_t data) {
	// SPI enable -- NSS goes from 1 to 0
	CS_LOW();
	//SPI1->CR1 |= SPI_CR1_SPE;
	while (!(SPI1->SR & SPI_SR_TXE));
	SPI1->DR = data;
	while (!(SPI1->SR & SPI_SR_TXE));  // ← добавить: ждём, пока уйдёт в shift register
	while (SPI1->SR & SPI_SR_BSY);
	volatile uint16_t dummy = SPI1->DR; // ← сбросить RXNE/OVR
	// waiting until the end of transmition
	//while (SPI1->SR & SPI_SR_BSY);
	//SPI1->CR1 &= ~SPI_CR1_SPE;
	CS_HIGH();
}

// SPI1 is on APB2 bus
void SPI1_Setup() {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	// SPI mode setup
	// PA4 - NSS, AF5, MODER=10
	// PA5 - CLK, AF5
	// PA7 - MOSI, AF5
	GPIOA->MODER &= ~(GPIO_MODER_MODE4);
	GPIOA->MODER |= GPIO_MODER_MODE4_1;
	GPIOA->MODER &= ~(GPIO_MODER_MODE5);
	GPIOA->MODER |= GPIO_MODER_MODE5_1;
	GPIOA->MODER &= ~(GPIO_MODER_MODE7);
	GPIOA->MODER |= GPIO_MODER_MODE7_1;
	// SPI alternate function setup
	GPIOA->AFR[0] |= GPIO_AFRL_AFRL4_AF5;
	GPIOA->AFR[0] |= GPIO_AFRL_AFRL5_AF5;
	GPIOA->AFR[0] |= GPIO_AFRL_AFRL7_AF5;
	// SPI setup:
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	// dataframe format: 16 bit
	SPI1->CR1 |= SPI_CR1_DFF;
	// master selection
	SPI1->CR1 |= SPI_CR1_MSTR;
	SPI1->CR1 |= SPI_CR1_CPHA;  // falling edge
	// NSS software (программный)
	SPI1->CR1 |= SPI_CR1_SSM;
	SPI1->CR1 |= SPI_CR1_SSI;
	// NSS hardware (аппаратный)
	//SPI1->CR2 |= SPI_CR2_SSOE;
	// CS
	GPIOA->MODER &= ~GPIO_MODER_MODE4;
	GPIOA->MODER |= GPIO_MODER_MODE4_0;
	// SPI enable
	SPI1->CR1 |= SPI_CR1_SPE;
}

void SysTick_Init() {
	// до скольки он считает
	SysTick->LOAD = (uint32_t)(100-1);
	// internal clock
	SysTick->CTRL |= (1U << 2);
	// exception when counts down zero
	SysTick->CTRL |= (1U << 1);
	// Enable
	SysTick->CTRL |= (1U << 0);
	NVIC_EnableIRQ(SysTick_IRQn);
}

// when SysTick counts down zero, increase DAC data
void SysTick_Handler() {
	if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) {
		if (data < 0xFFFF) {
		    data += 0x1;
		} else {
			data = 0x0;
		}
		data_changed = 1;
	}
}

int main() {
	SPI1_Setup();
	SysTick_Init();
	while(1) {
		if (data_changed) {
			SPI1_Send(data);
			data_changed = 0;
		}
	}
}
