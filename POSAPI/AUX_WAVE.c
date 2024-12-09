//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L (32-bit Platform)                                     **
//**  PRODUCT  : AS330                                                      **
//**                                                                        **
//**  FILE     : AUX_WAVE.C                                                 **
//**  MODULE   : 							    **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2010/02/04                                                 **
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

extern	UINT8		os_WAVE_SEQNO;
extern	UINT8		os_WAVE_TXI_H;
extern	UINT8		os_WAVE_TXI_L;
extern	UINT8		os_WAVE_RXI_H;
extern	UINT8		os_WAVE_RXI_L;

// ---------------------------------------------------------------------------
// FUNCTION: Calculate CRC according to CRC16 polynominal equation.
//		G(X) = X^16 + X^15 + X^2 + 1
// INPUT   : crc - old CRC value.
//           ch  - new input data.
// OUTPUT  : none.
// RETURN  : new CRC value.
// ---------------------------------------------------------------------------
UINT16	WAVE_GenerateCRC16( UINT16 crc, UINT16 ch )
{
int	i;


	ch <<= 8;
	
	for( i=8; i>0; i-- )
	   {
	   if( (ch^crc) & 0x8000 )
	     crc = (crc << 1)^0x8005;
	   else
	     crc <<= 1;
	   ch <<= 1;
	   }
	// printf("crc=0x%x\n",crc);   
	return( crc );
}

// ---------------------------------------------------------------------------
// FUNCTION: Task manager of AUX data link layer TOB reached.
// INPUT   : port - port number.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_AUX_DataLinkRxTOB_WAVE( UINT32 port )
{
	    os_AUX_RxDataBuffer[port].Len = 0;
	    os_AUX_Para[port].RxFlag = FALSE;
	    os_AUX_State[port] = AUX_STATE_IDLE;
}

