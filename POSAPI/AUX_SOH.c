//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L (32-bit Platform)                                     **
//**  PRODUCT  : AS330/QS200                                                **
//**                                                                        **
//**  FILE     : AUX_SOH.C                                                  **
//**  MODULE   : OS_AUX_DataLinkRxSOH()					    **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2007/12/31                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2007 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include "bsp_types.h"
#include "bsp_uart.h"
#include "DEV_AUX.h"

//extern	UINT32		OS_AUX_DataLinkRx_SOH( BSP_UART *pUart );
//extern	UINT32		OS_AUX_DataLinkTx_SOH( BSP_UART *pUart, UINT8 *pData );
//extern	UINT32		OS_AUX_DataLinkAck_DLL( BSP_UART *pUart, UINT8 Code );

extern	API_AUX		os_AUX_Para[];
extern	API_AUX		os_AUX_ParaBak[];
extern	AUX_DATA	os_AUX_RxDataBuffer[];
extern	AUX_DATA	os_AUX_TxDataBuffer[];
extern	UINT32		os_AUX_State[];
extern	UINT32		os_AUX_Status[];
extern	UINT32		os_AUX_Index[];
extern	UINT32		os_AUX_DataLen[];
extern	UINT32		os_AUX_DataCnt[];
extern	UINT32		os_AUX_LRC[];

extern	UINT32		os_SOH_LEN_TYPE;

// ---------------------------------------------------------------------------
// FUNCTION: Task manager of AUX data link layer TOB reached.
// INPUT   : port - port number.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_AUX_DataLinkRxTOB_SOH( UINT32 port )
{
	    os_AUX_RxDataBuffer[port].Len = 0;
	    os_AUX_Para[port].RxFlag = FALSE;
	    os_AUX_State[port] = AUX_STATE_IDLE;
}

