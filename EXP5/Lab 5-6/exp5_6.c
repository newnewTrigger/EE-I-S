//******************************************************************************************
//
// Copyright: 2020-2021, 上海交通大学电子工程系实验教学中心
// File name: LM75BD_Hardware_IIC.c
// Description: 本示例如何每隔1000ms读取温度传感器LM75BD的温度值  
//    1.左侧4个数码管显示第一个温度传感器LM75DB的值(ADDR:0x48)；第1位为符号位，正不显示，负显示-
//    2.右侧4个数码管显示第二个温度传感器LM75DB的值(ADDR:0x4F)；第1位为符号位，正不显示，负显示-
// Author:	上海交通大学电子工程系实验教学中心
// Version: 1.0.0.20210513 
// Date：2021-05-13
// History：
//
//******************************************************************************************

//******************************************************************************************
//
// 头文件
//
//******************************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"        // 基址宏定义
#include "inc/hw_types.h" 
#include "inc/hw_timer.h"         // ??????????
#include "inc/hw_ints.h"   
#include "driverlib/debug.h"      // 调试用
#include "driverlib/gpio.h"       // 通用IO口宏定义
#include "driverlib/pin_map.h"    // TM4C系列MCU外围设备管脚宏定义
#include "driverlib/sysctl.h"     // 系统控制定义
#include "driverlib/systick.h"    // SysTick Driver 原型
#include "driverlib/interrupt.h"  // NVIC Interrupt Controller Driver 原型
#include "driverlib/fpu.h"
#include "driverlib/timer.h"      // ?Timer???????  
#include "driverlib/pwm.h"   
#include "driverlib/uart.h"
#include "driverlib/adc.h"        


#include "JXL.c"
#include "tm1638.h"               // 与控制TM1638芯片有关的函数
#include "LM75BD.h"               // 与控制LM75BD芯片有关的函数
#include "PWM.h" 
#include "ADC.h"
//******************************************************************************************
//
// 宏定义
//
//******************************************************************************************
#define SYSTICK_FREQUENCY		50		// SysTick频率为50Hz，即循环定时周期20ms
#define  MilliSecond      4000 
#define V_T1s	50              // 1s软件定时器溢出值，50个20ms
#define V_T40ms	 2
//******************************************************************************************
//
// 函数原型声明
//
//******************************************************************************************
void GPIOInit(void);        // GPIO初始化
void SysTickInit(void);     // 设置SysTick中断 
void DevicesInit(void);     // MCU器件初始化，注：会调用上述函数
void UARTInit(void);   
void UARTStringPut(uint32_t ui32Base,const char *cMessage);
void  DelayMilliSec(uint32_t ui32DelaySecond);
void disp_number(uint32_t page,uint32_t column,uint8_t number,uint8_t inverse);
void refresh_temp_in(void);
void refresh_volt_in(void);
void refresh_temp_lcd(void);
void refresh_volt_lcd(void);
void trans_temp(void);
void play(void);

//******************************************************************************************
//
// 变量定义
//
//******************************************************************************************

uint8_t clock40ms = 0;
uint8_t clock40ms_flag = 0;




// 软件定时器计数
uint8_t clock1s = 0;

// 软件定时器溢出标志
uint8_t clock1s_flag = 0;

// 测试用计数器
uint32_t test_counter = 0;

// 8位数码管显示的数字或字母符号
// 注：板上数码位从左到右序号排列为4、5、6、7、0、1、2、3
uint8_t digit[8]={' ',' ',' ',' ',' ',' ',' ',' '};

// 8位小数点 1亮  0灭
// 注：板上数码位小数点从左到右序号排列为4、5、6、7、0、1、2、3
uint8_t pnt = 0x44;

// 8个LED指示灯状态，0灭，1亮
// 注：板上指示灯从左到右序号排列为7、6、5、4、3、2、1、0
//     对应元件LED8、LED7、LED6、LED5、LED4、LED3、LED2、LED1
uint8_t led[] = {1, 1, 1, 1, 1, 1, 1, 0};

uint8_t temp1[]={0,0,0,0};
uint8_t temp2[]={0,0,0,0};
uint8_t volt[]={0,0,0,0};
uint32_t freq_square=0;
uint8_t freq_lcd[]={1,0,1,0};
uint8_t freq_lcd_random[]={1,0,1,0};


// 当前按键值
uint8_t key_code = 0;
uint8_t key_cnt = 0;


uint8_t prestate=0;
uint8_t curstate=0;
uint8_t key=0;



uint8_t actmode=0;
uint8_t freqmode=1;

uint32_t range_flag=0;

// AIN2(PE1)  ADC???[0-4095]
uint32_t ui32ADC0Value;     

// AIN2???(???0.01V) [0.00-3.30]
uint32_t ui32ADC0Voltage; 


// 系统时钟频率 
uint32_t ui32SysClock;
uint32_t g_ui32SysClock;

// 返回温度值 1LSB=0.1℃
int16_t i16Temperature1;      
int16_t i16Temperature2;      

uint8_t ui8DigitRefresh = 0;


