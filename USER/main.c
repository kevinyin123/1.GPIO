

//头文件
#include "include.h" 
#include "stm32f10x_flash.h"



#include "modbus.h"


//--------------------------------------------

ErrorStatus HSEStartUpStatus;

EXTI_InitTypeDef EXTI_InitStructure;



//函数声明
void GPIO_Configuration(void);

//=============================================================================
//文件名称：Delay
//功能概要：延时
//参数说明：nCount：延时长短
//函数返回：无
//=============================================================================

void Delay(uint32_t nCount)
{
  for(; nCount != 0; nCount--);
}


/*******************************************************************************
* Function Name  : RCC_Configuration
* Description    : Configures the different system clocks.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RCC_Configuration(void)
{
  /* RCC system reset(for debug purpose) */
  RCC_DeInit();

  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);

  /* Wait till HSE is ready */
  HSEStartUpStatus = RCC_WaitForHSEStartUp();

  if(HSEStartUpStatus == SUCCESS)
  {
    /* HCLK = SYSCLK */
    RCC_HCLKConfig(RCC_SYSCLK_Div1); 
  
    /* PCLK2 = HCLK */
    RCC_PCLK2Config(RCC_HCLK_Div1); 

    /* PCLK1 = HCLK/2 */
    RCC_PCLK1Config(RCC_HCLK_Div2);

    /* Flash 2 wait state */
    FLASH_SetLatency(FLASH_Latency_2);
    /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

    /* PLLCLK = 8MHz * 9 = 72 MHz */
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

    /* Enable PLL */ 
    RCC_PLLCmd(ENABLE);

    /* Wait till PLL is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    {
    }

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    /* Wait till PLL is used as system clock source */
    while(RCC_GetSYSCLKSource() != 0x08)
    {
    }
  }
   
  /* Enable GPIOB, GPIOC and AFIO clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC |  RCC_APB2Periph_GPIOD  |
                         RCC_APB2Periph_AFIO, ENABLE);
}


// time1 
/************************************************
函数名称：void Capture_Config(void)
函数功能：对用到的时钟进行初始化，打开各模块时钟
入口参数：无
返回值：  无
说明  ：  主要是打开GPIOF和TIM3的时钟
************************************************/
void Capture_Config(void)
{
	
	GPIO_InitTypeDef GPIO_InitStructure;
       TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
   TIM_ICInitTypeDef  TIM_ICInitStructure;
	 NVIC_InitTypeDef NVIC_InitStructure;
	
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO,ENABLE);	//GPIOF时钟打开
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);	//TIM1时钟打开

   //X0- PA8 - TIM1-CH1
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	
   GPIO_Init(GPIOA,&GPIO_InitStructure);

   TIM_DeInit(TIM1);
   TIM_TimeBaseStructInit(&TIM_TimeBaseInitStructure);
   
	
  TIM_TimeBaseInitStructure.TIM_Period        = 0xffff;   // 16位计数
   TIM_TimeBaseInitStructure.TIM_Prescaler     = 144-1;   // 144分频 2us
   TIM_TimeBaseInitStructure.TIM_ClockDivision = 0;        // 不分割
   TIM_TimeBaseInitStructure.TIM_CounterMode   = TIM_CounterMode_Up; // 上升计数
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter=0;  
	 
   TIM_TimeBaseInit( TIM1, &TIM_TimeBaseInitStructure );
   
  TIM_ICInitStructure.TIM_Channel     = TIM_Channel_1;   //捕获通道 IC1
   TIM_ICInitStructure.TIM_ICPolarity  = TIM_ICPolarity_Rising;  // 上升沿触发
   TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;   //
   TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;// 直接映射
   TIM_ICInitStructure.TIM_ICFilter    = 0x00;  // 不滤波
   TIM_ICInit( TIM1, &TIM_ICInitStructure );
 
   TIM_Cmd( TIM1, ENABLE );
   TIM_ITConfig( TIM1,TIM_IT_CC1|TIM_IT_Update, ENABLE );//使能更新中断和捕获中断
   TIM_ClearFlag(TIM1, TIM_IT_CC1|TIM_IT_Update);
	 
  // NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

  // config NVIC 
  			   
   NVIC_InitStructure.NVIC_IRQChannel        = TIM1_CC_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority       = 11;
   NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
   NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel        = TIM1_UP_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority       = 12;
   NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
   NVIC_Init(&NVIC_InitStructure);

   
   
}



