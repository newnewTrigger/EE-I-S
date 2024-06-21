//*****************************************************************************
//
// Copyright: 2019-2020, 上海交通大学工程实践与科技创新II-A教学组
// File name: exp2_0.c
// Description:
//    1.开机或复位后，底板上右边4位数码管自动显示计时数值，最低位对应单位是0.1秒；
//    2.开机或复位后，底板上8个LED灯以跑马灯形式由左向右循环变换，约0.5秒变换1次；
//    3.当没有按键按下时，从左边数第二位数码管显示“0”；
//      当人工按下某键，数码管显示该键的编号；
//      此刻四位计时数码管暂停变化，停止计时，直到放开按键后自动继续计时。
// Author:	上海交通大学工程实践与科技创新II-A教学组
// Version: 1.0.0.20200208
// Date：2020-02-08
// History：
//
//*****************************************************************************

//*****************************************************************************
//
// 头文件
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"        // 基址宏定义
#include "inc/hw_types.h"         // 数据类型宏定义，寄存器访问函数
#include "driverlib/debug.h"      // 调试用
#include "driverlib/gpio.h"       // 通用IO口宏定义
#include "driverlib/pin_map.h"    // TM4C系列MCU外围设备管脚宏定义
#include "driverlib/sysctl.h"			// 系统控制定义
#include "driverlib/systick.h"    // SysTick Driver 原型
#include "driverlib/interrupt.h"  // NVIC Interrupt Controller Driver 原型

#include "tm1638.h"               // 与控制TM1638芯片有关的函数
#include "JLX12864G.h"

//*****************************************************************************
//
// 宏定义
//
//*****************************************************************************
#define SYSTICK_FREQUENCY		50		// SysTick频率为50Hz，即循环定时周期20ms

#define V_T100ms	5              // 0.1s软件定时器溢出值，5个20ms
#define V_T500ms	25             // 0.5s软件定时器溢出值，25个20ms
#define V_T2s	100             // 2s软件定时器溢出值，100个20ms
//*****************************************************************************
//
// 函数原型声明
//
//*****************************************************************************
void GPIOInit(void);        // GPIO初始化
void SysTickInit(void);     // 设置SysTick中断
void DevicesInit(void);     // MCU器件初始化，注：会调用上述函数
//*****************************************************************************
//
// 变量定义
//
//*****************************************************************************

// 软件定时器计数
uint8_t clock100ms = 0;
uint8_t clock500ms = 0;
uint8_t clock2s = 0;

// 软件定时器溢出标志
uint8_t clock100ms_flag = 0;
uint8_t clock500ms_flag = 0;
uint8_t clock2s_flag = 1;

//LCD屏幕状态机
uint8_t lcd_act = 0;

// 测试用计数器
uint32_t test_counter = 0;

// 8位数码管显示的数字或字母符号
// 注：板上数码位从左到右序号排列为4、5、6、7、0、1、2、3
uint8_t digit[8]= {' ',' ',' ',' ','_',' ','_',' '};

// 8位小数点 1亮  0灭
// 注：板上数码位小数点从左到右序号排列为4、5、6、7、0、1、2、3
uint8_t pnt = 0x04;

// 8个LED指示灯状态，0灭，1亮
// 注：板上指示灯从左到右序号排列为7、6、5、4、3、2、1、0
//     对应元件LED8、LED7、LED6、LED5、LED4、LED3、LED2、LED1
uint8_t led[] = {1, 1, 1, 1, 1, 1, 1, 0};

// 当前按键值
uint8_t key_code = 0;

// 系统时钟频率
uint32_t ui32SysClock;

