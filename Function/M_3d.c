#include "M_3d.h"
#include "math.h"
#include "SysCfg.h"



//LOC_3D_CORR_ADDR
/****************************************************************
*外部接口
****************************************************************/
#include "PT_uart.h"
#include "ComCmd_proc.h"
#include "Algorithm.h"
#include "TTL_uart.h"
#include "delay.h"
#include "eeprom_i2c.h"
#include "string.h"
static S32 Loc3DGetPtCurAngle(S32 *hor,S32 *ver)
{
    *hor = PTStruct.PanCurAngle;
    *ver = PTStruct.TiltCurAngle;
    return 0;
}

static S32 Loc3DCamViewToAd(S32 view)
{
    return alg->CalcCamAd(view);
}

static void Loc3DCamLocAd(S32 ad)
{
    CamLocAd(ad);
}
static void Loc3DIrLocView(S32 view)
{
    command_Set_FOV(view);
}
static void Loc3DPtLocAngle(S32 hor,S32 ver,S32 speed)
{
    PtSetHorAngle(hor);
    delay_ms(200);
    PtSetVerAngle(ver);
    delay_ms(200);
}





/****************************************************************
*内部函数
*x坐标范围0~640
*y坐标范围0~512
* 宽度范围0~640
* 高度范围0~512
****************************************************************/


#define LOC_3D_K 10


#define LOC_3D_HOR_MIDDLE_COORD (320.0)
#define LOC_3D_VER_MIDDLE_COORD (256.0)

#define LOC_3D_CAM_VIDEO_PROP 	(9.0/16.0)
#define LOC_3D_IR_VIDEO_PROP 	(3.0/4.0)





	


LOC_3D_CORR_T loc3DCorr = 
{
	.validNum 		=   0,
    .prop     		=   2,
    .horCorrDirec	=	LOC_3D_CORR_POSITIVE,
	.verCorrDirec	=	LOC_3D_CORR_POSITIVE,
};



static S32 Loc3DCalcAxis(S32 lensAngle, S32 *corrPtBuf)
{
    S32 i;
    S32 axisVal;
	float k;
    if (lensAngle >= loc3DCorr.camViewBuf[0])
    {
        axisVal = corrPtBuf[0];
    }
    else if (lensAngle <= loc3DCorr.camViewBuf[loc3DCorr.validNum-1])
    {
        axisVal = corrPtBuf[loc3DCorr.validNum-1];
    }
    else
    {
        for (i = 0; i < loc3DCorr.validNum; i++)
        {
            if ((lensAngle >= loc3DCorr.camViewBuf[i+1]) && (lensAngle <= loc3DCorr.camViewBuf[i]))
            {
            	k = (float)(corrPtBuf[i+1] - corrPtBuf[i])/(float)(loc3DCorr.camViewBuf[i] - loc3DCorr.camViewBuf[i+1]);
				
                axisVal = corrPtBuf[i] + (loc3DCorr.camViewBuf[i] - lensAngle) * k;
				break;
            }

        }
    }
    return axisVal;
}

static S32 Loc3DCalcRealPtHorPos(S32 curPos,S32 diffPos)
{
    S32 calcPos = 0;
	if(diffPos > 0) //right 
    {
    	calcPos = curPos + diffPos;
		if(calcPos >= 36000)
		{
			calcPos -= 36000;
		}
    }
    else //left 
    {
        if(curPos >= -diffPos)
        {
            calcPos = curPos + diffPos;
        }
        else 
        {
            calcPos = 36000 + curPos + diffPos;
        }
    }
	return calcPos;
}
static S32 Loc3DCalcRealPtVerPos(S32 curPos,S32 diffPos)
{
    S32 calcPos = 0;
	if(diffPos > 0) //up 
    {
        if(curPos > diffPos)
        {
            calcPos = curPos - diffPos;
        }
        else 
        {
            calcPos = 36000 + curPos - diffPos;
        }
    }
    else //down 
    {
        calcPos = curPos - diffPos;
        if(calcPos >= 36000)
        {
            calcPos = calcPos - 36000;
        }
    }
	return calcPos;
}
void Loc3DCoorChange(S16 x,S16 y,U16 w,U16 h,S16 *px,S16 *py,U16 *pw,U16 *ph)
{
    S16 calcX,calcY;
    float kX,kY ;
    if(x > 0x7f)
    {
        x = 0xff - x + 1;
        calcX = 127 - x;
    }
    else 
    {
        calcX = 127 + x;
    }
    if(y > 0x7f)
    {
        y = 0xff - y + 1;
        calcY = 127 + y;
    }
    else 
    {
        calcY = 127 - y;
    }
    
    kX  = 640.0/254.0;
    kY  = 512.0/254.0;
    *px = calcX*kX;
    *py = calcY*kY;
    *pw = w*kX;
    *ph = h*kY;
}


