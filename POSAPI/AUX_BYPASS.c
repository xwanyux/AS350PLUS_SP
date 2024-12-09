//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L (32-bit Platform)                                     **
//**  PRODUCT  : AS330		                                            **
//**                                                                        **
//**  FILE     : AUX_BYPASS.C                                               **
//**  MODULE   : OS_AUX_DataLinkTx_BYPASS()				    **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2009/08/26                                                 **
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
extern	UINT32		os_AUX_Index[];
extern	UINT32		os_AUX_DataLen[];
extern	UINT32		os_AUX_ExpLen[];
extern	UINT32		os_AUX_State[];

// ---------------------------------------------------------------------------
// FUNCTION: To determine whether AUX port is ready to transmit the next data string.
// INPUT   : pUart -- pointer to the BSP_UART structure.
// OUTPUT  : none.
// RETURN  : AUX_STATUS_FREE
//           AUX_STATUS_ERROR
//           AUX_STATUS_BUSY
// ---------------------------------------------------------------------------
UINT32	AUX_BYPASS_TxReady( BSP_UART *pUart )
{
	if( pUart->TxAccess==AUX_STATUS_FREE)
	  return( AUX_STATUS_FREE );
	else
	  return( AUX_STATUS_BUSY );
}

// ---------------------------------------------------------------------------
// FUNCTION: AUX data transmission by link layer format - BYPASS.
//	     Package Format: none.
// INPUT   : pUart
//           pData  - (LL-V)
// OUTPUT  : none.
// RETURN  : TRUE  - package transmitted ok.
//           FALSE - package transmitted failed. 
// ---------------------------------------------------------------------------
UINT32	OS_AUX_DataLinkTx_BYPASS( BSP_UART *pUart, UINT8 *pData )
{
UINT32	i;
UINT32	index;
UINT32	len;

	
	len = pData[0] + pData[1]*256;
	pData += 2;
	
	for( i=0; i<len; i++ )
	   os_AUX_TxDataBuffer[pUart->UartNum].Data[i] = *pData++;
	   
	os_AUX_TxDataBuffer[pUart->UartNum].Len = len;	// total package size
	
	// --- Send package to UARTx ---
//	os_AUX_Para[pUart->UartNum].Resend = os_AUX_ParaBak[pUart->UartNum].Resend;	// restore RESEND cnt
	
//	os_AUX_Para[pUart->UartNum].Acks = os_AUX_ParaBak[pUart->UartNum].Acks;		// restore ACKS cnt
//	os_AUX_Para[pUart->UartNum].Tor = os_AUX_ParaBak[pUart->UartNum].Tor;		// restore TOR
	
//	os_AUX_State[pUart->UartNum] = AUX_STATE_WAIT_ACK;				// change state to WAIT_ACK
	
	return( OS_AUX_SendPackage( pUart ) );
}

// ---------------------------------------------------------------------------
// FUNCTION: Task manager of AUX data link layer TOB reached.
// INPUT   : port - port number.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_AUX_DataLinkRxTOB_BYPASS( UINT32 port )
{
		if(os_AUX_ExpLen[port]==0)
	    os_AUX_RxDataBuffer[port].Len = os_AUX_DataLen[port];
	
	    os_AUX_Para[port].RxFlag = FALSE;
}

