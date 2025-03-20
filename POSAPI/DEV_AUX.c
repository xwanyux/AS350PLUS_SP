//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L (32-bit Platform)                                     **
//**  PRODUCT  : AS330/QS200                                                **
//**                                                                        **
//**  FILE     : DEV_AUX.C                                                  **
//**  MODULE   : 							    **
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
#include "bsp_tmr.h"
#include "DEV_AUX.h" 
#include "POSAPI.h"
//extern	UINT32	OS_AUX_SendPackage( BSP_UART *pUart );
//extern	UINT32	OS_AUX_ResendPackage( BSP_UART *pUart );

extern		UINT32	OS_AUX_DataLinkRxManager( BSP_UART *pUart );
extern		void	OS_AUX_DataLinkRxTOB( UINT32 port );
extern 		void 	OS_EnableTimer1();
extern		BSP_UART *os_pAUX0;
extern		BSP_UART *os_pAUX1;
extern		BSP_UART *os_pAUX2;
extern		BSP_UART *os_pAUX3;

extern		UINT32	os_SOH_LEN_TYPE;
extern		UINT32	os_DLL_ACK_MODE;

extern		UCHAR	os_DHN_AUX0;
extern		UCHAR	os_DHN_AUX1;
extern		UCHAR	os_DHN_AUX2;
extern		UCHAR	os_DHN_AUX3;

API_AUX		os_AUX_Para[BSP_MAX_UARTS];		// parameters of each AUX port
API_AUX		os_AUX_ParaBak[BSP_MAX_UARTS];		// backup for AUX parameters
AUX_DATA	os_AUX_RxDataBuffer[BSP_MAX_UARTS];	// Rx interface buffer
AUX_DATA	os_AUX_TxDataBuffer[BSP_MAX_UARTS];	// Tx interface buffer

UINT32		os_AUX_State[BSP_MAX_UARTS] = {0};		// state after sending package
UINT32		os_AUX_Status[BSP_MAX_UARTS] = {0};		// status of trasmission
UINT32		os_AUX_Index[BSP_MAX_UARTS] = {0};		// data buffer index
UINT32		os_AUX_DataLen[BSP_MAX_UARTS] = {0};		// data length
UINT32		os_AUX_DataCnt[BSP_MAX_UARTS] = {0};		// run-time data counter
UINT32		os_AUX_LRC[BSP_MAX_UARTS] = {0};		// LRC
UINT32		os_AUX_ExpLen[BSP_MAX_UARTS] = {0};		// expected rx length for BYPASS mode

UINT32		os_AUX_NakFlag[BSP_MAX_UARTS] = {0};		// 0=none of NAK received, 1=at least one ACK received

UINT8		os_WAVE_SEQNO = 0;			// WAVE sequence number 0x01..0xFF
UINT8		os_WAVE_TXI_H = 0x0B;			// default TXI
UINT8		os_WAVE_TXI_L = 0x01;			//
UINT8		os_WAVE_RXI_H = 0x0E;			// default RXI
UINT8		os_WAVE_RXI_L = 0x01;			//
BSP_TIMER * pFlowControlTimer=NULL;//used to require a timer
UINT32		TimeoutCount=0;
// ---------------------------------------------------------------------------
// FUNCTION: Initialize AUX device. (called by POST)
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_AUX_Init( void )
{
UINT32	i;
	
	for( i=0; i<BSP_MAX_UARTS; i++ )
	   {
	   os_AUX_Para[i].Mode = 0xFF;
	   os_AUX_Para[i].TxFlag = FALSE;
	   os_AUX_Para[i].RxFlag = FALSE;
	   }
}

