#include "esp8266.h"
/*
WIFI :HUAWEI-2303
密码: zyj15251884308

*/

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

//串口1发送一个字节
static void USART2_SendOneByte(uint8_t byte)
{
    ((UART_HandleTypeDef *)&UART2_Hander)->Instance->TDR =((uint16_t)byte &(uint16_t)0x01FF);
    while((((UART_HandleTypeDef *)&UART2_Hander)->Instance->ISR&0x40)==0);                          //等待发送完成
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
    }
    __HAL_UART_CLEAR_IT(&UART2_Hander,UART_IT_RXNE);
}

//向ESP8266发送定长数据
void ESP8266_ATSendBuf(uint8_t *buf,uint16_t len)
{
    memset(USART2_RX_BUF,0,1024);
    USART2_RX_STA =0;                   //接收标志置0
    //定长发送
    HAL_UART_Transmit(&UART2_Hander,buf,len,0xFFFF);

}
//向ESP8266发送字符串
void ESP8266_ATSendString(char * str)
{
    memset( USART2_RX_BUF,0,1024);
    USART2_RX_STA =0;
    while(*str)     USART2_SendOneByte(*str++);
}

//退出透传
void ESP8266_ExitUnvarnishedTrans(void)
{
    ESP8266_ATSendString("+++");
    delay_ms(50);
    ESP8266_ATSendString("+++");
    delay_ms(50);
}

//查找字符串中是否包含另一个字符串
uint8_t FindStr(char *dest,char *src,uint16_t retry_nms)
{
    retry_nms/=10;
    while(strstr(dest,src)==0 && retry_nms--)
    {
        delay_ms(10);
    }
    if(retry_nms)
    {
        return 1;
    }
    return 0;
}
/**
 * 功能：检查ESP8266是否正常
 * 参数：None
 * 返回值：ESP8266返回状态
 * 非0 ESP8266正常
*/

uint8_t ESP8266_Check(void)
{
    uint8_t check_cnt=5;
    while(check_cnt--)
    {
        memset(USART2_RX_BUF,0,1024);
        ESP8266_ATSendString("AT\r\n");
        if(FindStr((char *)USART2_RX_BUF,"OK",200)!=0)
        {
            return 1;
        }
    }
    return 0;
}


/**
 * 功能：初始化ESP8266
 * 参数：NOne
 * 返回值：初始化结果，非0初始化成功，0为失败
 * 
 * 
 * 
*/
uint8_t ESP8266_Init(void)
{
    memset(USART2_TX_BUF,0,1024);
    memset(USART2_RX_BUF,0,1024);

    ESP8266_ExitUnvarnishedTrans();                         //退出透传
    delay_ms(500);
    ESP8266_ATSendString("AT+RST\r\n");
    delay_ms(800);
    if(ESP8266_Check()==0)                                  //使用AT指令查询ESP8266是否存在
    {
        return 0;
    }
    memset(USART2_RX_BUF,0,1024);                           //清空缓冲区
    ESP8266_ATSendString("ATE0\r\n");                           //关闭回显
    if(FindStr((char *)USART2_RX_BUF,"OK",500)==0)      
    {
        return 0;                                           //设置不成功
    }
    return 1;                                               //设置成功

}

/**
 * 功能：恢复出厂设置
 * 参数：None
 * 返回值：None
 * 
 * 
*/
void ESP8266_Restore(void)
{
    ESP8266_ExitUnvarnishedTrans();                                 //退出透传
    delay_ms(500);
    ESP8266_ATSendString("AT+RESTORE\r\n");                         //恢复出厂
}

