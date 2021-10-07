#ifndef __ESP8266_H
#define __ESP8266_H


typedef struct {
    unsigned char *at_cmd;                                         //atָ��
    unsigned char *at_ack;                                          //atָ���Ԥ�ڽ��
    unsigned int at_timeout;                                        //atָ��س�ʱʱ��
    unsigned char at_sendnum;                                       //atָ���ط�����
    
}__atcom_sta;

typedef struct {
    __atcom_sta ATCOM_STA;                                          //ATָ���
    unsigned char at_type;                                          //AT����
    unsigned tcp_connect;                                           //tcp���ӱ�־
    unsigned char *tcp_send_dat;
    unsigned int tcp_send_len;          
}__esp8266_at_com;

extern __esp8266_at_com ESP8266_AT_COM;

void Star_at_waittim(unsigned int tim);
#endif