//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : APK_RREC.C                                                 **
//**  MODULE   : apk_ReadRecord()                                           **
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
//#include <GDATAEX.h>
//#include <EMVAPI.h>
#include "EMVAPK.h"
#include "APDU.H"
//#include <TOOLS.h>

// ---------------------------------------------------------------------------
// FUNCTION: Read a file record in a linear file.
// INPUT   : sfi    - short file id.
//           recnum - record number.
// OUTPUT  : recdata - 2L-V, the record data read.
// RETURN  : apkOK               (9000, 61La)
//           apkRecNotFound      (6A83)
//           apkFailed
// ---------------------------------------------------------------------------
UCHAR apk_ReadRecord( UCHAR sfi, UCHAR recnum, UCHAR *recdata )
{
UINT  iLength;


      if( apdu_READ_RECORD( sfi, recnum, recdata ) != apiOK )
        return( apkFailed );

      apk_CheckIsoPadding_Left( recdata );

      iLength = recdata[1]*256 + recdata[0];

      if( iLength > 2 )
        {
        if( ( recdata[iLength] == 0x90 ) && ( recdata[iLength+1] == 0x00 ) )
          return( apkOK );

        if( recdata[iLength] == 0x61 )
          return( apkOK );
        else
          return( apkFailed );
        }
      else // return only SW1 SW2
        {
        if( iLength == 2 )
          {
          // PATCH: PBOC2.0, 2006-02-11, 2CB02.30.1 EMV bulletin 29 (EMV4.1 BOOK1 P142)
          if( (recdata[2] == 0x6a) && (recdata[3] == 0x83) )
            return( apkRecNotFound );
          else
            return( apkFailed );
          }
        else
          return( apkFailed );
        }
}

