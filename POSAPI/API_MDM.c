//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L                                                       **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : API_MDM.C 	                                            **
//**  MODULE   : api_mdm_open()				                    **
//**		 api_mdm_close()					    **
//**		 api_mdm_status()					    **
//**		 api_mdm_txready()					    **
//**		 api_mdm_rxready()					    **
//**		 api_mdm_txstring()					    **
//**		 api_mdm_rxstring()					    **
//**									    **
//**  FUNCTION : API::MDM (MODEM Module)		    		    **
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
#include "OS_PROCS.h"
#include "POSAPI.h"
#include "DEV_MDM.h"

UCHAR		os_DHN_MDM = 0;



// ---------------------------------------------------------------------------
// FUNCTION: To check if DHN matched.
// INPUT   : dhn
// OUTPUT  : none.
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
UCHAR	MDM_CheckDHN( UCHAR dhn )
{

	if( (dhn == 0) || (dhn == os_DHN_MDM) )
	  return( TRUE );
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: To enable the modem device and establish the link to remote host.
// INPUT   : atcmd
//	     UCHAR length; // length of user-assigned AT-command. (max. 30 bytes)
//	     char  data[length]; // optional, user-assigned AT-command string,
//	                         // if "length" != 0.
//	     sbuf
//	     refer to API_HOST defintion.
// OUTPUT  : none.
// RETURN  : DeviceHandleNo
//           apiOutOfService
// ---------------------------------------------------------------------------
//UCHAR	api_mdm_open( UCHAR *atcmd, UCHAR *sbuf )
UCHAR api_mdm_open( UCHAR *atcmd, API_HOST *mdm_para )
{
//API_HOST mdm_para;
	//memmove(&mdm_para,sbuf,sizeof(mdm_para));
	if( os_DHN_MDM != 0 )	// already opened?
	  return( apiOutOfService );
	//os_DHN_MDM=OS_MDM_Open( atcmd, &mdm_para );
	os_DHN_MDM=OS_MDM_Open( atcmd, mdm_para );
	  if( os_DHN_MDM == FALSE )
	    return( apiOutOfService );
	
	//os_DHN_MDM = psDEV_MDM + 0x80;
	return( os_DHN_MDM );
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
UCHAR	api_mdm_close( UCHAR dhn, UINT delay )
{
UCHAR	status;

	status = apiFailed;
	
	if( MDM_CheckDHN( dhn ) == TRUE )
	  {
	  if( OS_MDM_Close( dhn, delay ) )
	    {
//	    os_DHN_MDM = 0;
	    status = apiOK;
	    }
	  os_DHN_MDM = 0;	// 2011-08-19
	  }

	return( status );
}

// ---------------------------------------------------------------------------
// FUNCTION: To get the connection status from modem device.
// INPUT   : dhn
//	     The specified device handle number.
// OUTPUT  : dbuf
//	     Modem status byte.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_mdm_status( UCHAR dhn, UCHAR *dbuf )
{
	if( MDM_CheckDHN( dhn ) == TRUE )
	  {
	  OS_MDM_Status( dbuf );
	  
	  return( apiOK );
	  }
	else
	  return( apiFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: To determine whether data is ready for input from the modem device.
// INPUT   : dhn
//           The specified device handle number.
// OUTPUT  : dbuf
//           UINT  length ;     // length of the received data string.
// RETURN  : apiReady
//           apiNotReady
//	     apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_mdm_rxready( UCHAR dhn, UCHAR *dbuf )
{
        if( MDM_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );
	
	if( OS_MDM_RxReady( dbuf ) )
	  return( apiReady );
	else
	  return( apiNotReady );
}

// ---------------------------------------------------------------------------
// FUNCTION: To read the incoming data string from modem receiving block-stream buffer.
// INPUT   : dhn
//           The specified device handle number.
// OUTPUT  : dbuf
//           UINT  length ;     	// length of the received data string.
//	     UCHAR data[length] ;	// data string.	
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_mdm_rxstring( UCHAR dhn, UCHAR *dbuf )
{
        if( MDM_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );
	
	if( OS_MDM_RxString( dbuf ) == TRUE )
	  return( apiOK );
	else
	  return( apiFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: To determine whether modem is ready to transmit the next data string.
// INPUT   : dhn
//           The specified device handle number.
// OUTPUT  : none.
// RETURN  : apiReady
//           apiNotReady
//	     apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_mdm_txready( UCHAR dhn )
{
	// printf("^^^^^^^^^^^^^^^^^^^^ dhn =%d\n", dhn );
        if( MDM_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );
	
	if( OS_MDM_TxReady() )
	  return( apiReady );
	else
	  return( apiNotReady );
}

// ---------------------------------------------------------------------------
// FUNCTION: To write the outgoing data string to modem transmitting stream buffer.
// INPUT   : dhn
//           The specified device handle number.
//           sbuf
//           UINT  length ;     	// length of data string to be transmitted.
//	     UCHAR data[length] ;	// data string.	
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_mdm_txstring( UCHAR dhn, UCHAR *sbuf )
{
        if( MDM_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );

	if( (sbuf[0] + sbuf[1]*256) == 0 )	// PATCH: 2011-09-27
	  return( apiFailed );			//
	
	if( OS_MDM_TxString( sbuf ) == TRUE )
	  return( apiOK );
	else
	  return( apiFailed );
}
