#include "init.h"

void System_Init()
{
	unsigned char temp;
	//---关闭led---//
	P0 = 0xff;
	//开锁存器
	temp = P2 & 0x1f;
	temp |= 0x80;
	P2 = temp;
	//关锁存器
	temp = P2 & 0x1f;
	P2 = temp;
	//---关闭外设---//
	P0 = 0x00;
	//开锁存器
	temp = P2 & 0x1f;
	temp |= 0xa0;
	P2 = temp;
	//关锁存器
	temp = P2 & 0x1f;
	P2 = temp;
}