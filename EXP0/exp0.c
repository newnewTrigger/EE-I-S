//*****************************************************************************
//
// Copyright: 2019-2020, II-
// File name: exp0.c
// Description: LED(D4-PF0)ÒÔ1000ºÁÃëÎªÖÜÆÚ»ºÂýÉÁË¸;
// µ±°´ÏÂUSR-SW1¼ü,LED(D4-PF0)ÒÔ100ºÁÃëÎªÖÜÆÚ»ºÂýÉÁË¸;
// ËÉ¿ªUSR-SW1¼ü,LED(D4-PF0)»Ö¸´ÒÔ1000ºÁÃëÎªÖÜÆÚ»ºÂýÉÁË¸
// Author: ??????????????? II-A ???
// Version: 1.0.0.20191230 
// Date:2019-12-30
// History:
//
//*****************************************************************************
//*****************************************************************************
//
// Í·ÎÄ¼þ
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h" // »ù´¡ºê¶¨Òå
#include "inc/hw_types.h" // Êý¾ÝÀàÐÍºê¶¨Òå£¬¼Ä´æÆ÷·ÃÎÊº¯Êý
#include "driverlib/debug.h" // µ÷ÊÔÓÃ
#include "driverlib/gpio.h" // Í¨ÓÃIO¿Úºê¶¨Òå
#include "driverlib/pin_map.h" // TM4CÏµÁÐMCUÍâÎ§Éè±¸¹Ü½Åºê¶¨Òå
#include "driverlib/sysctl.h" // ÏµÍ³¿ØÖÆºê¶¨Òå

#include "driverlib/uart.h"
#include "uartstdio.h"
//*****************************************************************************
//
// ºê¶¨Òå
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
SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION); // Ê¹ÄÜ F
while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)); // ???? F ????
SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ); // Ê¹ÄÜ J
while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)){}; // ???? J ????
// ???? F ?? 0 ?(PF0)?????
GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1); 
GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1);
// ?? J ?? 0 ???????,?????“????”
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