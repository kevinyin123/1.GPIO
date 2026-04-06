#include "include.h"

#include "modbus.h"


//===============================
// 单片机Modbus RTU Slave 程序
// 开 发  人:  印国平
// 联系方 式: QQ839407837 
// 公   司 :  苏州拓邦米特电子科技有限公司
//==============================================

 
 // bandrate: 9600 bps 
 // start bit:  1 
 // len:    8 
 // prity:   even
 // stop bit : 1  
 // 物理层：       RS485 
 // 从机地址:      0X01 
 // 使用串口：  STM32  UART2  
 

 /* ----------------------- Defines ------------------------------------------*/
#define REG_INPUT_START   0
#define REG_INPUT_NREGS   128     // 输入寄存器
#define REG_HOLDING_START 0
#define REG_HOLDING_NREGS 128     //保持寄存器
 
 /* ----------------------- Static variables ---------------------------------*/
  USHORT   usRegInputStart = REG_INPUT_START;
  USHORT   usRegInputBuf[REG_INPUT_NREGS]=
  {
	1,2,3,4,5,6,7,8,9,10,
	11,12,13,14,15,16,17,18,19,20,
	21,22,23,24,25,26,27,28,29,30,
	31,32,33,34,35,36,37,38,39,40
  };
  USHORT   usRegHoldingStart = REG_HOLDING_START;
  USHORT   usRegHoldingBuf[REG_HOLDING_NREGS]=
  {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25}; 



 /* ----------------------- Static variables ---------------------------------*/
 
 UCHAR	  ucMBAddress;// slaver address 
 eMBMode  eMBCurrentMode; //  
 //-----------------------------------------------------------------------------

 
 volatile eMBSndState eSndState;
 volatile eMBRcvState eRcvState;
 
 volatile UCHAR  ucRTUBuf[MB_SER_PDU_SIZE_MAX];
 
 volatile UCHAR *pucSndBufferCur;
 volatile USHORT usSndBufferCount;
 
 volatile USHORT usRcvBufferPos;
 
 //----------------------------------------------------------------------------
 
 /* ----------------------- Variables ----------------------------------------*/
 static eMBEventType eQueuedEvent;
 static BOOL	 xEventInQueue;
 //-----------------------------------------------------------------------------
 
 UCHAR RTUTimerT35ExpiredEn;  


//------------------------------------------------------------------------------------
//自定义参数
 USHORT  SampleIndex=1; 
 u8      TempSampleIndex=0; //  tempreture sample point   initial as  -20degree  
 u8      ChannelSampleEn=0; //  channel sample enable   flag 

 u16  DisMesureManual=0;  // 手动检测距离
 u16  HysMesureManual=0;  // 手动检测距离
 
//---------------------------------------------------------------------------------


/* An array of Modbus functions handlers which associates Modbus function
 * codes with implementing functions.
 */
static xMBFunctionHandler xFuncHandlers[MB_FUNC_HANDLERS_MAX] = {
#if MB_FUNC_OTHER_REP_SLAVEID_ENABLED > 0
    {MB_FUNC_OTHER_REPORT_SLAVEID, eMBFuncReportSlaveID}, // funcode=17 
#endif
#if MB_FUNC_READ_INPUT_ENABLED > 0
    {MB_FUNC_READ_INPUT_REGISTER, eMBFuncReadInputRegister}, // funcode=4 
#endif
#if MB_FUNC_READ_HOLDING_ENABLED > 0
    {MB_FUNC_READ_HOLDING_REGISTER, eMBFuncReadHoldingRegister},// funcode=3 
#endif
#if MB_FUNC_WRITE_MULTIPLE_HOLDING_ENABLED > 0
    {MB_FUNC_WRITE_MULTIPLE_REGISTERS, eMBFuncWriteMultipleHoldingRegister},// funcode=16 
#endif
#if MB_FUNC_WRITE_HOLDING_ENABLED > 0
    {MB_FUNC_WRITE_REGISTER, eMBFuncWriteHoldingRegister},// funcode=6 
#endif
#if MB_FUNC_READWRITE_HOLDING_ENABLED > 0
    {MB_FUNC_READWRITE_MULTIPLE_REGISTERS, eMBFuncReadWriteMultipleHoldingRegister},// funcode=23 
#endif
#if MB_FUNC_READ_COILS_ENABLED > 0
    {MB_FUNC_READ_COILS, eMBFuncReadCoils},// funcode=1 
#endif
#if MB_FUNC_WRITE_COIL_ENABLED > 0
    {MB_FUNC_WRITE_SINGLE_COIL, eMBFuncWriteCoil},// funcode=5 
#endif
#if MB_FUNC_WRITE_MULTIPLE_COILS_ENABLED > 0
    {MB_FUNC_WRITE_MULTIPLE_COILS, eMBFuncWriteMultipleCoils},// funcode=15 
#endif
#if MB_FUNC_READ_DISCRETE_INPUTS_ENABLED > 0
    {MB_FUNC_READ_DISCRETE_INPUTS, eMBFuncReadDiscreteInputs},// funcode=2 
#endif
};



//----------------------------------------------------------------------------------
//--- crc16----
//----------------------------------------------------------------------------------
static const UCHAR aucCRCHi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40
};

static const UCHAR aucCRCLo[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7,
    0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
    0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9,
    0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
    0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
    0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D,
    0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 
    0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF,
    0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1,
    0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
    0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 
    0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
    0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
    0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97,
    0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
    0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89,
    0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83,
    0x41, 0x81, 0x80, 0x40
};

USHORT
usMBCRC16( UCHAR * pucFrame, USHORT usLen )
{
    UCHAR           ucCRCHi = 0xFF;
    UCHAR           ucCRCLo = 0xFF;
    int             iIndex;

    while( usLen-- )
    {
        iIndex = ucCRCLo ^ *( pucFrame++ );
        ucCRCLo = ( UCHAR )( ucCRCHi ^ aucCRCHi[iIndex] );
        ucCRCHi = aucCRCLo[iIndex];
    }
    return ( USHORT )( ucCRCHi << 8 | ucCRCLo );
}

//----------end of the crc16--------




