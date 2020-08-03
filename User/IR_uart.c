#include "stm32f10x.h"
#include "hp_conf.h"
#include "IR_uart.h"
#include "SysCfg.h"
#include "delay.h"
#include "ComCmd_proc.h"
#include "protocol_type.h"
#include "eeprom_i2c.h"
#include "TVC_uart.h"
#include "LensDriver.h"



unsigned char  ZF_Inq_flag=0;
unsigned int IR_Lense_Angle_Data;

unsigned char 			    IR_UART_RecCnt = 0;
volatile unsigned  char 	IR_UART_RxBuf[200];
unsigned char 			    IR_UART_TxBuf[200];
volatile unsigned  char 	IR_UART_ProtoType;
volatile unsigned char      data_covert_flag = 0;
volatile unsigned  short    re_zoom;
volatile unsigned  short    re_focus;



static void RCC_IR_UART_Configuration(void)
{   
    RCC_APB2PeriphClockCmd(IR_UART_GPIO_CLK , ENABLE);	
    RCC_APB1PeriphClockCmd(IR_UART_UART_CLK , ENABLE);    
}
static void GPIO_IR_UART_Configuration(void)
{    
    GPIO_InitTypeDef    IR_UART_GPIO_InitStructure;
    IR_UART_GPIO_InitStructure.GPIO_Pin 	= IR_UART_RX_PIN;
    IR_UART_GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &IR_UART_GPIO_InitStructure);	    
    
    IR_UART_GPIO_InitStructure.GPIO_Pin 	= IR_UART_TX_PIN;
    IR_UART_GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    IR_UART_GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &IR_UART_GPIO_InitStructure);
}

static void UART_IR_UART_Configuration(unsigned int baud)
{
    USART_InitTypeDef IR_UART_UART_InitStructure;

    IR_UART_UART_InitStructure.USART_BaudRate = baud;
    IR_UART_UART_InitStructure.USART_WordLength = USART_WordLength_8b;
    IR_UART_UART_InitStructure.USART_StopBits = USART_StopBits_1;
    IR_UART_UART_InitStructure.USART_Parity = USART_Parity_No;
    IR_UART_UART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    IR_UART_UART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(IR_UART, &IR_UART_UART_InitStructure);
    USART_ClearFlag(IR_UART, USART_IT_RXNE);
    USART_ITConfig(IR_UART ,USART_IT_RXNE, ENABLE); 
        
    USART_Cmd(IR_UART, ENABLE);  
}

static void NVIC_IR_UART_Configuration(void)
{	
    NVIC_InitTypeDef IR_UART_NVIC_InitStructure;						 
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    
    IR_UART_NVIC_InitStructure.NVIC_IRQChannel = IR_UART_IRQn;
    IR_UART_NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    IR_UART_NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    IR_UART_NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&IR_UART_NVIC_InitStructure);
}


void IR_UART_Init(unsigned  int  baud)
{
    RCC_IR_UART_Configuration();
    GPIO_IR_UART_Configuration();
    UART_IR_UART_Configuration(baud);
    NVIC_IR_UART_Configuration();	  	
}


int IR_UART_SendData(void*buf,int len)
{
    int i,wait,count=0 ;
    unsigned char*data=(unsigned char*)buf ;
    for(i=0; i<len; i++)
    {
        USART_ClearFlag(IR_UART,USART_FLAG_TC);
        USART_SendData(IR_UART,data[count]);
        for(wait=0; !(USART_GetFlagStatus(IR_UART,USART_FLAG_TC)); wait++)
        {
            delay(10);
            if(wait>200)
            {
                return count ;
            }
        }
        count++;
    }
    return count ;
}
//void Set_IR_FocusSpeed(unsigned char speed)
//{
//    TIM_SetCompare3(TIM4, speed);
//}


