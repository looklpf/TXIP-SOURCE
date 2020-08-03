#ifndef __CAMERA_LENSE_H
#define __CAMERA_LENSE_H
#define CAMERA_LENSE
#define FOCUS_HI_SPEED 3
#define FOCUS_MD_SPEED 2
#define FOCUS_LO_SPEED 1
typedef struct
{
    volatile  unsigned char ZoomWide_Flag;                                        
    volatile  unsigned char ZoomTele_Flag;
    volatile  unsigned char FocusNear_Flag;
    volatile  unsigned char FocusFar_Flag;
    volatile  unsigned char IrisOpen_Flag;  
    volatile  unsigned char IrisClose_Flag;
    volatile  unsigned char ZoomSpeed;
    volatile  unsigned char FocusSpeed;
    volatile  unsigned short ZoomCurPosition;
    volatile  unsigned short FocusCurPosition;
    volatile  unsigned short ZoomMaxPosition;
    volatile  unsigned short ZoomMinPosition;
    volatile  unsigned short ZoomSetPosition;
    volatile  unsigned short FocusSetPosition;
    volatile  unsigned char ZoomSet_Flag;
    volatile  unsigned char FocusSet_Flag;
    unsigned char WideInc_flag;
    unsigned char NearInc_flag;
    unsigned char IR_WideInc_flag;
    unsigned char IR_NearInc_flag;
    unsigned char ZoomCurBigerSet_flag;//if (ZoomCurPosition > ZoomSetPosition)  ZoomBiger_flag=1
    unsigned char ZoomCurSmallerSet_flag;
    unsigned char FocusCurBigerSet_flag;
    unsigned char FocusCurSmallerSet_flag;
    volatile  unsigned char FocusNear_RunFlag;
    volatile  unsigned char FocusFar_RunFlag;
    volatile  unsigned char FCSaveFlag;
} CameraStruct_TypeDef;
extern unsigned short Zoom_Count_Cam,Zoom_Count_IR;  //20180831
extern unsigned short CamZoomCurPosPre;
extern unsigned int IR_correct_AD;
extern unsigned char IR_correct_flag;
static unsigned short CamZoom_time_cnt = 0;
extern volatile unsigned short ADC1_ConvertedBuf[4];
extern unsigned short ZoomAD[];
extern unsigned short LenseAngleX100[];
extern unsigned char init_fc_value;
extern unsigned char IR_init_fc_value;
void CameraLense_Init(void);

void CameraLense_Ctrl(CameraStruct_TypeDef *camera_ctrl);
void Set_Camera_FocusSpeed(unsigned char speed);
void Go_CameraLense_Position(CameraStruct_TypeDef *camera_ctrl);
void Go_IRLense_Position(CameraStruct_TypeDef *camera_ctrl);
unsigned char camera_lense_selftest(CameraStruct_TypeDef *camera_ctrl);
unsigned char IR_lense_selftest(CameraStruct_TypeDef *camera_ctrl);
unsigned short filter1_fine_CAMZOOM(void);  //20180822

void CamLocPos(CameraStruct_TypeDef *camera_ctrl);
void IrLocPos(CameraStruct_TypeDef *camera_ctrl);
#endif