// ---------------------------------------------------------------------------
// FUNCTION: AUX data link layer protocol - VisaWave.
//	     Package Format: 
//		TX: STX SEQ_NO(1) TXI(2) RXI(2) INST_ID(1) LEN(2)         DATA(n) CRC(2) ETX
//		RX: STX SEQ_NO(1) TXI(2) RXI(2) INST ID(1) LEN(2) RESP(1) DATA(n) CRC(2) ETX
//				                |<-------  MSG  ---------------->|
//		        |<-------------------------------  CRC  ---------------->|
// INPUT   : pUart
// OUTPUT  : none.
// RETURN  : TRUE  - package received ok
//           FALSE - package not available 
// ---------------------------------------------------------------------------
/*
UINT32	OS_AUX_DataLinkRx_WAVE( BSP_UART *pUart )
{
UINT32	State;
UINT32	count;
UINT32	DataLen = 0;
UINT32	i;
UINT32	result;
UINT8	data;
UINT16	crc16 = 0;


START:
	DataLen = 0;
	crc16 = 0;

	result = FALSE;

	if( BSP_UART_GetRxAvail( pUart ) == 0 )	// skip non-RxD events
	  return( result );
	  
	// Startup TOB process
	if( os_AUX_Para[pUart->UartNum].RxFlag == FALSE )
	  {
	  os_AUX_State[pUart->UartNum] = WAVE_STATE_IDLE;
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
	count = BSP_UART_Read( pUart, &data, 1 );	// read one byte
	     
	     switch( State )
	           {
	           case WAVE_STATE_IDLE:
	      
	                if( data == STX )				// check leading char
	                  {
	                  os_AUX_RxDataBuffer[pUart->UartNum].Len = 0;	// reset final length of package
	                  
	                  DataLen = 0;
	                  os_AUX_DataLen[pUart->UartNum] = 0;		// reset data length
	                  
	                  i = 0;					// reset index
	             
	                  State = WAVE_STATE_SEQNO;
	                  }
	             
	                break;
	           
	           case WAVE_STATE_SEQNO:
	           	
	           	crc16 = 0;
	           	os_AUX_LRC[pUart->UartNum] = 0;		// reset CRC16
	           	
	           	State = WAVE_STATE_TXI_H;
	           	break;
	           	
	           case	WAVE_STATE_TXI_H:
	           
	           	State = WAVE_STATE_TXI_L;
	           	break;
	           	
	           case	WAVE_STATE_TXI_L:
	           	
	           	State = WAVE_STATE_RXI_H;           
	           	break;

	           case	WAVE_STATE_RXI_H:
	           	
	           	State = WAVE_STATE_RXI_L;
	           	break;

	           case	WAVE_STATE_RXI_L:
	           	
	           	State = WAVE_STATE_INST;           
	           	break;

	           case	WAVE_STATE_INST:
	           	
	           	os_AUX_RxDataBuffer[pUart->UartNum].Data[i++] = data;	// msg offset 0x00
	           	
	           	State = WAVE_STATE_LEN_H;           
	           	break;

	           case	WAVE_STATE_LEN_H:
	           	
	           	os_AUX_RxDataBuffer[pUart->UartNum].Data[i++] = data;	// msg offset 0x01
	           	DataLen = data;
	           	DataLen <<= 8;
	           	
	           	State = WAVE_STATE_LEN_L;
	           	break;

	           case	WAVE_STATE_LEN_L:
	           	
	           	os_AUX_RxDataBuffer[pUart->UartNum].Data[i++] = data;	// msg offset 0x02
	           	DataLen |= data;
	           	
	           	if( DataLen )
	           	  State = WAVE_STATE_RESP;
	           	else
	           	  State = WAVE_STATE_CRC_H;
	           	  
	           	break;

	           case	WAVE_STATE_RESP:	// response code
	           	
			os_AUX_RxDataBuffer[pUart->UartNum].Data[i++] = data;
			
			if( --DataLen )
			  State = WAVE_STATE_DATA;
			else
			  State = WAVE_STATE_CRC_H;
			
			break;

	           case WAVE_STATE_DATA:
	      		
	      		os_AUX_RxDataBuffer[pUart->UartNum].Data[i++] = data;
	      		
			if( --DataLen )
			  State = WAVE_STATE_DATA;
			else
			  State = WAVE_STATE_CRC_H;

	                break;
	           
	           case WAVE_STATE_CRC_H:
	           
	           	if( data == (UINT8)((crc16 & 0xFF00) >> 8) )
	           	  State = WAVE_STATE_CRC_L;
	           	else
	           	  State = WAVE_STATE_IDLE;
	           	  
	           	break;

	           case WAVE_STATE_CRC_L:
	           
	           	if( data == (UINT8)(crc16 & 0x00FF) )
	           	  State = WAVE_STATE_ETX;
	           	else
	           	  State = WAVE_STATE_IDLE;
	           	  
	           	break;
		   
		   case WAVE_STATE_ETX:
		   
		   	if( data == ETX )
		   	  {
		   	  if( ++os_WAVE_SEQNO == 0 )	// seqno = 01..FF
		   	    os_WAVE_SEQNO = 1;
		   	  
		   	  // actual length = INST(1) + LEN_H(1) + LEN_L(1)+ DATA(LEN)
//		   	  count = os_AUX_RxDataBuffer[pUart->UartNum].Data[1]*256 + os_AUX_RxDataBuffer[pUart->UartNum].Data[2] + 3;
//		   	  os_AUX_RxDataBuffer[pUart->UartNum].Len = count;	// put LEN
			  os_AUX_RxDataBuffer[pUart->UartNum].Len = i;		// put LEN
		   	  
		   	  result = TRUE;
		   	  }
		   	
		   	os_AUX_Para[pUart->UartNum].RxFlag = FALSE;
		   	State = WAVE_STATE_IDLE;
		   	
		   	break;
	           
	           default:

	                break;
	                
	           } // switch()
	     
	     if( (State > WAVE_STATE_SEQNO) && (State < WAVE_STATE_CRC_L) )
	       crc16 = WAVE_GenerateCRC16( crc16, (UINT16)data );	// generate CRC16
	     
	     os_AUX_Index[pUart->UartNum] = i;		// update data index
	     os_AUX_State[pUart->UartNum] = State;	// update state machine
	     os_AUX_DataLen[pUart->UartNum] = DataLen;	// update data length
	     os_AUX_LRC[pUart->UartNum] = crc16;	// update CRC16

	goto START;	// PATCH: 2010-09-30, keep processing the queued FIFO data
	     
//	return( result );
}
*/

// ---------------------------------------------------------------------------
// FUNCTION: AUX data link layer protocol - VisaWave.
//	     Package Format: 
//		TX: STX SEQ_NO(1) TXI(2) RXI(2) INST_ID(1) LEN(2)         DATA(n) CRC(2) ETX
//		RX: STX SEQ_NO(1) TXI(2) RXI(2) INST ID(1) LEN(2) RESP(1) DATA(n) CRC(2) ETX
//				                |<-------  MSG  ---------------->|
//		        |<-------------------------------  CRC  ---------------->|
// INPUT   : pUart
// OUTPUT  : none.
// RETURN  : TRUE  - package received ok
//           FALSE - package not available 
// NOTE	   : Edit by West for while(OS_AUX_RxReady) polling rx used
// ---------------------------------------------------------------------------

