#ifndef _HMI_LCD_H 
#define _HMI_LCD_H 


extern uint  DcMeterV2_Value;
extern uint  DcMeterA2_Value;


extern void Display(void); 
extern void SendDataTo4094(void); 
extern void SMEC_Test(void); 
extern void Ele_load_Process(void);


extern u8  Flag_Alarm_Bits;

 extern void Alarm_Manage(void); 
 

  
#endif   

 
