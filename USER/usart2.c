
#include "include.h"	  

#include "modbus.h"



//--------------------------------------------------
//uart2  使用说明
//连接电脑，跟上位机通信 
//date:2022.4.29 
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


#define ENQ   0X05   // 询问,确认
#define EOT   0X04   //已经传输完毕  end of transfer 
#define ACK   0X06   // 确认 
#define NAK   0X15   // 非确认
 

  #define USART2_TIMEOUT_Setting  10    
  volatile UCHAR *pucSndBufferCur_uart2;

 //----------------------------------------------
//--------UART2 ----------------------------------
 u8 USART2_RX_BUF[32]; 
 u8 USART2_RX_CNT=0;
 
 u16  USART2_RX_TIMEOUT=100; 
 u8 TX2Buf[32];
 u8 TX2GP=0;
 u8 TX2SP=0; 
 u8 TX2Bufcount=0; 
unsigned char Uart2CmdBuf[32]; // 通信命令缓冲区
unsigned char Uart2CmdGp=0;
unsigned char Uart2CmdSp=0; 
unsigned char Uart2Cmd=0; 


 
 uchar Uart2TxCommState=0; 
 uchar Uart2RxCommState=0;	
 uchar Uart2RecvCmd=0; 


 // DELAY TO TURN OFF THE TX FUNCITON , 
 //FOR UART2  
 u8 Flag_Req_DlyOff_XMIT_UART2=0;
 u8 DlyOff_XMIT_TimOvCnt_UART2=2;

volatile eMBSndState eSndState_UART2;
volatile eMBRcvState eRcvState_UART2;

 
 //------------------------------------
 // UART 2	   FOR RS232
 // TX --PA2 
 // RX --PA3 
 //------------------------------------
   void Usart2_Init(u32 bound)
   {
	   //GPIO端口设置
	   GPIO_InitTypeDef GPIO_InitStructure;
	   USART_InitTypeDef USART_InitStructure;
	   NVIC_InitTypeDef NVIC_InitStructure;
 
		
	   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//使能GPIOA时钟
	   RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//使能USART2时钟
 
		 
	   //USART2_TX	 GPIOA.2  // PA2 
	   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
	   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	  //复用推挽输出
	   GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA2
   
	   //USART2_RX	 PA3	GPIOA.3初始化
	   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA3
	   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	   GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.3
 
	   RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART2,ENABLE);//复位串口2
	   RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART2,DISABLE);//停止复位
 
	   
   
	   //Usart2 NVIC 配置
	   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);	  //设置NVIC中断分组2:2位抢占优先级，2位响应优先级	 0-3;
	   
	   NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//抢占优先级3
	   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		 //子优先级3
	   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  //IRQ通道使能
	   NVIC_Init(&NVIC_InitStructure);	  //根据指定的参数初始化VIC寄存器
	 
	  //USART 初始化设置
   
	   USART_InitStructure.USART_BaudRate = bound;//串口波特率
	   USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	   USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	   USART_InitStructure.USART_Parity = USART_Parity_No;//无校验 
	   USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	   USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	  //收发模式
   
	   USART_Init(USART2, &USART_InitStructure); //初始化串口1
	   USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启串口接受中断
	   USART_Cmd(USART2, ENABLE);					 //使能串口1 
   }
 

 void GetDataUART2(void); 
  

//===========================
//input  :  *p  数组名称
//            length  数组长度   
//===========================
uchar getCheckSum_02(uchar *p,uchar length)
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



// for uart2   
void Uart2_Rx_old(void)   
{  

  // UART2  232   
  
  	    if(USART2_RX_TIMEOUT<USART2_TIMEOUT_Setting)USART2_RX_TIMEOUT++; 
        if((USART2_RX_TIMEOUT== USART2_TIMEOUT_Setting)&&(USART2_RX_CNT>0))  // get data   
        {
            USART2_RX_TIMEOUT=0; 
			GetDataUart2();  
            USART2_RX_CNT=0; 
        } 
        else
     	{
          ; 
  	    }
  
}

