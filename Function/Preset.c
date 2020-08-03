#include "Preset.h"
#include "ComCmd_proc.h"
#include "LensDriver.h"
#include "eeprom_i2c.h"
#include "delay.h"
#include "TTL_uart.h"
#include "SysCfg.h"
#include "LaserFunction.h"
#include "TVC_uart.h"
#include "VC_uart.h"
#include "timer.h" 
#include "hp_conf.h"
#include "WatchHome.h"
#include "Curise.h"
#include "LineScan.h"
#include "AppleScan.h"
#include "M_3d.h"
#include "Algorithm.h"
CMD_COMPLEX_E cmdComplex = COMPLEX_NONE;
char loc3DCorrCnt = 0;
char cmdDelFlag = 0;
void save_preset(unsigned char PresetNo)
{
    unsigned short addr = PresetNo * 16;
    unsigned char I2C_DataBuffer[16];

    if (PresetNo <= MAX_PRESET_NO)
    {
        PT_SetPreset(PresetNo);
    }

    I2C_DataBuffer[0]   =   PTStruct.PanCurAngle & 0xFF;
    I2C_DataBuffer[1]   = (PTStruct.PanCurAngle >> 8) & 0xFF;
    I2C_DataBuffer[2]   =   PTStruct.TiltCurAngle & 0xFF;
    I2C_DataBuffer[3]   = (PTStruct.TiltCurAngle >> 8) & 0xFF;
    #ifdef IPC_EN
        CameraStruct.ZoomCurPosition        =   Get_Camera_Zoom_Position();
        CameraStruct.FocusCurPosition       =   0;
        I2C_DataBuffer[4]   =   CameraStruct.ZoomCurPosition                        &   0xFF;
        I2C_DataBuffer[5]   = (CameraStruct.ZoomCurPosition       >>  8)  &   0xFF;
        I2C_DataBuffer[6]   =   CameraStruct.FocusCurPosition                       &   0xFF;
        I2C_DataBuffer[7]   = (CameraStruct.FocusCurPosition  >>  8)  &   0xFF;
    #else
        CameraStruct.ZoomCurPosition =  filter1_fine_CAMZOOM();//LENS->Camera->ReadZoomValue();
        CameraStruct.FocusCurPosition   =   LENS->Camera->ReadFocusValue();

        I2C_DataBuffer[4]   =   CameraStruct.ZoomCurPosition & 0xFF;
        I2C_DataBuffer[5]   = (CameraStruct.ZoomCurPosition >> 8) & 0xFF;
        I2C_DataBuffer[6]   =   CameraStruct.FocusCurPosition & 0xFF;
        I2C_DataBuffer[7]   = (CameraStruct.FocusCurPosition >> 8) & 0xFF;
        I2C_DataBuffer[8]   =   CameraStruct.FocusFar_RunFlag;
    #endif
//    command_Inq_FOV();
//    IRStruct.ZoomCurPosition  = IR_Lense_Angle_Data;
    command_Inq_ZFPos();
    IRStruct.ZoomCurPosition    =   get_IR_ZoomPos();
    IRStruct.FocusCurPosition   =   get_IR_FocusPos();

    I2C_DataBuffer[9]       =   IRStruct.ZoomCurPosition & 0xFF;
    I2C_DataBuffer[10]      = (IRStruct.ZoomCurPosition >> 8) & 0xFF;
    I2C_DataBuffer[11]      =   IRStruct.FocusCurPosition & 0xFF;
    I2C_DataBuffer[12]      = (IRStruct.FocusCurPosition >> 8) & 0xFF;
    I2C_DataBuffer[13]      =   IRStruct.FocusFar_RunFlag;

    EEPROM_I2C_WriteBuffer(addr, I2C_DataBuffer, 16);

}
void clear_prest(unsigned char PresetNo)
{
    unsigned short addr = PresetNo * 16;
    unsigned char I2C_DataBuffer[8];
    I2C_DataBuffer[0]   =   0xFF;
    I2C_DataBuffer[1]   =   0xFF;
    I2C_DataBuffer[2]   =   0xFF;
    I2C_DataBuffer[3]   =   0xFF;
    I2C_DataBuffer[4]   =   0xFF;
    I2C_DataBuffer[5]   =   0xFF;
    I2C_DataBuffer[6]   =   0xFF;
    I2C_DataBuffer[7]   =   0xFF;
    EEPROM_I2C_WriteBuffer(addr, I2C_DataBuffer, 8);
    if (PresetNo <= MAX_PRESET_NO)
    {
        PT_ClearPreset(PresetNo);
    }
}
void DelAllPreset()
{
    
    unsigned short i;
    unsigned char I2C_DataBuffer[8] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
    
    for(i = 1;i< 81;i++)
    {
        EEPROM_I2C_WriteBuffer(i*16, I2C_DataBuffer, 8);
        delay_ms(20);
    }
    
}


