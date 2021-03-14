#include "aht10.h"


void IIC_Delay(void)
{
    delay_us(100);
}
/**
 * @brief       IIC初始化函数
 * 
*/
void IIC_Init(void)
{
    GPIO_InitTypeDef        GPIO_InitStruct;

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    /*  
            SCL -   PD6         SDA -   PC1
    */
   GPIO_InitStruct.Pin =GPIO_PIN_1;
   GPIO_InitStruct.Mode =GPIO_MODE_OUTPUT_PP;               //推挽输出
   GPIO_InitStruct.Pull =GPIO_PULLUP;
   GPIO_InitStruct.Speed =GPIO_SPEED_FAST;
   HAL_GPIO_Init(GPIOC,&GPIO_InitStruct);

   GPIO_InitStruct.Pin =GPIO_PIN_6;
   GPIO_InitStruct.Mode =GPIO_MODE_OUTPUT_PP;
   GPIO_InitStruct.Pull =GPIO_PULLUP;
   GPIO_InitStruct.Speed =GPIO_SPEED_FAST;
   HAL_GPIO_Init(GPIOD,&GPIO_InitStruct);

   IIC_SDA(1);
   IIC_SCL(1);
}

/**
 * @brief   产生IIC起始信号
*/

void IIC_Start(void)
{
    SDA_OUT();              //sda线输出
    IIC_SDA(1);
    IIC_SCL(1);
    IIC_Delay();
    IIC_SDA(0);                 //start:当CLK线为高电平，数据线拉低是IIC开始信号
    IIC_Delay();
    IIC_SCL(0);                 //钳住I2C总线，准备发送或接受数据
}

/**
 * @brief 产生IIC停止信号
 * 
*/
void IIC_Stop(void)
{
    SDA_OUT();          //sda线输出
    IIC_SDA(0);
    IIC_SCL(1);
    IIC_Delay();
    IIC_SDA(1);         //STOP 当时钟信号为高，数据线从低变高
    IIC_Delay();
    IIC_SCL(0);         //发送I2C停止信号
}

/**
 * @brief 等待应答信号到来
 * 
 * @return u8  1,接受应答失败
 *             0,接受应答成功
*/
uint8_t IIC_Wait_Ack(void)
{
    uint8_t ucErrTime = 0;
    SDA_IN();
    IIC_SDA(1);
    IIC_Delay();
    IIC_SCL(1);
    IIC_Delay();
    while(READ_SDA)
    {
        ucErrTime++;
        if(ucErrTime > 250)
        {
            IIC_Stop();
            return 1;
        }
    }
    IIC_SCL(0);                 //时钟输出0
    return 0;
}

/**
 * @brief 产生ACK应答
*/
void IIC_Ack(void)
{
    IIC_SCL(0);
    SDA_OUT();
    IIC_SDA(0);
    IIC_Delay();
    IIC_SCL(1);
    IIC_Delay();
    IIC_SCL(0);
}
/**
 * @brief 不产生ACK应答
 * 
*/
void IIC_NAck(void)
{
    IIC_SCL(0);
    SDA_OUT();
    IIC_SDA(1);
    IIC_Delay();
    IIC_SCL(1);
    IIC_Delay();
    IIC_SCL(0);
}

/**
 * @brief IIC发送一个字节
 * 
 * @param txd 需要发送的数据
 * 
*/
void IIC_Send_Byte(uint8_t txd)
{
    uint8_t t;
    SDA_OUT();
    IIC_SCL(0);             //拉低时钟 开始数据传输
    for(t=0;t<8;t++)
    {
        IIC_SDA((txd & 0x80) >>7);       //右移7位   
        txd <<=1;                       //左移1位
        IIC_SCL(1);
        IIC_Delay();
        IIC_SCL(0);
        IIC_Delay();
    }
}

/**
 * @brief	读1个字节数据
 *
 * @param   ack		1，发送ACK		0，发送nACK
 *
 * @return  u8		返回读取数据
 */
uint8_t IIC_Read_Byte(uint8_t ack)
{
    uint8_t i,receive =0;
    SDA_IN();                           //SDA设置为输入
    for(i =0;i<8;i++)
    {
        IIC_SCL(0);
        IIC_Delay();
        IIC_SCL(1);
        receive <<=1;
        if(READ_SDA)
        {
            receive++;
        }
        IIC_Delay();
    }
    if(!ack)
    {
        IIC_NAck();             //发送nACK
    }else{
        IIC_Ack();              //发送ACK
    }
    return receive;
}

