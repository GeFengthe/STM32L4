#ifndef __ESP8266_H
#define __ESP8266_H
#include "FreeRTOS.h"
#include "semphr.h"

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

enum{
    ESP8266_SEMAPH =0,
}semaphr_binary;

//ATָ�������
enum{
    AT_TYPE_CHECK   =0,                                         //���ģ���Ƿ����
    AT_TYPE_CLOSE_SHOW,                                         //�رջ���
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

extern SemaphoreHandle_t sky_semaph[3];                                        //�����ź���

void Star_at_waittim(unsigned int tim);
extern void esp8266_thread_init(void);

extern void AT_CheckSend(void);
extern void AT_Mode_Correct_Check(unsigned char *at_ack_buf);

#endif