typedef struct 
{
    S32 hor;
    S32 ver;
    S32 horCorr;
    S32 verCorr;
}DIFF_T;

DIFF_T diff;
void Loc3DCamAction(LOC_3D_VAL_T val3D)
{

	float errAngle,k;
	S32 curHor,curVer,endHor,endVer,lensLocAngle,ad;
	Loc3DGetPtCurAngle(&curHor,&curVer);
	if(val3D.xCoord < LOC_3D_HOR_MIDDLE_COORD) //LEFT
	{
		k = (LOC_3D_HOR_MIDDLE_COORD - val3D.xCoord) / LOC_3D_HOR_MIDDLE_COORD;
        errAngle =  180.0 / PI * atan(k * tan(val3D.lensAngle * 0.01 * 0.5 * PI / 180.0));
        diff.hor = -(short)(errAngle * 100);
	}
	else //RIGHT
	{
		k = (val3D.xCoord - LOC_3D_HOR_MIDDLE_COORD) / LOC_3D_HOR_MIDDLE_COORD;
		errAngle =  180.0 / PI * atan(k * tan(val3D.lensAngle * 0.01 * 0.5 * PI / 180.0));
		diff.hor = (short)(errAngle * 100);
	}

	if (val3D.yCoord > LOC_3D_VER_MIDDLE_COORD)  //DOWN
	{
		k = (val3D.yCoord - LOC_3D_VER_MIDDLE_COORD) / LOC_3D_VER_MIDDLE_COORD;
		errAngle =  180.0 / PI * atan(k * tan(val3D.lensAngle * 0.01 * 0.5 * LOC_3D_CAM_VIDEO_PROP * PI / 180.0 ));
		diff.ver = -(short)(errAngle * 100);
	}
	else  //UP
	{
		k = (LOC_3D_VER_MIDDLE_COORD - val3D.yCoord) / LOC_3D_VER_MIDDLE_COORD;
		errAngle =  180.0 / PI * atan(k * tan(val3D.lensAngle * 0.01 * 0.5 * LOC_3D_CAM_VIDEO_PROP * PI / 180.0 ));
		diff.ver = (short)(errAngle * 100);
	}

	if ((val3D.xWidth > 0) && (val3D.yHeight > 0))
	{
		if (val3D.action == LOC_3D_ZOOM_IN)
		{
			errAngle = val3D.lensAngle * val3D.xWidth / 640;
			lensLocAngle = (short)(errAngle*10.0/(float)loc3DCorr.prop);
		}
		else if (val3D.action == LOC_3D_ZOOM_OUT)
		{
			errAngle = val3D.lensAngle *  640 / val3D.xWidth;
			lensLocAngle = (short)(errAngle*(float)loc3DCorr.prop/10.0);
			if(val3D.lensAngle >= lensLocAngle)
			{
				lensLocAngle = val3D.lensAngle;
			}
		}
		errAngle = Loc3DCalcAxis(lensLocAngle, loc3DCorr.horBuf) - Loc3DCalcAxis(val3D.lensAngle, loc3DCorr.horBuf);
		if(loc3DCorr.horCorrDirec == LOC_3D_CORR_NEGETIVE)
		{
            diff.horCorr = diff.hor + errAngle;
		}
		else 
		{
            diff.horCorr = diff.hor - errAngle;
		}
		
		errAngle =  Loc3DCalcAxis(lensLocAngle, loc3DCorr.verBuf) - Loc3DCalcAxis(val3D.lensAngle, loc3DCorr.verBuf);
		if(loc3DCorr.verCorrDirec == LOC_3D_CORR_NEGETIVE)
		{
            diff.verCorr = diff.ver + errAngle;
		}
		else 
		{
            diff.verCorr = diff.ver - errAngle;
		}
		ad = Loc3DCamViewToAd(lensLocAngle);
		Loc3DCamLocAd(ad);

	}
    else 
    {
        diff.horCorr = diff.hor;
        diff.verCorr = diff.ver;
    }
    
    endHor = Loc3DCalcRealPtHorPos(curHor,diff.horCorr);
	endVer = Loc3DCalcRealPtVerPos(curVer,diff.verCorr);
	Loc3DPtLocAngle(endHor,endVer,40);
}