/*

void
EnterCriticalSection( void )
{
  __disable_irq();
}

void
ExitCriticalSection( void )
{
  __enable_irq();
}

*/

 
 
 
 eMBErrorCode
 eMBInit( eMBMode eMode, UCHAR ucSlaveAddress, UCHAR ucPort, ULONG ulBaudRate, eMBParity eParity )
 {
	  eMBErrorCode	  eStatus = MB_ENOERR;
	  ucMBAddress = ucSlaveAddress;
	  eMBCurrentMode = eMode;
	  eMBState = STATE_ENABLED;
	  RTUTimerT35ExpiredEn=0;
	  
	  return eStatus;
 }
 
 
 
 eMBException
 prveMBError2Exception( eMBErrorCode eErrorCode )
 {
	 eMBException	 eStatus;
 
	 switch ( eErrorCode )
	 {
		 case MB_ENOERR:
			 eStatus = MB_EX_NONE;
			 break;
 
		 case MB_ENOREG:
			 eStatus = MB_EX_ILLEGAL_DATA_ADDRESS;
			 break;
 
		 case MB_ETIMEDOUT:
			 eStatus = MB_EX_SLAVE_BUSY;
			 break;
 
		 default:
			 eStatus = MB_EX_SLAVE_DEVICE_FAILURE;
			 break;
	 }
 
	 return eStatus;
 }
 
 //------------------------------------------------------------------------------------
#if MB_FUNC_OTHER_REP_SLAVEID_ENABLED > 0
 eMBErrorCode
 eMBSetSlaveID( UCHAR ucSlaveID, BOOL xIsRunning,
				UCHAR const *pucAdditional, USHORT usAdditionalLen )
 {
	;
 }
 
 eMBException
 eMBFuncReportSlaveID( UCHAR * pucFrame, USHORT * usLen )
 {
   ;
 }
#endif 
 
 
#if MB_FUNC_READ_INPUT_ENABLED > 0
 eMBException
 eMBFuncReadInputRegister( UCHAR * pucFrame, USHORT * usLen )
 {
	 USHORT 		 usRegAddress;
	 USHORT 		 usRegCount;
	 UCHAR			*pucFrameCur;
 
	 eMBException	 eStatus = MB_EX_NONE;
	 eMBErrorCode	 eRegStatus;
 
	 if( *usLen == ( MB_PDU_FUNC_READ_SIZE + MB_PDU_SIZE_MIN ) )
	 {
		 usRegAddress = ( USHORT )( pucFrame[MB_PDU_FUNC_READ_ADDR_OFF] << 8 );
		 usRegAddress |= ( USHORT )( pucFrame[MB_PDU_FUNC_READ_ADDR_OFF + 1] );
		 usRegAddress++;
 
		 usRegCount = ( USHORT )( pucFrame[MB_PDU_FUNC_READ_REGCNT_OFF] << 8 );
		 usRegCount |= ( USHORT )( pucFrame[MB_PDU_FUNC_READ_REGCNT_OFF + 1] );
 
		 /* Check if the number of registers to read is valid. If not
		  * return Modbus illegal data value exception. 
		  */
		 if( ( usRegCount >= 1 )
			 && ( usRegCount < MB_PDU_FUNC_READ_REGCNT_MAX ) )
		 {
			 /* Set the current PDU data pointer to the beginning. */
			 pucFrameCur = &pucFrame[MB_PDU_FUNC_OFF];
			 *usLen = MB_PDU_FUNC_OFF;
 
			 /* First byte contains the function code. */
			 *pucFrameCur++ = MB_FUNC_READ_INPUT_REGISTER;
			 *usLen += 1;
 
			 /* Second byte in the response contain the number of bytes. */
			 *pucFrameCur++ = ( UCHAR )( usRegCount * 2 );
			 *usLen += 1;
 
			 eRegStatus =eMBRegInputCB( pucFrameCur, usRegAddress, usRegCount );
 
			 /* If an error occured convert it into a Modbus exception. */
			 if( eRegStatus != MB_ENOERR )
			 {
				 eStatus = prveMBError2Exception( eRegStatus );
			 }
			 else
			 {
				 *usLen += usRegCount * 2;
			 }
		 }
		 else
		 {
			 eStatus = MB_EX_ILLEGAL_DATA_VALUE;
		 }
	 }
	 else
	 {
		 /* Can't be a valid read input register request because the length
		  * is incorrect. */
		 eStatus = MB_EX_ILLEGAL_DATA_VALUE;
	 }
	 return eStatus;
 }
 
#endif 
 
#if MB_FUNC_READ_HOLDING_ENABLED > 0
 
 eMBException
 eMBFuncReadHoldingRegister( UCHAR * pucFrame, USHORT * usLen )
 {
	 USHORT 		 usRegAddress;
	 USHORT 		 usRegCount;
	 UCHAR			*pucFrameCur;
 
	 eMBException	 eStatus = MB_EX_NONE;
	 eMBErrorCode	 eRegStatus;
 
	 if( *usLen == ( MB_PDU_FUNC_READ_SIZE + MB_PDU_SIZE_MIN ) )
	 {
		 usRegAddress = ( USHORT )( pucFrame[MB_PDU_FUNC_READ_ADDR_OFF] << 8 );
		 usRegAddress |= ( USHORT )( pucFrame[MB_PDU_FUNC_READ_ADDR_OFF + 1] );
		 usRegAddress++;
 
		 usRegCount = ( USHORT )( pucFrame[MB_PDU_FUNC_READ_REGCNT_OFF] << 8 );
		 usRegCount = ( USHORT )( pucFrame[MB_PDU_FUNC_READ_REGCNT_OFF + 1] );
 
		 /* Check if the number of registers to read is valid. If not
		  * return Modbus illegal data value exception. 
		  */
		 if( ( usRegCount >= 1 ) && ( usRegCount <= MB_PDU_FUNC_READ_REGCNT_MAX ) )
		 {
			 /* Set the current PDU data pointer to the beginning. */
			 pucFrameCur = &pucFrame[MB_PDU_FUNC_OFF];
			 *usLen = MB_PDU_FUNC_OFF;
 
			 /* First byte contains the function code. */
			 *pucFrameCur++ = MB_FUNC_READ_HOLDING_REGISTER;
			 *usLen += 1;
 
			 /* Second byte in the response contain the number of bytes. */
			 *pucFrameCur++ = ( UCHAR ) ( usRegCount * 2 );
			 *usLen += 1;
 
			 /* Make callback to fill the buffer. */
			 eRegStatus = eMBRegHoldingCB( pucFrameCur, usRegAddress, usRegCount, MB_REG_READ );
			 /* If an error occured convert it into a Modbus exception. */
			 if( eRegStatus != MB_ENOERR )
			 {
				 eStatus = prveMBError2Exception( eRegStatus );
			 }
			 else
			 {
				 *usLen += usRegCount * 2;
			 }
		 }
		 else
		 {
			 eStatus = MB_EX_ILLEGAL_DATA_VALUE;
		 }
	 }
	 else
	 {
		 /* Can't be a valid request because the length is incorrect. */
		 eStatus = MB_EX_ILLEGAL_DATA_VALUE;
	 }
	 return eStatus;
 }
 
