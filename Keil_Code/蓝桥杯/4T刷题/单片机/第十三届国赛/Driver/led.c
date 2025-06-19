#include "led.h"

idata unsigned char temp_0 = 0x00;
idata unsigned char temp_0_old = 0xff;

void Led_Disp(unsigned char *Led_Buf)
{
	unsigned char temp;
	temp_0 = 0x00;
	temp_0 = (Led_Buf[0] << 0) | (Led_Buf[1] << 1) | (Led_Buf[2] << 2) | (Led_Buf[3] << 3)
					|(Led_Buf[4] << 4) | (Led_Buf[5] << 5) | (Led_Buf[6] << 6) | (Led_Buf[7] << 7);
	if(temp_0 != temp_0_old)
	{
		P0 = ~temp_0;
		temp = P2 & 0x1f;
		temp |= 0x80;
		P2 = temp;
		temp = P2 & 0x1f;
		P2 = temp;
		temp_0_old = temp_0;
	}
}

//void Led_Off()
//{
//	unsigned char temp;
//	P0 = 0xff;
//	temp = P2 & 0x1f;
//	temp |= 0x80;
//	P2 = temp;
//	temp = P2 & 0x1f;
//	P2 = temp;
//	temp_0_old = 0x00;
//}

idata unsigned char temp_1 = 0x00;
idata unsigned char temp_1_old = 0xff;

void Relay(bit enable)
{
	unsigned char temp;
	if(enable)
		temp_1 |= 0x10;
	else
		temp_1 &= ~(0x10);
	if(temp_1 != temp_1_old)
	{
		P0 = temp_1;
		temp = P2 & 0x1f;
		temp |= 0xa0;
		P2 = temp;
		temp = P2 & 0x1f;
		P2 = temp;
		temp_1_old = temp_1;
	}
}
void Motor(bit enable)
{
	unsigned char temp;
	if(enable)
		temp_1 |= 0x20;
	else
		temp_1 &= ~(0x20);
	if(temp_1 != temp_1_old)
	{
		P0 = temp_1;
		temp = P2 & 0x1f;
		temp |= 0xa0;
		P2 = temp;
		temp = P2 & 0x1f;
		P2 = temp;
		temp_1_old = temp_1;
	}
}