#include "esp8266.h"
/*
WIFI :HUAWEI-2303
密码: zyj15251884308

*/
#define WIFISTA_SSID                    "HUAWEI-2303"               //wifi名称
#define WIFISTA_PASSWORD                "zyj15251884308"            //连接密码
#define WIFISTA_PORTNUM                 "8080"                      //连接端口号
#define WIFISTA_ENCRYPTION              "wpawpa2_aes"               //wpa/wpa2 aes 加密方式




uint16_t USART2_RX_STA =0;              //接收结束标志
static uint8_t USART2_TX_BUF[1024];     //发送BUFF
uint8_t USART2_RX_BUF[1024];

const char sta[]="AT+CWMODE=1";         //STA 模式
const char ap[] ="AT+CWMODE=2";         //AP 模式
const char sta_ap[] ="AT+CWMODE=3";     //STA_AP模式

//ESP8266结构体
// typedef struct esp8266_wifi{
//     uint8_t init_flag;
//     char *ssid;
//     char *password;
//     char *mode;

// }esp8266;


//typedef struct wifi{
//    const u8 * wifista_ssid,
//    const u8 * wifista_password,
//    const u8 * wifista_portnum,
//}wifi_sta;
    
UART_HandleTypeDef         UART2_Hander;


void init_uart2(void)
{
    GPIO_InitTypeDef    GPIO_InitStruct;
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART2_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = UART2_TX_PIN;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = UART2_RX_PIN;
    HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);
    
    UART2_Hander.Instance =USART2;
    UART2_Hander.Init.BaudRate = 115200;
    UART2_Hander.Init.Parity =UART_PARITY_NONE;
    UART2_Hander.Init.WordLength = UART_WORDLENGTH_8B;
    UART2_Hander.Init.StopBits = UART_STOPBITS_1;
    UART2_Hander.Init.Mode = UART_MODE_TX_RX;
    UART2_Hander.Init.HwFlowCtl = UART_HWCONTROL_NONE;                      //无硬件控制               
    HAL_UART_Init(&UART2_Hander);
    
    __HAL_UART_CLEAR_FLAG(&UART2_Hander,UART_FLAG_RXNE);
    __HAL_UART_ENABLE_IT(&UART2_Hander,USART_IT_RXNE);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
    HAL_NVIC_SetPriority(USART2_IRQn,2,0);
    init_TIM7(1000,7999);                                       //100ms 中断
    USART2_RX_STA =0;
    TIM7->CR1 &=~(1<<0);                                        //关闭定时器7
}


//串口3,printf 函数
//确保一次发送数据不超过USART3_MAX_SEND_LEN字节
void u2_printf(char* fmt,...)  
{  
	uint16_t i,j; 
	va_list ap; 
	va_start(ap,fmt);
	vsprintf((char*)USART2_TX_BUF,fmt,ap);
	va_end(ap);
	i=strlen((const char*)USART2_TX_BUF);		//此次发送数据的长度
	for(j=0;j<i;j++)							//循环发送数据
	{
		while((USART2->ISR&0x40)==0);			//循环发送,直到发送完毕   
		USART2->TDR=USART2_TX_BUF[j];  
	} 
}

//向ESP8266发送定长数据
void ESP8266_ATSendBuf(uint8_t *buf,uint16_t len)
{
    memset(USART2_RX_BUF,0,1024);
    USART2_RX_STA =0;                   //接收标志置0
    //定长发送
    HAL_UART_Transmit(&UART2_Hander,buf,len,0xFFFF);

}

void USART2_IRQHandler(void)
{
    uint8_t res;
    if(__HAL_UART_GET_IT(&UART2_Hander,UART_IT_RXNE) !=RESET)       //接收到数据
    {
        res =USART2->RDR;
        if((USART2_RX_STA &(1<<15))==0)
        {
            if(USART2_RX_STA <1024)
            {
                TIM7->CNT =0;                                           //计数器清空
                if(USART2_RX_STA==0)
                {
                    TIM7->CR1 |=1<<0;                                   //使能定时器7
                }
                USART2_RX_BUF[USART2_RX_STA++]=res;
            }else
            {
                USART2_RX_STA|=1<<15;
            }
        }
        __HAL_UART_CLEAR_IT(&UART2_Hander,UART_IT_RXNE);
    }
}

//ATK-ESP8266发送命令后,检测接收到的应答
//str:期待的应答结果
//返回值:0,没有得到期待的应答结果
//其他,期待应答结果的位置(str的位置)
uint8_t * atk_8266_check_cmd(uint8_t *src)
{
    char *strx =0;
    if(USART2_RX_STA &0x8000)
    {
        USART2_RX_BUF[USART2_RX_STA &0x7FFF] =0;            //添加字符串的结束符
        strx =strstr((const char *)USART2_RX_BUF,(const char *)src);       //查找应答
    }
    return (uint8_t *)strx;
}

