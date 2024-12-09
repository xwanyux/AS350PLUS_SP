//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : APDU_XAU.C                                                 **
//**  MODULE   : apdu_EXTERNAL_AUTHENTICATE()                               **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2003/02/06                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2003-2007 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include "POSAPI.h"
//#include <EMVDC.h>
#include "GDATAEX.h"
#include "EMVAPI.h"

// ---------------------------------------------------------------------------
// FUNCTION: the EXTERNAL AUTHENTICATE command asks the application in the ICC
//           to verify a cryptogram.
// INPUT   : iad      - 2L-V, the issuer authentication data.
//                      mandatory: 8 bytes containing the cryptogram.
//                      optional : 1~8 bytes are proprietary.
// OUTPUT  : response - 2L-V, the status word (SW1 SW2).
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR apdu_EXTERNAL_AUTHENTICATE( UCHAR *iad, UCHAR *response )
{
UCHAR i;
UCHAR c_apdu[254+8];
UINT  iLen;

      // --- DEBUG ONLY ---
#ifdef  L2_SW_DEBUG

      response[0] = 2;
      response[1] = 0;
      response[2] = 0x90;
      response[3] = 0x00;

      return( apiOK );

#endif
      // ------------------

      iLen = iad[0] + 5;
      c_apdu[0] = iLen & 0x00ff;         // length of APDU
      c_apdu[1] = (iLen & 0xff00) >> 8;  //

      c_apdu[2] = 0x00;                     // CLA
      c_apdu[3] = 0x82;                     // INS
      c_apdu[4] = 0x00;                     // P1
      c_apdu[5] = 0x00;                     // P2
      c_apdu[6] = iad[0];                   // Lc
      for(i=0; i<c_apdu[6]; i++)            // Data=ddol
         c_apdu[7+i] = iad[i+2];            //

      return( api_ifm_exchangeAPDU( g_dhn_icc, c_apdu, response ) );
}