uint16_t ui16Temp1,ui16Temp2;

// 系统当前功能状态
bool system_state = true;

//是否暂停
bool non_stop = true;

struct Note {
    int frequency;  // 频率，单位为Hz
    int duration;   // 持续时间，单位为20ms周期数
};

struct Note melody[] = {
	{440, 45}, // 6. ·，频率为440Hz，持续时间为900ms
	{659, 15}, // 3 _ ，频率为659Hz，持续时间为300ms
	{587, 15}, // 2 _ ，频率为587Hz，持续时间为300ms
	{659, 15}, // 3 _ ，频率为659Hz，持续时间为300ms
	{523, 15}, // 1 _ ，频率为523Hz，持续时间为300ms
	{494, 15}, // 7. _ ，频率为494Hz，持续时间为300ms
	
	{440, 120}, // 6. - - - ，频率为523Hz，持续时间为2400ms
	
	{659, 15}, // 3 _
	{880, 30}, // 6
	{784, 15}, // 5 _
	{659, 15}, // 3 _
	{587, 15}, // 2 _
	{523, 15}, // 1 _
	{587, 15}, // 2 _
	
	{659, 90}, // 3 - -
	{659, 15}, // 3 _
	{784, 15}, // 5 _
	
	{880, 45}, // 6 ·
	{988, 15}, // 7 _
	{880, 15}, // 6 _
	{784, 15}, // 5 _
	{659, 15}, // 3 _
	{587, 15}, // 2 _
	
	{523, 15},
	{523, 30},
	{587, 15},
	{659, 60},

	{587, 45},//
	{659, 15},
	{494, 15},
	{440, 15},
	{392, 30},//

	{440, 120},//6. - - -
    // 在此添加更多音符...
};

