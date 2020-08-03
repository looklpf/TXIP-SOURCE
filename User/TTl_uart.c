
#include "stm32f10x.h"	   
#include <stdio.h>
#include "hp_conf.h"
#include "delay.h"
#include "SysCfg.h"
#include "eeprom_i2c.h"
#include "TVC_uart.h"
#include "PT_uart.h"
#include "timer.h"	   
#include "camera_lense.h"
#include "ComCmd_proc.h"
#include "TTL_uart.h"

unsigned volatile char 	TTL_UART_RxBuf[22];
unsigned char 		    TTL_UART_TxBuf[10];
unsigned volatile char 	TTL_UART_ProtoType;
unsigned volatile char 	TTL_UART_RecCnt;
unsigned char 			TTL_UART_Rec_Flag = 0;


unsigned char 		    CAMERA_ZOOM_DATA_INQ_Flag = 0;
unsigned int 		    CAMERA_ZOOM_POSITION_DATA	= 0;




static void RCC_TTL_UART_Configuration(void)
{   
    RCC_APB2PeriphClockCmd(TTL_UART_GPIO_CLK , ENABLE);	
    RCC_APB1PeriphClockCmd(TTL_UART_UART_CLK , ENABLE);    
}


static void GPIO_TTL_UART_Configuration(void)
{    
    GPIO_InitTypeDef TTL_UART_GPIO_InitStructure;
    TTL_UART_GPIO_InitStructure.GPIO_Pin = TTL_UART_RX_PIN;
    TTL_UART_GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(TTL_UART_GPIO, &TTL_UART_GPIO_InitStructure);	    
    
    TTL_UART_GPIO_InitStructure.GPIO_Pin = TTL_UART_TX_PIN;
    TTL_UART_GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    TTL_UART_GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(TTL_UART_GPIO, &TTL_UART_GPIO_InitStructure);
}

static void UART_TTL_UART_Configuration(unsigned short baud)
{
    USART_InitTypeDef TTL_UART_UART_InitStructure;

    TTL_UART_UART_InitStructure.USART_BaudRate = baud;
    TTL_UART_UART_InitStructure.USART_WordLength = USART_WordLength_8b;
    TTL_UART_UART_InitStructure.USART_StopBits = USART_StopBits_1;
    TTL_UART_UART_InitStructure.USART_Parity = USART_Parity_No;
    TTL_UART_UART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    TTL_UART_UART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(TTL_UART, &TTL_UART_UART_InitStructure);
    USART_ClearFlag(TTL_UART,USART_IT_RXNE);
    USART_ITConfig(TTL_UART, USART_IT_RXNE, ENABLE); 
        
    USART_Cmd(TTL_UART, ENABLE);  
}

static void NVIC_TTL_UART_Configuration(void)
{	
    NVIC_InitTypeDef TTL_UART_NVIC_InitStructure;						 
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    TTL_UART_NVIC_InitStructure.NVIC_IRQChannel = TTL_UART_IRQn;
    TTL_UART_NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    TTL_UART_NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    TTL_UART_NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&TTL_UART_NVIC_InitStructure);
}


void TTL_UART_Init(unsigned short baud)
{
    RCC_TTL_UART_Configuration();
    GPIO_TTL_UART_Configuration();
    UART_TTL_UART_Configuration(baud);
    NVIC_TTL_UART_Configuration();	  	
}

