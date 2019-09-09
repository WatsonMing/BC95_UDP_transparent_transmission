#include "bc95.h"
#include "stmflash.h"



//要写入到STM32 FLASH的字符串数组
const u8 TEXT_Buffer[80]={"STM32F103 FLASH TEST"};
#define SIZE sizeof(TEXT_Buffer)		//数组长度HeartMsg
#define FLASH_SAVE_ADDR  0X08010800		

////????STM32Ff103C8T6 ? 00x8010000~0x801FFFF?????;

uint8_t RecvSignal[] = "+NSONMI:";
uint8_t RealSingal[] = ",6800,";
uint8_t AT_NSORF[] = "AT+NSORF=1,256\r";
uint8_t BC95_IP[30] = "0";
uint8_t BC95_IP_PORT[10] = "0";
uint8_t BC95_IP_LEN=0;
uint8_t BC95_IP_PORT_LEN=0;
uint8_t HeartMsg[30] = "0";
uint8_t HeartTime[10] = "0";
uint8_t NB_DATA[100] = "0";
uint8_t NB_CIMI[15] = "0";
uint8_t NB_CIMI_INT[15] = "0";
uint8_t NB_DATA_LEN[2]="0";
uint8_t HeartMsg_LEN=0;
uint8_t HeartTime_LEN=0;
uint8_t NB_MODE_BIT=0;
uint8_t MAIN_NB_printf=0;
unsigned char HeartMsgDATA[60]="0",HeartMsgDATA_LEN[2]="0";
unsigned long time_s=0;
unsigned long time_n=0;
#define	GPRS_BUFFER_SIZE		(255)

uint8_t GPRSRecvFlag = 0;

uint8_t GPRSRecvBuffer[GPRS_BUFFER_SIZE];
uint8_t GPRSRecvData[GPRS_BUFFER_SIZE];


uint8_t check_ack_timeout = 10;
uint8_t ue_exist_flag = 0;
uint8_t ue_need_reboot_flag = 0;

#define  delay_ms    Delay  


//检查返回的响应是否符合预期
//传入参数为预期返回的字符串
//返回0，为检测到预期值
//其他值，预期字符所在的位置
uint8_t* BC95_check_ack(char *str)
{
	
	char *strx=0;
	NB_MODE_BIT=0;
	if(usart2_rcvd_flag)		
	{ 
		usart2_rcvd_buf[usart2_rcvd_len]='\0';
		strx=strstr((const char*)usart2_rcvd_buf,(const char*)str);
	} 
	return (uint8_t*)strx;
}

//发生at指令函数
//cmd:at指令，ack：预期响应，waittime,超时时间
//返回0，发送成功
//返回1，发送超时
uint8_t BC95_send_cmd(char *cmd,char *ack,uint16_t waittime)
{
	uint8_t res=0; 
	usart2_rcvd_flag=0;
	usart2_rcvd_len = 0;
	GPRSRecvFlag = 0;
	memset(usart2_rcvd_buf,0,USART2_RX_BUF_LEN);
//	printf("%s\r\n",cmd);
	USARTx_printf(USART2,"%s\r\n",cmd);
//		USARTx_printf(USART1,"%s\r\n",cmd);
	if(ack&&waittime)
	{
		while(--waittime)	
		{
			delay_ms(20);
			if(usart2_rcvd_flag)
			{
				
				if(BC95_check_ack(ack))break;
				usart2_rcvd_flag=0;
			} 
		}
		if(waittime==0)res=1; 
	}
	return res;
} 
//上电程序，检测模块是否连接，检查配置是否为自动模式，是否为需要的频段

void BC95_power_on(void)
{
	//BC95_send_cmd(SET_UE_REBOOT,"REBOOT",100);
	check_ack_timeout = 10;
	ue_exist_flag = 1;
	while(BC95_send_cmd("AT","OK",100)&&check_ack_timeout)
	{
		if(check_ack_timeout)
		{
			check_ack_timeout--;
			ue_exist_flag = 0;
//			usart_send_str(USART1,"等待模块上电 \r\n");
		}
		delay_ms(1000);
	}
	//判断模块是否是自动连接模式，如果不是则将模块设置成自动模式
	if(ue_exist_flag&&!BC95_send_cmd(QUERY_UE_CONNECT_MODE,"AUTOCONNECT,FALSE",100))
	{
		check_ack_timeout = 3;
		while(check_ack_timeout)
		{
			check_ack_timeout--;
			if(BC95_send_cmd(SET_UE_AUTOCONNECT,"OK",100))
			{
//				USARTx_printf(USART1,"设置为自动模式成功！\r\n");
				break;
			}
			delay_ms(1000);
		}
		ue_need_reboot_flag =1;
	}
	//判断模块是否是默认设置频段，如果不是则设置成默认频段
	if(ue_exist_flag&&BC95_send_cmd(QUERY_UE_BAND,UE_DEFAULT_BAND,100))
	{
		;
		BC95_send_cmd(SET_UE_DEFAULT_BAND,UE_DEFAULT_BAND,100);
//		USARTx_printf(USART1,"设置默认频段！\r\n");
		ue_need_reboot_flag = 1;
	}
	//重启模块生效配置
	if(ue_exist_flag&&ue_need_reboot_flag)
	{
		ue_need_reboot_flag = 0;
		check_ack_timeout = 10;
		BC95_send_cmd(SET_UE_REBOOT,"REBOOT",100);
//		USARTx_printf(USART1,"重启模块！\r\n");
		while(check_ack_timeout&&!BC95_check_ack("Neul"))
		{
			if(BC95_check_ack("Neul"))
			{
				break;
			}else
			{
				check_ack_timeout--;
				delay_ms(1000);
			}
		}
	}
}