// ---------------------------------------------------------------------------
// FUNCTION: AUX data transmission by link layer format.
// INPUT   : pUart
// OUTPUT  : none.
// RETURN  : TRUE  - package transmitted ok.
//           FALSE - package transmitted failed. 
// ---------------------------------------------------------------------------
UINT32	OS_AUX_SendPackage( BSP_UART *pUart )
{
BSP_STATUS	Status;


	// PATCH: 2011-08-24, waiting for non-busy status
	do{
	  Status = BSP_UART_Write( pUart, os_AUX_TxDataBuffer[pUart->UartNum].Data, os_AUX_TxDataBuffer[pUart->UartNum].Len );
	  } while( Status == BSP_BUSY );
	
	os_AUX_NakFlag[pUart->UartNum] = 0;	// 2014-08-27
	
	// --- Send package to UARTx ---
	if(  Status == BSP_SUCCESS )
	  {
	  os_AUX_Para[pUart->UartNum].TxFlag = FALSE;
	  
	  os_AUX_Para[pUart->UartNum].Acks = os_AUX_ParaBak[pUart->UartNum].Acks;	// restore ACKS cnt
	  os_AUX_Para[pUart->UartNum].Tor = os_AUX_ParaBak[pUart->UartNum].Tor;		// restore TOR
	  
	  os_AUX_Para[pUart->UartNum].TxFlag = TRUE;
	  if(os_AUX_Para[pUart->UartNum].Mode==AUX_MODE_BYPASS)
		  pUart->TxAccess=AUX_STATUS_FREE;
	  
	  return( TRUE );
	  }
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: AUX data transmission by link layer format.
// INPUT   : pUart
// OUTPUT  : none.
// RETURN  : TRUE  - package transmitted ok.
//           FALSE - package transmitted failed. 
// ---------------------------------------------------------------------------
UINT32	OS_AUX_ResendPackage( BSP_UART *pUart )
{
UINT32	result;
UINT32	flag;

	
	if( os_AUX_Para[pUart->UartNum].Resend )
	  {
	/*
	  // PATCH: 2012-02-04
	  SDC_Disable_TxIrq( pUart );		// stop current sending process
	  pUart->TxCount = 0;			//
	  if( !(pUart->TxSem) )			//
	    BSP_SemRelease( &pUart->TxSem );	// release semaphore
	 */
	  pUart->TxAccess=AUX_STATUS_FREE;		// release semaphore
	  flag = os_AUX_NakFlag[pUart->UartNum];	// push flag
	  
	  os_AUX_Para[pUart->UartNum].Resend -= 1;	// retry--
	  result = OS_AUX_SendPackage( pUart );		// resend packet
	  
	  os_AUX_NakFlag[pUart->UartNum] = flag;	// pop flag
	  return( result );
	  }
	else
	  {
	  os_AUX_Para[pUart->UartNum].TxFlag = FALSE;	// end of resend process
//	  os_AUX_Status[pUart->UartNum] = AUX_STATUS_FREE;	// 2014-08-26, removed

	  os_AUX_State[pUart->UartNum] = 0;	// 2025-02-18
	  
	  return( FALSE );
	  }
}

// ---------------------------------------------------------------------------
// FUNCTION: Task to handle device timeout.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
//UINT TOBbak;
void	OS_AUX_TaskProcessTout( UINT32 port )
{
#if	0
BSP_UART *pUart;
	
	
	// Task for transmitting
	if( os_AUX_State[port] == AUX_STATE_WAIT_ACK )
	  {
	  if( (os_AUX_Para[port].Mode != 0xFF) && (os_AUX_Para[port].TxFlag == TRUE) )
	    {
	    if( os_AUX_Para[port].Tor ){
			// os_AUX_Para[port].Tor -= 1;
			os_AUX_Para[port].Tor -= TimeoutCount;
			if(os_AUX_Para[port].Tor>=TimeoutCount)
	      		os_AUX_Para[port].Tor -= TimeoutCount;
		    else
		  		os_AUX_Para[port].Tor=0;
		}
	    else
	      {
	      pUart = AUX_GetDHN( port );
	      if( pUart ){
	        OS_AUX_ResendPackage( pUart );	// resend last package
		  }
		 }
	    }
	  }
	else	// Task for receiving
	  {
	  if( (os_AUX_Para[port].Mode != 0xFF) && (os_AUX_Para[port].RxFlag == TRUE) )
	    {
			
			
	    if( os_AUX_Para[port].Tob )
		{
	    //   os_AUX_Para[port].Tob -= 1;
		  if(os_AUX_Para[port].Tob>=TimeoutCount)
	      	os_AUX_Para[port].Tob -= TimeoutCount;
		  else
		  	os_AUX_Para[port].Tob=0;
		  if(os_AUX_Para[port].Tob>os_AUX_ParaBak[port].Tob)//counting TOB>initial value must be overflow(unsigned 0-1)
			os_AUX_Para[port].Tob=0;
		}
	    else
	      OS_AUX_DataLinkRxTOB( port );	// 2009-11-01
	    }
	  }

#else	// 2025-02-04, using old AS350 method

BSP_UART *pUart;
	
	
	// Task for transmitting
	if( os_AUX_State[port] == AUX_STATE_WAIT_ACK )
	  {
	  if( (os_AUX_Para[port].Mode != 0xFF) && (os_AUX_Para[port].TxFlag == TRUE) )
	    {
	    if( os_AUX_Para[port].Tor )
	      os_AUX_Para[port].Tor -= 1;
	    else
	      {
	      pUart = AUX_GetDHN( port );
	      if( pUart )
	        OS_AUX_ResendPackage( pUart );	// resend last package
	      }
	    }
	  }
	else	// Task for receiving
	  {
	  if( (os_AUX_Para[port].Mode != 0xFF) && (os_AUX_Para[port].RxFlag == TRUE) )
	    {
	    if( os_AUX_Para[port].Tob )
	      os_AUX_Para[port].Tob -= 1;
	    else
	      OS_AUX_DataLinkRxTOB( port );	// 2009-11-01
	    }
	  }
#endif
}

// ---------------------------------------------------------------------------
// FUNCTION: Check if TOB is reached.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - timeout.
//           FALSE - in processing.
// ---------------------------------------------------------------------------
UINT32	OS_AUX_TaskCheckTob( BSP_UART *pUart )
{
	if( os_AUX_Para[pUart->UartNum].Tob == 0 )
	  return( TRUE );
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: Flow control of UART device. (called by system timer task)
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_AUX_FlowControl( void )
{
#if	0
UINT32	i;

	do
	{
		if(pFlowControlTimer)
		{
			if(BSP_TMR_GetTick(pFlowControlTimer,&TimeoutCount)>0)
			{
				// Protocol - DLL
				// printf("TimeoutCount=%ld\n",TimeoutCount);
				for( i=0; i<BSP_MAX_UARTS; i++ )
				   OS_AUX_TaskProcessTout( i );
			}
		}
		else
		{
			printf("OS_AUX_FlowControl pFlowControlTimer==NULL\n");
			return;
		}
			
	}while(1);
	
#else		// 2025-02-04, using old AS350 method, called by OS_Timer1Int()

UINT32	i;

	// Protocol - DLL
	for( i=0; i<BSP_MAX_UARTS; i++ )
	   OS_AUX_TaskProcessTout( i );
#endif
} 

// ---------------------------------------------------------------------------
// FUNCTION: ISR callback function for AUX.
// INPUT   : pUart -- registered UART structure.
// OUTPUT  : none.
// RETURN  : none.
// NOTE    : this isr will be triggered if
//	     1. Tx buffer is empty
//	     2. Rx buffer is available
//	     3. MODEM status changes
// ---------------------------------------------------------------------------
void	OS_AUX_IsrCallBack( BSP_UART *pUart )
{
//UINT8	data;
//UINT32	Iir;
UINT32	result;
//	Iir = BSP_RD32 (pUart->Base|UART_REG_IIR) & IIR_INT_STATUS_MSK;
//	if( pUart->TxCount == 0 )

	// Check response after sending one package
//	if( (pUart->Avail != 0) && (os_AUX_State[pUart->UartNum] == AUX_STATE_WAIT_ACK) )
//	  {
//	  if( *pUart->pRd == ACK )
//	    {
//	    // read all ACKs one by one
//	    do{
//	      BSP_UART_Read( pUart, (UINT8 *)&data, 1 );
//	      } while();
//	    }
//	  }

	// printf("os_AUX_State[pUart->UartNum]=0x%x\n",os_AUX_State[pUart->UartNum]);
	// Receiver Process
	if( os_AUX_State[pUart->UartNum] != AUX_STATE_WAIT_ACK )
	  result=OS_AUX_DataLinkRxManager( pUart );	// 2009-10-30
	
		// Receiver & Transmitter Processes
		switch( os_AUX_Para[pUart->UartNum].Mode )
			{
			case AUX_MODE_BYPASS:
			case AUX_MODE_BARCODE:
			case AUX_MODE_VISAWAVE:
			case AUX_MODE_SVC:
			
				os_AUX_Para[pUart->UartNum].Tob = os_AUX_ParaBak[pUart->UartNum].Tob; // restore TOB
				
				break;
				
			case AUX_MODE_SOH:
			case AUX_MODE_DLL:
			
				OS_AUX_DataLinkWaitAck_DLL( pUart );
				os_AUX_Para[pUart->UartNum].Tob = os_AUX_ParaBak[pUart->UartNum].Tob; // restore TOB
				
				break;
				
			case AUX_MODE_STX:
			
				
				( pUart );
				os_AUX_Para[pUart->UartNum].Tob = os_AUX_ParaBak[pUart->UartNum].Tob; // restore TOB
				
				break;
			
			case AUX_MODE_DLESTX2:
				break;
						
			case AUX_MODE_STXB:
				break;
			
			}
	
	
}

// ---------------------------------------------------------------------------
// FUNCTION: Task manager of AUX data link layer protocol (Receive).
// INPUT   : pUart -- registered UART structure.
// OUTPUT  : none.
// RETURN  : TRUE  - package received ok
//           FALSE - package not available 
// ---------------------------------------------------------------------------
UINT32	OS_AUX_DataLinkRxManager( BSP_UART *pUart )
{
	// printf("OS_AUX_DataLinkRxManager\n");
	switch( os_AUX_Para[pUart->UartNum].Mode )
	      {
	      case AUX_MODE_BYPASS:
	           return( OS_AUX_DataLinkRx_BYPASS( pUart ) );
	           
	      case AUX_MODE_SOH:
	           return( OS_AUX_DataLinkRx_SOH( pUart ) );
	           
	      case AUX_MODE_DLL:
	           return( OS_AUX_DataLinkRx_DLL( pUart ) );
	           
	      case AUX_MODE_STX:
	           return( OS_AUX_DataLinkRx_STX( pUart ) );
	           
	      case AUX_MODE_STXB:
	           break;
	           
	      case AUX_MODE_VISAWAVE:
	           return( OS_AUX_DataLinkRx_WAVE( pUart ) );

	      case AUX_MODE_SVC:
	           return( OS_AUX_DataLinkRx_SVC( pUart ) );
	           
	      case AUX_MODE_BARCODE:
	           return( OS_AUX_DataLinkRx_BARCODE( pUart ) );
	      
	      case AUX_MODE_DLESTX2:
	      	   return( OS_AUX_DataLinkRx_DLESTX2( pUart ) );
	      }
}
// ---------------------------------------------------------------------------
// FUNCTION: Task manager of AUX data link layer TOB reached.
// INPUT   : port - port number.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_AUX_DataLinkRxTOB( UINT32 port )
{
UINT8	data;

/*
	switch( port )	// PATCH: 2010-09-03, get rid of queued data
	      {
	      case BSP_UART_0:
	           while( BSP_UART_Read( os_pAUX0, (UINT8 *)&data, 1 ) );
	           break;

	      case BSP_UART_1:
	           while( BSP_UART_Read( os_pAUX1, (UINT8 *)&data, 1 ) );
	           break;

	      case BSP_UART_2:
	           while( BSP_UART_Read( os_pAUX2, (UINT8 *)&data, 1 ) );
	           break;
	      }
*/
	switch( os_AUX_Para[port].Mode )
	      {
	      case AUX_MODE_BYPASS:
	      
	           OS_AUX_DataLinkRxTOB_BYPASS( port );
	           break;
	           
	      case AUX_MODE_SOH:
	      
	           OS_AUX_DataLinkRxTOB_SOH( port );
	           break;
	           
	      case AUX_MODE_DLL:
	      
	           OS_AUX_DataLinkRxTOB_DLL( port );
	           break;
	           
	      case AUX_MODE_STX:
	      
	           OS_AUX_DataLinkRxTOB_STX( port );
	           break;
	           
	      case AUX_MODE_STXB:
	           break;
	           
	      case AUX_MODE_VISAWAVE:
	      case AUX_MODE_SVC:
	           OS_AUX_DataLinkRxTOB_WAVE( port );
	           
	      case AUX_MODE_BARCODE:
	      
	           OS_AUX_DataLinkRxTOB_BARCODE( port );
	           break;
	      
	      case AUX_MODE_DLESTX2:
	      
	      	   OS_AUX_DataLinkRxTOB_DLESTX2( port );
	      	   break;
	      }
		  if(os_AUX_Para[port].Mode!=AUX_MODE_BYPASS)
		  os_AUX_Index[port]=0;//reset index
}

// ---------------------------------------------------------------------------
// FUNCTION: Task manager of AUX data link layer protocol (Transmit).
// INPUT   : pUart -- registered UART structure.
//           pData -- data to be transmitted. (LL-V)
// OUTPUT  : none.
// RETURN  : TRUE  - package received ok
//           FALSE - package not available 
// ---------------------------------------------------------------------------
UINT32	OS_AUX_DataLinkTxManager( BSP_UART *pUart, UINT8 *pData )
{
	
	switch( os_AUX_Para[pUart->UartNum].Mode )
	      {
	      case AUX_MODE_BYPASS:
	           return( OS_AUX_DataLinkTx_BYPASS( pUart, pData) );
	           
	      case AUX_MODE_SOH:
	           return( OS_AUX_DataLinkTx_SOH( pUart, pData ) );
	           
	      case AUX_MODE_DLL:
	           return( OS_AUX_DataLinkTx_DLL( pUart, pData ) );
	           
	      case AUX_MODE_STX:
	           return( OS_AUX_DataLinkTx_STX( pUart, pData ) );
	           
	      case AUX_MODE_STXB:
	           break;
	           
	      case AUX_MODE_VISAWAVE:
	           return( OS_AUX_DataLinkTx_WAVE( pUart, pData) );

	      case AUX_MODE_SVC:
	           return( OS_AUX_DataLinkTx_SVC( pUart, pData) );
	           
	      case AUX_MODE_BARCODE:
	           break;
	           
	      case AUX_MODE_DLESTX2:
	           return( OS_AUX_DataLinkTx_DLESTX2( pUart, pData ) );
	           break;
	      }
}

// ---------------------------------------------------------------------------
// FUNCTION: Task manager of AUX data link layer ACK protocol.
// INPUT   : pUart -- registered UART structure.
// OUTPUT  : none.
// RETURN  : TRUE  - package received ok
//           FALSE - package not available 
// ---------------------------------------------------------------------------
UINT32	OS_AUX_DataLinkAckManager( BSP_UART *pUart )
{
UINT32	result;


	if( os_DLL_ACK_MODE )	// 2015-03-24, ACK by APP itself if AFTER-SERVICE mode enabled
	  {
	  switch( os_AUX_Para[pUart->UartNum].Mode )
	        {
	        case AUX_MODE_BYPASS:
	             break;
	             
	        case AUX_MODE_SOH:
	        case AUX_MODE_DLL:
	        case AUX_MODE_STX:
	        	
	             os_DLL_ACK_MODE = 0;
	             result = OS_AUX_DataLinkAck_DLL( pUart, ACK );
	             os_DLL_ACK_MODE = 1;
	             
	             return( result );
	             
	        case AUX_MODE_STXB:
	             break;
	             
	        case AUX_MODE_VISAWAVE:
	             break;
	             
	        case AUX_MODE_BARCODE:
	             break;
	        }
	  }
	      
	return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: Reset AUX status.
// INPUT   : pUart
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_AUX_ResetStatus( BSP_UART *pUart ) 
{
	os_AUX_Status[pUart->UartNum] = AUX_STATUS_FREE;
}

// ---------------------------------------------------------------------------
// FUNCTION: To gain access and enable of UART driver.
// INPUT   : Port
//           pAux
// OUTPUT  : none.
// RETURN  : NULLPTR (Failed)
//           pUart   	 (OK)
// ---------------------------------------------------------------------------
BSP_UART *OS_AUX_Open( UINT8 Port, API_AUX pAux )
{
BSP_UART *pUart = 0;
UINT32	baud;

UINT32	databits = 0;
UINT32	stopbits;
UINT32	parity;
UINT32	buffersize;
UINT32	FCTL;

//	OS_EnableTimer1();//Enable OS_AUX_FlowControl timer

	
	
	switch( pAux.Mode )
	      {
	      case AUX_MODE_BYPASS:
	      case AUX_MODE_SOH:
	      case AUX_MODE_DLL:
	      case AUX_MODE_STX:
	      case AUX_MODE_BARCODE:
	      case AUX_MODE_VISAWAVE:
	      case AUX_MODE_SVC:
	      case AUX_MODE_DLESTX2:
				//open com port
	           pUart = BSP_UART_Acquire( (UINT32)Port, OS_AUX_IsrCallBack );
	           break;
	           
	      case AUX_MODE_STXB:
	      default:
	      	   return( NULLPTR );
	      }
	
	if( pUart != NULLPTR )
	  {
	  switch( pAux.Baud & 0x0FE0 )
	        {
	        case COM_300:
	             baud = 300;

	             break;
	        case COM_600:
	             baud = 600;

	             break;
	        case COM_1200:
	             baud = 1200;

	             break;
	        case COM_2400:
	             baud = 2400;

	             break;
	        case COM_4800:
	             baud = 4800;

	             break;
	        case COM_9600:
	             baud = 9600;

	             break;
	        case COM_19200:
	             baud = 19200;

	             break;
			case COM_28800:
	             baud = 28800;

	             break;
	        case COM_38400:
	             baud = 38400;

	             break;
	        case COM_57600:
	             baud = 57600;

	             break;
	        case COM_115200:
	             baud = 115200;

	             break;
	        case COM_230400:
	             baud = 230400;

	             break;
	        case COM_460800:
	             baud = 460800;

	             break;
	        case COM_921600:
	             baud = 921600;

	             break;
	        case COM_1228800:
	             baud = 1228800;

	             break;
	        default:
	             baud = 9600;

	        }
	  
	  switch( pAux.Baud & 0x0003 )
	        {
	        case COM_CHR7:
	             databits = 7;
	             break;
	        case COM_CHR8:
	             databits = 8;
	             break;
	        default:
	             databits = 8;
	        }
	  
	  switch( pAux.Baud & 0x0004 )
	        {
	        case COM_STOP1:
	             stopbits = 1;

	             break;
	        case COM_STOP2:
	             stopbits = 2;

	             break;
	        default:
	             stopbits  = 1;

	        }
	        
	  switch( pAux.Baud & 0x0018 )
	        {
	        case COM_NOPARITY:
	             parity = UART_PARITY_NONE;


	             break;
	        case COM_ODDPARITY:
	             parity = UART_PARITY_ODD;


	             break;
	        case COM_EVENPARITY:
	             parity = UART_PARITY_EVEN;



	             break;
	        default:
	             parity = UART_PARITY_NONE;


	        }
	  
	  buffersize = pAux.BufferSize;	// PATCH: 2009-08-26
	  if( (buffersize < AUX_DEFAULT_BUF_SIZE) || (buffersize > AUX_MAX_BUF_SIZE) )
	    buffersize = AUX_DEFAULT_BUF_SIZE;
	
	//   printf("pUart->BufferSize=%d\n",pUart->BufferSize);
	  pUart->Mode = UART_MODE_IRQ;
	  pUart->Baud = baud;
	  pUart->DataBits = databits;
	  pUart->StopBits = stopbits;
	  pUart->Parity = parity;
	  pUart->BufferSize = buffersize;
	  pUart->FlowControl = UART_FLOW_NONE;
	  pUart->RxCallLevel = 1;
	  
	  //set port attribute
	  if( BSP_UART_Start( pUart ) != BSP_SUCCESS )
	    return( NULLPTR );
	  }
	  else{
		
		return( NULLPTR );
	  }
/*
	// 2010-04-02, enable DTR & RTS
	BSP_UART_SetModemControl( pUart, MCTL_DTR | MCTL_RTS );
*/	
/*
	// 2009-10-30, change receiver FIFO trigger level to 1 byte (BSP default 8 bytes), i.e., RxISR will be called per data byte.
	FCTL = FCTL_TRG1 | FCTL_CLRTXF | FCTL_CLRRXF | FCTL_FIFOEN;
	BSP_WR32( pUart->Base | UART_REG_FCTL, FCTL );
*/	
	
	// default settings
	if( pAux.Tob == 0 )
	  pAux.Tob = 100;
	if( pAux.Tor == 0 )
	  pAux.Tor = 300;
	if( pAux.Resend > MAX_AUX_ACKS )
	  pAux.Resend = 1;
	if( pAux.Acks > MAX_AUX_ACKS )
	  pAux.Acks = 1;
	  
	// REMOVED: 2009-08-26
	pAux.TxFlag = 0;
	pAux.RxFlag = 0;
//	pAux.BufferSize = AUX_DEFAULT_BUF_SIZE;
//	pAux.FlowCtrl = 0;
//	pAux.IoConfig = 0;

	os_AUX_ExpLen[pUart->UartNum] = 0;
	os_AUX_State[pUart->UartNum] = AUX_STATE_IDLE;	// 2011-05-09
	
	memmove( &os_AUX_Para[pUart->UartNum], &pAux, sizeof(API_AUX) );	// save   AUX parameters
	memmove( &os_AUX_ParaBak[pUart->UartNum], &pAux, sizeof(API_AUX) );	// backup AUX parameters
	OS_AUX_ResetStatus( pUart );						// reset status
	
	memset( &os_AUX_RxDataBuffer[pUart->UartNum], 0x00, sizeof(AUX_DATA) );	// 2009-10-30
	memset( &os_AUX_TxDataBuffer[pUart->UartNum], 0x00, sizeof(AUX_DATA) );	//
	os_AUX_DataLen[pUart->UartNum]=0;
	
#if	0
	if(pFlowControlTimer==NULL)
	{
		pFlowControlTimer=BSP_TMR_Acquire(1);//10msec
		BSP_TMR_Start( (void *)&OS_AUX_FlowControl,pFlowControlTimer );//Start the timer
		TimeoutCount=0;
	}
#endif

	return( pUart );
}

// ---------------------------------------------------------------------------
// FUNCTION: To deactivate the specified AUX port.
// INPUT   : pUart -- pointer to the BSP_UART structure.
// OUTPUT  : none.
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
UINT32	OS_AUX_Close( BSP_UART *pUart )
{	
	os_AUX_Para[pUart->UartNum].TxFlag = FALSE;//disable timer resend
	os_AUX_Para[pUart->UartNum].RxFlag = FALSE;//disable timer TOB counter
	
		
	if( BSP_UART_Release( pUart ) == BSP_SUCCESS )
	{
		if(pFlowControlTimer)
		{
			BSP_TMR_Stop( pFlowControlTimer );
			pFlowControlTimer=NULL;
		}
		return( TRUE );
	}
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: To determine whether data is ready for input from the AUX device.
// INPUT   : pUart -- pointer to the BSP_UART structure.
//	     type  -- TRUE : called by api_aux_rxready()
//		      FALSE: called by api_aux_rxstring()
// OUTPUT  : dbuf  -- length of incoming data if available.
// RETURN  : TRUE  -- data available.
//           FALSE -- data not available.
// ---------------------------------------------------------------------------
UINT32	OS_AUX_RxReady( BSP_UART *pUart, UINT8 *dbuf, UINT32 type )
{
UINT32	len;
UINT32	i;
UINT8	ACKresponse = ACK ;
static UINT8	res = 0,resBAK=0 ;
	
	
    // if(os_AUX_RxDataBuffer[pUart->UartNum].Len > 0)
    //     goto RX_READY;

	if( type && (os_AUX_Para[pUart->UartNum].Mode == AUX_MODE_BYPASS) )
	  {
		os_AUX_ExpLen[pUart->UartNum] = dbuf[0] + dbuf[1]*256;
	  }
	if( (os_AUX_Para[pUart->UartNum].RxFlag == FALSE) && (os_AUX_Para[pUart->UartNum].Mode != AUX_MODE_BYPASS) )
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
	  i = os_AUX_Index[pUart->UartNum];
	if(BSP_UART_Read( pUart, &os_AUX_RxDataBuffer[pUart->UartNum].Data[i], 1 )>0)
	{
		TimeoutCount=0;//init flow control timer
		OS_AUX_IsrCallBack( pUart );
	}
	// if(resBAK!=res)
	// 	printf("!!!!!!!!!!!res=%d\n",res);
	// resBAK=res;
	len =os_AUX_RxDataBuffer[pUart->UartNum].Len;
	
	
	
// RX_READY:		
	if( len )
	{
	  if( os_SOH_LEN_TYPE == 0 )
		{
			dbuf[0] = len & 0x00FF;			// PATCH: 2010-09-30
			dbuf[1] = (len & 0xFF00) >> 8;		// 
		}
		else
		{
			dbuf[0] = len & 0x000000FF;
			dbuf[1] = (len & 0x0000FF00) >> 8;
			dbuf[2] = (len & 0x00FF0000) >> 16;
			dbuf[3] = (len & 0xFF000000) >> 24;
		}
	  return( TRUE );
	}
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: To read the incoming data string from AUX receiving stream buffer.
// INPUT   : pUart -- pointer to the BSP_UART structure.
// OUTPUT  : dbuf
//           UINT  length ;     	// length of the received data string.
//	     UCHAR data[length] ;	// data string.	
// RETURN  : TRUE  -- process OK.
//           FALSE -- process has failed. (RFU)
// ---------------------------------------------------------------------------
UINT32	OS_AUX_RxString( BSP_UART *pUart, UINT8 *dbuf )
{
UINT32	len;
	len = os_AUX_RxDataBuffer[pUart->UartNum].Len;
	// printf("$$$$$$$$$$len=%d\n",len);
	//if( OS_AUX_RxReady( pUart, dbuf, FALSE ) == TRUE )
	if( len>0 )
	  {
	  // move Data & Length to application buffer
	  if( os_SOH_LEN_TYPE == 0 )
	    memmove( &dbuf[2], os_AUX_RxDataBuffer[pUart->UartNum].Data, len );
	  else
	    memmove( &dbuf[4], os_AUX_RxDataBuffer[pUart->UartNum].Data, len );
	  
	  
	  if( os_SOH_LEN_TYPE == 0 )
	    {
	    dbuf[0] = len & 0x00FF;		// PATCH: 2007-11-05
	    dbuf[1] = (len & 0xFF00) >> 8;	//
	    }
	  else
	    {
	    dbuf[0] = len & 0x000000FF;
	    dbuf[1] = (len & 0x0000FF00) >> 8;
	    dbuf[2] = (len & 0x00FF0000) >> 16;
	    dbuf[3] = (len & 0xFF000000) >> 24;
	    }
	  // clear os buffer
	  memset( os_AUX_RxDataBuffer[pUart->UartNum].Data, 0x00, len );
	  os_AUX_RxDataBuffer[pUart->UartNum].Len = 0;
	  os_AUX_DataLen[pUart->UartNum]=0;
	  os_AUX_Index[pUart->UartNum] = 0;
	  // ACK to sender
//	  OS_AUX_DataLinkAckManager( pUart );	// 2009-10-30, moved to ISR link layer

	  OS_AUX_DataLinkAckManager( pUart );	// 2015-03-24, restored for DLL special case
		return( TRUE );
	  }
	else
		return( FALSE );
	
}

// ---------------------------------------------------------------------------
// FUNCTION: To determine whether AUX port is ready to transmit the next data string.
// INPUT   : pUart -- pointer to the BSP_UART structure.
// OUTPUT  : none.
// RETURN  : AUX_STATUS_FREE
//           AUX_STATUS_ERROR
//           AUX_STATUS_BUSY
// ---------------------------------------------------------------------------
UINT32	OS_AUX_TxReady( BSP_UART *pUart )
{
	UCHAR data=0;//to store ACK/NAK message
	UINT32 Leng=0;
	if(os_AUX_Para[pUart->UartNum].Mode!=AUX_MODE_BYPASS&&
		os_AUX_Para[pUart->UartNum].Acks!=0){
	Leng=BSP_UART_Read( pUart, &data, 1 );//read 1 byte
	if(Leng>0)//if read somthing
	{
		switch( data )
	    	  {
	    	  case ACK:
	    	       
	    	    //    BSP_UART_Read( pUart, (UCHAR *)&data, 1 );	// flush ACK
	    	       
	    	       os_AUX_State[pUart->UartNum] = AUX_STATE_IDLE;
	    	       os_AUX_Status[pUart->UartNum] = AUX_STATUS_FREE;
	    	       
	    	       break;
	    	       
	    	  case NAK:
	    	  	    	  
	    	    //    BSP_UART_Read( pUart, (UCHAR *)&data, 1 );	// flush NAK
	    	       
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
	    	  
	    	       BSP_UART_Read( pUart, (UCHAR *)&data, 1 );	// flush unknown acknowledge
	    	       
	    	       os_AUX_Para[pUart->UartNum].TxFlag = TRUE;	// enable TimeOut task again for the next retry
		       
	    	       break;
	    	  }
		}
	}	
	switch( os_AUX_Para[pUart->UartNum].Mode )
	      {
	      case AUX_MODE_BYPASS:
	      
	      	   return( AUX_BYPASS_TxReady( pUart ) );
	      
	      case AUX_MODE_DLL:
	      case AUX_MODE_SOH:
	      case AUX_MODE_STX:
	      case AUX_MODE_VISAWAVE:
	      case AUX_MODE_SVC:
	      case AUX_MODE_DLESTX2:
	      
	      	   return( AUX_DLL_TxReady( pUart ) );
//	           return( os_AUX_Status[pUart->UartNum] );
	           
	      case AUX_MODE_STXB:
	           break;
	           
	      case AUX_MODE_BARCODE:
	           break;
	      }
}

// ---------------------------------------------------------------------------
// FUNCTION: To read the incoming data string from AUX receiving stream buffer.
// INPUT   : pUart -- pointer to the BSP_UART structure.
//           sbuf
//           UINT  length ;     	// length of data string to be transmitted.
//	     UCHAR data[length] ;	// data string.	
// RETURN  : TRUE  -- process OK.
//           FALSE -- process has failed. (RFU)
// ---------------------------------------------------------------------------
UINT32	OS_AUX_TxString( BSP_UART *pUart, UINT8 *sbuf )
{
	
	os_AUX_Status[pUart->UartNum] = AUX_STATUS_BUSY;
	TimeoutCount=0;//reset flow control timer
	return( OS_AUX_DataLinkTxManager( pUart, sbuf ) );
}

// ---------------------------------------------------------------------------
// FUNCTION: To alter the state of modem control signals. (RTS & DTR)
// INPUT   : pUart -- pointer to the BSP_UART structure.
//	     ctrl
//           The control state.
//	     	0 = de-assert both RTS and DTR
//		1 = assert DTR and de-assert RTS
//		2 = de-assert DTR and assert RTS
//		3 = assert both RTS and DTR
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UINT32	OS_AUX_SetModemControl( BSP_UART *pUart, UINT8 ctrl )
{
	if( ctrl > 3 )
	  return( FALSE );
	/*  
	if( BSP_UART_SetModemControl( pUart, ctrl ) == BSP_SUCCESS )
	  return( TRUE );
	else
	  return( FALSE );
  */
}

// ---------------------------------------------------------------------------
// FUNCTION: To get the state of modem control signals.
//		DCD, RI, DSR, CTS, DDCD, TERI, DDSR, DCTS
// INPUT   : dhn
//           The specified device handle number.
//           
//	     stat
//           The state of modem control signals.
//		bit0: DCTS	delta status change of CTS
//		   1: DDSR	delta status change of DSR
//		   2: TERI	trailing edge change on RI
//		   3: DDCD	delta status change of DCD
//		   4: CTS	inverted state of /CTS	(currently support)
//		   5: DSR	inverted state of /DSR
//		   6: RI	inverted state of /RI
//		   7: DCD	inverted state of /DCD
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	OS_AUX_GetModemStatus( BSP_UART *pUart, UINT8 *stat )
{
	/*
	if( BSP_UART_GetModemStatus( pUart ) == BSP_SUCCESS )
	  {
	  *stat = pUart->ModemStatus;
	  return( TRUE );
	  }
	else
	  return( FALSE );
  */
}

// ---------------------------------------------------------------------------
// FUNCTION: force to setup AUX mode.
// INPUT   : mode - AUX protocol.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	1	// moved from MFCOMM.c
void	OS_AUX_MFC_ForceModeMFC( UCHAR port )
{
//	if( os_MFC_MODE != auxSVC )
//	  {
//	  os_MFC_MODE = auxSVC;
	  os_AUX_Para[port].Mode = auxSVC;
//	  }
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: force to setup AUX mode.
// INPUT   : mode - AUX protocol.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	1	// moved from MFCOMM.c
void	OS_AUX_MFC_ForceModeWAVE( UCHAR port )
{
//	if( os_MFC_MODE != auxVISAWAVE )
//	  {
//	  os_MFC_MODE = auxVISAWAVE;
	  os_AUX_Para[port].Mode = auxVISAWAVE;
//	  }
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: check if MFC com port is open.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - COM port is opened.
//	     FALSE - COM port is closed.
// ---------------------------------------------------------------------------
#if	1	// moved from MFCOMM.c
UCHAR	OS_AUX_MFC_IsComPortReady( UCHAR port )
{
UCHAR	result = FALSE;


	switch( port )
	      {
	      case COM0:
	      	
	      	   if( os_DHN_AUX0 )
	      	     result = TRUE;
	      	   break;
	      	   
	      case COM1:
	      	
	      	   if( os_DHN_AUX1 )
	      	     result = TRUE;
	      	   break;
	      	   
	      case COM2:
	      	   
	      	   if( os_DHN_AUX2 )
	      	     result = TRUE;
	      	   break;
	      }
	      
	return( result );
}
#endif
