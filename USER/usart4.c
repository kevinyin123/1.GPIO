
#include "include.h"	  

#include "modbus.h"



 //--------------------------------------------------
//uart4 使用说明
//跟压缩机温控器用，通信速度，9600， PXF4
// MODBUS 协议
//date:  2022.5.31
// UART 4	  FOR 485
// TX ---PC10 
// RX ---PC11 
// RD ---PA15	
//--------------------------------------------------

 #include "stm32f10x_usart.h"
 #include "stm32f10x.h"
 #include "stm32f10x_iwdg.h"

   
  #define USART4_TIMEOUT_Setting  10    
  volatile UCHAR *pucSndBufferCur_uart4;

//
 SHORT SV=-150;  // -15.0 du ,  一个小数点
 SHORT PV=0;  //
 SHORT EV =0; 
 
 SHORT XV=-100;  //
 
 
 U8 PreCmd=0; 


 //----------------------------------------------
//--------UART4 ----------------------------------
 u8 USART4_RX_BUF[32]; 
 u8 USART4_RX_CNT=0;
 u16  USART4_RX_TIMEOUT=100; 
 u8 TX4Buf[32];
 u8 TX4GP=0;
 u8 TX4SP=0; 
 u8 TX4Bufcount=0; 
   
unsigned char Uart4CmdBuf[32]; // 通信命令缓冲区
unsigned char Uart4CmdGp=0;
unsigned char Uart4CmdSp=0; 
unsigned char Uart4Cmd=0; 

uchar Uart4TxCommState=0; 
uchar Uart4RxCommState=0;  
uchar Uart4RecvCmd=0; 



// DELAY TO TURN OFF THE TX FUNCITON , 
//FOR UART2  
u8 Flag_Req_DlyOff_XMIT_UART4=0;
u8 DlyOff_XMIT_TimOvCnt_UART4=2;


volatile eMBRcvState eRcvState_UART4;
volatile eMBSndState eSndState_UART4;


 //------------------------------------
 // UART 4	   FOR 485
 // TX ---PC10 
 // RX ---PC11 
 // RD ---PA15	
 //------------------------------------
   void Usart4_Init(u32 bound)
   {
	   //GPIO端口设置
	   GPIO_InitTypeDef GPIO_InitStructure;
	   USART_InitTypeDef USART_InitStructure;
	   NVIC_InitTypeDef NVIC_InitStructure;
 
		
	   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);//使能GPIOA时钟
	   RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);//使能USART4时钟
 
		 
	   //USART4_TX	  // PC10
	   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //
	   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	  //复用推挽输出
	   GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIOA2
   
	   //USART4_RX	// PC11  
	   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//
	   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//浮空输入
	   GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIOA.3
 
	   RCC_APB1PeriphResetCmd(RCC_APB1Periph_UART4,ENABLE);//复位串口2
	   RCC_APB1PeriphResetCmd(RCC_APB1Periph_UART4,DISABLE);//停止复位
  
   
	   //Usart4 NVIC 配置
	   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);	  //设置NVIC中断分组2:2位抢占优先级，2位响应优先级	 0-3;
	   
	   NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//抢占优先级3
	   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		 //子优先级3
	   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  //IRQ通道使能
	   NVIC_Init(&NVIC_InitStructure);	  //根据指定的参数初始化VIC寄存器
	 
	  //USART 初始化设置
   
	   USART_InitStructure.USART_BaudRate = bound;//串口波特率
	   USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式//9
	   USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	   USART_InitStructure.USART_Parity = USART_Parity_No;// 无需校验
	   USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	   USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	  //收发模式
   
	   USART_Init(UART4, &USART_InitStructure); //初始化串口
	   USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);//开启串口接受中断
	   USART_Cmd(UART4, ENABLE);					//使能串口 
   }
 



 void GetDataUART4(void); 
  

//===========================
//input  :  *p  数组名称
//            length  数组长度   
//===========================
uchar getCheckSum04(uchar *p,uchar length)
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






