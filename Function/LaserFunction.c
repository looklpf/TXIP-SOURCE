#include "LaserFunction.h"
#include "stm32f10x_gpio.h"
#include "LensDriver.h"
#include "eeprom_i2c.h"
#include "TTL_uart.h"
#include "MiscAlg.h"
#include "hp_conf.h"
#define LASER_CTR_DIR_EN  


//#define LMZ2075_LASER_12_80_808 
//#define HD60X12750_LASER_12_80_940
//#define HD60X12750_LASER_12_80
//#define Y34Z22RADP_LASER_12_80
//#define HDZ1922_LASER_30_30
//#define HDC2132_LASER_08_20
#define HDZ2150_LASER_10_50

#define SLEEP_H         do {GPIO_SetBits(GPIOE,GPIO_Pin_0)   ;} while(0);
#define SLEEP_L         do {GPIO_ResetBits(GPIOE,GPIO_Pin_0) ;} while(0);

#ifdef LASER_CTR_DIR_EN
#define DIR_L           do {GPIO_ResetBits(GPIOE,GPIO_Pin_1);} while(0);
#define DIR_H           do {GPIO_SetBits(GPIOE,GPIO_Pin_1)  ;} while(0);
#else
#define DIR_H           do {GPIO_ResetBits(GPIOE,GPIO_Pin_1);} while(0);
#define DIR_L           do {GPIO_SetBits(GPIOE,GPIO_Pin_1) ;} while(0);
#endif

#define STEP_H          do {GPIO_SetBits(GPIOB,GPIO_Pin_9)   ;} while(0);
#define STEP_L          do {GPIO_ResetBits(GPIOB,GPIO_Pin_9) ;} while(0);
#define POWER_OPEN      do {GPIO_ResetBits(GPIOE,GPIO_Pin_10);} while(0);
#define POWER_CLOSE     do {GPIO_SetBits(GPIOE,GPIO_Pin_10)  ;} while(0);

#define CAMERA_COLOR    do {GPIO_SetBits(GPIOE,GPIO_Pin_9)   ;} while(0);
#define CAMERA_BLACK    do {GPIO_ResetBits(GPIOE,GPIO_Pin_9) ;} while(0);

//#define POWER_OPEN      do {} while(0);  //test with no laser power on
    
    
#define POSITION_ERROR  100

/***************** To be optimized ******************/
char laserStatusChange = 0;
extern void delay_ms(u16 nms);		    								   
extern void delay_us(u32 nus);
extern int laserCheckCount;
/****************************************************/
volatile unsigned int   time2_Laser_Lens_Sleep_cnt  =	0;
unsigned char           Laser_Lens_Sleep_Flag       =	0;
LASER_VALUE_T laserValue = 
{
    .limitMin = 0,
    .limitMax = 15000, //self test range
    .position = 0,
    .endPosition = 0,
    .moveFlag = 0,
    .positionFlag = 0,
    .status = 0,
    .direction = 0,
    .synchronizeFlag = 1,
    .touchOffFlag = 0,
    .lightSensorOnThreshold = 2316,  //open  laser Threshold
    .lightSensorOffThreshold = 1778,  //close laser Threshold
    .lightSensorAuto = 1,
    .removeStep = 500,  // stepper motor border remove step numbers
    .errorFullStep = 0,
    .errorFirstStep = 0,
    .errorDir = 0,
    .errorFdir = 0,
    
};