volatile int currentNote = 0;     // 当前播放的音符索引
volatile int currentDuration = 0; // 当前音符已经播放的时间
//******************************************************************************************
//
// 主程序
//
//******************************************************************************************
 int main(void)
{

    DevicesInit();            //  MCU器件初始化
	
	SysCtlDelay(60 * ( ui32SysClock / 3000));   // 延时>60ms,等待TM1638上电完成
    TM1638_Init();	          // 初始化TM1638
	initial_lcd();

		UARTStringPut(UART6_BASE, (const char *)"AT+FRE=1010\r\n");
		UARTStringPut(UART6_BASE, (const char *)"AT+VOL=30\r\n");
		
		
		i16Temperature1 = GetTemputerature(LM75BD_ADR2);               
		i16Temperature2 = GetTemputerature(LM75BD_ADR1);  
					if(i16Temperature1 < 0)
				{
					ui16Temp1 = -i16Temperature1;
						temp1[0]=1;
				}    
				else
				{
					ui16Temp1 = i16Temperature1;
					temp1[0]=0;
				}	 

							
				if(i16Temperature2 < 0)
				{
					ui16Temp2 = -i16Temperature2;
					temp2[0]=1;
				}    
				else
				{
					ui16Temp2 = i16Temperature2;
					temp2[0]=0;
				}	
		
			temp1[1] = ui16Temp1 / 100;      	// 计算十位数
				temp1[2] = ui16Temp1 / 10 % 10; 	// 计算个位数
				temp1[3] = ui16Temp1 % 10;
		
			temp2[1] = ui16Temp2 / 100;      	// 计算十位数
				temp2[2] = ui16Temp2 / 10 % 10; 	// 计算个位数
				temp2[3] = ui16Temp2 % 10;
		
			
				ui32ADC0Value = ADC_Sample();  
				ui32ADC0Voltage = ui32ADC0Value * 3300 / 4095;
				
				volt[0] = (ui32ADC0Voltage / 1000) % 10; // ????????
				volt[1] = (ui32ADC0Voltage / 100) % 10;  // ?????????
				volt[2] = (ui32ADC0Voltage/10) % 10;         // ?????????        		
					volt[3] = ui32ADC0Voltage%10;
							
							clear_screen();
							display_GB2312_string(1,1,"电压:",0);
							display_GB2312_string(3,1,"温度:",0);
							disp_number(1,41,volt[0],0);
							display_GB2312_string(1,49,".",0);
							disp_number(1,57,volt[1],0);
							disp_number(1,65,volt[2],0);
				disp_number(1,73,volt[3],0);
							display_GB2312_string(1,81,"V",0);
							
							
							if(temp1[0]==1){display_GB2312_string(3,41,"-",0);}
							else{display_GB2312_string(3,41," ",0);}
							disp_number(3,49,temp1[1],0);
				disp_number(3,57,temp1[2],0);
							display_GB2312_string(3,65,".",0);
							disp_number(3,73,temp1[3],0);
							display_GB2312_string(3,89,"℃",0);
							
				if(temp2[0]==1){display_GB2312_string(5,41,"-",0);}
							else{display_GB2312_string(5,41," ",0);}
							disp_number(5,49,temp2[1],0);
				disp_number(5,57,temp2[2],0);
							display_GB2312_string(5,65,".",0);
							disp_number(5,73,temp2[3],0);
							display_GB2312_string(5,89,"℃",0);
							
							
		
		while (1)
		{
		if(system_state){}
		else {
			if (clock1s_flag == 1)      // ??0.1??????
				{
					clock1s_flag=0;
				refresh_temp_in();
					trans_temp();
				if(actmode ==0) {refresh_temp_lcd();}
				}
			
			if (clock40ms_flag == 1)      // ??0.1??????
				{
					clock40ms_flag=0;
				refresh_volt_in();
				if(actmode ==0) {refresh_volt_lcd();}
				}
				
				
				if(key==0){
					key=0;
				}
				else if(key==1){
				key=0;
					if(actmode ==0){
					actmode =1;
						clear_screen();
						display_GB2312_string(1,1,"载频:",0);
						if(freq_lcd[0]==0){freqmode =2;display_GB2312_string(1,41," ",0);}
							else{freqmode =1;disp_number(1,41,freq_lcd[0],1);}
							
						if(freq_lcd[0]==0){disp_number(1,49,freq_lcd[1],1);}
						else{disp_number(1,49,freq_lcd[1],0);}
						disp_number(1,57,freq_lcd[2],0);	
						display_GB2312_string(1,65,".",0);	
						disp_number(1,73,freq_lcd[3],0);
						display_GB2312_string(1,81,"MHz",0);
						display_GB2312_string(7,97,"确定",0);	
					}
				}
				else if(key==2){
				key=0;
					if(actmode ==0){
					actmode =1;
						clear_screen();
						display_GB2312_string(1,1,"载频:",0);
						if(freq_lcd[0]==0){freqmode =2;display_GB2312_string(1,41," ",0);}
							else{freqmode =1;disp_number(1,41,freq_lcd[0],1);}
							
						if(freq_lcd[0]==0){disp_number(1,49,freq_lcd[1],1);}
						else{disp_number(1,49,freq_lcd[1],0);}
						disp_number(1,57,freq_lcd[2],0);	
						display_GB2312_string(1,65,".",0);	
						disp_number(1,73,freq_lcd[3],0);
						display_GB2312_string(1,81,"MHz",0);
						display_GB2312_string(7,97,"确定",0);	
					}
			else if(actmode ==1){
					if(freqmode ==1){
							if(freq_lcd_random[0]!=9){
						++freq_lcd_random[0];
							disp_number(1,41,freq_lcd_random[0],1);
							}
							else{
							freq_lcd_random[0]=0;
								disp_number(1,41,freq_lcd_random[0],1);
							}
						}
						else if(freqmode ==2){
						if(freq_lcd_random[1]==9){
						freq_lcd_random[1]=0;
							disp_number(1,49,freq_lcd_random[1],1);
								if(freq_lcd_random[0]==9){
										freq_lcd_random[0]=0;
											disp_number(1,41,freq_lcd_random[0],0);
										}
										else{
										++freq_lcd_random[0];
											disp_number(1,41,freq_lcd_random[0],0);
										}
											
						}
						else{
						++freq_lcd_random[1];
							disp_number(1,49,freq_lcd_random[1],1);
						}
						}
						else if(freqmode ==3){
						if(freq_lcd_random[2]==9){
								freq_lcd_random[2]=0;
									disp_number(1,57,freq_lcd_random[2],1);
									if(freq_lcd_random[1]==9){
											freq_lcd_random[1]=0;
											disp_number(1,49,freq_lcd_random[1],0);
											if(freq_lcd_random[0]==9){
												freq_lcd_random[0]=0;
													disp_number(1,41,freq_lcd_random[0],0);
												}
												else{
												++freq_lcd_random[0];
													disp_number(1,41,freq_lcd_random[0],0);
												}
										}
										else{
											++freq_lcd_random[1];
											disp_number(1,49,freq_lcd_random[1],0);
										}
							}
							else{
								++freq_lcd_random[2];
									disp_number(1,57,freq_lcd_random[2],1);
							}
						}
						else if(freqmode ==5){
						if(freq_lcd_random[3]==9){
								freq_lcd_random[3]=0;
								disp_number(1,73,freq_lcd_random[3],1);
								if(freq_lcd_random[2]==9){
									freq_lcd_random[2]=0;
										disp_number(1,57,freq_lcd_random[2],0);
										if(freq_lcd_random[1]==9){
											freq_lcd_random[1]=0;
											disp_number(1,49,freq_lcd_random[1],0);
											if(freq_lcd_random[0]==9){
												freq_lcd_random[0]=0;
													disp_number(1,41,freq_lcd_random[0],0);
												}
												else{
												++freq_lcd_random[0];
													disp_number(1,41,freq_lcd_random[0],0);
												}								    
										}
										else{
											++freq_lcd_random[1];
											disp_number(1,49,freq_lcd_random[1],0);
										}
									}
									else{
									++freq_lcd_random[2];
										disp_number(1,57,freq_lcd_random[2],0);
									}
							}
							else{
								++freq_lcd_random[3];
								disp_number(1,73,freq_lcd_random[3],1);
							}
						}
						else if(freqmode ==7){
						
						}
				}			
				}
				else if(key==3){
				key=0;
					if(actmode ==0){
					actmode =1;
						clear_screen();
						display_GB2312_string(1,1,"载频:",0);
						if(freq_lcd[0]==0){freqmode =2;display_GB2312_string(1,41," ",0);}
							else{freqmode =1;disp_number(1,41,freq_lcd[0],1);}
							
						if(freq_lcd[0]==0){disp_number(1,49,freq_lcd[1],1);}
						else{disp_number(1,49,freq_lcd[1],0);}
						disp_number(1,57,freq_lcd[2],0);	
						display_GB2312_string(1,65,".",0);	
						disp_number(1,73,freq_lcd[3],0);
						display_GB2312_string(1,81,"MHz",0);
						display_GB2312_string(7,97,"确定",0);	
					}			
				}
				else if(key==4){
				key=0;
					if(actmode ==0){
					actmode =1;
						clear_screen();
						display_GB2312_string(1,1,"载频:",0);
						if(freq_lcd[0]==0){freqmode =2;display_GB2312_string(1,41," ",0);}
							else{freqmode =1;disp_number(1,41,freq_lcd[0],1);}
							
						if(freq_lcd[0]==0){disp_number(1,49,freq_lcd[1],1);}
						else{disp_number(1,49,freq_lcd[1],0);}
						disp_number(1,57,freq_lcd[2],0);	
						display_GB2312_string(1,65,".",0);	
						disp_number(1,73,freq_lcd[3],0);
						display_GB2312_string(1,81,"MHz",0);
						display_GB2312_string(7,97,"确定",0);	
					}
		else if(actmode ==1){
				if(freqmode ==1){
						freqmode =7;
							disp_number(1,41,freq_lcd_random[0],0);
							display_GB2312_string(7,97,"确定",1);
						}
						else if(freqmode ==2){
				if(freq_lcd_random[0]==0){
							freqmode =7;
								display_GB2312_string(7,97,"确定",1);
								disp_number(1,49,freq_lcd_random[1],0);
							}
							else {
							freqmode =1;
								disp_number(1,49,freq_lcd_random[1],0);
								disp_number(1,41,freq_lcd_random[0],1);
							}
						}
						else if(freqmode ==3){
						freqmode =2;
				disp_number(1,49,freq_lcd_random[1],1);
							disp_number(1,57,freq_lcd_random[2],0);
						}
						else if(freqmode ==5){
						freqmode =3;
								disp_number(1,57,freq_lcd_random[2],1);
							disp_number(1,73,freq_lcd_random[3],0);
						}
						else if(freqmode ==7){
						freqmode =5;
							display_GB2312_string(7,97,"确定",0);
							disp_number(1,73,freq_lcd_random[3],1);
						}
			}			
				}
				else if(key==5){
				key=0;
					if(actmode ==0){
					actmode =1;
						clear_screen();
						display_GB2312_string(1,1,"载频:",0);
						if(freq_lcd[0]==0){freqmode =2;display_GB2312_string(1,41," ",0);}
							else{freqmode =1;disp_number(1,41,freq_lcd[0],1);}
							
						if(freq_lcd[0]==0){disp_number(1,49,freq_lcd[1],1);}
						else{disp_number(1,49,freq_lcd[1],0);}
						disp_number(1,57,freq_lcd[2],0);	
						display_GB2312_string(1,65,".",0);	
						disp_number(1,73,freq_lcd[3],0);
						display_GB2312_string(1,81,"MHz",0);
						display_GB2312_string(7,97,"确定",0);	
					}
			else if(actmode ==1){
					if(freqmode ==7){
							range_flag =freq_lcd_random[0]*1000+freq_lcd_random[1]*100+freq_lcd_random[2]*10+freq_lcd_random[3];
							if(range_flag>=880&&range_flag<=1080){
								freq_lcd[0]=freq_lcd_random[0];
								freq_lcd[1]=freq_lcd_random[1];
									freq_lcd[2]=freq_lcd_random[2];
									freq_lcd[3]=freq_lcd_random[3];
									
									UARTCharPut(UART6_BASE,'A');
									UARTCharPut(UART6_BASE,'T');
									UARTCharPut(UART6_BASE,'+');
									UARTCharPut(UART6_BASE,'F');
									UARTCharPut(UART6_BASE,'R');
									UARTCharPut(UART6_BASE,'E');
									UARTCharPut(UART6_BASE,'=');
									if(freq_lcd[0]==0){
										UARTCharPut(UART6_BASE,'0'+freq_lcd[1]);
										UARTCharPut(UART6_BASE,'0'+freq_lcd[2]);
										UARTCharPut(UART6_BASE,'0'+freq_lcd[3]);
										UARTCharPut(UART6_BASE,'\r');
										UARTCharPut(UART6_BASE,'\n');
									}
									else{
									UARTCharPut(UART6_BASE,'0'+freq_lcd[0]);
										UARTCharPut(UART6_BASE,'0'+freq_lcd[1]);
										UARTCharPut(UART6_BASE,'0'+freq_lcd[2]);
										UARTCharPut(UART6_BASE,'0'+freq_lcd[3]);
										UARTCharPut(UART6_BASE,'\r');
										UARTCharPut(UART6_BASE,'\n');
									}
									
								actmode =0;
								clear_screen();
								display_GB2312_string(1,1,"电压:",0);
								display_GB2312_string(3,1,"温度:",0);
								disp_number(1,41,volt[0],0);
								display_GB2312_string(1,49,".",0);
								disp_number(1,57,volt[1],0);
								disp_number(1,65,volt[2],0);
					disp_number(1,73,volt[3],0);
								display_GB2312_string(1,81,"V",0);
								
								
								if(temp1[0]==1){display_GB2312_string(3,41,"-",0);}
								else{display_GB2312_string(3,41," ",0);}
								disp_number(3,49,temp1[1],0);
					disp_number(3,57,temp1[2],0);
								display_GB2312_string(3,65,".",0);
								disp_number(3,73,temp1[3],0);
								display_GB2312_string(3,89,"℃",0);
								
					if(temp2[0]==1){display_GB2312_string(5,41,"-",0);}
								else{display_GB2312_string(5,41," ",0);}
								disp_number(5,49,temp2[1],0);
					disp_number(5,57,temp2[2],0);
								display_GB2312_string(5,65,".",0);
								disp_number(5,73,temp2[3],0);
								display_GB2312_string(5,89,"℃",0);							 
									
									
								}
								else{
								freq_lcd_random[0]=freq_lcd[0];
								freq_lcd_random[1]=freq_lcd[1];
								freq_lcd_random[2]=freq_lcd[2];
								freq_lcd_random[3]=freq_lcd[3];
									
								clear_screen();
									display_GB2312_string(1,1,"载频超范围！",0);
									LCD_delay(81000);
									clear_screen();
									actmode =1;
						display_GB2312_string(1,1,"载频:",0);
						if(freq_lcd[0]==0){freqmode =2;display_GB2312_string(1,41," ",0);}
							else{freqmode =1;disp_number(1,41,freq_lcd[0],1);}
							
						if(freq_lcd[0]==0){disp_number(1,49,freq_lcd[1],1);}
						else{disp_number(1,49,freq_lcd[1],0);}
						disp_number(1,57,freq_lcd[2],0);	
						display_GB2312_string(1,65,".",0);	
						disp_number(1,73,freq_lcd[3],0);
						display_GB2312_string(1,81,"MHz",0);
						display_GB2312_string(7,97,"确定",0);								 
									
								}
						}
				}			
				}
				else if(key==6){
				key=0;
					if(actmode ==0){
					actmode =1;
						clear_screen();
						display_GB2312_string(1,1,"载频:",0);
						if(freq_lcd[0]==0){freqmode =2;display_GB2312_string(1,41," ",0);}
							else{freqmode =1;disp_number(1,41,freq_lcd[0],1);}
							
						if(freq_lcd[0]==0){disp_number(1,49,freq_lcd[1],1);}
						else{disp_number(1,49,freq_lcd[1],0);}
						disp_number(1,57,freq_lcd[2],0);	
						display_GB2312_string(1,65,".",0);	
						disp_number(1,73,freq_lcd[3],0);
						display_GB2312_string(1,81,"MHz",0);
						display_GB2312_string(7,97,"确定",0);	
					}
		else if(actmode ==1){
				if(freqmode ==1){
						freqmode =2;
							disp_number(1,41,freq_lcd_random[0],0);
							disp_number(1,49,freq_lcd_random[1],1);
						}
						else if(freqmode ==2){
						freqmode =3;
							disp_number(1,49,freq_lcd_random[1],0);
							disp_number(1,57,freq_lcd_random[2],1);
						}
						else if(freqmode ==3){
						freqmode =5;
							disp_number(1,57,freq_lcd_random[2],0);
							disp_number(1,73,freq_lcd_random[3],1);
						}
						else if(freqmode ==5){
						freqmode =7;
							disp_number(1,73,freq_lcd_random[3],0);
							display_GB2312_string(7,97,"确定",1);	
						}
						else if(freqmode ==7){
						if(freq_lcd_random[0]==0){
							freqmode =2;
								display_GB2312_string(7,97,"确定",0);
								disp_number(1,49,freq_lcd_random[1],1);
							}
							else {
								freqmode =1;
								display_GB2312_string(7,97,"确定",0);
								disp_number(1,41,freq_lcd_random[0],1);
							}
						}
			}			
				}
				else if(key==7){
				key=0;
					if(actmode ==0){
					actmode =1;
						clear_screen();
						display_GB2312_string(1,1,"载频:",0);
						if(freq_lcd[0]==0){freqmode =2;display_GB2312_string(1,41," ",0);}
							else{freqmode =1;disp_number(1,41,freq_lcd[0],1);}
							
						if(freq_lcd[0]==0){disp_number(1,49,freq_lcd[1],1);}
						else{disp_number(1,49,freq_lcd[1],0);}
						disp_number(1,57,freq_lcd[2],0);	
						display_GB2312_string(1,65,".",0);	
						disp_number(1,73,freq_lcd[3],0);
						display_GB2312_string(1,81,"MHz",0);
						display_GB2312_string(7,97,"确定",0);	
					}			
				}
				else if(key==8){
				key=0;
					if(actmode ==0){
					actmode =1;
						clear_screen();
						display_GB2312_string(1,1,"载频:",0);
						if(freq_lcd[0]==0){freqmode =2;display_GB2312_string(1,41," ",0);}
							else{freqmode =1;disp_number(1,41,freq_lcd[0],1);}
							
						if(freq_lcd[0]==0){disp_number(1,49,freq_lcd[1],1);}
						else{disp_number(1,49,freq_lcd[1],0);}
						disp_number(1,57,freq_lcd[2],0);	
						display_GB2312_string(1,65,".",0);	
						disp_number(1,73,freq_lcd[3],0);
						display_GB2312_string(1,81,"MHz",0);
						display_GB2312_string(7,97,"确定",0);	
					}
			else if(actmode ==1){
						if(freqmode ==1){
							if(freq_lcd_random[0]==0){
									freq_lcd_random[0]=9;
									disp_number(1,41,freq_lcd_random[0],1);
								}
								else{
									--freq_lcd_random[0];
									disp_number(1,41,freq_lcd_random[0],1);
								}
							}
							else if(freqmode ==2){
							if(freq_lcd_random[1]==0){
									freq_lcd_random[1]=9;
									disp_number(1,49,freq_lcd_random[1],1);
							if(freq_lcd_random[0]==0){
									freq_lcd_random[0]=9;
									disp_number(1,41,freq_lcd_random[0],0);
								}
								else{
									--freq_lcd_random[0];
									disp_number(1,41,freq_lcd_random[0],0);
								}							  
								}
								else{
									--freq_lcd_random[1];
									disp_number(1,49,freq_lcd_random[1],1);
								}
							}
							else if(freqmode ==3){
							if(freq_lcd_random[2]==0){
									freq_lcd_random[2]=9;
									disp_number(1,57,freq_lcd_random[2],1);
							if(freq_lcd_random[1]==0){
									freq_lcd_random[1]=9;
									disp_number(1,49,freq_lcd_random[1],0);
							if(freq_lcd_random[0]==0){
									freq_lcd_random[0]=9;
									disp_number(1,41,freq_lcd_random[0],0);
								}
								else{
									--freq_lcd_random[0];
									disp_number(1,41,freq_lcd_random[0],0);
								}							  
								}
								else{
									--freq_lcd_random[1];
									disp_number(1,49,freq_lcd_random[1],0);
								}							  
								}
								else{
									--freq_lcd_random[2];
									disp_number(1,57,freq_lcd_random[2],1);
								}
							}
							else if(freqmode ==5){
							if(freq_lcd_random[3]==0){
									freq_lcd_random[3]=9;
									disp_number(1,73,freq_lcd_random[3],1);
							if(freq_lcd_random[2]==0){
									freq_lcd_random[2]=9;
									disp_number(1,57,freq_lcd_random[2],0);
							if(freq_lcd_random[1]==0){
									freq_lcd_random[1]=9;
									disp_number(1,49,freq_lcd_random[1],0);
							if(freq_lcd_random[0]==0){
									freq_lcd_random[0]=9;
									disp_number(1,41,freq_lcd_random[0],0);
								}
								else{
									--freq_lcd_random[0];
									disp_number(1,41,freq_lcd_random[0],0);
								}							  
								}
								else{
									--freq_lcd_random[1];
									disp_number(1,49,freq_lcd_random[1],0);
								}							  
								}
								else{
									--freq_lcd_random[2];
									disp_number(1,57,freq_lcd_random[2],0);
								}							  
								}
								else{
									--freq_lcd_random[3];
									disp_number(1,73,freq_lcd_random[3],1);
								}
							}
							else if(freqmode ==7){
							
							}
					}			
				}
				else if(key==9){
				key=0;
					if(actmode ==0){
					actmode =1;
						clear_screen();
						display_GB2312_string(1,1,"载频:",0);
						if(freq_lcd[0]==0){freqmode =2;display_GB2312_string(1,41," ",0);}
							else{freqmode =1;disp_number(1,41,freq_lcd[0],1);}
							
						if(freq_lcd[0]==0){disp_number(1,49,freq_lcd[1],1);}
						else{disp_number(1,49,freq_lcd[1],0);}
						disp_number(1,57,freq_lcd[2],0);	
						display_GB2312_string(1,65,".",0);	
						disp_number(1,73,freq_lcd[3],0);
						display_GB2312_string(1,81,"MHz",0);
						display_GB2312_string(7,97,"确定",0);	
					}			
				}
			}	
		}
	
}


