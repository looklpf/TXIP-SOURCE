#ifndef LaserFunction__
#define LaserFunction__

extern volatile unsigned int time2_Laser_Lens_Sleep_cnt;
extern unsigned char Laser_Lens_Sleep_Flag;
//LASER_VALUE_T laserValue = 
//{
//    0,
//    #ifdef LASER_12_80_MM
//    16000, //self test range
//    #endif
//    0,
//    0,
//    0,
//    0,
//    0,
//    0,
//    1,
//    0,
//    2316,  //open  laser Threshold
//    1778,  //close laser Threshold
//    1,
//    500,  // stepper motor border remove step numbers
//    0,
//    0,
//};
typedef struct 
{
    long int        limitMin;									//激光步数最小值
    long int        limitMax;									//激光步数最大值
    long int        position;									//当前激光位置
    long int        endPosition;
    char            moveFlag;
    char            positionFlag;
    char            status;										//激光开关状态
    char            direction;
    char            synchronizeFlag;
    char            touchOffFlag;
    unsigned int    lightSensorOnThreshold;		//激光开启光敏阈值
    unsigned int    lightSensorOffThreshold;	//激光关闭光敏阈值  
    char            lightSensorAuto;
    unsigned int    removeStep;
    signed int      errorFullStep;						//同步整体矫正值
    signed int      errorFirstStep;						//同步首步矫正值
    unsigned char     errorDir;
    unsigned char  errorFdir;
}LASER_VALUE_T;


typedef struct
{
    void        (*Init              )       (void               );
    void        (*Move              )       (void               );
    void        (*Open              )       (void               );
    void        (*Close             )       (void               );
    void        (*Auto              )       (void               );
    void        (*Stop              )       (void               );
    void        (*Big               )       (void               );
    void        (*Small             )       (void               );
    void        (*SeltTest          )       (void               );
    void        (*GoTo              )       (long int position  );
    void        (*Synchronize       )       (void               );
    void        (*SynchronizeEnable )       (void               );
    void        (*SynchronizeDisable)       (void               );
    void        (*SetTouchOff       )       (char value         );
    void        (*AutoFunction      )       (void               );
    void        (*FullError         )       (void               );
    void        (*FirstError        )       (void               );
    void        (*Read)                     (void               );
    void        (*CorrectionCameraAD)       (unsigned short correctionAD);
    
}LASER_T;

extern LASER_T *pLaser;



#endif
