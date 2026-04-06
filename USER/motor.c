#include "include.h"
//#include "math.h"


#define MIN_DUTY				 35



// PC9 
#define EnableHomeCapture1	EXTI->IMR |= GPIO_Pin_9  // 使能外部中断线
#define DisableHomeCapture1	EXTI->IMR &=~GPIO_Pin_9 // 除能外部中断线

// PA8 
#define EnableNPN_SCapture	EXTI->IMR |= GPIO_Pin_8  // 使能外部中断线
#define DisableNPN_SCapture	EXTI->IMR &=~GPIO_Pin_8 // 除能外部中断线

#define WORKMODE_IDLE           0 
#define WORKMODE_FREERUN  1
#define WORKMODE_PAUSE       2


#define  ADD_DLYTIME                20 
#define  DEC_DLYTIME                 20  


int Puls_cnt=0;

int Puls_cnt_CH1=0;
int Puls_cnt_CH2=0;
int Puls_cnt_CH_REF=0;  // 参考基准
int Puls_cnt_CH_SPD=0;  // 测速通道


int Puls_cnt_Coder=0;
int Puls_cnt_CoderBackup=0;

int Puls_cnt_CoderBase=0;


// 丢数检测寄存器
u32  PulsLossMeasCnt_CH1=0; //通道1采集到的脉冲个数
u32  PulsLossMeasCnt_CH2=0; // 通道2采集到的脉冲个数
u32  PulsLossMeasCnt_CH1_backup=0; //通道1采集到的脉冲个数
u32  PulsLossMeasCnt_CH2_backup=0; // 通道2采集到的脉冲个数

u32  PulsLossMeasCnt_REF=0; // 参考基准通道采集到的脉冲个数
uchar  Flag_LossMeas=0; // 丢数检测状态标志


u32 CircleNum=0;

u8  Flag_Get_Dat_Req=0; 
u8   SavePoint=0; 
u16 TimeCntBuf[6];
u16 TimeCntBufBackup[6];
u16 LowSpeedMeasCnt=0; 





//调试备用程序
void TIM4_IRQHandler(void) //TIME4中断服务函数  需要设定中断优先级  即NVIC配置
{  



  if(TIM_GetITStatus(TIM4,TIM_IT_Update)!=RESET)//判断是否发生了更新(溢出)中断   
  {      
      TIM_ClearITPendingBit(TIM4,TIM_IT_Update);//清除溢出中断标志位    
  } 

   TIM_SetAutoreload(TIM4, 200); // 改变ARR, 改变频率
   TIM_SetCompare1(TIM4,200); 
   //PUL=!PUL; 

  
   
	 
}



// PA0 // FB_CH1 
void EXTI0_IRQHandler(void) 
{

  if(EXTI_GetITStatus(EXTI_Line0) != RESET)
  {
         EXTI_ClearITPendingBit(EXTI_Line0);

		 if(!FB_CH1_PIN)
		 {

		     Puls_cnt_CH1++;
		    if(Flag_LossMeas==1)
                      {
                         PulsLossMeasCnt_CH1++; 
                      } 
		}
		
		//Y0=!Y0;  
	   // DisableHomeCapture1;
  }

}

// PA1 // FB2_CH2
void EXTI1_IRQHandler(void) 
{

  if(EXTI_GetITStatus(EXTI_Line1) != RESET)
  {
                EXTI_ClearITPendingBit(EXTI_Line1);

                if(!FB_CH2_PIN)
               	{
		     Puls_cnt_CH2++;
		     if(Flag_LossMeas==1)
                      {
                          PulsLossMeasCnt_CH2++; 
                      } 
	        
               	}
		

	   // DisableHomeCapture1;
  }

}




// PA8 -X0， 基准传感器
// PA8 //BASE  SENSOR INPUT 
//void EXTI15_10_IRQHandler(void) 
void EXTI9_5_IRQHandler(void) 
{

 // PA8 , SENSOR PORT 
  if(EXTI_GetITStatus(EXTI_Line8) != RESET)
  {
		
    	EXTI_ClearITPendingBit(EXTI_Line8);
		//Y3=!Y3; 
	    //DisableNPN_SCapture;
    }
     
}


