
#include "include.h" 



uint  DcMeterV2_Value=0;
uint  DcMeterA2_Value=0;



// Y OUT 
// 1: OFF
// O: ON


void Display(void)
{
  static int cnt=0;


  cnt++;
  if(cnt>100)
  	{
  	   cnt=0; 
	   
            LED1=!LED1;
	   LED2=!LED2; 
	 
		 if(Flag_SysRun==1)
		 	{
		 	      Y0=  YOFF;   //  YELLOW   OFF  
                                  ///Y3=!Y3; ;
			      Y3 = YON;  	  // GREEN  ON 	
			  
		 	}
		 else
		 	{
		 	       Y0= YON;     // YELLOW  ON 
		                Y3=YOFF;    // GREEN  OFF
		              
		 	} 	   
  	}


   Monitor_timer3=TIM3->CNT;  

}



void Ele_load_Process(void)
{
     if(Ele_Load_Off_TimeOvCnt==0){Ele_Load_En=1;}
}



 
// ±¨¾¯¹ÜÀí
u16 AlarmGapCnt=0; 
u8  Flag_Alarm_Bits=0; 

void Alarm_Manage(void)
{
      static u8 alarm_step=0; 
      static u8 alarm_times=0; 


      
    if((SafeSwitch_State==0)&&(Flag_SysRun==1))
    {
           Flag_Alarm_Bits  |=0x01; 
    }
   else
   {
              Flag_Alarm_Bits&=0xFE  ;   
   }
  



   if(AlarmGapCnt>0)AlarmGapCnt--;  
	
   switch(alarm_step)
   	{ 

	   case 0: 
	   	     if(Flag_Alarm_Bits)
	   	     	{
	   	     	    alarm_times=0; 
                               alarm_step++;  
	   	     	}
	   	     break; 
	   case 1:
	   	   
			  BeepCode=1; 
			  AlarmGapCnt=10; 
			  alarm_step++; 
	   	      break; 
	   case 2: 
	   	     if(AlarmGapCnt==0)
	   	     	{
	   	     	    if(alarm_times<3)
	   	     	    {
	   	     	      alarm_times++;
					  alarm_step=1; 
					  
	   	     	    }
					else
					{ 
				       alarm_step++; 
					}
					
	   	     	} 
	   	      break;
	   case 3: 
	   	           Flag_Alarm_Bits=0 ;
			   alarm_step=0; 
	   	       break; 
	   default: 
	   	       break; 
			     			 
          ; 
   	}
  
}


