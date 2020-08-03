

#include "eeprom_i2c.h"



//释放I2C占用的所有资源
void EEPROM_DeInit(void);

//初始化I2C对EEPROM的读写
void EEPROM_I2C_Init(void);


//系统时钟为72M,则此延时为1ms
#define EEPROM_FLAG_TIMEOUT         ((uint32_t)72000)
//延时10ms,足够EEPROM读写一页128个字节的时间
#define EEPROM_LONG_TIMEOUT         ((uint32_t)(10*EEPROM_FLAG_TIMEOUT))

//AT24C64共有页,1页32个字节
//AT24C64B在使用的时候需按页进行写操作,否则将写入同一页的位置中去
#define AT24C64B_PAGESIZE 32

////释放I2C占用的所有资源
//void EEPROM_I2C_DeInit(void);
////初始化I2C对外部EEPROM的控制
//void EEPROM_I2C_Init(void);
//I2C1对外部EEPROM的控制,读取和发送的字节及方向
void EEPROM_DMAConfig(uint32_t pBuffer, uint32_t BufferSize, uint32_t Direction);


//全局函数初始化
//uint32_t EEPROM_I2C_ReadBuffer(uint8_t* pBuffer, uint16_t ReadAddr, uint16_t* NumByteToRead);
//void EEPROM_WriteBuffer(uint8_t* pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite);
uint32_t EEPROM_WritePage(uint8_t* pBuffer, uint16_t WriteAddr, uint8_t* NumByteToWrite);
uint32_t EEPROM_WaitStandbyState(void);

//超时操作,若读写的过程中出现超时,则调用回调函数
#define USE_DEFAULT_CRITICAL_CALLBACK
void EEPROM_EnterCriticalSection_UserCallback(void);
void EEPROM_ExitCriticalSection_UserCallback(void);



//读EEPROM数据长度的指针 -- 该指针指示是否读写完成 -- 在DMA接收完成中断中清零
uint16_t* m_EEPROM_I2C1_DataReadPointer;

//向EEPROM写入数据的长度的指针 -- 该指针指示是否写完成 -- 在DMA发送完成中断中清零
uint8_t* m_EEPROM_I2C1_DataWritePointer;

//读EEPROM的超时计数,若在规定的时间内未读取到数据,则返回
uint32_t  m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;

//调试，获取超时时间
//#define DEBUG_TIMEOUT
#ifdef 	DEBUG_TIMEOUT
unsigned char time_flag = 0;
#endif

GPIO_InitTypeDef  GPIO_InitStructure;
NVIC_InitTypeDef  NVIC_InitStructure;
DMA_InitTypeDef   EEPROM_DMA_InitStructure;
I2C_InitTypeDef   I2C_InitStructure;

//释放I2C占用的所有资源
void EEPROM_I2C_DeInit(void)
{	
		//关闭I2C1的功能
		I2C_Cmd(EEPROM_I2C1, DISABLE);
		
		//释放I2C1的所有资源
		I2C_DeInit(EEPROM_I2C1);
		
		//关闭I2C1的时钟
		RCC_APB1PeriphClockCmd(EEPROM_I2C1_CLK, DISABLE);
			
		//配置I2C1的SCL端口
		GPIO_InitStructure.GPIO_Pin = EEPROM_SCL_I2C1_SCL_PB6;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(EEPROM_SCL_I2C1_GPIO_PORT, &GPIO_InitStructure);
		
		//配置I2C1的SDA端口
		GPIO_InitStructure.GPIO_Pin = EEPROM_SDA_I2C1_SDA_PB7;
		GPIO_Init(EEPROM_SDA_I2C1_GPIO_PORT, &GPIO_InitStructure);
		
		//配置I2C1的DMA1发送通道
		NVIC_InitStructure.NVIC_IRQChannel = EEPROM_I2C1_DMA1_TX_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EEPROM_I2C1_DMA1_PREPRIO;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = EEPROM_I2C1_DMA1_SUBPRIO;
		NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
		NVIC_Init(&NVIC_InitStructure);
		
		//配置I2C1的DMA1接收通道
		NVIC_InitStructure.NVIC_IRQChannel = EEPROM_I2C1_DMA1_RX_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EEPROM_I2C1_DMA1_PREPRIO;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = EEPROM_I2C1_DMA1_SUBPRIO;
		NVIC_Init(&NVIC_InitStructure);   
		
		//禁止I2C1的DMA中断,并释放资源
		DMA_Cmd(EEPROM_I2C1_DMA1_CHANNEL_TX, DISABLE);
		DMA_Cmd(EEPROM_I2C1_DMA1_CHANNEL_RX, DISABLE);
		DMA_DeInit(EEPROM_I2C1_DMA1_CHANNEL_TX);
		DMA_DeInit(EEPROM_I2C1_DMA1_CHANNEL_RX);
}

