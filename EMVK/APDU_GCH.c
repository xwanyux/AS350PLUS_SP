//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : APDU_VER.C                                                 **
//**  MODULE   : apdu_GET_CHALLENGE()                                       **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2003/01/23                                                 **
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
//#include <EMVAPI.h>

// ---------------------------------------------------------------------------
// FUNCTION: the GET CHALLENGE command is used to obtain an unpredictable
//           number from the ICC for use in security-related procedure.
// INPUT   : none.
// OUTPUT  : response - 2L-V, V=RANDOM[8] SW1 SW2
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR apdu_GET_CHALLENGE( UCHAR *response )
{
UCHAR i;
UCHAR c_apdu[7];

      // --- DEBUG ONLY ---
#ifdef  L2_SW_DEBUG

      response[0] = 10;
      response[1] = 0;

      for( i=0; i<8; i++ )
         response[i+2] = i;

      response[i+2] = 0x90;
      response[i+3] = 0x00;

      return( apiOK );

#endif
      // ------------------

      c_apdu[0] = 5;                     // length of APDU
      c_apdu[1] = 0;                     //

      c_apdu[2] = 0x00;                     // CLA
      c_apdu[3] = 0x84;                     // INS
      c_apdu[4] = 0x00;                     // P1
      c_apdu[5] = 0x00;                     // P2
      c_apdu[6] = 0x00;                     // Le

//    *response = 0x2C;		// allocate max storage 300 bytes for SC response
//    *(response+1) = 0x01;	//
//printf("\napdu_GET_CHALLENGE() g_dhn_icc=%x\n", g_dhn_icc );
      return( api_ifm_exchangeAPDU( g_dhn_icc, c_apdu, response ) );
}


