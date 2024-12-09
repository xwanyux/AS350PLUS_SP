//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L (32-bit Platform)                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : DEV_MDM.C                                                  **
//**  MODULE   : 							    **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2009/05/05                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2009 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include "bsp_types.h"
#include "POSAPI.h"
#include "APK_MDM.h"


// ---------------------------------------------------------------------------
// FUNCTION: Initialize modem module, must be called at POST 
//	     to create soft modem tasks to speed up the process.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_MDM_Init( void )
{
//	apk_mdm_init();
}

// ---------------------------------------------------------------------------
// FUNCTION: To enable the modem device and establish the link to remote host.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
UINT32	OS_MDM_Open( UINT8 *atcmd, API_HOST *sbuf )
{
	UINT dhn;
	dhn=apk_mdm_open( atcmd, sbuf );
	if(  dhn != apiFailed )
	  return( dhn );
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: To disable the modem device and disconnect the link to host.
// INPUT   : dhn
//	     The specified device handle number.
//	     0x00 = to close all opened tasks.
//
//	     delay
//	     Time before disconnecting the link to host.
//	     0x0000 = disconnect immediately.
//	     0xFFFF= ignore the last "delay" setting and keep the link with host if it exists.
//	     Others  = delay time. (unit: depends on system timer resolution, default 10ms.)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UINT32	OS_MDM_Close( UINT8 dhn, UINT16 delay )
{
	if( apk_mdm_close( delay ) == apiOK )
	  return( TRUE );
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: To get the connection status from modem device.
// INPUT   : none.
// OUTPUT  : dbuf
//	     Modem status byte.
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
UINT32	OS_MDM_Status( UINT8 *dbuf )
{
	
	if( apk_mdm_status( dbuf ) == apiOK )
	  return( TRUE );
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: To determine whether data is ready for input from the modem device.
// INPUT   : none.
// OUTPUT  : dbuf
//           UINT  length ;     // length of the received data string.
// RETURN  : apiReady
//           apiNotReady
// ---------------------------------------------------------------------------
UINT32	OS_MDM_RxReady( UINT8 *dbuf )
{
	if( apk_mdm_rxready( dbuf ) == apiReady )
	  return( TRUE );
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: To read the incoming data string from modem receiving block-stream buffer.
// INPUT   : none.
// OUTPUT  : dbuf
//           UINT  length ;     	// length of the received data string.
//	     UCHAR data[length] ;	// data string.	
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UINT32	OS_MDM_RxString( UINT8 *dbuf )
{
	if( apk_mdm_rxstring( dbuf ) == apiOK )
	  return( TRUE );
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: To determine whether modem is ready to transmit the next data string.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiReady
//           apiNotReady
// ---------------------------------------------------------------------------
UINT32	OS_MDM_TxReady( void )
{	
	printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
	if( apk_mdm_txready() == apiReady )
	  return( TRUE );
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: To write the outgoing data string to modem transmitting stream buffer.
// INPUT   : none.
//           sbuf
//           UINT  length ;     	// length of data string to be transmitted.
//	     UCHAR data[length] ;	// data string.	
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UINT32	OS_MDM_TxString( UINT8 *sbuf )
{
	if( apk_mdm_txstring( sbuf ) == apiOK )
	  return( TRUE );
	else
	  return( FALSE );
}
