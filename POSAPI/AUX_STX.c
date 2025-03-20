//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L (32-bit Platform)                                     **
//**  PRODUCT  : AS330                                                      **
//**                                                                        **
//**  FILE     : AUX_STX.C                                                  **
//**  MODULE   : 							    **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2009/08/28                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2009 SymLink Corporation. All rights reserved.           **
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

extern	UINT32		os_AUX_NakFlag[];


// ---------------------------------------------------------------------------
// FUNCTION: Task manager of AUX data link layer TOB reached.
// INPUT   : port - port number.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_AUX_DataLinkRxTOB_STX( UINT32 port )
{
	    os_AUX_RxDataBuffer[port].Len = 0;
	    os_AUX_Para[port].RxFlag = FALSE;
	    os_AUX_State[port] = AUX_STATE_IDLE;
}

// ---------------------------------------------------------------------------
// FUNCTION: AUX data link layer protocol - STX.
//	     Package Format: STX Data ETX LRC
//			     LRC = XOR(Data..ETX)
//	     Acknowledge   : ACK = 0x06, NAK = 0x15
// INPUT   : pUart
// OUTPUT  : none.
// RETURN  : TRUE  - package received ok
//           FALSE - package not available 
// ---------------------------------------------------------------------------
/*
UINT32	OS_AUX_DataLinkRx_STX( BSP_UART *pUart )
{
UINT32	State;
UINT32	count;
UINT32	DataLen = 0;
UINT32	i;
UINT32	result;
UINT8	data;
UINT8	lrc = 0;


START:
	DataLen = 0;
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
	  os_AUX_LRC[pUart->UartNum] = 0;
	  
	  os_AUX_RxDataBuffer[pUart->UartNum].Len = 0;
	  }

	// retrive state machine
	i = os_AUX_Index[pUart->UartNum];
	DataLen = os_AUX_DataLen[pUart->UartNum];
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
	
	// Min. data received?
//	if( count < 4 )
//	  return( result );
	
	// Filter out all non-STX data one by one
//	while( *pUart->pRd != STX )
//	     {
//	     if( BSP_UART_Read( pUart, (UINT8 *)&data, 1 ) == 0 )
//	       return( result );
//	     }
	     
//	count = BSP_UART_GetRxAvail( pUart );	// min. package length
//	if( count < 4 )
//	  return( result );

//	DataLen = *(pUart->pRd + 1);		// get data LEN
//	if( count != (DataLen + 3) )		// whole package received?
//	  return( result );
	
	// --- Protocol State Machine ---
//	i = os_AUX_Index[pUart->UartNum];
//	DataLen = os_AUX_DataLen[pUart->UartNum];
//	lrc = os_AUX_LRC[pUart->UartNum];
//	State = os_AUX_State[pUart->UartNum];
	  
//	count = BSP_UART_Read( pUart, &os_AUX_RxDataBuffer[pUart->UartNum].Data[i], 1 );	// read one byte
	count = BSP_UART_Read( pUart, &data, 1 );	// read one byte
//	while( count-- )
//	     {
	     
	     switch( State )
	           {
	           case AUX_STATE_IDLE:
	      
	                if( data == STX )				// check leading char
	                  {
	                  os_AUX_RxDataBuffer[pUart->UartNum].Len = 0;	// reset final length of package
	                  
	                  DataLen = 0;
	                  os_AUX_DataLen[pUart->UartNum] = 0;		// reset data length
	                  
	                  lrc = 0;
	                  os_AUX_LRC[pUart->UartNum] = 0;		// reset LRC
	                  
	                  i = 0;					// reset index
	             
	                  State = AUX_STATE_DATA;
	                  }
	             
	                break;
	           	           
	           case AUX_STATE_DATA:
	      
	                lrc ^= data;					// check sum
	                
	                if( data == ETX )
	                  State = AUX_STATE_LRC;
	                else
	                  {
	                  os_AUX_RxDataBuffer[pUart->UartNum].Data[i] = data;
	                  
	                  DataLen++;					// RX_LEN++
	                  i++;						// next byte
	                  }
	           
//	                if( i > (pUart->BufferSize ) )			// out of range, reset state
//	                  {
//	                  os_AUX_Para[pUart->UartNum].RxFlag = FALSE;
//	           	  State = AUX_STATE_IDLE;
//	           	  
//	                  count = 0;
//	                  }

	                break;
	           
	           case AUX_STATE_LRC:
	      
	                if( data == lrc )				// good LRC (ACK will be sent after API reading)
	                  {
	                  os_AUX_RxDataBuffer[pUart->UartNum].Len = DataLen;	// put LEN & DATA without leading header
//	                  memmove( os_AUX_RxDataBuffer[pUart->UartNum].Data, &os_AUX_RxDataBuffer[pUart->UartNum].Data[1], DataLen );

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
	     os_AUX_LRC[pUart->UartNum] = lrc;		// update lrc
	     
//	     } // while(count)

	goto START;	// PATCH: 2010-09-30, keep processing the queued FIFO data
	     
//	return( result );
}
*/

// ---------------------------------------------------------------------------
// FUNCTION: AUX data link layer protocol - STX.
//	     Package Format: STX Data ETX LRC
//			     LRC = XOR(Data..ETX)
//	     Acknowledge   : ACK = 0x06, NAK = 0x15
// INPUT   : pUart
// OUTPUT  : none.
// RETURN  : TRUE  - package received ok
//           FALSE - package not available 
// NOTE	   : Edit by West for while(OS_AUX_RxReady) polling rx used
// ---------------------------------------------------------------------------

