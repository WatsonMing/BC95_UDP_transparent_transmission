/***************************************************************
*   File name    :  SHT20.h  
*   Description  :  SHT20 Humidity and Temperature Sensors
*     M C U      :  STC89C52RC
*   Compiler     :  Keil uVision V4.00a (C51)
*   Created by   :  dingshidong
*   Copyright    :  Copyright(c) 2012 Wuxi CASILINK Tech.CO.,Ltd   
*   Created data :  2012.08.07  
*   Modified data:        2012.08.10
*   Vision       :  V1.0
*****************************************************************/

/*****************Function Declaration*************************/
/*----define to easier to use-----*/
        #define uchar unsigned char         
        #define uint  unsigned int
        #define ulong unsigned long

/*================================================================
【  Name  】void Delay(uint t)
【Function】delay
【  Notes 】
【 Author 】dingshidong
【  Data  】2012.08.07
================================================================*/
void Delay(uint t);

/*================================================================
【  Name  】void I2CDelay (uchar t)
【Function】模拟IIC用的短延时
【  Notes 】
【 Author 】dingshidong
【  Data  】2012.08.07
================================================================*/
void I2CDelay (uchar t);

/*================================================================
【  Name  】void I2CInit(void)
【Function】I2C初始化，空闲状态
【  Notes 】
【 Author 】dingshidong
【  Data  】2012.08.07
================================================================*/
void I2CInit(void);

/*================================================================
【  Name  】void I2CStart(void)
【Function】I2C起始信号
【  Notes 】SCL、SDA同为高，SDA跳变成低之后，SCL跳变成低
【 Author 】dingshidong
【  Data  】2012.08.07
================================================================*/
void I2CStart(void);

/*================================================================
【名 称】void I2CStop(void)
【功 能】I2C停止信号
【备 注】SCL、SDA同为低，SCL跳变成高之后，SDA跳变成高
【作 者】dingshidong
【时 间】2012年8月7日
================================================================*/
void I2CStop(void);

/*================================================================
【  Name  】uchar I2C_Write_Byte(uchar WRByte)
【Function】I2C写一个字节数据，返回ACK或者NACK
【  Notes 】从高到低，依次发送
【 Author 】dingshidong
【  Data  】2012.08.07
================================================================*/
uchar I2C_Write_Byte(uchar Write_Byte);

/*================================================================
【  Name  】uchar I2C_Read_Byte(uchar AckValue)
【Function】I2C读一个字节数据，入口参数用于控制应答状态，ACK或者NACK
【  Notes 】从高到低，依次接收
【 Author 】dingshidong
【  Data  】2012.08.07
================================================================*/
uchar I2C_Read_Byte(uchar AckValue);


/*================================================================
【  Name  】void SoftReset(void)
【Function】SHT20软件复位，主函数中调用
【  Notes 】从高到低，依次接收
【 Author 】dingshidong
【  Data  】2012.08.07
================================================================*/
void SoftReset(void);

/*================================================================
【  Name  】void SET_Resolution(void)
【Function】写寄存器，设置分辨率
【  Notes 】
【 Author 】dingshidong
【  Data  】2012.08.07
================================================================*/
void SET_Resolution(void);

/*================================================================
【  Name  】float ReadSht20(char whatdo)
【Function】读取函数函数
【  Notes 】
【 Author 】dingshidong
【  Data  】2012.08.07
================================================================*/
float ReadSht20(char whatdo);