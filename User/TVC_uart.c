
#include <stdio.h>
#include "TVC_uart.h"
#include "SysCfg.h"
#include "delay.h"
#include "protocol_type.h"
#include "ComCmd_proc.h" 
#include "msgqueue.h"

volatile unsigned  char 	TVC_UART_RxBuf[BUF_LEN];
unsigned char 			    TVC_UART_TxBuf[BUF_LEN];
volatile unsigned  char 	TVC_UART_ProtoFlag = NULL;
volatile unsigned  char 	TVC_UART_RecCnt;


void RCC_TVC_UART_Configuration(void)
{   
    RCC_APB2PeriphClockCmd(TVC_UART_GPIO_CLK , ENABLE);	
    RCC_APB2PeriphClockCmd(TVC_UART_UART_CLK, ENABLE);    
}

void GPIO_TVC_UART_Configuration(void)
{    
    GPIO_InitTypeDef TVC_UART_GPIO_InitStructure;

    TVC_UART_GPIO_InitStructure.GPIO_Pin = TVC_UART_RX_PIN;
    TVC_UART_GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(TVC_UART_GPIO, &TVC_UART_GPIO_InitStructure);	    
    

    TVC_UART_GPIO_InitStructure.GPIO_Pin = TVC_UART_TX_PIN;
    TVC_UART_GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    TVC_UART_GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(TVC_UART_GPIO, &TVC_UART_GPIO_InitStructure);
}

void UART_TVC_UART_Configuration(unsigned short baud)
{
    USART_InitTypeDef TVC_UART_UART_InitStructure;
    
    TVC_UART_UART_InitStructure.USART_BaudRate = baud;
    TVC_UART_UART_InitStructure.USART_WordLength = USART_WordLength_8b;
    TVC_UART_UART_InitStructure.USART_StopBits = USART_StopBits_1;
    TVC_UART_UART_InitStructure.USART_Parity = USART_Parity_No;
    TVC_UART_UART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    TVC_UART_UART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(TVC_UART, &TVC_UART_UART_InitStructure);

    USART_ClearFlag(TVC_UART, USART_IT_RXNE);

    USART_ITConfig(TVC_UART, USART_IT_RXNE, ENABLE); 
    USART_Cmd(TVC_UART, ENABLE);  
}

void NVIC_TVC_UART_Configuration(void)
{	
    NVIC_InitTypeDef TVC_UART_NVIC_InitStructure;						  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 

    TVC_UART_NVIC_InitStructure.NVIC_IRQChannel = TVC_UART_IRQn;
    TVC_UART_NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    TVC_UART_NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    TVC_UART_NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&TVC_UART_NVIC_InitStructure);
}


void TVC_UART_Init(unsigned short baud)
{
    RCC_TVC_UART_Configuration();
    GPIO_TVC_UART_Configuration();
    UART_TVC_UART_Configuration(baud);
    NVIC_TVC_UART_Configuration();	  	
}

void TVC_UART_IRQHandler(void)
{					
    u8  temp8;		
    if(USART_GetITStatus(TVC_UART, USART_IT_RXNE) != RESET) 
    {
        USART_ClearITPendingBit(TVC_UART,USART_IT_RXNE); 
        temp8 = USART_ReceiveData(TVC_UART);  
        if(TVC_UART_RecCnt == 0)
        {
            TVC_UART_RxBuf[TVC_UART_RecCnt] = temp8;
        } 
        if(TVC_UART_RxBuf[0]== PELCOD_STX)
        {		      
            TVC_UART_RxBuf[TVC_UART_RecCnt] = temp8;
            TVC_UART_RecCnt++;
            if(TVC_UART_RecCnt == PELCOD_LEN)  //6 0531              
            {
                TVC_UART_RecCnt     =   0;   
//                TVC_Pelcod.addr     =	TVC_UART_RxBuf[1];                
//                TVC_Pelcod.cmd1     =	TVC_UART_RxBuf[2];
//                TVC_Pelcod.cmd2     =	TVC_UART_RxBuf[3];
//                TVC_Pelcod.data1    =	TVC_UART_RxBuf[4];
//                TVC_Pelcod.data2    =	TVC_UART_RxBuf[5];
//                TVC_Pelcod.crc      =	TVC_UART_RxBuf[6];							 
//                TVC_UART_ProtoFlag  =   1; 
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
        USART_ReceiveData(TVC_UART); 
    }
}


int TVC_UART_SendData(void*buf,int len)
{
    int i,wait,count=0;
    unsigned char *data = (unsigned char*)buf;	 
    for(i=0;i<len;i++)
    {
        if(TVC_UART_ProtoFlag && (len>40))
        {
            return 0;
        }
        USART_ClearFlag(TVC_UART,USART_FLAG_TC);
        USART_SendData(TVC_UART,data[count]); 
        for(wait=0;!(USART_GetFlagStatus(TVC_UART, USART_FLAG_TC));wait++)
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