UINT32	OS_AUX_DataLinkRx_STX( BSP_UART *pUart )
{
UINT32	State;
UINT32	count;
UINT32	DataLen = 0;
UINT32	i;
UINT32	result;
UINT8	data;
UINT8	lrc = 0;


//START:
	DataLen = 0;
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
	  os_AUX_LRC[pUart->UartNum] = 0;
	  os_AUX_RxDataBuffer[pUart->UartNum].Len = 0;
	  }

	// retrive state machine
	i = os_AUX_Index[pUart->UartNum];
	DataLen = os_AUX_DataLen[pUart->UartNum];
	lrc = os_AUX_LRC[pUart->UartNum];
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
	     
	     switch( State )
	           {
	           case AUX_STATE_IDLE:
	      
	                if( data == STX )				// check leading char
	                  {
	                  os_AUX_RxDataBuffer[pUart->UartNum].Len = 0;	// reset final length of package
	                  
	                  DataLen = 0;
	                  os_AUX_DataLen[pUart->UartNum] = 0;		// reset data length
	                  
	                  lrc = 0;
	                  os_AUX_LRC[pUart->UartNum] = 0;		// reset LRC
	                  
	                  i = 0;					// reset index
	             
	                  State = AUX_STATE_DATA;
	                  }
	             
	                break;
	           	           
	           case AUX_STATE_DATA:
	      
	                lrc ^= data;					// check sum
	                
	                if( data == ETX )
	                  State = AUX_STATE_LRC;
	                else
	                  {
	                  os_AUX_RxDataBuffer[pUart->UartNum].Data[i] = data;
	                  
	                  DataLen++;					// RX_LEN++
	                  i++;						// next byte
	                  }
	           
	                break;
	           
	           case AUX_STATE_LRC:
//	      			printf("data=%d  lrc=%d\n",data,lrc);
	                if( data == lrc )				// good LRC (ACK will be sent after API reading)
	                  {
	                  os_AUX_RxDataBuffer[pUart->UartNum].Len = DataLen;	// put LEN & DATA without leading header
//	                  memmove( os_AUX_RxDataBuffer[pUart->UartNum].Data, &os_AUX_RxDataBuffer[pUart->UartNum].Data[1], DataLen );

						OS_AUX_DataLinkAck_DLL( pUart, ACK );		// 2009-10-30
	                  result = TRUE;
					  return( result );
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
	     os_AUX_LRC[pUart->UartNum] = lrc;		// update lrc
	     
//	     } // while(count)

	//goto START;	// PATCH: 2010-09-30, keep processing the queued FIFO data
	     
	return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: Wait for ACK from the sender according to DLL protocol.
// INPUT   : pUart -- pointer to the BSP_UART structure.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_AUX_DataLinkWaitAck_STX( BSP_UART *pUart )
{
UINT8	data;

	if( os_AUX_State[pUart->UartNum] == AUX_STATE_WAIT_ACK )
	  {
	  if( pUart->Avail != 0 )
	    {
	    os_AUX_Para[pUart->UartNum].TxFlag = FALSE;	// disable TimeOut task
	    
	    data = *pUart->pRd;				// preview data

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
	    	       
	    	  case STX:	// don't flush STX, it seems to ACK the previous package
	    	  	    	  
	    	       os_AUX_State[pUart->UartNum] = AUX_STATE_IDLE;
	    	       os_AUX_Status[pUart->UartNum] = AUX_STATUS_FREE;
	    	       
	    	       os_AUX_Para[pUart->UartNum].RxFlag = FALSE;	// PATCH: 2009-11-05
	    	       OS_AUX_DataLinkRx_STX( pUart );			//
	    	       
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
//	     Package Format: STX Data ETX LRC
//			     LRC = XOR(Data..LRC)
//	     Acknowledge   : ACK = 0x06, NAK = 0x15
// INPUT   : pUart
//           pData  - (LL-V)
//	     Action - 0 = build package and send, 1 = resend the last package.
// OUTPUT  : none.
// RETURN  : TRUE  - package transmitted ok.
//           FALSE - package transmitted failed. 
// ---------------------------------------------------------------------------
UINT32	OS_AUX_DataLinkTx_STX( BSP_UART *pUart, UINT8 *pData )
{
UINT32	i;
UINT32	index;
UINT32	len;
UINT8	lrc;

	
  	// --- Build package ---
	index = 0;
	os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = STX;	// STX
	
	len = *pData++;
	len += (*pData++)*256;
	lrc = 0;	// init LRC
	
	for( i=0; i<len; i++ )						// Data(n)
	   {
	   lrc ^= *pData;
	   os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = *pData++;
	   }
	
	os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = ETX;	// ETX
	lrc ^= ETX;
	
	os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = lrc;	// LRC
	
	os_AUX_TxDataBuffer[pUart->UartNum].Len = index;		// total package size
	
	// --- Send package to UARTx ---
	os_AUX_Para[pUart->UartNum].Resend = os_AUX_ParaBak[pUart->UartNum].Resend;	// restore RESEND cnt
	
	os_AUX_State[pUart->UartNum] = AUX_STATE_WAIT_ACK;				// change state to WAIT_ACK
	
	return( OS_AUX_SendPackage( pUart ) );
}