#endif 
 
#if MB_FUNC_WRITE_MULTIPLE_HOLDING_ENABLED > 0
 eMBException
 eMBFuncWriteMultipleHoldingRegister( UCHAR * pucFrame, USHORT * usLen )
 {
	 USHORT 		 usRegAddress;
	 USHORT 		 usRegCount;
	 UCHAR			 ucRegByteCount;
 
	 eMBException	 eStatus = MB_EX_NONE;
	 eMBErrorCode	 eRegStatus;
 
	 if( *usLen >= ( MB_PDU_FUNC_WRITE_MUL_SIZE_MIN + MB_PDU_SIZE_MIN ) )
	 {
		 usRegAddress = ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_MUL_ADDR_OFF] << 8 );
		 usRegAddress |= ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_MUL_ADDR_OFF + 1] );
		 usRegAddress++;
 
		 usRegCount = ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_MUL_REGCNT_OFF] << 8 );
		 usRegCount |= ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_MUL_REGCNT_OFF + 1] );
 
		 ucRegByteCount = pucFrame[MB_PDU_FUNC_WRITE_MUL_BYTECNT_OFF];
 
		 if( ( usRegCount >= 1 ) &&
			 ( usRegCount <= MB_PDU_FUNC_WRITE_MUL_REGCNT_MAX ) &&
			 ( ucRegByteCount == ( UCHAR ) ( 2 * usRegCount ) ) )
		 {
			 /* Make callback to update the register values. */
			 eRegStatus =
				 eMBRegHoldingCB( &pucFrame[MB_PDU_FUNC_WRITE_MUL_VALUES_OFF],
								  usRegAddress, usRegCount, MB_REG_WRITE );
 
			 /* If an error occured convert it into a Modbus exception. */
			 if( eRegStatus != MB_ENOERR )
			 {
				 eStatus = prveMBError2Exception( eRegStatus );
			 }
			 else
			 {
				 /* The response contains the function code, the starting
				  * address and the quantity of registers. We reuse the
				  * old values in the buffer because they are still valid.
				  */
				 *usLen = MB_PDU_FUNC_WRITE_MUL_BYTECNT_OFF;
			 }
		 }
		 else
		 {
			 eStatus = MB_EX_ILLEGAL_DATA_VALUE;
		 }
	 }
	 else
	 {
		 /* Can't be a valid request because the length is incorrect. */
		 eStatus = MB_EX_ILLEGAL_DATA_VALUE;
	 }
	 return eStatus;
 }
#endif
 
#if MB_FUNC_WRITE_HOLDING_ENABLED > 0
 
 eMBException
 eMBFuncWriteHoldingRegister( UCHAR * pucFrame, USHORT * usLen )
 {
	 USHORT 		 usRegAddress;
	 eMBException	 eStatus = MB_EX_NONE;
	 eMBErrorCode	 eRegStatus;
 
	 if( *usLen == ( MB_PDU_FUNC_WRITE_SIZE + MB_PDU_SIZE_MIN ) )
	 {
		 usRegAddress = ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_ADDR_OFF] << 8 );
		 usRegAddress |= ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_ADDR_OFF + 1] );
		 usRegAddress++;
 
		 /* Make callback to update the value. */
		 eRegStatus = eMBRegHoldingCB( &pucFrame[MB_PDU_FUNC_WRITE_VALUE_OFF],
									   usRegAddress, 1, MB_REG_WRITE );
 
		 /* If an error occured convert it into a Modbus exception. */
		 if( eRegStatus != MB_ENOERR )
		 {
			 eStatus = prveMBError2Exception( eRegStatus );
		 }
	 }
	 else
	 {
		 /* Can't be a valid request because the length is incorrect. */
		 eStatus = MB_EX_ILLEGAL_DATA_VALUE;
	 }
	 return eStatus;
 }
#endif
 
 
#if MB_FUNC_READWRITE_HOLDING_ENABLED > 0
 
 eMBException
 eMBFuncReadWriteMultipleHoldingRegister( UCHAR * pucFrame, USHORT * usLen )
 {
	 USHORT 		 usRegReadAddress;
	 USHORT 		 usRegReadCount;
	 USHORT 		 usRegWriteAddress;
	 USHORT 		 usRegWriteCount;
	 UCHAR			 ucRegWriteByteCount;
	 UCHAR			*pucFrameCur;
 
	 eMBException	 eStatus = MB_EX_NONE;
	 eMBErrorCode	 eRegStatus;
 
	 if( *usLen >= ( MB_PDU_FUNC_READWRITE_SIZE_MIN + MB_PDU_SIZE_MIN ) )
	 {
		 usRegReadAddress = ( USHORT )( pucFrame[MB_PDU_FUNC_READWRITE_READ_ADDR_OFF] << 8U );
		 usRegReadAddress |= ( USHORT )( pucFrame[MB_PDU_FUNC_READWRITE_READ_ADDR_OFF + 1] );
		 usRegReadAddress++;
 
		 usRegReadCount = ( USHORT )( pucFrame[MB_PDU_FUNC_READWRITE_READ_REGCNT_OFF] << 8U );
		 usRegReadCount |= ( USHORT )( pucFrame[MB_PDU_FUNC_READWRITE_READ_REGCNT_OFF + 1] );
 
		 usRegWriteAddress = ( USHORT )( pucFrame[MB_PDU_FUNC_READWRITE_WRITE_ADDR_OFF] << 8U );
		 usRegWriteAddress |= ( USHORT )( pucFrame[MB_PDU_FUNC_READWRITE_WRITE_ADDR_OFF + 1] );
		 usRegWriteAddress++;
 
		 usRegWriteCount = ( USHORT )( pucFrame[MB_PDU_FUNC_READWRITE_WRITE_REGCNT_OFF] << 8U );
		 usRegWriteCount |= ( USHORT )( pucFrame[MB_PDU_FUNC_READWRITE_WRITE_REGCNT_OFF + 1] );
 
		 ucRegWriteByteCount = pucFrame[MB_PDU_FUNC_READWRITE_BYTECNT_OFF];
 
		 if( ( usRegReadCount >= 1 ) && ( usRegReadCount <= 0x7D ) &&
			 ( usRegWriteCount >= 1 ) && ( usRegWriteCount <= 0x79 ) &&
			 ( ( 2 * usRegWriteCount ) == ucRegWriteByteCount ) )
		 {
			 /* Make callback to update the register values. */
			 eRegStatus = eMBRegHoldingCB( &pucFrame[MB_PDU_FUNC_READWRITE_WRITE_VALUES_OFF],
										   usRegWriteAddress, usRegWriteCount, MB_REG_WRITE );
 
			 if( eRegStatus == MB_ENOERR )
			 {
				 /* Set the current PDU data pointer to the beginning. */
				 pucFrameCur = &pucFrame[MB_PDU_FUNC_OFF];
				 *usLen = MB_PDU_FUNC_OFF;
 
				 /* First byte contains the function code. */
				 *pucFrameCur++ = MB_FUNC_READWRITE_MULTIPLE_REGISTERS;
				 *usLen += 1;
 
				 /* Second byte in the response contain the number of bytes. */
				 *pucFrameCur++ = ( UCHAR ) ( usRegReadCount * 2 );
				 *usLen += 1;
 
				 /* Make the read callback. */
				 eRegStatus =
					 eMBRegHoldingCB( pucFrameCur, usRegReadAddress, usRegReadCount, MB_REG_READ );
				 if( eRegStatus == MB_ENOERR )
				 {
					 *usLen += 2 * usRegReadCount;
				 }
			 }
			 if( eRegStatus != MB_ENOERR )
			 {
				 eStatus = prveMBError2Exception( eRegStatus );
			 }
		 }
		 else
		 {
			 eStatus = MB_EX_ILLEGAL_DATA_VALUE;
		 }
	 }
	 return eStatus;
 }
 
