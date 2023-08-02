#include "delay.h"
#include "sys.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_UCOS
#include "includes.h"//ucos 使用	  
#endif
//////////////////////////////////////////////////////////////////////////////////  
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//使用SysTick的普通计数模式对延迟进行管理(支持ucosii)
//包括delay_us,delay_ms
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2014/5/2
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved
//********************************************************************************
//修改说明
//无
////////////////////////////////////////////////////////////////////////////////// 

static u8  fac_us=0;//us延时倍乘数
static u16 fac_ms=0;//ms延时倍乘数,在ucos下,代表每个节拍的ms数

#ifdef OS_CRITICAL_METHOD 	//如果OS_CRITICAL_METHOD定义了,说明使用ucosII了.
//systick中断服务函数,使用ucos时用到
void SysTick_Handler(void)
{
    OSIntEnter();       //进入中断
    OSTimeTick();       //调用ucos的时钟服务程序               
    OSIntExit();        //触发任务切换软中断
}
#endif

//初始化延迟函数
//当使用ucos的时候,此函数会初始化ucos的时钟节拍
//SYSTICK的时钟固定为HCLK时钟的1/8
//SYSCLK:系统时钟
void delay_init(u8 SYSCLK)
{
#ifdef OS_CRITICAL_METHOD   //如果OS_CRITICAL_METHOD定义了,说明使用ucosII了.
    u32 reload;
#endif
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
    fac_us=SYSCLK/8;        //不论是否使用ucos,fac_us都需要使用

#ifdef OS_CRITICAL_METHOD   //如果OS_CRITICAL_METHOD定义了,说明使用ucosII了
    reload=SYSCLK/8;        //每秒钟的计数次数 单位为K
    reload*=1000000/OS_TICKS_PER_SEC;//根据OS_TICKS_PER_SEC设定溢出时间
                            //reload为24位寄存器,最大值:16777216,在168M下,约合0.7989s左右
    fac_ms=1000/OS_TICKS_PER_SEC;//代表ucos可以延时的最少单位
    SysTick->CTRL|=SysTick_CTRL_TICKINT_Msk;    //开启SYSTICK中断
    SysTick->LOAD=reload;   //每1/OS_TICKS_PER_SEC秒中断一次
    SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk;     //开启SYSTICK
#else
    fac_ms=(u16)fac_us*1000;//非ucos下,代表每个ms需要的systick时钟数
#endif
}

#ifdef OS_CRITICAL_METHOD   //如果OS_CRITICAL_METHOD定义了,说明使用ucosII了
//延时nus
//nus:要延时的us数
void delay_us(u32 nus)
{
    u32 ticks;
    u32 told,tnow,tcnt=0;
    u32 reload=SysTick->LOAD;//LOAD的值
    ticks=nus*fac_us;//需要的节拍数
    tcnt=0;
    OSSchedLock();//阻止ucos调度，防止打断us延时
    told=SysTick->VAL;          //刚进入时的计数器值
    while(1)
    {
        tnow=SysTick->VAL;	
        if(tnow!=told)
        {
            if(tnow<told)tcnt+=told-tnow;//这里注意一下SYSTICK是一个递减的计数器就可以了
            else tcnt+=reload-tnow+told;
            told=tnow;
            if(tcnt>=ticks)break;//时间超过/等于要延迟的时间,则退出
        }
    };
    OSSchedUnlock();//开启ucos调度
}
//延时nms
//nms:要延时的ms数
void delay_ms(u16 nms)
{
    if(OSRunning==OS_TRUE&&OSLockNesting==0)//如果os已经在跑了
    {
        if(nms>=fac_ms)//延时的时间大于ucos的最少时间周期 
        {
            OSTimeDly(nms/fac_ms);//ucos延时
        }
        nms%=fac_ms;//ucos已经无法提供这么小的延时了,采用普通方式延时    
    }
    delay_us((u32)(nms*1000));//普通方式延时 
}
#else  //不用ucos时
//延时nus
//nus为要延时的us数
//注意:nus的值,不要大于798915us
void delay_us(u32 nus)
{
    u32 temp;
    SysTick->LOAD=nus*fac_us; //时间加载
    SysTick->VAL=0x00;        //清空计数器
    SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;          //开始倒数 
    do
    {
        temp=SysTick->CTRL;
    }
    while((temp&0x01)&&!(temp&(1<<16)));//等待时间到达   
    SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;       //关闭计数器
    SysTick->VAL =0X00;       //清空计数器
}
//延时nms
//注意nms的范围
//SysTick->LOAD为24位寄存器,所以,最大延时为:
//nms<=0xffffff*8*1000/SYSCLK
//SYSCLK单位为Hz,nms单位为ms
//对168M条件下,nms<=798ms 
void delay_xms(u16 nms)
{
    u32 temp;
    SysTick->LOAD=(u32)nms*fac_ms;//时间加载(SysTick->LOAD为24bit)
    SysTick->VAL =0x00;           //清空计数器
    SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;          //开始倒数
    do
    {
        temp=SysTick->CTRL;
    }
    while((temp&0x01)&&!(temp&(1<<16)));//等待时间到达   
    SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;       //关闭计数器
    SysTick->VAL =0X00;       //清空计数器
}

