#include "usart.h"
#include "stm32f10x.h"
#include "bc95.h"
__IO uint8_t usart1_rcvd_flag;
__IO uint8_t usart2_rcvd_flag;


__IO uint8_t usart1_rcvd_len = 0;
__IO uint8_t usart2_rcvd_len = 0;


uint8_t usart1_rcvd_buf[USART1_RX_BUF_LEN];
char usart2_rcvd_buf[USART2_RX_BUF_LEN];


void Init_USART(void)
{
	Init_USART1(115200);
	Init_USART2(9600);
}




//???????
void Init_USART1(uint32_t BaudRate)
{
   USART_InitTypeDef USART_InitStructure;
   NVIC_InitTypeDef NVIC_InitStructure; 
	 GPIO_InitTypeDef  GPIO_InitStructure;

   //?????RCC??
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

   //?????GPIO???
   /* Configure USART1 Rx (PA.10) as input floating */
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
   GPIO_Init(GPIOA, &GPIO_InitStructure);

   /* Configure USART1 Tx (PA.09) as alternate function push-pull */
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
   GPIO_Init(GPIOA, &GPIO_InitStructure);

   //????
   USART_InitStructure.USART_BaudRate = BaudRate;
   USART_InitStructure.USART_WordLength = USART_WordLength_8b;
   USART_InitStructure.USART_StopBits = USART_StopBits_1;
   USART_InitStructure.USART_Parity = USART_Parity_No;
   USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
   USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

   /* Configure USART1 */
   USART_Init(USART1, &USART_InitStructure);//????1

   /* Enable USART1 Receive interrupts ????????*/
   USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
   //??????????????
   //USART_ITConfig(USART1, USART_IT_TXE, ENABLE);

   /* Enable the USART1 */
   USART_Cmd(USART1, ENABLE);//????1

   //??????
   /* Configure the NVIC Preemption Priority Bits */  
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

   /* Enable the USART1 Interrupt */
   NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
}

void Init_USART2(uint32_t BaudRate)
{
   USART_InitTypeDef USART_InitStructure;
   NVIC_InitTypeDef NVIC_InitStructure; 
	 GPIO_InitTypeDef  GPIO_InitStructure;
   //?????RCC??
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 | RCC_APB2Periph_GPIOA, ENABLE);

   //?????GPIO???
   // Configure USART2 Rx (PA.3) as input floating 							
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
   GPIO_Init(GPIOA, &GPIO_InitStructure);

   // Configure USART2 Tx (PA.2) as alternate function push-pull 	
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
   GPIO_Init(GPIOA, &GPIO_InitStructure);

   //????
   USART_InitStructure.USART_BaudRate = BaudRate;
   USART_InitStructure.USART_WordLength = USART_WordLength_8b;
   USART_InitStructure.USART_StopBits = USART_StopBits_1;
   USART_InitStructure.USART_Parity = USART_Parity_No;
   USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
   USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;


   // Configure USART2 
   USART_Init(USART2, &USART_InitStructure);//????2

  // Enable USART1 Receive interrupts ????????
   USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
   //??????????????
   //USART_ITConfig(USART2, USART_IT_TXE, ENABLE);

   // Enable the USART2 
   USART_Cmd(USART2, ENABLE);//????1

   //??????
   //Configure the NVIC Preemption Priority Bits   
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

   // Enable the USART2 Interrupt 
   NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
}




void Stm32_Uartx_Init(USART_TypeDef* USARTx,uint32_t Baudrate,uint16_t Format)
{
	USART_InitTypeDef USART_InitStruct;
	USART_InitStruct.USART_BaudRate = Baudrate;
	switch(Format)
	{
		default:
		case FORMAT_8none:
		{ 
			USART_InitStruct.USART_WordLength = USART_WordLength_8b;
			USART_InitStruct.USART_Parity = USART_Parity_No;
			break;
		}
		case FORMAT_8even:
		{ 
			USART_InitStruct.USART_WordLength = USART_WordLength_9b;
			USART_InitStruct.USART_Parity = USART_Parity_Even;
			break;
		}
		case FORMAT_8odd:
		{ 
			USART_InitStruct.USART_WordLength = USART_WordLength_9b;
			USART_InitStruct.USART_Parity = USART_Parity_Odd;
			break;
		}
	}

	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; 
	USART_Init(USARTx, &USART_InitStruct);
	USART_Cmd(USARTx, ENABLE);
	USART_ClearFlag(USARTx, USART_FLAG_TC);

}


void usart_send_str(USART_TypeDef* USARTx,char *str)
{
	USART_GetFlagStatus(USARTx, USART_FLAG_TC);
	while((*str)!='\0')
	{
	  USART_SendData(USARTx,*str);
		while (USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
		str++;
	}
}

int fputc(int ch, FILE *f)
{
	USART_SendData(USART1, (uint8_t) ch);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);		
	return (ch);

}	

int fgetc(FILE *f)
{
	while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
	return (char)USART_ReceiveData(USART1);
}


