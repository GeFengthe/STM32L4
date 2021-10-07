#ifndef __UART_H
#define __UART_H
#include "stm32l4xx.h"
#include "timer.h"
#include "stm32l4xx_hal_rcc.h"

#define UART2_Tx_Pin                GPIO_PIN_2
#define UART2_Rx_Pin                GPIO_PIN_3
#define ESP8266_RST_Pin             GPIO_PIN_1                      //接esp8266的key引脚    低电平复位
#define ESP8266_LED_Pin             GPIO_PIN_0                      //接esp8266的固件下载控制脚:高电平:正常运行状态， 低电平:下载模式


#define UART2_Port                  GPIOA
#define ESP8266_Port                GPIOE

#define UART2_REC_LEN               2048

typedef struct {
    uint16_t uart_rx_sta;
    uint8_t  uart_rx_buf[UART2_REC_LEN];
}__uart_sta;

extern __uart_sta   uart2_sta;

void uart2_init(void);
void esp8266_init(void);
void UART2_SendStr(unsigned char *p);
void UART2_SendStr_Len(unsigned char *p, uint16_t len);
#endif

