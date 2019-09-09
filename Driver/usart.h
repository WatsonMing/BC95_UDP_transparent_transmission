#ifndef __USART_H
#define __USART_H
#include "stm32F10x.h"
#include "stdio.h"
#include <stdarg.h>


#define USART1_RX_BUF_LEN    128
#define USART2_RX_BUF_LEN    128

//#define USART1_BAUDRATE      9600
#define USART2_BAUDRATE      9600

#define FORMAT_8none         0x00
#define FORMAT_8even         0x01
#define FORMAT_8odd          0x02

#define BC95_RCV_BUF         usart2_rcvd_buf

void Init_USART(void);
void Init_USART1(uint32_t BaudRate);
void Init_USART2(uint32_t BaudRate);
void Stm32_Uartx_Init(USART_TypeDef* USARTx,uint32_t Baudrate,uint16_t Format);


extern __IO uint8_t usart1_rcvd_flag;
extern __IO uint8_t usart2_rcvd_flag;

extern __IO uint8_t usart1_rcvd_len;
extern __IO uint8_t usart2_rcvd_len;


extern uint8_t usart1_rcvd_buf[USART1_RX_BUF_LEN];
extern char usart2_rcvd_buf[USART2_RX_BUF_LEN];
void usart_send_str(USART_TypeDef* USARTx,char *str);
void USARTx_printf(USART_TypeDef* USARTx,char *Data,...);
char *myitoa(int value, char *string, int radix);
int myatoi(const char *str);
#endif

