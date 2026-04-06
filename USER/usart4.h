#ifndef __USART4_H
#define __USART4_H


#define USART4_REC_LEN  			200  	//定义最大接收字节数 200
#define EN_USART4_RX 			1		//使能（1）/禁止（0）串口1接收


#define MAX_LIMT_TEMPRETURE     1600 // 160du 
#define MIN_LIMT_TEMPRETURE     -400 // -40du 


extern u8 USART4_RX_CNT; 
extern  u16  USART4_RX_TIMEOUT; 

extern u8 Flag_Req_DlyOff_XMIT_UART4;
extern u8 DlyOff_XMIT_TimOvCnt_UART4;

extern  SHORT SV;  // -15.0 du ,  一个小数点
extern  SHORT PV;  //
extern  SHORT EV; 


extern void Usart4_Init(u32 bound);
extern void GetDataUart4(void); 
extern void Uart4_RS485(void); 
extern void Uart4InputCmd(uchar cmd); 
extern void Uart4_Manage(void); 



#endif


