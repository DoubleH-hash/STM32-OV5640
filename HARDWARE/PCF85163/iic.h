/******************************************************************************
* @ File name --> iic.h
* @ Author    --> By@ shun
* @ Version   --> V1.0
* @ Date      --> 07 - 10 - 2012
* @ Brief     --> MCU模拟IIC通讯函数
* @           --> 要改变传输频率，请修改延时函数中的数值即可
******************************************************************************/

#ifndef _iic_h_ 
#define _iic_h_

/******************************************************************************
                                 外部函数头文件                        
******************************************************************************/
#include "sys.h"

/******************************************************************************
                                 外部引脚修改区                        
******************************************************************************/
/*  IIC_SCL时钟端口、引脚定义 */
#define IIC_SCL_PORT 			GPIOB   
#define IIC_SCL_PIN 			(GPIO_Pin_8)
#define IIC_SCL_PORT_RCC		RCC_AHB1Periph_GPIOB

/*  IIC_SDA时钟端口、引脚定义 */
#define IIC_SDA_PORT 			GPIOB  
#define IIC_SDA_PIN 			(GPIO_Pin_9)
#define IIC_SDA_PORT_RCC		RCC_AHB1Periph_GPIOB


/******************************************************************************
                             对于低速晶振的支持
                     是否使用延时函数进行调整通讯频率
******************************************************************************/

#define _USER_DELAY_CLK					1	//定义了则使用延时调整通讯频率
											//0：不使用延时函数调整通讯频率，对于低速MCU时候用
											//1：使用延时函数调整通讯频率，对于高速MCU时候用

/******************************************************************************
                                位带操作
******************************************************************************/
#define IIC_SCL					PBout(8)
#define IIC_SDA					PBout(9)	//IIC发送数据用
#define IN_SDA					PBin(9)	    //IIC读取数据用


/******************************************************************************
                               通讯频率延时函数
                    需要调整通讯频率的请修改此函数值即可
******************************************************************************/

#if	_USER_DELAY_CLK==1	//定义了则使用

	#define IIC_Delay()                 delay_us(2) //要改变请修改delay_us()中的数值即可

#endif

/******************************************************************************
                                 外部功能函数
******************************************************************************/

void IIC_GPIO_Init(void);	    //GPIO初始化

void IIC_Start(void);	        //IIC启动

void IIC_Stop(void);	        //IIC停止

void IIC_Ack(u8 a);	            //主机向从机发送应答信号

u8 IIC_Write_Byte(u8 dat);	    //向IIC总线发送一个字节数据

u8 IIC_Read_Byte(void);	        //从IIC总线上读取一个字节数据



#endif  /* end iic.h */




