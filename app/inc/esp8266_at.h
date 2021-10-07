#ifndef __ESP8266_H
#define __ESP8266_H


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

extern __esp8266_at_com ESP8266_AT_COM;

void Star_at_waittim(unsigned int tim);
#endif