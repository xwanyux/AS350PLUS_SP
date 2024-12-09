//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : APDU_GDT.C                                                 **
//**  MODULE   : apdu_GET_DATA()                                            **
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
#include "POSAPI.h"
//#include <EMVDC.h>
#include "GDATAEX.h"
//#include <EMVAPI.h>

// ---------------------------------------------------------------------------
// FUNCTION: to retrieve a primitve data object not encapsulated in a record
//           within the current applicaiton.
// INPUT   : tag1    - the MSB of Tag of the requested DO. (0 if single tag)
//           tag2    - the LSB of Tag of the requested DO.
//                     e.g.
//                     9F36: ATC (b2)
//                     9F13: Last On Line ATC Register (b2)
//                     9F17: PIN Try Counter (b1)
//                     9F4F: Log Format (EMV2000, var)
// OUTPUT  : dataobj - 2L-Data SW1 SW2, the returned data object.
//                     where: Data=T-L-V (T=P1P2)
//                            SW1 SW2=90 00 -- OK
//                                   =6A 81 -- function not supported
//                                   =6A 88 -- referenced data not found
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR apdu_GET_DATA( UCHAR tag1, UCHAR tag2, UCHAR *dataobj )
{
UCHAR c_apdu[7];

      // --- DEBUG ONLY ---
#ifdef  L2_SW_DEBUG

UCHAR *ptrobj;
UCHAR len;

UCHAR ATC[]=      {
                  0x9f, 0x36, 2, 0x00, 0x01,
                  0x90, 0x00
                  };

UCHAR LON_ATC_REG[]= {
                  0x9f, 0x13, 2, 0x00, 0x01,
                  0x90, 0x00
                  };

UCHAR PIN_TRY_CNT[]= {
                  0x9f, 0x17, 1, 0x03,
                  0x90, 0x00
                  };

      if( (tag1 == 0x9f) && (tag2 == 0x36) )
        {
        len = 7;
        ptrobj = ATC;
        }

      if( (tag1 == 0x9f) && (tag2 == 0x13) )
        {
        len = 7;
        ptrobj = LON_ATC_REG;
        }

      if( (tag1 == 0x9f) && (tag2 == 0x17) )
        {
        len = 6;
        ptrobj = PIN_TRY_CNT;
        }

      dataobj[0] = len;
      dataobj[1] = 0;

      memmove( &dataobj[2], ptrobj, len );

      return( apiOK );

#endif
      // ------------------

      c_apdu[0] = 5;                     // length of APDU
      c_apdu[1] = 0;                     //

      c_apdu[2] = 0x80;                     // CLA
      c_apdu[3] = 0xCA;                     // INS
      c_apdu[4] = tag1;                     // P1
      c_apdu[5] = tag2;                     // P2
      c_apdu[6] = 0;                        // Le

//    *dataobj = 0x2C;		// allocate max storage 300 bytes for SC response
//    *(dataobj+1) = 0x01;	//
      return( api_ifm_exchangeAPDU( g_dhn_icc, c_apdu, dataobj ) );
}


