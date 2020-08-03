#ifndef LensDriver__
#define LensDriver__


#include "stm32f10x.h"



typedef struct
{
    void            (*Wide)                 (void);
    void            (*Tele)                 (void);
    void            (*Far)                  (void);
    void            (*Near)                 (void);
    void            (*ZoomStop)             (void);
    void            (*FocusStop)            (void);
    unsigned short  (*ReadZoomValue)        (void);
    unsigned short  (*ReadFocusValue)       (void);
}Camera_T;

typedef struct
{
    void            (*Wide)                 (void);
    void            (*Tele)                 (void);
    void            (*Far)                  (void);
    void            (*Near)                 (void);
    void            (*ZoomStop)             (void);
    void            (*FocusStop)            (void);
    unsigned short  (*ReadZoomValue)        (void);
    unsigned short  (*ReadFocusValue)       (void);
}IR_T;

typedef struct
{
    void                (*init)                 (void);
    Camera_T            *Camera;
    IR_T                *IR;
    unsigned short      (*ReadLightSensorValue) (void);
}LENS_T;

extern LENS_T *LENS;

void CamReadFocusSpeed(void);
void CamSetFocusSpeed(unsigned char val);
void CamFocusPwmCnt(void);
#endif
