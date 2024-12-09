//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : APDU_SEL.C                                                 **
//**  MODULE   : apdu_SELECT()                                              **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2002/12/06                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2002-2007 SymLink Corporation. All rights reserved.      **
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
// FUNCTION: The SELECT command is used to select the ICC PSE, DDF, or ADF
//           corresponding to the submitted file name or AID.
// INPUT   : filename   - 1L-V
//           occurrence - 0x00=first or only occurrence.
//                        0x01=next occurrence.
// OUTPUT  : fci - 2L-V type, the file control information.
// REF     : g_dhn_icc
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR apdu_SELECT( UCHAR *filename, UCHAR occurrence, UCHAR *fci )
{
UCHAR i;
UCHAR c_apdu[24];

      // --- DEBUG ONLY ---
#ifdef  L2_SW_DEBUG

UCHAR rapdu[]={ 0x6f, 0x15, 0x84, 0x0e, 0x31, 0x50, 0x41, 0x59, 0x2e,
                0x53, 0x59, 0x53, 0x2e, 0x44, 0x44, 0x46, 0x30, 0x31,
                0xa5, 0x03, 0x88, 0x01, 0x01, 0x90, 0x00};

      fci[0] = 25;
      fci[1] = 0;

      memmove( &fci[2], rapdu, 25 );
      return( 0 );

#endif
      // ------------------

      c_apdu[0] = filename[0] + 6;      // length of APDU
      c_apdu[1] = 0x00;                 //
      c_apdu[2] = 0x00;                     // CLA
      c_apdu[3] = 0xa4;                     // INS
      c_apdu[4] = 0x04;                     // P1
      c_apdu[5] = (occurrence << 1) & 0x03; // P2
      c_apdu[6] = filename[0];              // Lc
      for(i=0; i<filename[0]; i++)          // Data=filename (5~16 bytes)
         c_apdu[7+i] = filename[1+i];       //
      c_apdu[7+i] = 0x00;                   // Le

//    *fci = 0x2C;		// allocate max storage 300 bytes for SC response
//    *(fci+1) = 0x01;		//
      return( api_ifm_exchangeAPDU( g_dhn_icc, c_apdu, fci ) );
}

