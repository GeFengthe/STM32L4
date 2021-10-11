#include "delay.h"

void delay_us(uint32_t nus)
{
	uint32_t tickcnt=0,ticksrt,tickend;
	uint32_t reload = SysTick->LOAD;                                         //获取装载值
    uint32_t ticktotal =nus *80;                                           //总节拍数
    ticksrt =SysTick->VAL;
    while(1)
    {
        tickend =SysTick->VAL;
        if(ticksrt >tickend)
            tickcnt +=ticksrt -tickend;
        else
            tickcnt +=reload -tickend+ticksrt;
        ticksrt =tickend;
        if(tickcnt >=ticktotal)
            break;     
    }	
}
//最大五十几秒
void delay_ms(u32 nms)
{
    while(nms--)
    {
        delay_us(1000);
    }
}





