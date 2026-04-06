#ifndef _KEY_H
#define _KEY_H



#define  AN_XD_NUM   5   // 4  
#define  AN_CA_NUM  1000  // 1.5s 

#define  CMD_START    1
#define  CMD_STOP      2



extern  unsigned char ucKeySec;   //被触发的按键编号 
extern  uchar FunNb; 

extern  unsigned char MCGS_KeyValue;
extern  u8 JogKeyVal; 


 extern  u8  Flag_Start_Cmd;

extern u8 Flag_Clear_Req; // 清零请求
	
 extern u8  Flag_SysRun ;
 extern u8  KeyCmd;


 
 
 

extern void  key_scan(void); 

extern void Jog_Keyscan(void); 


extern void  ReadKey(void); 


extern u8 SafeSwitch_State;
extern u8	 keyval; 


extern  void KeyscanNormal(void);




extern uchar Flag_Beep_ON_Req;
extern  u8    BeepEnable;
extern  u8    BeepCode;
extern   uint  oBeepCnt;	
extern  void BeepDriver(void); 

 
#endif 
  