/**
 * 功能：连接热点
 * 参数:
 *          ssid:热点名
 *          pwd:热点密码
 * 返回值:连接结果，非0连接成功，0连接失败
 * 
 * 
*/
uint8_t ESP8266_ConnectAP(char *ssid,char *pswd)
{
    uint8_t cnt =5;
    while(cnt--)
    {
        memset(USART2_RX_BUF,0,1024);
        ESP8266_ATSendString("AT+CWMODE_CUR=1\r\n");                            //设置为station模式 （——cur是代表不保存到flash)
        if(FindStr((char*)USART2_RX_BUF,"OK",200)!=0)
        {
            break;
        }
    }
    if(cnt==0)
    {
        return 0;
    }
    cnt =2;
    while(cnt--)
    {
        memset(USART2_TX_BUF,0,1024);
        memset(USART2_RX_BUF,0,1024);
        sprintf((char *)USART2_RX_BUF,"AT+CWJAP_CUR=\"%s\",\"%s\"\r\n",ssid,pswd);                  //连接目标AP
        ESP8266_ATSendString((char *)USART2_RX_BUF);
        if(FindStr((char *)USART2_RX_BUF,"OK",8000)!=0)                                             //连接成功且分配IP
        {
            return 1;
        }
    }
    return 0;
}
/**
 * 开启透传模式
 * 
 **/
 static uint8_t ESP8266_OpenTransmission(void)
 {
     //设置透传模式
     uint8_t cnt=2;
     while(cnt--)
     {
         memset(USART2_RX_BUF,0,1024);
         ESP8266_ATSendString("AT+CIPMODE=1\r\n");
         if(FindStr((char*)USART2_RX_BUF,"OK",200)!=0)
         {
             return 1;
         }
         
     }
     return 0;
 } 
/**
 * 功能：使用指定协议(TCP/UDP)连接到服务器
 * 参数：
 *         mode:协议类型 "TCP","UDP"
 *         ip:目标服务器IP
 *         port:目标是服务器端口号
 * 返回值：
 *         连接结果,非0连接成功,0连接失败
 * 说明： 
 *         失败的原因有以下几种(UART通信和ESP8266正常情况下)
 *         1. 远程服务器IP和端口号有误
 *         2. 未连接AP
 *         3. 服务器端禁止添加(一般不会发生)
 */
uint8_t ESP8266_ConnectServer(char *mode,char* ip,uint16_t port)
{
    uint8_t cnt;
    ESP8266_ExitUnvarnishedTrans();                                 //多次连接需退出透传
    delay_ms(500);
    cnt =2;
    while(cnt--)
    {
        memset((char*)USART2_TX_BUF,0,1024);
        memset((char*)USART2_RX_BUF,0,1024);
        sprintf((char*)USART2_TX_BUF,"AT+CIPSTART=\"%s\",\"%s\",%d\r\n",mode,ip,port);
        ESP8266_ATSendString((char*)USART2_TX_BUF);
        if(FindStr((char*)USART2_RX_BUF,"CONNECT",4000)!=0 )
        {
            break;
        }
    }
    if(cnt ==0)
    {
        return 0;
    }
    //设置透传模式
    if(ESP8266_OpenTransmission()==0)   return 0;

    //开启发送状态
    cnt=2;
    while(cnt--)
    {
        memset(USART2_RX_BUF,0,1024);
        ESP8266_ATSendString("AT+CIPSEND\r\n");
        if(FindStr((char*)USART2_RX_BUF,">",200)!=0)
        {
            return -1;
        }
    }
    return 0;
}
/**
 * 功能：主动和服务器断开连接
 * 参数：None
 * 返回值：
 *         连接结果,非0断开成功,0断开失败
 */
uint8_t DisconnectServer(void)
{
    uint8_t cnt=2;
    ESP8266_ExitUnvarnishedTrans();                         //退出透传
    delay_ms(500);
    while(cnt--)
    {
        memset(USART2_RX_BUF,0,1024);                       //清空缓冲区
        ESP8266_ATSendString("AT+CIPCLOSE\r\n");
        if(FindStr((char*)USART2_RX_BUF,"CLOSED",200)!=0)
        {
            break;
        }
    }
    if(cnt) return 1;
    return 0;
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