
#include "include.h"	  

#include "modbus.h"


//--------------------------------------------------
//uart1	使用说明
//485(1), 用于连接传感器测量的各个子板
//1-5 meter board 
//bridge board
//power board
// ele-load board 
// if_board row1,row2,row3 
//bandrate: 19200  
//date: 
//--------------------------------------------------



   /*===============================================================================
  2 Copyright:
  3 Version:
  4 Author:    
  5 Date: 2017/11/3
  6 Description:
  7     配置独立看门狗初始化函数，在主函数中运行IWDG_ReloadCounter进行喂狗主函数必须在4s内进行一次喂狗不然系统会复位；
  8     函数功能是将接收固定长度的字符串，并将接收后的字符串通过串口发送出去
  9 revise Description:
 10 ===============================================================================*/
  #include "stm32f10x_usart.h"
  #include "stm32f10x.h"
  #include "stm32f10x_iwdg.h"


  #define USART1_TIMEOUT_Setting  5     
  #define RS485_DEVICE_LOCAL_ADDR   101 

 


// 1-5   meter      
// 10  bridge board  
// 11  power board 
// 12  ele_load board 
// 13  tempreture board 
// 20  if_board row1  
// 21  if_baord row2
// 22  if_baord row3 
// 23  coding_switch bd 
  _CONST_ U8  SlaverScanTab[13]={1,2,3,4,5,10,11,12,13,20,21,22,23};

 u8   SlaverIndex=0;  


 //------UART1-----------------------------
  u8 USART1_RX_BUF[32]; 
  u8 USART1_RX_CNT=0;
  u16  USART1_RX_TIMEOUT=100; 
  u8 SlaveAddr=0; 
  u8 TXBuf[16];
  u8 TXGP=0;
  u8 TXSP=0; 
  u8 TXBufcount=0; 
  

volatile UCHAR *pucSndBufferCur_uart1;

unsigned char UartCmdBuf[32]; // 通信命令缓冲区
unsigned char UartCmdGp=0;
unsigned char UartCmdSp=0; 

//-------------------------------

unsigned char DisMeasCmd=0; // 测距板的通信命令
u8 DataReturnCmd=0;  // 数据返回命令 

//---------------------------------



uint   U16_METER01_V_Val=0; 
uint   U16_METER02_V_Val=0;
uint   U16_METER03_V_Val=0;
uint   U16_METER04_V_Val=0;
uint   U16_METER05_V_Val=0;

uint   U16_METER01_I_Val=0; 
uint   U16_METER02_I_Val=0;
uint   U16_METER03_I_Val=0;
uint   U16_METER04_I_Val=0;
uint   U16_METER05_I_Val=0;




//-----------------------
  s16  MonitorDat1=0;
  u16  MonitorDat2=0; 
  u16  MonitorDat3=0;
  u16  MonitorDat4=0;
  u16  MonitorDat5=0X55;  
  u16  MonitorDat6=0X55;
  u16  MonitorDat7=0X55;
  u16  MonitorDat8=0X55; 

  

 u8 CurAccessAddr=0x01; // 当前访问地址
 u8 CommunicationType=1; 


 // delay to turn off the TX FUNCTION, WAIT FOR THE LAST BYTE TO TRANSMIT  
 // FOR UART1 
 u8 Flag_Req_DlyOff_485TX=0;
 u8 DlyOff485TX_TimOvCnt=2;

volatile eMBSndState eSndState_UART1;
volatile eMBRcvState eRcvState_UART1;

 void IWDG_Configuration(void); 

 //------------------------------------
 // UART 1	 FOR 485 
 // PA9-----  TX 
 // PA10----  RX 
 // PA11---   RD 
 //------------------------------------
   void Usart1_Init(u32 bound)
   {
	   //GPIO端口设置
	   GPIO_InitTypeDef GPIO_InitStructure;
	   USART_InitTypeDef USART_InitStructure;
	   NVIC_InitTypeDef NVIC_InitStructure;
		
	   RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC, ENABLE);//使能USART1,GPIOA,C时钟
		 
	   //USART1_TX	 GPIOA.9
	   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
	   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	  //复用推挽输出
	   GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9
   
	   //USART1_RX		GPIOA.10初始化
	   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
	   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//上拉输入
	   GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10  
   
	   //Usart1 NVIC 配置
	   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);	  //设置NVIC中断分组2:2位抢占优先级，2位响应优先级	 0-3;
	   
	   NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//抢占优先级3
	   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		 //子优先级3
	   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  //IRQ通道使能
	   NVIC_Init(&NVIC_InitStructure);	  //根据指定的参数初始化VIC寄存器
	 
	  //USART 初始化设置
   
	   USART_InitStructure.USART_BaudRate = bound;//串口波特率
	   USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	   USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	   USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	   USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	   USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	  //收发模式
   
	   USART_Init(USART1, &USART_InitStructure); //初始化串口1
	   USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启串口接受中断
	   USART_Cmd(USART1, ENABLE);					 //使能串口1 
   }