//检查模块的网络状态，检测器件LED1会闪烁，LED1常亮为附网注网成功
//此函数不检查联网状态，仅检查附网注网状态，联网状态可以使用BC95_send_cmd，单独检测
//附网注网失败或者超时返回0，返回1附网注网成功，返回2附网成功
uint8_t query_net_status(void)
{
	uint8_t res = 0;
	uint8_t attached_flag = 0;
	uint8_t registered_flag = 0;
	check_ack_timeout = 20;
//	led_need_blink = 1;
	while(!(attached_flag&&registered_flag)&&check_ack_timeout)
	{
		if(!BC95_send_cmd(QUERY_UE_SCCON_STATS,SET_UE_SCCON,100))
		{
			attached_flag = 1;
			registered_flag = 1;
			res = 1;
//			led_need_blink =0;
//			setLEDs(LED1);
//			USARTx_printf(USART1,"附网、注网成功！r\n");
			break;
		}else
		{
			if(!attached_flag)
			{
				if(!BC95_send_cmd(QUERY_UE_ATTACH_STATS,UE_ATTACHED_STATS,100))
				{
//					USARTx_printf(USART1,"附网成功!\r\n");
					attached_flag = 1;
					res =2;
				}else
				{
//					USARTx_printf(USART1,"正在附网...\r\n");
//					setLEDs(LED1);
					attached_flag = 0;
				}
			}
			if(attached_flag&&!registered_flag)
			{
				if(attached_flag&&!BC95_send_cmd(QUERY_UE_EREG_STATS,UE_EREGISTERED_STATS,100))
				{
//					USARTx_printf(USART1,"注网成功！\r\n");
					registered_flag = 1;
//					led_need_blink =0;
//					setLEDs(LED1);
					res =1;
				}else
				{
//					USARTx_printf(USART1,"正在注网...\r\n");
					registered_flag = 0;
				}
			}				
		}
		check_ack_timeout--;
		delay_ms(500);
		if(!check_ack_timeout&&!attached_flag&&!registered_flag)
		{
//			led_need_blink =0;
//			ResetLEDs(LED1);
//			USARTx_printf(USART1,"附网、注网失败！\r\n");
		}
	}
	return res;
}

//读取数据，截取接收缓存中所需的数据保存到des,pos为起始地址，len为截取长度
void get_str_data(char* des,char pos,char len)
{
	memcpy(des,usart2_rcvd_buf+pos,len);
}	

//创建UDP链接，传入本地UDP端口号，返回0-6的socket id号，
uint8_t creat_UDP_socket(char* local_port)
{
	char data[10]="";
	uint8_t socket_id = 7;
	char temp[64]="AT+NSOCR=DGRAM,17,";
	strcat(temp,local_port);
	strcat(temp,",1");
//	if(!BC95_send_cmd(temp,"OK",100))
//	{
//		get_str_data(data,2,1);
//		socket_id = (uint8_t)myatoi(data);
//		USARTx_printf(USART1,"Socket创建成功，句柄ID--> %d！\r\n",socket_id);
//		return socket_id;
//	}
//	USARTx_printf(USART1,"Socket创建失败，已经创建或端口被占用！\r\n");
	return socket_id;
}
void NPING_IP(char* local_port)
{
	char temp[30]="AT+NPING=";
	strcat(temp,local_port);
	if(!BC95_send_cmd(temp,"OK",100))
	{
//		USARTx_printf(USART1,"%s！\r\n",temp);
	}
	//else USARTx_printf(USART1,"PING_INON！\r\n");
}
void NRB(void)
{
	char temp[6]="AT+NRB";
	if(!BC95_send_cmd(temp,"OK",100))
	{
		USARTx_printf(USART1,"%s！\r\n",temp);
	}
	//else USARTx_printf(USART1,"PING_INON！\r\n");
}
//创建UDP链接，传入本地UDP端口号，返回0-6的socket id号，
uint8_t creat_TCP_socket(char* local_port)
{
	char data[10]="";
	uint8_t socket_id = 7;
	char temp[64]="AT+NSOCR=STREAM,6,";
	strcat(temp,local_port);
	strcat(temp,",1");
	if(!BC95_send_cmd(temp,"OK",100))
	{
		get_str_data(data,2,1);
		socket_id = (uint8_t)myatoi(data);
//		USARTx_printf(USART1,"Socket创建成功，句柄ID--> %d！\r\n",socket_id);
		return socket_id;
	}
//	USARTx_printf(USART1,"Socket创建失败，已经创建或端口被占用！\r\n");
	return socket_id;
}


