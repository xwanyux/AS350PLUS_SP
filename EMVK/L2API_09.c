//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS330_mPOS						    **
//**                                                                        **
//**  FILE     : L2API_09.C                                                 **
//**  MODULE   : api_emv_CardholderVerification()                           **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2018/08/08                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2018 SymLink Corporation. All rights reserved.	    **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include <string.h>

#include "EMVKconfig.h"

#include "POSAPI.h"
#include "GDATAEX.h"
#include "EMVAPI.h"
#include "EMVAPK.h"

// ---------------------------------------------------------------------------
// FUNCTION: cardholder verification is performed to ensure that the person
//           presenting the ICC is the one to whom the applicaiton in the card
//           was issued.
//           (1) Offline Plaintext PIN.
//           (2) Offline Enciphered PIN.
//           (3) Online PIN.
//           (4) Signature.
// INPUT   : tout      - time out for PIN entry in seconds.
//           msg       - message to be shown on PIN PAD prior to enter PIN.
//                       if no PIN required or LEN=0, this message will be ignored.
//                       Structure: ROW[1] COL[1] FONT[1] LEN[1] MSG[n].
//	     mod       - ISO format
//	     idx       - key slot index
// OUTPUT  : epb       - LL-V, the enciphered PIN data for online (ISO 9564-1).
//                             if LL=0, no online PIN data.
//	     ksn       - LL-V, key serial number for online.
//			       if LL=0, no online KSN data.
// REF     : g_ibuf
// RETURN  : emvOK     - to display "PIN OK" and proceed to TRM.
//           emvFailed - (1) CVM not supported in ICC AIP, in this case
//                           term should perform "MSR CVM" or proceed to TRM.
//                       (2) CVM List not present (action same as 1).
//                       (3) CVM not successful.
//           emvOutOfService - terminate current transaction.
// ---------------------------------------------------------------------------
UCHAR api_emv_CardholderVerification( UINT tout, UCHAR *msg, UCHAR *epb, UCHAR *ksn, UCHAR mod, UCHAR idx )
{
UCHAR i;
UCHAR aip[2];
UINT  iLen;
UCHAR xamt[4]; // X amount
UCHAR yamt[4]; // Y amount
UCHAR mcode;   // method code
UCHAR ccode;   // condition code
UCHAR cnt;
UCHAR *ptrcvm;
UCHAR result;
UCHAR ap_cc[2];  // icc application currency code
UCHAR tx_cc[2];  // terminal transaction currency code
UCHAR amt_b[4];
UCHAR cvml[254]; // 2L-V
UCHAR tcap[3];   // terminal capabilities
UCHAR fCVM = FALSE;
UCHAR fCVMnosup = FALSE; //20090317_Richard: EMV 4.1e, 2CJ.083.04


      memset( epb, 0x00, 10 );	// 2010-03-18
      memset( ksn, 0x00, 12 );	//
      
      // get current terminal capabilites
      apk_ReadRamDataTERM( ADDR_TERM_CAP, 3, tcap );

      // get current transaction amount (binary)
      apk_ReadRamDataTERM( ADDR_TERM_AMT_AUTH_B, 4, amt_b );

      // clear CVM results to "Not Available"
      g_term_CVMR[0] = CVR_NOT_AVAILABLE;
      g_term_CVMR[1] = 0x00;
      g_term_CVMR[2] = 0x00;
      api_emv_PutDataElement( DE_TERM, ADDR_TERM_CVMR, 3, g_term_CVMR );

      // get transaction currency code
      api_emv_GetDataElement( DE_TERM, ADDR_TERM_TX_CC, 2, tx_cc );

      // get icc_AIP[0..1]
      api_emv_GetDataElement( DE_ICC, ADDR_ICC_AIP, 2, aip );
      if( (aip[0] & AIP0_CVM) == 0 ) // CVM supported?
        return( emvFailed );

      // CVM List present?
      api_emv_GetDataElement( DE_ICC, ADDR_ICC_CVM_LIST, 2, (UCHAR *)&iLen );

      if( iLen == 0 )
        {
        g_term_TVR[0] |= TVR0_ICC_DATA_MISSING;
        api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR, 1, &g_term_TVR[0] );

        return( emvFailed );
        }
      else if(iLen&1)
      {
    	  return emvOutOfService;//08-dec-26 charles
      }
      else
        {
        if( iLen < 10 )
          {
          if( iLen == 8 )
            {
            g_term_TVR[0] |= TVR0_ICC_DATA_MISSING; // PATCH: 2006-10-05
            api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR, 1, &g_term_TVR[0] );
            return emvFailed;//09-jan-08 charles
            }
          else
            {
            g_term_TVR[2] |= TVR2_CVM_NOT_SUCCESS;
            api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR+2, 1, &g_term_TVR[2] );
            return emvOutOfService;//08-dec-26 charles ( emvFailed ); // 2CL.044.00, CVML format error
            
            }

          }
        }


      // read CVM List to "cvml[]", length = iLen
      api_emv_GetDataElement( DE_ICC, ADDR_ICC_CVM_LIST+2, iLen, cvml );

      memmove( xamt, &cvml[0], 4 ); // X value
      memmove( yamt, &cvml[4], 4 ); // Y value

      cnt = (iLen - 8) / 2; // total number of pairs of CVM
      ptrcvm = &cvml[8];    // pointer to the start of 2-byte pair CVM

      for( i=0; i<cnt; i++ )
         {
         fCVMnosup = FALSE; //20090317_Richard: EMV 4.1e, 2CJ.083.04
         mcode = *ptrcvm++; // method
       //g_term_CVMR[0] = mcode;
         ccode = *ptrcvm++; // condition
       //g_term_CVMR[1] = ccode;

         // Check BYPASS conditions (and proceed to the next entry)

         // (1) conditions are not satisfied
         // (3) condition code is outside the range of codes
//       if( (ccode > CVC_AMT_OVER_Y) || (ccode == CVC_CASH_OR_CASHBACK)
//         || (ccode == CVC_RFU4) || (ccode == CVC_RFU5) )
//         continue;

         // (2) icc data required by the condition is not present
         //     eg. "Application Currency Code" is not present
         if( (ccode >= CVC_AMT_UNDER_X) && (ccode <= CVC_AMT_OVER_Y) )
           {
           api_emv_GetDataElement( DE_ICC, ADDR_ICC_AP_CC, 2, (UCHAR *)&iLen );
           if( iLen == 0 )
             continue; // app currency code not present
           api_emv_GetDataElement( DE_ICC, ADDR_ICC_AP_CC+2, 2, ap_cc );
           }

         // --------------------------
         //  CVM Condition Processing
         // --------------------------

         switch( ccode )
               {
               case CVC_ALWAYS:               // do unconditionally
               case CVC_NOT_CASH_OR_CASHBACK: // only goods & services
               case CVC_TERM_SUPPORT_CVM:     // always supported

                    break; // process rule

               case CVC_AMT_UNDER_X:

                    if( TL_memcmp( tx_cc, ap_cc, 2 ) != 0 )
                      continue; // not satisfied, next CVM entry

                    // TxAmt < X ?
                //  TL_bcd2hex( 5, g_term_tx_amt, temp );
                    if( TL_memcmp( amt_b, xamt, 4 ) < 0 )
                      break; // process rule
                    else
                      continue; // next CVM entry

               case CVC_AMT_OVER_X:

                    if( TL_memcmp( tx_cc, ap_cc, 2 ) != 0 )
                      continue; // not satisfied, next CVM entry

                    // TxAmt > X ?
                //  TL_bcd2hex( 5, g_term_tx_amt, temp );
                    if( TL_memcmp( amt_b, xamt, 4 ) > 0 )
                      break; // process rule
                    else
                      continue; // next CVM entry

               case CVC_AMT_UNDER_Y:

                    if( TL_memcmp( tx_cc, ap_cc, 2 ) != 0 )
                      continue; // not satisfied, next CVM entry

                    // TxAmt < Y ?
                //  TL_bcd2hex( 5, g_term_tx_amt, temp );
                    if( TL_memcmp( amt_b, yamt, 4 ) < 0 )
                      break; // process rule
                    else
                      continue; // next CVM entry

               case CVC_AMT_OVER_Y:

                    if( TL_memcmp( tx_cc, ap_cc, 2 ) != 0 )
                      continue; // not satisfied, next CVM entry

                    // TxAmt > Y ?
                //  TL_bcd2hex( 5, g_term_tx_amt, temp );
                    if( TL_memcmp( amt_b, yamt, 4 ) > 0 )
                      break; // process rule
                    else
                      continue; // next CVM entry

               default: // others, terminal does not support
                    continue; // next CVM List entry
               }


         // ---------------------
         //  CVM Rule Processing
         // ---------------------

         switch( mcode & 0xbf )  // mask off b7
               {
               case CVR_FAIL_CVM_PROCESSING:

          	    g_term_CVMR[0] = mcode;
          	    g_term_CVMR[1] = ccode;
          	    fCVM = TRUE;
                    goto ACTION_FAILED_2;//2008-dec-26 charles

               case CVR_PLAINTEXT_PIN:

                    if( (tcap[1] & CAP1_PPIN_ICC_VERIFY) == 0 )
                      {
                      fCVMnosup = TRUE; //20090317_Richard: EMV 4.1e, 2CJ.083.04
              	      goto ACTION_FAILED;
                      }

		    g_term_CVMR[0] = mcode;
          	    g_term_CVMR[1] = ccode;
          	    fCVM = TRUE;
          	    
                    result = apk_CVM_PlaintextPIN( tout, msg );
                    break;

               case CVR_PLAINTEXT_PIN_AND_SIGN:

                    if( (tcap[1] & CAP1_PPIN_ICC_VERIFY) == 0 )
                      {
                      fCVMnosup = TRUE; //20090317_Richard: EMV 4.1e, 2CJ.083.04
              	      goto ACTION_FAILED;
                      }

                    if( (tcap[1] & CAP1_PAPER_SIGN) == 0 ) // PATCH: 2005/03/11
                      {
                      fCVMnosup = TRUE; //20090317_Richard: EMV 4.1e, 2CJ.083.04
              	      goto ACTION_FAILED;
                      }

          	    g_term_CVMR[0] = mcode;
          	    g_term_CVMR[1] = ccode;
          	    fCVM = TRUE;
          	    
                    result = apk_CVM_PlaintextPIN( tout, msg );
                    if( result == apkOK )
                      result = apkUnknown; // because of signature
                    break;

               case CVR_ENCIPHERED_PIN:

                    if( (tcap[1] & CAP1_EPIN_OFFLINE_VERIFY) == 0 )
                      {
                      fCVMnosup = TRUE; //20090317_Richard: EMV 4.1e, 2CJ.083.04
              	      goto ACTION_FAILED;
                      }

          	    g_term_CVMR[0] = mcode;
          	    g_term_CVMR[1] = ccode;
          	    fCVM = TRUE;

                    result = apk_CVM_EncipheredPIN( tout, msg );
                    break;

               case CVR_ENCIPHERED_PIN_AND_SIGN:

                    if( (tcap[1] & CAP1_EPIN_OFFLINE_VERIFY) == 0 )
                      {
                      fCVMnosup = TRUE; //20090317_Richard: EMV 4.1e, 2CJ.083.04
              	      goto ACTION_FAILED;
                      }

                    if( (tcap[1] & CAP1_PAPER_SIGN) == 0 ) // PATCH: 2005/03/11
                      {
                      fCVMnosup = TRUE; //20090317_Richard: EMV 4.1e, 2CJ.083.04
              	      goto ACTION_FAILED;
                      }

          	    g_term_CVMR[0] = mcode;
          	    g_term_CVMR[1] = ccode;
          	    fCVM = TRUE;

                    result = apk_CVM_EncipheredPIN( tout, msg );
                    if( result == apkOK )
                      result = apkUnknown; // because of signature
                    break;

               case CVR_SIGN_PAPER:

                    if( (tcap[1] & CAP1_PAPER_SIGN) == 0 ) // PATCH: 2005/03/11
                      {
                      fCVMnosup = TRUE; //20090317_Richard: EMV 4.1e, 2CJ.083.04
              	      goto ACTION_FAILED;
                      }

          	    g_term_CVMR[0] = mcode;
          	    g_term_CVMR[1] = ccode;
          	    fCVM = TRUE;

                    result = apkUnknown;
                    break;

               case CVR_ENCIPHERED_PIN_ONLINE:

		    if( (tcap[1] & CAP1_EPIN_ONLINE_VERIFY) == 0 )
                      {
                      fCVMnosup = TRUE; //20090317_Richard: EMV 4.1e, 2CJ.083.04
              	      goto ACTION_FAILED;
                      }
                      
          	    g_term_CVMR[0] = mcode;
          	    g_term_CVMR[1] = ccode;
          	    fCVM = TRUE;

                    // PATCH: 2005/05/11
                    result = apk_CVM_OnlineEncipheredPIN( tout, msg, epb, ksn, mod, idx );
                    if( result == apkOK )
                      {
                      g_term_TVR[2] |= TVR2_ONLINE_PIN_ENTERED; // PATCH: 2005/05/19, 2CM.020.00
                      api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR+2, 1, &g_term_TVR[2] );

                      result = apkUnknown;
                      }

                    break;

               case CVR_NO_CVM_REQUIRED:
               
               	    if( (tcap[1] & CAP1_NO_CVM_REQUIRED) == 0 )
                      {
                      fCVMnosup = TRUE; //20090317_Richard: EMV 4.1e, 2CJ.083.04
              	      goto ACTION_FAILED;
                      }

          	    g_term_CVMR[0] = mcode;
          	    g_term_CVMR[1] = ccode;
          	    fCVM = TRUE;

                    result = apkOK;
                    break;

#ifdef  L2_PBOC20

               case CVR_VERIFY_CH_LICENSE:  // PATCH: PBOC2.0 ONLY, 2006-02-14

                    if( (tcap[1] & CAP1_CH_LICENSE_VERIFY) == 0 )
                      continue;

                    result = PBOC_VerifyCardholderLicense();
                    if( result == TRUE )
                      result = apkOK;
                    else
                      result = apkIncorrect;
                    break;

#endif

               default: // others, not supported

                    result = apkNotSupported;
                    break;

               } // switch( mcode )

