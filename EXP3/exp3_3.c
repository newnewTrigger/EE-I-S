//*****************************************************************************
//
// Copyright: 2020-2023, �Ϻ���ͨ��ѧ���ӹ���ϵʵ���ѧ����
// File name: exp3_2DEMO.c
// Description: 
//    ����������\rΪ������
//    ��UART0�ж��У����սϳ������ ���������з��ظ����⴮�� ��
// 
// Author:	�Ϻ���ͨ��ѧ���ӹ���ϵʵ���ѧ����
// Version: 1.0.0.20230503 
// Date��2023-05
// History��
//
//*****************************************************************************

//*****************************************************************************
//
// ͷ�ļ�
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"        // ��ַ�궨��
#include "inc/hw_types.h"         // �������ͺ궨�壬�Ĵ������ʺ���
#include "inc/hw_ints.h"          // ���ж��йصĺ궨��
#include "driverlib/debug.h"      // ������
#include "driverlib/gpio.h"       // ͨ��IO�ں궨��ͺ���ԭ��
#include "driverlib/pin_map.h"    // TM4Cϵ��MCU��Χ�豸�ܽź궨��
#include "driverlib/sysctl.h"	  // ϵͳ���ƶ���
#include "driverlib/systick.h"    // SysTick Driver ԭ��
#include "driverlib/interrupt.h"  // NVIC Interrupt Controller Driver ԭ��
#include "driverlib/uart.h"       // ��UART�йصĺ궨��ͺ���ԭ��

#include "tm1638.h"               // �����TM1638оƬ�йصĺ궨��ͺ���ԭ��

//*****************************************************************************
//
// �궨��
//
//*****************************************************************************
#define SYSTICK_FREQUENCY		50		// SysTickƵ��Ϊ50Hz����ѭ����ʱ����20ms

#define V_T1s	50                  // 1s�����ʱ�����ֵ��50��20ms

//*****************************************************************************
//
// ����ԭ������
//
//*****************************************************************************
void GPIOInit(void);        // GPIO��ʼ��
void SysTickInit(void);     // ����SysTick�ж�
void UARTInit(void);        // UART��ʼ��
void DevicesInit(void);     // MCU������ʼ����ע���������������
void UARTStringPutNOBlocking(uint32_t ui32Base,uint8_t *cMessage);// ��UART�����ַ���
bool match1(uint8_t *str);
bool match2(uint8_t *str);
bool match3(uint8_t *str);
void add_time(uint8_t h, uint8_t c, uint8_t s);
void time_str(void);

//*****************************************************************************
//
// ��������
//
//*****************************************************************************

// 8λ�������ʾ�����ֻ���ĸ����
// ע����������λ�������������Ϊ4��5��6��7��0��1��2��3
uint8_t digit[8]={0,0,' ',' ',0,0,0,0};

// 8λС���� 1��  0��
// ע����������λС����������������Ϊ4��5��6��7��0��1��2��3
uint8_t pnt = 0xA0;

// 8��LEDָʾ��״̬��0��1��
// ע������ָʾ�ƴ������������Ϊ7��6��5��4��3��2��1��0
//     ��ӦԪ��LED8��LED7��LED6��LED5��LED4��LED3��LED2��LED1
uint8_t led[] = {0, 0, 0, 0, 0, 0, 0, 1};

// ��ǰ����ֵ
volatile uint8_t key_code = 0;

// ���ڼ�¼ǰһ�ΰ������ʱ�ļ���״̬��0��ʾ�޼����£�1�м�����
volatile uint8_t key_state = 0;

// ��¼ϵͳ����״̬
volatile uint8_t  run_state = 0;

// ϵͳʱ��Ƶ�� 
uint32_t ui32SysClock;

//�洢����
uint8_t uart_receive_buffer[200] = {'\0'};
uint8_t uart_receive_len = 0;

//�����Ƿ������ϣ�����\r��1��
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
// ������
//
//*****************************************************************************
int main(void)
{

    DevicesInit();            //  MCU������ʼ��
	
    SysCtlDelay(60 * ( ui32SysClock / 3000)); // ��ʱ>60ms,�ȴ�TM1638�ϵ����
    TM1638_Init();	          // ��ʼ��TM1638
	
    UARTStringPutNOBlocking(UART0_BASE, (uint8_t*)"TEST\r\n");
	
	
	
    while (1)
    {				
			if(T1s ==1){
				T1s = 0;
				add_time(0, 0, 1);
				time_str();
				
				UARTStringPutNOBlocking(UART0_BASE, (uint8_t*)"�������л����񹲺͹�����ʱ��");
				UARTStringPutNOBlocking(UART0_BASE, (uint8_t*)time_now);
				UARTStringPutNOBlocking(UART0_BASE, (uint8_t*)"\n");
			}
			
      
    }	
}

