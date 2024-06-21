//单界面菜单示例，反白表示游标位置
//可由底板3*3按键控制
//   ⑴   ⑵   ⑶
//   ⑷   ⑸   ⑹
//   ⑺   ⑻   ⑼
// ⑸表示enter
// ⑷⑹控制光标位置
//
//  ————————————
//  |                      |
//  |    工作模式: 模式A    |
//  |    工作参数: 1.1Hz    |
//  |                      |
//  ————————————

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h" 
#include "inc/hw_types.h" 
#include "driverlib/pin_map.h"  
#include "driverlib/sysctl.h"	
#include "JLX12864G.h"

#define KEYTMR_OF  10
extern uint8_t key_code;
extern uint8_t NOKEY_clock10s_flag;

unsigned int opmode=0;
int opmode_counter=0;

int freq1 = 1;
int freq2 = 1;

struct struct_act
{
	unsigned char num;
	unsigned char *str[20];
	unsigned char x[20],y[20],inverse[20];
} ;

//////////////////////////////
//工作模式设置
struct struct_act a0;

unsigned char stringopmode_A[]="模式A";
unsigned char stringopmode_B[]="模式B";
unsigned char stringopmode_C[]="模式C";

struct struct_act *act[1];                     

unsigned char a0_s0[]="工作模式:";
unsigned char *a0_s1;

//工作参数设置
unsigned char freq_1[] = "1";
unsigned char freq_2[] = "1";

struct struct_act *freq[1];

unsigned char a0_s2[] = "工作参数:";
unsigned char *a0_s3;
unsigned char *a0_s5;

unsigned char a0_s4[]=".";
unsigned char a0_s6[]="Hz";
//////////////////////////////

unsigned int ui_state=0;  //状态号

unsigned int key_ENTER_state=0;
unsigned int key_ENTER_prestate=0;
unsigned int ENTER_key_timer=0;
unsigned int key_ENTER_flag=0; 

unsigned int key_DOWN_state=0;
unsigned int key_DOWN_prestate=0;
unsigned int key_DOWN_timer=0;
unsigned int key_DOWN_flag=0; 

unsigned int key_UP_state=0;
unsigned int key_UP_prestate=0;
unsigned int key_UP_timer=0;
unsigned int key_UP_flag=0; 

unsigned int key_INCREASE_state=0;
unsigned int key_INCREASE_prestate=0;
unsigned int key_INCREASE_timer=0;
unsigned int key_INCREASE_flag=0; 

unsigned int key_DECREASE_state=0;
unsigned int key_DECREASE_prestate=0;
unsigned int key_DECREASE_timer=0;
unsigned int key_DECREASE_flag=0; 


void in_de_opmode(unsigned int opmode)//opmode工作模式增减
{
		switch (opmode_counter)
		{
			case 0: act[0]->str[1]= stringopmode_A;break;
			case 1: act[0]->str[1]= stringopmode_B;break;
			case 2: act[0]->str[1]= stringopmode_C;break;						
			default: break;	 
		} 	 
}

void itoaopmode (int opmode, unsigned char **instropmode)//将后台opmode工作模式转换为lcd屏幕数据
{
	switch (opmode)
	{
		case 0: *instropmode = stringopmode_A;break;
		case 1: *instropmode = stringopmode_B;break;
		case 2: *instropmode = stringopmode_C;break;
		default: break;
	}
}

void itoafrequency (int freq1, int freq2, unsigned char **instrfrequency1, unsigned char **instrfrequency2)//将后台opmode工作模式转换为lcd屏幕数据
{
	freq_1[0] = '0' + freq1;
	freq_2[0] = '0' + freq2;
	*instrfrequency1 = freq_1;
	*instrfrequency2 = freq_2;
}



void ENTER_detect(void)
{
	switch(key_ENTER_state)
	{
		case 0:
			if(key_code==5)
			{key_ENTER_state=1; key_ENTER_flag=1;} break;
		case 1:
			if (key_code!=5)
			{key_ENTER_state=0;} break;
		default: {key_ENTER_state=0;} break;
			
	}
}