// for uart4  485 
void Uart4_Rx(void)
{  

   // static u8 i=0; 
	
  	    if(USART4_RX_TIMEOUT<USART4_TIMEOUT_Setting)USART4_RX_TIMEOUT++; 
		
        if((USART4_RX_TIMEOUT== USART4_TIMEOUT_Setting)&&(USART4_RX_CNT>0))  // get data   
        {
            USART4_RX_TIMEOUT=0; 
			GetDataUart4();  
            USART4_RX_CNT=0; 
        } 
        else
     	{
           ; 
  	    }
  
}


void
vMBPortSerialEnable_UART4( BOOL xRxEnable, BOOL xTxEnable )
{
    ENTER_CRITICAL_SECTION(  );
    if( xRxEnable )
    {
        USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
    }
    else
    {
       USART_ITConfig(UART4, USART_IT_RXNE, DISABLE);
    }
    if( xTxEnable )
    {
       USART_ITConfig(UART4, USART_IT_TXE, ENABLE);
	   UART4_485_RD=1; 
    }
    else
    {
       USART_ITConfig(UART4, USART_IT_TXE, DISABLE);
	  
    }
    EXIT_CRITICAL_SECTION(  );
}




  /**
 63 * USART1发送len个字节.
 64 * buf:发送区首地址
 65 * len:发送的字节数(为了和本代码的接收匹配,这里建议不要超过64个字节)
 66 **/
  void USART4_Send_Data(u8 *buf,u16 len)
  {
      u16 t;
    //  GPIO_SetBits(GPIOA,GPIO_Pin_9);
   //  RS485_TX_EN=1;            //设置为发送模式
      for(t=0;t<len;t++)        //循环发送数据
      {           
          while(USART_GetFlagStatus(UART4,USART_FLAG_TC)==RESET); //循环发送,直到发送完毕   
          USART_SendData(UART4,buf[t]); 
      }     
      while(USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET);     
	  

  }



void Uart4_Tx(void)
{  
            
  if((Flag_Req_DlyOff_XMIT_UART4==1)&&(DlyOff_XMIT_TimOvCnt_UART4==0))
  	{
  	  Flag_Req_DlyOff_XMIT_UART4=0;  
      UART4_485_RD=0; // siwtch to rx mode  
  	}
  
}


BOOL
xMBPortSerialPutByte_UART4( CHAR ucByte )
{
	// while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET); //循环发送,直到发送完毕   
     USART_SendData(UART4,ucByte); 
	 return TRUE; 
}

BOOL
xMBRTUTransmitFSM_UART4( void )
{
    BOOL            xNeedPoll = FALSE;


    switch ( eSndState_UART4)
    {
        /* We should not get a transmitter event if the transmitter is in
         * idle state.  */
    case STATE_TX_IDLE:
        /* enable receiver/disable transmitter. */
        vMBPortSerialEnable_UART4( TRUE, FALSE );
        break;

    case STATE_TX_XMIT:	 
        /* check if we are finished. */
        if( TX4Bufcount != 0 )
        {
            xMBPortSerialPutByte_UART4( ( CHAR )*pucSndBufferCur_uart4);
            pucSndBufferCur_uart4++;  /* next byte in sendbuffer. */
            TX4Bufcount--;	
        }
        else
        {
            /* Disable transmitter. This prevents another transmit buffer
                  * empty interrupt. */
            
             vMBPortSerialEnable_UART4( TRUE, FALSE );

			 Flag_Req_DlyOff_XMIT_UART4=1;
             DlyOff_XMIT_TimOvCnt_UART4=2;
			
            eSndState_UART4= STATE_TX_IDLE;
        }
        break;
    }

    return xNeedPoll;
}



  /**
 98 * 接收指定长度的字符串
 99 * 比如接收固定大小为21个字节的字符串
100 **/
 void UART4_IRQHandler(void)                    //串口4中断服务程序
 {

 
     u8 Res; 
	 
	if(USART_GetITStatus(UART4,USART_IT_TXE)) //   TransmitterEmpty 
	{
	   xMBRTUTransmitFSM_UART4();  //     for uart4  	
	}
    else if(USART_GetITStatus(UART4, USART_IT_RXNE)) 
    {    
         USART4_RX_TIMEOUT=0; 
		 
         Res =USART_ReceiveData(UART4);    //读取接收到的数据    
         if(USART4_RX_CNT<32)//对于接收指定长度的字符串
         {
             USART4_RX_BUF[USART4_RX_CNT]=Res;        //记录接收到的值    
             USART4_RX_CNT++;                        //接收数据增加1 
         }  
		 
    }


     //溢出-如果发生溢出需要先读SR,再读DR寄存器则可清除不断入中断的问题
     if(USART_GetFlagStatus(UART4,USART_FLAG_ORE) == SET)
     {
         USART_ReceiveData(UART4);
         USART_ClearFlag(UART4,USART_FLAG_ORE);  
     }

	 
    // USART_ClearFlag(USART2,USART_IT_TXE); //一定要清除接收中断
     USART_ClearFlag(UART4,USART_IT_RXNE); //一定要清除接收中断
	 
 }