// PB15   光电编码器
// X4  


void EXTI15_10_IRQHandler(void) 
{

  // PB15 
  if(EXTI_GetITStatus(EXTI_Line15) != RESET)
  {
		
    	EXTI_ClearITPendingBit(EXTI_Line15);

       if(!Coder_PIN)
       	{
            Puls_cnt_Coder++;	
            Puls_cnt_CoderBase++;   
			
	    if(Puls_cnt_CoderBase>5)
	    {
                   Puls_cnt_CoderBase=0;  

	       if(Flag_Clear_Req==1)
	     	{
                        Flag_Clear_Req=0;
                         CircleNum=0; 
                    //-------------------------
                     PulsLossMeasCnt_CH1=0; 
		    PulsLossMeasCnt_CH2=0; 
                     PulsLossMeasCnt_CH1_backup =  0;
		   PulsLossMeasCnt_CH2_backup	= 0;
	           //---------------------------	   
		      
	     	} 
		else
		{
                     CircleNum++; 
                    //-------------------------
                     PulsLossMeasCnt_CH1_backup =  PulsLossMeasCnt_CH1;
		   PulsLossMeasCnt_CH2_backup	= PulsLossMeasCnt_CH2;
	           //---------------------------	   
		}
	     
	     
	    }
       	}
			
    }
     
}



void EnableExitInterrupt(void)
{
     EXTI->RTSR &= ~EXTI_Line8; // 关闭上升沿检测
     EXTI->FTSR &= ~EXTI_Line8; // 关闭下降沿检测
     EnableNPN_SCapture; 
     EXTI->RTSR |= EXTI_Line8; // 使能上升沿检测
     EXTI->FTSR |= EXTI_Line8; // 使能下降沿检测
}





void MotorContor(void)
{


     if(ControlValue>600)ControlValue=600; 

        
       if(Flag_SysRun==1)
    	{
                 Y2=YON ;    // MOTOR EN  RUN 
                 PwmDuty= OutPutDuty+Min_Duty; 
    	}
	else
	{
                Y2=YOFF ;  // STOP 
                PwmDuty=0;		
	}

	 TIM_SetCompare1(TIM8,PwmDuty);  

}



//
//  SPEED =  PULS*2/3 RPM（转/分） .
//  SYS-FREQ  = PULS X26/25;

// entern period 10ms  
void CalcuSpeed(void)
{
         u8  scale_num=0; 
		 
               static  u8 dlycnt=20;  //  200ms 
			    
                float tmp=0;
		u32  tempsum=0; 
		u32  tempsum1=0; 
		 u8  i=0; 

	if(dlycnt>0){dlycnt--; return; }
	dlycnt=20; 

     //  500000X13= 6500000   
     //  6500000/3= 2166666
     //   21666666 
     
      //  tmp =2166666/MonitorDat2; 
     //   SysRun_Freq =tmp;
	
	if(Flag_Get_Dat_Req==1)
	{
                     Flag_Get_Dat_Req =0; 
					 
                     tempsum=0; 
		   for(i=0;i<6;i++)
		   {
                             tempsum+=TimeCntBufBackup[i]; 
		   }
		   
		   tempsum1 = tempsum;
		   
		   tempsum/=6; 	
		   
		 //  tmp=  2166666/tempsum; 
		 //  SysRun_Freq =tmp;  

                     tmp =   30000000/ tempsum1;
		  MotorSpeed	 =tmp ; 	

                 
                  switch(TestPlateType)
          	{
       	         case 0:     scale_num=26;  //M12 
				 break;  
		case 1:     scale_num=16;  // M18 
			        break;
		 case 2:    scale_num=12;  // M24 
		 	        break; 
		case 3:    scale_num=10;  // M30 
			     break;
	          default:   
		  	     scale_num=26; 
		  	    break; 			 	   
               } 

	      SysRun_Freq =     tmp*scale_num/60; 

		  
		   
	}
	else
	{
	        if(LowSpeedMeasCnt>5)
	     	{
                      MotorSpeed=0; 
		    SysRun_Freq=0; 
	     	} 
                
	}

        
   
	 	
}



