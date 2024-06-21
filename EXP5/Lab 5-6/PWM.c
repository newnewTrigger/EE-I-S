//*****************************************************************************
//
// PWM.c - API for PWM.
//
// Copyrightï¿½ï¿½2020-2021,ï¿½Ïºï¿½ï¿½ï¿½Í¨ï¿½ï¿½Ñ§ï¿½ï¿½ï¿½Ó¹ï¿½ï¿½ï¿½ÏµÊµï¿½ï¿½ï¿½Ñ§ï¿½ï¿½ï¿½ï¿?
// 
// Author:	ï¿½Ïºï¿½ï¿½ï¿½Í¨ï¿½ï¿½Ñ§ï¿½ï¿½ï¿½Ó¹ï¿½ï¿½ï¿½ÏµÊµï¿½ï¿½ï¿½Ñ§ï¿½ï¿½ï¿½ï¿?
// Version: 1.0.0.20210508 
// Dateï¿½ï¿½2021-05-08
// Historyï¿½ï¿½
//
//*****************************************************************************

#include "PWM.h"
#include "driverlib/pin_map.h"


extern uint32_t g_ui32SysClock;    // ÏµÍ³Ê±ï¿½ï¿½


void PWMInit()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);     // PWM0Ê¹ï¿½ï¿½ 
    
    PWMOutputState(PWM0_BASE, PWM_OUT_4_BIT, true); // Ê¹ÄÜ(ÔÊÐí)PWM0_4µÄÊä³ö
    PWMOutputState(PWM0_BASE, PWM_OUT_3_BIT, true); // Ê¹ï¿½ï¿½(ï¿½ï¿½ï¿½ï¿½)PWM0_4ï¿½ï¿½ï¿½ï¿½ï¿?
    //PWMGenEnable(PWM0_BASE, PWM_GEN_1);             //Ê¹ï¿½ï¿½PWM0Ä£ï¿½ï¿½ï¿?2ï¿½Å·ï¿½ï¿½ï¿½ï¿½ï¿½(ï¿½ï¿½Îª4ï¿½ï¿½PWMï¿½ï¿½2ï¿½Å·ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½)
    //PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, g_ui32SysClock / ui32Freq_Hz); // ï¿½ï¿½ï¿½ï¿½Freq_Hzï¿½ï¿½ï¿½ï¿½PWMï¿½ï¿½ï¿½ï¿½
   
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);    // Ê¹ÄÜGPIOG
    GPIOPinConfigure(GPIO_PG0_M0PWM4);              // ÅäÖÃÒý½Å¸´ÓÃ
    GPIOPinTypePWM(GPIO_PORTG_BASE, GPIO_PIN_0);    // Òý½ÅÓ³Éä
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);    // Ê¹ï¿½ï¿½GPIOG
    GPIOPinConfigure(GPIO_PF3_M0PWM3);              // ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Å¸ï¿½ï¿½ï¿½
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_3);    // ï¿½ï¿½ï¿½ï¿½Ó³ï¿½ï¿½
    
    PWMGenConfigure(PWM0_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);   //ï¿½ï¿½ï¿½ï¿½PWMï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
    //PWMPulseWidthSet(PWM0_BASE, PWM_OUT_7,(PWMGenPeriodGet(PWM0_BASE, PWM_GEN_1)/ 2)); //ï¿½ï¿½ï¿½ï¿½Õ¼ï¿½Õ±ï¿½Îª50%
}


void PWMStart(uint32_t ui32Freq_Hz)
{
    PWMGenDisable(PWM0_BASE, PWM_GEN_2);
    PWMGenDisable(PWM0_BASE, PWM_GEN_1);     //Ê¹ï¿½ï¿½PWM0Ä£ï¿½ï¿½ï¿?2ï¿½Å·ï¿½ï¿½ï¿½ï¿½ï¿½(ï¿½ï¿½Îª4ï¿½ï¿½PWMï¿½ï¿½2ï¿½Å·ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½)   
    
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, g_ui32SysClock / ui32Freq_Hz); // ¸ù¾ÝFreq_HzÉèÖÃPWMÖÜÆÚ
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4,(PWMGenPeriodGet(PWM0_BASE, PWM_GEN_2)/ 2));
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, g_ui32SysClock / ui32Freq_Hz); // ï¿½ï¿½ï¿½ï¿½Freq_Hzï¿½ï¿½ï¿½ï¿½PWMï¿½ï¿½ï¿½ï¿½
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3,(PWMGenPeriodGet(PWM0_BASE, PWM_GEN_1)/ 2)); //ï¿½ï¿½ï¿½ï¿½Õ¼ï¿½Õ±ï¿½Îª50%
    
    PWMGenEnable(PWM0_BASE, PWM_GEN_2);
    PWMGenEnable(PWM0_BASE, PWM_GEN_1);     //Ê¹ï¿½ï¿½PWM0Ä£ï¿½ï¿½ï¿?2ï¿½Å·ï¿½ï¿½ï¿½ï¿½ï¿½(ï¿½ï¿½Îª4ï¿½ï¿½PWMï¿½ï¿½2ï¿½Å·ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½)   
}

void PWMStart2(uint32_t ui32Freq_Hz)
{
    PWMGenDisable(PWM0_BASE, PWM_GEN_2);
    
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, g_ui32SysClock / ui32Freq_Hz); // ¸ù¾ÝFreq_HzÉèÖÃPWMÖÜÆÚ
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4,(PWMGenPeriodGet(PWM0_BASE, PWM_GEN_2)/ 2));
    
    PWMGenEnable(PWM0_BASE, PWM_GEN_2);
}

//*******************************************************************************************************
// 
// ï¿½ï¿½ï¿½ï¿½Ô­ï¿½Í£ï¿½void PWMStop()
// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ü£ï¿½M0PWM4(PG0)Í£Ö¹ï¿½ï¿½ï¿½ï¿½PWMï¿½Åºï¿½
// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Öµï¿½ï¿½ï¿½ï¿½
//
//*******************************************************************************************************
void PWMStop()
{
    PWMGenDisable(PWM0_BASE, PWM_GEN_2);
    PWMGenDisable(PWM0_BASE, PWM_GEN_1);   // M0PWM4(PG0)Í£Ö¹ï¿½ï¿½ï¿½ï¿½PWMï¿½Åºï¿½
}

void PWMStop1()
{
    PWMGenDisable(PWM0_BASE, PWM_GEN_1);   // M0PWM4(PG0)Í£Ö¹ï¿½ï¿½ï¿½ï¿½PWMï¿½Åºï¿½
}
