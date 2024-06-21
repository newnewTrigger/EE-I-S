#include "JLX12864G.h"


////ʹ��GPIO
void LCD_PORT_init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);						//Enable PortC
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC));
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);						//Enable PortK
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK));
	  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);						//Enable PortB
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);						//Enable PortE
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));

    GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_0 | GPIO_PIN_1| GPIO_PIN_2);
    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE,  GPIO_PIN_5 | GPIO_PIN_4);
	  GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_4 );
    GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_6) ;	//PC6����Ϊ����

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);						//DELAYʱ�����
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));	    //DELAYʱ�����
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,  GPIO_PIN_1);    //DELAYʱ�����
}
//��ʱ
void LCD_delay(int n_ms)
{
    int j,k;
//	TEST_L;
    for(j=0; j<n_ms; j++)
        for(k=0; k<110; k++);
//	TEST_H;
}
//����ʱ
void LCD_delay_us(int n_us)
{
    int j,k;
    for(j=0; j<n_us; j++)
        for(k=0; k<1; k++);
}
//дָ�LCD ģ��
void transfer_command_lcd(int data1)
{
    char i;
    LCD_CS_L;
    LCD_RS_L;
    for(i=0; i<8; i++)
    {
        LCD_SCK_L;
        //delay_us(10); //��������ʱ
        if(data1&0X80) LCD_SDA_H;
        else LCD_SDA_L;
        //delay_us(10); //��������ʱ
        data1=data1<<=1;
        LCD_SCK_H;
    }
    LCD_CS_H;
}
//д���ݵ�LCD ģ��
void transfer_data_lcd(int data1)
{
    char i;
    LCD_CS_L;
    LCD_RS_H;
    for(i=0; i<8; i++)
    {
        LCD_SCK_L;
//delay_us(10); //��������ʱ
        if(data1&0x80) LCD_SDA_H;
        else LCD_SDA_L;
        LCD_SCK_H;
//delay_us(10); //��������ʱ
        data1=data1<<=1;
    }
    LCD_CS_H;

}
//LCD ģ���ʼ��
void LCD_Init()
{
//		LCD_PORT_init();
    LCD_RST_L; //�͵�ƽ��λ
    LCD_delay(100);
    LCD_RST_H; //��λ���
    LCD_delay(100);
    transfer_command_lcd(0xe2); //��λ
    LCD_delay(5);
    transfer_command_lcd(0x2c); //��ѹ����1
    LCD_delay(50);
    transfer_command_lcd(0x2e); //��ѹ����2
    LCD_delay(50);
    transfer_command_lcd(0x2f); //��ѹ����3
    LCD_delay(5);
    transfer_command_lcd(0x23); //�ֵ��Աȶȣ������÷�Χ0x20��0x27
    transfer_command_lcd(0x81); //΢���Աȶ�
    transfer_command_lcd(0x28); //΢���Աȶȵ�ֵ�������÷�Χ0x00��0x3f
    transfer_command_lcd(0xa2); //1/9 ƫѹ�ȣ�bias��
    transfer_command_lcd(0xc8); //��ɨ��˳�򣺴��ϵ���
    transfer_command_lcd(0xa0); //��ɨ��˳�򣺴�����
    transfer_command_lcd(0x40); //��ʼ�У���һ�п�ʼ
    transfer_command_lcd(0xaf); //����ʾ
}
void lcd_address(unsigned int page,unsigned int column)
{
    column=column-0x01;
    transfer_command_lcd(0xb0+page-1); //����ҳ��ַ��ÿ8 ��Ϊһҳ��ȫ����64 �У����ֳ�8 ҳ
    transfer_command_lcd(0x10+(column>>4&0x0f)); //�����е�ַ�ĸ�4 λ
    transfer_command_lcd(column&0x0f); //�����е�ַ�ĵ�4 λ
}
//ȫ������
void clear_screen()
{
    unsigned char i,j;
    for(i=0; i<9; i++)
    {
        transfer_command_lcd(0xb0+i);
        transfer_command_lcd(0x10);
        transfer_command_lcd(0x00);
        for(j=0; j<132; j++)
        {
            transfer_data_lcd(0x00);
        }
    }
}
//������ʾ
void full_display()
{
    int i,j;
    for(i=0; i<8; i++)
    {
        lcd_address(i+1,0);
        for(j=0; j<128; j++)
        {
            transfer_data_lcd(0xff);
        }
    }
}
//��ʾ128x64 ����ͼ��
void display_128x64(unsigned char *dp)
{
    unsigned int i,j;
    for(j=0; j<8; j++)
    {
        lcd_address(j+1,1);
        for (i=0; i<128; i++)
        {
            transfer_data_lcd(*dp); //д���ݵ�LCD,ÿд��һ��8 λ�����ݺ��е�ַ�Զ���1
            dp++;
        }
    }
}
//��ʾ16x16 ����ͼ�񡢺��֡���Ƨ�ֻ�16x16 ���������ͼ��
void display_graphic_16x16(unsigned char page,unsigned char column,unsigned char *dp)
{
    unsigned int i,j;
    for(j=0; j<2; j++)
    {
        lcd_address(page+j,column);
        for (i=0; i<16; i++)
        {
            transfer_data_lcd(*dp); //д���ݵ�LCD,ÿд��һ��8 λ�����ݺ��е�ַ�Զ���1
            dp++;
        }
    }
}
//��ʾ8x16 ����ͼ��ASCII, ��8x16 ����������ַ�������ͼ��
void display_graphic_8x16(unsigned char page,unsigned char column,unsigned char *dp)
{
    unsigned int i,j;
    for(j=0; j<2; j++)
    {
        lcd_address(page+j,column);
        for (i=0; i<8; i++)
        {
            transfer_data_lcd(*dp); //д���ݵ�LCD,ÿд��һ��8 λ�����ݺ��е�ַ�Զ���1
            dp++;
        }
    }
}
//��ʾ5X8 ����ͼ��ASCII, ��5x8 ����������ַ�������ͼ��
void display_graphic_5x8(unsigned char page,unsigned char column,unsigned char *dp)
{
    unsigned int i;
    lcd_address(page,column);
    for (i=0; i<6; i++)
    {
        transfer_data_lcd(*dp);
        dp++;
    }
}
//��ָ�����Ѷ�ֿ�IC
void send_command_to_ROM( unsigned char datu )
{
    unsigned char i;
    for(i=0; i<8; i++ )
    {
        LCD_ROM_SCK_L;
        LCD_delay_us(10);
        if(datu&0x80) LCD_ROM_IN_H;
        else LCD_ROM_IN_L;
        datu = datu<<1;
        LCD_ROM_SCK_H;
        LCD_delay_us(10);
    }
}
//�Ӿ���Ѷ�ֿ�IC ��ȡ���ֻ��ַ����ݣ�1 ���ֽڣ�
static unsigned char get_data_from_ROM( )
{
    unsigned char i;
    unsigned char ret_data=0;
    for(i=0; i<8; i++)
    {
        LCD_ROM_OUT_H;
        LCD_ROM_SCK_L;
//LCD_delay_us(1);
        ret_data=ret_data<<1;
        if( LCD_ROM_OUT )
            ret_data=ret_data+1;
        else
            ret_data=ret_data+0;
        LCD_ROM_SCK_H;
//LCD_delay_us(1);
    }
    return(ret_data);
}
//��ָ����ַ��������д��Һ����ָ����page,column)������,�Ƿ񷴰���ʾ
void get_and_write_16x16(unsigned long fontaddr,unsigned char page,unsigned char column,unsigned char inverse)
{
    unsigned char i,j,disp_data;
    LCD_ROM_CS_L;
    send_command_to_ROM(0x03);
    send_command_to_ROM((fontaddr&0xff0000)>>16); //��ַ�ĸ�8 λ,��24 λ
    send_command_to_ROM((fontaddr&0xff00)>>8); //��ַ����8 λ,��24 λ
    send_command_to_ROM(fontaddr&0xff); //��ַ�ĵ�8 λ,��24 λ
    for(j=0; j<2; j++)
    {
        lcd_address(page+j,column);
        for(i=0; i<16; i++ )
        {
            disp_data=get_data_from_ROM();
            if (inverse==0) transfer_data_lcd(disp_data); //д���ݵ�LCD,ÿд��1 �ֽڵ����ݺ��е�ַ�Զ���1
            else transfer_data_lcd(~disp_data);
        }
    }
    LCD_ROM_CS_H;
}
//��ָ����ַ��������д��Һ����ָ����page,column)������,�Ƿ񷴰���ʾ
void get_and_write_8x16(unsigned long fontaddr,unsigned char page,unsigned char column,unsigned char inverse)
{
    unsigned char i,j,disp_data;
    LCD_ROM_CS_L;
    send_command_to_ROM(0x03);
    send_command_to_ROM((fontaddr&0xff0000)>>16); //��ַ�ĸ�8 λ,��24 λ
    send_command_to_ROM((fontaddr&0xff00)>>8); //��ַ����8 λ,��24 λ
    send_command_to_ROM(fontaddr&0xff); //��ַ�ĵ�8 λ,��24 λ
    for(j=0; j<2; j++)
    {
        lcd_address(page+j,column);
        for(i=0; i<8; i++ )
        {
            disp_data=get_data_from_ROM();
            if (inverse==0) transfer_data_lcd(disp_data); //д���ݵ�LCD,ÿд��1 �ֽڵ����ݺ��е�ַ�Զ���1
            else transfer_data_lcd(~disp_data);
        }
    }
    LCD_ROM_CS_H;
}
//��ָ����ַ��������д��Һ����ָ����page,column)������,�Ƿ񷴰���ʾ
void get_and_write_5x8(unsigned long fontaddr,unsigned char page,unsigned char column,unsigned char inverse)
{
    unsigned char i,disp_data;
    LCD_ROM_CS_L;
    send_command_to_ROM(0x03);
    send_command_to_ROM((fontaddr&0xff0000)>>16); //��ַ�ĸ�8 λ,��24 λ
    send_command_to_ROM((fontaddr&0xff00)>>8); //��ַ����8 λ,��24 λ
    send_command_to_ROM(fontaddr&0xff); //��ַ�ĵ�8 λ,��24 λ
    lcd_address(page,column);
    for(i=0; i<5; i++ )
    {
        disp_data=get_data_from_ROM();
        if (inverse==0) transfer_data_lcd(disp_data); //д���ݵ�LCD,ÿд��1 �ֽڵ����ݺ��е�ַ�Զ���1
        else transfer_data_lcd(~disp_data);
    }
    LCD_ROM_CS_H;
}
//****************************************************************
unsigned long fontaddr=0;
void display_GB2312_string(unsigned char page,unsigned char column,unsigned char *text,unsigned char inverse)
{
    unsigned char i= 0;
    while((text[i]>0x00))
    {
        if(((text[i]>=0xb0) &&(text[i]<=0xf7))&&(text[i+1]>=0xa1))
        {
//������壨GB2312�������ھ���Ѷ�ֿ�IC �еĵ�ַ�����¹�ʽ�����㣺
//Address = ((MSB - 0xB0) * 94 + (LSB - 0xA1)+ 846)*32+ BaseAdd;BaseAdd=0
//���ڵ���8 λ��Ƭ���г˷�������⣬���Է�����ȡ��ַ
            fontaddr = (text[i]- 0xb0)*94;
            fontaddr += (text[i+1]-0xa1)+846;
            fontaddr = (unsigned long)(fontaddr*32);
            get_and_write_16x16(fontaddr,page,column,inverse); //��ָ����ַ��������д��Һ����ָ����page,column)������
            i+=2;
            column+=16;
        }
        else if(((text[i]>=0xa1) &&(text[i]<=0xa3))&&(text[i+1]>=0xa1))
        {
//������壨GB2312��15x16 ����ַ��ھ���Ѷ�ֿ�IC �еĵ�ַ�����¹�ʽ�����㣺
//Address = ((MSB - 0xa1) * 94 + (LSB - 0xA1))*32+ BaseAdd;BaseAdd=0
//���ڵ���8 λ��Ƭ���г˷�������⣬���Է�����ȡ��ַ
            fontaddr = (text[i]- 0xa1)*94;
            fontaddr += (text[i+1]-0xa1);
            fontaddr = (unsigned long)(fontaddr*32);
            get_and_write_16x16(fontaddr,page,column,inverse); //��ָ����ַ��������д��Һ����ָ����page,column)������
            i+=2;
            column+=16;
        }
        else if((text[i]>=0x20) &&(text[i]<=0x7e))
        {
            fontaddr = (text[i]- 0x20);
            fontaddr = (unsigned long)(fontaddr*16);
            fontaddr = (unsigned long)(fontaddr+0x3cf80);
            get_and_write_8x16(fontaddr,page,column,inverse); //��ָ����ַ��������д��Һ����ָ����page,column)������
            i+=1;
            column+=8;
        }
        else
            i++;
    }
}
void display_string_5x8(unsigned char page,unsigned char column,unsigned char *text,unsigned char inverse)
{
    unsigned char i= 0;
    while((text[i]>0x00))
    {
        if((text[i]>=0x20) &&(text[i]<=0x7e))
        {
            fontaddr = (text[i]- 0x20);
            fontaddr = (unsigned long)(fontaddr*8);
            fontaddr = (unsigned long)(fontaddr+0x3bfc0);
            get_and_write_5x8(fontaddr,page,column,inverse); //��ָ����ַ��������д��Һ����ָ����page,column)������
            i+=1;
            column+=6;
        }
        else
            i++;
    }
}
//===============main program===================
//void main(void)
//{
//    while(1)
//    {
//        initial_lcd();
//        clear_screen(); //clear all dots
//        display_128x64(bmp1);
//        LCD_delay(2000);
//        display_GB2312_string(1,1,"12864,�������ֿ�"); //�ڵ�1 ҳ����1 �У���ʾһ��16x16 �����ֻ�8x16 ��ASCII ��
//        display_GB2312_string(3,1,"16X16 ���庺�ֿ�,"); //��ʾһ��16x16 �����ֻ�8x16 ��ASCII ��.������ͬ
//        display_GB2312_string(5,1,"��8X16 ����ASCII,");
//        display_GB2312_string(7,1,"��5x8 ����ASCII ��");
//        LCD_delay(4000);
//        clear_screen();
//        display_GB2312_string(1,1,"����Ѷ�����ڶ���");
//        display_GB2312_string(3,1,"������ʮһ������");
//        display_GB2312_string(5,1,"��Ҫ����Һ��ģ��");
//        display_GB2312_string(7,1,"Ʒ��������Ϸ���");
//        LCD_delay(4000);
//        display_GB2312_string(1,1,"GB2312 �����ֿ⼰");
//        display_GB2312_string(3,1,"��ͼ�͹��ܣ�����");
//        display_GB2312_string(5,1,"����ֻ�ͼ�����");
//        display_GB2312_string(7,1,"Ƨ�֣����磺 ");
//        display_graphic_16x16(7,97,jiong1); //�ڵ�7ҳ����81����ʾ�����Ա���Ƨ���֡��塱
//        display_graphic_16x16(7,113,lei1); //��ʾ�����Ա���Ƨ����"����
//        LCD_delay(4000);
//        clear_screen();
//        display_GB2312_string(1,1,"<!@#$%^&*()_-+]/"); //�ڵ�1 ҳ����1 �У���ʾһ��16x16 �����ֻ�8*16 ��ASCII ��
//        display_string_5x8(3,1,"<!@#$%^&*()_-+]/;.,?[");//�ڵ�3 ҳ����1 �У���ʾһ��5x8 �����ASCII ��
//        display_string_5x8(4,1,"JLX electronics Co., ");//��ʾһ��5x8 �����ASCII ��
//        display_string_5x8(5,1,"Ltd. established at ");//��ʾһ��5x8 �����ASCII ��
//        display_string_5x8(6,1,"year 2004.Focus LCM. ");//��ʾһ��5x8 �����ASCII ��
//        display_string_5x8(7,1,"TEL:0755-29784961 ");//��ʾһ��5x8 �����ASCII ��
//        display_string_5x8(8,1,"FAX:0755-29784964 ");//��ʾһ��5x8 �����ASCII ��
//        LCD_delay(4000);
//        display_GB2312_string(1,1,"����������������"); //�ڵ�1 ҳ����1 �У���ʾһ��16x16 �����ֻ�8x16 ��ASCII ��
//        display_GB2312_string(3,1,"����������������"); //��ʾһ��16x16 �����ֻ�8x16 ��ASCII ��.������ͬ
//        display_GB2312_string(5,1,"����������������");
//        display_GB2312_string(7,1,"����������������");
//        LCD_delay(4000);
//        display_GB2312_string(1,1,"����������������");
//        display_GB2312_string(3,1,"����������������");
//        display_GB2312_string(5,1,"����������������");
//        display_GB2312_string(7,1,"����������������");
//        LCD_delay(4000);
//    }
//}
