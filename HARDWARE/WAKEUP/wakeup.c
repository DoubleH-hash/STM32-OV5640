#include "sys.h"
#include "delay.h" 
#include "timer.h"
#include "wakeup.h"
#include "led.h"



//进入待机模式
void Sys_Enter_Standby(void)
{			 
		DCMI_Cmd(DISABLE);	//DCMI关闭

   	RCC->APB2RSTR|=0X01FC;   //关闭所有外设
	
		Sys_Standby();   //进入待机模式
}