void set_pan_preset(void)
{
    if(PTStruct.PanSetAngle > 36000)
    {
        return;
    }
    unsigned short PanDiffAngle;
    PTStruct.Pan_PresetSpeedSet_Flag = SET ;
    PanRevCnt       = 0;
    //判断转动的方向
    if (PTStruct.PanSetAngle > PTStruct.PanCurAngle)
    {
        PanDiffAngle            =   PTStruct.PanSetAngle - PTStruct.PanCurAngle ;
        PTStruct.PanCtrl    =   PT_RIGHT ;
        if (PanDiffAngle > 18000)
        {
            PanDiffAngle            =   36000 - PTStruct.PanSetAngle + PTStruct.PanCurAngle ;
            PTStruct.PanCtrl    =   PT_LEFT ;
        }
    }
    else
    {
        PanDiffAngle            =   PTStruct.PanCurAngle - PTStruct.PanSetAngle ;
        PTStruct.PanCtrl    =   PT_LEFT ;
        if (PanDiffAngle > 18000)
        {
            PanDiffAngle            =   36000 - PTStruct.PanCurAngle + PTStruct.PanSetAngle ;
            PTStruct.PanCtrl    =   PT_RIGHT ;
        }
    }
    if (PanDiffAngle < 60)
    {
        PTStruct.PanCtrl                                    =   0;
        PTStruct.Pan_PresetSpeedSet_Flag    = RESET;
    }
    else if (PanDiffAngle < 400)
    {
        if (PTStruct.PresetSpeed > 8)
        {
            PTStruct.PanSpeed   =   8;
        }
        else
        {
            PTStruct.PanSpeed   =   PTStruct.PresetSpeed;
        }
    }
    else if (PanDiffAngle < 800)
    {
        if (PTStruct.PresetSpeed > 12)
        {
            PTStruct.PanSpeed   =   12;
        }
        else
        {
            PTStruct.PanSpeed   =   PTStruct.PresetSpeed;
        }
    }
    else if (PanDiffAngle < 1500)
    {
        if (PTStruct.PresetSpeed > 14)
        {
            PTStruct.PanSpeed   =   14;
        }
        else
        {
            PTStruct.PanSpeed   =   PTStruct.PresetSpeed;
        }
    }
    else
    {
        PTStruct.PanSpeed   =   PTStruct.PresetSpeed;
    }
}


void set_tilt_preset(void)
{
    if(PTStruct.TiltSetAngle > 36000)
    {
        return;
    }
    unsigned short  TiltDiffAngle;
    PTStruct.Tilt_PresetSpeedSet_Flag   =   SET;
    TiltRevCnt      = 0;
    if (PTStruct.TiltSetAngle > PTStruct.TiltCurAngle)
    {
        PTStruct.TiltCtrl   =   PT_DOWN;
        TiltDiffAngle           =   PTStruct.TiltSetAngle - PTStruct.TiltCurAngle;
        if (TiltDiffAngle > 9000)
        {
            PTStruct.TiltCtrl   =   PT_UP;
            TiltDiffAngle           =   36000 - PTStruct.TiltSetAngle + PTStruct.TiltCurAngle;
        }
    }
    else
    {
        PTStruct.TiltCtrl   =   PT_UP;
        TiltDiffAngle           =   PTStruct.TiltCurAngle - PTStruct.TiltSetAngle;
        if (TiltDiffAngle > 9000)
        {
            PTStruct.TiltCtrl   =   PT_DOWN;
            TiltDiffAngle           =   36000 - PTStruct.TiltCurAngle + PTStruct.TiltSetAngle;
        }
    }
    if (TiltDiffAngle < 60)
    {
        PTStruct.TiltCtrl                                   =   0;
        PTStruct.Tilt_PresetSpeedSet_Flag   =   RESET;
        //PT_CallPreset(PresetNo);
    }
    else if (TiltDiffAngle < 400)
    {
        if (PTStruct.PresetSpeed > 8)
        {
            PTStruct.TiltSpeed  =   8;
        }
        else
        {
            PTStruct.TiltSpeed  =   PTStruct.PresetSpeed;
        }
    }
    else if (TiltDiffAngle < 800)
    {
        if (PTStruct.PresetSpeed > 12)
        {
            PTStruct.TiltSpeed  =   12;
        }
        else
        {
            PTStruct.TiltSpeed  =   PTStruct.PresetSpeed;
        }
    }
    else
    {
        PTStruct.TiltSpeed  =   PTStruct.PresetSpeed;
    }
}