#endif
 
 
 
#if MB_FUNC_READ_COILS_ENABLED > 0
 
 eMBException
 eMBFuncReadCoils( UCHAR * pucFrame, USHORT * usLen )
 {
	 USHORT 		 usRegAddress;
	 USHORT 		 usCoilCount;
	 UCHAR			 ucNBytes;
	 UCHAR			*pucFrameCur;
 
	 eMBException	 eStatus = MB_EX_NONE;
	 eMBErrorCode	 eRegStatus;
 
	 if( *usLen == ( MB_PDU_FUNC_READ_SIZE + MB_PDU_SIZE_MIN ) )
	 {
		 usRegAddress = ( USHORT )( pucFrame[MB_PDU_FUNC_READ_ADDR_OFF] << 8 );
		 usRegAddress |= ( USHORT )( pucFrame[MB_PDU_FUNC_READ_ADDR_OFF + 1] );
		 usRegAddress++;
 
		 usCoilCount = ( USHORT )( pucFrame[MB_PDU_FUNC_READ_COILCNT_OFF] << 8 );
		 usCoilCount |= ( USHORT )( pucFrame[MB_PDU_FUNC_READ_COILCNT_OFF + 1] );
 
		 /* Check if the number of registers to read is valid. If not
		  * return Modbus illegal data value exception. 
		  */
		 if( ( usCoilCount >= 1 ) &&
			 ( usCoilCount < MB_PDU_FUNC_READ_COILCNT_MAX ) )
		 {
			 /* Set the current PDU data pointer to the beginning. */
			 pucFrameCur = &pucFrame[MB_PDU_FUNC_OFF];
			 *usLen = MB_PDU_FUNC_OFF;
 
			 /* First byte contains the function code. */
			 *pucFrameCur++ = MB_FUNC_READ_COILS;
			 *usLen += 1;
 
			 /* Test if the quantity of coils is a multiple of 8. If not last
			  * byte is only partially field with unused coils set to zero. */
			 if( ( usCoilCount & 0x0007 ) != 0 )
			 {
				 ucNBytes = ( UCHAR )( usCoilCount / 8 + 1 );
			 }
			 else
			 {
				 ucNBytes = ( UCHAR )( usCoilCount / 8 );
			 }
			 *pucFrameCur++ = ucNBytes;
			 *usLen += 1;
 
			 eRegStatus =
				 eMBRegCoilsCB( pucFrameCur, usRegAddress, usCoilCount,
								MB_REG_READ );
 
			 /* If an error occured convert it into a Modbus exception. */
			 if( eRegStatus != MB_ENOERR )
			 {
				 eStatus = prveMBError2Exception( eRegStatus );
			 }
			 else
			 {
				 /* The response contains the function code, the starting address
				  * and the quantity of registers. We reuse the old values in the 
				  * buffer because they are still valid. */
				 *usLen += ucNBytes;;
			 }
		 }
		 else
		 {
			 eStatus = MB_EX_ILLEGAL_DATA_VALUE;
		 }
	 }
	 else
	 {
		 /* Can't be a valid read coil register request because the length
		  * is incorrect. */
		 eStatus = MB_EX_ILLEGAL_DATA_VALUE;
	 }
	 return eStatus;
 }
#endif    
 
 
#if MB_FUNC_WRITE_COIL_ENABLED > 0
 eMBException
 eMBFuncWriteCoil( UCHAR * pucFrame, USHORT * usLen )
 {
	 USHORT 		 usRegAddress;
	 UCHAR			 ucBuf[2];
 
	 eMBException	 eStatus = MB_EX_NONE;
	 eMBErrorCode	 eRegStatus;
 
	 if( *usLen == ( MB_PDU_FUNC_WRITE_SIZE + MB_PDU_SIZE_MIN ) )
	 {
		 usRegAddress = ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_ADDR_OFF] << 8 );
		 usRegAddress |= ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_ADDR_OFF + 1] );
		 usRegAddress++;
 
		 if( ( pucFrame[MB_PDU_FUNC_WRITE_VALUE_OFF + 1] == 0x00 ) &&
			 ( ( pucFrame[MB_PDU_FUNC_WRITE_VALUE_OFF] == 0xFF ) ||
			   ( pucFrame[MB_PDU_FUNC_WRITE_VALUE_OFF] == 0x00 ) ) )
		 {
			 ucBuf[1] = 0;
			 if( pucFrame[MB_PDU_FUNC_WRITE_VALUE_OFF] == 0xFF )
			 {
				 ucBuf[0] = 1;
			 }
			 else
			 {
				 ucBuf[0] = 0;
			 }
			 eRegStatus =
				 eMBRegCoilsCB( &ucBuf[0], usRegAddress, 1, MB_REG_WRITE );
 
			 /* If an error occured convert it into a Modbus exception. */
			 if( eRegStatus != MB_ENOERR )
			 {
				 eStatus = prveMBError2Exception( eRegStatus );
			 }
		 }
		 else
		 {
			 eStatus = MB_EX_ILLEGAL_DATA_VALUE;
		 }
	 }
	 else
	 {
		 /* Can't be a valid write coil register request because the length
		  * is incorrect. */
		 eStatus = MB_EX_ILLEGAL_DATA_VALUE;
	 }
	 return eStatus;
 }
 
