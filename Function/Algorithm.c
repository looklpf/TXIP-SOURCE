#include "Algorithm.h"
#include "hp_conf.h"


static char cameraCorrectionDirection = 0;



#ifdef HDC2132
unsigned int cameraADValue[]=		
{
	0x4000,0x3EAF,0x3DC4,0x3BF3,0x3A10,
	0x37A2,0x3451,0x328B,0x2EF7,0x2B30,
	0x28A2,0x2581,0x22F3,0x1F3E,0x1D44,
    0x17F5,0x136D,0x0EE4,0x0A5C,0x0862,
    0x05D4,0x03D9,0x01DF,0x0
};

unsigned int cameraAngleValue[]=  
{
    153, 170, 194, 233, 273,
	306, 382, 441, 556, 706,
	814, 983, 1101,1314,1409,
	1732,2038,2357,2705,2753,
	3048,3228,3380,3525

};
#endif

#ifdef HDZ1930
unsigned int cameraADValue[]=		
{
    16384,15861,14600,13680,12351,
    11387,10316,9623,8791,7808,
    6321,5419,3642,2124,983,
    0,

};

unsigned int cameraAngleValue[]=  
{
    329,465,697,862,1127,
    1375,1679,1888,2179,2463,
    3058,3421,4141,4826,5437,
    5792,


};
#endif

#ifdef HDZ1933
unsigned int cameraADValue[]=		
{
	16384, 16271, 16183, 15996, 15858,
    15630, 15496, 14877, 14579, 13954,
    13500, 13036, 12457, 11475, 10075,
    8144 , 7283 , 5070 , 3686 , 2020 ,
    0
};

unsigned int cameraAngleValue[]=  
{
    241, 274, 300, 335, 365,
    400, 424, 513, 557, 657,
    738, 810, 943, 1177,1566,
    2189,2492,3329,3951,4631,
    5544
};
#endif

#ifdef HDZ2132SH
unsigned int cameraADValue[]=		
{
    0x4000,0x3F3F,0x3DE1,0x3C5E,0x3AA4,0x38A9,0x3726,0x3551,
    0x3316,0x30DB,0x2F58,0x2C93,0x29BB,0x2724,0x25B4,0x23FA,
    0x21F5,0x1FF2,0x1E38,0x1B57,0x182D,0x15DF,0x1348,0x101E,
    0x0D78,0x0B83,0x09C9,0x07C5,0x052E,0x0297,0x0000,
};
//角度值扩大100倍	
unsigned int cameraAngleValue[]=		
{
    139, 168, 195, 231, 267, 305,  339, 381, 
    437, 518, 559, 684, 816,919,983,1091,
    1216,1318,1424,1573,1785,1949,2155,2352,
    2548,2697,2838,3028,3234,3420,3687,
};
//unsigned int Visible_Lense_Focal_Length[]=
//{
//		256,210,180,151,130,113,101,90,
//		78,65,60,49,41,36,33,30,
//		27,24,23,20,18,16,15,14,
//		13,12,11,10,9,8,7
//};
#endif

#ifdef HDZ2150SH
unsigned int cameraADValue[]=		
{
    16384,16070,15207,13728,12361,
    11128,9783,8640,7206,5861,
    4696,3351,2006,0,

};
//角度值扩大100倍	
unsigned int cameraAngleValue[]=		
{
    98,172,309,481,667,
    832,1078,1368,1739,2401,
    2993,3776,4661,6450,

};
#endif


#ifdef HDZ1930SH
unsigned int cameraADValue[]=		
{
    16384,16116,15515,14099,11931,
    9657,7339,5021,2704,0,
};
//角度值扩大100倍	
unsigned int cameraAngleValue[]=		
{
    247,306,431,644,1079,
    1709,2362,3396,4288,5846,
};
#endif
#ifdef VSSCZ2035HB
unsigned int cameraADValue[]=		
{
    16384,15979,15049,13925,11282,
    8768,6189,3610,958,0,
};
//角度值扩大100倍	
unsigned int cameraAngleValue[]=		
{
    139,189,263,358,695,
    1175,1794,2504,3349,3651,
};
#endif