unsigned char jiong1[]= { //-- 文字: 囧 --
//-- 宋体12; 此字体下对应的点阵为：宽x 高=16x16 --
    0x00,0xFE,0x82,0x42,0xA2,0x9E,0x8A,0x82,0x86,0x8A,0xB2,0x62,0x02,0xFE,0x00,0x00,
    0x00,0x7F,0x40,0x40,0x7F,0x40,0x40,0x40,0x40,0x40,0x7F,0x40,0x40,0x7F,0x00,0x00
};
unsigned char lei1[]= { //-- 文字: 畾 --
    //-- 宋体12; 此字体下对应的点阵为：宽x 高=16x16 --
    0x80,0x80,0x80,0xBF,0xA5,0xA5,0xA5,0x3F,0xA5,0xA5,0xA5,0xBF,0x80,0x80,0x80,0x00,
    0x7F,0x24,0x24,0x3F,0x24,0x24,0x7F,0x00,0x7F,0x24,0x24,0x3F,0x24,0x24,0x7F,0x00
};
unsigned char bmp1[];

//*****************************************************************************
//
// 主程序
//
//*****************************************************************************
int main(void)
{
    uint8_t temp,i;
    DevicesInit();            //  MCU器件初始化

    while (clock100ms < 3);   // 延时>60ms,等待TM1638上电完成
    TM1638_Init();	          // 初始化TM1638
	
//lcd移植测试
		LCD_Init();
//		clear_screen();
//		display_128x64(bmp1);

    while (1)
    {

        if (clock100ms_flag == 1)      // 检查0.1秒定时是否到
        {
            clock100ms_flag		= 0;
            // 每0.1秒累加计时值在数码管上以十进制显示，有键按下时暂停计时
            if (key_code == 0)
            {
                if (++test_counter >= 10000) test_counter = 0;
                digit[0] = test_counter / 1000; 	    // 计算百位数
                digit[1] = test_counter / 100 % 10; 	// 计算十位数
                digit[2] = test_counter / 10 % 10; 	  // 计算个位数
                digit[3] = test_counter % 10;         // 计算百分位数
            }
        }

        if (clock500ms_flag == 1)   // 检查0.5秒定时是否到
        {
            clock500ms_flag = 0;
            // 8个指示灯以走马灯方式，每0.5秒向右（循环）移动一格
            temp = led[0];
            for (i = 0; i < 7; i++) led[i] = led[i + 1];
            led[7] = temp;
        }
				
				if (clock2s_flag == 1)
				{
						clock2s_flag=0;
						TEST_H;
						switch (lcd_act)
						{
							case 0:
								lcd_act++;clear_screen();
								display_128x64(bmp1);
								break;									
							case 1: 
								lcd_act++;	clear_screen();							
								display_GB2312_string(1,1,"12864,带中文字库",0); //在第1 页，第1 列，显示一串16x16 点阵汉字或8x16 的ASCII 字
								display_GB2312_string(3,1,"16X16 简体汉字库,",0); //显示一串16x16 点阵汉字或8x16 的ASCII 字.以下雷同
								display_GB2312_string(5,1,"或8X16 点阵ASCII,",0);
								display_GB2312_string(7,1,"或5x8 点阵ASCII 码",0);
								break;
							case 2:
								lcd_act++;clear_screen();
								display_GB2312_string(1,1,"晶联讯成立于二零",1);
								display_GB2312_string(3,1,"零四年十一月七日",1);
								display_GB2312_string(5,1,"主要生产液晶模块",1);
								display_GB2312_string(7,1,"品质至上真诚服务",1);	
								break;	
							case 3:
								lcd_act++;clear_screen();
								display_GB2312_string(1,1,"GB2312简体字库及",1);
								display_GB2312_string(3,1,"有图型功能，可自",0);
								display_GB2312_string(5,1,"编大字或图像或生",1);
								display_GB2312_string(7,1,"僻字，例如： ",0);
								display_graphic_16x16(7,97,jiong1); //在第7页，第81列显示单个自编生僻汉字“囧”
								display_graphic_16x16(7,113,lei1); //显示单个自编生僻汉字"畾“
								break;								
							case 4:
								lcd_act++;clear_screen();
								display_GB2312_string(1,1,"<!@#$%^&*()_-+]/",0); //在第1 页，第1 列，显示一串16x16 点阵汉字或8*16 的ASCII 字
								display_string_5x8(3,1,"<!@#$%^&*()_-+]/;.,?[",0);//在第3 页，第1 列，显示一串5x8 点阵的ASCII 字
								display_string_5x8(4,1,"JLX electronics Co., ",0);//显示一串5x8 点阵的ASCII 字
								display_string_5x8(5,1,"Ltd. established at ",0);//显示一串5x8 点阵的ASCII 字
								display_string_5x8(6,1,"year 2004.Focus LCM. ",0);//显示一串5x8 点阵的ASCII 字
								display_string_5x8(7,1,"TEL:0755-29784961 ",0);//显示一串5x8 点阵的ASCII 字
								display_string_5x8(8,1,"FAX:0755-29784964 ",0);//显示一串5x8 点阵的ASCII 字
								break;	
							case 5:
								lcd_act++;clear_screen();
								display_GB2312_string(1,1,"啊阿埃挨哎唉哀皑",0); //在第1 页，第1 列，显示一串16x16 点阵汉字或8x16 的ASCII 字
								display_GB2312_string(3,1,"癌蔼矮艾碍爱隘鞍",0); //显示一串16x16 点阵汉字或8x16 的ASCII 字.以下雷同
								display_GB2312_string(5,1,"氨安俺按暗岸胺案",0);
								display_GB2312_string(7,1,"肮昂盎凹敖熬翱袄",0);			
								break;								
							case 6:
								lcd_act=0;clear_screen();
								display_GB2312_string(1,1,"鬟鬣麽麾縻麂麇麈",0);
								display_GB2312_string(3,1,"麋麒鏖麝麟黛黜黝",0);
								display_GB2312_string(5,1,"黠黟黢黩黧黥黪黯",0);
								display_GB2312_string(7,1,"鼢鼬鼯鼹鼷鼽鼾齄",0);		
								break;

							default: break;								
						}
						TEST_L;
					
				}

    }

}