#ifdef HD60X12750_LASER_12_80_940
unsigned int cameraAD[]=		
{
    0x0f22,0x0efd,0x0eb0,0x0e73,0x0e38,0x0df9,0x0dc6,0x0ddf,
    0x0d8f,0x0d62,0x0d24,0x0ce6,0x0cb6,0x0c88,0x0c40,0x0bef,
    0x0b83,0x0b34,0x0af1,0x0a94,0x0a3a,0x0a0b,0x09c6,0x0980,
    0x0921,0x08db,0x0876,0x083c,0x07f6,0x078f,0x072d,0x06df,
    0x0660,0x0632,0x05a9,0x052a,0x04cb,0x047c,0x03f7,0x03b4,
    0x0334,	
};
unsigned int laserStep[]=	
{ 
    0x0120,0x0160,0x01c0,0x0220,0x0260,0x02e0,0x0360,0x03a0,
    0x0440,0x04a0,0x0540,0x05c0,0x0620,0x0660,0x06c0,0x0720,
    0x07a0,0x0800,0x0840,0x08a0,0x08e0,0x0920,0x0960,0x09a0,
    0x0a20,0x0aa0,0x0b40,0x0bc0,0x0c40,0x0d00,0x0de0,0x0f20,
    0x1080,0x1180,0x1340,0x1540,0x16a0,0x17e0,0x1920,0x1ba0,
    0x1da0,		
};	
#endif
   



#ifdef LMZ2075_LASER_12_80_808
unsigned int cameraAD[]=		
{
    0x0ce0,0x0cc0,0x0ca0,0x0c80,0x0c70,0x0c60,0x0c50,0x0c40,
    0x0c30,0x0c20,0x0c10,0x0c00,0x0bf0,0x0be0,0x0bd0,0x0bc0,
    0x0bb0,0x0ba0,0x0b90,0x0b80,0x0b70,0x0b60,0x0b50,0x0b40,
    0x0b30,0x0b20,0x0ae0,0x0ac0,0x0aa0,0x0a50,0x0a10,0x09d0,
    0x0980,0x0930,0x0890,0x0870,0x0810,0x07b0,0x0750,0x0720,
    0x06c0,0x0670,0x0640,0x05e0,0x0590,0x0530,0x04e0,0x04b0,
    0x0410,0x0390,0x0350,0x0100,
};
unsigned int laserStep[]=	
{ 
    0x0300,0x0340,0x0380,0x03a0,0x03b0,0x0400,0x0450,0x04a0,
    0x0500,0x0510,0x0530,0x0560,0x0590,0x05c0,0x0600,0x0650,
    0x0660,0x0670,0x0680,0x0690,0x06a0,0x06c0,0x0700,0x0750,
    0x07a0,0x0800,0x0830,0x0860,0x0890,0x08c0,0x0900,0x0a00,
    0x0b00,0x0b80,0x0c00,0x0d00,0x0e00,0x0f00,0x1000,0x1100,
    0x1200,0x1300,0x1400,0x1500,0x1600,0x1700,0x1800,0x1900,
    0x1a00,0x1b00,0x1c00,0x1d00,		
};
#endif

#ifdef Y34Z22RADP_LASER_12_80

unsigned int cameraAD[]=	
{    
	0x0ef0,0x0ef0,0x0ef0,0x0ee0,0x0ed0,0x0ec0,0x0eb0,0x0ea0,
	  //0x0ed4,0x0ed3,0x0ed2,0x0ed1,0x0ed0,0x0ec0,0x0eb0,0x0ea0,
    0x0e90,0x0e80,0x0e70,0x0e60,0x0e50,0x0e40,0x0e30,0x0e20,
    0x0e10,0x0e00,0x0df0,0x0dc0,0x0d90,0x0d60,0x0d30,0x0cd0,
    0x0c70,0x0c10,0x0bb0,0x0b70,0x0b20,0x0ae0,0x0a80,0x0a30,
    0x09e0,0x0990,0x0950,0x0910,0x08d0,0x0890,0x0860,0x0830,
    0x0800,0x07d0,0x0790,0x0760,0x0720,0x06f0,0x06c0,0x06a0,
    0x0670,0x0650,0x0600,0x05a0,0x0530,0x0500,0x04c0,0x0490,
    0x0450,0x03e0,0x0390,0x0330,0x02d0,0x0280,0x0250,0x01f0,
    0x01a0,0x01a0,
};
unsigned int laserStep[]=		
{ 
    0x0100,0x0130,0x0160,0x0190,0x01b0,0x01d0,0x0200,0x0230,
    0x0260,0x0290,0x02b0,0x02d0,0x0300,0x0330,0x0360,0x0390,
    0x03b0,0x03d0,0x0400,0x0440,0x0480,0x04c0,0x0500,0x0550,
    0x05a0,0x0600,0x0640,0x0680,0x06c0,0x0700,0x0740,0x0780,
    0x07c0,0x0800,0x0840,0x0880,0x08c0,0x0900,0x0940,0x0980,
    0x09c0,0x0a00,0x0a40,0x0a80,0x0ac0,0x0b00,0x0b40,0x0b80,
    0x0bc0,0x0c00,0x0c80,0x0d00,0x0e00,0x0e80,0x0f00,0x0f80,
    0x1000,0x1100,0x1200,0x1400,0x1500,0x1700,0x1800,0x1900,
    0x1a00,0x1b00,
};	
#endif

