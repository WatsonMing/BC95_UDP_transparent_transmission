#include "stm32f10x.h"
#include "systick.h"
#include "bc95.h"
#include "usart.h"
#include "timer.h"

//void led_init(void)
//{
//		GPIO_InitTypeDef  gpio_init;
//    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB , ENABLE);
//    gpio_init.GPIO_Pin   = GPIO_Pin_0;
//    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
//    gpio_init.GPIO_Mode  = GPIO_Mode_Out_PP;
//    GPIO_Init(GPIOB, &gpio_init); 

//}	
extern void ShellPutString(char *str);

int main(void)
{
	SystemInit(); 
//	led_init();
	Init_USART();
	SysTick_Init();
	TIM3_Int_Init(4999,7199);//500ms
//	TIM3_Int_Init(1,7199);//500ms
	MAIN_NB_printf=0;
     
	BC95_Test_Demo();
	while(1);
}	