//===========================
//input  :  *p  数组名称
//            length  数组长度   
//===========================
uchar getCheckSum(uchar *p,uchar length)
{
uint u16checkSum;
uchar i;

  u16checkSum=*p;
  p++;
  for(i=0;i<(length-1);i++)
  {
	    u16checkSum+=*p;
        p++;
  }
	  i=(~((u16checkSum>>8)+(uchar)u16checkSum));
  return i;

}



//------------------------------------------------------------------------
// 使能OR除能串口接收发送中断
// 需更具CPU进行改写 
//--------------------------------------------------------------------------
void
vMBPortSerialEnable_UART1( BOOL xRxEnable, BOOL xTxEnable )
{
    ENTER_CRITICAL_SECTION(  );
    if( xRxEnable )
    {
        USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    }
    else
    {
       USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
    }
    if( xTxEnable )
    {
       USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
	   UART1_485_RD=1; 
    }
    else
    {
       USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
	   //UART1_485_RD=0; 
    }
    EXIT_CRITICAL_SECTION(  );
}









  /**
 63 * USART1发送len个字节.
 64 * buf:发送区首地址
 65 * len:发送的字节数(为了和本代码的接收匹配,这里建议不要超过64个字节)
 66 **/
  void USART1_Send_Data(u8 *buf,u16 len)
  {
      u16 t;
   //   GPIO_SetBits(GPIOC,GPIO_Pin_9);
  //  RS485_TX_EN=1;            //设置为发送模式
    UART1_485_RD=1; 
      for(t=0;t<len;t++)        //循环发送数据
      {           
          while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET); //循环发送,直到发送完毕   
          USART_SendData(USART1,buf[t]); 
      }     
      while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);        
      GPIO_ResetBits(GPIOC,GPIO_Pin_9);
  //    RS485_TX_EN=0;                //设置为接收模式    
       UART1_485_RD=0; 
  }


 // for uart1 
 
void Uart1_Tx(void)
{  

  if((Flag_Req_DlyOff_485TX==1)&&(DlyOff485TX_TimOvCnt==0))
  	{
  	  Flag_Req_DlyOff_485TX=0;  
      UART1_485_RD=0; // siwtch to rx mode  
  	}
}

BOOL
xMBPortSerialPutByte_UART1( CHAR ucByte )
{
	// while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET); //循环发送,直到发送完毕   
     USART_SendData(USART1,ucByte); 
	 return TRUE; 
}



BOOL
xMBRTUTransmitFSM_UART1( void )
{
    BOOL            xNeedPoll = FALSE;


    switch ( eSndState_UART1)
    {
        /* We should not get a transmitter event if the transmitter is in
         * idle state.  */
    case STATE_TX_IDLE:
        /* enable receiver/disable transmitter. */
        vMBPortSerialEnable_UART1( TRUE, FALSE );
        break;

    case STATE_TX_XMIT:	 
        /* check if we are finished. */
        if( TXBufcount != 0 )
        {
            xMBPortSerialPutByte_UART1( ( CHAR )*pucSndBufferCur_uart1);
            pucSndBufferCur_uart1++;  /* next byte in sendbuffer. */
            TXBufcount--;	
        }
        else
        {
            /* Disable transmitter. This prevents another transmit buffer
                  * empty interrupt. */
            
             vMBPortSerialEnable_UART1( TRUE, FALSE );
			Flag_Req_DlyOff_485TX=1;
            DlyOff485TX_TimOvCnt=2;
			
            eSndState_UART1= STATE_TX_IDLE;
        }
        break;
    }

    return xNeedPoll;
}



  /**
 98 * 接收指定长度的字符串
 99 * 比如接收固定大小为21个字节的字符串
100 **/
 void USART1_IRQHandler(void)                    //串口1中断服务程序
 {

 
     u8 Res;

    
    if(USART_GetITStatus(USART1,USART_IT_TXE)) //   TransmitterEmpty 
	{ 
	    xMBRTUTransmitFSM_UART1();  // modbus 状态机    for uart1  
	}
	else  if(USART_GetITStatus(USART1, USART_IT_RXNE)) 
     {
             USART1_RX_TIMEOUT=0; 
			 
             Res =USART_ReceiveData(USART1);    //读取接收到的数据     
             if(USART1_RX_CNT<32)//对于接收指定长度的字符串
             {
                 USART1_RX_BUF[USART1_RX_CNT]=Res;        //记录接收到的值    
                 USART1_RX_CNT++;                                        //接收数据增加1 
             }             
      }
	 
          //溢出-如果发生溢出需要先读SR,再读DR寄存器则可清除不断入中断的问题
     if(USART_GetFlagStatus(USART1,USART_FLAG_ORE) == SET)
     {
         USART_ReceiveData(USART1);
         USART_ClearFlag(USART1,USART_FLAG_ORE);
     }
      USART_ClearFlag(USART1,USART_IT_RXNE); //一定要清除接收中断
 }




