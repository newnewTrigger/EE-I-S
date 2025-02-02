//*****************************************************************************
//
// Copyright: 2019-2020, II-
// File name: exp0.c
// Description: LED(D4-PF0)以1000毫秒为周期缓慢闪烁;
// 当按下USR-SW1键,LED(D4-PF0)以100毫秒为周期缓慢闪烁;
// 松开USR-SW1键,LED(D4-PF0)恢复以1000毫秒为周期缓慢闪烁
// Author: ??????????????? II-A ???
// Version: 1.0.0.20191230 
// Date:2019-12-30
// History:
//
//*****************************************************************************
//*****************************************************************************
//
// 头文件
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h" // 基础宏定义
#include "inc/hw_types.h" // 数据类型宏定义，寄存器访问函数
#include "driverlib/debug.h" // 调试用
#include "driverlib/gpio.h" // 通用IO口宏定义
#include "driverlib/pin_map.h" // TM4C系列MCU外围设备管脚宏定义
#include "driverlib/sysctl.h" // 系统控制宏定义

#include "driverlib/uart.h"
#include "uartstdio.h"
//*****************************************************************************
//
// 宏定义
//
//*****************************************************************************

//*****************************************************************************
void DelayMilliSec(uint32_t ui32DelaySecond); // ??????,?????
void GPIOInit(void); // GPIO ???
void PNonoff(uint8_t ui8KeyValue1, uint8_t ui8KeyValue2); // ????????,?? PF0 ?????
//*****************************************************************************
//
// ???
//
//*****************************************************************************
int main(void)
{
uint8_t ui8KeyValue1, ui8KeyValue2;
GPIOInit(); // GPIO ???
while(1) // ????
 {
// ?? PJ0 ?? 0-?? 1-??
ui8KeyValue1 = GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0);
ui8KeyValue2 = GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1); 	 
PNonoff(ui8KeyValue1, ui8KeyValue2); // ?????????,?? PF0 ?????
}
}
void GPIOInit(void)
{
SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION); // 使能 F
while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)); // ???? F ????
SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ); // 使能 J
while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)){}; // ???? J ????
// ???? F ?? 0 ?(PF0)?????
GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1); 
GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1);
// ?? J ?? 0 ???????,?????�????�
GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0,GPIO_STRENGTH_2MA,
 GPIO_PIN_TYPE_STD_WPU); 
GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_1,GPIO_STRENGTH_2MA,
 GPIO_PIN_TYPE_STD_WPU); 
}
//*****************************************************************************
// 
// ????:void PF0Flash(uint8_t ui8KeyValue)
// ????:????????,?? PF0 ??????0-??,1-??
// ????:ui8KeyValue:???
//
//*****************************************************************************
void PNonoff(uint8_t ui8KeyValue1, uint8_t ui8KeyValue2)
{
if (ui8KeyValue1 == 0) // USR_SW1-PJ0 ??
GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
else // USR_SW1-PJ0 ??
GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0x0);

if (ui8KeyValue2 == 0) // USR_SW2-PJ1 ??
GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);
else // USR_SW2-PJ1 ??
GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0x0); // ?? D4-PF0 LED
//DelayMilliSec(ui32DelayTime); // ?? ui32DelayTime ??
//DelayMilliSec(ui32DelayTime);
}