void IR_UART_IRQHandler(void)
{
    static unsigned char rec_cnt=0;
    if(USART_GetITStatus(IR_UART, USART_IT_RXNE) != RESET)
    {
        USART_ClearITPendingBit(IR_UART,USART_IT_RXNE);
        IR_UART_RxBuf[rec_cnt] = USART_ReceiveData(IR_UART);
        if(IR_UART_RxBuf[0] == 0xF0)
        {
            if (IR_UART_RxBuf[rec_cnt] == 0xFF) 
            {             
                if(IR_UART_RxBuf[3] == 0xC9)
                {
                    ZF_Inq_flag = 0;
                    re_zoom  = IR_UART_RxBuf[5]<< 8 | IR_UART_RxBuf[4] ;
                    re_focus = IR_UART_RxBuf[7]<< 8 | IR_UART_RxBuf[6] ;
                }	
                else if(IR_UART_RxBuf[3] == 0xCC)
                {
                    if(IR_UART_RxBuf[4] == 0x01)
                    {
                        IR_Lense_Angle_Data = ((IR_UART_RxBuf[6] << 8) | IR_UART_RxBuf[5]);
                    }
                }		
                IR_UART_RecCnt=rec_cnt;	
                rec_cnt			= 0;			
                data_covert_flag = 0;
            }
            else if (IR_UART_RxBuf[rec_cnt] == 0xF5)
            {
                data_covert_flag = 1;
                rec_cnt++;
            }
            else if (data_covert_flag == 1)
            {
                IR_UART_RxBuf[rec_cnt - 1] = IR_UART_RxBuf[rec_cnt] | 0xf0;
                data_covert_flag = 0;
            }
            else
            {
                rec_cnt++;
            }
                
            if (rec_cnt > 40 - 3)
            {
                rec_cnt = 0;
            }
        }
        else
        { 
            rec_cnt = 0;    
        }
    }
    else
    {
        USART_ReceiveData(IR_UART);
    }
}



void IRLense_Ctrl(CameraStruct_TypeDef *IR)
{
    static char flag = 0;
    if(IR->ZoomWide_Flag)
    {
        LENS->IR->Wide();
        flag = 1;
    }
    else if(IR->ZoomTele_Flag)
    {
        LENS->IR->Tele();
        flag = 1; 
    }
    else if(IR->FocusFar_Flag)
    {
        LENS->IR->Far();
        flag = 2;
    }
    else if(IR->FocusNear_Flag)
    {
        LENS->IR->Near();
        flag = 2;
    }
    else if(IR->IrisClose_Flag || IR->IrisOpen_Flag)
    {
    }
    else
    {
        if(flag ==1)
        {
            LENS->IR->ZoomStop();
        }
        if(flag ==2)
        {
            LENS->IR->FocusStop(); 
        }
        flag =0;
        
    }

}