// timer2
void time2_init(u16 per,u16 pre)
{  

  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;  

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);//使能TIM2的时钟    
  TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;//TIM_CKD_DIV1是.h文件中已经定义好的，TIM_CKD_DIV1=0，也就是时钟分频因子为0    
  TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;//计数方式为向上计数 
  TIM_TimeBaseStructure.TIM_Period=per;//周期    
  TIM_TimeBaseStructure.TIM_Prescaler=pre;//分频系数  
  TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);  
  TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);//使能TIM2的更新中断 
  TIM_Cmd(TIM2,ENABLE);//使能TIM2
}



void time3_init(u16 per,u16 pre)
{  
  GPIO_InitTypeDef GPIO_InitStructure;
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;  
  TIM_ICInitTypeDef TIM_ICInitStructure;   	

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);//使能TIM3的时钟    


    //PA5,PA6,PA7, 
   GPIO_InitStructure.GPIO_Pin =(GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7);
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //浮空上拉输入
   GPIO_Init(GPIOA, &GPIO_InitStructure);     

  
  TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;//TIM_CKD_DIV1是.h文件中已经定义好的，TIM_CKD_DIV1=0，也就是时钟分频因子为0    
  TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;//计数方式为向上计数 
  TIM_TimeBaseStructure.TIM_Period=per;//周期    
  TIM_TimeBaseStructure.TIM_Prescaler=pre;//分频系数  
  TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);  

 TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_BothEdge ,TIM_ICPolarity_BothEdge);// 使用编码器模式3，上升沿下降沿都计数
 TIM_ICStructInit(&TIM_ICInitStructure);// 将结构体中的内容缺省输入
 TIM_ICInitStructure.TIM_ICFilter = 6; // 选择输入比较滤波器
 TIM_ICInit(TIM3, &TIM_ICInitStructure);  // 载入结构体


  TIM_ClearFlag(TIM3, TIM_FLAG_Update);//清除更新标志	
  TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);//使能TIM3的更新中断 

  TIM3->CNT = 30000;// clr the cnt
  
 TIM_Cmd(TIM3,ENABLE);//使能TIM3

  
}



void TIM3_NVIC_INIT(void)
{   

  NVIC_InitTypeDef NVIC_InitStructure; 

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);//设定中断优先级分组0   
  NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn;//设定中断向量   本程序为TIM3的中断   
  NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;//使能响应中断向量的中断响应   
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//配置中断向量的抢占优先级   
  NVIC_InitStructure.NVIC_IRQChannelSubPriority=3;//配置中断向量的响应优先级    
  NVIC_Init(&NVIC_InitStructure);//根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

}



