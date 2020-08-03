#ifndef __MSGQUEUE_H
#define __MSGQUEUE_H

#include "stm32f10x.h"

typedef struct 
{
	__IO uint8_t msg_proto;
	__IO uint8_t msg_length;
	__IO uint8_t msg_data[50];
}MSGQUEUE;


void Join_Queque (uint8_t proto, uint8_t length, uint8_t *msg_data);
MSGQUEUE Get_Queue(void);


#endif