#ifdef HDZ1922_LASER_30_30          
				
unsigned int cameraAD[]=		
{
    0x4000,0x3d86,0x3c14,0x39e9,0x372d,0x3382,0x3172,0x2ed0,
    0x2b75,0x28ee,0x2537,0x21b4,0x1e32,0x1b26,0x16a8,0x0fa4,
    0x06b6,0x0000,
};
unsigned int laserStep[]=		
{ 
    0x0000,0x0180,0x0180,0x0400,0x0580,0x0780,0x0800,0x0A80,
    0x0C00,0x0D00,0x0F00,0x1080,0x1180,0x1300,0x1480,0x1600,
    0x1800,0x1A00,
};											
																		
#endif

#ifdef HD60X12750_LASER_12_80   
unsigned int cameraAD[]=		
{
    0x0ecd,0x0e4e,0x0dea,0x0da3,0x0d61,0x0d16,0x0ca1,0x0c5f,
    0x0c17,0x0bbf,0x0b70,0x0b23,0x0ac6,0x0a65,0x0a0c,0x09bd,
    0x095e,0x090c,0x08ae,0x0852,0x07f0,0x0795,0x073d,0x06f2,
    0x069c,0x0642,0x05f9,0x05ac,0x056d,0x0521,0x04d6,0x0469,
    0x0441,0x0413,0x0395,0x035b,0x02dc,0x0257,0x0094,														
};
unsigned int laserStep[]=		
{ 
    0x00e0,0x0160,0x0220,0x02a0,0x0320,0x0360,0x0420,0x0480,
    0x04e0,0x05a0,0x0620,0x06a0,0x0700,0x0760,0x07c0,0x0840,
    0x08e0,0x0940,0x09c0,0x0a40,0x0b00,0x0ba0,0x0c80,0x0d40,
    0x0e40,0x0f60,0x1000,0x1120,0x1240,0x1340,0x1460,0x15e0,
    0x1760,0x17a0,0x1900,0x1a00,0x1c60,0x1e60,0x1ea0,
};
#endif