//TIM4 PWM部分初始化 
//PWM输出初始化
//arr：自动重装值
//psc：时钟预分频数
void TIM4_PWM_Init(u16 arr,u16 psc)
{  
      GPIO_InitTypeDef GPIO_InitStructure;
      TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
      TIM_OCInitTypeDef  TIM_OCInitStructure;
                                                    
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);	//使能定时器4时钟
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB  | RCC_APB2Periph_AFIO, ENABLE);  //使能GPIO外设和AFIO复用功能模块时钟
  
    //  GPIO_PinRemapConfig(GPIO_Remap_TIM4, ENABLE); //Timer4部分重映射  TIM4_CH1->PB6 
   
    //设置该引脚为复用输出功能,输出TIM4 CH1的PWM脉冲波形	GPIOB.6
     // GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; //TIM_CH2
     // GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //推挽输出 复用功能 
     // GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    //  GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIO

       // 设置为普通输出口
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; //TIM4_CH1
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   //设置为普通输出口
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
      GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIO
	  
 
	   //初始化TIM4
      TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
      TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值 
      TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
      TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
      TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

      //初始化TIM4 Channel1 PWM模式	 
    //  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式: 
    //  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
    //  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
  
      TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing; //选择定时器模式: 冻结模式
      TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
      TIM_OCInitStructure.TIM_Pulse = 10;
      TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

	  TIM_OC1Init(TIM4, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM4 OC1
      TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);  //使能TIM4在CCR1上的预装载寄存器

    //  TIM_SelectOnePulseMode(TIM4,TIM_OPMode_Single);  // 单脉冲模式

	  TIM_ClearFlag(TIM4, TIM_FLAG_Update);
      TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);//使能TIM4的更新中断 
 
     //  TIM_Cmd(TIM4, ENABLE);  //使能TIM3
 }

void TIM4_NVIC_INIT(void)
{   

  NVIC_InitTypeDef NVIC_InitStructure; 

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);//设定中断优先级分组0   
  NVIC_InitStructure.NVIC_IRQChannel=TIM4_IRQn;//设定中断向量   本程序为TIM4的中断   
  NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;//使能响应中断向量的中断响应   
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//配置中断向量的抢占优先级   
  NVIC_InitStructure.NVIC_IRQChannelSubPriority=3;//配置中断向量的响应优先级    
  NVIC_Init(&NVIC_InitStructure);//根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

}


