#include "stm32f10x.h"
#include "systick.h"



#define _SYSTICK_H_GLOBAL_

__IO uint32_t TimingDelay;

/*SysTick��ʼ��  ****************************/
void SysTick_Init(void)
{
    if (SysTick_Config(SystemCoreClock / 1000))    //1ms
    { 
        while (1); //��ʼ��ʧ��
    }
		NVIC_SetPriority(SysTick_IRQn, 0x0);
}



/**********���ⲿ���õ���ʱ����*********************************/

void Delay(__IO uint32_t nTime)
{ 
    TimingDelay = nTime;
    
    while(TimingDelay != 0)
		{
				IWDG_ReloadCounter();	
		}
}







/****SysTick�жϵ��ú���**************************/
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