int PtzZeroAngleCal_Call_H(int Angle,int zeroAngle,int overInt)
{
    int AngleCal = 0;
 
	if(overInt == 36000)
	{
		if(Angle == 0xaaaa)
		{
			return Angle;
		}
	}
    AngleCal = Angle + zeroAngle;
    if(AngleCal >= overInt)
    {
        AngleCal = AngleCal - overInt;
    }
    
    return AngleCal;
}

int PtzZeroAngleCal_Call_V(int AngleVertical,int AngleStep,int overInt)
{
    int angle_temp = 0;
    int angle_cal = 0;

	if(overInt == 36000)
	{
		if(AngleVertical == 0xaaaa)
		{
			return AngleVertical;
		}
	}

    if(AngleStep >= 0 && AngleStep <= (overInt/4) )
    {
        angle_temp =  0 - (AngleStep);
    }
    else if(AngleStep >= (overInt/3) && AngleStep <= (overInt))
    {
        angle_temp =  ((overInt) - AngleStep);
    }

    angle_cal = AngleVertical - (angle_temp);

    if(angle_cal < 0)
    {
        angle_cal = AngleVertical - (angle_temp) + overInt;
    }

    if( angle_cal >= overInt)
    {
        angle_cal = angle_cal - overInt;
    }
    
    return angle_cal;
}
unsigned char  LocAngle(unsigned int hp,unsigned int vp,unsigned char speed)
{
//    PTStruct.PanSetAngle = PtzZeroAngleCal_Call_H(hp,sysPara.zeroPos.hor,36000);
//    PTStruct.TiltSetAngle = PtzZeroAngleCal_Call_V(vp,sysPara.zeroPos.ver,36000); 
    PTStruct.PanSetAngle    =   hp;
    PTStruct.TiltSetAngle   =   vp;
    if (speed)
    {
        PTStruct.PresetSpeed    =   speed;
        set_pan_preset();
        set_tilt_preset();
        if (!PTStruct.Pan_PresetSpeedSet_Flag && !PTStruct.Tilt_PresetSpeedSet_Flag)
        {
            //水平，俯仰误差都小于0.6度时。
            PT_SetPanAngle(&PTStruct);
            delay_ms(20);
            PT_SetTiltAngle(&PTStruct);
        }
    }
    else
    {
        PT_SetPanAngle(&PTStruct);
        delay_ms(20);
        PT_SetTiltAngle(&PTStruct);
        PTStruct.PanAngleSet_Flag       =   1;
        PTStruct.TiltAngleSet_Flag  =   1;
        PTStruct.PresetSpeed                =   0;
    }
    return 0;
}


