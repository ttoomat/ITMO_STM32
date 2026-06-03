/* File: main.c
 * Date: 03.06.2026
 * Purpose: I2C current sensor use.
 */
#include <inttypes.h>
#include "stm32f4xx.h"

#define GPIO_AFRH_AFRH8_AF4 (4U << 0U)
#define GPIO_AFRH_AFRH9_AF4 (4U << 4U)

#define APB1_FREQ (16000000U)
#define APB1_FREQ_Meg (APB1_FREQ / 1000000U)
#define I2C1_FREQ (100000U)
#define I2C1_CCR_VALUE (APB1_FREQ / I2C1_FREQ / 2)
#define I2C1_TRISE_VALUE (APB1_FREQ_Meg + 1)

/* SCL -- PB8
 * SDA -- PB9
 */
void I2C_init() {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	// MODE = AF (10)
	GPIOB->MODER &= ~(GPIO_MODER_MODE8 | GPIO_MODER_MODE9); // очищаем биты перед установкой
	GPIOB->MODER |= GPIO_MODER_MODE8_1;
	GPIOB->MODER |= GPIO_MODER_MODE9_1;
	// AFRH for 8-15 pins AF=4
	GPIOB->AFR[1] |= GPIO_AFRH_AFRH8_AF4;
	GPIOB->AFR[1] |= GPIO_AFRH_AFRH9_AF4;
	// pull-up (01) for I2C
	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR8 | GPIO_PUPDR_PUPDR9);
	GPIOB->PUPDR |= GPIO_PUPDR_PUPDR8_0;
	GPIOB->PUPDR |= GPIO_PUPDR_PUPDR9_0;
	// open-drain
	GPIOB->OTYPER |= GPIO_OTYPER_OT8 | GPIO_OTYPER_OT9;
	// I2C
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
	// reset
	I2C1->CR1 |= I2C_CR1_SWRST;
	// out of reset
	I2C1->CR1 &= ~I2C_CR1_SWRST;
	// APB2 freq = 16MegHz
	I2C1->CR2 |= APB1_FREQ_Meg;
	// Standart speed 100 kHz
	/* APB1 freq = 16MegHz
	 * 1/100kHz=1/16MegHz*(2x)
	 * 2x=160
	 * x=80 (т.к. 160=t_high+t_low)
	 * CCR=APB2_freq/desired_freq/2
	 */
	I2C1->CCR = I2C1_CCR_VALUE;
	/* TRISE
	 * 1us/ (1/APB1_freq_Meg)+1
	 */
	I2C1->TRISE = I2C1_TRISE_VALUE;
	// enable
	I2C1->CR1 |= I2C_CR1_PE;
}

// NOT READY
void I2C1_byte_read() {
	// wait until the bus is not busy
	while (I2C1->SR2 & I2C_SR2_BUSY);
	// generate start
	I2C->CR1 |= I2C_CR1_START;
}

int main() {
	I2C_init();
	while (1) {

	}
}