#ifdef HDC2132_LASER_08_20   
unsigned int cameraAD[]=		
{
    0x4000, 0x3fcd, 0x3f9b, 0x3f4c, 0x3f3a, 0x3f02, 0x3edd, 0x3eb4,
    0x3e81, 0x3e45, 0x3e29, 0x3de0, 0x3db2, 0x3d5b, 0x3d16, 0x3c87,
    0x3bd1, 0x3b57, 0x3abf, 0x3a63, 0x39c6, 0x3904, 0x3868, 0x3810,
    0x37e7, 0x3742, 0x3620, 0x3604, 0x3543, 0x349d, 0x345d, 0x3425,
    0x3331, 0x3321, 0x3311, 0x31c5, 0x3156, 0x30de, 0x3070, 0x3030,
    0x2fb8, 0x2f81, 0x2ea4, 0x2e1a, 0x2d90, 0x2d21, 0x2c60, 0x2c1f,
    0x2ab8, 0x282f, 0x27a5, 0x271f, 0x2630, 0x25ef, 0x256a, 0x2509,
    0x23b4, 0x230a, 0x22a5, 0x21b5, 0x20c6, 0x1f91, 0x1f51, 0x1eeb,
    0x1cba, 0x1c34, 0x1bae, 0x1b04, 0x1a7f, 0x19d9, 0x18c9, 0x174f,
    0x1685, 0x15ba, 0x14f0, 0x139b, 0x128b, 0x0fb8, 0x0e1e, 0x0bff,
    0x09e0, 0x07c1,													
};
unsigned int laserStep[]=		
{ 
    0x0000, 0x0120, 0x01a0, 0x0220, 0x0260, 0x02e0, 0x0360, 0x03a0,
    0x03e0, 0x0420, 0x0460, 0x04a0, 0x04e0, 0x0520, 0x0560, 0x05a0,
    0x05e0, 0x0640, 0x0680, 0x06c0, 0x0700, 0x0740, 0x07a0, 0x07e0,
    0x0820, 0x0860, 0x08c0, 0x0900, 0x0960, 0x09a0, 0x09e0, 0x0a20,
    0x0a60, 0x0aa0, 0x0ae0, 0x0b40, 0x0b80, 0x0bc0, 0x0c00, 0x0c40,
    0x0c80, 0x0cc0, 0x0d40, 0x0dc0, 0x0e40, 0x0ec0, 0x0f40, 0x0fc0,
    0x1040, 0x10c0, 0x1140, 0x11c0, 0x1240, 0x12c0, 0x1340, 0x13c0,
    0x1440, 0x14c0, 0x1540, 0x15c0, 0x1640, 0x16c0, 0x1740, 0x17c0,
    0x1840, 0x18c0, 0x1940, 0x19c0, 0x1a40, 0x1ac0, 0x1b40, 0x1bc0,
    0x1c80, 0x1d40, 0x1e40, 0x1f40, 0x2040, 0x2140, 0x2240, 0x2340,
    0x2440, 0x2540,
};
#endif

#ifdef HDZ2150_LASER_10_50
unsigned int cameraAD[] =
{
    0x3fc8,0x3fbd,0x3fb2,0x3f9b,0x3f85,0x3f7a,
    0x3f6e,0x3f58,0x3f4d,0x3f41,0x3f36,0x3f2b,0x3f15,0x3efe,
    0x3ef3,0x3edd,0x3ec6,0x3eb0,0x3ea5,0x3e99,0x3e8e,0x3e83,
    0x3e61,0x3e4b,0x3e29,0x3ddb,0x3d81,0x3ce4,0x3bc1,0x3a87,
    0x390a,0x384c,0x36a2,0x363d,0x3552,0x33e0,0x324c,0x308c,
    0x3006,0x2e9f,0x2cdf,0x2c58,0x2ac5,0x29fb,0x2910,0x27d6,
    0x26eb,0x25b1,0x23f1,0x228a,0x217d,0x2070,0x1e56,0x1c96,
    0x1a22,0x196f,0x17af,0x15ee,0x1488,0x12c8
};

