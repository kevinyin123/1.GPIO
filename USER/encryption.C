#include "include.h"


 
#include <stdlib.h>


// 加入加密处理头文件
#include "smec98sp.h"
#include "iic_smec98sp.h"



/*
  1.获取SMEC98SP的UID号, 获取STM32的ID, 获取STM32随机数
  2.验证PIN
  3.内外部认证
  4.SHA1=>前置数据^随机数
  5.密文读
  6.读数据
  7.写数据
  8.构造算法(PA口数据->密文送加密芯片, 密文返回)

  如果直接引用,请将print的调试信息去除
*/

void SMEC_Encrypt(void)
{

    

	/*各种密钥,不会在I2C线路上传输,可以使用同一组.应该将密钥分散存储,防止主控芯片被破解后,被攻击者在二进制码中找到密钥 */
	unsigned char InternalKey[16] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};//内部认证密钥,必须和SMEC98SP一致
	unsigned char ExternalKey[16] = {0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F};//外部认证密钥,必须和SMEC98SP一致
	unsigned char SHA1_Key[16] = {0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F};   //哈希算法认证密钥,必须和SMEC98SP一致
	unsigned char MKey[16] = {0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F};   //主控密钥,用于产生过程密钥,必须和SMEC98SP一致

	unsigned char Pin[8] = {0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc};			//Pin认证密钥,必须和SMEC98SP一致
		
	unsigned char bStm32Uid[12] = {0};			//存放STM32的UID
	unsigned char bSmec98spUid[12] = {0};		//存放SMEC98SP的UID
	unsigned short RandomSeek;					//随机数种子
	unsigned char bRandom[8] = {0};				//存放随机数
	unsigned char bSessionKey[8] = {0};			//存放过程密钥,过程密钥为临时产生的密钥
	unsigned char bDataBuf[64] = {0};
	unsigned char ret, bLen;
	unsigned short i, j;

    static  unsigned char dlytmp=10; 



    if(dlytmp>0){dlytmp--;return;}
	dlytmp=10;   //100ms 


    // 调用加密芯片内部计算档位的程序
     //解码档位开关
            // 111110      class0  3E
            // 111101      class1  3D
            // 111011      class2  3B
            // 110111      class3  37
            // 101111      class4  2F
            // 011111      class5  1F
  
	//bDataBuf[0] = DC_VOL_CLASS;  // class5  
	
	ret = SMEC_CircleAlg(bDataBuf, 1, bDataBuf, &bLen);
	if(ret) // error  
	{
	   // while(1);  //  error     died  
	  //  DC_CLASS=0;   // error  
	    return;  
	}
	
	//DC_CLASS=  bDataBuf[0];  // get the result  


}




