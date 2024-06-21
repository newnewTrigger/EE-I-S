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
extern uint8_t NOKEY_clock5s_flag;


int opmode_counter_1=0;
int opmode_counter_2=0;

unsigned char freq_1[] = "1";
unsigned char freq_2[] = "1";

int freq0_1 = 1;
int freq0_2 = 1;

int freq3_1 = 1;
int freq3_2 = 1;

struct struct_act
{
	unsigned char num;
	unsigned char *str[20];
	unsigned char x[20],y[20],inverse[20];
} ;

//////////////////////////////
//State0
struct struct_act a0;
unsigned char *a0_s0;
unsigned char *a0_s1;
unsigned char a0_s2[] = ".";
unsigned char *a0_s3;
unsigned char a0_s4[] = "Hz";
unsigned char a0_s5[] = "设置";

//State1
struct struct_act a1;
unsigned char a1_s0[] = "工作模式:";
unsigned char a1_s1[] = "工作参数:";
unsigned char a1_s2[] = "返回";

//State2
struct struct_act a2;
unsigned char a2_s0[] = "工作模式:";
unsigned char *a2_s1;
unsigned char a2_s2[] = "确定";
unsigned char a2_s3[] = "取消";

//State3
struct struct_act a3;
unsigned char a3_s0[] = "工作参数:";
unsigned char *a3_s1;
unsigned char a3_s2[] = ".";
unsigned char *a3_s3;
unsigned char a3_s4[] = "Hz";
unsigned char a3_s5[] = "确定";
unsigned char a3_s6[] = "取消";

//State4
struct struct_act a4;
unsigned char a4_s0[] = "工作参数不合法";

unsigned char stringopmode_A[]="模式A";
unsigned char stringopmode_B[]="模式B";
unsigned char stringopmode_C[]="模式C";

struct struct_act *act[1];                     

//////////////////////////////

unsigned int act_state = 0;
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
//State: a0
	a0.num=6;

	itoaopmode(opmode_counter_1,&a0_s0);

	a0.str[0]=a0_s0; a0.x[0]=3;  a0.y[0]=3;  a0.inverse[0]=0; 

	itoafrequency(freq0_1, freq0_2, &a0_s1, &a0_s3);

	a0.str[1]=a0_s1; a0.x[1]=3;  a0.y[1]=11;  a0.inverse[1]=0;		
	a0.str[2]=a0_s2; a0.x[2]=3;  a0.y[2]=12;  a0.inverse[2]=0; 
	a0.str[3]=a0_s3; a0.x[3]=3;  a0.y[3]=13;  a0.inverse[3]=0;		
	a0.str[4]=a0_s4; a0.x[4]=3;  a0.y[4]=14;  a0.inverse[4]=0;
	a0.str[5]=a0_s5; a0.x[5]=7;  a0.y[5]=1;  a0.inverse[5]=0;      

//State: 1
	a1.num=3;

	a1.str[0]=a1_s0; a1.x[0]=3;  a1.y[0]=3;  a1.inverse[0]=1; 
	a1.str[1]=a1_s1; a1.x[1]=5;  a1.y[1]=3;  a1.inverse[1]=0;		
	a1.str[2]=a1_s2; a1.x[2]=7;  a1.y[2]=13;  a1.inverse[2]=0; 

//State: 2
	a2.num=4;

	a2.str[0]=a2_s0; a2.x[0]=3;  a2.y[0]=1;  a2.inverse[0]=0; 

	itoaopmode(opmode_counter_2,&a2_s1);

	a2.str[1]=a2_s1; a2.x[1]=3;  a2.y[1]=11;  a2.inverse[1]=1;		
	a2.str[2]=a2_s2; a2.x[2]=7;  a2.y[2]=1;  a2.inverse[2]=0; 
	a2.str[3]=a2_s3; a2.x[3]=7;  a2.y[3]=13;  a2.inverse[3]=0;	

//State: 3
	a3.num=7;

	a3.str[0]=a3_s0; a3.x[0]=3;  a3.y[0]=1;  a3.inverse[0]=0; 

	itoafrequency(freq3_1, freq3_2, &a3_s1, &a3_s3);

	a3.str[1]=a3_s1; a3.x[1]=3;  a3.y[1]=11;  a3.inverse[1]=1;		
	a3.str[2]=a3_s2; a3.x[2]=3;  a3.y[2]=12;  a3.inverse[2]=0; 
	a3.str[3]=a3_s3; a3.x[3]=3;  a3.y[3]=13;  a3.inverse[3]=0;		
	a3.str[4]=a3_s4; a3.x[4]=3;  a3.y[4]=14;  a3.inverse[4]=0;
	a3.str[5]=a3_s5; a3.x[5]=7;  a3.y[5]=1;  a3.inverse[5]=0;      
	a3.str[6]=a3_s6; a3.x[6]=7;  a3.y[6]=13;  a3.inverse[6]=0; 