#endif
 
 
#if MB_FUNC_WRITE_MULTIPLE_COILS_ENABLED > 0
 eMBException
 eMBFuncWriteMultipleCoils( UCHAR * pucFrame, USHORT * usLen )
 {
	 USHORT 		 usRegAddress;
	 USHORT 		 usCoilCnt;
	 UCHAR			 ucByteCount;
	 UCHAR			 ucByteCountVerify;
 
	 eMBException	 eStatus = MB_EX_NONE;
	 eMBErrorCode	 eRegStatus;
 
	 if( *usLen > ( MB_PDU_FUNC_WRITE_SIZE + MB_PDU_SIZE_MIN ) )
	 {
		 usRegAddress = ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_MUL_ADDR_OFF] << 8 );
		 usRegAddress |= ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_MUL_ADDR_OFF + 1] );
		 usRegAddress++;
 
		 usCoilCnt = ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_MUL_COILCNT_OFF] << 8 );
		 usCoilCnt |= ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_MUL_COILCNT_OFF + 1] );
 
		 ucByteCount = pucFrame[MB_PDU_FUNC_WRITE_MUL_BYTECNT_OFF];
 
		 /* Compute the number of expected bytes in the request. */
		 if( ( usCoilCnt & 0x0007 ) != 0 )
		 {
			 ucByteCountVerify = ( UCHAR )( usCoilCnt / 8 + 1 );
		 }
		 else
		 {
			 ucByteCountVerify = ( UCHAR )( usCoilCnt / 8 );
		 }
 
		 if( ( usCoilCnt >= 1 ) &&
			 ( usCoilCnt <= MB_PDU_FUNC_WRITE_MUL_COILCNT_MAX ) &&
			 ( ucByteCountVerify == ucByteCount ) )
		 {
			 eRegStatus =
				 eMBRegCoilsCB( &pucFrame[MB_PDU_FUNC_WRITE_MUL_VALUES_OFF],
								usRegAddress, usCoilCnt, MB_REG_WRITE );
 
			 /* If an error occured convert it into a Modbus exception. */
			 if( eRegStatus != MB_ENOERR )
			 {
				 eStatus = prveMBError2Exception( eRegStatus );
			 }
			 else
			 {
				 /* The response contains the function code, the starting address
				  * and the quantity of registers. We reuse the old values in the 
				  * buffer because they are still valid. */
				 *usLen = MB_PDU_FUNC_WRITE_MUL_BYTECNT_OFF;
			 }
		 }
		 else
		 {
			 eStatus = MB_EX_ILLEGAL_DATA_VALUE;
		 }
	 }
	 else
	 {
		 /* Can't be a valid write coil register request because the length
		  * is incorrect. */
		 eStatus = MB_EX_ILLEGAL_DATA_VALUE;
	 }
	 return eStatus;
 }
 
#endif
 
#if MB_FUNC_READ_COILS_ENABLED > 0
 
 eMBException
 eMBFuncReadDiscreteInputs( UCHAR * pucFrame, USHORT * usLen )
 {
	 USHORT 		 usRegAddress;
	 USHORT 		 usDiscreteCnt;
	 UCHAR			 ucNBytes;
	 UCHAR			*pucFrameCur;
 
	 eMBException	 eStatus = MB_EX_NONE;
	 eMBErrorCode	 eRegStatus;
 
	 if( *usLen == ( MB_PDU_FUNC_READ_SIZE + MB_PDU_SIZE_MIN ) )
	 {
		 usRegAddress = ( USHORT )( pucFrame[MB_PDU_FUNC_READ_ADDR_OFF] << 8 );
		 usRegAddress |= ( USHORT )( pucFrame[MB_PDU_FUNC_READ_ADDR_OFF + 1] );
		 usRegAddress++;
 
		 usDiscreteCnt = ( USHORT )( pucFrame[MB_PDU_FUNC_READ_DISCCNT_OFF] << 8 );
		 usDiscreteCnt |= ( USHORT )( pucFrame[MB_PDU_FUNC_READ_DISCCNT_OFF + 1] );
 
		 /* Check if the number of registers to read is valid. If not
		  * return Modbus illegal data value exception. 
		  */
		 if( ( usDiscreteCnt >= 1 ) &&
			 ( usDiscreteCnt < MB_PDU_FUNC_READ_DISCCNT_MAX ) )
		 {
			 /* Set the current PDU data pointer to the beginning. */
			 pucFrameCur = &pucFrame[MB_PDU_FUNC_OFF];
			 *usLen = MB_PDU_FUNC_OFF;
 
			 /* First byte contains the function code. */
			 *pucFrameCur++ = MB_FUNC_READ_DISCRETE_INPUTS;
			 *usLen += 1;
 
			 /* Test if the quantity of coils is a multiple of 8. If not last
			  * byte is only partially field with unused coils set to zero. */
			 if( ( usDiscreteCnt & 0x0007 ) != 0 )
			 {
				 ucNBytes = ( UCHAR ) ( usDiscreteCnt / 8 + 1 );
			 }
			 else
			 {
				 ucNBytes = ( UCHAR ) ( usDiscreteCnt / 8 );
			 }
			 *pucFrameCur++ = ucNBytes;
			 *usLen += 1;
 
			 eRegStatus =
				 eMBRegDiscreteCB( pucFrameCur, usRegAddress, usDiscreteCnt );
 
			 /* If an error occured convert it into a Modbus exception. */
			 if( eRegStatus != MB_ENOERR )
			 {
				 eStatus = prveMBError2Exception( eRegStatus );
			 }
			 else
			 {
				 /* The response contains the function code, the starting address
				  * and the quantity of registers. We reuse the old values in the 
				  * buffer because they are still valid. */
				 *usLen += ucNBytes;;
			 }
		 }
		 else
		 {
			 eStatus = MB_EX_ILLEGAL_DATA_VALUE;
		 }
	 }
	 else
	 {
		 /* Can't be a valid read coil register request because the length
		  * is incorrect. */
		 eStatus = MB_EX_ILLEGAL_DATA_VALUE;
	 }
	 return eStatus;
 }
 
