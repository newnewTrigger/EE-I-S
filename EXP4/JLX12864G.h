#ifndef	_JLX12864G_H
#define	_JLX12864G_H

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif


#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h" 
#include "inc/hw_types.h" 
#include "driverlib/pin_map.h"  
#include "driverlib/sysctl.h"	



//��������JLX12864Һ������TM4C1294�ĹܽŶ��壬ʹ��GPIO����LCD����IC�ӿڡ��ֿ�IC�ӿ�
#define LCD_CS_H GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, GPIO_PIN_4)		//LCD_CS <--> PB4
#define LCD_CS_L GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, 0)
#define LCD_SCK_H GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, GPIO_PIN_5)		//LCD_SCK <--> PB5
#define LCD_SCK_L GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, 0)
#define LCD_SDA_H GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_PIN_4)		//LCD_SDA <--> PE4
#define LCD_SDA_L GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_4, 0)
#define LCD_RST_H GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_0, GPIO_PIN_0)		//LCD_RST <--> PK0
#define LCD_RST_L GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_0, 0 )
#define LCD_RS_H GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_1, GPIO_PIN_1)		//LCD_RS <--> PK1
#define LCD_RS_L GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_1, 0)

#define LCD_ROM_IN_H GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_2, GPIO_PIN_2)		//ROM_IN <--> PK2
#define LCD_ROM_IN_L GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_2, 0)
#define LCD_ROM_OUT_H GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, GPIO_PIN_6)		//ROM_OUT <--> PC6
#define LCD_ROM_OUT_L GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0)
#define LCD_ROM_SCK_H GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, GPIO_PIN_5)		//ROM_SCK <--> PC5
#define LCD_ROM_SCK_L GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0)
#define LCD_ROM_CS_H GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_PIN_4)		//ROM_CS <--> PC4
#define LCD_ROM_CS_L GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, 0)

#define LCD_ROM_OUT GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_6)

#define TEST_H GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1)		//DELAYʱ����Թܽ�
#define TEST_L GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0)


//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************
extern void LCD_PORT_init(void);

//��ʱ
extern void LCD_delay(int n_ms);

//����ʱ
extern void LCD_delay_us(int n_us); 
 
// дָ�LCDģ��
extern void transfer_command_lcd(int data1);

// д���ݵ�LCDģ��
extern void transfer_data_lcd(int data1);

//LCDģ���ʼ��
extern void LCD_Init(void);

extern void lcd_address(unsigned int page,unsigned int column); 

//ȫ������
extern void clear_screen(void);
 
//������ʾ
extern void full_display(void); 

//��ʾ128x64 ����ͼ��
extern void display_128x64(unsigned char *dp);

//��ʾ32x32 ����ͼ��ASCII, ��32x32 ����������ַ�������ͼ��
extern void display_graphic_32x32(unsigned char page,unsigned char column,unsigned char *dp); 

//��ʾ16x16 ����ͼ�񡢺��֡���Ƨ�ֻ�16x16 ���������ͼ��
extern void display_graphic_16x16(unsigned char page,unsigned char column,unsigned char *dp); 

//��ʾ8x16 ����ͼ��ASCII, ��8x16 ����������ַ�������ͼ��
extern void display_graphic_8x16(unsigned char page,unsigned char column,unsigned char *dp); 

//��ָ�����Ѷ�ֿ�IC
extern void send_command_to_ROM(unsigned char datu);

//�Ӿ���Ѷ�ֿ�IC ��ȡ���ֻ��ַ����ݣ�1 ���ֽڣ�
//extern static unsigned char get_data_from_ROM();

//��ָ����ַ��������д��Һ����ָ����page,column)������
extern void get_and_write_16x16(unsigned long fontaddr,unsigned char page,unsigned char column,unsigned char inverse);

//��ָ����ַ��������д��Һ����ָ����page,column)������
extern void get_and_write_8x16(unsigned long fontaddr,unsigned char page,unsigned char column,unsigned char inverse);

//�ڵ� page�У���column�У���ʾһ�� 16x16 �����ֻ� 8x16 �� ASCII �֣��Ƿ񷴰���ʾ
extern void display_GB2312_string(unsigned char page,unsigned char column,unsigned char *text,unsigned char inverse);

extern void display_string_5x8(unsigned char page,unsigned char column,unsigned char *text,unsigned char inverse);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __TM1638_H__

