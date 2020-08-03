

#include "eeprom_i2c.h"



//�ͷ�I2Cռ�õ�������Դ
void EEPROM_DeInit(void);

//��ʼ��I2C��EEPROM�Ķ�д
void EEPROM_I2C_Init(void);


//ϵͳʱ��Ϊ72M,�����ʱΪ1ms
#define EEPROM_FLAG_TIMEOUT         ((uint32_t)72000)
//��ʱ10ms,�㹻EEPROM��дһҳ128���ֽڵ�ʱ��
#define EEPROM_LONG_TIMEOUT         ((uint32_t)(10*EEPROM_FLAG_TIMEOUT))

//AT24C64����ҳ,1ҳ32���ֽ�
//AT24C64B��ʹ�õ�ʱ���谴ҳ����д����,����д��ͬһҳ��λ����ȥ
#define AT24C64B_PAGESIZE 32

////�ͷ�I2Cռ�õ�������Դ
//void EEPROM_I2C_DeInit(void);
////��ʼ��I2C���ⲿEEPROM�Ŀ���
//void EEPROM_I2C_Init(void);
//I2C1���ⲿEEPROM�Ŀ���,��ȡ�ͷ��͵��ֽڼ�����
void EEPROM_DMAConfig(uint32_t pBuffer, uint32_t BufferSize, uint32_t Direction);


//ȫ�ֺ�����ʼ��
//uint32_t EEPROM_I2C_ReadBuffer(uint8_t* pBuffer, uint16_t ReadAddr, uint16_t* NumByteToRead);
//void EEPROM_WriteBuffer(uint8_t* pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite);
uint32_t EEPROM_WritePage(uint8_t* pBuffer, uint16_t WriteAddr, uint8_t* NumByteToWrite);
uint32_t EEPROM_WaitStandbyState(void);

//��ʱ����,����д�Ĺ����г��ֳ�ʱ,����ûص�����
#define USE_DEFAULT_CRITICAL_CALLBACK
void EEPROM_EnterCriticalSection_UserCallback(void);
void EEPROM_ExitCriticalSection_UserCallback(void);



//��EEPROM���ݳ��ȵ�ָ�� -- ��ָ��ָʾ�Ƿ��д��� -- ��DMA��������ж�������
uint16_t* m_EEPROM_I2C1_DataReadPointer;

//��EEPROMд�����ݵĳ��ȵ�ָ�� -- ��ָ��ָʾ�Ƿ�д��� -- ��DMA��������ж�������
uint8_t* m_EEPROM_I2C1_DataWritePointer;

//��EEPROM�ĳ�ʱ����,���ڹ涨��ʱ����δ��ȡ������,�򷵻�
uint32_t  m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;

//���ԣ���ȡ��ʱʱ��
//#define DEBUG_TIMEOUT
#ifdef 	DEBUG_TIMEOUT
unsigned char time_flag = 0;
#endif

GPIO_InitTypeDef  GPIO_InitStructure;
NVIC_InitTypeDef  NVIC_InitStructure;
DMA_InitTypeDef   EEPROM_DMA_InitStructure;
I2C_InitTypeDef   I2C_InitStructure;

//�ͷ�I2Cռ�õ�������Դ
void EEPROM_I2C_DeInit(void)
{	
		//�ر�I2C1�Ĺ���
		I2C_Cmd(EEPROM_I2C1, DISABLE);
		
		//�ͷ�I2C1��������Դ
		I2C_DeInit(EEPROM_I2C1);
		
		//�ر�I2C1��ʱ��
		RCC_APB1PeriphClockCmd(EEPROM_I2C1_CLK, DISABLE);
			
		//����I2C1��SCL�˿�
		GPIO_InitStructure.GPIO_Pin = EEPROM_SCL_I2C1_SCL_PB6;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(EEPROM_SCL_I2C1_GPIO_PORT, &GPIO_InitStructure);
		
		//����I2C1��SDA�˿�
		GPIO_InitStructure.GPIO_Pin = EEPROM_SDA_I2C1_SDA_PB7;
		GPIO_Init(EEPROM_SDA_I2C1_GPIO_PORT, &GPIO_InitStructure);
		
		//����I2C1��DMA1����ͨ��
		NVIC_InitStructure.NVIC_IRQChannel = EEPROM_I2C1_DMA1_TX_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EEPROM_I2C1_DMA1_PREPRIO;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = EEPROM_I2C1_DMA1_SUBPRIO;
		NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
		NVIC_Init(&NVIC_InitStructure);
		
		//����I2C1��DMA1����ͨ��
		NVIC_InitStructure.NVIC_IRQChannel = EEPROM_I2C1_DMA1_RX_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EEPROM_I2C1_DMA1_PREPRIO;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = EEPROM_I2C1_DMA1_SUBPRIO;
		NVIC_Init(&NVIC_InitStructure);   
		
		//��ֹI2C1��DMA�ж�,���ͷ���Դ
		DMA_Cmd(EEPROM_I2C1_DMA1_CHANNEL_TX, DISABLE);
		DMA_Cmd(EEPROM_I2C1_DMA1_CHANNEL_RX, DISABLE);
		DMA_DeInit(EEPROM_I2C1_DMA1_CHANNEL_TX);
		DMA_DeInit(EEPROM_I2C1_DMA1_CHANNEL_RX);
}

