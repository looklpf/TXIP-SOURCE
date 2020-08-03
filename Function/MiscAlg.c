#include "MiscAlg.h"

static unsigned int MiscUnIntAbs(unsigned int x,unsigned int y)
{
    if(x > y)
    {
        return x-y;
    }
    else 
    {
        return y-x;
    }
}
static unsigned short MiscUnShortAbs(unsigned short x,unsigned short y)
{
    if(x > y)
    {
        return x-y;
    }
    else 
    {
        return y-x;
    }
}
static unsigned char MiscPelcodCrc(unsigned char *p)
{
    return (*(p+1) + *(p+2) + *(p+3) + *(p+4) + *(p+5))&0xff;
}



MISC_ALG_T miscAlgInit = 
{
    .UnIntAbs       = MiscUnIntAbs,
    .UnShortAbs     = MiscUnShortAbs,
    .PelcodCrc      = MiscPelcodCrc,
   
};
MISC_ALG_T *pMiscAlg = &miscAlgInit;

