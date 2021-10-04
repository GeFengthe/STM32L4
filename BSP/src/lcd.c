#include "lcd.h"
#include "delay.h"
#include "stdio.h"
#include "font.h"

#define LCD_BUFF_SIZE           1152         //һ�������ֽ�
#define LCD_TOTAL_SIZE          240*240*2       //
static uint8_t lcd_buff[LCD_BUFF_SIZE];


#define POINT_COLOR             DARKBLUE
#define BACK_COLOR              WHITE

static void LCD_Gpio_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    /*
    	LCD_PWR:	PB7
    	LCD_RST:	PB6
    	LCD_DC:		PB4
    	LCD_CS:		PD7
    */
    GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = GPIO_PIN_7;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_RESET);

    LCD_CS(0);
    LCD_PWR(0);

    LCD_RST(0);
    delay_ms(120);
    LCD_RST(1);

    Init_spi();	//��ʼ��SPI3�ӿ�
}


/**
 * @brief	LCD�ײ�SPI�������ݺ���
 *
 * @param   data	���ݵ���ʼ��ַ
 * @param   size	�������ݴ�С
 *
 * @return  void
 */
static void LCD_SPI_Send(u8 *data, u16 size)
{
    if(SPI3_WriteByte(data, size)==0)
    {
        printf("write success\r\n");
    }else
    {
        printf("write fail\r\n");
    }
}

//д����
static void LCD_Write_Cmd(u8 cmd)
{
    LCD_DC(0);

    LCD_SPI_Send(&cmd, 1);
}


/**
 * @brief	д���ݵ�LCD
 *
 * @param   cmd		��Ҫ���͵�����
 *
 * @return  void
 */
static void LCD_Write_Data(u8 data)
{
    LCD_DC(1);

    LCD_SPI_Send(&data, 1);
}

/**
 * @brief	д����ֵ����ݵ�LCD
 *
 * @param   cmd		��Ҫ���͵�����
 *
 * @return  void
 */
void LCD_Write_HalfWord(const u16 da)
{
    u8 data[2] = {0};

    data[0] = da >> 8;
    data[1] = da;

    LCD_DC(1);
    LCD_SPI_Send(data, 2);
}
/**
 * ��������д��LCD��������
 *
 * @param   x1,y1	�������
 * @param   x2,y2	�յ�����
 *
 * @return  void
 */
void LCD_Address_Set(u16 x1, u16 y1, u16 x2, u16 y2)
{
    LCD_Write_Cmd(0x2a);
    LCD_Write_Data(x1 >> 8);
    LCD_Write_Data(x1);
    LCD_Write_Data(x2 >> 8);
    LCD_Write_Data(x2);

    LCD_Write_Cmd(0x2b);
    LCD_Write_Data(y1 >> 8);
    LCD_Write_Data(y1);
    LCD_Write_Data(y2 >> 8);
    LCD_Write_Data(y2);

    LCD_Write_Cmd(0x2C);
}
//��һ����ɫ����
void LCD_Clean(uint16_t color)
{
    uint16_t i,j;
    uint8_t tmp[2];
    tmp[0] = color >>8;
    tmp[1] = color;
    
    LCD_Address_Set(0, 0, LCD_Width - 1, LCD_Height - 1);
    
    for(i = 0;i<LCD_BUFF_SIZE/2;i++)
    {
        lcd_buff[i*2] = tmp[0];
        lcd_buff[i*2+1]=tmp[1];
    }
    LCD_DC(1);
    for(j = 0;j<(LCD_TOTAL_SIZE/LCD_BUFF_SIZE);j++)
    {
        LCD_SPI_Send(lcd_buff,LCD_BUFF_SIZE);
    }
}




/**
 * �������ɫ����
 *
 * @param   x,y		��������
 *
 * @return  void
 */
void LCD_Draw_ColorPoint(u16 x, u16 y,u16 color)
{
    LCD_Address_Set(x, y, x, y);
    LCD_Write_HalfWord(color);
}


/**
 * @brief	��ʾһ��ASCII���ַ�
 *
 * @param   x,y		��ʾ��ʼ����
 * @param   chr		��Ҫ��ʾ���ַ�
 * @param   size	�����С(֧��16/24/32������)
 *
 * @return  void
 */