void TTL_UART_IRQHandler(void)	
{					
    u8  temp8;	 
    if(USART_GetITStatus(TTL_UART, USART_IT_RXNE) != RESET) 
    {
        USART_ClearITPendingBit(TTL_UART,USART_IT_RXNE); 
        temp8 = USART_ReceiveData(TTL_UART);  
        if(TTL_UART_RecCnt == 0)
        {
                TTL_UART_RxBuf[TTL_UART_RecCnt] = temp8;
        } 
        if((TTL_UART_RxBuf[0]== 0x90)&&(CAMERA_ZOOM_DATA_INQ_Flag==1))
        {		      
            TTL_UART_RxBuf[TTL_UART_RecCnt] = temp8;
            if(TTL_UART_RxBuf[TTL_UART_RecCnt] == 0xff)             
            {
                TTL_UART_RecCnt						= 0;
                TTL_UART_Rec_Flag 				= 1;
                CAMERA_ZOOM_DATA_INQ_Flag	= 0;
                
                CAMERA_ZOOM_POSITION_DATA	= TTL_UART_RxBuf[2];
                CAMERA_ZOOM_POSITION_DATA	= (CAMERA_ZOOM_POSITION_DATA  <<  4)+TTL_UART_RxBuf[3];
                CAMERA_ZOOM_POSITION_DATA	= (CAMERA_ZOOM_POSITION_DATA  <<  4)+TTL_UART_RxBuf[4];
                CAMERA_ZOOM_POSITION_DATA	= (CAMERA_ZOOM_POSITION_DATA  <<  4)+TTL_UART_RxBuf[5];
            }
            else
            {							
                TTL_UART_RecCnt++;
            }
        }
        else 	
        {
            TTL_UART_RecCnt = 0;
        } 
    }
    else
    {
        USART_ReceiveData(TTL_UART); 
    }	
}

int TTL_UART_SendData(void*buf,int len)
{
    int i,wait,count=0;
    unsigned char *data = (unsigned char*)buf;	 
    for(i=0;i<len;i++)
    {
        if(len > 40)
        {
            return 0;
        }
        USART_ClearFlag(TTL_UART,USART_FLAG_TC);
        USART_SendData(TTL_UART,data[count]); 
        for(wait=0;!(USART_GetFlagStatus(TTL_UART, USART_FLAG_TC));wait++)
        {
            delay(1);	  
            if(wait>200)
            {
                return count;
            }           
        }        
        count++;
    }   
    return count;
}

unsigned int 	Get_Camera_Zoom_Position(void)
{
    unsigned int n = 0;
    char i;
    TTL_UART_TxBuf[n++] = 0x81;
    TTL_UART_TxBuf[n++] = 0x09;                
    TTL_UART_TxBuf[n++] = 0x04;
    TTL_UART_TxBuf[n++] = 0x47;
    TTL_UART_TxBuf[n++] = 0xff;
    
    TTL_UART_SendData(TTL_UART_TxBuf,	n);
    CAMERA_ZOOM_DATA_INQ_Flag = 1;
    
    while((TVC_UART_ProtoFlag == NULL)&&(TTL_UART_Rec_Flag==0))
    {
        i++;
        delay_ms(200);
        if(i>10)
        {
            break;
        }
    }
    TTL_UART_Rec_Flag =	0;
    TTL_UART_RecCnt = 0;
    return CAMERA_ZOOM_POSITION_DATA;
}


void Set_Camera_Zoom_Position(void)
{
    unsigned int n = 0;
    
    TTL_UART_TxBuf[n++] = 0x81;
    TTL_UART_TxBuf[n++] = 0x01;                
    TTL_UART_TxBuf[n++] = 0x04;
    TTL_UART_TxBuf[n++] = 0x47;
    TTL_UART_TxBuf[n++] = (CameraStruct.ZoomSetPosition >> 12)&0x0f;
    TTL_UART_TxBuf[n++] = (CameraStruct.ZoomSetPosition >> 8)&0x0f;
    TTL_UART_TxBuf[n++] = (CameraStruct.ZoomSetPosition >> 4)&0x0f;
    TTL_UART_TxBuf[n++] = CameraStruct.ZoomSetPosition&0x0f;
    TTL_UART_TxBuf[n++] = 0xff;
    
    TTL_UART_SendData(TTL_UART_TxBuf,n);
}

