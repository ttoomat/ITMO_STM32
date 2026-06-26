/* File: main.c
 * Date: 19.06.2026
 * MCU: STM32G474
 * Purpose: MOSFET driver control.
 * MOSFETs: IRF3710.
 * Drivers: NSi6651A.
 */
#include <inttypes.h>
#include "stm32g4xx.h"

// TIM16 bus, 16MHz
#define APB2_FREQ (16000000U)
#define PWM_duty_cycle (0.5)
#define PWM_desired_freq (1000U)

// SPI1
#define DAC_Saw_freq (50U)
#define MAX_DAC_DATA (655U)
#define GPIO_AFRL_AFRL4_AF5 (5U << 16U)
#define GPIO_AFRL_AFRL5_AF5 (5U << 20U)
#define GPIO_AFRL_AFRL7_AF5 (5U << 28U)

volatile uint32_t data = 0x0000; // SPI data for external DAC
volatile uint8_t data_changed = 0; // flag

#define CS_LOW()  (GPIOA->BSRR = GPIO_BSRR_BR_4) // SPI1
#define CS_HIGH() (GPIOA->BSRR = GPIO_BSRR_BS_4) // SPI1

// SysTick
#define SYSCLK_FREQ   (16000000U)
#define SysTick_FREQ   (DAC_Saw_freq * MAX_DAC_DATA)

// TODO: SysTick for delays, Timer for DAC
volatile uint16_t ms_ticks = 0;

// when SysTick counts down zero, increase DAC data
void SysTick_Handler() {
	if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) {
		if (data < 0xFFFF) {
		    data += 100;
		} else {
			data = 0x0;
		}
		data_changed = 1;
	}
}

void SysTick_setup() {
  // 1. Program reload value.
  SysTick->LOAD = (uint32_t)(SYSCLK_FREQ / SysTick_FREQ - 1);
  // 2. Clear current value.
  SysTick->VAL = 0;
  // 3. Program Control and Status register.
  // internal clock, exception when counts down zero, enable
  SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
		  SysTick_CTRL_TICKINT_Msk |
		  SysTick_CTRL_ENABLE_Msk;
  // no need in NVIC setup!
}

// PA12 - TIM16_CH1
// General purpose timer, PWM
// no interrupts
void TIM16_setup() {
	// GPIO setup
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	// MODE12 = 10, Alternate function
	GPIOA->MODER &= ~(GPIO_MODER_MODE12);
	GPIOA->MODER |= GPIO_MODER_MODE12_1;
	// AFRH12 = AF1 (0001)
	GPIOA->AFR[1] &= ~(GPIO_AFRH_AFSEL12);
	GPIOA->AFR[1] |= GPIO_AFRH_AFSEL12_0;

	// Timer setup
	RCC->APB2ENR |= RCC_APB2ENR_TIM16EN;
	// 1. Настроить CCMR1
	// PWM 1 mode (0110) = PWM output
	TIM16->CCMR1 &= ~(TIM_CCMR1_OC1M);
	TIM16->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;
	// изменение скважности только по update event (= preload)
	TIM16->CCMR1 |= TIM_CCMR1_OC1PE;
	// 2. Настроить CCER (разрешение выхода)
	TIM16->CCER |= TIM_CCER_CC1E;
	// 3. Настроить BDTR (MOE = 1)
	TIM16->BDTR |= TIM_BDTR_MOE;
	// 4. Записать PSC и ARR
	TIM16->PSC = 1600-1;
	TIM16->ARR = APB2_FREQ / (TIM16->PSC + 1) / PWM_desired_freq - 1;
	// 5. Записать начальное значение в CCR1
	TIM16->CCR1 = TIM16->ARR * PWM_duty_cycle;
	// 6. Сгенерировать событие обновления
	TIM16->EGR |= TIM_EGR_UG;
	// 7. Запустить таймер
	// Timer enable
	TIM16->CR1 |= TIM_CR1_CEN;
}

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

// SPI1 for external DAC
// SPI1 is on APB2 bus
void SPI1_Setup() {
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
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
	SPI1->CR1 &= ~SPI_CR1_SPE;
	// dataframe format: 16 bit
	SPI1->CR2 |= SPI_CR2_DS_3 | SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0;
	SPI1->CR2 |= SPI_CR2_FRXTH; // FIFO threshold = 16-bit
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

int main() {
	// PWM 1000 Hz 50% generation
	TIM16_setup();
	// for delay_ms
	SysTick_setup();
	SPI1_Setup();
	while (1) {
		if (data_changed) {
			SPI1_Send(data);
			data_changed = 0;
		}
	}
	return 0;
}