UINT32	OS_AUX_DataLinkRx_WAVE( BSP_UART *pUart )
{
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
	// printf("os_AUX_DataLen[pUart->UartNum]=%d\n",os_AUX_DataLen[pUart->UartNum]);
	// Startup TOB process
	if( os_AUX_Para[pUart->UartNum].RxFlag == FALSE )
	  {
	  os_AUX_State[pUart->UartNum] = WAVE_STATE_IDLE;
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
		  printf("@@@@@@@@2out of range\n");
	  os_AUX_RxDataBuffer[pUart->UartNum].Len = 0;
	  
	  os_AUX_Para[pUart->UartNum].RxFlag = FALSE;
	  os_AUX_State[pUart->UartNum] = AUX_STATE_IDLE;
	  
	  while( BSP_UART_Read( pUart, (UINT8 *)&data, 1 ) );	// get rid of all incoming data
	  return( result );
	  }

	
	// --- Protocol State Machine ---
	    //  printf("State=%d\n",State);
	     switch( State )
	           {
	           case WAVE_STATE_IDLE:
	      
	                if( data == STX )				// check leading char
	                  {
	                  os_AUX_RxDataBuffer[pUart->UartNum].Len = 0;	// reset final length of package
	                  
	                  DataLen = 0;
	                  os_AUX_DataLen[pUart->UartNum] = 0;		// reset data length
	                  
	                  i = 0;					// reset index
	             
	                  State = WAVE_STATE_SEQNO;
	                  }
	             
	                break;
	           
	           case WAVE_STATE_SEQNO:
	           	
	           	crc16 = 0;
	           	os_AUX_LRC[pUart->UartNum] = 0;		// reset CRC16
	           	
	           	State = WAVE_STATE_TXI_H;
	           	break;
	           	
	           case	WAVE_STATE_TXI_H:
	           
	           	State = WAVE_STATE_TXI_L;
	           	break;
	           	
	           case	WAVE_STATE_TXI_L:
	           	
	           	State = WAVE_STATE_RXI_H;           
	           	break;

	           case	WAVE_STATE_RXI_H:
	           	
	           	State = WAVE_STATE_RXI_L;
	           	break;

	           case	WAVE_STATE_RXI_L:
	           	
	           	State = WAVE_STATE_INST;           
	           	break;

	           case	WAVE_STATE_INST:
	           	
	           	os_AUX_RxDataBuffer[pUart->UartNum].Data[i++] = data;	// msg offset 0x00
	           	
	           	State = WAVE_STATE_LEN_H;           
	           	break;

	           case	WAVE_STATE_LEN_H:
	           	
	           	os_AUX_RxDataBuffer[pUart->UartNum].Data[i++] = data;	// msg offset 0x01
	           	DataLen = data;
	           	DataLen <<= 8;
	           	// printf("data=%d  <<8=%d\n",data,DataLen);
	           	State = WAVE_STATE_LEN_L;
	           	break;

	           case	WAVE_STATE_LEN_L:
	           	
	           	os_AUX_RxDataBuffer[pUart->UartNum].Data[i++] = data;	// msg offset 0x02
	           	DataLen |= data;
	           	
	           	if( DataLen )
	           	  State = WAVE_STATE_RESP;
	           	else
	           	  State = WAVE_STATE_CRC_H;
	           	  
	           	break;

	           case	WAVE_STATE_RESP:	// response code
	           	
			os_AUX_RxDataBuffer[pUart->UartNum].Data[i++] = data;
			
			if( --DataLen )
			  State = WAVE_STATE_DATA;
			else
			  State = WAVE_STATE_CRC_H;
			
			break;

	           case WAVE_STATE_DATA:
	      		
	      		os_AUX_RxDataBuffer[pUart->UartNum].Data[i++] = data;
	      		
			if( --DataLen )
			  State = WAVE_STATE_DATA;
			else
			  State = WAVE_STATE_CRC_H;

	                break;
	           
	           case WAVE_STATE_CRC_H:
	           
	           	if( data == (UINT8)((crc16 & 0xFF00) >> 8) )
	           	  State = WAVE_STATE_CRC_L;
	           	else
	           	  State = WAVE_STATE_IDLE;
	           	  
	           	break;

	           case WAVE_STATE_CRC_L:
	           
	           	if( data == (UINT8)(crc16 & 0x00FF) )
	           	  State = WAVE_STATE_ETX;
	           	else
	           	  State = WAVE_STATE_IDLE;
	           	  
	           	break;
		   
		   case WAVE_STATE_ETX:
		   
		   	if( data == ETX )
		   	  {
		   	  if( ++os_WAVE_SEQNO == 0 )	// seqno = 01..FF
		   	    os_WAVE_SEQNO = 1;
		   	  
		   	  // actual length = INST(1) + LEN_H(1) + LEN_L(1)+ DATA(LEN)
//		   	  count = os_AUX_RxDataBuffer[pUart->UartNum].Data[1]*256 + os_AUX_RxDataBuffer[pUart->UartNum].Data[2] + 3;
//		   	  os_AUX_RxDataBuffer[pUart->UartNum].Len = count;	// put LEN
			  os_AUX_RxDataBuffer[pUart->UartNum].Len = i;		// put LEN
		   	  
		   	  result = TRUE;
			  return( result );
		   	  }
		   	
		   	os_AUX_Para[pUart->UartNum].RxFlag = FALSE;
		   	State = WAVE_STATE_IDLE;
		   	
		   	break;
	           
	           default:

	                break;
	                
	           } // switch()
	     
	     if( (State > WAVE_STATE_SEQNO) && (State < WAVE_STATE_CRC_L) )
	       crc16 = WAVE_GenerateCRC16( crc16, (UINT16)data );	// generate CRC16
	     
	     os_AUX_Index[pUart->UartNum] = i;		// update data index
	     os_AUX_State[pUart->UartNum] = State;	// update state machine
	     os_AUX_DataLen[pUart->UartNum] = DataLen;	// update data length
	     os_AUX_LRC[pUart->UartNum] = crc16;	// update CRC16
		//  printf("DataLen=%d\n",DataLen);
		// printf("crc=0x%x\n",crc16);
	//goto START;	// PATCH: 2010-09-30, keep processing the queued FIFO data
	     
	return( result );
}
// ---------------------------------------------------------------------------
// FUNCTION: Wait for ACK from the sender according to DLL protocol.
// INPUT   : pUart -- pointer to the BSP_UART structure.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
//void	OS_AUX_DataLinkWaitAck_WAVE( BSP_UART *pUart )
//{
//	// NA
//}

