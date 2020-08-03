#ifndef __CURISE__
#define __CURISE__


typedef struct
{
    unsigned int  cnt;
    unsigned char enable;
    unsigned char presetNum;
    unsigned char timeReach;
}CURISE_FUN_T;



void CruiseInit(void);
void CuriseEnable(void);
void CuriseFun(void);
void CuriseTimeCnt(void);
unsigned char CuriseGetStatus(void);






#endif


