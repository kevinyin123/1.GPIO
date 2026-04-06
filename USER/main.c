

//ͷ�ļ�
#include "include.h" 
#include "stm32f10x_flash.h"



#include "modbus.h"


//--------------------------------------------

ErrorStatus HSEStartUpStatus;

EXTI_InitTypeDef EXTI_InitStructure;



//��������
void GPIO_Configuration(void);

//=============================================================================
//�ļ����ƣ�Delay
//���ܸ�Ҫ����ʱ
//����˵����nCount����ʱ����
//�������أ���
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
�������ƣ�void Capture_Config(void)
�������ܣ����õ���ʱ�ӽ��г�ʼ�����򿪸�ģ��ʱ��
��ڲ�������
����ֵ��  ��
˵��  ��  ��Ҫ�Ǵ�GPIOF��TIM3��ʱ��
************************************************/
void Capture_Config(void)
{
	
	GPIO_InitTypeDef GPIO_InitStructure;
       TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
   TIM_ICInitTypeDef  TIM_ICInitStructure;
	 NVIC_InitTypeDef NVIC_InitStructure;
	
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO,ENABLE);	//GPIOFʱ�Ӵ�
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);	//TIM1ʱ�Ӵ�

   //X0- PA8 - TIM1-CH1
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	
   GPIO_Init(GPIOA,&GPIO_InitStructure);

   TIM_DeInit(TIM1);
   TIM_TimeBaseStructInit(&TIM_TimeBaseInitStructure);
   
	
  TIM_TimeBaseInitStructure.TIM_Period        = 0xffff;   // 16λ����
   TIM_TimeBaseInitStructure.TIM_Prescaler     = 144-1;   // 144��Ƶ 2us
   TIM_TimeBaseInitStructure.TIM_ClockDivision = 0;        // ���ָ�
   TIM_TimeBaseInitStructure.TIM_CounterMode   = TIM_CounterMode_Up; // ��������
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter=0;  
	 
   TIM_TimeBaseInit( TIM1, &TIM_TimeBaseInitStructure );
   
  TIM_ICInitStructure.TIM_Channel     = TIM_Channel_1;   //����ͨ�� IC1
   TIM_ICInitStructure.TIM_ICPolarity  = TIM_ICPolarity_Rising;  // �����ش���
   TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;   //
   TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;// ֱ��ӳ��
   TIM_ICInitStructure.TIM_ICFilter    = 0x00;  // ���˲�
   TIM_ICInit( TIM1, &TIM_ICInitStructure );
 
   TIM_Cmd( TIM1, ENABLE );
   TIM_ITConfig( TIM1,TIM_IT_CC1|TIM_IT_Update, ENABLE );//ʹ�ܸ����жϺͲ����ж�
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

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);//ʹ��TIM2��ʱ��    
  TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;//TIM_CKD_DIV1��.h�ļ����Ѿ�����õģ�TIM_CKD_DIV1=0��Ҳ����ʱ�ӷ�Ƶ����Ϊ0    
  TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;//������ʽΪ���ϼ��� 
  TIM_TimeBaseStructure.TIM_Period=per;//����    
  TIM_TimeBaseStructure.TIM_Prescaler=pre;//��Ƶϵ��  
  TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);  
  TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);//ʹ��TIM2�ĸ����ж� 
  TIM_Cmd(TIM2,ENABLE);//ʹ��TIM2
}



