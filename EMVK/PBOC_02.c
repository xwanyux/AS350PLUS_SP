//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : PBOC 2.0 LEVEL 2 Debit/Credit                              **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : PBOC_02.C                                                  **
//**  MODULE   : Functions used for PBOC2.0 specification.                  **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2006/01/24                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2006-2009 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include <string.h>

#include "POSAPI.h"
#include "UI.h"
#include "GDATAEX.h"
#include "EMVAPI.h"
#include "EMVAPK.h"
#include "PBOC.H"


// ---------------------------------------------------------------------------
void  PutIccLogTitle( UCHAR tag1, UCHAR tag2, UCHAR *row, UCHAR *col )
{
UINT  iTag;
UCHAR buf[32];

      iTag = tag1*256 + tag2;

      switch( iTag )
            {
            case 0x009A:
                 memmove( buf, (UCHAR *)PBOCmsg_date, 5 );
                 UI_PutMsg( 1, COL_LEFTMOST, FONT0, 5, buf );
                 *row = 1;
                 *col = 5;
                 break;

            case 0x9F21:
                 memmove( buf, (UCHAR *)PBOCmsg_time, 5 );
                 UI_PutMsg( 2, COL_LEFTMOST, FONT0, 5, buf );
                 *row = 2;
                 *col = 5;
                 break;

            case 0x9F02:
                 memmove( buf, (UCHAR *)PBOCmsg_amt_auth, 8 );
                 UI_PutMsg( 3, COL_LEFTMOST, FONT0, 10, buf );
                 *row = 3;
                 *col = 8;
                 break;

            case 0x9F03:
                 memmove( buf, (UCHAR *)PBOCmsg_amt_other, 8 );
                 UI_PutMsg( 4, COL_LEFTMOST, FONT0, 10, buf );
                 *row = 4;
                 *col = 8;
                 break;

            case 0x9F1A:
                 memmove( buf, (UCHAR *)PBOCmsg_country_code, 14 );
                 UI_PutMsg( 5, COL_LEFTMOST, FONT0, 14, buf );
                 *row = 5;
                 *col = 14;
                 break;

            case 0x5F2A:
                 memmove( buf, (UCHAR *)PBOCmsg_currency_code, 14 );
                 UI_PutMsg( 6, COL_LEFTMOST, FONT0, 14, buf );
                 *row = 6;
                 *col = 14;
                 break;

            case 0x9F4E:
                 memmove( buf, (UCHAR *)PBOCmsg_merchant_name, 14 );
                 UI_PutMsg( 1, COL_LEFTMOST, FONT0, 14, buf );
                 *row = 2;
                 *col = 0;
                 break;

            case 0x009C:
                 memmove( buf, (UCHAR *)PBOCmsg_trans_type, 11 );
                 UI_PutMsg( 3, COL_LEFTMOST, FONT0, 11, buf );
                 *row = 3;
                 *col = 11;
                 break;

            case 0x9F36:
                 memmove( buf, (UCHAR *)PBOCmsg_atc, 4 );
                 UI_PutMsg( 4, COL_LEFTMOST, FONT0, 4, buf );
                 *row = 4;
                 *col = 4;
                 break;

            case 0xFF01: // (more)
                 memmove( buf, (UCHAR *)PBOCmsg_more, 6 );
                 UI_PutMsg( 7, COL_RIGHTMOST, FONT0, 6, buf );
                 *row = 0;
                 *col = 0;
                 UI_WaitKey();
                 UI_ClearRow( 1, 7, FONT0 );
                 break;

            case 0xFF02: // (next)
                 memmove( buf, (UCHAR *)PBOCmsg_next, 6 );
                 UI_PutMsg( 7, COL_RIGHTMOST, FONT0, 6, buf );
                 *row = 0;
                 *col = 0;
                 UI_WaitKey();
                 break;

            }
}