void Set_Camera_Black(void)
{
    unsigned int n = 0;
    
    TTL_UART_TxBuf[n++] = 0x81;
    TTL_UART_TxBuf[n++] = 0x01;                
    TTL_UART_TxBuf[n++] = 0x04;
    TTL_UART_TxBuf[n++] = 0x01;
    TTL_UART_TxBuf[n++] = 0x02;
    TTL_UART_TxBuf[n++] = 0xff;
    
    TTL_UART_SendData(TTL_UART_TxBuf,	n);
}

void Set_Camera_Fog_Off(void)
{
    unsigned int n = 0;
    
    TTL_UART_TxBuf[n++] = 0x81;
    TTL_UART_TxBuf[n++] = 0x01;                
    TTL_UART_TxBuf[n++] = 0x09;
    TTL_UART_TxBuf[n++] = 0x31;
    TTL_UART_TxBuf[n++] = 0x00;
    TTL_UART_TxBuf[n++] = 0xff;
    
    TTL_UART_SendData(TTL_UART_TxBuf,	n);
}


void Set_Camera_Fog_ON(void)
{
    unsigned int n = 0;
    
    TTL_UART_TxBuf[n++] = 0x81;
    TTL_UART_TxBuf[n++] = 0x01;                
    TTL_UART_TxBuf[n++] = 0x09;
    TTL_UART_TxBuf[n++] = 0x31;
    TTL_UART_TxBuf[n++] = 0x0a;
    TTL_UART_TxBuf[n++] = 0xff;
    
    TTL_UART_SendData(TTL_UART_TxBuf,	n);
}

void Camera_ZoomWide(void)
{
    unsigned int n = 0;
    
    TTL_UART_TxBuf[n++] = 0x81;
    TTL_UART_TxBuf[n++] = 0x01;                
    TTL_UART_TxBuf[n++] = 0x04;
    TTL_UART_TxBuf[n++] = 0x07;
    TTL_UART_TxBuf[n++] = 0x37;
    TTL_UART_TxBuf[n++] = 0xff;
    
    TTL_UART_SendData(TTL_UART_TxBuf,	n);
}

void Camera_ZoomTele(void)
{
    unsigned int n = 0;
    
    TTL_UART_TxBuf[n++] = 0x81;
    TTL_UART_TxBuf[n++] = 0x01;                
    TTL_UART_TxBuf[n++] = 0x04;
    TTL_UART_TxBuf[n++] = 0x07;
    TTL_UART_TxBuf[n++] = 0x27;
    TTL_UART_TxBuf[n++] = 0xff;
    
    TTL_UART_SendData(TTL_UART_TxBuf,	n);
}

void Camera_Zoom_stop(void)
{
    unsigned int n = 0;
    
    TTL_UART_TxBuf[n++] = 0x81;
    TTL_UART_TxBuf[n++] = 0x01;                
    TTL_UART_TxBuf[n++] = 0x04;
    TTL_UART_TxBuf[n++] = 0x07;
    TTL_UART_TxBuf[n++] = 0x00;
    TTL_UART_TxBuf[n++] = 0xff;
    
    TTL_UART_SendData(TTL_UART_TxBuf,	n);
}

void Camera_FocusFar(void)
{
    unsigned int n = 0;
    
    TTL_UART_TxBuf[n++] = 0x81;
    TTL_UART_TxBuf[n++] = 0x01;                
    TTL_UART_TxBuf[n++] = 0x04;
    TTL_UART_TxBuf[n++] = 0x08;
    TTL_UART_TxBuf[n++] = 0x27;
    TTL_UART_TxBuf[n++] = 0xff;
    
    TTL_UART_SendData(TTL_UART_TxBuf,	n);
}

