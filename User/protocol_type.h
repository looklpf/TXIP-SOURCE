#ifndef __PROTOCOL_H
#define __PROTOCOL_H 

#define PELCOD_STX	 0xFF  //Pelco_d头标志
#define PELCOD_LEN   7 	   //Pelco_d协议长度
#define PELCOP_STX   0xA0  //Pelco_p头标志
#define PELCOP_ETX	 0xAF  //Pelco_p结束标志
#define PELCOP_LEN   8	   //Pelco_p协议长度
#define BP_STX       0xF0
#define BP_ETX       0xFF
#define BP_LEN    
#define PELCO_D 1
#define PELCO_P 2
#define BP_PROTO     3

typedef struct
{
    unsigned char addr;                                        
    unsigned char cmd1;
    unsigned char cmd2;
    unsigned char data1;
    unsigned char data2;  
    unsigned char crc;
    unsigned char ExtendCmd_flag;
} PelcodStruct_TypeDef;




#endif