unsigned char  call_preset(unsigned char PresetNo, unsigned char speed)
{
    unsigned short  addr    =   PresetNo * 16;
    unsigned char   I2C_DataBuffer[16];
    EEPROM_I2C_ReadBuffer(addr, I2C_DataBuffer, 16);
    delay_ms(50);
    if (I2C_DataBuffer[1] == 0xFF)
    {
        return 1;
    }
    PTStruct.PanSetAngle    =   I2C_DataBuffer[1];
    PTStruct.PanSetAngle    = ((PTStruct.PanSetAngle << 8) & 0xFF00) | I2C_DataBuffer[0];
    PTStruct.TiltSetAngle   =   I2C_DataBuffer[3];
    PTStruct.TiltSetAngle   = ((PTStruct.TiltSetAngle << 8) & 0xFF00) | I2C_DataBuffer[2];


    if (speed)
    {
        PTStruct.PresetSpeed    =   speed;
        set_pan_preset();
        set_tilt_preset();
        if (!PTStruct.Pan_PresetSpeedSet_Flag && !PTStruct.Tilt_PresetSpeedSet_Flag)
        {
            //水平，俯仰误差都小于0.6度时。
            PT_SetPanAngle(&PTStruct);
            delay_ms(20);
            PT_SetTiltAngle(&PTStruct);
        }
    }
    else
    {
        if (PresetNo <= MAX_PRESET_NO)
        {
            PT_CallPreset(PresetNo);
        }
        PTStruct.PanAngleSet_Flag       =   1;
        PTStruct.TiltAngleSet_Flag  =   1;
        PTStruct.PresetSpeed                =   0;
    }

    #ifdef IPC_EN
        //镜头预置位参数初始化，执行函数在主循环中执行Go_CameraLense_Position()
        CameraStruct.ZoomSetPosition        =   I2C_DataBuffer[5];
        CameraStruct.ZoomSetPosition        = ((CameraStruct.ZoomSetPosition  << 8)    &   0xFF00) |   I2C_DataBuffer[4];
        CameraStruct.FocusSetPosition       =   I2C_DataBuffer[7];
        CameraStruct.FocusSetPosition       = ((CameraStruct.FocusSetPosition << 8)    &   0xFF00) |   I2C_DataBuffer[6];
        delay_ms(500);//延时500ms，一体机网络与232信号分开
        Set_Camera_Zoom_Position();
    #else
        //镜头预置位参数初始化，执行函数在主循环中执行Go_CameraLense_Position()
        CameraStruct.ZoomSetPosition        =   I2C_DataBuffer[5];
        CameraStruct.ZoomSetPosition        = ((CameraStruct.ZoomSetPosition << 8) & 0xFF00) | I2C_DataBuffer[4];
        CameraStruct.ZoomSet_Flag           =   1;
        CameraStruct.ZoomCurBigerSet_flag   =   0;
        CameraStruct.ZoomCurSmallerSet_flag =   0;
        CameraStruct.FocusSetPosition       =   I2C_DataBuffer[7];
        CameraStruct.FocusSetPosition       = ((CameraStruct.FocusSetPosition << 8) & 0xFF00) | I2C_DataBuffer[6];
        CameraStruct.FocusSet_Flag          =   1;
        CameraStruct.FocusCurBigerSet_flag  =   0;
        CameraStruct.FocusCurSmallerSet_flag    =   0;
        CameraStruct.FCSaveFlag             =   I2C_DataBuffer[8];

        init_fc_value                       =   1;
        Zoom_Count_Cam = 0;
        CamZoom_time_cnt = 0;
        CamZoomCurPosPre =  0;
        CamAutoFocusCloseNoSave();
    #endif
    IRStruct.ZoomSetPosition            =   I2C_DataBuffer[10];
    IRStruct.ZoomSetPosition            = ((IRStruct.ZoomSetPosition << 8) & 0xFF00) | I2C_DataBuffer[9];
    IRStruct.ZoomSet_Flag               =   1;
    IRStruct.ZoomCurBigerSet_flag       =   0;
    IRStruct.ZoomCurSmallerSet_flag     =   0;
    IRStruct.FocusSetPosition           =   I2C_DataBuffer[12];
    IRStruct.FocusSetPosition           = ((IRStruct.FocusSetPosition << 8) & 0xFF00) | I2C_DataBuffer[11];


    return 0;
}


