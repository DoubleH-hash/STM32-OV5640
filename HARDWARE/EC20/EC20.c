#include "ec20.h"
#include "stdlib.h"
#include "string.h"
#include "usart.h"	
#include "rtc.h"
#include "pcf8563.h"

char *strx,*extstrx,*Readystrx;

void Clear_Buffer(void)//��ջ���
{
        u8 i;
        for(i=0;i<200;i++)
					RxBuffer[i]=0;//����
        Rxcouter=0;
}

void EC20_Reset(void) {
    printf("+++"); //�˳�͸��
    delay_ms(1500);
    printf("AT+CFUN=1,1\r\n"); //����EC20
}

void EC200_GPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//ʹ��GPIOCʱ��
	  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
  GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��
	
	EC200_ON = 1;
	delay_ms(1000);
	EC200_ON = 0;
	delay_ms(5000);
	
	EC200_RESET = 1;
	delay_ms(1000);
	EC200_RESET = 0;
	delay_ms(5000);
	
}

unsigned char  EC20_Init(void)
{
				unsigned int timeout = 0;
				unsigned char retry = 0 ;
				
				EC20_Reset();
				delay_ms(1000);
				EC200_GPIOInit();

AT_OK:	
        printf("AT\r\n"); 
        delay_ms(500);
        strx=strstr((const char*)RxBuffer,(const char*)"OK");//����OK
        while(strx==NULL && (timeout < 10))
        {
            delay_ms(500);
            strx=strstr((const char*)RxBuffer,(const char*)"OK");//����OK
					  timeout ++;
        }
				Clear_Buffer();
				if(timeout >= 10 && (retry == 0)) {timeout = 0;retry = 1;goto AT_OK;}
				else if(timeout >= 10 && (retry == 1)) return 1;
				timeout = 0;retry = 0;
					
        printf("ATE0\r\n"); //�رջ���
        delay_ms(500);
        Clear_Buffer();	
        printf("AT+CSQ\r\n"); //���CSQ
        delay_ms(500);
        printf("ATI\r\n"); //���ģ��İ汾��
        delay_ms(500);
				Clear_Buffer();	
        printf("AT+QGPS?\r\n");//��ѯGPS�ϵ����
        delay_ms(500);
        strx=strstr((const char*)RxBuffer,(const char*)"+QGPS: 1");//�����ϵ�����
        if(strx==NULL)
					printf("AT+QGPS=1\r\n");//��ģ������ϵ�
        delay_ms(500);
        Clear_Buffer();
AT_CPIN:
        printf("AT+CPIN?\r\n");//���SIM���Ƿ���λ
        delay_ms(500);
        strx=strstr((const char*)RxBuffer,(const char*)"+CPIN: READY");//�鿴�Ƿ񷵻�ready
        while(strx==NULL && (timeout < 10))
        {
            delay_ms(500);
            strx=strstr((const char*)RxBuffer,(const char*)"+CPIN: READY");//���SIM���Ƿ���λ���ȴ�����λ�������ʶ�𲻵���ʣ��Ĺ�����û������
						if(strx != NULL) break;
						timeout ++;
        }
        Clear_Buffer();
				if(timeout >= 10 && (retry == 0)) {timeout = 0;retry = 1; goto AT_CPIN;}
				else if(timeout >= 10 && (retry == 1)) return 2;
				timeout = 0;retry = 0;
				
AT_CREG:
        printf("AT+CREG?\r\n");//�鿴�Ƿ�ע��GSM����
        delay_ms(500);
        strx=strstr((const char*)RxBuffer,(const char*)"+CREG: 0,1");//��������
        extstrx=strstr((const char*)RxBuffer,(const char*)"+CREG: 0,5");//��������������
        while(strx==NULL && extstrx==NULL && (timeout < 10))
        {
						printf("AT+CREG?\r\n");
            delay_ms(500);
            strx=strstr((const char*)RxBuffer,(const char*)"+CREG: 0,1");//��������
            extstrx=strstr((const char*)RxBuffer,(const char*)"+CREG: 0,5");//��������������
						if(strx != NULL || extstrx != NULL) break;
						timeout ++;
        }
        Clear_Buffer();
				if(timeout >= 10 && (retry == 0))  { timeout = 0;retry = 1; goto AT_CREG;}
				else if(timeout >= 10 && (retry == 1)) return 3;
				timeout = 0;retry = 0;
        
AT_CGREG:
        printf("AT+CGREG?\r\n");//�鿴�Ƿ�ע��GPRS����
        delay_ms(500);
        strx=strstr((const char*)RxBuffer,(const char*)"+CGREG: 0,1");//��������Ҫ��ֻ��ע��ɹ����ſ��Խ���GPRS���ݴ��䡣
        extstrx=strstr((const char*)RxBuffer,(const char*)"+CGREG: 0,5");//��������������
        while(strx==NULL && extstrx==NULL && (timeout < 10))
        {
						printf("AT+CGREG?\r\n");
            delay_ms(500);
            strx=strstr((const char*)RxBuffer,(const char*)"+CGREG: 0,1");//��������Ҫ��ֻ��ע��ɹ����ſ��Խ���GPRS���ݴ��䡣
            extstrx=strstr((const char*)RxBuffer,(const char*)"+CGREG: 0,5");//��������������
						if(strx != NULL || extstrx != NULL) break;
						timeout ++;
        }
        Clear_Buffer();
				if(timeout >= 10 && (retry == 0))  {timeout = 0;retry = 1; goto AT_CGREG;}
				else if(timeout >= 10 && (retry == 1)) return 4;
				timeout = 0;retry = 0;
        
				printf("AT+COPS?\r\n");//�鿴ע�ᵽ�ĸ���Ӫ�̣�֧���ƶ� ��ͨ ���� 
        delay_ms(500);
        Clear_Buffer();
        
				printf("AT+QICLOSE=0\r\n");//�ر�socket����
        delay_ms(500);
        Clear_Buffer();
        
AT_QICSGP:
				printf("AT+QICSGP=1,1,\042CMNET\042,\042\042,\042\042,0\r\n");//����APN�����û���������
        delay_ms(500);
        strx=strstr((const char*)RxBuffer,(const char*)"OK");//�����ɹ�
        while(strx==NULL && (timeout < 10))
        {
            delay_ms(500);
            strx=strstr((const char*)RxBuffer,(const char*)"OK");////�����ɹ�
						if(strx != NULL) break;
						timeout ++;
        }
        Clear_Buffer();
				if(timeout >= 10 && (retry == 0))  {timeout = 0;retry = 1; goto AT_QICSGP;}
				else if(timeout >= 10 && (retry == 1)) return 5;
				timeout = 0;retry = 0;
       
AT_QIDEACT:				
				printf("AT+QIDEACT=1\r\n");//ȥ����
        delay_ms(500);
        strx=strstr((const char*)RxBuffer,(const char*)"OK");//�����ɹ�
        while(strx==NULL && (timeout < 10))
        {
            delay_ms(500);
            strx=strstr((const char*)RxBuffer,(const char*)"OK");//�����ɹ�
						if(strx != NULL) break;
						timeout ++;
        }
        Clear_Buffer();
				if(timeout >= 10 && (retry == 0))  {timeout = 0;retry = 1; goto AT_QIDEACT;}
				else if(timeout >= 10 && (retry == 1)) return 6;
				timeout = 0;retry = 0;
     
AT_QIACT:				
				printf("AT+QIACT=1\r\n");//����
        delay_ms(500);
        strx=strstr((const char*)RxBuffer,(const char*)"OK");//�����ɹ�
        while(strx==NULL && (timeout < 10))
        {
            delay_ms(500);
            strx=strstr((const char*)RxBuffer,(const char*)"OK");//�����ɹ�
						if(strx != NULL) break;
						timeout ++;
        }
        Clear_Buffer();
				if(timeout >= 10 && (retry == 0))  {timeout = 0;retry = 1; goto AT_QIACT;}
				else if(timeout >= 10 && (retry == 1)) return 7;
				timeout = 0;retry = 0;
        
AT_CTZU:
				printf("AT+CTZU=1\r\n");//����ʱ���Զ���ʱ
        delay_ms(100);
        strx=strstr((const char*)RxBuffer,(const char*)"OK");//�����ɹ�
        while(strx==NULL && (timeout < 10))
        {
            delay_ms(100);
            strx=strstr((const char*)RxBuffer,(const char*)"OK");//�����ɹ�
						if(strx != NULL) break;
						timeout ++;
        }
        Clear_Buffer();
				if(timeout >= 10 && (retry == 0))  {timeout = 0;retry = 1; goto AT_CTZU;}
				else if(timeout >= 10 && (retry == 1)) return 8;
				timeout = 0;retry = 0;
				
				EC20_GetUTC();  //��ȡRTCʱ��
				
				printf("AT+QIACT?\r\n");//��ȡ��ǰ����IP��ַ
        delay_ms(500);
        Clear_Buffer();
        
AT_QIOPEN:
				printf("AT+QIOPEN=1,0,\"TCP\",\"111.4.127.27\",1008,0,2\r\n");//����Ϊ͸��ģʽ,͸��ģʽ�󲻿���ʹ��ATָ��
        delay_ms(500);
        strx=strstr((const char*)RxBuffer,(const char*)"CONNECT");//����Ƿ��½�ɹ�
        while(strx==NULL && timeout < 10)
        {
						printf("AT+QIOPEN=1,0,\"TCP\",\"111.4.127.27\",1008,0,2\r\n");
						delay_ms(500);
            strx=strstr((const char*)RxBuffer,(const char*)"CONNECT");//����Ƿ��½�ɹ�
						if(strx != NULL) break;
						timeout ++;
        }
        Clear_Buffer();
        if(timeout >= 10 && (retry == 0))  {timeout = 0;retry = 1; goto AT_QIOPEN;}
				else if(timeout >= 10 && (retry == 1)) return 9;
				
				Clear_Buffer();
				return 0;
}


