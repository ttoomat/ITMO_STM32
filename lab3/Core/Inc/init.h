/* init.h
 * author: ttoomat
 * date: October, 13, 2025.
 */

#ifndef INC_INIT_H_
#define INC_INIT_H_

#include <inttypes.h>
#include "stm32f446xx.h"

void RCC_Init();     // prescalers, clk
void DAC_Init();     // sine Analog output
void USART2_Init();  // for computer communication
void SysTick_Init(); // time for sine

#endif /* INC_INIT_H_ */