void Loc3DIrAction(LOC_3D_VAL_T val3D)
{
    float errAngle,k;
	S32 curHor,curVer,endHor,endVer,lensLocAngle;
	Loc3DGetPtCurAngle(&curHor,&curVer);
    val3D.lensAngle = 1900; //test
	if(val3D.xCoord < LOC_3D_HOR_MIDDLE_COORD) //LEFT
	{
		k = (LOC_3D_HOR_MIDDLE_COORD - val3D.xCoord) / LOC_3D_HOR_MIDDLE_COORD;
        errAngle =  180.0 / PI * atan(k * tan(val3D.lensAngle * 0.01 * 0.5 * PI / 180.0));
        diff.hor = -(short)(errAngle * 100);
	}
	else //RIGHT
	{
		k = (val3D.xCoord - LOC_3D_HOR_MIDDLE_COORD) / LOC_3D_HOR_MIDDLE_COORD;
		errAngle =  180.0 / PI * atan(k * tan(val3D.lensAngle * 0.01 * 0.5 * PI / 180.0));
		diff.hor = (short)(errAngle * 100);
	}

	if (val3D.yCoord > LOC_3D_VER_MIDDLE_COORD)  //DOWN
	{
		k = (val3D.yCoord - LOC_3D_VER_MIDDLE_COORD) / LOC_3D_VER_MIDDLE_COORD;
		errAngle =  180.0 / PI * atan(k * tan(val3D.lensAngle * 0.01 * 0.5 * LOC_3D_CAM_VIDEO_PROP * PI / 180.0 ));
		diff.ver = -(short)(errAngle * 100);
	}
	else  //UP
	{
		k = (LOC_3D_VER_MIDDLE_COORD - val3D.yCoord) / LOC_3D_VER_MIDDLE_COORD;
		errAngle =  180.0 / PI * atan(k * tan(val3D.lensAngle * 0.01 * 0.5 * LOC_3D_CAM_VIDEO_PROP * PI / 180.0 ));
		diff.ver = (short)(errAngle * 100);
	}

	if ((val3D.xWidth > 0) && (val3D.yHeight > 0))
	{
		if (val3D.action == LOC_3D_ZOOM_IN)
		{
			errAngle = val3D.lensAngle * val3D.xWidth / 640;
			lensLocAngle = (short)(errAngle*10.0/(float)loc3DCorr.prop);
		}
		else if (val3D.action == LOC_3D_ZOOM_OUT)
		{
			errAngle = val3D.lensAngle *  640 / val3D.xWidth;
			lensLocAngle = (short)(errAngle*(float)loc3DCorr.prop/10.0);
			if(val3D.lensAngle >= lensLocAngle)
			{
				lensLocAngle = val3D.lensAngle;
			}
		}
        Loc3DIrLocView(lensLocAngle);

	}
    
    endHor = Loc3DCalcRealPtHorPos(curHor,diff.hor);
	endVer = Loc3DCalcRealPtVerPos(curVer,diff.ver);
	Loc3DPtLocAngle(endHor,endVer,40);
  
	
}

