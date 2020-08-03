#ifndef __TVC_UART_H
#define __TVC_UART_H 	

#define BUF_LEN  20  

extern volatile unsigned char 	TVC_UART_RxBuf[BUF_LEN];	  
extern unsigned char 		    VC_UART_TxBuf[BUF_LEN];		
extern unsigned volatile char 	TVC_UART_RecCnt;		    
extern volatile unsigned  char 	TVC_UART_ProtoFlag;

void TVC_UART_Init(unsigned short baud);
int TVC_UART_SendData(void*buf,int len);
#endif

