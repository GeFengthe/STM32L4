#include "stm32l4xx.h"
#include "sys.h"
#include "delay.h"
#include "stm32l4xx_hal_gpio.h"
#include "lcd.h"
#include "usart.h"
#include "esp8266.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"             //ʹ���ź���Ҫ������ͷ�ļ�
#include "task.h"
#include "mqtt.h"
#include "aht10.h"
#include "timers.h"
#include "esp8266_at.h"
#include "uart2.h"


#define LED_R_Pin               GPIO_PIN_7
#define LED_G_Pin               GPIO_PIN_8
#define LED_B_Pin               GPIO_PIN_9

#define LED_Port                GPIOE


#define LED_R(x)                (x?HAL_GPIO_WritePin(LED_Port,LED_R_Pin,GPIO_PIN_SET):HAL_GPIO_WritePin(LED_Port,LED_R_Pin,GPIO_PIN_RESET))

#define LED_G(x)                (x?HAL_GPIO_WritePin(LED_Port,LED_G_Pin,GPIO_PIN_SET):HAL_GPIO_WritePin(LED_Port,LED_G_Pin,GPIO_PIN_RESET))
            
#define LED_B(x)                (x?HAL_GPIO_WritePin(LED_Port,LED_B_Pin,GPIO_PIN_SET):HAL_GPIO_WritePin(LED_Port,LED_B_Pin,GPIO_PIN_RESET))

#define OFF                     1
#define ON                      0



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
static void thread_init(void *parameter);
static void IOT_Esp8266(void *parameter);
static void IOT_Contral(void *parameter);
static void MQTT_CreateHeartTimer(void);


//MQTTҵ���ʼ��
void ESP8266_MQTT_Init(void);



//������
static TaskHandle_t Thread_init_Handle=	NULL;


//��ʱ��
 TimerHandle_t MQTT_HeartTimer =NULL;

//MQTT�����ϱ�������
char mqtt_message[300];

void Sky_app_thread(void);
void Led_init(void);


int main()
{
	BaseType_t xReturn =pdPASS;
    SystemClock_Config();
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
    HAL_Init();
	xReturn = xTaskCreate((TaskFunction_t )thread_init,						//������ں���
						  (const char *)"thread init",						//��������
						  (const uint16_t)512,							//����ջ��С
						  (void *)NULL,									//������ں�������
						  (UBaseType_t) 1,								//�������ȼ�
						  (TaskHandle_t *)&Thread_init_Handle);				//������
	if(pdPASS ==xReturn)
		vTaskStartScheduler();
	else
		return -1;
   
}

static void board_init(void)
{
    Led_init();
    uart_init(115200);
    Tim7_init();
    esp8266_init();
    uart2_init();
}
//���̴߳�������
static void Sky_thread_init(void)
{
    BaseType_t xReturn =pdPASS;
    xReturn = xTaskCreate( (TaskFunction_t ) Sky_app_thread, "sky app", 1000, NULL, 3, NULL);
    if(pdPASS != xReturn)
    {
        printf( "app create error\r\n");
    }
}


//��ʼ��
static void thread_init(void *parameter)
{
    board_init();                                   //�����ʼ��
    Sky_thread_init();
//    vTaskStartScheduler();
    vTaskDelete(Thread_init_Handle);  
}

void Led_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    __HAL_RCC_GPIOE_CLK_ENABLE();
    GPIO_InitStruct.Pin = LED_R_Pin;
    GPIO_InitStruct.Pull =GPIO_PULLUP;
    GPIO_InitStruct.Speed =GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Mode =GPIO_MODE_OUTPUT_PP;
    
    HAL_GPIO_Init(LED_Port, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = LED_G_Pin;
    HAL_GPIO_Init(LED_Port, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = LED_B_Pin; 
    HAL_GPIO_Init(LED_Port, &GPIO_InitStruct);
    LED_R(OFF);
    LED_G(OFF);
    LED_B(OFF);
}


void Sky_app_thread(void)
{
    uint8_t led_flag =0;
    AT_CheckSend();
    while(1)
    {
        if(uart2_sta.uart_rx_sta &0X8000)       //���յ�����
        {
            uart2_sta.uart_rx_buf[uart2_sta.uart_rx_sta&0x7FFF] = '\0';
            printf("%s",uart2_sta.uart_rx_buf);
            AT_Mode_Correct_Check(uart2_sta.uart_rx_buf);
            uart2_sta.uart_rx_sta =0;
        }
        if(led_flag)
        {
            led_flag =0;
            LED_B(ON);
        }else
        {
            led_flag =1;
            LED_B(OFF);
        }
        vTaskDelay(200);
    }
}