unsigned char  CallPreset(unsigned char PresetNo, unsigned char speed)
{
    unsigned short  addr    =   PresetNo * 16;
    unsigned char   I2C_DataBuffer[16];
    EEPROM_I2C_ReadBuffer(addr, I2C_DataBuffer, 16);
    delay_ms(50);
    if (I2C_DataBuffer[1] == 0xFF)
    {
        return 1;
    }
    PTStruct.PanSetAngle    =   I2C_DataBuffer[1];
    PTStruct.PanSetAngle    = ((PTStruct.PanSetAngle << 8) & 0xFF00) | I2C_DataBuffer[0];
    PTStruct.TiltSetAngle   =   I2C_DataBuffer[3];
    PTStruct.TiltSetAngle   = ((PTStruct.TiltSetAngle << 8) & 0xFF00) | I2C_DataBuffer[2];


    if (speed)
    {
        PTStruct.PresetSpeed    =   speed;
        set_pan_preset();
        set_tilt_preset();
        if (!PTStruct.Pan_PresetSpeedSet_Flag && !PTStruct.Tilt_PresetSpeedSet_Flag)
        {
            //水平，俯仰误差都小于0.6度时。
            PT_SetPanAngle(&PTStruct);
            delay_ms(20);
            PT_SetTiltAngle(&PTStruct);
        }
    }
    else
    {
//        if (PresetNo <= MAX_PRESET_NO)
//        {
//            PT_CallPreset(PresetNo);
//        }
        PT_SetPanAngle(&PTStruct);
        delay_ms(200);
        PT_SetTiltAngle(&PTStruct);
        PTStruct.PanAngleSet_Flag       =   1;
        PTStruct.TiltAngleSet_Flag  =   1;
        PTStruct.PresetSpeed                =   0;
    }

    #ifdef IPC_EN
        //镜头预置位参数初始化，执行函数在主循环中执行Go_CameraLense_Position()
        CameraStruct.ZoomSetPosition        =   I2C_DataBuffer[5];
        CameraStruct.ZoomSetPosition        = ((CameraStruct.ZoomSetPosition  << 8)    &   0xFF00) |   I2C_DataBuffer[4];
        CameraStruct.FocusSetPosition       =   I2C_DataBuffer[7];
        CameraStruct.FocusSetPosition       = ((CameraStruct.FocusSetPosition << 8)    &   0xFF00) |   I2C_DataBuffer[6];
        delay_ms(500);//延时500ms，一体机网络与232信号分开
        Set_Camera_Zoom_Position();
    #else
        //镜头预置位参数初始化，执行函数在主循环中执行Go_CameraLense_Position()
        CameraStruct.ZoomSetPosition                    =   I2C_DataBuffer[5];
        CameraStruct.ZoomSetPosition                    = ((CameraStruct.ZoomSetPosition << 8) & 0xFF00) | I2C_DataBuffer[4];
        CameraStruct.ZoomSet_Flag                           =   1;
        CameraStruct.ZoomCurBigerSet_flag           =   0;
        CameraStruct.ZoomCurSmallerSet_flag     =   0;
        CameraStruct.FocusSetPosition                   =   I2C_DataBuffer[7];
        CameraStruct.FocusSetPosition                   = ((CameraStruct.FocusSetPosition << 8) & 0xFF00) | I2C_DataBuffer[6];
        CameraStruct.FocusSet_Flag                      =   1;
        CameraStruct.FocusCurBigerSet_flag      =   0;
        CameraStruct.FocusCurSmallerSet_flag    =   0;
        CameraStruct.FCSaveFlag                             =   I2C_DataBuffer[8];

        init_fc_value                                                   =   1;
        Zoom_Count_Cam = 0;
        CamZoom_time_cnt = 0;
        CamZoomCurPosPre =  0;
    #endif
    IRStruct.ZoomSetPosition            =   I2C_DataBuffer[10];
    IRStruct.ZoomSetPosition            = ((IRStruct.ZoomSetPosition << 8) & 0xFF00) | I2C_DataBuffer[9];
    IRStruct.ZoomSet_Flag               =   1;
    IRStruct.ZoomCurBigerSet_flag       =   0;
    IRStruct.ZoomCurSmallerSet_flag     =   0;
    IRStruct.FocusSetPosition           =   I2C_DataBuffer[12];
    IRStruct.FocusSetPosition           = ((IRStruct.FocusSetPosition << 8) & 0xFF00) | I2C_DataBuffer[11];


    return 0;
}

unsigned char  CallPtHorPreset(unsigned char PresetNo, unsigned char speed,unsigned char dir)
{
    unsigned short  addr    =   PresetNo * 16;
    unsigned char   I2C_DataBuffer[2];
    EEPROM_I2C_ReadBuffer(addr, I2C_DataBuffer, 2);
    delay_ms(50);
    if (I2C_DataBuffer[1] == 0xFF)
    {
        return 1;
    }
    PTStruct.PanSetAngle    =   I2C_DataBuffer[1];
    PTStruct.PanSetAngle    = ((PTStruct.PanSetAngle << 8) & 0xFF00) | I2C_DataBuffer[0];
    if(speed == 0)
    {
        speed = 20;
    }
    if (speed)
    {
        PTStruct.PresetSpeed = speed;
        set_pan_preset();
        if(dir != 6)
        {
            PTStruct.PanCtrl =  dir ;
        }
        if (!PTStruct.Pan_PresetSpeedSet_Flag)
        {
            PT_SetPanAngle(&PTStruct);
        }
    }


    return 0;
}
unsigned char  CallPtAbsVerPos(unsigned int pos, unsigned char speed,unsigned char dir)
{

    if(PTStruct.TiltSetAngle > pos)
    {
        PTStruct.TiltSetAngle -= pos;
    }
    else 
    {
        PTStruct.TiltSetAngle = 36000 + PTStruct.TiltSetAngle - pos;
    }
    if(speed == 0)
    {
        speed = 20;
    }
    if (speed)
    {
        PTStruct.PresetSpeed = speed;
        set_tilt_preset();
        if(dir != 6)
        {
            PTStruct.TiltCtrl = dir ;
        }
        if (!PTStruct.Tilt_PresetSpeedSet_Flag)
        {
            PT_SetTiltAngle(&PTStruct);
        }
    }
    return 0;
}
unsigned char  CallPtVerPos(unsigned int pos, unsigned char speed,unsigned char dir)
{

    PTStruct.TiltSetAngle = pos;
    if(speed == 0)
    {
        speed = 20;
    }
    if (speed)
    {
        PTStruct.PresetSpeed = speed;
        set_tilt_preset();
        if(dir != 6)
        {
            PTStruct.TiltCtrl = dir ;
        }
        if (!PTStruct.Tilt_PresetSpeedSet_Flag)
        {
            PT_SetTiltAngle(&PTStruct);
        }
    }
    return 0;
}

