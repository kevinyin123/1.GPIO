
#include <string.h>
#include "stm32f10x.h"
#include "conf.h" 
#include "iic_smec98sp.h"			 
#include "hmi_lcd.h"
#include "param.h"

#include "modbus.h"




#define EEP_ADDR    0XA0 

int readdat=0; 


unsigned char ControlByte;		//Control Byte
unsigned char HighAdd;			//High Address byte
unsigned char LowAdd;	//Low and High Address byte



unsigned int eeByteWriteI2C(unsigned char ControlByte, unsigned char LowAdd, unsigned char data)
{   
   
    bool ret; 
	
 	IIC_Start(); 				//Generate Start Condition
	ret=IIC_Send(ControlByte); //Write Control Byte
	ret=IIC_Send(LowAdd);		//Write start address 
	ret=IIC_Send(data);		//Write start address 
	IIC_Stop();			//Generate Stop;

    return (ret); 
}


unsigned int eeByteReadI2C(unsigned char ControlByte, unsigned char Address, unsigned char *Data, unsigned char Length)
{

    bool ret; 
	

	IIC_Start(); 	//Generate Start Condition 
	ret=IIC_Send(ControlByte);//Write Control Byte
	ret=IIC_Send(Address);	  //Write start address
	DelayMS(1000); 
	IIC_Start();				//Generate restart condition
	ret=IIC_Send(ControlByte | 0x01);	//Write control byte for read
    *Data= IIC_Read(IIC_NOACK);  
    IIC_Stop();

	return (ret); 
	
}






void IniEEprom(void)
{

	unsigned char CheckHi=0;
	unsigned char CheckLo=0;

	
     ControlByte = 0xA0;
     LowAdd = 0x00;	
     HighAdd = 0x00;


     eeByteReadI2C(ControlByte,LowAdd+6,&CheckHi,1);
     eeByteReadI2C(ControlByte,LowAdd+7,&CheckLo,1);

	 if((CheckHi==0x55)&&(CheckLo==0xaa)) // initialed   
	 	{
           eeByteReadI2C(ControlByte,LowAdd,&NP_Mode,1);  
		   eeByteReadI2C(ControlByte,LowAdd+1,&PowerSupplyMode,1);   
		   eeByteReadI2C(ControlByte,LowAdd+2,&TestPlateType,1);   
	 	}
	 else
	 	{
	 	   
	 	   NP_Mode=0;
		   PowerSupplyMode=0; 
		   TestPlateType=0;

		   eeByteWriteI2C(ControlByte,LowAdd,NP_Mode);             DelayMS(100);
		   eeByteWriteI2C(ControlByte,LowAdd+1,PowerSupplyMode);   DelayMS(100);
		   eeByteWriteI2C(ControlByte,LowAdd+2,TestPlateType);     DelayMS(100);

		   
		   eeByteWriteI2C(ControlByte,LowAdd+6,0x55);     DelayMS(100);
		   eeByteWriteI2C(ControlByte,LowAdd+7,0xaa);     DelayMS(100);
   
	 	}

	 NP_Mode&=0x03;
	 PowerSupplyMode&=0x01;
	 TestPlateType&=0x0F;  

	 usRegHoldingBuf[1]=NP_Mode;
	 usRegHoldingBuf[2]=PowerSupplyMode;
	 usRegHoldingBuf[3]=TestPlateType;


	 
	 

}


void SaveEEprom(void)
{
  
  
     ControlByte = 0xA0;
     LowAdd = 0x00;	
     HighAdd = 0x00; 		

	eeByteWriteI2C(ControlByte,LowAdd,NP_Mode);             DelayMS(100);
	eeByteWriteI2C(ControlByte,LowAdd+1,PowerSupplyMode);   DelayMS(100);
	eeByteWriteI2C(ControlByte,LowAdd+2,TestPlateType);     DelayMS(100);
	 
	 
}




//-------------------------------------------------------------------------------------------
//上电的时候先用临时变量将EEPROM里面的数据暂存下来
//上电10秒后等触摸屏往下发过数据后再将EEPROM里面的数据调出来
//-------------------------------------------------------------------------------------------


void EEprom_Manage(void)
{

 ; 
}