void Uart2_Rx(void)   
{  

     // UART2  232   
  
	
     if(RTUTimerT35ExpiredEn==1)
  	{
  	    if(USART2_RX_TIMEOUT<USART2_TIMEOUT_Setting)USART2_RX_TIMEOUT++; 
        if((USART2_RX_TIMEOUT== USART2_TIMEOUT_Setting))   
        {
         USART2_RX_TIMEOUT=0; 
		 (void)xMBRTUTimerT35Expired();    //  OVER T35,THEN DEAL THE RECIEVE DATA   
         USART2_RX_CNT=0; 
        } 
        else
     	{
          ; 
  	    }
  	}
  
}



void vMBPortSerialEnable_UART2( BOOL xRxEnable, BOOL xTxEnable )
{
    ENTER_CRITICAL_SECTION(  );
    if( xRxEnable )
    {
        USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    }
    else
    {
       USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
    }
    if( xTxEnable )
    {
       USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
	    
    }
    else
    {
       USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
	  
    }
    EXIT_CRITICAL_SECTION(  );
}




  /**
 63 * USART1发送len个字节.
 64 * buf:发送区首地址
 65 * len:发送的字节数(为了和本代码的接收匹配,这里建议不要超过64个字节)
 66 **/
  void USART2_Send_Data(u8 *buf,u16 len)
  {
      u16 t;
    //  GPIO_SetBits(GPIOA,GPIO_Pin_9);
   //  RS485_TX_EN=1;            //设置为发送模式
      for(t=0;t<len;t++)        //循环发送数据
      {           
          while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET); //循环发送,直到发送完毕   
          USART_SendData(USART2,buf[t]); 
      }     
      while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);     
	  
  //     GPIO_ResetBits(GPIOC,GPIO_Pin_9);
  //    RS485_TX_EN=0;                //设置为接收模式    
  }



BOOL
xMBPortSerialPutByte( CHAR ucByte )
{
	// while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET); //循环发送,直到发送完毕   
     USART_SendData(USART2,ucByte); 
	 return TRUE; 
}




BOOL xMBPortSerialPutByte_UART2( CHAR ucByte )
{
	// while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET); //循环发送,直到发送完毕   
     USART_SendData(USART2,ucByte); 
	 return TRUE; 
}

BOOL
xMBRTUTransmitFSM_UART2( void )
{
    BOOL            xNeedPoll = FALSE;


    switch ( eSndState_UART2)
    {
        /* We should not get a transmitter event if the transmitter is in
         * idle state.  */
    case STATE_TX_IDLE:
        /* enable receiver/disable transmitter. */
        vMBPortSerialEnable_UART2( TRUE, FALSE );
        break;

    case STATE_TX_XMIT:	 
        /* check if we are finished. */
        if( TX2Bufcount != 0 )
        {
            xMBPortSerialPutByte_UART2( ( CHAR )*pucSndBufferCur_uart2);
            pucSndBufferCur_uart2++;  /* next byte in sendbuffer. */
            TX2Bufcount--;	
        }
        else
        {
            /* Disable transmitter. This prevents another transmit buffer
                  * empty interrupt. */
            
             vMBPortSerialEnable_UART2( TRUE, FALSE );
             eSndState_UART2= STATE_TX_IDLE;
        }
        break;
    }

    return xNeedPoll;
}



  /**
 98 * 接收指定长度的字符串
 99 * 比如接收固定大小为21个字节的字符串
100 **/
 void USART2_IRQHandler_old(void)                    //串口2中断服务程序
 {

 
     u8 Res; 
 
	if(USART_GetITStatus(USART2,USART_IT_TXE)) //   TransmitterEmpty 
	{
	   xMBRTUTransmitFSM_UART2();  //  for uart2   	
	}
    else if(USART_GetITStatus(USART2, USART_IT_RXNE)) 
    {    
         USART2_RX_TIMEOUT=0; 	 
         Res =USART_ReceiveData(USART2);    //读取接收到的数据    
         if(USART2_RX_CNT<32)//对于接收指定长度的字符串
         {
             USART2_RX_BUF[USART2_RX_CNT]=Res;        //记录接收到的值    
             USART2_RX_CNT++;                        //接收数据增加1 
         }  
		 
    }

 
     //溢出-如果发生溢出需要先读SR,再读DR寄存器则可清除不断入中断的问题
     if(USART_GetFlagStatus(USART2,USART_FLAG_ORE) == SET)
     {
         USART_ReceiveData(USART2);
         USART_ClearFlag(USART2,USART_FLAG_ORE);  
     }

	 
    // USART_ClearFlag(USART2,USART_IT_TXE); //一定要清除接收中断
     USART_ClearFlag(USART2,USART_IT_RXNE); //一定要清除接收中断
	 
 }