void CallPresetsAnalysis(unsigned char number)
{
    if (number <= MAX_PRESET_NO)
    {
        call_preset(number,0);
    }
    else if (number == 84)
    {
        if (sysPara.brush == 1)
        {
            sysPara.brush = 0;
            BrushOff();
        }
    }
    else if (number == 85) //加热关
    {
        GPIO_SetBits(GPIOD, GPIO_Pin_15);
    }
    else if(number == 89)
    {
        WatchHomeSetType(0);
    }
    else if (number == 98)
    {
        CuriseEnable();
    }
    else if (number == 99)
    {
        LineScanEnable();
    }
    else if (number == 100)
    {
        AppleScanEnable();
    }
    
    else if (number == 121)
    {
        if (TVC_Pelcod.addr == sysPara.camAddr)
        {
            CamAutoFocusClose();
        }
        if (TVC_Pelcod.addr == sysPara.irAddr)
        {
            IrCloseZoomAutoFocus();
        }

    }
    
    else if (number == 123)
    {
        sysPara.autoSpeed = 0;
        SysParaSave();
//        buf[0] = sysPara.autoSpeed;
//        EEPROM_I2C_WriteBuffer(EEPROM_SYSINFO_ADDR + 4, buf, 1);
    }
    else if (number == 124)
    {
        sysPara.powerRemember = 0;
        SysParaSave();
//        buf[0] = sysPara.powerRemember;
//        EEPROM_I2C_WriteBuffer(EEPROM_SYSINFO_ADDR+5, buf, 1);

    }
#ifdef LASER_EN 
    else if (number == 125)
    {
        pLaser->Open();
    }
    else if (number == 126)
    {
        pLaser->Close();
    }
    else if (number == 127)
    {
        pLaser->Auto();
    }
    else if (number == 128)
    {
        pLaser->SynchronizeDisable();
    }
#endif
    else if (number == 129)
    {
        sysPara.alarmStop = 0;
        SysParaSave();
//        buf[0] = sysPara.alarmStop;
//        EEPROM_I2C_WriteBuffer(EEPROM_SYSINFO_ADDR+6, buf, 1);
    }
    else if(number == 131)
    {
        IrCloseLocViewAutoFocus();
    }
    else if(number == 136)
    {
        if (TVC_Pelcod.addr == sysPara.camAddr)
        {
            if(loc3DCorrCnt < 12)
            {
                S32 horDiffAngle,verDiffAngle;
                U32 view,ad;
                loc3DCorrCnt ++;
                #ifdef IPC_EN
                    ad = Get_Camera_Zoom_Position();
                #else
                    ad = LENS->Camera->ReadZoomValue();
                #endif
                view = alg->CalcCamAngle(ad);
                if(loc3DCorrCnt == 1)
                {
                    Loc3DSetBasicAngle(PTStruct.PanCurAngle,PTStruct.TiltCurAngle);
                    loc3DCorr.validNum = loc3DCorrCnt - 1;
                    horDiffAngle = Loc3DCalcHorErr(PTStruct.PanCurAngle);
                    verDiffAngle = Loc3DCalcVerErr(PTStruct.TiltCurAngle);
                    loc3DCorr.horBuf[loc3DCorr.validNum]      = horDiffAngle;
                    loc3DCorr.verBuf[loc3DCorr.validNum]      = verDiffAngle;
                    loc3DCorr.camViewBuf[loc3DCorr.validNum]  = view;
                }
                else 
                {
                    loc3DCorr.validNum = loc3DCorrCnt - 1;
                    horDiffAngle = Loc3DCalcHorErr(PTStruct.PanCurAngle);
                    verDiffAngle = Loc3DCalcVerErr(PTStruct.TiltCurAngle);
                    loc3DCorr.horBuf[loc3DCorr.validNum]      = horDiffAngle;
                    loc3DCorr.verBuf[loc3DCorr.validNum]      = verDiffAngle;
                    loc3DCorr.camViewBuf[loc3DCorr.validNum]  = view;
                    
                }
            }
        }
        
    }
    else if (number == 200)
    {
        sysPara.ptReturn = 0;
//        buf[0] = sysPara.ptReturn;
//        EEPROM_I2C_WriteBuffer(EEPROM_SYSINFO_ADDR, buf, 1);
        SysParaSave();
        delay_ms(50);
    }
    else if (number == 203)
    {
        sysPara.viewReturn = 0;
//        buf[0] = sysPara.viewReturn;
//        EEPROM_I2C_WriteBuffer(EEPROM_SYSINFO_ADDR + 1, buf, 1);
        SysParaSave();
        delay_ms(50);
    }
    else if (number == 231)
    {
        sysPara.zeroPos.hor = 0;
        SysParaSave();
        delay_ms(50);
    }
    else if (number == 232)
    {
        sysPara.zeroPos.ver = 0;
        SysParaSave();
        delay_ms(50);
    }
    else if (number == 241)
    {
        PtRestart();
    }
}


