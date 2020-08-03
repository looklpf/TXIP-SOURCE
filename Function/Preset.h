#ifndef __PRESET__
#define __PRESET__

typedef enum
{
    COMPLEX_NONE,
    COMPLEX_CURISE_SET_MINUTE,
    COMPLEX_CURISE_SET_SECOND,
    COMPLEX_CURISE_SET_SPEED,
    COMPLEX_LASER_FULL,
    COMPLEX_LASER_FIR,
    COMPLEX_FOCUS_SPEED,
    COMPLEX_WATCH_HOME,
    COMPLEX_APPLE_SCAN_SET_STEP_ANGLE,
    COMPLEX_TEST,
}CMD_COMPLEX_E;

extern CMD_COMPLEX_E cmdComplex;

void CallPresetsAnalysis(unsigned char number);
void SetPresetsAnalysis(unsigned char number);
void DeletePresetsAnalysis(unsigned char number);
unsigned char  call_preset(unsigned char PresetNo, unsigned char speed);
unsigned char  LocAngle(unsigned int hp,unsigned int vp,unsigned char speed);
void SetComplexPresetsAnalysis(unsigned char num);
void save_preset(unsigned char PresetNo);
unsigned char  CallPtHorPreset(unsigned char PresetNo, unsigned char speed,unsigned char dir);
unsigned char  CallPtAbsVerPos(unsigned int pos, unsigned char speed,unsigned char dir);
unsigned char  CallPreset(unsigned char PresetNo, unsigned char speed);
unsigned char  CallPtVerPos(unsigned int pos, unsigned char speed,unsigned char dir);
int PtzZeroAngleCal_Call_H(int Angle,int zeroAngle,int overInt);
int PtzZeroAngleCal_Call_V(int AngleVertical,int AngleStep,int overInt);
#endif


