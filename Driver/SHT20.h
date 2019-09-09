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
��  Name  ��void Delay(uint t)
��Function��delay
��  Notes ��
�� Author ��dingshidong
��  Data  ��2012.08.07
================================================================*/
void Delay(uint t);

/*================================================================
��  Name  ��void I2CDelay (uchar t)
��Function��ģ��IIC�õĶ���ʱ
��  Notes ��
�� Author ��dingshidong
��  Data  ��2012.08.07
================================================================*/
void I2CDelay (uchar t);

/*================================================================
��  Name  ��void I2CInit(void)
��Function��I2C��ʼ��������״̬
��  Notes ��
�� Author ��dingshidong
��  Data  ��2012.08.07
================================================================*/
void I2CInit(void);

/*================================================================
��  Name  ��void I2CStart(void)
��Function��I2C��ʼ�ź�
��  Notes ��SCL��SDAͬΪ�ߣ�SDA����ɵ�֮��SCL����ɵ�
�� Author ��dingshidong
��  Data  ��2012.08.07
================================================================*/
void I2CStart(void);

/*================================================================
���� �ơ�void I2CStop(void)
���� �ܡ�I2Cֹͣ�ź�
���� ע��SCL��SDAͬΪ�ͣ�SCL����ɸ�֮��SDA����ɸ�
���� �ߡ�dingshidong
��ʱ �䡿2012��8��7��
================================================================*/
void I2CStop(void);

/*================================================================
��  Name  ��uchar I2C_Write_Byte(uchar WRByte)
��Function��I2Cдһ���ֽ����ݣ�����ACK����NACK
��  Notes ���Ӹߵ��ͣ����η���
�� Author ��dingshidong
��  Data  ��2012.08.07
================================================================*/
uchar I2C_Write_Byte(uchar Write_Byte);

/*================================================================
��  Name  ��uchar I2C_Read_Byte(uchar AckValue)
��Function��I2C��һ���ֽ����ݣ���ڲ������ڿ���Ӧ��״̬��ACK����NACK
��  Notes ���Ӹߵ��ͣ����ν���
�� Author ��dingshidong
��  Data  ��2012.08.07
================================================================*/
uchar I2C_Read_Byte(uchar AckValue);


/*================================================================
��  Name  ��void SoftReset(void)
��Function��SHT20�����λ���������е���
��  Notes ���Ӹߵ��ͣ����ν���
�� Author ��dingshidong
��  Data  ��2012.08.07
================================================================*/
void SoftReset(void);

/*================================================================
��  Name  ��void SET_Resolution(void)
��Function��д�Ĵ��������÷ֱ���
��  Notes ��
�� Author ��dingshidong
��  Data  ��2012.08.07
================================================================*/
void SET_Resolution(void);

/*================================================================
��  Name  ��float ReadSht20(char whatdo)
��Function����ȡ��������
��  Notes ��
�� Author ��dingshidong
��  Data  ��2012.08.07
================================================================*/
float ReadSht20(char whatdo);