#include "stm32f10x_iwdg.h"
#include "SysCfg.h"
#include "ComCmd_proc.h"
#include "hp_conf.h"
#include "LaserFunction.h"
#include "LensDriver.h"
#include "Curise.h"
#include "LineScan.h"
#include "AppleScan.h"
#include "TTL_uart.h"
#include "WatchHome.h"
#include "delay.h"

int main(void)
{
    SysInit(); 
    SysCheck();
    IWDG_Configuration(); 

#ifdef LASER_EN
    pLaser->Init();
    pLaser->Read();
    CameraColor();
    pLaser->SeltTest();
#endif
    SysFunInit();
    while (1)
    {		  
        ComCmd_Proc();     
        PT_CallPreset_WithSpeedSet(&PTStruct);
        IrLocPos(&IRStruct);
        CuriseFun();
        LineScanFun();
        AppleScanFun();
        WatchHomeFun();
        DataReturn();
        LocPtAngle();
    #ifdef LASER_EN
        pLaser->Synchronize();
        pLaser->AutoFunction();
    #endif
    #ifdef IPC_EN
    #else
        CamLocPos(&CameraStruct);
    #endif
        IWDG_ReloadCounter();

    } 
}



