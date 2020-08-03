#include "stm32f10x.h"
#include "camera_lense.h"
#include "SysCfg.h"
#include "PT_uart.h"
#include "delay.h"
#include "hp_conf.h"
#include "TVC_uart.h"
#include "IR_uart.h"
#include "ComCmd_proc.h"
#include "stm32f10x_adc.h"
#include "Algorithm.h"
#include "LensDriver.h"
#include "LaserFunction.h"
#include "hp_conf.h"
#include "VC_uart.h"
extern 	CameraStruct_TypeDef CameraStruct; 
unsigned short 					fc_value							=	0;
unsigned char 					init_fc_value					=	0;
unsigned char 					IR_init_fc_value			=	0;
volatile unsigned short ADC1_ConvertedBuf[4] 	= {0,0,0,};
extern 									PTStruct_TypeDef PTStruct;
extern unsigned short 	FocusSetPosition2;
extern unsigned short 	IR_FocusSetPosition2;
extern unsigned char 		FocusSet_Correct;
unsigned short CamZoomCurPosPre=0;
unsigned int IR_correct_AD;
unsigned char IR_correct_flag;
unsigned short Zoom_Count_Cam=0,Zoom_Count_IR=0;  //20180831
volatile unsigned char CameraLense_ZoomStop_Flag = 0;
//-------------------- Yamano Y34Z15RADP--------------------
unsigned short ZoomAD[]={3736,3536,3443,3260,3178,3050,2935,2804,2670,2474,2370,2260,2160,2047,1950,1756,1587,1432,1310,1221,1112,1000,910,785,683,582,482,385,235};
unsigned short LenseAngleX100[]={1974,1696,1562,1353,1263,1137,1023,911,813,686,630,567,514,455,418,344,287,246,214,197,174,155,137,120,106,94,84,73,61};
  
