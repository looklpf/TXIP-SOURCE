#include "WatchHome.h"
#include "SysCfg.h"
#include "ComCmd_proc.h"
#include "Preset.h"
#include "timer.h"
#include "delay.h"
#include "Curise.h"
#include "AppleScan.h"
#include "LineScan.h"

WATCH_HOME_FUN_T watchHomeFun;

void WatchHomeSetType(unsigned char type)
{
    if(type > 4)
    {
        return;
    }
    sysPara.watchHome.type = type;
    SysParaSave();
}

void WatchHomeInit()
{
    watchHomeFun.cnt = 0;
    watchHomeFun.actionFlag = 1;
    watchHomeFun.actionTime = 300;
}

void WatchHomeTimeCnt()
{
    if (sysPara.watchHome.type == 0)
    {
        return;
    }  
    if(CuriseGetStatus() != 0 )
    {
        return ;
    }
    if(LineScanGetStatus() != 0)
    {
        return;
    }
    if(AppleScanGetStatus() != 0)
    {
        return ;
    }
    if(watchHomeFun.actionFlag == 0)
    {
        return ;
    }
    watchHomeFun.cnt ++;
    if(watchHomeFun.cnt > 0xffff)
    {
        watchHomeFun.cnt = 0;
    }

}

void WatchHomeFun(void)
{
    if(watchHomeFun.cnt < watchHomeFun.actionTime)
    {
        return ;
    }
    watchHomeFun.cnt = 0;
    switch (sysPara.watchHome.type)
    {
        case 1:  
            call_preset(1,0);
            break;
        case 2:   
            CuriseEnable();
            break;
        case 3:  
            LineScanEnable();
            break;
        case 4:  
            AppleScanEnable();
            break;
        default:
            break;
    }
    watchHomeFun.actionFlag = 0;
}


