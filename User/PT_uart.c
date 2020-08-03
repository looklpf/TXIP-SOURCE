
#include <stdio.h>
#include "PT_uart.h" 
#include "SysCfg.h" 
#include "delay.h" 
#include "TVC_uart.h" 
#include "stm32f10x_dma.h" 
#include "protocol_type.h"
#include "ComCmd_proc.h"
#include <math.h>
#include "MiscAlg.h"
#include "LensDriver.h"
#include "hp_conf.h"
#include "TTL_uart.h"
#include "Algorithm.h"
unsigned volatile char 	PT_UART_RxBuf1[7];
unsigned char 		    PT_UART_TxBuf[7];
unsigned volatile char 	PT_UART_ProtoType = 0;
unsigned volatile char 	PT_UART_RecCnt ;
unsigned char 		    TiltInv_flag;
unsigned int 		    PanRevCnt		=0;
unsigned int 		    TiltRevCnt		=0;


void RCC_PT_UART_Configuration(void)
{     
    RCC_APB2PeriphClockCmd(PT_UART_GPIO_CLK, ENABLE);	
    RCC_APB1PeriphClockCmd(PT_UART_UART_CLK, ENABLE); 
}


void GPIO_PT_UART_Configuration(void)
{    
    GPIO_InitTypeDef    PT_UART_GPIO_InitStructure;

    PT_UART_GPIO_InitStructure.GPIO_Pin     = PT_UART_RX_PIN;
    PT_UART_GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IPU;
    GPIO_Init(PT_UART_GPIO,&PT_UART_GPIO_InitStructure);	  

    PT_UART_GPIO_InitStructure.GPIO_Pin     = PT_UART_TX_PIN;
    PT_UART_GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    PT_UART_GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF_PP;
    GPIO_Init(PT_UART_GPIO,&PT_UART_GPIO_InitStructure);
}

void UART_PT_UART_Configuration(unsigned short baud)
{
    USART_InitTypeDef   PT_UART_UART_InitStructure;  

    PT_UART_UART_InitStructure.USART_BaudRate   = baud;
    PT_UART_UART_InitStructure.USART_WordLength = USART_WordLength_8b;
    PT_UART_UART_InitStructure.USART_StopBits   = USART_StopBits_1;
    PT_UART_UART_InitStructure.USART_Parity     = USART_Parity_No;
    PT_UART_UART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    PT_UART_UART_InitStructure.USART_Mode       = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(PT_UART, &PT_UART_UART_InitStructure);
    USART_ClearFlag(PT_UART, USART_IT_RXNE);

    USART_ITConfig(PT_UART, USART_IT_RXNE, ENABLE); 		
    USART_Cmd(PT_UART, ENABLE); 
}

void NVIC_PT_UART_Configuration(void)
{	
    NVIC_InitTypeDef 	PT_UART_NVIC_InitStructure; 
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);   
 
    PT_UART_NVIC_InitStructure.NVIC_IRQChannel 		= PT_UART_IRQn;
    PT_UART_NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority 	= 1;
    PT_UART_NVIC_InitStructure.NVIC_IRQChannelSubPriority 			= 0;
    PT_UART_NVIC_InitStructure.NVIC_IRQChannelCmd 		            = ENABLE;
    NVIC_Init(&PT_UART_NVIC_InitStructure);	  
}


void PT_UART_Init(unsigned short baud)
{
    RCC_PT_UART_Configuration();
    GPIO_PT_UART_Configuration();
    UART_PT_UART_Configuration(baud);
    NVIC_PT_UART_Configuration();	  	
}


