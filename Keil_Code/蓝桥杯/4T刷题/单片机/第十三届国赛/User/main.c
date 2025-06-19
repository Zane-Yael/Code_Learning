#include <STC15F2K60S2.H>
#include "init.h"
#include "led.h"
#include "key.h"
#include "seg.h"

#include "ultrasonic.h"
#include "iic.h"

/*======Variables And Parameters======*/
//led
pdata unsigned char Led_Buf[8] = {0,0,0,0,0,0,0,0};
bit Led_1_Flag;
bit Led_2_Flag;
bit Led_3_Flag;
idata unsigned char Led_1_Time_100ms;
idata unsigned char Led_2_Time_100ms;
idata unsigned char Led_3_Time_100ms;
//key
idata unsigned char Key_Val,Key_Down,Key_Up,Key_Old;
idata unsigned char Key_Slow_Down;
bit Long_Process_Flag;
idata unsigned int Process_Time_1s;
//seg
pdata unsigned char Seg_Buf[8] = {10,10,10,10,10,10,10,10};
idata unsigned char Seg_Pos;
idata unsigned char Seg_Slow_Down;
idata unsigned char Seg_Disp_Mode;//0-频率 1-湿度 2-测距 3-参数
idata unsigned char Parameters_Mode;//0-frep 1-humidity 2-distance
//ultrasonic
idata unsigned char temp;
idata unsigned char Distance;
bit Distance_Flag;//0-cm 1-m
idata unsigned char Distance_Para = 6;
idata unsigned char Distance_Slow_Down;
//Frep
idata unsigned int Frep_Hz;
idata unsigned int Frep_KHz_x10;
idata unsigned char Frep_KHz_x10_Para = 90;
bit Frep_Flag;//0-hz 1-khz
idata unsigned int Time_1s;
//AD_DA
idata unsigned char AD_DA_Slow_Down;
//Humidity
idata unsigned char Humidity;
idata unsigned char Humidity_Para = 40;
//Relay
idata unsigned char Relay_Num;
bit Relay_Work,Relay_Work_Old;
//pwm
data unsigned char pwm_period;
data unsigned char pwm_compare = 0;
/*======Key======*/
void Key_Proc()
{
	if(Key_Slow_Down < 20) return;
	Key_Slow_Down = 0;
	
	Key_Val = Key_Read();
	Key_Down = Key_Val & (Key_Old ^ Key_Val);
	Key_Up = ~Key_Val & (Key_Old ^ Key_Val);
	Key_Old = Key_Val;
	
	if(Seg_Disp_Mode == 1)
	{
		if(Key_Down == 7)
			Long_Process_Flag = 1;
		if(Key_Up == 7)
		{
			if(Process_Time_1s >= 1000)
			{
				Relay_Num = 0;
				EEPROM_Write(&Relay_Num,0,1);
			}
			Long_Process_Flag = 0;
		}
	}
	switch(Key_Down)
	{
		case 4://界面切换
			Seg_Disp_Mode = (++Seg_Disp_Mode) % 4;
		if(Seg_Disp_Mode == 3)
			Parameters_Mode = 0;
		break;
		case 5://参数界面切换
			if(Seg_Disp_Mode == 3)
				Parameters_Mode = (++Parameters_Mode) % 3;
		break;
		case 6://加
			if(Parameters_Mode == 0)
			{
				Frep_KHz_x10_Para += 5;
				if(Frep_KHz_x10_Para == 125)
					Frep_KHz_x10_Para = 10;
			}
			else if(Parameters_Mode == 1)
			{
				Humidity_Para += 10;
				if(Humidity_Para == 70)
					Humidity_Para = 10;
			}
			else
			{
				if(++Distance_Para == 13)
					Distance_Para = 1;
			}
			if(Seg_Disp_Mode == 2)
				Distance_Flag ^= 1;
		break;
		case 7://减
			if(Parameters_Mode == 0)
			{
				Frep_KHz_x10_Para -= 5;
				if(Frep_KHz_x10_Para == 5)
					Frep_KHz_x10_Para = 120;
			}
			else if(Parameters_Mode == 1)
			{
				Humidity_Para -= 10;
				if(Humidity_Para == 0)
					Humidity_Para = 60;
			}
			else
			{
				if(--Distance_Para == 0)
					Distance_Para = 12;
			}
			if(Seg_Disp_Mode == 0)
				Frep_Flag ^= 1;
		break;
	}
}
/*======Seg======*/
void Seg_Proc()
{
	if(Seg_Slow_Down < 60) return;
	Seg_Slow_Down = 0;
	
	switch(Seg_Disp_Mode)
	{
		case 0://frep
			Seg_Buf[0] = 11;
			Seg_Buf[1] = 10;
			if(Frep_Flag)
			{
				Seg_Buf[2] =  10;
				Seg_Buf[3] = 	10;
				Seg_Buf[4] =  10;
				Seg_Buf[5] = (Frep_KHz_x10 > 100)?Frep_KHz_x10 / 100 % 10:10;
				Seg_Buf[6] = (Frep_KHz_x10 / 10 % 10) + '.';
				Seg_Buf[7] = Frep_KHz_x10 % 10;
			}
			else
			{
				Seg_Buf[2] = (Frep_Hz > 100000) ? Frep_Hz / 100000 % 10 : 10;
				Seg_Buf[3] = (Frep_Hz > 10000) ? Frep_Hz / 10000 % 10 : 10;
				Seg_Buf[4] = (Frep_Hz > 1000) ? Frep_Hz / 1000 % 10 : 10;
				Seg_Buf[5] = (Frep_Hz > 100) ? Frep_Hz / 100 % 10 : 10;
				Seg_Buf[6] = (Frep_Hz > 10) ? Frep_Hz / 10 % 10 : 10;
				Seg_Buf[7] = Frep_Hz % 10;
			}
		break;
		case 1://humidity
			Seg_Buf[0] = 12;
			Seg_Buf[1] = 10;
			Seg_Buf[2] = 10;
			Seg_Buf[3] = 10;
			Seg_Buf[4] = 10;
	    Seg_Buf[5] = 10;
			Seg_Buf[6] = Humidity / 10 % 10;
			Seg_Buf[7] = Humidity % 10;
		break;
		case 2://测距界面
			Seg_Buf[0] = 13;
			Seg_Buf[1] = 10;
			Seg_Buf[2] = 10;
			Seg_Buf[3] = 10;
			Seg_Buf[4] = 10;
			if(Distance_Flag)
			{
				Seg_Buf[5] = (Distance / 100 % 10) + '.';
				Seg_Buf[6] = Distance / 10 % 10;
				Seg_Buf[7] = Distance % 10;
			}
			else
			{
				Seg_Buf[5] = (Distance > 100)?(Distance / 100 % 10):10;
				Seg_Buf[6] = (Distance > 10)?(Distance / 10 % 10):10;
				Seg_Buf[7] = Distance % 10;
			}
		break;
		case 3://参数界面
			Seg_Buf[0] = 14;
			Seg_Buf[1] = Parameters_Mode + 1;
			Seg_Buf[2] = 10;
			Seg_Buf[3] = 10;
			Seg_Buf[4] = 10;
			if(Parameters_Mode == 0)
			{
				Seg_Buf[5] = (Frep_KHz_x10_Para > 100)?Frep_KHz_x10_Para / 100 % 10:10;
				Seg_Buf[6] = (Frep_KHz_x10_Para / 10 % 10) + '.';
				Seg_Buf[7] = Frep_KHz_x10_Para % 10;
			}
			else if(Parameters_Mode == 1)
			{
				Seg_Buf[5] = 10;
				Seg_Buf[6] = Humidity_Para / 10 % 10;
				Seg_Buf[7] = Humidity_Para % 10;
			}
			else
			{
				Seg_Buf[5] = 10;
				Seg_Buf[6] = (Distance_Para / 10 % 10) + '.';
				Seg_Buf[7] = Distance_Para % 10;
			}
		break;
	}
}
/*======Led======*/
void Led_Proc()
{
	if(Seg_Disp_Mode == 3)
	{
		Led_Buf[0] = Led_1_Flag;
		Led_Buf[1] = Led_2_Flag;
		Led_Buf[2] = Led_3_Flag;
	}
	else
	{
		Led_Buf[0] = (Seg_Disp_Mode == 0);
		Led_Buf[1] = (Seg_Disp_Mode == 1);
		Led_Buf[2] = (Seg_Disp_Mode == 2);
	}
	Led_Buf[3] = (Frep_KHz_x10 > Frep_KHz_x10_Para);
	Led_Buf[4] = (Humidity> Humidity_Para);
	Led_Buf[5] = (Distance > Distance_Para * 10);
	
	Relay_Work = (Distance > Distance_Para * 10);
	if(Relay_Work != Relay_Work_Old)
	{
		Relay(Relay_Work);
		Relay_Num++;
		EEPROM_Write(&Relay_Num,0,1);
		Relay_Work_Old = Relay_Work;
	}
}
/*======AD_DA======*/
void AD_DA_Proc()
{
	if(AD_DA_Slow_Down < 120) return;
	AD_DA_Slow_Down = 0;
	
	Humidity = AD_Read(0x43) / 51.0 * 20;
	if(Humidity < Humidity_Para)
		DA_Write(51);
	else if(Humidity > 80)
		DA_Write(255);
	else
		DA_Write(51 + (((4.0/(80 - Humidity_Para))*(Humidity - Humidity_Para)))*51);
}
/*======Ultrasonic======*/
void Ultrasonic_Proc()
{
	if(Distance_Slow_Down < 120) return;
	Distance_Slow_Down = 0;
	
	temp = Ultrasonic_Read();
	if(temp != 0)
		Distance = temp;
}
/*======Timer_0======*/
void Timer0_Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TMOD |= 0x05;
	TL0 = 0x00;				//设置定时初始值
	TH0 = 0x00;	//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
}
/*======Timer_1======*/
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

