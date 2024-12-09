//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : APDU_VER.C                                                 **
//**  MODULE   : apdu_VERIFY()                                              **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2003/01/22                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2003-2007 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include <string.h>

#include "POSAPI.h"
//#include "EMVDC.h"
#include "GDATAEX.h"
#include "EMVAPI.h"
// ---------------------------------------------------------------------------
// FUNCTION: the VERIFY command initates in the ICC the comparison of the
//           Transaction PIN Data sent in the data field of the command with
//           the reference PIN data associcated with the application.
//           (for CVM offline PIN)
// INPUT   : qualifier - verification data format.
//                       0x80 = Plaintext PIN format.
//                       0x88 = Enciphered PIN format.
//           length    - length of pindata.
//           pindata   - the PIN block data. (formated)
// OUTPUT  : response  - 2L-V, V=SW1 SW2
//                       90 00 - OK
//                       63 Cx - x times try count left.
//                       69 83 - authen method blocked.
//                       69 84 - referenced data invalidated.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR apdu_VERIFY( UCHAR qualifier, UCHAR length, UCHAR *pindata, UCHAR *response )
{
UCHAR i;
UCHAR c_apdu[254+9];

      // --- DEBUG ONLY ---
#ifdef  L2_SW_DEBUG

      response[0] = 2;
      response[1] = 0;

      response[2] = 0x90;
      response[3] = 0x00;

      return( apiOK );

#endif
      // ------------------

      c_apdu[0] = 5 + length;            // length of APDU
      c_apdu[1] = 0;                     //

      c_apdu[2] = 0x00;                     // CLA
      c_apdu[3] = 0x20;                     // INS
      c_apdu[4] = 0x00;                     // P1
      c_apdu[5] = qualifier;                // P2
      c_apdu[6] = length;                   // Lc
      memmove( &c_apdu[7], pindata, length ); // PIN data

      return( api_ifm_exchangeAPDU( g_dhn_icc, c_apdu, response ) );
}