// ---------------------------------------------------------------------------
// FUNCTION: AUX data transmission by link layer format - VisaWAVE.
//	     Package Format: STX SeqNo TxI RxI Data(n) CRC ETX
//			     V() = InstructionID(1) + DataLen(2) + Data(n)
// INPUT   : pUart
//           pData  - (LL-V)
// OUTPUT  : none.
// RETURN  : TRUE  - package transmitted ok.
//           FALSE - package transmitted failed. 
// ---------------------------------------------------------------------------
UINT32	OS_AUX_DataLinkTx_WAVE( BSP_UART *pUart, UINT8 *pData )
{
UINT32	i;
UINT32	index;
UINT32	len;
UINT16	crc16;
UINT8	inst;
	
	
	inst = *(pData+2);	// reset seqno if the following instructions
//	if( (inst == WAVE_INST_POLL) || (inst == WAVE_INST_SET_OPTI) || (inst == WAVE_INST_SET_PARA) )
//	  os_WAVE_SEQNO = 0;

	if( ((inst & 0xF0) != 0x20) && ((inst & 0xF0) != 0x30) )	// PATCH: 2014-04-05
	  os_WAVE_SEQNO = 0;						// reset seq number if not SECURITY & TRANSACTION messages
	
  	// --- Build package ---
	index = 0;
	os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = STX;		// STX
	
	crc16 = 0;	// reset CRC16
	
	os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = os_WAVE_SEQNO;	// sequence no
	crc16 = WAVE_GenerateCRC16( crc16, (UINT16)os_WAVE_SEQNO );
	
	os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = os_WAVE_TXI_H;	// sender index
	crc16 = WAVE_GenerateCRC16( crc16, (UINT16)os_WAVE_TXI_H );		//
	os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = os_WAVE_TXI_L;	//
	crc16 = WAVE_GenerateCRC16( crc16, (UINT16)os_WAVE_TXI_L );		//
	
	os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = os_WAVE_RXI_H;	// receiver index
	crc16 = WAVE_GenerateCRC16( crc16, (UINT16)os_WAVE_RXI_H );		//
	os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = os_WAVE_RXI_L;	//
	crc16 = WAVE_GenerateCRC16( crc16, (UINT16)os_WAVE_RXI_L );		//
	
	len = *pData++;
	len += (*pData++)*256;
	
	for( i=0; i<len; i++ )							// Data(n)
	   {
	   os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = *pData;
	   crc16 = WAVE_GenerateCRC16( crc16, (UINT16)*pData++ );
	   }
	
	os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = (crc16 & 0xFF00) >> 8;
	os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = (crc16 & 0x00FF);	// CRC16
	
	os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = ETX;		// ETX
	
	if( ++os_WAVE_SEQNO == 0 )	// seqno++ (01..FF)
	  os_WAVE_SEQNO = 1;
	
	os_AUX_TxDataBuffer[pUart->UartNum].Len = index;				// total package size
	
	// --- Send package to UARTx ---
	os_AUX_Para[pUart->UartNum].Resend = os_AUX_ParaBak[pUart->UartNum].Resend;	// restore RESEND cnt
	
	os_AUX_State[pUart->UartNum] = WAVE_STATE_IDLE;				// change state to IDLE
	// printf("crc=0x%x\n",crc16);
	return( OS_AUX_SendPackage( pUart ) );
}
