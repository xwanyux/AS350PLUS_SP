//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : APK_PRES.C                                                 **
//**  MODULE   : apk_ProcessRestict_AVN()                                   **
//**             apk_ProcessRestict_AUC()                                   **
//**             apk_ProcessRestrict_AED()                                  **
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
//#include <APDU.H>
#include "TOOLS.h"

// ---------------------------------------------------------------------------
// FUNCTION: processing restriction for Application Version Number.
// INPUT   : none.
// OUTPUT  : none.
// REF     : g_selected_aid_index
// RETURN  : apkOK
//           apkFailed (terminal AVN & icc AVN are different)
// ---------------------------------------------------------------------------
UCHAR apk_ProcessRestrict_AVN( void )
{
//UCHAR i;
//UCHAR term_aid[TERM_AID_LEN];
//UCHAR selected_aid[SELECTED_AID_LEN];
UCHAR term_avn[2];
UCHAR icc_avn[4];

      apk_ReadRamDataICC( ADDR_ICC_AVN, 4, icc_avn ); // read icc AVN

      if( (icc_avn[1]*256+icc_avn[0]) != 0 ) // icc AVN present?
        {
//      // get the selected AID
//      // LEN[1] AID[16]
//      apk_ReadRamDataICC( ADDR_SELECTED_AID, SELECTED_AID_LEN, selected_aid );
//
//      for( i=0; i<MAX_AID_CNT; i++ )
//         {
//         // get term AID
//         // LEN[1] ASI[1] AID[16]
//         apk_ReadRamDataTERM( ADDR_TERM_AID_01+i*TERM_AID_LEN, TERM_AID_LEN, term_aid );
//
//         // matching AID
//      // if( TL_memcmp( &selected_aid[1], &term_aid[2], selected_aid[0] ) == 0 )
//
//         // PATCH: 2003-06-27 for both exact and partial AID matching
//         if( TL_memcmp( &selected_aid[1], &term_aid[2], term_aid[0]-1 ) == 0 )
//           break; // equal
//         }
//
//      g_selected_aid_index = i; // backup the target aid number

        // read target terminal AVN
//      apk_ReadRamDataTERM( ADDR_AVN_01+i*AVN_LEN, AVN_LEN, term_aid );
//      apk_WriteRamDataTERM( ADDR_TERM_AVN, AVN_LEN, term_aid );
        apk_ReadRamDataTERM( ADDR_TERM_AVN, AVN_LEN, term_avn );

        // compared with icc AVN
        if( (icc_avn[2] == term_avn[0]) && (icc_avn[3] == term_avn[1]) )
          return( apkOK );
        else
          return( apkFailed );
        }

      return( apkOK );
}

