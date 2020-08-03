#include "SysCfg.h"
#include "eeprom_i2c.h"
#include "stm32f10x.h"	   
#include <stdio.h>
#include "stdlib.h"
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
#include "stm32f10x_iwdg.h"
#include "VC_uart.h"
#include "LensDriver.h"
#include "Algorithm.h"
#include "LaserFunction.h"
#include "WatchHome.h"
#include "Curise.h"
#include "LineScan.h"
#include "AppleScan.h"
#include "M_3d.h"
SYS_PARA_T sysPara = 
{
    .camAddr        =   1,
    .irAddr         =   2,
    .camBaud        =   2400,
    .irBaud         =   2400,
    .returnCnt      =   0,
    .brush          =   0,
    .fog            =   0,
    .camReturnType  =   0,
    .sysTest        =   0,
};

void SysParaDefaultInit()
{
    
    sysPara.eepromEnable        =   0;
    sysPara.ptReturn            =   1;
    sysPara.viewReturn          =   1;
    sysPara.returnFreq          =   3;
    sysPara.autoSpeed           =   0;
    sysPara.powerRemember       =   0;
    sysPara.alarmStop           =   0;
    sysPara.camFocusSpeed       =   10;
    
    sysPara.curise.minute       =   0;
    sysPara.curise.second       =   30;  
    sysPara.curise.speed        =   20;
    
    sysPara.watchHome.type      =   0;
    
    sysPara.appleScan.stepAngle =   20;
    
    sysPara.zeroPos.hor         =   0;
    sysPara.zeroPos.ver         =   0;

}
void SysParaSave()
{
    unsigned char buf[20];
    buf[0]  = sysPara.eepromEnable;
    buf[1]  = sysPara.ptReturn;
    buf[2]  = sysPara.viewReturn;
    buf[3]  = sysPara.returnFreq;
    buf[4]  = sysPara.autoSpeed;
    buf[5]  = sysPara.powerRemember;
    buf[6]  = sysPara.alarmStop;
    buf[7]  = sysPara.camFocusSpeed;
    
    buf[8]  = sysPara.curise.minute;
    buf[9]  = sysPara.curise.second;
    buf[10] = sysPara.curise.speed;

    buf[11] = sysPara.watchHome.type;
    
    buf[12] = sysPara.appleScan.stepAngle;
    
    buf[13] = (sysPara.zeroPos.hor>>8)&0xff;
    buf[14] = (sysPara.zeroPos.hor>>0)&0xff;
    buf[15] = (sysPara.zeroPos.ver>>8)&0xff;
    buf[16] = (sysPara.zeroPos.ver>>0)&0xff;
    EEPROM_I2C_WriteBuffer(EEPROM_SYSINFO_ADDR,buf,17);
}
void SysParaRead()
{
    unsigned char buf[20];
    EEPROM_I2C_ReadBuffer(EEPROM_SYSINFO_ADDR, buf, 17);
    
    sysPara.eepromEnable        =   buf[0];
    sysPara.ptReturn            =   buf[1];
    sysPara.viewReturn          =   buf[2];
    sysPara.returnFreq          =   buf[3];
    sysPara.autoSpeed           =   buf[4];
    sysPara.powerRemember       =   buf[5];
    sysPara.alarmStop           =   buf[6];
    sysPara.camFocusSpeed       =   buf[7];
    
    sysPara.curise.minute       =   buf[8];
    sysPara.curise.second       =   buf[9];
    sysPara.curise.speed        =   buf[10];
    
    sysPara.watchHome.type      =   buf[11];
    
    sysPara.appleScan.stepAngle =   buf[12];
    
    sysPara.zeroPos.hor         =   (buf[13]<<8)|buf[14];
    sysPara.zeroPos.ver         =   (buf[15]<<8)|buf[16];
}
void SysParaInit()
{
    unsigned char buf[1];
    EEPROM_I2C_ReadBuffer(EEPROM_SYSINFO_ADDR, buf, 1);
    sysPara.eepromEnable = buf[0];
    if(sysPara.eepromEnable == 0xff)
    {
        SysParaDefaultInit();
        SysParaSave();
    }
    else 
    {
        SysParaRead();
    }
    
}



char SysStatusCheck()   //设备忙返回1  设备空闲返回0
{
    if(PTStruct.PanAngleSet_Flag)
    {
        return 1;
    }
    if(PTStruct.TiltAngleSet_Flag)
    {
        return 1;
    }
    if(PTStruct.Pan_PresetSpeedSet_Flag)
    {
        return 1;
    }
    if(PTStruct.Tilt_PresetSpeedSet_Flag)
    {
        return 1;
    }
    if(CameraStruct.ZoomSet_Flag)
    {
        return 1;
    }
    if(IRStruct.ZoomSet_Flag)
    {
        return 1;
    }
    return 0;
}


void IWDG_Configuration(void)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_256);
    IWDG_SetReload(4062);
    IWDG_ReloadCounter();
    IWDG_Enable();
}
void CamLensColor()
{
    GPIO_SetBits(GPIOE,GPIO_Pin_9); 
}
void CamLensBlack()
{
    GPIO_ResetBits(GPIOE,GPIO_Pin_9); 
}
void FogOn() //HDC208_HD60X12750DIR透雾控制
{
    CamLensBlack();
    GPIO_SetBits(GPIOE,GPIO_Pin_4);  
    GPIO_ResetBits(GPIOE,GPIO_Pin_5);  
    delay_s(1);
    delay_s(1);
    delay_s(1);
    GPIO_ResetBits(GPIOE,GPIO_Pin_4);  
    GPIO_ResetBits(GPIOE,GPIO_Pin_5); 
}
void FogOff() //HDC208_HD60X12750DIR透雾控制
{
    CamLensColor();
    GPIO_SetBits(GPIOE,GPIO_Pin_5);  
    GPIO_ResetBits(GPIOE,GPIO_Pin_4);  
    delay_s(1); 
    delay_s(1);
    delay_s(1);    
    GPIO_ResetBits(GPIOE,GPIO_Pin_4);  
    GPIO_ResetBits(GPIOE,GPIO_Pin_5); 
}
void BrushOn()
{
    GPIO_ResetBits(GPIOE,GPIO_Pin_8);  

}
void BrushOff()
{
    GPIO_SetBits(GPIOE,GPIO_Pin_8);  
}

void GpioInit()
{
    GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);//heart
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;          
	GPIO_Init(GPIOD, &GPIO_InitStructure);	
    GPIO_SetBits(GPIOD,GPIO_Pin_15);  
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);//fog brush
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_8|GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;          
	GPIO_Init(GPIOE, &GPIO_InitStructure);	
    BrushOff();
    FogOff();

    
}

void SysInit()
{
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x2000);
    delay_init(72); 	
    EEPROM_I2C_Init();
//    IR_Init();
    time2_init();
    time3_init();
    TIM5_NVIC_Configuration();
    TIM5_Configuration();
    START_TIME5;
    CameraLense_Init();
    time4_init();
    
    LENS->init();
    GpioInit();
    VC_UART_Init(sysPara.camBaud);
    TVC_UART_Init(sysPara.irBaud);
    PT_UART_Init(9600);
    IR_UART_Init(19200); 
    TTL_UART_Init(9600);
    Set_Camera_FocusSpeed(80);
    SysParaInit();
    Loc3DCorrParaRead();
}
void SysCheck()
{   
#ifdef IPC_EN
#else
    camera_lense_selftest(&CameraStruct);
#endif
}



void SysFunInit()
{
    CruiseInit();
    LineScanFunInit();
    AppleScanInit();
    WatchHomeInit();
    CamReadAutoFocus();
}