#ifdef HDZ2121SH
unsigned int cameraADValue[]=		
{
    16384,14357,12394,10412,8791,
    6895,5008,3116,1222,0,
};
//角度值扩大100倍	
unsigned int cameraAngleValue[]=		
{
    239,592,929,1447,1951,
    2637,3417,4253,5186,5934,

};
#endif









#ifdef  DS2CD4_HD3210R																	
unsigned int cameraADValue[]=		
{
    236,270,316,335,368,401,453,523,
    579,653,687,713,774,818,856,888,
    915,968,1062,1096,1164,1193,1274,1373,
    1434,1539,1594,1634,1747,1845,1898,1976,
    2061,2105,2182,2208,2295,2364,2442,2526,
    2580,2641,2691,2735,2800,2885,2946,3024,
    3094,3124,3191,3247,3296,3338,3372,3416,
    3462,3500,3558,3600,3682,3754,3816,3850,
};
unsigned int cameraAngleValue[]=		
{
    131,135,146,148,154,162,172,187,
    203,217,228,241,260,278,292,302,
    314,336,376,394,435,458,506,573,
    619,708,762,772,884,976,1040,1107,
    1185,1218,1285,1326,1414,1511,1583,1701,
    1718,1786,1844,1903,1997,2115,2195,2297,
    2384,2439,2528,2635,2694,2749,2801,2865,
    2946,2985,3081,3145,3284,3410,3514,3584,
};														
#endif





#ifdef  HDC208_TLZ15500DF28IRMP																		
unsigned int cameraADValue[]=		
{
    2985,2844,2636,2437,2220,
    1945,1744,1487,1280,1026,
    726,470,164
};
unsigned int cameraAngleValue[]=		
{
    82,133,216,316,433,
    623,769,993,1182,1439,
    1781,2097,2519,
};														
#endif




#ifdef  HDC208_HD60X12750DIR																		
unsigned int cameraADValue[]=		
{
    4032,3760,3544,3248,3096,
    2810,2623,2390,2135,1988,
    1710,1515,1251,1024,805,
    579,456,
};
unsigned int cameraAngleValue[]=		
{
    62,83,109,155,188,
    267,339,448,661,715,
    963,1167,1486,1822,2228,
    2679,2954,
};														
#endif

/*************************************************/
#ifdef HDC208_LMZ2075
														
unsigned int cameraADValue[]=		
{
    0x0d00,0x0c90,0x0c50,0x0c00,0x0bd0,0x0b70,0x0b30,0x0b10,
    0x0ad0,0x0a90,0x0a50,0x0a10,0x09c0,0x0980,0x0940,0x0880,
    0x0860,0x07f0,0x0780,0x0700,0x06c0,0x0660,0x05d0,0x0550,
    0x04c0,0x0450,0x03e0,0x0350,0x0300,
};
	
unsigned int cameraAngleValue[]=		
{
    55,	 63,  73,  90, 107, 148, 172, 193,
    221, 260, 294, 335, 374, 419, 461, 482, 
    508, 687, 788, 885, 945,1037,1168,1308,
    1436,1557,1680,1839,1954,
};	
#endif
#ifdef HDC208_HZ2075RDC
unsigned int cameraADValue[]=		
{
    0x006d,0x0088,0x00aa,0x00ee,0x014b,0x0193,0x01ee,0x021c,
    0x0284,0x02d9,0x0311,0x035b,0x0394,0x03d2,0x041c,0x0453,
    0x048a,0x04c6,0x04f9,0x054d,0x059e,0x05e4,0x0628,0x0681,
    0x06e3,0x072f,0x07a6,0x07e1,0x0838,0x0884,0x08f0,0x095b,
    0x09af,0x0a0f,0x0a60,0x0ac2,0x0b13,0x0b62,0x0bc0,0x0c0a,
    0x0c49,0x0cb2,0x0d0f,0x0d5b,0x0d9f,0x0df6,0x0e28,0x0e9a,
    0x0f05,0x0f3d,0x0f76,
};
unsigned int cameraAngleValue[]=		
{
    62,67,70,76,86,96,106,114,
    120,127,149,168,177,187,206,228,
    237,254,263,290,303,329,353,381,
    415,443,492,517,558,599,640,692,
    744,792,836,897,940,986,1056,1115,
    1171,1234,1301,1379,1425,1502,1550,1664,
    1755,1831,1885,
};
#endif
/**********************H62Z1235PDC ********************/
#ifdef HDC208_H62Z1235PDC															
unsigned int cameraADValue[]=		
{
    0x0eac,0x0e26,0x0d0a,0x0bbf,0x0b09,0x09c7,0x08b3,0x074b,
    0x0676,0x063c,0x054e,0x0482,0x0473,0x03cd,0x0346,0x017f,
    0x00df,0x00bc,
};
unsigned int cameraAngleValue[]=		
{
    58, 66,	98, 152, 196,  296, 438, 734,
    918, 1010, 1274, 1530, 1552,  1782, 1984, 2696,
    3004, 3208,
};	