// ---------------------------------------------------------------------------
// FUNCTION: processing restriction for Application Usage Control.
// INPUT   : none.
// OUTPUT  : none.
// REF     :
// RETURN  : apkOK
//           apkFailed
// NOTE    : only GOODS & SERVICES are supported.
// ---------------------------------------------------------------------------
UCHAR apk_ProcessRestrict_AUC( void )
{
UCHAR icc_auc[4];       // 2L-V[2]
UCHAR isu_cntr_code[4]; // 2L-V[2]
UCHAR term_cntr_code[2];

      apk_ReadRamDataICC( ADDR_ICC_AUC, 4, icc_auc );
      apk_ReadRamDataICC( ADDR_ICC_ISU_CNTR_CODE, 4, isu_cntr_code );

      // AUC present?
      if( (icc_auc[1]*256 + icc_auc[0]) != 0 )
        {
        // valid at terminal other ATM?
        if( (icc_auc[2] & AUC0_VALID_AT_TERM_NOT_ATM) == 0 )
          return( apkFailed );
        }

      // AUC & Country Code both present?
      if( ((icc_auc[1]*256 + icc_auc[0]) != 0) && ((isu_cntr_code[1]*256 + isu_cntr_code[0]) != 0) )
        {
        // issuer country code = terminal countery code?
        apk_ReadRamDataTERM( ADDR_TERM_CNTR_CODE, 2, term_cntr_code );

        if( (isu_cntr_code[2] == term_cntr_code[0]) && (isu_cntr_code[3] == term_cntr_code[1]) )
          {
          // PATCH: PBOC2.0, 2006-02-14, EMV AN#27
          if( ((icc_auc[2] & AUC0_VALID_DOMESTIC_GOODS ) != AUC0_VALID_DOMESTIC_GOODS) &&
              ((icc_auc[2] & AUC0_VALID_DOMESTIC_SERVICES ) != AUC0_VALID_DOMESTIC_SERVICES) )
            return( apkFailed );

          // PATCH: PBOC2.0, 2006-02-10
//        if( g_term_tx_type == TT_GOODS )
//          {
//          if( (icc_auc[2] & AUC0_VALID_DOMESTIC_GOODS ) != AUC0_VALID_DOMESTIC_GOODS )
//            return( apkFailed);
//          }
//        else // TT_SERVICES
//          {
//          if( (icc_auc[2] & AUC0_VALID_DOMESTIC_SERVICES ) != AUC0_VALID_DOMESTIC_SERVICES )
//            return( apkFailed);
//          }

//        // AUC indicates Tx type allowed for deomestic Tx?
//        if( (icc_auc[2] & (AUC0_VALID_DOMESTIC_GOODS + AUC0_VALID_DOMESTIC_SERVICES)) !=
//          (AUC0_VALID_DOMESTIC_GOODS + AUC0_VALID_DOMESTIC_SERVICES) )
//          return( apkFailed);
          }
        else // country codes are different
          {
          // PATCH: PBOC2.0, 2006-02-14, EMV AN#27
          if( ((icc_auc[2] & AUC0_VALID_INTN_GOODS ) != AUC0_VALID_INTN_GOODS) &&
              ((icc_auc[2] & AUC0_VALID_INTN_SERVICES ) != AUC0_VALID_INTN_SERVICES) )
            return( apkFailed );

          // PATCH: PBOC2.0, 2006-02-10
//        if( g_term_tx_type == TT_GOODS )
//          {
//          if( (icc_auc[2] & AUC0_VALID_INTN_GOODS ) != AUC0_VALID_INTN_GOODS )
//            return( apkFailed);
//          }
//        else // TT_SERVICES
//          {
//          if( (icc_auc[2] & AUC0_VALID_INTN_SERVICES ) != AUC0_VALID_INTN_SERVICES )
//            return( apkFailed);
//          }

//        // AUC indicates Tx type allowed for international Tx?
//        if( (icc_auc[2] & (AUC0_VALID_INTN_GOODS + AUC0_VALID_INTN_SERVICES)) !=
//          (AUC0_VALID_INTN_GOODS + AUC0_VALID_INTN_SERVICES) )
//          return( apkFailed );
          }
        }

      return( apkOK );
}

// ---------------------------------------------------------------------------
// FUNCTION: processing restriction for Application Effective/Expiration Dates
//           Checking.
//           YY=00~49: 20YY
//           YY=50~99: 19YY
// INPUT   : none.
// OUTPUT  : none.
// REF     :
// RETURN  : 0 = OK		//apkOK
//           1 = NOT_EFFECTIVE	//apkNotReady - not effetive
//           2 = EXPIRED	//apkFailed   - expired
//	     3 = EXPIRED + NOT_EFFECTIVE
// ---------------------------------------------------------------------------
UCHAR apk_ProcessRestrict_AED( void )
{
UCHAR buf1[13];
UCHAR buf2[4];
UCHAR result = 0;
      // get current date time
      TL_GetDateTime( &buf1[1] );  // current date
      buf1[0]=6;
      TL_asc2bcd( 3, &buf2[1], buf1 );    // convert to BCD, buf2: YYMMDD
      buf2[0] = TL_SetCentury( buf2[1] ); // &buf2[0]: CCYYMMDD, CC=19 or 20

      // get application effective date
      apk_ReadRamDataICC( ADDR_ICC_AP_EFFECT_DATE, 5, buf1 );
      if( (buf1[1]*256 + buf1[0]) != 0 ) // "Effective Date" present?
        {
        // DATE < Effective Date?
        buf1[1] = TL_SetCentury( buf1[2] ); // &buf1[1]: CCYYMMDD, CC=19 or 20

        if( TL_CompareDate( buf2, &buf1[1] ) < 0 )
        	result |= 1;
//          return( apkNotReady );

        }

      // get application expiration date
      apk_ReadRamDataICC( ADDR_ICC_AP_EXPIRE_DATE, 5, buf1 );
      buf1[1] = TL_SetCentury( buf1[2] );   // &buf1[1]: CCYYMMDD, CC=19 or 20

      // DATE > Expiration Date?
      if( TL_CompareDate( buf2, &buf1[1] ) > 0 )
    	  result |= 2;
//        return( apkFailed );
//      else
        return result;
}