void time3_init(u16 per,u16 pre)
{  
  GPIO_InitTypeDef GPIO_InitStructure;
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;  
  TIM_ICInitTypeDef TIM_ICInitStructure;   	

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);//ʹ��TIM3��ʱ��    


    //PA5,PA6,PA7, 
   GPIO_InitStructure.GPIO_Pin =(GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7);
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //������������
   GPIO_Init(GPIOA, &GPIO_InitStructure);     

  
  TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;//TIM_CKD_DIV1��.h�ļ����Ѿ�����õģ�TIM_CKD_DIV1=0��Ҳ����ʱ�ӷ�Ƶ����Ϊ0    
  TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;//������ʽΪ���ϼ��� 
  TIM_TimeBaseStructure.TIM_Period=per;//����    
  TIM_TimeBaseStructure.TIM_Prescaler=pre;//��Ƶϵ��  
  TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);  

 TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_BothEdge ,TIM_ICPolarity_BothEdge);// ʹ�ñ�����ģʽ3���������½��ض�����
 TIM_ICStructInit(&TIM_ICInitStructure);// ���ṹ���е�����ȱʡ����
 TIM_ICInitStructure.TIM_ICFilter = 6; // ѡ������Ƚ��˲���
 TIM_ICInit(TIM3, &TIM_ICInitStructure);  // ����ṹ��


  TIM_ClearFlag(TIM3, TIM_FLAG_Update);//������±�־	
  TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);//ʹ��TIM3�ĸ����ж� 

  TIM3->CNT = 30000;// clr the cnt
  
 TIM_Cmd(TIM3,ENABLE);//ʹ��TIM3

  
}



void TIM3_NVIC_INIT(void)
{   

  NVIC_InitTypeDef NVIC_InitStructure; 

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);//�趨�ж����ȼ�����0   
  NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn;//�趨�ж�����   ������ΪTIM3���ж�   
  NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;//ʹ����Ӧ�ж��������ж���Ӧ   
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//�����ж���������ռ���ȼ�   
  NVIC_InitStructure.NVIC_IRQChannelSubPriority=3;//�����ж���������Ӧ���ȼ�    
  NVIC_Init(&NVIC_InitStructure);//����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���

}



//TIM4 PWM���ֳ�ʼ�� 
//PWM�����ʼ��
//arr���Զ���װֵ
//psc��ʱ��Ԥ��Ƶ��
void TIM4_PWM_Init(u16 arr,u16 psc)
{  
      GPIO_InitTypeDef GPIO_InitStructure;
      TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
      TIM_OCInitTypeDef  TIM_OCInitStructure;
                                                    
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);	//ʹ�ܶ�ʱ��4ʱ��
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB  | RCC_APB2Periph_AFIO, ENABLE);  //ʹ��GPIO�����AFIO���ù���ģ��ʱ��
  
    //  GPIO_PinRemapConfig(GPIO_Remap_TIM4, ENABLE); //Timer4������ӳ��  TIM4_CH1->PB6 
   
    //���ø�����Ϊ�����������,���TIM4 CH1��PWM���岨��	GPIOB.6
     // GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; //TIM_CH2
     // GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //������� ���ù��� 
     // GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    //  GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIO

       // ����Ϊ��ͨ�����
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; //TIM4_CH1
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   //����Ϊ��ͨ�����
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
      GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIO
	  
 
	   //��ʼ��TIM4
      TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
      TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ 
      TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
      TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
      TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

      //��ʼ��TIM4 Channel1 PWMģʽ	 
    //  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //ѡ��ʱ��ģʽ: 
    //  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
    //  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //�������:TIM����Ƚϼ��Ը�
  
      TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing; //ѡ��ʱ��ģʽ: ����ģʽ
      TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
      TIM_OCInitStructure.TIM_Pulse = 10;
      TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

	  TIM_OC1Init(TIM4, &TIM_OCInitStructure);  //����Tָ���Ĳ�����ʼ������TIM4 OC1
      TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);  //ʹ��TIM4��CCR1�ϵ�Ԥװ�ؼĴ���

    //  TIM_SelectOnePulseMode(TIM4,TIM_OPMode_Single);  // ������ģʽ

	  TIM_ClearFlag(TIM4, TIM_FLAG_Update);
      TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);//ʹ��TIM4�ĸ����ж� 
 
     //  TIM_Cmd(TIM4, ENABLE);  //ʹ��TIM3
 }

void TIM4_NVIC_INIT(void)
{   

  NVIC_InitTypeDef NVIC_InitStructure; 

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);//�趨�ж����ȼ�����0   
  NVIC_InitStructure.NVIC_IRQChannel=TIM4_IRQn;//�趨�ж�����   ������ΪTIM4���ж�   
  NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;//ʹ����Ӧ�ж��������ж���Ӧ   
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//�����ж���������ռ���ȼ�   
  NVIC_InitStructure.NVIC_IRQChannelSubPriority=3;//�����ж���������Ӧ���ȼ�    
  NVIC_Init(&NVIC_InitStructure);//����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���

}


