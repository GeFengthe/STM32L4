#ifndef __ATH10_H
#define __ATH10_H
#include "stm32l4xx.h"
#include "delay.h"

//IO方向设置
#define SDA_IN()            {GPIOC->MODER &=~(3<<(1*2));GPIOC->MODER |=0<<(1*2);}               //PC1输入模式
#define SDA_OUT()           {GPIOC->MODER &=~(3<<(1*2));GPIOC->MODER |=1<<(1*2);}               //PC1输出模式

//IO操作
#define IIC_SCL(n)          (n?HAL_GPIO_WritePin(GPIOD,GPIO_PIN_6,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOD,GPIO_PIN_6,GPIO_PIN_RESET))
#define IIC_SDA(n)          (n?HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_RESET))
#define READ_SDA            HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_1)                  //输入SDA

void IIC_Init(void);
void IIC_Start(void);
void IIC_Stop(void);
uint8_t IIC_Wait_Ack(void);
void IIC_Ack(void);
void IIC_NAck(void);
void IIC_Send_Byte(uint8_t txd);
uint8_t IIC_Read_Byte(uint8_t ack);


#endif