#endif
 
 
 
 //-------------------------------------------------------------------------
 //CB: call back   
 //-------------------------------------------------------------------------
 
 eMBErrorCode
 eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
 {
	 eMBErrorCode	 eStatus = MB_ENOERR;
	 int			 iRegIndex;
 
	 if( ( usAddress >= REG_INPUT_START )
		 && ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
	 {
		 iRegIndex = ( int )( usAddress - usRegInputStart );
		 while( usNRegs > 0 )
		 {
			 *pucRegBuffer++ = ( unsigned char )( usRegInputBuf[iRegIndex] >> 8 );
			 *pucRegBuffer++ = ( unsigned char )( usRegInputBuf[iRegIndex] & 0xFF );
			 iRegIndex++;
			 usNRegs--;
		 }
	 }
	 else
	 {
		 eStatus = MB_ENOREG;
	 }
 
	 return eStatus;
 }
 
 eMBErrorCode
 eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
 {
	 eMBErrorCode	 eStatus = MB_ENOERR;
	 int			 iRegIndex;
 
	 if( ( usAddress >= REG_HOLDING_START ) &&
		 ( usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS ) )
	 {
		 iRegIndex = ( int )( usAddress - usRegHoldingStart );
		 switch ( eMode )
		 {
			 /* Pass current register values to the protocol stack. */
		 case MB_REG_READ:
			 while( usNRegs > 0 )
			 {
				 *pucRegBuffer++ = ( unsigned char )( usRegHoldingBuf[iRegIndex] >> 8 );
				 *pucRegBuffer++ = ( unsigned char )( usRegHoldingBuf[iRegIndex] & 0xFF );
				 iRegIndex++;
				 usNRegs--;
			 }
			 break;
 
			 /* Update current register values with new values from the
			  * protocol stack. */
		 case MB_REG_WRITE:
			 while( usNRegs > 0 )
			 {
				 usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
				 usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
				 iRegIndex++;
				 usNRegs--;
			 }
		 }
	 }
	 else
	 {
		 eStatus = MB_ENOREG;
	 }
	 return eStatus;
 }
 
 
 
 eMBErrorCode
 eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode )
 {
	 return MB_ENOREG;
 }
 
 eMBErrorCode
 eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
 {
	 return MB_ENOREG;
 }
 
 
 
 //----------------------------------------------------
 
 void
 EnterCriticalSection( void )
 {
   __disable_irq();
 }
 
 void
 ExitCriticalSection( void )
 {
   __enable_irq();
 }
 
 
 //------------------------------------------------------------------------
 // 使能OR除能串口接收发送中断
 // 需更具CPU进行改写 
 //--------------------------------------------------------------------------
 void
 vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
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
 
 //-------------------------------------------------------------------
 /* ----------------------- Start implementation -----------------------------*/
 BOOL
 xMBPortEventInit( void )
 {
	 xEventInQueue = FALSE;
	 return TRUE;
 }
 
 BOOL
 xMBPortEventPost( eMBEventType eEvent )
 {
	 xEventInQueue = TRUE;
	 eQueuedEvent = eEvent;
	 return TRUE;
 }
 
 BOOL
 xMBPortEventGet( eMBEventType * eEvent )
 {
	 BOOL			 xEventHappened = FALSE;
 
	 if( xEventInQueue )
	 {
		 *eEvent = eQueuedEvent;
		 xEventInQueue = FALSE;
		 xEventHappened = TRUE;
	 }
	 return xEventHappened;
 }
 
 
 
 //-----------------------------------------------------------------------------------------
 // ----RTU-------
 //------------------------------------------------------------------------------------------
 
 // 超时定时器启动
 void
 vMBPortTimersEnable( void )
 {
	// Reset timer counter 
	 RTUTimerT35ExpiredEn=1; 
	 USART2_RX_TIMEOUT=0; 
 }
 
 // 超时定时器关闭 
 void
 vMBPortTimersDisable( void )
 { 
	 RTUTimerT35ExpiredEn=0; 
 }
 
 
 
 void
 eMBRTUStart( void )
 {
	 ENTER_CRITICAL_SECTION(  );
 
	 eRcvState = STATE_RX_INIT;
	 vMBPortSerialEnable( TRUE, FALSE );
	 vMBPortTimersEnable(  );
 
	 EXIT_CRITICAL_SECTION(  );
 }
 
 
 //----------------------------------------------------------------
 // 判断数据的合法性包括长度，CRC校验是否过关
 //----------------------------------------------------------------
 eMBErrorCode
 eMBRTUReceive( UCHAR * pucRcvAddress, UCHAR ** pucFrame, USHORT * pusLength )
 {
	 BOOL			 xFrameReceived = FALSE;
	 eMBErrorCode	 eStatus = MB_ENOERR;
 
	 ENTER_CRITICAL_SECTION(  );
	 
	 if(!(usRcvBufferPos < MB_SER_PDU_SIZE_MAX )){return;} // 
	 
 
	 /* Length and CRC check */
	 if( ( usRcvBufferPos >= MB_SER_PDU_SIZE_MIN )
		 && ( usMBCRC16( ( UCHAR * ) ucRTUBuf, usRcvBufferPos ) == 0 ) )
	 {
		 /* Save the address field. All frames are passed to the upper layed
		  * and the decision if a frame is used is done there.
		  */
		 *pucRcvAddress = ucRTUBuf[MB_SER_PDU_ADDR_OFF];
 
		 /* Total length of Modbus-PDU is Modbus-Serial-Line-PDU minus
		  * size of address field and CRC checksum.
		  */
		 *pusLength = ( USHORT )( usRcvBufferPos - MB_SER_PDU_PDU_OFF - MB_SER_PDU_SIZE_CRC );
 
		 /* Return the start of the Modbus PDU to the caller. */
		 *pucFrame = ( UCHAR * ) & ucRTUBuf[MB_SER_PDU_PDU_OFF];
		 xFrameReceived = TRUE;
	 }
	 else
	 {
		 eStatus = MB_EIO;
	 }
 
	 EXIT_CRITICAL_SECTION(  );
	 return eStatus;
 }
 
 
 eMBErrorCode
 eMBRTUSend( UCHAR ucSlaveAddress, const UCHAR * pucFrame, USHORT usLength )
 {
	 eMBErrorCode	 eStatus = MB_ENOERR;
	 USHORT 		 usCRC16;
 
	 ENTER_CRITICAL_SECTION(  );
 
	 /* Check if the receiver is still in idle state. If not we where to
	  * slow with processing the received frame and the master sent another
	  * frame on the network. We have to abort sending the frame.
	  */
	 if( eRcvState == STATE_RX_IDLE )
	 {
		 /* First byte before the Modbus-PDU is the slave address. */
		 pucSndBufferCur = ( UCHAR * ) pucFrame - 1;
		 usSndBufferCount = 1;
 
		 /* Now copy the Modbus-PDU into the Modbus-Serial-Line-PDU. */
		 pucSndBufferCur[MB_SER_PDU_ADDR_OFF] = ucSlaveAddress;
		 usSndBufferCount += usLength;
 
		 /* Calculate CRC16 checksum for Modbus-Serial-Line-PDU. */
		 usCRC16 = usMBCRC16( ( UCHAR * ) pucSndBufferCur, usSndBufferCount );
		 ucRTUBuf[usSndBufferCount++] = ( UCHAR )( usCRC16 & 0xFF );
		 ucRTUBuf[usSndBufferCount++] = ( UCHAR )( usCRC16 >> 8 );
 
		 /* Activate the transmitter. */
		 eSndState = STATE_TX_XMIT;
		  
		 vMBPortSerialEnable( FALSE, TRUE ); // serial eanble ,ready to send data 
 
		 
	 }
	 else
	 {
		 eStatus = MB_EIO;
	 }
	 EXIT_CRITICAL_SECTION(  );
	 return eStatus;
 }
 
 
 //---------------------------------------------------------------------------------------------
 
 
 
 
 BOOL
 xMBRTUReceiveFSM( UCHAR u8byte )
 {
	 BOOL			 xTaskNeedSwitch = FALSE;
	 UCHAR			 ucByte;
 
	 ucByte = u8byte; 
	 
	 switch ( eRcvState )
	 {
		 /* If we have received a character in the init state we have to
		  * wait until the frame is finished.
		  */
	 case STATE_RX_INIT:
		 vMBPortTimersEnable(  );
		 break;
 
		 /* In the error state we wait until all characters in the
		  * damaged frame are transmitted.
		  */
	 case STATE_RX_ERROR:
		 vMBPortTimersEnable(  );
		 break;
 
		 /* In the idle state we wait for a new character. If a character
		  * is received the t1.5 and t3.5 timers are started and the
		  * receiver is in the state STATE_RX_RECEIVCE.
		  */
	 case STATE_RX_IDLE:
		 usRcvBufferPos = 0;
		 ucRTUBuf[usRcvBufferPos++] = ucByte;
		 eRcvState = STATE_RX_RCV;
 
		 /* Enable t3.5 timers. */
		 vMBPortTimersEnable(  );
		
		 break;
 
	   /* We are currently receiving a frame. Reset the timer after
		  * every character received. If more than the maximum possible
		  * number of bytes in a modbus frame is received the frame is
		  * ignored.
		  */
	 case STATE_RX_RCV:
		 if( usRcvBufferPos < MB_SER_PDU_SIZE_MAX )
		 {
			 ucRTUBuf[usRcvBufferPos++] = ucByte;
		 }
		 else
		 {
			 eRcvState = STATE_RX_ERROR;
		 }
		 vMBPortTimersEnable(  );
		 break;
	 }
	 return xTaskNeedSwitch;
 }
 
 
 
 
 
 BOOL
 xMBRTUTransmitFSM( void )
 {
	 BOOL			 xNeedPoll = FALSE;
 
 
	 switch ( eSndState )
	 {
		 /* We should not get a transmitter event if the transmitter is in
		  * idle state.  */
	 case STATE_TX_IDLE:
		 /* enable receiver/disable transmitter. */
		 vMBPortSerialEnable( TRUE, FALSE );
		 break;
 
	 case STATE_TX_XMIT:  
		 /* check if we are finished. */
		 if( usSndBufferCount != 0 )
		 {
			 xMBPortSerialPutByte( ( CHAR )*pucSndBufferCur );
			 pucSndBufferCur++;  /* next byte in sendbuffer. */
			 usSndBufferCount--; 
		 }
		 else
		 {
			 xNeedPoll = xMBPortEventPost( EV_FRAME_SENT );
			 /* Disable transmitter. This prevents another transmit buffer
			  * empty interrupt. */
			 vMBPortSerialEnable( TRUE, FALSE );
			 eSndState = STATE_TX_IDLE;
		 }
		 break;
	 }
 
	 return xNeedPoll;
 }
 
  
 
 BOOL
 xMBRTUTimerT35Expired( void )
 {
	 BOOL			 xNeedPoll = FALSE;
 
	 switch ( eRcvState )
	 {
	 case STATE_RX_INIT:
		 xNeedPoll = xMBPortEventPost( EV_READY ); //  刚初始化时，如果发生第一次中断，则做准备
		 break;
	 case STATE_RX_RCV:
		 xNeedPoll = xMBPortEventPost( EV_FRAME_RECEIVED );
		 break;
	 case STATE_RX_ERROR:
		 break;
	 }
 
	 vMBPortTimersDisable(	);
	 eRcvState = STATE_RX_IDLE;
 
	 return xNeedPoll;
 }
 
 
 

 eMBErrorCode
 eMBEnable( void )
 {
	 eMBErrorCode	 eStatus = MB_ENOERR;
 
	 
	 if( eMBState == STATE_DISABLED )
	 {
		 /* Activate the protocol stack. */
		// pvMBFrameStartCur(  );
		 eMBRTUStart(); 
		 eMBState = STATE_ENABLED;
	 }
	 else
	 {
		 eStatus = MB_EILLSTATE;
	 }
	 return eStatus;
 }
 
 //---------------------------------------------------
 //----modbus polling--- 
 //-----------------------------------------------------
 eMBErrorCode
 eMBPoll(void)
 {
	 static UCHAR	*ucMBFrame;
	 static UCHAR	 ucRcvAddress;
	 static UCHAR	 ucFunctionCode;
	 static USHORT	 usLength;
	 static eMBException eException;
 
	 int			 i;
	 eMBErrorCode	 eStatus = MB_ENOERR;
	 eMBEventType	 eEvent;
 
	
	 /* Check if the protocol stack is ready. */
 
	 eMBState= STATE_ENABLED;
	 
	 if( eMBState != STATE_ENABLED )
	 {
		 return MB_EILLSTATE;
	 }	
 
	// xMBPortEventPost( EV_FRAME_RECEIVED );  // 在接收完成时调用该函数就可以将事件放入邮箱
	// in this program:   xMBRTUTimerT35Expired 
 
	
	 if( xMBPortEventGet( &eEvent ) == TRUE )
	 {
		 switch ( eEvent )
		 {
		 case EV_READY: 
			 break;
 
		 case EV_FRAME_RECEIVED:
			 eStatus = eMBRTUReceive( &ucRcvAddress, &ucMBFrame, &usLength );
			 if( eStatus == MB_ENOERR )
			 {
				 /* Check if the frame is for us. If not ignore the frame. */
				 if( ( ucRcvAddress == ucMBAddress ) || ( ucRcvAddress == MB_ADDRESS_BROADCAST ) )
				 {
					 ( void )xMBPortEventPost( EV_EXECUTE );
				 }
			 }
			 break;
 
		 case EV_EXECUTE:
			 ucFunctionCode = ucMBFrame[MB_PDU_FUNC_OFF];
			 eException = MB_EX_ILLEGAL_FUNCTION;
			 for( i = 0; i < MB_FUNC_HANDLERS_MAX; i++ )
			 {
				 /* No more function handlers registered. Abort. */
				 if( xFuncHandlers[i].ucFunctionCode == 0 )
				 {
					 break;
				 }
				 else if( xFuncHandlers[i].ucFunctionCode == ucFunctionCode )
				 {
					 eException = xFuncHandlers[i].pxHandler( ucMBFrame, &usLength );
					 break;
				 }
			 }
 
			 if( ucRcvAddress != MB_ADDRESS_BROADCAST )
			 {
				 if( eException != MB_EX_NONE )
				 {
					 /* An exception occured. Build an error frame. */
					 usLength = 0;
					 ucMBFrame[usLength++] = ( UCHAR )( ucFunctionCode | MB_FUNC_ERROR );
					 ucMBFrame[usLength++] = eException;
				 }
				 
				 eStatus = eMBRTUSend( ucMBAddress, ucMBFrame, usLength ); // 发送数据
			 }
			 break;
			 break;
 
		 case EV_FRAME_SENT:
			 break;
		 }
	 }
 
 }
 
 //------end-------------------------------------------
 
 // 清给地址里面的数据
 void ClearBuf(void)
 {
 
   u8 i;
   for(i=0;i<63;i++)
	 {
		usRegInputBuf[i]=0;  
		usRegHoldingBuf[i]=0; 
	 }
 
  
 
 }
 
