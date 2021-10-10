#include "delay.h"

void delay_us(u32 nus)
{
	u32 tickcnt,ticksrt,tickend;
	u32 reload = SysTick->LOAD;                                         //��ȡװ��ֵ
    u32 ticktotal =nus *80;                                             //�ܽ�����
    ticksrt =SysTick->VAL;
    while(1)
    {
        tickend =SysTick->VAL;
        if(tickend >ticksrt)
            tickcnt +=tickend -ticksrt;
        else
            tickcnt +=reload -tickend+ticksrt;
        ticksrt =tickend;
        if(tickcnt >=ticktotal)
            break;     
    }
	
}
//�����ʮ����
void delay_ms(u32 nms)
{
    while(nms--)
    {
        delay_us(1000);
    }
}





