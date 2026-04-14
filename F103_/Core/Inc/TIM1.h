/* File: TIM1.h
 * Date: 14.04.2026
 * Purpose: TIM1 headers & extern variable.
 */
#ifndef INC_TIM1_H_
#define INC_TIM1_H_

extern volatile uint32_t time_cnt;

void TIM1_Setup();
void TIM1_UP_IRQHandler();

#endif /* INC_TIM1_H_ */

