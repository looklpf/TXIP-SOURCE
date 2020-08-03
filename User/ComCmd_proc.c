
#include "TVC_uart.h"
#include "VC_uart.h"
#include <stdio.h>
#include "hp_conf.h"
#include "ComCmd_proc.h"
#include "eeprom_i2c.h"
#include "delay.h"
#include "timer.h"
#include "TTL_uart.h"
#include "camera_lense.h"
#include "Algorithm.h"
#include "LensDriver.h"
#include "SysCfg.h"
#include "MiscAlg.h"
#include "PT_uart.h"
#include "LaserFunction.h"
#include "Preset.h"
#include "WatchHome.h"
#include "Curise.h"
#include "LineScan.h"
#include "AppleScan.h"
#include "msgqueue.h"
#include "M_3d.h"
unsigned char           I2C_DataBuffer[21];
PelcodStruct_TypeDef    TVC_Pelcod;
CameraStruct_TypeDef    CameraStruct;
CameraStruct_TypeDef    IRStruct;
PTStruct_TypeDef        PTStruct;

unsigned short          PanPreAngle;
unsigned short          TiltPreAngle;

unsigned short          FocusSetPosition2;

unsigned char           FocusSet_Correct;
RETURN_VALUE_T          returnValue;
char cmdComplexCnt = 0;
void clear_function_flag(void)
{
    if (TVC_Pelcod.ExtendCmd_flag && ((TVC_Pelcod.cmd2 == 0x23) || (TVC_Pelcod.cmd2 == 0x51) || (TVC_Pelcod.cmd2 == 0x53) || (TVC_Pelcod.cmd2 == 0x55) || (TVC_Pelcod.cmd2 == 0x75) || (TVC_Pelcod.cmd2 == 0x65) || ((TVC_Pelcod.cmd2 == 0x07) && ((TVC_Pelcod.data2 == 101)))))
    {
        //角度查询命令，暂停命令不影响当前运行状态。
        //0x23 海康轨迹发送命令
    }

    else
    {
        WatchHomeInit();

        if ((TVC_Pelcod.cmd2 != 0x4B) && (TVC_Pelcod.cmd2 != 0x4D))
        {
            if ((TVC_Pelcod.cmd2 != 0x4F) && (TVC_Pelcod.cmd2 != 0x57))
            {
                CameraStruct.ZoomSet_Flag = RESET;
                CameraStruct.FocusSet_Flag = RESET;
                IRStruct.ZoomSet_Flag = RESET;
                IRStruct.FocusSet_Flag = RESET;
                PTStruct.PanAngleSet_Flag = RESET;
                PTStruct.TiltAngleSet_Flag = RESET;
                PTStruct.Pan_PresetSpeedSet_Flag = RESET ;
                PTStruct.Tilt_PresetSpeedSet_Flag = RESET;
            }
        }
//        PTStruct.Pan_PresetSpeedSet_Flag = RESET ;
//        PTStruct.Tilt_PresetSpeedSet_Flag = RESET;
        if ((TVC_Pelcod.cmd1 == 0x10) && (TVC_Pelcod.cmd1 == 0x10))
        {
            return ;
        }
        if ((TVC_Pelcod.cmd1 == 0x11) && (TVC_Pelcod.cmd1 == 0x11))
        {
            return ;
        }
        CruiseInit();
        LineScanFunInit();
        AppleScanInit();
    }
    cmdComplexCnt = 0;
}
unsigned char flagH = 0,flagV = 0;
unsigned int  locHorPos,locVerPos;
unsigned char locFlag = 0;
unsigned char locSpeed;