/*
CMD :指令
ack：应答 无应答即是NULL
waittime：超时时长 单位10ms
*/

uint8_t atk_8266_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
    u8 res =0;
    USART2_RX_STA = 0;
    u2_printf("%s\r\n",cmd);                                //发送命令
    if(ack && waittime)
    {
        while(--waittime)
        {
            delay_ms(10);
            if(USART2_RX_STA &0x8000)                       //接收到期待的应答结果
            {
                if(atk_8266_check_cmd((u8 *)ack))
                {
                    printf("ack:%s\r\n",(u8 *)ack);
                    break;
                }
                printf("ack is no wait----\r\n");
                USART2_RX_STA =0;
            }
        }
        if(waittime ==0)
            res =1;
    }
    return res;
}


/*
ESP8266模式选择函数
mode :模式选择 AP STA   AP+STA
*/
u8 esp_8266_mode(char *mode)
{
    u8 ret =0;
    u8 *p;
    p =(u8 *)mode;
    if(atk_8266_send_cmd(p,(u8 *)"OK",60))
    {
        ret =1;
    }
    if(atk_8266_send_cmd((u8*)"AT+RST",(u8*)"OK",60))               //重启生效
    {
        ret =1;
    }
    return ret;
}

//esp8266 连接wifi函数
/*ssid :wifi名字
pass_word:密码
*/
u8 esp_8266_connect_wifi(char *ssid,char *pass_word)
{
    u8 ret =0;
    //连接wifi
    u8 p[50];
    sprintf((char *)p,"AT+CWJAP=\"%s\",\"%s\"",ssid,pass_word);
    printf("cmd=%s\r\n",p);
    if(atk_8266_send_cmd(p,(u8*)"OK",800))
    {
        ret =1;
    }
    //关闭回显
    if(atk_8266_send_cmd((u8 *)"ATE0",(u8 *)"OK",60))
    {
        ret =1;
    }
    return ret;
    
}
/*
*连接服务器
*tpye：TCP或者UDP
*addr：ip地址
*port: 端口号
*/
u8 esp_8266_connect_server(char *type,char *addr,u16 port)
{
    u8 ret =0;
    u8 p[500];
    memset(USART2_RX_BUF,0,1024);
    sprintf((char *)p,"AT+CIPSTART=\"%s\",\"%s\",%d",type,addr,port);
	if(atk_8266_send_cmd(p,(u8*)"OK",80))
	{
		ret =1;
	}
	return ret;
}
//打开透传模式
u8 esp_8266_passthrough(void)
{
	u8 ret=0;
	u8 p[50];
	//设置为透传模式
	if(atk_8266_send_cmd((u8 *)"AT+CIPMODE=1",(u8 *)"OK",50))
	{
		ret =1;
	}
	if(atk_8266_send_cmd((u8 *)"AT+CIPSEND",(u8 *)">",50))
	{
		ret =1;
	}
	return ret;
}

void ESP8266_Init(void)
{

}


////连接wifi
//u8 esp_8266_sta_con(char *ssid,char *pass_word)
//{
//    u8 ret =0;
//    u8 *p;
//    if(atk_8266_send_cmd((u8*)"AT+CWMODE=1",(u8*)"OK",100))
//    {
//        printf("AT+MODE=ERROR\r\n");
//        return ret;
//    }
//    delay_ms(1000);
//    p=(u8 *)"AT+CWJAP=\"HUAWEI-2303\",\"zyj15251884308\"";
////    sprintf((char *)p,"AT+CWJAP=\"%s\",\"%s\"",ssid,pass_word);
//    ret=atk_8266_send_cmd(p,(u8*)"OK",300);
//    return ret;
//}

//u8 esp_8266_test_client(void)
//{
//    u8 ret =0;
//    u8 *p;
////    sprintf((char *)p,"AT+CIPSTART=\"TCP\",\"192.168.3.39\",8080");
//    p=(u8*)"AT+CIPSTART=\"TCP\",\"192.168.3.39\",8080";
//    atk_8266_send_cmd(p,(u8*)"OK",300);
//    delay_ms(2000);
//    atk_8266_send_cmd((u8*)"AT+CIPSEND=10",(u8*)"OK",300);
//    delay_ms(500);
//    while(1)
//    {
//        u2_printf("%s\r\n","1234567890");
//        delay_ms(2000);
//        atk_8266_send_cmd((u8*)"AT+CIPSEND=10",(u8*)"OK",300);
//        delay_ms(2000);
//    }
//}