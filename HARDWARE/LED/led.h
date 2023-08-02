#ifndef __LED_H
#define __LED_H
#include "sys.h"
#include "delay.h"



#define LED3_RED PBout(12)	 
#define LED2_GREEN PBout(13)
#define LED1_WHITE PBout(14)

#define WK_DOWN_JP7   PCin(5)
#define WK_UP_JP6 PBin(0)


#define KEY1_IN   PGin(6)   //急停按键
#define KEY2_IN 	PGin(7)
#define KEY3_IN 	PGin(8)


#define DJ12 PAout(1)
#define DJZFZ PAout(0)



void LED_Init(void);   //led初始化  

void camera_up(void);
void camera_down(void);

#endif
