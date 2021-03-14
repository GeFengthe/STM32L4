#include "delay.h"

void delay_us(u32 nus)
{
	u32 tickcnt,ticksrt,tickend;
	u32 reload = SysTick->LOAD;                                         //获取装载值
    u32 ticktotal =nus *80;                                           //总节拍数
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
    delay_us(nms*1000);
}





