//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : L2API_04.C                                                 **
//**  MODULE   : api_emv_PutDataElement()                                   **
//**             api_emv_GetDataElement()                                   **
//**             api_emv_ClrDataElement()                                   **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2002/12/19                                                 **
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

// ---------------------------------------------------------------------------
// FUNCTION: save data element to PAGE SRAM.
// INPUT   : source  - data source (DE_TERM, DE_ICC)
//         : address - begin address to write.
//           length  - length of the data element.
//           data    - the data element to be saved.
// OUTPUT  : none.
// RETURN  : emvOK
//           emvFailed
// ---------------------------------------------------------------------------
UCHAR api_emv_PutDataElement( UCHAR source, ULONG address, ULONG length, UCHAR *data )
{
      switch( source )
            {
            case DE_TERM:
                 apk_WriteRamDataTERM( address, length, data );
                 break;

            case DE_ICC:
                 apk_WriteRamDataICC( address, length, data );
                 break;

            case DE_KEY:
                 apk_WriteRamDataKEY( address, length, data );
                 break;
            }
}

// ---------------------------------------------------------------------------
// FUNCTION: get data element from PAGE SRAM.
// INPUT   : source  - data source (terminal, icc, or issuer).
//         : address - begin address to write.
//           length  - length of the data element.
// OUTPUT  : data    - the data element read.
// RETURN  : emvOK
//           emvFailed
// ---------------------------------------------------------------------------
UCHAR api_emv_GetDataElement( UCHAR source, ULONG address, ULONG length, UCHAR *data )
{
      switch( source )
            {
            case DE_TERM:
                 apk_ReadRamDataTERM( address, length, data );
                 break;

            case DE_ICC:
                 apk_ReadRamDataICC( address, length, data );
                 break;

            case DE_KEY:
                 apk_ReadRamDataKEY( address, length, data );
                 break;
            }
}

// ---------------------------------------------------------------------------
// FUNCTION: clear data element from PAGE SRAM.
// INPUT   : source  - data source (terminal, icc, or issuer).
//         : address - begin address to write.
//           length  - length of the data element.
// OUTPUT  : data    - the data element read.
// RETURN  : emvOK
//           emvFailed
// ---------------------------------------------------------------------------
UCHAR api_emv_ClrDataElement( UCHAR source, ULONG address, ULONG length, UCHAR data )
{
      switch( source )
            {
            case DE_TERM:
                 apk_ClearRamDataTERM( address, length, data );
                 break;

            case DE_ICC:
                 apk_ClearRamDataICC( address, length, data );
                 break;

            case DE_KEY:
                 apk_ClearRamDataKEY( address, length, data );
                 break;
            }
}
