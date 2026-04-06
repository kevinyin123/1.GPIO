


//eeprom
#define WP           PCout(1)    




#define UART1_485_RD  PAout(11)  //PA11 
#define UART3_485_RD  PAout(7)  //PA7
#define UART4_485_RD  PAout(15)  //PA15



// 
#define  LED1      PBout(1)
#define  LED2      PAout(12) 

//
#define  CTR_5V    PBout(9)

//-----------------------------------------------



//X0-X7
#define X0  PAin(8) 
#define X1  PCin(9) 
#define X2  PCin(8) 
#define X3  PCin(7) 
#define X4  PBin(15) 
#define X5  PBin(14) 
#define X6  PBin(13)
#define X7  PBin(12) 

//Y0-Y3
#define Y0  PBout(6)
#define Y1  PBout(7)
#define Y2  PCout(12)
#define Y3  PDout(2)


//key  
#define KEY1     X1    
#define KEY2     X2   
#define KEY3     X3   
//--------------------------

#define KEY4     X4   
#define KEY5     X5  


//Y0-Y11 端口全部都要取反，因为加了一个74HC14反向器

#define MeasSensPIN X0 

#define  FB_CH1_PIN        PAin(0) 
#define  FB_CH2_PIN        PAin(1) 
#define  Coder_PIN           PAin(8)


// motor  
#define PUL           Y0
#define DIR           Y1  
#define SRV_ON        Y2
#define A_CLR         Y3


//-----------------------------------------

// BELL
#define   Bell          Y1