// uart4  
void TX4send(uchar dat)
{
    TX4Buf[TX4SP] = dat;
    TX4SP++; 
    TX4SP&=0x1f;   //32  
    TX4Bufcount++; 
} 


// for uart4
// for pxf4  tempreture controller 
// 命令和功能码是分开的， 功能码对应modbus的通信
// 0x01: 读取温度
// 0x02: 写入温度
// 0x03: 开温控器
// 0X04: 关温控器


void GetTx4buffer(unsigned char cmd)
	{
		
		//	u8 lenth; 
			u8 funcode; 
			u8 addr; 
			u16 ReadStartAddress=0; 
		//	u16 WriteStartAddress=0;
			//u16 WriteData=0; 
			u16 bytes; 
			u8 dat1; 
			
			USHORT    usCRC16;
			
				  
			switch(cmd)
			{
				
			 case 0X01:   // read PV 
					
					TX4SP=TX4GP=0; TX4Bufcount=0; 
					addr=0x01;  
					funcode=0x04; 
					ReadStartAddress=2000;  // PV地址
					bytes=2;
					
					TX4send(addr);  // 站号No. 
					TX4send(funcode); 
					TX4send((ReadStartAddress>>8&0xff)); // hi8bits
					TX4send((ReadStartAddress&0xff));// lo8bits 
					TX4send((bytes>>8&0xff)); // hi8bits
					TX4send((bytes&0xff));// lo8bits 
					usCRC16 = usMBCRC16((UCHAR*)TX4Buf,TX4Bufcount);
                    TX4send ((UCHAR)(usCRC16 & 0xFF)); // CRC是先传低位，后传高位
                    TX4send((UCHAR)(usCRC16 >> 8));
					pucSndBufferCur_uart4= (UCHAR *)TX4Buf;	
					eSndState_UART4= STATE_TX_XMIT; 
					vMBPortSerialEnable_UART4( TRUE, TRUE); // serial eanble ,ready to send data 
					break;	 
					
			 case 0X02:   // 
					TX4SP=TX4GP=0; TX4Bufcount=0; 
					addr=0x01;  
					break;	
					
			 case 0x03: // power board 
			 
			      TX4SP=TX4GP=0; TX4Bufcount=0; 
			      addr=1;
		          funcode=0XE2;
				  
				  dat1=(PowerSupplyMode<<4)|NP_Mode;  //  000x 00xx;   
				   
				  
                  TX4send(addr);     // local  addr  
	 	          TX4send(funcode);  //
	 	          TX4send(dat1);
	 	          TX4send(getCheckSum(TX4Buf,3));  // checksum    

			      pucSndBufferCur_uart4= ( UCHAR * )TX4Buf; 	
			      eSndState_UART4= STATE_TX_XMIT;
                  vMBPortSerialEnable_UART4( TRUE, TRUE ); // serial eanble ,ready to send data    
		 	      break; 		
					
					
			  default: break; 
			 
			  
			} 

			PreCmd = cmd; 
			
			
	}

    