void command_zoomwide(void)
{
	 IR_UART_TxBuf[0]=0xF0;
	 IR_UART_TxBuf[1]=0x03;
	 IR_UART_TxBuf[2]=0x26;
	 IR_UART_TxBuf[3]=0x11;
	 IR_UART_TxBuf[4]=0x0f;
	 IR_UART_TxBuf[5]=0x46;
	 IR_UART_TxBuf[6]=0xFF;
	 IR_UART_SendData(IR_UART_TxBuf,7);
}
/********************************************************************************************************
**函数名称ommand_zoomtele(void)
**函数功能：
********************************************************************************************************/
void command_zoomtele(void)
{
	IR_UART_TxBuf[0]=0xF0;
	IR_UART_TxBuf[1]=0x03;
	IR_UART_TxBuf[2]=0x26;
	IR_UART_TxBuf[3]=0x11;
	IR_UART_TxBuf[4]=0x00;
	IR_UART_TxBuf[5]=0x37;
	IR_UART_TxBuf[6]=0xFF;
	IR_UART_SendData(IR_UART_TxBuf,7);
}
/********************************************************************************************************
**函数名称：command_zoomstop(void)
**函数功能：
********************************************************************************************************/
void command_zoomstop(void)
{
	IR_UART_TxBuf[0]=0xF0;
	IR_UART_TxBuf[1]=0x02;
	IR_UART_TxBuf[2]=0x26;
	IR_UART_TxBuf[3]=0x10;
	IR_UART_TxBuf[4]=0x36;
	IR_UART_TxBuf[5]=0xFF;
	IR_UART_SendData(IR_UART_TxBuf,6);
}
/********************************************************************************************************
**函数名称：void command_FocusNear()
**函数功能：聚焦近命令
********************************************************************************************************/
void command_FocusNear(void)
{
    IR_UART_TxBuf[0]	=	0xF0;
    IR_UART_TxBuf[1]	=	0x03;
    IR_UART_TxBuf[2]	=	0x26;
    IR_UART_TxBuf[3]	=	0x01;
    IR_UART_TxBuf[4]	=	0x00;
    IR_UART_TxBuf[5]	=	IR_UART_TxBuf[2]+IR_UART_TxBuf[3]+IR_UART_TxBuf[4];
    IR_UART_TxBuf[6]	=	0xFF;
    IR_UART_SendData(IR_UART_TxBuf,7);
}
/********************************************************************************************************
**函数名称：void command_FocusFar()
**函数功能：聚焦远命令
********************************************************************************************************/
void command_FocusFar(void)
{
    IR_UART_TxBuf[0]	=	0xF0;
    IR_UART_TxBuf[1]	=	0x03;
    IR_UART_TxBuf[2]	=	0x26;
    IR_UART_TxBuf[3]	=	0x01;
    IR_UART_TxBuf[4]	=	0x0F;
    IR_UART_TxBuf[5]	=	IR_UART_TxBuf[2]+IR_UART_TxBuf[3]+IR_UART_TxBuf[4];
    IR_UART_TxBuf[6]	=	0xFF;
    IR_UART_SendData(IR_UART_TxBuf,7);
}
/********************************************************************************************************
**函数名称：void command_FocusFar()
**函数功能：聚焦远命令
********************************************************************************************************/
void command_FocusStop(void)
{
    IR_UART_TxBuf[0]	=	0xF0;
    IR_UART_TxBuf[1]	=	0x03;
    IR_UART_TxBuf[2]	=	0x26;
    IR_UART_TxBuf[3]	=	0x10;
    IR_UART_TxBuf[4]	=	0x00;
    IR_UART_TxBuf[5]	=	IR_UART_TxBuf[2]+IR_UART_TxBuf[3]+IR_UART_TxBuf[4];
    IR_UART_TxBuf[6]	=	0xFF;
    IR_UART_SendData(IR_UART_TxBuf,7);
}
/********************************************************************************************************
**函数名称：void command_autofocus()
**函数功能：启动自动聚焦
********************************************************************************************************/
void command_autofocus(void)
{
    IR_UART_TxBuf[0]	=	0xF0;
    IR_UART_TxBuf[1]	=	0x02;
    IR_UART_TxBuf[2]	=	0x26;
    IR_UART_TxBuf[3]	=	0x34;
    IR_UART_TxBuf[4]	=	0x5A;
    IR_UART_TxBuf[5]	=	0xFF;
    IR_UART_SendData(IR_UART_TxBuf,6);
}

