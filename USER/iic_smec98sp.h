#ifndef __IIC_H
#define __IIC_H



#define IIC_NOACK         TRUE
#define IIC_ACK           FALSE
#define HIGHT             TRUE
#define LOW               FALSE

#define IIC_DELAYTIME     2			 //50
#define ACKCHECKTIME      2000


extern void IIC_Start(void);
extern void IIC_Stop(void);

extern bool IIC_Send(unsigned char ucVal);
extern unsigned char IIC_Read(bool bACK);




void SMEC_I2cInit(void);
bool IIC_ReadWithAddr(unsigned char addr, unsigned char *buf, unsigned char len);
bool IIC_WriteWithAddr(unsigned char addr, unsigned char *buf, unsigned char len);

#endif	   
