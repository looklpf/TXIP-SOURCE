#ifndef __VC_UART_H
#define __VC_UART_H 	

#define BUF_LEN  20  

extern volatile unsigned char TVC_UART_RxBuf[BUF_LEN];	  //TVC_UART接收缓冲区
extern unsigned char TVC_UART_TxBuf[BUF_LEN];			  //TVC_UART发送缓冲区
extern unsigned volatile char TVC_UART_RecCnt;			  //TVC_UART接收数据计数
extern volatile unsigned  char TVC_UART_ProtoFlag;

void VC_UART_Init(unsigned short baud);
int VC_UART_SendData(void*buf,int len);
void CamAutoFocusClose(void);
void CamAutoFocusOpen(void);
void CamAutoFocusOnce(void);
void CamReadAutoFocus(void);
void CamAutoFocusCloseNoSave(void);
void CamAutoFocusOpenNoSave(void);
extern char camAutoFocusFlag ;
#endif
