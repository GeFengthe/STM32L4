#include "timer.h"
#include "esp8266.h"
TIM_HandleTypeDef TIM7_Handler;

void init_TIM7(uint16_t arr,uint16_t psc)
{
    TIM7_Handler.Instance = TIM7;
    TIM7_Handler.Init.Prescaler =psc;                   //分频系数
    TIM7_Handler.Init.CounterMode = TIM_COUNTERMODE_UP; //向上计数器
    TIM7_Handler.Init.Period =arr;                      //自动装载值
    TIM7_Handler.Init.ClockDivision =TIM_CLOCKDIVISION_DIV1;    //时钟分频因子
    HAL_TIM_Base_Init(&TIM7_Handler);
    __HAL_TIM_CLEAR_FLAG(&TIM7_Handler,TIM_EventSource_Update);
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
    USART2_RX_STA |=1<<15;                               //标记接收完成
    printf("TIM7 IS HANDLER\r\n");
    __HAL_TIM_CLEAR_FLAG(&TIM7_Handler,TIM_EventSource_Update);     //清除TIM7更新中断标志
    TIM7->CR1 &=~(1<<0);                                 //关闭定时器7
}