void DOWN_detect(void)
{
	if (key_code==6) ///////////////////	 DOWN
	{
		key_DOWN_prestate=key_DOWN_state;		
		key_DOWN_state=0;
		if (key_DOWN_prestate==1) key_DOWN_flag=1;
		
	}
	else
	{
		key_DOWN_prestate = key_DOWN_state; 
		key_DOWN_state=1;	
	}

}


void UP_detect(void)
{
	switch(key_UP_state)
	{
		case 0:
			if(key_code==4)
			{key_UP_state=1; key_UP_flag=1;} break;
		case 1:
			if (key_code!=4)
			{key_UP_state=0;} break;
		default: {key_UP_state=0;} break;
			
	}
}


void INCREASE_detect(void)
{
	if (key_code==2) ///////////////////	 INCREASE	
	{
		key_INCREASE_prestate=key_INCREASE_state;		
		key_INCREASE_state=0;
		if (key_INCREASE_prestate==1) 
		{	key_INCREASE_flag=1;	key_INCREASE_timer =0;	}
		else if (key_INCREASE_prestate==0)
		{
			if 	(++key_INCREASE_timer>=KEYTMR_OF)
			{ key_INCREASE_flag=1; key_INCREASE_timer=0;}  
		}
	}
	else
	{
		key_INCREASE_prestate = key_INCREASE_state; 
		key_INCREASE_state=1;
		key_INCREASE_timer=0;	
	}
}

void DECREASE_detect(void)
{
	if (key_code==8) ///////////////////	 DECREASE	
	{
		key_DECREASE_prestate=key_DECREASE_state;		
		key_DECREASE_state=0;
		if (key_DECREASE_prestate==1) 
		{	key_DECREASE_flag=1;	key_DECREASE_timer =0;	}
		else if (key_DECREASE_prestate==0)
		{
			if 	(++key_DECREASE_timer>=KEYTMR_OF)
			{ key_DECREASE_flag=1; key_DECREASE_timer=0;}  
		}
	}
	else
	{
		key_DECREASE_prestate = key_DECREASE_state; 
		key_DECREASE_state=1;
		key_DECREASE_timer=0;	
	}
}


void display_ui_act(unsigned int i)
{		
	unsigned int j=0;
	clear_screen();

	for (j=0;j<act[i]->num;j++) 
	{
		display_GB2312_string(act[i]->x[j],(act[i]->y[j]-1)*8+1,act[i]->str[j],act[i]->inverse[j]);		
	}
}



void initial_act(void)
{
	itoaopmode(opmode_counter,&a0_s1);

//State: a0
	a0.num=7;
	a0.str[0]=a0_s0; a0.x[0]=3;  a0.y[0]=1;  a0.inverse[0]=0; 
	a0.str[1]=a0_s1; a0.x[1]=3;  a0.y[1]=11;  a0.inverse[1]=0;		///////act0_1
	

	itoafrequency(freq1, freq2, &a0_s3, &a0_s5);

	a0.str[2]=a0_s2; a0.x[2]=5;  a0.y[2]=1;  a0.inverse[2]=0; 
	a0.str[3]=a0_s3; a0.x[3]=5;  a0.y[3]=11;  a0.inverse[3]=0;		///////act0_3
	a0.str[4]=a0_s4; a0.x[4]=5;  a0.y[4]=12;  a0.inverse[4]=0;
	a0.str[5]=a0_s5; a0.x[5]=5;  a0.y[5]=13;  a0.inverse[5]=0;      ///////act0_5
	a0.str[6]=a0_s6; a0.x[6]=5;  a0.y[6]=14;  a0.inverse[6]=0;

	act[0]=&a0;

	display_ui_act(0);
}

void ui_proc0(void)
{

	if(key_code!=0)
	{
		key_UP_flag=0;key_DOWN_flag=0;key_INCREASE_flag=0;key_DECREASE_flag=0;key_ENTER_flag=0;
		act[0]->inverse[1]=1;
		display_GB2312_string(act[0]->x[1],(act[0]->y[1]-1)*8+1,act[0]->str[1],act[0]->inverse[1]);
		ui_state=001;
	}

}

