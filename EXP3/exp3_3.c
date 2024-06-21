//*****************************************************************************
//
// Copyright: 2020-2023, 上海交通大学电子工程系实验教学中心
// File name: exp3_2DEMO.c
// Description: 
//    发送命令以\r为结束符
//    在UART0中断中，接收较长的命令， 在主程序中返回给虚拟串口 。
// 
// Author:	上海交通大学电子工程系实验教学中心
// Version: 1.0.0.20230503 
// Date：2023-05
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
#include <string.h>
#include "inc/hw_memmap.h"        // 基址宏定义
#include "inc/hw_types.h"         // 数据类型宏定义，寄存器访问函数
#include "inc/hw_ints.h"          // 与中断有关的宏定义
#include "driverlib/debug.h"      // 调试用
#include "driverlib/gpio.h"       // 通用IO口宏定义和函数原型
#include "driverlib/pin_map.h"    // TM4C系列MCU外围设备管脚宏定义
#include "driverlib/sysctl.h"	  // 系统控制定义
#include "driverlib/systick.h"    // SysTick Driver 原型
#include "driverlib/interrupt.h"  // NVIC Interrupt Controller Driver 原型
#include "driverlib/uart.h"       // 与UART有关的宏定义和函数原型

#include "tm1638.h"               // 与控制TM1638芯片有关的宏定义和函数原型

//*****************************************************************************
//
// 宏定义
//
//*****************************************************************************
#define SYSTICK_FREQUENCY		50		// SysTick频率为50Hz，即循环定时周期20ms

#define V_T1s	50                  // 1s软件定时器溢出值，50个20ms

//*****************************************************************************
//
// 函数原型声明
//
//*****************************************************************************
void GPIOInit(void);        // GPIO初始化
void SysTickInit(void);     // 设置SysTick中断
void UARTInit(void);        // UART初始化
void DevicesInit(void);     // MCU器件初始化，注：会调用上述函数
void UARTStringPutNOBlocking(uint32_t ui32Base,uint8_t *cMessage);// 向UART发送字符串
bool match1(uint8_t *str);
bool match2(uint8_t *str);
bool match3(uint8_t *str);
void add_time(uint8_t h, uint8_t c, uint8_t s);
void time_str(void);

//*****************************************************************************
//
// 变量定义
//
//*****************************************************************************

// 8位数码管显示的数字或字母符号
// 注：板上数码位从左到右序号排列为4、5、6、7、0、1、2、3
uint8_t digit[8]={0,0,' ',' ',0,0,0,0};

// 8位小数点 1亮  0灭
// 注：板上数码位小数点从左到右序号排列为4、5、6、7、0、1、2、3
uint8_t pnt = 0xA0;

// 8个LED指示灯状态，0灭，1亮
// 注：板上指示灯从左到右序号排列为7、6、5、4、3、2、1、0
//     对应元件LED8、LED7、LED6、LED5、LED4、LED3、LED2、LED1
uint8_t led[] = {0, 0, 0, 0, 0, 0, 0, 1};

// 当前按键值
volatile uint8_t key_code = 0;

// 用于记录前一次按键检测时的键盘状态，0表示无键按下，1有键按下
volatile uint8_t key_state = 0;

// 记录系统运行状态
volatile uint8_t  run_state = 0;

// 系统时钟频率 
uint32_t ui32SysClock;

//存储命令
uint8_t uart_receive_buffer[200] = {'\0'};
uint8_t uart_receive_len = 0;

//命令是否接受完毕（出现\r置1）
uint8_t cmd_state = 0;

uint8_t AT_GET[] = "AT+GET";
uint8_t AT_SET[] = "AT+SET";
uint8_t AT_INC[] = "AT+INC";

uint8_t time_h = 0;
uint8_t time_c = 0;
uint8_t time_s = 0;

uint8_t time_now[10] = "00:00:00";

bool T1s = 0;
uint8_t T1_count = 0;

uint32_t ii = 0;