//for modbus 
void USART2_IRQHandler(void)					//串口2中断服务程序
{


	u8 Res; 

   if(USART_GetITStatus(USART2,USART_IT_TXE)) //   TransmitterEmpty 
   {
	   //xMBRTUTransmitFSM_UART2();  //  for uart2    
	   (void) xMBRTUTransmitFSM();  // modbus 状态机   
   }
   else if(USART_GetITStatus(USART2, USART_IT_RXNE)) 
   {	
		USART2_RX_TIMEOUT=0;	
		Res =USART_ReceiveData(USART2);    //读取接收到的数据	 
		xMBRTUReceiveFSM(Res);// 用状态机接收 
		
   }


	//溢出-如果发生溢出需要先读SR,再读DR寄存器则可清除不断入中断的问题
	if(USART_GetFlagStatus(USART2,USART_FLAG_ORE) == SET)
	{
		USART_ReceiveData(USART2);
		USART_ClearFlag(USART2,USART_FLAG_ORE);  
	}

	
   // USART_ClearFlag(USART2,USART_IT_TXE); //一定要清除接收中断
	USART_ClearFlag(USART2,USART_IT_RXNE); //一定要清除接收中断
	
}






// uart2  
void TX2send(uchar dat)
{
    TX2Buf[TX2SP] = dat;
    TX2SP++; 
    TX2SP&=0x1f;   //32  
    TX2Bufcount++; 
} 


