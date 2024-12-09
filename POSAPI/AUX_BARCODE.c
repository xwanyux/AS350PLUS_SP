//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L (32-bit Platform)                                     **
//**  PRODUCT  : AS330		                                            **
//**                                                                        **
//**  FILE     : AUX_BARCODE.C                                              **
//**  MODULE   : OS_AUX_DataLinkTx_BARCODE()				    **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2010/02/03                                                 **
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
// NOTE	   : This function is not supported due to barcode reader is an one-way device.
// ---------------------------------------------------------------------------
//UINT32	AUX_BARCODE_TxReady( BSP_UART *pUart )
//{
//}

// ---------------------------------------------------------------------------
// FUNCTION: AUX data transmission by link layer format - BYPASS.
//	     Package Format: none.
// INPUT   : pUart
//           pData  - (LL-V)
// OUTPUT  : none.
// RETURN  : TRUE  - package transmitted ok.
//           FALSE - package transmitted failed. 
// NOTE	   : This function is not supported due to barcode reader is an one-way device.
// ---------------------------------------------------------------------------
//UINT32	OS_AUX_DataLinkTx_BARCODE( BSP_UART *pUart, UINT8 *pData )
//{
//}

// ---------------------------------------------------------------------------
// FUNCTION: Task manager of AUX data link layer TOB reached.
// INPUT   : port - port number.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_AUX_DataLinkRxTOB_BARCODE( UINT32 port )
{
	    os_AUX_RxDataBuffer[port].Len = os_AUX_DataLen[port];
	    os_AUX_Para[port].RxFlag = FALSE;
}

// ---------------------------------------------------------------------------
// FUNCTION: AUX data link layer protocol - BARCODE.
//	     Package Format: BARCODE_STRING <CR>
// INPUT   : pUart
// OUTPUT  : none.
// RETURN  : TRUE  - package received ok
//           FALSE - package not available 
// ---------------------------------------------------------------------------
UINT32	OS_AUX_DataLinkRx_BARCODE( BSP_UART *pUart )
{
UINT32	count;
UINT32	result;
UINT32	i;
UINT32	DataLen = 0;
UINT8	data;
	

START:
	DataLen = 0;

	result = FALSE;

	if( BSP_UART_GetRxAvail( pUart ) == 0 )	// skip non-RxD events
	  return( result );

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
	
	os_AUX_RxDataBuffer[pUart->UartNum].Data[i] = data;
	
	DataLen++;					// RX_LEN++
	i++;						// next byte
	
	  // TOB or 'CR' reached
	  if( data == CR )	// CR?
	    {
	    os_AUX_RxDataBuffer[pUart->UartNum].Len = DataLen -1 ;	// put LEN of DATA without CR
	    
	    os_AUX_Para[pUart->UartNum].RxFlag = FALSE;
	  
	    result = TRUE;
	    }
	
	os_AUX_Index[pUart->UartNum] = i;		// update data index
	os_AUX_DataLen[pUart->UartNum] = DataLen;	// update data length

	goto START;	// PATCH: 2010-09-30, keep processing the queued FIFO data
 
//	return( result );
}
