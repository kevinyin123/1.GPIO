#include "include.h"


 
u8 Flag_SysSampState=OFF;   //THE whole  SYSTEM SAMPLE SATE  
u8 NP_Mode=0;   // 0: npn ,  1: pnp  
u8 SysVolSel=0; //0 :  12v    1:  24v    
u8 NP_ModeTmp=0;   // 0: npn ,  1: pnp  
u8 SysVolSelTmp=0; //0 :  12v    1:  24v    
u16 TempSampleNum_MASKTmp=0;  // 

u16 Sys_Voltage=0; // 工作电压  
u16 Sys_TotalCurrent=0;// 工作总电流
SHORT Sys_CurTempreture=0;
SHORT EnvTempreture=0;  // Environment 环境


// 实时监控的值
u16 CurChQuiet_I=0; // 当前通道静态电流
u16 CurChLoad_I=0; // 当前通道负载电流
u16 CurChVdrop=0; // 当前通道电压降


//测距过程中测量出来的值
u16 CurChMeasQuiet_I=0; // 当前通道静态电流正在测量的值
u16 CurChMeasLoad_I=0; // 当前通道负载电流正在测量的值
u16 CurChMeasVdrop=0; // 当前通道电压降正在测量的值



u8 CurMesRow=1;
u8 CurMesCh=0;

u8 Power_Sw=1; 
u8 Mode_Vol=0;
U8 Mode_Np=0;
U8 Mode_NoNc=0; 
u8 Mode_Ch=0; 
SHORT  SpRef=800; // 传感器参考距离

u8 CH1_Np=0;  // 0: npn, 1:pnp  
u8 CH2_Np=0;  // 0: npn, 1:pnp  
u8 PowerSupplyMode=0;  // 0: internal      1: external 
u8 TestPlateType=0;   // 测试盘类型， 0，1，2，3
u16 MotorSpeed=0; 
u16 SysRun_Freq=0;
u16 CH1_Freq=0;
u16 CH2_Freq=0;
u32 CH1_Cnt=0;
u32 CH2_Cnt=0;
u32 CH1_Pass_Cnt=0;
u32 CH2_Pass_Cnt=0;

u16 PwmDuty=0; 
u16 ControlValue=0; 
u16 OutPutDuty=0; 
u16 Min_Duty=0; 




u8 Ele_Load_En=0;
u8 Flag_Ch_Change=0;

u16 Ele_Load_Off_TimeOvCnt=0;//电子负载关闭超时




unsigned char Pingbang=0;
unsigned char WakeNb=0; 
u8  Timovcnt_GetDataFromEEprom=10; 

int  EncodePos=0; 
int  Monitor_timer3=0; 

u16 TestTimeDly=10;

void TimeBase(void)
{ 
   static uint count_ms=0;
   
   if(DlyOff485TX_TimOvCnt>0)DlyOff485TX_TimOvCnt--; 
   if(DlyOff_XMIT_TimOvCnt_UART2>0)DlyOff_XMIT_TimOvCnt_UART2--;  
   if(DlyOff_XMIT_TimOvCnt_UART4>0)DlyOff_XMIT_TimOvCnt_UART4--;  
 
   if(Ele_Load_Off_TimeOvCnt>0)Ele_Load_Off_TimeOvCnt--;
   
   
    
   count_ms++;
   if(count_ms>999) // 1s   
   	{ 
   	   count_ms=0; 
       if(Timovcnt_GetDataFromEEprom>0)Timovcnt_GetDataFromEEprom--; 
	   if(TestTimeDly>0)TestTimeDly--; 
	   
         

	  // LED1=!LED1;  
   	}
   
}



void DelayMS(unsigned int n) 
{
    int i,j;  
    for(i = n;i>0;i--)
        for(j=1000;j>0;j--) ; 
}


void ChannelSim(void)
{
 ;
  
}


