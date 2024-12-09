//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : APDU_IAU.C                                                 **
//**  MODULE   : apdu_INTERNAL_AUTHENTICATE()                               **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2003/01/18                                                 **
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
// FUNCTION: initiate the computation of the Signed Dynamic Application Data
//           by the card using the challenge data sent from the IFD and data
//           and a relevant private key stored in the card.
// INPUT   : ddol - 1L-V, the DDOL related data elements.
// OUTPUT  : data - 2L-V, the returned data object.
//                  Format1: [80-L-V]
//                  Format1: 77-L-V...[9F4B-L-V]...
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR apdu_INTERNAL_AUTHENTICATE( UCHAR *ddol, UCHAR *data )
{
UCHAR i;
UCHAR c_apdu[254+8];
UINT  iLen;

      // --- DEBUG ONLY ---
//#ifdef  L2_SW_DEBUG
//
//UCHAR vsdc_pkm[]= {
//                   0xC6, 0x96, 0x03, 0x42, 0x13, 0xD7, 0xD8, 0x54, 0x69, 0x84, 0x57, 0x9D, 0x1D, 0x0F, 0x0E, 0xA5,
//                   0x19, 0xCF, 0xF8, 0xDE, 0xFF, 0xC4, 0x29, 0x35, 0x4C, 0xF3, 0xA8, 0x71, 0xA6, 0xF7, 0x18, 0x3F,
//                   0x12, 0x28, 0xDA, 0x5C, 0x74, 0x70, 0xC0, 0x55, 0x38, 0x71, 0x00, 0xCB, 0x93, 0x5A, 0x71, 0x2C,
//                   0x4E, 0x28, 0x64, 0xDF, 0x5D, 0x64, 0xBA, 0x93, 0xFE, 0x7E, 0x63, 0xE7, 0x1F, 0x25, 0xB1, 0xE5,
//                   0xF5, 0x29, 0x85, 0x75, 0xEB, 0xE1, 0xC6, 0x3A, 0xA6, 0x17, 0x70, 0x69, 0x17, 0x91, 0x1D, 0xC2,
//                   0xA7, 0x5A, 0xC2, 0x8B, 0x25, 0x1C, 0x7E, 0xF4, 0x0F, 0x23, 0x65, 0x91, 0x24, 0x90, 0xB9, 0x39,
//                   0xBC, 0xA2, 0x12, 0x4A, 0x30, 0xA2, 0x8F, 0x54, 0x40, 0x2C, 0x34, 0xAE, 0xCA, 0x33, 0x1A, 0xB6,
//                   0x7E, 0x1E, 0x79, 0xB2, 0x85, 0xDD, 0x57, 0x71, 0xB5, 0xD9, 0xFF, 0x79, 0xEA, 0x63, 0x0B, 0x75
//                  };
//
//      pkm[0]=128;
//      pkm[1]=0;
//      memmove( &pkm[2], vsdc_pkm, 128 );
//
//      pke[0]=3;
//      pke[1]=0;
//      pke[2]=0;
//
//      return( apiOK );
//
//#endif
      // ------------------

      iLen = ddol[0] + 6;
      c_apdu[0] = iLen & 0x00ff;         // length of APDU
      c_apdu[1] = (iLen & 0xff00) >> 8;  //

      c_apdu[2] = 0x00;                     // CLA
      c_apdu[3] = 0x88;                     // INS
      c_apdu[4] = 0x00;                     // P1
      c_apdu[5] = 0x00;                     // P2
      c_apdu[6] = ddol[0];                  // Lc
      for(i=0; i<c_apdu[6]; i++)            // Data=ddol
         c_apdu[7+i] = ddol[i+1];           //
      c_apdu[7+i] = 0x00;                   // Le

//    *data = 0x2C;		// allocate max storage 300 bytes for SC response
//    *(data+1) = 0x01;		//
      return( api_ifm_exchangeAPDU( g_dhn_icc, c_apdu, data ) );
}


