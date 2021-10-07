#include "timer.h"
#include "esp8266.h"
TIM_HandleTypeDef TIM7_Handler;

unsigned int tim_buff[TIM_MAX];

unsigned char Tim7_flag=0;

//10ms ��ʱ��  ʱ��=((1+Pre)/80M)*(cnt+1)
void Tim7_init(void)
{
    TIM7_Handler.Instance = TIM7;
    TIM7_Handler.Init.Prescaler =799;                   //��Ƶϵ��
    TIM7_Handler.Init.CounterMode = TIM_COUNTERMODE_UP; //���ϼ�����
    TIM7_Handler.Init.Period =999;                      //�Զ�װ��ֵ
    TIM7_Handler.Init.ClockDivision =TIM_CLOCKDIVISION_DIV1;    //ʱ�ӷ�Ƶ����
    HAL_TIM_Base_Init(&TIM7_Handler);
    __HAL_TIM_CLEAR_FLAG(&TIM7_Handler,TIM_IT_UPDATE);
    HAL_TIM_Base_Start_IT(&TIM7_Handler);                       //ʹ�ܶ�ʱ��3�Ͷ�ʱ���жϣ�TIM_IT_UPDATE
}
//��ʱ���ײ�����������ʱ�ӣ������ж����ȼ�
//�˺����ᱻHAL_TIM_Base_Init()��������
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if(htim->Instance ==TIM7)
    {
        __HAL_RCC_TIM7_CLK_ENABLE();
        HAL_NVIC_SetPriority(TIM7_IRQn,3,0);            //�����ж����ȼ�����ռ���ȼ�0�������ȼ�1
        HAL_NVIC_EnableIRQ(TIM7_IRQn);                  //����TIM7�ж�
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
            uart2_sta.uart_rx_sta |=0x8000;                                         //���λ��һ����ʾ�������
        }
        if(tim_buff[AT_TIM_ID] >1)  tim_buff[AT_TIM_ID]--;
        else if(tim_buff[AT_TIM_ID] ==1)
        {
            tim_buff[AT_TIM_ID] =0;
            Tim7_flag |=AT_FLAG;                                                    //ATָ�ʱ
        }
        
    }
}