void EEPROM_I2C_Init(void)
{   
		//��I2C1��SDA��SCL����ʹ�õ�ʱ��
		RCC_APB2PeriphClockCmd(EEPROM_SCL_I2C1_GPIO_CLK | EEPROM_SDA_I2C1_GPIO_CLK, ENABLE);

		//��I2C1��ʱ��
		RCC_APB1PeriphClockCmd(EEPROM_I2C1_CLK, ENABLE);

		//�򿪸���IO�ڹ��� --2012-03-23
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);

		//����I2C1��SCL�˿�
		GPIO_InitStructure.GPIO_Pin = EEPROM_SCL_I2C1_SCL_PB6;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
		GPIO_Init(EEPROM_SCL_I2C1_GPIO_PORT, &GPIO_InitStructure);

		//����I2C1��SDA�˿�
		GPIO_InitStructure.GPIO_Pin = EEPROM_SDA_I2C1_SDA_PB7;
		GPIO_Init(EEPROM_SDA_I2C1_GPIO_PORT, &GPIO_InitStructure); 

		//added 2012-03-23
		//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

		//����I2C1��DMA1����ͨ��
		NVIC_InitStructure.NVIC_IRQChannel = EEPROM_I2C1_DMA1_TX_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EEPROM_I2C1_DMA1_PREPRIO;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = EEPROM_I2C1_DMA1_SUBPRIO;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);

		//����I2C1��DMA1����ͨ��
		NVIC_InitStructure.NVIC_IRQChannel = EEPROM_I2C1_DMA1_RX_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EEPROM_I2C1_DMA1_PREPRIO;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = EEPROM_I2C1_DMA1_SUBPRIO;
		NVIC_Init(&NVIC_InitStructure);  

		//����I2C1��DMAͨ��,��ʹ��DMAʱ��
		RCC_AHBPeriphClockCmd(EEPROM_I2C1_DMA1_CLK, ENABLE);

		//����I2C1��DMA1�ķ���ͨ��
		DMA_DeInit(EEPROM_I2C1_DMA1_CHANNEL_TX);
		EEPROM_DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)EEPROM_I2C1_DR_Address;
		//�ò��������ں�EEPROM��Χ����ͨѶ�Ĺ��������� -- �ڴ��ַ,��Ž��պͷ��͵�����
		EEPROM_DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)0;   
		//�ò��������ں�EEPROM��Χ����ͨѶ�Ĺ��������� -- ��־Ϊ���ͻ��߽���
		EEPROM_DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;    
		//�ò��������ں�EEPROM��Χ����ͨѶ�Ĺ��������� -- ͨѶ���ֽ�
		EEPROM_DMA_InitStructure.DMA_BufferSize = 0xFFFF;            
		EEPROM_DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		EEPROM_DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		EEPROM_DMA_InitStructure.DMA_PeripheralDataSize = DMA_MemoryDataSize_Byte;
		EEPROM_DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
		EEPROM_DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
		EEPROM_DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
		EEPROM_DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
		DMA_Init(EEPROM_I2C1_DMA1_CHANNEL_TX, &EEPROM_DMA_InitStructure);  

		//����I2C1��DMA1�Ľ���ͨ��
		DMA_DeInit(EEPROM_I2C1_DMA1_CHANNEL_RX);
		DMA_Init(EEPROM_I2C1_DMA1_CHANNEL_RX, &EEPROM_DMA_InitStructure);  

		//ʹ��I2C1��MDA1�Ľ��պͷ���ͨ�����ж�
		DMA_ITConfig(EEPROM_I2C1_DMA1_CHANNEL_TX, DMA_IT_TC, ENABLE);
		DMA_ITConfig(EEPROM_I2C1_DMA1_CHANNEL_RX, DMA_IT_TC, ENABLE);   

		//����I2C1��ģʽΪ��ģʽ
		I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
		I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
		//����I2C1��д��EEPROM�ĵ�ַ -- 0xA0 --��־��һ������
		I2C_InitStructure.I2C_OwnAddress1 = EEPROM_I2C1_AT24C64_SLAVE_ADDRESS7;
		I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
		I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
		//����I2C1��д��EEPROM������
		I2C_InitStructure.I2C_ClockSpeed = EEPROM_I2C1_AT24C64_SPEED;

		//��I2C1��ʱ��
		I2C_Cmd(EEPROM_I2C1, ENABLE);
		//ʹ��I2C1�Ĳ�������
		I2C_Init(EEPROM_I2C1, &I2C_InitStructure);	
		//����I2C1��DMA1ͨ��,��ʹ�������ж�	
		I2C_DMACmd(EEPROM_I2C1, ENABLE);
}

