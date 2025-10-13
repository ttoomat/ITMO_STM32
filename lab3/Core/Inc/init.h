/* init.h
 * author: ttoomat
 * date: October, 13, 2025.
 */

#ifndef INC_INIT_H_
#define INC_INIT_H_

#include <inttypes.h>
#include "stm32f446xx.h"

void RCC_Init();
void DAC_Init();
void USART2_Init();
void TIM2_Init();

#endif /* INC_INIT_H_ */
