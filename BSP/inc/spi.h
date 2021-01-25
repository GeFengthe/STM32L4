#ifndef __SPI_H
#define __SPI_H
#include "stm32l4xx.h"

void Init_spi(void);
uint8_t SPI3_ReadWriteByte(uint8_t TxData);
uint8_t SPI3_WriteByte(uint8_t *Txdata,uint16_t size);


#endif