//State: 4
	a4.num=1;

	a4.str[0]=a4_s0; a4.x[0]=3;  a4.y[0]=1;  a4.inverse[0]=0; 


//Output with act[0]
	act[0]=&a0;

	display_ui_act(0);
}



void ui_proc0_0(void)
{

	if(key_code!=0)
	{
		key_UP_flag=0;key_DOWN_flag=0;key_INCREASE_flag=0;key_DECREASE_flag=0;key_ENTER_flag=0;
		a0.inverse[5] = 1;
		act[0] = &a0;
		display_ui_act(0);
		ui_state=001;
	}

}

void ui_proc0_001(void)
{
	if(key_UP_flag)
	{
		key_UP_flag=0;
	}
	else if (key_DOWN_flag)
	{
		key_DOWN_flag=0;
	}
	else if (key_INCREASE_flag)
	{
		key_INCREASE_flag=0;
	}
		else if (key_DECREASE_flag)
	{
		key_DECREASE_flag=0;
	}
	else if(key_ENTER_flag)
	{
		key_ENTER_flag=0;
		a0.inverse[5]=0;
		act[0] = &a1;
		display_ui_act(0);
		act_state = 1;
		ui_state = 0;
	}
	if(NOKEY_clock10s_flag==1)
	{
		NOKEY_clock10s_flag = 0;
		a0.inverse[5]=0;
		display_ui_act(0);
		ui_state=0;		
	}

}



void ui_proc1_000(void)
{
	if(key_UP_flag)
	{
		a1.inverse[0]=0;
		a1.inverse[2]=1;
		key_UP_flag=0;
		display_ui_act(0);
		ui_state = 002;
	}
	else if (key_DOWN_flag)
	{
		a1.inverse[0]=0;
		a1.inverse[1]=1;
		key_DOWN_flag=0;
		display_ui_act(0);
		ui_state = 001;
	}
	else if (key_INCREASE_flag)
	{
		key_INCREASE_flag=0;
	}
		else if (key_DECREASE_flag)
	{
		key_DECREASE_flag=0;
	}

	else if(key_ENTER_flag)
	{
		key_ENTER_flag=0;
		a1.inverse[0]=0;
		a1.inverse[2]=1;
		act_state = 2;
		ui_state = 001;
		act[0] = &a2;
		display_ui_act(0);
	}
}

void ui_proc1_001(void)
{
	if(key_UP_flag)
	{
		a1.inverse[1]=0;
		a1.inverse[0]=1;
		key_UP_flag=0;
		display_ui_act(0);
		ui_state = 000;
	}
	else if (key_DOWN_flag)
	{
		a1.inverse[1]=0;
		a1.inverse[2]=1;
		key_DOWN_flag=0;
		display_ui_act(0);
		ui_state = 002;
	}
	else if (key_INCREASE_flag)
	{
		key_INCREASE_flag=0;
	}
		else if (key_DECREASE_flag)
	{
		key_DECREASE_flag=0;
	}

	else if(key_ENTER_flag)
	{
		key_ENTER_flag=0;
		a1.inverse[1]=0;
		a1.inverse[2]=1;
		act_state = 3;
		ui_state = 001;
		act[0] = &a3;
		display_ui_act(0);
	}
}

void ui_proc1_002(void)
{
	if(key_UP_flag)
	{
		a1.inverse[2]=0;
		a1.inverse[1]=1;
		key_UP_flag=0;
		display_ui_act(0);
		ui_state = 001;
	}
	else if (key_DOWN_flag)
	{
		a1.inverse[2]=0;
		a1.inverse[0]=1;
		key_DOWN_flag=0;
		display_ui_act(0);
		ui_state = 000;
	}
	else if (key_INCREASE_flag)
	{
		key_INCREASE_flag=0;
	}
		else if (key_DECREASE_flag)
	{
		key_DECREASE_flag=0;
	}

	else if(key_ENTER_flag)
	{
		key_ENTER_flag=0;
		a1.inverse[2]=0;
		a1.inverse[0]=1;
		act_state = 0;
		ui_state = 0;
		act[0] = &a0;
		display_ui_act(0);
	}
}