//*****************************************************************************
//
// 函数原型：void GPIOInit(void)
// 函数功能：GPIO初始化。使能PortK，设置PK4,PK5为输出；使能PortM，设置PM0为输出。
//          （PK4连接TM1638的STB，PK5连接TM1638的DIO，PM0连接TM1638的CLK）
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void GPIOInit(void)
{
    /////配置TM1638芯片管脚
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);						// 使能端口 K
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK)) {};		// 等待端口 K准备完毕

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);						// 使能端口 M
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM)) {};		// 等待端口 M准备完毕

    // 设置端口 K的第4,5位（PK4,PK5）为输出引脚		PK4-STB  PK5-DIO
    GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_4|GPIO_PIN_5);
    // 设置端口 M的第0位（PM0）为输出引脚   PM0-CLK
    GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_0);
			
			/////配置LCD屏幕JXL12864G管脚
		LCD_PORT_init();

}

//*****************************************************************************
//
// 函数原型：SysTickInit(void)
// 函数功能：设置SysTick中断
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void SysTickInit(void)
{
    SysTickPeriodSet(ui32SysClock/SYSTICK_FREQUENCY); // 设置心跳节拍,定时周期20ms
    SysTickEnable();  			// SysTick使能
    SysTickIntEnable();			// SysTick中断允许
}

//*****************************************************************************
//
// 函数原型：DevicesInit(void)
// 函数功能：CU器件初始化，包括系统时钟设置、GPIO初始化和SysTick中断设置
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void DevicesInit(void)
{
    // 使用外部25MHz主时钟源，经过PLL，然后分频为20MHz
    ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN |
                                       SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480),
                                      20000000);
    GPIOInit();             // GPIO初始化
    SysTickInit();          // 设置SysTick中断
    IntMasterEnable();			// 总中断允许
}

//*****************************************************************************
//
// 函数原型：void SysTick_Handler(void)
// 函数功能：SysTick中断服务程序
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void SysTick_Handler(void)       // 定时周期为20ms
{
    // 0.1秒钟软定时器计数
    if (++clock100ms >= V_T100ms)
    {
        clock100ms_flag = 1; // 当0.1秒到时，溢出标志置1
        clock100ms = 0;
    }

    // 0.5秒钟软定时器计数
    if (++clock500ms >= V_T500ms)
    {
        clock500ms_flag = 1; // 当0.5秒到时，溢出标志置1
        clock500ms = 0;
    }

		    // 2秒钟软定时器计数
    if (++clock2s >= V_T2s)
    {
        clock2s_flag = 1; // 当0.5秒到时，溢出标志置1
        clock2s = 0;
    }
    // 刷新全部数码管和LED指示灯
    TM1638_RefreshDIGIandLED(digit, pnt, led);

    // 检查当前键盘输入，0代表无键操作，1-9表示有对应按键
    // 键号显示在一位数码管上
    key_code = TM1638_Readkeyboard();

    digit[5] = key_code;
}

