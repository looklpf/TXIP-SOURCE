#include "LensDriver.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
//#include "DevelopMenu.h"
#include "hp_conf.h"
#include "eeprom_i2c.h"
#include "IR_uart.h"
#ifndef BOARD_OLD_EN
static void GpioInit()
{
    GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;          
	GPIO_Init(GPIOD, &GPIO_InitStructure);	
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;          
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
}


static volatile unsigned short adc1ConvertedBuf[5] 	= {0,0,0,};
static void ADCInit(void)
{
    GPIO_InitTypeDef CameraLense_ADC_GPIO_InitStructure;
    ADC_InitTypeDef  ADC1_InitStructure;
    DMA_InitTypeDef  DMA1_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOB, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);	
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);

    CameraLense_ADC_GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_2|GPIO_Pin_4|GPIO_Pin_5;
    CameraLense_ADC_GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AIN;	 
    GPIO_Init(GPIOC, &CameraLense_ADC_GPIO_InitStructure);
    
    CameraLense_ADC_GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_0|GPIO_Pin_1;
    CameraLense_ADC_GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AIN;	 
    GPIO_Init(GPIOB, &CameraLense_ADC_GPIO_InitStructure);
    

    ADC1_InitStructure.ADC_Mode 				    = ADC_Mode_Independent;				
    ADC1_InitStructure.ADC_ScanConvMode 			= ENABLE;		 									
    ADC1_InitStructure.ADC_ContinuousConvMode       = ENABLE;  										
    ADC1_InitStructure.ADC_ExternalTrigConv 	    = ADC_ExternalTrigConv_None; 	
    ADC1_InitStructure.ADC_DataAlign 				= ADC_DataAlign_Right;				
    ADC1_InitStructure.ADC_NbrOfChannel 			= 5; 													
    ADC_Init(ADC1, &ADC1_InitStructure);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 1, ADC_SampleTime_239Cycles5);  
    ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 2, ADC_SampleTime_239Cycles5);	  
    ADC_RegularChannelConfig(ADC1, ADC_Channel_8 , 3, ADC_SampleTime_239Cycles5);  
    ADC_RegularChannelConfig(ADC1, ADC_Channel_9 , 4, ADC_SampleTime_239Cycles5);	
    ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 5, ADC_SampleTime_239Cycles5);
    ADC_Cmd(ADC1, ENABLE);

    ADC_DMACmd(ADC1,ENABLE);

    ADC_ResetCalibration(ADC1);

    while(ADC_GetResetCalibrationStatus(ADC1));

    ADC_StartCalibration(ADC1);

    while(ADC_GetCalibrationStatus(ADC1));

    ADC_SoftwareStartConvCmd(ADC1,ENABLE);


    DMA_DeInit(DMA1_Channel1);
    DMA1_InitStructure.DMA_PeripheralBaseAddr   = (u32)&ADC1->DR;
    DMA1_InitStructure.DMA_MemoryBaseAddr 		= (uint32_t) adc1ConvertedBuf;
    DMA1_InitStructure.DMA_DIR 					= DMA_DIR_PeripheralSRC;
    DMA1_InitStructure.DMA_BufferSize 			= 5;
    DMA1_InitStructure.DMA_PeripheralInc 		= DMA_PeripheralInc_Disable;
    DMA1_InitStructure.DMA_MemoryInc 			= DMA_MemoryInc_Enable;
    DMA1_InitStructure.DMA_PeripheralDataSize   = DMA_PeripheralDataSize_HalfWord;
    DMA1_InitStructure.DMA_MemoryDataSize 		= DMA_MemoryDataSize_HalfWord;
    DMA1_InitStructure.DMA_Mode 				= DMA_Mode_Circular;
    DMA1_InitStructure.DMA_Priority 			= DMA_Priority_High;
    DMA1_InitStructure.DMA_M2M 					= DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1,&DMA1_InitStructure);
    DMA_Cmd(DMA1_Channel1,ENABLE);

}




static void LensInit()
{
    GpioInit();
    ADCInit();
}



