//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L                                                       **
//**  PRODUCT  : AS330/QS200                                                **
//**                                                                        **
//**  FILE     : API_AUX.C 	                                            **
//**  MODULE   : api_aux_open()				                    **
//**		 api_aux_close()					    **
//**		 api_aux_rxready()					    **
//**		 api_aux_txready()					    **
//**		 api_aux_rxstring()					    **
//**		 api_aux_txstring()					    **
//**		 api_aux_rxsetting()					    **
//**		 api_aux_SetModemControl()				    **
//**									    **
//**  FUNCTION : API::AUX (Auxiliary UART Module)			    **
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
#include "bsp_uart.h"
#include "OS_PROCS.h"
#include "POSAPI.h"
#include "DEV_AUX.h"
#include <stdio.h>
UINT		os_DHN_AUX0 = 0;
UINT		os_DHN_AUX1 = 0;
UINT		os_DHN_AUX2 = 0;
BSP_UART	*os_pAUX0;
BSP_UART	*os_pAUX1;
BSP_UART	*os_pAUX2;

UINT32		os_SOH_LEN_TYPE = 0;			// 0=2-byte, 1=4-byte
UINT32		os_DLL_ACK_MODE = 0;			// 0=real-time, 1=after-service
UCHAR		IfOpenedCom0=0;//to note whether uart port opened.	
UCHAR		IfOpenedCom1=0;	
UCHAR		IfOpenedCom2=0;	
extern	API_AUX		os_AUX_Para[];
extern	UINT32		os_AUX_State[];
// ---------------------------------------------------------------------------
// FUNCTION: To check if DHN matched.
// INPUT   : dhn
// OUTPUT  : none.
// RETURN  : pUart   -- pointer to BSP_UART structure.
//	     NULLPTR -- invalid device.
// ---------------------------------------------------------------------------
BSP_UART *AUX_CheckDHN( UINT dhn )
{	      
		if		( (dhn == os_DHN_AUX0)&&IfOpenedCom0 )
			return( os_pAUX0 );
    
		else if	( (dhn == os_DHN_AUX1)&&IfOpenedCom1 )
			return( os_pAUX1 );
       	      
		else if	( (dhn == os_DHN_AUX2)&&IfOpenedCom2 )
			return( os_pAUX2 );
		else
			return( NULLPTR );
	   
	      
}

// ---------------------------------------------------------------------------
// FUNCTION: To get BSP_UART pointer of the specified port.
// INPUT   : port -- port number.
// OUTPUT  : none.
// RETURN  : pUart   -- pointer to BSP_UART structure.
//	     NULLPTR -- invalid device.
// ---------------------------------------------------------------------------
BSP_UART *AUX_GetDHN( ULONG port )
{
	switch( port )
	      {
	      case BSP_UART_0:
	      
	           if( os_DHN_AUX0>0 )
	             return( os_pAUX0 );
	           else
	             return( NULLPTR );
	             
	           break;
	           
	      case BSP_UART_1:
	      
	          if( os_DHN_AUX1>0 )
	             return( os_pAUX1 );
	           else
	             return( NULLPTR );
	             
	           break;
	           
	      case BSP_UART_2:
	      
	           if( os_DHN_AUX2>0 )
	             return( os_pAUX2 );
	           else
	             return( NULLPTR );
	             
	           break;
	      }
}

