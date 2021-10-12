#ifndef __ESP8266_H
#define __ESP8266_H
#include "FreeRTOS.h"
#include "semphr.h"

typedef struct {
    unsigned char *at_cmd;                                         //at指令
    unsigned char *at_ack;                                          //at指令返回预期结果
    unsigned int at_timeout;                                        //at指令返回超时时间
    unsigned char at_sendnum;                                       //at指令重发次数
    
}__atcom_sta;

typedef struct {
    __atcom_sta ATCOM_STA;                                          //AT指令发送
    unsigned char at_type;                                          //AT类型
    unsigned tcp_connect;                                           //tcp连接标志
    unsigned char *tcp_send_dat;
    unsigned int tcp_send_len;          
}__esp8266_at_com;

enum{
    ESP8266_SEMAPH =0,
}semaphr_binary;

//AT指令工作处理
enum{
    AT_TYPE_CHECK   =0,                                         //检查模块是否存在
    AT_TYPE_CLOSE_SHOW,                                         //关闭回显
    AT_TYPE_ESPMODE_STA,
    AT_TYPE_ESPMODE_SET,
    AT_TYPE_ESPWIFI_STA,
    AT_TYPE_ESPWIFI_SET,
    AT_TYPE_CONNECT_SERVER,
    AT_TYPE_DISCON_SERVER,
}AT_TYPE;


enum{
    AT_TAB_CHECK =0,
    AT_TAB_CLOSE_SHOE,
    AT_TAB_ESPMODE_STA,
    AT_TAB_ESPMODE_SET,
    AT_TAB_ESPWIFI_STA,
    AT_TAB_ESPWIFI_SET,
    AT_TAB_CONNECT_SERVER,
    AT_TAB_DISCON_SERVER,
}AT_ESP8266_TAB;

extern __esp8266_at_com ESP8266_AT_COM;

extern SemaphoreHandle_t sky_semaph[3];                                        //三个信号量

void Star_at_waittim(unsigned int tim);
extern void esp8266_thread_init(void);

extern void AT_CheckSend(void);
extern void AT_Mode_Correct_Check(unsigned char *at_ack_buf);

#endif