unsigned int laserStep[] =
{
    0x0541,0x0564,0x0587,0x05aa,0x05cd,0x05f0,
    0x0613,0x0659,0x067c,0x069f,0x06c2,0x06e5,0x0708,0x072b,
    0x074e,0x0771,0x0794,0x07b7,0x07da,0x07fd,0x0820,0x0843,
    0x0866,0x0889,0x08ac,0x08cf,0x08f2,0x0915,0x0938,0x097e,
    0x09c4,0x0a0a,0x0a50,0x0a73,0x0ab9,0x0aff,0x0b68,0x0bd1,
    0x0c17,0x0c5d,0x0ca3,0x0cc6,0x0d0c,0x0d52,0x0dbb,0x0e24,
    0x0eb0,0x0f82,0x1031,0x10e0,0x116c,0x11f8,0x1379,0x14b4,
    0x167b,0x17fc,0x1a4f,0x1c16,0x1ce8,0x1e00
};
#endif
static void LaserInit()
{
    int len,i;
    
    GPIO_InitTypeDef GPIO_InitStructure;  
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOE, ENABLE);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_9|GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;          
    GPIO_Init(GPIOE, &GPIO_InitStructure);	

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;          
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    POWER_CLOSE;
    len = sizeof(laserStep)/4;
    for(i=0;i<len;i++)
    {
        laserStep[i] = 2*laserStep[i];
    }
    
}
static void LaserCorrectionCameraADValue(unsigned short correctionAD)
{
    char i ;
    unsigned char arrayNumber = sizeof(cameraAD)/4;

    if(correctionAD > cameraAD[0])
    {
        correctionAD = correctionAD - cameraAD[0];
        for(i=0;i<arrayNumber;i++)
        {
            cameraAD[i] += correctionAD;
        }
    }
    else 
    {
        correctionAD = cameraAD[0] - correctionAD;
        for(i=0;i<arrayNumber;i++)
        {
            cameraAD[i] -= correctionAD;
        }
    }
    
}
static void LaserSetTouchOff(char value)
{
    if((laserValue.status != 0)&&(laserValue.synchronizeFlag == 1))
    {
        laserValue.touchOffFlag = value;
    }
}
static void LaserSynchronizeEnable()
{
    laserValue.synchronizeFlag = 1;
}
static void LaserSynchronizeDisable()
{
    laserValue.synchronizeFlag = 0;
}
static void LaserOpen()
{
    POWER_OPEN;
    CAMERA_BLACK;
    laserValue.status = 1;
    laserValue.lightSensorAuto = 0;
    CameraBlack();
}
static void LaserClose()
{
    POWER_CLOSE;
    CAMERA_COLOR;
    laserValue.status = 0;
    laserValue.lightSensorAuto = 0;
    CameraColor();
}
static void LaserAuto()
{
    laserValue.lightSensorAuto = 1;
    laserCheckCount = 0;
}
static void LaserAutoOpen()
{
    POWER_OPEN;
    CAMERA_BLACK;
    laserValue.status = 1;
    CameraBlack();
}
static void LaserAutoClose()
{
    POWER_CLOSE;
    CAMERA_COLOR;
    laserValue.status = 0;
    CameraColor();
}
static void LaserStop()
{
    //SLEEP_L;
    laserValue.moveFlag  = 0;
    laserValue.direction = 0;
	
	time2_Laser_Lens_Sleep_cnt		=	0;
	Laser_Lens_Sleep_Flag			=	1;
}

static void LaserBig()
{
    if(laserValue.status == 1)
    {
        SLEEP_H;
        DIR_H;
        laserValue.moveFlag  = 1;
        laserValue.direction = 1;
    }
}
static void LaserSmall()
{
    if(laserValue.status == 1)
    {
        SLEEP_H;
        DIR_L;
        laserValue.moveFlag  = 1;
        laserValue.direction = 2;
    }
}

static long int AbsLongInt(unsigned int num1,unsigned int num2)
{
    if(num1 > num2)
    {
        return num1 - num2;
    }
    else
    {
        return num2 - num1;
    }
}

