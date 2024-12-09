//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L (32-bit Platform)                                     **
//**  PRODUCT  : AS330/QS200                                                **
//**                                                                        **
//**  FILE     : AUX_DLL.C                                                  **
//**  MODULE   : OS_AUX_DataLinkRxDLL()					    **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2007/09/17                                                 **
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

//extern	UINT32		OS_AUX_DataLinkRx_DLL( BSP_UART *pUart );
//extern	UINT32		OS_AUX_DataLinkTx_DLL( BSP_UART *pUart, UINT8 *pData );
//extern	UINT32		OS_AUX_DataLinkAck_DLL( BSP_UART *pUart, UINT8 Code );
//extern	void		OS_AUX_DataLinkWaitAck_DLL( BSP_UART *pUart );

extern	UINT32	OS_AUX_SendPackage( BSP_UART *pUart );
extern	UINT32	OS_AUX_ResendPackage( BSP_UART *pUart );


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

extern	UINT32		os_AUX_NakFlag[];

extern	UINT32		os_DLL_ACK_MODE;


// ---------------------------------------------------------------------------
// FUNCTION: To determine whether AUX port is ready to transmit the next data string.
// INPUT   : pUart -- pointer to the BSP_UART structure.
// OUTPUT  : none.
// RETURN  : AUX_STATUS_FREE
//           AUX_STATUS_ERROR
//           AUX_STATUS_BUSY
// ---------------------------------------------------------------------------
UINT32	AUX_DLL_TxReady( BSP_UART *pUart )
{
	if( os_AUX_Para[pUart->UartNum].Acks )
	  return( os_AUX_Status[pUart->UartNum] );

	return( pUart->TxAccess );
}

// ---------------------------------------------------------------------------
// FUNCTION: Task manager of AUX data link layer TOB reached.
// INPUT   : port - port number.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_AUX_DataLinkRxTOB_DLL( UINT32 port )
{
	    os_AUX_RxDataBuffer[port].Len = 0;
	    os_AUX_Para[port].RxFlag = FALSE;
	    os_AUX_State[port] = AUX_STATE_IDLE;
}