void GPIO_CameraLense_Configuration(void)
{
		GPIO_InitTypeDef 					CameraLense_GPIO_InitStructure;
		 /* GPIOD Periph clock enable */
		RCC_APB2PeriphClockCmd(CAMERA_LENSE_CLK, ENABLE);	 //
		/* Configure zoom and focus pins in output pushpull mode */
		CameraLense_GPIO_InitStructure.GPIO_Pin = CAMERA_FOCUS_PIN1 | CAMERA_FOCUS_PIN2 | CAMERA_ZOOM_PIN1 | CAMERA_ZOOM_PIN2 | CAMERA_IC_PIN1 | CAMERA_IC_PIN2 ;
		GPIO_WriteBit(CAMERA_LENSE_GPIO, CAMERA_FOCUS_PIN1 , Bit_SET);
		GPIO_WriteBit(CAMERA_LENSE_GPIO, CAMERA_FOCUS_PIN2 , Bit_SET);   
		GPIO_WriteBit(CAMERA_LENSE_GPIO, CAMERA_ZOOM_PIN1 , Bit_SET);
		GPIO_WriteBit(CAMERA_LENSE_GPIO, CAMERA_ZOOM_PIN2 , Bit_SET);
		 
		CameraLense_GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		CameraLense_GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP; //推挽输出
		GPIO_Init(CAMERA_LENSE_GPIO, &CameraLense_GPIO_InitStructure);   	
		
		//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	
		//CameraLense_GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		//CameraLense_GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //推挽输出
		//GPIO_Init(GPIOB, &CameraLense_GPIO_InitStructure); 
}
//电机驱动芯片使能端PWM控制
void TIM4CH3_PWM_Init(void)
{
		GPIO_InitTypeDef 						FocusSpeed_GPIO_InitStructure;
		TIM_TimeBaseInitTypeDef  		TIM_TimeBaseStructure;
		TIM_OCInitTypeDef  					TIM_OCInitStructure;
		RCC_APB2PeriphClockCmd(CAMERA_FOCUSSPEED_CLK, ENABLE);	
		//使能端引脚配置,focus speed控制
		FocusSpeed_GPIO_InitStructure.GPIO_Pin 		= CAMERA_FOCUSSPEED_PIN;
		FocusSpeed_GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
		FocusSpeed_GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF_PP; //推挽输出
		GPIO_Init(CAMERA_FOCUSSPEED_GPIO, 		&FocusSpeed_GPIO_InitStructure);
		//配置TIM4
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, 	ENABLE); 

		/* -----------------------------------------------------------------------
		TIM4 Configuration: generate 2 PWM signals with fixed frequency；they can change with 3 different duty cycles:
		TIM4CLK = 72 MHz, Prescaler = 0x0, TIM4 counter clock = 72 MHz
		TIM4 ARR Register = 999 => TIM4 Frequency = TIM4 counter clock/(ARR + 1)
		TIM4 Frequency = 72 KHz.

		TIM4 Channel3 duty cycle = (TIM4_CCR3/ TIM4_ARR )* 100 = 85% ；(TIM4_CCR3/ TIM4_ARR))* 100 = 50%  ; (TIM4_CCR4/ TIM4_ARR))* 100 = 25%
		TIM4 Channel4 duty cycle 同上
		----------------------------------------------------------------------- */
		/* Time base configuration */		 
		TIM_TimeBaseStructure.TIM_Period 					= 99;       	//重载值，1MHz / (99 + 1)= 10k
		TIM_TimeBaseStructure.TIM_Prescaler 			= 72 - 1;	    //设置预分频：72Mhz / 72 = 1MHz = TIM4 counter clock
		TIM_TimeBaseStructure.TIM_ClockDivision 	= 0;					//设置时钟分频系数：不分频
		TIM_TimeBaseStructure.TIM_CounterMode 		= TIM_CounterMode_Up;  //向上计数模式

		TIM_TimeBaseInit(TIM4, 							&TIM_TimeBaseStructure);

		/* PWM1 Mode configuration: Channel3 */
		TIM_OCInitStructure.TIM_OCMode 			= TIM_OCMode_PWM1;	    //模式1 高电平所占比例
		TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	
		TIM_OCInitStructure.TIM_Pulse 			= 100;	   							//设置跳变值，占空比85%
		TIM_OCInitStructure.TIM_OCPolarity 	= TIM_OCPolarity_High;  //当定时器计数值小于Pulse值时为高电平

		TIM_OC3Init(TIM4, 					&TIM_OCInitStructure);	 	//使能TIM4通道3
		TIM_OC3PreloadConfig(TIM4, 	TIM_OCPreload_Enable);
		
		TIM_OC4Init(TIM4, 					&TIM_OCInitStructure);	 	//使能TIM4通道3
		TIM_OC4PreloadConfig(TIM4, 	TIM_OCPreload_Enable);

		TIM_ARRPreloadConfig(TIM4, 	ENABLE);			 						// 使能TIM4重载寄存器ARR
		/* TIM4 enable counter */
		TIM_Cmd(TIM4, 							ENABLE);             			//使能定时器4	
}
void Set_Camera_FocusSpeed(unsigned char speed)
{

		TIM_SetCompare4(TIM4, speed);
}
void ADC_CameraLense_Init(void)
{
		GPIO_InitTypeDef 			CameraLense_ADC_GPIO_InitStructure;
		ADC_InitTypeDef 			ADC1_InitStructure;
		DMA_InitTypeDef 			DMA1_InitStructure;
		//--------------------- clock config-------------------------
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC, ENABLE);
		RCC_ADCCLKConfig(RCC_PCLK2_Div6);	
		/* Enable DMA1 clock */
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

		//-----------------------GPIO configure-----------------------
		// Configure PC.10 and PC.11 (ADC Channel10 and Channel11) as analog input 
		CameraLense_ADC_GPIO_InitStructure.GPIO_Pin 	= CAMERA_FOCUS_ADC_PIN | CAMERA_ZOOM_ADC_PIN | IR_ZOOM_ADC_PIN | IR_FOCUS_ADC_PIN;
		CameraLense_ADC_GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AIN;	 //模拟输入
		GPIO_Init(CAMERA_ADC_GPIO, &CameraLense_ADC_GPIO_InitStructure);

		//----------------------- ADC1 configuration --------------------
		ADC1_InitStructure.ADC_Mode 							= ADC_Mode_Independent;				//独立模式
		ADC1_InitStructure.ADC_ScanConvMode 			= ENABLE;		 									//连续多通道模式
		ADC1_InitStructure.ADC_ContinuousConvMode = ENABLE;  										//连续转换
		ADC1_InitStructure.ADC_ExternalTrigConv 	= ADC_ExternalTrigConv_None; 	//不受外界
		ADC1_InitStructure.ADC_DataAlign 					= ADC_DataAlign_Right;				//右对齐
		ADC1_InitStructure.ADC_NbrOfChannel 			= 4; 													//扫描通道数2
		ADC_Init(ADC1, &ADC1_InitStructure);
		// ADC1 regular channels configuration  
		ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_239Cycles5);  	//规则序列1  
		ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_239Cycles5);	  //规则序列2
		ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 3, ADC_SampleTime_239Cycles5);  	//规则序列1  
		ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 4, ADC_SampleTime_239Cycles5);	  //规则序列2
		// Enable ADC1 
		ADC_Cmd(ADC1, ENABLE);
		//开启ADC的DMA支持
		ADC_DMACmd(ADC1,ENABLE);
		// Enable ADC1 reset calibration register  
		ADC_ResetCalibration(ADC1);
		// Check the end of ADC1 reset calibration register 
		while(ADC_GetResetCalibrationStatus(ADC1));
		// Start ADC1 calibration 
		ADC_StartCalibration(ADC1);
		// Check the end of ADC1 calibration 
		while(ADC_GetCalibrationStatus(ADC1));
		//启动ADC1的DMA软件转换功能
		ADC_SoftwareStartConvCmd(ADC1,ENABLE);

		//----------------------------- DMA1 configuration ------------------------------------------------
		//   NVIC_InitTypeDef  NVIC_InitStructure;
		//   //配置ADC1的DMA1接收通道
		//	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
		//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		//	NVIC_Init(&NVIC_InitStructure); 

		// DMA1 channel1 configuration
		DMA_DeInit(DMA1_Channel1);
		DMA1_InitStructure.DMA_PeripheralBaseAddr = (u32)&ADC1->DR;//ADC1_DR_Address;
		DMA1_InitStructure.DMA_MemoryBaseAddr 		= (uint32_t) ADC1_ConvertedBuf;
		DMA1_InitStructure.DMA_DIR 								= DMA_DIR_PeripheralSRC;
		DMA1_InitStructure.DMA_BufferSize 				= 4;
		DMA1_InitStructure.DMA_PeripheralInc 			= DMA_PeripheralInc_Disable;
		DMA1_InitStructure.DMA_MemoryInc 					= DMA_MemoryInc_Enable;
		DMA1_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
		DMA1_InitStructure.DMA_MemoryDataSize 		= DMA_MemoryDataSize_HalfWord;
		DMA1_InitStructure.DMA_Mode 							= DMA_Mode_Circular;
		DMA1_InitStructure.DMA_Priority 					= DMA_Priority_High;
		DMA1_InitStructure.DMA_M2M 								= DMA_M2M_Disable;
		DMA_Init(DMA1_Channel1	, 	&DMA1_InitStructure);
		DMA_Cmd(DMA1_Channel1		,		ENABLE);
		//  //开启接收中断
		//  DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);		
}

