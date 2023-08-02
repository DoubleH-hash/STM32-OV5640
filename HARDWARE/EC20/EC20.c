#include "ec20.h"
#include "stdlib.h"
#include "string.h"
#include "usart.h"	
#include "rtc.h"
#include "pcf8563.h"

char *strx,*extstrx,*Readystrx;

void Clear_Buffer(void)//清空缓存
{
        u8 i;
        for(i=0;i<200;i++)
					RxBuffer[i]=0;//缓存
        Rxcouter=0;
}

void EC20_Reset(void) {
    printf("+++"); //退出透传
    delay_ms(1500);
    printf("AT+CFUN=1,1\r\n"); //重启EC20
}

void EC200_GPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIOC时钟
	  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化
	
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
        strx=strstr((const char*)RxBuffer,(const char*)"OK");//返回OK
        while(strx==NULL && (timeout < 10))
        {
            delay_ms(500);
            strx=strstr((const char*)RxBuffer,(const char*)"OK");//返回OK
					  timeout ++;
        }
				Clear_Buffer();
				if(timeout >= 10 && (retry == 0)) {timeout = 0;retry = 1;goto AT_OK;}
				else if(timeout >= 10 && (retry == 1)) return 1;
				timeout = 0;retry = 0;
					
        printf("ATE0\r\n"); //关闭回显
        delay_ms(500);
        Clear_Buffer();	
        printf("AT+CSQ\r\n"); //检查CSQ
        delay_ms(500);
        printf("ATI\r\n"); //检查模块的版本号
        delay_ms(500);
				Clear_Buffer();	
        printf("AT+QGPS?\r\n");//查询GPS上电情况
        delay_ms(500);
        strx=strstr((const char*)RxBuffer,(const char*)"+QGPS: 1");//返回上电正常
        if(strx==NULL)
					printf("AT+QGPS=1\r\n");//对模块进行上电
        delay_ms(500);
        Clear_Buffer();
AT_CPIN:
        printf("AT+CPIN?\r\n");//检查SIM卡是否在位
        delay_ms(500);
        strx=strstr((const char*)RxBuffer,(const char*)"+CPIN: READY");//查看是否返回ready
        while(strx==NULL && (timeout < 10))
        {
            delay_ms(500);
            strx=strstr((const char*)RxBuffer,(const char*)"+CPIN: READY");//检查SIM卡是否在位，等待卡在位，如果卡识别不到，剩余的工作就没法做了
						if(strx != NULL) break;
						timeout ++;
        }
        Clear_Buffer();
				if(timeout >= 10 && (retry == 0)) {timeout = 0;retry = 1; goto AT_CPIN;}
				else if(timeout >= 10 && (retry == 1)) return 2;
				timeout = 0;retry = 0;
				
AT_CREG:
        printf("AT+CREG?\r\n");//查看是否注册GSM网络
        delay_ms(500);
        strx=strstr((const char*)RxBuffer,(const char*)"+CREG: 0,1");//返回正常
        extstrx=strstr((const char*)RxBuffer,(const char*)"+CREG: 0,5");//返回正常，漫游
        while(strx==NULL && extstrx==NULL && (timeout < 10))
        {
						printf("AT+CREG?\r\n");
            delay_ms(500);
            strx=strstr((const char*)RxBuffer,(const char*)"+CREG: 0,1");//返回正常
            extstrx=strstr((const char*)RxBuffer,(const char*)"+CREG: 0,5");//返回正常，漫游
						if(strx != NULL || extstrx != NULL) break;
						timeout ++;
        }
        Clear_Buffer();
				if(timeout >= 10 && (retry == 0))  { timeout = 0;retry = 1; goto AT_CREG;}
				else if(timeout >= 10 && (retry == 1)) return 3;
				timeout = 0;retry = 0;
        
