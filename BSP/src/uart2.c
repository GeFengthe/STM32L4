#include "uart2.h"
#include "delay.h"

static UART_HandleTypeDef         UART2_Hander;

__uart_sta uart2_sta;

void esp8266_init(void)
{
    GPIO_InitTypeDef    GPIO_InitStruct;
    __HAL_RCC_GPIOE_CLK_ENABLE();
    GPIO_InitStruct.Pin =ESP8266_RST_Pin;
    GPIO_InitStruct.Pull =GPIO_PULLUP;
    GPIO_InitStruct.Speed =GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Mode =GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(ESP8266_Port, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin =ESP8266_LED_Pin;
    HAL_GPIO_Init(ESP8266_Port, &GPIO_InitStruct);
    
    HAL_GPIO_WritePin(ESP8266_Port, ESP8266_LED_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ESP8266_Port, ESP8266_RST_Pin, GPIO_PIN_RESET);
    delay_ms(500);
    HAL_GPIO_WritePin(ESP8266_Port, ESP8266_RST_Pin, GPIO_PIN_SET);
    delay_ms(3000);
}

void uart2_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct;
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART2_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = UART2_Tx_Pin;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(UART2_Port,&GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = UART2_Rx_Pin;
    HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);
    
    
    UART2_Hander.Instance =USART2;
    UART2_Hander.Init.BaudRate = 115200;
    UART2_Hander.Init.Parity =UART_PARITY_NONE;
    UART2_Hander.Init.WordLength = UART_WORDLENGTH_8B;
    UART2_Hander.Init.StopBits = UART_STOPBITS_1;
    UART2_Hander.Init.Mode = UART_MODE_TX_RX;
    UART2_Hander.Init.HwFlowCtl = UART_HWCONTROL_NONE;                      //无硬件控制               
    HAL_UART_Init(&UART2_Hander);
    
    //__HAL_UART_CLEAR_FLAG(&UART2_Hander,UART_FLAG_RXNE);                    //清除中断
    __HAL_UART_ENABLE_IT(&UART2_Hander,USART_IT_RXNE);                      //开启接收中断
    HAL_NVIC_EnableIRQ(USART2_IRQn);
    HAL_NVIC_SetPriority(USART2_IRQn,2,0);                                  //设置中断优先级
}

//发送一个字节数据
void UART2SendByte(unsigned char SendData)
{
    (&UART2_Hander)->Instance->TDR = ((uint16_t)SendData &(uint16_t)0x1FF);
    while(((&UART2_Hander)->Instance->ISR &0x40) ==0);                                      //等待发送完成
}

void UART2_SendStr(unsigned char *p)
{
    while(*p !='\0')
    {
        UART2SendByte(*p);
        p++;
    }
}
//按长度发送
void UART2_SendStr_Len(unsigned char *p, uint16_t len)
{
    uint16_t i;
    for(i =0;i<len;i++)
    {
        UART2SendByte(*p);
        p++;
    }
}

void USART2_IRQHandler(void)
{
    uint8_t Res;
    if(__HAL_UART_GET_IT_SOURCE(&UART2_Hander,UART_IT_RXNE) != RESET)
    {
        //HAL_UART_Receive(&UART2_Hander, &Res, 1, 1000);
        Res = USART2->RDR;                                                      //接受数据
        if(uart2_sta.uart_rx_sta < (UART2_REC_LEN-1))
        {
            STAR_UART2_TIMER(3);
            uart2_sta.uart_rx_buf[uart2_sta.uart_rx_sta++] =Res;
        }else{
            uart2_sta.uart_rx_sta |=0x8000;
            STOP_UART2_TIMER;
        }    
    }
}