void CameraLense_Init(void)
{
    GPIO_CameraLense_Configuration();
    TIM4CH3_PWM_Init();//电机驱动芯片使能端PWM控制
    ADC_CameraLense_Init();
}



extern void Camera_ZoomWide(void);
extern void Camera_ZoomTele(void);
extern void Camera_Zoom_stop(void);
extern void Camera_FocusFar(void);
extern void Camera_FocusNear(void);
extern void Camera_Focus_stop(void);

extern RETURN_VALUE_T returnValue;
//extern INQUIRE_VALUE_T inquireValue;
void CameraLense_Ctrl(CameraStruct_TypeDef *camera_ctrl)
{
#ifdef IPC_EN
    if(camera_ctrl->ZoomWide_Flag)
    {
//        returnValue.cameraZoomFlag = 1;
//        inquireValue.cameraZoomFlag = 1;
        pLaser->SetTouchOff(1);
        Camera_ZoomWide();
        delay_ms(100);
    }
    else if(camera_ctrl->ZoomTele_Flag)
    {
//        returnValue.cameraZoomFlag = 1;
//        inquireValue.cameraZoomFlag = 1;
        pLaser->SetTouchOff(1);
        Camera_ZoomTele();
        delay_ms(100);
    }
    else if(camera_ctrl->FocusFar_Flag)
    {
        
        Camera_FocusFar(); 
        delay_ms(100);
    }
    else if(camera_ctrl->FocusNear_Flag)
    {
        
        Camera_FocusNear(); 
        delay_ms(100);
    }
    else
    {
//        returnValue.cameraZoomFlag = 1;
//        inquireValue.cameraZoomFlag = 1;
        Camera_Zoom_stop();
        Camera_Focus_stop();
        pLaser->SetTouchOff(0);
        delay_ms(100);
    }
#else
    if(camera_ctrl->ZoomWide_Flag)
    {
//        returnValue.cameraZoomFlag = 1;
//        inquireValue.cameraZoomFlag = 1;
        LENS->Camera->Wide();
        pLaser->SetTouchOff(1);
    }
    else if(camera_ctrl->ZoomTele_Flag)
    {
//        returnValue.cameraZoomFlag = 1;
//        inquireValue.cameraZoomFlag = 1;
        LENS->Camera->Tele();
        pLaser->SetTouchOff(1);
    }
    else if(camera_ctrl->FocusFar_Flag)
    {
        LENS->Camera->Far();            
    }
    else if(camera_ctrl->FocusNear_Flag)
    {
        LENS->Camera->Near();            
    }
    else
    {
//        returnValue.cameraZoomFlag = 1;
//        inquireValue.cameraZoomFlag = 1;
        LENS->Camera->ZoomStop();
        LENS->Camera->FocusStop();
        pLaser->SetTouchOff(0);
    }
#endif
#ifdef LASER_EN
    static char laserFlag = 0;
    if(camera_ctrl->IrisOpen_Flag)
    {
        pLaser->SetTouchOff(0);
        pLaser->Big();
        laserFlag = 1; 
    }
    else if(camera_ctrl->IrisClose_Flag)
    {
        pLaser->SetTouchOff(0);
        pLaser->Small();
        laserFlag = 1;
    }
    else
    {
        if(laserFlag == 1)
        {
            pLaser->SetTouchOff(0);
            laserFlag = 0;
            pLaser->Stop();
        }
    }
#endif
}