//TIM8 PWM���ֳ�ʼ�� 
//PWM�����ʼ��
//arr���Զ���װֵ
//psc��ʱ��Ԥ��Ƶ��
void TIM8_PWM_Init(u16 arr,u16 psc)
{  
      GPIO_InitTypeDef GPIO_InitStructure;
      TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
      TIM_OCInitTypeDef  TIM_OCInitStructure;

	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);										
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);	//ʹ�ܶ�ʱ��8ʱ��
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);  //ʹ��GPIO�����AFIO���ù���ģ��ʱ��
  
     // GPIO_PinRemapConfig(GPIO_Remap_TIM4, ENABLE); //Timer4������ӳ��  TIM4_CH1->PB6 
   
    //���ø�����Ϊ�����������,���TIM4 CH1��PWM���岨��	GPIOB.6
     // GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; //TIM_CH2
     // GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //������� ���ù��� 
     // GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    //  GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIO

       // ����Ϊ��ͨ�����
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; //TIM8_CH1 // PC6 
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   ////������� ���ù��� 
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
      GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIO
	  

           TIM_DeInit(TIM8);
	  TIM_OCStructInit(&TIM_OCInitStructure);
	  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	  

	   //��ʼ��TIM8
      TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
      TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ 
      TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
      TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
      TIM_TimeBaseStructure.TIM_RepetitionCounter=0;  
	  
      TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

      //��ʼ��TIM4 Channel1 PWMģʽ	 
    //  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //ѡ��ʱ��ģʽ: 
    //  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
    //  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //�������:TIM����Ƚϼ��Ը�


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

	  
  
      TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //ѡ��ʱ��ģʽ: ����ģʽ
      TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	  TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;  // ��ֹ����ͨ�����
      TIM_OCInitStructure.TIM_Pulse = 10;  //CCR, ����ռ�ձȣ�
      TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	  TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
	  TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset ;//TIM8�رտ���״̬(ʹ��TIM8ʱ����)
	  TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset ;//TIM8�رտ���״̬(ʹ��TIM8ʱ����)
	
	

	  TIM_OC1Init(TIM8, &TIM_OCInitStructure);  //����Tָ���Ĳ�����ʼ������TIM4 OC1
      TIM_OC1PreloadConfig(TIM8, TIM_OCPreload_Enable);  //ʹ��TIM4��CCR1�ϵ�Ԥװ�ؼĴ���
      TIM_ARRPreloadConfig(TIM8, ENABLE); //ʹ��TIMx��ARR�ϵ�Ԥװ�ؼĴ���


	 // TIM_ClearFlag(TIM8, TIM_FLAG_Update);
	 
      TIM_Cmd(TIM8, ENABLE);  //ʹ��TIM8
	  TIM_CtrlPWMOutputs(TIM8,ENABLE); // MOE �����ʹ��
	 
 
    
 }


 #define PWM_FREQENCY  4000
 
 static void timer8_PWM_Init(void)
{
     GPIO_InitTypeDef GPIO_InitStructure; 
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);	//ʹ�ܶ�ʱ��8ʱ��
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC |RCC_APB2Periph_AFIO, ENABLE);  //ʹ��GPIO�����AFIO���ù���ģ��ʱ��

	      // ����Ϊ��ͨ�����
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; //TIM8_CH1 // PC6 
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   ////������� ���ù��� 
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
      GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIO
      
    
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

	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable; //�رջ���ͨ�� 
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = (90 * PWM_FREQENCY )/100;          
    TIM_OC1Init(TIM8, &TIM_OCInitStructure);                      
    TIM_OC1PreloadConfig(TIM8, TIM_OCPreload_Enable);
    
    TIM_ARRPreloadConfig(TIM8, ENABLE);                            

    /* TIM8 enable counter */
    TIM_Cmd(TIM8, ENABLE);               
    TIM_CtrlPWMOutputs(TIM8, ENABLE);   
}


