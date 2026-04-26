/* File: main.c
 * Date: 15.04.2026
 * Purpose: external DAC sine wave generation project.
 * External DAC: DAC8830
 * Vref (for Vpow=3.1V): 1.24 V
 */
#include "stm32f4xx.h"

#define GPIO_AFRL_AFRL4_AF5 (5U << 16U)
#define GPIO_AFRL_AFRL5_AF5 (5U << 20U)
#define GPIO_AFRL_AFRL7_AF5 (5U << 28U)
#define APB2_FREQ (16000000UL)

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

void SPI1_Send(uint16_t data) {
	// SPI enable -- NSS goes from 1 to 0
	//SPI1->CR1 |= SPI_CR1_SPE;
	while (!(SPI1->SR & SPI_SR_TXE));
	SPI1->DR = data;
	// waiting until the end of transmition
	while (SPI1->SR & SPI_SR_BSY);
	//SPI1->CR1 &= ~SPI_CR1_SPE;
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
	// NSS software (программный)
	//SPI1->CR1 |= SPI_CR1_SSM;
	//SPI1->CR1 |= SPI_CR1_SSI;
	// NSS hardware (аппаратный)
	SPI1->CR2 |= SPI_CR2_SSOE;
	// SPI enable
	SPI1->CR1 |= SPI_CR1_SPE;
}

int main() {
	//LED_Setup();
	SPI1_Setup();
	uint16_t data = 0xf000;//0xFFFF;
	//SPI1_Send(data);
	while(1) {
		SPI1_Send(data);
	}
}
