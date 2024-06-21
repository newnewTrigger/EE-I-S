//******************************************************************************************
//
// Copyright: 2020-2021, �Ϻ���ͨ��ѧ���ӹ���ϵʵ���ѧ����
// File name: LM75BD_Hardware_IIC.c
// Description: ��ʾ�����ÿ��1000ms��ȡ�¶ȴ�����LM75BD���¶�ֵ  
//    1.���4���������ʾ��һ���¶ȴ�����LM75DB��ֵ(ADDR:0x48)����1λΪ����λ��������ʾ������ʾ-
//    2.�Ҳ�4���������ʾ�ڶ����¶ȴ�����LM75DB��ֵ(ADDR:0x4F)����1λΪ����λ��������ʾ������ʾ-
// Author:	�Ϻ���ͨ��ѧ���ӹ���ϵʵ���ѧ����
// Version: 1.0.0.20210513 
// Date��2021-05-13
// History��
//
//******************************************************************************************

//******************************************************************************************
//
// ͷ�ļ�
//
//******************************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"        // ��ַ�궨��
#include "inc/hw_types.h" 
#include "inc/hw_timer.h"         // ??????????
#include "inc/hw_ints.h"   
#include "driverlib/debug.h"      // ������
#include "driverlib/gpio.h"       // ͨ��IO�ں궨��
#include "driverlib/pin_map.h"    // TM4Cϵ��MCU��Χ�豸�ܽź궨��
#include "driverlib/sysctl.h"     // ϵͳ���ƶ���
#include "driverlib/systick.h"    // SysTick Driver ԭ��
#include "driverlib/interrupt.h"  // NVIC Interrupt Controller Driver ԭ��
#include "driverlib/fpu.h"
#include "driverlib/timer.h"      // ?Timer???????  
#include "driverlib/pwm.h"   
#include "driverlib/uart.h"
#include "driverlib/adc.h"        


#include "JXL.c"
#include "tm1638.h"               // �����TM1638оƬ�йصĺ���
#include "LM75BD.h"               // �����LM75BDоƬ�йصĺ���
#include "PWM.h" 
#include "ADC.h"
//******************************************************************************************
//
// �궨��
//
//******************************************************************************************
#define SYSTICK_FREQUENCY		50		// SysTickƵ��Ϊ50Hz����ѭ����ʱ����20ms
#define  MilliSecond      4000 
#define V_T1s	50              // 1s�����ʱ�����ֵ��50��20ms
#define V_T40ms	 2
//******************************************************************************************
//
// ����ԭ������
//
//******************************************************************************************
void GPIOInit(void);        // GPIO��ʼ��
void SysTickInit(void);     // ����SysTick�ж� 
void DevicesInit(void);     // MCU������ʼ����ע���������������
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
// ��������
//
//******************************************************************************************

uint8_t clock40ms = 0;
uint8_t clock40ms_flag = 0;




// �����ʱ������
uint8_t clock1s = 0;

// �����ʱ�������־
uint8_t clock1s_flag = 0;

// �����ü�����
uint32_t test_counter = 0;

// 8λ�������ʾ�����ֻ���ĸ����
// ע����������λ�������������Ϊ4��5��6��7��0��1��2��3
uint8_t digit[8]={' ',' ',' ',' ',' ',' ',' ',' '};

// 8λС���� 1��  0��
// ע����������λС����������������Ϊ4��5��6��7��0��1��2��3
uint8_t pnt = 0x44;

// 8��LEDָʾ��״̬��0��1��
// ע������ָʾ�ƴ������������Ϊ7��6��5��4��3��2��1��0
//     ��ӦԪ��LED8��LED7��LED6��LED5��LED4��LED3��LED2��LED1
uint8_t led[] = {1, 1, 1, 1, 1, 1, 1, 0};

uint8_t temp1[]={0,0,0,0};
uint8_t temp2[]={0,0,0,0};
uint8_t volt[]={0,0,0,0};
uint32_t freq_square=0;
uint8_t freq_lcd[]={1,0,1,0};
uint8_t freq_lcd_random[]={1,0,1,0};


// ��ǰ����ֵ
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


// ϵͳʱ��Ƶ�� 
uint32_t ui32SysClock;
uint32_t g_ui32SysClock;

// �����¶�ֵ 1LSB=0.1��
int16_t i16Temperature1;      
int16_t i16Temperature2;      