void LCD_ShowChar(uint16_t x, uint16_t y, char chr, uint8_t size)
{
    uint8_t temp, t1, t;
    uint8_t csize;		//�õ�����һ���ַ���Ӧ������ռ���ֽ���
    uint16_t colortemp;
    uint8_t sta;

    chr = chr - ' '; //�õ�ƫ�ƺ��ֵ��ASCII�ֿ��Ǵӿո�ʼȡģ������-' '���Ƕ�Ӧ�ַ����ֿ⣩

    if((x > (LCD_Width - size / 2)) || (y > (LCD_Height - size)))	return;

    LCD_Address_Set(x, y, x + size / 2 - 1, y + size - 1);//(x,y,x+8-1,y+16-1)

    if((size == 16) || (size == 32) )	//16��32������
    {
        csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2);

        for(t = 0; t < csize; t++)
        {
            if(size == 16)temp = asc2_1608[chr][t];	//����1608����
            else if(size == 32)temp = asc2_3216[chr][t];	//����3216����
            else return;			//û�е��ֿ�

            for(t1 = 0; t1 < 8; t1++)
            {
                if(temp & 0x80) colortemp = POINT_COLOR;
                else colortemp = BACK_COLOR;

                LCD_Write_HalfWord(colortemp);
                temp <<= 1;
            }
        }
    }

	else if  (size == 12)	//12������
	{
        csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2);

        for(t = 0; t < csize; t++)
        {
            temp = asc2_1206[chr][t];

            for(t1 = 0; t1 < 6; t1++)
            {
                if(temp & 0x80) colortemp = POINT_COLOR;
                else colortemp = BACK_COLOR;

                LCD_Write_HalfWord(colortemp);
                temp <<= 1;
            }
        }
    }
	
    else if(size == 24)		//24������
    {
        csize = (size * 16) / 8;

        for(t = 0; t < csize; t++)
        {
            temp = asc2_2412[chr][t];

            if(t % 2 == 0)sta = 8;
            else sta = 4;

            for(t1 = 0; t1 < sta; t1++)
            {
                if(temp & 0x80) colortemp = POINT_COLOR;
                else colortemp = BACK_COLOR;

                LCD_Write_HalfWord(colortemp);
                temp <<= 1;
            }
        }
    }
}
/**
 * @brief	��ʾ�ַ���
 *
 * @param   x,y		�������
 * @param   width	�ַ���ʾ������
 * @param   height	�ַ���ʾ����߶�
 * @param   size	�����С
 * @param   p		�ַ�����ʼ��ַ
 *
 * @return  void
 */
void LCD_ShowString(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t size,char *p)
{
    uint8_t x0 =x;
    width +=x;
    height +=y;
    while((*p <='~')&&(*p >=' '))
    {
        if(x >=width)
        {
            x =x0;
            y +=size;
        }
        if(y >=height)
        {
            break;                          //�˳�
        }
        LCD_ShowChar(x,y,*p,size);
        x +=size/2;
        p++;
    }
}


void LCD_Init(void)
{
    LCD_Gpio_Init();	//Ӳ���ӿڳ�ʼ��

    delay_ms(150);
    /* Sleep Out */
    LCD_Write_Cmd(0x11);
    /* wait for power stability */
    delay_ms(150);

    /* Memory Data Access Control */
    LCD_Write_Cmd(0x36);
    LCD_Write_Data(0x00);

    /* RGB 5-6-5-bit  */
    LCD_Write_Cmd(0x3A);
    LCD_Write_Data(0x65);

    /* Porch Setting */
    LCD_Write_Cmd(0xB2);
    LCD_Write_Data(0x0C);
    LCD_Write_Data(0x0C);
    LCD_Write_Data(0x00);
    LCD_Write_Data(0x33);
    LCD_Write_Data(0x33);

    /*  Gate Control */
    LCD_Write_Cmd(0xB7);
    LCD_Write_Data(0x72);

    /* VCOM Setting */
    LCD_Write_Cmd(0xBB);
    LCD_Write_Data(0x3D);   //Vcom=1.625V

    /* LCM Control */
    LCD_Write_Cmd(0xC0);
    LCD_Write_Data(0x2C);

    /* VDV and VRH Command Enable */
    LCD_Write_Cmd(0xC2);
    LCD_Write_Data(0x01);

    /* VRH Set */
    LCD_Write_Cmd(0xC3);
    LCD_Write_Data(0x19);

    /* VDV Set */
    LCD_Write_Cmd(0xC4);
    LCD_Write_Data(0x20);

    /* Frame Rate Control in Normal Mode */
    LCD_Write_Cmd(0xC6);
    LCD_Write_Data(0x0F);	//60MHZ

    /* Power Control 1 */
    LCD_Write_Cmd(0xD0);
    LCD_Write_Data(0xA4);
    LCD_Write_Data(0xA1);

    /* Positive Voltage Gamma Control */
    LCD_Write_Cmd(0xE0);
    LCD_Write_Data(0xD0);
    LCD_Write_Data(0x04);
    LCD_Write_Data(0x0D);
    LCD_Write_Data(0x11);
    LCD_Write_Data(0x13);
    LCD_Write_Data(0x2B);
    LCD_Write_Data(0x3F);
    LCD_Write_Data(0x54);
    LCD_Write_Data(0x4C);
    LCD_Write_Data(0x18);
    LCD_Write_Data(0x0D);
    LCD_Write_Data(0x0B);
    LCD_Write_Data(0x1F);
    LCD_Write_Data(0x23);

    /* Negative Voltage Gamma Control */
    LCD_Write_Cmd(0xE1);
    LCD_Write_Data(0xD0);
    LCD_Write_Data(0x04);
    LCD_Write_Data(0x0C);
    LCD_Write_Data(0x11);
    LCD_Write_Data(0x13);
    LCD_Write_Data(0x2C);
    LCD_Write_Data(0x3F);
    LCD_Write_Data(0x44);
    LCD_Write_Data(0x51);
    LCD_Write_Data(0x2F);
    LCD_Write_Data(0x1F);
    LCD_Write_Data(0x1F);
    LCD_Write_Data(0x20);
    LCD_Write_Data(0x23);

    /* Display Inversion On */
    LCD_Write_Cmd(0x21);

    LCD_Write_Cmd(0x29);

    LCD_Address_Set(0, 0, LCD_Width - 1, LCD_Height - 1);

    LCD_Clean(WHITE);

    /*����ʾ*/
    LCD_PWR(1);
}