void trans_temp(void)  //使用第二路温度值
{
      freq_square =92.5*(temp2[1]*10+temp2[2]+0.1*temp2[3])+300;
	    PWMStart2(freq_square);
}



void refresh_volt_lcd(void)
{
            disp_number(1,41,volt[0],0);
						disp_number(1,57,volt[1],0);
						disp_number(1,65,volt[2],0);
	          disp_number(1,73,volt[3],0);
}






void refresh_temp_lcd(void)
{
            if(temp1[0]==1){display_GB2312_string(3,41,"-",0);}
						else{display_GB2312_string(3,41," ",0);}
						disp_number(3,49,temp1[1],0);
	          disp_number(3,57,temp1[2],0);
						disp_number(3,73,temp1[3],0);
						
	          if(temp2[0]==1){display_GB2312_string(5,41,"-",0);}
						else{display_GB2312_string(5,41," ",0);}
						disp_number(5,49,temp2[1],0);
	          disp_number(5,57,temp2[2],0);
						disp_number(5,73,temp2[3],0);
}







void DelayMilliSec(uint32_t ui32DelaySecond)  
{
    uint32_t ui32Loop;
	
    ui32DelaySecond = ui32DelaySecond * MilliSecond;
    for(ui32Loop = 0; ui32Loop < ui32DelaySecond; ui32Loop++){ };
}


