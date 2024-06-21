//*****************************************************************************
//
// LM75BD.c - API for LM75BD.
//
// Copyright��2020-2021,�Ϻ���ͨ��ѧ���ӹ���ϵʵ���ѧ����
// 
// Author:	�Ϻ���ͨ��ѧ���ӹ���ϵʵ���ѧ����
// Version: 1.0.0.20210513 
// Date��2021-05-13
// History��
//
//*****************************************************************************

#include "LM75BD.h"
extern uint32_t ui32SysClock;    // ϵͳʱ��


//******************************************************************************************
// 
// ����ԭ�ͣ�void I2C0Init(void)
// �������ܣ�I2C0��ʼ����PB2-I2C0SCL��PB3-I2C0SDA��
// ������������
// ��������ֵ����
//
//******************************************************************************************
void I2C0Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	GPIOPinConfigure(GPIO_PB3_I2C0SDA);
    GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

    I2CMasterInitExpClk(I2C0_BASE,ui32SysClock, true);		//config I2C0 400k
	I2CMasterEnable(I2C0_BASE);	
}

//******************************************************************************************
// 
// ����ԭ�ͣ�void Delay(uint32_t value)
// �������ܣ�ʱ�Ӻ���
// ����������uint32_t value��ѭ������
// ��������ֵ����
//
//******************************************************************************************
void Delay(uint32_t value)
{
	uint32_t ui32Loop;
	for(ui32Loop = 0; ui32Loop < value; ui32Loop++){};
}

//******************************************************************************************
// 
// ����ԭ�ͣ�int16_t GetTemputerature(uint8_t adr)
// �������ܣ���ȡLM75BD�¶�ֵ
// ����������uint8_t addr��LM75BD��I2C��ַ
// ��������ֵ����ȡ���¶�ֵ�� 1LSB=0.1�棩
//
//******************************************************************************************
int16_t GetTemputerature(uint8_t addr)
{	
    
    uint16_t ui16Temputerature;  //�¶�ֵ
	
    float Temputerature;         // ʮ�����¶�ֵ
    
    uint16_t Temp_LSB;    // ����¶ȵĵ��ֽڣ���3λ��Ч 
    uint16_t Temp_MSB;    // ����¶ȵĸ��ֽ�
    
    I2CMasterSlaveAddrSet(I2C0_BASE, addr, true);  // ����Slave��ַ
    while(I2CMasterBusy(I2C0_BASE)) {};    

	I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_BURST_RECEIVE_START);

    while(I2CMasterBusy(I2C0_BASE)) {};    
    
    if(I2CMasterErr(I2C0_BASE) == I2C_MASTER_ERR_NONE)
    Temp_MSB = I2CMasterDataGet(I2C0_BASE);  // ��ȡ�¶ȵĸ��ֽ�
     
    I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
  	while(I2CMasterBusy(I2C0_BASE)){};
    if(I2CMasterErr(I2C0_BASE) == I2C_MASTER_ERR_NONE)                   
    Temp_LSB = I2CMasterDataGet(I2C0_BASE);  // ��ȡ�¶ȵĵ��ֽ�,��3λ��Ч
  
    Delay(100);    // �ʵ��ӳ٣�����ʵ������������е���
    
	ui16Temputerature = (Temp_MSB << 8 ) + Temp_LSB;

	// ȡ�¶�������
	ui16Temputerature = ui16Temputerature >> 5; // 1LSB=0.125��
	
	if(ui16Temputerature & 0x0400)  // ���¶�
		Temputerature = -((~(ui16Temputerature & 0x03FF ) + 1) & 0x03FF) * 0.125;
	else  // ���¶� 
		Temputerature = 0.125 * ui16Temputerature;
    
    return ((int16_t)(Temputerature * 10));
}

