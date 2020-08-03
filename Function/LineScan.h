#ifndef __LINE_SCAN__
#define __LINE_SCAN__

typedef struct
{
    unsigned int  cnt;
    unsigned char enable;
    unsigned int  posFlag;
    unsigned char timeReach;
}LINE_SCAN_FUN_T;


void LineScanEnable(void);
void LineScanFun(void);
void LineScanFunInit(void);
unsigned char LineScanGetStatus(void);
void LineScanTimeCnt(void);
void LineScanSetLeftPos(void);
void LineScanSetRightPos(void);
#endif