//创建UDP链接，传入本地UDP端口号，返回0-6的socket id号，
uint8_t connect_TCP_srever(char *socket,char *hostIP,char *port)//
{
	char ptr[600]="AT+NSOCO=";
	strcat(ptr,socket);
	strcat(ptr,",");
	strcat(ptr,hostIP);
	strcat(ptr,",");
	strcat(ptr,port);
	if(!BC95_send_cmd(ptr,"OK",200))
	{
//		USARTx_printf(USART1,"TCP连接服务器成功 --> %s！\r\n",ptr);	
		return 0;
	}
	return 1;

}


uint8_t send_TCP_msg(char *socket,char *dataLen,char *data)
{
	char ptr[600]="AT+NSOSD=";
	strcat(ptr,socket);
	strcat(ptr,",");
	strcat(ptr,dataLen);
	strcat(ptr,",");
	strcat(ptr,data);
	if(!BC95_send_cmd(ptr,"OK",200))
	{
//		USARTx_printf(USART1,"发送数据--> %s！\r\n",ptr);	
		return 0;
	}
	return 1;
}
//发送数据函数，传入socket,主机IP，远程主机端口，数据长度，数据
//这里暂时使用字符串参数
//返回值0，发送成功（鉴于UDP为报文传输，数据主机是否接收到模块是无法确认的）
//返回值1，发送失败
uint8_t send_UDP_msg(char *socket,char *hostIP,char *port,char *dataLen,char *data)
{
	char ptr[600]="AT+NSOST=";
	strcat(ptr,socket);
	strcat(ptr,",");
	strcat(ptr,hostIP);
	strcat(ptr,",");
	strcat(ptr,port);
	strcat(ptr,",");
	strcat(ptr,dataLen);
	strcat(ptr,",");
	strcat(ptr,data);
	if(!BC95_send_cmd(ptr,"OK",200))
	{
//		USARTx_printf(USART1,"发送数据--> %s！\r\n",ptr);	
		return 0;
	}
	else
	{
////		USARTx_printf(USART1,"发送数据--> %s！\r\n",ptr);	
	}
	return 1;
}
//接收数据处理函数，暂不提供实现方法
uint8_t *receive_udp(char *socket,char *dataLen)
{

	return 0;
}

#define		FULL		(0xFF)

/* 查找字符串*/
uint8_t LookForStr(uint8_t *s, uint8_t *t)
{
	uint8_t i = 0;
	uint8_t *s_temp;
	uint8_t *m_temp;
	uint8_t *t_temp;
	
	if (s==0 || t==0) return 0;
	
	for (s_temp=s; *s_temp!='\0'; s_temp++,i++)
	{
		for (m_temp=s_temp, t_temp=t; *t_temp!='\0' && *t_temp==*m_temp; t_temp++, m_temp++);
		if (*t_temp == '\0')
		{
			return i;
		}
	}
	
	return FULL;
}

typedef unsigned char  BOOLEAN;
typedef unsigned char  INT8U;                    /* Unsigned  8 bit quantity                           */
typedef signed   char  INT8S;                    /* Signed    8 bit quantity                           */
typedef unsigned short INT16U;                   /* Unsigned 16 bit quantity                           */
typedef signed   short INT16S;                   /* Signed   16 bit quantity                           */
typedef unsigned long  INT32U;                   /* Unsigned 32 bit quantity                           */
typedef signed   long  INT32S;                   /* Signed   32 bit quantity                           */
typedef float          FP32;                     /* Single precision floating point                    */
typedef double         FP64;                     /* Double precision floating point                    */

#ifndef __cplusplus
    typedef enum {FALSE = 0, TRUE = !FALSE} bool;
#endif