//ASCII: AT+CCLK?  +CCLK: "20/03/19,07:15:16+32"  OK 
unsigned char EC20_GetUTC()
{
	unsigned char time[6] = {0,0,0,
													0,0,0};   //������ ʱ����
  u8 timeout = 0, retry = 0;
AT_CCLK:
	printf("AT+CCLK?\r\n");
	delay_ms(100);
	
	strx=strstr((const char*)RxBuffer,(const char*)"+CCLK");
	while(strx == NULL && timeout < 10){
		
		printf("AT+CCLK?\r\n");
		delay_ms(500);
		strx=strstr((const char*)RxBuffer,(const char*)"+CCLK");
		if(strx != NULL) break;
    timeout ++;
	}
  if(timeout >= 10 && (retry == 0))  {timeout = 0;retry = 1; goto AT_CCLK;}
	else if(timeout >= 10 && (retry == 1)) return 10;
	
  strx += 8;
	time[0] = (*(strx ) - 0x30) * 10 + (*(strx + 1) - 0x30);
  time[1] = (*(strx + 3) - 0x30) * 10 + (*(strx + 4) - 0x30);
  time[2] = (*(strx + 6) - 0x30) * 10 + (*(strx + 7) - 0x30);

  time[3] = (*(strx + 9) - 0x30) * 10 + (*(strx + 10) - 0x30) + 8;  //UTCʱ�� ת��Ϊ����ʱ��
	if(time[3] > 24) time[3] -= 24;
  time[4] = (*(strx + 12) - 0x30) * 10 + (*(strx + 13) - 0x30);
  time[5] = (*(strx + 15) - 0x30) * 10 + (*(strx + 16) - 0x30);

	Clear_Buffer();
	
	PCF8563_Set_Times(PCF_Format_BIN, PCF_Century_20xx, time[0], time[1], time[2], time[3], time[4], time[5]);
	
  //RTC_Set_Date(time[0], time[1], time[2], RTC_Get_Week(time[0],time[1],time[2]));    //�� ��Ĭ��Ϊ1
	//RTC_Set_Time(time[3], time[4], time[5], RTC_H12_AM);

  return 0;
}

