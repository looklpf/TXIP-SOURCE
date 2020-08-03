#include "stm32f10x.h"
#include "ComCmd_proc.h"
#include "IR_uart.h"
#include "SysCfg.h"
#include "LensDriver.h"
#include "LaserFunction.h"
#include "WatchHome.h"
#include "Curise.h"
#include "LineScan.h"
#include "AppleScan.h"
extern volatile unsigned char CameraLense_ZoomStop_Flag;
volatile unsigned char CameraLense_ZoomStop_Timer_Cnt=0;


int laserCheckCount = 0;
extern char laserStatusChange;

void time2_init(void)
{
    TIM_TimeBaseInitTypeDef TIM2_TimeBaseStructure ;  
    NVIC_InitTypeDef NVIC_InitStructure;		    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);                       

    TIM2_TimeBaseStructure.TIM_Period = 10000;	 //  定时1s
    TIM2_TimeBaseStructure.TIM_Prescaler = 7199;	  
    TIM2_TimeBaseStructure.TIM_ClockDivision = 0;   
    TIM2_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
    TIM_TimeBaseInit(TIM2, &TIM2_TimeBaseStructure); 
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); 
    TIM_Cmd(TIM2, ENABLE);
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
}

 





void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) 
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);	
    } 
    CuriseTimeCnt();
    LineScanTimeCnt();
    AppleScanTimeCnt();
    WatchHomeTimeCnt();
    ClearCmdComplexCnt();
}


void time3_init(void)
{
    TIM_TimeBaseInitTypeDef TIM3_TimeBaseStructure ;  
    NVIC_InitTypeDef TIM3_NVIC_InitStructure;		  	    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 , ENABLE);                          

    TIM3_TimeBaseStructure.TIM_Period = 0x2710;	 //  定时100ms
    TIM3_TimeBaseStructure.TIM_Prescaler = 719;	  
    TIM3_TimeBaseStructure.TIM_ClockDivision = 0;   
    TIM3_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
    TIM_TimeBaseInit(TIM3, &TIM3_TimeBaseStructure); 
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM3, ENABLE); 
		
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    TIM3_NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    TIM3_NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    TIM3_NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    TIM3_NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&TIM3_NVIC_InitStructure);
		
}



void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) 
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);	
	} 	
    if(laserStatusChange == 1)
    {
        laserCheckCount ++ ;
    }
	if((sysPara.ptReturn == 1)||(sysPara.viewReturn ==1))
    {
        sysPara.returnCnt ++;
    }
  
}

void time4_init(void)
{
    TIM_TimeBaseInitTypeDef TIM2_TimeBaseStructure ;  
    NVIC_InitTypeDef NVIC_InitStructure;		      
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4 , ENABLE);                         

    TIM2_TimeBaseStructure.TIM_Period = 4;	  
    TIM2_TimeBaseStructure.TIM_Prescaler = 7199;	  
    TIM2_TimeBaseStructure.TIM_ClockDivision = 0;   
    TIM2_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
    TIM_TimeBaseInit(TIM4, &TIM2_TimeBaseStructure); 
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);  
    TIM_Cmd(TIM4, ENABLE); 
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
}


void TIM4_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) 
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);	
	} 	
    pLaser->Move();
    CamFocusPwmCnt();
}
void TIM5_NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  													
    NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;	  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;	
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void TIM5_Configuration(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5 , ENABLE);
    TIM_DeInit(TIM5);
    TIM_TimeBaseStructure.TIM_Period=10000;		 								
    TIM_TimeBaseStructure.TIM_Prescaler= (72 - 1);				    
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 	
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
    TIM_ClearFlag(TIM5, TIM_FLAG_Update);							    		
    TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE);
    TIM_Cmd(TIM5, ENABLE);																		
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5 , DISABLE);		  
}
void TIM5_IRQHandler(void)
{
    if ( TIM_GetITStatus(TIM5 , TIM_IT_Update) != RESET ) 
    {	
        TIM_ClearITPendingBit(TIM5 , TIM_FLAG_Update);    
    }	
    if(CameraLense_ZoomStop_Flag)
    {
        CameraLense_ZoomStop_Timer_Cnt++;
        if(CameraLense_ZoomStop_Timer_Cnt >= 5)
        {
            CameraLense_ZoomStop_Timer_Cnt = 0;
            CameraLense_ZoomStop_Flag = 0;
        }
    }
}
