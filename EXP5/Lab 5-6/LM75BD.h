//*****************************************************************************
//
// LM75BD.h - Prototypes for the LM75BD driver.
//
// Copyright��2020-2021,�Ϻ���ͨ��ѧ���ӹ���ϵʵ���ѧ����
// 
// Author: �Ϻ���ͨ��ѧ���ӹ���ϵʵ���ѧ����
// Version: 1.0.0.20210513 
// Date��2021-05-13
// History��
//
//*****************************************************************************

#ifndef __LM75BD_H__
#define __LM75BD_H__

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"        // ��ַ�궨��
#include "inc/hw_types.h"         // �������ͺ궨�壬�Ĵ������ʺ���
#include "inc/hw_i2c.h"
#include "driverlib/debug.h"      // ������
#include "driverlib/gpio.h"       // ͨ��IO�ں궨��
#include "driverlib/pin_map.h"    // TM4Cϵ��MCU��Χ�豸�ܽź궨��
#include "driverlib/sysctl.h"	  // ϵͳ���ƶ���
#include "driverlib/i2c.h"
#include "driverlib/fpu.h"

// LM75BD�ĵ�ַ��1 0 0 1 A2 A1 A0
#define LM75BD_ADR1   0x48     // A2 A1 A0 = 0 0 0    
#define LM75BD_ADR2   0x4F     // A2 A1 A0 = 1 1 1 

//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************
// ��ʼ��I2C0	
extern void I2C0Init(void);

// ʱ�Ӻ���
extern void Delay(uint32_t value);

// ��ȡLM75BD�¶�ֵ
extern int16_t GetTemputerature(uint8_t addr);
//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __LM75BD_H__
