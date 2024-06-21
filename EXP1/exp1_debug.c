//*****************************************************************************
//
// Copyright: 2019-2020, II-
// File name: exp0.c
// Description: LED(D4-PF0)��1000����Ϊ���ڻ�����˸;
// ������USR-SW1��,LED(D4-PF0)��100����Ϊ���ڻ�����˸;
// �ɿ�USR-SW1��,LED(D4-PF0)�ָ���1000����Ϊ���ڻ�����˸
// Author: ??????????????? II-A ???
// Version: 1.0.0.20191230 
// Date:2019-12-30
// History:
//
//*****************************************************************************
//*****************************************************************************
//
// ͷ�ļ�
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h" // �����궨��
#include "inc/hw_types.h" // �������ͺ궨�壬�Ĵ������ʺ���
#include "driverlib/debug.h" // ������
#include "driverlib/gpio.h" // ͨ��IO�ں궨��
#include "driverlib/pin_map.h" // TM4Cϵ��MCU��Χ�豸�ܽź궨��
#include "driverlib/sysctl.h" // ϵͳ���ƺ궨��

#include "driverlib/uart.h"
#include "uartstdio.h"
//*****************************************************************************
//
// �궨��
//
//*****************************************************************************

#define MilliSecond 4000 // ?? 1ms ????????
#define FASTFLASHTIME 100 // ???(100ms)
#define SLOWFLASHTIME 1000 // ???(1000ms)
//*****************************************************************************
//
// ??????
//
//*****************************************************************************
void DelayMilliSec(uint32_t ui32DelaySecond); // ??????,?????
void GPIOInit(void); // GPIO ???
void PF0Flash(uint8_t ui8KeyValue); // ????????,?? PF0 ?????
//*****************************************************************************
//
// ???
//
//*****************************************************************************

uint32_t g_ui32SysClock; // System Clock


int main(void)
{
uint8_t ui8KeyValue;
	
g_ui32SysClock = SysCtlClockFreqSet(SYSCTL_OSC_INT | SYSCTL_USE_OSC, 16000000);

GPIOInit(); // GPIO ???
while(1) // ????
 {
// ?? PJ0 ?? 0-?? 1-??
ui8KeyValue = GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0); 
PF0Flash(ui8KeyValue); // ?????????,?? PF0 ?????
}
}
//*****************************************************************************
//
// ????:void DelayMilliSec(uint32_t ui32DelaySecond) 
// ????:??????,?????
// ????:ui32DelaySecond:?????
//
//*****************************************************************************
void DelayMilliSec(uint32_t ui32DelaySecond) 
{
uint32_t ui32Loop;
ui32DelaySecond = ui32DelaySecond * MilliSecond;
for(ui32Loop = 0; ui32Loop < ui32DelaySecond; ui32Loop++){ };
}
//*****************************************************************************
//
// ????:void GPIOInit(void)
// ????:GPIO ?????? PortF,?? PF0 ???;
// ?? PortJ,?? PJ0 ???
// ????:?
//
//*****************************************************************************
void GPIOInit(void)
{
SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // ʹ�� F
while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)); // ???? F ????
SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ); // ʹ�� J
while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)){}; // ???? J ????
// ???? F ?? 0 ?(PF0)?????
 GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0); 
// ???? J ?? 0 ?(PJ0)?????
GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE,GPIO_PIN_0);
// ?? J ?? 0 ???????,??????????
GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0,GPIO_STRENGTH_2MA,
 GPIO_PIN_TYPE_STD_WPU); 
}
//*****************************************************************************
// 
// ????:void PF0Flash(uint8_t ui8KeyValue)
// ????:????????,?? PF0 ??????0-??,1-??
// ????:ui8KeyValue:???
//
//*****************************************************************************
void PF0Flash(uint8_t ui8KeyValue)
{
 uint32_t ui32DelayTime;
	
if (ui8KeyValue == 0) // USR_SW1-PJ0 ??
ui32DelayTime = SLOWFLASHTIME;
else // USR_SW1-PJ0 ??
ui32DelayTime = FASTFLASHTIME
;
GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0); // ?? D4-PF0 LED
DelayMilliSec(ui32DelayTime); // ?? ui32DelayTime ??
GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x0); // ?? D4-PF0 LED
DelayMilliSec(ui32DelayTime); // ?? ui32DelayTime ??
}