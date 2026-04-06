
#include "include.h"	


#include "modbus.h"




   //--------------------------------------------------
   //uart3 使用说明
   //RS232(2), 用于232扩展
   //date:2022.4.29 
   //
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

   
  #define USART3_TIMEOUT_Setting  10    
  volatile UCHAR *pucSndBufferCur_uart3;

 //----------------------------------------------
//--------UART2 ----------------------------------
 u8 USART3_RX_BUF[32]; 
 u8 USART3_RX_CNT=0;
 u16  USART3_RX_TIMEOUT=100; 
 u8 TX3Buf[32];
 u8 TX3GP=0;
 u8 TX3SP=0; 
 u8 TX3Bufcount=0; 
   
unsigned char Uart3CmdBuf[32]; // 通信命令缓冲区
unsigned char Uart3CmdGp=0;
unsigned char Uart3CmdSp=0; 
unsigned char Uart3Cmd=0; 

uchar Uart3TxCommState=0; 
uchar Uart3RxCommState=0;  
uchar Uart3RecvCmd=0; 



u8 Flag_Req_DlyOff_XMIT_UART3=0;
u8 DlyOff_XMIT_TimOvCnt_UART3=3;

volatile eMBRcvState eRcvState_UART3;
volatile eMBSndState eSndState_UART3;

 void GetDataUart3(void)
 	
 { 
	   uchar i=0; 
//	   uchar checksum=0;
	   uchar cmd; 
	   
//	   uchar addr=0;  
	   uchar Headcode=0; 
//	   uchar lenth=0; 
	   
	 
		Headcode= USART3_RX_BUF[0];  
	
		if(Headcode==0x5A)
		{ 
			cmd=USART3_RX_BUF[1]; 	

			switch(cmd)
			{

			  case 0x01:
			  	       Uart3InputCmd(0x20);
			  	       break;
			  default:	 break;  
							 
			}	
		   
		}

		for(i=0;i<32;i++)
		{
		  USART3_RX_BUF[i]=0;
		}
	
	}


 

 
 //------------------------------------
 // UART 3	  FOR	485     //  2024.12.29 
 // TX -- PB10 
 // RX -- PB11	 
 //------------------------------------
   void Usart3_Init(u32 bound)
   {
	   //GPIO端口设置
	   GPIO_InitTypeDef GPIO_InitStructure;
	   USART_InitTypeDef USART_InitStructure;
	   NVIC_InitTypeDef NVIC_InitStructure;
 
		
	   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//使能GPIOA时钟
	   RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);//使能USART2时钟
 
		 
	   //USART3_TX	  // PB10 
	   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PB10
	   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	  //复用推挽输出
	   GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOA2
   
	   //USART3_RX	 // PB11   初始化
	   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PB11
	   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	   GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOA.3
 
	   RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART3,ENABLE);//复位串口3
	   RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART3,DISABLE);//停止复位
 
	   
   
	   //Usart3 NVIC 配置
	   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	  //设置NVIC中断分组2:2位抢占优先级，2位响应优先级	 0-3;
	   
	   NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//抢占优先级3
	   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		 //子优先级3
	   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  //IRQ通道使能
	   NVIC_Init(&NVIC_InitStructure);	  //根据指定的参数初始化VIC寄存器
	 
	  //USART 初始化设置
   
	   USART_InitStructure.USART_BaudRate = bound;//串口波特率
	   USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	   USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	   USART_InitStructure.USART_Parity = USART_Parity_No;//无校验 
	   USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	   USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	  //收发模式
   
	   USART_Init(USART3, &USART_InitStructure); //初始化串口
	   USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启串口接受中断
	   USART_Cmd(USART3, ENABLE);					 //使能串口 
   }




//===========================
//input  :  *p  数组名称
//            length  数组长度   
//===========================
uchar getCheckSum03(uchar *p,uchar length)
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