void SetPresetsAnalysis(unsigned char number)
{
    if (number <= MAX_PRESET_NO)
    {
        save_preset(number);
    }
    else if (number == 81)
    {
        if (sysPara.fog == 0)
        {
            sysPara.fog = 1;
            FogOn();
            CameraBlack();
        }
    }
    else if (number == 82)
    {
        if (sysPara.fog == 1)
        {
            sysPara.fog = 0;
            FogOff();
            CameraColor();
        }
    }
    else if (number == 84)
    {
        if (sysPara.brush == 0)
        {
            sysPara.brush = 1;
            BrushOn();
        }
    }
    else if (number == 85)//加热开
    {
        GPIO_ResetBits(GPIOD, GPIO_Pin_15);
    }
    else if(number == 89)
    {
        cmdComplex = COMPLEX_WATCH_HOME;
        
    }
    else if (number == 92)
    {
        LineScanSetLeftPos();
    }
    else if (number == 93)
    {
        LineScanSetRightPos();
    }
    else if (number == 98)
    {
        cmdComplex = COMPLEX_CURISE_SET_SPEED;
    }
    else if (number == 103)
    {
        AppleScanSetLeftDownPos();
    }
    else if (number == 104)
    {
        AppleScanSetRightUpPos();
    }
    else if(number == 106)
    {
        cmdComplex = COMPLEX_APPLE_SCAN_SET_STEP_ANGLE;
    }
    else if (number == 107)
    {
        cmdComplex = COMPLEX_CURISE_SET_MINUTE;
    }
    else if (number == 108)
    {
        cmdComplex = COMPLEX_CURISE_SET_SECOND;
    }
    else if (number == 115)
    {
        cmdComplex = COMPLEX_LASER_FULL ;
    }
    else if (number == 116)
    {
        cmdComplex = COMPLEX_LASER_FIR ;
    }
    else if (number == 120)
    {
        cmdComplex = COMPLEX_FOCUS_SPEED;
    }
    else if (number == 121)
    {
        if (TVC_Pelcod.addr == sysPara.camAddr)
        {
            CamAutoFocusOpen();
        }
        if (TVC_Pelcod.addr == sysPara.irAddr)
        {
            IrOpenZoomAutoFocus();
        }

    }
    
    else if (number == 123)
    {
        sysPara.autoSpeed = 1;
        SysParaSave();
    }
    else if (number == 124)
    {
        sysPara.powerRemember = 1;
        SysParaSave();
    }
#ifdef LASER_EN 
    else if (number == 125)
    {
        pLaser->Open();
    }
    else if (number == 126)
    {
        pLaser->Close();
    }
    else if (number == 127)
    {
        pLaser->Auto();
    }
    else if (number == 128)
    {
        pLaser->SynchronizeEnable();
    }
#endif
    else if (number == 129)
    {
        sysPara.alarmStop = 1;
        SysParaSave();
    }
    else if(number == 131)
    {
        IrOpenLocViewAutoFocus();
    }
    else if(number == 136)
    {
        Loc3DCorrParaWrite();
    }
    else if (number == 200)
    {
        sysPara.ptReturn = 1;
//        buf[0] = sysPara.ptReturn;
//        EEPROM_I2C_WriteBuffer(EEPROM_SYSINFO_ADDR, buf, 1);
        SysParaSave();
        delay_ms(50);
    }
    else if (number == 203)
    {
        sysPara.viewReturn = 1;
        SysParaSave();
        delay_ms(50);
    }
    else if (number == 231)
    {
        sysPara.zeroPos.hor = PTStruct.PanCurAngle;
        SysParaSave();
        delay_ms(50);
    }
    else if (number == 232)
    {
        sysPara.zeroPos.ver = PTStruct.TiltCurAngle;
        SysParaSave();
        delay_ms(50);
    }
    else if (number == 241)
    {
        __set_FAULTMASK(1);
        NVIC_SystemReset();
    }
    else if (number == 242) //开启云台实时回传
    {
        PT_SetPreset(105);
    }
    else if (number == 243)
    {
        cmdComplex = COMPLEX_TEST;
    }
    
}
void SetComplexPresetsAnalysis(unsigned char num)
{
    switch ((char)cmdComplex)
    {
    case COMPLEX_CURISE_SET_MINUTE:
        CuriseSetDwellTime(0, num);
        break;
    case COMPLEX_CURISE_SET_SECOND:
        CuriseSetDwellTime(1, num);
        break;
    case COMPLEX_CURISE_SET_SPEED:
        if (((num > 23) && (num < 49)) || (num == 20))
        {
            sysPara.curise.speed = num - 20;
            SysParaSave();
        }
//        if (((num > 23) && (num < 36)) || (num == 20))
//        {
//            sysPara.curise.speed = num - 20;
//            SysParaSave();
//        }
        break;
    case COMPLEX_LASER_FULL:
        pLaser->FullError();
        break;
    case COMPLEX_LASER_FIR:
        pLaser->FirstError();
        break;
    case COMPLEX_FOCUS_SPEED:
        if ((num > 0) && (num < 11))
        {
            CamSetFocusSpeed(num);
        }
        break;
    case COMPLEX_WATCH_HOME:
        WatchHomeInit();
        WatchHomeSetType(num);
        break;
    case COMPLEX_APPLE_SCAN_SET_STEP_ANGLE:
        AppleScanSetStepAngle(num);                                                                                                                 
        break;
    case COMPLEX_TEST:

        break;
    default:
        break;
    }
    cmdComplex = COMPLEX_NONE;
}

