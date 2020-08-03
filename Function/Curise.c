#include "Curise.h"
#include "SysCfg.h"
#include "ComCmd_proc.h"
#include "Preset.h"
#include "timer.h"
#include "delay.h"


CURISE_FUN_T curiseFun ;



unsigned int CuriseGetDwellTime()
{
    return (sysPara.curise.minute*60 + sysPara.curise.second);
}

void CuriseSetDwellTime(char type,char val)
{
    char flag = 0;
    switch (type)
    {
        case 0:
            sysPara.curise.minute = val - 1;
            if(sysPara.curise.minute > 59)
            {
                sysPara.curise.minute = 59;
            }
            flag = 1;
            break;
        case 1:
            
            sysPara.curise.second = val - 1;
            if(sysPara.curise.second > 59)
            {
                sysPara.curise.second = 59;
            }
            if(sysPara.curise.minute == 0)
            {
                if(sysPara.curise.second < 10)
                {
                    sysPara.curise.second = 10;
                }
            }
            flag = 1;
            break;
        default:
            break;
    }
    if(flag == 1)
    {
        SysParaSave();
    }
    
}


void CuriseEnable()
{
    curiseFun.enable    = 1;
    curiseFun.cnt       = 0;
    curiseFun.presetNum = 1;
    curiseFun.timeReach = 1;
}

void CruiseInit()
{
    curiseFun.enable    = 0;
    curiseFun.cnt       = 0;
    curiseFun.presetNum = 1;
    curiseFun.timeReach = 1;
}

unsigned char CuriseGetStatus()
{
    return curiseFun.enable;
}
void CuriseTimeCnt()
{
    if(curiseFun.enable == 0)
    {
        return;
    }
    if(SysStatusCheck() != 0)
    {
        return;
    }
    curiseFun.cnt++;
    if(curiseFun.cnt == CuriseGetDwellTime())    
    {
        curiseFun.cnt       = 0;
        curiseFun.timeReach = 1;
    }
    
}
void CuriseFun(void)
{
    if(curiseFun.enable == 0)
    {
        return ;
    }
    char flag ;
    if (curiseFun.timeReach == 1)
    {
        
        flag = call_preset(curiseFun.presetNum, sysPara.curise.speed); 
        curiseFun.presetNum ++;
        if(curiseFun.presetNum > 20)
        {
            curiseFun.presetNum =  1;
        }   
        if(flag == 0)
        {
            curiseFun.timeReach = 0 ; 
        }            
    }
    
}