unsigned char bmp1[]= {
    /*
    Image size:128 X 64 X 1BPP=1024 bytes.
    Every 8 rows formed one page.
    1 byte=Row7:Row6:Row5:Row4:Row3:Row2:Row1:Row0 (That means row0 is at the lowest bit(s).)
    */

    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x80,0x80,0x80,0x40,0xc0,0x80,0xf8,0x58,0x58,0xf8,0x00,0xe0,0xf0,0x70,
    0xf0,0xa0,0x00,0x80,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xb8,0xb8,0xbe,0xbe,0xb6,0xb6,0xb7,
    0xb3,0xbb,0xbb,0x91,0x11,0xb8,0x00,0x00,0xff,0xf5,0xfd,0xff,0x00,0xff,0xff,0xf5,
    0xff,0x09,0x03,0x0f,0x3d,0x77,0xfe,0xf8,0xe0,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x80,0x80,0x80,0x80,0x80,0x80,0x9d,0x99,0x99,0x99,0xd9,0x19,0x19,
    0x99,0x19,0x19,0x59,0x81,0x9d,0x80,0x80,0x9f,0x9f,0x9f,0x9f,0x80,0x9f,0x9f,0x9f,
    0x9f,0x88,0x80,0x80,0x80,0x80,0x81,0x87,0x9f,0x8f,0x8f,0x84,0x80,0x80,0x80,0x80,
    0x80,0x80,0x80,0x80,0x80,0x80,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x1f,0x3f,0x3f,0x03,0x01,0x3f,0x3f,0x3f,0x1f,0x8f,0xe3,0xf0,0x7c,0x1f,
    0x07,0x71,0x7c,0x7e,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0xff,0xff,0xff,0x7f,0x7f,
    0x7f,0x7f,0x7f,0x7f,0xff,0xff,0xff,0x7f,0x7f,0x7f,0x3f,0x3f,0x1f,0x0f,0x0f,0x0f,
    0xe7,0x3f,0x7f,0xe7,0xc7,0x03,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xe0,0xf8,0x3e,0x0f,0x03,0x00,0x00,0x00,
    0x03,0x82,0xc6,0xfe,0x3e,0x32,0x32,0x32,0x32,0x32,0x33,0x33,0x33,0x33,0x32,0x32,
    0x36,0xfe,0xc6,0x83,0x03,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x8f,0xfc,0xfc,0x8f,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x78,
    0xf0,0xf0,0xe0,0xe0,0xf0,0xfc,0x9f,0x8f,0x03,0x00,0x80,0x80,0x80,0x98,0x9c,0x9e,
    0x9f,0x93,0x93,0x93,0x93,0x93,0x93,0x93,0x93,0x93,0x93,0x93,0x93,0x93,0x93,0x93,
    0x93,0x93,0x93,0x93,0x9f,0x9f,0x9c,0x98,0x80,0x80,0x80,0x00,0x00,0x00,0x00,0x00,
    0x3f,0x20,0x21,0x3f,0x1f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x01,0x01,0x03,0x03,0x07,0x07,0x0f,0x0f,0x07,0x01,0xff,0x80,0x84,0x82,0xbe,0xbc,
    0x80,0xb4,0xbe,0xaa,0xaa,0xaa,0xaa,0xbe,0x80,0x80,0xae,0xaa,0xaa,0xaa,0xaa,0xbe,
    0x80,0x80,0x80,0xbe,0xaa,0xaa,0xaa,0xaa,0xba,0x80,0x81,0xff,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,
    0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
    0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