uint8_t ui8DigitRefresh = 0;


uint16_t ui16Temp1,ui16Temp2;

// ϵͳ��ǰ����״̬
bool system_state = true;

//�Ƿ���ͣ
bool non_stop = true;

struct Note {
    int frequency;  // Ƶ�ʣ���λΪHz
    int duration;   // ����ʱ�䣬��λΪ20ms������
};

struct Note melody[] = {
	{440, 45}, // 6. ����Ƶ��Ϊ440Hz������ʱ��Ϊ900ms
	{659, 15}, // 3 _ ��Ƶ��Ϊ659Hz������ʱ��Ϊ300ms
	{587, 15}, // 2 _ ��Ƶ��Ϊ587Hz������ʱ��Ϊ300ms
	{659, 15}, // 3 _ ��Ƶ��Ϊ659Hz������ʱ��Ϊ300ms
	{523, 15}, // 1 _ ��Ƶ��Ϊ523Hz������ʱ��Ϊ300ms
	{494, 15}, // 7. _ ��Ƶ��Ϊ494Hz������ʱ��Ϊ300ms
	
	{440, 120}, // 6. - - - ��Ƶ��Ϊ523Hz������ʱ��Ϊ2400ms
	
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
	
	{880, 45}, // 6 ��
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
    // �ڴ���Ӹ�������...
};

