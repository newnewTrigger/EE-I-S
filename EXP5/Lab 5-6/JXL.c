#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"        // 基址宏定义
#include "inc/hw_types.h"         // 数据类型宏定义，寄存器访问函数
#include "driverlib/debug.h"      // 调试用
#include "driverlib/gpio.h"       // 通用IO口宏定义
#include "driverlib/pin_map.h"    // TM4C系列MCU外围设备管脚宏定义
#include "driverlib/sysctl.h"	  // 系统控制定义
#define LCD_CS_H GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4,GPIO_PIN_4)
#define LCD_CS_L GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4,0)
#define LCD_SCK_H GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_5,GPIO_PIN_5)
#define LCD_SCK_L GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_5,0)
#define LCD_SDA_H GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_4,GPIO_PIN_4)
#define LCD_SDA_L GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_4,0)
#define LCD_RST_H GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_0,GPIO_PIN_0)
#define LCD_RST_L GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_0,0)
#define LCD_RS_H GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_1,GPIO_PIN_1)
#define LCD_RS_L GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_1,0)

#define LCD_ROM_IN_H GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_2,GPIO_PIN_2)
#define LCD_ROM_IN_L GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_2,0)
#define LCD_ROM_SCK_H GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_5,GPIO_PIN_5)
#define LCD_ROM_SCK_L GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_5,0)
#define LCD_ROM_CS_H GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_4,GPIO_PIN_4)
#define LCD_ROM_CS_L GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_4,0)

#define LCD_ROM_OUT GPIOPinRead(GPIO_PORTC_BASE,GPIO_PIN_6)

#define TEST_H GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1,GPIO_PIN_1)
#define TEST_L GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1,0)


uint8_t bmp1[]; 
uint8_t jiong1[]={//-- ??: ? -- 
//-- ?? 12; ??????????:? x ?=16x16 -- 
0x00,0xFE,0x82,0x42,0xA2,0x9E,0x8A,0x82,0x86,0x8A,0xB2,0x62,0x02,0xFE,0x00,0x00, 
0x00,0x7F,0x40,0x40,0x7F,0x40,0x40,0x40,0x40,0x40,0x7F,0x40,0x40,0x7F,0x00,0x00}; 
uint8_t lei1[]={//-- ??: ? -- 
//-- ?? 12; ??????????:? x ?=16x16 -- 
0x80,0x80,0x80,0xBF,0xA5,0xA5,0xA5,0x3F,0xA5,0xA5,0xA5,0xBF,0x80,0x80,0x80,0x00, 
0x7F,0x24,0x24,0x3F,0x24,0x24,0x7F,0x00,0x7F,0x24,0x24,0x3F,0x24,0x24,0x7F,0x00}; 

void LCD_PORT_init(void){
      SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);		   
      while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC));
      SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);		   
      while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK));     
      SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);		   
      while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));	
      SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);		   
      while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));


    GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2); 
	  GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_5|GPIO_PIN_4);
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_4|GPIO_PIN_5); 
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_4);
    GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_6);


     	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);		   
      while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));
	    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);
	    
}





//?? 
void LCD_delay(int n_ms) 
{ 
 int j,k; 
	TEST_L;
 for(j=0;j<n_ms;j++) 
 for(k=0;k<110;k++); 
	TEST_H;
} 
//??? 
void LCD_delay_us(int n_us) 
{ 
 int j,k; 
	TEST_L;
 for(j=0;j<n_us;j++) 
 for(k=0;k<1;k++); 
	TEST_H;
} 