/**
 * @brief	向ATH10写入数据
 *
 * @param   cmd		命令
 * @param   data	要写入的数据
 * @param   len		写入数据大小
 *
 * @return  u8		0,正常,其他,错误代码
 */
uint8_t AHT10_Write_Data(uint8_t cmd,uint8_t *data,uint8_t len)
{
    IIC_Start();
    IIC_Send_Byte((AHT10_IIC_ADDR << 1) | 0);                       //addr+读写指令     1表示读取   0表示写入 
    if(IIC_Wait_Ack())                                              //等待应答
    {
        IIC_Stop();
        return 1;
    }
    IIC_Send_Byte(cmd);                                             //写寄存器地址
    IIC_Wait_Ack();

    for(uint8_t i;i<len;i++)
    {
        IIC_Send_Byte(data[i]);
        IIC_Wait_Ack();
    }
    IIC_Stop();
    return 0;
}
/**
 * @brief	读一个字节
 *
 * @param   void
 *
 * @return  u8		读到的数据
 */
uint8_t AHT10_ReadOneByte(void)
{
    uint8_t res;
    IIC_Start();
    IIC_Send_Byte((AHT10_IIC_ADDR <<1) |0x01);                  //发送器件地址+读命令

    if(IIC_Wait_Ack())
    {
        IIC_Stop();
        return 1;
    }
    res =IIC_Read_Byte(0);
    IIC_Stop();
    return res;
}
/**
 * @brief	读数据
 *
 * @param   data	数据缓存
 * @param   len		读数据大小
 *
 * @return  u8		0,正常,其他,错误代码
 */
uint8_t AHT10_Read_Data(uint8_t *data,uint8_t len)
{
    IIC_Start();
    IIC_Send_Byte((AHT10_IIC_ADDR) <<1|0x01);

    if(IIC_Wait_Ack())
    {
        IIC_Stop();
        return 1;
    }

    for(uint8_t i=0;i< len;i++)
    {
        if(i ==(len -1))
        {
            data[i] =IIC_Read_Byte(0);
        }else{
            data[i] =IIC_Read_Byte(1);
        }
    }
    IIC_Stop();
    return 0;
}

/**
 * @brief	读取温度数据
 *
 * @param   void
 *
 * @return  float	温度数据（单位：摄氏度）
 */

float AHT10_Read_Temperature(void)
{
    uint8_t res;
    uint8_t cmd[2] ={0,0};
    uint8_t temp[6];
    float cur_temp;

    res =AHT10_Write_Data(AHT10_GET_DATA,cmd,2);
    if(res) return 1;

    res =AHT10_Read_Data(temp,6);

    if(res) return 1;

    cur_temp = ((temp[3] &0xf) <<16 |temp[4]<<8 |temp[5])* 200.0/(1<<20)-50;
    return cur_temp;

}

/**
 * @brief	读取湿度数据
 *
 * @param   void
 *
 * @return  float	湿度数据（单位：%RH）
 */
float AHT10_Read_Humidity(void)
{
    uint8_t res =0;
    uint8_t cmd[2] ={0,0};
    uint8_t humi[6];
    float cur_humi;

    res =AHT10_Write_Data(AHT10_GET_DATA,cmd,2);                            //发送读取数据命令

    if(res) return 1;

    res =AHT10_Read_Data(humi,6);                                       //读取数据
    if(res) return 1;

    cur_humi =(humi[1] << 12 | humi[2] <<4 |(humi[3] & 0xF0)) * 100.0/(1<<20);

    return cur_humi;
}

/**
 * @brief	ATH10传感器初始化
 *
 * @param   void
 *
 * @return  u8		0,初始化成功，其他,失败
 */
uint8_t AHT10_Init(void)
{
    uint8_t res;
    uint8_t temp[5] ={0,0};
    IIC_Init();

    res = AHT10_Write_Data(AHT10_NORMAL_CMD,temp, 2);

    if(res !=0) return 1;

    delay_ms(300);
    temp[0] =0x08;
    temp[1] =0x00;
    res =AHT10_Write_Data(AHT10_CALIBRATION_CMD,temp,2);

    if(res !=0) return 1;
    delay_ms(300);

    return 0;
}

