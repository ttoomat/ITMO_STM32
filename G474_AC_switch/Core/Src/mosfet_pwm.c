/* File: mosfet_pwm.c
 * Date: 30.06.2026
 * Purpose: MOSFET constant PWM generation.
 */
#include <inttypes.h>
#include "stm32g4xx.h"

#include "mosfet_pwm.h"

// TIM16 bus, 16MHz
#define APB2_FREQ (16000000U)
#define PWM_duty_cycle (0.5)
#define PWM_desired_freq (1000U)

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