// uart1  
void TXsend(uchar dat)
{
    TXBuf[TXSP] = dat;
    TXSP++; 
    TXSP&=0x0f;   //16  
    TXBufcount++; 
} 


// for uart1  
void GetTxbuffer(unsigned char addr)
{

 u8 dat1;
 u8 dat2; 
 
 
 u8  funcode;

  TXSP=TXGP=0; TXBufcount=0; 
	
  switch(addr)
  	{
         //1-5  meter  ,公用一个接口 
	     case 1:;      
		 case 2:;
		 case 3:;
		 case 4:;  
		 case 5:  
		 	      funcode=0XE1; 
                  TXsend(addr);   // local  addr  
	 	          TXsend(funcode);  //
	 	          TXsend(getCheckSum(TXBuf,2));  // checksum    
			      pucSndBufferCur_uart1= ( UCHAR * )TXBuf; 	
			      eSndState_UART1= STATE_TX_XMIT;
                  vMBPortSerialEnable_UART1( FALSE, TRUE ); // serial eanble ,ready to send data    
		 	      break; 
				  
         case 10: // bridge board  
		 	      funcode=0XE2; 
			
                  TXsend(addr); // local  addr  
	 	          TXsend(funcode);  //
	 	          TXsend(Mode_Np); //桥接板目前只需要NPN,PNP的信息
	 	          TXsend(getCheckSum(TXBuf,3));  // checksum    
			      pucSndBufferCur_uart1= ( UCHAR * )TXBuf; 	
			      eSndState_UART1= STATE_TX_XMIT;
                  vMBPortSerialEnable_UART1( FALSE, TRUE ); // serial eanble ,ready to send data    
		 	      break; 
		 case 11: // power board 
		          funcode=0XE2;
				  dat1=(Power_Sw<<4)|Mode_Vol; 
				  
                  TXsend(addr); // local  addr  
	 	          TXsend(funcode);  //
	 	          TXsend(dat1);
	 	          TXsend(getCheckSum(TXBuf,3));  // checksum    
			      pucSndBufferCur_uart1= ( UCHAR * )TXBuf; 	
			      eSndState_UART1= STATE_TX_XMIT;
                  vMBPortSerialEnable_UART1( FALSE, TRUE ); // serial eanble ,ready to send data    
		 	      break; 
		 case 12: //ele-load board  
		          funcode=0XE2; //dat1=0x55; 
		          dat1=Ele_Load_En; 
                           TXsend(addr); // local  addr  
	 	          TXsend(funcode);  //
	 	          TXsend(dat1);
	 	          TXsend(getCheckSum(TXBuf,3));  // checksum    
			      pucSndBufferCur_uart1= ( UCHAR * )TXBuf; 	
			      eSndState_UART1= STATE_TX_XMIT;
                  vMBPortSerialEnable_UART1( FALSE, TRUE ); // serial eanble ,ready to send data    
		 	      break;
		case 13: //tempreture board  
		          funcode=0XE1; // 读取温度
		          dat1=0x55; 
                  TXsend(addr); // local  addr  
	 	          TXsend(funcode);  //
	 	          TXsend(dat1);
	 	          TXsend(getCheckSum(TXBuf,3));  // checksum    
			      pucSndBufferCur_uart1= ( UCHAR * )TXBuf; 	
			      eSndState_UART1= STATE_TX_XMIT;
                  vMBPortSerialEnable_UART1( FALSE, TRUE ); // serial eanble ,ready to send data    
		 	      break;  

		 case 20: //IF-BOARD  ROW1
		          funcode=0XE2; 
				  if(CurMesRow==1) // 如果轮到该组，才发送数据，否则发送0
				  {
				    dat1=CurMesCh;
				  }
				  else
				  { 
				    dat1=0;
				  } 
				  
                  TXsend(addr); // local  addr  
	 	          TXsend(funcode);  //
	 	          TXsend(dat1);
	 	          TXsend(getCheckSum(TXBuf,3));  // checksum    
			      pucSndBufferCur_uart1= ( UCHAR * )TXBuf; 	
			      eSndState_UART1= STATE_TX_XMIT;
                  vMBPortSerialEnable_UART1( FALSE, TRUE ); // serial eanble ,ready to send data    
		 	      break; 
		 case 21: //IF-BOARD ROW2 
		          funcode=0XE2; 
				   if(CurMesRow==2) // 如果轮到该组，才发送数据，否则发送0
				  {
				    dat1=CurMesCh;
				  }
				  else
				  { 
				    dat1=0;
				  } 
                  TXsend(addr); // local  addr  
	 	          TXsend(funcode);  //
	 	          TXsend(dat1);
	 	          TXsend(getCheckSum(TXBuf,3));  // checksum    
			      pucSndBufferCur_uart1= ( UCHAR * )TXBuf; 	
			      eSndState_UART1= STATE_TX_XMIT;
                  vMBPortSerialEnable_UART1( FALSE, TRUE ); // serial eanble ,ready to send data    
		 	      break; 
		 case 22: //IF-BOARD ROW3 
		          funcode=0XE2; 
				  if(CurMesRow==3) // 如果轮到该组，才发送数据，否则发送0
				  {
				    dat1=CurMesCh;
				  }
				  else
				  { 
				    dat1=0;
				  } 
                  TXsend(addr); // local  addr  
	 	          TXsend(funcode);  //
	 	          TXsend(dat1);
	 	          TXsend(getCheckSum(TXBuf,3));  // checksum    
			      pucSndBufferCur_uart1= ( UCHAR * )TXBuf; 	
			      eSndState_UART1= STATE_TX_XMIT;
                  vMBPortSerialEnable_UART1( FALSE, TRUE ); // serial eanble ,ready to send data      
		 	      break; 
	   	 case 23: //coding_switch bd 
                              
		          funcode=0XE4; 
				  dat1=0x55;dat2=0xaa; 
				  
                  TXsend(addr); // local  addr  
	 	          TXsend(funcode);  //
	 	          TXsend(dat1);
				  TXsend(dat2);
	 	          TXsend(getCheckSum(TXBuf,4));  // checksum    
			      pucSndBufferCur_uart1= ( UCHAR * )TXBuf; 	
			      eSndState_UART1= STATE_TX_XMIT;
                  vMBPortSerialEnable_UART1( FALSE, TRUE ); // serial eanble ,ready to send data      
		 	      break;  
			
	  default: break;  
	  
  	} 
  }