#endif  

#ifdef  HDC208_Y34Z22RADP	//配HDC208  XS201804119 20180407													
unsigned int cameraADValue[]=		
{
    0x0ef7,0x0e98,0x0e37,0x0de1,0x0d8d,0x0d30,0x0cb4,0x0c7b,
    0x0c19,0x0bba,0x0b58,0x0af2,0x0a98,0x0a4b,0x09e5,0x0982,
    0x0928,0x08c7,0x086b,0x0804,0x07af,0x0757,0x06fd,0x069d,
    0x063c,0x05d8,0x0577,0x0516,0x04b5,0x0446,0x03e7,0x0388,
    0x032b,0x02c9,0x0269,0x020d,0x01b0,0x0160
};

unsigned int cameraAngleValue[]=		
{
    54,	 66,  67,  77,  87,  95, 107, 120, 
    135,149, 159, 180, 204, 224, 246, 274,
    303,331, 359, 395, 433, 471, 511, 562,
    603,659, 718, 783, 854, 933,1017,1095,
    1192,1289,1390,1503,1624,1726,
};		
#endif

#ifdef  HDC407_HD62X12775DIR													
unsigned int cameraADValue[]=		
{
    3984,3560,3170,2790,2400,
    2000,1279,900,600,239,

};

unsigned int cameraAngleValue[]=		
{
    79,94,151,244,378,
    587,1264,1801,2379,3238,

};		
#endif

#ifdef  HDC407_Y60Z16RHADPVT													
unsigned int cameraADValue[]=		
{
//    3761, 3458, 3106, 2900, 2663, 
//    2488, 2339, 2061, 1765, 1293, 
//    939,  590,  366
    
    3781,3426,3128,2853,2534,
    2221,1932,1607,1329,1052,
    714,314,

};

unsigned int cameraAngleValue[]=		
{
//    43,  57,  83,  104, 142,  
//    185,  240, 423, 688, 1196, 
//    1591, 2043, 2352
    46,64,86,118,186,
    320,574,904,1199,1524,
    1931,2486,

};		
#endif
/*************************************************************************/



static unsigned int AlgGetZoomInAd()   //长焦
{
    return cameraADValue[0];
}
static unsigned int AlgGetZoomOutAd()  //广角
{
    unsigned char arrayNumber = sizeof(cameraADValue)/4;
    return cameraADValue[arrayNumber-1];
}
static void AlgCorrCamAdValue(char direction,unsigned short correctionAD)
{
    char i ;
    unsigned char arrayNumber = sizeof(cameraADValue)/4;
//    unsigned int buf[arrayNumber];
    if(direction == 1)
    {
        cameraCorrectionDirection = 1;
//        for(i=0;i<arrayNumber;i++)
//        {
//            buf[i] = cameraADValue[arrayNumber-i-1];
//        }
//        for(i=0;i<arrayNumber;i++)
//        {
//            cameraADValue[i] = buf[i];
//        }
    }
    else
    {
        cameraCorrectionDirection = 0;
    }
    
    
    
    
    if(correctionAD > cameraADValue[0])
    {
        correctionAD = correctionAD - cameraADValue[0];
        for(i=0;i<arrayNumber;i++)
        {
            cameraADValue[i] += correctionAD;
        }
    }
    else 
    {
        correctionAD = cameraADValue[0] - correctionAD;
        for(i=0;i<arrayNumber;i++)
        {
            cameraADValue[i] -= correctionAD;
        }
    }
    
}