void LossManage(void)
{
      u8  scale_num=0; 
	  
	  	 
       CH1_Cnt = PulsLossMeasCnt_CH1_backup;
       CH2_Cnt = PulsLossMeasCnt_CH2_backup;

       switch(TestPlateType)
       	{
       	         case 0:     scale_num=26;  //M12 
				 break;  
		case 1:     scale_num=16;  // M18 
			        break;
		 case 2:    scale_num=12;  // M24 
		 	        break; 
		case 3:    scale_num=10;  // M30 
			     break;
	      default:   
		  	     scale_num=26; 
		  	    break; 			 	   
       } 
	   
       CH1_Pass_Cnt  =CircleNum *scale_num; 
       CH2_Pass_Cnt  =CH1_Pass_Cnt; 
 
}



u8 SysWorkMode=0;
u8 RunState=0; 
u8 PauseState=0; 

void TaskSwitch(void)
{

  
          switch(SysWorkMode)
         {
             case   WORKMODE_IDLE:
		  	                                    if(KeyCmd==CMD_START)
		  	                               	{
		  	                               	      KeyCmd=0; 
							      RunState=0; 
							      SysWorkMode= WORKMODE_FREERUN;	  
		  	                               	}
		 	                                   break; 
										   
             case   WORKMODE_FREERUN:
			 	                         if(KeyCmd==CMD_STOP)
		  	                               	{
		  	                               	        KeyCmd=0;  						  
							       PauseState=0; 
							       SysWorkMode= WORKMODE_PAUSE;	  
		  	                               	}
		  	                             
			                               break;
										   
             case   WORKMODE_PAUSE:
			 	                      ;
		  	                             
			                              break; 
       }

   
}


void Idle1(void)
{
        ; 
}


int AddTime=0;
int DecTime=0; 

void FreeRun(void)
{

      Min_Duty= MIN_DUTY;  
   	
       switch(RunState)
      	{
              case 0: 
			  Flag_SysRun=1;    
			  OutPutDuty=0; 
			  
			  RunState=1; 
			 break; 
	     case 1:
		           if(ControlValue>OutPutDuty)
		           {
		                 AddTime++;
			        if(AddTime>ADD_DLYTIME)
			       {
                                       if( OutPutDuty<ControlValue)
				   {
				        OutPutDuty++;
                                       } 
			            AddTime=0; 
			     }    				  
		          }
		         else
		         {
                                   DecTime++;
				if(DecTime>DEC_DLYTIME)
				{
				      if( ControlValue<OutPutDuty)
				      {
				             if(OutPutDuty>0) {OutPutDuty--;} 
                                         } 
			             DecTime=0; 
                                    
				}       				  
		         } 
		 	 break; 
	     case 2: 
			break; 
              default:
			 break; 
      	}
	 
       

}


void Pause(void)
{
      Min_Duty= 20;  
       
        switch(PauseState)
  	{

             case 0: 
			   PauseState=1; 	
			   DecTime=0;  
			    break; 
	    case 1:
			        DecTime++;
				if(DecTime>DEC_DLYTIME)
				{  
				
				      if(OutPutDuty>0) {OutPutDuty--;} 
				      else  if(MotorSpeed<150)
				      	{
				      	     PauseState=2 ; 
				      	} 
					
			               DecTime=0; 
				}   
			   break; 
	   case 2:
	   	           Flag_SysRun=0;   
			  PauseState=0;
			  SysWorkMode= WORKMODE_IDLE; 
	   	           break;  
 
            default:
		          break; 
 
  	}



   
     ;  
}

void MotorRunManage(void)
{

     TaskSwitch(); 
     
     switch(SysWorkMode)
     {
          case   WORKMODE_IDLE:
		  	                               Idle1(); 
		 	                               break; 
										   
          case   WORKMODE_FREERUN:
		  	                               FreeRun(); 
			                               break;
										   
          case   WORKMODE_PAUSE:
		  	                               Pause(); 
			                              break; 
          
     }

}