// ---------------------------------------------------------------------------
// FUNCTION: build and print out the ICC Log DOL. (screen dump)
// INPUT   : ptrobj - pointer to the requested DOL. (from Tag 9F4F)
//                    berLEN T1-L1 T2-L2 T3-L3...
//           log    - the contents of the Tn-Ln.
// OUTPUT  : none.
// REF     : g_ibuf
// RETURN  : TRUE  - OK
//           FALSE - something wrong in TLV format (reference only)
//
//           012345678901234567890
//         0 RECORD:NN
//         1 DATE:YYYYMMDD
//         2 TIME:HHMMSS
//         3 AMT AUTH :NNNNNNNNNNN
//         4 AMT OTHER:NNNNNNNNNNN
//         5 COUNTRY CODE :NNNN
//         6 CURRENCY CODE:NNNN
//         7                (more)
//        
//         0 RECORD:NN
//         1 MERCHANT NAME:
//         2 XXXXXXXXXXXXXXXXXXXX
//         3 TRANS TYPE:NN
//         4 ATC:NN
//           ...
//         7                (next)
// ---------------------------------------------------------------------------
void  Build_LogDOL( UCHAR *ptrobj, UCHAR *log )
{
UCHAR dol_len;
UCHAR de[4];
UCHAR i;
UCHAR tag1, tag2;
UCHAR len;
UCHAR index;
//UCHAR buf[2];
UCHAR cnt;
UCHAR row;
UCHAR col;
UCHAR buf1[32];
UCHAR buf2[32];

      row = 1;
//      UI_PrintClear();

//    *length = 0; // reset output length

      // format: LEN[n], T1-L1, T2-L2, T3-L3....
      //         where: LEN = total length, T1 = 1 or 2 bytes, Ln = 1 byte.

      dol_len = apk_GetBERLEN( ptrobj, &cnt ); // get total length
      if( dol_len == 0 ) // empty DOL?
        return;

      ptrobj += cnt;

      do{
        tag1 = *ptrobj++;    // MSB of tag
        tag2 = *ptrobj++;    // LSB of tag

        // check constructed data element
        if( apk_CheckConsTag( tag1 ) == TRUE )
          {
          len = (UCHAR)apk_GetBERLEN( --ptrobj, &cnt );
          ptrobj += cnt;
//        if( len == 0 )
//          flag = FALSE;

          dol_len -= (1+cnt);
          goto DOL_UNKNOWN;
          }

        if( apk_CheckWordTag( tag1 ) == TRUE ) // double-byte tag?
          {
          len = (UCHAR)apk_GetBERLEN( ptrobj, &cnt );
          ptrobj += cnt;
//        if( len == 0 )
//          flag = FALSE;

          dol_len -= (2+cnt);
          }
        else // single-byte tag
          {
          len = (UCHAR)apk_GetBERLEN( --ptrobj, &cnt );
          ptrobj += cnt;
//        if( len == 0 )
//          flag = FALSE;

          tag2 = tag1;
          tag1 = 0;

          dol_len -= (1+cnt);

          if( apk_CheckTermTag( tag2 ) == FALSE )
            {
            if( apk_CheckIccTag( tag2 ) == FALSE )
              {
              if( apk_CheckIssuerTag( tag2 ) == FALSE )
                // a single-byte tag, but its source is neither TERMINAL, ICC or ISSUER
                goto DOL_UNKNOWN;
              }
            }
          }

//  TL_DispHexByte(1,0,tag1);
//  TL_DispHexByte(1,3,tag2);
//  TL_DispHexByte(1,6,len);

        if( (tag1 == 0x9F) && (tag2 == 0x4E) ) // merchant name
          {
          de[0] = 0x9F;
          de[1] = 0x4E;
          de[2] = DEF_ANS;
          de[3] = 20;
          goto FIND_TDE;
          }

        // ------ tag(s) source is "TERMINAL" ? ------
        index = Scan_TDE( tag1, tag2, &de[0] );

        if( index != 255 )
          {
FIND_TDE:
//  TL_DispHexByte(0,0,0x11);
//  UI_WaitKey();

          // retrieve the specified data element, TAG1 TAG2 FORMAT LEN
//  TL_DumpHexData(0,2,4, de);

          // PATCH: PBOC2.0, 2006-02-16, amount (auth and other)
          if( ((de[0] == 0x9f) && (de[1] == 0x02)) || ((de[0] == 0x9f) && (de[1] == 0x03)) )
            {
            PutIccLogTitle( de[0], de[1], (UCHAR *)&row, (UCHAR *)&col );

            TL_bcd2asc( 6, log, buf1 ); // convert to ascii(L-V)
            TL_insert_decimal_point( 0x04, buf1 );
            TL_trim_decimal( g_term_tx_exp, g_term_decimal_point, buf1 ); // NNNNN

            // show it
            TL_trim_asc( 0, buf1, buf2 ); // ignore the leading zeros
            UI_PutStr( row, col, FONT0, buf2[0], &buf2[1] );
            log += len;
            continue;
            }


          if( (de[2] == DEF_AN) || (de[2] == DEF_ANS) )
            {
            PutIccLogTitle( de[0], de[1], (UCHAR *)&row, (UCHAR *)&col );
            UI_PutStr( row, col, FONT0, len, log );
            log += len;
            }
          else
            {
            PutIccLogTitle( de[0], de[1], (UCHAR *)&row, (UCHAR *)&col );
            for( i=0; i<len; i++ )
               TL_DispHexByte( row, col+i*2, *log++ );
            }

          if( (de[0] == 0x5F) && (de[1] == 0x2A) )
            {
            de[0] = 0xFF;
            de[1] = 0x01;
            PutIccLogTitle( de[0], de[1], (UCHAR *)&row, (UCHAR *)&col );
            }
          else
            {
            if( (de[0] == 0x9F) && (de[1] == 0x36) )
              {
              de[0] = 0xFF;
              de[1] = 0x02;
              PutIccLogTitle( de[0], de[1], (UCHAR *)&row, (UCHAR *)&col );
              }
            }

//        row++;
//        if( row >= 8 )
//          {
//          UI_WaitKey();
//          UI_ClearRow( 1, 7, FONT0 );
//          row = 1;
//          }

//    UI_WaitKey();

//        goto DOL_NEXT;
          continue;
          }

//  TL_DispHexByte(0,0,0x22);
//  UI_WaitKey();

        // ------ tag(s) source is "ICC" ? ------
        index = Scan_IDE( tag1, tag2, &de[0] );

        if( index != 255 )
          {
          goto FIND_TDE;
          }
        else // tag(s) source is not found
          {

DOL_UNKNOWN:
          // fill with hex 0
          for( i=0; i<len; i++ )
             g_ibuf[i] = 0x00;

          }

//DOL_NEXT:
//      *length += len; // accumulate data element length
        
        } while( dol_len > 0 );

//    UI_WaitKey();
}

