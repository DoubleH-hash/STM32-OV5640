#include "sys.h"
#include "delay.h" 
#include "timer.h"
#include "wakeup.h"
#include "led.h"



//�������ģʽ
void Sys_Enter_Standby(void)
{			 
		DCMI_Cmd(DISABLE);	//DCMI�ر�

   	RCC->APB2RSTR|=0X01FC;   //�ر���������
	
		Sys_Standby();   //�������ģʽ
}




