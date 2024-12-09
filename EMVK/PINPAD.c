//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS330	                                                    **
//**                                                                        **
//**  FILE     : PINPAD.C                                                   **
//**  MODULE   : PP_GePIN()                                                 **
//**             PP_VerifyPIN()                                             **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2018/08/08                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2018 SymLink Corporation. All rights reserved.	    **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include <string.h>

#include "POSAPI.h"
#include "EMVDC.h"
#include "GDATAEX.h"
#include "EMVAPI.h"
//#include "EMVAPK.H"
#include "EMVDCMSG.h"
//#include "APDU.H"
#include "TOOLS.h"
#include "UI.h"

#include "PEDAPI.h"

// ---------------------------------------------------------------------------
// FUNCTION: display "INCORRECT PIN"
// ---------------------------------------------------------------------------
void	PP_show_incorrect_pin( void )
{
	UI_ClearScreen();
	// UI_PutMsg( 0, COL_MIDWAY, FONT2, 8, (UCHAR *)stdmsg_C_incorrect_pin );
	UI_PutMsg( 1, COL_MIDWAY, FONT1, 13, (UCHAR *)stdmsg_incorrect_pin );
}

// ---------------------------------------------------------------------------
// FUNCTION: display "LAST PIN TRY"
// ---------------------------------------------------------------------------
void	PP_show_last_pin_try( void )
{
	UI_ClearScreen();
	// UI_PutMsg( 0, COL_MIDWAY, FONT2, 12, (UCHAR *)stdmsg_C_last_pin_try );
	UI_PutMsg( 1, COL_MIDWAY, FONT1, 12, (UCHAR *)stdmsg_last_pin_try );
}

// ---------------------------------------------------------------------------
// FUNCTION: display "PIN OK"
// ---------------------------------------------------------------------------
void	PP_show_pin_ok( void )
{
	UI_ClearScreen();
	// UI_PutMsg( 0, COL_MIDWAY, FONT2, 8, (UCHAR *)stdmsg_C_pin_ok );
	UI_PutMsg( 1, COL_MIDWAY, FONT1, 6, (UCHAR *)stdmsg_pin_ok );
}

// ---------------------------------------------------------------------------
// FUNCTION: display "PLEASE WAIT"
// ---------------------------------------------------------------------------
void	PP_show_please_wait( void )
{
	UI_ClearScreen();
	// UI_PutMsg( 0, COL_MIDWAY, FONT2+attrCLEARWRITE, 6, (UCHAR *)stdmsg_C_please_wait );
	UI_PutMsg( 1, COL_MIDWAY, FONT1+attrCLEARWRITE, 11, (UCHAR *)stdmsg_please_wait );
}

// ---------------------------------------------------------------------------
// FUNCTION: request PIN (4-12 digits) from cardholder.
// INPUT   : amt   - total amount to be confirmed on display. (external)
//                   format: LEN(1) + ASCII(n)
//			     e.g., 0x07, "$100.00"
//	     tout  - time out for PIN entry in seconds.
// OUTPUT  : none.
// REF	   : g_dhn_pinpad.
// RETURN  : apiOK           - confirmed. (by entering OK)
//           apiFailed       - aborted.   (by entering CANCEL)
//           apiDeviceError  - PIN pad is malfunctioning.
//           apiOutOfService - timeout. (-1)
// ---------------------------------------------------------------------------
UCHAR	PP_GetPIN( UINT tout, UCHAR *amt )
{
UCHAR	status;


	// special case for EMV test requirement
	if( g_dhn_pinpad == apiDeviceError )
	  return( apiDeviceError );

	status = api_ped_GetPin( tout, amt );
	switch( status )
	      {
	      case apiOK:
	      	   break;
	      	   
	      case apiNotReady:	// bypass
	      	   status = apiFailed;
	      	   break;
	      	   
	      case apiFailed:	// timeout
	      	   status = apiOutOfService;
	      	   break;
	      }

	return( status );
}

// ---------------------------------------------------------------------------
// FUNCTION: construct a plaintext PIN block. (ISO 9564 format 2)
// INPUT   : none.
// OUTPUT  : pinblock - the plaintext pin block.
// RETURN  : none.
// ---------------------------------------------------------------------------
//void	PP_GenPinBlock( UCHAR *pinblock )	// 2014-10-29, removed.
//{
//	api_ped_GenPinBlock( pinblock );
//}