void EEPROM_I2C_Init(void)
{   
		//打开I2C1的SDA和SCL引脚使用的时钟
		RCC_APB2PeriphClockCmd(EEPROM_SCL_I2C1_GPIO_CLK | EEPROM_SDA_I2C1_GPIO_CLK, ENABLE);

		//打开I2C1的时钟
		RCC_APB1PeriphClockCmd(EEPROM_I2C1_CLK, ENABLE);

		//打开复用IO口功能 --2012-03-23
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);

		//配置I2C1的SCL端口
		GPIO_InitStructure.GPIO_Pin = EEPROM_SCL_I2C1_SCL_PB6;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
		GPIO_Init(EEPROM_SCL_I2C1_GPIO_PORT, &GPIO_InitStructure);

		//配置I2C1的SDA端口
		GPIO_InitStructure.GPIO_Pin = EEPROM_SDA_I2C1_SDA_PB7;
		GPIO_Init(EEPROM_SDA_I2C1_GPIO_PORT, &GPIO_InitStructure); 

		//added 2012-03-23
		//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

		//配置I2C1的DMA1发送通道
		NVIC_InitStructure.NVIC_IRQChannel = EEPROM_I2C1_DMA1_TX_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EEPROM_I2C1_DMA1_PREPRIO;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = EEPROM_I2C1_DMA1_SUBPRIO;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);

		//配置I2C1的DMA1接收通道
		NVIC_InitStructure.NVIC_IRQChannel = EEPROM_I2C1_DMA1_RX_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EEPROM_I2C1_DMA1_PREPRIO;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = EEPROM_I2C1_DMA1_SUBPRIO;
		NVIC_Init(&NVIC_InitStructure);  

		//配置I2C1的DMA通道,并使能DMA时钟
		RCC_AHBPeriphClockCmd(EEPROM_I2C1_DMA1_CLK, ENABLE);

		//配置I2C1的DMA1的发送通道
		DMA_DeInit(EEPROM_I2C1_DMA1_CHANNEL_TX);
		EEPROM_DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)EEPROM_I2C1_DR_Address;
		//该参数可以在和EEPROM外围器件通讯的过程中设置 -- 内存地址,存放接收和发送的数据
		EEPROM_DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)0;   
		//该参数可以在和EEPROM外围器件通讯的过程中设置 -- 标志为发送或者接收
		EEPROM_DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;    
		//该参数可以在和EEPROM外围器件通讯的过程中设置 -- 通讯的字节
		EEPROM_DMA_InitStructure.DMA_BufferSize = 0xFFFF;            
		EEPROM_DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		EEPROM_DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		EEPROM_DMA_InitStructure.DMA_PeripheralDataSize = DMA_MemoryDataSize_Byte;
		EEPROM_DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
		EEPROM_DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
		EEPROM_DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
		EEPROM_DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
		DMA_Init(EEPROM_I2C1_DMA1_CHANNEL_TX, &EEPROM_DMA_InitStructure);  

		//配置I2C1的DMA1的接收通道
		DMA_DeInit(EEPROM_I2C1_DMA1_CHANNEL_RX);
		DMA_Init(EEPROM_I2C1_DMA1_CHANNEL_RX, &EEPROM_DMA_InitStructure);  

		//使能I2C1的MDA1的接收和发送通道的中断
		DMA_ITConfig(EEPROM_I2C1_DMA1_CHANNEL_TX, DMA_IT_TC, ENABLE);
		DMA_ITConfig(EEPROM_I2C1_DMA1_CHANNEL_RX, DMA_IT_TC, ENABLE);   

		//设置I2C1的模式为主模式
		I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
		I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
		//设置I2C1读写的EEPROM的地址 -- 0xA0 --标志第一个器件
		I2C_InitStructure.I2C_OwnAddress1 = EEPROM_I2C1_AT24C64_SLAVE_ADDRESS7;
		I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
		I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
		//设置I2C1读写的EEPROM的速率
		I2C_InitStructure.I2C_ClockSpeed = EEPROM_I2C1_AT24C64_SPEED;

		//打开I2C1的时钟
		I2C_Cmd(EEPROM_I2C1, ENABLE);
		//使能I2C1的参数设置
		I2C_Init(EEPROM_I2C1, &I2C_InitStructure);	
		//开启I2C1的DMA1通道,并使能所有中断	
		I2C_DMACmd(EEPROM_I2C1, ENABLE);
}