AT_CGREG:
        printf("AT+CGREG?\r\n");//查看是否注册GPRS网络
        delay_ms(500);
        strx=strstr((const char*)RxBuffer,(const char*)"+CGREG: 0,1");//，这里重要，只有注册成功，才可以进行GPRS数据传输。
        extstrx=strstr((const char*)RxBuffer,(const char*)"+CGREG: 0,5");//返回正常，漫游
        while(strx==NULL && extstrx==NULL && (timeout < 10))
        {
						printf("AT+CGREG?\r\n");
            delay_ms(500);
            strx=strstr((const char*)RxBuffer,(const char*)"+CGREG: 0,1");//，这里重要，只有注册成功，才可以进行GPRS数据传输。
            extstrx=strstr((const char*)RxBuffer,(const char*)"+CGREG: 0,5");//返回正常，漫游
						if(strx != NULL || extstrx != NULL) break;
						timeout ++;
        }
        Clear_Buffer();
				if(timeout >= 10 && (retry == 0))  {timeout = 0;retry = 1; goto AT_CGREG;}
				else if(timeout >= 10 && (retry == 1)) return 4;
				timeout = 0;retry = 0;
        
				printf("AT+COPS?\r\n");//查看注册到哪个运营商，支持移动 联通 电信 
        delay_ms(500);
        Clear_Buffer();
        
				printf("AT+QICLOSE=0\r\n");//关闭socket连接
        delay_ms(500);
        Clear_Buffer();
        
AT_QICSGP:
				printf("AT+QICSGP=1,1,\042CMNET\042,\042\042,\042\042,0\r\n");//接入APN，无用户名和密码
        delay_ms(500);
        strx=strstr((const char*)RxBuffer,(const char*)"OK");//开启成功
        while(strx==NULL && (timeout < 10))
        {
            delay_ms(500);
            strx=strstr((const char*)RxBuffer,(const char*)"OK");////开启成功
						if(strx != NULL) break;
						timeout ++;
        }
        Clear_Buffer();
				if(timeout >= 10 && (retry == 0))  {timeout = 0;retry = 1; goto AT_QICSGP;}
				else if(timeout >= 10 && (retry == 1)) return 5;
				timeout = 0;retry = 0;
       
AT_QIDEACT:				
				printf("AT+QIDEACT=1\r\n");//去激活
        delay_ms(500);
        strx=strstr((const char*)RxBuffer,(const char*)"OK");//开启成功
        while(strx==NULL && (timeout < 10))
        {
            delay_ms(500);
            strx=strstr((const char*)RxBuffer,(const char*)"OK");//开启成功
						if(strx != NULL) break;
						timeout ++;
        }
        Clear_Buffer();
				if(timeout >= 10 && (retry == 0))  {timeout = 0;retry = 1; goto AT_QIDEACT;}
				else if(timeout >= 10 && (retry == 1)) return 6;
				timeout = 0;retry = 0;
     
AT_QIACT:				
				printf("AT+QIACT=1\r\n");//激活
        delay_ms(500);
        strx=strstr((const char*)RxBuffer,(const char*)"OK");//开启成功
        while(strx==NULL && (timeout < 10))
        {
            delay_ms(500);
            strx=strstr((const char*)RxBuffer,(const char*)"OK");//开启成功
						if(strx != NULL) break;
						timeout ++;
        }
        Clear_Buffer();
				if(timeout >= 10 && (retry == 0))  {timeout = 0;retry = 1; goto AT_QIACT;}
				else if(timeout >= 10 && (retry == 1)) return 7;
				timeout = 0;retry = 0;
        
AT_CTZU:
				printf("AT+CTZU=1\r\n");//激活时间自动对时
        delay_ms(100);
        strx=strstr((const char*)RxBuffer,(const char*)"OK");//开启成功
        while(strx==NULL && (timeout < 10))
        {
            delay_ms(100);
            strx=strstr((const char*)RxBuffer,(const char*)"OK");//开启成功
						if(strx != NULL) break;
						timeout ++;
        }
        Clear_Buffer();
				if(timeout >= 10 && (retry == 0))  {timeout = 0;retry = 1; goto AT_CTZU;}
				else if(timeout >= 10 && (retry == 1)) return 8;
				timeout = 0;retry = 0;
				
				EC20_GetUTC();  //获取RTC时间
				
				printf("AT+QIACT?\r\n");//获取当前卡的IP地址
        delay_ms(500);
        Clear_Buffer();
        