void command_Inq_ZFPos(void)
{
    unsigned char buf[7] = {0xff,0x01,0x77,0x77};
    ZF_Inq_flag = 1;
    char i = 0;
    IR_UART_TxBuf[0]=0xF0;
    IR_UART_TxBuf[1]=0x02;
    IR_UART_TxBuf[2]=0x26;
    IR_UART_TxBuf[3]=0xC9;
    IR_UART_TxBuf[4]=0xEF;
    IR_UART_TxBuf[5]=0xFF;
    while((ZF_Inq_flag == 1)&&(i < 20))
    {
        IR_UART_SendData(IR_UART_TxBuf,6);	
        delay_ms(200);
        i++;
    }
    ZF_Inq_flag = 0;
    buf[4] = i;
    TVC_UART_SendData(buf, 7);

}
unsigned short get_IR_ZoomPos(void)
{
//	while(ZF_Inq_flag);
    char i = 0;
	while((ZF_Inq_flag)&&(i<10))
    {
       i++;
       delay_ms(20);
       
    }
	return re_zoom;
}
unsigned short get_IR_FocusPos(void)
{
	return re_focus;
}
void setIRZOOM_FOCUS(unsigned short zoom, unsigned short focus)
{
    int i = 0;
    unsigned char ZOOM_tmpH, ZOOM_tmpL,FOCUS_tmpH,FOCUS_tmpL,crc;

    delay_ms(50);	//查询和定位间隔20ms
	
    IR_UART_TxBuf[i++]=0xF0;
    IR_UART_TxBuf[i++]=0x06;
    IR_UART_TxBuf[i++]=0x26;
    IR_UART_TxBuf[i++]=0xCA;

    //zoom
    ZOOM_tmpL = zoom & 0xff;
    ZOOM_tmpH = (zoom >> 8) & 0xff;
    if ((ZOOM_tmpL == 0xf0) || (ZOOM_tmpL == 0xf5) || (ZOOM_tmpL == 0xff))  //转义字符
    {
        IR_UART_TxBuf[i++] = 0xf5;
        IR_UART_TxBuf[i++] = ZOOM_tmpL & 0x0f;
    }
    else
    {
        IR_UART_TxBuf[i++] = ZOOM_tmpL;
    }
    if ((ZOOM_tmpH == 0xf0) || (ZOOM_tmpH == 0xf5) || (ZOOM_tmpH == 0xff))  //转义字符
    {
        IR_UART_TxBuf[i++] = 0xf5;
        IR_UART_TxBuf[i++] = ZOOM_tmpH & 0x0f;
    }
    else
    {
        IR_UART_TxBuf[i++] = ZOOM_tmpH;
    }
	  //focus
	  FOCUS_tmpL = focus & 0xff;
    FOCUS_tmpH = (focus >> 8) & 0xff;
	  if ((FOCUS_tmpL == 0xf0) || (FOCUS_tmpL == 0xf5) || (FOCUS_tmpL == 0xff))  //转义字符
    {
        IR_UART_TxBuf[i++] = 0xf5;
        IR_UART_TxBuf[i++] = FOCUS_tmpL & 0x0f;
    }
    else
    {
        IR_UART_TxBuf[i++] = FOCUS_tmpL;
    }
    if ((FOCUS_tmpH == 0xf0) || (FOCUS_tmpH == 0xf5) || (FOCUS_tmpH == 0xff))  //转义字符
    {
        IR_UART_TxBuf[i++] = 0xf5;
        IR_UART_TxBuf[i++] = FOCUS_tmpH & 0x0f;
    }
    else
    {
        IR_UART_TxBuf[i++] = FOCUS_tmpH;
    }
	  //crc
    crc=(IR_UART_TxBuf[2]+IR_UART_TxBuf[3]+ZOOM_tmpL+ZOOM_tmpH+FOCUS_tmpL+FOCUS_tmpH) & 0xFF;
    if ((crc == 0xf0) || (crc == 0xf5) || (crc == 0xff))
    {
        IR_UART_TxBuf[i++] = 0xf5;
        IR_UART_TxBuf[i++] = crc & 0x0f;
    }
    else
    {
        IR_UART_TxBuf[i++] = crc;
    }
    IR_UART_TxBuf[i++]=0xFF;
    IR_UART_SendData(IR_UART_TxBuf, i);
}
void command_Inq_FOV(void)
{
  //  F0 03 26 CC 01 F3 FF
    IR_UART_TxBuf[0]=0xF0;
    IR_UART_TxBuf[1]=0x03;
    IR_UART_TxBuf[2]=0x26;
    IR_UART_TxBuf[3]=0xCC;
    IR_UART_TxBuf[4]=0x01;
    IR_UART_TxBuf[5]=0xF3;
    IR_UART_TxBuf[6]=0xFF;
    IR_UART_SendData(IR_UART_TxBuf,7);	
}