//I2C1对外部EEPROM的控制,读取和发送的字节及方向
void EEPROM_DMAConfig(uint32_t pBuffer, uint32_t BufferSize, uint32_t Direction)
{ 
		//重新配置DMA的发送和接收通道,并重新初始化
		if (Direction == EEPROM_I2C1_DIRECTION_TX)
		{
				EEPROM_DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)pBuffer;
				EEPROM_DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;    
				EEPROM_DMA_InitStructure.DMA_BufferSize = (uint32_t)BufferSize;  

				DMA_Init(EEPROM_I2C1_DMA1_CHANNEL_TX, &EEPROM_DMA_InitStructure);  
		}
		else if (Direction == EEPROM_I2C1_DIRECTION_RX)
		{ 
				EEPROM_DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)pBuffer;
				EEPROM_DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
				EEPROM_DMA_InitStructure.DMA_BufferSize = (uint32_t)BufferSize;
				 
				DMA_Init(EEPROM_I2C1_DMA1_CHANNEL_RX, &EEPROM_DMA_InitStructure);    
		}
}

//从EEPROM中读取数据,若读成功则返回 
uint32_t EEPROM_I2C_Read(uint8_t* pBuffer, uint16_t ReadAddr, uint16_t NumByteToRead)
{ 
		static unsigned short readPoint = 0;
		readPoint =  NumByteToRead;
		//该变量在DMA接收完成中断中清零,可检测该值是否为0,从而确定DMA传输是否完成
		m_EEPROM_I2C1_DataReadPointer = &readPoint;
		//added 2012-03-23
		I2C_ITConfig(EEPROM_I2C1,I2C_IT_ERR,ENABLE);
		//若总线忙时,则等待,直到总线释放
		//若在指定的时间内,未释放,则读错误
		m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;
		while(I2C_GetFlagStatus(EEPROM_I2C1, I2C_FLAG_BUSY))
		{
				if((m_EEPROM_I2C1_Timeout--) == 0)
				return EEPROM_READ_FAILED;
		}
		/*****************************向EEPROM中写入读的地址*****************************/
		//I2C1产生传输START
		I2C_GenerateSTART(EEPROM_I2C1, ENABLE);
		//检测最近一次的I2C1事件是否为模式选择的事件
		m_EEPROM_I2C1_Timeout = EEPROM_FLAG_TIMEOUT;
		while(!I2C_CheckEvent(EEPROM_I2C1, I2C_EVENT_MASTER_MODE_SELECT))
		{
				if((m_EEPROM_I2C1_Timeout--) == 0)
				return EEPROM_READ_FAILED;
		}
		//向外部EEPROM发送片选地址
		I2C_Send7bitAddress(EEPROM_I2C1, EEPROM_I2C1_AT24C64_SLAVE_ADDRESS7, I2C_Direction_Transmitter);
		//检测最近一次的I2C1事件是否为发送事件
		m_EEPROM_I2C1_Timeout = EEPROM_FLAG_TIMEOUT;
		while(!I2C_CheckEvent(EEPROM_I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
		{
				if((m_EEPROM_I2C1_Timeout--) == 0)
				return EEPROM_READ_FAILED;
		} 
		//AT24C64B需要16位的地址选择
		//向外部的EEPROM发送高地址选择
		I2C_SendData(EEPROM_I2C1, (uint8_t)((ReadAddr & 0xFF00) >> 8));    
		//检测最近一次的I2C1事件是否为发送事件
		m_EEPROM_I2C1_Timeout = EEPROM_FLAG_TIMEOUT;
		while(!I2C_CheckEvent(EEPROM_I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
		{
				if((m_EEPROM_I2C1_Timeout--) == 0)
				return EEPROM_READ_FAILED;
		}
		//向外部的EEPROM发送低地址选择
		I2C_SendData(EEPROM_I2C1, (uint8_t)(ReadAddr & 0x00FF));    
		//检测I2C1是否完成字节的发送操作
		m_EEPROM_I2C1_Timeout = EEPROM_FLAG_TIMEOUT;
		while(I2C_GetFlagStatus(EEPROM_I2C1, I2C_FLAG_BTF) == RESET)
		{
				if((m_EEPROM_I2C1_Timeout--) == 0)
				return EEPROM_READ_FAILED;
		}

		/*****************************向EEPROM中写入读的地址*****************************/
		//I2C1产生第二次传输START
		I2C_GenerateSTART(EEPROM_I2C1, ENABLE);
		//检测最近一次的I2C1事件是否为模式选择的事件
		m_EEPROM_I2C1_Timeout = EEPROM_FLAG_TIMEOUT;
		while(!I2C_CheckEvent(EEPROM_I2C1, I2C_EVENT_MASTER_MODE_SELECT))
		{
				if((m_EEPROM_I2C1_Timeout--) == 0)
				return EEPROM_READ_FAILED;
		}
		//向外部的EEPROM发送读地址
		I2C_Send7bitAddress(EEPROM_I2C1, EEPROM_I2C1_AT24C64_SLAVE_ADDRESS7, I2C_Direction_Receiver);  
		//下面判读读取字节的个数，若<2 -- 直接读取; 若>2使用DMA方式
		if ((uint16_t)(NumByteToRead) < 2)
		{
				//检测地址发送标志位ADSL是否设置,若为1则地址发送未结束,继续等待 0--可以进行发送
				m_EEPROM_I2C1_Timeout = EEPROM_FLAG_TIMEOUT;
				while(I2C_GetFlagStatus(EEPROM_I2C1, I2C_FLAG_ADDR) == RESET)
				{
						if((m_EEPROM_I2C1_Timeout--) == 0)
						return EEPROM_READ_FAILED;
				}     

				//关闭I2C的应答
				I2C_AcknowledgeConfig(EEPROM_I2C1, DISABLE);   

				//关闭中断,进入临界区
				EEPROM_EnterCriticalSection_UserCallback();    
				//清空I2C1的状态
				(void)EEPROM_I2C1->SR2;    
				//I2C1产生停止
				I2C_GenerateSTOP(EEPROM_I2C1, ENABLE);   
				//打开中断,离开临界区
				EEPROM_ExitCriticalSection_UserCallback();

				//等待接收外部EEPROM的返回数据
				m_EEPROM_I2C1_Timeout = EEPROM_FLAG_TIMEOUT;
				while(I2C_GetFlagStatus(EEPROM_I2C1, I2C_FLAG_RXNE) == RESET)
				{
						if((m_EEPROM_I2C1_Timeout--) == 0)
						return EEPROM_READ_FAILED;
				}

				//读取返回的字节
				*pBuffer = I2C_ReceiveData(EEPROM_I2C1);

				//字节指针自减
				(uint16_t)(NumByteToRead)--;        

				//确定停止位已经清除
				m_EEPROM_I2C1_Timeout = EEPROM_FLAG_TIMEOUT;
				while(EEPROM_I2C1->CR1 & I2C_CR1_STOP)
				{
						if((m_EEPROM_I2C1_Timeout--) == 0)
						return EEPROM_READ_FAILED;
				}  

				//使能I2C1的应答,为下次读取做准备
				I2C_AcknowledgeConfig(EEPROM_I2C1, ENABLE);    
		}
		else
		{
				//读字节>1的时候,使用DMA的方式
				m_EEPROM_I2C1_Timeout = EEPROM_FLAG_TIMEOUT;
				while(!I2C_CheckEvent(EEPROM_I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
				{
						if((m_EEPROM_I2C1_Timeout--) == 0)
						return EEPROM_READ_FAILED;
				}  

				//重新配置I2C1的DMA通道为读通道
				EEPROM_DMAConfig((uint32_t)pBuffer, (uint16_t)(NumByteToRead), EEPROM_I2C1_DIRECTION_RX);
				//I2C1的下一次DMA传输为最后一次传输
				I2C_DMALastTransferCmd(EEPROM_I2C1, ENABLE); 
				//使能DMA的接收功能
				DMA_Cmd(EEPROM_I2C1_DMA1_CHANNEL_RX, ENABLE);  

				//等待DMA接收完成,在接收完成之后m_EEPROM_I2C1_DataReadPointer清零
				m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;
				while(*m_EEPROM_I2C1_DataReadPointer > 0)
				{
						//在里面加入超时判断,暂时先等待,需要确定传输的时间

						//若在指定的时间内未读取成功,则返回错误
						if((m_EEPROM_I2C1_Timeout--) == 0)
						return EEPROM_READ_FAILED;
				}
		}	
		//如果读操作成功,则返回读成功
		return EEPROM_READ_OK;
}

void EEPROM_I2C_ReadBuffer(uint16_t ReadAddr,uint8_t* pBuffer,  uint16_t NumByteToRead)
{
		unsigned short eeprom_ReadBuffer_Size;
		eeprom_ReadBuffer_Size =  NumByteToRead;
		EEPROM_I2C_Read(pBuffer,ReadAddr,eeprom_ReadBuffer_Size);
}

uint32_t EEPROM_WritePage(uint8_t* pBuffer, uint16_t WriteAddr, uint8_t* NumByteToWrite)
{ 
		//该变量在DMA发送完成中断中清零,可检测该值是否为0,从而确定DMA传输是否完成
		m_EEPROM_I2C1_DataWritePointer = NumByteToWrite;  

		//added 2012-03-24
		I2C_ITConfig(EEPROM_I2C1,I2C_IT_ERR,ENABLE);

		//若总线忙时,则等待,直到总线释放
		//若在指定的时间内,未释放,则读错误
		m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;
		while(I2C_GetFlagStatus(EEPROM_I2C1, I2C_FLAG_BUSY))
		{
				if((m_EEPROM_I2C1_Timeout--) == 0)
				return EEPROM_READ_FAILED;
		}

		/*****************************向EEPROM中写入读的地址*****************************/
		//I2C1产生传输START
		I2C_GenerateSTART(EEPROM_I2C1, ENABLE);  
		//检测最近一次的I2C1事件是否为模式选择的事件
		m_EEPROM_I2C1_Timeout = EEPROM_FLAG_TIMEOUT;
		while(!I2C_CheckEvent(EEPROM_I2C1, I2C_EVENT_MASTER_MODE_SELECT))
		{
				if((m_EEPROM_I2C1_Timeout--) == 0)
				return EEPROM_READ_FAILED;
		}

		//向外部EEPROM发送片选地址
		I2C_Send7bitAddress(EEPROM_I2C1, EEPROM_I2C1_AT24C64_SLAVE_ADDRESS7, I2C_Direction_Transmitter);
		//检测最近一次的I2C1事件是否为发送事件,等待外部器件的应答
		while(!I2C_CheckEvent(EEPROM_I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
		{
				if((m_EEPROM_I2C1_Timeout--) == 0)
				return EEPROM_READ_FAILED;

				#ifdef 	DEBUG_TIMEOUT
				if(0 == time_flag)
				{
						time_flag = 1;	
				}
				else if(1 == time_flag)
				{
						time_flag = 0;	
				}
				#endif		
		}
		//AT24C64B需要16位的地址选择
		//向外部的EEPROM发送高地址选择
		I2C_SendData(EEPROM_I2C1, (uint8_t)((WriteAddr & 0xFF00) >> 8));
		//检测最近一次的I2C1事件是否为字节发送事件
		m_EEPROM_I2C1_Timeout = EEPROM_FLAG_TIMEOUT;  
		while(!I2C_CheckEvent(EEPROM_I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
		{
				if((m_EEPROM_I2C1_Timeout--) == 0)
				return EEPROM_READ_FAILED;
		}  
		//向外部的EEPROM发送低地址选择
		I2C_SendData(EEPROM_I2C1, (uint8_t)(WriteAddr & 0x00FF));
		//检测最近一次的I2C1事件是否为字节发送事件
		m_EEPROM_I2C1_Timeout = EEPROM_FLAG_TIMEOUT;
		while(!I2C_CheckEvent(EEPROM_I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
		{
				if((m_EEPROM_I2C1_Timeout--) == 0)
				return EEPROM_READ_FAILED;
		}  
		//重新配置I2C1的DMA通道为发送通道
		EEPROM_DMAConfig((uint32_t)pBuffer, (uint8_t)(*NumByteToWrite), EEPROM_I2C1_DIRECTION_TX);    
		//使能DMA的发送通道
		DMA_Cmd(EEPROM_I2C1_DMA1_CHANNEL_TX, ENABLE); 
		//如果写配置完成则返回OK
		return EEPROM_WRITE_OK;
}

void EEPROM_Write(uint8_t* pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite)
{
		uint8_t NumOfPage = 0, NumOfSingle = 0, count = 0;
		uint16_t Addr = 0;

		//每次写入EEPROM的字节数
		static uint8_t  EEPROM_WriteDataNum = 0;

		//取写入地址在某一页的那个位置
		Addr = WriteAddr % AT24C64B_PAGESIZE;
		//取最后一页未写入数据部分的大小
		count = AT24C64B_PAGESIZE - Addr;

		//写入数据的长度占用满一页的个数
		NumOfPage =  NumByteToWrite / AT24C64B_PAGESIZE;
		//写入数据的长度占用不足一页的个数
		NumOfSingle = NumByteToWrite % AT24C64B_PAGESIZE;

		//写入的地址刚好在EEPROM地址中一页的开始
		if(Addr == 0) 
		{
				//写入数据的长度不足一页
				if(NumOfPage == 0) 
				{
						//写入不足一页的数据的个数
						EEPROM_WriteDataNum = NumOfSingle;
						//开启DMA写操作
						EEPROM_WritePage(pBuffer, WriteAddr, (uint8_t*)(&EEPROM_WriteDataNum));
						//等待DMA写操作完成
						m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;
						while (EEPROM_WriteDataNum > 0)
						{
								if((m_EEPROM_I2C1_Timeout--) == 0) 				
								//return EEPROM_READ_FAILED;
								//若指定时间内写操作完成不了，则退出
								break;
						}			
						//等待外部EEPROM设备写入数据完成
						EEPROM_WaitStandbyState();
				}
				else  
				{
						//写入数据的长度多于一页

						//先写入满一页的数据
						while(NumOfPage--)
						{
								//直接向当前地址中写入1页大小的数据
								EEPROM_WriteDataNum = AT24C64B_PAGESIZE;
								//启动DMA写操作        
								EEPROM_WritePage(pBuffer, WriteAddr, (uint8_t*)(&EEPROM_WriteDataNum)); 
								//等待DMA写操作完成
								m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;
								while (EEPROM_WriteDataNum > 0)
								{
										if((m_EEPROM_I2C1_Timeout--) == 0) 				
										//return EEPROM_READ_FAILED;
										//若指定时间内写操作完成不了，则退出
										break;
								}    
								//等待外部EEPROM设备写入数据完成
								EEPROM_WaitStandbyState();

								//写入地址增加，定位下一个写入的地址
								WriteAddr +=  AT24C64B_PAGESIZE;
								//去掉已经写入的数据，定位写入字节的指针到指定位置
								pBuffer += AT24C64B_PAGESIZE;
						}

						//第二步写入剩余不足一页的数据
						if(NumOfSingle!=0)
						{
								//写入不足一页的数据的个数
								EEPROM_WriteDataNum = NumOfSingle;
								//开启DMA写操作
								EEPROM_WritePage(pBuffer, WriteAddr, (uint8_t*)(&EEPROM_WriteDataNum));
								//等待DMA写操作完成
								m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;
								while (EEPROM_WriteDataNum > 0)
								{
										if((m_EEPROM_I2C1_Timeout--) == 0) 				
										//return EEPROM_READ_FAILED;
										//若指定时间内写操作完成不了，则退出
										break;
								}				
								//等待外部EEPROM设备写入数据完成
								EEPROM_WaitStandbyState();
						}
				}
		}
		else if(Addr != 0)
		{
				//写入的地址不在EEPROM地址中一页的开始

				//写入数据大小不足一页	
				if(NumOfPage == 0) 
				{
						//写入字节的个数多于当前页剩余的空闲字节
						if (NumByteToWrite > count)
						{
						//先向EEPROM中的当前页写入和当前页剩余的空闲字节相同的数据
						EEPROM_WriteDataNum = count;        
						//开启DMA写操作
						EEPROM_WritePage(pBuffer, WriteAddr, (uint8_t*)(&EEPROM_WriteDataNum));
						//等待DMA写操作完成
						m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;
						while (EEPROM_WriteDataNum > 0)
						{
								if((m_EEPROM_I2C1_Timeout--) == 0) 				
								//return EEPROM_READ_FAILED;
								//若指定时间内写操作完成不了，则退出
								break;
						}				
						//等待外部EEPROM设备写入数据完成
						EEPROM_WaitStandbyState();    

						//然后从下一个页的开始写入剩余的字节数
						EEPROM_WriteDataNum = (NumByteToWrite - count);          
						//开启DMA写操作
						EEPROM_WritePage((uint8_t*)(pBuffer + count), (WriteAddr + count), (uint8_t*)(&EEPROM_WriteDataNum));
						//等待DMA写操作完成
						m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;
						while (EEPROM_WriteDataNum > 0)
						{
								if((m_EEPROM_I2C1_Timeout--) == 0) 				
								//return EEPROM_READ_FAILED;
								//若指定时间内写操作完成不了，则退出
								break;
						}				
						//等待外部EEPROM设备写入数据完成
						EEPROM_WaitStandbyState();       
						}
						else if (NumByteToWrite <= count)      
						{
								//写入字节的个数小于当前页剩余的空闲位置

								//直接向该地址写入所有数据
								EEPROM_WriteDataNum = NumOfSingle;  
								//开启DMA写操作
								EEPROM_WritePage(pBuffer, WriteAddr, (uint8_t*)(&EEPROM_WriteDataNum));
								//等待DMA写操作完成
								m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;
								while (EEPROM_WriteDataNum > 0)
								{
										if((m_EEPROM_I2C1_Timeout--) == 0) 				
										//return EEPROM_READ_FAILED;
										//若指定时间内写操作完成不了，则退出
										break;
								}				
								//等待外部EEPROM设备写入数据完成
								EEPROM_WaitStandbyState();           
						}     
				}
				else if(NumOfPage != 0)     
				{
						//写入数据大小超过一页

						//判断将当前页写完之后,缓冲区还剩余多少个字节
						NumByteToWrite -= count;
						//判断剩余字节可写多少页
						NumOfPage =  NumByteToWrite / AT24C64B_PAGESIZE;
						//判断剩余字节不足一页的数据
						NumOfSingle = NumByteToWrite % AT24C64B_PAGESIZE;

						//先向EEPROM中的当前页写入和当前页剩余的空闲字节相同的数据
						if(count != 0)
						{				
								EEPROM_WriteDataNum = count;     
								//开启DMA写操作
								EEPROM_WritePage(pBuffer, WriteAddr, (uint8_t*)(&EEPROM_WriteDataNum));
								//等待DMA写操作完成
								m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;
								while (EEPROM_WriteDataNum > 0)
								{
										if((m_EEPROM_I2C1_Timeout--) == 0) 				
										//return EEPROM_READ_FAILED;
										//若指定时间内写操作完成不了，则退出
										break;
								}				
								//等待外部EEPROM设备写入数据完成
								EEPROM_WaitStandbyState();  

								//重新定位下一个写入的地址,肯定为一页的开始
								WriteAddr += count;
								//去掉已经写完的字节数,定位到需要写入的字符的地址
								pBuffer += count;
						} 

						//先后向新的页中写入1页的内容
						while(NumOfPage--)
						{
								//直接向当前地址中写入1页大小的数据
								EEPROM_WriteDataNum = AT24C64B_PAGESIZE;   
								//开启DMA写操作
								EEPROM_WritePage(pBuffer, WriteAddr, (uint8_t*)(&EEPROM_WriteDataNum));
								//等待DMA写操作完成
								m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;
								while (EEPROM_WriteDataNum > 0)
								{
										if((m_EEPROM_I2C1_Timeout--) == 0) 				
										//return EEPROM_READ_FAILED;
										//若指定时间内写操作完成不了，则退出
										break;
								}				
								//等待外部EEPROM设备写入数据完成
								EEPROM_WaitStandbyState();  

								//重新定位下一个写入的地址,肯定为一页的开始
								WriteAddr +=  AT24C64B_PAGESIZE;
								//去掉已经写完的字节数,定位到需要写入的字符的地址
								pBuffer += AT24C64B_PAGESIZE;  
						}

						//最后向新的页中写入不足一页的内容的字节数据
						if(NumOfSingle != 0)
						{	
								//直接向当前地址中写入剩余的数据
								EEPROM_WriteDataNum = NumOfSingle;      
								//开启DMA写操作
								EEPROM_WritePage(pBuffer, WriteAddr, (uint8_t*)(&EEPROM_WriteDataNum)); 
								//等待DMA写操作完成
								m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;
								while (EEPROM_WriteDataNum > 0)
								{
										if((m_EEPROM_I2C1_Timeout--) == 0) 				
										//return EEPROM_READ_FAILED;
										//若指定时间内写操作完成不了，则退出
										break;
								}				
								//等待外部EEPROM设备写入数据完成
								EEPROM_WaitStandbyState(); 
						}
				}//写入数据大小超过一页
		}//写入的地址不在EEPROM地址中一页的开始  
}
//EEPROM_Write()的第三个参数需要付给变量，进行再次封装
void EEPROM_I2C_WriteBuffer( uint16_t WriteAddr,uint8_t* pBuffer, uint16_t NumByteToWrite)
{
		unsigned short eeprom_WriteBuffer_Size;
		eeprom_WriteBuffer_Size =  NumByteToWrite;
		EEPROM_Write(pBuffer,WriteAddr,eeprom_WriteBuffer_Size);
}
//等待发送完成的数据的最大个数
#define EEPROM_MAX_TRIALS_NUMBER    150

/************************************************************************
功能: 
	等待EEPROM已经完成写操作。一般在最后一次写操作完成之后使用，
在外部EEPROM设备接收到需要写入的字节后，它需要额外的时间来完成本次写操作，
在这期间它将不会对I2C的任何命令产生应答信号，直到本次写操作完成。

返回值:
	完成返回--EEPROM_READ_OK
	未完成返回--EEPROM_READ_FAILED	
************************************************************************/
uint32_t EEPROM_WaitStandbyState(void)      
{
		uint16_t tmpSR1 = 0;
		uint32_t EEPROM_Trials = 0;

		//等待I2C1总线释放
		m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;
		while(I2C_GetFlagStatus(EEPROM_I2C1, I2C_FLAG_BUSY))
		{
				if((m_EEPROM_I2C1_Timeout--) == 0) 				
				return EEPROM_READ_FAILED;
		}

		while (1)
		{
				//I2C发送一个START命令
				I2C_GenerateSTART(EEPROM_I2C1, ENABLE);

				//检测最近一次的I2C事件是否为模式选择事件
				m_EEPROM_I2C1_Timeout = EEPROM_FLAG_TIMEOUT;
				while(!I2C_CheckEvent(EEPROM_I2C1, I2C_EVENT_MASTER_MODE_SELECT))
				{
						if((m_EEPROM_I2C1_Timeout--) == 0) 				
						return EEPROM_READ_FAILED;
				}    

				//向外部器件发送地址选择
				I2C_Send7bitAddress(EEPROM_I2C1, EEPROM_I2C1_AT24C64_SLAVE_ADDRESS7, I2C_Direction_Transmitter);

				//等待外部器件的返回应答信号
				m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;		
				//检测I2C是否发送完成数据,是否有应答错误
				//若发送完成且无应答错误,则发送成功
				//若未发送完成或者有应答错误,则等待
				do
				{     
						//读状态寄存器
						tmpSR1 = EEPROM_I2C1->SR1;

						//若在指定时间未检测到数据返回，则发生超时
						if((m_EEPROM_I2C1_Timeout--) == 0) 				
						return EEPROM_READ_FAILED;
				}
				while((tmpSR1 & (I2C_SR1_ADDR | I2C_SR1_AF)) == 0);

				//检测是否发送完成
				if (tmpSR1 & I2C_SR1_ADDR)
				{
						//发送完成之后,读清零状态寄存器
						(void)EEPROM_I2C1->SR2;

						//I2C发送一个停止位
						I2C_GenerateSTOP(EEPROM_I2C1, ENABLE);

						//返回成功,退出
						return EEPROM_READ_OK;
				}
				else
				{
						//清除应答错误标志位
						I2C_ClearFlag(EEPROM_I2C1, I2C_FLAG_AF);                  
				}

				//如果等到最大的个数,则返回失败
				if (EEPROM_Trials++ == EEPROM_MAX_TRIALS_NUMBER)
				{
						return EEPROM_READ_FAILED;
				}
		}//end--of--while(1)
}

void EEPROM_I2C1_DMA1_TX_IRQHandler(void)
{
		unsigned char tx_TimeOut = 0;

		//检测DMA发送是否完成
		if(DMA_GetFlagStatus(EEPROM_I2C1_DMA1_FLAG_TX_TC) != RESET)
		{  
				//禁止DMA的发送中断，并清除中断标志位
				DMA_Cmd(EEPROM_I2C1_DMA1_CHANNEL_TX, DISABLE);
				//added 2012-03-24
				DMA_ClearFlag(EEPROM_I2C1_DMA1_FLAG_TX_TC);
				DMA_ClearFlag(EEPROM_I2C1_DMA1_FLAG_TX_GL);

				//等待所有在I2C总线上的数据发送完成
				m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;
				while(!I2C_GetFlagStatus(EEPROM_I2C1, I2C_FLAG_BTF))
				{
						//若在指定的时间内未发送完成,则超时
						if((m_EEPROM_I2C1_Timeout--) == 0) 
						{
								tx_TimeOut = 1;
								break;
						}
				}

				//I2C1产生停止位
				I2C_GenerateSTOP(EEPROM_I2C1, ENABLE);

				//读I2C1的状态寄存器,清空事件
				(void)EEPROM_I2C1->SR1;
				(void)EEPROM_I2C1->SR2;

				//如果未超时,则清零数据的长度指针地址内容
				if (!tx_TimeOut)
				{
						//清零数据写指针
						*m_EEPROM_I2C1_DataWritePointer = 0;  
				}		
		}
}

void EEPROM_I2C1_DMA1_RX_IRQHandler(void)
{
		//检测DMA接收是否完成
		if(DMA_GetITStatus(EEPROM_I2C1_DMA1_FLAG_RX_TC) != RESET)
		{      
				//I2C1发送停止位
				I2C_GenerateSTOP(EEPROM_I2C1, ENABLE);    

				//禁止DMA的接收通道中断，并清除中断标志位
				DMA_Cmd(EEPROM_I2C1_DMA1_CHANNEL_RX, DISABLE);
				//added 2012-03-24
				DMA_ClearFlag(EEPROM_I2C1_DMA1_FLAG_RX_TC);
				DMA_ClearFlag(EEPROM_I2C1_DMA1_FLAG_RX_GL);

				//清零数据读指针
				*m_EEPROM_I2C1_DataReadPointer = 0;
		}
}


#ifdef USE_DEFAULT_CRITICAL_CALLBACK
void EEPROM_EnterCriticalSection_UserCallback(void)
{
		__disable_irq();  
}

void EEPROM_ExitCriticalSection_UserCallback(void)
{
		__enable_irq();
}
#endif