unsigned short filter1_fine_CAMZOOM()
{
	char count;
	int ADC_Sum1=0;
	unsigned short i;
	unsigned short ADC_Buffer1_Temp[16] = {0};
	for(count=0;count<16;count++)
	{
		ADC_Buffer1_Temp[count] = LENS->Camera->ReadZoomValue();
		ADC_Sum1 += ADC_Buffer1_Temp[count];
		for(i = 0;i < 0x1FF;i ++);
	}
	return (unsigned short)(ADC_Sum1 >> 4);	
}




char AsserPara(unsigned short val)
{
	if(val > 5000)
	{
		return 1;
	}
	if(val < 15)
	{
		return 1;
	}
	return 0;
}
 void Go_ZoomPosition(CameraStruct_TypeDef *camera_ctrl)
{

	
    if(camera_ctrl->ZoomSet_Flag)
    {
			  
		camera_ctrl->ZoomCurPosition = filter1_fine_CAMZOOM();
		if(AsserPara(camera_ctrl->ZoomCurPosition) == 1)
		{
			return ;
		}
		CamZoom_time_cnt++;
		if(CamZoom_time_cnt >= 0x4FF)
		{   
			CamZoom_time_cnt=0;
			
			if((CamZoomCurPosPre>=(camera_ctrl->ZoomCurPosition-5)) && (CamZoomCurPosPre<=(camera_ctrl->ZoomCurPosition+5)))
			{					
				camera_ctrl->ZoomCurBigerSet_flag=0;
				camera_ctrl->ZoomCurSmallerSet_flag=0;
				Zoom_Count_Cam=0;
				camera_ctrl->ZoomSet_Flag = 0;
				LENS->Camera->ZoomStop();
				return ;						
			}
			else
			{
				CamZoomCurPosPre=camera_ctrl->ZoomCurPosition;						  
			}
		}
			  
		if((camera_ctrl->ZoomCurPosition > (camera_ctrl->ZoomSetPosition + 100))
		||(camera_ctrl->ZoomCurPosition < (camera_ctrl->ZoomSetPosition - 100)))
		{
			Zoom_Count_Cam=0;
			
			if(camera_ctrl->ZoomCurPosition > (camera_ctrl->ZoomSetPosition + 100))
			{
				if(camera_ctrl->WideInc_flag)
				{
					LENS->Camera->Tele();
				}
				else
				{
					LENS->Camera->Wide();
				}
			}
			else if(camera_ctrl->ZoomCurPosition < (camera_ctrl->ZoomSetPosition - 100))
			{
				if(camera_ctrl->WideInc_flag)
				{
					LENS->Camera->Wide();
				}
				else
				{
					LENS->Camera->Tele();
				}
			}
		}
		else 
		{
			if(!CameraLense_ZoomStop_Flag)  //TIM5定时器50ms清零
			{
				Zoom_Count_Cam++;
				if(Zoom_Count_Cam>=30) //50
				{
					camera_ctrl->ZoomCurSmallerSet_flag=1;
					camera_ctrl->ZoomCurBigerSet_flag=1;
				}
				
				if(camera_ctrl->ZoomCurPosition > (camera_ctrl->ZoomSetPosition + 30))   //10
				{
					camera_ctrl->ZoomCurBigerSet_flag=1;
					if(camera_ctrl->WideInc_flag)
					{
                        LENS->Camera->Tele();
					}
					else
					{
                        LENS->Camera->Wide();
					}
					delay_ms(20);
                    LENS->Camera->ZoomStop();
					CameraLense_ZoomStop_Flag=1;
				}
				else if(camera_ctrl->ZoomCurPosition < (camera_ctrl->ZoomSetPosition - 30)) //10
				{
					camera_ctrl->ZoomCurSmallerSet_flag=1;
					if(camera_ctrl->WideInc_flag)
					{
						LENS->Camera->Wide();
					}
					else
					{
						LENS->Camera->Tele();
					}
					delay_ms(20);
					LENS->Camera->ZoomStop();
					CameraLense_ZoomStop_Flag=1;
				}
				else
				{
					camera_ctrl->ZoomCurSmallerSet_flag=1;
					camera_ctrl->ZoomCurBigerSet_flag=1;
				}
				if(camera_ctrl->ZoomCurBigerSet_flag && camera_ctrl->ZoomCurSmallerSet_flag)
				{
					camera_ctrl->ZoomCurBigerSet_flag=0;
					camera_ctrl->ZoomCurSmallerSet_flag=0;
					Zoom_Count_Cam=0;
					camera_ctrl->ZoomSet_Flag = 0;
					LENS->Camera->ZoomStop();
                    if(camAutoFocusFlag == 1)
                    {
                        CamAutoFocusOpenNoSave();
                        delay_ms(100);
                    }
                    CamAutoFocusOnce();
				}
			}
	  }
		
    }
}
void Go_FocusPosition(CameraStruct_TypeDef *camera_ctrl)
{	
    if(camera_ctrl->FocusSet_Flag)
    {			
        if(FocusSet_Correct > 0)
        {
            if(init_fc_value)
            {
                camera_ctrl->FocusCurPosition = LENS->Camera->ReadFocusValue();
                if(camera_ctrl->FocusCurPosition > FocusSetPosition2)
                {
                    if(camera_ctrl->NearInc_flag)
                    {
                        LENS->Camera->Far();
                    }
                    else
                    {
                        LENS->Camera->Near();
                    }
                    if(!camera_ctrl->FocusCurBigerSet_flag)
                    {			
                        camera_ctrl->FocusCurBigerSet_flag=1;						
                        if(camera_ctrl->FCSaveFlag != camera_ctrl->FocusFar_RunFlag)
                        {
                            FocusSetPosition2=camera_ctrl->FocusSetPosition-64;//FocusSet_Correct*4;
                        }
                        else
                        {
                            init_fc_value	=	0;
                            LENS->Camera->FocusStop();
                            camera_ctrl->FocusCurBigerSet_flag		=	0;
                            camera_ctrl->FocusCurSmallerSet_flag	=	0;
                        }
                    }	
                }

                if(camera_ctrl->FocusCurPosition < FocusSetPosition2)
                {
                    if(camera_ctrl->NearInc_flag)
                    {
                        LENS->Camera->Near();
                    }
                    else
                    {
                        LENS->Camera->Far();
                    }
                    if(!camera_ctrl->FocusCurSmallerSet_flag)
                    {
                        camera_ctrl->FocusCurSmallerSet_flag=1;
                        if(camera_ctrl->FCSaveFlag != camera_ctrl->FocusFar_RunFlag)
                        {
                            FocusSetPosition2=camera_ctrl->FocusSetPosition+64;//FocusSet_Correct*4;	
                        }
                        else
                        {								
                            init_fc_value	=	0;
                            LENS->Camera->FocusStop();                                                    
                            camera_ctrl->FocusCurBigerSet_flag		=	0;
                            camera_ctrl->FocusCurSmallerSet_flag	=	0;
                        }
                    }						
                }
            }

            else
            {
		
                camera_ctrl->FocusCurPosition =LENS->Camera->ReadFocusValue();

                if(camera_ctrl->FocusCurPosition > camera_ctrl->FocusSetPosition)
                {
                    if(!camera_ctrl->FocusCurBigerSet_flag)
                    {
                            camera_ctrl->FocusSetPosition=camera_ctrl->FocusSetPosition-FocusSet_Correct;
                    }
                    camera_ctrl->FocusCurBigerSet_flag=1;

                    if(camera_ctrl->NearInc_flag)
                    {
                        LENS->Camera->Far();
                    }
                    else
                    {
                        LENS->Camera->Near();
                    }
                }
                if(camera_ctrl->FocusCurPosition < camera_ctrl->FocusSetPosition)
                {
                    if(!camera_ctrl->FocusCurSmallerSet_flag)
                    {
                            camera_ctrl->FocusSetPosition=camera_ctrl->FocusSetPosition+FocusSet_Correct;	//+
                    }
                    camera_ctrl->FocusCurSmallerSet_flag=1;
                    if(camera_ctrl->NearInc_flag)
                    {
                        LENS->Camera->Near();
                    }
                    else
                    {
                        LENS->Camera->Far();
                    }
                }
                if(camera_ctrl->FocusCurBigerSet_flag && camera_ctrl->FocusCurSmallerSet_flag)
                {
                    LENS->Camera->FocusStop();                                    
                    camera_ctrl->FocusCurBigerSet_flag		=	0;
                    camera_ctrl->FocusCurSmallerSet_flag	=	0;
                    camera_ctrl->FocusSet_Flag 						= 0;		
                }
            }
        }
        else
        {
		
            camera_ctrl->FocusCurPosition = LENS->Camera->ReadFocusValue();

            if(camera_ctrl->FocusCurPosition > camera_ctrl->FocusSetPosition)
            {
                if(!camera_ctrl->FocusCurBigerSet_flag)
                {
                        camera_ctrl->FocusSetPosition=camera_ctrl->FocusSetPosition-0;
                }
                camera_ctrl->FocusCurBigerSet_flag=1;

                if(camera_ctrl->NearInc_flag)
                {
                    LENS->Camera->Far();
                }
                else
                {
                    LENS->Camera->Near();
                }
            }

            if(camera_ctrl->FocusCurPosition < camera_ctrl->FocusSetPosition)
            {
                if(!camera_ctrl->FocusCurSmallerSet_flag)
                {
                        camera_ctrl->FocusSetPosition=camera_ctrl->FocusSetPosition+0;	//+
                }
                camera_ctrl->FocusCurSmallerSet_flag=1;
                if(camera_ctrl->NearInc_flag)
                {
                    LENS->Camera->Near();
                }
                else
                {
                    LENS->Camera->Far();
                }
            }
            if(camera_ctrl->FocusCurBigerSet_flag && camera_ctrl->FocusCurSmallerSet_flag)
            {
                LENS->Camera->FocusStop();                            
                camera_ctrl->FocusCurBigerSet_flag		=	0;
                camera_ctrl->FocusCurSmallerSet_flag	=	0;
                camera_ctrl->FocusSet_Flag 				= 0;	
            }
        }
    }	
}