volatile int currentNote = 0;     // ��ǰ���ŵ���������
volatile int currentDuration = 0; // ��ǰ�����Ѿ����ŵ�ʱ��
//******************************************************************************************
//
// ������
//
//******************************************************************************************
 int main(void)
{

    DevicesInit();            //  MCU������ʼ��
	
	SysCtlDelay(60 * ( ui32SysClock / 3000));   // ��ʱ>60ms,�ȴ�TM1638�ϵ����
    TM1638_Init();	          // ��ʼ��TM1638
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
		
			temp1[1] = ui16Temp1 / 100;      	// ����ʮλ��
				temp1[2] = ui16Temp1 / 10 % 10; 	// �����λ��
				temp1[3] = ui16Temp1 % 10;
		
			temp2[1] = ui16Temp2 / 100;      	// ����ʮλ��
				temp2[2] = ui16Temp2 / 10 % 10; 	// �����λ��
				temp2[3] = ui16Temp2 % 10;
		
			
				ui32ADC0Value = ADC_Sample();  
				ui32ADC0Voltage = ui32ADC0Value * 3300 / 4095;
				
				volt[0] = (ui32ADC0Voltage / 1000) % 10; // ????????
				volt[1] = (ui32ADC0Voltage / 100) % 10;  // ?????????
				volt[2] = (ui32ADC0Voltage/10) % 10;         // ?????????        		
					volt[3] = ui32ADC0Voltage%10;
							
							clear_screen();
							display_GB2312_string(1,1,"��ѹ:",0);
							display_GB2312_string(3,1,"�¶�:",0);
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
							display_GB2312_string(3,89,"��",0);
							
				if(temp2[0]==1){display_GB2312_string(5,41,"-",0);}
							else{display_GB2312_string(5,41," ",0);}
							disp_number(5,49,temp2[1],0);
				disp_number(5,57,temp2[2],0);
							display_GB2312_string(5,65,".",0);
							disp_number(5,73,temp2[3],0);
							display_GB2312_string(5,89,"��",0);
							
							
		
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
						display_GB2312_string(1,1,"��Ƶ:",0);
						if(freq_lcd[0]==0){freqmode =2;display_GB2312_string(1,41," ",0);}
							else{freqmode =1;disp_number(1,41,freq_lcd[0],1);}
							
						if(freq_lcd[0]==0){disp_number(1,49,freq_lcd[1],1);}
						else{disp_number(1,49,freq_lcd[1],0);}
						disp_number(1,57,freq_lcd[2],0);	
						display_GB2312_string(1,65,".",0);	
						disp_number(1,73,freq_lcd[3],0);
						display_GB2312_string(1,81,"MHz",0);
						display_GB2312_string(7,97,"ȷ��",0);	
					}
				}
				else if(key==2){
				key=0;
					if(actmode ==0){
					actmode =1;
						clear_screen();
						display_GB2312_string(1,1,"��Ƶ:",0);
						if(freq_lcd[0]==0){freqmode =2;display_GB2312_string(1,41," ",0);}
							else{freqmode =1;disp_number(1,41,freq_lcd[0],1);}
							
						if(freq_lcd[0]==0){disp_number(1,49,freq_lcd[1],1);}
						else{disp_number(1,49,freq_lcd[1],0);}
						disp_number(1,57,freq_lcd[2],0);	
						display_GB2312_string(1,65,".",0);	
						disp_number(1,73,freq_lcd[3],0);
						display_GB2312_string(1,81,"MHz",0);
						display_GB2312_string(7,97,"ȷ��",0);	
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
						display_GB2312_string(1,1,"��Ƶ:",0);
						if(freq_lcd[0]==0){freqmode =2;display_GB2312_string(1,41," ",0);}
							else{freqmode =1;disp_number(1,41,freq_lcd[0],1);}
							
						if(freq_lcd[0]==0){disp_number(1,49,freq_lcd[1],1);}
						else{disp_number(1,49,freq_lcd[1],0);}
						disp_number(1,57,freq_lcd[2],0);	
						display_GB2312_string(1,65,".",0);	
						disp_number(1,73,freq_lcd[3],0);
						display_GB2312_string(1,81,"MHz",0);
						display_GB2312_string(7,97,"ȷ��",0);	
					}			
				}
				else if(key==4){
				key=0;
					if(actmode ==0){
					actmode =1;
						clear_screen();
						display_GB2312_string(1,1,"��Ƶ:",0);
						if(freq_lcd[0]==0){freqmode =2;display_GB2312_string(1,41," ",0);}
							else{freqmode =1;disp_number(1,41,freq_lcd[0],1);}
							
						if(freq_lcd[0]==0){disp_number(1,49,freq_lcd[1],1);}
						else{disp_number(1,49,freq_lcd[1],0);}
						disp_number(1,57,freq_lcd[2],0);	
						display_GB2312_string(1,65,".",0);	
						disp_number(1,73,freq_lcd[3],0);
						display_GB2312_string(1,81,"MHz",0);
						display_GB2312_string(7,97,"ȷ��",0);	
					}
		else if(actmode ==1){
				if(freqmode ==1){
						freqmode =7;
							disp_number(1,41,freq_lcd_random[0],0);
							display_GB2312_string(7,97,"ȷ��",1);
						}
						else if(freqmode ==2){
				if(freq_lcd_random[0]==0){
							freqmode =7;
								display_GB2312_string(7,97,"ȷ��",1);
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
							display_GB2312_string(7,97,"ȷ��",0);
							disp_number(1,73,freq_lcd_random[3],1);
						}
			}			
				}
				else if(key==5){
				key=0;
					if(actmode ==0){
					actmode =1;
						clear_screen();
						display_GB2312_string(1,1,"��Ƶ:",0);
						if(freq_lcd[0]==0){freqmode =2;display_GB2312_string(1,41," ",0);}
							else{freqmode =1;disp_number(1,41,freq_lcd[0],1);}
							
						if(freq_lcd[0]==0){disp_number(1,49,freq_lcd[1],1);}
						else{disp_number(1,49,freq_lcd[1],0);}
						disp_number(1,57,freq_lcd[2],0);	
						display_GB2312_string(1,65,".",0);	
						disp_number(1,73,freq_lcd[3],0);
						display_GB2312_string(1,81,"MHz",0);
						display_GB2312_string(7,97,"ȷ��",0);	
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
								display_GB2312_string(1,1,"��ѹ:",0);
								display_GB2312_string(3,1,"�¶�:",0);
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
								display_GB2312_string(3,89,"��",0);
								
					if(temp2[0]==1){display_GB2312_string(5,41,"-",0);}
								else{display_GB2312_string(5,41," ",0);}
								disp_number(5,49,temp2[1],0);
					disp_number(5,57,temp2[2],0);
								display_GB2312_string(5,65,".",0);
								disp_number(5,73,temp2[3],0);
								display_GB2312_string(5,89,"��",0);							 
									
									
								}
								else{
								freq_lcd_random[0]=freq_lcd[0];
								freq_lcd_random[1]=freq_lcd[1];
								freq_lcd_random[2]=freq_lcd[2];
								freq_lcd_random[3]=freq_lcd[3];
									
								clear_screen();
									display_GB2312_string(1,1,"��Ƶ����Χ��",0);
									LCD_delay(81000);
									clear_screen();
									actmode =1;
						display_GB2312_string(1,1,"��Ƶ:",0);
						if(freq_lcd[0]==0){freqmode =2;display_GB2312_string(1,41," ",0);}
							else{freqmode =1;disp_number(1,41,freq_lcd[0],1);}
							
						if(freq_lcd[0]==0){disp_number(1,49,freq_lcd[1],1);}
						else{disp_number(1,49,freq_lcd[1],0);}
						disp_number(1,57,freq_lcd[2],0);	
						display_GB2312_string(1,65,".",0);	
						disp_number(1,73,freq_lcd[3],0);
						display_GB2312_string(1,81,"MHz",0);
						display_GB2312_string(7,97,"ȷ��",0);								 
									
								}
						}
				}			
				}
				else if(key==6){
				key=0;
					if(actmode ==0){
					actmode =1;
						clear_screen();
						display_GB2312_string(1,1,"��Ƶ:",0);
						if(freq_lcd[0]==0){freqmode =2;display_GB2312_string(1,41," ",0);}
							else{freqmode =1;disp_number(1,41,freq_lcd[0],1);}
							
						if(freq_lcd[0]==0){disp_number(1,49,freq_lcd[1],1);}
						else{disp_number(1,49,freq_lcd[1],0);}
						disp_number(1,57,freq_lcd[2],0);	
						display_GB2312_string(1,65,".",0);	
						disp_number(1,73,freq_lcd[3],0);
						display_GB2312_string(1,81,"MHz",0);
						display_GB2312_string(7,97,"ȷ��",0);	
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
							display_GB2312_string(7,97,"ȷ��",1);	
						}
						else if(freqmode ==7){
						if(freq_lcd_random[0]==0){
							freqmode =2;
								display_GB2312_string(7,97,"ȷ��",0);
								disp_number(1,49,freq_lcd_random[1],1);
							}
							else {
								freqmode =1;
								display_GB2312_string(7,97,"ȷ��",0);
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
						display_GB2312_string(1,1,"��Ƶ:",0);
						if(freq_lcd[0]==0){freqmode =2;display_GB2312_string(1,41," ",0);}
							else{freqmode =1;disp_number(1,41,freq_lcd[0],1);}
							
						if(freq_lcd[0]==0){disp_number(1,49,freq_lcd[1],1);}
						else{disp_number(1,49,freq_lcd[1],0);}
						disp_number(1,57,freq_lcd[2],0);	
						display_GB2312_string(1,65,".",0);	
						disp_number(1,73,freq_lcd[3],0);
						display_GB2312_string(1,81,"MHz",0);
						display_GB2312_string(7,97,"ȷ��",0);	
					}			
				}
				else if(key==8){
				key=0;
					if(actmode ==0){
					actmode =1;
						clear_screen();
						display_GB2312_string(1,1,"��Ƶ:",0);
						if(freq_lcd[0]==0){freqmode =2;display_GB2312_string(1,41," ",0);}
							else{freqmode =1;disp_number(1,41,freq_lcd[0],1);}
							
						if(freq_lcd[0]==0){disp_number(1,49,freq_lcd[1],1);}
						else{disp_number(1,49,freq_lcd[1],0);}
						disp_number(1,57,freq_lcd[2],0);	
						display_GB2312_string(1,65,".",0);	
						disp_number(1,73,freq_lcd[3],0);
						display_GB2312_string(1,81,"MHz",0);
						display_GB2312_string(7,97,"ȷ��",0);	
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
						display_GB2312_string(1,1,"��Ƶ:",0);
						if(freq_lcd[0]==0){freqmode =2;display_GB2312_string(1,41," ",0);}
							else{freqmode =1;disp_number(1,41,freq_lcd[0],1);}
							
						if(freq_lcd[0]==0){disp_number(1,49,freq_lcd[1],1);}
						else{disp_number(1,49,freq_lcd[1],0);}
						disp_number(1,57,freq_lcd[2],0);	
						display_GB2312_string(1,65,".",0);	
						disp_number(1,73,freq_lcd[3],0);
						display_GB2312_string(1,81,"MHz",0);
						display_GB2312_string(7,97,"ȷ��",0);	
					}			
				}
			}	
		}
	
}


