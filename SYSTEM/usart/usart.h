#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "stm32f4xx_conf.h"
#include "sys.h" 

	  	
extern u8 RxBuffer[200],Rxcouter;
//����봮���жϽ��գ��벻Ҫע�����º궨��


void uart3_init(u32 bound);
void uart2_init(u32 bound);



#endif