extern int VC_UART_SendData(void*buf,int len);

void CamFocusLoc(CameraStruct_TypeDef *camera_ctrl)
{
    if((camera_ctrl->ZoomSet_Flag == 0) && camera_ctrl->FocusSet_Flag)
    {
        int i=0;
        while(camera_ctrl->FocusSet_Flag && i < 0x3FFFFF)
        {
            Go_FocusPosition(camera_ctrl);
            i++;
            if(TVC_UART_ProtoFlag)
            break;
        }
        if(camera_ctrl->FocusSet_Flag)
        {
            LENS->Camera->FocusStop();
            camera_ctrl->FocusSet_Flag = 0;
        }
    }
}

void CamLocPos(CameraStruct_TypeDef *camera_ctrl)
{
    if(!PTStruct.PanAngleSet_Flag&&!PTStruct.TiltAngleSet_Flag&&!PTStruct.Pan_PresetSpeedSet_Flag&&!PTStruct.Tilt_PresetSpeedSet_Flag)
    {
        Go_ZoomPosition(camera_ctrl);
    }
}

void SelfDelay(unsigned char sec)
{
    unsigned char i = 0;
    for(i = 0;i< sec;i ++)
    {
        delay_s(1);
    }
}

unsigned char camera_lense_selftest(CameraStruct_TypeDef *camera_ctrl)
{		
    unsigned int value1,value2;
    
    LENS->Camera->Tele(); //长焦方向
    SelfDelay(20);
    LENS->Camera->ZoomStop();
    delay_ms(500);
    value2 = LENS->Camera->ReadZoomValue();
    
    LENS->Camera->Wide();  //广角方向
    SelfDelay(20);
    LENS->Camera->ZoomStop();        
    delay_ms(500);
    value1 = LENS->Camera->ReadZoomValue();
   
    if(value1 > value2)  // 广角AD > 长焦 AD   可见光镜头一般无需校正，XS201911397校正，怀疑镜头更改AD
    {
        alg->CorrCamAdValue(1,value2);
        CameraStruct.WideInc_flag = 1;
    }
    else 
    {
        alg->CorrCamAdValue(0,value2);
    }
    #ifdef LASER_EN
        pLaser->CorrectionCameraAD(value2);
    #endif
 
    
    return 0;
}


void IrLocPos(CameraStruct_TypeDef *camera_ctrl)
{
    if((camera_ctrl->ZoomSet_Flag == 0)&&(camera_ctrl->FocusSet_Flag == 0))
    {
        return ;
    }
        
    if(!PTStruct.PanAngleSet_Flag&&!PTStruct.TiltAngleSet_Flag&& \
        !PTStruct.Pan_PresetSpeedSet_Flag&&!PTStruct.Tilt_PresetSpeedSet_Flag&&!CameraStruct.ZoomSet_Flag)
    {
        #ifdef IR_ZOOM_EN
        #else
            setIRZOOM_FOCUS(camera_ctrl->ZoomSetPosition, camera_ctrl->FocusSetPosition);
        #endif
        camera_ctrl->ZoomSet_Flag=0;
        camera_ctrl->FocusSet_Flag=0;
    }
    
}


