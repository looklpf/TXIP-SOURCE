#ifndef __MISC_ALG__
#define __MISC_ALG__



typedef struct
{
    unsigned int    (*UnIntAbs)     (unsigned int x,unsigned int y);
    unsigned short  (*UnShortAbs)   (unsigned short x,unsigned short y);
    
    unsigned char   (*PelcodCrc)    (unsigned char *p);
}MISC_ALG_T;


extern MISC_ALG_T *pMiscAlg;








#endif