void Pelcod_ExtendCmd_proc(PelcodStruct_TypeDef *Pelcod)
{  

    switch (Pelcod->cmd2)
    {
        case 0x07 :
            CallPresetsAnalysis(Pelcod->data2);
            break ;
        case 0x03 :
            if (cmdComplex == COMPLEX_NONE)
            {
                SetPresetsAnalysis(Pelcod->data2);
            }
            else
            {
                SetComplexPresetsAnalysis(Pelcod->data2);
            }
            break ;
        case 0x05 :
            DeletePresetsAnalysis(Pelcod->data2);
            break ;
        case 0x09 : //辅助开关 开
            if (Pelcod->data2 == 1)
            {
                if (sysPara.brush == 0)
                {
                    sysPara.brush = 1;
                    BrushOn();
                }
            }
//            else if (Pelcod->data2 == 2)
//            {
//                if (sysPara.fog == 0)
//                {
//                    sysPara.fog = 1;
//                    Set_Camera_Black_To_Laser_Board();
//                    FogOn();
//                }
//            }
            break ;
        case 0x0B ://辅助开关 关
            if (Pelcod->data2 == 1)
            {
                if (sysPara.brush == 1)
                {
                    sysPara.brush = 0;
                    BrushOff();
                }
            }
//            if (Pelcod->data2 == 2)
//            {
//                if (sysPara.fog == 1)
//                {
//                    sysPara.fog = 0;
//                    Set_Camera_Black_To_Laser_Colour();
//                    FogOff();
//                }
//            }
            break ;    
        case 0x4B://水平定位
            locHorPos = Pelcod->data1 << 8 | Pelcod->data2;
            locSpeed  = Pelcod->cmd1;
            if(locFlag == 0)
            {
                locFlag = 1;
            }
            break;
        case 0x4D://俯仰定位
            locVerPos = Pelcod->data1 << 8 | Pelcod->data2;
            locSpeed  = Pelcod->cmd1;
            if(locFlag == 1)
            {
                locFlag = 2;
            }
            break;
        case 0x4F: //视场角定位
            if (TVC_Pelcod.addr == sysPara.camAddr)
            {
                CameraStruct.ZoomSetPosition = Pelcod->data1;
                CameraStruct.ZoomSetPosition = (CameraStruct.ZoomSetPosition << 8) | Pelcod->data2;
                CameraStruct.ZoomSetPosition = alg->CalcCamAd(CameraStruct.ZoomSetPosition);
                #ifdef IPC_EN
                    Set_Camera_Zoom_Position();
                #else
                    CameraStruct.ZoomSet_Flag    = SET;
                #endif
            }
            if (TVC_Pelcod.addr == sysPara.irAddr)
            {
                IRStruct.ZoomSetPosition    = Pelcod->data1;
                IRStruct.ZoomSetPosition    = (IRStruct.ZoomSetPosition << 8) | Pelcod->data2;
                command_Set_FOV(IRStruct.ZoomSetPosition);
            }
            break;
//        case 0x51:
//        {
//            //水平角度查询
//            if (!AngleDataReturn_flag)
//            {
//                if (PTStruct.PanCurAngle >= PTStruct.PanOriAngle)
//                {
//                    offset_angle = PTStruct.PanCurAngle - PTStruct.PanOriAngle;
//                }
//                else
//                {
//                    offset_angle = 36000  + PTStruct.PanCurAngle - PTStruct.PanOriAngle;
//                }
//                if (TVC_ProtoType == PELCO_D)
//                {
//                    TVC_UART_TxBuf[0] = PELCOD_STX;
//                    TVC_UART_TxBuf[1] = TVC_addr;
//                    TVC_UART_TxBuf[2] = 0;
//                    TVC_UART_TxBuf[3] = 0x59;
//                    TVC_UART_TxBuf[4] = (offset_angle >> 8) & 0xFF;
//                    TVC_UART_TxBuf[5] = offset_angle & 0xFF;
//                    TVC_UART_TxBuf[6] = (unsigned char)(TVC_UART_TxBuf[1] + TVC_UART_TxBuf[2] + TVC_UART_TxBuf[3] + TVC_UART_TxBuf[4] + TVC_UART_TxBuf[5]);
//                    TVC_UART_SendData(TVC_UART_TxBuf, 7);
//                }
//                else if (TVC_ProtoType == PELCO_P)
//                {
//                    TVC_UART_TxBuf[0] = PELCOP_STX;
//                    TVC_UART_TxBuf[1] = TVC_addr;
//                    TVC_UART_TxBuf[2] = 0;
//                    TVC_UART_TxBuf[3] = 0x59;
//                    TVC_UART_TxBuf[4] = (offset_angle >> 8) & 0xFF;
//                    TVC_UART_TxBuf[5] = offset_angle & 0xFF;
//                    TVC_UART_TxBuf[6] = PELCOP_ETX;
//                    TVC_UART_TxBuf[7] = (unsigned char)(TVC_UART_TxBuf[1] + TVC_UART_TxBuf[2] + TVC_UART_TxBuf[3] + TVC_UART_TxBuf[4] + TVC_UART_TxBuf[5]);
//                    TVC_UART_SendData(TVC_UART_TxBuf, 8);
//                }
//            }
//            break;
//        }
//        case 0x53:
//        {
//            //俯仰角度查询
//            if (!AngleDataReturn_flag)
//            {
//                if (PTStruct.TiltCurAngle >= PTStruct.TiltOriAngle)
//                {
//                    offset_angle = PTStruct.TiltCurAngle - PTStruct.TiltOriAngle;
//                }
//                else
//                {
//                    offset_angle = 36000 + PTStruct.TiltCurAngle - PTStruct.TiltOriAngle;
//                }
//                if (TVC_ProtoType == PELCO_D)
//                {
//                    TVC_UART_TxBuf[0] = PELCOD_STX;
//                    TVC_UART_TxBuf[1] = TVC_addr;
//                    TVC_UART_TxBuf[2] = 0;
//                    TVC_UART_TxBuf[3] = 0x5B;
//                    TVC_UART_TxBuf[4] = (offset_angle >> 8) & 0xFF;
//                    TVC_UART_TxBuf[5] = offset_angle & 0xFF;
//                    TVC_UART_TxBuf[6] = (unsigned char)(TVC_UART_TxBuf[1] + TVC_UART_TxBuf[2] + TVC_UART_TxBuf[3] + TVC_UART_TxBuf[4] + TVC_UART_TxBuf[5]);
//                    TVC_UART_SendData(TVC_UART_TxBuf, 7);
//                }
//                else if (TVC_ProtoType == PELCO_P)
//                {
//                    TVC_UART_TxBuf[0] = PELCOP_STX;
//                    TVC_UART_TxBuf[1] = TVC_addr;
//                    TVC_UART_TxBuf[2] = 0;
//                    TVC_UART_TxBuf[3] = 0x5B;
//                    TVC_UART_TxBuf[4] = (offset_angle >> 8) & 0xFF;
//                    TVC_UART_TxBuf[5] = offset_angle & 0xFF;
//                    TVC_UART_TxBuf[6] = PELCOP_ETX;
//                    TVC_UART_TxBuf[7] = (unsigned char)(TVC_UART_TxBuf[1] + TVC_UART_TxBuf[2] + TVC_UART_TxBuf[3] + TVC_UART_TxBuf[4] + TVC_UART_TxBuf[5]);
//                    TVC_UART_SendData(TVC_UART_TxBuf, 8);
//                }
//            }
//            break;
//        }

//        case 0x55:
//        {

//            //可见光角度查询
//            if (TVC_Pelcod.addr == TVC_addr)
//            {
//                if (inquireValue.cameraZoomFlag == 1)
//                {
//                    inquireValue.cameraZoomFlag = 0;
//                    if (value.camera == IPC)
//                    {
//                        CameraStruct.ZoomCurPosition = Get_Camera_Zoom_Position();
//                    }
//                    else
//                    {
//                        CameraStruct.ZoomCurPosition = LENS->Camera->ReadZoomValue();
//                    }
//                    if (value.dataReturn == ANGLE)
//                    {
//                        CameraStruct.ZoomCurPosition = DataReturn->CalculationCameraAngle(CameraStruct.ZoomCurPosition);
//                    }
//                    inquireValue.cameraValue = CameraStruct.ZoomCurPosition;
//                }

//                if (TVC_ProtoType == PELCO_D)
//                {
//                    TVC_UART_TxBuf[0] = PELCOD_STX;
//                    TVC_UART_TxBuf[1] = TVC_addr;
//                    TVC_UART_TxBuf[2] = 0;
//                    TVC_UART_TxBuf[3] = 0x5D;
//                    TVC_UART_TxBuf[4] = (inquireValue.cameraValue >> 8) & 0xFF;
//                    TVC_UART_TxBuf[5] = inquireValue.cameraValue & 0xFF;
//                    TVC_UART_TxBuf[6] = (unsigned char)(TVC_UART_TxBuf[1] + TVC_UART_TxBuf[2] + TVC_UART_TxBuf[3] + TVC_UART_TxBuf[4] + TVC_UART_TxBuf[5]);
//                    TVC_UART_SendData(TVC_UART_TxBuf, 7);
//                }

//            }

//            else if (TVC_Pelcod.addr == IR_addr)
//            {
//                if (inquireValue.irZoomFlag == 1)
//                {
//                    inquireValue.irZoomFlag = 0;
//                    IRStruct.ZoomCurPosition    =   LENS->IR->ReadZoomValue();

//                    if (value.dataReturn == ANGLE)
//                    {
//                        IRStruct.ZoomCurPosition = DataReturn->CalculationIRAngle(IRStruct.ZoomCurPosition);
//                    }
//                    inquireValue.irValue = IRStruct.ZoomCurPosition ;
//                }
//                if (TVC_ProtoType == PELCO_D)
//                {
//                    TVC_UART_TxBuf[0] = PELCOD_STX;
//                    TVC_UART_TxBuf[1] = IR_addr;
//                    TVC_UART_TxBuf[2] = 0;
//                    TVC_UART_TxBuf[3] = 0x5D;
//                    TVC_UART_TxBuf[4] = (inquireValue.irValue >> 8) & 0xFF;
//                    TVC_UART_TxBuf[5] = inquireValue.irValue & 0xFF;
//                    TVC_UART_TxBuf[6] = (unsigned char)(TVC_UART_TxBuf[1] + TVC_UART_TxBuf[2] + TVC_UART_TxBuf[3] + TVC_UART_TxBuf[4] + TVC_UART_TxBuf[5]);
//                    TVC_UART_SendData(TVC_UART_TxBuf, 7);
//                }

//            }
//            break;
//        }
        case 0x57: //聚焦定位
            
//            if (TVC_Pelcod.addr == sysPara.camAddr)
//            {
//                CameraStruct.FocusSetPosition   =   Pelcod->data1 & 0x7F;
//                CameraStruct.FocusSetPosition   =   (CameraStruct.FocusSetPosition << 8) | Pelcod->data2;
//                CameraStruct.FocusSet_Flag      =   SET;
//                CameraStruct.FCSaveFlag         =   Pelcod->data1 >> 7;
//                FocusSetPosition2               =   CameraStruct.FocusSetPosition;
//                init_fc_value                   =   1;
//            }
            if (TVC_Pelcod.addr == sysPara.irAddr)
            {
                IRStruct.FocusSetPosition   =   Pelcod->data1 & 0x7F;
                IRStruct.FocusSetPosition   =   (IRStruct.FocusSetPosition << 8) | Pelcod->data2;
                IrLocFocus(IRStruct.FocusSetPosition);
            }
            break;
        case 0x65://聚焦值查询
//            if (TVC_Pelcod.addr == sysPara.camAddr)
//            {
//                CameraStruct.FocusCurPosition   =   LENS->Camera->ReadFocusValue();
//                TVC_UART_TxBuf[0] = PELCOD_STX;
//                TVC_UART_TxBuf[1] = TVC_Pelcod.addr;
//                TVC_UART_TxBuf[2] = 0;
//                TVC_UART_TxBuf[3] = 0x6D;
//                TVC_UART_TxBuf[4] = (CameraStruct.FocusCurPosition >> 8) & 0xFF;
//                TVC_UART_TxBuf[4] = TVC_UART_TxBuf[4] | (CameraStruct.FocusFar_RunFlag << 7);
//                TVC_UART_TxBuf[5] = CameraStruct.FocusCurPosition & 0xFF;
//                TVC_UART_TxBuf[6] = (unsigned char)(TVC_UART_TxBuf[1] + TVC_UART_TxBuf[2] + TVC_UART_TxBuf[3] + TVC_UART_TxBuf[4] + TVC_UART_TxBuf[5]);
//                TVC_UART_SendData(TVC_UART_TxBuf, 7);   
//            }
            if (TVC_Pelcod.addr == sysPara.irAddr)
            {
                command_Inq_ZFPos();
                IRStruct.ZoomCurPosition    =   get_IR_ZoomPos();
                IRStruct.FocusCurPosition   =   get_IR_FocusPos();
                IRStruct.FocusCurPosition   =   IRStruct.FocusCurPosition&0x7fff;
                TVC_UART_TxBuf[0] = PELCOD_STX;
                TVC_UART_TxBuf[1] = TVC_Pelcod.addr;
                TVC_UART_TxBuf[2] = 0;
                TVC_UART_TxBuf[3] = 0x6D;
                TVC_UART_TxBuf[4] = (IRStruct.FocusCurPosition >> 8) & 0xFF;
                TVC_UART_TxBuf[4] = TVC_UART_TxBuf[4] | (IRStruct.FocusFar_RunFlag << 7);
                TVC_UART_TxBuf[5] = IRStruct.FocusCurPosition & 0xFF;
                TVC_UART_TxBuf[6] = (unsigned char)(TVC_UART_TxBuf[1] + TVC_UART_TxBuf[2] + TVC_UART_TxBuf[3] + TVC_UART_TxBuf[4] + TVC_UART_TxBuf[5]);
                TVC_UART_SendData(TVC_UART_TxBuf, 7);
            }
            break;

        case 0xF1: //重启
            __set_FAULTMASK(1);
            NVIC_SystemReset();
            break;
        default :
            break ;
    }
}