void disp_number(uint32_t page,uint32_t column,uint8_t number,uint8_t inverse){
      if(number ==0){display_GB2312_string(page,column,"0",inverse);}
			if(number ==1){display_GB2312_string(page,column,"1",inverse);}
			if(number ==2){display_GB2312_string(page,column,"2",inverse);}
			if(number ==3){display_GB2312_string(page,column,"3",inverse);}
			if(number ==4){display_GB2312_string(page,column,"4",inverse);}
			if(number ==5){display_GB2312_string(page,column,"5",inverse);}
			if(number ==6){display_GB2312_string(page,column,"6",inverse);}
			if(number ==7){display_GB2312_string(page,column,"7",inverse);}
			if(number ==8){display_GB2312_string(page,column,"8",inverse);}
			if(number ==9){display_GB2312_string(page,column,"9",inverse);}
}

void refresh_temp_in(void){
         i16Temperature1 = GetTemputerature(LM75BD_ADR2);               
       i16Temperature2 = GetTemputerature(LM75BD_ADR1);  
			      if(i16Temperature1 < 0)
            {
                ui16Temp1 = -i16Temperature1;
                    temp1[0]=1;
            }    
            else
            {
                ui16Temp1 = i16Temperature1;
                temp1[0]=0;
            }	 

						
            if(i16Temperature2 < 0)
            {
                ui16Temp2 = -i16Temperature2;
                temp2[0]=1;
            }    
            else
            {
                ui16Temp2 = i16Temperature2;
                temp2[0]=0;
            }	
	
	    temp1[1] = ui16Temp1 / 100;      	// 计算十位数
			temp1[2] = ui16Temp1 / 10 % 10; 	// 计算个位数
			temp1[3] = ui16Temp1 % 10;
	
	    temp2[1] = ui16Temp2 / 100;      	// 计算十位数
			temp2[2] = ui16Temp2 / 10 % 10; 	// 计算个位数
			temp2[3] = ui16Temp2 % 10;
}


