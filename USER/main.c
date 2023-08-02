#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "timer.h" 
#include "ov5640.h" 
#include "dcmi.h" 
#include "ec20.h"
#include <stdlib.h>
#include <string.h>
#include "rtc.h"
#include "wakeup.h"
#include "sram.h"
#include "pcf8563.h"
#include "adc.h"

#define dma_half_buf_size 5*1024
int jpeg_buf_size= dma_half_buf_size;

__align(4) u32 dma_half_buf1[dma_half_buf_size];
__align(4) u32 dma_half_buf2[dma_half_buf_size];

#define SRAM_buf_size 200 * 1024

#if (__ARMCC_VERSION >= 6010050)
__align(4) u32 SRAM_jpeg_buf[SRAM_buf_size] __attribute__((section(".bss.ARM.__at_0x68000000")));
#else
__align(4) u32 SRAM_jpeg_buf[SRAM_buf_size] __attribute__((at(0X68000000)));
#endif


volatile u32 jpeg_data_len=0;           //buf中的JPEG有效数据长度 
volatile u8 jpeg_data_ok=0;             //JPEG数据采集完成标志 
                                        //0,数据没有采集完;
                                        //1,数据采集完了,但是还没处理;
                                        //2,数据已经处理完成了,可以开始下一帧接收
volatile u32 nowdmalen=0;
volatile u32 dmatime = 0;

typedef struct
{
	_PCF8563_Date_Typedef date;
	_PCF8563_Time_Typedef time;
	_PCF8563_Alarm_Typedef alarm;
	u8 centry;
	
}PCF8563;

//JPEG尺寸支持列表
const u16 jpeg_img_size_tbl[][2]=
{
    176,144,    //QCIF
    160,120,    //QQVGA
    352,288,    //CIF
    320,240,    //QVGA
    640,480,    //VGA
    800,600,    //SVGA
    1024,768,   //XGA
    1280,1024,  //SXGA
    1600,1200,  //UXGA
}; 

//查找FFD8
static u8 * find_ffd8(int xi ,u8 * buf, int size)
{
    int i = xi;
    u8 x8[] = {0xFF,0xD8};
    u16 *x16 = (u16*)&x8;
    for(;i<size;i++)
    {
        if (i == size-1)
            return 0;
        if (*x16 == *(u16*)(buf+i))
        {
            return buf+i;
        }
    }
    return 0;
}

//查找FFD9
static u8 * find_ffd9(int xi ,u8 * buf, int size)
{
    int i = xi;
    u8 x8[] = {0xFF,0xD9};
    u16 *x16 = (u16*)&x8;
    for(;i<size;i++)
    {
        if (i == size-1)
            return 0;
        if (*x16 == *(u16*)(buf+i))
            return buf+i;
    }
    return 0;
}

//获取图片数据
static char * get_img_data(uint32_t * len)
{
    u8 * _res ;	   				//start jpeg ff d8
    u8 * _res2;
		
		_res = find_ffd8(0,(u8*)SRAM_jpeg_buf, nowdmalen * 4);
		_res2 = find_ffd9(_res-(u8*)SRAM_jpeg_buf,(u8*)SRAM_jpeg_buf, nowdmalen * 4);
	
    if (_res>0 && _res2>0)
    {
        *len = (_res2-_res) + 2;	  //len
        return (char*)_res;			  //start
    }
    return 0;
}

void camera_start(void)
{ 
    OV5640_WR_Reg(0X3017,0XFF);	//开启OV5650输出
    OV5640_WR_Reg(0X3018,0XFF); 
    //GPIOC8/9/11切换为 DCMI接口
    //GPIO_AF_Set(GPIOE,0,13);	//PC8,AF13  DCMI_D2
    //GPIO_AF_Set(GPIOE,1,13);	//PC9,AF13  DCMI_D3
    //GPIO_AF_Set(GPIOC,11,13);	//PC11,AF13 DCMI_D4  
}

void camera_stop(void)
{
    OV5640_WR_Reg(0X3017,0X00);	//关闭OV5650输出
    OV5640_WR_Reg(0X3018,0X00); 
}

