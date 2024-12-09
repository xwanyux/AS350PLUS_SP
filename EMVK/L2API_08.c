//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : L2API_08.C                                                 **
//**  MODULE   : api_emv_ProcessingRestrictions()                           **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2003/01/20                                                 **
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
#include "EMVAPK.h"

// ---------------------------------------------------------------------------
// FUNCTION: to determine the degree of compatibility of the application in
//           the terminal with the application in the ICC and to make any
//           necessary adjustments, including possible rejection of the
//           transaction.
//           (1) AVN: Application Version Number
//           (2) AUC: Application Usage Control
//           (3) AED: Application Effective/Expiration Dates Checking
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : emvOK
//           emvFailed
// ---------------------------------------------------------------------------
UCHAR api_emv_ProcessingRestrictions( void )
{
UCHAR result;

      // Application Version Number
      if( apk_ProcessRestrict_AVN() == apkFailed )
        g_term_TVR[1] |= TVR1_ICC_TERM_DIFF_AP_VER;

      // Application Usage Control
      if( apk_ProcessRestrict_AUC() == apkFailed )
        g_term_TVR[1] |= TVR1_SERVICE_NOT_ALLOWED;

      // Application Effective / Expiration Date
      result = apk_ProcessRestrict_AED();
      if( result &1 )
        g_term_TVR[1] |= TVR1_AP_NOT_YET_EFFECTIVE;
//      else
//        {
        if( result &2 )
          g_term_TVR[1] |= TVR1_EXPIRED_AP;
//        }

      // update TVR
      api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR+1, 1, &g_term_TVR[1] );

      return( emvOK );
}

