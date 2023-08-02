#include "usmart.h"
#include "usmart_str.h"
////////////////////////////�û�������///////////////////////////////////////////////
//������Ҫ�������õ��ĺ�����������ͷ�ļ�(�û��Լ����) 
#include "delay.h"	
#include "ov5640.h"   
#include "dcmi.h"    
#include "sccb.h"   
											  
//�������б��ʼ��(�û��Լ����)
//�û�ֱ������������Ҫִ�еĺ�����������Ҵ�
struct _m_usmart_nametab usmart_nametab[]=
{
#if USMART_USE_WRFUNS==1 	//���ʹ���˶�д����
	(void*)read_addr,"u32 read_addr(u32 addr)",
	(void*)write_addr,"void write_addr(u32 addr,u32 val)",	 
#endif		   
	(void*)OV5640_Brightness,"void OV5640_Brightness(u8 bright)",
 	(void*)OV5640_Contrast,"void OV5640_Contrast(u8 contrast)",
 	(void*)OV5640_Color_Saturation,"void OV5640_Color_Saturation(u8 sat)",
 	(void*)OV5640_Light_Mode,"void OV5640_Light_Mode(u8 mode)",
 	(void*)OV5640_Special_Effects,"void OV5640_Special_Effects(u8 eft)",

 	(void*)OV5640_Sharpness,"void OV5640_Sharpness(u8 sharp)",
 	(void*)OV5640_Test_Pattern,"void OV5640_Test_Pattern(u8 mode)", 
	(void*)OV5640_Flash_Ctrl,"void OV5640_Flash_Ctrl(u8 sw)",
		
  (void*)OV5640_WR_Reg,"u8 OV5640_WR_Reg(u16 reg,u8 data)",
  (void*)OV5640_RD_Reg,"u8 OV5640_RD_Reg(u16 reg)",
	(void*)DCMI_CR_Set,"void DCMI_CR_Set(u8 pclk,u8 hsync,u8 vsync)",
  (void*)DCMI_Set_Window,"void DCMI_Set_Window(u16 sx,u16 sy,u16 width,u16 height)", 
 
	(void*)OV5640_Focus_Init,"u8 OV5640_Focus_Init(void)",
	(void*)OV5640_Focus_Single,"u8 OV5640_Focus_Single(void)", 
 	(void*)OV5640_Focus_Constant,"u8 OV5640_Focus_Constant(void)",
		
  (void*)OV5640_OutSize_Set,"u8 OV5640_OutSize_Set(u16 offx,u16 offy,u16 width,u16 height)",
 	(void*)OV5640_ImageWin_Set,"u8 OV5640_ImageWin_Set(u16 offx,u16 offy,u16 width,u16 height)",
		
	
};						  
///////////////////////////////////END///////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//�������ƹ�������ʼ��
//�õ������ܿغ���������
//�õ�����������
struct _m_usmart_dev usmart_dev=
{
	usmart_nametab,
	usmart_init,
	usmart_cmd_rec,
	usmart_exe,
	usmart_scan,
	sizeof(usmart_nametab)/sizeof(struct _m_usmart_nametab),//��������
	0,	  	//��������
	0,	 	//����ID
	1,		//������ʾ����,0,10����;1,16����
	0,		//��������.bitx:,0,����;1,�ַ���	    
	0,	  	//ÿ�������ĳ����ݴ��,��ҪMAX_PARM��0��ʼ��
	0,		//�����Ĳ���,��ҪPARM_LEN��0��ʼ��
};   



















