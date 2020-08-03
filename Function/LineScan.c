#include "LineScan.h"
#include "SysCfg.h"
#include "ComCmd_proc.h"
#include "Preset.h"
#include "timer.h"
#include "delay.h"

LINE_SCAN_FUN_T lineScanFun;

void LineScanFunInit()
{
    lineScanFun.cnt         = 0;
    lineScanFun.enable      = 0;
    lineScanFun.posFlag     = 0;
    lineScanFun.timeReach   = 0;
}
unsigned char LineScanGetStatus()
{
    return lineScanFun.enable;
}

void LineScanSetLeftPos()
{
    save_preset(92);
}
void LineScanSetRightPos()
{
    save_preset(93);
}
void LineScanEnable()
{

    CallPreset(92,0);
    lineScanFun.enable      = 1;
    lineScanFun.posFlag     = 0;
    lineScanFun.timeReach   = 0;
}


void LineScanTimeCnt()
{
    if(lineScanFun.enable == 0)
    {
        return ;
    }
    if(SysStatusCheck() != 0)
    {
        return;
    }
    lineScanFun.cnt++;
    if(lineScanFun.cnt > 2)    
    {
        lineScanFun.cnt       = 0;
        lineScanFun.timeReach = 1;
    }
    
}
void LineScanFun(void)
{
    if(lineScanFun.enable == 0)
    {
        return ;
    }
    if(lineScanFun.timeReach == 1)
    {
        lineScanFun.timeReach = 0;
        switch (lineScanFun.posFlag)
        {
            case 0:
                CallPtHorPreset(93,sysPara.curise.speed,PT_RIGHT); 
                lineScanFun.posFlag = 1;            
                break;
            case 1:
                CallPtHorPreset(92,sysPara.curise.speed,PT_LEFT);  
                lineScanFun.posFlag = 0;
                break;
        }
    }
  
    
}

