#include "AppleScan.h"
#include "SysCfg.h"
#include "ComCmd_proc.h"
#include "Preset.h"
#include "timer.h"
#include "delay.h"
#include "eeprom_i2c.h"
#include "MiscAlg.h"
APPLE_SCAN_FUN_T appleScanFun;

void AppleScanInit()
{
    appleScanFun.cnt = 0;
    appleScanFun.enable = 0;
    appleScanFun.posFlag = 0;
    appleScanFun.timeReach = 0;
    appleScanFun.stepAngle = sysPara.appleScan.stepAngle*10;
}

void AppleScanSetStepAngle(unsigned char num)
{
    sysPara.appleScan.stepAngle = num;
    appleScanFun.stepAngle = sysPara.appleScan.stepAngle*10;
    SysParaSave();
}
void AppleScanEnable()
{

    unsigned char   I2C_DataBuffer[4];
    EEPROM_I2C_ReadBuffer(95*16, I2C_DataBuffer, 4);
    delay_ms(50);
    if (I2C_DataBuffer[1] == 0xFF)
    {
        return ;
    }
    PTStruct.PanSetAngle    =   I2C_DataBuffer[1];
    PTStruct.PanSetAngle    = ((PTStruct.PanSetAngle << 8) & 0xFF00) | I2C_DataBuffer[0];
    PTStruct.TiltSetAngle   =   I2C_DataBuffer[3];
    PTStruct.TiltSetAngle   = ((PTStruct.TiltSetAngle << 8) & 0xFF00) | I2C_DataBuffer[2];
    appleScanFun.upBoard    = PTStruct.TiltSetAngle;
    CallPreset(94,0);
    appleScanFun.cnt = 0;
    appleScanFun.enable = 1;
    appleScanFun.posFlag = 0;
    appleScanFun.timeReach = 0;
    

}
unsigned char AppleScanGetStatus()
{
    return appleScanFun.enable;
}
void AppleScanSetLeftDownPos()
{
    save_preset(94);
}
void AppleScanSetRightUpPos()
{
    save_preset(95);
}
unsigned char AppleScanCheckUpBoard(unsigned short curPos,unsigned short boardPos)
{
    unsigned short curVal,boardVal;
    curVal = pMiscAlg->UnShortAbs(curPos,9000);
    boardVal = pMiscAlg->UnShortAbs(boardPos,9000);
    if(curVal > 18000)
    {
        curVal = 36000 - curVal;
    }
    if(boardVal > 18000)
    {
        boardVal = 36000 - boardVal;
    }
    if(curVal >= boardVal)
    {
        return 0; //到达
    }
    else if(pMiscAlg->UnShortAbs(curVal,boardVal) <= 100)
    {
        return 0;  //到达
    }
    else 
    {
        return 1;  //未到
    }
   
    
}

void AppleScanTimeCnt()
{
    if(appleScanFun.enable == 0)
    {
        return ;
    }
    if(SysStatusCheck() != 0)
    {
        return;
    }
    appleScanFun.cnt++;
    if(appleScanFun.cnt > 2)    
    {
        appleScanFun.cnt       = 0;
        appleScanFun.timeReach = 1;
    }
}

static void AppleScanRight()
{
    CallPtHorPreset(95,sysPara.curise.speed,PT_RIGHT);
    if(AppleScanCheckUpBoard(PTStruct.TiltCurAngle,appleScanFun.upBoard) == 0)
    {
        appleScanFun.posFlag = 4;
    }
    else 
    {
        appleScanFun.posFlag = 1;
    } 
}
static void AppleScanRightBoardUp()
{
    unsigned short endPos;
    if(PTStruct.TiltCurAngle > appleScanFun.stepAngle)
    {
        endPos = PTStruct.TiltCurAngle - appleScanFun.stepAngle;
    }
    else 
    {
        endPos = 36000 + PTStruct.TiltSetAngle - appleScanFun.stepAngle;
    }
    if(AppleScanCheckUpBoard(endPos,appleScanFun.upBoard) == 0)
    {
        CallPtVerPos(appleScanFun.upBoard,sysPara.curise.speed,PT_UP); 
    }
    else 
    {
        CallPtAbsVerPos(appleScanFun.stepAngle,sysPara.curise.speed,PT_UP);  
    }
    appleScanFun.posFlag = 2;
}

static void AppleScanLeft()
{
    CallPtHorPreset(94,sysPara.curise.speed,PT_LEFT); 
    if(AppleScanCheckUpBoard(PTStruct.TiltCurAngle,appleScanFun.upBoard) == 0)
    {
        appleScanFun.posFlag = 4;
    }
    else 
    {
        appleScanFun.posFlag = 3;
    }
}

static void AppleScanLeftBoardUp()
{
    unsigned short endPos;
    if(PTStruct.TiltCurAngle > appleScanFun.stepAngle)
    {
        endPos = PTStruct.TiltCurAngle - appleScanFun.stepAngle;
    }
    else 
    {
        endPos = 36000 + PTStruct.TiltSetAngle - appleScanFun.stepAngle;
    }
    if(AppleScanCheckUpBoard(endPos,appleScanFun.upBoard) == 0)
    {
        CallPtVerPos(appleScanFun.upBoard,sysPara.curise.speed,PT_UP); 
    }
    else 
    {
        CallPtAbsVerPos(appleScanFun.stepAngle,sysPara.curise.speed,PT_UP);  
    }
    appleScanFun.posFlag = 0;
}
void AppleScanFun(void)
{
    if(appleScanFun.enable == 0)
    {
        return ;
    }
    if(appleScanFun.timeReach == 0)
    {
        return ;
    }
    appleScanFun.timeReach = 0;   
    switch (appleScanFun.posFlag)
    {
        case 0:
            AppleScanRight();      
            break;
        case 1:
            AppleScanRightBoardUp();
            break;
        case 2:
            AppleScanLeft();
            break;
        case 3:
            AppleScanLeftBoardUp();
            break;
        case 4:
            AppleScanEnable();
            break;
    }
  
}
