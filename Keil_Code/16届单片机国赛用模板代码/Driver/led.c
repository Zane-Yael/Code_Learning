#include "led.h"

idata unsigned char temp_0 = 0x00;
idata unsigned char temp_0_old = 0xff;

void Led_Disp(*Led_Buf)
{
	temp_0 = 0x00;
	temp_0 = Led_Buf[0] << 0 | Led_Buf[1] << 1 | Led_Buf[2] << 2 | Led_Buf[3] << 3 |
	Led_Buf[4] << 4 | Led_Buf[5] << 5 | Led_Buf[6] << 6 | Led_Buf[7] << 7;
	if(temp_0 != temp_0_old)
	{
		P0 = ~temp_0;
		P2 = P2 & 0x7f | 0x80;
		P2 &= 0x7f;
		temp_0_old = temp_0;
	}
}

void Led_Off()
{
	P0 = 0xff;
	P2 = P2 & 0x7f | 0x80;
	P2 &= 0x7f;
	temp_0_old = 0x00;
}

idata unsigned char temp_1 = 0x00;
idata unsigned char temp_0_old = 0xff;

void Beep(bit enable)
{
	
}
