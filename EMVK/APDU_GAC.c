//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : APDU_GAC.C                                                 **
//**  MODULE   : apdu_GENERATE_AC()                                         **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2003/01/30                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2003-2007 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include "POSAPI.h"
//#include "EMVDC.h"
#include "GDATAEX.h"
#include "EMVAPI.h"

// ---------------------------------------------------------------------------
// FUNCTION: the GENERATE AC (Application Cryptogram) command sends
//           transaction-related data to the ICC, which computes and returns
//           a cryptogram.
// INPUT   : qualifier - qualify the type of the cryptogram. (AC_XXX)
//           cdol      - 1L-V, CDOL1 or CDOL2.
// OUTPUT  : ac        - 2L-V, cryptogram-related data.
//                             V: 80-L-[Data] SW1 SW2      or
//                             V: 77-L-[...Data...] SW1 SW2
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR apdu_GENERATE_AC( UCHAR qualifier, UCHAR *cdol, UCHAR *ac )
{
UCHAR i;
UCHAR c_apdu[252+8];
UINT  iLen;

      // --- DEBUG ONLY ---
#ifdef  L2_SW_DEBUG

      // FORMAT 1: 80-L-V SW1 SW2
//    ac[0] = 22;
//    ac[1] = 0;
//
//    ac[2] = 0x80;
//    ac[3] = 18;
//
//    ac[4] = 0x40;     // cryptogram information data [1]
//    ac[5] = 0x00;     // ATC[2]
//    ac[6] = 0x01;
//    ac[7]  = 0x11;    // AC[8]
//    ac[8]  = 0x22;
//    ac[9]  = 0x33;
//    ac[10] = 0x44;
//    ac[11] = 0x55;
//    ac[12] = 0x66;
//    ac[13] = 0x77;
//    ac[14] = 0x88;
//
//    ac[15] = 0x06;    // issuer application data [7]
//    ac[16] = 0x01;
//    ac[17] = 0x0a;
//    ac[18] = 0x04;
//    ac[19] = 0x00;
//    ac[20] = 0x00;
//    ac[21] = 0x00;
//
//    ac[22] = 0x90;
//    ac[23] = 0x00;

      // FORMAT 2: 77-L-[...V...] SW1 SW2

      ac[0] = 34;
      ac[1] = 0;

      ac[2] = 0x77;
      ac[3] = 30;

      ac[4] = 0x9f;
      ac[5] = 0x27;
      ac[6] = 1;
      ac[7] = 0x40;     // CID[1]

      ac[8] = 0x9f;
      ac[9] = 0x36;
      ac[10] = 0x02;
      ac[11] = 0x00;    // ATC[2]
      ac[12] = 0x01;

      ac[13] = 0x9f;    // AC[8]
      ac[14] = 0x26;
      ac[15] = 0x08;
      ac[16] = 0x11;
      ac[17] = 0x22;
      ac[18] = 0x33;
      ac[19] = 0x44;
      ac[20] = 0x55;
      ac[21] = 0x66;
      ac[22] = 0x77;
      ac[23] = 0x88;

      ac[24] = 0x9f;
      ac[25] = 0x10;
      ac[26] = 0x07;
      ac[27] = 0x06;    // issuer application data [7]
      ac[28] = 0x01;
      ac[29] = 0x0a;
      ac[30] = 0x04;
      ac[31] = 0x00;
      ac[32] = 0x00;
      ac[33] = 0x00;

      ac[34] = 0x90;
      ac[35] = 0x00;

      return( apiOK );

#endif
      // ------------------

      iLen = cdol[0] + 6;

      c_apdu[0] = iLen & 0x00ff;        // length of APDU
      c_apdu[1] = (iLen & 0xff00) >> 8; //

      c_apdu[2] = 0x80;                     // CLA
      c_apdu[3] = 0xae;                     // INS
      c_apdu[4] = qualifier;                // P1
      c_apdu[5] = 0x00;                     // P2
      c_apdu[6] = cdol[0];                  // Lc
      for(i=0; i<cdol[0]; i++)              // Data
         c_apdu[7+i] = cdol[i+1];           //
      c_apdu[7+i] = 0x00;                   // Le

      return( api_ifm_exchangeAPDU( g_dhn_icc, c_apdu, ac ) );
}