void ui_proc001(void)
{
	if(key_UP_flag)
	{
		act[0]->inverse[1]=0;
		act[0]->inverse[5]=1;
		key_UP_flag=0;
		display_ui_act(0);
		ui_state = 005;
	}
	else if (key_DOWN_flag)
	{
		act[0]->inverse[1]=0;
		act[0]->inverse[3]=1;
		key_DOWN_flag=0;
		display_ui_act(0);
		ui_state = 003;
	}
	else if (key_INCREASE_flag)
	{
		key_INCREASE_flag=0;
		opmode_counter++;
		if (opmode_counter>2) opmode_counter=0;
		in_de_opmode(opmode_counter);
		display_ui_act(0);
		ui_state=001;
	}
		else if (key_DECREASE_flag)
	{
		key_DECREASE_flag=0;
		opmode_counter--;
		if (opmode_counter<0) opmode_counter=2;
		in_de_opmode(opmode_counter);
		display_ui_act(0);
		ui_state=001;
	}

	else if(key_ENTER_flag)
	{
		key_ENTER_flag=0;
	}
	if(NOKEY_clock10s_flag==1)
	{
		NOKEY_clock10s_flag = 0;
		act[0]->inverse[1]=0;
		display_ui_act(0);
		ui_state=0;		
	}

}

void ui_proc003(void)
{
	if(key_UP_flag)
	{
		act[0]->inverse[3]=0;
		act[0]->inverse[1]=1;
		key_UP_flag=0;
		display_ui_act(0);
		ui_state = 001;
	}
	else if (key_DOWN_flag)
	{
		act[0]->inverse[3]=0;
		act[0]->inverse[5]=1;
		key_DOWN_flag=0;
		display_ui_act(0);
		ui_state = 005;
	}
	else if (key_INCREASE_flag)
	{
		key_INCREASE_flag=0;
		freq1++;
		if (freq1>9) freq1=0;
		itoafrequency(freq1, freq2, &a0_s3, &a0_s5);
		display_ui_act(0);
		ui_state=003;
	}
		else if (key_DECREASE_flag)
	{
		key_DECREASE_flag=0;
		freq1--;
		if (freq1<0) freq1=9;
		itoafrequency(freq1, freq2, &a0_s3, &a0_s5);
		display_ui_act(0);
		ui_state=003;
	}

	else if(key_ENTER_flag)
	{
		key_ENTER_flag=0;
	}
	if(NOKEY_clock10s_flag==1)
	{
		NOKEY_clock10s_flag = 0;
		act[0]->inverse[3]=0; 
		display_ui_act(0);
		ui_state=0;		
	}
}

void ui_proc005(void)
{
	if(key_UP_flag)
	{
		act[0]->inverse[5]=0;
		act[0]->inverse[3]=1;
		key_UP_flag=0;
		display_ui_act(0);
		ui_state = 003;
	}
	else if (key_DOWN_flag)
	{
		act[0]->inverse[5]=0;
		act[0]->inverse[1]=1;
		key_DOWN_flag=0;
		display_ui_act(0);
		ui_state = 001;
	}
	else if (key_INCREASE_flag)
	{
		key_INCREASE_flag=0;
		freq2++;
		if (freq2>9) freq2=0;
		itoafrequency(freq1, freq2, &a0_s3, &a0_s5);
		display_ui_act(0);
		ui_state=005;
	}
		else if (key_DECREASE_flag)
	{
		key_DECREASE_flag=0;
		freq2--;
		if (freq2<0) freq2=9;
		itoafrequency(freq1, freq2, &a0_s3, &a0_s5);
		display_ui_act(0);
		ui_state=005;
	}

	else if(key_ENTER_flag)
	{
		key_ENTER_flag=0;
	}
	if(NOKEY_clock10s_flag==1)
	{
		NOKEY_clock10s_flag = 0;
		act[0]->inverse[5]=0; 
		display_ui_act(0);
		ui_state=0;		
	}
}

void ui_state_proc(unsigned int ui_state)
{
	switch (ui_state)
		{
			case 0: ui_proc0(); break;
			case 001: ui_proc001(); break;
			case 003: ui_proc003(); break;
			case 005: ui_proc005(); break;

			default: break;
		}

}
