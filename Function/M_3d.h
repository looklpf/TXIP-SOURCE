#ifndef __M_3D_H__
#define __M_3D_H__

#include "ipc_type.h"

//#ifdef __cplusplus
//#if __cplusplus
//extern "C"{
//#endif
//#endif /* End of #ifdef __cplusplus */



typedef enum
{
	LOC_3D_ZOOM_IN,
	LOC_3D_ZOOM_OUT,
	LOC_3D_MAX,
}LOC_3D_E;

typedef enum 
{
	LOC_3D_CORR_POSITIVE  = 1,
	LOC_3D_CORR_NEGETIVE  = 2,
}LOC_3D_CORR_E;

typedef struct 
{
	LOC_3D_E 	action;
	S16			xCoord;
    S16			yCoord;
    U16			xWidth;
    U16			yHeight;
	U16 		lensAngle;
}LOC_3D_VAL_T;

typedef struct 
{
    U32 basicHor;
    U32 basicVer;
    S32 horBuf[10];
    S32 verBuf[10];
    S32 camViewBuf[10];
	S32 validNum;
    U8  prop;
	LOC_3D_CORR_E  horCorrDirec;
	LOC_3D_CORR_E  verCorrDirec;
}LOC_3D_CORR_T;

void Loc3DCamAction(LOC_3D_VAL_T val3D);
void Loc3DIrAction(LOC_3D_VAL_T val3D);

S32  Loc3DCalcHorErr(S32 curAngle);
S32  Loc3DCalcVerErr(S32 curAngle);

void Loc3DSetBasicAngle(S32 basicHorAngle,S32 basicVerAngle);

S32 *Loc3DGetHorBuf(void);
S32 *Loc3DGetVerBuf(void);
S32 *Loc3DGetCamViewBuf(void);
S32 *Loc3DGetValidNum(void);
void Loc3DPrintf(void);



void Loc3DCoorChange(S16 x,S16 y,U16 w,U16 h,S16 *px,S16 *py,U16 *pw,U16 *ph);
void Loc3DSetHorCorrDirect(S8 direct);
void Loc3DSetVerCorrDirect(S8 direct);
void Loc3DCorrParaDefault(void);
void Loc3DCorrParaWrite(void);
void Loc3DCorrParaRead(void);


extern LOC_3D_CORR_T loc3DCorr;



//#ifdef __cplusplus
//#if __cplusplus
//}
//#endif
//#endif /* End of #ifdef __cplusplus */


#endif

