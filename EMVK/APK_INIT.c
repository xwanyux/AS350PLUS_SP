//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : APK_INIT.C                                                 **
//**  MODULE   : apk_InitApp()                                              **
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
//#include <GDATAEX.h>
#include "EMVAPI.h"
#include "EMVAPK.h"
#include "APDU.H"
//#include <TOOLS.h>

// ---------------------------------------------------------------------------
// FUNCTION: initiate application by issuing "GET PROCESSING OPTIONS" command
//           to ICC.
// INPUT   : pdol - the PDOL data elements.
// OUTPUT  : data - the response data from ICC.
// RETURN  : apkOK               (9000, 61La)
//           apkCondNotSatisfied (6985) --> select next application.
//           apkFailed                  --> terminate transaction.
//           apkFallBack         (6D00, 6E00) --> JCB SPEC, fall back to MSR
// ---------------------------------------------------------------------------
UCHAR apk_InitApp( UCHAR *pdol, UCHAR *data )
{
UINT  iLen;
UCHAR padding;
UCHAR padlen[1];

      if( apdu_GET_PROCESSING_OPTIONS( pdol, data ) != apiOK )
        return( apkFailed );

      apk_CheckIsoPadding_Left( data );

      iLen = data[1]*256 + data[0];

      // check legal length
      if( (iLen != 2) && (iLen < 10) )
        return( emvFailed );

      if( iLen != 2 ) // length >= 10
        {
        if( apk_CheckIsoPadding_Right( data, padlen ) == FALSE )
          return( emvFailed );

        if( ( data[iLen] == 0x90 ) && ( data[iLen+1] == 0x00 ) )
          return( apkOK );

        if( ( data[iLen] == 0x69 ) && ( data[iLen+1] == 0x85 ) )
          return( apkCondNotSatisfied );

        if( data[iLen] == 0x61 )
          return( apkOK );
        else
          return( apkFailed );
        }
      else // return only SW1 SW2
        {
        if( (data[2] == 0x69) && (data[3] == 0x85) )
          return( apkCondNotSatisfied );
        else
          {
    //    if( ((data[2] == 0x6D) && (data[3] == 0x00)) ||
    //        ((data[2] == 0x6E) && (data[3] == 0x00)) )
    //      return( apkFallBack ); // JCB SPEC (deleted by JCB v1.2 amendments)
    //    else
            return( apkFailed );   // EMV SPEC
          }
        }
}