// ---------------------------------------------------------------------------
// FUNCTION: Waiting for inserting ICC and then process ATR function.
// INPUT   : none.
// OUTPUT  : atr - to store the contents of ATR, max 32 bytes.
// RETURN  : emvOK
//           emvFailed
//           emvAborted      (aborted by user)
//           emvOutOfService (RSA-SAM has failed or not been installed)
// ---------------------------------------------------------------------------
UCHAR PBOC_startup( UCHAR *atr )
{
UCHAR buffer[34];
UCHAR flag;


STARTUP:

      UI_ClearScreen();
      flag = 0;
//    g_dhn_msr = api_msr_open( isoTrack1 + isoTrack2 ); // MSR track 1 & 2

      while(1)
           {
           if( api_emv_CardPresent( g_dhn_icc ) == emvReady )
             {
//           EMVDC_SetupPosEntryMode( PEM_ICC );

//           api_msr_close( g_dhn_msr );
             break;
             }
           else
             {
             // "INSERT CARD"
             if( flag == 0 )
               {
               PBOC_DISP_insert_card();
               flag = 1;
               }

             if( api_kbd_status(g_dhn_kbd, buffer) == apiReady ) // key pressed?
               {
               if( UI_WaitKey() == 'x' )   // cancel?
                 {
//               api_msr_close( g_dhn_msr );
                 return( emvAborted ); // abort
                 }
               }

             // MSR
//           api_msr_status( g_dhn_msr, 0, buffer );
//           if( (buffer[0] == msrSwiped) && ((buffer[1] == msrDataOK) || (buffer[2] == msrDataOK)) )
//             {
//             if( EMVDC_MsrProcessing( PEM_MSR, buffer ) )
//               return( apiNotReady );	// for PCI MSR Online Transaction Only
//             else
//               goto STARTUP;		// for EMV L2 only
//             }

             }
           }

      UI_ClearRow( 0, 1, FONT1 );

      // show "PLEASE WAIT"
      PBOC_DISP_please_wait();

#ifdef	USE_RSA_SAM

      // reset RSA-SAM
      if( api_emv_ATR( g_dhn_sam, atr ) != emvOK )
        return( emvOutOfService );
      else
        {
        // select AID = "A0 00 00 00 00 00 02"
        if( api_emv_SelectSAM() != emvOK )
          return( emvOutOfService );

        api_emv_CleanSAM();              // garbage collection
        }
#endif

      // reset target ICC
      if( api_emv_ATR( g_dhn_icc, atr ) == emvOK )
        {
//      TL_WaitTime( 20 );
        return( emvOK );
        }
      else
        {
        // CARD ERROR
        // REMOVE CARD

        return( emvFailed );
        }
}

