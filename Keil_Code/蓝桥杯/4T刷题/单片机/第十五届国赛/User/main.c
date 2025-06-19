#include <STC15F2K60S2.H>

#include "init.h"
#include "led.h"
#include "key.h"
#include "seg.h"

#include "iic.h"
#include "ultrasonic.h"
#include "uart.h"

#include "stdio.h"
#include "string.h"
#include "math.h"

/*======variables and parameters======*/
//Led
pdata unsigned char Led_Buf[8] = {0,0,0,0,0,0,0,0};
idata bit Led_1_Flag;
idata unsigned char Led_1_Time_100ms;
idata unsigned int Led_3_Time_3000ms;
idata bit Flash_3 = 1;
idata bit Relay_Now,Relay_Old;
//Seg
pdata unsigned char Seg_Buf[8] = {10,10,10,10,10,10,10,10};
idata unsigned char Seg_Pos = 0;
idata unsigned char Seg_Slow_Down;
idata unsigned char Seg_Disp_Mode;//0-坐标界面 1-速度界面 2-参数界面
//Key
idata unsigned char Key_Val, Key_Old, Key_Up, Key_Down; // 分别存储当前键值、旧键值、弹起键值、按下键值
idata unsigned char Key_Slow_Down;
//Uart
idata unsigned char Uart_Rec_Index;
pdata unsigned char Uart_Rec_Buf[9] = 0;
bit Uart_Rec_Falg;
idata unsigned char Uart_Rec_Tick;
//idata unsigned char Char_Read;
idata unsigned char Result;
//Frep
idata unsigned int Frep;
idata unsigned int Time_1s;	
//variables
idata unsigned char Moving_Mode;//0-空闲 1-运行 2-等待(等待之前必须是运行)
//parameters
idata unsigned int x,y;
idata unsigned int Location_Goal_x;
idata unsigned int Location_Goal_y;
idata unsigned int Location_Devi_x;
idata unsigned int Location_Devi_y;
idata unsigned int Location_x;
idata unsigned int Location_y;
idata bit Got_it_Flag;
idata unsigned int Speed_x10;
idata float Speed;
idata unsigned char Para_R_x10 = 10;
idata char Para_B = 0;
idata bit Para_Flag;//0-R 1-B
idata float Distance_Devi_Goal,Distance_Devi_Goal_Init;
idata unsigned int Timers;
idata bit Move_Flag;
idata float Distance_Devi_Goal_temp;
idata bit Finish_Move;
//Ultrasonic
idata unsigned char Distance;
idata unsigned char Distance_Slow_Down;
idata unsigned char Distance_temp;
idata bit Obsa_Flag;
//AD_DA
idata unsigned char AD_1_Data;
idata unsigned char AD_DA_Slow_Down;
idata bit Work_Time;//0-night 1-day
/*======Task======*/
void Task_Proc()
{
	if(Moving_Mode == 1)
		Speed_x10 = (3.14*Para_R_x10*Frep/10) + Para_B;
	else
		Speed_x10 = 0;
	
	if(Distance < 30)
	{
		if(Moving_Mode == 1)
		{
			Moving_Mode = 2;
			Obsa_Flag = 1;
		}
	}
	else
		Obsa_Flag = 0;
	
	if(Moving_Mode == 1)
	{
		Speed = (3.14*Para_R_x10*Frep/100) + Para_B;
		Move_Flag = 1;
	}
	else
	{
		Speed = 0;
		Move_Flag = 0;
	}
	if(Got_it_Flag == 1)
	{
		Distance_Devi_Goal_temp = (Speed * Timers) / 1000.0;
		Location_Devi_x = Distance_Devi_Goal_temp*(Location_Goal_x / Distance_Devi_Goal_Init);
		Location_Devi_y = Distance_Devi_Goal_temp*(Location_Goal_y / Distance_Devi_Goal_Init);
		Location_x = abs(Location_Goal_x - Location_Devi_x);
		Location_y = abs(Location_Goal_y - Location_Devi_y);
		Distance_Devi_Goal_Init = sqrt(Location_Goal_x*Location_Goal_x+Location_Goal_y*Location_Goal_y);
		Distance_Devi_Goal = sqrt(Location_x*Location_x+Location_y*Location_y);
		
		if(Distance_Devi_Goal_temp >= Distance_Devi_Goal_Init)
		{
			Moving_Mode = 0;
			Finish_Move = 1;
			Timers = 0;
			Flash_3 = 0;
		}
		else
			Finish_Move = 0;
	}
}
/*======Key======*/
void Key_Proc()
{
	// 减速计数，每10ms进入一次，实现按键消抖
	if (Key_Slow_Down < 10) return;
	Key_Slow_Down = 0;

	// 读取当前按键状态
	Key_Val = Key_Read();
	// 通过位运算计算出刚刚被按下的键（下降沿检测）
	Key_Down = Key_Val & (Key_Val ^ Key_Old);
	// 通过位运算计算出刚刚被弹起的键（上升沿检测）
	Key_Up = ~Key_Val & (Key_Val ^ Key_Old);
	// 更新旧键值，为下一次检测做准备
	Key_Old = Key_Val;
	
	switch(Key_Down)
	{
		case 4://启动按键
			if(Moving_Mode == 0)
			{
				if(Got_it_Flag == 1)
					Moving_Mode = 1;
			}
			else if(Moving_Mode == 2)
			{
				if(Obsa_Flag == 0)
					Moving_Mode = 1;
			}
		break;
		case 5://重置按键
			if(Moving_Mode == 0)
			{
				Location_Devi_x = Location_Devi_y = 0;
				Timers = 0;
			}
		break;
		case 8://界面按键
			Seg_Disp_Mode = (++Seg_Disp_Mode) % 3;
			if(Seg_Disp_Mode == 2)
				Para_Flag = 0;
		break;
		case 9://选择按键
			if(Seg_Disp_Mode == 2)
				Para_Flag ^= 1;
		break;
		case 12://加按键
			if(Seg_Disp_Mode == 2)
			{
				if(Para_Flag)//B
				{
					Para_B += 5;
					if(Para_B == 95)
						Para_B = -90;
				}
				else
				{
					Para_R_x10++;
					if(Para_R_x10 == 21)
						Para_R_x10 = 10;
				}
			}
		break;
		case 13://减按键
			if(Seg_Disp_Mode == 2)
			{
				if(Para_Flag)//B
				{
					Para_B -= 5;
					if(Para_B == -95)
						Para_B = 90;
				}
				else
				{
					Para_R_x10--;
					if(Para_R_x10 == 9)
						Para_R_x10 = 20;
				}
			}
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
		case 0://坐标
			Seg_Buf[0] = 12;
			if(Moving_Mode)//等待 or 运行
			{
				Seg_Buf[1] = (Location_Goal_x >= 100)?Location_Goal_x / 100 % 10:10;
				Seg_Buf[2] = (Location_Goal_x >= 10)?Location_Goal_x / 10 % 10:10;
				Seg_Buf[3] = Location_Goal_x % 10;
				Seg_Buf[4] = 11;
				Seg_Buf[5] = (Location_Goal_y >= 100)?Location_Goal_y / 100 % 10:10;
				Seg_Buf[6] = (Location_Goal_y >= 10)?Location_Goal_y / 10 % 10:10;
				Seg_Buf[7] = Location_Goal_y % 10;
			}
			else
			{
				Seg_Buf[1] = (Location_Devi_x >= 100)?Location_Devi_x / 100 % 10:10;
				Seg_Buf[2] = (Location_Devi_x >= 10)?Location_Devi_x / 10 % 10:10;
				Seg_Buf[3] = Location_Devi_x % 10;
				Seg_Buf[4] = 11;
				Seg_Buf[5] = (Location_Devi_y >= 100)?Location_Devi_y / 100 % 10:10;
				Seg_Buf[6] = (Location_Devi_y >= 10)?Location_Devi_y / 10 % 10:10;
				Seg_Buf[7] = Location_Devi_y % 10;
			}
		break;
		case 1://速度
			Seg_Buf[0] = 13;
			Seg_Buf[2] = 10;
			if(Moving_Mode == 1)//运行
			{
				Seg_Buf[1] = 1;
				Seg_Buf[3] = (Speed_x10 >= 10000)?Speed_x10 / 10000 % 10:10;
				Seg_Buf[4] = (Speed_x10 >= 1000)?Speed_x10 / 1000 % 10:10;
				Seg_Buf[5] = (Speed_x10 >= 100)?Speed_x10 / 100 % 10:10;
				Seg_Buf[6] = (Speed_x10 >= 10)?Speed_x10 / 10 % 10 + '.':10 + '.';
				Seg_Buf[7] = Speed_x10 % 10;
			}
			else if(Moving_Mode == 0)//空闲
			{
				Seg_Buf[1] = 2;
				Seg_Buf[3] = 11;
				Seg_Buf[4] = 11;
				Seg_Buf[5] = 11;
				Seg_Buf[6] = 11;
				Seg_Buf[7] = 11;
			}
			else//等待
			{
				Seg_Buf[1] = 3;
				Seg_Buf[3] = 10;
				Seg_Buf[4] = 10;
				Seg_Buf[5] = (Distance >= 100)?Distance / 100 % 10:10;
				Seg_Buf[6] = (Distance >= 10)?Distance / 10 % 10:10;
				Seg_Buf[7] = Distance % 10;
			}
		break;
		case 2://参数
			Seg_Buf[0] = 14;
			Seg_Buf[1] = 10;
			Seg_Buf[2] = Para_R_x10 / 10 % 10 + '.';
			Seg_Buf[3] = Para_R_x10 % 10;
			Seg_Buf[4] = 10;
			if(Para_B >= 0)
			{
				Seg_Buf[5] = 10;
				Seg_Buf[6] = (Para_B >= 10)?Para_B / 10 % 10:10;
				Seg_Buf[7] = Para_B % 10;
			}
			else
			{
				Seg_Buf[5] = (abs(Para_B) >= 10)?11:10;
				Seg_Buf[6] = (abs(Para_B) >= 10)?abs(Para_B) / 10 % 10:11;
				Seg_Buf[7] = abs(Para_B) % 10;
			}
		break;
	}
}
/*======Led======*/
void Led_Proc()
{
	//led1
	if(Moving_Mode == 0)
		Led_Buf[0] = 0;
	else if(Moving_Mode == 1)
		Led_Buf[0] = 1;
	else
		Led_Buf[0] = Led_1_Flag?1:0;
	//led2
	if(Moving_Mode == 1)
		Led_Buf[1] = Work_Time?0:1;
	else
		Led_Buf[1] = 0;
	//led3
	if(Finish_Move)
		Led_Buf[2] = Flash_3?0:1;

	Relay_Now = (Moving_Mode == 1)?1:0;
	if(Relay_Now != Relay_Old)
	{
		Relay(Relay_Now);
		Relay_Old = Relay_Now;
	}
	
	Led_Disp(Led_Buf);
}
/*======AD_DA======*/
void AD_DA()
{
	if(AD_DA_Slow_Down < 140) return;
	AD_DA_Slow_Down = 0;
	
	AD_1_Data = AD_Read(0x41);
	Work_Time = (AD_1_Data > 1.2*51);
}
/*======Uart======*/
void Uart_Proc()
{
	if(Uart_Rec_Index == 0) return;
	if(Uart_Rec_Tick >= 10)
	{
		Uart_Rec_Falg = 0;
		Uart_Rec_Tick = 0;
		//设置目的地坐标
		if(Uart_Rec_Buf[0] == '(' && Uart_Rec_Buf[Uart_Rec_Index - 1] == ')')
		{
			Result = sscanf(Uart_Rec_Buf,"(%u,%u)%n",&x,&y);
			if(Result == 2)
			{
				if(Moving_Mode == 0)
				{
					Location_Goal_x = x;
					Location_Goal_y = y;
					Got_it_Flag = 1;
					printf("Got it");
				}
			}
		}
		//查询设备状态
		else if(strcmp(Uart_Rec_Buf,"?") == 0)
		{
			if(Moving_Mode == 0)
				printf("Idle");
			else if(Moving_Mode == 2)
				printf("Wait");
			else
				printf("Busy");
		}
		//查询设备位置
		else if(strcmp(Uart_Rec_Buf,"#") == 0)
			printf("(%u,%u)",Location_Devi_x,Location_Devi_y);
		else
			printf("Error");
		memset(Uart_Rec_Buf,0,Uart_Rec_Index);
		Uart_Rec_Index = 0;
	}
}
/*======Ultrasonic======*/
void Ultrasonic_Proc()
{
	if(Distance_Slow_Down < 120) return;
	Distance_Slow_Down = 0;
	
	Distance_temp = Ultrasonic_Read();
	if(Distance_temp != 0)
		Distance = Distance_temp;
}
/*======Timer_0======*/
void Timer0_Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TMOD |= 0x05;
	TL0 = 0x00;				//设置定时初始值
	TH0 = 0x00;				//设置定时初始值
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
		Frep = (TH0 << 8)|TL0;
		TH0 = TL0 = 0;
	}
	if(Uart_Rec_Falg)
		Uart_Rec_Tick++;
	
	if(++Led_1_Time_100ms == 100)
	{
		Led_1_Time_100ms = 0;
		Led_1_Flag ^= 1;
	}
	if(Move_Flag)
		Timers++;
	if(Flash_3 == 0)
	{
		if(++Led_3_Time_3000ms == 3000)
		{
			Led_3_Time_3000ms = 0;
			Flash_3 = 1;
		}
	}
}
/*======Uart_Interrupt======*/
void Uart1_Isr(void) interrupt 4
{
	if(RI)				//检测串口1接收中断
	{
		Uart_Rec_Falg = 1;
		Uart_Rec_Tick = 0;
		Uart_Rec_Buf[Uart_Rec_Index++] = SBUF;
		RI = 0;			//清除串口1接收中断请求位
		if(Uart_Rec_Index > 9)
		{
			memset(Uart_Rec_Buf,0,10);
			Uart_Rec_Index = 0;
		}
	}
}

/*======Main======*/
void main()
{
	System_Init();
	Timer0_Init();
	Uart1_Init();
	Timer1_Init();
	while(1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
		AD_DA();
		Ultrasonic_Proc();
		Uart_Proc();
		Task_Proc();
	}
}