void Uart3_Tx(void)
{  
            
  if((Flag_Req_DlyOff_XMIT_UART3==1)&&(DlyOff_XMIT_TimOvCnt_UART3==0))
  	{
  	  Flag_Req_DlyOff_XMIT_UART3=0;  
      UART3_485_RD=0;  // siwtch to rx mode  
  	}
  
}


// for uart3   
void Uart3_Rx(void)
{  

   // static u8 i=0; 
	
  // UART3  232   
  
  	    if(USART3_RX_TIMEOUT<USART3_TIMEOUT_Setting)USART3_RX_TIMEOUT++; 
        if((USART3_RX_TIMEOUT== USART3_TIMEOUT_Setting)&&(USART3_RX_CNT>0))  // get data   
        {
            USART3_RX_TIMEOUT=0; 
			GetDataUart3();  
            USART3_RX_CNT=0; 
        } 
        else
     	{
          ; 
  	    }
  
}


void
vMBPortSerialEnable_UART3( BOOL xRxEnable, BOOL xTxEnable )
{
    ENTER_CRITICAL_SECTION(  );
    if( xRxEnable )
    {
        USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    }
    else
    {
       USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);
    }
    if( xTxEnable )
    {
       USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
	   UART3_485_RD=1; 
	   
    }
    else
    {
       USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
	  
    }
    EXIT_CRITICAL_SECTION(  );
}




  /**
 63 * USART1发送len个字节.
 64 * buf:发送区首地址
 65 * len:发送的字节数(为了和本代码的接收匹配,这里建议不要超过64个字节)
 66 **/
  void USART3_Send_Data(u8 *buf,u16 len)
  {
      u16 t;
    //  GPIO_SetBits(GPIOA,GPIO_Pin_9);
   //  RS485_TX_EN=1;            //设置为发送模式
      for(t=0;t<len;t++)        //循环发送数据
      {           
          while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET); //循环发送,直到发送完毕   
          USART_SendData(USART3,buf[t]); 
      }     
      while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);     
	  

  }



BOOL
xMBPortSerialPutByte_UART3( CHAR ucByte )
{
	// while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET); //循环发送,直到发送完毕   
     USART_SendData(USART3,ucByte); 
	 return TRUE; 
}

BOOL
xMBRTUTransmitFSM_UART3( void )
{
    BOOL            xNeedPoll = FALSE;


    switch ( eSndState_UART3)
    {
        /* We should not get a transmitter event if the transmitter is in
         * idle state.  */
    case STATE_TX_IDLE:
        /* enable receiver/disable transmitter. */
        vMBPortSerialEnable_UART3( TRUE, FALSE );
        break;

    case STATE_TX_XMIT:	 
        /* check if we are finished. */
        if( TX3Bufcount != 0 )
        {
            xMBPortSerialPutByte_UART3( ( CHAR )*pucSndBufferCur_uart3);
            pucSndBufferCur_uart3++;  /* next byte in sendbuffer. */
            TX3Bufcount--;	
        }
        else
        {
            /* Disable transmitter. This prevents another transmit buffer
                  * empty interrupt. */
            
             vMBPortSerialEnable_UART3( TRUE, FALSE );

			 Flag_Req_DlyOff_XMIT_UART3=1;
             DlyOff_XMIT_TimOvCnt_UART3=2;
			 
             eSndState_UART3= STATE_TX_IDLE;
        }
        break;
    }

    return xNeedPoll;
}



  /**
 98 * 接收指定长度的字符串
 99 * 比如接收固定大小为21个字节的字符串
100 **/
 void USART3_IRQHandler(void)                    //串口2中断服务程序
 {

 
     u8 Res; 

	 
	if(USART_GetITStatus(USART3,USART_IT_TXE)) //   TransmitterEmpty 
	{
	   xMBRTUTransmitFSM_UART3();  //     for uart3   	
	}
    else if(USART_GetITStatus(USART3, USART_IT_RXNE)) 
    {    
         USART3_RX_TIMEOUT=0; 	 
         Res =USART_ReceiveData(USART3);    //读取接收到的数据    
         if(USART3_RX_CNT<32)//对于接收指定长度的字符串
         {
             USART3_RX_BUF[USART3_RX_CNT]=Res;        //记录接收到的值    
             USART3_RX_CNT++;                        //接收数据增加1 
         }  
		 
    }

 
     //溢出-如果发生溢出需要先读SR,再读DR寄存器则可清除不断入中断的问题
     if(USART_GetFlagStatus(USART3,USART_FLAG_ORE) == SET)
     {
         USART_ReceiveData(USART3);
         USART_ClearFlag(USART3,USART_FLAG_ORE);  
     }

	 
    // USART_ClearFlag(USART2,USART_IT_TXE); //一定要清除接收中断
     USART_ClearFlag(USART3,USART_IT_RXNE); //一定要清除接收中断
	 
 }