// ---------------------------------------------------------------------------
// FUNCTION: AUX data link layer protocol - DLL.
// Package Format: SOH LEN(1) Data(n) LRC
//			     LRC = XOR(LEN..Data)
//	     Acknowledge   : ACK = 0x06, NAK = 0x15
// INPUT   : pUart
// OUTPUT  : none.
// RETURN  : TRUE  - package received ok
//           FALSE - package not available 
// ---------------------------------------------------------------------------
/*
UINT32	OS_AUX_DataLinkRx_DLL( BSP_UART *pUart )
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
//	DataLen = *(pUart->pRd + 1);		// get data LEN
//	if( count != (DataLen + 3) )		// whole package received?
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

	                  State = AUX_STATE_LEN;
	                  }
	             
	                break;
	           
	           case AUX_STATE_LEN:
	      
	                if( data == 0 )
	                  State = AUX_STATE_IDLE;			// invalid length
	                else
	                  {
	                  DataCnt = data;				// char count
	                  lrc = data;           			// init LRC
	             
	                  State = AUX_STATE_DATA;
	                  }
	      
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
//	                  memmove( os_AUX_RxDataBuffer[pUart->UartNum].Data, &os_AUX_RxDataBuffer[pUart->UartNum].Data[2], DataLen );
	                  
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
// FUNCTION: AUX data link layer protocol - DLL.
// Package Format: SOH LEN(1) Data(n) LRC
//			     LRC = XOR(LEN..Data)
//	     Acknowledge   : ACK = 0x06, NAK = 0x15
// INPUT   : pUart
// OUTPUT  : none.
// RETURN  : TRUE  - package received ok
//           FALSE - package not available 
// NOTE	   : Edit by West for while(OS_AUX_RxReady) polling rx used
// ---------------------------------------------------------------------------

UINT32	OS_AUX_DataLinkRx_DLL( BSP_UART *pUart )
{
UINT32	State;
UINT32	count;
UINT32	DataLen = 0;
UINT32	DataCnt = 0;
UINT32	i;
UINT32	result;
UINT8	data;
UINT8	lrc = 0;
	

//START:
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
	  
	//   while( BSP_UART_Read( pUart, (UINT8 *)&data, 1 ) );	// get rid of all incoming data
	  return( result );
	  }
		// printf("State=%d data=%d DataLen=%d DataCnt=%d\n",State,data,DataLen,DataCnt);	
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

	                  State = AUX_STATE_LEN;
	                  }
	             
	                break;
	           
	           case AUX_STATE_LEN:
	      
	                if( data == 0 )
	                  State = AUX_STATE_IDLE;			// invalid length
	                else
	                  {
	                  DataCnt = data;				// char count
	                  lrc = data;           			// init LRC
	                  State = AUX_STATE_DATA;
	                  }
	      
	                break;
	           
	           case AUX_STATE_DATA:
	      
	                lrc ^= data;					// check sum

			os_AUX_RxDataBuffer[pUart->UartNum].Data[i] = data;
			
	                DataLen++;					// RX_LEN++
	                i++;						// next byte
			
			if( DataLen == DataCnt )			// end of data field?
			{
			  State = AUX_STATE_LRC;
			}
	                break;
	           
	           case AUX_STATE_LRC:
	      
	                if( data == lrc )				// good LRC (ACK will be sent after API reading)
	                  {
	                  os_AUX_RxDataBuffer[pUart->UartNum].Len = DataLen;	// put LEN & DATA without leading header
//	                  memmove( os_AUX_RxDataBuffer[pUart->UartNum].Data, &os_AUX_RxDataBuffer[pUart->UartNum].Data[2], DataLen );
	                  
	                  OS_AUX_DataLinkAck_DLL( pUart, ACK );		// 2009-10-30
	                  result = TRUE;
					  // return( result );
					  goto After_LRC;
	                  }
	                else
	                  {	// bad LRC, NAK to sender
	                  OS_AUX_DataLinkAck_DLL( pUart, NAK );
	                  }
After_LRC:
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
// FUNCTION: Respond ACK to the sender according to DLL protocol.
// INPUT   : pUart -- pointer to the BSP_UART structure.
//           Code  -- ACK or NAK.
// OUTPUT  : none.
// RETURN  : TRUE  - OK.
//           FALSE - device failed or busy.
// ---------------------------------------------------------------------------
UINT32	OS_AUX_DataLinkAck_DLL( BSP_UART *pUart, UINT8 Code )
{
UINT32	acks;
UINT8	data[MAX_AUX_ACKS];	// max. 8 ACKs


	if( os_DLL_ACK_MODE && (Code == ACK) )	// 2015-03-24
	  return( TRUE );	// do nothing if AFTER-SERVICE and ACK by APP itself

	acks = os_AUX_ParaBak[pUart->UartNum].Acks;	// repetitive count
	
	if( acks == 0 )
	  return( TRUE );
	
	if( Code == NAK )	// 2009-10-30, send only one if NAK
	  acks = 1;
	  
	if( acks > MAX_AUX_ACKS )
	  acks = MAX_AUX_ACKS;
	
	memset( data, Code, acks );
	if( BSP_UART_Write( pUart, data, acks ) == BSP_SUCCESS )
	  return( TRUE );
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: Wait for ACK from the sender according to DLL protocol.
// INPUT   : pUart -- pointer to the BSP_UART structure.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_AUX_DataLinkWaitAck_DLL( BSP_UART *pUart )
{
UINT8	data;
	// printf("os_AUX_State[pUart->UartNum]=%d pUart->Avail=%d\n",os_AUX_State[pUart->UartNum],pUart->Avail);
	if( os_AUX_State[pUart->UartNum] == AUX_STATE_WAIT_ACK )
	  {
	  if( pUart->Avail != 0 )
	    {
	    os_AUX_Para[pUart->UartNum].TxFlag = FALSE;	// disable TimeOut task
	    
	    data = *pUart->pRd;				// preview data
		// printf("data=0x%02x\n",data);
	    switch( data )
	    	  {
	    	  case ACK:
	    	       
	    	       BSP_UART_Read( pUart, (UINT8 *)&data, 1 );	// flush ACK
	    	       
	    	       os_AUX_State[pUart->UartNum] = AUX_STATE_IDLE;
	    	       os_AUX_Status[pUart->UartNum] = AUX_STATUS_FREE;
	    	       
	    	       break;
	    	       
	    	  case NAK:
	    	  	    	  
	    	       BSP_UART_Read( pUart, (UINT8 *)&data, 1 );	// flush NAK
	    	       
	    	       if( os_AUX_NakFlag[pUart->UartNum] != 0 )	// NAK ever received?
			 os_AUX_Para[pUart->UartNum].TxFlag = TRUE;	// enable TimeOut task again for the next retry
	    	       else
	    	         {
	    	         os_AUX_NakFlag[pUart->UartNum] = 1;		// set NAK ever received flag
	    	         
	    	         if( OS_AUX_ResendPackage( pUart ) == FALSE )	// resend last package
	    	           os_AUX_Status[pUart->UartNum] = AUX_STATUS_ERROR;
	    	         }
	    	       
	    	       break;
	    	       
	    	  case SOH:	// don't flush SOH, it seems to ACK the previous package
	    	  	    	  
	    	       os_AUX_State[pUart->UartNum] = AUX_STATE_IDLE;
	    	       os_AUX_Status[pUart->UartNum] = AUX_STATUS_FREE;
	    	       
	    	       os_AUX_Para[pUart->UartNum].RxFlag = FALSE;	// PATCH: 2009-11-05
	    	       OS_AUX_DataLinkRx_DLL( pUart );			//
	    	       
	    	       break;
	    	       
	    	  default:
	    	  
	    	       BSP_UART_Read( pUart, (UINT8 *)&data, 1 );	// flush unknown acknowledge
	    	       
	    	       os_AUX_Para[pUart->UartNum].TxFlag = TRUE;	// enable TimeOut task again for the next retry
		       
	    	       break;
	    	  }
	    }
	  }
}

// ---------------------------------------------------------------------------
// FUNCTION: AUX data transmission by link layer format - DLL.
//	     Package Format: SOH LEN(1) Data(n) LRC
//			     LRC = XOR(LEN..Data)
//	     Acknowledge   : ACK = 0x06, NAK = 0x15
// INPUT   : pUart
//           pData  - (LL-V)
//	     Action - 0 = build package and send, 1 = resend the last package.
// OUTPUT  : none.
// RETURN  : TRUE  - package transmitted ok.
//           FALSE - package transmitted failed. 
// ---------------------------------------------------------------------------
UINT32	OS_AUX_DataLinkTx_DLL( BSP_UART *pUart, UINT8 *pData )
{
UINT32	i;
UINT32	index;
UINT32	len;
UINT8	lrc;

	
  	// --- Build package ---
	index = 0;
	os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = SOH;	// SOH
	
	len = *pData;
	lrc = len;	// init LRC
	os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = len;	// LEN
	
	pData += 2;							// Data(n)
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
