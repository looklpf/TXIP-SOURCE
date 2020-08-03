#ifndef __PT_UART_h
#define __PT_UART_h
#include "camera_lense.h"

#define PT_ADDR 	1
#define PT_RIGHT 	0x02 
#define PT_LEFT  	0x04 
#define PT_UP    	0x08 
#define PT_DOWN  	0x10

typedef struct
{
    volatile  unsigned char 	PanCtrl;                                        
    volatile  unsigned char 	TiltCtrl;

    volatile  unsigned char 	PanSpeed;		    //ˮƽ�ٶ�
    volatile  unsigned char 	TiltSpeed; 			//�����ٶ�
    volatile  unsigned short 	PanCurAngle;		//ˮƽ�Ƕ�
    volatile  unsigned short 	TiltCurAngle;		//�����Ƕ�
    volatile  unsigned short 	PanOriAngle;		//ˮƽԭ��Ƕ�
    volatile  unsigned short 	TiltOriAngle;		//����ԭ��Ƕ�
    volatile  unsigned short 	PanSetAngle;
    volatile  unsigned short 	TiltSetAngle;
    volatile  unsigned char 	PanAngleSet_Flag;		//ˮƽԤ��λ��Ƕȶ�λ��ʶ��1��ʾˮƽ���ڶ�λ���У�0��ʾˮƽ����ָ��λ��
    volatile  unsigned char 	TiltAngleSet_Flag;	    //����Ԥ��λ��Ƕȶ�λ��ʶ��ͬ��
    volatile  unsigned char 	Pan_PresetSpeedSet_Flag;
    volatile  unsigned char 	Tilt_PresetSpeedSet_Flag;
    volatile  unsigned char 	PresetSpeed;
} PTStruct_TypeDef;


extern unsigned char 	        PT_UART_TxBuf[7];	    //PT_UART���ͻ�����
extern volatile unsigned  char 	PT_UART_ProtoType;	   //PT_UARTͨ��Э��
extern unsigned int 		    PanRevCnt		;
extern unsigned int 		    TiltRevCnt		;


void 	PT_UART_Init(unsigned short baud); 
int 	PT_UART_SendData(void*buf,int len); 
void 	PT_ctrl(PTStruct_TypeDef *PTCtrl,CameraStruct_TypeDef *camera);
void 	PT_SetPreset(unsigned char PresetNo);
void 	PT_ClearPreset(unsigned char PresetNo);
void 	PT_CallPreset(unsigned char PresetNo);
void 	PT_CallPreset_WithSpeedSet(PTStruct_TypeDef *PT);
void 	PT_SetTiltAngle(PTStruct_TypeDef *PT);
void 	PT_SetPanAngle(PTStruct_TypeDef *PT);

void    PtSetHorAngle(unsigned int angle);
void    PtSetVerAngle(unsigned int angle);
void    PtSetHorZero(void);
void    PtSetVerZero(void);
void    PtDefaultHorZero(void);
void    PtDefaultVerZero(void);
void    PtRestart(void);


void PtUp(unsigned char speed);
void PtDown(unsigned char speed);
void PtLeft(unsigned char speed);
void PtRight(unsigned char speed);
void PtLeftUp(unsigned char hSpeed,unsigned char vSpeed);
void PtLeftDown(unsigned char hSpeed,unsigned char vSpeed);
void PtRightUp(unsigned char hSpeed,unsigned char vSpeed);
void PtRightDown(unsigned char hSpeed,unsigned char vSpeed);
void PtStop(void);


#endif 
