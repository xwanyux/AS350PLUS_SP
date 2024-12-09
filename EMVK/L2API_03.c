//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : L2API_03.C                                                 **
//**  MODULE   : api_emv_FinalSelection()                                   **
//**             api_emv_RemoveApplication()                                **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2002/12/10                                                 **
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
#include "EMVAPI.h"
#include "EMVAPK.h"

// ---------------------------------------------------------------------------
// FUNCTION: Select the application to be run.
// INPUT   : appname - LINK[1] LEN[1] NAME[]
// OUTPUT  : fci     - 2L-V, the file control information. (RFU)
// REF     : g_ibuf
// RETURN  : emvOK
//           emvFailed
//           emvOutOfService - mandatory data objects missing.
// NOTE    : if the return value equals "emvFailed", the final selected
//           application shall be removed from the system candidate list.
//           ie, the length field of the ordered candidate list is 0 and
//           name field is cleared to space characters.
// ---------------------------------------------------------------------------
UCHAR api_emv_FinalSelection( UCHAR *appname )
{
UINT  addr;
UINT  iLen;
UCHAR *ptrobj;
UCHAR *ptrde;
UCHAR result;
UCHAR cnt;
UCHAR *ptrnext;

//UCHAR temp[128];


      // clear all ICC-File-Related data elements & records to 0
      iLen = ADDR_ICC_FILE_DE_END;
      iLen -= ADDR_ICC_FILE_DE_START;
      apk_ClearRamDataICC( ADDR_ICC_FILE_DE_START, iLen, 0 );

      g_ibuf[0] = 0;
      g_ibuf[1] = 0;
      apk_WriteRamDataICC( ADDR_ICC_LOG_ENTRY, 2, g_ibuf );

      // read AID or DFNAME by using appname.link
      addr = ADDR_CANDIDATE_01 + CANDIDATE_LEN*appname[0];
      apk_ReadRamDataTERM( addr, CANDIDATE_LEN, g_ibuf );

      ptrobj = apk_FindTag( 0x4f, 0x00, g_ibuf );  // Tag=4F (AID)? or
      if( ptrobj != 0 )
        {
        iLen = *ptrobj;
        apk_WriteRamDataICC( ADDR_ICC_AID, 2, (UCHAR *)&iLen );
        apk_WriteRamDataICC( ADDR_ICC_AID+2, iLen, ptrobj+1 );
        }
      else
        {
        ptrobj = apk_FindTag( 0x84, 0x00, g_ibuf );  // Tag=84 (DFNAME)

        iLen = *ptrobj;
        apk_WriteRamDataICC( ADDR_ICC_DFNAME, 2, (UCHAR *)&iLen );
        apk_WriteRamDataICC( ADDR_ICC_DFNAME+2, iLen, ptrobj+1 );
        }

      // find & save "Application Label"
      ptrde = apk_FindTag( 0x50, 0x00, g_ibuf );
      if( ptrde != 0 )
        {
        iLen = *ptrde++;
        apk_WriteRamDataICC( ADDR_ICC_AP_LABEL, 2, (UCHAR *)&iLen );
        apk_WriteRamDataICC( ADDR_ICC_AP_LABEL+2, iLen, ptrde );
        }

      // find & save "Application Preferred Name"
      ptrde = apk_FindTag( 0x9f, 0x12, g_ibuf );
      if( ptrde != 0 )
        {
        iLen = *ptrde++;
        apk_WriteRamDataICC( ADDR_ICC_AP_PREFER_NAME, 2, (UCHAR *)&iLen );
        apk_WriteRamDataICC( ADDR_ICC_AP_PREFER_NAME+2, iLen, ptrde );
        }

      // find & save "Application Priority Indicator"
      ptrde = apk_FindTag( 0x87, 0x00, g_ibuf );
      if( ptrde != 0 )
        {
        iLen = *ptrde++;
        apk_WriteRamDataICC( ADDR_ICC_AP_PI, 2, (UCHAR *)&iLen );
        apk_WriteRamDataICC( ADDR_ICC_AP_PI+2, iLen, ptrde );
        }

//TL_DumpHexData(0,0,32, ptrobj);

      // select the application by using "AID" or "DFNAME"
      result = apk_SelectADF( g_ibuf, ptrobj, 0 );

//TL_DumpHexData(0,0,g_ibuf[0]+g_ibuf[1]*256+2, g_ibuf);

//TL_DumpHexData(0,2,32, ptrobj);

      // NOTE: the contents of ptrobj will be changed to FCI after calling
      //       apk_SelectADF() due to "api_ifm_exchangeAPDU()",
      //       this is an unknown problem ! (2005/04/29)

      if( result == apkFailed )
        return( emvOutOfService );

      if( result == apkOK )
        {
        // save the FCI of the selected file to SELECTED_FCI[]
        apk_ClearRamDataICC( ADDR_SELECTED_FCI, SELECTED_FCI_LEN, 0x00 );
        apk_WriteRamDataICC( ADDR_SELECTED_FCI, g_ibuf[1]*256+g_ibuf[0], &g_ibuf[2] );

        // save the selected AID or DFNAME to SELECTED_AID[] and TERM_AID[]
//      iLen = *(ptrobj+2);
//      apk_WriteRamDataICC( ADDR_SELECTED_AID, iLen+1, ptrobj+2 );
//
//      apk_WriteRamDataICC( ADDR_ICC_AID, 2, (UCHAR *)&iLen );
//      apk_WriteRamDataICC( ADDR_ICC_AID+2, iLen, ptrobj+3 );
//
//      apk_ClearRamDataTERM( ADDR_TERM_AID, 16, 0x00 );
//      apk_WriteRamDataTERM( ADDR_TERM_AID, *(ptrobj+2), ptrobj+3 );

        // PATCH: 2003-06-10
        // find & save DF Name
        ptrde = apk_FindTag( 0x84, 0x00, &g_ibuf[2] );
        if( ptrde != 0 )
          {
          iLen = *ptrde++;
          apk_WriteRamDataICC( ADDR_ICC_DFNAME, 2, (UCHAR *)&iLen );
          apk_WriteRamDataICC( ADDR_ICC_DFNAME+2, iLen, ptrde );

          // save the selected AID or DFNAME to SELECTED_AID[] and TERM_AID[]
          apk_WriteRamDataICC( ADDR_SELECTED_AID, 1, (UCHAR *)&iLen );
          apk_WriteRamDataICC( ADDR_SELECTED_AID+1, iLen, ptrde );

          apk_WriteRamDataICC( ADDR_ICC_AID, 2, (UCHAR *)&iLen );
          apk_WriteRamDataICC( ADDR_ICC_AID+2, iLen, ptrde );

          apk_ClearRamDataTERM( ADDR_TERM_AID, 16, 0x00 );
          apk_WriteRamDataTERM( ADDR_TERM_AID, iLen, ptrde );
          }
        else
          return( emvOutOfService );

        // find & save "PDOL"
        ptrde = apk_FindTag( 0x9f, 0x38, &g_ibuf[2] );
        if( ptrde != 0 )
          {
          iLen = *ptrde++;
          apk_WriteRamDataICC( ADDR_ICC_PDOL, 2, (UCHAR *)&iLen );
          apk_WriteRamDataICC( ADDR_ICC_PDOL+2, iLen, ptrde );
          }

        // find & save "Language Preference"
        ptrde = apk_FindTag( 0x5f, 0x2d, &g_ibuf[2] );
        if( ptrde != 0 )
          {
          iLen = *ptrde++;
          apk_WriteRamDataICC( ADDR_ICC_LANG_PREFER, 2, (UCHAR *)&iLen );
          apk_WriteRamDataICC( ADDR_ICC_LANG_PREFER+2, iLen, ptrde );
          }

        // find & save "Issuer Code Table Index"
        ptrde = apk_FindTag( 0x9f, 0x11, &g_ibuf[2] );
        if( ptrde != 0 )
          {
          iLen = *ptrde++;
          apk_WriteRamDataICC( ADDR_ICC_ISU_CTI, 2, (UCHAR *)&iLen );
          apk_WriteRamDataICC( ADDR_ICC_ISU_CTI+2, iLen, ptrde );
          }

	// PATCH: 2011-03-25, find "Application Preferred Name"
	ptrde = apk_FindTag( 0x9f, 0x12, &g_ibuf[2] );
        if( ptrde != 0 )
          {
          iLen = *ptrde++;
          apk_WriteRamDataICC( ADDR_ICC_AP_PREFER_NAME, 2, (UCHAR *)&iLen );
          apk_WriteRamDataICC( ADDR_ICC_AP_PREFER_NAME+2, iLen, ptrde );
          }	
          
	if( g_kernel != KERNEL_PBOC )
	  goto CLEAR_TDE;
	  
        // find "Issuer Discretionary Data" for PBOC2,0
        ptrde = apk_FindTag( 0xbf, 0x0c, &g_ibuf[2] );
        if( ptrde != 0 )
          {
          iLen = apk_GetBERLEN( ptrde, &cnt );
          ptrde += cnt;

FIND_LOG_ENTRY:

          ptrnext = apk_GetBERTLV( (UCHAR *)&iLen, ptrde, g_ibuf );

          if( ptrnext == (UCHAR *)-1 ) // invalid?
            return( emvFailed );

          if( ptrnext != 0 ) // EOF?
            {
            if( (g_ibuf[0] == 0x9f) && (g_ibuf[1] == 0x4d) ) // log entry?
              {
              if( (g_ibuf[2] == 0x02) && (g_ibuf[3] == 0x00) )
                {
                apk_WriteRamDataICC( ADDR_ICC_LOG_ENTRY, 4, &g_ibuf[2] ); // L(2)-V(2)
                }
              }
            else
              {
              ptrde = ptrnext;
              goto FIND_LOG_ENTRY;
              }
            }
          }


CLEAR_TDE:

        // --------------------------------------------------------------------------
        // Clear terminal-related data objects for the current transaction
        //       Amount, Authorized (binary & numeric) : excluding adjustments
        //       Amount, Other (binary & numeric)      : excluding adjustments
        //       Transaction Amount                    : including tips & adjustments
        //       Transaction Date
        //       Transaction Time
        //       Unpredictable Number
        // --------------------------------------------------------------------------
        apk_ClearRamDataTERM( ADDR_TERM_AMT_AUTH_B, 4, 0x00 );
        apk_ClearRamDataTERM( ADDR_TERM_AMT_AUTH_N, 6, 0x00 );
        apk_ClearRamDataTERM( ADDR_TERM_AMT_OTHER_B, 4, 0x00 );
        apk_ClearRamDataTERM( ADDR_TERM_AMT_OTHER_N, 6, 0x00 );
        apk_ClearRamDataTERM( ADDR_TERM_TX_AMT, 6, 0x00 );

        apk_ClearRamDataTERM( ADDR_TERM_TX_DATE, 3, 0xff );
        apk_ClearRamDataTERM( ADDR_TERM_TX_TIME, 3, 0xff );
        apk_ClearRamDataTERM( ADDR_TERM_UPD_NBR, 4, 0x00 );
        apk_ClearRamDataTERM( ADDR_TERM_UPD_NBR_LEN, 1, 0x00 );
        apk_ClearRamDataTERM( ADDR_TERM_ARC, 2, 0x00 );
        apk_ClearRamDataTERM( ADDR_TERM_ISR, 1, 0x00 );

        apk_ClearRamDataTERM( ADDR_ISU_AUTH_CODE, 6, 0x00 );
        apk_ClearRamDataTERM( ADDR_ISU_ARC, 2, 0x00 );
        apk_ClearRamDataTERM( ADDR_ISU_AUTH_DATA, 16, 0x00 );
        apk_ClearRamDataTERM( ADDR_ISU_SCRIPT_TEMP, 2, 0x00 );

        apk_ClearRamDataICC( ADDR_ICC_ISU_PKM, 2, 0x00 ); // SDA & DDA
        apk_ClearRamDataICC( ADDR_ICC_PKM, 2, 0x00 );     // DDA

        // clear EPB & KSN data for online Enciphered PIN Data
        apk_ClearRamDataTERM( ADDR_TERM_KSN, 12, 0x00 );
        apk_ClearRamDataTERM( ADDR_TERM_EPIN_DATA, 10, 0x00 );

        return( emvOK );
        }
      else
        {
        // select failed, remove it from candidate list
        apk_RemoveCandidateList( appname[0] );
        return( emvFailed );
        }
}

