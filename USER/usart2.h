#ifndef __USART2_H
#define __USART2_H


#define USART2_REC_LEN  			200  	//定义最大接收字节数 200
#define EN_USART2_RX 			1		//使能（1）/禁止（0）串口1接收


extern  u16  USART2_RX_TIMEOUT; 
extern  u8 DlyOff_XMIT_TimOvCnt_UART2; 


extern BOOL xMBPortSerialPutByte( CHAR ucByte );




extern void Usart2_Init(u32 bound);
extern void GetDataUart2(void); 
extern void Uart2_RS485(void); 
extern void Uart2InputCmd(uchar cmd); 
extern void Uart2_Manage(void); 

#endif


