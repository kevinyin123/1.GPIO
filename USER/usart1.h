#ifndef __USART_H
#define __USART_H




#define USART_REC_LEN  			200  	//定义最大接收字节数 200
#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1接收
	  	
  extern u8 USART1_RX_BUF[32]; 
  extern u8 USART1_RX_CNT; 


extern u8 CommunicationType;  // 通信类型
extern u8 Flag_Req_DlyOff_485TX;
extern u8 DlyOff485TX_TimOvCnt;
extern unsigned char DisMeasCmd; // 测距板的通信命令

extern u8 DataReturnCmd;  // 数据返回命令 

extern void Usart1_Init(u32 bound); 
extern uchar getCheckSum(uchar *p,uchar length); 
extern void  SendChar(uchar t); 
extern void Uart1_Rx(void); 
extern void Uart1_Tx(void);

extern void Uart(void); 


extern void Uart1_Manage(void);


extern uint   U16_METER01_V_Val; 
extern uint   U16_METER02_V_Val;
extern uint   U16_METER03_V_Val;
extern uint   U16_METER04_V_Val;
extern uint   U16_METER05_V_Val;

extern uint   U16_METER01_I_Val; 
extern uint   U16_METER02_I_Val;
extern uint   U16_METER03_I_Val;
extern uint   U16_METER04_I_Val;
extern uint   U16_METER05_I_Val;



extern   s16  MonitorDat1;
extern   u16  MonitorDat2;
extern   u16  MonitorDat3;
extern   u16  MonitorDat4;  
extern   u16  MonitorDat5;
extern   u16  MonitorDat6;
extern   u16  MonitorDat7;
extern   u16  MonitorDat8;

#endif