void LocPtAngle()
{
    if(locFlag != 2)
    {
        return ;
    }
    locFlag = 0;
    locHorPos = PtzZeroAngleCal_Call_H(locHorPos,sysPara.zeroPos.hor,36000);
    locVerPos = PtzZeroAngleCal_Call_H(locVerPos,sysPara.zeroPos.ver,36000);
    LocAngle(locHorPos,locVerPos,locSpeed);
    
}



typedef struct 
{
    unsigned char head;
    unsigned char addr;
    unsigned char cmd1;
    unsigned char cmd2;
    unsigned char data1;
    unsigned char data2;
    unsigned char checkSum;
}PELCO_D_T;

unsigned char PrtlFilter(unsigned char cmd1,unsigned cmd2)
{
    if(sysPara.alarmStop == 0)
    {
        if ((cmd1 == 0x10) && (cmd2 == 0x10))
        {
            return 1;
        }
        if ((cmd1 == 0x11) && (cmd2 == 0x11))
        {
            return 1;
        }
    }
    if(cmd2 == 0x67)
    {
        return 1;
    }
    return 0;
}
int ComCmd_Proc1(void)
{
    MSGQUEUE queue;
	queue = Get_Queue();
    if(queue.msg_proto != 1)
    {
        return -1;
    }
    PELCO_D_T rev;
    unsigned char calcSum;
    rev.head        =   queue.msg_data[0];
    rev.addr        =   queue.msg_data[1];
    rev.cmd1        =   queue.msg_data[2];
    rev.cmd2        =   queue.msg_data[3];
    rev.data1       =   queue.msg_data[4];
    rev.data2       =   queue.msg_data[5];
    rev.checkSum    =   queue.msg_data[6];

    if ((rev.addr != sysPara.camAddr) && (rev.addr != sysPara.irAddr)) 
    {
        return -2 ;
    }
    calcSum = pMiscAlg->PelcodCrc((unsigned char *)queue.msg_data);
    if(calcSum !=  rev.checkSum)
    {
        return -3;
    }
    if(PrtlFilter(rev.cmd1,rev.cmd2) == 1)
    {
        return -4;
    }
    clear_function_flag();
    switch ((rev.cmd1<<8)|rev.cmd2)
    {
        case 0x0008:
            PtUp(rev.data2);
            break;
        case 0x0010:
            PtDown(rev.data2);
            break;
        case 0x0004:
            PtLeft(rev.data1);
            break;
        case 0x0002:	
            PtRight(rev.data1);	
            break;
        case 0x000c:
            PtLeftUp(rev.data1,rev.data2);
            break;
        case 0x0014:
            PtLeftDown(rev.data1,rev.data2);	
            break;
        case 0x000a:
            PtRightUp(rev.data1,rev.data2);	
            break;
        case 0x0012:
            PtRightDown(rev.data1,rev.data2);		
            break;
        case 0x0000:
            PtStop();
            break;
//        case 0x0020:
//            ptlPelcodRes.cmdType = PELCOD_TELE;
//            break;
//        case 0x0040:
//            ptlPelcodRes.cmdType = PELCOD_WIDE;
//            break;
//        case 0x0080:
//            ptlPelcodRes.cmdType = PELCOD_FAR;
//            break;
//        case 0x0100:
//            ptlPelcodRes.cmdType = PELCOD_NEAR;
//            break; 
//        case 0x0200:
//            ptlPelcodRes.cmdType = PELCOD_LASER_BIG;
//            break; 
//        case 0x0400:
//            ptlPelcodRes.cmdType = PELCOD_LASER_SMALL;
//            break;         
//        case 0x0000:
//            ptlPelcodRes.cmdType = PELCOD_STOP;isrSs++;
//            break;
//        case 0x0007:			
//            ptlPelcodRes.cmdType = PELCOD_CALL_PES;
//            break;
//        case 0x0003:			
//            ptlPelcodRes.cmdType = PELCOD_SET_PES;
//            break;
//        case 0x0005:			
//            ptlPelcodRes.cmdType = PELCOD_DEL_PES;
//            break;
//        case 0x004b:
//            ptlPelcodRes.cmdType = PELCOD_HOR_LOC;
//            break;
//        case 0x004d:
//            ptlPelcodRes.cmdType = PELCOD_VER_LOC;
//            break;
//        case 0x004f:
//            ptlPelcodRes.cmdType = PELCOD_ZOOM_LOC;
//            break;
//        case 0x0057:
//            ptlPelcodRes.cmdType = PELCOD_FOCUS_LOC;
//            break;
//        case 0x0051:
//            ptlPelcodRes.cmdType = PELCOD_HOR_INQU;
//            break;
//        case 0x0053:
//            ptlPelcodRes.cmdType = PELCOD_VER_INQU;
//            break;
//        case 0x0055:
//            ptlPelcodRes.cmdType = PELCOD_ZOOM_INQU;
//            break;
//        case 0x0065:
//            ptlPelcodRes.cmdType = PELCOD_FOCUS_INQU;
//            break;
//        case 0x00f1:
//            ptlPelcodRes.cmdType = PELCOD_RESTART;
//            break;
//        
//        case 0x0059:
//            ptlPelcodRes.cmdType = PELCOD_PT_HOR_READ;
//            break;
//        case 0x005b:
//            ptlPelcodRes.cmdType = PELCOD_PT_VER_READ;
//            break;
    }
//    TVC_Pelcod.ExtendCmd_flag   =   TVC_Pelcod.cmd2 & 0x01 ;
//    clear_function_flag();
//    if (TVC_Pelcod.ExtendCmd_flag == 0)
//    {
//        if (TVC_Pelcod.addr == sysPara.camAddr)
//        {
//            CameraStruct.ZoomWide_Flag  =   TVC_Pelcod.cmd2 & 0x40 ? 1 : 0;
//            CameraStruct.ZoomTele_Flag  =   TVC_Pelcod.cmd2 & 0x20 ? 1 : 0;
//            CameraStruct.FocusFar_Flag  =   TVC_Pelcod.cmd2 & 0x80 ? 1 : 0;
//            CameraStruct.FocusNear_Flag =   TVC_Pelcod.cmd1 & 0x01 ? 1 : 0;
//            CameraStruct.IrisOpen_Flag  =   TVC_Pelcod.cmd1 & 0x02 ? 1 : 0;
//            CameraStruct.IrisClose_Flag =   TVC_Pelcod.cmd1 & 0x04 ? 1 : 0;
//            PTStruct.PanCtrl            =   TVC_Pelcod.cmd2 & 0x06;
//            PTStruct.TiltCtrl           =   TVC_Pelcod.cmd2 & 0x18;
//            PTStruct.PanSpeed           =   TVC_Pelcod.data1;
//            PTStruct.TiltSpeed          =   TVC_Pelcod.data2;
//        }
//        else
//        {  
//            IRStruct.ZoomWide_Flag  =   TVC_Pelcod.cmd2 & 0x40 ? 1 : 0;
//            IRStruct.ZoomTele_Flag  =   TVC_Pelcod.cmd2 & 0x20 ? 1 : 0;
//            IRStruct.FocusFar_Flag  =   TVC_Pelcod.cmd2 & 0x80 ? 1 : 0;
//            IRStruct.FocusNear_Flag =   TVC_Pelcod.cmd1 & 0x01 ? 1 : 0;

//            IRStruct.IrisOpen_Flag = TVC_Pelcod.cmd1 & 0x02 ? 1 : 0 ;
//            IRStruct.IrisClose_Flag = TVC_Pelcod.cmd1 & 0x04 ? 1 : 0 ;
//            PTStruct.PanCtrl = TVC_Pelcod.cmd2 & 0x06 ;
//            PTStruct.TiltCtrl = TVC_Pelcod.cmd2 & 0x18 ;
//            PTStruct.PanSpeed = TVC_Pelcod.data1 ;
//            PTStruct.TiltSpeed = TVC_Pelcod.data2 ;
//        }

//        if (TVC_Pelcod.addr == sysPara.camAddr)
//        {
//            PT_ctrl(&PTStruct, &CameraStruct);
//            CameraLense_Ctrl(&CameraStruct);
//        }

//        if(TVC_Pelcod.addr == sysPara.irAddr)
//        {
//            PT_ctrl(&PTStruct, NULL);
//            IRLense_Ctrl(&IRStruct);
//        }
//    }
//    else
//    {
//        Pelcod_ExtendCmd_proc(&TVC_Pelcod);
//    }
    return 0;
}