static void CameraTele()
{
    GPIO_ResetBits(GPIOA,GPIO_Pin_7);
    GPIO_SetBits(GPIOA,GPIO_Pin_6);
    
}
static void CameraWide()
{
    GPIO_ResetBits(GPIOA,GPIO_Pin_6);
    GPIO_SetBits(GPIOA,GPIO_Pin_7);
}
static void CameraZoomStop()
{
    GPIO_ResetBits(GPIOA,GPIO_Pin_6);
    GPIO_ResetBits(GPIOA,GPIO_Pin_7);
}
char camFarFlag = 0;
char camNearFlag = 0;
char camFocusStopFlag = 0;
char camFocusPwmCnt = 0;
//unsigned char camFocusPwmSpeed = 10;

void CamFarMove()
{
    GPIO_ResetBits(GPIOA,GPIO_Pin_4);
    GPIO_SetBits(GPIOA,GPIO_Pin_5);
}
void CamNearMove()
{
    GPIO_ResetBits(GPIOA,GPIO_Pin_5);
    GPIO_SetBits(GPIOA,GPIO_Pin_4);
}
void CamFocusStop()
{
    GPIO_ResetBits(GPIOA,GPIO_Pin_4);
    GPIO_ResetBits(GPIOA,GPIO_Pin_5);
}
static void CameraFar()
{
    camFarFlag = 1;
    camFocusPwmCnt = 0;
}
static void CameraNear()
{
    camNearFlag = 1;
    camFocusPwmCnt = 0;
}
static void CameraFocusStop()
{
    camFarFlag = 0;
    camNearFlag = 0;
    CamFocusStop();
}
void CamSetFocusSpeed(unsigned char val)
{
    unsigned char buf[1];
    if((val > 0)&&(val < 11))
    {
        sysPara.camFocusSpeed = val;
    }
    else 
    {
        sysPara.camFocusSpeed = 10;
    }
    buf[0] = sysPara.camFocusSpeed;
    EEPROM_I2C_WriteBuffer(EEPROM_SYSINFO_ADDR+7,buf,1);
    
}

void CamFocusPwmCnt()
{
    if(camFarFlag == 1)
    {
        if(camFocusPwmCnt <= sysPara.camFocusSpeed)
        {
            CamFarMove();   
        }
        else 
        {
            CamFocusStop();
        }
        
        camFocusPwmCnt ++ ;
        if(camFocusPwmCnt > 20)
        {
            camFocusPwmCnt = 0;
        }
    }
    else if(camNearFlag == 1)
    {
        if(camFocusPwmCnt <= sysPara.camFocusSpeed)
        {
            CamNearMove();   
        }
        else 
        {
            CamFocusStop();
        }
        camFocusPwmCnt ++ ;
        if(camFocusPwmCnt > 20)
        {
            camFocusPwmCnt = 0;
        }
    }

}

static void IRWide()
{

    command_zoomwide();
    
    
}
static void IRTele()
{

    command_zoomtele();
    
    
}
static void IRZoomStop()
{

    command_zoomstop();
 
 
    
}

static void IRFar()
{

    command_FocusFar();
    
    
}
static void IRNear()
{

    command_FocusNear();
    
    
}
static void IRFocusStop()
{

    command_FocusStop();
    
    
}

static unsigned short ReadCameraZoomValue()
{
    return adc1ConvertedBuf[0];
}
static unsigned short ReadCameraFocusValue()
{
    return adc1ConvertedBuf[1];
}

static unsigned short ReadIRZoomValue()
{
    return adc1ConvertedBuf[2];
}
static unsigned short ReadIRFocusValue()
{
    return adc1ConvertedBuf[3];
}
static unsigned short ReadLightSensorValue()
{
    return adc1ConvertedBuf[4];
}
#endif

#ifdef BOARD_OLD_EN
static void GpioInit()
{
    GPIO_InitTypeDef GPIO_InitStructure;

    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;          
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
}


