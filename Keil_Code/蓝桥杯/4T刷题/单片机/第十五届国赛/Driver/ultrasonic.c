#include "ultrasonic.h"
#include "intrins.h"
sbit US_TX = P1^0;
sbit US_RX = P1^1;

void Delay12us(void)	//@12.000MHz
{
	unsigned char data i;

	_nop_();
	i = 3;
	while (--i);
}

void Ultrasonic_Init()
{
	unsigned char i;
	for(i = 0;i < 8;i++)
	{
		US_TX = 1;
		Delay12us();
		US_TX = 0;
		Delay12us();
	}
}

unsigned char Ultrasonic_Read()
{
	unsigned int time;//"int"!!
	
	CMOD = 0x00;
	CH = CL = 0;
	
	EA = 0;
	Ultrasonic_Init();
	EA = 1;
	
	CR = 1;
	while(US_RX && !CF);
	CR = 0;
	if(!CF)
	{
		time = (CH << 8 | CL);
		return (time * 0.017);
	}
	else
	{
		CF = 0;
		return 0;
	}
}