void refresh_volt_in(void){
      ui32ADC0Value= (ADC_Sample()+ADC_Sample()+ADC_Sample()+ADC_Sample()+ADC_Sample()+ADC_Sample()+ADC_Sample()+ADC_Sample()+ADC_Sample()+ADC_Sample()+ADC_Sample()+ADC_Sample()+ADC_Sample()+ADC_Sample()+ADC_Sample())/15;
			 ui32ADC0Voltage = ui32ADC0Value * 3300 / 4095;
            
            volt[0] = (ui32ADC0Voltage / 1000) % 10; // ????????
            volt[1] = (ui32ADC0Voltage / 100) % 10;  // ?????????
            volt[2] = (ui32ADC0Voltage/10) % 10;         // ?????????        		
			      volt[3] = ui32ADC0Voltage%10;
}



//******************************************************************************************
//
// 函数原型：void GPIOInit(void)
// 函数功能：GPIO初始化。
//     使能PortK，设置PK4,PK5为输出；使能PortM，设置PM0为输出；
//     PK4连接TM1638的STB，PK5连接TM1638的DIO，PM0连接TM1638的CLK；
//     使能PortL，设置LPL0,PL1为输出；PL0和PL1分别连接DAC6571的SDA和SCL。
// 函数参数：无
// 函数返回值：无
//
//******************************************************************************************
void GPIOInit(void)
{
	//配置TM1638芯片管脚
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);				// 使能端口 K	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK)){};		// 等待端口 K 准备完毕		
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);				// 使能端口 M	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM)){};		// 等待端口 M 准备完毕	

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);				// 使能端口 M	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)){};		// 等待端口 M 准备完毕		
	
    // 设置端口 K 的第4,5位（PK4,PK5）为输出引脚		PK4-STB  PK5-DIO
	GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_4|GPIO_PIN_5);
	// 设置端口 M 的第0位（PM0）为输出引脚   PM0-CLK
	GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_0);	

	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);
                
}