void Camera_FocusNear(void)
{
    unsigned int n = 0;
    
    TTL_UART_TxBuf[n++] = 0x81;
    TTL_UART_TxBuf[n++] = 0x01;                
    TTL_UART_TxBuf[n++] = 0x04;
    TTL_UART_TxBuf[n++] = 0x08;
    TTL_UART_TxBuf[n++] = 0x37;
    TTL_UART_TxBuf[n++] = 0xff;
    
    TTL_UART_SendData(TTL_UART_TxBuf,	n);
}

void Camera_Focus_stop(void)
{
    unsigned int n = 0;
    
    TTL_UART_TxBuf[n++] = 0x81;
    TTL_UART_TxBuf[n++] = 0x01;                
    TTL_UART_TxBuf[n++] = 0x04;
    TTL_UART_TxBuf[n++] = 0x08;
    TTL_UART_TxBuf[n++] = 0x00;
    TTL_UART_TxBuf[n++] = 0xff;
    
    TTL_UART_SendData(TTL_UART_TxBuf,	n);
}



void Set_Camera_Black_To_Laser_Board(void)
{
    unsigned int n = 0;
    
    TTL_UART_TxBuf[n++] = 0xff;
    TTL_UART_TxBuf[n++] = 0x01;                
    TTL_UART_TxBuf[n++] = 0x00;
    TTL_UART_TxBuf[n++] = 0x09;
    TTL_UART_TxBuf[n++] = 0x00;
    TTL_UART_TxBuf[n++] = 0x01;
    TTL_UART_TxBuf[n++] = 0x0b;
    
    TTL_UART_SendData(TTL_UART_TxBuf,	n);
}

void Set_Camera_Black_To_Laser_Colour(void)
{
    unsigned int n = 0;
    
    TTL_UART_TxBuf[n++] = 0xff;
    TTL_UART_TxBuf[n++] = 0x01;                
    TTL_UART_TxBuf[n++] = 0x00;
    TTL_UART_TxBuf[n++] = 0x0b;
    TTL_UART_TxBuf[n++] = 0x00;
    TTL_UART_TxBuf[n++] = 0x01;
    TTL_UART_TxBuf[n++] = 0x0d;
    
    TTL_UART_SendData(TTL_UART_TxBuf,	n);
}


void CameraColor(void)
{
    unsigned int n = 0;
    
    TTL_UART_TxBuf[n++] = 0x81;
    TTL_UART_TxBuf[n++] = 0x01;                
    TTL_UART_TxBuf[n++] = 0x04;
    TTL_UART_TxBuf[n++] = 0x01;
    TTL_UART_TxBuf[n++] = 0x03;
    TTL_UART_TxBuf[n++] = 0xff;
    
    TTL_UART_SendData(TTL_UART_TxBuf,	n);
}


void CameraBlack(void)
{
    unsigned int n = 0;
    
    TTL_UART_TxBuf[n++] = 0x81;
    TTL_UART_TxBuf[n++] = 0x01;                
    TTL_UART_TxBuf[n++] = 0x04;
    TTL_UART_TxBuf[n++] = 0x01;
    TTL_UART_TxBuf[n++] = 0x02;
    TTL_UART_TxBuf[n++] = 0xff;
    
    TTL_UART_SendData(TTL_UART_TxBuf,	n);
}
void CamLocAd(int ad)
{
    unsigned int n = 0;
    
    TTL_UART_TxBuf[n++] = 0x81;
    TTL_UART_TxBuf[n++] = 0x01;                
    TTL_UART_TxBuf[n++] = 0x04;
    TTL_UART_TxBuf[n++] = 0x47;
    TTL_UART_TxBuf[n++] = (ad >> 12)&0x0f;
    TTL_UART_TxBuf[n++] = (ad >> 8)&0x0f;
    TTL_UART_TxBuf[n++] = (ad >> 4)&0x0f;
    TTL_UART_TxBuf[n++] = ad&0x0f;
    TTL_UART_TxBuf[n++] = 0xff;
    
    TTL_UART_SendData(TTL_UART_TxBuf,n);
}
