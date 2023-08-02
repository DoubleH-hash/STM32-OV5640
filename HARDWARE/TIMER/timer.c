#include "timer.h"
#include "led.h"
#include "usart.h"
#include "adc.h"

extern u8 ov_frame;
extern volatile u16 jpeg_data_len;


//ͨ�ö�ʱ��3�жϳ�ʼ��
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//����ʹ�õ��Ƕ�ʱ��3!
void TIM3_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  ///ʹ��TIM3ʱ��

    TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //��ʱ����Ƶ
    TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
    TIM_TimeBaseInitStructure.TIM_Period=arr;   //�Զ���װ��ֵ
    TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
    
    TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);

    TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //����ʱ��3�����ж�
    TIM_Cmd(TIM3,ENABLE); //ʹ�ܶ�ʱ��3

    NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; //��ʱ��3�ж�
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x03; //��ռ���ȼ�1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x01; //�����ȼ�3
    NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
    NVIC_Init(&NVIC_InitStructure);

}

//��ʱ��3�жϷ�����
void TIM3_IRQHandler(void)
{
		float bat = 0.0,temp = 0.0; 
		u16 adcx;
	static unsigned char time_flag = 0;

    if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //����ж�
    {
			//AD�ɼ�
			
			adcx=Get_Adc_Average(ADC_Channel_5,5);
			temp=(float)adcx*3.3/4095.0;
			bat = temp * 6.0;
			
			if(bat < 6.0){
				
				if(time_flag == 0) {
					LED2_GREEN = 0;
					LED3_RED = 0;
					LED1_WHITE = 0;
					 time_flag = 1;
				}
				else{
					LED2_GREEN = 1;
					LED3_RED = 1;
					LED1_WHITE = 1;
					time_flag = 0;
				}
				
			}
			
			ov_frame=0;
    }
    TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //����жϱ�־λ
}
