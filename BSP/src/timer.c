#include "timer.h"
#include "esp8266.h"
TIM_HandleTypeDef TIM7_Handler;

unsigned int tim_buff[TIM_MAX];

unsigned char Tim7_flag=0;

//10ms 定时器  时间=((1+Pre)/80M)*(cnt+1)
void Tim7_init(void)
{
    TIM7_Handler.Instance = TIM7;
    TIM7_Handler.Init.Prescaler =799;                   //分频系数
    TIM7_Handler.Init.CounterMode = TIM_COUNTERMODE_UP; //向上计数器
    TIM7_Handler.Init.Period =999;                      //自动装载值
    TIM7_Handler.Init.ClockDivision =TIM_CLOCKDIVISION_DIV1;    //时钟分频因子
    HAL_TIM_Base_Init(&TIM7_Handler);
    __HAL_TIM_CLEAR_FLAG(&TIM7_Handler,TIM_IT_UPDATE);
    HAL_TIM_Base_Start_IT(&TIM7_Handler);                       //使能定时器3和定时器中断：TIM_IT_UPDATE
}
//定时器底册驱动，开启时钟，设置中断优先级
//此函数会被HAL_TIM_Base_Init()函数调用
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if(htim->Instance ==TIM7)
    {
        __HAL_RCC_TIM7_CLK_ENABLE();
        HAL_NVIC_SetPriority(TIM7_IRQn,3,0);            //设置中断优先级，抢占优先级0，子优先级1
        HAL_NVIC_EnableIRQ(TIM7_IRQn);                  //开启TIM7中断
    }
}

void TIM7_IRQHandler(void)
{
    if(__HAL_TIM_GET_IT_SOURCE(&TIM7_Handler,TIM_IT_UPDATE) !=RESET)
    {
        __HAL_TIM_CLEAR_FLAG(&TIM7_Handler,TIM_IT_UPDATE);
        if(tim_buff[UART2_TIM_ID] >1) tim_buff[UART2_TIM_ID]--;
        else if(tim_buff[UART2_TIM_ID] == 1)
        {
            tim_buff[UART2_TIM_ID] =0;
            uart2_sta.uart_rx_sta |=0x8000;                                         //最高位置一，表示接收完成
        }
        if(tim_buff[AT_TIM_ID] >1)  tim_buff[AT_TIM_ID]--;
        else if(tim_buff[AT_TIM_ID] ==1)
        {
            tim_buff[AT_TIM_ID] =0;
            Tim7_flag |=AT_FLAG;                                                    //AT指令超时
        }
        
    }
}

