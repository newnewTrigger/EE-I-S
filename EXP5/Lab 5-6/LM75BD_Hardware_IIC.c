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
#include "inc/hw_types.h"         // �������ͺ궨�壬�Ĵ������ʺ���
#include "driverlib/debug.h"      // ������
#include "driverlib/gpio.h"       // ͨ��IO�ں궨��
#include "driverlib/pin_map.h"    // TM4Cϵ��MCU��Χ�豸�ܽź궨��
#include "driverlib/sysctl.h"     // ϵͳ���ƶ���
#include "driverlib/systick.h"    // SysTick Driver ԭ��
#include "driverlib/interrupt.h"  // NVIC Interrupt Controller Driver ԭ��
#include "driverlib/fpu.h"
#include "driverlib/uart.h"

#include "tm1638.h"               // �����TM1638оƬ�йصĺ���
#include "LM75BD.h"               // �����LM75BDоƬ�йصĺ���

//******************************************************************************************
//
// �궨��
//
//******************************************************************************************
#define SYSTICK_FREQUENCY		50		// SysTickƵ��Ϊ50Hz����ѭ����ʱ����20ms

#define V_T1000ms	50              // 1s�����ʱ�����ֵ��50��20ms

//******************************************************************************************
//
// ����ԭ������
//
//******************************************************************************************
void GPIOInit(void);        // GPIO��ʼ��
void SysTickInit(void);     // ����SysTick�ж� 
void DevicesInit(void);     // MCU������ʼ����ע���������������
//******************************************************************************************
//
// ��������
//
//******************************************************************************************

// �����ʱ������
uint8_t clock1000ms = 0;

// �����ʱ�������־
uint8_t clock1000ms_flag = 0;

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

// ��ǰ����ֵ
uint8_t key_code = 0;
uint8_t key_cnt = 0;

// ϵͳʱ��Ƶ�� 
uint32_t ui32SysClock;

// �����¶�ֵ 1LSB=0.1��
int16_t i16Temperature1;      
int16_t i16Temperature2;      

uint8_t ui8DigitRefresh = 0;
//******************************************************************************************
//
// ������
//
//******************************************************************************************
 int main(void)
{
    uint16_t ui16Temp1,ui16Temp2;
    DevicesInit();            //  MCU������ʼ��
	
	SysCtlDelay(60 * ( ui32SysClock / 3000));   // ��ʱ>60ms,�ȴ�TM1638�ϵ����
    TM1638_Init();	          // ��ʼ��TM1638
	    
	while (1)
	{			       
        if (clock1000ms_flag == 1)      // ���1�붨ʱ�Ƿ�
		{
			clock1000ms_flag = 0;
                
            i16Temperature1 = GetTemputerature(LM75BD_ADR2);               
            i16Temperature2 = GetTemputerature(LM75BD_ADR1);    
            
            ui8DigitRefresh = 1;
            
			if(i16Temperature1 < 0)
            {
                ui16Temp1 = -i16Temperature1;
                    digit[0] = '-';
            }    
            else
            {
                ui16Temp1 = i16Temperature1;
                digit[0] = ' ';
            }
           
			digit[1] = ui16Temp1 / 100;      	// ����ʮλ��
			digit[2] = ui16Temp1 / 10 % 10; 	// �����λ��
			digit[3] = ui16Temp1 % 10;          // ����ʮ��λ��

            if(i16Temperature2 < 0)
            {
                ui16Temp2 = -i16Temperature2;
                digit[4] = '-';
            }    
            else
            {
                ui16Temp2 = i16Temperature2;
                digit[4] = ' ';
            }


			digit[5] = ui16Temp2 / 100;      	// ����ʮλ��
			digit[6] = ui16Temp2 / 10 % 10; 	// �����λ��
			digit[7] = ui16Temp2 % 10;          // ����ʮ��λ��
 
            ui8DigitRefresh = 0;

         }
	}
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
	
    // ���ö˿� K �ĵ�4,5λ��PK4,PK5��Ϊ�������		PK4-STB  PK5-DIO
	GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_4|GPIO_PIN_5);
	// ���ö˿� M �ĵ�0λ��PM0��Ϊ�������   PM0-CLK
	GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_0);	
                
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

    FPULazyStackingEnable(); 
    FPUEnable(); 
   
	GPIOInit();             // GPIO��ʼ��
    I2C0Init();             // I2C0��ʼ��
    SysTickInit();          // ����SysTick�ж�
    IntMasterEnable();		// ���ж�����
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
	if (++clock1000ms >= V_T1000ms)
	{
		clock1000ms_flag = 1; // ��0.1�뵽ʱ�������־��1
		clock1000ms = 0;
	}
	
	// ˢ��ȫ������ܺ�LEDָʾ��
	if(ui8DigitRefresh == 0)
    TM1638_RefreshDIGIandLED(digit, pnt, led);

	// ��鵱ǰ�������룬0�����޼�������1-9��ʾ�ж�Ӧ����
	// ������ʾ��һλ�������
	key_code = TM1638_Readkeyboard();
}
