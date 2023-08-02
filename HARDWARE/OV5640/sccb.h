#ifndef __SCCB_H
#define __SCCB_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//OV系列摄像头 SCCB 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2015/12/30
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 


//IO方向设置
#define SCCB_SDA_IN()  {GPIOD->MODER&=~(3<<(6*2));GPIOD->MODER|=0<<6*2;}	//PD7 输入
#define SCCB_SDA_OUT() {GPIOD->MODER&=~(3<<(6*2));GPIOD->MODER|=1<<6*2;} 	//PD7 输出


//IO操作函数	 
#define SCCB_SCL    		PDout(7)	 	//SCL
#define SCCB_SDA    		PDout(6) 		//SDA	 

#define SCCB_READ_SDA    	PDin(6)  		//输入SDA     

///////////////////////////////////////////
void SCCB_Init(void);
void SCCB_Start(void);
void SCCB_Stop(void);
void SCCB_No_Ack(void);
u8 SCCB_WR_Byte(u8 dat);
u8 SCCB_RD_Byte(void); 
#endif













