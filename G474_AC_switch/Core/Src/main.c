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
//#define DAC_Saw_freq (50U)
//#define MAX_DAC_DATA (655U)
#define GPIO_AFRL_AFRL4_AF5 (5U << 16U)
#define GPIO_AFRL_AFRL5_AF5 (5U << 20U)
#define GPIO_AFRL_AFRL7_AF5 (5U << 28U)

volatile uint32_t data = 0; // ext DAC data
volatile uint32_t time = 0; // systick time
volatile uint8_t data_changed = 0; // flag

#define CS_LOW()  (GPIOA->BSRR = GPIO_BSRR_BR_4) // SPI1
#define CS_HIGH() (GPIOA->BSRR = GPIO_BSRR_BS_4) // SPI1

// 255 точек синус сдвинутый вверх, максимум 65535
/*
// скрипт для создания такой таблицы:
#include "math.h"
#include <iostream>
using namespace std;

int main() {
  // number_of_points_per_period
  int n = 255;
  // max DAC data
  int max_a = 65535;
  int a = (int)(max_a / 2);
  for (int i = 0; i < n; i++) {
    cout << (int)(a * sin(2 * M_PI * i / n) + a) << ", ";
      if (i % 5 == 4) cout << endl;
  }
}
 */
uint32_t SINE_LUT[255] = {
		32500, 33300, 34100, 34900, 35698, 36493, 37287, 38077, 38864, 39648,
		40427, 41201, 41970, 42733, 43490, 44240, 44983, 45718, 46446, 47165,
		47875, 48576, 49267, 49948, 50618, 51277, 51925, 52561, 53185, 53796,
		54395, 54980, 55551, 56109, 56652, 57180, 57694, 58192, 58674, 59141,
		59592, 60026, 60443, 60843, 61227, 61592, 61940, 62271, 62583, 62877,
		63152, 63409, 63647, 63866, 64066, 64247, 64409, 64551, 64674, 64777,
		64861, 64925, 64969, 64994, 64999, 64984, 64950, 64895, 64821, 64728,
		64615, 64482, 64330, 64159, 63968, 63759, 63530, 63283, 63017, 62732,
		62429, 62108, 61769, 61412, 61037, 60645, 60236, 59811, 59368, 58910,
		58435, 57945, 57439, 56918, 56382, 55832, 55267, 54689, 54097, 53492,
		52875, 52245, 51603, 50949, 50284, 49609, 48923, 48227, 47521, 46807,
		46083, 45352, 44612, 43866, 43112, 42352, 41586, 40814, 40038, 39257,
		38471, 37682, 36890, 36096, 35299, 34500, 33700, 32900, 32099, 31299,
		30499, 29700, 28903, 28109, 27317, 26528, 25742, 24961, 24185, 23413,
		22647, 21887, 21133, 20387, 19647, 18916, 18192, 17478, 16772, 16076,
		15390, 14715, 14050, 13396, 12754, 12124, 11507, 10902, 10310, 9732,
		9167, 8617, 8081, 7560, 7054, 6564, 6089, 5631, 5188, 4763,
		4354, 3962, 3587, 3230, 2891, 2570, 2267, 1982, 1716, 1469,
		1240, 1031, 840, 669, 517, 384, 271, 178, 104, 49,
		15, 0, 5, 30, 74, 138, 222, 325, 448, 590,
		752, 933, 1133, 1352, 1590, 1847, 2122, 2416, 2728, 3059,
		3407, 3772, 4156, 4556, 4973, 5407, 5858, 6325, 6807, 7305,
		7819, 8347, 8890, 9448, 10019, 10604, 11203, 11814, 12438, 13074,
		13722, 14381, 15051, 15732, 16423, 17124, 17834, 18553, 19281, 20016,
		20759, 21509, 22266, 23029, 23798, 24572, 25351, 26135, 26922, 27712,
		28506, 29301, 30099, 30899, 31699
};

// SysTick
#define SYSCLK_FREQ   (16000000U)
#define SysTick_FREQ   (1600)

// TODO: SysTick for delays, Timer for DAC
volatile uint16_t ms_ticks = 0;

// when SysTick counts down zero, increase DAC data
void SysTick_Handler() {
	if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) {
		time = (time + 1) & 0xFF; // чтоб не делать %
		data = SINE_LUT[time];
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
	while (!(SPI1->SR & SPI_SR_TXE));
	SPI1->DR = data;
	while (SPI1->SR & SPI_SR_BSY);
	volatile uint16_t dummy = SPI1->DR; // ← сбросить RXNE/OVR
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
	//TIM16_setup();
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

