#ifndef __TIMER_H
#define __TIMER_H
#include "stm32l4xx.h"
#include "uart2.h"

#define TIM_MAX         4

#define UART2_TIM_ID    0
#define AT_TIM_ID       1

#define AT_FLAG         0x80                    //AT指令超时标志位

#define STAR_UART2_TIMER(x)         tim_buff[UART2_TIM_ID] = x;
#define STOP_UART2_TIMER            tim_buff[UART2_TIM_ID] =0;

#define STAR_ATESP8266_TIMER(x)     tim_buff[AT_TIM_ID] = x;
#define STOP_ATESP8266_TIMER        tim_buff[AT_TIM_ID] = 0;


extern unsigned int tim_buff[TIM_MAX];
extern unsigned char Tim7_flag;

void Tim7_init(void);
#endif