AT_QIOPEN:
				printf("AT+QIOPEN=1,0,\"TCP\",\"111.4.127.27\",1008,0,2\r\n");//设置为透传模式,透传模式后不可以使用AT指令
        delay_ms(500);
        strx=strstr((const char*)RxBuffer,(const char*)"CONNECT");//检查是否登陆成功
        while(strx==NULL && timeout < 10)
        {
						printf("AT+QIOPEN=1,0,\"TCP\",\"111.4.127.27\",1008,0,2\r\n");
						delay_ms(500);
            strx=strstr((const char*)RxBuffer,(const char*)"CONNECT");//检查是否登陆成功
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
													0,0,0};   //年月日 时分秒
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

  time[3] = (*(strx + 9) - 0x30) * 10 + (*(strx + 10) - 0x30) + 8;  //UTC时间 转换为北京时间
	if(time[3] > 24) time[3] -= 24;
  time[4] = (*(strx + 12) - 0x30) * 10 + (*(strx + 13) - 0x30);
  time[5] = (*(strx + 15) - 0x30) * 10 + (*(strx + 16) - 0x30);

	Clear_Buffer();
	
	PCF8563_Set_Times(PCF_Format_BIN, PCF_Century_20xx, time[0], time[1], time[2], time[3], time[4], time[5]);
	
  //RTC_Set_Date(time[0], time[1], time[2], RTC_Get_Week(time[0],time[1],time[2]));    //周 先默认为1
	//RTC_Set_Time(time[3], time[4], time[5], RTC_H12_AM);

  return 0;
}

///发送字符型数据
void EC20Send_StrData(char *bufferdata)
{
    u8 untildata;
    printf("AT+QISEND=0\r\n");
    delay_ms(100);
    printf(bufferdata);
    delay_ms(100);	
    USART_SendData(USART2, (u8) 0x1a);//发送完成函数
    while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET){}
    
		delay_ms(100);
    strx=strstr((char*)RxBuffer,(char*)"SEND OK");//是否正确发送
    while(strx==NULL)
    {
        strx=strstr((char*)RxBuffer,(char*)"SEND OK");//是否正确发送
        delay_ms(10);
    }
    delay_ms(100);
    Clear_Buffer();
    
		printf("AT+QISEND=0,0\r\n");
    delay_ms(200);
    strx=strstr((char*)RxBuffer,(char*)"+QISEND:");//发送剩余字节数据
    while(untildata)
    {
        printf("AT+QISEND=0,0\r\n");
        delay_ms(200);
        strx=strstr((char*)RxBuffer,(char*)"+QISEND:");//发送剩余字节数据
        strx=strstr((char*)strx,(char*)",");//获取第一个,
        strx=strstr((char*)(strx+1),(char*)",");//获取第二个,
        untildata=*(strx+1)-0x30;
        Clear_Buffer();
    }

}


///发送十六进制
void EC20Send_HexData(char *bufferdata)
{
    u8 untildata;
    printf("AT+QISENDEX=0,\042%s\042\r\n",bufferdata);
    delay_ms(100);
    USART_SendData(USART2, (u8) 0x1a);//发送完成函数
    while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET){}
    delay_ms(100);
    strx=strstr((char*)RxBuffer,(char*)"OK");//是否正确发送
    while(strx==NULL)
    {
        strx=strstr((char*)RxBuffer,(char*)"OK");//是否正确发送
        delay_ms(10);
    }
    delay_ms(100);
    Clear_Buffer();
    printf("AT+QISEND=0,0\r\n");
    delay_ms(200);
    strx=strstr((char*)RxBuffer,(char*)"+QISEND:");//发送剩余字节数据
    while(untildata)
    {
        printf("AT+QISEND=0,0\r\n");
        delay_ms(200);
        strx=strstr((char*)RxBuffer,(char*)"+QISEND:");//发送剩余字节数据
        strx=strstr((char*)strx,(char*)",");//获取第一个,
        strx=strstr((char*)(strx+1),(char*)",");//获取第二个,
        untildata=*(strx+1)-0x30;
        Clear_Buffer();
    }
}


///透传模式下接受数据
void EC20Send_RecAccessMode(void)
{
    strx=strstr((const char*)RxBuffer,(const char*)"Rec data");//接收到服务器下发数据
    if(strx)
    {
        printf("Rec data is ok\r\n");//数据发回去
        delay_ms(500);
        Clear_Buffer();
    }
    strx=strstr((const char*)RxBuffer,(const char*)"CLOSED");//服务器主动关闭
    if(strx)
    {
        while(1)
        {
           //看门狗复位，当然也可以加入其他的措施来处理
        }
    }
}

