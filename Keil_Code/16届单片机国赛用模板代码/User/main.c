#include <STC12C5A60S2.H>
#include "led.h"
#include "key.h"
#include "seg.h"
#include "ds1302.h"
#include "ds18b30.h"
#include "iic.h"
#include "uart.h"
#include "ultrasonic.h"
#include "init.h"
#include "math.h"
#include "string.h"
#include "stdio.h"

/*======变量======*/
xdata unsigned char Led_Buf[8] = {0,0,0,0,0,0,0,0};
idata unsigned char Key_Val,Key_Down,Key_Up,Key_Old;
idata unsigned char Key_Slow_Down; 
xdata unsigned char Seg_Buf[8] = {10,10,10,10,10,10,10,10};
idata unsigned char Seg_Pos;
idata unsigned char Seg_Slow_Down;