S32 Loc3DCalcHorErr(S32 curAngle)  
{
	S32 absInt;
	if(loc3DCorr.basicHor > curAngle)
    {
       absInt =  loc3DCorr.basicHor - curAngle;
    }
    else 
    {
        absInt = curAngle - loc3DCorr.basicHor;
    }
	if(absInt > 18000)
	{
		absInt = 36000 - absInt;
	}
	return absInt;

	
}
S32 Loc3DCalcVerErr(S32 curAngle)
{
	S32 absInt;
	if(loc3DCorr.basicVer > curAngle)
    {
       absInt =  loc3DCorr.basicVer - curAngle;
    }
    else 
    {
        absInt = curAngle - loc3DCorr.basicVer;
    }
	if(absInt > 18000)
	{
		absInt = 36000 - absInt;
	}

	return absInt;

}


void Loc3DSetBasicAngle(S32 basicHorAngle,S32 basicVerAngle)
{
    loc3DCorr.basicHor = basicHorAngle;
	loc3DCorr.basicVer = basicVerAngle;
}


void Loc3DSetHorCorrDirect(S8 direct)
{
    if(direct == 1)
    {
        loc3DCorr.horCorrDirec = LOC_3D_CORR_POSITIVE;
    }
    else if(direct == 2)
    {
        loc3DCorr.horCorrDirec = LOC_3D_CORR_NEGETIVE;
    }
}
void Loc3DSetVerCorrDirect(S8 direct)
{
    if(direct == 1)
    {
        loc3DCorr.verCorrDirec = LOC_3D_CORR_POSITIVE;
    }
    else if(direct == 2)
    {
        loc3DCorr.verCorrDirec = LOC_3D_CORR_NEGETIVE;
    }
}
void Loc3DCorrParaDefault()
{
	loc3DCorr.horCorrDirec = LOC_3D_CORR_POSITIVE;
	loc3DCorr.verCorrDirec = LOC_3D_CORR_POSITIVE;
    loc3DCorr.validNum = 0;
    loc3DCorr.prop = 5;
    memset(&loc3DCorr.horBuf, 0, sizeof(loc3DCorr.horBuf));
    memset(&loc3DCorr.verBuf, 0, sizeof(loc3DCorr.verBuf));
    memset(&loc3DCorr.camViewBuf, 0, sizeof(loc3DCorr.camViewBuf));
    Loc3DCorrParaWrite();
}
void Loc3DCorrParaWrite()
{
    unsigned char buf[80];
    unsigned char i = 0;
    buf[i++] = loc3DCorr.horCorrDirec;
    buf[i++] = loc3DCorr.verCorrDirec;
    buf[i++] = loc3DCorr.validNum;
    buf[i++] = loc3DCorr.prop;
    
    buf[i++] = (loc3DCorr.horBuf[0]>>8)&0xff;
    buf[i++] = (loc3DCorr.horBuf[0]>>0)&0xff;
    buf[i++] = (loc3DCorr.horBuf[1]>>8)&0xff;
    buf[i++] = (loc3DCorr.horBuf[1]>>0)&0xff;
    buf[i++] = (loc3DCorr.horBuf[2]>>8)&0xff;
    buf[i++] = (loc3DCorr.horBuf[2]>>0)&0xff;
    buf[i++] = (loc3DCorr.horBuf[3]>>8)&0xff;
    buf[i++] = (loc3DCorr.horBuf[3]>>0)&0xff;
    buf[i++] = (loc3DCorr.horBuf[4]>>8)&0xff;
    buf[i++] = (loc3DCorr.horBuf[4]>>0)&0xff;
    buf[i++] = (loc3DCorr.horBuf[5]>>8)&0xff;
    buf[i++] = (loc3DCorr.horBuf[5]>>0)&0xff;
    buf[i++] = (loc3DCorr.horBuf[6]>>8)&0xff;
    buf[i++] = (loc3DCorr.horBuf[6]>>0)&0xff;
    buf[i++] = (loc3DCorr.horBuf[7]>>8)&0xff;
    buf[i++] = (loc3DCorr.horBuf[7]>>0)&0xff;
    buf[i++] = (loc3DCorr.horBuf[8]>>8)&0xff;
    buf[i++] = (loc3DCorr.horBuf[8]>>0)&0xff;
    buf[i++] = (loc3DCorr.horBuf[9]>>8)&0xff;
    buf[i++] = (loc3DCorr.horBuf[9]>>0)&0xff;
    
    buf[i++] = (loc3DCorr.verBuf[0]>>8)&0xff;
    buf[i++] = (loc3DCorr.verBuf[0]>>0)&0xff;
    buf[i++] = (loc3DCorr.verBuf[1]>>8)&0xff;
    buf[i++] = (loc3DCorr.verBuf[1]>>0)&0xff;
    buf[i++] = (loc3DCorr.verBuf[2]>>8)&0xff;
    buf[i++] = (loc3DCorr.verBuf[2]>>0)&0xff;
    buf[i++] = (loc3DCorr.verBuf[3]>>8)&0xff;
    buf[i++] = (loc3DCorr.verBuf[3]>>0)&0xff;
    buf[i++] = (loc3DCorr.verBuf[4]>>8)&0xff;
    buf[i++] = (loc3DCorr.verBuf[4]>>0)&0xff;
    buf[i++] = (loc3DCorr.verBuf[5]>>8)&0xff;
    buf[i++] = (loc3DCorr.verBuf[5]>>0)&0xff;
    buf[i++] = (loc3DCorr.verBuf[6]>>8)&0xff;
    buf[i++] = (loc3DCorr.verBuf[6]>>0)&0xff;
    buf[i++] = (loc3DCorr.verBuf[7]>>8)&0xff;
    buf[i++] = (loc3DCorr.verBuf[7]>>0)&0xff;
    buf[i++] = (loc3DCorr.verBuf[8]>>8)&0xff;
    buf[i++] = (loc3DCorr.verBuf[8]>>0)&0xff;
    buf[i++] = (loc3DCorr.verBuf[9]>>8)&0xff;
    buf[i++] = (loc3DCorr.verBuf[9]>>0)&0xff;
    
    buf[i++] = (loc3DCorr.camViewBuf[0]>>8)&0xff;
    buf[i++] = (loc3DCorr.camViewBuf[0]>>0)&0xff;
    buf[i++] = (loc3DCorr.camViewBuf[1]>>8)&0xff;
    buf[i++] = (loc3DCorr.camViewBuf[1]>>0)&0xff;
    buf[i++] = (loc3DCorr.camViewBuf[2]>>8)&0xff;
    buf[i++] = (loc3DCorr.camViewBuf[2]>>0)&0xff;
    buf[i++] = (loc3DCorr.camViewBuf[3]>>8)&0xff;
    buf[i++] = (loc3DCorr.camViewBuf[3]>>0)&0xff;
    buf[i++] = (loc3DCorr.camViewBuf[4]>>8)&0xff;
    buf[i++] = (loc3DCorr.camViewBuf[4]>>0)&0xff;
    buf[i++] = (loc3DCorr.camViewBuf[5]>>8)&0xff;
    buf[i++] = (loc3DCorr.camViewBuf[5]>>0)&0xff;
    buf[i++] = (loc3DCorr.camViewBuf[6]>>8)&0xff;
    buf[i++] = (loc3DCorr.camViewBuf[6]>>0)&0xff;
    buf[i++] = (loc3DCorr.camViewBuf[7]>>8)&0xff;
    buf[i++] = (loc3DCorr.camViewBuf[7]>>0)&0xff;
    buf[i++] = (loc3DCorr.camViewBuf[8]>>8)&0xff;
    buf[i++] = (loc3DCorr.camViewBuf[8]>>0)&0xff;
    buf[i++] = (loc3DCorr.camViewBuf[9]>>8)&0xff;
    buf[i++] = (loc3DCorr.camViewBuf[9]>>0)&0xff;
    EEPROM_I2C_WriteBuffer(LOC_3D_CORR_ADDR,buf,i);
}
void Loc3DCorrParaRead()
{
    unsigned char buf[80];
    EEPROM_I2C_ReadBuffer(LOC_3D_CORR_ADDR, buf, 64);
    if(buf[0] == 0xff)
    {
        Loc3DCorrParaDefault();
    }
    else 
    {
        loc3DCorr.horCorrDirec  = (LOC_3D_CORR_E)buf[0];
        loc3DCorr.verCorrDirec  = (LOC_3D_CORR_E)buf[1];  
        loc3DCorr.validNum      = buf[2];
        loc3DCorr.prop          = buf[3];
        
        loc3DCorr.horBuf[0]     = (buf[4]<<8)|buf[5];
        loc3DCorr.horBuf[1]     = (buf[6]<<8)|buf[7];
        loc3DCorr.horBuf[2]     = (buf[8]<<8)|buf[9];
        loc3DCorr.horBuf[3]     = (buf[10]<<8)|buf[11];
        loc3DCorr.horBuf[4]     = (buf[12]<<8)|buf[13];
        loc3DCorr.horBuf[5]     = (buf[14]<<8)|buf[15];
        loc3DCorr.horBuf[6]     = (buf[16]<<8)|buf[17];
        loc3DCorr.horBuf[7]     = (buf[18]<<8)|buf[19];
        loc3DCorr.horBuf[8]     = (buf[20]<<8)|buf[21];
        loc3DCorr.horBuf[9]     = (buf[22]<<8)|buf[23];
        
        loc3DCorr.verBuf[0]     = (buf[24]<<8)|buf[25];
        loc3DCorr.verBuf[1]     = (buf[26]<<8)|buf[27];
        loc3DCorr.verBuf[2]     = (buf[28]<<8)|buf[29];
        loc3DCorr.verBuf[3]     = (buf[30]<<8)|buf[31];
        loc3DCorr.verBuf[4]     = (buf[32]<<8)|buf[33];
        loc3DCorr.verBuf[5]     = (buf[34]<<8)|buf[35];
        loc3DCorr.verBuf[6]     = (buf[36]<<8)|buf[37];
        loc3DCorr.verBuf[7]     = (buf[38]<<8)|buf[39];
        loc3DCorr.verBuf[8]     = (buf[40]<<8)|buf[41];
        loc3DCorr.verBuf[9]     = (buf[42]<<8)|buf[43];
        
        loc3DCorr.camViewBuf[0]     = (buf[44]<<8)|buf[45];
        loc3DCorr.camViewBuf[1]     = (buf[46]<<8)|buf[47];
        loc3DCorr.camViewBuf[2]     = (buf[48]<<8)|buf[49];
        loc3DCorr.camViewBuf[3]     = (buf[50]<<8)|buf[51];
        loc3DCorr.camViewBuf[4]     = (buf[52]<<8)|buf[53];
        loc3DCorr.camViewBuf[5]     = (buf[54]<<8)|buf[55];
        loc3DCorr.camViewBuf[6]     = (buf[56]<<8)|buf[57];
        loc3DCorr.camViewBuf[7]     = (buf[58]<<8)|buf[59];
        loc3DCorr.camViewBuf[8]     = (buf[60]<<8)|buf[61];
        loc3DCorr.camViewBuf[9]     = (buf[62]<<8)|buf[63];
    }
    
    
}
S32 *Loc3DGetHorBuf()
{
	return loc3DCorr.horBuf;
}

S32 *Loc3DGetVerBuf()
{
	return loc3DCorr.verBuf;
}

S32 *Loc3DGetCamViewBuf()
{
	return loc3DCorr.camViewBuf;
}

S32 *Loc3DGetValidNum()
{
	return &loc3DCorr.validNum;
}

void Loc3DPrintf()
{


	if(loc3DCorr.horBuf[loc3DCorr.validNum] == 0)
	{
		loc3DCorr.horCorrDirec = LOC_3D_CORR_NEGETIVE;
	}
	else 
	{
		loc3DCorr.horCorrDirec = LOC_3D_CORR_POSITIVE;
	}

	if(loc3DCorr.verBuf[loc3DCorr.validNum] == 0)
	{
		loc3DCorr.verCorrDirec = LOC_3D_CORR_NEGETIVE;
	}
	else 
	{
		loc3DCorr.verCorrDirec = LOC_3D_CORR_POSITIVE;
	}

}