//???? LCD ?? 
void transfer_command_lcd(int data1) 
{ 
 char i; 
 LCD_CS_L; 
 LCD_RS_L;
 for(i=0;i<8;i++) 
 { 
 LCD_SCK_L; 
 //delay_us(10); //????? 
 if(data1&0x80) LCD_SDA_H; 
 else LCD_SDA_L; 
 LCD_SCK_H; 
 //delay_us(10); //????? 
 data1=data1<<=1; 
 } 
 LCD_CS_H; 
} 
//???? LCD ?? 
void transfer_data_lcd(int data1) 
{ 
 char i;
 LCD_CS_L; 
 LCD_RS_H; 
 for(i=0;i<8;i++) 
 { 
 LCD_SCK_L; 
 if(data1&0x80) LCD_SDA_H; 
 else LCD_SDA_L; 
 LCD_SCK_H; 
 data1=data1<<=1; 
 } 
 LCD_CS_H; 
} 
//LCD ????? 
void initial_lcd() 
{ 
	LCD_PORT_init();
	
 LCD_RST_L; //????? 
 LCD_delay(100); 
 LCD_RST_H; //???? 
 LCD_delay(100); 
 transfer_command_lcd(0xe2); //??? 
 LCD_delay(5); 
 transfer_command_lcd(0x2c); //???? 1 
 LCD_delay(50); 
 transfer_command_lcd(0x2e); //???? 2 
 LCD_delay(50); 
 transfer_command_lcd(0x2f); //???? 3 
 LCD_delay(5); 
 transfer_command_lcd(0x23); //?????,????? 0x20~0x27 
 transfer_command_lcd(0x81); //????? 
 transfer_command_lcd(0x28); //???????,????? 0x00~0x3f 
 transfer_command_lcd(0xa2); //1/9 ???(bias) 
 transfer_command_lcd(0xc8); //?????:???? 
 transfer_command_lcd(0xa0); //?????:???? 
 transfer_command_lcd(0x40); //???:????? 
 transfer_command_lcd(0xaf); //??? 
} 
void lcd_address(uint32_t page,uint32_t column) 
{ 
 column=column-0x01; 
 transfer_command_lcd(0xb0+page-1); //?????,? 8 ????,??? 64 ?,??? 8 ? 
 transfer_command_lcd(0x10+(column>>4&0x0f)); //??????? 4 ? 
 transfer_command_lcd(column&0x0f); //??????? 4 ? 
} 
//???? 
void clear_screen() 
{ 
 unsigned char i,j; 
 
 
 for(i=0;i<9;i++) 
 { 
 transfer_command_lcd(0xb0+i); 
 transfer_command_lcd(0x10); 
 transfer_command_lcd(0x00); 
 for(j=0;j<132;j++) 
 { 
 transfer_data_lcd(0x00); 
 } 
 } 
}
//?? 128x64 ???? 
void display_128x64(uint8_t *dp) 
{ 
 uint32_t i,j; 
 
 for(j=0;j<8;j++) 
 { 
 lcd_address(j+1,1); 
 for (i=0;i<128;i++) 
 { 
 transfer_data_lcd(*dp); //???? LCD,????? 8 ??????????? 1 
 dp++; 
 } 
 } 
} 
//?? 16x16 ???????????? 16x16 ??????? 
void display_graphic_16x16(uint8_t page,uint8_t column,uint8_t *dp) 
{ 
 uint32_t i,j; 
 for(j=0;j<2;j++) 
 { 
 lcd_address(page+j,column); 
 for (i=0;i<16;i++) 
 { 
 transfer_data_lcd(*dp); //???? LCD,????? 8 ??????????? 1 
 dp++; 
 } 
 } 
} 
//?? 8x16 ?????ASCII, ? 8x16 ???????????? 
void display_graphic_8x16(uint8_t page,uint8_t column,uint8_t *dp) 
{ 
 uint32_t i,j; 
 for(j=0;j<2;j++) 
 { 
 lcd_address(page+j,column); 
 for (i=0;i<8;i++) 
 { 
 transfer_data_lcd(*dp); //???? LCD,????? 8 ??????????? 1 
 dp++; 
 } 
 } 
} 
//?? 5X8 ?????ASCII, ? 5x8 ???????????? 
void display_graphic_5x8(uint8_t page,uint8_t column,uint8_t *dp) 
{ 
 uint32_t i; 
 lcd_address(page,column); 
 for (i=0;i<6;i++) 
 { 
 transfer_data_lcd(*dp); 
 dp++; 
 } 
 
} 
//????????? IC 
void send_command_to_ROM( uint8_t datu ) 
{
uint8_t i; 
 for(i=0;i<8;i++ ) 
 { 
 LCD_ROM_SCK_L; 
 LCD_delay_us(10); 
 if(datu&0x80)LCD_ROM_IN_H; 
 else LCD_ROM_IN_L; 
 datu = datu<<1; 
 LCD_ROM_SCK_H; 
 LCD_delay_us(10); 
 } 
} 
//?????? IC ?????????(1 ???) 
static uint8_t get_data_from_ROM( ) 
{ 
 uint8_t i; 
 uint8_t ret_data=0; 
 for(i=0;i<8;i++) 
 {  
 LCD_ROM_SCK_L; 
 //delay_us(1); 
 ret_data=ret_data<<1; 
 if( LCD_ROM_OUT ) 
 ret_data=ret_data+1; 
 else 
 ret_data=ret_data+0; 
 LCD_ROM_SCK_H; 
 //delay_us(1); 
 } 
 return(ret_data); 
} 
//????????????????(page,column)??? 
void get_and_write_16x16(unsigned long fontaddr,uint8_t page,uint8_t column,uint8_t inverse) 
{ 
 uint8_t i,j,disp_data; 
 LCD_ROM_CS_L; 
 send_command_to_ROM(0x03); 
 send_command_to_ROM((fontaddr&0xff0000)>>16); //???? 8 ?,? 24 ? 
 send_command_to_ROM((fontaddr&0xff00)>>8); //???? 8 ?,? 24 ? 
 send_command_to_ROM(fontaddr&0xff); //???? 8 ?,? 24 ? 
 for(j=0;j<2;j++) 
 { 
 lcd_address(page+j,column); 
 for(i=0; i<16; i++ ) 
 {
 if(inverse==0)	 
 disp_data=get_data_from_ROM(); 
 else
	    disp_data=~(get_data_from_ROM()); 
 transfer_data_lcd(disp_data); //???? LCD,??? 1 ???????????? 1 
 } 
 } 
 LCD_ROM_CS_H; 
} 
//????????????????(page,column)??? 
void get_and_write_8x16(unsigned long fontaddr,uint8_t page,uint8_t column,uint8_t inverse)
{ 
 uint8_t i,j,disp_data; 
 LCD_ROM_CS_L; 
 send_command_to_ROM(0x03); 
 send_command_to_ROM((fontaddr&0xff0000)>>16); //???? 8 ?,? 24 ? 
 send_command_to_ROM((fontaddr&0xff00)>>8); //???? 8 ?,? 24 ? 
 send_command_to_ROM(fontaddr&0xff); //???? 8 ?,? 24 ? 
 for(j=0;j<2;j++)
 { 
 lcd_address(page+j,column); 
 for(i=0; i<8; i++ ) 
 { 
if(inverse==0)	 
 disp_data=get_data_from_ROM(); 
 else
	    disp_data=~(get_data_from_ROM());  
 transfer_data_lcd(disp_data); //???? LCD,??? 1 ???????????? 1 
 } 
 } 
 LCD_ROM_CS_H; 
} 
//????????????????(page,column)??? 
void get_and_write_5x8(unsigned long fontaddr,uint8_t page,uint8_t column,uint8_t inverse) 
{ 
 uint8_t i,disp_data; 
 LCD_ROM_CS_L; 
 send_command_to_ROM(0x03); 
 send_command_to_ROM((fontaddr&0xff0000)>>16); //???? 8 ?,? 24 ? 
 send_command_to_ROM((fontaddr&0xff00)>>8); //???? 8 ?,? 24 ? 
 send_command_to_ROM(fontaddr&0xff); //???? 8 ?,? 24 ? 
 lcd_address(page,column); 
 for(i=0; i<5; i++ ) 
 { 
if(inverse==0)	 
 disp_data=get_data_from_ROM(); 
 else
	    disp_data=~(get_data_from_ROM()); 
 transfer_data_lcd(disp_data); //???? LCD,??? 1 ???????????? 1 
 } 
 LCD_ROM_CS_H; 
} 
//**************************************************************** 
unsigned long fontaddr=0; 
void display_GB2312_string(uint8_t page,uint8_t column,uint8_t *text,uint8_t inverse)
{ 
 uint8_t i= 0; 
 while((text[i]>0x00)) 
 { 
 if(((text[i]>=0xb0) &&(text[i]<=0xf7))&&(text[i+1]>=0xa1)) 
 { 
 //????(GB2312)???????? IC ????????????: 
 //Address = ((MSB - 0xB0) * 94 + (LSB - 0xA1)+ 846)*32+ BaseAdd;BaseAdd=0 
 //???? 8 ???????????,???????? 
 fontaddr = (text[i]- 0xb0)*94; 
 fontaddr += (text[i+1]-0xa1)+846; 
 fontaddr = (unsigned long)(fontaddr*32); 
 get_and_write_16x16(fontaddr,page,column,inverse); //????????????????(page,column)??? 
 i+=2; 
 column+=16; 
 } 
 else if(((text[i]>=0xa1) &&(text[i]<=0xa3))&&(text[i+1]>=0xa1)) 
 { 
 //????(GB2312)15x16 ?????????? IC ????????????: 
 //Address = ((MSB - 0xa1) * 94 + (LSB - 0xA1))*32+ BaseAdd;BaseAdd=0 
 //???? 8 ???????????,???????? 
 fontaddr = (text[i]- 0xa1)*94; 
 fontaddr += (text[i+1]-0xa1); 
 fontaddr = (unsigned long)(fontaddr*32); 
 
 get_and_write_16x16(fontaddr,page,column,inverse); //????????????????(page,column)??? 
 i+=2; 
 column+=16; 
 } 
 else if((text[i]>=0x20) &&(text[i]<=0x7e))
 { 
 fontaddr = (text[i]- 0x20); 
 fontaddr = (unsigned long)(fontaddr*16); 
 fontaddr = (unsigned long)(fontaddr+0x3cf80); 
 get_and_write_8x16(fontaddr,page,column,inverse); //????????????????(page,column)??? 
 i+=1; 
 column+=8; 
 } 
 else 
 i++; 
 } 
 
} 
void display_string_5x8(uint8_t page,uint8_t column,uint8_t *text,uint8_t inverse) 
{ 
 unsigned char i= 0; 
 while((text[i]>0x00)) 
 { 
 
 if((text[i]>=0x20) &&(text[i]<=0x7e)) 
 { 
 fontaddr = (text[i]- 0x20); 
 fontaddr = (unsigned long)(fontaddr*8); 
 fontaddr = (unsigned long)(fontaddr+0x3bfc0); 
 get_and_write_5x8(fontaddr,page,column,inverse); //????????????????(page,column)??? 
 i+=1; 
 column+=6; 
 } 
 else 
 i++; 
 } 
} 






