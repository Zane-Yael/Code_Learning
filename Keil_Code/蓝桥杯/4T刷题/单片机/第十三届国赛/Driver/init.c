#include "init.h"

void System_Init()
{
	unsigned char temp;
	
	P0 = 0xff;
	temp = P2 & 0x1f;
	temp |= 0x80;
	P2 = temp;
	temp = P2 & 0x1f;
	P2 = temp;
	
	P0 = 0x00;
	temp = P2 & 0x1f;
	temp |= 0xa0;
	P2 = temp;
	temp = P2 & 0x1f;
	P2 = temp;
}