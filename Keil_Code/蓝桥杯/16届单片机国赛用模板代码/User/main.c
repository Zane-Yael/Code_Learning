/*======头文件声明区======*/
#include <STC15F2K60S2.H>

#include "init.h"
#include "led.h"
#include "key.h"
#include "seg.h"

#include "ds1302.h"
#include "onewire.h"
#include "iic.h"

/*======变量声明区域======*/
pdata unsigned char Led_Buf[8] = {0,0,0,0,0,0,0,0}; 
idata unsigned char Key_Val,Key_Down,Key_Up,Key_Old;
idata unsigned char Key_Slow_Down;
pdata unsigned char Seg_Buf[8] = {10,10,10,10,10,10,10,10};
idata unsigned char Seg_Pos;
idata unsigned char Seg_Slow_Down; 

pdata unsigned char Rtc[3] = {23,55,24};
idata unsigned char Rtc_Slow_Down;
idata unsigned int Temperature_x10;
idata unsigned char Temperature_Slow_Down;
idata unsigned char AD_1_Data_x10,AD_3_Data_x10;
idata unsigned char AD_DA_Slow_Down;
pdata unsigned char EEPROM_Data_W[8] = {1,2,3,4,5,6,7,8};
pdata unsigned char EEPROM_Data_R[8] = {0,0,0,0,0,0,0,0};

idata unsigned char Distance;
idata unsigned char Distance_Slow_Down;
idata unsigned int Frep;
idata unsigned int Timer_1s; 
/*======key======*/
void Key_Proc()
{
	if(Key_Slow_Down < 10) return;
	Key_Slow_Down = 0;
	
}
/*======seg======*/
void Seg_Proc()
{
	if(Seg_Slow_Down < 60) return;
	Seg_Slow_Down = 0;
}
/*======led======*/
void Led_Proc()
{
	
}
/*======Rtc======*/
void Get_Time()
{
	if(Rtc_Slow_Down < 100) return;
	Rtc_Slow_Down = 0;
}

/*======Temperature======*/
void Get_Temperature()
{
	if(AD_DA_Slow_Down < 100) return;
	AD_DA_Slow_Down = 0;
}

/*======AD_DA======*/
void AD_DA_Proc()
{
	if(Temperature_Slow_Down < 150) return;
	Temperature_Slow_Down = 0;
}
/*======Ultrasonic======*/
void Get_Distance()
{
	if(Distance_Slow_Down < 100) return;
	Distance_Slow_Down = 0;
}
/*======按键处理函数======*/
/*======定时器0======*/
void Timer0_Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TMOD |= 0x05;     //设置计数器模式
	TL0 = 0x00;				//设置定时初始值
	TH0 = 0x00;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
}

/*======定时器1======*/
void Timer1_Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0xBF;			//定时器时钟12T模式
	TMOD &= 0x0F;			//设置定时器模式
	TL1 = 0x18;				//设置定时初始值
	TH1 = 0xFC;				//设置定时初始值
	TF1 = 0;				//清除TF1标志
	TR1 = 1;				//定时器1开始计时
	ET1 = 1;				//使能定时器1中断
	EA = 1;
}
/*======定时器1中断======*/
void Timer1_Isr(void) interrupt 3
{
	Key_Slow_Down++;
	Seg_Slow_Down++;
	Temperature_Slow_Down++;
	Rtc_Slow_Down++;
	AD_DA_Slow_Down++;
	
	Seg_Pos = (++Seg_Pos) % 8;
	if(Seg_Buf[Seg_Pos] > 20)
		Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos] - '.',1);
	else
		Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],0);
	Led_Disp(Led_Buf);
	
	if(++Timer_1s == 1000)
	{
		Timer_1s = 0;
		Frep = (TH0 << 8) | TL0;
		TH0 = TL0 = 0;
	}
}

/*======main======*/
void main()
{
	System_Init();
	
	EEPROM_Read(EEPROM_Data_R,0,8);
	EEPROM_Write(EEPROM_Data_W,0,8);
	EEPROM_Read(EEPROM_Data_R,0,8);
	
	Timer0_Init();
	Timer1_Init();
	while(1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
		Get_Time();
		Get_Temperature();
		AD_DA_Proc();
		
	}
}