void UARTStringPut(uint32_t ui32Base,const char *cMessage)
{
	while(*cMessage != '\0') {
		UARTCharPut(ui32Base, *(cMessage++));
	  
	}
	DelayMilliSec(1);
}



void UARTInit(void)
{
    // ????
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);        // ??UART0??   
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);        // ???? A
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));	// ???? A???? 

    GPIOPinConfigure(GPIO_PA0_U0RX);				  // ??PA0?UART0 RX??
    GPIOPinConfigure(GPIO_PA1_U0TX);    			  // ??PA1?UART0 TX??        

	// ???? A??0,1?(PA0,PA1)?UART??
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);   

    // ?????????
    UARTConfigSetExpClk(UART0_BASE, 
	                    ui32SysClock,
	                    38400,                  // ???:115200
	                    (UART_CONFIG_WLEN_8 |    // ???:8
	                     UART_CONFIG_STOP_ONE |  // ???:1
	                     UART_CONFIG_PAR_NONE)); // ???:?
	
    
	
	    // ????
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART6);        // ??UART0??   
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);        // ???? A
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOP));	// ???? A???? 



    GPIOPinConfigure(GPIO_PP0_U6RX);				  // ??PA0?UART0 RX??
    GPIOPinConfigure(GPIO_PP1_U6TX);    			  // ??PA1?UART0 TX??        

  	// ???? A??0,1?(PA0,PA1)?UART??
    GPIOPinTypeUART(GPIO_PORTP_BASE, GPIO_PIN_0 | GPIO_PIN_1); 

    // ?????????
    UARTConfigSetExpClk(UART6_BASE, 
	                    ui32SysClock,
	                    38400,                  // ???:115200
	                    (UART_CONFIG_WLEN_8 |    // ???:8
	                     UART_CONFIG_STOP_ONE |  // ???:1
	                     UART_CONFIG_PAR_NONE)); // ???:?
	
	
}