BOOLEAN GetField(INT8U *pData, INT8U *pField, INT32S nFieldNum, INT32S nMaxFieldLen)
{
    INT32S i, i2, nField;
    //
    // Validate params
    //
    if(pData == NULL || pField == NULL || nMaxFieldLen <= 0)
    {
        return FALSE;
    }

    //
    // Go to the beginning of the selected field
    //
    i = 0;
    nField = 0;
    while(nField != nFieldNum && pData[i])
    {
        if(pData[i] == ',')
        {
            nField++;
        }

        i++;

        if(pData[i] == NULL)
        {
            pField[0] = '\0';
            return FALSE;
        }
    }

    if(pData[i] == ',' || pData[i] == '*')
    {
        pField[0] = '\0';
        return FALSE;
    }

    //
    // copy field from pData to Field
    //
    i2 = 0;
    while(pData[i] != ',' && pData[i] != '*' && pData[i])
    {
        pField[i2] = pData[i];
        i2++;
        i++;

        //
        // check if field is too big to fit on passed parameter. If it is,
        // crop returned field to its max length.
        //
        if(i2 >= nMaxFieldLen)
        {
            i2 = nMaxFieldLen - 1;
            break;
        }
    }
    pField[i2] = '\0';

    return TRUE;
}

static void HexToStr(char *str, const char *hex, int hex_len)
{
	int i = 0;
	unsigned char tmp = 0;
	
	for(i = 0; i < hex_len; i++)
	{
		tmp = hex[i] >> 4;
		if(tmp > 9)
			*str++ = tmp + 0x37;
		else
			*str++ = tmp + '0';
		
		tmp = hex[i] & 0xF;
		if(tmp > 9)
			*str++ = tmp + 0x37;
		else
			*str++ = tmp + '0';
	}
}

static int StrToHex(char *hex, const char *str)
{
	int hex_len = strlen(str)/2;
	unsigned char tmp_val = 0;
	int i = 0;
	
	for(i = 0; i < hex_len; i++)
	{
		tmp_val = ((str[2 * i] > '9') ? (str[2 * i] - 0x37) : (str[2 * i] - '0'));
		*hex = tmp_val << 4;
		tmp_val = ((str[2 * i + 1] > '9') ? (str[2 * i + 1] - 0x37) : (str[2 * i + 1] - '0'));
		*hex++ |= tmp_val;
	}
	return hex_len;
}



extern u32  sys1mstick;
#define GetSysTick()  sys1mstick;

#define MAXFIELD	250


uint8_t RecvGPRSData(uint8_t *buffer, uint8_t *pDat)
{
	uint16_t i, len;
	uint8_t  Dat[MAXFIELD];
	GetField(buffer,Dat,3,MAXFIELD);
	len = atoi((char *)Dat);
	
	GetField(buffer,Dat,4,MAXFIELD);
	StrToHex(pDat,Dat);
//	USARTx_printf(USART1," \r\n%s","Rev Dat: ");
//	for(i=0; i < len; i++)
//	{
//	  USARTx_printf(USART1," %d",pDat[i]);
//	}
//	USARTx_printf(USART1," \r\n");
	return len;
}