#define addr_offset  2
 
  
 void DataSampleTestProgram(void)
 {
 
 
 
	static u16 dly=200; 
	static u8 i=0; 
	static u8 j=0; 
	static u8 ch=1; 
	u16  key=0; 
	 
	 MCGS_KeyValue=0; 
	 key=0;
  
	 if(usRegHoldingBuf[62]&0x0001){key=1;}
	 if(usRegHoldingBuf[62]&0x0002){key=2;}
	 if(usRegHoldingBuf[62]&0x0004){key=3;}
	 if(usRegHoldingBuf[62]&0x0008){key=4;}
	 if(usRegHoldingBuf[62]&0x0010){key=5;}
	 if(usRegHoldingBuf[62]&0x0020){key=6;}
	 if(usRegHoldingBuf[62]&0x0040){key=7;}
	 if(usRegHoldingBuf[62]&0x0080){key=8;}
	 if(usRegHoldingBuf[62]&0x0100){key=9;}
	 
	   
	// NP_Mode= usRegHoldingBuf[63];
	// SysVolSel=usRegHoldingBuf[64];
	 if((NP_Mode!= usRegHoldingBuf[63])||(SysVolSel!= usRegHoldingBuf[64]))
		 {
 
			NP_Mode= usRegHoldingBuf[63];
			SysVolSel=usRegHoldingBuf[64]; 
			
		 }
	   MCGS_KeyValue=key;  
 
 //----------------------------------------------------
  
	 usRegInputBuf[62]=SampleIndex;  
	 usRegInputBuf[63]=ch; 
	 usRegInputBuf[64]=0;  // current tempreture   
	// usRegInputBuf[64]=TempSampleNum_MIn; 					
		 
	 usRegInputBuf[65]=0;	
	// usRegInputBuf[65]=TempretureMonitor;   
	// usRegInputBuf[65]=TempretureSynTimeCnt ; 
	
	 usRegInputBuf[66]=0; // 系统工作状态 
	 usRegInputBuf[67]=0; //系统坐标
 
	 usRegInputBuf[68]=DisMesureManual; 
	 usRegInputBuf[69]=HysMesureManual; 
	 usRegInputBuf[70]=0 ; 
	 
	 SampleIndex=0; 
   
   
 }
 
 
  

