#ifndef __TIMER_H
#define __TIMER_H
#include "stm32F10x.h"

void TIM2_Int_Init(uint16_t arr,uint16_t psc);
void TIM3_Int_Init(uint16_t arr,uint16_t psc);
void TIM4_Int_Init(uint16_t arr,uint16_t psc);


#endif