// for uart2  
// 对于RS232的传输，不需要地址了
//因为是1对1的传输,
void GetTx2buffer(unsigned char cmd)
{

	u8 lenth; 
	int tmp1,tmp2,tmp3,tmp4,tmp5; 
	u8 u8tmp1,u8tmp2;
	u16 u16tmp1;
	
	USHORT    usCRC16;
	
	
	
	
	
	 
     switch(cmd)
  	{
     	
          case 0x01:  //获取系统信息
	 	       TX2SP=TX2GP=0; TX2Bufcount=0; 

			tmp1= Sys_CurTempreture;  //
                        //  tmp1= SV; //临时测试
                          	
			tmp2= SV; // 
			tmp3= EnvTempreture;//Sys_CurTempreture
			tmp4= Sys_Voltage; 
			tmp5= Sys_TotalCurrent;

			u8tmp1=0; 
		
			
				
		        lenth=11; 
		        TX2send(0x5A); //5A    帧头
	 	        TX2send(cmd); 
			TX2send(lenth);
			
			TX2send((tmp1>>8)&0XFF); // HI 8BITS	
			TX2send(tmp1&0xFF); //	LOW 8BITS	
			
                           TX2send((tmp2>>8)&0XFF); // HI 8BITS	
			TX2send(tmp2&0xFF); //	LOW 8BITS	

			TX2send((tmp3>>8)&0XFF); // HI 8BITS	
			TX2send(tmp3&0xFF); //	LOW 8BITS	

			TX2send((tmp4>>8)&0XFF); // HI 8BITS	
			TX2send(tmp4&0xFF); //	LOW 8BITS	

			TX2send((tmp5>>8)&0XFF); // HI 8BITS	
			TX2send(tmp5&0xFF); //	LOW 8BITS	

			TX2send(u8tmp1); //  1byte
			
                          TX2send(getCheckSum(TX2Buf,TX2Bufcount));  // checksum    
                          pucSndBufferCur_uart2= ( UCHAR * )TX2Buf; 	
		        eSndState_UART2= STATE_TX_XMIT;
                          vMBPortSerialEnable_UART2( TRUE, TRUE ); // serial eanble ,ready to send data 
                          break; 
			 
     	case 0x02:  //获取系统坐标
	 	        TX2SP=TX2GP=0; TX2Bufcount=0; 

			lenth=6; 
		    TX2send(0x01);// addr 
	 	    TX2send(cmd); 
			TX2send(lenth);
			TX2send((tmp1>>8)&0XFF); // HI 8BITS	
			TX2send(tmp1&0xFF); //	LOW 8BITS	
			TX2send((tmp2>>8)&0XFF); // HI 8BITS	
			TX2send(tmp2&0xFF); //	LOW 8BITS	
                         	TX2send((u16tmp1>>8)&0XFF); // HI 8BITS	
			TX2send(u16tmp1&0xFF); //	LOW 8BITS	
			
                         TX2send(getCheckSum(TX2Buf,TX2Bufcount));  // checksum    
                         pucSndBufferCur_uart2= ( UCHAR * )TX2Buf; 	
			eSndState_UART2= STATE_TX_XMIT;
                        vMBPortSerialEnable_UART2( TRUE, TRUE ); // serial eanble ,ready to send data 
                       break; 
			
	 case 0x04:  
	 	         TX2SP=TX2GP=0; TX2Bufcount=0; 

				// MonitorDat2=99; 
				 
			    lenth=2; 
				  TX2send(0x01);// addr 
	 	          TX2send(0x04); 
			      TX2send(0x02);
				  TX2send((MonitorDat2>>8)&0xff);  // hi8bits   
				  TX2send(MonitorDat2&0xff);  // lo8bits 
				  lenth = TX2Bufcount; 
				  usCRC16 = usMBCRC16((UCHAR*)TX2Buf,lenth);
					
				  TX2send(usCRC16&0xff); // crc  low  
				  TX2send((usCRC16>>8)&0xff); // crc  hi 
		          pucSndBufferCur_uart2= ( UCHAR * )TX2Buf; 	
			      eSndState_UART2= STATE_TX_XMIT;
                  vMBPortSerialEnable_UART2( TRUE, TRUE ); // serial eanble ,ready to send data 
                 break; 
			
	case 0x20: //
	 	    TX2SP=TX2GP=0; TX2Bufcount=0; 
			
	         
	
			lenth=4; 
		        TX2send(0x5A); //5A    帧头
	 	        TX2send(cmd); 
			TX2send(lenth);
			TX2send(tmp1&0xFF); //	LOW 8BITS	
			TX2send((tmp1>>8)&0XFF); // HI 8BITS	
			TX2send(tmp2&0xFF); //   LOW 8BITS   
			TX2send((tmp2>>8)&0XFF); // HI 8BITS	

            TX2send(getCheckSum(TX2Buf,TX2Bufcount));  // checksum    
            pucSndBufferCur_uart2= ( UCHAR * )TX2Buf; 	
			eSndState_UART2= STATE_TX_XMIT;
            vMBPortSerialEnable_UART2( TRUE, TRUE ); // serial eanble ,ready to send data 
	 	    break; 

   
	 case  0x30: // 系统参数
	 	    
		    TX2SP=TX2GP=0; TX2Bufcount=0; 
            tmp1= Sys_Voltage; 
			tmp2= Sys_TotalCurrent;
	
			
            lenth=6; 
		    TX2send(0x5A); //5A    帧头
	 	    TX2send(cmd); 
			TX2send(lenth);
			TX2send(tmp1&0xFF); //	LOW 8BITS	
			TX2send((tmp1>>8)&0XFF); // HI 8BITS	
			TX2send(tmp2&0xFF); //   LOW 8BITS   
			TX2send((tmp2>>8)&0XFF); // HI 8BITS		
			TX2send(tmp3&0xFF); //	 LOW 8BITS	 
			TX2send((tmp3>>8)&0XFF); // HI 8BITS		
            TX2send(getCheckSum(TX2Buf,9));  // checksum    
            pucSndBufferCur_uart2= ( UCHAR * )TX2Buf; 	
			eSndState_UART2= STATE_TX_XMIT;
            vMBPortSerialEnable_UART2( TRUE, TRUE ); // serial eanble ,ready to send data 
	 	    break; 
			
		 case  0x31:
	 	    
		    TX2SP=TX2GP=0; TX2Bufcount=0; 
            tmp1= CurChQuiet_I;  //静态电流
			tmp2= CurChLoad_I;//负载电流 
			tmp3= CurChVdrop; // 电压降
			
            lenth=6; 
		    TX2send(0x5A); //5A    帧头
	 	    TX2send(cmd); 
			TX2send(lenth);
			TX2send(tmp1&0xFF); //	LOW 8BITS	
			TX2send((tmp1>>8)&0XFF); // HI 8BITS	
			TX2send(tmp2&0xFF); //   LOW 8BITS   
			TX2send((tmp2>>8)&0XFF); // HI 8BITS		
			TX2send(tmp3&0xFF); //	 LOW 8BITS	 
			TX2send((tmp3>>8)&0XFF); // HI 8BITS		
            TX2send(getCheckSum(TX2Buf,9));  // checksum    
            pucSndBufferCur_uart2= ( UCHAR * )TX2Buf; 	
			eSndState_UART2= STATE_TX_XMIT;
            vMBPortSerialEnable_UART2( TRUE, TRUE ); // serial eanble ,ready to send data 
	 	    break; 	 
	  case  0x32:
	 	    
		    TX2SP=TX2GP=0; TX2Bufcount=0; 
            tmp1= Sys_CurTempreture;  //
			tmp2= EnvTempreture;//Sys_CurTempreture
			tmp3= SV; // 
			
            lenth=6; 
		    TX2send(0x5A); //5A    帧头
	 	    TX2send(cmd); 
			TX2send(lenth);
			TX2send(tmp1&0xFF); //	LOW 8BITS	
			TX2send((tmp1>>8)&0XFF); // HI 8BITS	
			TX2send(tmp2&0xFF); //   LOW 8BITS   
			TX2send((tmp2>>8)&0XFF); // HI 8BITS		
			TX2send(tmp3&0xFF); //	 LOW 8BITS	 
			TX2send((tmp3>>8)&0XFF); // HI 8BITS		
            TX2send(getCheckSum(TX2Buf,9));  // checksum    
            pucSndBufferCur_uart2= ( UCHAR * )TX2Buf; 	
			eSndState_UART2= STATE_TX_XMIT;
            vMBPortSerialEnable_UART2( TRUE, TRUE ); // serial eanble ,ready to send data 
	 	    break; 	 
	 case  0x40: // 测试负数的发送
	 	    
		    TX2SP=TX2GP=0; TX2Bufcount=0; 
            tmp1= -193; 
			
			TX2send((tmp1>>8)&0XFF); // HI 8BITS	 
		    TX2send(tmp1&0xFF); //	LOW 8BITS	

			
            pucSndBufferCur_uart2= ( UCHAR * )TX2Buf; 	
			eSndState_UART2= STATE_TX_XMIT;
            vMBPortSerialEnable_UART2( TRUE, TRUE ); // serial eanble ,ready to send data 
	 	    break; 	 



				   
     case 0Xaa:
            TX2SP=TX2GP=0; TX2Bufcount=0; 
	 	    TX2send(0x5A); //5A  
            TX2send(MonitorDat1&0xFF); //   LOW 8BITS   
	 	    TX2send((MonitorDat1>>8)&0XFF); // HI 8BITS   
            TX2send(MonitorDat2&0xFF); //   LOW 8BITS   
	 	    TX2send((MonitorDat2>>8)&0XFF); // HI 8BITS   
            TX2send(MonitorDat3&0xFF); //   LOW 8BITS   
	 	    TX2send((MonitorDat3>>8)&0XFF); // HI 8BITS   
            TX2send(MonitorDat4&0xFF); //   LOW 8BITS   
	 	    TX2send((MonitorDat4>>8)&0XFF); // HI 8BITS  
	 	    TX2send(MonitorDat5&0xFF); //   LOW 8BITS   
	 	    TX2send((MonitorDat5>>8)&0XFF); // HI 8BITS   
	 	    TX2send(MonitorDat6&0xFF); //   LOW 8BITS   
	 	    TX2send((MonitorDat6>>8)&0XFF); // HI 8BITS   
	 	    TX2send(MonitorDat7&0xFF); //   LOW 8BITS   
	 	    TX2send((MonitorDat7>>8)&0XFF); // HI 8BITS   
	 	    TX2send(MonitorDat8&0xFF); //   LOW 8BITS   
	 	    TX2send((MonitorDat8>>8)&0XFF); // HI 8BITS   
			
            TX2send(getCheckSum(TX2Buf,TX2Bufcount));  // checksum    
            
            pucSndBufferCur_uart2= ( UCHAR * )TX2Buf; 	
			eSndState_UART2= STATE_TX_XMIT;
            vMBPortSerialEnable_UART2( FALSE, TRUE ); // serial eanble ,ready to send data 
            
	 	    break;
      
				
	  default: break;  
	  
  	} 
 }
    