static volatile unsigned short adc1ConvertedBuf[4] 	= {0,0,0,};
static void ADCInit(void)
{
    GPIO_InitTypeDef CameraLense_ADC_GPIO_InitStructure;
    ADC_InitTypeDef  ADC1_InitStructure;
    DMA_InitTypeDef  DMA1_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOB, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);	
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);

    CameraLense_ADC_GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
    CameraLense_ADC_GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AIN;	 
    GPIO_Init(GPIOC, &CameraLense_ADC_GPIO_InitStructure);
    
    ADC1_InitStructure.ADC_Mode 				    = ADC_Mode_Independent;				
    ADC1_InitStructure.ADC_ScanConvMode 			= ENABLE;		 									
    ADC1_InitStructure.ADC_ContinuousConvMode       = ENABLE;  										
    ADC1_InitStructure.ADC_ExternalTrigConv 	    = ADC_ExternalTrigConv_None; 	
    ADC1_InitStructure.ADC_DataAlign 				= ADC_DataAlign_Right;				
    ADC1_InitStructure.ADC_NbrOfChannel 			= 4; 													
    ADC_Init(ADC1, &ADC1_InitStructure);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 1, ADC_SampleTime_239Cycles5);  
    ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 2, ADC_SampleTime_239Cycles5);	  
    ADC_RegularChannelConfig(ADC1, ADC_Channel_13 ,3, ADC_SampleTime_239Cycles5);  
    ADC_RegularChannelConfig(ADC1, ADC_Channel_12 ,4, ADC_SampleTime_239Cycles5);	

    ADC_Cmd(ADC1, ENABLE);

    ADC_DMACmd(ADC1,ENABLE);

    ADC_ResetCalibration(ADC1);

    while(ADC_GetResetCalibrationStatus(ADC1));

    ADC_StartCalibration(ADC1);

    while(ADC_GetCalibrationStatus(ADC1));

    ADC_SoftwareStartConvCmd(ADC1,ENABLE);


    DMA_DeInit(DMA1_Channel1);
    DMA1_InitStructure.DMA_PeripheralBaseAddr   = (u32)&ADC1->DR;
    DMA1_InitStructure.DMA_MemoryBaseAddr 		= (uint32_t) adc1ConvertedBuf;
    DMA1_InitStructure.DMA_DIR 					= DMA_DIR_PeripheralSRC;
    DMA1_InitStructure.DMA_BufferSize 			= 4;
    DMA1_InitStructure.DMA_PeripheralInc 		= DMA_PeripheralInc_Disable;
    DMA1_InitStructure.DMA_MemoryInc 			= DMA_MemoryInc_Enable;
    DMA1_InitStructure.DMA_PeripheralDataSize   = DMA_PeripheralDataSize_HalfWord;
    DMA1_InitStructure.DMA_MemoryDataSize 		= DMA_MemoryDataSize_HalfWord;
    DMA1_InitStructure.DMA_Mode 				= DMA_Mode_Circular;
    DMA1_InitStructure.DMA_Priority 			= DMA_Priority_High;
    DMA1_InitStructure.DMA_M2M 					= DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1,&DMA1_InitStructure);
    DMA_Cmd(DMA1_Channel1,ENABLE);

}




static void LensInit()
{
    GpioInit();
    ADCInit();
}



static void CameraTele()
{
    GPIO_ResetBits(GPIOC,GPIO_Pin_6);
    GPIO_SetBits(GPIOC,GPIO_Pin_7);
    
}
static void CameraWide()
{
    
    GPIO_ResetBits(GPIOC,GPIO_Pin_7);
    GPIO_SetBits(GPIOC,GPIO_Pin_6);
}
static void CameraZoomStop()
{
    GPIO_ResetBits(GPIOC,GPIO_Pin_6);
    GPIO_ResetBits(GPIOC,GPIO_Pin_7);
}

static void CameraFar()
{
    GPIO_ResetBits(GPIOC,GPIO_Pin_8);
    GPIO_SetBits(GPIOC,GPIO_Pin_9);
}
static void CameraNear()
{
    GPIO_ResetBits(GPIOC,GPIO_Pin_9);
    GPIO_SetBits(GPIOC,GPIO_Pin_8);
}
static void CameraFocusStop()
{
    GPIO_ResetBits(GPIOC,GPIO_Pin_8);
    GPIO_ResetBits(GPIOC,GPIO_Pin_9);
}


