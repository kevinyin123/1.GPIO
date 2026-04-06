#ifndef __USART3_H
#define __USART3_H




#define USART3_REC_LEN  			200  	//定义最大接收字节数 200
#define EN_USART3_RX 			1		//使能（1）/禁止（0）串口1接收

extern u8 USART3_RX_CNT; 

extern  u8 Flag_Req_DlyOff_XMIT_UART3;
extern  u8 DlyOff_XMIT_TimOvCnt_UART3;


extern  u16  USART3_RX_TIMEOUT; 

 
extern void Usart3_Init(u32 bound);

extern void GetDataUart3(void); 

extern void Uart3_RS485(void); 

extern void Uart3InputCmd(uchar cmd); 
extern void Uart3_Manage(void); 



#endif