void trans_temp(void)  //ʹ�õڶ�·�¶�ֵ
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
	
	    temp1[1] = ui16Temp1 / 100;      	// ����ʮλ��
			temp1[2] = ui16Temp1 / 10 % 10; 	// �����λ��
			temp1[3] = ui16Temp1 % 10;
	
	    temp2[1] = ui16Temp2 / 100;      	// ����ʮλ��
			temp2[2] = ui16Temp2 / 10 % 10; 	// �����λ��
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
// ����ԭ�ͣ�void GPIOInit(void)
// �������ܣ�GPIO��ʼ����
//     ʹ��PortK������PK4,PK5Ϊ�����ʹ��PortM������PM0Ϊ�����
//     PK4����TM1638��STB��PK5����TM1638��DIO��PM0����TM1638��CLK��
//     ʹ��PortL������LPL0,PL1Ϊ�����PL0��PL1�ֱ�����DAC6571��SDA��SCL��
// ������������
// ��������ֵ����
//
//******************************************************************************************
void GPIOInit(void)
{
	//����TM1638оƬ�ܽ�
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);				// ʹ�ܶ˿� K	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK)){};		// �ȴ��˿� K ׼�����		
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);				// ʹ�ܶ˿� M	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM)){};		// �ȴ��˿� M ׼�����	

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);				// ʹ�ܶ˿� M	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)){};		// �ȴ��˿� M ׼�����		
	
    // ���ö˿� K �ĵ�4,5λ��PK4,PK5��Ϊ�������		PK4-STB  PK5-DIO
	GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_4|GPIO_PIN_5);
	// ���ö˿� M �ĵ�0λ��PM0��Ϊ�������   PM0-CLK
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
// ����ԭ�ͣ�SysTickInit(void)
// �������ܣ�����SysTick�ж�
// ������������
// ��������ֵ����
//
//******************************************************************************************
void SysTickInit(void)
{
	SysTickPeriodSet(ui32SysClock/SYSTICK_FREQUENCY); // ������������,��ʱ����20ms
	SysTickEnable();  			// SysTickʹ��
	SysTickIntEnable();			// SysTick�ж�����
}