// ---------------------------------------------------------------------------
// FUNCTION: Close session and wait for removing card due to error found or
//           normal end of process.
//           The related message should be shown at the 1'st row of display
//           before calling this function.
// INPUT   : wait -- 0 = wait N seconds
//                   1 = wait unitil any key is pressed.
//                 255 = no waiting.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void PBOC_close_session( UCHAR wait )
{

      api_emv_Deactivate( g_dhn_icc ); // disable all ICC contacts
#ifdef	USE_RSA_SAM
      api_emv_Deactivate( g_dhn_sam ); // disable all SAM contacts
#endif

      switch( wait )
            {
            case 0:
                 TL_WaitTimeAndKey( 300 );      // wait 3 sec
                 break;

            case 1:
                 UI_WaitKey();                  // wait key stroke
                 break;

            case 255:
                 break;                         // no waiting
            }

//    if( wait == 0 )
//      TL_WaitTimeAndKey( 150 );      // wait 1.5 sec
//    else
//      UI_WaitKey();                  // wait key stroke

      UI_CloseKeyAll();

      PBOC_DISP_remove_card();          // instruct to remove card
      while( api_emv_CardPresent( g_dhn_icc ) == emvReady );

      api_emv_CloseSession( g_dhn_icc );
#ifdef	USE_RSA_SAM
      api_emv_CloseSession( g_dhn_sam );
#endif

      UI_OpenKeyAll();
}

