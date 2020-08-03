#ifndef __SYS_CFG__
#define __SYS_CFG__

/*************************************************************
*64k eeprom 64*1024/8=8192=0x2000
*存储地址范围0x0000~0x1ffff
*预置位长度16，预置位存储地址0~255*16 = 0~4080=0x00~0xff0
**************************************************************/

#define EEPROM_SYSINFO_ADDR         0x1000

#define LOC_3D_CORR_ADDR            0x1200
#define CAM_AUTO_FOCUS_ADDR         0x1b00
#pragma pack(1)

typedef struct 
{
    char minute;
    char second;
    char speed;
}CURISE_PARA_T;

typedef struct 
{
    unsigned char stepAngle;
}APPLE_SCAN_PARA_T;



typedef struct 
{
    unsigned char type;
}WATCH_HOME_PARA_T;


typedef struct
{
    unsigned int hor;
    unsigned int ver;
}ZERO_POS_PARA_T;


typedef struct 
{
/**********无需EEPROM保存***********/   
    unsigned char       camAddr;
    unsigned char       irAddr;
    unsigned int        camBaud;
    unsigned int        irBaud;
    unsigned char       returnCnt;
    unsigned char       brush;
    unsigned char       fog;
    unsigned char       camReturnType;
    unsigned char       sysTest;
/**********需EEPROM保存***********/   
    unsigned char       eepromEnable;
    unsigned char       ptReturn;
    unsigned char       viewReturn;
    unsigned char       returnFreq;
    unsigned char       autoSpeed;
    unsigned char       powerRemember;
    unsigned char       alarmStop;
    unsigned char       camFocusSpeed;
    CURISE_PARA_T       curise;
    WATCH_HOME_PARA_T   watchHome;
    APPLE_SCAN_PARA_T   appleScan;
    ZERO_POS_PARA_T     zeroPos;         
}SYS_PARA_T;




#pragma pack(0)




unsigned int    CuriseGetDwellTime          (void);
void            CuriseSetDwellTime          (char type,char val);
void            SysParaRead                 (void);
void            EnableFunction              (void);
void            Wait                        (void);
void            PowerOffRememberFunction    (void);
void            IWDG_Configuration          (void);
void            FogFunction                 (void);




#define TVC_UART 							USART1
#define TVC_UART_UART_CLK 		            RCC_APB2Periph_USART1
#define TVC_UART_GPIO_CLK 		            RCC_APB2Periph_GPIOA
#define TVC_UART_IRQn 				        USART1_IRQn	
#define TVC_UART_IRQHandler 	            USART1_IRQHandler	
#define TVC_UART_GPIO 				        GPIOA
#define TVC_UART_TX_PIN 			        GPIO_Pin_9
#define TVC_UART_RX_PIN 			        GPIO_Pin_10

#define PT_UART 							USART2
#define PT_UART_UART_CLK 			        RCC_APB1Periph_USART2

#define PT_UART_GPIO_CLK 			        RCC_APB2Periph_GPIOA
#define PT_UART_IRQn 					    USART2_IRQn 
#define PT_UART_IRQHandler 		            USART2_IRQHandler
#define PT_UART_GPIO 					    GPIOA

#define PT_UART_TX_PIN 				        GPIO_Pin_2 

#define PT_UART_RX_PIN 				        GPIO_Pin_3

#define VC_UART 							USART3
#define VC_UART_UART_CLK 			        RCC_APB1Periph_USART3
#define VC_UART_GPIO_CLK 			        RCC_APB2Periph_GPIOB
#define VC_UART_IRQn 					    USART3_IRQn	
#define VC_UART_IRQHandler 		            USART3_IRQHandler	
#define VC_UART_GPIO 					    GPIOB
#define VC_UART_TX_PIN 				        GPIO_Pin_10
#define VC_UART_RX_PIN 				        GPIO_Pin_11

	

