#ifndef _PARAM_H 
#define _PARAM_H 

extern unsigned char Pingbang;
extern unsigned char WakeNb; 


extern u8 Flag_SysSampState; 
extern u8 NP_Mode; 
extern u8 SysVolSel; //0 :  12v    1:  24v    
extern u8 NP_ModeTmp;   // 0: npn ,  1: pnp  
extern u8 SysVolSelTmp; //0 :  12v    1:  24v    
extern u16 TempSampleNum_MASKTmp; 


extern  u16 Sys_Voltage; // 工作电压	
extern	u16 Sys_TotalCurrent;// 工作总电流
extern  SHORT Sys_CurTempreture;
extern  SHORT EnvTempreture;  // Environment 环境

 
extern 	u16 CurChQuiet_I; // 当前通道静态电流
extern 	u16 CurChLoad_I; // 当前通道负载电流
extern 	u16 CurChVdrop; // 当前通道电压降


extern	u16 CurChMeasQuiet_I; // 当前通道静态电流  
extern	u16 CurChMeasLoad_I; // 当前通道负载电流
extern	u16 CurChMeasVdrop; // 当前通道电压降
	
extern  u8 CurMesRow;
extern  u8 CurMesCh;

extern u8 Power_Sw; 
extern u8 Mode_Vol;
extern U8 Mode_Np;
extern U8 Mode_NoNc; 
extern u8 Mode_Ch; 

extern 	u8 CH1_Np;  // 0: npn, 1:pnp	
extern	u8 CH2_Np;  // 0: npn, 1:pnp	
extern	u8 PowerSupplyMode;  // 0: internal	   1: external 
extern  u8 TestPlateType; 
extern  u16 MotorSpeed; 
extern u16 SysRun_Freq;
extern u16 CH1_Freq;
extern u16 CH2_Freq;
extern u32 CH1_Cnt;
extern u32 CH2_Cnt;
extern u32 CH1_Pass_Cnt;
extern u32 CH2_Pass_Cnt;

extern u16 PwmDuty; 
extern u16 ControlValue;
extern u16 OutPutDuty; 
extern u16 Min_Duty;





extern SHORT  SpRef; // 传感器参考距离

extern u8  Ele_Load_En;
extern u8 Flag_Ch_Change;

extern u16 Ele_Load_Off_TimeOvCnt;//电子负载关闭超时
	
extern u8  Timovcnt_GetDataFromEEprom; 

extern int  EncodePos; 
extern int  Monitor_timer3; 

extern u16 TestTimeDly;



extern void TimeBase(void); 
extern void DelayMS(unsigned int n); 
extern void ChannelSim(void); 




#endif  
