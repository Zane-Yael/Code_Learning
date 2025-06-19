#include "seg.h"

pdata unsigned char Seg_Dula[] = {0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xff};

void Seg_Disp(unsigned char wela,unsigned char dula,bit point)
{
	unsigned char temp;
	//消隐
	P0 = 0xff;
	//开锁存器
	temp = P2 & 0x1f;
	temp |= 0xe0;
	P2 = temp;
	//关锁存器
	temp = P2 & 0x1f;
	P2 = temp;
	
	//位选
	P0 = 0x01 << wela;
	//开锁存器
	temp = P2 & 0x1f;
	temp |= 0xc0;
	P2 = temp;
	//关锁存器
	temp = P2 & 0x1f;
	P2 = temp;
	
	//段选
	P0 = Seg_Dula[dula];
	if(point)
		P0 &= 0x7f;
	//开锁存器
	temp = P2 & 0x1f;
	temp |= 0xe0;
	P2 = temp;
	//关锁存器
	temp = P2 & 0x1f;
	P2 = temp;
}