#define TTL_UART 							UART4 
#define TTL_UART_UART_CLK  		            RCC_APB1Periph_UART4 
#define TTL_UART_GPIO_CLK 		            RCC_APB2Periph_GPIOC 
#define TTL_UART_IRQn 				        UART4_IRQn		
#define TTL_UART_IRQHandler 	            UART4_IRQHandler	 
#define TTL_UART_GPIO 				        GPIOC
#define TTL_UART_RX_PIN 			        GPIO_Pin_11	   
#define TTL_UART_TX_PIN 			        GPIO_Pin_10	

#define IR_UART 							UART5
#define IR_UART_UART_CLK 			        RCC_APB1Periph_UART5
#define IR_UART_GPIO_CLK 			        RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD
#define IR_UART_IRQn 					    UART5_IRQn
#define IR_UART_IRQHandler 		            UART5_IRQHandler
#define IR_UART_RX_PIN 				        GPIO_Pin_2
#define IR_UART_TX_PIN 				        GPIO_Pin_12

#define EEPROM_I2C1    					    I2C1
#define EEPROM_I2C1_CLK    					RCC_APB1Periph_I2C1

#define EEPROM_SCL_I2C1_SCL_PB6 		    GPIO_Pin_6
#define EEPROM_SCL_I2C1_GPIO_PORT		    GPIOB                
#define EEPROM_SCL_I2C1_GPIO_CLK  	        RCC_APB2Periph_GPIOB

#define EEPROM_SDA_I2C1_SDA_PB7    	        GPIO_Pin_7
#define EEPROM_SDA_I2C1_GPIO_PORT  	        GPIOB                     
#define EEPROM_SDA_I2C1_GPIO_CLK   	        RCC_APB2Periph_GPIOB

#define CAMERA_LENSE_CLK 				    RCC_APB2Periph_GPIOC
#define	CAMERA_LENSE_GPIO 			        GPIOC

#define CAMERA_FOCUS_PIN1  			        GPIO_Pin_8	
#define CAMERA_FOCUS_PIN2  			        GPIO_Pin_9
#define CAMERA_ZOOM_PIN1  			        GPIO_Pin_7  	
#define CAMERA_ZOOM_PIN2  			        GPIO_Pin_6 	
#define CAMERA_IC_PIN1  				    GPIO_Pin_4
#define CAMERA_IC_PIN2  				    GPIO_Pin_5

#define CAMERA_FOCUSSPEED_CLK  	            RCC_APB2Periph_GPIOB 		
#define	CAMERA_FOCUSSPEED_GPIO 	            GPIOB
#define CAMERA_FOCUSSPEED_PIN 	            GPIO_Pin_8 | GPIO_Pin_9 

#define CAMERA_ADC_GPIO 				    GPIOC
#define CAMERA_ZOOM_ADC_PIN  		        GPIO_Pin_1
#define CAMERA_FOCUS_ADC_PIN 		        GPIO_Pin_0
#define IR_ZOOM_ADC_PIN      		        GPIO_Pin_3
#define IR_FOCUS_ADC_PIN     		        GPIO_Pin_2

#define IR_LENSE_CLK 						RCC_APB2Periph_GPIOA
#define	IR_LENSE_ZOOM_GPIO 			        GPIOA
#define IR_ZOOM_PIN1  					    GPIO_Pin_4 
#define IR_ZOOM_PIN2  					    GPIO_Pin_5
#define	IR_LENSE_FOCUS_GPIO 		        GPIOA
#define IR_FOCUS_PIN1  					    GPIO_Pin_6
#define IR_FOCUS_PIN2  					    GPIO_Pin_7


extern SYS_PARA_T sysPara;




void SysInit(void);
void SysCheck(void);
void SysParaDefaultInit(void);
void FogOff(void);
void FogOn(void);
void BrushOn(void);
void BrushOff(void);



void SysParaDefaultInit(void);
void SysParaSave(void);
char SysStatusCheck(void);
void SysFunInit(void);
#endif
