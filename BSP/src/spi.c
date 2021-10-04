#include "spi.h"

SPI_HandleTypeDef	SPI_HandleStruct;


void Init_spi(void)
{
	
	SPI_HandleStruct.Instance = SPI3;
	SPI_HandleStruct.Init.Mode = SPI_MODE_MASTER;					//����ģʽ
	SPI_HandleStruct.Init.Direction = SPI_DIRECTION_2LINES;			//˫��ģʽ
	SPI_HandleStruct.Init.DataSize = SPI_DATASIZE_8BIT;
	SPI_HandleStruct.Init.CLKPolarity = SPI_POLARITY_HIGH;			//����ͬ��ʱ�ӵĿ���״̬Ϊ�ߵ�ƽ
	SPI_HandleStruct.Init.CLKPhase = SPI_PHASE_2EDGE;				//����ͬ��ʱ�ӵĵڶ��������أ��������½������ݱ�����
	SPI_HandleStruct.Init.NSS = SPI_NSS_SOFT;
	SPI_HandleStruct.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;	//���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ256
	SPI_HandleStruct.Init.FirstBit =SPI_FIRSTBIT_MSB;					//ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
	SPI_HandleStruct.Init.TIMode = SPI_TIMODE_DISABLE;					//�ر�TIģʽ
	SPI_HandleStruct.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;	//�ر�Ӳ��CRCУ��
	SPI_HandleStruct.Init.CRCPolynomial = 7;							//crc����ʽ��ֵ
	HAL_SPI_Init(&SPI_HandleStruct);
    __HAL_SPI_ENABLE(&SPI_HandleStruct);
	
}

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_GPIOB_CLK_ENABLE();       //ʹ��GPIOBʱ��
    __HAL_RCC_SPI3_CLK_ENABLE();

    //PB3.5
    GPIO_Initure.Pin=GPIO_PIN_3|GPIO_PIN_5;
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;              //�����������
    GPIO_Initure.Pull=GPIO_PULLUP;                  //����
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;   //80Mhz           
    GPIO_Initure.Alternate=GPIO_AF6_SPI3;           //PB3.5����ΪSPI3
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
}

//��дһ���ֽ�
uint8_t SPI3_ReadWriteByte(uint8_t TxData)
{
	uint8_t Rxdata;
	Rxdata =HAL_SPI_TransmitReceive(&SPI_HandleStruct,&TxData , &Rxdata,1,300);
	return Rxdata;
}

//дһ���ֽ�
uint8_t SPI3_WriteByte(uint8_t *Txdata,uint16_t size)
{
	return HAL_SPI_Transmit(&SPI_HandleStruct,Txdata, size, 300);
	
}