//******************************************************************************************
// 
// 函数原型：SysTickInit(void)
// 函数功能：设置SysTick中断
// 函数参数：无
// 函数返回值：无
//
//******************************************************************************************
void SysTickInit(void)
{
	SysTickPeriodSet(ui32SysClock/SYSTICK_FREQUENCY); // 设置心跳节拍,定时周期20ms
	SysTickEnable();  			// SysTick使能
	SysTickIntEnable();			// SysTick中断允许
}

//******************************************************************************************
// 
// 函数原型：void DevicesInit(void)
// 函数功能：MCU器件初始化，包括系统时钟设置、GPIO初始化和SysTick中断设置
// 函数参数：无
// 函数返回值：无
//
//******************************************************************************************
void DevicesInit(void)
{
	// 使用外部25MHz主时钟源，经过PLL，然后分频为16MHz
	ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN | 
	                                   SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 
	                                   16000000);
	g_ui32SysClock=ui32SysClock;

    FPULazyStackingEnable(); 
    FPUEnable(); 
   
	GPIOInit();             // GPIO初始化
    I2C0Init();             // I2C0初始化
    SysTickInit();          // 设置SysTick中断
	  UARTInit(); 
    IntMasterEnable();		// 总中断允许
	 ADCInit(); 
	 PWMInit();  
}

//******************************************************************************************
// 
// 函数原型：void SysTick_Handler(void)
// 函数功能：SysTick中断服务程序
// 函数参数：无
// 函数返回值：无
//
//******************************************************************************************
void SysTick_Handler(void)       // 定时周期为20ms
{
	// 0.1秒钟软定时器计数
	if (++clock1s >= V_T1s)
	{
		clock1s_flag = 1; // 当0.1秒到时，溢出标志置1
		clock1s = 0;
	}
	
	
	if (++clock40ms >= V_T40ms)
	{
		clock40ms_flag = 1; // ?40ms??,?????1
		clock40ms = 0;
	}
	
	
	
	
	// 刷新全部数码管和LED指示灯
	if(ui8DigitRefresh == 0)
    TM1638_RefreshDIGIandLED(digit, pnt, led);

	// 检查当前键盘输入，0代表无键操作，1-9表示有对应按键
	// 键号显示在一位数码管上
	key_code = TM1638_Readkeyboard();

	
	prestate =curstate ;
	curstate =key_code ;
	if(prestate ==0&&curstate !=0){
		key=curstate ;
	} else {
		key=0;}

	if(key == 1){
		system_state = !system_state;
	}

	if(key == 3){
		non_stop = !non_stop;
	}
	
	if(system_state && non_stop){
		play();
	} else {
		PWMStop1();
	}
	
}

void play() {
    if (currentDuration == 0) {
        // 如果当前音符的持续时间已经结束，开始播放下一个音符
        PWMStart(melody[currentNote].frequency);
        currentDuration = melody[currentNote].duration;
        currentNote++;
        if (currentNote >= sizeof(melody) / sizeof(melody[0])) {
            currentNote = 0; // 循环播放
        }
    }
    currentDuration--;
}