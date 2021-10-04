#include "stm32l4xx.h"
#include "sys.h"
#include "delay.h"
#include "stm32l4xx_hal_gpio.h"
#include "lcd.h"
#include "usart.h"
#include "esp8266.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"             //ʹ���ź���Ҫ������ͷ�ļ�
#include "mqtt.h"
#include "aht10.h"
#include "timers.h"


#define LED_R_Pin               GPIO_PIN_7
#define LED_G_Pin               GPIO_PIN_8
#define LED_B_Pin               GPIO_PIN_9

#define LED_Port                GPIOE

#define LED_R_ON                HAL_GPIO_WritePin(LED_Port,LED_R_Pin,GPIO_PIN_RESET)
#define LED_R_OFF               HAL_GPIO_WritePin(LED_Port,LED_R_Pin,GPIO_PIN_SET)

#define WIFISTA_SSID                    "HUAWEI-2303"               //wifi����
#define WIFISTA_PASSWORD                "zyj15251884308"            //��������
//�����Ʒ�������Ϣ
#define MQTT_BROKERADDRESS              "a16NWgAwqsz.iot-as-mqtt.cn-shanghai.aliyuncs.com"
#define MQTT_PORTNUM                    1883
#define MQTT_CLIENTID                   "00001|securemode=3,signmethod=hmacsha1|"
#define MQTT_USARNAME                   "Device2&a16NWgAwqsz"
#define MQTT_PASSWORD                   "630CF42D5F5CF44F51C8889800702BD0076E813E"          //

#define MQTT_PUBLISH_TOPIC              "/sys/a16NWgAwqsz/Device2/thing/event/property/post"
#define MQTT_SUBSCRIBE_TOPIC            "/sys/a16NWgAwqsz/Device2/thing/service/property/set"


void LED_Init(void);
//������ں���
static void appmain(void *parameter);
static void IOT_Esp8266(void *parameter);
static void IOT_Contral(void *parameter);
static void MQTT_CreateHeartTimer(void);


//MQTTҵ���ʼ��
void ESP8266_MQTT_Init(void);



//������
static TaskHandle_t AppMain_Handle=	NULL;
static TaskHandle_t ESP8266_Handle= NULL;
static TaskHandle_t IOTContral_Handle =NULL;

//�ź���
SemaphoreHandle_t esp8266_Semaphore;                                    //ESP8266�ź���

//��ʱ��
 TimerHandle_t MQTT_HeartTimer =NULL;

//MQTT�����ϱ�������
char mqtt_message[300];

int main()
{
	BaseType_t xReturn =pdPASS;
    HAL_Init();
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
    SystemClock_Config();
    LED_Init();
    init_uart2();
    uart_init(115200);
    // ESP8266_MQTT_Init();    
//    init_uart2();
	xReturn = xTaskCreate((TaskFunction_t )appmain,						//������ں���
						  (const char *)"appmain",						//��������
						  (const uint16_t)512,							//����ջ��С
						  (void *)NULL,									//������ں�������
						  (UBaseType_t) 1,								//�������ȼ�
						  (TaskHandle_t *)&AppMain_Handle);				//������
	if(pdPASS ==xReturn)
		vTaskStartScheduler();
	else
		return -1;
    while(1)
    {
        
    }    
}

static void appmain(void *parameter)
{
    BaseType_t xReturn =pdPASS;
    taskENTER_CRITICAL();                                               //�����ٽ���
    AHT10_Init();
    LCD_Init();
    LCD_ShowString(0, 100, 240, 16, 16,"temp: ");
    LCD_ShowString(100, 100, 140, 16, 16, "55");
    esp8266_Semaphore =xSemaphoreCreateBinary();                        //�����ź���
    if(esp8266_Semaphore ==NULL)
    {
        printf("Semaphore error\r\n");
    }
    xReturn = xTaskCreate((TaskHandle_t )IOT_Esp8266,
                          (const char *)"ESP8266",
                          (const uint16_t)512*5,
                          (void *)NULL,
                          (UBaseType_t)4,
                          (TaskHandle_t *)&ESP8266_Handle);

    if(pdPASS !=xReturn)
    {
        return ;
    }
    xReturn = xTaskCreate((TaskHandle_t )IOT_Contral,
                          (const char *)"Contral",
                          (const uint16_t)512,
                          (void *)NULL,
                          (UBaseType_t)3,
                          (TaskHandle_t *)&IOTContral_Handle);
    if(pdPASS !=xReturn)
    {
        return ;
    }
    vTaskDelete(AppMain_Handle);
    taskEXIT_CRITICAL();
     
}