// ---------------------------------------------------------------------------
// FUNCTION: eliminate the finally selected application from candidate list.
// INPUT   : index - the finally selected item number.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void api_emv_RemoveApplication( UCHAR index )
{
UINT  addr;
UCHAR appname[CANDIDATE_NAME_LEN];

      // retrieve the finally selected file data
      addr = ADDR_CANDIDATE_NAME_01 + CANDIDATE_NAME_LEN*index;
      apk_ReadRamDataTERM( addr, CANDIDATE_NAME_LEN, appname );

      // remove it from name list
      apk_RemoveCandidateList( appname[0] );
}

// ---------------------------------------------------------------------------
// FUNCTION: if only one application is supported, check its application
//           priority indicator (API) for auto selection.
//           (1) listcnt = 1 and API.bit8 = 0
//           (2) listcnt > 1 and all APIs.bit8 = 0
// INPUT   : occurrence - 0x00 = the 1'st time to select.
//                        0x01 = the following time to select.
//         : listcnt    - number of items in the list. (1..n)
// OUTPUT  : item    - the item number to be auto selected if return TRUE.
// REF     : g_api[]
// RETURN  : TURE  - it may be selected without confirmation of cardholder.
//           FLASE - it cannot be selected without confirmation of cardholder.
// ---------------------------------------------------------------------------
UCHAR api_emv_AutoSelectApplication( UCHAR occurrence, UCHAR listcnt, UCHAR *item )
{
UCHAR buf[CANDIDATE_LEN];
UCHAR *ptrobj;
UCHAR cnt;
UCHAR i;

      *item = 0; // the 1'st one has the highest priority

      if( (occurrence == 0) && (listcnt == 1) )
        {
        // case 1: RESTORED on 2003-10-03 (will check EMV test case)
        apk_ReadRamDataTERM( ADDR_CANDIDATE_01, CANDIDATE_LEN, buf );
        ptrobj = apk_FindTag( 0x87, 0x00, buf ); // find API
        if( ptrobj == 0 ) // not found
          return( TRUE );

//      apk_GetBERLEN( ptrobj, &cnt );
//      ptrobj += cnt; // ptr to API
//
//      if( *ptrobj & 0x80 ) // check bit8=1 (must be confirmed) ?
//        return( FALSE );
//      else
//        return( TRUE );

        if( g_api[0] & 0x80 ) // check bit8=1 (must be confirmed) ?
          return( FALSE );
        else
          return( TRUE );
        }
      else
        {

        return( FALSE ); // temp solution

        // case 2
//      for( i=0; i<listcnt; i++ )
//         {
//         apk_ReadRamDataTERM( ADDR_CANDIDATE_01+i*CANDIDATE_LEN, CANDIDATE_LEN, buf );
//         ptrobj = apk_FindTag( 0x87, 0x00, buf ); // find API
//         if( ptrobj == 0 ) // not found
//           return( TRUE );
//
//         apk_GetBERLEN( ptrobj, &cnt );
//         ptrobj += cnt; // ptr to API
//
//         if( *ptrobj & 0x80 ) // check bit8=1 (must be confirmed) ?
//           return( FALSE );
//         }
//
//      return( TRUE );

        for( i=0; i<listcnt; i++ )
           {
           if( g_api[i] & 0x80 )
             return( FALSE );
           }
        return( TRUE );
        }
}