//===============main program=================== 











uint8_t bmp1[]={ 
//-- ???????:D:\????\My Pictures\12864-555.bmp -- 
//-- ?? x ??=128x64 -- 
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x3F,0x1F,0x0F,0x07,0x03,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x80,0x80,0xC0,0xC0,0xC0,0xC0,0x20,0x70,0x60,0xFE,0xFE,0xAE,0xFE,0xFE,0x00,0x00,0xF8,0xF8,0xB8,0xF8,0xD0,0x80,0xC0,0xE0,0xC0,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x03,0x07,0x0F,0x1F,0x3F,0x7F,0xFF,0xFF,0xFB,0xF1,0xF0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x98,0xB8,0xBE,0xBF,0xBF,0xBF,0xBF,0xBB,0xBB,0xBB,0xB9,0xB9,0x11,0x10,0xB8,0x90,0x00,0xFF,0xFF,0xFA,0xFF,0xFF,0x00,0x00,0xFF,0xFF,0xFA,0xFF,0x1D,0x01,0x07,0x1D,0x7F,0xFB,0xFE,0xF8,0xE0,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x07,0x0F,0x1F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x19,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x11,0x10,0x3B,0x11,0x00,0x3F,0x3F,0x3F,0x3F,0x3F,0x00,0x00,0x3F,0x3F,0x3F,0x3F,0x10,0x00,0x00,0x00,0x00,0x01,0x03,0x0F,0x1F,0x1F,0x1F,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x7C,0xFE,0xFF,0xFF,0x07,0x07,0xFF,0xFF,0xFF,0xFF,0x7F,0x3F,0x8F,0xC3,0xF0,0xFC,0x3F,0x0E,0xC2,0xF0,0xFC,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x7F,0x3F,0x3F,0x3F,0x9F,0xDF,0xFF,0xDF,0x9F,0x0F,0x0F,0x0F,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xE0,0xF8,0x7E,0x1F,0x07,0x03,0x00,0x08,0x19,0x39,0x79,0xF9,0xF9,0x99,0x99,0x99,0x99,0x99,0x99,0x9F,0x9F,0x9F,0x9F,0x9B,0x99,0x99,0xF9,0xF9,0x79,0x39,0x1F,0x0F,0x07,0x03,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0xE1,0xE1,0xE1,0x7F,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0xC0,0xF0,0xFC,0x7E,0x1F,0x07,0x01,0x00,0x00,0x80,0xC0,0xE0,0xF0,0xB8,0x3C,0x3E,0x3F,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x3B,0x3F,0x3E,0x3C,0xF8,0xF0,0xE0,0xC0,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF8,0x1C,0x1F,0x1C,0xF8,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x02,0x0F,0x3F,0x3F,0x7E,0x7E,0xFE,0xFF,0xFF,0xFB,0xF1,0xF0,0x60,0xE0,0xF0,0x10,0x10,0xD3,0xD3,0x93,0x13,0x13,0x93,0xD3,0xD3,0x53,0x53,0xD3,0xD3,0x93,0x13,0x13,0x93,0xD3,0x53,0x53,0x53,0xD3,0xD3,0x13,0x13,0x93,0xD3,0xD3,0x53,0x53,0x53,0xD3,0xD0,0x10,0x30,0xF0,0x00,0x00,0x00,0x00,0x00,0x03,0x06,0x04,0x06,0x07,0x00,0x00,0x80,0xE0,0xF0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xF8,0xF0,0xE0,0xC0,0x80,0x80,0x80,0x81,0x83,0x83,0x81,0x80,0xBF,0xFF,0xE0,0xE0,0xE0,0xEF,0xEF,0xE0,0xE0,0xEF,0xEF,0xEB,0xEA,0xEA,0xEB,0xEF,0xE5,0xE0,0xE0,0xEB,0xEB,0xEA,0xEA,0xEA,0xEF,0xEF,0xE0,0xE0,0xE7,0xEF,0xEB,0xEA,0xEA,0xEB,0xEE,0xEE,0xE0,0xE0,0xBF,0x80,0x80,0x80,0x80,0x80,0x80,0xC0,0xE0,0xF0,0xF8,0xFC,0xFF,0xFF,0x9F,0x9F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
};



