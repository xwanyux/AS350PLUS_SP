//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : 							    **
//**  PRODUCT  : AS350 PLUS						    **
//**                                                                        **
//**  FILE     : OS_PIN3-1.C						    **
//**  MODULE   : 			   				    **
//**                                                                        **
//**  FUNCTION : OS::PED3-1 (PIN Block Generation by AES-DUKPT)		    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2023/04/11                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2023 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include <string.h>

#if	1
#include "PEDKconfig.h"

#include "POSAPI.h"
#include "OS_LIB.h"
#include "OS_SECM.h"
#include "OS_PED.h"

//#include "DEV_SRAM.h"
#include "DEV_PED.h"
#include "ANS_TR31_2010.h"
#include "SRED_Func.h"
#else
#include "bsp_types.h"
#include "POSAPI.h"
#include "OS_LIB.h"
#include "OS_SECM.h"
#include "OS_PED.h"
#include "DEV_SRAM.h"
#include "DEV_PED.h"
#include "ANS_TR31.h"
#endif


extern  UINT8   *ped_pin_ptr;
extern	UCHAR	AES_DUKPT_RequestPinEntry(  UCHAR keyType, UCHAR *pan, UCHAR *pinblock, UCHAR *ksn, UCHAR *epb );	// defined in DUKPT-AES.c


// ---------------------------------------------------------------------------
//  This function is used to replace "PED_DUKPT_GenPinBlock()" in OS_PED3.c
// ---------------------------------------------------------------------------
// FUNCTION: To generate encrypted PIN block by using DUKPT key algorithm.
// INPUT   : mode   - algorithm of PIN block.
//		     bit1~4: (original)
//		     0: ISO 9564 Format 0 (ANSI X9.8)
//		     1: ISO 9564 Format 1
//		     2: ISO 9564 Format 2 (NA)
//		     3: ISO 9564 Format 3
//		     bit5~8: (new for key type)
//			 0: _2TDEA_
//			 1: _3TDEA_
//			 2: _AES128_
//			 3: _AES192_
//			 4: _AES256_
//	         pan    - PAN digits. (L-V)
// OUTPUT  : epb    - encrypted pin block (8/16/24/32 bytes).
//	         ksn    - key serial number.  (fixed 12 bytes).
// RETURN  : apiOK
//           apiFailed
//	         apiDeviceError	// PED inoperative due to over use limit.
//
// NOTE    : the first parameter of the following PED API shall be modified.
//	         api_ped_GenPinBlock_DUKPT( UINT mode, UCHAR *pan, UCHAR *epb, UCHAR *ksn );
// ---------------------------------------------------------------------------
//UINT8	PED_AES_DUKPT_GenPinBlock( UINT16 mode, UINT8 *pan, UINT8 *epb, UINT8 *ksn )
UINT8	PED_AES_DUKPT_GenPinBlock( UINT8 mode, UINT8 *pan, UINT8 *epb, UINT8 *ksn )
{
UINT8	pin[PED_PIN_SLOT_LEN];
UINT8	pinblock[8];

UINT8	result;
UINT16	keyType;
UINT8   isoFormat;


	result = apiOK;
	
    // isoFormat = mode & 0x00ff;
	// keyType = (mode & 0xff00) >> 8;

    isoFormat = mode & 0x0f;
	keyType = mode & 0xf0;

    if((isoFormat == EPB_ISO0) || (isoFormat == EPB_ISO1) || (isoFormat == EPB_ISO3))
    {
        if((keyType == 2) || (keyType == 3) || (keyType == 4))  // AES-128, AES-192, AES-256
            return apiFailed;
    }
    else if(isoFormat == EPB_ISO4)
    {
        if((keyType == 0) || (keyType == 1))    // TDES-128, TDES-192
            return apiFailed;
    }
	
	// restore PIN data
	// OS_SECM_GetData( ADDR_PED_PIN, PED_PIN_SLOT_LEN, pin ); // L-V
    memcpy(pin, ped_pin_ptr, PED_PIN_SLOT_LEN); // L-V
	if( pin[0] == 0 )
	    return( apiFailed ); // PIN not available
	
	PED_InUse( TRUE );
	
	// generate plaintext PIN Block
    switch(isoFormat)
	      {
	      case EPB_ISO0:

                if(pan[0] != 0)
                    PED_GenPinBlock_ISO0(pin, pan, pinblock);
	           else
	             result = apiFailed;
	             
	  	   break;
	      	   
	      case EPB_ISO1:
	      
	           PED_GenPinBlock_ISO1( pin, pinblock );
	           break;
	      
#if 0
	      case EPB_ISO2:
	      
	           PED_GenPinBlock_ISO2( pin, pinblock );
	           break;
#endif
	           
	      case EPB_ISO3:

                if(pan[0] != 0)
				   PED_GenPinBlock_ISO3(pin, pan, pinblock);
	      	   else
	      	     result = apiFailed;
	      	     
	           break;

          case EPB_ISO4:
            
               break;
	             
	      default:
	           result = apiFailed;
	           break;
	      }

	// generate EPB & KSN by using DUKPT key
	if( result == apiOK )
	  {
	  if( !AES_DUKPT_RequestPinEntry( keyType, pan, pinblock, ksn, epb ) )	// NEW
	    result = apiDeviceError;
	  }

	memset( pin, 0x00, sizeof(pin) );			// clear PIN buffer
	memset( pinblock, 0x00, sizeof(pinblock) );		// clear PIN block

	// OS_SECM_ClearData( ADDR_PED_PIN, PED_PIN_SLOT_LEN, 0 );	// clear PIN data
    if(ped_pin_ptr != NULL)
        PED_ClearPin();  // deallocate PIN data memory
	
	PED_InUse( FALSE );
	
	return( result );
}