// ---------------------------------------------------------------------------
// FUNCTION: AUX data link layer protocol - BYPASS.
//	     Package Format: none.
// INPUT   : pUart
// OUTPUT  : none.
// RETURN  : TRUE  - package received ok
//           FALSE - package not available 
// ---------------------------------------------------------------------------
/*
UINT32	OS_AUX_DataLinkRx_BYPASS( BSP_UART *pUart )
{
UINT32	count;
UINT32	result;
UINT32	i;
UINT32	DataLen = 0;
UINT8	data;
	

START:
	DataLen = 0;

	result = FALSE;

	// if( BSP_UART_GetRxAvail( pUart ) == 0 )	// skip non-RxD events
	  // return( result );

//	count = BSP_UART_GetRxAvail( pUart );	// min. package length
//	if( count )
//	  {
	  // Startup TOB process
	  if( os_AUX_Para[pUart->UartNum].RxFlag == FALSE )
	    {
	    os_AUX_State[pUart->UartNum] = AUX_STATE_IDLE;
	    os_AUX_Para[pUart->UartNum].Tob = os_AUX_ParaBak[pUart->UartNum].Tob;
	    os_AUX_Para[pUart->UartNum].RxFlag = TRUE;
	    
	    os_AUX_Index[pUart->UartNum] = 0;
	    os_AUX_DataLen[pUart->UartNum] = 0;
	    
	    os_AUX_RxDataBuffer[pUart->UartNum].Len = 0;
	    }
	
	// retrive state machine
	i = os_AUX_Index[pUart->UartNum];
	DataLen = os_AUX_DataLen[pUart->UartNum];
	
	// out of range?
	if( DataLen > (pUart->BufferSize) )
	  {
	  os_AUX_RxDataBuffer[pUart->UartNum].Len = 0;
	  
	  os_AUX_Para[pUart->UartNum].RxFlag = FALSE;
	  
	  while( BSP_UART_Read( pUart, (UINT8 *)&data, 1 ) );	// get rid of all incoming data
	  return( result );
	  }
	
	count = BSP_UART_Read( pUart, &data, 1 );	// read one byte
	if(count==0)
		goto START;
	
	os_AUX_RxDataBuffer[pUart->UartNum].Data[i] = data;
	
	DataLen++;					// RX_LEN++
	i++;						// next byte
	
	  // TOB or expected LEN reached
	  if( (OS_AUX_TaskCheckTob( pUart ) == TRUE) || ((os_AUX_ExpLen[pUart->UartNum]) && (DataLen >= os_AUX_ExpLen[pUart->UartNum])) )
	  //if( (os_AUX_ExpLen[pUart->UartNum]) && (DataLen >= os_AUX_ExpLen[pUart->UartNum]) )
	    {
	    os_AUX_RxDataBuffer[pUart->UartNum].Len = DataLen;	// put LEN & DATA without leading header
	    
	    os_AUX_Para[pUart->UartNum].RxFlag = FALSE;
	  
//	    count = BSP_UART_Read( pUart, os_AUX_RxDataBuffer[pUart->UartNum].Data, pUart->BufferSize );	// read all available data
//	    os_AUX_RxDataBuffer[pUart->UartNum].Len = count;	// actual length of data read
	  
	    result = TRUE;
		return result;
	    }
	    
//	  }
	
	os_AUX_Index[pUart->UartNum] = i;		// update data index
	os_AUX_DataLen[pUart->UartNum] = DataLen;	// update data length

	goto START;	// PATCH: 2010-09-30, keep processing the queued FIFO data
	  
//	return( result );
}
*/
// ---------------------------------------------------------------------------
// FUNCTION: AUX data link layer protocol - BYPASS.
//	     Package Format: none.
// INPUT   : pUart
// OUTPUT  : none.
// RETURN  : TRUE  - package received ok
//           FALSE - package not available 
// NOTE	   : Edit by West for while(OS_AUX_RxReady) polling rx used
// ---------------------------------------------------------------------------
UINT32	OS_AUX_DataLinkRx_BYPASS( BSP_UART *pUart )
{
UINT32	count;
UINT32	result;
UINT32	DataLen = 0;
UINT8	data;
	

START:
	DataLen = os_AUX_DataLen[pUart->UartNum];

	result = FALSE;


	
	/*
	// TOB or expected LEN reached
	if( ((OS_AUX_TaskCheckTob( pUart ) == TRUE)&&(os_AUX_DataLen[pUart->UartNum]!=0)) || ((os_AUX_ExpLen[pUart->UartNum]) && (DataLen >= os_AUX_ExpLen[pUart->UartNum])) )
	 //if( (os_AUX_ExpLen[pUart->UartNum]) && (DataLen >= os_AUX_ExpLen[pUart->UartNum]) )
	   {
	   os_AUX_RxDataBuffer[pUart->UartNum].Len = DataLen;	// put LEN & DATA without leading header
	   
	   os_AUX_Para[pUart->UartNum].RxFlag = FALSE;
	 
 
		result = TRUE;
		return result;
	   }
	 */
	 
	if( (os_AUX_Para[pUart->UartNum].RxFlag == FALSE) && (os_AUX_ExpLen[pUart->UartNum]==0) )
	    {
	    os_AUX_State[pUart->UartNum] = AUX_STATE_IDLE;
	    os_AUX_Para[pUart->UartNum].Tob = os_AUX_ParaBak[pUart->UartNum].Tob;
	    os_AUX_Para[pUart->UartNum].RxFlag = TRUE;
	    
	    //os_AUX_Index[pUart->UartNum] = 0;
	    //os_AUX_DataLen[pUart->UartNum] = 0;
	    //os_AUX_RxDataBuffer[pUart->UartNum].Data[i]=0;
	    os_AUX_RxDataBuffer[pUart->UartNum].Len = 0;
	    }
		
	// retrive state machine
	
	
	DataLen++;					// RX_LEN++

	// out of range?
	if( DataLen > (pUart->BufferSize) )
	  {
	  os_AUX_RxDataBuffer[pUart->UartNum].Len = 0;
	  
	  os_AUX_Para[pUart->UartNum].RxFlag = FALSE;
	  
	  while( BSP_UART_Read( pUart, (UINT8 *)&data, 1 ) );	// get rid of all incoming data
	  return( result );
	  }
	  
	os_AUX_DataLen[pUart->UartNum] = DataLen;	// update data length
	os_AUX_Index[pUart->UartNum]++;  
	
	if(os_AUX_ExpLen[pUart->UartNum]>0)
	{
		if(DataLen >= os_AUX_ExpLen[pUart->UartNum])
		{
			
			os_AUX_RxDataBuffer[pUart->UartNum].Len = DataLen;	// put LEN & DATA without leading header	   
			os_AUX_Para[pUart->UartNum].RxFlag = FALSE; 
			result = TRUE;
			return result;
		}
	}  
	    
	return( result );
}