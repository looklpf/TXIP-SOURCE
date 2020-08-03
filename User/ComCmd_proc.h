#ifndef __COMCMD_PRO_H
#define __COMCMD_PRO_H 
#include "camera_lense.h" 
#include "IR_uart.h"
#include "PT_uart.h" 
#include "protocol_type.h" 
#include "SysCfg.h"

#define MAX_PRESET_NO 80


extern unsigned int IR_Lense_Angle_Data;


extern PelcodStruct_TypeDef TVC_Pelcod ;

extern CameraStruct_TypeDef CameraStruct;
extern CameraStruct_TypeDef IRStruct;
extern PTStruct_TypeDef PTStruct;


int ComCmd_Proc(void);

void ClearCmdComplexCnt(void);

void LocPtAngle(void);



typedef struct
{
    unsigned short cameraValue;
    unsigned short irValue;
}RETURN_VALUE_T;



void DataReturn(void);
char DevStatusCheck(void);

#endif