void GetData(void)
	{ 
	 
	   uchar i=0; 
	   uchar checksum=0;
	   uchar addr=0; 
	   u16 tmp=0; 
	   
	   u16	 hi,lo;
	
		
		addr= USART1_RX_BUF[0];
	
	
		switch(addr)
			{

		     case 23:  // 23#,   coding_switch  board  
			      checksum= getCheckSum(USART1_RX_BUF,6);   
	              if(checksum== USART1_RX_BUF[6])
	               {
	                   //KeyBbInfBytes = USART1_RX_BUF[2];   //null  
		               hi=  USART1_RX_BUF[3]; 
                                lo=  USART1_RX_BUF[4]; 
			     tmp= (hi<<8)+lo; 
			     ControlValue=tmp;  // kuo da 10 bei .
        
	                }
		  	     break;	
			default:  
					 break;  
					 
					 
			}
	
	
	for(i=0;i<20;i++){ USART1_RX_BUF[i]=0;}    
	
	}




// for uart1   
void Uart1_Rx(void)
{  



  // UART1  485    
  
  if(USART1_RX_TIMEOUT<USART1_TIMEOUT_Setting)USART1_RX_TIMEOUT++; 

  if((USART1_RX_TIMEOUT== USART1_TIMEOUT_Setting)&&(USART1_RX_CNT>0)) // get data  
  {
     USART1_RX_TIMEOUT=0; 
	 GetData();   
     USART1_RX_CNT=0; 
  } 
  
}



void Uart1_RS485(void)
{
    static u8  ComDly=0; 
 

   
    ComDly++;
	if(ComDly>100) // 20ms 扫描一个 地址
	{
	  ComDly=0; 
	 // CurAccessAddr = SlaverScanTab[SlaverIndex]; 
	  CurAccessAddr=23;  //codingswitch board 
	  GetTxbuffer(CurAccessAddr); 

	  //SlaverIndex++;
	  //if(SlaverIndex>11)SlaverIndex=0;  

	}

  
}


void InputCmd(uchar cmd)
{
     UartCmdBuf[UartCmdSp]=cmd;
     UartCmdSp++; 
	 UartCmdSp&=0x1F; // 32    
} 


void Uart1_Manage(void)
{
   Uart1_Rx(); 
   Uart1_Tx();

   Uart1_RS485();  // for inter board  
}