void PT_UART_IRQHandler(void)
{
    u8 temp8 ;
    static unsigned short 	PanPreAngle,TiltPreAngle;
    static unsigned char    TiltStopErr_flag = 0;
    if(USART_GetITStatus(PT_UART,USART_IT_RXNE)!=RESET)
    {
        USART_ClearITPendingBit(PT_UART,USART_IT_RXNE); 
        temp8=USART_ReceiveData(PT_UART);
        
        if(PT_UART_RecCnt==0)
        {
            PT_UART_RxBuf1[PT_UART_RecCnt]	= temp8;
        }
        if(PT_UART_RxBuf1[0]==PELCOD_STX)
        {
            PT_UART_RxBuf1[PT_UART_RecCnt]	= temp8;
            PT_UART_RecCnt++;
            if(PT_UART_RecCnt==PELCOD_LEN)
            {
                PT_UART_RecCnt	=	0; 
                if(PT_UART_RxBuf1[3]==0x59)
                {
                    PTStruct.PanCurAngle 	= PT_UART_RxBuf1[4];
                    PTStruct.PanCurAngle 	= (PTStruct.PanCurAngle << 8) | PT_UART_RxBuf1[5];  
                    PT_UART_ProtoType		= PELCO_D;							 
					if(PTStruct.Pan_PresetSpeedSet_Flag)
					{
						PanRevCnt++;
                        if(PanRevCnt>30) 
                        {
                            PanRevCnt = 0;
                            if(PanPreAngle != PTStruct.PanCurAngle)
                            {
                                    PanPreAngle = PTStruct.PanCurAngle;
                                   
                            }
                            else //if(PanPreAngle == PTStruct.PanCurAngle)
                            {				
								PTStruct.PanCtrl	=	0;
                                PTStruct.Pan_PresetSpeedSet_Flag = 0;
                            }
                        }

					}
                    else if(PTStruct.PanAngleSet_Flag)
                    {
                        PanRevCnt++;
                        if(PanRevCnt>30) 
                        {
                            PanRevCnt	=	0;
                            if(PanPreAngle != PTStruct.PanCurAngle)
                            {
                                PanPreAngle 		= PTStruct.PanCurAngle;
                            }
                            else //if(PanPreAngle == PTStruct.PanCurAngle)
                            {				
								PTStruct.PanCtrl	=	0;
                                PTStruct.PanAngleSet_Flag = 0;
                            }
                        }
                        if(PTStruct.PanCurAngle == PTStruct.PanSetAngle)
                        {
                            PanRevCnt	=	0;
                            PTStruct.PanAngleSet_Flag = 0;
                        }
                    }							
                }
                else if(PT_UART_RxBuf1[3]==0x5B)
                {
                    PTStruct.TiltCurAngle = PT_UART_RxBuf1[4];
                    PTStruct.TiltCurAngle = (PTStruct.TiltCurAngle << 8) | PT_UART_RxBuf1[5];
                    PT_UART_ProtoType			= PELCO_D;
					if(PTStruct.Tilt_PresetSpeedSet_Flag)
					{
						TiltRevCnt++;
                        if(TiltRevCnt>30)//给10次接收到角度
                        {
                            TiltRevCnt	=	0;
                            if(TiltPreAngle != PTStruct.TiltCurAngle)
                            {
								TiltPreAngle		=	PTStruct.TiltCurAngle;
								TiltStopErr_flag	=	0;
                            }
                            else 
                            {				
								PTStruct.TiltCtrl	=	0;
                                PTStruct.Tilt_PresetSpeedSet_Flag = 0;
                            }
                        }

					}
                    else if(PTStruct.TiltAngleSet_Flag)
                    {
                        TiltRevCnt++;
                        if(TiltRevCnt>30)//给10次接收到角度
                        {
                            TiltRevCnt	=	0;
                            if(TiltPreAngle != PTStruct.TiltCurAngle)
                            {
								TiltPreAngle		=	PTStruct.TiltCurAngle;
								TiltStopErr_flag	=	0;
                            }
                            else if(TiltPreAngle	== PTStruct.TiltCurAngle)
                            {				
								TiltStopErr_flag	=	1;
                            }
                        }
                        if((PTStruct.TiltCurAngle == PTStruct.TiltSetAngle) || TiltStopErr_flag)
                        {
                            TiltRevCnt											=	0;
                            TiltStopErr_flag						=	0;
                            PTStruct.TiltCtrl						=	0;
                            PTStruct.TiltAngleSet_Flag 	= 0;
                        }							 
                    }							
                }
            }
        }        
    }
    else
    {
        USART_ReceiveData(PT_UART);
    }   
}