// �����ⲿ�ж�
void EXTILine_Config(void)
{

  // ���� F407�����÷�ʽ
 //  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);  //  ����F407�����÷�ʽ
  // SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, AxisEXTIPinSource[0]);  // ����407�����÷�ʽ
  
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE); //����AFIOʱ��	

    // PA0,PA1,PA8

    //PA0,PA1 as	exti   port 
   // GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0|GPIO_PinSource1);  //������ǰ��F103���õ����ú���
   GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);  //������ǰ��F103���õ����ú���
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);  //������ǰ��F103���õ����ú���


   //PA8 as  exti   port 
   GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource8); 

   // PB15 
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource15); 
 


   // �ȹر������ء��½��ؼ��
   
    EXTI->RTSR &= ~EXTI_Line0; // �ر������ؼ��
    EXTI->FTSR &= ~EXTI_Line0; // �ر��½��ؼ��

    EXTI->RTSR &= ~EXTI_Line1; // �ر������ؼ��
    EXTI->FTSR &= ~EXTI_Line1; // �ر��½��ؼ��
    
    EXTI->RTSR &= ~EXTI_Line8; // �ر������ؼ��
    EXTI->FTSR &= ~EXTI_Line8; // �ر��½��ؼ��

	
    
    EXTI->RTSR &= ~EXTI_Line15; // �ر������ؼ��
    EXTI->FTSR &= ~EXTI_Line15; // �ر��½��ؼ��


    EXTI_StructInit(&EXTI_InitStructure); 
	 

 // Ƶ�ʼ��ʹ���ж�
 // ����Ϊ�����½����ж�ģʽ
 //���жϳ�������Ҫ���ݳ����������������������
 
  EXTI_InitStructure.EXTI_Line = EXTI_Line0;// �����ж���
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  //   EXTI_Trigger_Rising
  EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
  EXTI_Init(&EXTI_InitStructure);	


  EXTI_InitStructure.EXTI_Line = EXTI_Line1;// �����ж���
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  //   EXTI_Trigger_Rising
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);	


  EXTI_InitStructure.EXTI_Line = EXTI_Line8;// �����ж���
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  //   EXTI_Trigger_Rising
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);	

   EXTI_InitStructure.EXTI_Line = EXTI_Line15;// �����ж���
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  //   EXTI_Trigger_Rising
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);	


  
}


void NVIC_Configuration(void)
{


   NVIC_InitTypeDef NVIC_InitStructure;

   // TIM2 
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);//�趨�ж����ȼ�����0   
   NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn;//�趨�ж�����   ������ΪTIM3���ж�   
   NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;//ʹ����Ӧ�ж��������ж���Ӧ   
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//�����ж���������ռ���ȼ�   
   NVIC_InitStructure.NVIC_IRQChannelSubPriority=4;//�����ж���������Ӧ���ȼ�    
   NVIC_Init(&NVIC_InitStructure);//����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���

   // TIM3 
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);//�趨�ж����ȼ�����0   
   NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn;//�趨�ж�����   ������ΪTIM3���ж�   
   NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;//ʹ����Ӧ�ж��������ж���Ӧ   
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//�����ж���������ռ���ȼ�   
   NVIC_InitStructure.NVIC_IRQChannelSubPriority=5;//�����ж���������Ӧ���ȼ�    
   NVIC_Init(&NVIC_InitStructure);//����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���

  // TIM4 
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);//�趨�ж����ȼ�����0   
  NVIC_InitStructure.NVIC_IRQChannel=TIM4_IRQn;//�趨�ж�����   ������ΪTIM4���ж�   
  NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;//ʹ����Ӧ�ж��������ж���Ӧ   
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//�����ж���������ռ���ȼ�   
  NVIC_InitStructure.NVIC_IRQChannelSubPriority=6;//�����ж���������Ӧ���ȼ�    
  NVIC_Init(&NVIC_InitStructure);//����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���

 

  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);//�趨�ж����ȼ�����1   
  // EXIT0   �ⲿ�ж���
  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // ��ռ���ȼ�
   NVIC_InitStructure.NVIC_IRQChannelSubPriority=7;//��Ӧ���ȼ�
  NVIC_Init(&NVIC_InitStructure);
  // EXIT1   �ⲿ�ж���
  NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority=8;//��Ӧ���ȼ�
  NVIC_Init(&NVIC_InitStructure);
 // EXIT9-5   �ⲿ�ж���
  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority=9;//��Ӧ���ȼ�
  NVIC_Init(&NVIC_InitStructure);

