//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : APDU_XAU.C                                                 **
//**  MODULE   : apdu_ISSUER_SCRIPT_COMMAND()                               **
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
// FUNCTION: send one issuer script command to ICC.
// INPUT   : len      - length of the issuer script command.
//           cmd      - the issuer script command.
// OUTPUT  : response - 2L-V, the status word (SW1 SW2).
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR apdu_ISSUER_SCRIPT_COMMAND( UINT len, UCHAR *cmd, UCHAR *response )
{
UINT  i;
UCHAR c_apdu[254+8];

      // --- DEBUG ONLY ---
#ifdef  L2_SW_DEBUG

      response[0] = 2;
      response[1] = 0;
      response[2] = 0x90;
      response[3] = 0x00;

      return( apiOK );

#endif
      // ------------------

      c_apdu[0] = len & 0x00ff;         // length of APDU
      c_apdu[1] = (len & 0xff00) >> 8;  //

      for(i=0; i<len; i++)                  // script command APDU
         c_apdu[2+i] = *cmd++;              //

      return( api_ifm_exchangeAPDU( g_dhn_icc, c_apdu, response ) );
}