///�����ַ�������
void EC20Send_StrData(char *bufferdata)
{
    u8 untildata;
    printf("AT+QISEND=0\r\n");
    delay_ms(100);
    printf(bufferdata);
    delay_ms(100);	
    USART_SendData(USART2, (u8) 0x1a);//������ɺ���
    while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET){}
    
		delay_ms(100);
    strx=strstr((char*)RxBuffer,(char*)"SEND OK");//�Ƿ���ȷ����
    while(strx==NULL)
    {
        strx=strstr((char*)RxBuffer,(char*)"SEND OK");//�Ƿ���ȷ����
        delay_ms(10);
    }
    delay_ms(100);
    Clear_Buffer();
    
		printf("AT+QISEND=0,0\r\n");
    delay_ms(200);
    strx=strstr((char*)RxBuffer,(char*)"+QISEND:");//����ʣ���ֽ�����
    while(untildata)
    {
        printf("AT+QISEND=0,0\r\n");
        delay_ms(200);
        strx=strstr((char*)RxBuffer,(char*)"+QISEND:");//����ʣ���ֽ�����
        strx=strstr((char*)strx,(char*)",");//��ȡ��һ��,
        strx=strstr((char*)(strx+1),(char*)",");//��ȡ�ڶ���,
        untildata=*(strx+1)-0x30;
        Clear_Buffer();
    }

}