static unsigned int AlgCalcCamAngle(unsigned int adValue)
{
	unsigned int  i,n1,n2,n3;
	unsigned char left,right;
	unsigned int  angleValue;
	float k1;
    unsigned char arrayNumber = sizeof(cameraADValue)/4;
    if(cameraCorrectionDirection == 0)
    {
        if(adValue >= cameraADValue[0])
        {
            return cameraAngleValue[0];
        }
        if(adValue <= cameraADValue[arrayNumber-1])
        {
            return cameraAngleValue[arrayNumber-1];
        }
        for(i=1;i<arrayNumber;i++)
        {
            
            if(adValue < cameraADValue[i])
            {
                
            }
            else
            {
                left  = i-1;
                right = i;
                break;
            }

        }

        n1 = adValue - cameraADValue[right];
        n2 = cameraADValue[left] - cameraADValue[right];
        k1 = (float )n1/(float)n2;	
        n3 = (float )(cameraAngleValue[right] - cameraAngleValue[left]);
        
        angleValue = cameraAngleValue[right] - (unsigned int )(k1*n3);
    }
    else
    {
        if(adValue <= cameraADValue[0])
        {
            return cameraAngleValue[0];
        }
        if(adValue >= cameraADValue[arrayNumber-1])
        {
            return cameraAngleValue[arrayNumber-1];
        }
        for(i=1;i<arrayNumber;i++)
        {
            
            if(adValue > cameraADValue[i])
            {
                
            }
            else
            {
                left  = i-1;
                right = i;
                break;
            }

        }

        n1 = cameraADValue[right] - adValue;
        n2 = cameraADValue[right] - cameraADValue[left];
        k1 = (float )n1/(float)n2;	
        n3 = (float )(cameraAngleValue[right] - cameraAngleValue[left]);
        
        angleValue = cameraAngleValue[right] - (unsigned int )(k1*n3);
    }
	return angleValue;	
}



static unsigned int AlgCalcCamAd(unsigned int angleValue)
{
	unsigned int i,n1,n2,n3;
	unsigned char left,right;
	unsigned int adValue;
	float k1;
    unsigned char arrayNumber = sizeof(cameraADValue)/4;
	if(angleValue >= cameraAngleValue[arrayNumber-1])
	{
		return cameraADValue[arrayNumber-1];
	}
	if(angleValue <= cameraAngleValue[0])
	{
		return cameraADValue[0];
	}
	for(i=1;i<arrayNumber;i++)
	{
		if(angleValue > cameraAngleValue[i])
		{
			 
		}
		else
		{
			left  = i-1;
			right = i;
			break;
		}	
	}
    
	n1 = cameraAngleValue[right] - angleValue;
	n2 = cameraAngleValue[right] - cameraAngleValue[left];
	k1 = (float )n1/(float)n2;	
    if(cameraCorrectionDirection == 0)
    {
        n3 = (float)(cameraADValue[left] - cameraADValue[right]);
        
        adValue = cameraADValue[right] + k1*n3;
    }
    else
    {
        n3 = (float)(cameraADValue[right] - cameraADValue[left]);
        
        adValue = cameraADValue[right] - k1*n3;
    }

	return adValue;	
}





ALG_T algInit = 
{
    .CorrCamAdValue =   AlgCorrCamAdValue,
    .CalcCamAngle   =   AlgCalcCamAngle,
    .CalcCamAd      =   AlgCalcCamAd,
    .GetZoomInAd    =   AlgGetZoomInAd,
    .GetZoomOutAd   =   AlgGetZoomOutAd,

};
ALG_T *alg = &algInit;

