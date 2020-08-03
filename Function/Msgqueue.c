#include <string.h>
#include "msgqueue.h"

#define QUEUE_NUM		20
MSGQUEUE msgqueue[QUEUE_NUM] = {{0x00,0x00,0x00}};

__IO uint8_t jq_p = 0;
__IO uint8_t gq_p = 0;
__IO uint8_t queue_cnt = 0;

void Join_Queque(uint8_t proto, uint8_t length, uint8_t *msg_data)
{
	uint8_t i = 0;
	if(jq_p >= QUEUE_NUM)
	{
		 jq_p= 0;
	}
	
	queue_cnt++;
	if(queue_cnt > QUEUE_NUM)
	{
		jq_p = 0;  //如果存储长度越界，存储速度超过获取速度，则置零重新开始。
		gq_p = 0;
		queue_cnt = 1;
	}
	msgqueue[jq_p].msg_proto 	= proto;
	msgqueue[jq_p].msg_length = length;
	
	for(i=0;i<length;i++)
	{
		msgqueue[jq_p].msg_data[i] 	= msg_data[i];
	}
	
	jq_p++;
}

MSGQUEUE Get_Queue(void)								//FIFO
{
	MSGQUEUE queue = {0x00,0x00,0x00};
	memset(&queue,0,sizeof(MSGQUEUE));
	if(gq_p >= QUEUE_NUM)
	{
		gq_p = 0;
	}
	/*
	if(!queue_cnt)
	{
		if(jq_p!=gq_p)
		{
			queue = msgqueue[gq_p];
		  gq_p++;
	  }
  }	
	else
	*/
	if(queue_cnt)
	{
		queue_cnt--;
		queue = msgqueue[gq_p];
		gq_p++;
	}
	
	return queue;
}



