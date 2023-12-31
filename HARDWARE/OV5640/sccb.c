#include "sys.h"
#include "sccb.h"
#include "delay.h"


void SCCB_Delay(void)
{
    delay_us(2);
}
//初始化SCCB接口 
void SCCB_Init(void)
{
    RCC->AHB1ENR|=1<<3;     //使能外设PORTD时钟	
    GPIO_Set(GPIOD,PIN6|PIN7,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_50M,GPIO_PUPD_PU);	//PD6,7 推挽输出
    GPIOD->ODR|=3<<6;           //PD6/7输出1  
    SCCB_SDA_OUT();
}

//SCCB起始信号
//当时钟为高的时候,数据线的高到低,为SCCB起始信号
//在激活状态下,SDA和SCL均为低电平
void SCCB_Start(void)
{
    SCCB_SDA=1;     //数据线高电平
    SCCB_SCL=1;     //在时钟线高的时候数据线由高至低
    SCCB_Delay();
    SCCB_SDA=0;
    SCCB_Delay();
    SCCB_SCL=0;     //数据线恢复低电平，单操作函数必要	  
}

//SCCB停止信号
//当时钟为高的时候,数据线的低到高,为SCCB停止信号
//空闲状况下,SDA,SCL均为高电平
void SCCB_Stop(void)
{
    SCCB_SDA=0;
    SCCB_Delay();
    SCCB_SCL=1;
    SCCB_Delay();
    SCCB_SDA=1;
    SCCB_Delay();
}  
//产生NA信号
void SCCB_No_Ack(void)
{
    SCCB_Delay();
    SCCB_SDA=1;	
    SCCB_SCL=1;	
    SCCB_Delay();
    SCCB_SCL=0;	
    SCCB_Delay();
    SCCB_SDA=0;	
    SCCB_Delay();
}
//SCCB,写入一个字节
//返回值:0,成功;1,失败. 
u8 SCCB_WR_Byte(u8 dat)
{
    u8 j,res;
    for(j=0;j<8;j++) //循环8次发送数据
    {
        if(dat&0x80)SCCB_SDA=1;	
        else SCCB_SDA=0;
        dat<<=1;
        SCCB_Delay();
        SCCB_SCL=1;	
        SCCB_Delay();
        SCCB_SCL=0;
    }
    SCCB_SDA_IN();      //设置SDA为输入 
    SCCB_Delay();
    SCCB_SCL=1;//接收第九位,以判断是否发送成功
    SCCB_Delay();
    if(SCCB_READ_SDA)res=1;  //SDA=1发送失败，返回1
    else res=0;         //SDA=0发送成功，返回0
    SCCB_SCL=0;
    SCCB_SDA_OUT();//设置SDA为输出
    return res;  
}
//SCCB 读取一个字节
//在SCL的上升沿,数据锁存
//返回值:读到的数据
u8 SCCB_RD_Byte(void)
{
    u8 temp=0,j;
    SCCB_SDA_IN();      //设置SDA为输入  
    for(j=8;j>0;j--)    //循环8次接收数据
    {
        SCCB_Delay();
        SCCB_SCL=1;
        temp=temp<<1;
        if(SCCB_READ_SDA)temp++;   
        SCCB_Delay();
        SCCB_SCL=0;
    }
    SCCB_SDA_OUT();     //设置SDA为输出    
    return temp;
}