// ---------------------------------------------------------------------------
// FUNCTION: To enable the auxiliary port.
// INPUT   : port -- UART port number.
// 	     pAux
//	     ->Mode		// data link protocol
//	     ->Baud		// baud rate
//	     ->Tob		// inter-byte timeout for receiving character string
//	     ->Tor		// timeout of waiting for response
//	     ->Resend		// re-transmit limit
//	     ->Acks		// no. of repetitive acknowledgement to the received message
//				// the followings are new elements
//	     ->BufferSize	// size of the transmit and receive driver buffer (in bytes)
//	     ->FlowCtrl		// flow control
//	     ->IoConfig		// io control for RTC,CTS,DTR,DSR...
//	     sbuf
//		same structure as API_AUX.
// OUTPUT  : none.
// RETURN  : DeviceHandleNumber
//           apiOutOfService
// ---------------------------------------------------------------------------
// Wayne modify 2020/08/12
UCHAR	api_aux_open( UCHAR port, API_AUX pAux )
{
UINT	dhn;
// API_AUX	pAux;
	
	// memmove( &pAux, sbuf, sizeof(API_AUX) );
	//dhn = port + psDEV_AUX + 0x80;
	//if(COM1)
	// 2015-04-27, force to close COM port for AS350S4 app
	switch( port )
	      {
	      case COM0:
	      	   
	      	   if( IfOpenedCom0 != 0 )
	      	     api_aux_close( os_DHN_AUX0 );
	           break;
	           
	      case COM1:

	      	   if( IfOpenedCom1 != 0 )
	      	     api_aux_close( os_DHN_AUX1 );
	           break; 
		  case COM2:

	      	   if( IfOpenedCom2 != 0 )
	      	     api_aux_close( os_DHN_AUX2 );
	           break; 
	      }
	switch( port )
	      {
	      case COM0:
	      
	      	   if( IfOpenedCom0 != 0 )
	      	     return( apiOutOfService );
	      	   else
	      	     {
	      	     
	      	     os_pAUX0 = OS_AUX_Open( port, pAux );
	      	     if( os_pAUX0 == NULLPTR )
	      	       return( apiOutOfService );
	      	     }
				 printf("COM0 open\n");
				 dhn=os_pAUX0->Fd;
				 os_DHN_AUX0 = dhn;
				 IfOpenedCom0=1;
				printf("COM0 dhn:%d\n",dhn);
	      	   break;
	      
	      case COM1:
	      
	      	   if( IfOpenedCom1 != 0 )
	      	     return( apiOutOfService );
	      	   else
	      	     {	      	     
	      	     os_pAUX1 = OS_AUX_Open( port, pAux );
	      	     if( os_pAUX1 == NULLPTR )
	      	       return( apiOutOfService );
	      	     }
				 printf("COM1 open\n");
				 dhn=os_pAUX1->Fd;				 
				 os_DHN_AUX1 = dhn;
				 IfOpenedCom1=1;
				 printf("COM1 dhn:%d\n",dhn);

	      	   break;
	         
		  case COM2:
	      
	      	   if( IfOpenedCom2 != 0 )
	      	     return( apiOutOfService );
	      	   else
	      	     {	      	     
	      	     os_pAUX2 = OS_AUX_Open( port, pAux );
	      	     if( os_pAUX2 == NULLPTR )
	      	       return( apiOutOfService );
	      	     }
				printf("COM2 open\n");
				 dhn=os_pAUX2->Fd;				 
				 os_DHN_AUX2 = dhn;
				 IfOpenedCom2=1;
				 printf("COM2 dhn:%d\n",dhn);
	      	   break;
	      default:
	           return( apiOutOfService );
	      }
	    
	return( dhn ); 
}

// ---------------------------------------------------------------------------
// FUNCTION: To disable AUX port.
// INPUT   : dhn
//	     The specified device handle number.
//	     0x00 = to close all opened tasks. (NOT SUPPORTED)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR 	api_aux_close( UCHAR dhn )
{	
UCHAR result;
BSP_UART *pUart=NULL;
	if(dhn!=0){
		pUart=AUX_CheckDHN( dhn );
		if(pUart==NULLPTR)
		{
			printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
			return( apiFailed );
		}
			
		
		result=OS_AUX_Close( pUart );	
		if(result)
		{
			if(os_DHN_AUX0 == pUart->Fd){
				IfOpenedCom0 = 0;		
			}
			else if(os_DHN_AUX1 == pUart->Fd){
				IfOpenedCom1 = 0;
			}
			else if(os_DHN_AUX2 == pUart->Fd){
				IfOpenedCom2 = 0;
			}
			
			os_SOH_LEN_TYPE = 0;
			os_DLL_ACK_MODE = 0;
			return( apiOK );
		}
		else
			return( apiFailed );
		
	}
	else
		return( apiFailed );
	
	//result=OS_AUX_Close( pUart );
	
	
	
}

// ---------------------------------------------------------------------------
// FUNCTION: To determine whether data is ready for input from the AUX device.
// INPUT   : dhn
//           The specified device handle number.
// OUTPUT  : dbuf
//           UINT  length ;     // length of the received data string.
//				if BYPASS mode is used, the caller can assign
//				the expected length of data to be received to
//				accelerate the performance.
//				if the value of "length" is 0, the incomming data
//				can only be captured after reaching TOB in BYPASS
//				mode.
// RETURN  : apiReady
//           apiNotReady
//	     apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_aux_rxready( UCHAR dhn, UCHAR *dbuf )
{
BSP_UART *pUart;

        pUart = AUX_CheckDHN( dhn );
	if( pUart == NULLPTR )
	  return( apiFailed );
	
	if( OS_AUX_RxReady( pUart, dbuf, TRUE ) == TRUE )
	  return( apiReady );
	else
	  return( apiNotReady );
}

