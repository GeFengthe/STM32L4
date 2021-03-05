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
#include "semphr.h"                                                 //使用信号量要包含的头文件

#define LED_R_Pin               GPIO_PIN_7
#define LED_G_Pin               GPIO_PIN_8
#define LED_B_Pin               GPIO_PIN_9

#define LED_Port                GPIOE

#define LED_R_ON                HAL_GPIO_WritePin(LED_Port,LED_R_Pin,GPIO_PIN_RESET)
#define LED_R_OFF               HAL_GPIO_WritePin(LED_Port,LED_R_Pin,GPIO_PIN_SET)

void LED_Init(void);
//任务入口函数
static void appmain(void *parameter);
static void IOT_Esp8266(void *parameter);
static void IOT_Contral(void *parameter);





//任务句柄
static TaskHandle_t AppMain_Handle=	NULL;
static TaskHandle_t ESP8266_Handle= NULL;
static TaskHandle_t IOTContral_Handle =NULL;

//信号量
SemaphoreHandle_t esp8266_Semaphore;                                    //ESP8266信号量

int main()
{
	BaseType_t xReturn =pdPASS;
//    HAL_Init();
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
    SystemClock_Config();
    LED_Init();
    init_uart2();
    uart_init(115200);
    
//    init_uart2();
	xReturn = xTaskCreate((TaskFunction_t )appmain,						//任务入口函数
						  (const char *)"appmain",						//任务名字
						  (const uint16_t)512,							//任务栈大小
						  (void *)NULL,									//任务入口函数参数
						  (UBaseType_t) 1,								//任务优先级
						  (TaskHandle_t *)&AppMain_Handle);				//任务句柄
	if(pdPASS ==xReturn)
		vTaskStartScheduler();
	else
		return -1;
    while(1)
    {
        
    }    
}

//灯初始化函数
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

static void appmain(void *parameter)
{
    BaseType_t xReturn =pdPASS;
    taskENTER_CRITICAL();                                               //进入临界区
    esp8266_Semaphore =xSemaphoreCreateBinary();                        //创建信号量
    if(esp8266_Semaphore ==NULL)
    {
        printf("Semaphore error\r\n");
    }
    xReturn = xTaskCreate((TaskHandle_t )IOT_Esp8266,
                          (const char *)"ESP8266",
                          (const uint16_t)512,
                          (void *)NULL,
                          (UBaseType_t)3,
                          (TaskHandle_t *)&ESP8266_Handle);

    if(pdPASS !=xReturn)
    {
        return ;
    }
    xReturn = xTaskCreate((TaskHandle_t )IOT_Contral,
                          (const char *)"Contral",
                          (const uint16_t)512,
                          (void *)NULL,
                          (UBaseType_t)2,
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
    uint8_t err;
    static uint8_t wifi_stu=1;                                              //ESP8266需要初始换连接服务器
    char *p=NULL;
    printf(" now in IOT_Esp8266\r\n");
    if(wifi_stu ==1)
    {
        err=atk_8266_send_cmd((uint8_t *)"AT",(uint8_t *)"OK",60);
        if(err ==1)
        {
            printf("ESP8266 AT test Error!\r\n");
            return;
        }
        p="AT+CWMODE=3";                                    //选择模式
        err=esp_8266_mode(p);
        if(err ==1)
        {
            printf("ESP8266 MODE Error");
            return ;
        }
        memset(p,0,strlen("AT+CWMODE=3"));
        err = esp_8266_connect_wifi("HUAWEI-2303","zyj15251884308");
        if(err ==1)
        {
            printf("ESP8266 connect wifi err\r\n");
            return ;
        }
        err =esp_8266_connect_server("TCP","192.168.3.40",8080);
        if(err ==1)
        {
            printf("ESP8266 connect server err\r\n");
        }
        err =esp_8266_passthrough();
        if(err ==1)
        {
            printf("ESP8266 passthrough mode erroe\r\n");
        }
        wifi_stu =0;

    }
    while(1)
    {
        if(xSemaphoreTake(esp8266_Semaphore,portMAX_DELAY)==pdTRUE)            //无限等待信号量
        {
            esp_8266_SendData((u8 *)"ESP8266",strlen("ESP8266"));
            printf("send data\r\n");
        }
        
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