//TIM8 PWM部分初始化 
//PWM输出初始化
//arr：自动重装值
//psc：时钟预分频数
void TIM8_PWM_Init(u16 arr,u16 psc)
{  
      GPIO_InitTypeDef GPIO_InitStructure;
      TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
      TIM_OCInitTypeDef  TIM_OCInitStructure;

	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);										
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);	//使能定时器8时钟
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);  //使能GPIO外设和AFIO复用功能模块时钟
  
     // GPIO_PinRemapConfig(GPIO_Remap_TIM4, ENABLE); //Timer4部分重映射  TIM4_CH1->PB6 
   
    //设置该引脚为复用输出功能,输出TIM4 CH1的PWM脉冲波形	GPIOB.6
     // GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; //TIM_CH2
     // GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //推挽输出 复用功能 
     // GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    //  GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIO

       // 设置为普通输出口
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; //TIM8_CH1 // PC6 
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   ////推挽输出 复用功能 
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
      GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIO
	  

           TIM_DeInit(TIM8);
	  TIM_OCStructInit(&TIM_OCInitStructure);
	  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	  

	   //初始化TIM8
      TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
      TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值 
      TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
      TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
      TIM_TimeBaseStructure.TIM_RepetitionCounter=0;  
	  
      TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

      //初始化TIM4 Channel1 PWM模式	 
    //  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式: 
    //  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
    //  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高


   //-----------------------------------
   // TIM_OCInitStruct->TIM_OCMode = TIM_OCMode_Timing;
   // TIM_OCInitStruct->TIM_OutputState = TIM_OutputState_Disable;
   // TIM_OCInitStruct->TIM_OutputNState = TIM_OutputNState_Disable;
   // TIM_OCInitStruct->TIM_Pulse = 0x0000;
   // TIM_OCInitStruct->TIM_OCPolarity = TIM_OCPolarity_High;
   // TIM_OCInitStruct->TIM_OCNPolarity = TIM_OCPolarity_High;
   // TIM_OCInitStruct->TIM_OCIdleState = TIM_OCIdleState_Reset;
   // TIM_OCInitStruct->TIM_OCNIdleState = TIM_OCNIdleState_Reset;

	//--------------------------------------

	  
  
      TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //选择定时器模式: 冻结模式
      TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	  TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;  // 禁止互补通道输出
      TIM_OCInitStructure.TIM_Pulse = 10;  //CCR, 设置占空比，
      TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	  TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
	  TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset ;//TIM8关闭空闲状态(使用TIM8时候开启)
	  TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset ;//TIM8关闭空闲状态(使用TIM8时候开启)
	
	

	  TIM_OC1Init(TIM8, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM4 OC1
      TIM_OC1PreloadConfig(TIM8, TIM_OCPreload_Enable);  //使能TIM4在CCR1上的预装载寄存器
      TIM_ARRPreloadConfig(TIM8, ENABLE); //使能TIMx在ARR上的预装载寄存器


	 // TIM_ClearFlag(TIM8, TIM_FLAG_Update);
	 
      TIM_Cmd(TIM8, ENABLE);  //使能TIM8
	  TIM_CtrlPWMOutputs(TIM8,ENABLE); // MOE 主输出使能
	 
 
    
 }


 #define PWM_FREQENCY  4000
 
 static void timer8_PWM_Init(void)
{
     GPIO_InitTypeDef GPIO_InitStructure; 
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);	//使能定时器8时钟
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC |RCC_APB2Periph_AFIO, ENABLE);  //使能GPIO外设和AFIO复用功能模块时钟

	      // 设置为普通输出口
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; //TIM8_CH1 // PC6 
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   ////推挽输出 复用功能 
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
      GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIO
      
    
    TIM_DeInit(TIM8);
    //TIM_InternalClockConfig(TIM8);  
    
    /* Time base configuration */    
    TIM_TimeBaseStructure.TIM_Prescaler = 71;        // prescaler = 71, TIM_CLK = 72MHZ/(71+1) = 1MHZ.    
    TIM_TimeBaseStructure.TIM_Period = 1000 -1 ;         // ?????0???999,??1000?,???????
                                                    // pwm F = 1MHZ/(3999+1) = 250HZ.  
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1 ;    
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //??????
    TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);
    
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;      
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; 


    /* PWM1 Mode configuration: Channel1 */

	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable; //关闭互补通道 
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = (90 * PWM_FREQENCY )/100;          
    TIM_OC1Init(TIM8, &TIM_OCInitStructure);                      
    TIM_OC1PreloadConfig(TIM8, TIM_OCPreload_Enable);
    
    TIM_ARRPreloadConfig(TIM8, ENABLE);                            

    /* TIM8 enable counter */
    TIM_Cmd(TIM8, ENABLE);               
    TIM_CtrlPWMOutputs(TIM8, ENABLE);   
}


// 配置外部中断
void EXTILine_Config(void)
{

  // 这是 F407的配置方式
 //  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);  //  这是F407的配置方式
  // SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, AxisEXTIPinSource[0]);  // 这是407的配置方式
  
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE); //开启AFIO时钟	

    // PA0,PA1,PA8

    //PA0,PA1 as	exti   port 
   // GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0|GPIO_PinSource1);  //这是以前的F103所用的配置函数
   GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);  //这是以前的F103所用的配置函数
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);  //这是以前的F103所用的配置函数


   //PA8 as  exti   port 
   GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource8); 

   // PB15 
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource15); 
 


   // 先关闭上升沿、下降沿检测
   
    EXTI->RTSR &= ~EXTI_Line0; // 关闭上升沿检测
    EXTI->FTSR &= ~EXTI_Line0; // 关闭下降沿检测

    EXTI->RTSR &= ~EXTI_Line1; // 关闭上升沿检测
    EXTI->FTSR &= ~EXTI_Line1; // 关闭下降沿检测
    
    EXTI->RTSR &= ~EXTI_Line8; // 关闭上升沿检测
    EXTI->FTSR &= ~EXTI_Line8; // 关闭下降沿检测

	
    
    EXTI->RTSR &= ~EXTI_Line15; // 关闭上升沿检测
    EXTI->FTSR &= ~EXTI_Line15; // 关闭下降沿检测


    EXTI_StructInit(&EXTI_InitStructure); 
	 

 // 频率检测使用中断
 // 设置为上升下降沿中断模式
 //在中断程序中需要根据常开常闭类型来处理检测结果
 
  EXTI_InitStructure.EXTI_Line = EXTI_Line0;// 设置中断线
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  //   EXTI_Trigger_Rising
  EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
  EXTI_Init(&EXTI_InitStructure);	


  EXTI_InitStructure.EXTI_Line = EXTI_Line1;// 设置中断线
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  //   EXTI_Trigger_Rising
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);	


  EXTI_InitStructure.EXTI_Line = EXTI_Line8;// 设置中断线
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  //   EXTI_Trigger_Rising
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);	

   EXTI_InitStructure.EXTI_Line = EXTI_Line15;// 设置中断线
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  //   EXTI_Trigger_Rising
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);	


  
}