// ---------------------------------------------------------------------------
// FUNCTION: construct an encrypted PIN block. (by using fixed key)
// INPUT   : pan    - full PAN digits or transaction field for ISO3.
//		      format: L-V(n)
//	     mod      ISO format	(new)
//	     idx      key slot index	(new)
// OUTPUT  : epb    - the encrypted pin block.
//		      format: LL-V(8)
//	     ksn    - key serial number for DUKPT if available.
//		      format: LL-V(10)
// RETURN  : apiOK
//	     apiFailed
// ---------------------------------------------------------------------------
//UCHAR	PP_GenEncrypedPinBlock( UCHAR *pan, UCHAR *epb, UCHAR *ksn )
UCHAR	PP_GenEncrypedPinBlock( UCHAR *pan, UCHAR *epb, UCHAR *ksn, UCHAR mod, UCHAR idx )
{
UCHAR	result;
UCHAR	mode;
UCHAR	index;
UCHAR	temp[12];
UCHAR	epb2[16];
UCHAR	ksn2[10];


	api_emv_ClrDataElement( DE_TERM, ADDR_TERM_EPIN_DATA, 10, 0x00 );
	api_emv_ClrDataElement( DE_TERM, ADDR_TERM_KSN, 12, 0x00 );

	memset( ksn, 0x00, 12 );
	memset( epb, 0x00, 10 );
	
	result = apiFailed;
	
	// The following two settings shall be assigned according to the actual key management requirement
	mode = mod	;	//EPB_ISO0;	// algorithm for PIN block - ISO9564 Format 0
	index = idx;		//0;		// key slot index for PIN usage - key slot #0
		
	switch( api_ped_GetKeyMode() )
	      {
#if 0
	      case PED_KEY_MODE_FIXED:
      	    
		   result = api_ped_GenPinBlock_FXKEY( mode, index, pan, epb2 );
		   if( result == apiOK )
		     {
		     epb[0] = 8;
		     epb[1] = 0;
		     memmove( &epb[2], epb2, 8 );
		     api_emv_PutDataElement( DE_TERM, ADDR_TERM_EPIN_DATA, 10, epb );	// save EPB for online transmission
		     }
		   break;
#endif
      	    	 
	      case PED_KEY_MODE_MS:
      	    
		   result = api_ped_GenPinBlock_MSKEY( mode, index, pan, epb2 );
		   if( result == apiOK )
		     {
		     epb[0] = 8;
		     epb[1] = 0;
		     memmove( &epb[2], epb2, 8 );
		     api_emv_PutDataElement( DE_TERM, ADDR_TERM_EPIN_DATA, 10, epb );	// save EPB for online transmission
		     }
		   break;
      	    	 
	      case PED_KEY_MODE_DUKPT:
      	    
		   result = api_ped_GenPinBlock_DUKPT( mode, pan, epb2, ksn2 );
		   if( result == apiOK )
		     {
		     epb[0] = 8;
		     epb[1] = 0;
		     memmove( &epb[2], epb2, 8 );
		     api_emv_PutDataElement( DE_TERM, ADDR_TERM_EPIN_DATA, 10, epb );	// save EPB for online transmission
		     
		     ksn[0] = 10;
	  	     ksn[1] = 0;
	  	     memmove( &ksn[2], ksn2, 10 );
	  	     api_emv_PutDataElement( DE_TERM, ADDR_TERM_KSN, 12, ksn );		// save KSN for online transmission
		     }
		   break;
		   
	      case PED_KEY_MODE_ISO4:	// 2019-03-26
      	    
      	    	   result = api_ped_GenPinBlock_AESKEY( mode, index, pan, epb2 );	// must be mode=4, index=0
		   if( result == apiOK )
		     {
		     epb[0] = 16;
		     epb[1] = 0;
		     memmove( &epb[2], epb2, 16 );
		     api_emv_PutDataElement( DE_TERM, ADDR_TERM_EPIN_DATA, 18, epb );	// save EPB for online transmission
		     }
		   break;
      	    }
	
	return( result );
}
