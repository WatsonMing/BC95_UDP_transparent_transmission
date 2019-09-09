#ifndef _SYSTICK_H_
#define _SYSTICK_H_


#ifdef __cplusplus
extern "C" {
#endif
    //////////////////////////////////////////////////////////////////
#ifndef _SYSTICK_H_GLOBAL_
#define     SYSTICK_EXT   extern
#else
#define     SYSTICK_EXT
#endif
	
#include "stm32f10x.h"	

void SysTick_Init(void);
void Delay(uint32_t nTime);
void TimingDelay_Decrement(void);

SYSTICK_EXT u32 sys1msTick;
#define GetSysTime()  sys1msTick

#endif
