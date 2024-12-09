//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : APDU_GPO.C                                                 **
//**  MODULE   : apdu_GET_PROCESSING_OPTIONS()                              **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2002/12/25                                                 **
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
// FUNCTION: The GET PROCESSING OPTIONS command initiates the transaction
//           within the ICC.
// INPUT   : pdol - T-L-V, the PDOL related data elements.
// OUTPUT  : data - 2L-V, the returned data object. (AIP, AFL)
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR apdu_GET_PROCESSING_OPTIONS( UCHAR *pdol, UCHAR *data )
{
UCHAR i;
UCHAR c_apdu[254];
UINT  iLen;
//UCHAR cnt;

      // --- DEBUG ONLY ---
#ifdef  L2_SW_DEBUG

//UCHAR rapdu[]={ 0x80, 0x06,                          // FORMAT (1)
//             // 0x1c, 0x00, 0x08, 0x01, 0x05, 0x02,  // without SDA
//                0x5c, 0x00, 0x08, 0x01, 0x05, 0x02,  // with SDA
//                0x90, 0x00 };

UCHAR rapdu[]={ 0x77, 0x0a,                          // FORMAT (2)
                0x82, 0x02,  0x5c, 0x00,
             // 0x94, 0x04,  0x08, 0x01, 0x05, 0x00, // without SDA
                0x94, 0x04,  0x08, 0x01, 0x05, 0x02, // with SDA
                0x90, 0x00 };

      data[0] = 14;
      data[1] = 0;

      memmove( &data[2], rapdu, 14 );
      return( 0 );

#endif
      // ------------------

      if( (pdol[1] & 0x80) == 0 )       // check BER length
        iLen = pdol[1] + 8;             // 6 + (tag83 + len)
      else
        iLen = pdol[2] + 9;             // 6 + (tag83 + 81 + len)

      c_apdu[0] = iLen & 0x00ff;        // length of APDU
      c_apdu[1] = (iLen & 0xff00) >> 8; //

      c_apdu[2] = 0x80;                     // CLA
      c_apdu[3] = 0xa8;                     // INS
      c_apdu[4] = 0x00;                     // P1
      c_apdu[5] = 0x00;                     // P2
      c_apdu[6] = iLen - 6;                 // Lc
      for(i=0; i<c_apdu[6]; i++)            // Data=pdol (83-L-V)
         c_apdu[7+i] = pdol[i];             //
      c_apdu[7+i] = 0x00;                   // Le

//    *data = 0x2C;		// allocate max storage 300 bytes for SC response
//    *(data+1) = 0x01;		//
      return( api_ifm_exchangeAPDU( g_dhn_icc, c_apdu, data ) );
}

