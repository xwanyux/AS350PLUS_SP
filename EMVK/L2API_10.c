//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : L2API_10.C                                                 **
//**  MODULE   : api_emv_FloorLimitChecking()                               **
//**             api_emv_RandomTransactionSelection()                       **
//**             api_emv_VelocityChecking()                                 **
//**             api_emv_TerminalRiskManagement()                           **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2003/01/27                                                 **
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
// FUNCTION: The TRM is that portion of risk management performed by the
//           terminal to:
//           -- protect the acquirer, issuer, and system from fraud.
//           -- provide positive issuer autheorization for high-value trans.
//           -- ensure that trans. initiated from ICCs go online periodically
//              to protect against threats that might be undetectable in an
//              offline environment.
//
//         * (1) Floor limit checking.
//           (2) Random transaction selection.
//           (3) Velocity checking.
// INPUT   : amt1 - the current transaction amount, including adjustments. (n10+n2)
//           amt2 - the most recent entry amount with the same PAN. (n10+n2)
//                  this amount value may be zero due to no matched PAN or
//                  no log available.
//                  where: n2 (minor unit, is reserved for decimal part)
// OUTPUT  : none.
// REF     :
// RETURN  : emvOK       (amount > TFL)
//           emvFailed   (amount <=TFL)
//           emvNotReady (skip checking)
// ---------------------------------------------------------------------------
UCHAR api_emv_FloorLimitChecking( UCHAR *amt1, UCHAR *amt2 )
{
//    if( (g_icc_AIP[0] & AIP0_TRM) == 0 ) // TRM supported in AIP?
//      return( emvNotReady ); // no, skip checking

      // Floor Limit Checking
      if( apk_FloorLimitChecking( amt1, amt2 ) == apkOK )
        {
        g_term_TVR[3] |= TVR3_TX_EXCEEDS_FLOOR_LIMIT;
        api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR+3, 1, &g_term_TVR[3] );

        return( emvOK );
        }
      else
        return( emvFailed );

}