static void LaserSelfTest()
{
    laserValue.status = 1;
	laserValue.position	=	laserValue.limitMax	+	1000;
    LaserSmall();
    while(laserValue.position > 0);
    //delay_ms(100);
    LaserStop();
    delay_ms(10);
    LaserBig();
    while(laserValue.position < laserValue.limitMax);
    //delay_ms(100);
    LaserStop();
    
    laserValue.status = 0;
    //laserValue.limitMax -= laserValue.removeStep;
    //laserValue.limitMin += laserValue.removeStep;
    laserValue.position = laserValue.limitMax;
    POWER_CLOSE;
    CAMERA_COLOR;
}
static void LaserStepMove()
{
    static char flag = 0;
    if((laserValue.position >= laserValue.limitMax) && (laserValue.direction == 1))
    {
        LaserStop();
    }
    else if((laserValue.position <= laserValue.limitMin) && (laserValue.direction == 2))
    {
        LaserStop();
    }
    else
    {
        if(flag == 0)
        {
            flag = 1;
            STEP_H;
        }
        else if(flag == 1)
        {
            flag = 0;
            STEP_L;
        }
        switch (laserValue.direction)
        {
            case 1:
                laserValue.position ++;
                break;
            case 2:
                laserValue.position --;
                break;
        }
    }
}
static void LaserGoToPosition(long int position)
{
    laserValue.positionFlag  = 1;
    laserValue.endPosition = position;
    if(AbsLongInt(laserValue.position,position) > POSITION_ERROR)
    {
        if(laserValue.position < position)
        {
            LaserBig();
        }
        else if(laserValue.position > position)
        {
            LaserSmall();
        }
        else
        {
            LaserStop();
        }
    }
		else
		{
			LaserStop();
		}
		
    
}
static void LaserStopPosition()
{
    if(laserValue.positionFlag == 1)
    {
        if(AbsLongInt(laserValue.position,laserValue.endPosition) < POSITION_ERROR)
        {
            LaserStop();
            laserValue.positionFlag = 0;
            
        }
    }
}
static void LaserAutoFunction()
{
    if(laserValue.lightSensorAuto == 1)
    {
        if((LENS->ReadLightSensorValue() > laserValue.lightSensorOnThreshold)&&(laserValue.status == 0))
        {
            laserStatusChange = 1;
            if(laserCheckCount > 150)
            {
                laserCheckCount = 0;
                laserStatusChange = 0;
                LaserAutoOpen();
            }
        }
        else if((LENS->ReadLightSensorValue() < laserValue.lightSensorOffThreshold)&&(laserValue.status == 1))
        {
            laserStatusChange = 1;
            if(laserCheckCount > 150)
            {
                laserCheckCount = 0;
                laserStatusChange = 0;
                LaserAutoClose();
            }
        }
    }
}

static void LaserSynchronize()
{
    if(laserValue.synchronizeFlag == 0)
    {
        return;
    }
    if(laserValue.touchOffFlag == 0)
    {
        return;
    }
    unsigned char arrayCount , arrayNumber;
    unsigned short cameraZoomValue = 0xffff;
    static unsigned short cameraZoomLastValue = 0xffff;
    arrayNumber = sizeof(cameraAD)/4;
    
    #ifdef IPC_EN
        cameraZoomValue = Get_Camera_Zoom_Position();	
    #else
        cameraZoomValue = LENS->Camera->ReadZoomValue();  
    #endif 
    if(pMiscAlg->UnShortAbs(cameraZoomLastValue,cameraZoomValue)< 50)
    {
        return ;
    }
    cameraZoomLastValue = cameraZoomValue;
    for(arrayCount=0;arrayCount < arrayNumber;arrayCount++)
    {
        if( cameraZoomValue >= cameraAD[arrayCount])
        {
            break;
        }
    }
    if(arrayCount > (arrayNumber - 1))
    {
        arrayCount = arrayNumber - 1;
    }
    if(arrayCount == 0)
    {
        if(laserValue.errorFdir == 0)
        {
            LaserGoToPosition(laserStep[0] + laserValue.errorFirstStep);
        }
        else if(laserValue.errorFdir == 1)
        {
            LaserGoToPosition(laserStep[0] - laserValue.errorFirstStep);
        }
    }
    else if(laserValue.errorDir == 0)
    {
        LaserGoToPosition(laserStep[arrayCount] + laserValue.errorFullStep);
    }
    else if(laserValue.errorDir == 1)
    {
        if(laserStep[arrayCount] > laserValue.errorFullStep)
        {
            LaserGoToPosition(laserStep[arrayCount] - laserValue.errorFullStep);
        }
        else 
        {
            LaserGoToPosition(laserValue.errorFullStep - laserStep[arrayCount]);
        }
        
    }
   
}