int ComCmd_Proc(void)
{
    MSGQUEUE queue;
	queue = Get_Queue();
    if(queue.msg_proto != 1)
    {
        return -1;
    }
    PELCO_D_T rev;
    unsigned char calcSum;
    rev.head        =   queue.msg_data[0];
    rev.addr        =   queue.msg_data[1];
    rev.cmd1        =   queue.msg_data[2];
    rev.cmd2        =   queue.msg_data[3];
    rev.data1       =   queue.msg_data[4];
    rev.data2       =   queue.msg_data[5];
    rev.checkSum    =   queue.msg_data[6];
    
    TVC_Pelcod.addr     =	queue.msg_data[1];                
    TVC_Pelcod.cmd1     =	queue.msg_data[2];
    TVC_Pelcod.cmd2     =	queue.msg_data[3];
    TVC_Pelcod.data1    =	queue.msg_data[4];
    TVC_Pelcod.data2    =	queue.msg_data[5];
    TVC_Pelcod.crc      =	queue.msg_data[6];	
    if ((rev.addr != sysPara.camAddr) && (rev.addr != sysPara.irAddr)) 
    {
        return -2 ;
    }
    if ((TVC_Pelcod.cmd1 == 0xC0) || (TVC_Pelcod.cmd1 == 0xC1))
    {
        LOC_3D_VAL_T val3D;
        unsigned int ad;
        if(TVC_Pelcod.cmd1 == 0xC0)
        {
            val3D.action = LOC_3D_ZOOM_IN;
        }
        else 
        {
            val3D.action = LOC_3D_ZOOM_OUT;
        }
        if (TVC_Pelcod.addr == sysPara.camAddr)
        {
            clear_function_flag();
            #ifdef IPC_EN
                ad = Get_Camera_Zoom_Position();
            #else
                ad = LENS->Camera->ReadZoomValue();
            #endif
            val3D.lensAngle = alg->CalcCamAngle(ad);
            Loc3DCoorChange(TVC_Pelcod.cmd2,TVC_Pelcod.data1,TVC_Pelcod.data2,TVC_Pelcod.crc,&val3D.xCoord,&val3D.yCoord,&val3D.xWidth,&val3D.yHeight);
            Loc3DCamAction(val3D);
            TVC_UART_ProtoFlag = NULL;
            return 1;
        }
        else if (TVC_Pelcod.addr == sysPara.irAddr)
        {
            clear_function_flag();
            command_Inq_FOV();
            delay_ms(150);
            Loc3DCoorChange(TVC_Pelcod.cmd2,TVC_Pelcod.data1,TVC_Pelcod.data2,TVC_Pelcod.crc,&val3D.xCoord,&val3D.yCoord,&val3D.xWidth,&val3D.yHeight);
            val3D.lensAngle = IR_Lense_Angle_Data;
            Loc3DIrAction(val3D);
            TVC_UART_ProtoFlag = NULL;
            return 1;
        }
        else
        {
            return -2;
        }
    }
    calcSum = pMiscAlg->PelcodCrc((unsigned char *)queue.msg_data);
    if(calcSum !=  rev.checkSum)
    {
        return -3;
    }
    if(PrtlFilter(rev.cmd1,rev.cmd2) == 1)
    {
        return -4;
    }
    clear_function_flag();
    
    TVC_Pelcod.ExtendCmd_flag   =   TVC_Pelcod.cmd2 & 0x01 ;
    clear_function_flag();
    if (TVC_Pelcod.ExtendCmd_flag == 0)
    {
        if (TVC_Pelcod.addr == sysPara.camAddr)
        {
            CameraStruct.ZoomWide_Flag  =   TVC_Pelcod.cmd2 & 0x40 ? 1 : 0;
            CameraStruct.ZoomTele_Flag  =   TVC_Pelcod.cmd2 & 0x20 ? 1 : 0;
            CameraStruct.FocusFar_Flag  =   TVC_Pelcod.cmd2 & 0x80 ? 1 : 0;
            CameraStruct.FocusNear_Flag =   TVC_Pelcod.cmd1 & 0x01 ? 1 : 0;
            CameraStruct.IrisOpen_Flag  =   TVC_Pelcod.cmd1 & 0x02 ? 1 : 0;
            CameraStruct.IrisClose_Flag =   TVC_Pelcod.cmd1 & 0x04 ? 1 : 0;
            PTStruct.PanCtrl            =   TVC_Pelcod.cmd2 & 0x06;
            PTStruct.TiltCtrl           =   TVC_Pelcod.cmd2 & 0x18;
            PTStruct.PanSpeed           =   TVC_Pelcod.data1;
            PTStruct.TiltSpeed          =   TVC_Pelcod.data2;
        }
        else
        {  
            IRStruct.ZoomWide_Flag  =   TVC_Pelcod.cmd2 & 0x40 ? 1 : 0;
            IRStruct.ZoomTele_Flag  =   TVC_Pelcod.cmd2 & 0x20 ? 1 : 0;
            IRStruct.FocusFar_Flag  =   TVC_Pelcod.cmd2 & 0x80 ? 1 : 0;
            IRStruct.FocusNear_Flag =   TVC_Pelcod.cmd1 & 0x01 ? 1 : 0;

            IRStruct.IrisOpen_Flag = TVC_Pelcod.cmd1 & 0x02 ? 1 : 0 ;
            IRStruct.IrisClose_Flag = TVC_Pelcod.cmd1 & 0x04 ? 1 : 0 ;
            PTStruct.PanCtrl = TVC_Pelcod.cmd2 & 0x06 ;
            PTStruct.TiltCtrl = TVC_Pelcod.cmd2 & 0x18 ;
            PTStruct.PanSpeed = TVC_Pelcod.data1 ;
            PTStruct.TiltSpeed = TVC_Pelcod.data2 ;
        }

        if (TVC_Pelcod.addr == sysPara.camAddr)
        {
            PT_ctrl(&PTStruct, &CameraStruct);
            CameraLense_Ctrl(&CameraStruct);
        }

        if(TVC_Pelcod.addr == sysPara.irAddr)
        {
            PT_ctrl(&PTStruct, &IRStruct);
            IRLense_Ctrl(&IRStruct);
        }
    }
    else
    {
        Pelcod_ExtendCmd_proc(&TVC_Pelcod);
    }
    return 0;
}