char *myitoa(int value, char *string, int radix)
{
    int     i, d;
    int     flag = 0;
    char    *ptr = string;

    /* This implementation only works for decimal numbers. */
    if (radix != 10)
    {
        *ptr = 0;
        return string;
    }

    if (!value)
    {
        *ptr++ = 0x30;
        *ptr = 0;
        return string;
    }

    /* if this is a negative value insert the minus sign. */
    if (value < 0)
    {
        *ptr++ = '-';

        /* Make the value positive. */
        value *= -1;
    }

    for (i = 10000; i > 0; i /= 10)
    {
        d = value / i;

        if (d || flag)
        {
            *ptr++ = (char)(d + 0x30);
            value -= (d * i);
            flag = 1;
        }
    }

    /* Null terminate the string. */
    *ptr = 0;

    return string;

} /* NCL_Itoa */

//字符串转整形
int myatoi(const char *str)
{
	int s=0;
	uint8_t falg=0;
	
	while(*str==' ')
	{
		str++;
	}

	if(*str=='-'||*str=='+')
	{
		if(*str=='-')
		falg=1;
		str++;
	}

	while(*str>='0'&&*str<='9')
	{
		s=s*10+*str-'0';
		str++;
		if(s<0)
		{
			s=2147483647;
			break;
		}
	}
	return s*(falg?-1:1);
}


void USARTx_printf(USART_TypeDef* USARTx,char *Data,...)
{
  const char *str;
  int d;   
  char buf[16];
  va_list ap;
  va_start(ap, Data);
  while ( *Data != 0)
  {	
			USART_GetFlagStatus(USARTx, USART_FLAG_TC);
      if ( *Data == 0x5c )  //'\'
      {									  
              switch ( *++Data )
              {
                      case 'r':
                              USART_SendData(USARTx,0x0d);
                              Data ++;
                              break;

                      case 'n':
                              USART_SendData(USARTx,0x0a);	
                              Data ++;
                              break;
                      
                      default:
                              Data ++;
                          break;
              }			 
      }
      else if ( *Data == '%')
      {
				switch ( *++Data )
				{				
				case 's':	
					str = va_arg(ap, const char *);
					for ( ; *str; str++) 
					{
						USART_SendData(USARTx,*str);
						while (USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
					}
					Data++;
					break;
				case 'd':	
					d = va_arg(ap, int);
					myitoa(d, buf, 10);
					for (str = buf; *str; str++) 
					{
						USART_SendData(USARTx,*str);
						while (USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
					}
					Data++;
					break;
				case 'x':	
					d = va_arg(ap, int);
					myitoa(d, buf, 16);
					for (str = buf; *str; str++) 
					{
						USART_SendData(USARTx,*str);
						while (USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
					}
					Data++;
					break;					
				default:
					Data++;
					break;
			}		 
    } 
    else 
		{
			USART_SendData(USARTx,*Data++);
			while (USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
		}
  }
}

extern uint8_t GPRSRecvFlag ;
extern uint8_t GPRSRecvBuffer[255] ;
void USART2_IRQHandler(void)
{
    u8 tmp;
    

	if( USART_GetITStatus(USART2, USART_IT_RXNE ) == SET )
	{
			tmp = USART_ReceiveData(USART2) & 0xFF;
			usart2_rcvd_buf[usart2_rcvd_len] = tmp;
			usart2_rcvd_flag = 1 ;  	

	if(MAIN_NB_printf==1)
	{USART_SendData(USART1, tmp);	
	}
      if(usart2_rcvd_len >= 2)
			{
			  if((usart2_rcvd_buf[usart2_rcvd_len] == 0x0A) && (usart2_rcvd_buf[usart2_rcvd_len-1] == 0x0D))
			  {				
					GPRSRecvFlag = 1;
					
//					memcpy(GPRSRecvBuffer,usart2_rcvd_buf, usart2_rcvd_len);
				}
			}	
			
			usart2_rcvd_len++;
		
		
	}	
}


void USART1_IRQHandler(void)
{
    u8 tmp;
    

	if( USART_GetITStatus(USART1, USART_IT_RXNE ) == SET )
	{
		
		tmp= USART_ReceiveData(USART1) & 0xFF;	
		usart1_rcvd_buf[usart1_rcvd_len++] = tmp;
		
		if(tmp==';'){
		usart1_rcvd_flag = 1 ; 
		}
		
		
	}	
}



void ShellPutCh(char data)
{

    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);	//等待知道THR变空
    //改动延时时间1--10
  //  ShellDelay(10);
    USART_SendData(USART1, (u8) data);

}



#if 1
void ShellPuts(char *str)
{
    while(*str)
        ShellPutCh(*str++);

}
#endif



void ShellPutChar(char data)
{
    ShellPutCh(data);


}

void ShellPutString(char *str)
{
    while(*str)
        ShellPutCh(*str++);

}

void ShellPutData(u8 *data, u8 len)
{
    while(len--)
        ShellPutCh(*data++);

}






