//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L (32-bit Platform)                                     **
//**  PRODUCT  : AS330                                                      **
//**                                                                        **
//**  FILE     : AUX_SVC.C                                                  **
//**  MODULE   : 							    **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2010/02/06                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2010 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include "bsp_types.h"
#include "bsp_uart.h"
#include "DEV_AUX.h"

extern	API_AUX		os_AUX_Para[];
extern	API_AUX		os_AUX_ParaBak[];
extern	AUX_DATA	os_AUX_RxDataBuffer[];
extern	AUX_DATA	os_AUX_TxDataBuffer[];
extern	UINT32		os_AUX_State[];
extern	UINT32		os_AUX_Status[];
extern	UINT32		os_AUX_Index[];
extern	UINT32		os_AUX_DataLen[];
extern	UINT32		os_AUX_LRC[];

//extern	UINT8		os_WAVE_SEQNO;
//extern	UINT8		os_WAVE_TXI_H;
//extern	UINT8		os_WAVE_TXI_L;
//extern	UINT8		os_WAVE_RXI_H;
//extern	UINT8		os_WAVE_RXI_L;


// ---------------------------------------------------------------------------
// FUNCTION: AUX data link layer protocol - SVC.
//	     Package Format: 
//		STX [HEADER(4) LEN(2) DATA(n)] CRC(2) ETX
//		    |<-------  MSG  -------->|
//		    |<-------  CRC  -------->|
// INPUT   : pUart
// OUTPUT  : none.
// RETURN  : TRUE  - package received ok
//           FALSE - package not available 
// ---------------------------------------------------------------------------
UINT32	OS_AUX_DataLinkRx_SVC( BSP_UART *pUart )
{
UINT32	fSTX = FALSE;
UINT32	State;
UINT32	count;
UINT32	DataLen = 0;
UINT32	i;
UINT32	result;
UINT8	data;
UINT16	crc16 = 0;


//START:
	DataLen = 0;
	crc16 = 0;

	result = FALSE;

	// Startup TOB process
	if( os_AUX_Para[pUart->UartNum].RxFlag == FALSE )
	  {
	  os_AUX_State[pUart->UartNum] = SVC_STATE_IDLE;
	  os_AUX_Para[pUart->UartNum].Tob = os_AUX_ParaBak[pUart->UartNum].Tob;
	  os_AUX_Para[pUart->UartNum].RxFlag = TRUE;
	  
	  os_AUX_Index[pUart->UartNum] = 0;
	  os_AUX_DataLen[pUart->UartNum] = 0;
	  os_AUX_LRC[pUart->UartNum] = 0;
	  os_AUX_RxDataBuffer[pUart->UartNum].Len = 0;
	  }

	// retrive state machine
	i = os_AUX_Index[pUart->UartNum];
	DataLen = os_AUX_DataLen[pUart->UartNum];
	crc16 = os_AUX_LRC[pUart->UartNum];
	State = os_AUX_State[pUart->UartNum];
	data=os_AUX_RxDataBuffer[pUart->UartNum].Data[i];
	// out of range?
	if( DataLen > (pUart->BufferSize) )
	  {
	  os_AUX_RxDataBuffer[pUart->UartNum].Len = 0;
	  
	  os_AUX_Para[pUart->UartNum].RxFlag = FALSE;
	  os_AUX_State[pUart->UartNum] = AUX_STATE_IDLE;
	  
	  while( BSP_UART_Read( pUart, (UINT8 *)&data, 1 ) );	// get rid of all incoming data
	  return( result );
	  }

	
	// --- Protocol State Machine ---
	     
	     switch( State )
	           {
	           case SVC_STATE_IDLE:
	      
	                if( data == STX )				// check leading char
	                  {
	                  os_AUX_RxDataBuffer[pUart->UartNum].Len = 0;	// reset final length of package
	                  
	                  DataLen = 0;
	                  os_AUX_DataLen[pUart->UartNum] = 0;		// reset data length
	                  
	                  i = 0;					// reset index
	                  
	                  crc16 = 0;
	           	  os_AUX_LRC[pUart->UartNum] = 0;		// reset CRC16
	           	
	                  State = SVC_STATE_HEAD;
	                  
	                  fSTX = TRUE;
	                  }
	             
	                break;
	           
	           case SVC_STATE_HEAD:
	           	
	           	os_AUX_RxDataBuffer[pUart->UartNum].Data[i++] = data;
	           	if( i == 4 )				// 4 bytes of header received?
	           	  State = SVC_STATE_LEN_H;
	           	
	           	break;

	           case	SVC_STATE_LEN_H:
	           	
	           	os_AUX_RxDataBuffer[pUart->UartNum].Data[i++] = data;	// msg offset 0x01
	           	DataLen = data;
	           	DataLen <<= 8;
	           	
	           	State = SVC_STATE_LEN_L;
	           	break;

	           case	SVC_STATE_LEN_L:
	           	
	           	os_AUX_RxDataBuffer[pUart->UartNum].Data[i++] = data;	// msg offset 0x02
	           	DataLen |= data;
	           	
	           	if( DataLen )
	           	  State = SVC_STATE_DATA;
	           	else
	           	  State = SVC_STATE_CRC_H;
	           	  
	           	break;

	           case SVC_STATE_DATA:
	      		
	      		os_AUX_RxDataBuffer[pUart->UartNum].Data[i++] = data;
	      		
			if( --DataLen )
			  State = SVC_STATE_DATA;
			else
			  State = SVC_STATE_CRC_H;

	                break;
	           
	           case SVC_STATE_CRC_H:
	           
	           	if( data == (UINT8)((crc16 & 0xFF00) >> 8) )
	           	  State = SVC_STATE_CRC_L;
	           	else
	           	  State = SVC_STATE_IDLE;
	           	  
	           	break;

	           case SVC_STATE_CRC_L:
	           
	           	if( data == (UINT8)(crc16 & 0x00FF) )
	           	  State = SVC_STATE_ETX;
	           	else
	           	  State = SVC_STATE_IDLE;
	           	  
	           	break;
		   
		   case SVC_STATE_ETX:
		   
		   	if( data == ETX )
		   	  {		   	  
		   	  // actual length = HEADER(4) + LEN_H(1) + LEN_L(1)+ DATA(LEN)
			  os_AUX_RxDataBuffer[pUart->UartNum].Len = i;		// put LEN
		   	  
		   	  result = TRUE;
			  return( result );
		   	  }
		   	
		   	os_AUX_Para[pUart->UartNum].RxFlag = FALSE;
		   	State = SVC_STATE_IDLE;
		   	
		   	break;
	           
	           default:

	                break;
	                
	           } // switch()
	     
	     if( !fSTX && (State > SVC_STATE_IDLE) && (State < SVC_STATE_CRC_L) )
	       crc16 = WAVE_GenerateCRC16( crc16, (UINT16)data );	// generate CRC16

	     os_AUX_Index[pUart->UartNum] = i;		// update data index
	     os_AUX_State[pUart->UartNum] = State;	// update state machine
	     os_AUX_DataLen[pUart->UartNum] = DataLen;	// update data length
	     os_AUX_LRC[pUart->UartNum] = crc16;	// update CRC16

	//goto START;	// PATCH: 2010-09-30, keep processing the queued FIFO data
	     
	return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: Wait for ACK from the sender according to DLL protocol.
// INPUT   : pUart -- pointer to the BSP_UART structure.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
//void	OS_AUX_DataLinkWaitAck_SVC( BSP_UART *pUart )
//{
//	// NA
//}

// ---------------------------------------------------------------------------
// FUNCTION: AUX data transmission by link layer format - SVC.
//		STX [HEADER(4) LEN(2) DATA(n)] CRC(2) ETX
//		    |<-------  MSG  -------->|
//		    |<-------  CRC  -------->|
//		    The MSG is handled by AP.
// INPUT   : pUart
//           pData  - (LL-V), V=MSG
// OUTPUT  : none.
// RETURN  : TRUE  - package transmitted ok.
//           FALSE - package transmitted failed. 
// ---------------------------------------------------------------------------
UINT32	OS_AUX_DataLinkTx_SVC( BSP_UART *pUart, UINT8 *pData )
{
UINT32	i;
UINT32	index;
UINT32	len;
UINT16	crc16;
	
	
  	// --- Build package ---
	index = 0;
	os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = STX;		// STX
	
	crc16 = 0;	// reset CRC16
	
	len = *pData++;
	len += (*pData++)*256;
	
	for( i=0; i<len; i++ )							// Header(4)+Data(n)
	   {
	   os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = *pData;
	   crc16 = WAVE_GenerateCRC16( crc16, (UINT16)*pData++ );
	   }
	
	os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = (crc16 & 0xFF00) >> 8;
	os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = (crc16 & 0x00FF);	// CRC16
	
	os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = ETX;		// ETX
	
	os_AUX_TxDataBuffer[pUart->UartNum].Len = index;				// total package size
	
	// --- Send package to UARTx ---
	os_AUX_Para[pUart->UartNum].Resend = os_AUX_ParaBak[pUart->UartNum].Resend;	// restore RESEND cnt
	
	os_AUX_State[pUart->UartNum] = SVC_STATE_IDLE;					// change state to IDLE
	// printf("receive data=\n");
    // for(i=0;i<index;i++)
	// 	printf(" 0x%x ",os_AUX_TxDataBuffer[pUart->UartNum].Data[i]);
    // printf("\n");
	return( OS_AUX_SendPackage( pUart ) );
}