void ui_proc2_001(void)
{
	if(key_UP_flag)
	{
		a2.inverse[1]=0;
		a2.inverse[3]=1;
		key_UP_flag=0;
		display_ui_act(0);
		ui_state = 003;
	}
	else if (key_DOWN_flag)
	{
		a2.inverse[1]=0;
		a2.inverse[2]=1;
		key_DOWN_flag=0;
		display_ui_act(0);
		ui_state = 002;
	}
	else if (key_INCREASE_flag)
	{
		key_INCREASE_flag=0;
		opmode_counter_2++;
		if (opmode_counter_2>2) opmode_counter_2=0;
		itoaopmode(opmode_counter_2, &a2_s1);
		a2.str[1] = a2_s1;
		display_ui_act(0);
		ui_state=001;
	}
		else if (key_DECREASE_flag)
	{
		key_DECREASE_flag=0;
		opmode_counter_2--;
		if (opmode_counter_2<0) opmode_counter_2=2;
		itoaopmode(opmode_counter_2, &a2_s1);
		a2.str[1] = a2_s1;
		display_ui_act(0);
		ui_state=001;
	}

	else if(key_ENTER_flag)
	{
		key_ENTER_flag=0;
	}
}

void ui_proc2_002(void)
{
	if(key_UP_flag)
	{
		a2.inverse[2]=0;
		a2.inverse[1]=1;
		key_UP_flag=0;
		display_ui_act(0);
		ui_state = 001;
	}
	else if (key_DOWN_flag)
	{
		a2.inverse[2]=0;
		a2.inverse[3]=1;
		key_DOWN_flag=0;
		display_ui_act(0);
		ui_state = 003;
	}
	else if (key_INCREASE_flag)
	{
		key_INCREASE_flag=0;
	}
		else if (key_DECREASE_flag)
	{
		key_DECREASE_flag=0;
	}

	else if(key_ENTER_flag)
	{
		key_ENTER_flag=0;
		a2.inverse[2]=0;
		a2.inverse[1]=1;
		opmode_counter_1 = opmode_counter_2;
		itoaopmode(opmode_counter_1,&a0_s0);
		a0.str[0]=a0_s0;
		act[0] = &a1;
		display_ui_act(0);
		act_state = 1;
		ui_state = 002;
	}
}

void ui_proc2_003(void)
{
	if(key_UP_flag)
	{
		a2.inverse[3]=0;
		a2.inverse[2]=1;
		key_UP_flag=0;
		display_ui_act(0);
		ui_state = 002;
	}
	else if (key_DOWN_flag)
	{
		a2.inverse[3]=0;
		a2.inverse[1]=1;
		key_DOWN_flag=0;
		display_ui_act(0);
		ui_state = 001;
	}
	else if (key_INCREASE_flag)
	{
		key_INCREASE_flag=0;
	}
		else if (key_DECREASE_flag)
	{
		key_DECREASE_flag=0;
	}

	else if(key_ENTER_flag)
	{
		key_ENTER_flag=0;
		a2.inverse[3]=0;
		a2.inverse[1]=1;
		opmode_counter_2 = opmode_counter_1;
		itoaopmode(opmode_counter_2,&a2_s1);
		a2.str[1]=a2_s1;
		act[0] = &a1;
		display_ui_act(0);
		act_state = 1;
		ui_state = 002;
	}
}



void ui_proc3_001(void)
{
	if(key_UP_flag)
	{
		a3.inverse[1]=0;
		a3.inverse[6]=1;
		key_UP_flag=0;
		display_ui_act(0);
		ui_state = 006;
	}
	else if (key_DOWN_flag)
	{
		a3.inverse[1]=0;
		a3.inverse[3]=1;
		key_DOWN_flag=0;
		display_ui_act(0);
		ui_state = 003;
	}
	else if (key_INCREASE_flag)
	{
		key_INCREASE_flag=0;
		freq3_1++;
		if (freq3_1>9) freq3_1=0;
		itoafrequency(freq3_1, freq3_2, &a3_s1, &a3_s3);
		a3.str[1] = a3_s1;
		a3.str[3] = a3_s3;
		display_ui_act(0);
	}
		else if (key_DECREASE_flag)
	{
		key_DECREASE_flag=0;
		freq3_1--;
		if (freq3_1<0) freq3_1=9;
		itoafrequency(freq3_1, freq3_2, &a3_s1, &a3_s3);
		a3.str[1] = a3_s1;
		a3.str[3] = a3_s3;
		display_ui_act(0);
	}

	else if(key_ENTER_flag)
	{
		key_ENTER_flag=0;
	}
}