//处理JPEG数据
//当采集完一帧JPEG数据后,调用此函数,切换JPEG BUF.开始下一帧采集.
void jpeg_data_process(void)
{
		u32 rlen = 0,i = 0;
		u32 *pbuf;
	
    if(1)//只有在JPEG格式下,才需要做处理.
    {	
			if(jpeg_data_ok==0){
				DMA_Cmd(DMA2_Stream1, DISABLE);//停止当前传输
        while (DMA_GetCmdStatus(DMA2_Stream1) != DISABLE){}//等待DMA2_Stream1可配置
				rlen = dma_half_buf_size - DMA_GetCurrDataCounter(DMA2_Stream1);
				
				pbuf = SRAM_jpeg_buf + nowdmalen;
			
				if(DMA2_Stream1->CR&(1<<19)){   //读取buff1的数据
					for(i = 0; i < rlen ; i++)
					{
						pbuf[i] = dma_half_buf2[i];
					}
					
				}
				else{					//读取buff0的数据
					for(i = 0; i < rlen ; i++)
					{
						pbuf[i] = dma_half_buf1[i];
						
					}
				}
				
				jpeg_data_ok=1;     //标记JPEG数据采集完按成,等待其他函数处理
			}
			if(jpeg_data_ok==2){
				DMA_SetCurrDataCounter(DMA2_Stream1,dma_half_buf_size);
				DMA_Cmd(DMA2_Stream1, ENABLE);//重新传输
					
				nowdmalen = 0;
				jpeg_data_ok = 0;
				
			}
    }
}

void DMA_CallBack(void)//DMA传输完成中断
{
		u32 i = 0 ;
		u32 *pbuf ;
		
		pbuf = SRAM_jpeg_buf + nowdmalen;
	
		if(DMA2_Stream1->CR&(1<<19) && dmatime == 0){
				
			for(i = 0; i < dma_half_buf_size ; i++)
			{
					//SRAM_jpeg_buf[i + dma_nowaddr] = dma_half_buf1[i];
					pbuf[i] = dma_half_buf1[i];
			}
			nowdmalen += dma_half_buf_size;
			
			dmatime = 1;
		}
		else if(dmatime == 1)
		{
			for(i = 0; i < dma_half_buf_size ; i++)
			{
					//SRAM_jpeg_buf[i + dma_nowaddr] = dma_half_buf2[i];
					pbuf[i] = dma_half_buf2[i];
			}
			nowdmalen += dma_half_buf_size;
			
			dmatime = 0;
		}
		
}

u8 send_JPEG(char *p,u32 jpeglength)
{
		u32 i = 0;
		unsigned char len[4] ;
		
		len[0] = (jpeglength ) & 0xff;
		len[1] = (jpeglength >> 8) & 0xff;
		len[2] = (jpeglength >> 16) & 0xff;
		len[3] = (jpeglength >> 24) & 0xff;
	
		for(i = 0 ; i < 4; i++)
		{
			while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET);
			USART_SendData(USART2,len[i]);
		}
	
    for (i=0; i < jpeglength; i++)
    {
       while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET);    //循环发送,直到发送完毕
       USART_SendData(USART2,p[i]);
				
			LED1_WHITE=!LED1_WHITE;
    }
		
		LED1_WHITE = 0;
		return 1;
}

u8  JPEG_istrue(char *p,u32 jpeglength)
{
	u8 ff_num = 0;
	u32 i = 0;
	
	if(jpeglength == 0) return 0;
	
	for(i = jpeglength - 30; i < jpeglength; i++){
		if(*(p + i) == 0xff )
		{
			ff_num ++;
		}
	}
	
	if(ff_num > 17) return 0;   //阈值越低 精度越高
	else return 1;
	
}

//JPEG测试
//JPEG数据,通过串口2发送给电脑.
void take_photo(void)
{
    char *p;
		u32 jpeglength = 0;
		u32 i = 0;
	
		nowdmalen = 0;
		jpeg_data_ok = 0;
		dmatime = 0;

    OV5640_Focus_Single();//对焦
    DCMI_Start();       //启动传输
	
    for(i = 0;i < 5;i++)   //前面的图片不要 摄像头刚启动
    {
        while(jpeg_data_ok!=1);	//等待第一帧图片采集完
        jpeg_data_ok=2;         //忽略本帧图片,启动下一帧采集
        while(jpeg_data_ok!=1);	//等待第二帧图片采集完
        jpeg_data_ok=2;         //忽略本帧图片,启动下一帧采集
    }
		
		delay_ms(1000);
		
		while(jpeg_data_ok!=1);	//等待第二帧图片采集完
        jpeg_data_ok=2;         //忽略本帧图片,启动下一帧采集
		
		//OV5640_Flash_Ctrl(1);//打开闪光灯
		//OV5640_Flash_Ctrl(0);//关闭闪光灯
		
    while(1)
    {
			if(jpeg_data_ok==1) //已经采集完一帧图像了
			{
					p=get_img_data(&jpeglength);    //获取图片
				
					if(JPEG_istrue(p,jpeglength))
					{
						send_JPEG(p,jpeglength);
						
						break;
					}
					
					jpeg_data_ok = 2;  //标记jpeg数据处理完了,可以让DMA去采集下一帧了.
			}
    }
		
		DCMI_Stop();
}

