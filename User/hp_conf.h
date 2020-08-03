#ifndef __HP_CONF_H
#define __HP_CONF_H


/***********************需手动配置参数*********************/
//3D使能
#define LOC_3D_EN

//激光功能使能
//#define LASER_EN  

//定焦热像使能
//#define IR_ZOOM_EN


//JEC7250 侧载云台使能
//#define YT_JEC_EN



//一体机型号
//#define HDC2132   //DS-2ZMN3207
//#define HDZ1933
//#define HDZ1930    // DS-2ZMN3007
//#define HDZ2132SH
//#define HDZ2150SH
//#define HDZ1930SH
//#define VSSCZ2035HB
//#define HDZ2121SH

//枪机型号
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












/*******************以下参数无需手动配置******************/
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