///����ʮ������
void EC20Send_HexData(char *bufferdata)
{
    u8 untildata;
    printf("AT+QISENDEX=0,\042%s\042\r\n",bufferdata);
    delay_ms(100);
    USART_SendData(USART2, (u8) 0x1a);//������ɺ���
    while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET){}
    delay_ms(100);
    strx=strstr((char*)RxBuffer,(char*)"OK");//�Ƿ���ȷ����
    while(strx==NULL)
    {
        strx=strstr((char*)RxBuffer,(char*)"OK");//�Ƿ���ȷ����
        delay_ms(10);
    }
    delay_ms(100);
    Clear_Buffer();
    printf("AT+QISEND=0,0\r\n");
    delay_ms(200);
    strx=strstr((char*)RxBuffer,(char*)"+QISEND:");//����ʣ���ֽ�����
    while(untildata)
    {
        printf("AT+QISEND=0,0\r\n");
        delay_ms(200);
        strx=strstr((char*)RxBuffer,(char*)"+QISEND:");//����ʣ���ֽ�����
        strx=strstr((char*)strx,(char*)",");//��ȡ��һ��,
        strx=strstr((char*)(strx+1),(char*)",");//��ȡ�ڶ���,
        untildata=*(strx+1)-0x30;
        Clear_Buffer();
    }
}


///͸��ģʽ�½�������
void EC20Send_RecAccessMode(void)
{
    strx=strstr((const char*)RxBuffer,(const char*)"Rec data");//���յ��������·�����
    if(strx)
    {
        printf("Rec data is ok\r\n");//���ݷ���ȥ
        delay_ms(500);
        Clear_Buffer();
    }
    strx=strstr((const char*)RxBuffer,(const char*)"CLOSED");//�����������ر�
    if(strx)
    {
        while(1)
        {
           //���Ź���λ����ȻҲ���Լ��������Ĵ�ʩ������
        }
    }
}

