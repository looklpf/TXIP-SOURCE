
#include <stdio.h>
#include "VC_uart.h"
#include "SysCfg.h"
#include "delay.h"
#include "protocol_type.h"
#include "ComCmd_proc.h" 
#include "msgqueue.h"
#include "eeprom_i2c.h"
char camAutoFocusFlag = 0;

static void RCC_VC_UART_Configuration(void)
{   
    RCC_APB2PeriphClockCmd(VC_UART_GPIO_CLK , ENABLE);	
    RCC_APB1PeriphClockCmd(VC_UART_UART_CLK, ENABLE);    
}


static void GPIO_VC_UART_Configuration(void)
{    
    GPIO_InitTypeDef VC_UART_GPIO_InitStructure;
    VC_UART_GPIO_InitStructure.GPIO_Pin 	= VC_UART_RX_PIN;
    VC_UART_GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IN_FLOATING;
    GPIO_Init(VC_UART_GPIO, &VC_UART_GPIO_InitStructure);	    
    
    VC_UART_GPIO_InitStructure.GPIO_Pin 	= VC_UART_TX_PIN;
    VC_UART_GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    VC_UART_GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF_PP;
    GPIO_Init(VC_UART_GPIO, &VC_UART_GPIO_InitStructure);
}

static void UART_VC_UART_Configuration(unsigned short baud)
{
    USART_InitTypeDef VC_UART_UART_InitStructure;
	VC_UART_UART_InitStructure.USART_BaudRate = baud;
	VC_UART_UART_InitStructure.USART_WordLength = USART_WordLength_8b;
	VC_UART_UART_InitStructure.USART_StopBits = USART_StopBits_1;
	VC_UART_UART_InitStructure.USART_Parity = USART_Parity_No;
	VC_UART_UART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	VC_UART_UART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(VC_UART, &VC_UART_UART_InitStructure);
	USART_ClearFlag(VC_UART,USART_IT_RXNE);
	USART_ITConfig(VC_UART, USART_IT_RXNE, ENABLE); 
	USART_Cmd(VC_UART, ENABLE);  
}

static void NVIC_VC_UART_Configuration(void)
{	
    NVIC_InitTypeDef VC_UART_NVIC_InitStructure;						  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    VC_UART_NVIC_InitStructure.NVIC_IRQChannel = VC_UART_IRQn;
    VC_UART_NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    VC_UART_NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    VC_UART_NVIC_InitStructure.NVIC_IRQChannelCmd  = ENABLE;
    NVIC_Init(&VC_UART_NVIC_InitStructure);
}


void VC_UART_Init(unsigned short baud)
{
    RCC_VC_UART_Configuration();
    GPIO_VC_UART_Configuration();
    UART_VC_UART_Configuration(baud);
    NVIC_VC_UART_Configuration();	  	
}

void VC_UART_IRQHandler(void)	
{					
    u8  temp8;	 
    if(USART_GetITStatus(VC_UART, USART_IT_RXNE) != RESET) 
    {
        USART_ClearITPendingBit(VC_UART,USART_IT_RXNE); 
        temp8 = USART_ReceiveData(VC_UART);  
        if(TVC_UART_RecCnt == 0)
        {
            TVC_UART_RxBuf[TVC_UART_RecCnt] = temp8;
        } 
        if(TVC_UART_RxBuf[0]== PELCOD_STX)
        {		      
            TVC_UART_RxBuf[TVC_UART_RecCnt] = temp8;
            TVC_UART_RecCnt++;
            if(TVC_UART_RecCnt == PELCOD_LEN)             
            {
                TVC_UART_RecCnt     =   0;   
                Join_Queque(1, PELCOD_LEN, (uint8_t *)TVC_UART_RxBuf);		
            }
        } 
        else 	
        {
            TVC_UART_RecCnt = 0;
        }
    } 
    else
    {
        USART_ReceiveData(VC_UART); 
    }	
}

void CamReadAutoFocus()
{
    unsigned char buf[1];
    EEPROM_I2C_ReadBuffer(CAM_AUTO_FOCUS_ADDR, buf, 1);
    camAutoFocusFlag = buf[0];
    if((camAutoFocusFlag != 0)&&(camAutoFocusFlag != 1))
    {
        camAutoFocusFlag = 1;
        buf[0] = camAutoFocusFlag;
        EEPROM_I2C_WriteBuffer(CAM_AUTO_FOCUS_ADDR,buf,1);
    }
}

void CamAutoFocusClose()
{
    unsigned char buf[7];
    buf[0] = 0xff;
    buf[1] = 0x01;
    buf[2] = 0x00;
    buf[3] = 0x07;
    buf[4] = 0x00;
    buf[5] = 0x79;
    buf[6] = 0x81;
    VC_UART_SendData(buf, 7);
    camAutoFocusFlag = 0;
    buf[0] = camAutoFocusFlag;
    EEPROM_I2C_WriteBuffer(CAM_AUTO_FOCUS_ADDR,buf,1);
 
}

void CamAutoFocusOpen()
{
    unsigned char buf[7];
    buf[0] = 0xff;
    buf[1] = 0x01;
    buf[2] = 0x00;
    buf[3] = 0x03;
    buf[4] = 0x00;
    buf[5] = 0x79;
    buf[6] = 0x7d;
    VC_UART_SendData(buf, 7);
    camAutoFocusFlag = 1;
    buf[0] = camAutoFocusFlag;
    EEPROM_I2C_WriteBuffer(CAM_AUTO_FOCUS_ADDR,buf,1);
}
void CamAutoFocusCloseNoSave()
{
    unsigned char buf[7];
    buf[0] = 0xff;
    buf[1] = 0x01;
    buf[2] = 0x00;
    buf[3] = 0x07;
    buf[4] = 0x00;
    buf[5] = 0x79;
    buf[6] = 0x81;
    VC_UART_SendData(buf, 7);
}

void CamAutoFocusOpenNoSave()
{
    unsigned char buf[7];
    buf[0] = 0xff;
    buf[1] = 0x01;
    buf[2] = 0x00;
    buf[3] = 0x03;
    buf[4] = 0x00;
    buf[5] = 0x79;
    buf[6] = 0x7d;
    VC_UART_SendData(buf, 7);
}
void CamAutoFocusOnce()
{
    unsigned char buf[7];
    buf[0] = 0xff;
    buf[1] = 0x01;
    buf[2] = 0x00;
    buf[3] = 0x05;
    buf[4] = 0x00;
    buf[5] = 0x79;
    buf[6] = 0x7f;
    VC_UART_SendData(buf, 7);
}

int VC_UART_SendData(void*buf,int len)
{
    int i,wait,count=0;
    unsigned char *data = (unsigned char*)buf;	 
    for(i=0;i<len;i++)
    {
        if(TVC_UART_ProtoFlag && (len > 40))
        {
            return 0;
        }
        USART_ClearFlag(VC_UART,USART_FLAG_TC);
        USART_SendData(VC_UART,data[count]); 
        for(wait=0;!(USART_GetFlagStatus(VC_UART, USART_FLAG_TC));wait++)
        {
            delay(10);	         
            if(wait>200)
            {
                return count;
            }           
        }                      
        count++;
    }   
    return count;
}
