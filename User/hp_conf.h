#ifndef __HP_CONF_H
#define __HP_CONF_H


/***********************���ֶ����ò���*********************/
//3Dʹ��
#define LOC_3D_EN

//���⹦��ʹ��
//#define LASER_EN  

//��������ʹ��
//#define IR_ZOOM_EN


//JEC7250 ������̨ʹ��
//#define YT_JEC_EN



//һ����ͺ�
//#define HDC2132   //DS-2ZMN3207
//#define HDZ1933
//#define HDZ1930    // DS-2ZMN3007
//#define HDZ2132SH
//#define HDZ2150SH
//#define HDZ1930SH
//#define VSSCZ2035HB
//#define HDZ2121SH

//ǹ���ͺ�
//#define HDC208_H62Z1235PDC   
#define HDC208_Y34Z22RADP	    
//#define HDC208_LMZ2075        //KOWA 2075
//#define HDC208_HD60X12750DIR  //HD62X-12775-DIR
//#define HDC208_TLZ15500DF28IRMP
//#define HDC208_HZ2075RDC
//#define DS2CD4_HD3210R
//#define HDC407_HD62X12775DIR
//#define HDC407_Y60Z16RHADPVT
/**********************************************************/












/*******************���²��������ֶ�����******************/
#ifdef HDC2132
    #define IPC_EN
#endif
#ifdef HDZ1933
    #define IPC_EN
#endif
#ifdef HDZ1930
    #define IPC_EN
#endif
#ifdef HDZ2132SH
    #define IPC_EN
#endif
#ifdef HDZ2150SH
    #define IPC_EN
#endif
#ifdef HDZ1930SH
    #define IPC_EN
#endif
#ifdef VSSCZ2035HB
    #define IPC_EN
#endif
#ifdef HDZ2121SH
    #define IPC_EN
#endif
#endif
