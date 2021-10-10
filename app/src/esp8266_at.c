#include "esp8266_at.h"
#include "uart2.h"
#include "string.h"
#include "timer.h"
#include "stdlib.h"
#include "stdio.h"
#include "task.h"


//�ź���
SemaphoreHandle_t sky_semaph[3];                                        //�����ź���

const __atcom_sta atesp8266_tab[] ={
{"AT"                           , "OK"      ,100, 50},
{"ATE0"                         , "OK"      ,100, 50},                      //�رջ���
};

__esp8266_at_com ESP8266_AT_COM;
//�ַ������ͺ���
void ATCode_Sendstr(unsigned char *str)
{
    UART2_SendStr(str);
}
//�������ݳ��ȷ��ͽӿ�
//dat:����ָ��  len:���ݳ���
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
//����ATָ��
void ATCode_Send_Cmd(unsigned char *cmd)
{
    if(NULL != cmd)
    {
        uart2_sta.uart_rx_sta =0;                                                   //��ջ�����
        ATCode_Sendstr(cmd);
        ATCode_Sendstr("\r\n");
    }
}
//ATָ���
void AT_Mode_Send_Com(__atcom_sta *at_mode_com, unsigned char at_type)
{
    ESP8266_AT_COM.ATCOM_STA.at_ack             = at_mode_com->at_ack;
    ESP8266_AT_COM.ATCOM_STA.at_cmd             = at_mode_com->at_cmd;
    ESP8266_AT_COM.ATCOM_STA.at_sendnum         = at_mode_com->at_sendnum;
    ESP8266_AT_COM.ATCOM_STA.at_timeout         = at_mode_com->at_timeout;
    
    ESP8266_AT_COM.at_type = at_type;
    
    Stop_at_timer();
    ATCode_Send_Cmd(ESP8266_AT_COM.ATCOM_STA.at_cmd);
    Star_at_waittim(ESP8266_AT_COM.ATCOM_STA.at_timeout);
}

//AT ����

void AT_CheckSend(void)
{
    AT_Mode_Send_Com((__atcom_sta *)&atesp8266_tab[0], AT_TYPE_CHECK);
}



//ATָ����Ӧ��ȷ����
void AT_Mode_Correct_response(unsigned char *ack_buff)
{
    
}



//ATָ�� ���ز������
void AT_Mode_Correct_Check(unsigned char *at_ack_buf)
{
    char *strx =NULL;
    if(ESP8266_AT_COM.ATCOM_STA.at_ack ==NULL) return ;
    
    strx = strstr((char*)at_ack_buf, (char*)ESP8266_AT_COM.ATCOM_STA.at_ack);
    if(strx !=NULL)
    {
        printf("%s",at_ack_buf);
        printf(" ack success");
    }else
    {
        printf("%s",at_ack_buf);
    }
    
}



static void esp8266_thread_cb(void)
{
//    BaseType_t xReturn =pdPASS;
    while(1)
    {
//        xReturn = xSemaphoreTake(sky_semaph[ESP8266_SEMAPH],portMAX_DELAY);
//        if(xReturn ==pdPASS)
//        {
//            
//        }

    }
}


// esp8266 �����̺߳��ź������� ���ȼ�2
void esp8266_thread_init(void)
{
    BaseType_t xReturn =pdPASS;
    
    xReturn =xTaskCreate((TaskFunction_t)esp8266_thread_cb, "esp8266 task", 5000, NULL, 2,NULL);
    if(pdPASS == xReturn)
    {
        printf(" esp8266 task �������\r\n");
    }
    
}