u16  tstdat20=0; 
u32  tstdat21u32=10000000; 
u32  tstdat22u32=200000; 
 u32  tstdat23u32=300000; 
 u32  tstdat24u32=400000; 
u16   Np_Type=0; 


 void ModbusTestcode(void)
 {
    static u8 dly=100;

	if(dly>0){dly--;return;}
	
    dly=20; 

	

    tstdat20++;  
	tstdat21u32++;
	tstdat22u32++;
	tstdat23u32++;
	tstdat24u32++;
	
	//MotorSpeed+=100; 
	//if(MotorSpeed>10000)MotorSpeed=0; 
	
	//SysRun_Freq++; if(SysRun_Freq>100)SysRun_Freq=0; 
	
	//CH1_Freq++; if(CH1_Freq>200)CH1_Freq=0; 
	//CH2_Freq++; if(CH2_Freq>300)CH2_Freq=0; 
	
    //    CH1_Cnt+=10; 
	//CH2_Cnt+=20; 
	//CH1_Pass_Cnt+=30;
	//CH2_Pass_Cnt+=40; 

	//MotorSpeed= Puls_cnt_Coder; 
	

	
	usRegInputBuf[1]=MotorSpeed;
	usRegInputBuf[2]=SysRun_Freq;    //     SysRun_Freq  MonitorDat2
	usRegInputBuf[3]=CH1_Freq;
	usRegInputBuf[4]=CH2_Freq;   // OutPutDuty   CH2_Freq
	usRegInputBuf[5]=ControlValue;   // control vvalue 
         
	
	usRegInputBuf[10]=(CH1_Cnt>>16)&0xffff; // hi16bits 
	usRegInputBuf[11]=CH1_Cnt&0xffff;  // lo16bits
        usRegInputBuf[12]=(CH2_Cnt>>16)&0xffff; // hi16bits 
	usRegInputBuf[13]=CH2_Cnt&0xffff;  // lo16bits
	usRegInputBuf[14]=(CH1_Pass_Cnt>>16)&0xffff; // hi16bits 
	usRegInputBuf[15]=CH1_Pass_Cnt&0xffff;  // lo16bits
	usRegInputBuf[16]=(CH2_Pass_Cnt>>16)&0xffff; // hi16bits 
	usRegInputBuf[17]=CH2_Pass_Cnt&0xffff;  // lo16bits


 
	//usRegHoldingBuf[0]=Np_Type; 
	
	if((NP_Mode!= usRegHoldingBuf[1])||(PowerSupplyMode!= usRegHoldingBuf[2])||(TestPlateType!= usRegHoldingBuf[3]))
	{
	    NP_Mode= usRegHoldingBuf[1];	
		PowerSupplyMode= usRegHoldingBuf[2];
		TestPlateType= usRegHoldingBuf[3];  
		// save eeprom     
		 SaveEEprom();  
	} 

 }

 
 