//*****************************************************************************
//
// 主程序
//
//*****************************************************************************
int main(void)
{

    DevicesInit();            //  MCU器件初始化
	
    SysCtlDelay(60 * ( ui32SysClock / 3000)); // 延时>60ms,等待TM1638上电完成
    TM1638_Init();	          // 初始化TM1638
	
    UARTStringPutNOBlocking(UART0_BASE, (uint8_t*)"TEST\r\n");
	
	
	
    while (1)
    {				
			if(T1s ==1){
				T1s = 0;
				add_time(0, 0, 1);
				time_str();
				
				UARTStringPutNOBlocking(UART0_BASE, (uint8_t*)"现在是中华人民共和国北京时间");
				UARTStringPutNOBlocking(UART0_BASE, (uint8_t*)time_now);
				UARTStringPutNOBlocking(UART0_BASE, (uint8_t*)"\n");
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
// 函数原型：void UARTStringPut(uint32_t ui32Base,const char *cMessage)
// 函数功能：向UART模块发送字符串
// 函数参数：ui32Base：UART模块
//          cMessage：待发送字符串  
// 函数返回值：无
//
//*****************************************************************************
void UARTStringPutNOBlocking(uint32_t ui32Base,uint8_t *cMessage)
{
    bool TXFIFO_free = 0;
    
	while(*cMessage != '\0')
    {
        TXFIFO_free = UARTCharPutNonBlocking(UART0_BASE, *(cMessage));
        if(TXFIFO_free) 
        {  
            cMessage++;
        }
        TXFIFO_free = 0;
    }
}

//*****************************************************************************
//
// 函数原型：void UARTInit(void)
// 函数功能：UART初始化。使能UART0，设置PA0,PA1为UART0 RX,TX引脚；
//          设置波特率及帧格式。
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void UARTInit(void)
{
    // 引脚配置
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);        // 使能UART0模块   
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);        // 使能端口 A
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));	// 等待端口 A准备完毕 

    GPIOPinConfigure(GPIO_PA0_U0RX);				  // 设置PA0为UART0 RX引脚
    GPIOPinConfigure(GPIO_PA1_U0TX);    			  // 设置PA1为UART0 TX引脚        

	// 设置端口 A的第0,1位（PA0,PA1）为UART引脚
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);   

    // 波特率及帧格式设置
    UARTConfigSetExpClk(UART0_BASE, 
	                    ui32SysClock,
	                    300,                  // 波特率：115200
	                    (UART_CONFIG_WLEN_8 |    // 数据位：8
	                     UART_CONFIG_STOP_ONE |  // 停止位：1
	                     UART_CONFIG_PAR_NONE)); // 校验位：无
	
    
    UARTFIFOLevelSet(UART0_BASE,UART_FIFO_TX1_8,UART_FIFO_RX7_8);
    IntEnable(INT_UART0); // UART0 中断允许
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT); // 使能UART0 RX,RT 中断
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
    UARTInit();             // UART初始化 
    IntMasterEnable();		// 总中断允许
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
	if(T1_count < V_T1s){
		++T1_count;
	} else {
		T1_count = 0;
		T1s = 1;
	}
	
	digit[4] = time_h/10;
	digit[5] = time_h%10;
	digit[6] = time_c/10;
	digit[7] = time_c%10;
	digit[0] = time_s/10;
	digit[1] = time_s%10;
		
	// 刷新全部数码管和LED指示灯
	TM1638_RefreshDIGIandLED(digit, pnt, led);

	// 检查当前键盘输入，0代表无键操作，1-9表示有对应按键
	// 键号显示在一位数码管上
	key_code = TM1638_Readkeyboard();

	// 按键操作在SysTick中断服务程序中的状态转移处理程序
	
	
}

//*****************************************************************************
// 
// 函数原型：void UART0_Handler(void)
// 函数功能：UART0中断服务程序
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void UART0_Handler(void)
{
    int32_t uart0_int_status;
    uint8_t uart_receive_char;
    volatile uint32_t i;
    
    uart0_int_status = UARTIntStatus(UART0_BASE, true); // 取中断状态
    UARTIntClear(UART0_BASE, uart0_int_status); // 清中断标志
  
    while(UARTCharsAvail(UART0_BASE)) // 重复从接收FIFO 读取字符
    {
        uart_receive_char = UARTCharGetNonBlocking(UART0_BASE); // 读入一个字符
        
        if(uart_receive_char != '\r'){ // 命令未结束
            uart_receive_buffer[uart_receive_len] = uart_receive_char;
						++uart_receive_len;
        } else {
            cmd_state = 1;
            uart_receive_buffer[uart_receive_len] = '\0';
        }
    }        
} 

bool match1(uint8_t *str){//GET
	uint8_t i = 0;
	for(; i < 6; ++i){
		if(str[i] != AT_GET[i]) return false;
	}
	
	
	if(str[7] == '\0') return true;
	return false;
}


bool match2(uint8_t *str){//SET
	uint8_t i = 0;
	for(; i < 6; ++i){
		if(str[i] != AT_SET[i]) return false;
	}
	
	if(str[8] == ':' && str[11] == ':' && str[15] == '\0'){
		return true;
	}
	
	return false;
}

bool match3(uint8_t *str){//INC
	uint8_t i = 0;
	for(; i < 6; ++i){
		if(str[i] != AT_INC[i]) return false;
	}
	
	if(str[8] == ':' && str[11] == ':' && str[15] == '\0'){
		return true;
	}
	
	return false;
}


void add_time(uint8_t h, uint8_t c, uint8_t s){
	time_s += s;
	if(time_s >= 60){
		time_s -= 60;
		c++;
	}
	
	time_c += c;
	if(time_c >= 60){
		time_c -= 60;
		h++;
	}
	
	time_h += h;
	if(time_h >= 24){
		time_c -= 24;
	}
}



void time_str(void){
	time_now[0] = time_h/10 + '0';
	time_now[1] = time_h%10 + '0';
	time_now[3] = time_c/10 + '0';
	time_now[4] = time_c%10 + '0';
	time_now[6] = time_s/10 + '0';
	time_now[7] = time_s%10 + '0';
}


