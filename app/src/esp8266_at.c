#include "esp8266_at.h"
#include "uart2.h"
#include "string.h"
#include "timer.h"
#include "stdlib.h"
#include "stdio.h"


const __atcom_sta atesp8266_tab[] ={
{"AT"                           , "OK"      ,100, 50},
{"ATE0"                         , "OK"      ,100, 50},                      //关闭回显
};

__esp8266_at_com ESP8266_AT_COM;
//字符串发送函数
void ATCode_Sendstr(unsigned char *str)
{
    UART2_SendStr(str);
}
//按照数据长度发送接口
//dat:数据指针  len:数据长度
void ATCode_SendStr_Len(unsigned char *dat, unsigned int len)
{
    UART2_SendStr_Len(dat, len);
}

void Star_at_waittim(unsigned int tim)
{
    STAR_ATESP8266_TIMER(tim);
}

void Stop_at_timer(void)
{
    STOP_ATESP8266_TIMER;
}
//AT指令 返回参数检查
void AT_Mode_Correct_Check(unsigned char *at_ack_buf)
{
    if(ESP8266_AT_COM.ATCOM_STA.at_ack ==NULL) return ;
    
    
}