// ---------------------------------------------------------------------------
// FUNCTION: The TRM is that portion of risk management performed by the
//           terminal to:
//           -- protect the acquirer, issuer, and system from fraud.
//           -- provide positive issuer autheorization for high-value trans.
//           -- ensure that trans. initiated from ICCs go online periodically
//              to protect against threats that might be undetectable in an
//              offline environment.
//
//           (1) Floor limit checking.
//         * (2) Random transaction selection.
//           (3) Velocity checking.
// INPUT   : amt1 - the current transaction amount, including adjustments.
//           amt2 - the current transaction amount, excluding adjustments.
// OUTPUT  : none.
// REF     :
// RETURN  : emvOK       (selected for online)
//           emvFailed   (not selected)
//           emvNotReady (skip checking)
// ---------------------------------------------------------------------------
UCHAR api_emv_RandomTransactionSelection( UCHAR *amt1, UCHAR *amt2 )
{
//    if( (g_icc_AIP[0] & AIP0_TRM) == 0 ) // TRM supported in AIP?
//      return( emvNotReady ); // no, skip checking

      if( apk_RandomTransactionSelection( amt1, amt2 ) == apkOK )
        {
        g_term_TVR[3] |= TVR3_TX_SELECTED_RAND_ONLINE;
        api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR+3, 1, &g_term_TVR[3] );

        return( emvOK );
        }
      else
        return( emvFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: The TRM is that portion of risk management performed by the
//           terminal to:
//           -- protect the acquirer, issuer, and system from fraud.
//           -- provide positive issuer autheorization for high-value trans.
//           -- ensure that trans. initiated from ICCs go online periodically
//              to protect against threats that might be undetectable in an
//              offline environment.
//
//           (1) Floor limit checking.
//           (2) Random transaction selection.
//         * (3) Velocity checking.
// INPUT   : none.
// OUTPUT  : none.
// REF     :
// RETURN  : emvOK       (should be selected for online)
//           emvFailed   (not selected)
//           emvNotReady (skip checking)
//
//           emvOutOfService (invalid status word of GET_DATA -- trans. shall be terminated)
// ---------------------------------------------------------------------------
UCHAR api_emv_VelocityChecking( void )
{
UCHAR result;

//    if( (g_icc_AIP[0] & AIP0_TRM) == 0 ) // TRM supported in AIP?
//      return( emvNotReady ); // no, skip checking

      result = apk_VelocityChecking();
      switch( result )
            {
            case apkOK:
                 break;

            case apkIncorrect:
                 break;

            case apkTRM4:

                 g_term_TVR[3] |= (TVR3_LCOFF_LIMIT_EXCEEDED + TVR3_UCOFF_LIMIT_EXCEEDED);
                 api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR+3, 1, &g_term_TVR[3] );
                 break;

            case apkNotReady:

                 g_term_TVR[0] |= TVR0_ICC_DATA_MISSING;
                 api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR, 1, &g_term_TVR[0] );

                 g_term_TVR[3] |= (TVR3_LCOFF_LIMIT_EXCEEDED + TVR3_UCOFF_LIMIT_EXCEEDED);
                 api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR+3, 1, &g_term_TVR[3] );
                 break;

            case apkOutOfService:

                 return( emvOutOfService );

            default: // check TRM1, TRM2, TRM3, TRM5

                 result &= 0x0F;

                 if( result & apkTRM1 )
                   g_term_TVR[3] |= TVR3_LCOFF_LIMIT_EXCEEDED;

                 if( result & apkTRM2 )
                   g_term_TVR[3] |= TVR3_UCOFF_LIMIT_EXCEEDED;

                 api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR+3, 1, &g_term_TVR[3] );

                 if( result & apkTRM3 )
                   {
                   g_term_TVR[1] |= TVR1_NEW_CARD;
                   api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR+1, 1, &g_term_TVR[1] );
                   }

                 if( result & apkTRM5 ) // ATC missing
                   {
                   g_term_TVR[0] |= TVR0_ICC_DATA_MISSING;
                   api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR, 1, &g_term_TVR[0] );

                   g_term_TVR[3] |= (TVR3_LCOFF_LIMIT_EXCEEDED + TVR3_UCOFF_LIMIT_EXCEEDED);
                   api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR+3, 1, &g_term_TVR[3] );
                   }

                 return( emvOK );

            }

        return( emvFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: The TRM is that portion of risk management performed by the
//           terminal to:
//           -- protect the acquirer, issuer, and system from fraud.
//           -- provide positive issuer autheorization for high-value trans.
//           -- ensure that trans. initiated from ICCs go online periodically
//              to protect against threats that might be undetectable in an
//              offline environment.
//
//         * (1) Floor limit checking.
//         * (2) Random transaction selection.
//         * (3) Velocity checking.
// INPUT   : amt_tx   - the current transaction amount, including adjustments. (n10+n2)
//           amt_auth - the current transaction amount, excluding adjustments.
//           amt_log  - the most recent entry amount with the same PAN. (n10+n2)
//                      this amount value may be zero due to no matched PAN or
//                      no log available.
//                      where: n2 (minor unit, is reserved for decimal part)
// OUTPUT  : none.
// REF     :
// RETURN  : emvOK     (TRM performed)
//           emvFailed (TRM not performed)
//
//           emvOutOfService (trans. shall be terminated)
// ---------------------------------------------------------------------------
UCHAR api_emv_TerminalRiskManagement( UCHAR *amt_tx, UCHAR *amt_auth, UCHAR *amt_log )
{
// 2011-01-07, remove the following condition according to the NOTE of section 10.6 on EMV 4.2 BOOK3
//	       also required on the MasterCard TIP test case: INT 27 CONF T01 S01 
// 2011-07-19, Visa ADVT     : Test case 4
//      if( (g_icc_AIP[0] & AIP0_TRM) == 0 ) // TRM supported in AIP?
//        return( emvFailed );               // no, skip checking

      // 1. TRM -- Floor Limit Checking
      api_emv_FloorLimitChecking( amt_tx, amt_log );

      // 2. TRM -- Random Transaction Selection
      api_emv_RandomTransactionSelection( amt_tx, amt_auth );

      // 3. TRM -- Velocity Checking
      if( api_emv_VelocityChecking() == emvOutOfService )
        return( emvOutOfService ); // PATCH: 2006-10-02

      g_term_TSI[0] |= TSI0_TRM_PERFORMED;
      api_emv_PutDataElement( DE_TERM, ADDR_TERM_TSI, 1, &g_term_TSI[0] );

      return( emvOK );
}

