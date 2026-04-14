/* File: main.c
 * Date: 07.04.2026
 * Purpose: DAC sine wave generation project.
 */
#include "stm32f1xx.h"
#include "main.h"

int main() {
	MODE_R_Setup();
	TIM1_Setup();
	while (1) {
		if (time_cnt >= 3) {
			LED_toggle();
			time_cnt = 0;
		}
	}
	return 0;
}

