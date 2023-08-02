#include "led.h"

//LED IO初始化
void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOG, ENABLE);//使能GPIOC F时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);  //SYSCFG时钟使能
    

    //LED : PF 7 8 9   高电平亮
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化
		GPIO_ResetBits(GPIOB,GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14);
	
		//光电开关   WK1 C5  WK2 B0
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		
		//急停按键
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8; 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOG, &GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		

    //ZFZ:PA1   DJ12:PA0   高电平有效
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化
		GPIO_ResetBits(GPIOA,GPIO_Pin_1|GPIO_Pin_0);



    
}

void camera_up(void) {
	u8 emc_stop = 0;
	
run:	
	DJ12 = 1;	
	while(WK_UP_JP6 == 1){    //上升
		DJZFZ = 1;
		delay_ms(10);

		if(KEY1_IN == 0){   //急停
			delay_ms(10);
			while(KEY1_IN == 0);
			emc_stop = 1;
			LED2_GREEN = 0;
			break;
		}
	}
	DJ12 = 0;
	
	while(emc_stop == 1){
		
		if(KEY1_IN == 0){   //取消急停
			delay_ms(10);
			while(KEY1_IN == 0);
			
			LED2_GREEN = 1;
			emc_stop = 0;
			goto run;
		}
		
	}
	
}

void camera_down(void) {
	u8 emc_stop = 0;
	
run:
	DJ12 = 1;
	
	while(WK_DOWN_JP7 == 1){     //下降
		DJZFZ = 0;
		delay_ms(10);
		
		if(KEY1_IN == 0){   //急停
			delay_ms(10);
			while(KEY1_IN == 0);
			emc_stop = 1;
			LED2_GREEN = 0;
			break;
		}
	}
	
	DJ12 = 0;
	
		while(emc_stop == 1){
		
			if(KEY1_IN == 0){   //取消急停
				delay_ms(10);
				while(KEY1_IN == 0);
				emc_stop = 0;
				LED2_GREEN = 1;
				goto run;
			}
		
	}
}