// uart3 
void TX3send(uchar dat)
{
    TX3Buf[TX3SP] = dat;
    TX3SP++; 
    TX3SP&=0x1f;   //32  
    TX3Bufcount++; 
} 


// for uart3  
// 对于RS232的传输，不需要地址了
//因为是1对1的传输,
void GetTx3buffer(unsigned char cmd)
{
	    u8 funcode; 
		u8 lenth; 
		u8 addr; 
		u8 dat1; 
		 
		switch(cmd)
		{
			
		 case 0X01:
				
				TX3SP=TX3GP=0; TX3Bufcount=0; 
				lenth=0; 
				TX3send(0x6A); //5A    帧头
				TX3send(cmd); 
				TX3send(lenth); 
				TX3send(getCheckSum(TX3Buf,3));  // checksum	
				pucSndBufferCur_uart3= ( UCHAR * )TX3Buf;	
				eSndState_UART3= STATE_TX_XMIT; 
				vMBPortSerialEnable_UART3( TRUE, TRUE ); // serial eanble ,ready to send data 
				break;	
				
          case 0x03: // LOAD  board 
			 
			      TX3SP=TX3GP=0; TX3Bufcount=0; 
			      addr=1;
		          funcode=0XE2;
				  
				  dat1=(PowerSupplyMode<<4)|NP_Mode;  //  000x 00xx;   
				   
				  
                  TX3send(addr);     // local  addr  
	 	          TX3send(funcode);  //
	 	          TX3send(dat1);
	 	          TX3send(getCheckSum(TX3Buf,3));  // checksum    

			      pucSndBufferCur_uart3= ( UCHAR * )TX3Buf; 	
			      eSndState_UART3= STATE_TX_XMIT;
                  vMBPortSerialEnable_UART3( TRUE, TRUE ); // serial eanble ,ready to send data    
		 	      break; 	


				
		  default: break;  
		  
		} 
}

    


void Uart3InputCmd(uchar cmd)
{
     Uart3CmdBuf[Uart3CmdSp]=cmd;
     Uart3CmdSp++; 
	 Uart3CmdSp&=0x1F; // 32    
} 

void Uart3_RS485(void)
{
   static int  ComDly=500; 
//   static u8  state=0; 
//   u8  Res; 
    
    
            
    if(ComDly>0)ComDly--; 
    if(ComDly==0)  // t=50ms 
    {  
 	  ComDly=500;  //  
 	  
      Uart3InputCmd(0x03);  
     
    }

    if(Uart3CmdGp!=Uart3CmdSp)
  	{
       Uart3Cmd=Uart3CmdBuf[Uart3CmdGp++]; 
	   Uart3CmdGp&=0x1f; 
	   GetTx3buffer(Uart3Cmd);    
  	}
       
}



void Uart3TxCommManage(void)
{
   ; 
}

void Uart3RxCommManage(void)
{
   ;
}

void Uart3_Manage(void)
{
  Uart3_Tx(); 
  Uart3_Rx(); 
  Uart3_RS485();
}




