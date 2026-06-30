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
#define GPIO_AFRL_AFRL4_AF5 (5U << 16U)
#define GPIO_AFRL_AFRL5_AF5 (5U << 20U)
#define GPIO_AFRL_AFRL7_AF5 (5U << 28U)

volatile uint32_t data = 0; // ext DAC data
volatile uint32_t time = 0; // systick time
volatile uint8_t data_changed = 0; // flag

#define CS_LOW()  (GPIOA->BSRR = GPIO_BSRR_BR_4) // SPI1
#define CS_HIGH() (GPIOA->BSRR = GPIO_BSRR_BS_4) // SPI1

// 255 точек синус сдвинутый вверх, максимум 4095
/*
// скрипт для создания такой таблицы:
#include "math.h"
#include <iostream>
using namespace std;

int main() {
  // number_of_points_per_period
  int n = 255;
  // max DAC data
  int max_a = 4095;
  int a = (int)(max_a / 2);
  for (int i = 0; i < n; i++) {
    cout << (int)(a * sin(2 * M_PI * i / n) + a) << ", ";
      if (i % 5 == 4) cout << endl;
  }
}
 */
uint32_t SINE_LUT[255] = {
		2047, 2097, 2147, 2198, 2248, 2298, 2348, 2398, 2447, 2497,
		2546, 2595, 2643, 2691, 2739, 2786, 2833, 2879, 2925, 2970,
		3015, 3059, 3103, 3145, 3188, 3229, 3270, 3310, 3349, 3388,
		3426, 3462, 3498, 3534, 3568, 3601, 3633, 3665, 3695, 3725,
		3753, 3780, 3807, 3832, 3856, 3879, 3901, 3922, 3941, 3960,
		3977, 3993, 4008, 4022, 4035, 4046, 4056, 4065, 4073, 4079,
		4085, 4089, 4092, 4093, 4093, 4093, 4090, 4087, 4082, 4076,
		4069, 4061, 4051, 4041, 4029, 4015, 4001, 3985, 3969, 3951,
		3932, 3911, 3890, 3868, 3844, 3819, 3793, 3767, 3739, 3710,
		3680, 3649, 3617, 3584, 3551, 3516, 3481, 3444, 3407, 3369,
		3330, 3290, 3250, 3209, 3167, 3124, 3081, 3037, 2993, 2948,
		2902, 2856, 2809, 2762, 2715, 2667, 2619, 2570, 2521, 2472,
		2423, 2373, 2323, 2273, 2223, 2173, 2122, 2072, 2021, 1971,
		1920, 1870, 1820, 1770, 1720, 1670, 1621, 1572, 1523, 1474,
		1426, 1378, 1331, 1284, 1237, 1191, 1145, 1100, 1056, 1012,
		969, 926, 884, 843, 803, 763, 724, 686, 649, 612,
		577, 542, 509, 476, 444, 413, 383, 354, 326, 300,
		274, 249, 225, 203, 182, 161, 142, 124, 108, 92,
		78, 64, 52, 42, 32, 24, 17, 11, 6, 3,
		0, 0, 0, 1, 4, 8, 14, 20, 28, 37,
		47, 58, 71, 85, 100, 116, 133, 152, 171, 192,
		214, 237, 261, 286, 313, 340, 368, 398, 428, 460,
		492, 525, 559, 595, 631, 667, 705, 744, 783, 823,
		864, 905, 948, 990, 1034, 1078, 1123, 1168, 1214, 1260,
		1307, 1354, 1402, 1450, 1498, 1547, 1596, 1646, 1695, 1745,
		1795, 1845, 1895, 1946, 1996
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

void DAC1_Setup() {
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	// PA4 - аналоговый режим
	GPIOA->MODER |= GPIO_MODER_MODE4;
	RCC->AHB2ENR |= RCC_AHB2ENR_DAC1EN;
	// DAC1_CH1
	DAC1->CR = 0;
	DAC1->MCR = 0;
	DAC1->CR |= DAC_CR_EN1; // channel 1 enable
}

int main() {
	// PWM 1000 Hz 50% generation
	TIM16_setup();
	// for delay_ms
	SysTick_setup();
	// sin
	DAC1_Setup();
	while (1) {
		if (data_changed) {
			DAC1->DHR12R1 = data;
			data_changed = 0;
		}
	}
	return 0;
}

