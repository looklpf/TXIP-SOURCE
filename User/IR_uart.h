#ifndef __IR_UART_H
#define __IR_UART_H
#include "camera_lense.h"

void IR_UART_Init(unsigned  int  baud);
void IRLense_Ctrl(CameraStruct_TypeDef *IR);

void command_autofocus(void);
void command_zoomwide(void);
void command_zoomtele(void);
void command_zoomstop(void);
void command_FocusNear(void);
void command_FocusFar(void);
void command_FocusStop(void);

void command_Set_FOV(unsigned short FOVValue);
void command_Inq_FOV(void);
void save_IrAlarm_seting(void);
void command_Inq_ZFPos(void);
unsigned short get_IR_ZoomPos(void);
unsigned short get_IR_FocusPos(void);
void setIRZOOM_FOCUS(unsigned short zoom, unsigned short focus);

void IrOpenLocViewAutoFocus(void);
void IrCloseLocViewAutoFocus(void);
void IrOpenZoomAutoFocus(void);
void IrCloseZoomAutoFocus(void);
void IrLocFocus(unsigned int val);
extern unsigned char  ZF_Inq_flag;

#endif