// ---------------------------------------------------------------------------
// FUNC  : Setup all realted EMV parameters for the selected AID.
//         except: Application Version Number (AVN)
//
// INPUT : none.
// OUTPUT: none.
// REF   : g_selected_aid_index
// RETURN: none.
// ---------------------------------------------------------------------------
void  api_emv_SetupCardParameters( void )
{
UCHAR i;
UCHAR term_aid[TERM_AID_LEN];
UCHAR selected_aid[SELECTED_AID_LEN];

      // get the selected AID
      // LEN[1] AID[16]
      apk_ReadRamDataICC( ADDR_SELECTED_AID, SELECTED_AID_LEN, selected_aid );

      for( i=0; i<MAX_AID_CNT; i++ )
         {
         // get term AID
         // LEN[1] ASI[1] AID[16]
         apk_ReadRamDataTERM( ADDR_TERM_AID_01+i*TERM_AID_LEN, TERM_AID_LEN, term_aid );

         // matching AID
      // if( TL_memcmp( &selected_aid[1], &term_aid[2], selected_aid[0] ) == 0 )

         // PATCH: 2003-06-27 for both exact and partial AID matching
         if( TL_memcmp( &selected_aid[1], &term_aid[2], term_aid[0]-1 ) == 0 )
           break; // equal
         }

      g_selected_aid_index = i; // backup the target aid number

      // --- Load The Target EMV Parameters ---  (including TERM_AVN(2), TERM_FL(4), TFL(6), TFL_FLAG(1))

      // load the "Transaction Currency Code"
//    apk_ReadRamDataTERM( ADDR_TERM_TX_CC_01+g_selected_aid_index*TX_CC_LEN, TX_CC_LEN, term_aid );
//    apk_WriteRamDataTERM( ADDR_TERM_TX_CC, TX_CC_LEN, term_aid );

      // load the "Transaction Currency Exponent"
//    apk_ReadRamDataTERM( ADDR_TERM_TX_CE_01+g_selected_aid_index*TX_CE_LEN, TX_CE_LEN, term_aid );
//    apk_WriteRamDataTERM( ADDR_TERM_TX_CE, TX_CE_LEN, term_aid );

      // load the "Acquirer ID"
//    apk_ReadRamDataTERM( ADDR_TERM_ACQ_ID_01+g_selected_aid_index*ACQ_ID_LEN, ACQ_ID_LEN, term_aid );
//    apk_WriteRamDataTERM( ADDR_TERM_ACQ_ID, ACQ_ID_LEN, term_aid );

      // load the "Application Version Number"
      apk_ReadRamDataTERM( ADDR_AVN_01+g_selected_aid_index*AVN_LEN, AVN_LEN, term_aid );
      apk_WriteRamDataTERM( ADDR_TERM_AVN, AVN_LEN, term_aid );

      // load the "Merchant Category Code"
//    apk_ReadRamDataTERM( ADDR_TERM_MCC_01+g_selected_aid_index*MCC_LEN, MCC_LEN, term_aid );
//    apk_WriteRamDataTERM( ADDR_TERM_MCC, MCC_LEN, term_aid );

      // load the "Terminal Floor Limit" & "Flag"
      apk_ReadRamDataTERM( ADDR_TFL_01+TFL_LEN*g_selected_aid_index, TFL_LEN, term_aid );
      apk_WriteRamDataTERM( ADDR_TFL, TFL_LEN, term_aid );  // bcd format

      TL_bcd2hex( TFL_LEN-1, &term_aid[1], selected_aid );   // PATCH: PBOC20, 2006-02-07
      apk_WriteRamDataTERM( ADDR_TERM_FL, 4, selected_aid ); // binary foramt

      apk_ReadRamDataTERM( ADDR_TFL_FLAG_01+g_selected_aid_index, 1, term_aid );
      apk_WriteRamDataTERM( ADDR_TFL_FLAG, 1, term_aid );

      // load the "Terminal Capabilities"
//    apk_ReadRamDataTERM( ADDR_TERM_CAP_01+TERM_CAP_LEN*g_selected_aid_index, TERM_CAP_LEN, term_aid );
//    apk_WriteRamDataTERM( ADDR_TERM_CAP, TERM_CAP_LEN, term_aid );

      // load the "Additional Terminal Capabilities"
//    apk_ReadRamDataTERM( ADDR_TERM_ADD_CAP_01+TERM_ADD_CAP_LEN*g_selected_aid_index, TERM_ADD_CAP_LEN, term_aid );
//    apk_WriteRamDataTERM( ADDR_TERM_ADD_CAP, TERM_ADD_CAP_LEN, term_aid );

      // load the target "TAC_Default"
      apk_ReadRamDataTERM( ADDR_TERM_TAC_DEFAULT_01+TAC_LEN*g_selected_aid_index, TAC_LEN, term_aid );
      apk_WriteRamDataTERM( ADDR_TERM_TAC_DEFAULT, TAC_LEN, term_aid );

      // load the target "TAC_Denial"
      apk_ReadRamDataTERM( ADDR_TERM_TAC_DENIAL_01+TAC_LEN*g_selected_aid_index, TAC_LEN, term_aid );
      apk_WriteRamDataTERM( ADDR_TERM_TAC_DENIAL, TAC_LEN, term_aid );

      // load the target "TAC_Online"
      apk_ReadRamDataTERM( ADDR_TERM_TAC_ONLINE_01+TAC_LEN*g_selected_aid_index, TAC_LEN, term_aid );
      apk_WriteRamDataTERM( ADDR_TERM_TAC_ONLINE, TAC_LEN, term_aid );

      // load the "Threshold Value for Biased Random Selection"
      apk_ReadRamDataTERM( ADDR_TERM_BRS_THRESHOLD_01+BRS_THRESHOLD_LEN*g_selected_aid_index, BRS_THRESHOLD_LEN, term_aid );
      apk_WriteRamDataTERM( ADDR_TERM_BRS_THRESHOLD, BRS_THRESHOLD_LEN, term_aid );

      // load the "Target Percentage for Random Selection"
      apk_ReadRamDataTERM( ADDR_TERM_RS_TP_01+RS_TP_LEN*g_selected_aid_index, RS_TP_LEN, term_aid );
      apk_WriteRamDataTERM( ADDR_TERM_RS_TP, RS_TP_LEN, term_aid );

      // load the "Max Target Percentage for Biased Random Selection"
      apk_ReadRamDataTERM( ADDR_TERM_BRS_MTP_01+BRS_MTP_LEN*g_selected_aid_index, BRS_MTP_LEN, term_aid );
      apk_WriteRamDataTERM( ADDR_TERM_BRS_MTP, BRS_MTP_LEN, term_aid );
}
