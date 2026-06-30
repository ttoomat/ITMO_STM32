/* File: main.c
 * Date: 19.06.2026
 * MCU: STM32G474
 * Purpose: MOSFET driver control.
 * MOSFETs: IRF3710.
 * Drivers: NSi6651A.
 */
#include <inttypes.h>
#include "stm32g4xx.h"

#include "mosfet_pwm.h" // PA12
#include "sin_gen.h"    // PA4

int main() {
	// PWM 1000 Hz 50% generation
	TIM16_setup();
	// 50 Hz sin
	DAC1_Setup();
	while (1) {
		DAC1_update_data();
	}
}

