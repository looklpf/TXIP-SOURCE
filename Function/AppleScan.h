#ifndef __APPLE_SCAN__
#define __APPLE_SCAN__


typedef struct
{
    unsigned int    cnt;
    unsigned char   enable;
    unsigned int    posFlag;
    unsigned char   timeReach;
    unsigned short  upBoard;
    unsigned short  stepAngle;
}APPLE_SCAN_FUN_T;


void AppleScanFun(void);
void AppleScanEnable(void);
void AppleScanInit(void);
void AppleScanSetLeftDownPos(void);
void AppleScanSetRightUpPos(void);
void AppleScanTimeCnt(void);
unsigned char AppleScanGetStatus(void);
void AppleScanSetStepAngle(unsigned char num);
#endif