// ---------------------------------------------------------------------------
// FUNCTION: To read the incoming data string from AUX receiving stream buffer.
// INPUT   : dhn
//           The specified device handle number.
// OUTPUT  : dbuf
//           UINT  length ;     	// length of the received data string.
//	     UCHAR data[length] ;	// data string.	
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_aux_rxstring( UCHAR dhn, UCHAR *dbuf )
{
BSP_UART *pUart;

        pUart = AUX_CheckDHN( dhn );
	if( pUart == NULLPTR )
	  return( apiFailed );
	
	if( OS_AUX_RxString( pUart, dbuf ) == TRUE )
	  return( apiOK );
	else
	  return( apiFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: To determine whether AUX port is ready to transmit the next data string.
// INPUT   : dhn
//           The specified device handle number.
// OUTPUT  : none.
// RETURN  : apiReady
//           apiNotReady
//	     apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_aux_txready( UCHAR dhn )
{
BSP_UART *pUart;
UINT32	result;

        pUart = AUX_CheckDHN( dhn );
	if( pUart == NULLPTR )
	  return( apiFailed );
	
	result = OS_AUX_TxReady( pUart );
	switch( result )
	      {
	      case AUX_STATUS_FREE:
	           return( apiReady );
	           
	      case AUX_STATUS_BUSY:
	      	   return( apiNotReady );
	      
	      	   if( os_AUX_Para[pUart->UartNum].Mode == AUX_MODE_BYPASS )
	      	     return( apiNotReady );
	      
	      	   if( os_AUX_Para[pUart->UartNum].Resend )	// 2014-08-26
	             return( apiNotReady );
	           else
	             {
	             if( AUX_BYPASS_TxReady( pUart ) != AUX_STATUS_FREE )	// whole packet sent?
	               return( apiNotReady );					// not yet
	             else
	               {
	               if( os_AUX_State[pUart->UartNum] == AUX_STATE_WAIT_ACK )
	                 {
	                 if( os_AUX_Para[pUart->UartNum].Tor )
	                   return( apiNotReady );
	                 }
	                 
	               // else goto the following default case
	               }
		     }
	           
	      default:
	           OS_AUX_ResetStatus( pUart );	// reset status for next check
	           return( apiFailed );
	      }
}

// ---------------------------------------------------------------------------
// FUNCTION: To write the outgoing data string to AUX transmitting stream buffer.
// INPUT   : dhn
//           The specified device handle number.
//           sbuf
//           UINT  length ;     	// length of data string to be transmitted.
//	     UCHAR data[length] ;	// data string.	
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_aux_txstring( UCHAR dhn, UCHAR *sbuf )
{
BSP_UART *pUart;

        pUart = AUX_CheckDHN( dhn );
	if( pUart == NULLPTR )
	  return( apiFailed );

	if( os_SOH_LEN_TYPE == 0 )
	  {	
	  if( (sbuf[0] + sbuf[1]*256) == 0 )	// PATCH: 2011-09-27
	    return( apiFailed );		//
	  }
	else	// 2014-08-26
	  {
	  if( (sbuf[0] == 0) && (sbuf[1] == 0) && (sbuf[2] == 0) && (sbuf[3] == 0) )
	    return( apiFailed );
	  }
	
	if( OS_AUX_TxString( pUart, sbuf ) == TRUE )
	  return( apiOK );
	else
	  return( apiFailed );
}


/*
// ---------------------------------------------------------------------------
// FUNCTION: To alter the state of modem control signals. (RTS & DTR)
// INPUT   : dhn
//           The specified device handle number.
//           
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
UCHAR	api_aux_SetModemControl( UCHAR dhn, UCHAR ctrl  )
{
BSP_UART *pUart;

        pUart = AUX_CheckDHN( dhn );
	if( pUart == NULLPTR )
	  return( apiFailed );
	
	if( OS_AUX_SetModemControl( pUart, ctrl ) == TRUE )
	  return( apiOK );
	else
	  return( apiFailed );		
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
UCHAR	api_aux_GetModemStatus( UCHAR dhn, UCHAR *stat )
{
BSP_UART *pUart;

        pUart = AUX_CheckDHN( dhn );
	if( pUart == NULLPTR )
	  return( apiFailed );
	
	if( OS_AUX_GetModemStatus( pUart, stat ) == TRUE )
	  return( apiOK );
	else
	  return( apiFailed );		
}
*/


// ---------------------------------------------------------------------------
// FUNCTION: To setup long length type for SOH protocol.
// INPUT   : dhn
//	     The specified device handle number.
//	     flag
//	     0 = 2 bytes length (default)
//	     1 = 4 bytes length
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR 	api_aux_SetLongLen( UINT dhn, UCHAR flag )
{
BSP_UART *pUart;


        pUart = AUX_CheckDHN( dhn );
	if( pUart == NULLPTR )
	  return( apiFailed );
	  
	if( flag == 0 )
	  os_SOH_LEN_TYPE = 0;
	else
	  os_SOH_LEN_TYPE = 1;
	  
	return( apiOK );
}

// ---------------------------------------------------------------------------
// FUNCTION: To setup ACK response behavior, real-time or after-service mode.
// INPUT   : dhn
//	     The specified device handle number.
//	     mode
//	     0 = real-time mode (default).
//	     1 = after-service mode, ACK only after APP has processed the
//		 incoming message, especially when using DLL protocol.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR 	api_aux_SetAckMode( UINT dhn, UCHAR mode )
{
BSP_UART *pUart;


        pUart = AUX_CheckDHN( dhn );
	if( pUart == NULLPTR )
	  return( apiFailed );
	  
	if( mode == 0 )
	  os_DLL_ACK_MODE = 0;
	else
	  os_DLL_ACK_MODE = 1;
	  
	return( apiOK );
}