//注网附网成功之后循环发送数据。
void BC95_Test_Demo(void)
{
	unsigned char len=0,n;
	unsigned char tab[16]={"0123456789abcdef"};
	uint8_t WHILE1;
   u32  tick = 0,start_tick=0;
	u32  tick2 = 0,start_tick2=0;
	unsigned char UART_DATA_TIME=0,q;	
	start_tick = GetSysTick();	

	NRB();
	BC95_power_on();
	
  WHILE1=0;
	time_s=0;
	STMFLASH_Read(FLASH_SAVE_ADDR,(u16*)usart1_rcvd_buf,80);
	 if(MAIN_flash1(usart1_rcvd_buf)==1)
	 {
		
	 }
	 STMFLASH_Read(FLASH_SAVE_ADDR+256,(u16*)usart1_rcvd_buf,80);
	 if(MAIN_flash2(usart1_rcvd_buf)==1)
	 {
		 
	 }
	 
	  while(HeartMsg[len]!=0x00)len++;
		 for(n=0;n<len;n++)
		 {
			 q=((HeartMsg[n]>>4)&0x0f);
			HeartMsgDATA[n*2] = tab[q];
			 q=(HeartMsg[n]&0x0f);
			HeartMsgDATA[n*2+1] = tab[q];
		 }
		// len=len*2;
		 if(len<10)
		 {
			HeartMsgDATA_LEN[0]=len%10+0x30; 
		 }
		 else
		 {
			 HeartMsgDATA_LEN[0]=len/10+0x30; 
			 HeartMsgDATA_LEN[1]=len%10+0x30; 
		 }

	if(query_net_status())
	{
		usart2_rcvd_len=0;
	  USARTx_printf(USART2,"AT+CIMI\r\n");//
		while(usart2_rcvd_len<25);
		for(n=0;n<15;n++)
		{
			NB_CIMI_INT[n]=usart2_rcvd_buf[2+n];
		}
		NPING_IP(BC95_IP);
		creat_UDP_socket(BC95_IP_PORT);
		//USARTx_printf(USART2,"5555555555555555555555555555555555555555555555555555555555555\r\n");
		
		MAIN_NB_printf=1;//开启透传
		USARTx_printf(USART1,"reboot;"); 
		WHILE1=1;
		while(1)
		{
			
			if(WHILE1==1)
			{
				
				
				if(time_s>=time_n)
				{
					time_s=0;
				//	sizeof(TEXT_Buffer)		//数组长度HeartMsg
					
					send_UDP_msg("1",BC95_IP,BC95_IP_PORT,HeartMsgDATA_LEN,HeartMsgDATA);
				}
//			tick = GetSysTick();
//			if (tick >= (start_tick + 15000))
//			{	
//				start_tick = tick;
//				send_UDP_msg("1",BC95_IP,BC95_IP_PORT,"3","303132");
//			}	

				if (GPRSRecvFlag == 1)	
				{
					GPRSRecvFlag = 0;
						
					if(BC95_check_ack(RecvSignal))
					{
						
							usart2_rcvd_flag=0;
							usart2_rcvd_len = 0;
							GPRSRecvFlag = 0;						

							USARTx_printf(USART2,"%s\r\n",AT_NSORF);
//						  USARTx_printf(USART1,"%s\r\n",AT_NSORF);
						
						  
					}
					else if (BC95_check_ack(RealSingal))
					{
						RecvGPRSData(usart2_rcvd_buf,GPRSRecvBuffer);
						
							usart2_rcvd_flag=0;
							usart2_rcvd_len = 0;
							GPRSRecvFlag = 0;	

					}
					
					memset(GPRSRecvBuffer, 0, GPRS_BUFFER_SIZE);
				}		
			}	
				
				
				tick2 = GetSysTick();
      if (tick2 >= (start_tick2 + 5))
			{	unsigned char i;
				start_tick2 = tick2;
				if(UART_DATA_TIME==1)
				{
					UART_DATA_TIME=0;
					usart1_rcvd_len=0;
					for(i=0;i<80;i++){usart1_rcvd_buf[i]=0;}
				}
			}	


         if(usart1_rcvd_flag==1)
				 {
					 
					 UART_DATA_TIME=1;
					 usart1_rcvd_flag=0;
					 if(usart1_rcvd_len>=2)
					 {
						  unsigned char i=0;
						 if(usart1_rcvd_buf[0]=='I'
							&&usart1_rcvd_buf[1]=='D' 
						 &&usart1_rcvd_buf[2]==';'
							)
						 {
							
//							 USARTx_printf(USART1,"\r\nID\r\n");//
							 USARTx_printf(USART2,"AT+CIMI\r\n");//
							 for(i=0;i<80;i++){usart1_rcvd_buf[i]=0;}
							 
						 }
					 }
					 
					 if(usart1_rcvd_len>=7)
					 {
						 unsigned char i=0;
						 if(usart1_rcvd_buf[0]=='s'
							&&usart1_rcvd_buf[1]=='e' 
							&&usart1_rcvd_buf[2]=='t'  
							&&usart1_rcvd_buf[3]=='C'  
							&&usart1_rcvd_buf[4]=='o'  
							 &&usart1_rcvd_buf[5]=='n'  
							 &&usart1_rcvd_buf[6]=='f'
							 &&usart1_rcvd_buf[7]=='i'
							 &&usart1_rcvd_buf[8]=='g'
							 &&usart1_rcvd_buf[9]=='u'
							 &&usart1_rcvd_buf[10]=='r'
							 &&usart1_rcvd_buf[11]=='e'
							 &&usart1_rcvd_buf[12]==';'
						)
						 {
							 MAIN_NB_printf=0;//关闭透传
							 NB_MODE_BIT=1;
							 TIM_Cmd(TIM3, DISABLE);  //使能TIMx
							 WHILE1=0;
//							 USARTx_printf(USART1,"\r\nsetConfigure\r\n");
//							 USARTx_printf(USART1,"\r\nok\r\n"); 
							 for(i=0;i<80;i++){usart1_rcvd_buf[i]=0;}
							 usart1_rcvd_len=0;
							 start_tick = tick;
						 }
						 if(usart1_rcvd_buf[0]=='g'
							&&usart1_rcvd_buf[1]=='e' 
							&&usart1_rcvd_buf[2]=='t'  
							&&usart1_rcvd_buf[3]=='C'  
							&&usart1_rcvd_buf[4]=='o'  
						  &&usart1_rcvd_buf[5]=='n'  
						  &&usart1_rcvd_buf[6]=='f'
						 &&usart1_rcvd_buf[7]=='i'
						 &&usart1_rcvd_buf[8]=='g'
						 &&usart1_rcvd_buf[9]=='u'
						 &&usart1_rcvd_buf[10]=='r'
						 &&usart1_rcvd_buf[11]=='e'
						 &&usart1_rcvd_buf[12]==';'
							)
						 {unsigned char q;
//							 USARTx_printf(USART1,"\r\ngetConfigure\r\n");
							 STMFLASH_Read(FLASH_SAVE_ADDR,(u16*)usart1_rcvd_buf,80);
							 if(MAIN_flash1(usart1_rcvd_buf)==1)
							 {
//								 USARTx_printf(USART1,"\r\nokf\r\n");
							 }
							 else
							 {
//								 USARTx_printf(USART1,"\r\nERROR\r\n"); 
							 }
							 STMFLASH_Read(FLASH_SAVE_ADDR+256,(u16*)usart1_rcvd_buf,80);
							 if(MAIN_flash2(usart1_rcvd_buf)==1)
							 {
//								 USARTx_printf(USART1,"\r\nokf\r\n");
							 }
							 else
							 {
//								 USARTx_printf(USART1,"\r\nERROR\r\n"); 
							 }
							 USARTx_printf(USART1,"IP:"); 
							 USARTx_printf(USART1,BC95_IP);
		                     USARTx_printf(USART1,",COM:") ;
							 USARTx_printf(USART1,BC95_IP_PORT);
							 USARTx_printf(USART1,",HeartMsg:");
							 USARTx_printf(USART1,HeartMsg);
							 USARTx_printf(USART1,",HeartTime:");
							 USARTx_printf(USART1,HeartTime);
							 USARTx_printf(USART1,"\n");
								
		 
//							 while(HeartMsg[len]!=0x00)len++;
//							 for(n=0;n<len;n++)
//							 {
//								 q=((HeartMsg[n]>>4)&0x0f);
//								HeartMsgDATA[n*2] = tab[q];
//								 q=(HeartMsg[n]&0x0f);
//								HeartMsgDATA[n*2+1] = tab[q];
//							 }
//							// len=len*2;
//							 if(len<10)
//							 {
//								HeartMsgDATA_LEN[0]=len%10+0x30; 
//							 }
//							 else
//							 {
//								 HeartMsgDATA_LEN[0]=len/10+0x30; 
//								 HeartMsgDATA_LEN[1]=len%10+0x30; 
//							 }
//								 
//								 time_s=time_n-30;
							 
							 
							 
							 
							
							 
							 for(i=0;i<80;i++){usart1_rcvd_buf[i]=0;}
							 usart1_rcvd_len=0;
							 start_tick = tick;
							 
//							 if(NB_MODE_BIT==1)
//							 {
//								 NB_MODE_BIT=0;
//									 NRB();  //复位//
//									 BC95_power_on();
//									 
//									 NPING_IP(BC95_IP);
//									 creat_UDP_socket(BC95_IP_PORT);
//							 }
							  
							 
						 }
						 if(NB_MODE_BIT==1)
						 {
						 
							 if(MAIN_flash1(usart1_rcvd_buf)==1)
							 {
	//							 USARTx_printf(USART1,"\r\nok1\r\n"); 
								 STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)usart1_rcvd_buf,80);
								 for(i=0;i<80;i++){usart1_rcvd_buf[i]=0;}
								 USARTx_printf(USART1,"ok;"); 
								 
										 NRB();  //复位//
										 BC95_power_on();
										 
										 NPING_IP(BC95_IP);
										 
								 TIM_Cmd(TIM3, ENABLE);  //使能TIMxDISABLE
								
								 USARTx_printf(USART1,"reboot;"); 
								 NB_MODE_BIT=0;
								 MAIN_NB_printf=1;//开启透传
									WHILE1=1;
								 creat_UDP_socket(BC95_IP_PORT);
									
								 
							 }
					   
							 if(MAIN_flash2(usart1_rcvd_buf)==1)
							 {
	//							 USARTx_printf(USART1,"\r\nok2\r\n"); 
								 STMFLASH_Write(FLASH_SAVE_ADDR+256,(u16*)usart1_rcvd_buf,80);
								 for(i=0;i<80;i++){usart1_rcvd_buf[i]=0;}
								 USARTx_printf(USART1,"ok;"); 
								 
									NRB();  //复位//
										 BC95_power_on();
										 
										 NPING_IP(BC95_IP);
										 
								 TIM_Cmd(TIM3, ENABLE);  //使能TIMxDISABLE
								
								 USARTx_printf(USART1,"reboot;"); 
								 NB_MODE_BIT=0;
								 MAIN_NB_printf=1;//开启透传
									WHILE1=1;
								 creat_UDP_socket(BC95_IP_PORT);
							 }
							}
//						 if(MAIN_flash(usart1_rcvd_buf)==1)
//						 {
//							 STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)usart1_rcvd_buf,80);
//							 
////							 USARTx_printf(USART1,"IP:"); 
////							 USARTx_printf(USART1,BC95_IP);
////		             USARTx_printf(USART1,",COM:") ;
////							 USARTx_printf(USART1,BC95_IP_PORT);
////								 USARTx_printf(USART1,",HeartMsg:");
////							 USARTx_printf(USART1,HeartMsg);
////								 USARTx_printf(USART1,",HeartTime:");
////							 USARTx_printf(USART1,HeartTime);
////								 USARTx_printf(USART1,"\n");
//							 
//							 
//							 USARTx_printf(USART1,"\r\nok\r\n"); 
//						 }
						 
						if(usart1_rcvd_len>=20)
					 {
						 unsigned char i=0,m=0,n=0;
						 if(usart1_rcvd_buf[0]=='I'
							&&usart1_rcvd_buf[1]=='D' 
							&&usart1_rcvd_buf[2]==':'  
							)
						 {
							i=i+2;
						  m=i+1;
						 
						  while(usart1_rcvd_buf[i]!=','){i++;}
						  for(n=0;n<i-m;n++){NB_CIMI[n]=usart1_rcvd_buf[m+n];}
							i++;
							if(CIMI_Data(NB_CIMI,NB_CIMI_INT,15)==15)
							{
								if(usart1_rcvd_buf[i+0]=='D'
								 &&usart1_rcvd_buf[i+1]=='A' 
								 &&usart1_rcvd_buf[i+2]=='T'  
								 &&usart1_rcvd_buf[i+3]=='A'  
								 &&usart1_rcvd_buf[i+4]==':'  
								)
								{
									i=i+4;
									m=i+1;
								 
									while(usart1_rcvd_buf[i]!=','){i++;}
									for(n=0;n<i-m;n++){NB_DATA[n]=usart1_rcvd_buf[m+n];}
									i++;
								}	
								if(usart1_rcvd_buf[i+0]=='T'
								 &&usart1_rcvd_buf[i+1]=='Y' 
								 &&usart1_rcvd_buf[i+2]=='P'  
								 &&usart1_rcvd_buf[i+3]=='E'  
								 &&usart1_rcvd_buf[i+4]==':'  
								)
								{
									i=i+4;
									m=i+1;
								 
									while(usart1_rcvd_buf[i]!=';'){i++;}
									for(n=0;n<i-m;n++){NB_DATA_LEN[n]=usart1_rcvd_buf[m+n];}
									send_UDP_msg("1",BC95_IP,BC95_IP_PORT,NB_DATA_LEN,NB_DATA);
									
								}
							}
							
							
							
							 for(i=0;i<80;i++){usart1_rcvd_buf[i]=0;}
							 
						 }
					 }
						
						 
						 
					 }
					 
					 
					 
				 }






				
		}
		
	}
}
unsigned char MAIN_flash2(unsigned char *buf)
{
	unsigned char i=0,m=0,n=0;
	//HeartMsg:a1q,HeartTime:92;
			if(buf[0]=='H'
				 &&buf[1]=='e' 
				 &&buf[2]=='a'  
				 &&buf[3]=='r'
				 &&buf[4]=='t'
				 &&buf[5]=='M'
				 &&buf[6]=='s'
				 &&buf[7]=='g'
				 &&buf[8]==':'
				 ) 
				 {
					 i=i+8;
					 m=i+1;
					 
					 while(usart1_rcvd_buf[i]!=','){i++;}
					 for(n=0;n<30;n++){HeartMsg[n]=0;}
					 for(n=0;n<i-m;n++){HeartMsg[n]=buf[m+n];}
					 HeartMsg_LEN=i-m;
//						UART_Put_Data(USART1,HeartMsg,i-m);
//						UART_Put_Data(USART1,"\r\n",strlen("\r\n"));
				 }
				 if(buf[i+1]=='H'
				 &&buf[i+2]=='e' 
				 &&buf[i+3]=='a'  
				 &&buf[i+4]=='r'
				 &&buf[i+5]=='t'
				 &&buf[i+6]=='T'
				 &&buf[i+7]=='i'
				 &&buf[i+8]=='m'
				 &&buf[i+9]=='e'
				 &&buf[i+10]==':'
				 ) 
				 {
					 i=i+10;
					 m=i+1;
					 
					 while(usart1_rcvd_buf[i]!=';'){i++;}
					 for(n=0;n<10;n++){HeartTime[n]=0;}
					 for(n=0;n<i-m;n++){HeartTime[n]=buf[m+n];}
					 HeartTime_LEN=i-m;
//						UART_Put_Data(USART1,HeartTime,i-m);
//						UART_Put_Data(USART1,"\r\n",strlen("\r\n"));
					 time_n=(((HeartTime[0]-0x30)*10)+(HeartTime[1]-0x30));
					 time_n=time_n*7200;
					 return 1;
				 }
			 
	 else return 0;
}