//I2C1���ⲿEEPROM�Ŀ���,��ȡ�ͷ��͵��ֽڼ�����
void EEPROM_DMAConfig(uint32_t pBuffer, uint32_t BufferSize, uint32_t Direction)
{ 
		//��������DMA�ķ��ͺͽ���ͨ��,�����³�ʼ��
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

//��EEPROM�ж�ȡ����,�����ɹ��򷵻� 
uint32_t EEPROM_I2C_Read(uint8_t* pBuffer, uint16_t ReadAddr, uint16_t NumByteToRead)
{ 
		static unsigned short readPoint = 0;
		readPoint =  NumByteToRead;
		//�ñ�����DMA��������ж�������,�ɼ���ֵ�Ƿ�Ϊ0,�Ӷ�ȷ��DMA�����Ƿ����
		m_EEPROM_I2C1_DataReadPointer = &readPoint;
		//added 2012-03-23
		I2C_ITConfig(EEPROM_I2C1,I2C_IT_ERR,ENABLE);
		//������æʱ,��ȴ�,ֱ�������ͷ�
		//����ָ����ʱ����,δ�ͷ�,�������
		m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;
		while(I2C_GetFlagStatus(EEPROM_I2C1, I2C_FLAG_BUSY))
		{
				if((m_EEPROM_I2C1_Timeout--) == 0)
				return EEPROM_READ_FAILED;
		}
		/*****************************��EEPROM��д����ĵ�ַ*****************************/
		//I2C1��������START
		I2C_GenerateSTART(EEPROM_I2C1, ENABLE);
		//������һ�ε�I2C1�¼��Ƿ�Ϊģʽѡ����¼�
		m_EEPROM_I2C1_Timeout = EEPROM_FLAG_TIMEOUT;
		while(!I2C_CheckEvent(EEPROM_I2C1, I2C_EVENT_MASTER_MODE_SELECT))
		{
				if((m_EEPROM_I2C1_Timeout--) == 0)
				return EEPROM_READ_FAILED;
		}
		//���ⲿEEPROM����Ƭѡ��ַ
		I2C_Send7bitAddress(EEPROM_I2C1, EEPROM_I2C1_AT24C64_SLAVE_ADDRESS7, I2C_Direction_Transmitter);
		//������һ�ε�I2C1�¼��Ƿ�Ϊ�����¼�
		m_EEPROM_I2C1_Timeout = EEPROM_FLAG_TIMEOUT;
		while(!I2C_CheckEvent(EEPROM_I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
		{
				if((m_EEPROM_I2C1_Timeout--) == 0)
				return EEPROM_READ_FAILED;
		} 
		//AT24C64B��Ҫ16λ�ĵ�ַѡ��
		//���ⲿ��EEPROM���͸ߵ�ַѡ��
		I2C_SendData(EEPROM_I2C1, (uint8_t)((ReadAddr & 0xFF00) >> 8));    
		//������һ�ε�I2C1�¼��Ƿ�Ϊ�����¼�
		m_EEPROM_I2C1_Timeout = EEPROM_FLAG_TIMEOUT;
		while(!I2C_CheckEvent(EEPROM_I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
		{
				if((m_EEPROM_I2C1_Timeout--) == 0)
				return EEPROM_READ_FAILED;
		}
		//���ⲿ��EEPROM���͵͵�ַѡ��
		I2C_SendData(EEPROM_I2C1, (uint8_t)(ReadAddr & 0x00FF));    
		//���I2C1�Ƿ�����ֽڵķ��Ͳ���
		m_EEPROM_I2C1_Timeout = EEPROM_FLAG_TIMEOUT;
		while(I2C_GetFlagStatus(EEPROM_I2C1, I2C_FLAG_BTF) == RESET)
		{
				if((m_EEPROM_I2C1_Timeout--) == 0)
				return EEPROM_READ_FAILED;
		}

		/*****************************��EEPROM��д����ĵ�ַ*****************************/
		//I2C1�����ڶ��δ���START
		I2C_GenerateSTART(EEPROM_I2C1, ENABLE);
		//������һ�ε�I2C1�¼��Ƿ�Ϊģʽѡ����¼�
		m_EEPROM_I2C1_Timeout = EEPROM_FLAG_TIMEOUT;
		while(!I2C_CheckEvent(EEPROM_I2C1, I2C_EVENT_MASTER_MODE_SELECT))
		{
				if((m_EEPROM_I2C1_Timeout--) == 0)
				return EEPROM_READ_FAILED;
		}
		//���ⲿ��EEPROM���Ͷ���ַ
		I2C_Send7bitAddress(EEPROM_I2C1, EEPROM_I2C1_AT24C64_SLAVE_ADDRESS7, I2C_Direction_Receiver);  
		//�����ж���ȡ�ֽڵĸ�������<2 -- ֱ�Ӷ�ȡ; ��>2ʹ��DMA��ʽ
		if ((uint16_t)(NumByteToRead) < 2)
		{
				//����ַ���ͱ�־λADSL�Ƿ�����,��Ϊ1���ַ����δ����,�����ȴ� 0--���Խ��з���
				m_EEPROM_I2C1_Timeout = EEPROM_FLAG_TIMEOUT;
				while(I2C_GetFlagStatus(EEPROM_I2C1, I2C_FLAG_ADDR) == RESET)
				{
						if((m_EEPROM_I2C1_Timeout--) == 0)
						return EEPROM_READ_FAILED;
				}     

				//�ر�I2C��Ӧ��
				I2C_AcknowledgeConfig(EEPROM_I2C1, DISABLE);   

				//�ر��ж�,�����ٽ���
				EEPROM_EnterCriticalSection_UserCallback();    
				//���I2C1��״̬
				(void)EEPROM_I2C1->SR2;    
				//I2C1����ֹͣ
				I2C_GenerateSTOP(EEPROM_I2C1, ENABLE);   
				//���ж�,�뿪�ٽ���
				EEPROM_ExitCriticalSection_UserCallback();

				//�ȴ������ⲿEEPROM�ķ�������
				m_EEPROM_I2C1_Timeout = EEPROM_FLAG_TIMEOUT;
				while(I2C_GetFlagStatus(EEPROM_I2C1, I2C_FLAG_RXNE) == RESET)
				{
						if((m_EEPROM_I2C1_Timeout--) == 0)
						return EEPROM_READ_FAILED;
				}

				//��ȡ���ص��ֽ�
				*pBuffer = I2C_ReceiveData(EEPROM_I2C1);

				//�ֽ�ָ���Լ�
				(uint16_t)(NumByteToRead)--;        

				//ȷ��ֹͣλ�Ѿ����
				m_EEPROM_I2C1_Timeout = EEPROM_FLAG_TIMEOUT;
				while(EEPROM_I2C1->CR1 & I2C_CR1_STOP)
				{
						if((m_EEPROM_I2C1_Timeout--) == 0)
						return EEPROM_READ_FAILED;
				}  

				//ʹ��I2C1��Ӧ��,Ϊ�´ζ�ȡ��׼��
				I2C_AcknowledgeConfig(EEPROM_I2C1, ENABLE);    
		}
		else
		{
				//���ֽ�>1��ʱ��,ʹ��DMA�ķ�ʽ
				m_EEPROM_I2C1_Timeout = EEPROM_FLAG_TIMEOUT;
				while(!I2C_CheckEvent(EEPROM_I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
				{
						if((m_EEPROM_I2C1_Timeout--) == 0)
						return EEPROM_READ_FAILED;
				}  

				//��������I2C1��DMAͨ��Ϊ��ͨ��
				EEPROM_DMAConfig((uint32_t)pBuffer, (uint16_t)(NumByteToRead), EEPROM_I2C1_DIRECTION_RX);
				//I2C1����һ��DMA����Ϊ���һ�δ���
				I2C_DMALastTransferCmd(EEPROM_I2C1, ENABLE); 
				//ʹ��DMA�Ľ��չ���
				DMA_Cmd(EEPROM_I2C1_DMA1_CHANNEL_RX, ENABLE);  

				//�ȴ�DMA�������,�ڽ������֮��m_EEPROM_I2C1_DataReadPointer����
				m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;
				while(*m_EEPROM_I2C1_DataReadPointer > 0)
				{
						//��������볬ʱ�ж�,��ʱ�ȵȴ�,��Ҫȷ�������ʱ��

						//����ָ����ʱ����δ��ȡ�ɹ�,�򷵻ش���
						if((m_EEPROM_I2C1_Timeout--) == 0)
						return EEPROM_READ_FAILED;
				}
		}	
		//����������ɹ�,�򷵻ض��ɹ�
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
		//�ñ�����DMA��������ж�������,�ɼ���ֵ�Ƿ�Ϊ0,�Ӷ�ȷ��DMA�����Ƿ����
		m_EEPROM_I2C1_DataWritePointer = NumByteToWrite;  

		//added 2012-03-24
		I2C_ITConfig(EEPROM_I2C1,I2C_IT_ERR,ENABLE);

		//������æʱ,��ȴ�,ֱ�������ͷ�
		//����ָ����ʱ����,δ�ͷ�,�������
		m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;
		while(I2C_GetFlagStatus(EEPROM_I2C1, I2C_FLAG_BUSY))
		{
				if((m_EEPROM_I2C1_Timeout--) == 0)
				return EEPROM_READ_FAILED;
		}

		/*****************************��EEPROM��д����ĵ�ַ*****************************/
		//I2C1��������START
		I2C_GenerateSTART(EEPROM_I2C1, ENABLE);  
		//������һ�ε�I2C1�¼��Ƿ�Ϊģʽѡ����¼�
		m_EEPROM_I2C1_Timeout = EEPROM_FLAG_TIMEOUT;
		while(!I2C_CheckEvent(EEPROM_I2C1, I2C_EVENT_MASTER_MODE_SELECT))
		{
				if((m_EEPROM_I2C1_Timeout--) == 0)
				return EEPROM_READ_FAILED;
		}

		//���ⲿEEPROM����Ƭѡ��ַ
		I2C_Send7bitAddress(EEPROM_I2C1, EEPROM_I2C1_AT24C64_SLAVE_ADDRESS7, I2C_Direction_Transmitter);
		//������һ�ε�I2C1�¼��Ƿ�Ϊ�����¼�,�ȴ��ⲿ������Ӧ��
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
		//AT24C64B��Ҫ16λ�ĵ�ַѡ��
		//���ⲿ��EEPROM���͸ߵ�ַѡ��
		I2C_SendData(EEPROM_I2C1, (uint8_t)((WriteAddr & 0xFF00) >> 8));
		//������һ�ε�I2C1�¼��Ƿ�Ϊ�ֽڷ����¼�
		m_EEPROM_I2C1_Timeout = EEPROM_FLAG_TIMEOUT;  
		while(!I2C_CheckEvent(EEPROM_I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
		{
				if((m_EEPROM_I2C1_Timeout--) == 0)
				return EEPROM_READ_FAILED;
		}  
		//���ⲿ��EEPROM���͵͵�ַѡ��
		I2C_SendData(EEPROM_I2C1, (uint8_t)(WriteAddr & 0x00FF));
		//������һ�ε�I2C1�¼��Ƿ�Ϊ�ֽڷ����¼�
		m_EEPROM_I2C1_Timeout = EEPROM_FLAG_TIMEOUT;
		while(!I2C_CheckEvent(EEPROM_I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
		{
				if((m_EEPROM_I2C1_Timeout--) == 0)
				return EEPROM_READ_FAILED;
		}  
		//��������I2C1��DMAͨ��Ϊ����ͨ��
		EEPROM_DMAConfig((uint32_t)pBuffer, (uint8_t)(*NumByteToWrite), EEPROM_I2C1_DIRECTION_TX);    
		//ʹ��DMA�ķ���ͨ��
		DMA_Cmd(EEPROM_I2C1_DMA1_CHANNEL_TX, ENABLE); 
		//���д��������򷵻�OK
		return EEPROM_WRITE_OK;
}

void EEPROM_Write(uint8_t* pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite)
{
		uint8_t NumOfPage = 0, NumOfSingle = 0, count = 0;
		uint16_t Addr = 0;

		//ÿ��д��EEPROM���ֽ���
		static uint8_t  EEPROM_WriteDataNum = 0;

		//ȡд���ַ��ĳһҳ���Ǹ�λ��
		Addr = WriteAddr % AT24C64B_PAGESIZE;
		//ȡ���һҳδд�����ݲ��ֵĴ�С
		count = AT24C64B_PAGESIZE - Addr;

		//д�����ݵĳ���ռ����һҳ�ĸ���
		NumOfPage =  NumByteToWrite / AT24C64B_PAGESIZE;
		//д�����ݵĳ���ռ�ò���һҳ�ĸ���
		NumOfSingle = NumByteToWrite % AT24C64B_PAGESIZE;

		//д��ĵ�ַ�պ���EEPROM��ַ��һҳ�Ŀ�ʼ
		if(Addr == 0) 
		{
				//д�����ݵĳ��Ȳ���һҳ
				if(NumOfPage == 0) 
				{
						//д�벻��һҳ�����ݵĸ���
						EEPROM_WriteDataNum = NumOfSingle;
						//����DMAд����
						EEPROM_WritePage(pBuffer, WriteAddr, (uint8_t*)(&EEPROM_WriteDataNum));
						//�ȴ�DMAд�������
						m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;
						while (EEPROM_WriteDataNum > 0)
						{
								if((m_EEPROM_I2C1_Timeout--) == 0) 				
								//return EEPROM_READ_FAILED;
								//��ָ��ʱ����д������ɲ��ˣ����˳�
								break;
						}			
						//�ȴ��ⲿEEPROM�豸д���������
						EEPROM_WaitStandbyState();
				}
				else  
				{
						//д�����ݵĳ��ȶ���һҳ

						//��д����һҳ������
						while(NumOfPage--)
						{
								//ֱ����ǰ��ַ��д��1ҳ��С������
								EEPROM_WriteDataNum = AT24C64B_PAGESIZE;
								//����DMAд����        
								EEPROM_WritePage(pBuffer, WriteAddr, (uint8_t*)(&EEPROM_WriteDataNum)); 
								//�ȴ�DMAд�������
								m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;
								while (EEPROM_WriteDataNum > 0)
								{
										if((m_EEPROM_I2C1_Timeout--) == 0) 				
										//return EEPROM_READ_FAILED;
										//��ָ��ʱ����д������ɲ��ˣ����˳�
										break;
								}    
								//�ȴ��ⲿEEPROM�豸д���������
								EEPROM_WaitStandbyState();

								//д���ַ���ӣ���λ��һ��д��ĵ�ַ
								WriteAddr +=  AT24C64B_PAGESIZE;
								//ȥ���Ѿ�д������ݣ���λд���ֽڵ�ָ�뵽ָ��λ��
								pBuffer += AT24C64B_PAGESIZE;
						}

						//�ڶ���д��ʣ�಻��һҳ������
						if(NumOfSingle!=0)
						{
								//д�벻��һҳ�����ݵĸ���
								EEPROM_WriteDataNum = NumOfSingle;
								//����DMAд����
								EEPROM_WritePage(pBuffer, WriteAddr, (uint8_t*)(&EEPROM_WriteDataNum));
								//�ȴ�DMAд�������
								m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;
								while (EEPROM_WriteDataNum > 0)
								{
										if((m_EEPROM_I2C1_Timeout--) == 0) 				
										//return EEPROM_READ_FAILED;
										//��ָ��ʱ����д������ɲ��ˣ����˳�
										break;
								}				
								//�ȴ��ⲿEEPROM�豸д���������
								EEPROM_WaitStandbyState();
						}
				}
		}
		else if(Addr != 0)
		{
				//д��ĵ�ַ����EEPROM��ַ��һҳ�Ŀ�ʼ

				//д�����ݴ�С����һҳ	
				if(NumOfPage == 0) 
				{
						//д���ֽڵĸ������ڵ�ǰҳʣ��Ŀ����ֽ�
						if (NumByteToWrite > count)
						{
						//����EEPROM�еĵ�ǰҳд��͵�ǰҳʣ��Ŀ����ֽ���ͬ������
						EEPROM_WriteDataNum = count;        
						//����DMAд����
						EEPROM_WritePage(pBuffer, WriteAddr, (uint8_t*)(&EEPROM_WriteDataNum));
						//�ȴ�DMAд�������
						m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;
						while (EEPROM_WriteDataNum > 0)
						{
								if((m_EEPROM_I2C1_Timeout--) == 0) 				
								//return EEPROM_READ_FAILED;
								//��ָ��ʱ����д������ɲ��ˣ����˳�
								break;
						}				
						//�ȴ��ⲿEEPROM�豸д���������
						EEPROM_WaitStandbyState();    

						//Ȼ�����һ��ҳ�Ŀ�ʼд��ʣ����ֽ���
						EEPROM_WriteDataNum = (NumByteToWrite - count);          
						//����DMAд����
						EEPROM_WritePage((uint8_t*)(pBuffer + count), (WriteAddr + count), (uint8_t*)(&EEPROM_WriteDataNum));
						//�ȴ�DMAд�������
						m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;
						while (EEPROM_WriteDataNum > 0)
						{
								if((m_EEPROM_I2C1_Timeout--) == 0) 				
								//return EEPROM_READ_FAILED;
								//��ָ��ʱ����д������ɲ��ˣ����˳�
								break;
						}				
						//�ȴ��ⲿEEPROM�豸д���������
						EEPROM_WaitStandbyState();       
						}
						else if (NumByteToWrite <= count)      
						{
								//д���ֽڵĸ���С�ڵ�ǰҳʣ��Ŀ���λ��

								//ֱ����õ�ַд����������
								EEPROM_WriteDataNum = NumOfSingle;  
								//����DMAд����
								EEPROM_WritePage(pBuffer, WriteAddr, (uint8_t*)(&EEPROM_WriteDataNum));
								//�ȴ�DMAд�������
								m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;
								while (EEPROM_WriteDataNum > 0)
								{
										if((m_EEPROM_I2C1_Timeout--) == 0) 				
										//return EEPROM_READ_FAILED;
										//��ָ��ʱ����д������ɲ��ˣ����˳�
										break;
								}				
								//�ȴ��ⲿEEPROM�豸д���������
								EEPROM_WaitStandbyState();           
						}     
				}
				else if(NumOfPage != 0)     
				{
						//д�����ݴ�С����һҳ

						//�жϽ���ǰҳд��֮��,��������ʣ����ٸ��ֽ�
						NumByteToWrite -= count;
						//�ж�ʣ���ֽڿ�д����ҳ
						NumOfPage =  NumByteToWrite / AT24C64B_PAGESIZE;
						//�ж�ʣ���ֽڲ���һҳ������
						NumOfSingle = NumByteToWrite % AT24C64B_PAGESIZE;

						//����EEPROM�еĵ�ǰҳд��͵�ǰҳʣ��Ŀ����ֽ���ͬ������
						if(count != 0)
						{				
								EEPROM_WriteDataNum = count;     
								//����DMAд����
								EEPROM_WritePage(pBuffer, WriteAddr, (uint8_t*)(&EEPROM_WriteDataNum));
								//�ȴ�DMAд�������
								m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;
								while (EEPROM_WriteDataNum > 0)
								{
										if((m_EEPROM_I2C1_Timeout--) == 0) 				
										//return EEPROM_READ_FAILED;
										//��ָ��ʱ����д������ɲ��ˣ����˳�
										break;
								}				
								//�ȴ��ⲿEEPROM�豸д���������
								EEPROM_WaitStandbyState();  

								//���¶�λ��һ��д��ĵ�ַ,�϶�Ϊһҳ�Ŀ�ʼ
								WriteAddr += count;
								//ȥ���Ѿ�д����ֽ���,��λ����Ҫд����ַ��ĵ�ַ
								pBuffer += count;
						} 

						//�Ⱥ����µ�ҳ��д��1ҳ������
						while(NumOfPage--)
						{
								//ֱ����ǰ��ַ��д��1ҳ��С������
								EEPROM_WriteDataNum = AT24C64B_PAGESIZE;   
								//����DMAд����
								EEPROM_WritePage(pBuffer, WriteAddr, (uint8_t*)(&EEPROM_WriteDataNum));
								//�ȴ�DMAд�������
								m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;
								while (EEPROM_WriteDataNum > 0)
								{
										if((m_EEPROM_I2C1_Timeout--) == 0) 				
										//return EEPROM_READ_FAILED;
										//��ָ��ʱ����д������ɲ��ˣ����˳�
										break;
								}				
								//�ȴ��ⲿEEPROM�豸д���������
								EEPROM_WaitStandbyState();  

								//���¶�λ��һ��д��ĵ�ַ,�϶�Ϊһҳ�Ŀ�ʼ
								WriteAddr +=  AT24C64B_PAGESIZE;
								//ȥ���Ѿ�д����ֽ���,��λ����Ҫд����ַ��ĵ�ַ
								pBuffer += AT24C64B_PAGESIZE;  
						}

						//������µ�ҳ��д�벻��һҳ�����ݵ��ֽ�����
						if(NumOfSingle != 0)
						{	
								//ֱ����ǰ��ַ��д��ʣ�������
								EEPROM_WriteDataNum = NumOfSingle;      
								//����DMAд����
								EEPROM_WritePage(pBuffer, WriteAddr, (uint8_t*)(&EEPROM_WriteDataNum)); 
								//�ȴ�DMAд�������
								m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;
								while (EEPROM_WriteDataNum > 0)
								{
										if((m_EEPROM_I2C1_Timeout--) == 0) 				
										//return EEPROM_READ_FAILED;
										//��ָ��ʱ����д������ɲ��ˣ����˳�
										break;
								}				
								//�ȴ��ⲿEEPROM�豸д���������
								EEPROM_WaitStandbyState(); 
						}
				}//д�����ݴ�С����һҳ
		}//д��ĵ�ַ����EEPROM��ַ��һҳ�Ŀ�ʼ  
}
//EEPROM_Write()�ĵ�����������Ҫ���������������ٴη�װ
void EEPROM_I2C_WriteBuffer( uint16_t WriteAddr,uint8_t* pBuffer, uint16_t NumByteToWrite)
{
		unsigned short eeprom_WriteBuffer_Size;
		eeprom_WriteBuffer_Size =  NumByteToWrite;
		EEPROM_Write(pBuffer,WriteAddr,eeprom_WriteBuffer_Size);
}
//�ȴ�������ɵ����ݵ�������
#define EEPROM_MAX_TRIALS_NUMBER    150

/************************************************************************
����: 
	�ȴ�EEPROM�Ѿ����д������һ�������һ��д�������֮��ʹ�ã�
���ⲿEEPROM�豸���յ���Ҫд����ֽں�����Ҫ�����ʱ������ɱ���д������
�����ڼ����������I2C���κ��������Ӧ���źţ�ֱ������д������ɡ�

����ֵ:
	��ɷ���--EEPROM_READ_OK
	δ��ɷ���--EEPROM_READ_FAILED	
************************************************************************/
uint32_t EEPROM_WaitStandbyState(void)      
{
		uint16_t tmpSR1 = 0;
		uint32_t EEPROM_Trials = 0;

		//�ȴ�I2C1�����ͷ�
		m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;
		while(I2C_GetFlagStatus(EEPROM_I2C1, I2C_FLAG_BUSY))
		{
				if((m_EEPROM_I2C1_Timeout--) == 0) 				
				return EEPROM_READ_FAILED;
		}

		while (1)
		{
				//I2C����һ��START����
				I2C_GenerateSTART(EEPROM_I2C1, ENABLE);

				//������һ�ε�I2C�¼��Ƿ�Ϊģʽѡ���¼�
				m_EEPROM_I2C1_Timeout = EEPROM_FLAG_TIMEOUT;
				while(!I2C_CheckEvent(EEPROM_I2C1, I2C_EVENT_MASTER_MODE_SELECT))
				{
						if((m_EEPROM_I2C1_Timeout--) == 0) 				
						return EEPROM_READ_FAILED;
				}    

				//���ⲿ�������͵�ַѡ��
				I2C_Send7bitAddress(EEPROM_I2C1, EEPROM_I2C1_AT24C64_SLAVE_ADDRESS7, I2C_Direction_Transmitter);

				//�ȴ��ⲿ�����ķ���Ӧ���ź�
				m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;		
				//���I2C�Ƿ����������,�Ƿ���Ӧ�����
				//�������������Ӧ�����,���ͳɹ�
				//��δ������ɻ�����Ӧ�����,��ȴ�
				do
				{     
						//��״̬�Ĵ���
						tmpSR1 = EEPROM_I2C1->SR1;

						//����ָ��ʱ��δ��⵽���ݷ��أ�������ʱ
						if((m_EEPROM_I2C1_Timeout--) == 0) 				
						return EEPROM_READ_FAILED;
				}
				while((tmpSR1 & (I2C_SR1_ADDR | I2C_SR1_AF)) == 0);

				//����Ƿ������
				if (tmpSR1 & I2C_SR1_ADDR)
				{
						//�������֮��,������״̬�Ĵ���
						(void)EEPROM_I2C1->SR2;

						//I2C����һ��ֹͣλ
						I2C_GenerateSTOP(EEPROM_I2C1, ENABLE);

						//���سɹ�,�˳�
						return EEPROM_READ_OK;
				}
				else
				{
						//���Ӧ������־λ
						I2C_ClearFlag(EEPROM_I2C1, I2C_FLAG_AF);                  
				}

				//����ȵ����ĸ���,�򷵻�ʧ��
				if (EEPROM_Trials++ == EEPROM_MAX_TRIALS_NUMBER)
				{
						return EEPROM_READ_FAILED;
				}
		}//end--of--while(1)
}

void EEPROM_I2C1_DMA1_TX_IRQHandler(void)
{
		unsigned char tx_TimeOut = 0;

		//���DMA�����Ƿ����
		if(DMA_GetFlagStatus(EEPROM_I2C1_DMA1_FLAG_TX_TC) != RESET)
		{  
				//��ֹDMA�ķ����жϣ�������жϱ�־λ
				DMA_Cmd(EEPROM_I2C1_DMA1_CHANNEL_TX, DISABLE);
				//added 2012-03-24
				DMA_ClearFlag(EEPROM_I2C1_DMA1_FLAG_TX_TC);
				DMA_ClearFlag(EEPROM_I2C1_DMA1_FLAG_TX_GL);

				//�ȴ�������I2C�����ϵ����ݷ������
				m_EEPROM_I2C1_Timeout = EEPROM_LONG_TIMEOUT;
				while(!I2C_GetFlagStatus(EEPROM_I2C1, I2C_FLAG_BTF))
				{
						//����ָ����ʱ����δ�������,��ʱ
						if((m_EEPROM_I2C1_Timeout--) == 0) 
						{
								tx_TimeOut = 1;
								break;
						}
				}

				//I2C1����ֹͣλ
				I2C_GenerateSTOP(EEPROM_I2C1, ENABLE);

				//��I2C1��״̬�Ĵ���,����¼�
				(void)EEPROM_I2C1->SR1;
				(void)EEPROM_I2C1->SR2;

				//���δ��ʱ,���������ݵĳ���ָ���ַ����
				if (!tx_TimeOut)
				{
						//��������дָ��
						*m_EEPROM_I2C1_DataWritePointer = 0;  
				}		
		}
}

void EEPROM_I2C1_DMA1_RX_IRQHandler(void)
{
		//���DMA�����Ƿ����
		if(DMA_GetITStatus(EEPROM_I2C1_DMA1_FLAG_RX_TC) != RESET)
		{      
				//I2C1����ֹͣλ
				I2C_GenerateSTOP(EEPROM_I2C1, ENABLE);    

				//��ֹDMA�Ľ���ͨ���жϣ�������жϱ�־λ
				DMA_Cmd(EEPROM_I2C1_DMA1_CHANNEL_RX, DISABLE);
				//added 2012-03-24
				DMA_ClearFlag(EEPROM_I2C1_DMA1_FLAG_RX_TC);
				DMA_ClearFlag(EEPROM_I2C1_DMA1_FLAG_RX_GL);

				//�������ݶ�ָ��
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