// static u8 delaying_times = 0;//叠加执行延时的次数
// static u16 delaying_finish = 0;//记录最多16个的递归溢出事件中，每一个是否都已经记数溢出
// void delay_xms(u16 nms)
// {
// 	u32 last_systick_val;
// 	if(delaying_times != 0)//如果主程序在跑delay函数的过程中，发生中断并在中断中又进入了delay函数
// 	{
// 		last_systick_val = SysTick->VAL;//将上次的计数器的值保存下来以便退出中断后回去时可以从该值继续递减
// 		//如果上次记数已经溢出，代表着上次的delay已经记数完成，将该次溢出事件记录下来，以便出了中断回到原delay函数时，可以直接跳出while
// 		//delaying_finish是16位的，最多可以记录16次溢出事件，即16层的递归
// 		if(SysTick->CTRL & (1 << 16))delaying_finish |= (1 << (delaying_times - 1));
// 	}
// 	delaying_times ++;
// 	SysTick->LOAD = (u32)fac_ms * nms;//自动重装载值
// 	SysTick->VAL = 0x00;//清除计时器的值
// 	SysTick->CTRL |= (1 << 0);//SysTick使能，使能后定时器开始倒数
// 	while(!(SysTick->CTRL & (1 << 16)))//判断是否减到0，减到0时CTRL的第16位会置1，读取后会自动置0
// 	{
// 		//如果在中断中计数器已经溢出，就退出while,并且对应中断位清零
// 		if(delaying_finish & (1 << (delaying_times- 1)))
// 		{
// 			delaying_finish &= ~(1 << (delaying_times- 1));
// 			break;
// 		}
// 	}
// 	delaying_times --;
// 	if(delaying_times == 0)
// 	{
// 		SysTick->CTRL &= ~(1 << 0);//关闭SysTick，关闭后记数器将不再倒数
// 		SysTick->VAL = 0x00;//清除计时器的值（执行关闭SysTick程序时，记数器又开始了新一轮的倒数，所以关闭后记数器的值不为0）
// 	}
// 	else
// 	{
// 		/* 读取CTRL寄存器的同时，CTRL的第16位会变为0，关闭SysTick后给VAL寄存器赋值再使能的原因
// 		 * 1.若未关闭SysTick，且先将CTRL的第16位清零后再给VAL寄存器赋值，则在赋值的过程中计数器可能会记数到0，从而导致CTRL的第16位又被置1
// 		 * 2.若未关闭SysTick，且先给VAL寄存器赋值后再将CTRL的第16位清零，则在清零的过程中计数器会继续递减并且可能在CTRL的第16位完成清零前就溢出
// 		 * 所以必须关闭SysTick，且赋值完需要再使能使得递归回原函数的while中计数器会继续递减
// 		 */
// 		SysTick->CTRL &= ~(1 << 0);//关闭SysTick，关闭后记数器将不再倒数
// 		SysTick->LOAD = last_systick_val;
// 		SysTick->VAL = 0x00;//清除计时器的值
// 		SysTick->CTRL |= (1 << 0);//SysTick使能，使能后定时器开始倒数
// 	}
// }


//延时nms 
//nms:0~65535
void delay_ms(u16 nms)
{
    u8 repeat=nms/500;	//这里用540,是考虑到某些客户可能超频使用,
                        //比如超频到248M的时候,delay_xms最大只能延时541ms左右了
    u16 remain=nms%500;
    while(repeat)
    {
        delay_xms(500);
        repeat--;
    }
    if(remain)delay_xms(remain);
    
} 
#endif



