//******************************************************************************************
// 
// ����ԭ�ͣ�void DevicesInit(void)
// �������ܣ�MCU������ʼ��������ϵͳʱ�����á�GPIO��ʼ����SysTick�ж�����
// ������������
// ��������ֵ����
//
//******************************************************************************************
void DevicesInit(void)
{
	// ʹ���ⲿ25MHz��ʱ��Դ������PLL��Ȼ���ƵΪ16MHz
	ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN | 
	                                   SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 
	                                   16000000);
	g_ui32SysClock=ui32SysClock;

    FPULazyStackingEnable(); 
    FPUEnable(); 
   
	GPIOInit();             // GPIO��ʼ��
    I2C0Init();             // I2C0��ʼ��
    SysTickInit();          // ����SysTick�ж�
	  UARTInit(); 
    IntMasterEnable();		// ���ж�����
	 ADCInit(); 
	 PWMInit();  
}

//******************************************************************************************
// 
// ����ԭ�ͣ�void SysTick_Handler(void)
// �������ܣ�SysTick�жϷ������
// ������������
// ��������ֵ����
//
//******************************************************************************************
void SysTick_Handler(void)       // ��ʱ����Ϊ20ms
{
	// 0.1������ʱ������
	if (++clock1s >= V_T1s)
	{
		clock1s_flag = 1; // ��0.1�뵽ʱ�������־��1
		clock1s = 0;
	}
	
	
	if (++clock40ms >= V_T40ms)
	{
		clock40ms_flag = 1; // ?40ms??,?????1
		clock40ms = 0;
	}
	
	
	
	
	// ˢ��ȫ������ܺ�LEDָʾ��
	if(ui8DigitRefresh == 0)
    TM1638_RefreshDIGIandLED(digit, pnt, led);

	// ��鵱ǰ�������룬0�����޼�������1-9��ʾ�ж�Ӧ����
	// ������ʾ��һλ�������
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
        // �����ǰ�����ĳ���ʱ���Ѿ���������ʼ������һ������
        PWMStart(melody[currentNote].frequency);
        currentDuration = melody[currentNote].duration;
        currentNote++;
        if (currentNote >= sizeof(melody) / sizeof(melody[0])) {
            currentNote = 0; // ѭ������
        }
    }
    currentDuration--;
}