//*****************************************************************************
//
// ����ԭ�ͣ�void GPIOInit(void)
// �������ܣ�GPIO��ʼ����ʹ��PortK������PK4,PK5Ϊ�����ʹ��PortM������PM0Ϊ�����
//          ��PK4����TM1638��STB��PK5����TM1638��DIO��PM0����TM1638��CLK��
// ������������
// ��������ֵ����
//
//*****************************************************************************
void GPIOInit(void)
{
	//����TM1638оƬ�ܽ�
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);				// ʹ�ܶ˿� K	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK)){};		// �ȴ��˿� K׼�����		
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);				// ʹ�ܶ˿� M	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM)){};		// �ȴ��˿� M׼�����		
	
    // ���ö˿� K�ĵ�4,5λ��PK4,PK5��Ϊ�������		PK4-STB  PK5-DIO
	GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_4|GPIO_PIN_5);
	// ���ö˿� M�ĵ�0λ��PM0��Ϊ�������   PM0-CLK
	GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_0);	
}

//*****************************************************************************
// 
// ����ԭ�ͣ�SysTickInit(void)
// �������ܣ�����SysTick�ж�
// ������������
// ��������ֵ����
//
//*****************************************************************************
void SysTickInit(void)
{
	SysTickPeriodSet(ui32SysClock/SYSTICK_FREQUENCY); // ������������,��ʱ����20ms
	SysTickEnable();  			// SysTickʹ��
	SysTickIntEnable();			// SysTick�ж�����
}

//*****************************************************************************
//
// ����ԭ�ͣ�void UARTStringPut(uint32_t ui32Base,const char *cMessage)
// �������ܣ���UARTģ�鷢���ַ���
// ����������ui32Base��UARTģ��
//          cMessage���������ַ���  
// ��������ֵ����
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
// ����ԭ�ͣ�void UARTInit(void)
// �������ܣ�UART��ʼ����ʹ��UART0������PA0,PA1ΪUART0 RX,TX���ţ�
//          ���ò����ʼ�֡��ʽ��
// ������������
// ��������ֵ����
//
//*****************************************************************************
void UARTInit(void)
{
    // ��������
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);        // ʹ��UART0ģ��   
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);        // ʹ�ܶ˿� A
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));	// �ȴ��˿� A׼����� 

    GPIOPinConfigure(GPIO_PA0_U0RX);				  // ����PA0ΪUART0 RX����
    GPIOPinConfigure(GPIO_PA1_U0TX);    			  // ����PA1ΪUART0 TX����        

	// ���ö˿� A�ĵ�0,1λ��PA0,PA1��ΪUART����
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);   

    // �����ʼ�֡��ʽ����
    UARTConfigSetExpClk(UART0_BASE, 
	                    ui32SysClock,
	                    300,                  // �����ʣ�115200
	                    (UART_CONFIG_WLEN_8 |    // ����λ��8
	                     UART_CONFIG_STOP_ONE |  // ֹͣλ��1
	                     UART_CONFIG_PAR_NONE)); // У��λ����
	
    
    UARTFIFOLevelSet(UART0_BASE,UART_FIFO_TX1_8,UART_FIFO_RX7_8);
    IntEnable(INT_UART0); // UART0 �ж�����
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT); // ʹ��UART0 RX,RT �ж�
}

//*****************************************************************************
// 
// ����ԭ�ͣ�DevicesInit(void)
// �������ܣ�CU������ʼ��������ϵͳʱ�����á�GPIO��ʼ����SysTick�ж�����
// ������������
// ��������ֵ����
//
//*****************************************************************************
void DevicesInit(void)
{
	// ʹ���ⲿ25MHz��ʱ��Դ������PLL��Ȼ���ƵΪ20MHz
	ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN | 
	                                   SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 
	                                   20000000);

	GPIOInit();             // GPIO��ʼ��
    SysTickInit();          // ����SysTick�ж�
    UARTInit();             // UART��ʼ�� 
    IntMasterEnable();		// ���ж�����
}

//*****************************************************************************
// 
// ����ԭ�ͣ�void SysTick_Handler(void)
// �������ܣ�SysTick�жϷ������
// ������������
// ��������ֵ����
//
//*****************************************************************************
void SysTick_Handler(void)       // ��ʱ����Ϊ20ms
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
		
	// ˢ��ȫ������ܺ�LEDָʾ��
	TM1638_RefreshDIGIandLED(digit, pnt, led);

	// ��鵱ǰ�������룬0�����޼�������1-9��ʾ�ж�Ӧ����
	// ������ʾ��һλ�������
	key_code = TM1638_Readkeyboard();

	// ����������SysTick�жϷ�������е�״̬ת�ƴ������
	
	
}

//*****************************************************************************
// 
// ����ԭ�ͣ�void UART0_Handler(void)
// �������ܣ�UART0�жϷ������
// ������������
// ��������ֵ����
//
//*****************************************************************************
void UART0_Handler(void)
{
    int32_t uart0_int_status;
    uint8_t uart_receive_char;
    volatile uint32_t i;
    
    uart0_int_status = UARTIntStatus(UART0_BASE, true); // ȡ�ж�״̬
    UARTIntClear(UART0_BASE, uart0_int_status); // ���жϱ�־
  
    while(UARTCharsAvail(UART0_BASE)) // �ظ��ӽ���FIFO ��ȡ�ַ�
    {
        uart_receive_char = UARTCharGetNonBlocking(UART0_BASE); // ����һ���ַ�
        
        if(uart_receive_char != '\r'){ // ����δ����
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


