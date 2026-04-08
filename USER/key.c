
#include "include.h"


 u8  Flag_Start_Cmd=0; 

 unsigned char ucKeyStep=1; 
 unsigned char ucRowRecord=1; //��¼��ǰɨ�赽�ڼ�����
 unsigned int  uiKeyTimeCnt=0; //����ȥ������ʱ������
 unsigned char ucKeyLock=0; //���������������ı�����־ 
 unsigned char ucKeySec=0;   //�������İ������ 
 unsigned int  KeyVal_FlagL=0; //  ��������־λ
 unsigned int  KeyVal_FlagM=0; //  ��������־λ
 unsigned int  KeyVal_FlagH=0; //  ��������־λ
 unsigned char QuickKeyVal=0; // ��ݼ���ֵ 

  
 unsigned char MCGS_KeyValue=0; 
  

unsigned char Keytmp=0; 

unsigned char PreSensFbState=0; 

u8 Flag_Clear_Req=0; // ��������

u8 SafeSwitch_State=0;



void  key_scan(void)
{
  u8 tmp=0; 
  if(KEY1){tmp|=0x01;}
  if(!KEY2){tmp|=0x02;}
  if(KEY3){tmp|=0x04;}



  if(tmp==0x01){ucKeySec=1;}
  else if(tmp==0x02){ucKeySec=2;}
  else if(tmp==0x04){ucKeySec=3;}
  else              {ucKeySec=0;}

 // ucKeySec=0; // �����ð�������

   

}


u8 JogKeyVal=0; 



uchar FunNb=0;  

u8  Flag_Run=0; 
u8  Motor_dir=0; 
u8  Flag_SysRun=0; 
u8  KeyCmd=0; 


 

// k1 x1  start
void k_key1(void) 
{ 
      // Flag_SysRun=1; 
     
       if(SafeSwitch_State==1) // ����״̬��������
       	{
               KeyCmd=  CMD_START;    
	      Flag_Beep_ON_Req=1;  
       	}
	else
	{
                  Flag_Alarm_Bits |=0x02;  // set the alarm flag 
	}
    
 	  
}

// k2  x2 stop 
void k_key2(void) 
{
      // Flag_SysRun=0; 
        KeyCmd=  CMD_STOP; 
	Flag_Beep_ON_Req=1;  
}


// k3  x3 clear 
void k_key3(void) 
{
     Flag_Clear_Req=1; 

}
//k4   motor - front   
void k_key4(void) 
{


   Flag_Beep_ON_Req=1;  
}
//k5    motor - back 
void k_key5(void) 
{


  Flag_Beep_ON_Req=1; 
 ; 

}
//k6   motor - home 
void k_key6(void) 
{

  Flag_Beep_ON_Req=1;  

}

//k7   sensor- zero   
void k_key7(void) 
{
 

  Flag_Beep_ON_Req=1;  

}

//k8 MaualMeasure  
void k_key8(void) 
{
  
  Flag_Beep_ON_Req=1;  

}

//k9   MeasureReady    
void k_key9(void) 
{

  Flag_Beep_ON_Req=1;  

}

//-------------------------------------------------------------------

unsigned char Key_Num=0x00;   // ���μ���
unsigned char Key_Backup=0x00; // ���ݼ���
unsigned char Key_Cjnum=0;	// ����������
unsigned char Key_Dis_F=0; // ������ֹ��Ӧ
unsigned char Key_Scan_F=0; // �������ʹ�ܣ�10MSһ��  
unsigned char Key_Last=0;  // �ϴμ�ֵ  
unsigned char Key_LastEX=0;  // �ϴμ�ֵ  



unsigned char Key_Num2=0x00;   // ���μ���
unsigned char Key_Backup2=0x00; // ���ݼ���
unsigned int  Key_Cjnum2=0;  // ����������
unsigned char Key_Dis_F2=0; // ������ֹ��Ӧ
unsigned char Key_Scan_F2=0; // �������ʹ�ܣ�10MSһ��   



// ����Ϊ��������ʱ��� 
void ShortKeyAction (void)
 {
   
  switch(Key_Backup)
	 {	 
		 case 0:   ; break;     //  
		 case 1:    k_key1(); break; 
		 case 2:    k_key2();break;
		 case 3:    k_key3();break; 
		 case 4:    k_key4();break; 
		 case 5:    k_key5();break; 
		 case 6:    k_key6();break;
		 case 7:    k_key7();break; 
		 case 8:    k_key8();break; 
		 case 9:    k_key9();break; 
    
		 default :   break; 	 
	 }
 
 }

//----------------------------------
// ����Ϊ�����ͷ�ʱ������
 
void ShortKeyActionEX (void)
 {
   
  switch(Key_LastEX)
	 {	 
		 	
          case 0:  ;  break;     //  
		  case 1:  ; break; 
		  default :   break; 	 
	 }
 }


void ReadKey(void)
{

  Key_Num= ucKeySec;  

  if((Key_Num!=0)&&(Key_Num==Key_Backup))  // ����а����������ϴ���ͬ(����û���ɿ�)
  {

            if(!Key_Dis_F)  // ���Ұ���û�д���  
		 	{
                if(Key_Cjnum<AN_XD_NUM)
                {
                   Key_Cjnum++; // �����������Լ�
                }
                else // �������趨����ʱ���ж�Ϊ������������
                {
                  
		         ShortKeyAction(); // �̻�ɢת���� 

				// Flag_Beep_ON_Req=1;
				  
		         Key_LastEX=Key_Backup;   
			     Key_Cjnum=0;   // ����������   
			     Key_Dis_F=1;  //  ����������־Ϊ�������  
                }	               
		 	}	
		 
		  Key_Backup=Key_Num;   // ��¼�¼���   
 
  }
  else
 	{
 	   
        Key_Backup=Key_Num;  //   ��¼�¼��� 
		Key_Dis_F=0;   //  ��־���Խ����µİ�������  
		Key_Cjnum=0;   // ����������   

		if((Key_Num==0)&&(Key_LastEX!=0)) // �ͷŰ���
		{  
            ShortKeyActionEX(); 
			Key_LastEX=0; // clr   	
		}

 	}
   
}


u8  keyval=0; 
u8  keytmp=0; 


 void KeyscanNormal(void)
 { 
	  unsigned char tmp;
	 
	  tmp =!X5;  // 2024.4.10
	  if ((keytmp&1) == tmp) 
	  {
		  if ((keytmp&0xf0) == 0xf0) 
		  {    
			keyval = keytmp;
		  }
		  else
		  {  
			keytmp = keytmp + 0x10;
		  }
	  }
	  else
	  {   
		  keytmp = tmp;
	  }



	  if(keyval==0XF1)
	  {
                     SafeSwitch_State=1;    // ����״̬
	  }
	 else
	 {
                     SafeSwitch_State=0;   // ����״̬
	 }


	  
 }




 uchar Flag_Beep_ON_Req=0; 
 u8    BeepEnable=0; 
 u8    BeepCode=0;  
 uint  oBeepCnt=0;	
 
 void BeepDriver(void)
 {

	if(BeepCode==1)
	 { 
	         BeepCode=0;
		 oBeepCnt=10;
		 
		 Flag_Beep_ON_Req=0;
	 }
        
        if(oBeepCnt>0)oBeepCnt--; 
	 
	if(oBeepCnt>0)Bell=YON;
	else		  Bell=YOFF;	 
	  
 }