void GetDataUart2(void)
{ 
	   uchar i=0; 
	   uchar checksum=0;
	   uchar cmd; 
	   uchar addr=1; 
	   uchar funcode=0; 
	   

	//   static uchar  senddatTemp=1; 
	   
//	   uchar lenth=0; 
	   

	
	 
		addr= USART2_RX_BUF[0];  
	
		if(addr==0x01)
		{ 
			funcode=USART2_RX_BUF[1]; 
		
		   
			switch(funcode)
			{

               case 0X02:
                          // MonitorDat2+=1;
						break;	 
			   case 0X03:
                           //  MonitorDat2+=1;
						break;
			   case 0X04:
                        //MonitorDat2+=1; 
						//Uart2InputCmd(0x04); 
						 GetTx2buffer(0x04);    
						break;	
			  case 0X0f:
                           // MonitorDat2+=1; 
						break;	
			   case 0X10:
					     checksum= getCheckSum(USART2_RX_BUF,3);	
						if(checksum== USART2_RX_BUF[3])
						{
						   //MonitorDat2+=1;
						  // MonitorDat2&=0x1ff;
						   Uart2InputCmd(0x02);  
						} 
						break;	

			  default:	 break;  
							 
			}	
		   
		}

		for(i=0;i<21;i++)
		{
		   USART2_RX_BUF[i]=0;
		   ;
		}
	
	}


// for uart2 
// 
void  SendChar(uchar t)
{
    USART_SendData(USART2,t);
    while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    while((USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET));//等待串口发送完毕
}

void Uart2InputCmd(uchar cmd)
{
     Uart2CmdBuf[Uart2CmdSp]=cmd;
     Uart2CmdSp++; 
	 Uart2CmdSp&=0x1F; // 32    
} 

 
void Uart2_RS232(void)
{
   static u16  ComDly=0; 
   static u8  state=0; 
//   static u8  senddly=0;

  
            
  if(ComDly>0)ComDly--; 
  if(ComDly==0)  // t=500ms 
  {  

 	  ComDly=100;   //  
	  state++;
	  state&=0x03; 

	  MonitorDat2++; 
	  
     // xMBPortSerialPutByte_UART2(0x55);
       
          if(Uart2CmdGp!=Uart2CmdSp)
  	      {
            Uart2Cmd=Uart2CmdBuf[Uart2CmdGp++]; 
	        Uart2CmdGp&=0x1f; 
	        //GetTx2buffer(Uart2Cmd);   
  	      }   
  	}
	
}


void Uart2_Manage(void)
{
  Uart2_Rx();
  Uart2_RS232();  //串口驱动
}