int main(void)
{
		//RTC_TimeTypeDef RTC_TimeStruct;
		PCF8563 my_pcf8563;
		u8 i = 0;
		u8 run_camera_flag = 0;
	
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
    delay_init(168);  //初始化延时函数

		//My_RTC_Init();		//初始化RTC   目前的电路中无低速晶振

		uart2_init(115200);  
    uart3_init(115200);     //初始化串口3波特率为115200
	
		Adc_Init();
		TIM3_Int_Init(10000-1,8400-1);//10Khz计数,1秒钟中断一次
	
    LED_Init();             //初始化LED
	
		while(PCF8563_Init()){
			delay_ms(200);
			LED3_RED = !LED3_RED;
		}
		/*
		my_pcf8563.alarm.RTC_AlarmNewState = RTC_AlarmNewState_Open_INT_Enable;  //打开闹钟，并打开中断
		my_pcf8563.alarm.RTC_AlarmType = PCF_Alarm_DaysOpen;
		my_pcf8563.alarm.RTC_AlarmHours = 9;
		my_pcf8563.alarm.RTC_AlarmMinutes = 30;
		PCF8563_SetAlarm(PCF_Format_BIN, &my_pcf8563.alarm);   //设置闹钟
		*/
		
		/*while(1){
			if(PCF8563_isAlarm())
			{
				PCF8563_ClearAlarm();
			}
		}*/

		//__set_FAULTMASK(1); //关闭总中断
    //NVIC_SystemReset(); //请求单片机重启
	
		FSMC_SRAM_Init();
	
		while(OV5640_Init())//初始化OV2640
    {
			for(i = 0; i < 10; i++){
				LED3_RED = 1;
        delay_ms(200);
				LED3_RED = 0;
			}
    }
   
    OV5640_JPEG_Mode();
    OV5640_Focus_Init();
    OV5640_Light_Mode(0);   //自动模式
    OV5640_Color_Saturation(3);//色彩饱和度0
    OV5640_Brightness(4);   //亮度0
    OV5640_Contrast(3);     //对比度0
    OV5640_Sharpness(33);   //自动锐度
    OV5640_Focus_Constant();//启动持续对焦
		//OV5640_Exposure(4);   //exposure:0~6,代表补偿-3~3.
		OV5640_Special_Effects(3);   //特效为黑白
		
		
		while(EC20_Init()!= 0)    //EC20初始化  同时将RTC时间进行同步
		{
			delay_ms(200);
			LED3_RED = !LED3_RED;
		}
		
		//RTC_GetTime(RTC_Format_BIN,&RTC_TimeStruct);  //获取一下RTC时间
		
		PCF8563_GetTime(PCF_Format_BIN, &my_pcf8563.time);
		PCF8563_GetDate(PCF_Format_BIN, &my_pcf8563.centry, &my_pcf8563.date); //获取外部RTC时间

    u8 size= 6;  //默认是SVGA 640*480尺寸   6
    MY_DCMI_Init();//DCMI配置
    
		DCMI_DMA_Double_Init((u32)&dma_half_buf1, (u32)&dma_half_buf2, dma_half_buf_size);
		
    OV5640_OutSize_Set(16,4,jpeg_img_size_tbl[size][0],jpeg_img_size_tbl[size][1]);    //尺寸为  320,240
		
    while(1) {

			PCF8563_GetTime(PCF_Format_BIN, &my_pcf8563.time); 
			
			if(run_camera_flag == 0 && my_pcf8563.time.RTC_Hours == 8 && my_pcf8563.time.RTC_Minutes == 30 && my_pcf8563.time.RTC_Seconds == 0){
				run_camera_flag = 1;
			}
			if(run_camera_flag == 0 && my_pcf8563.time.RTC_Hours == 12 && my_pcf8563.time.RTC_Minutes == 0 && my_pcf8563.time.RTC_Seconds == 0){
				run_camera_flag = 1;
			}
			if(run_camera_flag == 0 && my_pcf8563.time.RTC_Hours == 17 && my_pcf8563.time.RTC_Minutes == 0 && my_pcf8563.time.RTC_Seconds == 0){
				run_camera_flag = 1;
			}
			if(run_camera_flag == 0 && my_pcf8563.time.RTC_Hours == 22 && my_pcf8563.time.RTC_Minutes == 0 && my_pcf8563.time.RTC_Seconds == 0){
				run_camera_flag = 1;
			}
			
			
			if(run_camera_flag){
        LED2_GREEN = 1;
				camera_up();
        delay_ms(1 * 1000);
				
        take_photo();
			
        camera_down();
        LED2_GREEN = 0;
        delay_ms(5 * 1000);
				
				run_camera_flag = 0;
			}
			
			delay_ms(1000);

			//一次动作完成后，配置WAKEUP中断，进入休眠
			//RTC_GetWakeupSec(RTC_TimeStruct);  //获取从当前时间到第二天9点共需要的秒
			//RTC_Set_WakeUp(RTC_WakeUpClock_CK_SPRE_16bits, 2400);  //16分频  2400S
			//Sys_Enter_Standby(); //进入待机模式
    }
}