static void IRWide()
{
    DevelopMenuValue_T value;
    value = DevelopMenu->GetValue();
    if(value.thermal == UWA384)
    {
        GPIO_ResetBits(GPIOA,GPIO_Pin_4);
        GPIO_SetBits(GPIOA,GPIO_Pin_5);
    }
    if((value.thermal == HP_ATC_4)||(value.thermal == HP_ATC_6))
    {
        command_zoomwide();
    }
    
}
static void IRTele()
{
    DevelopMenuValue_T value;
    value = DevelopMenu->GetValue();
    if(value.thermal == UWA384)
    {
        GPIO_ResetBits(GPIOA,GPIO_Pin_5);
        GPIO_SetBits(GPIOA,GPIO_Pin_4);
    }
    if((value.thermal == HP_ATC_4)||(value.thermal == HP_ATC_6))
    {
        command_zoomtele();
    }
    
}
static void IRZoomStop()
{
    DevelopMenuValue_T value;
    value = DevelopMenu->GetValue();
    if(value.thermal == UWA384)
    {
        GPIO_ResetBits(GPIOA,GPIO_Pin_4);
        GPIO_ResetBits(GPIOA,GPIO_Pin_5);
    }
    if((value.thermal == HP_ATC_4)||(value.thermal == HP_ATC_6))
    {
        command_zoomstop();
    }
 
    
}

static void IRFar()
{
    DevelopMenuValue_T value;
    value = DevelopMenu->GetValue();
    if(value.thermal == UWA384)
    {
        GPIO_ResetBits(GPIOA,GPIO_Pin_6);
        GPIO_SetBits(GPIOA,GPIO_Pin_7);
    }
    if((value.thermal == HP_ATC_4)||(value.thermal == HP_ATC_6))
    {
        command_FocusFar();
    }
    
}
static void IRNear()
{
    DevelopMenuValue_T value;
    value = DevelopMenu->GetValue();
    if(value.thermal == UWA384)
    {
        GPIO_ResetBits(GPIOA,GPIO_Pin_7);
        GPIO_SetBits(GPIOA,GPIO_Pin_6);
    }
    if((value.thermal == HP_ATC_4)||(value.thermal == HP_ATC_6))
    {
        command_FocusNear();
    }
    
}
static void IRFocusStop()
{
    DevelopMenuValue_T value;
    value = DevelopMenu->GetValue();
    if(value.thermal == UWA384)
    {
        GPIO_ResetBits(GPIOA,GPIO_Pin_6);
        GPIO_ResetBits(GPIOA,GPIO_Pin_7);
    }
    if((value.thermal == HP_ATC_4)||(value.thermal == HP_ATC_6))
    {
        command_FocusStop();
    }
    
}

static unsigned short ReadCameraZoomValue()
{
    return adc1ConvertedBuf[0];
}
static unsigned short ReadCameraFocusValue()
{
    return adc1ConvertedBuf[1];
}

static unsigned short ReadIRZoomValue()
{
    return adc1ConvertedBuf[2];
}
static unsigned short ReadIRFocusValue()
{
    return adc1ConvertedBuf[3];
}

#endif

Camera_T Camera_init =
{
    .Wide           = CameraWide,
    .Tele           = CameraTele,
    .ZoomStop       = CameraZoomStop,
    .Far            = CameraFar,
    .Near           = CameraNear,
    .FocusStop      = CameraFocusStop,
    .ReadZoomValue  = ReadCameraZoomValue,
    .ReadFocusValue = ReadCameraFocusValue,
};

IR_T IR_init =
{
	.Wide           = IRWide,
    .Tele           = IRTele,
    .ZoomStop       = IRZoomStop,
    .Far            = IRFar,
    .Near           = IRNear,
    .FocusStop      = IRFocusStop,
    .ReadZoomValue  = ReadIRZoomValue,
    .ReadFocusValue = ReadIRFocusValue,
    
};



LENS_T LENS_init =
{
    .init                   =   LensInit,
	.Camera                 =   &Camera_init,
    .IR                     =   &IR_init,
    .ReadLightSensorValue   =   ReadLightSensorValue,
};

LENS_T *LENS = &LENS_init;

