#ifndef __EC20_H
#define __EC20_H
#include "sys.h"
#include "delay.h"
#include "usart.h"



#define EC200_ON PCout(1)	 
#define EC200_RESET PCout(2)	 
	


void EC200_GPIOInit(void);

void EC20_Reset(void);
unsigned char  EC20_Init(void);
void EC20Send_StrData(char *bufferdata);
void EC20Send_HexData(char *bufferdata);		
void Clear_Buffer(void);
void EC20Send_RecAccessMode(void);

unsigned char EC20_GetUTC(void);

#endif