void NVIC_Configuration(void)
{


   NVIC_InitTypeDef NVIC_InitStructure;

   // TIM2 
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);//设定中断优先级分组0   
   NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn;//设定中断向量   本程序为TIM3的中断   
   NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;//使能响应中断向量的中断响应   
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//配置中断向量的抢占优先级   
   NVIC_InitStructure.NVIC_IRQChannelSubPriority=4;//配置中断向量的响应优先级    
   NVIC_Init(&NVIC_InitStructure);//根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

   // TIM3 
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);//设定中断优先级分组0   
   NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn;//设定中断向量   本程序为TIM3的中断   
   NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;//使能响应中断向量的中断响应   
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//配置中断向量的抢占优先级   
   NVIC_InitStructure.NVIC_IRQChannelSubPriority=5;//配置中断向量的响应优先级    
   NVIC_Init(&NVIC_InitStructure);//根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

  // TIM4 
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);//设定中断优先级分组0   
  NVIC_InitStructure.NVIC_IRQChannel=TIM4_IRQn;//设定中断向量   本程序为TIM4的中断   
  NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;//使能响应中断向量的中断响应   
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//配置中断向量的抢占优先级   
  NVIC_InitStructure.NVIC_IRQChannelSubPriority=6;//配置中断向量的响应优先级    
  NVIC_Init(&NVIC_InitStructure);//根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

 

  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);//设定中断优先级分组1   
  // EXIT0   外部中断先
  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // 抢占优先级
   NVIC_InitStructure.NVIC_IRQChannelSubPriority=7;//响应优先级
  NVIC_Init(&NVIC_InitStructure);
  // EXIT1   外部中断先
  NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority=8;//响应优先级
  NVIC_Init(&NVIC_InitStructure);
 // EXIT9-5   外部中断先
  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority=9;//响应优先级
  NVIC_Init(&NVIC_InitStructure);

// EXIT15-10	 外部中断先
 NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority=10;//响应优先级
  NVIC_Init(&NVIC_InitStructure);

 
	
}


void SetEXTI_Trigger_Falling(void)
{

  EXTI_InitStructure.EXTI_Line = EXTI_Line8|EXTI_Line9;// 设置中断线
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  //   EXTI_Trigger_Rising
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);	
}



void SetEXTI_Trigger_Rising(void)
{

  EXTI_InitStructure.EXTI_Line = EXTI_Line8|EXTI_Line9;// 设置中断线
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //   EXTI_Trigger_Rising
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);	
}