int PT_UART_SendData(void*buf,int len)
{
    int i,wait,count    =	0;
    unsigned char*data	=	(unsigned char*)buf;
    for(i=0;i<len;i++)
    {
        USART_ClearFlag(PT_UART,	USART_FLAG_TC);
        USART_SendData(PT_UART,		data[count]);
        for(wait=0;	!(USART_GetFlagStatus(PT_UART,USART_FLAG_TC));	wait++)
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
void PT_stop(void)
{
    unsigned int i,n 				= 0;
    unsigned char check_sum = 0;
    
    PT_UART_TxBuf[n++] = PELCOD_STX;
    PT_UART_TxBuf[n++] = PT_ADDR;
    PT_UART_TxBuf[n++] = 0; 	
    PT_UART_TxBuf[n++] = 0; 	
    PT_UART_TxBuf[n++] = 0; 
    PT_UART_TxBuf[n++] = 0; 	
    for(i = 1; i < n; i++)
    {
            check_sum = check_sum + PT_UART_TxBuf[i]; 
    }
    PT_UART_TxBuf[n++] 	= check_sum & 0xFF;		  
    PT_UART_SendData(PT_UART_TxBuf,	n);  
}

void PT_ctrl(PTStruct_TypeDef *PTCtrl,CameraStruct_TypeDef *camera)
{
    
    unsigned int i,n = 0;
    unsigned char check_sum = 0;
    unsigned char cmd = PTCtrl->PanCtrl | PTCtrl->TiltCtrl;
    if(cmd > 0x14)
    {
        return ;
    }
    if(cmd != NULL)
    {
        PT_UART_TxBuf[n++] = PELCOD_STX;
        PT_UART_TxBuf[n++] = PT_ADDR;
        PT_UART_TxBuf[n++] = 0;
        PT_UART_TxBuf[n++] = cmd;
        if(camera == NULL)
        {
            PT_UART_TxBuf[n++] = PTCtrl->PanSpeed;
            PT_UART_TxBuf[n++] = PTCtrl->TiltSpeed;
        }
        else
        {
            if(sysPara.autoSpeed == 1)
            {
                #ifdef IPC_EN
                    unsigned int val;
                    float valf;
                    camera->ZoomCurPosition = Get_Camera_Zoom_Position();
                    
                    val = (0x4000-camera->ZoomCurPosition);
                    valf = (float)val/0x4000;
                    valf = valf*64+6;
           
                    if(valf < PTCtrl->PanSpeed)
                    {
                        PTCtrl->PanSpeed = valf;
                    }
                    if(valf < PTCtrl->TiltSpeed)
                    {
                        PTCtrl->TiltSpeed = valf;
                    }
                    PT_UART_TxBuf[n++] = PTCtrl->PanSpeed ;
                    PT_UART_TxBuf[n++] = PTCtrl->TiltSpeed;
                #else
                    unsigned int curDistance,allDistance;
                    float k;
                    camera->ZoomCurPosition = LENS->Camera->ReadZoomValue();
                    allDistance = pMiscAlg->UnIntAbs(alg->GetZoomOutAd(),alg->GetZoomInAd());
                    curDistance = pMiscAlg->UnIntAbs(camera->ZoomCurPosition,alg->GetZoomInAd());
                    k = (float)curDistance/(float)allDistance;
                    k = k*64 + 6;
                    if(k < PTCtrl->PanSpeed)
                    {
                        PTCtrl->PanSpeed = k;
                    }
                    if(k < PTCtrl->TiltSpeed)
                    {
                        PTCtrl->TiltSpeed = k;
                    }
                    PT_UART_TxBuf[n++] = PTCtrl->PanSpeed ;
                    PT_UART_TxBuf[n++] = PTCtrl->TiltSpeed;
//                    if(camera->WideInc_flag)
//                    {
//                        
//                        PT_UART_TxBuf[n++] = (PTCtrl->PanSpeed > 6)? ((PTCtrl->PanSpeed-6) * fabs(camera->ZoomCurPosition - camera->ZoomMinPosition) / (camera->ZoomMaxPosition - camera->ZoomMinPosition)  + 6) : PTCtrl->PanSpeed;
//                        PT_UART_TxBuf[n++] = (PTCtrl->TiltSpeed>6)?((PTCtrl->TiltSpeed-6) * fabs(camera->ZoomCurPosition - camera->ZoomMinPosition) / (camera->ZoomMaxPosition - camera->ZoomMinPosition)  + 6) : PTCtrl->TiltSpeed;
//                    }
//                    else
//                    {
//                        PT_UART_TxBuf[n++] = (PTCtrl->PanSpeed > 6)	? ((PTCtrl->PanSpeed-6) * fabs(camera->ZoomMaxPosition - camera->ZoomCurPosition) / (camera->ZoomMaxPosition - camera->ZoomMinPosition)  + 6) 	: PTCtrl->PanSpeed;
//                        PT_UART_TxBuf[n++] = (PTCtrl->TiltSpeed>6)	?	((PTCtrl->TiltSpeed-6) * fabs(camera->ZoomMaxPosition - camera->ZoomCurPosition) / (camera->ZoomMaxPosition - camera->ZoomMinPosition)  + 6) 	: PTCtrl->TiltSpeed;				
//                    }
                #endif    
                
            }
            else
            {
                PT_UART_TxBuf[n++] = PTCtrl->PanSpeed;
                PT_UART_TxBuf[n++] = PTCtrl->TiltSpeed;
            }
        }
        for(i = 1; i < n; i++)
        {
            check_sum = check_sum + PT_UART_TxBuf[i]; 
        }
        PT_UART_TxBuf[n++] = check_sum 	& 0xFF;		
        PT_UART_SendData(PT_UART_TxBuf,	n);
    }
    else
    {
        PT_stop();
    }		
}

void PT_SetPreset(unsigned char PresetNo)
{
    unsigned int i,n 				= 0;
    unsigned char check_sum = 0;
    PT_UART_TxBuf[n++] = PELCOD_STX;
    PT_UART_TxBuf[n++] = PT_ADDR;
    PT_UART_TxBuf[n++] = 0;
    PT_UART_TxBuf[n++] = 0x03;
    PT_UART_TxBuf[n++] = 0;		
    PT_UART_TxBuf[n++] = PresetNo;
    for(i = 1; i < n; i++)
    {
        check_sum = check_sum + PT_UART_TxBuf[i]; 
    }
    PT_UART_TxBuf[n++] = check_sum & 0xFF;		
    PT_UART_SendData(PT_UART_TxBuf,n);
}
void PT_ClearPreset(unsigned char PresetNo)
{
    unsigned int i,n 					= 0;
    unsigned char check_sum 	= 0;
    PT_UART_TxBuf[n++] = PELCOD_STX;
    PT_UART_TxBuf[n++] = PT_ADDR;
    PT_UART_TxBuf[n++] = 0;
    PT_UART_TxBuf[n++] = 0x05;
    PT_UART_TxBuf[n++] = 0;		
    PT_UART_TxBuf[n++] = PresetNo;
    for(i = 1; i < n; i++)
    {
        check_sum = check_sum + PT_UART_TxBuf[i]; 
    }
    PT_UART_TxBuf[n++] = check_sum & 0xFF;		
    PT_UART_SendData(PT_UART_TxBuf,	n);
}

void PT_CallPreset(unsigned char PresetNo)
{
    unsigned int i,n 					= 0;
    unsigned char check_sum 	= 0;
    PT_UART_TxBuf[n++] = PELCOD_STX;
    PT_UART_TxBuf[n++] = PT_ADDR;
    PT_UART_TxBuf[n++] = 0;
    PT_UART_TxBuf[n++] = 0x07;
    PT_UART_TxBuf[n++] = 0;		
    PT_UART_TxBuf[n++] = PresetNo;
    for(i = 1; i < n; i++)
    {
        check_sum = check_sum + PT_UART_TxBuf[i]; 
    }
    PT_UART_TxBuf[n++] = check_sum 	& 0xFF;		
    PT_UART_SendData(PT_UART_TxBuf,	n);
}

void PT_CallPreset_WithSpeedSet(PTStruct_TypeDef *PT)
{
    unsigned short 	        PanDiffAngle,TiltDiffAngle; 
    static unsigned char 	near_flag	=	0;

    if(PTStruct.Pan_PresetSpeedSet_Flag)
    {
        if(PT->PanCtrl == PT_RIGHT)
        {
            if (PT->PanSetAngle >= PT->PanCurAngle)
            {
                PanDiffAngle = PT->PanSetAngle - PT->PanCurAngle;
            }
            else
            {
                PanDiffAngle = 36000 - PT->PanCurAngle + PT->PanSetAngle;
            }
        }
        else if(PT->PanCtrl == PT_LEFT)
        {
            if(PT->PanSetAngle <= PT->PanCurAngle)
            {
                PanDiffAngle = PT->PanCurAngle - PT->PanSetAngle;
            }
            else
            {
                PanDiffAngle = 36000 - PT->PanSetAngle + PT->PanCurAngle;
            }
        }		
        if(PanDiffAngle<200)
        {
            near_flag													=	0;			
            PTStruct.Pan_PresetSpeedSet_Flag 	= RESET;
            PTStruct.PanCtrl 									= 0;
            if(!PTStruct.Tilt_PresetSpeedSet_Flag)
            {
                PT_SetPanAngle(&PTStruct);
                delay_ms(100);
                PT_SetTiltAngle(&PTStruct);
            }							
        }
        else if(PanDiffAngle<400)
        {
            near_flag	=	1;
            if(PTStruct.PresetSpeed > 6)
            {
                PTStruct.PanSpeed = 6;
            }
            else
            {
                PTStruct.PanSpeed = PTStruct.PresetSpeed;
            }
        }
        else if(PanDiffAngle<800)
        {
            near_flag	=	1;
            if(PTStruct.PresetSpeed > 10)
            {
                PTStruct.PanSpeed = 10;
            }
            else
            {
                PTStruct.PanSpeed = PTStruct.PresetSpeed;
            }
        }

        else if(PanDiffAngle<1500)
        {
            if(PTStruct.PresetSpeed > 12)
            {
                PTStruct.PanSpeed = 12;
            }
            else
            {
                PTStruct.PanSpeed = PTStruct.PresetSpeed;
            }
        } 
        else
        {	
            if(near_flag)//解决速度大时出现走过现象
            {
                near_flag   = 0;
                PT->Pan_PresetSpeedSet_Flag = RESET;
                PT->PanCtrl = 0;
                if(!PT->Tilt_PresetSpeedSet_Flag)
                {
                    PT_SetPanAngle(PT);
                    delay_ms(100);
                    PT_SetTiltAngle(PT);
                }
            }
            else
            {		          					
                PTStruct.PanSpeed = PTStruct.PresetSpeed;					
            }	
        }
    }

    //计算俯仰速度							
    if(PTStruct.Tilt_PresetSpeedSet_Flag)
    {
        if(PTStruct.TiltSetAngle>PTStruct.TiltCurAngle)
        {
            TiltDiffAngle	=	PTStruct.TiltSetAngle-PTStruct.TiltCurAngle ;
            if(TiltDiffAngle>9000)
            {
                if(PTStruct.TiltCtrl==PT_DOWN)
                {
                    TiltInv_flag	=	1;
                }
                else
                {
                    TiltInv_flag	=	0;
                }
                //PTStruct.TiltCtrl=PT_UP ;
                TiltDiffAngle	=	36000-PTStruct.TiltSetAngle+PTStruct.TiltCurAngle ;
            }
            else
            {
                if(PTStruct.TiltCtrl==PT_UP)
                {
                    TiltInv_flag=1;
                }
                else
                {
                    TiltInv_flag=0;
                }
                    //PTStruct.TiltCtrl=PT_DOWN ;
            }
        }
        else 
        {										
            TiltDiffAngle	=	PTStruct.TiltCurAngle-PTStruct.TiltSetAngle ;
            if(TiltDiffAngle>9000)
            {
                if(PTStruct.TiltCtrl==PT_UP)
                {
                    TiltInv_flag	=	1;
                }
                else
                {
                    TiltInv_flag	=	0;
                }
                //PTStruct.TiltCtrl=PT_DOWN ;
                TiltDiffAngle	=	36000-PTStruct.TiltCurAngle+PTStruct.TiltSetAngle ;
            }
            else
            {
                if(PTStruct.TiltCtrl==PT_DOWN)
                {
                    TiltInv_flag	=	1;
                }
                else
                {
                    TiltInv_flag	=	0;
                }
                    //PTStruct.TiltCtrl=PT_UP ;
            }
        }
        if((TiltDiffAngle<200) || TiltInv_flag)
        {			
            if(TiltInv_flag)
            {
                TiltInv_flag	=	0;
            }										
            PTStruct.Tilt_PresetSpeedSet_Flag = RESET;
            PTStruct.TiltCtrl 								= 0;								
            if(!PTStruct.Pan_PresetSpeedSet_Flag)
            {
                PT_SetTiltAngle(&PTStruct);
                delay_ms(100);	      
                PT_SetPanAngle(&PTStruct);
            }
        }
        else if(TiltDiffAngle<400)
        {
            if(PTStruct.PresetSpeed > 5)
            {
                PTStruct.TiltSpeed = 5;
            }
            else
            {
                PTStruct.TiltSpeed = PTStruct.PresetSpeed;
            }
        }
        else if(TiltDiffAngle<800)
        {
            if(PTStruct.PresetSpeed > 9)
            {
                PTStruct.TiltSpeed = 9;
            }
            else
            {
                PTStruct.TiltSpeed = PTStruct.PresetSpeed;
            }
        }
        else if(TiltDiffAngle<1500)
        {
            if(PTStruct.PresetSpeed > 11)
            {
                PTStruct.TiltSpeed = 11;
            }
            else
            {
                PTStruct.TiltSpeed = PTStruct.PresetSpeed;
            }
        } 
        else
        {
            PTStruct.TiltSpeed = PTStruct.PresetSpeed;
        }
    }
    if(PT->Pan_PresetSpeedSet_Flag || PT->Tilt_PresetSpeedSet_Flag)
    {
        PT_ctrl(PT,NULL);	
        delay_ms(50);
    }
}



void PT_SetPanAngle(PTStruct_TypeDef *PT)
{
    if(PT->PanSetAngle > 36000)
    {
        return;
    }
    unsigned int i,n = 0;
    unsigned char check_sum = 0;
    PT_UART_TxBuf[n++] = PELCOD_STX;
    PT_UART_TxBuf[n++] = PT_ADDR;
    PT_UART_TxBuf[n++] = 0;
    PT_UART_TxBuf[n++] = 0x4B;
    PT_UART_TxBuf[n++] = (PT->PanSetAngle >> 8) & 0xFF;		
    PT_UART_TxBuf[n++] = PT->PanSetAngle & 0xFF;
    for(i = 1; i < n; i++)
    {
        check_sum = check_sum + PT_UART_TxBuf[i]; 
    }
    PT_UART_TxBuf[n++] = check_sum & 0xFF;		
    PT_UART_SendData(PT_UART_TxBuf,n);
    PT->PanAngleSet_Flag = SET;
}

void PT_SetTiltAngle(PTStruct_TypeDef *PT)
{
    if(PT->TiltSetAngle > 36000)
    {
        return;
    }
    unsigned int i,n 				= 0;
    unsigned char check_sum = 0;
    PT_UART_TxBuf[n++] = PELCOD_STX;
    PT_UART_TxBuf[n++] = PT_ADDR;
    PT_UART_TxBuf[n++] = 0;
    PT_UART_TxBuf[n++] = 0x4D;
    PT_UART_TxBuf[n++] = (PT->TiltSetAngle >> 8) & 0xFF;		
    PT_UART_TxBuf[n++] = PT->TiltSetAngle & 0xFF;
    for(i = 1; i < n; i++)
    {
        check_sum = check_sum + PT_UART_TxBuf[i]; 
    }
    PT_UART_TxBuf[n++] = check_sum & 0xFF;		
    PT_UART_SendData(PT_UART_TxBuf,n);
    PT->TiltAngleSet_Flag = SET;
}

void PtSetHorAngle(unsigned int angle)
{
    unsigned char buf[]={0xff,0x01,0x00,0x4b,0x00,0x00,0};
    buf[4] = (angle>>8)&0xff;
    buf[5] = (angle>>0)&0xff;
    buf[6] = pMiscAlg->PelcodCrc(buf);
    PT_UART_SendData(buf,7);
    if(sysPara.sysTest == 1)
    {
        TVC_UART_SendData(buf,7);
    }
}
void PtSetVerAngle(unsigned int angle)
{
    unsigned char buf[]={0xff,0x01,0x00,0x4d,0x00,0x00,0};
    buf[4] = (angle>>8)&0xff;
    buf[5] = (angle>>0)&0xff;
    buf[6] = pMiscAlg->PelcodCrc(buf);
    PT_UART_SendData(buf,7);
    if(sysPara.sysTest == 1)
    {
        TVC_UART_SendData(buf,7); 
    }
    
}
void PtSetHorZero(void)
{
    #ifdef YT_JEC_EN
    PT_SetPreset(201);
    #else
    PT_SetPreset(103);
    #endif
}
void PtSetVerZero(void)
{
    #ifdef YT_JEC_EN
    PT_SetPreset(202);
    #else
    PT_SetPreset(104);
    #endif
}
void PtDefaultHorZero(void)
{
    #ifdef YT_JEC_EN
    PT_ClearPreset(201);
    #else
    PT_CallPreset(103);
    #endif
}
void PtDefaultVerZero(void)
{
    #ifdef YT_JEC_EN
    PT_ClearPreset(202);
    #else
    PT_CallPreset(104);
    #endif
}

void PtRestart(void)
{
    PT_CallPreset(254);
}

void PtUp(unsigned char speed)
{
    unsigned char buf[]={0xff,0x01,0x00,0x08,0x00,0,0};
    buf[5] = speed;
    buf[6] = pMiscAlg->PelcodCrc(buf);
    PT_UART_SendData(buf,7);
}

void PtDown(unsigned char speed)
{
    unsigned char buf[]={0xff,0x01,0x00,0x10,0x00,0,0};
    buf[5] = speed;
    buf[6] = pMiscAlg->PelcodCrc(buf);
    PT_UART_SendData(buf,7);
}

void PtLeft(unsigned char speed)
{
    unsigned char buf[]={0xff,0x01,0x00,0x04,0,0x00,0};
    buf[4] = speed;
    buf[6] = pMiscAlg->PelcodCrc(buf);
    PT_UART_SendData(buf,7);
}

void PtRight(unsigned char speed)
{
    unsigned char buf[]={0xff,0x01,0x00,0x02,0,0x00,0};
    buf[4] = speed;
    buf[6] = pMiscAlg->PelcodCrc(buf);
    PT_UART_SendData(buf,7);
}
void PtLeftUp(unsigned char hSpeed,unsigned char vSpeed)
{
    unsigned char buf[]={0xff,0x01,0x00,0x0c,0,0,0};
    buf[4] = hSpeed;
    buf[5] = vSpeed;
    buf[6] = pMiscAlg->PelcodCrc(buf);
    PT_UART_SendData(buf,7);
}

void PtLeftDown(unsigned char hSpeed,unsigned char vSpeed)
{
    unsigned char buf[]={0xff,0x01,0x00,0x14,0,0,0};
    buf[4] = hSpeed;
    buf[5] = vSpeed;
    buf[6] = pMiscAlg->PelcodCrc(buf);
    PT_UART_SendData(buf,7);
}

void PtRightUp(unsigned char hSpeed,unsigned char vSpeed)
{
    unsigned char buf[]={0xff,0x01,0x00,0x0a,0,0,0};
    buf[4] = hSpeed;
    buf[5] = vSpeed;
    buf[6] = pMiscAlg->PelcodCrc(buf);
    PT_UART_SendData(buf,7);
}

void PtRightDown(unsigned char hSpeed,unsigned char vSpeed)
{
    unsigned char buf[]={0xff,0x01,0x00,0x12,0,0,0};
    buf[4] = hSpeed;
    buf[5] = vSpeed;
    buf[6] = pMiscAlg->PelcodCrc(buf);
    PT_UART_SendData(buf,7);
}

void PtStop(void)
{
    unsigned char buf[]={0xff,0x01,0x00,0x00,0x00,0x00,0};
    buf[6] = pMiscAlg->PelcodCrc(buf);
    PT_UART_SendData(buf,7);
}

