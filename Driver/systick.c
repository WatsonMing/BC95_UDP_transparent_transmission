#include "stm32f10x.h"
#include "systick.h"



#define _SYSTICK_H_GLOBAL_

__IO uint32_t TimingDelay;

/*SysTick初始化  ****************************/
void SysTick_Init(void)
{
    if (SysTick_Config(SystemCoreClock / 1000))    //1ms
    { 
        while (1); //初始化失败
    }
		NVIC_SetPriority(SysTick_IRQn, 0x0);
}



/**********供外部调用的延时函数*********************************/

void Delay(__IO uint32_t nTime)
{ 
    TimingDelay = nTime;
    
    while(TimingDelay != 0)
		{
				IWDG_ReloadCounter();	
		}
}







/****SysTick中断调用函数**************************/
void TimingDelay_Decrement(void)
{
    if (TimingDelay != 0x00)
    { 
        TimingDelay--;
    }
		
}


/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
u32  sys1mstick = 0;
void SysTick_Handler(void)
{
	sys1mstick++;
  TimingDelay_Decrement();
}