// ---------------------------------------------------------------------------
// FUNCTION: Select the application to be run.
// INPUT   : appname - LINK[1] LEN[1] NAME[]
//           type    - 0 = normal selection.
//                     1 = special selection.
// OUTPUT  : fci     - 2L-V, the file control information. (RFU)
// REF     : g_ibuf
// RETURN  : emvOK
//           emvFailed
//           emvOutOfService - mandatory data objects missing.
// NOTE    : if the return value equals "emvFailed", the final selected
//           application shall be removed from the system candidate list.
//           ie, the length field of the ordered candidate list is 0 and
//           name field is cleared to space characters.
//           used by PBOC2.0 Only
// ---------------------------------------------------------------------------
UCHAR PBOC_emv_FinalSelection( UCHAR type, UCHAR *appname, UCHAR *fci )
{
UINT  addr;
UINT  iLen;
UCHAR *ptrobj;
UCHAR *ptrde;
UCHAR result;
UCHAR cnt;
UCHAR *ptrnext;

//UCHAR temp[128];


      if( type  == 0 )
        {
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


        // select the application by using "AID" or "DFNAME"
        result = apk_SelectADF( g_ibuf, ptrobj, 0 );

//  PBOC_init_test_para( g_ibuf );
//  result = apkFileInvalidated;

        }
      else
        {
        memmove( g_ibuf, fci, fci[0]+fci[1]*256+2 ); // retrieve FCI for analyzing, 2L-V
        result = apkOK; // for type = 1, assume it is good for parsing
        }


      if( result == apkFileInvalidated ) // 6283 - application blocked
        {
        memmove( fci, g_ibuf, g_ibuf[0]+g_ibuf[1]*256+2 ); // backup FCI although application blocked, 2L-V
        return( result );
        }

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

        // clear epin & ksn data for Enciphered PIN online
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
// FUNCTION: (1) Create candidate list.
//           (2) Selection the application to be run from the candidate list.
//               (Final selection)
// INPUT   : occurrence - 0x00 = the 1'st time to select.
//                        0x01 = the following time to select.
// OUTPUT  : candid - the candidate id, index to ADDR_CANDIDATE table.
// OUTPUT  : fci    - 2L-V, the file control information.
// RETURN  : emvOK
//           emvNotReady (the application cannot be applied) --> try next one.
//           emvFailed
// NOTE    : used by PBOC2.0 - Read ICC Log Only
// ---------------------------------------------------------------------------
UCHAR PBOC_select_application( UCHAR occurrence, UCHAR *candid, UCHAR *fci )
{
UCHAR result;
UCHAR result2;
UCHAR list_cnt;
UCHAR list[MAX_CANDIDATE_NAME_CNT][CANDIDATE_NAME_LEN];
UCHAR buffer[6];

      // if 1'st time, to create the candidate list
      if( occurrence == 0x00 )
        {
        // create the candidate list
        result = api_emv_CreateCandidateList_PBOC();
        if( result == emvFailed )
          return( result );
        if( result == emvNotReady ) // no mutually supported applications
          return( emvFailed );

        if( result == emvNotSupported ) // PATCH: PBOC2.0, 2006-02-13, to be falled back to msr process
          return( result );
        }

      // get the candidate list
      //
      // format: LINK[1] LEN1[1] NAME1[16]
      //         LINK[1] LEN2[1] NAME2[16]...
      //         LINK[1] LEN16[1] NAME16[16]

      while(1)
           {
           list_cnt = api_emv_GetCandidateList( &list[0][0] );
           if( list_cnt == 0 )
             return( emvFailed ); // no any application supported

           // special check for auto-selecting application
           if( api_emv_AutoSelectApplication( occurrence, list_cnt, &result ) == TRUE )
             goto SEL_APP;

//         TL_DispHexByte(0,0,list_cnt);
//         UI_WaitKey();
//         TL_DumpHexData(0,0,18, &list[0][0]);
//         TL_DumpHexData(0,0,18, &list[1][0]);
//         for(;;);

           // show the candidate list to attendant
           UI_ClearScreen();
           PBOC_DISP_select_app();

           // wait for attendant's selection for 30 seconds
           buffer[0] = 2;
           buffer[1] = 4;
           buffer[2] = list_cnt;
           buffer[3] = CANDIDATE_NAME_LEN;
           buffer[4] = 1;
           buffer[5] = FONT0;
           result = TL_ListBox( 0, &buffer[0], &list[0][0], 00 ); // always wait

SEL_APP:
           g_candidate_name_index = result; // backup the selected item# for later use

           if( result == 255 )
             return( emvFailed ); // timeout or aborted

           // final selection
           result2 = PBOC_emv_FinalSelection( 0, &list[result][0], fci );
           if( result2 == 0x80 ) // 6283 - application blocked
             return( result2 );

           if( result2 == emvOutOfService )
             return( emvFailed );

           if( result2 == emvFailed )
             {
             if( list_cnt <= 1 )
               return( emvFailed );
             else
               {
               // show "TRY AGAIN" for 1 second
        //     UI_BeepLong();
        //     UI_ClearScreen();
        //     PBOC_DISP_try_again();
        //     TL_WaitTimeAndKey( 100 );
        //
        //     continue;

               return( emvNotReady );
               }
             }
           else // target application is confirmed OK
             {
             candid = (UCHAR *)&list[result][0];

             api_emv_SetupCardParameters(); // setup card related parameters

             return( emvOK );
             }
           } // while(1)

}

// ---------------------------------------------------------------------------
// FUNCTION: read and print out the transaction logs in ICC.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : emvOK
//           emvFailed
// ---------------------------------------------------------------------------
UCHAR PBOC_ReadIccLog( void )
{
UCHAR result;
UCHAR occurrence;
UCHAR buffer[21];
UINT  iLen;
UCHAR sfi;
UCHAR cnt;
UCHAR rec_num;
UCHAR fci[260];


      UI_ClearRow( 3, 1, FONT0 );
//      memmove( buffer, (UCHAR *)msg_okstart, 19 );
//      UI_PutMsg( 7, COL_LEFTMOST, FONT0, 19, buffer );
//      UI_WaitKey();

      result = PBOC_startup( fci );
      if( result != emvOK )
        {
        PBOC_DISP_out_of_service();
        PBOC_close_session(0);
        return( emvFailed );
        }

      occurrence = 0;

      // --- Select Application ---
SELECT_APP:

      UI_OpenKeyAll();

      result = PBOC_select_application( occurrence, &g_candidate_list_index, fci );

      if( result == 0x80 ) // PATCH: PBOC2.0, 2006-02-17, 2CQ006, 6283 - application blocked
        PBOC_emv_FinalSelection( 1, buffer, fci ); // get FCI response

      if( result == emvNotReady )
        {
        // show "TRY AGAIN" for 1.5 second
        UI_BeepLong();
        UI_ClearScreen();
        PBOC_DISP_try_again();
        TL_WaitTimeAndKey( 150 );

        occurrence = 1;
        goto SELECT_APP; // select another application
        }

      if( result == emvFailed )
        {
        PBOC_DISP_not_accepted();
        PBOC_close_session(0);
        return( emvFailed );
        }
      else // application selected OK
        {
        apk_ReadRamDataICC( ADDR_ICC_LOG_ENTRY, 4, buffer ); // retrieve log entry = SFI(1) + CNT(1)

//  TL_DumpHexData(0,0,4, buffer);
//  buffer[0] = 4;
//  buffer[1] = 0;
//  buffer[2] = 0x0b; // sfi
//  buffer[3] = 0x02; // cnt
    

        if( buffer[0] == 0 )
          return( emvFailed );

        sfi = buffer[2];
        cnt = buffer[3];

        // --- Get Log Format ---

GET_LOG_FORMAT:

        result = apdu_GET_DATA( 0x9f, 0x4f, g_temp );

// --- Test Pattern ---
//  result = apiOK;
//  g_temp[0] = 30;
//  g_temp[1] = 0;
//
//  g_temp[2] = 0x9f;
//  g_temp[3] = 0x4f;
//  g_temp[4] = 0x19;
//
//  g_temp[5] = 0x9a;
//  g_temp[6] = 0x03;
//
//  g_temp[7] = 0x9f;
//  g_temp[8] = 0x21;
//  g_temp[9] = 0x03;
//
//  g_temp[10] = 0x9f;
//  g_temp[11] = 0x02;
//  g_temp[12] = 0x06;
//
//  g_temp[13] = 0x9f;
//  g_temp[14] = 0x03;
//  g_temp[15] = 0x06;
//
//  g_temp[16] = 0x9f;
//  g_temp[17] = 0x1a;
//  g_temp[18] = 0x02;
//
//  g_temp[19] = 0x5f;
//  g_temp[20] = 0x2a;
//  g_temp[21] = 0x02;
//
//  g_temp[22] = 0x9f;
//  g_temp[23] = 0x4e;
//  g_temp[24] = 0x14;
//
//  g_temp[25] = 0x9c;
//  g_temp[26] = 0x01;
//
//  g_temp[27] = 0x9f;
//  g_temp[28] = 0x36;
//  g_temp[29] = 0x02;
//
//  g_temp[30] = 0x90;
//  g_temp[31] = 0x00;

        if( result  == apiOK )
          {
          iLen = g_temp[1]*256 + g_temp[0];
          if( (iLen < 6) || (g_temp[2] != 0x9f) || (g_temp[3] != 0x4f) ||
              (g_temp[iLen] != 0x90) || (g_temp[iLen+1] != 0x00) )
            return( emvFailed );
          else
            {
            // L L 9F 4F L V
            // the log format: V = T1-L1, T2-L2,...Tn-Ln
            iLen = g_temp[4];
            if( (iLen == 0) || (iLen > 248) )
              return( emvFailed ); // out of memory

            apk_WriteRamDataICC( ADDR_ICC_LOG_FORMAT, 2, (UCHAR *)&iLen ); // save log format
            apk_WriteRamDataICC( ADDR_ICC_LOG_FORMAT+2, iLen, (UCHAR *)&g_temp[5] );

            // --- Read ICC Log Records ---
            rec_num = 1; // set record number to 1 for next read
            while( cnt )
                 {
                 g_ibuf[0] = 0; // clear output length
                 g_ibuf[1] = 0; //
                 result = apk_ReadRecord( sfi, rec_num, g_ibuf );  // read log record

// --- Test Pattern ---
//  result = apiOK;
//  g_ibuf[0] = 47;
//  g_ibuf[1] = 0;
//
//  g_ibuf[2] = 0x06;
//  g_ibuf[3] = 0x02;
//  g_ibuf[4] = 0x13;
//
//  g_ibuf[5] = 0x12;
//  g_ibuf[6] = 0x34;
//  g_ibuf[7] = 0x56;
//
//  g_ibuf[8] = 0x00;
//  g_ibuf[9] = 0x00;
//  g_ibuf[10] = 0x00;
//  g_ibuf[11] = 0x12;
//  g_ibuf[12] = 0x34;
//  g_ibuf[13] = 0x56;
//
//  g_ibuf[14] = 0x00;
//  g_ibuf[15] = 0x00;
//  g_ibuf[16] = 0x00;
//  g_ibuf[17] = 0x00;
//  g_ibuf[18] = 0x00;
//  g_ibuf[19] = 0x00;
//
//  g_ibuf[20] = 0x01;
//  g_ibuf[21] = 0x58;
//
//  g_ibuf[22] = 0x01;
//  g_ibuf[23] = 0x58;
//
//  g_ibuf[24] = 0x41;
//  g_ibuf[25] = 0x42;
//  g_ibuf[26] = 0x43;
//  g_ibuf[27] = 0x44;
//  g_ibuf[28] = 0x45;
//  g_ibuf[29] = 0x46;
//  g_ibuf[30] = 0x47;
//  g_ibuf[31] = 0x48;
//  g_ibuf[32] = 0x49;
//  g_ibuf[33] = 0x4a;
//  g_ibuf[34] = 0x4b;
//  g_ibuf[35] = 0x4c;
//  g_ibuf[36] = 0x4d;
//  g_ibuf[37] = 0x4e;
//  g_ibuf[38] = 0x4f;
//  g_ibuf[39] = 0x50;
//  g_ibuf[40] = 0x51;
//  g_ibuf[41] = 0x52;
//  g_ibuf[42] = 0x53;
//  g_ibuf[43] = 0x54;
//
//  g_ibuf[44] = 0x00;
//
//  g_ibuf[45] = 0x11;
//  g_ibuf[46] = 0x22;
//
//  g_ibuf[47] = 0x90;
//  g_ibuf[48] = 0x00;


                 if( result == apkFailed )
                   return( emvFailed );

                 if( result == apkRecNotFound )
                   return( emvOK ); // EOF

                 // FORMAT: 2L [(V1)-(V2)...(Vn)], no 70-L  
//               if(g_ibuf[2] != 0x70)
//                 return( emvFailed );
//               else
//                 {
                   // print out the contents of each log record
                   UI_ClearScreen();

                   memmove( buffer, (UCHAR *)PBOCmsg_recno, 7 );
                   UI_PutMsg( 0, COL_LEFTMOST, FONT0+attrCLEARWRITE, 7, buffer );
                   buffer[0] = TL_itoa( (UINT)rec_num, &buffer[1] );
                   UI_PutStr( 0, 7, FONT0, buffer[0], &buffer[1] );

                   Build_LogDOL( &g_temp[4], &g_ibuf[2] );
//                 }

                 cnt--;
                 rec_num++; // next record
                 }
            }
          }
        else
          return( emvFailed );

        }

      UI_ClearScreen();
      return( emvOK );
}

// ---------------------------------------------------------------------------
// FUNCTION: check cardholder's license (PBOC2.0 ONLY)
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - failed
// ---------------------------------------------------------------------------
UCHAR PBOC_VerifyCardholderLicense( void )
{
UCHAR aip[2];    // application interchange profile
UCHAR tcap[3];   // terminal capabilities
UCHAR buf1[50];
UCHAR buf2[8];
UINT  iLen;
UINT  iLen2;


      // get icc_AIP[0..1]
//    api_emv_GetDataElement( DE_ICC, ADDR_ICC_AIP, 2, aip );
//    if( (aip[0] & AIP0_CVM) == 0 ) // CVM supported?
//      return;
  
      // get current terminal capabilites
//    apk_ReadRamDataTERM( ADDR_TERM_CAP, 3, tcap );
//    if( (tcap[1] & CAP1_CH_LICENSE_VERIFY) == 0 )
//      return;

//    memset( &g_ibuf[2], 0x41, 40 );
//    g_ibuf[0] = 40;
//    g_ibuf[1] = 0;
//    apk_WriteRamDataICC( ADDR_ICC_CH_LICENSE_ID, 42, g_ibuf );
//    g_ibuf[0] = 1;
//    g_ibuf[1] = 0;
//    g_ibuf[2] = 0;
//    apk_WriteRamDataICC( ADDR_ICC_CH_LICENSE_TYPE, 3, g_ibuf );

      // ---------------------------------------------------------------
      apk_ReadRamDataICC( ADDR_ICC_CH_LICENSE_ID, 42, buf1 );
      apk_ReadRamDataICC( ADDR_ICC_CH_LICENSE_TYPE, 3, buf2 );
      iLen = buf1[0] + buf1[1]*256;
      iLen2 = buf2[0] + buf2[1]*256;

//    if( iLen2 == 0 ) // no license type
//      return( FALSE );

      UI_ClearScreen();
      PBOC_DISP_verify_ch_license();

      if( iLen2 != 0 ) // license type here
        {
        switch( buf2[2] )
              {
              case 00:
                   PBOC_DISP_license_00();
                   break;

              case 01:
                   PBOC_DISP_license_01();
                   break;

              case 02:
                   PBOC_DISP_license_02();
                   break;

              case 03:
                   PBOC_DISP_license_03();
                   break;

              case 04:
                   PBOC_DISP_license_04();
                   break;

              default:
                   PBOC_DISP_license_05();
                   break;
              }
        }

      // show license number to merchant for checking
      if( iLen > 21 )
        {
        UI_PutStr( 4, 0, FONT0+attrCLEARWRITE, 21, &buf1[2] );
        if( (iLen-21) != 0 )
          UI_PutStr( 5, 0, FONT0+attrCLEARWRITE, iLen-21, &buf1[2+21] );
        }
      else
        {
        if( iLen != 0 )
          UI_PutStr( 4, 0, FONT0+attrCLEARWRITE, iLen, &buf1[2] );
        }

      PBOC_DISP_correct_or_not();
      while(1)
           {
           buf1[0] = UI_WaitKey();
           if( buf1[0] == 'y' )
             {
             UI_ClearScreen();
             return( TRUE );
             }

           if( buf1[0] == 'x' )
             {
             UI_ClearScreen();
             return( FALSE );
             }
           }
}
