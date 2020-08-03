/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : eeprom_i2c.h
* Author             : LPWIN 
* Version            : V2.0.1
* Date               : 02/08/2014
* Description        : Header for eeprom_i2c.c module ,modify from  i2c_ee.h
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/
/* Define to prevent recursive inclusion ------------------------------------ */
#ifndef __EEPROM_I2C_H
#define __EEPROM_I2C_H

#include "stm32f10x.h"
#include "SysCfg.h"
//当前使用的型号
#define EEPROM_I2C1_AT24C64
//AT24C64从器件的读写频率,STM32的I2C限制读写的频率不能高于400K
#define EEPROM_I2C1_AT24C64_SPEED               300000
//AT24C64从器件的地址
#define EEPROM_I2C1_AT24C64_SLAVE_ADDRESS7      0xA0


#define EEPROM_I2C1_DMA1                      DMA1   
#define EEPROM_I2C1_DMA1_CHANNEL_TX           DMA1_Channel6
#define EEPROM_I2C1_DMA1_CHANNEL_RX           DMA1_Channel7 
#define EEPROM_I2C1_DMA1_FLAG_TX_TC           DMA1_IT_TC6   
#define EEPROM_I2C1_DMA1_FLAG_TX_GL           DMA1_IT_GL6 
#define EEPROM_I2C1_DMA1_FLAG_RX_TC           DMA1_IT_TC7 
#define EEPROM_I2C1_DMA1_FLAG_RX_GL           DMA1_IT_GL7    
#define EEPROM_I2C1_DMA1_CLK                  RCC_AHBPeriph_DMA1
#define EEPROM_I2C1_DR_Address               ((uint32_t)0x40005410)

#define EEPROM_I2C1_DMA1_USE_DMA
   
#define EEPROM_I2C1_DMA1_TX_IRQn              DMA1_Channel6_IRQn
#define EEPROM_I2C1_DMA1_RX_IRQn              DMA1_Channel7_IRQn
#define EEPROM_I2C1_DMA1_TX_IRQHandler        DMA1_Channel6_IRQHandler
#define EEPROM_I2C1_DMA1_RX_IRQHandler        DMA1_Channel7_IRQHandler  
#define EEPROM_I2C1_DMA1_PREPRIO              0
#define EEPROM_I2C1_DMA1_SUBPRIO              0
 
#define EEPROM_I2C1_DIRECTION_TX             0
#define EEPROM_I2C1_DIRECTION_RX             1

//读EEPROM返回值
#define EEPROM_READ_OK 0
#define EEPROM_READ_FAILED 1

//写EEPROM返回值
#define EEPROM_WRITE_OK 0
#define EEPROM_WRITE_FAILED 1

//释放I2C占用的所有资源
void EEPROM_I2C_DeInit(void);
//初始化I2C对外部EEPROM的控制
void EEPROM_I2C_Init(void);


//全局函数初始化
void EEPROM_I2C_ReadBuffer(uint16_t ReadAddr,uint8_t* pBuffer,  uint16_t NumByteToRead);
void EEPROM_I2C_WriteBuffer( uint16_t WriteAddr,uint8_t* pBuffer, uint16_t NumByteToWrite);

#endif
