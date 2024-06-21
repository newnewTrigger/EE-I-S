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
#include "inc/hw_types.h"         // 数据类型宏定义，寄存器访问函数
#include "driverlib/debug.h"      // 调试用
#include "driverlib/gpio.h"       // 通用IO口宏定义
#include "driverlib/pin_map.h"    // TM4C系列MCU外围设备管脚宏定义
#include "driverlib/sysctl.h"     // 系统控制定义
#include "driverlib/systick.h"    // SysTick Driver 原型
#include "driverlib/interrupt.h"  // NVIC Interrupt Controller Driver 原型
#include "driverlib/fpu.h"
#include "driverlib/uart.h"

#include "tm1638.h"               // 与控制TM1638芯片有关的函数
#include "LM75BD.h"               // 与控制LM75BD芯片有关的函数

//******************************************************************************************
//
// 宏定义
//
//******************************************************************************************
#define SYSTICK_FREQUENCY		50		// SysTick频率为50Hz，即循环定时周期20ms

#define V_T1000ms	50              // 1s软件定时器溢出值，50个20ms

//******************************************************************************************
//
// 函数原型声明
//
//******************************************************************************************
void GPIOInit(void);        // GPIO初始化
void SysTickInit(void);     // 设置SysTick中断 
void DevicesInit(void);     // MCU器件初始化，注：会调用上述函数
//******************************************************************************************
//
// 变量定义
//
//******************************************************************************************

// 软件定时器计数
uint8_t clock1000ms = 0;

// 软件定时器溢出标志
uint8_t clock1000ms_flag = 0;

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

// 当前按键值
uint8_t key_code = 0;
uint8_t key_cnt = 0;

// 系统时钟频率 
uint32_t ui32SysClock;

// 返回温度值 1LSB=0.1℃
int16_t i16Temperature1;      
int16_t i16Temperature2;      

uint8_t ui8DigitRefresh = 0;
//******************************************************************************************
//
// 主程序
//
//******************************************************************************************
 int main(void)
{
    uint16_t ui16Temp1,ui16Temp2;
    DevicesInit();            //  MCU器件初始化
	
	SysCtlDelay(60 * ( ui32SysClock / 3000));   // 延时>60ms,等待TM1638上电完成
    TM1638_Init();	          // 初始化TM1638
	    
	while (1)
	{			       
        if (clock1000ms_flag == 1)      // 检查1秒定时是否到
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
           
			digit[1] = ui16Temp1 / 100;      	// 计算十位数
			digit[2] = ui16Temp1 / 10 % 10; 	// 计算个位数
			digit[3] = ui16Temp1 % 10;          // 计算十分位数

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


			digit[5] = ui16Temp2 / 100;      	// 计算十位数
			digit[6] = ui16Temp2 / 10 % 10; 	// 计算个位数
			digit[7] = ui16Temp2 % 10;          // 计算十分位数
 
            ui8DigitRefresh = 0;

         }
	}
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
	
    // 设置端口 K 的第4,5位（PK4,PK5）为输出引脚		PK4-STB  PK5-DIO
	GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_4|GPIO_PIN_5);
	// 设置端口 M 的第0位（PM0）为输出引脚   PM0-CLK
	GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_0);	
                
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

    FPULazyStackingEnable(); 
    FPUEnable(); 
   
	GPIOInit();             // GPIO初始化
    I2C0Init();             // I2C0初始化
    SysTickInit();          // 设置SysTick中断
    IntMasterEnable();		// 总中断允许
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
	if (++clock1000ms >= V_T1000ms)
	{
		clock1000ms_flag = 1; // 当0.1秒到时，溢出标志置1
		clock1000ms = 0;
	}
	
	// 刷新全部数码管和LED指示灯
	if(ui8DigitRefresh == 0)
    TM1638_RefreshDIGIandLED(digit, pnt, led);

	// 检查当前键盘输入，0代表无键操作，1-9表示有对应按键
	// 键号显示在一位数码管上
	key_code = TM1638_Readkeyboard();
}
