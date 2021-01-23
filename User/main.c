#include "stm32l4xx.h"
#include "sys.h"
#include "delay.h"
#include "stm32l4xx_hal_gpio.h"
void LED_Init(void);
int main()
{
    HAL_Init();
    SystemClock_Config();
    LED_Init();
    while(1)
    {
        
    }
    
    
    
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



