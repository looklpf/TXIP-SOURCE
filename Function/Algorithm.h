#ifndef __ALGORITHM__
#define __ALGORITHM__



typedef struct
{
    void            (*CorrCamAdValue)       (char direction,unsigned short correctionAD);
    unsigned int    (*CalcCamAngle)         (unsigned int adValue);
    unsigned int    (*CalcCamAd)            (unsigned int angleValue);
    unsigned int    (*GetZoomInAd)          (void);
    unsigned int    (*GetZoomOutAd)         (void);


}ALG_T;


extern ALG_T *alg;






#endif