VERIFY_RESULT:

         switch( result )
               {
               case apkOK:

                    g_term_CVMR[2] = CVMR_SUCCESSFUL;
                    goto ACTION_OK;

               case apkNotReady: // PIN entry bypassed

                    g_term_TVR[2] |= TVR2_PIN_REQ_PIN_NOT_ENTERED;
                    api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR+2, 1, &g_term_TVR[2] );

                //  goto ACTION_FAILED; // EMV96
                    continue;           // EMV2000

               case apkFailed:

                    g_term_TVR[2] |= TVR2_PIN_TRY_LIMIT_EXCEEDED;
                    api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR+2, 1, &g_term_TVR[2] );

                    goto ACTION_FAILED;

               case apkUnknown: // signature or...

                    g_term_CVMR[2] = CVMR_UNKNOWN;

                    goto ACTION_OK;

               case apkNotSupported:

                    g_term_TVR[2] |= TVR2_UNRECONGNIZED_CVM;
                    api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR+2, 1, &g_term_TVR[2] );

                    goto ACTION_FAILED;

               case apkOutOfService:

                    return( emvOutOfService );

               case apkDeviceError: // PIN pad is malfunctioning

                    g_term_TVR[2] |= TVR2_PIN_REQ_PINPAD_NOT_WORK;
                    api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR+2, 1, &g_term_TVR[2] );

                    goto ACTION_FAILED;

               case apkIncorrect: // bad PIN Encipherment PK certificate
                                  // PATCH: 2003-06-18
                    goto ACTION_FAILED;
               }