void ui_proc3_003(void)
{
	if(key_UP_flag)
	{
		a3.inverse[3]=0;
		a3.inverse[1]=1;
		key_UP_flag=0;
		display_ui_act(0);
		ui_state = 001;
	}
	else if (key_DOWN_flag)
	{
		a3.inverse[3]=0;
		a3.inverse[5]=1;
		key_DOWN_flag=0;
		display_ui_act(0);
		ui_state = 005;
	}
	else if (key_INCREASE_flag)
	{
		key_INCREASE_flag=0;
		freq3_2++;
		if (freq3_2>9) freq3_2=0;
		itoafrequency(freq3_1, freq3_2, &a3_s1, &a3_s3);
		a3.str[1] = a3_s1;
		a3.str[3] = a3_s3;
		display_ui_act(0);
	}
		else if (key_DECREASE_flag)
	{
		key_DECREASE_flag=0;
		freq3_2--;
		if (freq3_2<0) freq3_2=9;
		itoafrequency(freq3_1, freq3_2, &a3_s1, &a3_s3);
		a3.str[1] = a3_s1;
		a3.str[3] = a3_s3;
		display_ui_act(0);
	}

	else if(key_ENTER_flag)
	{
		key_ENTER_flag=0;
	}
}

void ui_proc3_005(void)
{
	if(key_UP_flag)
	{
		a3.inverse[5]=0;
		a3.inverse[3]=1;
		key_UP_flag=0;
		display_ui_act(0);
		ui_state = 003;
	}
	else if (key_DOWN_flag)
	{
		a3.inverse[5]=0;
		a3.inverse[6]=1;
		key_DOWN_flag=0;
		display_ui_act(0);
		ui_state = 006;
	}
	else if (key_INCREASE_flag)
	{
		key_INCREASE_flag=0;
	}
		else if (key_DECREASE_flag)
	{
		key_DECREASE_flag=0;
	}

	else if(key_ENTER_flag)
	{
		key_ENTER_flag=0;
		a3.inverse[5]=0;
		a3.inverse[1]=1;
		if(freq3_1 == 0 || (freq3_1 == 9 && freq3_2 != 0)){
			act_state = 4;
			ui_state = 0;
			NOKEY_clock5s_flag = 0;
			act[0] = &a4;
			display_ui_act(0);
		} else {
			freq0_1 = freq3_1;
			freq0_2 = freq3_2;
			itoafrequency(freq0_1, freq0_2, &a0_s1, &a0_s3);
			a0.str[1] = a0_s1;
			a0.str[3] = a0_s3;
			act[0] = &a1;
			display_ui_act(0);
			act_state = 1;
			ui_state = 002;
		}
	}
}

void ui_proc3_006(void)
{
	if(key_UP_flag)
	{
		a3.inverse[6]=0;
		a3.inverse[5]=1;
		key_UP_flag=0;
		display_ui_act(0);
		ui_state = 005;
	}
	else if (key_DOWN_flag)
	{
		a3.inverse[6]=0;
		a3.inverse[1]=1;
		key_DOWN_flag=0;
		display_ui_act(0);
		ui_state = 001;
	}
	else if (key_INCREASE_flag)
	{
		key_INCREASE_flag=0;
	}
		else if (key_DECREASE_flag)
	{
		key_DECREASE_flag=0;
	}

	else if(key_ENTER_flag)
	{
		key_ENTER_flag=0;
		a3.inverse[6]=0;
		a3.inverse[1]=1;
		freq3_1 = freq0_1;
		freq3_2 = freq0_2;
		itoafrequency(freq3_1, freq3_2, &a3_s1, &a3_s3);
		a3.str[1] = a3_s1;
		a3.str[3] = a3_s3;
		act[0] = &a1;
		display_ui_act(0);
		act_state = 1;
		ui_state = 002;
	}
}



void ui_proc4_0(void)
{
	if(key_code!=0)
	{
		key_UP_flag=0;key_DOWN_flag=0;key_INCREASE_flag=0;key_DECREASE_flag=0;key_ENTER_flag=0;
	}
	if(NOKEY_clock5s_flag==1)
	{
		act[0] = &a3;
		display_ui_act(0);
		act_state = 3;
		ui_state = 001;
	}
}






void ui_state_proc(unsigned int ui_state)
{
	switch (act_state)
		{
			case 0: 
				switch (ui_state){
					case 0: ui_proc0_0(); break;
					case 001: ui_proc0_001(); break;

					default: break;
				}
			break;
			case 1: switch (ui_state){
					case 000: ui_proc1_000(); break;
					case 001: ui_proc1_001(); break;
					case 002: ui_proc1_002(); break;

					default: break;
				}
			break;
			case 2: switch (ui_state){
					case 001: ui_proc2_001(); break;
					case 002: ui_proc2_002(); break;
					case 003: ui_proc2_003(); break;

					default: break;
				}
			break;
			case 3: switch (ui_state){
					case 001: ui_proc3_001(); break;
					case 003: ui_proc3_003(); break;
					case 005: ui_proc3_005(); break;
					case 006: ui_proc3_006(); break;

					default: break;
				}
			break;
			case 4: switch (ui_state){
					case 0: ui_proc4_0(); break;

					default: break;
				}
			break;

			default: break;
		}

}