void PTControl(unsigned char dir, unsigned char hSpeed, unsigned vSpeed)
{
    unsigned char buf[7];
    buf[0] = 0xff;
    buf[1] = 0x01;
    buf[2] = 0;
    buf[3] = dir;
    buf[4] = hSpeed;
    buf[5] = vSpeed;
    buf[6] = (buf[1] + buf[2] + buf[3] + buf[4] + buf[5]) & 0xff;

    PT_UART_SendData(buf, 7);

}


void ClearCmdComplexCnt()
{
    if (cmdComplex != COMPLEX_NONE)
    {
        cmdComplexCnt ++;
        if (cmdComplexCnt > 20)
        {
            cmdComplexCnt = 0;
            cmdComplex = COMPLEX_NONE;
        }
    }
}

int PtzZeroAngleCal(int Angle,int zeroAngle)
{
    int AngleCal = 0;

    if(Angle>=zeroAngle)
    {
        AngleCal = Angle - zeroAngle;
    }
    else
    {
        AngleCal = 36000 + Angle - zeroAngle;
    }
    return AngleCal;
}

void PTAngleReturn()
{
    int hp,vp;
    hp = PtzZeroAngleCal(PTStruct.PanCurAngle,sysPara.zeroPos.hor);
    vp = PtzZeroAngleCal(PTStruct.TiltCurAngle,sysPara.zeroPos.ver);
    TVC_UART_TxBuf[0] = 0xFF;
    TVC_UART_TxBuf[1] = sysPara.camAddr;
    TVC_UART_TxBuf[2] = 0;
    TVC_UART_TxBuf[3] = 0x59;
    TVC_UART_TxBuf[4] = (hp >> 8) & 0xFF;
    TVC_UART_TxBuf[5] = hp & 0xFF;
    TVC_UART_TxBuf[6] = (unsigned char)(TVC_UART_TxBuf[1] + TVC_UART_TxBuf[2] + TVC_UART_TxBuf[3] + TVC_UART_TxBuf[4] + TVC_UART_TxBuf[5]);
    TVC_UART_SendData(TVC_UART_TxBuf, 7);

    TVC_UART_TxBuf[0] = 0xFF;
    TVC_UART_TxBuf[1] = sysPara.camAddr;
    TVC_UART_TxBuf[2] = 0;
    TVC_UART_TxBuf[3] = 0x5B;
    TVC_UART_TxBuf[4] = (vp >> 8) & 0xFF;
    TVC_UART_TxBuf[5] = vp & 0xFF;
    TVC_UART_TxBuf[6] = (unsigned char)(TVC_UART_TxBuf[1] + TVC_UART_TxBuf[2] + TVC_UART_TxBuf[3] + TVC_UART_TxBuf[4] + TVC_UART_TxBuf[5]);
    TVC_UART_SendData(TVC_UART_TxBuf, 7);
    
}