// EXIT15-10	 �ⲿ�ж���
 NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority=10;//��Ӧ���ȼ�
  NVIC_Init(&NVIC_InitStructure);

 
	
}


void SetEXTI_Trigger_Falling(void)
{

  EXTI_InitStructure.EXTI_Line = EXTI_Line8|EXTI_Line9;// �����ж���
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  //   EXTI_Trigger_Rising
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);	
}



void SetEXTI_Trigger_Rising(void)
{

  EXTI_InitStructure.EXTI_Line = EXTI_Line8|EXTI_Line9;// �����ж���
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //   EXTI_Trigger_Rising
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);	
}


u16 IR_ThisPulse=0; 
u16 IR_LastPulse=0; 
u16 IR_PulseSub=0; 

 
  // 
 void TIM1_CC_IRQHandler(void) //TIME2�жϷ�����	��Ҫ�趨�ж����ȼ�	��NVIC����
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
void TIM1_UP_IRQHandler(void) //TIME2�жϷ�����  ��Ҫ�趨�ж����ȼ�  ��NVIC����
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
void TIM2_IRQHandler(void) //TIME2�жϷ�����  ��Ҫ�趨�ж����ȼ�  ��NVIC����
{  

   if(TIM_GetITStatus(TIM2,TIM_IT_Update)!=RESET)//�ж��Ƿ����˸���(���)�ж�   
   {      
   TIM_ClearITPendingBit(TIM2,TIM_IT_Update);//�������жϱ�־λ    
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

void TIM3_IRQHandler(void) //TIME3�жϷ�����  ��Ҫ�趨�ж����ȼ�  ��NVIC����
{  

  if(TIM_GetITStatus(TIM3,TIM_IT_Update)!=RESET)//�ж��Ƿ����˸���(���)�ж�   
  {      
   TIM_ClearITPendingBit(TIM3,TIM_IT_Update);//�������жϱ�־λ    
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
//�ļ����ƣ�main
//���ܸ�Ҫ��������
//����˵������
//�������أ�int
//=============================================================================
int main(void)
{
       
           RCC_Configuration();  
	   GPIO_Configuration();
	   
           SMEC_I2cInit(); 
	   IIC_SetSdaPin(); IIC_SetSclPin();
	   
	      
	    Usart1_Init(19200);    //���ڲ���������  19200   // 485 PORT       //
        Usart2_Init(9600);    //���ڲ���������9600, for touch screen 
        Usart3_Init(19200);   // rs485,for ele-load   2024.12.29 
	  //  Usart4_Init(19200);   // rs485, for ele-load   ---old ,new  is null

             time2_init(1000,71);  

	   // 72/72=1M,   TCY=1US     20000*1US=20MS 
        //  time3_init(1000,71);  //������65535��Ԥ��Ƶ0��
  

	  // TIM4_PWM_Init(509,8);  // psc: = 8+1=9     SET the time4's driver frequency as 8MHZ( 72/(8+1))        
	   //TIM_SetCompare1(TIM4,200);


       //TIM8 PWM1 config// PC6, TIM8-CH1
	   TIM8_PWM_Init(1000,71);
	   TIM_SetAutoreload(TIM8, 1000); // �ı�ARR, �ı�Ƶ��
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
//�ļ����ƣ�GPIO_Configuration
//���ܸ�Ҫ��GPIO��ʼ��
//����˵������
//�������أ���
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
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // �ڲ���������ģʽ 
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
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // �ڲ���������ģʽ 
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
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // �ڲ���������ģʽ 
   GPIO_Init(GPIOA, &GPIO_InitStructure);     

  // PARTA ANALOG  DAC 
  // PA4 
   GPIO_InitStructure.GPIO_Pin =GPIO_Pin_4;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;  // 
   GPIO_Init(GPIOA, &GPIO_InitStructure);  
  
 // RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);//ʹ�ܶ˿ڸ���ʱ�� 
 // GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);//ʧ��JTAG 

   LED1=1;
   LED2=1; 

   LED2=1;
 
   Y0=1;Y1=1;Y2=1;Y3=1;
   WP=0; 
   
 

  
}