ACTION_FAILED:
         if( (ccode == CVC_TERM_SUPPORT_CVM)&& (fCVMnosup == TRUE) )//20090317_Richard: EMV 4.1e, 2CJ.083.04
            {
            continue;
            }
         if( (mcode & CVR_APPLY_NEXT_CVR) == 0 ) // apply succeeding CVM if failed?
           break;

         } // for( next cvm entry )


      // no more CVM entry satisfied
//    if( (g_term_TVR[2] & TVR2_UNRECONGNIZED_CVM) == 0 ) // PATCH: 2006-10-04
//      {
//      g_term_CVMR[0] = CVR_NOT_AVAILABLE;
//      g_term_CVMR[1] = 0x00;
//      g_term_CVMR[2] = 0x01;	//2008-dec-25	Charles
//      api_emv_PutDataElement( DE_TERM, ADDR_TERM_CVMR, 3, g_term_CVMR );
//
//      goto ACTION_FAILED_3;
//      }

  if ( fCVM == FALSE )
    {
      g_term_CVMR[0] = CVR_NOT_AVAILABLE;
      g_term_CVMR[1] = 0x00;
    }

ACTION_FAILED_2:

      g_term_CVMR[2] = CVMR_FAILED;
      api_emv_PutDataElement( DE_TERM, ADDR_TERM_CVMR, 3, g_term_CVMR );

ACTION_FAILED_3:

      g_term_TVR[2] |= TVR2_CVM_NOT_SUCCESS;
      api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR+2, 1, &g_term_TVR[2] );

      g_term_TSI[0] |= TSI0_CVM_PERFORMED;
      api_emv_PutDataElement( DE_TERM, ADDR_TERM_TSI, 1, &g_term_TSI[0] );

      return( emvFailed );

ACTION_OK:

      api_emv_PutDataElement( DE_TERM, ADDR_TERM_CVMR, 3, g_term_CVMR );

      g_term_TSI[0] |= TSI0_CVM_PERFORMED;
      api_emv_PutDataElement( DE_TERM, ADDR_TERM_TSI, 1, &g_term_TSI[0] );

      return( emvOK );

}