void CameraAngleReturn()
{
    #ifdef IPC_EN
        returnValue.cameraValue = Get_Camera_Zoom_Position();
    #else
        returnValue.cameraValue = LENS->Camera->ReadZoomValue();
    #endif
    if(sysPara.camReturnType == 0)
    {
        returnValue.cameraValue = alg->CalcCamAngle(returnValue.cameraValue);
    }
    TVC_UART_TxBuf[0] = PELCOD_STX;
    TVC_UART_TxBuf[1] = sysPara.camAddr;
    TVC_UART_TxBuf[2] = 0;
    TVC_UART_TxBuf[3] = 0x5D;
    TVC_UART_TxBuf[4] = (returnValue.cameraValue >> 8) & 0xFF;
    TVC_UART_TxBuf[5] = returnValue.cameraValue & 0xFF;
    TVC_UART_TxBuf[6] = (unsigned char)(TVC_UART_TxBuf[1] + TVC_UART_TxBuf[2] + TVC_UART_TxBuf[3] + TVC_UART_TxBuf[4] + TVC_UART_TxBuf[5]);
    TVC_UART_SendData(TVC_UART_TxBuf, 7);

}
void IRAngleReturn()
{
    command_Inq_FOV();
    returnValue.irValue = IR_Lense_Angle_Data;
    TVC_UART_TxBuf[0] = PELCOD_STX;
    TVC_UART_TxBuf[1] = sysPara.irAddr;
    TVC_UART_TxBuf[2] = 0;
    TVC_UART_TxBuf[3] = 0x5D;
    TVC_UART_TxBuf[4] = (returnValue.irValue >> 8) & 0xFF;
    TVC_UART_TxBuf[5] = returnValue.irValue & 0xFF;
    TVC_UART_TxBuf[6] = (unsigned char)(TVC_UART_TxBuf[1] + TVC_UART_TxBuf[2] + TVC_UART_TxBuf[3] + TVC_UART_TxBuf[4] + TVC_UART_TxBuf[5]);
    TVC_UART_SendData(TVC_UART_TxBuf, 7);

}


void DataReturn(void)
{
    if((sysPara.ptReturn == 0)&&(sysPara.viewReturn == 0))
    {
        return ;
    }
    if(sysPara.returnCnt < sysPara.returnFreq)
    {
        return;
    }

    sysPara.returnCnt = 0;
    if(sysPara.ptReturn == 1)
    {
        PTAngleReturn();
    }
    if(sysPara.viewReturn == 1)
    {
        CameraAngleReturn();                
        IRAngleReturn();
    }


}