// ---------------------------------------------------------------------------
// FUNCTION: AUX data link layer protocol - SOH.
//	     Package Format: SOH LEN(2) Data(n) LRC
//			     LRC = XOR(LEN..Data)
//	     Acknowledge   : ACK = 0x06, NAK = 0x15
// INPUT   : pUart
// OUTPUT  : none.
// RETURN  : TRUE  - package received ok
//           FALSE - package not available 
// ---------------------------------------------------------------------------
/*
UINT32	OS_AUX_DataLinkRx_SOH( BSP_UART *pUart )
{
UINT32	State;
UINT32	count;
UINT32	DataLen = 0;
UINT32	DataCnt = 0;
UINT32	i;
UINT32	result;
UINT8	data;
UINT8	lrc = 0;
	

START:
	DataLen = 0;
	DataCnt = 0;
	lrc = 0;

	result = FALSE;

	if( BSP_UART_GetRxAvail( pUart ) == 0 )	// skip non-RxD events
	  return( result );
	
//	count = BSP_UART_GetRxAvail( pUart );	// min. package length
//	if( count == 0 )
//	  {
//	  if( OS_AUX_TaskCheckTob( pUart ) == TRUE )
//	    {
//	    os_AUX_RxDataBuffer[pUart->UartNum].Len = 0;
//	    os_AUX_Para[pUart->UartNum].RxFlag = FALSE;
//	    os_AUX_State[pUart->UartNum] = AUX_STATE_IDLE;
//	    }
//	  return( result );
//	  }
	  
	// Startup TOB process
	if( os_AUX_Para[pUart->UartNum].RxFlag == FALSE )
	  {
	  os_AUX_State[pUart->UartNum] = AUX_STATE_IDLE;
	  os_AUX_Para[pUart->UartNum].Tob = os_AUX_ParaBak[pUart->UartNum].Tob;
	  os_AUX_Para[pUart->UartNum].RxFlag = TRUE;
	  
	  os_AUX_Index[pUart->UartNum] = 0;
	  os_AUX_DataLen[pUart->UartNum] = 0;
	  os_AUX_DataCnt[pUart->UartNum] = 0;
	  os_AUX_LRC[pUart->UartNum] = 0;
	  
	  os_AUX_RxDataBuffer[pUart->UartNum].Len = 0;
	  }

	// retrive state machine
	i = os_AUX_Index[pUart->UartNum];
	DataLen = os_AUX_DataLen[pUart->UartNum];
	DataCnt = os_AUX_DataCnt[pUart->UartNum];
	lrc = os_AUX_LRC[pUart->UartNum];
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
	
//	// Min. data received?
//	if( count < 4 )
//	  return( result );
//	
//	// Filter out all non-SOH data one by one
//	while( *pUart->pRd != SOH )
//	     {
//	     if( BSP_UART_Read( pUart, (UINT8 *)&data, 1 ) == 0 )
//	       return( result );
//	     }
//	     
//	count = BSP_UART_GetRxAvail( pUart );	// min. package length
//	if( count < 4 )
//	  return( result );
//
//	DataLen = *(pUart->pRd + 1);		// get data LEN_L
//	DataLen += (*(pUart->pRd + 2)*256);	//	    LEN_H
//	if( count != (DataLen + 4) )		// whole package received?
//	  return( result );
	
	// --- Protocol State Machine ---
//	i = 0;
//	State = AUX_STATE_IDLE;
	
//	count = BSP_UART_Read( pUart, os_AUX_RxDataBuffer[pUart->UartNum].Data, count );	// read the whole package
	count = BSP_UART_Read( pUart, &data, 1 );	// read one byte
//	while( count-- )
//	     {
//	     data = os_AUX_RxDataBuffer[pUart->UartNum].Data[i];	// get 1 byte of package
	     
	     switch( State )
	           {
	           case AUX_STATE_IDLE:
	      
	                if( data == SOH )				// check leading char
	                  {
	                  os_AUX_RxDataBuffer[pUart->UartNum].Len = 0;	// reset final length of package
	                  
	                  DataLen = 0;
	                  DataCnt = 0;
	                  os_AUX_DataLen[pUart->UartNum] = 0;		// reset data length
	                  
	                  lrc = 0;
	                  os_AUX_LRC[pUart->UartNum] = 0;		// reset LRC
	                  
	                  i = 0;					// reset index
	                          
	                  State = AUX_STATE_LEN_L;
	                  }
	             
	                break;
	           
	           case AUX_STATE_LEN_L:
	      
//	                if( data == 0 )
//	                  count = 0;					// invalid length
//	                else
//	                  {
	                  DataCnt = data;				// char count (L)
	                  lrc = data;           			// init LRC
	             
	                  if( os_SOH_LEN_TYPE == 0 )
	                    State = AUX_STATE_LEN_H;
	                  else
	                    State = AUX_STATE_LEN_L2;
//	                  }
	      
	                break;
	           
	           case AUX_STATE_LEN_L2:
	                
	                lrc ^= data;
	                DataCnt += data*0x100;
	                
	                State = AUX_STATE_LEN_H;
	                
	                break;
	           
	           case AUX_STATE_LEN_H:
	      
	      		lrc ^= data;
	      		
			if( os_SOH_LEN_TYPE == 0 )
			  {
	      		  DataCnt += data*256;				// char count (H)
	      		  
	      		  if( DataCnt != 0 )
	                    State = AUX_STATE_DATA;
	                  else
	                    State = AUX_STATE_IDLE;			// invalid length
	                  }
	                else
	                  {
	                  DataCnt += data*0x10000;
	                  State = AUX_STATE_LEN_H2;
	                  }
	      
	                break;
	           
	           case AUX_STATE_LEN_H2:
			
			lrc ^= data;
			
			DataCnt += data*0x1000000;
			
	      		if( DataCnt != 0 )
	                  State = AUX_STATE_DATA;
	                else
	                  State = AUX_STATE_IDLE;			// invalid length
			
			break;
			
	           case AUX_STATE_DATA:
	      
	                lrc ^= data;					// check sum
	           
////	                if( i > AUX_DEFAULT_BUF_SIZE )			// out of range, reset state
//			if( i > (pUart->BufferSize) )			// PATCH: 2009-08-28
//	                  count = 0;
//	                else
//	                  {  
//	                  if( --DataCnt == 0 )				// end of data field?
//	                    State = AUX_STATE_LRC;
//	                  }

			os_AUX_RxDataBuffer[pUart->UartNum].Data[i] = data;
			
	                DataLen++;					// RX_LEN++
	                i++;						// next byte
			
			if( DataLen == DataCnt )			// end of data field?
			  State = AUX_STATE_LRC;
	             
	                break;
	           
	           case AUX_STATE_LRC:
	      
	                if( data == lrc )				// good LRC (ACK will be sent after API reading)
	                  {
	                  os_AUX_RxDataBuffer[pUart->UartNum].Len = DataLen;	// put LEN & DATA without leading header
//	                  memmove( os_AUX_RxDataBuffer[pUart->UartNum].Data, &os_AUX_RxDataBuffer[pUart->UartNum].Data[2+1], DataLen );
	                  
	                  OS_AUX_DataLinkAck_DLL( pUart, ACK );		// 2009-10-30
	                  result = TRUE;
	                  }
	                else
	                  {	// bad LRC, NAK to sender
	                  OS_AUX_DataLinkAck_DLL( pUart, NAK );
	                  }
	           	
	           	os_AUX_Para[pUart->UartNum].RxFlag = FALSE;
	           	State = AUX_STATE_IDLE;
	           	
//	                count = 0;
	           
	                break;
	           
	           default:
	           
//	                count = 0;
	                break;
	                
	           } // switch()
	     
//	     ++i; // next byte

	     os_AUX_Index[pUart->UartNum] = i;		// update data index
	     os_AUX_State[pUart->UartNum] = State;	// update state machine
	     os_AUX_DataLen[pUart->UartNum] = DataLen;	// update data length
	     os_AUX_DataCnt[pUart->UartNum] = DataCnt;	// update data cnt (the expected data LEN)
	     os_AUX_LRC[pUart->UartNum] = lrc;		// update lrc

//	     } // while(count)

	goto START;	// PATCH: 2010-09-30, keep processing the queued FIFO data
	     
//	return( result );
}
*/
// ---------------------------------------------------------------------------
// FUNCTION: AUX data link layer protocol - SOH.
//	     Package Format: SOH LEN(2) Data(n) LRC
//			     LRC = XOR(LEN..Data)
//	     Acknowledge   : ACK = 0x06, NAK = 0x15
// INPUT   : pUart
// OUTPUT  : none.
// RETURN  : TRUE  - package received ok
//           FALSE - package not available 
// NOTE	   : Edit by West for while(OS_AUX_RxReady) polling rx used
// ---------------------------------------------------------------------------
UINT32	OS_AUX_DataLinkRx_SOH( BSP_UART *pUart )
{
UINT32	State;
UINT32	count;
UINT32	DataLen = 0;
UINT32	DataCnt = 0;
UINT32	i;
UINT32	result;
UINT8	data;
UINT8	lrc = 0;
	

	DataLen = 0;
	DataCnt = 0;
	lrc = 0;

	result = FALSE;
	

	// Startup TOB process
	if( os_AUX_Para[pUart->UartNum].RxFlag == FALSE )
	  {
	  os_AUX_State[pUart->UartNum] = AUX_STATE_IDLE;
	  os_AUX_Para[pUart->UartNum].Tob = os_AUX_ParaBak[pUart->UartNum].Tob;
	  os_AUX_Para[pUart->UartNum].RxFlag = TRUE;
	  
	  os_AUX_Index[pUart->UartNum] = 0;
	  os_AUX_DataLen[pUart->UartNum] = 0;
	  os_AUX_DataCnt[pUart->UartNum] = 0;
	  os_AUX_LRC[pUart->UartNum] = 0;
	  os_AUX_RxDataBuffer[pUart->UartNum].Len = 0;
	  }

	// retrive state machine
	i = os_AUX_Index[pUart->UartNum];
	DataLen = os_AUX_DataLen[pUart->UartNum];
	DataCnt = os_AUX_DataCnt[pUart->UartNum];
	lrc = os_AUX_LRC[pUart->UartNum];
	State = os_AUX_State[pUart->UartNum];
	data=os_AUX_RxDataBuffer[pUart->UartNum].Data[i];
	// out of range?
	if( DataLen > (pUart->BufferSize) )
	  {
	  os_AUX_RxDataBuffer[pUart->UartNum].Len = 0;
	  
	  os_AUX_Para[pUart->UartNum].RxFlag = FALSE;
	  os_AUX_State[pUart->UartNum] = AUX_STATE_IDLE;
	  
//	  while( BSP_UART_Read( pUart, (UINT8 *)&data, 1 ) );	// get rid of all incoming data
	  return( result );
	  }
	
	//count = BSP_UART_Read( pUart, &data, 1 );	// read one byte
	     switch( State )
	           {
	           case AUX_STATE_IDLE:
	      
	                if( data == SOH )				// check leading char
	                  {
	                  os_AUX_RxDataBuffer[pUart->UartNum].Len = 0;	// reset final length of package
	                  
	                  DataLen = 0;
	                  DataCnt = 0;
	                  os_AUX_DataLen[pUart->UartNum] = 0;		// reset data length
	                  
	                  lrc = 0;
	                  os_AUX_LRC[pUart->UartNum] = 0;		// reset LRC
	                  
	                  i = 0;					// reset index
	                          
	                  State = AUX_STATE_LEN_L;
	                  }
	             
	                break;
	           
	           case AUX_STATE_LEN_L:
	      

	                  DataCnt = data;				// char count (L)
	                  lrc = data;           			// init LRC
	                  if( os_SOH_LEN_TYPE == 0 )
	                    State = AUX_STATE_LEN_H;
	                  else
	                    State = AUX_STATE_LEN_L2;
	      
	                break;
	           
	           case AUX_STATE_LEN_L2:
	                
	                lrc ^= data;
	                DataCnt += data*0x100;
	                
	                State = AUX_STATE_LEN_H;
	                
	                break;
	           
	           case AUX_STATE_LEN_H:
	      
	      		lrc ^= data;
	      		
			if( os_SOH_LEN_TYPE == 0 )
			  {
	      		  DataCnt += data*256;				// char count (H)
	      		  
	      		  if( DataCnt != 0 )
	                    State = AUX_STATE_DATA;
	                  else
	                    State = AUX_STATE_IDLE;			// invalid length
	                  }
	                else
	                  {
	                  DataCnt += data*0x10000;
	                  State = AUX_STATE_LEN_H2;
	                  }
	      
	                break;
	           
	           case AUX_STATE_LEN_H2:
			
			lrc ^= data;
			
			DataCnt += data*0x1000000;
			
	      		if( DataCnt != 0 )
	                  State = AUX_STATE_DATA;
	                else
	                  State = AUX_STATE_IDLE;			// invalid length
			
			break;
			
	           case AUX_STATE_DATA:
	      
	                lrc ^= data;					// check sum

			os_AUX_RxDataBuffer[pUart->UartNum].Data[i] = data;
			
	                DataLen++;					// RX_LEN++
	                i++;						// next byte
			
			if( DataLen == DataCnt )			// end of data field?
			  State = AUX_STATE_LRC;
	             
	                break;
	           
	           case AUX_STATE_LRC:
//	      			printf("\nlrc=%d\n",lrc);
	                if( data == lrc )				// good LRC (ACK will be sent after API reading)
	                  {
	                  os_AUX_RxDataBuffer[pUart->UartNum].Len = DataLen;	// put LEN & DATA without leading header
//	                  memmove( os_AUX_RxDataBuffer[pUart->UartNum].Data, &os_AUX_RxDataBuffer[pUart->UartNum].Data[2+1], DataLen );
	                  
	                  OS_AUX_DataLinkAck_DLL( pUart, ACK );		// 2009-10-30
	                  result = TRUE;
//					  return result;
	                  }
	                else
	                  {	// bad LRC, NAK to sender
	                  OS_AUX_DataLinkAck_DLL( pUart, NAK );
	                  }
	           	
	           	os_AUX_Para[pUart->UartNum].RxFlag = FALSE;
	           	State = AUX_STATE_IDLE;
	           	
//	                count = 0;
	           
	                break;
	           
	           default:
	           
//	                count = 0;
	                break;
	                
	           } // switch()
	     


	     os_AUX_Index[pUart->UartNum] = i;		// update data index
	     os_AUX_State[pUart->UartNum] = State;	// update state machine
	     os_AUX_DataLen[pUart->UartNum] = DataLen;	// update data length
	     os_AUX_DataCnt[pUart->UartNum] = DataCnt;	// update data cnt (the expected data LEN)
	     os_AUX_LRC[pUart->UartNum] = lrc;		// update lrc



	//goto START;	// PATCH: 2010-09-30, keep processing the queued FIFO data
	     
	return( result );
}
// ---------------------------------------------------------------------------
// FUNCTION: AUX data transmission by link layer format - SOH.
//	     Package Format: SOH LEN(2) Data(n) LRC
//			     LRC = XOR(LEN..Data)
//	     Acknowledge   : ACK = 0x06, NAK = 0x15
// INPUT   : pUart
//           pData  - (LL-V)
//	     Action - 0 = build package and send, 1 = resend the last package.
// OUTPUT  : none.
// RETURN  : TRUE  - package transmitted ok.
//           FALSE - package transmitted failed. 
// ---------------------------------------------------------------------------
UINT32	OS_AUX_DataLinkTx_SOH( BSP_UART *pUart, UINT8 *pData )
{
UINT32	i;
UINT32	index;
UINT32	len;
UINT8	len_l, len_h;
UINT8	len_l2, len_h2;
UINT8	lrc;

	
  	// --- Build package ---
	index = 0;
	os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = SOH;	// SOH
	
	len_l = *pData++;
	os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = len_l;	// LEN_L
	lrc = len_l;			// init LRC
	if( os_SOH_LEN_TYPE == 0 )
	  {  
	  len_h = *pData++;		// PATCH: 2009-11-01
	  os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = len_h;	// LEN_H
	  lrc ^= len_h;
	  len = len_l + (len_h*256);	// total LEN
	  }
	else
	  {
	  len_l2 = *pData++;
	  os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = len_l2;
	  lrc ^= len_l2;

	  len_h = *pData++;
	  os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = len_h;
	  lrc ^= len_h;
	  
	  len_h2 = *pData++;
	  os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = len_h2;
	  lrc ^= len_h2;
	  
	  len = len_l + len_l2*0x100 + len_h*0x10000 + len_h2*0x1000000; 
	  }
	
//	pData += 1;							// Data(n)
	for( i=0; i<len; i++ )
	   {
	   lrc ^= *pData;
	   os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = *pData++;
	   }
	   
	os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = lrc;	// LRC
	
	os_AUX_TxDataBuffer[pUart->UartNum].Len = index;		// total package size
	
	// --- Send package to UARTx ---
	 os_AUX_Para[pUart->UartNum].Resend = os_AUX_ParaBak[pUart->UartNum].Resend;	// restore RESEND cnt
	
//	os_AUX_Para[pUart->UartNum].Acks = os_AUX_ParaBak[pUart->UartNum].Acks;		// restore ACKS cnt
//	os_AUX_Para[pUart->UartNum].Tor = os_AUX_ParaBak[pUart->UartNum].Tor;		// restore TOR
	
	os_AUX_State[pUart->UartNum] = AUX_STATE_WAIT_ACK;				// change state to WAIT_ACK
	
	return( OS_AUX_SendPackage( pUart ) );
}
