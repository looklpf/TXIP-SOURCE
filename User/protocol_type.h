#ifndef __PROTOCOL_H
#define __PROTOCOL_H 

#define PELCOD_STX	 0xFF  //Pelco_dͷ��־
#define PELCOD_LEN   7 	   //Pelco_dЭ�鳤��
#define PELCOP_STX   0xA0  //Pelco_pͷ��־
#define PELCOP_ETX	 0xAF  //Pelco_p������־
#define PELCOP_LEN   8	   //Pelco_pЭ�鳤��
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