void IOT_Esp8266(void *parameter)
{
    // uint8_t cnt=1;
    float temp,hum;
    USART2_RX_STA=0;
    while(1)
    {
//        if(tcnt %10 ==0)
//        {
            delay_ms(1000);
            temp =AHT10_Read_Temperature();
            hum =AHT10_Read_Humidity();
            sprintf(mqtt_message,"{\"method\":\"thing.service.property.set\",\"id\":\"303155086\",\"params\":{\"temperature\":%.1f,\"humidity\":%.1f},\"version\":\"1.0.0\"}",temp,hum);
            // MQTT_PublishData_Pack(MQTT_PUBLISH_TOPIC,mqtt_message,0);
            printf(" temp =%.1f,  hum=%.1f\r\n",temp,hum);
            printf("USART2_RX_STA=%d\r\n",USART2_RX_STA);
            // if(USART2_RX_STA ==1)
            // {
            //     for(uint16_t i=0;i<USART2_RX_LEN;i++)
            //     {
            //         printf("0x%x   ",USART2_RX_BUF[i]);
            //     }
            //     printf("\r\n");
            //     USART2_RX_LEN =0;
            //     USART2_RX_STA =0;
            // }
            // 
//            delay_ms(5500);
//        }
//        tcnt++;

    }

}

void IOT_Contral(void *parameter)
{
    static u8 cnt =0;
    while(1)
    {
        LED_R_ON;
        delay_ms(1000);
        LED_R_OFF;
        printf("IOT LED IS RUNNING\r\n");
        cnt ++;
        if(cnt ==5)
        {
            xSemaphoreGive(esp8266_Semaphore);
            printf("is give esp8266 semphor\r\n");
            cnt =0;
        }
        vTaskDelay(1000);   
    }

}

/******************************  �������ģʽ����  *****************************/
//�������ģʽ�ȴ��ֶ�����
void Enter_ErrorMode(uint8_t mode)
{
    LED_R_ON;
    while(1)
    {
        switch (mode)
        {
        case 0: printf("ESP8266��ʼ��ʧ��!\r\n");
            break;
        case 1: printf("ESP8266�����ȵ�ʧ��!\r\n");
            break;
        case 2: printf("ESP8266���Ӱ����Ʒ�����ʧ��!\r\n");
            break;
        case 3: printf("ESP8266������MQTT��¼ʧ��\r\n");
            break;
        case 4: printf("ESP8266������MQTT��������ʧ��!\r\n");
            break;
        
        default: printf("Nothing\r\n");
            break;
        }
        printf("������������\r\n");
        delay_ms(1000);
    }
}

/******************************  STM32 MQTTҵ�����  *****************************/
//MQTT��ʼ������
void ESP8266_MQTT_Init(void)
{
    uint8_t status=0;
    if(ESP8266_Init())
    {
        printf("ESP8266��ʼ���ɹ�!\r\n");
        status++;
    }else{
        Enter_ErrorMode(0);
    }
    //�����ȵ�
    if(status==1)
    {
        if(ESP8266_ConnectAP(WIFISTA_SSID,WIFISTA_PASSWORD))
        {
            printf("ESP8266�����ȵ�ɹ�!\r\n");
            status++;
        }
        else{
            Enter_ErrorMode(1);
        }
    }
    //���Ӱ�����
    if(status==2)
    {
        if(ESP8266_ConnectServer("TCP",MQTT_BROKERADDRESS,1883)!=0)
        {
            printf("ESP8266���Ӱ����Ʒ������ɹ�!\r\n");
            status++;
        }else{
            Enter_ErrorMode(2);
        }
    }
    //��¼MQTT
    if(status==3)
    {
        if(MQTT_Connect_Pack(MQTT_CLIENTID,MQTT_USARNAME,MQTT_PASSWORD) !=0)
        {
            printf("ESP8266������MQTT��¼�ɹ�!\r\n");
            MQTT_CreateHeartTimer();
            status++;
        }else{
            Enter_ErrorMode(3);
        }
    }
    if(status==4)
    {
        if(MQTT_SubsrcibeTopic(MQTT_SUBSCRIBE_TOPIC,0,1)!=0)
        {
            printf("ESP8266���ĳɹ�");
        }else{
            Enter_ErrorMode(4);
        }
    }
}

//�Ƴ�ʼ������
void LED_Init(void)
{
    GPIO_InitTypeDef        GPIO_InitStructure;
    
    __HAL_RCC_GPIOE_CLK_ENABLE();
    
    GPIO_InitStructure.Pin=GPIO_PIN_7;
    GPIO_InitStructure.Mode=GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Speed=GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStructure.Pull=GPIO_PULLUP;
    HAL_GPIO_Init(GPIOE,&GPIO_InitStructure);
    
    HAL_GPIO_WritePin(GPIOE,GPIO_PIN_7,GPIO_PIN_SET);
}

void MQTT_HeartTimer_Handler(TimerHandle_t xTimer)
{
    MQTT_SendHeart();
    printf("MQTT Heart timer running\r\n");
}

//����������ʱ��
void MQTT_CreateHeartTimer(void)
{
    if(MQTT_HeartTimer ==NULL)
    {
        MQTT_HeartTimer = xTimerCreate((const char*)"MQTT_Heart",1000*60,pdTRUE,(void *)1,MQTT_HeartTimer_Handler);
    }
    if(MQTT_HeartTimer!=NULL)
    {
        xTimerStart(MQTT_HeartTimer,0);
    }
    
}