void DeletePresetsAnalysis(unsigned char number)
{
    if(cmdDelFlag == 1)
    {
        if(number < 11)
        {
            loc3DCorr.prop = number;
        }
        Loc3DCorrParaWrite();
        cmdDelFlag = 0;
    }
    else 
    {
        if (number <= MAX_PRESET_NO)
        {
            clear_prest(number);
        }
        else if(number == 81)
        {
            sysPara.returnFreq = 1;
            SysParaSave();
        }
        else if(number == 82)
        {
            sysPara.returnFreq = 2;
            SysParaSave();
        }
        else if(number == 83)
        {
            sysPara.returnFreq = 3;
            SysParaSave();
        }
        else if(number == 84)
        {
            sysPara.returnFreq = 4;
            SysParaSave();
        }
        else if(number == 85)  //设置可见光回传视场角
        {
            sysPara.camReturnType = 0;
        }
        else if(number == 86) //设置可见光回传AD
        {
            sysPara.camReturnType = 1;
        }
        else if(number == 87) //删除预置位
        {
            DelAllPreset();
        }
        else if(number == 88)//恢复默认参数
        {
            SysParaDefaultInit();
            SysParaSave();
            AppleScanInit();
            delay_ms(200);
            Loc3DCorrParaDefault();
            delay_ms(200);
        }
        else if(number == 89)
        {
            sysPara.sysTest = 0;
        }
        //3D校正方向
        else if(number == 90)
        {
            Loc3DSetHorCorrDirect(1);
        }
        else if(number == 91)
        {
            Loc3DSetHorCorrDirect(2);
        }
        else if(number == 92)
        {
            Loc3DSetVerCorrDirect(1);
        }
        else if(number == 93)
        {
            Loc3DSetVerCorrDirect(2);
        }
        else if(number == 94) //设置3D比例
        {
            cmdDelFlag = 1;
        }
        else if (number == 121)
        {
            command_autofocus();
            delay_ms(200);
            IrOpenZoomAutoFocus();
        }
    }
        
    
}