static void LaserMove()
{
    if((laserValue.moveFlag == 1)&&(laserValue.status != 0))
    {
        LaserStopPosition();
        LaserStepMove();
    }
}

static void LaserFullError()
{
    unsigned char buf[3];
    unsigned int posAarray;
    unsigned char arrayCount , arrayNumber;
    unsigned short cameraZoomValue;
    arrayNumber = sizeof(cameraAD)/4;

    #ifdef IPC_EN
        cameraZoomValue = Get_Camera_Zoom_Position();	
    #else
        cameraZoomValue = LENS->Camera->ReadZoomValue();  
    #endif
    for(arrayCount=0;arrayCount<arrayNumber;arrayCount++)
    {
        if( cameraZoomValue > cameraAD[arrayCount])
        {
            break;
        }
    }
    posAarray = laserStep[arrayCount];
    if(laserValue.position > posAarray)
    {
        laserValue.errorFullStep = laserValue.position - posAarray;
        laserValue.errorDir = 0;
    }
    else 
    {
        laserValue.errorFullStep = posAarray - laserValue.position ;
        laserValue.errorDir = 1;
    }
    buf[0] = (laserValue.errorFullStep>>8)&0xff;
    buf[1] = (laserValue.errorFullStep>>0)&0xff;
    buf[2] = laserValue.errorDir;
    EEPROM_I2C_WriteBuffer(0x1910,buf,3);
}

static void LaserFirstError()
{
    unsigned char buf[3];
    unsigned int posAarray;

    posAarray = laserStep[0];
    if(laserValue.position > posAarray)
    {
        laserValue.errorFirstStep = laserValue.position - posAarray;
        laserValue.errorFdir = 0;
    }
    else 
    {
        laserValue.errorFirstStep = posAarray - laserValue.position ;
        laserValue.errorFdir = 1;
    }
    buf[0] = (laserValue.errorFirstStep>>8)&0xff;
    buf[1] = (laserValue.errorFirstStep>>0)&0xff;
    buf[2] = laserValue.errorFdir;
    EEPROM_I2C_WriteBuffer(0x1913,buf,3);
}
static void LaserRead()
{
    unsigned char buf[6];
    EEPROM_I2C_ReadBuffer(0x1910,buf,6);
    laserValue.errorFullStep = (buf[0]<<8)|buf[1];
    laserValue.errorDir = buf[2];
    if(laserValue.errorFullStep == 0xff)
    {
        laserValue.errorFullStep = 0;
    }
    if(laserValue.errorDir == 0xff)
    {
        laserValue.errorDir = 0;
    }
    
    laserValue.errorFirstStep = (buf[3]<<8)|buf[4];
    laserValue.errorFdir = buf[5];
    if(laserValue.errorFullStep == 0xff)
    {
        laserValue.errorFirstStep = 0;
    }
    if(laserValue.errorDir == 0xff)
    {
        laserValue.errorFdir = 0;
    }
    
    
}



LASER_T laserInit = 
{
    .Init                   =   LaserInit,
    .Move                   =   LaserMove,
    .Open                   =   LaserOpen,
    .Close                  =   LaserClose,
    .Auto                   =   LaserAuto,
    .Stop                   =   LaserStop,
    .Big                    =   LaserBig,
    .Small                  =   LaserSmall,
    .SeltTest               =   LaserSelfTest,
    .GoTo                   =   LaserGoToPosition,
    .Synchronize            =   LaserSynchronize,
    .SynchronizeEnable      =   LaserSynchronizeEnable,
    .SynchronizeDisable     =   LaserSynchronizeDisable,
    .SetTouchOff            =   LaserSetTouchOff,
    .AutoFunction           =   LaserAutoFunction,
    .FullError              =   LaserFullError,
    .FirstError             =   LaserFirstError,
    .Read                   =   LaserRead,
    .CorrectionCameraAD     =   LaserCorrectionCameraADValue,
};

LASER_T *pLaser = &laserInit ;







