#ifndef __WATCH_HOME__
#define __WATCH_HOME__



typedef struct 
{
    unsigned int  cnt;
    unsigned int  actionTime;
    unsigned char actionFlag ; 
}WATCH_HOME_FUN_T;



void WatchHomeFun(void);
void WatchHomeSetType(unsigned char type);
void WatchHomeInit(void);
void WatchHomeTimeCnt(void);
void WatchHomeInit(void);

#endif