u16 IR_ThisPulse=0; 
u16 IR_LastPulse=0; 
u16 IR_PulseSub=0; 

 
  // 
 void TIM1_CC_IRQHandler(void) //TIME2中断服务函数	需要设定中断优先级	即NVIC配置
 {	

    if(TIM_GetITStatus(TIM1 , TIM_IT_CC1) != RESET ) 
    {    
           TIM_ClearITPendingBit(TIM1 , TIM_IT_CC1);
          IR_ThisPulse = TIM_GetCapture1(TIM1);	// GET CCR1  

	  //if(!Coder_PIN)

           if(IR_ThisPulse>IR_LastPulse)
	   {
	               if(LowSpeedMeasCnt==0){
                              IR_PulseSub= IR_ThisPulse- IR_LastPulse; 
	         	}
			 else{
                             IR_PulseSub=0xffff; 
			}
	   }
          else									
	   { 
	           if(LowSpeedMeasCnt<2){
		     IR_PulseSub = 0xffff-IR_LastPulse+IR_ThisPulse;  
	           }
		 else {
                     IR_PulseSub=0xffff; 
		} 
	   }

	  MonitorDat2 =IR_PulseSub;
	  TimeCntBuf[SavePoint++]= IR_PulseSub;
	  if(SavePoint>5)
	  {
	  	  SavePoint=0;  
		 
		  TimeCntBufBackup[0]= TimeCntBuf[0]; 
		  TimeCntBufBackup[1]= TimeCntBuf[1]; 
		  TimeCntBufBackup[2]= TimeCntBuf[2]; 
	          TimeCntBufBackup[3]= TimeCntBuf[3]; 
		 TimeCntBufBackup[4]= TimeCntBuf[4]; 
	          TimeCntBufBackup[5]= TimeCntBuf[5];  
		 Flag_Get_Dat_Req=1;   

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

	    
         LowSpeedMeasCnt=0;  // clr the cnt 
	
         IR_LastPulse = IR_ThisPulse; 
   }

	
	
  }


 // 
void TIM1_UP_IRQHandler(void) //TIME2中断服务函数  需要设定中断优先级  即NVIC配置
{  
         if(TIM_GetITStatus(TIM1,TIM_IT_Update) != RESET)
         {
	 TIM_ClearITPendingBit(TIM1 , TIM_IT_Update);

	
           if(LowSpeedMeasCnt<10)
		 {
		     LowSpeedMeasCnt++; 
		 } 
    	}
    
 }



int TimeCountMs=0; 
int timeCountMin=0; 
 int time_cnt=0; 

 
 // 1MS 
void TIM2_IRQHandler(void) //TIME2中断服务函数  需要设定中断优先级  即NVIC配置
{  

   if(TIM_GetITStatus(TIM2,TIM_IT_Update)!=RESET)//判断是否发生了更新(溢出)中断   
   {      
   TIM_ClearITPendingBit(TIM2,TIM_IT_Update);//清除溢出中断标志位    
   }  

       Pingbang=0; 

        time_cnt++;
	if(time_cnt>=1000) // 1000ms=1s  
	{
	       time_cnt=0; 
		   
               //------------------------
                  CH1_Freq =   Puls_cnt_CH1;
		CH2_Freq =   Puls_cnt_CH2;
                  Puls_cnt_CoderBackup= Puls_cnt_Coder; 
                 
                   

           	Puls_cnt_CH1=0; 
		Puls_cnt_CH2=0;
		Puls_cnt_Coder=0;
              //-------------------------
	  	   

	}

 
}




// null 

void TIM3_IRQHandler(void) //TIME3中断服务函数  需要设定中断优先级  即NVIC配置
{  

  if(TIM_GetITStatus(TIM3,TIM_IT_Update)!=RESET)//判断是否发生了更新(溢出)中断   
  {      
   TIM_ClearITPendingBit(TIM3,TIM_IT_Update);//清除溢出中断标志位    
  }  


  //Monitor_timer3=TIM3->CNT;
 
}


// 
void IniSysParameter(void)
{
   ClearBuf();
   
}




void Testcode(void)
{
    
    if(TestTimeDly==0)
  	{
  	   TestTimeDly=10;  // 10s 
  	     
  	}
    
    
}

//=============================================================================
//文件名称：main
//功能概要：主函数
//参数说明：无
//函数返回：int
//=============================================================================
int main(void)
{
       
           RCC_Configuration();  
	   GPIO_Configuration();
	   
           SMEC_I2cInit(); 
	   IIC_SetSdaPin(); IIC_SetSclPin();
	   
	      
	    Usart1_Init(19200);    //串口波特率设置  19200   // 485 PORT       //
        Usart2_Init(9600);    //串口波特率设置9600, for touch screen 
        Usart3_Init(19200);   // rs485,for ele-load   2024.12.29 
	  //  Usart4_Init(19200);   // rs485, for ele-load   ---old ,new  is null

             time2_init(1000,71);  

	   // 72/72=1M,   TCY=1US     20000*1US=20MS 
        //  time3_init(1000,71);  //最大计数65535，预分频0，
  

	  // TIM4_PWM_Init(509,8);  // psc: = 8+1=9     SET the time4's driver frequency as 8MHZ( 72/(8+1))        
	   //TIM_SetCompare1(TIM4,200);


       //TIM8 PWM1 config// PC6, TIM8-CH1
	   TIM8_PWM_Init(1000,71);
	   TIM_SetAutoreload(TIM8, 1000); // 改变ARR, 改变频率
           TIM_SetCompare1(TIM8,100);  
		   
	   Capture_Config();  //time1 
   
	   //-------------------------------------------------------------------------
	   
           EXTILine_Config(); 
	   
	   NVIC_Configuration();
 
	   Delay(0xfffff); 
    
	   Delay(0xFFffff);

	   IniSysParameter();
	   
	   WP=0; 
      
           IniEEprom(); // 
	
	   
	   UART1_485_RD=0;  // recevied   mode    
	   UART3_485_RD=0;  // recevied   mode    
	   UART4_485_RD=0;  // recevied   mode    
	   
	   Bell=YOFF;

	   CTR_5V=1; 
	   
            Flag_LossMeas=1;
		
       
	   // mobus inital	 addr=1,
		 (void)eMBInit( MB_RTU, 0x01, 2, 9600, MB_PAR_EVEN );
		 eMBRTUStart(); 


	 
    while (1)
	{
	   
        if(Pingbang==0) //  1MS    
		{  
		  
                  // LED1=!LED1;
                   // Y0=!Y0; 
		 //  Y1=!Y1;  
		  // Y2=!Y2; 
		  // Y3=!Y3;   

		 //  Y1=1; 

		   WakeNb++;
		    if(WakeNb>9) // 10ms 
		   {
                             WakeNb=0;  
		   }
 
	 		   
                    TimeBase();    

	 	   if(WakeNb==1){  Display();  }
		   if(WakeNb==2){  EEprom_Manage();}
                     if(WakeNb==3){ KeyscanNormal() ;}      
		   if(WakeNb==4){  key_scan(); ReadKey(); }   
		   if(WakeNb==5){  CalcuSpeed() ;LossManage(); }
		   if(WakeNb==5){  MotorContor(); MotorRunManage(); }
		   if(WakeNb==6){  Ele_load_Process();} //
		   
		   if(WakeNb==7){Alarm_Manage() ;  }
                    if(WakeNb==7){ BeepDriver();          }
                    if(WakeNb==8){ Testcode(); ModbusTestcode(); } 

		   (void) eMBPoll();  // MODBUS POLLING   // uart2

		   Uart1_Manage(); //   and codeswith_bd 
		   Uart2_Manage(); // rs232 for touch screen 
           Uart3_Manage(); //  485 for load_board   2024.12.29 
            //   Uart4_Manage(); // 485 for load_board 
           
           
           Pingbang=1; 
	  	}	
		 
    }
}




//=============================================================================
//文件名称：GPIO_Configuration
//功能概要：GPIO初始化
//参数说明：无
//函数返回：无
//=============================================================================
void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;


  // PD,PC,PB,PA  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); 


  //=====PORTD========================================================================
 // OUT	CONFIG 
 // PD2   Y3
 //===================================================================================			  
   GPIO_InitStructure.GPIO_Pin =(GPIO_Pin_2);
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
   GPIO_Init(GPIOD, &GPIO_InitStructure);
   
//-----------END PORTD----------------------------------------------------------------


 
 // RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC, ENABLE); 						 
//=======POARTC======================================================================
// PC  OUT CONFIG  
// PC0  NULL 
// PC1  WP 
// PC2  I2C_SCL 
// PC3  I2C_SDA
// PC4  NULL 
// PC5  NULL 
// PC6  PWM1   
// PC10  UART4_TX 
// PC12 Y2 

// PC13 Y6
// PC14 Y7 
// PC15 Y8 
//=============================================================================			 
  GPIO_InitStructure.GPIO_Pin = (GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_10|GPIO_Pin_12);
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
  GPIO_Init(GPIOC, &GPIO_InitStructure);
 
  //PC   INPUT    CONFITE  
  // PC7   X3
  // PC8   X2
  // PC9   X1
  // PC11  UART4_RX 
  // PC13   KA  
  //PC14  EA 
  //PC15   EB 
  
   GPIO_InitStructure.GPIO_Pin =(GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_11|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 内部上拉输入模式 
   GPIO_Init(GPIOC, &GPIO_InitStructure);  
   
//--------END PORTC--------------------------------------------------------------------------------------




//=====POARTB========================================================================
//PORTB 
// OUT  
//PB0   BELL   
//PB1   LED1 
//PB3   Y4 
//PB4   Y5  
//PB5   NULL 
//PB6  Y0 
//PB7  Y1 
//PB9   5V_CTR 
//PB10  NULL 
//PB11

//=============================================================================			 
  GPIO_InitStructure.GPIO_Pin =(GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_9);
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
  GPIO_Init(GPIOB, &GPIO_InitStructure);



 // IN

 // PB2  BOOT1
 // PB8   NULL 
 // PB11  UART3-RX 
 // PB12  X7  
 // PB13  X6 
 // PB14  X5 
 // PB15  X4 
 
   GPIO_InitStructure.GPIO_Pin =(GPIO_Pin_2|GPIO_Pin_8|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 内部上拉输入模式 
   GPIO_Init(GPIOB, &GPIO_InitStructure);   
   
//----------END POARTB-------------------------------------------------------------------------------------   


// RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
// GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); 
// RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA , ENABLE);

  
//=============================================================================
// OUT  CONFIG 
// PA2, 
// PA2 UART2_TX  
// PA7   R85_RD3 
// PA9 ,UART1_TX
// PA11, 485_RD1  
// PA12, LED2 
// PA15, 485_RD2 
//===================================================================================			 
  GPIO_InitStructure.GPIO_Pin =(GPIO_Pin_2|GPIO_Pin_4|GPIO_Pin_7|GPIO_Pin_9|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_15);
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
  GPIO_Init(GPIOA, &GPIO_InitStructure);


   //PORTA  IN 
   //PA0,PA1,PA3, 
   //PA3 UART2_RX
  //PA5(EDZ),PA6(EDA),PA7(EDB), 
  //PA8  X0 
  //PA10  UART1_RX 
  

   GPIO_InitStructure.GPIO_Pin =(GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_3|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_10);
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 内部上拉输入模式 
   GPIO_Init(GPIOA, &GPIO_InitStructure);     

  // PARTA ANALOG  DAC 
  // PA4 
   GPIO_InitStructure.GPIO_Pin =GPIO_Pin_4;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;  // 
   GPIO_Init(GPIOA, &GPIO_InitStructure);  
  
 // RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);//使能端口复用时钟 
 // GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);//失能JTAG 

   LED1=1;
   LED2=1; 
 
   Y0=1;Y1=1;Y2=1;Y3=1;
   WP=0; 
   
 

  
}