unsigned char MAIN_flash1(unsigned char *buf)
{
	unsigned char i=0,m=0,n=0;
	if(buf[0]=='I'
		 &&buf[1]=='P' 
		 &&buf[2]==':'  
		)
	 {
		 i=3;
		 while(buf[i]!=','){i++;}
		  for(n=0;n<30;n++){BC95_IP[n]=0;}
		 for(n=0;n<i-3;n++){BC95_IP[n]=buf[3+n];}
		 BC95_IP_LEN=i-3;
//		 UART_Put_Data(USART1,BC95_IP,i-3);
//		 UART_Put_Data(USART1,"\r\n",strlen("\r\n"));
		 
			if(buf[i+1]=='C'
				 &&buf[i+2]=='O' 
				 &&buf[i+3]=='M'  
				 &&buf[i+4]==':'
				 ) 
				 {
					 i=i+4;
					 m=i+1;
					 
					 while(buf[i]!=';'){i++;}
					 for(n=0;n<10;n++){BC95_IP_PORT[n]=0;}
					 for(n=0;n<i-m;n++){BC95_IP_PORT[n]=buf[m+n];}
					 BC95_IP_PORT_LEN=i-m;
					 return 1;
				 }
		
			 }
	 else return 0;
}

unsigned char MAIN_flash(unsigned char *buf)
{
	unsigned char i=0,m=0,n=0;
	if(buf[0]=='I'
		 &&buf[1]=='P' 
		 &&buf[2]==':'  
		)
	 {
		 i=3;
		 while(buf[i]!=','){i++;}
		 for(n=0;n<i-3;n++){BC95_IP[n]=buf[3+n];}
		 BC95_IP_LEN=i-3;
//		 UART_Put_Data(USART1,BC95_IP,i-3);
//		 UART_Put_Data(USART1,"\r\n",strlen("\r\n"));
		 
			if(buf[i+1]=='C'
				 &&buf[i+2]=='O' 
				 &&buf[i+3]=='M'  
				 &&buf[i+4]==':'
				 ) 
				 {
					 
					 i=i+4;
					 
					 m=i+1;
					 
					 while(buf[i]!=','){i++;}
					 for(n=0;n<30;n++){BC95_IP_PORT[n]=0;}
					 for(n=0;n<i-m;n++){BC95_IP_PORT[n]=buf[m+n];}
					 BC95_IP_PORT_LEN=i-m;
//						UART_Put_Data(USART1,BC95_IP_PORT,i-m);
//						UART_Put_Data(USART1,"\r\n",strlen("\r\n"));
				 }
			if(buf[i+1]=='H'
				 &&buf[i+2]=='e' 
				 &&buf[i+3]=='a'  
				 &&buf[i+4]=='r'
				 &&buf[i+5]=='t'
				 &&buf[i+6]=='M'
				 &&buf[i+7]=='s'
				 &&buf[i+8]=='g'
				 &&buf[i+9]==':'
				 ) 
				 {
					 i=i+9;
					 m=i+1;
					 
					 while(usart1_rcvd_buf[i]!=','){i++;}
					 for(n=0;n<10;n++){HeartMsg[n]=buf[m+n];}
					 for(n=0;n<i-m;n++){HeartMsg[n]=buf[m+n];}
					 HeartMsg_LEN=i-m;
//						UART_Put_Data(USART1,HeartMsg,i-m);
//						UART_Put_Data(USART1,"\r\n",strlen("\r\n"));
				 }
				 if(buf[i+1]=='H'
				 &&buf[i+2]=='e' 
				 &&buf[i+3]=='a'  
				 &&buf[i+4]=='r'
				 &&buf[i+5]=='t'
				 &&buf[i+6]=='T'
				 &&buf[i+7]=='i'
				 &&buf[i+8]=='m'
				 &&buf[i+9]=='e'
				 &&buf[i+10]==':'
				 ) 
				 {
					 i=i+10;
					 m=i+1;
					 
					 while(usart1_rcvd_buf[i]!=';'){i++;}
					 for(n=0;n<i-m;n++){HeartTime[n]=buf[m+n];}
					 HeartTime_LEN=i-m;
//						UART_Put_Data(USART1,HeartTime,i-m);
//						UART_Put_Data(USART1,"\r\n",strlen("\r\n"));
					 time_n=(((HeartTime[0]-0x30)*10)+(HeartTime[1]-0x30));
					 time_n=time_n*7200;
					 return 1;
				 }
			 }
	 else return 0;
}



void TIM3_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
		{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIMx更新中断标志 
			
			time_s++;
			
		}
}


unsigned char CIMI_Data(unsigned char *put1,unsigned char *put2,unsigned char LEN)
{
	unsigned char i=0;
	  while(LEN--)
		{
			if(put1[i]==put2[i]){i++;}
		}
    
		return i;
		

}


void UART_Put_Data(USART_TypeDef* USARTx,u8 *data, u8 len)
{
	
    while(len--)
		{
			while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);	//等待知道THR变空
			USART_SendData(USARTx,*data++);
		}
		
		

}

