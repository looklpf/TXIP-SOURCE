#ifndef __TTL_UART_h
#define __TTL_UART_h

#include "protocol_type.h" 

#define PT_ADDR 1

extern unsigned char CAMERA_ZOOM_DATA_INQ_Flag;
extern unsigned int CAMERA_ZOOM_POSITION_DATA;


void TTL_UART_Init(unsigned short baud);
int TTL_UART_SendData(void*buf,int len);
unsigned int Get_Camera_Zoom_Position(void);
void Set_Camera_Zoom_Position(void);

void Set_Camera_Black(void);
void Set_Camera_Fog_Off(void);
void Set_Camera_Fog_ON(void);

void Camera_FocusNear(void);
void Camera_FocusFar(void);
void Camera_Focus_stop(void);
void Camera_ZoomTele(void);
void Camera_ZoomWide(void);
void Camera_Zoom_stop(void);

															
void Set_Camera_Black_To_Laser_Board(void);
void Set_Camera_Black_To_Laser_Colour(void);
                                                            
void CameraColor(void);
void CameraBlack(void);
void CamLocAd(int ad);
#endif
