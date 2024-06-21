//*******************************************************************************************************
//
// Copyright: 2021-2022, 上海交通大学电子工程系实验教学中心
// File name: PWM_Demo.c  
// Description: 本示例展示如何利用M0PWM4/PG0引脚产生方波（占空比为50%的PWM信号）
// 1.按键1，产生方波信号
// 2.按键2，停止方波信号的产生           
// Author:	上海交通大学电子工程系实验教学中心
// Version: 1.0.0.20210508 
// Date：2021-05-08
// History：
//
//*******************************************************************************************************

//*******************************************************************************************************
//
// 头文件
//
//*******************************************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"        // 基址宏定义
#include "inc/hw_types.h"         // 数据类型宏定义，寄存器访问函数
#include "inc/hw_timer.h"         // 与定时器有关的宏定义
#include "inc/hw_ints.h"          // 与中断有关的宏定义
#include "driverlib/debug.h"      // 调试用
#include "driverlib/gpio.h"       // 通用IO口宏定义和函数原型
#include "driverlib/pin_map.h"    // TM4C系列MCU外围设备管脚宏定义
#include "driverlib/sysctl.h"	  // 系统控制定义
#include "driverlib/systick.h"    // SysTick Driver 函数原型
#include "driverlib/interrupt.h"  // NVIC中断控制驱动函数原型
#include "driverlib/timer.h"      // 与Timer有关的函数原型  
#include "driverlib/pwm.h"        // 与Timer有关的函数原型
#include "driverlib/uart.h"
#include "driverlib/fpu.h"

#include "tm1638.h"               // 与控制TM1638芯片有关的宏定义和函数原型
#include "PWM.h"                  // 与PWM相关的函数原型

//*******************************************************************************************************
//
// 宏定义
//
//*******************************************************************************************************
#define SYSTICK_FREQUENCY		50		// SysTick频率为50Hz，即循环定时周期20ms

//*******************************************************************************************************
//
// 函数原型声明
//
//*******************************************************************************************************
void GPIOInit(void);        // GPIO初始化
void SysTickInit(void);     // 设置SysTick中断
void DevicesInit(void);     // MCU器件初始化，注：会调用上述函数

//*******************************************************************************************************
//
// 变量定义
//
//*******************************************************************************************************

// 8位数码管显示的数字或字母符号
// 注：板上数码位从左到右序号排列为4、5、6、7、0、1、2、3
uint8_t digit[8]={' ',' ',' ',' ',' ',' ',' ',' '};

// 8位小数点 1亮  0灭
// 注：板上数码位小数点从左到右序号排列为4、5、6、7、0、1、2、3
uint8_t pnt = 0x00;

// 8个LED指示灯状态，0灭，1亮
// 注：板上指示灯从左到右序号排列为7、6、5、4、3、2、1、0
//     对应元件LED8、LED7、LED6、LED5、LED4、LED3、LED2、LED1
uint8_t led[] = {0, 0, 0, 0, 0, 0, 0, 0};

// 当前按键值
volatile uint8_t key_code = 0;

// 用于记录前一次按键检测时的键盘状态，0表示无键按下，1有键按下
volatile uint8_t key_state = 0;

// 按键操作有效标记，0代表无新操作，1代表有键操作
volatile uint8_t  key_flag = 0;

// 系统时钟频率 
//uint32_t g_ui32SysClock;
uint32_t ui32SysClock;

//*******************************************************************************************************
//
// 主程序
//
//*******************************************************************************************************
int main(void)
{

    uint32_t ui32Freq = 38000;  // 方波频率，根据实际需要进行修改
    
    DevicesInit();            //  MCU器件初始化
	
    SysCtlDelay(60 * ( ui32SysClock / 3000));   // 延时>60ms,等待TM1638上电完成
    TM1638_Init();	          // 初始化TM1638
	                            
    while (1)
    {	
        if(key_flag == 1)    // 有键按下
        {
            key_flag = 0;   
            
            switch(key_code)
            {               
                case 1:   // 产生频率为ui32Freq的方波信号     
                    PWMStart(ui32Freq);
                    break;
                case 2:      // 停止产生PWM信号
                    PWMStop();
                    break;
                default:
                    break;
            }
        }
    }
}

//*******************************************************************************************************
//
// 函数原型：void GPIOInit(void)
// 函数功能：GPIO初始化。使能PortK，设置PK4,PK5为输出；使能PortM，设置PM0为输出。
//          （PK4连接TM1638的STB，PK5连接TM1638的DIO，PM0连接TM1638的CLK）
// 函数参数：无
// 函数返回值：无
//
//*******************************************************************************************************
void GPIOInit(void)
{
	//配置TM1638芯片管脚
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);				// 使能端口 K	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK)){};		// 等待端口 K准备完毕		
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);				// 使能端口 M	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM)){};		// 等待端口 M准备完毕		
	
    // 设置端口 K的第4,5位（PK4,PK5）为输出引脚		PK4-STB  PK5-DIO
	GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_4|GPIO_PIN_5);
	// 设置端口 M的第0位（PM0）为输出引脚   PM0-CLK
	GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_0);	
}

//*******************************************************************************************************
// 
// 函数原型：SysTickInit(void)
// 函数功能：设置SysTick中断
// 函数参数：无
// 函数返回值：无
//
//*******************************************************************************************************
void SysTickInit(void)
{
	SysTickPeriodSet(ui32SysClock/SYSTICK_FREQUENCY); // 设置心跳节拍,定时周期20ms
	SysTickEnable();  			// SysTick使能
	SysTickIntEnable();			// SysTick中断允许
}

//*******************************************************************************************************
// 
// 函数原型：DevicesInit(void)
// 函数功能：MCU器件初始化，包括系统时钟设置、GPIO初始化和SysTick中断设置
// 函数参数：无
// 函数返回值：无
//
//*******************************************************************************************************
void DevicesInit(void)
{
	// 使用外部25MHz主时钟源，经过PLL，然后分频为16MHz
	ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN | 
	                                   SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 
	                                   16000000);

    FPULazyStackingEnable(); 
    FPUEnable(); 

	GPIOInit();             // GPIO初始化
    PWMInit();              // PWM初始化 
    SysTickInit();          // 设置SysTick中断
    IntMasterEnable();	    // 总中断允许          
}

//*******************************************************************************************************
// 
// 函数原型：void SysTick_Handler(void)
// 函数功能：SysTick中断服务程序，检测按键动作，刷新数码管显示
// 函数参数：无
// 函数返回值：无
//
//*******************************************************************************************************
void SysTick_Handler(void)    // 定时周期为20ms
{

	// 刷新全部数码管和LED指示灯
	TM1638_RefreshDIGIandLED(digit, pnt, led);

	// 检查当前键盘输入，0代表无键操作，1-9表示有对应按键
	// 键号显示在一位数码管上
	key_code = TM1638_Readkeyboard();

	// 按键操作在SysTick中断服务程序中的状态转移处理程序
	
	// key_state用于记录前一次按键检测时的键盘状态，0表示无键按下，1有键按下
	switch (key_state)
	{
        case 0:   // 前一次按键检测时无键按下
            if (key_code > 0)   // 本次按键检测有键按下
            {
                key_state = 1;
                key_flag = 1;			
            }
		    break;
        case 1:    // 前一次按键检测时有键按下
            if (key_code == 0)  // 本次按键检测时无键按下
            {
                key_state = 0;
            }
            break;
        default:
            key_state = 0;
            break;
    }	
}