void command_Set_FOV(unsigned short FOVValue)
{
	int n=0;
	unsigned char data[2];
	data[0]=FOVValue;
	data[1]=FOVValue>>8;
	IR_UART_TxBuf[n++] = 0xF0;
	IR_UART_TxBuf[n++] = 0;
	IR_UART_TxBuf[n++] = 0x26;
	IR_UART_TxBuf[n++] = 0xCB;
	IR_UART_TxBuf[n++] = 0x03;
	IR_UART_TxBuf[n++] = data[0];
	IR_UART_TxBuf[n++] = data[1];
	IR_UART_TxBuf[1]   = n-2;
	int nCheckSum = 0;
	for (int i = 2; i < n; ++i)
	{			 
		nCheckSum += IR_UART_TxBuf[i];
	}
	n=5;
	for(int i=0;i<2;++i)
	{
		if(data[i]==0xF0)
		{
			IR_UART_TxBuf[n++] = 0xF5;
			IR_UART_TxBuf[n++] = 0x00;		
		}
		else if(data[i]==0xF5)
		{
			IR_UART_TxBuf[n++] = 0xF5;
			IR_UART_TxBuf[n++] = 0x05;
		}
		else if(data[i]==0xFF)
		{
			IR_UART_TxBuf[n++] = 0xF5;
			IR_UART_TxBuf[n++] = 0x0F;
		}
		else
		{
			IR_UART_TxBuf[n++] = data[i];
		}
	}

	unsigned char temp8=(unsigned char)nCheckSum;
	if(temp8==0xF0)
	{
		IR_UART_TxBuf[n++] = 0xF5;
		IR_UART_TxBuf[n++] = 0x00;		
	}
	else if(temp8==0xF5)
	{
		IR_UART_TxBuf[n++] = 0xF5;
		IR_UART_TxBuf[n++] = 0x05;
	}
	else if(temp8==0xFF)
	{
		IR_UART_TxBuf[n++] = 0xF5;
		IR_UART_TxBuf[n++] = 0x0F;
	}
	else
	{
		IR_UART_TxBuf[n++]=temp8;
	}
	IR_UART_TxBuf[n++] = 0xff;
    IR_UART_SendData(IR_UART_TxBuf,n);		
}
void IrOpenLocViewAutoFocus(void)
{
    unsigned char buf[8] = {0xf0,0x04,0x26,0xc2,0x79,0x03,0x64,0xff};
    IR_UART_SendData(buf,8);	
}
void IrCloseLocViewAutoFocus(void)
{
    unsigned char buf[8] = {0xf0,0x04,0x26,0xc2,0x79,0x02,0x63,0xff};
    IR_UART_SendData(buf,8);	
}
void IrOpenZoomAutoFocus(void)
{
    unsigned char buf[8] = {0xf0,0x04,0x26,0xc2,0x79,0x01,0x62,0xff};
    IR_UART_SendData(buf,8);	
}
void IrCloseZoomAutoFocus(void)
{
    unsigned char buf[8] = {0xf0,0x04,0x26,0xc2,0x79,0x00,0x61,0xff};
    IR_UART_SendData(buf,8);	
}

void IrLocFocus(unsigned int val)
{
    int n=0;
	unsigned char data[2];
	data[0]=val&0xff;
	data[1]=(val>>8)&0xff;
	IR_UART_TxBuf[n++] = 0xF0;
	IR_UART_TxBuf[n++] = 6;
	IR_UART_TxBuf[n++] = 0x26;
	IR_UART_TxBuf[n++] = 0xCA;
    IR_UART_TxBuf[n++] = 0x00;
    IR_UART_TxBuf[n++] = 0x00;
	IR_UART_TxBuf[n++] = data[0];
	IR_UART_TxBuf[n++] = data[1];
	IR_UART_TxBuf[1]   = n-2;
	int nCheckSum = 0;
	for (int i = 2; i < n; ++i)
	{			 
		nCheckSum += IR_UART_TxBuf[i];
	}
	n=6;
	for(int i=0;i<2;++i)
	{
		if(data[i]==0xF0)
		{
			IR_UART_TxBuf[n++] = 0xF5;
			IR_UART_TxBuf[n++] = 0x00;		
		}
		else if(data[i]==0xF5)
		{
			IR_UART_TxBuf[n++] = 0xF5;
			IR_UART_TxBuf[n++] = 0x05;
		}
		else if(data[i]==0xFF)
		{
			IR_UART_TxBuf[n++] = 0xF5;
			IR_UART_TxBuf[n++] = 0x0F;
		}
		else
		{
			IR_UART_TxBuf[n++] = data[i];
		}
	}

	unsigned char temp8=(unsigned char)nCheckSum;
	if(temp8==0xF0)
	{
		IR_UART_TxBuf[n++] = 0xF5;
		IR_UART_TxBuf[n++] = 0x00;		
	}
	else if(temp8==0xF5)
	{
		IR_UART_TxBuf[n++] = 0xF5;
		IR_UART_TxBuf[n++] = 0x05;
	}
	else if(temp8==0xFF)
	{
		IR_UART_TxBuf[n++] = 0xF5;
		IR_UART_TxBuf[n++] = 0x0F;
	}
	else
	{
		IR_UART_TxBuf[n++]=temp8;
	}
	IR_UART_TxBuf[n++] = 0xff;
    IR_UART_SendData(IR_UART_TxBuf,n);	
}