// 接收到从机发出的数据，
// 特别注意，CRC是先传低字节后传高字节
void GetDataUart4(void)
	{ 
		   uchar i=0; 
		  // uchar checksum=0;
		 //  uchar cmd; 
		   
		   uchar addr=0;  
		   uchar funcode=0; 
		   uchar readbytes=0; 
		   USHORT    usCRC16;
		   USHORT tmp=0; 
		   uchar lenth=0; 

		 
			addr= USART4_RX_BUF[0];
			funcode=USART4_RX_BUF[1];
			
			if(addr==1) // 从在地址为1 
			{
			   
              switch(funcode)
              	{
                   case 0x03:  
				   	           readbytes =USART4_RX_BUF[2]; 
							   lenth = readbytes+3; 
                               usCRC16 = usMBCRC16((UCHAR*)USART4_RX_BUF,lenth);
							   tmp= ((USART4_RX_BUF[lenth+1]<<8)|(USART4_RX_BUF[lenth])); 
							   if(tmp==usCRC16) // 判断校验是否成功
							   	{
							   	   // 判断上次的请求命令是什么就接收什么
							   	   //因为接收的时候是不会返回地址的,所以得根据请求的命令来接收所要的数据 
								  if(PreCmd==0x02)            
							   	  {
                                      SV=(USART4_RX_BUF[3]<<8)|USART4_RX_BUF[4];  // 读取SV1 
							   	  } 
								  
							   	} 	 
				   	           break;  
				   case 0x04:
							   readbytes =USART4_RX_BUF[2]; 
							   lenth = readbytes+3; 
							   usCRC16 = usMBCRC16((UCHAR*)USART4_RX_BUF,lenth);
							    tmp= ((USART4_RX_BUF[lenth+1]<<8)|(USART4_RX_BUF[lenth])); 
							   if(tmp==usCRC16) // 判断校验是否成功
							   {
							      if(PreCmd==0x01)//              
							   	  {
                                      PV=(USART4_RX_BUF[3]<<8)|USART4_RX_BUF[4];   // 读取PV 
							   	  }  
							   }
							   
				   	           break; 
				   case 0x06:  
				   	             
				   	           
				   	           break; 
              	}
			}


			for(i=0;i<32;i++)
			{
			  USART4_RX_BUF[i]=0;
			}

			PreCmd=0; // 清除命令
		
		}


void Uart4InputCmd(uchar cmd)
{
     Uart4CmdBuf[Uart4CmdSp]=cmd;
     Uart4CmdSp++; 
	 Uart4CmdSp&=0x1F; // 32    
} 


void Uart4_RS485(void)
	{
	   static u16  ComDly=0; 
	   static u8   frame_gap_dly=200;
	   static u8   state=0; 
	   
	

	  if(ComDly>0)ComDly--; 
	  if(ComDly==0)  // t=50ms 
	  {  
		    ComDly=100;  //  1s

		    state++;
	        state&=0x03; 

			Uart4InputCmd(0x03);// 
		 
	  }


      //最少要间隔100ms才能发送一帧数据
	  if(frame_gap_dly>0){frame_gap_dly--;}
	  else
	  {
	    frame_gap_dly=50;
	    if(Uart4CmdGp!=Uart4CmdSp)
	    {
		   Uart4Cmd=Uart4CmdBuf[Uart4CmdGp++]; 
		   Uart4CmdGp&=0x1f; 
		   GetTx4buffer(Uart4Cmd);	  
	    }
	  }
		   
	}



void Uart4TxCommManage(void)
{
   ; 
}

void Uart4RxCommManage(void)
{
   ;
}

void Uart4_Manage(void)
{
  Uart4_Tx();
  Uart4_Rx(); 
  Uart4_RS485();
  
}




