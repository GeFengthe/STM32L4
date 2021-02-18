#include "stm32l4xx.h"
#include "sys.h"
#include "delay.h"
#include "stm32l4xx_hal_gpio.h"
#include "lcd.h"
#include "usart.h"
#include "includes.h"
#include "esp8266.h"

//UCOSIII中以下优先级用户程序不能使用
//将这些优先级分配给了UCOSIII的5个系统内部任务
//优先级0：中断服务管理任务 OS_IntQTask()   默认关的
//优先级1: 时钟节拍任务OS_TickTask()
//优先级2: 定时任务OS_TmrTask()            默认为11
//优先级OS_CFG_PRIO_MAX-2: 统计任务    OS_StatTask()
//优先级OS_CFG_PRIO_MAX-1: 空闲任务    OS_IdleTask()
void LED_Init(void);


//任务优先级
#define APP_MAIN_TASK_PRIO              3

//任务堆栈大小
#define APP_MAIN_TASK_SIZE              512

//任务控制块
OS_TCB AppMainTaskTCB;

//任务堆栈
CPU_STK APP_MAIN_TASK_STK[APP_MAIN_TASK_SIZE];

//任务函数
void app_main(void *p_arg);

int main()
{
    OS_ERR err;
    CPU_SR_ALLOC();
    HAL_Init();
    SystemClock_Config();
    delay_init(80);
    LED_Init();
    uart_init(115200);
    init_uart2();
    OSInit(&err);                   //初始化UCOSIII
    OS_CRITICAL_ENTER();            //进入临界区
//    delay_ms(500);
    OSTaskCreate((OS_TCB *)&AppMainTaskTCB,
                 (CPU_CHAR *) "app_main task",
                 (OS_TASK_PTR ) app_main,
                 (void *)       0,                                                      //传递给任务函数的参数
                 (OS_PRIO)      APP_MAIN_TASK_PRIO,                                     //任务优先级
                 (CPU_STK *)    &APP_MAIN_TASK_STK[0],                                  //任务堆栈基地址
                 (CPU_STK_SIZE) APP_MAIN_TASK_SIZE/10,                                  //任务堆栈深度限位
                 (CPU_STK_SIZE) APP_MAIN_TASK_SIZE,                                     //
                 (OS_MSG_QTY)   0,                                                      //任务内部消息队列能够接受的最大数目，为0时禁止接收消息
                 (OS_TICK)      0,                                                      //当使能时间片轮转时的时间，片长度为0时为默认长度
                 (void *)       0,                                                      //用户补充的存储区
                 (OS_OPT)       OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR|OS_OPT_TASK_SAVE_FP,
                 (OS_ERR *)     &err);
      OS_CRITICAL_EXIT();                                                               //退出临界区
      OSStart(&err);                                                                    //开启UCOSIII
    while(1)
    {
        
    }    
}

void app_main(void *p_arg)
{
    OS_ERR err;
    CPU_SR_ALLOC();
    p_arg =p_arg;
}

void LED_Init(void)
{
    GPIO_InitTypeDef        GPIO_InitStructure;
    
    __HAL_RCC_GPIOE_CLK_ENABLE();
    
    GPIO_InitStructure.Pin=GPIO_PIN_7;
    GPIO_InitStructure.Mode=GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Speed=GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStructure.Pull=GPIO_PULLUP;
    HAL_GPIO_Init(GPIOE,&GPIO_InitStructure);
    
    HAL_GPIO_WritePin(GPIOE,GPIO_PIN_7,GPIO_PIN_RESET);
}