void Timer2_Init(void)		//100微秒@12.000MHz
{
	AUXR &= 0xFB;			//定时器时钟12T模式
	T2L = 0x9C;				//设置定时初始值
	T2H = 0xFF;				//设置定时初始值
	AUXR |= 0x10;			//定时器2开始计时
	IE2 |= 0x04;			//使能定时器2中断
	EA = 1;
}
/*======Timer_1_Interrupt======*/
void Timer1_Isr(void) interrupt 3
{
	Key_Slow_Down++;
	Seg_Slow_Down++;
	Distance_Slow_Down++;
	AD_DA_Slow_Down++;
	
	Seg_Pos = (++Seg_Pos) % 8;
	if(Seg_Buf[Seg_Pos] > 20)
		Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos] - '.',1);
	else
		Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],0);
	
	
	if(++Time_1s == 1000)
	{
		Time_1s = 0;
		Frep_Hz = (TH0 << 8) | TL0;
		Frep_KHz_x10  = Frep_Hz / 100;
		if(Frep_KHz_x10 > Frep_KHz_x10_Para)
			pwm_compare = 8;
		else
			pwm_compare = 2;
		TH0 = TL0 = 0;	
	}
	if(Long_Process_Flag)
	{
		if(++Process_Time_1s == 1000)
			Process_Time_1s = 1001;
	}
	else
		Process_Time_1s = 0;
	if(Seg_Disp_Mode)
	{
		switch(Parameters_Mode)
		{
			case 0:
				Led_2_Time_100ms = 0;
				Led_2_Flag = 0;
				Led_3_Time_100ms = 0;
				Led_3_Flag = 0;
			if(++Led_1_Time_100ms == 100)
			{
				Led_1_Time_100ms = 0;
				Led_1_Flag ^= 1;
			}
			break;
			case 1:
				Led_1_Time_100ms = 0;
				Led_1_Flag = 0;
				Led_3_Time_100ms = 0;
				Led_3_Flag = 0;
			if(++Led_2_Time_100ms == 100)
			{
				Led_2_Time_100ms = 0;
				Led_2_Flag ^= 1;
			}
			break;
			case 2:
				Led_1_Time_100ms = 0;
				Led_1_Flag = 0;
				Led_2_Time_100ms = 0;
				Led_2_Flag = 0;
			if(++Led_3_Time_100ms == 100)
			{
				Led_3_Time_100ms = 0;
				Led_3_Flag ^= 1;
			}
			break;
		}
	}
	else
	{
		Led_1_Time_100ms = 0;
		Led_1_Flag = 0;
		Led_2_Time_100ms = 0;
		Led_2_Flag = 0;
		Led_3_Time_100ms = 0;
		Led_3_Flag = 0;
	}
	
	Led_Disp(Led_Buf);
}

void Timer2_Isr(void) interrupt 12
{
	pwm_period = (++pwm_period) % 10;
	Motor(pwm_period <= pwm_compare);
}
/*======Main======*/
void main()
{
	System_Init();
	Timer0_Init();
	Timer1_Init();
	Timer2_Init();
	while(1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
		AD_DA_Proc();
		Ultrasonic_Proc();
	}
}