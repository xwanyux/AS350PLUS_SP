//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : PBOC 2.0 LEVEL 2 Debit/Credit                              **
//**  PRODUCT  : AS330_mPOS						    **
//**                                                                        **
//**  FILE     : PBOC20_2.C                                                 **
//**  MODULE   : Functions used for PBOC2.0 specification.                  **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2018/08/08                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2006-2018 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include <string.h>

#include "EMVKconfig.h"

#include "POSAPI.h"
#include "EMVDC.h"
#include "GDATAEX.h"
#include "EMVAPI.h"
#include "EMVAPK.h"
#include "EMVDCMSG.h"
#include "XCONSTP5.h"
#include "UI.h"

// ---------------------------------------------------------------------------
// FUNC  : Setup all realted EMV parameters for the selected AID.
//         except: Application Version Number (AVN)
//
// INPUT : none.
// OUTPUT: none.
// REF   : g_selected_aid_index
// RETURN: none.
// ---------------------------------------------------------------------------
//#ifdef  L2_PBOC20
void  PBOC_SetupCardParameters( void )
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
//#endif

// ---------------------------------------------------------------------------
// FUNCTION: Convert AID data format from hex to ascii.
// INPUT   : ibuf - LEN1(1) DATA1(n)
//                  LEN2(1) DATA2(n)
//                  ...
// OUTPUT  : obuf - LEN1(1) DATA1(n*2)
//                  LEN2(1) DATA2(n*2)
//                  ...
// RETURN  : none.
// ---------------------------------------------------------------------------
//#ifdef  L2_PBOC20
void ConvertAidDataFormat1( UCHAR *ibuf, UCHAR *obuf )
{
UCHAR i, j;
UCHAR buf[3];

      for( i=0; i<MAX_AID_CNT; i++ )
         {
         obuf[i*35] = ibuf[i*TERM_AID_LEN]*2; // length of AID entry

         if( ibuf[i*TERM_AID_LEN] !=0 )
           {
           for( j=0; j<ibuf[i*TERM_AID_LEN]; j++ )
              {
              TL_hex2asc( 1, &ibuf[j+(i*TERM_AID_LEN+1)], buf );
              memmove( &obuf[j*2+(i*35+1)], &buf[1], 2 );
              }
           }
         else
           {
           obuf[i*35] = 2;      // dummy settings
           obuf[i*35+1] = 0x30; //
           obuf[i*35+2] = 0x30; //
           }
         }
}
//#endif

// ---------------------------------------------------------------------------
// FUNCTION: Data entry routine.
// INPUT   : row       - start row number.
//           col       - start col number.
//           len       - length of guideline.
//           guideline - empty patterns.
//           databyte  - length of data bytes.
//           ibuf      - the old values.
// OUTPUT  : obuf      - the new values. L-V
// RETURN  : TRUE  - ok.
//           FALSE - aborted.
// ---------------------------------------------------------------------------
//#ifdef  L2_PBOC20
UCHAR RequestDataEntry( UCHAR row, UCHAR col, UCHAR len, UCHAR *guideline, UCHAR databyte, UCHAR *ibuf, UCHAR *obuf )
{
UCHAR i;
UCHAR cnt;
UCHAR result;
UCHAR bak_col;

      bak_col = col;

FILLER:
      // show null filler
      UI_PutStr( row, col, FONT0+attrCLEARWRITE, len, guideline );

      i = 0;
      cnt = 0;

      while(1)
           {
           memset( obuf, 0x00, 2 );
           result = TL_GetAlphaNumKey( 0, row, col, FONT0, 2, obuf );
           if( result == FALSE )
             {
             if( obuf[0] == KEY_CLEAR )
               {
               col = bak_col;
               goto FILLER;
               }
             else
               return( FALSE );
             }

//         if( (obuf[0] == 0) && (obuf[1] == 0) )
//           return( FALSE );

           ibuf[i++] = obuf[0];
           ibuf[i++] = obuf[1];

           col += 3;

           UI_PutStr( row, col, FONT0, 2, guideline );

           if( ++cnt == databyte )
             break;
           }

      // update
      for( i=0; i<databyte; i++ )
         {
         if( ibuf[i*2] != 0x00 )
           obuf[i+1]= TL_ascw2hexb( &ibuf[i*2] ); // convert to hex format
         else
           break;
         }
      obuf[0] = i; // actual length of outputs

//    for( i=0; i<databyte; i++ )
//       {
//       if( obuf[i+1] == 0 )
//         {
//         obuf[0] = i; // actual length of outputs
//         break;
//         }
//       }

      return( TRUE );
}
//#endif

// ---------------------------------------------------------------------------
// FUNCTION: Delete one AID from kernel.
// INPUT   : aid   - the AID start entry.
//           index - the target AID index. (00.15)
// OUTPUT  : none.
// RETURN  : TRUE  - ok.
//           FALSE - failed.
// ---------------------------------------------------------------------------
//#ifdef  L2_PBOC20
UCHAR DeleteOneAID( UCHAR index, UCHAR *aid )
{
UCHAR i;

      if( index > MAX_AID_CNT )
        return( FALSE );

      i = 0;
      for( i=0; i<(MAX_AID_CNT-index-1); i++ )
         memmove( &aid[TERM_AID_LEN*(index+i)], &aid[TERM_AID_LEN*(index+i+1)], TERM_AID_LEN );

      aid[TERM_AID_LEN*(index+i)] = 0x00;

      return( TRUE );
}
//#endif

// ---------------------------------------------------------------------------
// FUNCTION: setup AID
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
//
//             01234567890123456789
//           0 SET AID
//           1 > NNNNNNNN
//           2   NNNNNNNN
//           3   NNNNNNNN  <- AID = LEN[1] ASI[1] AID[16]
//               ...
//           7 PLEASE SELECT AID
// ---------------------------------------------------------------------------
//#ifdef  L2_PBOC20
void  PBOC_SetAID( void )
{
UCHAR buffer[21];
UCHAR buffer2[21];
UCHAR list[MAX_AID_CNT][TERM_AID_LEN];
UCHAR abuf[MAX_AID_CNT][35];
UCHAR result;
UCHAR start;
UCHAR i;

SET_AID_START:

      memset( &list[0][0], 0x00, MAX_AID_CNT*TERM_AID_LEN );

      start = 0;
      while(1)
           {
           UI_ClearScreen();
           UI_PutMsg( 0, COL_LEFTMOST, FONT0, EMVDC_ITEM_LEN, (UCHAR *)&EMVDC_Func_Table[EMVDC_FUNC_LEN*FN_SET_AID+1] );
           memmove( buffer, (UCHAR *)msg_pls_select_aid, 17 );
           UI_PutMsg( 7, COL_LEFTMOST, FONT0, 17, buffer );

           buffer[0] = 1; // starting row number
           buffer[1] = MAX_DSP_ROW_CNT;
           buffer[2] = MAX_AID_CNT;        // max list cnt
           buffer[3] = 35;                 // item length
           buffer[4] = 0; // offset of LEN field in item
           buffer[5] = FONT0;

           // read AIDs from memory
           api_emv_GetDataElement( DE_TERM, ADDR_TERM_AID_START, MAX_AID_CNT*TERM_AID_LEN, &list[0][0] );

           // convert hex data to ascii
           ConvertAidDataFormat1( &list[0][0], &abuf[0][0] );

//         for( i=0; i<16; i++ )
//            {
//            TL_DispHexByte(0,0,i);
//            TL_DumpHexData(0,1, abuf[i][0]+1, &abuf[i][0] );
//            }
//         for(;;);

//         g_test_flag = 1;
           result = TL_ListBox( start, &buffer[0], &abuf[0][0], 00 ); // no timeout

           if( result == 0xff ) // aborted
             return;

           // modify AID & ASI according to the selected index (00..15)
           UI_ClearScreen();

           // ASI
           memmove( buffer, (UCHAR *)msg_asi, 4 );
           UI_PutStr( 0, 0, FONT0, 4, buffer );
           memmove( buffer, &abuf[result][1], 2 );
           UI_PutStr( 0, 4, FONT0, 2, buffer );

           // AID
           memmove( buffer, (UCHAR *)msg_aid, 4 );
           UI_PutStr( 1, 0, FONT0, 4, buffer );
           memmove( buffer, &abuf[result][3], abuf[result][0]-2 );
           UI_PutStr( 1, 4, FONT0, abuf[result][0]-2, buffer );

           // update?
//         if( TL_UpdateReq( 0, 3, COL_LEFTMOST, FONT0 ) == FALSE )
//           return;

           // 0-DELETE, 1-UPDATE ?
           UI_ClearRow( 7, 1, FONT0 );
           memmove( buffer, (UCHAR *)msg_0delete_1update, 18 );
           UI_PutStr( 3, 0, FONT0+attrCLEARWRITE, 18, buffer );

           do
            {
            i = UI_WaitKey();

            switch( i )
                  {
                  case '0': // delete
                       DeleteOneAID( result, &list[0][0] );
                       api_emv_PutDataElement( DE_TERM, ADDR_TERM_AID_START, MAX_AID_CNT*TERM_AID_LEN, &list[0][0] );

                       goto SET_AID_START;

                  case '1': // update
                       break;

                  case 'x': // cancel
                       goto SET_AID_START;
                  }
            } while( (i != '0') && (i != '1') );

           memmove( buffer, (UCHAR *)msg_asi, 4 ); // ASI=?
           UI_PutStr( 2, 0, FONT0+attrCLEARWRITE, 4, buffer );

           memmove( buffer, (UCHAR *)msg_null_asi, 2 );
           if( RequestDataEntry( 3, 0, 2, buffer, 1, &list[result][1], buffer ) == TRUE )
             api_emv_PutDataElement( DE_TERM, ADDR_TERM_AID_01+result*TERM_AID_LEN+1, 1, &buffer[1] );

           memmove( buffer, (UCHAR *)msg_aid, 4 ); // AID=?
           UI_PutStr( 2, 0, FONT0+attrCLEARWRITE, 4, buffer );

           memmove( buffer, (UCHAR *)msg_null_aid, 20 );
           if( RequestDataEntry( 3, 0, 20, buffer, 7, &list[result][2], buffer2 ) == TRUE )
             {
             if( buffer2[0] != 0 )
               {
               api_emv_PutDataElement( DE_TERM, ADDR_TERM_AID_01+result*TERM_AID_LEN+2, 16, &buffer2[1] );
               buffer2[0] = 1 + buffer2[0];
               api_emv_PutDataElement( DE_TERM, ADDR_TERM_AID_01+result*TERM_AID_LEN, 1, buffer2 ); // put final length of ASI+AID
               }
             }

           start = result;
           } // while(1)

}
//#endif

// ---------------------------------------------------------------------------
// FUNCTION: select AID
// INPUT   : cursor - the start cursor positon.
// OUTPUT  : none.
// RETURN  : -1     - if aborted or time out.
//         : others - item number of the selection. (0..15)
//
//             01234567890123456789
//           0 [TITLE]
//           1 > NNNNNNNN
//           2   NNNNNNNN
//           3   NNNNNNNN  <- AID = LEN[1] ASI[1] AID[16]
//               ...
//           7 PLEASE SELECT AID
// ---------------------------------------------------------------------------
//#ifdef  L2_PBOC20
UCHAR PBOC_SelectAID( UCHAR cursor )
{
UCHAR buffer[21];
UCHAR list[MAX_AID_CNT][TERM_AID_LEN];
UCHAR abuf[MAX_AID_CNT][35];
UCHAR result;

      memset( &list[0][0], 0x00, MAX_AID_CNT*TERM_AID_LEN );

//    start = 0;
//    while(1)
//         {
           memmove( buffer, (UCHAR *)msg_pls_select_aid, 17 );
           UI_PutMsg( 7, COL_LEFTMOST, FONT0, 17, buffer );

           buffer[0] = 1; // starting row number
           buffer[1] = MAX_DSP_ROW_CNT;
           buffer[2] = MAX_AID_CNT;        // max list cnt
           buffer[3] = 35;                 // item length
           buffer[4] = 0; // offset of LEN field in item
           buffer[5] = FONT0;

           // read AIDs from memory
           api_emv_GetDataElement( DE_TERM, ADDR_TERM_AID_START, MAX_AID_CNT*TERM_AID_LEN, &list[0][0] );

           // convert hex data to ascii
           ConvertAidDataFormat1( &list[0][0], &abuf[0][0] );

           result = TL_ListBox( cursor, &buffer[0], &abuf[0][0], 00 ); // no timeout

           UI_ClearRow( 1, 7, FONT0 );

           return( result );

//         if( result == 0xff ) // aborted
//           return( result );
//
//         start = result;
//         } // while(1)

}
//#endif

// ---------------------------------------------------------------------------
// FUNCTION: print out the data element.
// INPUT   : de  = tag1, tag2, dataformat, length
//                    dataformat = n, b, ans
//           data   = the contents of the data element
// OUTPUT  : none.
// REF     : g_ibuf
// RETURN  : none.
// ---------------------------------------------------------------------------
#ifdef  L2_PBOC20
void  Print_DataElement( UCHAR *de, UCHAR *data )
{
UCHAR buf[40];
UCHAR len;
      
      len = de[3];
      TL_PrintHexData( len, data, buf );
      buf[len] = 0x0a;
      UI_PrintPutStr( len+1, buf );
}
#endif

// ---------------------------------------------------------------------------
#ifdef  L2_PBOC20
void  PutIccLogTitle( UCHAR tag1, UCHAR tag2, UCHAR *row, UCHAR *col )
{
UINT  iTag;
UCHAR buf[32];

      iTag = tag1*256 + tag2;

      switch( iTag )
            {
            case 0x009A:
                 memmove( buf, (UCHAR *)msg_date, 5 );
                 UI_PutMsg( 1, COL_LEFTMOST, FONT0, 5, buf );
                 *row = 1;
                 *col = 5;
                 break;

            case 0x9F21:
                 memmove( buf, (UCHAR *)msg_time, 5 );
                 UI_PutMsg( 2, COL_LEFTMOST, FONT0, 5, buf );
                 *row = 2;
                 *col = 5;
                 break;

            case 0x9F02:
                 memmove( buf, (UCHAR *)msg_amt_auth, 8 );
                 UI_PutMsg( 3, COL_LEFTMOST, FONT0, 10, buf );
                 *row = 3;
                 *col = 8;
                 break;

            case 0x9F03:
                 memmove( buf, (UCHAR *)msg_amt_other, 8 );
                 UI_PutMsg( 4, COL_LEFTMOST, FONT0, 10, buf );
                 *row = 4;
                 *col = 8;
                 break;

            case 0x9F1A:
                 memmove( buf, (UCHAR *)msg_country_code, 14 );
                 UI_PutMsg( 5, COL_LEFTMOST, FONT0, 14, buf );
                 *row = 5;
                 *col = 14;
                 break;

            case 0x5F2A:
                 memmove( buf, (UCHAR *)msg_currency_code, 14 );
                 UI_PutMsg( 6, COL_LEFTMOST, FONT0, 14, buf );
                 *row = 6;
                 *col = 14;
                 break;

            case 0x9F4E:
                 memmove( buf, (UCHAR *)msg_merchant_name, 14 );
                 UI_PutMsg( 1, COL_LEFTMOST, FONT0, 14, buf );
                 *row = 2;
                 *col = 0;
                 break;

            case 0x009C:
                 memmove( buf, (UCHAR *)msg_trans_type, 11 );
                 UI_PutMsg( 3, COL_LEFTMOST, FONT0, 11, buf );
                 *row = 3;
                 *col = 11;
                 break;

            case 0x9F36:
                 memmove( buf, (UCHAR *)msg_atc, 4 );
                 UI_PutMsg( 4, COL_LEFTMOST, FONT0, 4, buf );
                 *row = 4;
                 *col = 4;
                 break;

            case 0xFF01: // (more)
                 memmove( buf, (UCHAR *)msg_more, 6 );
                 UI_PutMsg( 7, COL_RIGHTMOST, FONT0, 6, buf );
                 *row = 0;
                 *col = 0;
                 UI_WaitKey();
                 UI_ClearRow( 1, 7, FONT0 );
                 break;

            case 0xFF02: // (next)
                 memmove( buf, (UCHAR *)msg_next, 6 );
                 UI_PutMsg( 7, COL_RIGHTMOST, FONT0, 6, buf );
                 *row = 0;
                 *col = 0;
                 UI_WaitKey();
                 break;

            }
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: build and print out the ICC Log DOL.
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
#ifdef  L2_PBOC20
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
      UI_PrintClear();

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
#endif

// ---------------------------------------------------------------------------
//void  PBOC_init_test_para( UCHAR *fci )
//{
//UCHAR buf[] = {
//
//   62, 0,
//   0x6F, 0x3A, 0x84, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x03, 0x10, 0x10, 0xA5, 0x2F, 0x87, 0x01, 
//   0x01, 0x9F, 0x38, 0x12, 0x9F, 0x1A, 0x02, 0x9F, 0x33, 0x03, 0x9F, 0x40, 0x05, 0x9F, 0x1B, 0x04,
//   0x9F, 0x09, 0x02, 0x9F, 0x35, 0x01, 0x5F, 0x2D, 0x08, 0x65, 0x73, 0x65, 0x6E, 0x66, 0x72, 0x64,
//   0x65, 0x9F, 0x11, 0x01, 0x01, 0xBF, 0x0C, 0x05, 0x9F, 0x4D, 0x02, 0x0B, 0x0A, 0x62, 0x83, 
//   };
//
//      memmove( fci, buf, 64 );
//
//
//}

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
#ifdef  L2_PBOC20
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
#endif

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
#ifdef  L2_PBOC20
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
        result = api_emv_CreateCandidateList();
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
           DISP_select_app();

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
        //     DISP_STD_try_again();
        //     TL_WaitTimeAndKey( 100 );
        //
        //     continue;

               return( emvNotReady );
               }
             }
           else // target application is confirmed OK
             {
             candid = (UCHAR *)&list[result][0];

             PBOC_SetupCardParameters(); // setup card related parameters

             return( emvOK );
             }
           } // while(1)

}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: general icc get response command.
// INPUT   : len  - the expected length of data to be retrieved.
// OUTPUT  : data - the response data retrieved from SAM.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
//UCHAR PBOC_GET_RESPONSE( UCHAR len, UCHAR *data )
//{
//UCHAR c_apdu[7];
//UINT  iLen;
//
//      c_apdu[0] = 0x05;                 // length of APDU
//      c_apdu[1] = 0x00;                 //
//
//      c_apdu[2] = 0x00;                     // CLA
//      c_apdu[3] = 0xC0;                     // INS
//      c_apdu[4] = 0x00;                     // P1
//      c_apdu[5] = 0x00;                     // P2
//      c_apdu[6] = len;                      // Le
//
//      if( api_ifm_exchangeAPDU( g_dhn_icc, c_apdu, data ) != apiOK )
//        return( apiFailed );
//
////    iLen = data[1]*256 + data[0];
////    if( ((iLen - 2) != len) || (data[iLen] != 0x90) || (data[iLen+1] != 0x00) )
////      return( apiFailed );
////    else
//        return( apiOK );
//}
//
// ---------------------------------------------------------------------------
// FUNCTION: read and print out the transaction logs in ICC.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : emvOK
//           emvFailed
// ---------------------------------------------------------------------------
#ifdef  L2_PBOC20
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
      memmove( buffer, (UCHAR *)msg_okstart, 19 );
      UI_PutMsg( 7, COL_LEFTMOST, FONT0, 19, buffer );
      UI_WaitKey();

      result = EMVDC_startup( g_temp );
      if( result != emvOK )
        {
        DISP_out_of_service();
        EMVDC_close_session(0);
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
        DISP_STD_try_again();
        TL_WaitTimeAndKey( 150 );

        occurrence = 1;
        goto SELECT_APP; // select another application
        }

      if( result == emvFailed )
        {
        DISP_STD_not_accepted();
        EMVDC_close_session(0);
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

                   memmove( buffer, (UCHAR *)msg_recno, 7 );
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
#endif
// ---------------------------------------------------------------------------
// FUNCTION: setup Default DDOL for terminal.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
//
//             01234567890123456789
//           0 SET DDOL
//           1 NN NN NN NN NN NN NN
//           2 NN................NN
//           3 NN................NN
//           4 NN................NN
//           7 UPDATE(Y/N)?
// ---------------------------------------------------------------------------
//#ifdef  L2_PBOC20
void  PBOC_SetDDOL( void )
{
UINT  i;
UINT  iLen;
UCHAR row;
UCHAR col;
UCHAR buf[64];
UCHAR result;


KEYIN_DDOL_ST:

      UI_ClearRow( 1, 7, FONT0 );
      api_emv_GetDataElement( DE_TERM, ADDR_TERM_DDOL, 2, (UCHAR *)&iLen );
      if( iLen == 0 )
        goto KEYIN_DDOL;
      api_emv_GetDataElement( DE_TERM, ADDR_TERM_DDOL+2, iLen, g_ibuf );

      row = 1;
      col = 0;
      for( i=0; i<iLen; i++ )
         {
         TL_DispHexByte( row, col, g_ibuf[i] );
         col += 3;
         if( col >=19 )
           {
           col = 0;
           row += 1;
           if( row >= 8 )
             row = 1;
           }
         }

      // 0-DELETE, 1-UPDATE ?
      memmove( buf, (UCHAR *)msg_0delete_1update, 18 );
      UI_PutStr( 7, 0, FONT0+attrCLEARWRITE, 18, buf );
      result = UI_WaitKey();
      if( result == 'x' )
        return;

      if( result == '0' )
        {
        g_temp[0] = 0;
        g_temp[1] = 0;
        api_emv_PutDataElement( DE_TERM, ADDR_TERM_DDOL, 2, g_temp );
        goto KEYIN_DDOL_ST;
        }

//    if( TL_UpdateReq( 0, 7, COL_LEFTMOST, FONT0+attrCLEARWRITE ) == TRUE ) // update?
//      {
KEYIN_DDOL:

//      UI_ClearRow( 1, 7, FONT0 );

        // keyin
        if( TL_GetAlphaNumDigits( 7, 0, FONT0, 35, g_ibuf ) == FALSE )
          return;

        if( ((g_ibuf[0] % 2) !=0) || (g_ibuf[0] <= 2) )
          goto KEYIN_DDOL;

        // update
        iLen = g_ibuf[0] / 2;
        for( i=0; i<iLen; i++ )
           g_temp[i+2] = TL_ascw2hexb( &g_ibuf[1+i*2] ); // convert to hex format

        g_temp[0] = (UCHAR)iLen;
        g_temp[1] = 0;
        api_emv_PutDataElement( DE_TERM, ADDR_TERM_DDOL, iLen+2, g_temp );

        goto KEYIN_DDOL_ST;
//      }
}
//#endif

// ---------------------------------------------------------------------------
// FUNCTION: setup Default TDOL for terminal.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
//
//             01234567890123456789
//           0 SET TDOL
//           1 NN NN NN NN NN NN NN
//           2 NN................NN
//           3 NN................NN
//           4 NN................NN
//           7 UPDATE(Y/N)?
// ---------------------------------------------------------------------------
//#ifdef  L2_PBOC20
void  PBOC_SetTDOL( void )
{
UINT  i;
UINT  iLen;
UCHAR row;
UCHAR col;
UCHAR buf[64];
UCHAR result;


KEYIN_TDOL_ST:

      UI_ClearRow( 1, 7, FONT0 );
      api_emv_GetDataElement( DE_TERM, ADDR_TERM_TDOL, 2, (UCHAR *)&iLen );
      if( iLen == 0 )
        goto KEYIN_TDOL;
      api_emv_GetDataElement( DE_TERM, ADDR_TERM_TDOL+2, iLen, g_ibuf );

      row = 1;
      col = 0;
      for( i=0; i<iLen; i++ )
         {
         TL_DispHexByte( row, col, g_ibuf[i] );
         col += 3;
         if( col >=19 )
           {
           col = 0;
           row += 1;
           if( row >= 8 )
             row = 1;
           }
         }

      // 0-DELETE, 1-UPDATE ?
      memmove( buf, (UCHAR *)msg_0delete_1update, 18 );
      UI_PutStr( 7, 0, FONT0+attrCLEARWRITE, 18, buf );
      result = UI_WaitKey();
      if( result == 'x' )
        return;

      if( result == '0' )
        {
        g_temp[0] = 0;
        g_temp[1] = 0;
        api_emv_PutDataElement( DE_TERM, ADDR_TERM_TDOL, 2, g_temp );
        goto KEYIN_TDOL_ST;
        }

//    if( TL_UpdateReq( 0, 7, COL_LEFTMOST, FONT0+attrCLEARWRITE ) == TRUE ) // update?
//      {
KEYIN_TDOL:

//      UI_ClearRow( 1, 7, FONT0 );

        // keyin
        if( TL_GetAlphaNumDigits( 7, 0, FONT0, 35, g_ibuf ) == FALSE )
          return;

        if( ((g_ibuf[0] % 2) !=0) || (g_ibuf[0] <= 2) )
          goto KEYIN_TDOL;

        // update
        iLen = g_ibuf[0] / 2;
        for( i=0; i<iLen; i++ )
           g_temp[i+2] = TL_ascw2hexb( &g_ibuf[1+i*2] ); // convert to hex format

        g_temp[0] = (UCHAR)iLen;
        g_temp[1] = 0;
        api_emv_PutDataElement( DE_TERM, ADDR_TERM_TDOL, iLen+2, g_temp );

        goto KEYIN_TDOL_ST;
//      }
}
//#endif

// ---------------------------------------------------------------------------
// FUNCTION: setup transaction type.            
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
//
//             01234567890123456789
//           0 SET TRANS TYPE
//           1 GOODS (or SERVICE)
//           2                       
//           3 
//           4                     
//           7 1-GOODS    2-SERVICE
// ---------------------------------------------------------------------------
#ifdef  L2_PBOC20
void PBOC_SetTransType( void )
{
UCHAR result;
UCHAR fLoop;
UCHAR buf[25];
      
      memmove( buf, msg_1goods_2services, 20 );
      UI_PutStr( 7, 0, FONT0+attrCLEARWRITE, 20, buf );
      while(1)
           {
           // current setting: GOODS or SERVICE
           DISP_trans_type();

        // if( g_term_tx_type == 0 )
        //   {
        //   buf[0] = 5;
        //   memmove( &buf[1], msg_goods, 5 );
        //   }
        // else
        //   {
        //   buf[0] = 7;
        //   memmove( &buf[1], msg_services, 8 );
        //   }
        // UI_PutStr( 1, 0, FONT0+attrCLEARWRITE, buf[0], &buf[1] );

           fLoop = TRUE;
           while(fLoop)
                {
                result = UI_WaitKey();
                switch( result )
                      {
                      case '1': // goods
                           g_term_tx_type = TT_GOODS;
                           fLoop = FALSE;
                           break;

                      case '2': // services
                           g_term_tx_type = TT_SERVICES;
                           fLoop = FALSE;
                           break;

                      case 'x': // exit
                      case 'y': // ok
                           return;
                      }
                }
           }
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: check cardholder's license (PBOC2.0 ONLY)
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - failed
// ---------------------------------------------------------------------------
#ifdef  L2_PBOC20
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
      DISP_verify_ch_license();

      if( iLen2 != 0 ) // license type here
        {
        switch( buf2[2] )
              {
              case 00:
                   DISP_license_00();
                   break;

              case 01:
                   DISP_license_01();
                   break;

              case 02:
                   DISP_license_02();
                   break;

              case 03:
                   DISP_license_03();
                   break;

              case 04:
                   DISP_license_04();
                   break;

              default:
                   DISP_license_05();
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

      DISP_correct_or_not();
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
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Using terimal default TDOL.
//           This function is used to solve the problem such as:
//           CDOL1 or CDOL2 = 95 05, 9B 02, 98 14
//           where Tag 98 (TC Hash) is calcuated after TVR dol has been built,
//                 but the bit - using default TDOL is not set.
// INPUT   : none.
// OUTPUT  : none.
// REF     : g_ibuf
//           g_term_TVR
// RETURN  : none.
// ---------------------------------------------------------------------------
#ifdef  L2_PBOC20
void  Check_TerminalDefaultTDOL( void )
{


      apk_ReadRamDataICC( ADDR_ICC_TDOL, 254, g_ibuf );

      if( (g_ibuf[1]*256+g_ibuf[0]) == 0 ) // ICC does not contain TDOL
        {
        // (2) using TERM default TDOL (from payment system)
        //     - if a default TDOL is required but is not present
        //       in the terminal (because the payment system does not
        //       support), a default TDOL with no data objects in the
        //       list shall be assumed.
        //     - set TVR.default_TDOL_used = 1

        apk_ReadRamDataTERM( ADDR_TERM_TDOL, 254, g_ibuf );

        if( (g_ibuf[1]*256+g_ibuf[0]) != 0 ) // terminal contains default TDOL, set TVR
          {
          g_term_TVR[4] |= TVR4_DEFAULT_TDOL_USED; // set TVR bit
          api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR+4, 1, &g_term_TVR[4] );
          }
        }
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Check TC Hash (Tag 98 in CDOL1).
// INPUT   : type = DOL_CDOL1 or DOL_CDOL2
// OUTPUT  : none.
// REF     : g_ibuf
// RETURN  : TRUE  - TC Hash Value Tage is in CDOL1.
//           FALSE - not found.
// NOTE    : Call this function prior to building CDOL1 and CDOL2.
// ---------------------------------------------------------------------------
UCHAR PBOC_CheckTcHash( UCHAR type )
{
UCHAR dol_len;
UCHAR de[4];
UCHAR tag1, tag2;
UCHAR len;
UCHAR index;
UCHAR buf[2];
UCHAR cnt;
UCHAR dol[254];
UCHAR *ptrobj;

      ptrobj = &dol[0]; // pointer to CDOL

      if( type == DOL_CDOL1 )
        apk_ReadRamDataICC( ADDR_ICC_CDOL1, 254, ptrobj );
      else
        apk_ReadRamDataICC( ADDR_ICC_CDOL2, 254, ptrobj );

      ptrobj[1] = ptrobj[0]; // berLEN
      ptrobj[0] = 0x81;      //

      // format: LEN[n], T1-L1, T2-L2, T3-L3....
      //         where: LEN = total length, T1 = 1 or 2 bytes, Ln = 1 byte.

      dol_len = apk_GetBERLEN( ptrobj, &cnt ); // get total length
      if( dol_len == 0 ) // empty DOL?
        return( FALSE );

      ptrobj += cnt;

      do{
        tag1 = *ptrobj++;    // MSB of tag
        tag2 = *ptrobj++;    // LSB of tag

        // check constructed data element
        if( apk_CheckConsTag( tag1 ) == TRUE )
          {
          len = (UCHAR)apk_GetBERLEN( --ptrobj, &cnt );
          ptrobj += cnt;

          dol_len -= (1+cnt);
          continue;
          }

        if( apk_CheckWordTag( tag1 ) == TRUE ) // double-byte tag?
          {
          len = (UCHAR)apk_GetBERLEN( ptrobj, &cnt );
          ptrobj += cnt;

          dol_len -= (2+cnt);
          }
        else // single-byte tag
          {
          len = (UCHAR)apk_GetBERLEN( --ptrobj, &cnt );
          ptrobj += cnt;

          tag2 = tag1;
          tag1 = 0;

          dol_len -= (1+cnt);

//        if( apk_CheckTermTag( tag2 ) == FALSE )
//          {
//          if( apk_CheckIccTag( tag2 ) == FALSE )
//            {
//            if( apk_CheckIssuerTag( tag2 ) == FALSE )
//              // a single-byte tag, but its source is neither TERMINAL, ICC or ISSUER
//              continue;
//            }
//          }
          }

        // ------ tag(s) source is "TERMINAL" ? ------
        index = Scan_TDE( tag1, tag2, &de[0] );

        if( index != 255 )
          {
          // Check specical tags for:
          // (1) 9A  : Transaction Date(n6)
          // (2) 9F21: Transaction Time(n6)
          // (3) 98  : TC Hash Value(b20)
          // (4) 9F37: Unpredictable Number(b4)
          if( (tag1 == 0x00) && (tag2 == 0x98) )
            {
            Check_TerminalDefaultTDOL();
            return( TRUE );
            }
          }

        } while( dol_len > 0 );

      return( FALSE );
}


// ---------------------------------------------------------------------------
//  TEST AREA
// ---------------------------------------------------------------------------

//// ---------------------------------------------------------------------------
//UCHAR apk_ParseLenFCI2( UCHAR *ptrfci, UCHAR *padlen )
//{
//UCHAR i;
//UCHAR c1, c2, c3;
//UINT  iL1, iL2, iL3, iL4;
//UINT  lenfci;
//UCHAR padding;
//UCHAR *ptrobj;
//UCHAR *ptrrec;
//
//      ptrrec = ptrfci;
//      lenfci = ptrfci[1]*256 + ptrfci[0];
//      ptrfci += 2; // pointer to T-L-V
//
//      iL1 = apk_GetBERLEN( ptrfci+1, &c1 );       // FCI Template
//
//      ptrobj = apk_FindTag( 0x84, 0x00, ptrfci ); // DF Name
//      iL2 = apk_GetBERLEN( ptrobj, &c2 );
//
//      // check if "A5" is just following "84"
//      iL4 = 0; // length of unexpected data objects btw A5 & 84
//      if( *(ptrobj + iL2 + c2) != 0xA5 )
//        {
//        while( *(ptrobj + iL2 + c2) != 0xA5 )
//             {
//             iL4++;
//             ptrobj++;
//             }
//        }
//
//      ptrobj = apk_FindTag( 0xA5, 0x00, ptrfci ); // FCI Prorietary Template
//      iL3 = apk_GetBERLEN( ptrobj, &c3 );
//
//      // (1) lenfci - 2 = L1 + 1 + C1 or
//      // (2) ISO rigth padding (0x00 or 0xFF)
//
//      if( apk_CheckIsoPadding_Right( ptrrec, padlen ) == FALSE )
//        return( FALSE );
//
//      // L1 - (1 + L2 + C2) = L3 + 1 +C3
//
//      if( (iL1 - (1 + iL2 + c2) - iL4) != (iL3 + 1 + c3) )
//        return( FALSE );
//
//      return( TRUE );
//}
//
//// ---------------------------------------------------------------------------
//UCHAR apk_ParseTLV2( UCHAR reclen[], UCHAR *ptrobj, UCHAR padlen )
//{
//UCHAR *ptrnext;
//UCHAR *ptrdo;
//UCHAR tlv[261];
//UCHAR len[2];
//UINT  iLen;
//
//      len[0] = reclen[0];
//      len[1] = reclen[1];
//      ptrdo = ptrobj;
//
//      while(1)
//           {
//           // read one data element
//           ptrnext = apk_GetBERTLV2( &len[0], ptrdo, tlv, padlen );
//           if( ptrnext == (UCHAR *)-1 )
//             return( FALSE );
//
//           if( ptrnext != 0 )
//             {
//             ptrdo = ptrnext; // update next data object
//
//             iLen = len[1]*256 + len[0]; // remaining length
//             if( (iLen == 1) || (iLen == 2) )
//               return( FALSE );
//             }
//           else
//             return( TRUE );
//           }
//}
//
//// ---------------------------------------------------------------------------
//// FUNCTION: send one issuer script command to ICC.
//// INPUT   : len      - length of the issuer script command.
////           cmd      - the issuer script command.
//// OUTPUT  : response - 2L-V, the status word (SW1 SW2).
//// RETURN  : apiOK
////           apiFailed
//// -------------------------------------------------------------------------
//UCHAR apdu_ISSUER_SCRIPT_COMMAND2( UINT len, UCHAR *cmd, UCHAR *response )
//{
//UINT  i;
//UCHAR c_apdu[254+8];
//
//
//    TL_DispHexWord(0,0,len);
//    TL_DumpHexData(0,1,len, cmd);
//
//    if( cmd[0] == 0x8c )
//      {
//      response[0] = 2;
//      response[1] = 0;
//      response[2] = 0x6d;
//      response[3] = 0x00;
//      }
//    else
//      {
//      response[0] = 2;
//      response[1] = 0;
//      response[2] = 0x90;
//      response[3] = 0x00;
//      }
//      return( apiOK );
//
//      // ------------------
//
////    c_apdu[0] = len & 0x00ff;         // length of APDU
////    c_apdu[1] = (len & 0xff00) >> 8;  //
////
////    for(i=0; i<len; i++)                  // script command APDU
////       c_apdu[2+i] = *cmd++;              //
////
////    return( api_ifm_exchangeAPDU( g_dhn_icc, c_apdu, response ) );
//}
//
//
// ---------------------------------------------------------------------------
// FUNCTION: process the issuer script commands with Template Tag=0x71.
//           (processed before issuing the final GENERATE AC command)
//           (1) read one template head [7x-L-(9F18-L-V)]
//           (2) read one script command [86-Ln-Vn]
//           (3) loop (2)
//           (4) loop (1)
// INPUT   : ist - 2L-[T-L-V], issuer scripts template.
//                 T-L-(9F18-04-ScriptID[4])-[Commands], T=71 or 72.
//                 Commands: [86-L1-CMD1][86-L2-CMD2][86-L3-CMD3]...
//                 ScriptID: optional, also "9F18-04".
//                 The "ist" is located at larger ADDR_ISU_SCRIPT_TEMP[] pool
//                 instead of at limited global memory.
//                 format: 2L-DATA[n]
//           tag - the script tag to be process. (0x71 or 0x72)
// OUTPUT  : none.
// REF     : g_ibuf
// RETURN  : apkOK           (SW1SW2 = 90xx, 62xx, 63xx)
//           apkFailed       (SW1SW2 = 69xx, 6Axx)
//           apkOutOfService (not supported or device error)
//           apkNotReady     (not performed, eg. tag=71 but isc=72...)
//
// NOTE    : terminal shall maintain the "Issuer Script Results" (ADDR_TERM_ISR)
//           BYTE 1: bit5-8
//                        0=script not performed
//                        1=script processing failed
//                        2=script processing successful
//                   bit1-4
//                        0=not specified (OK)
//                        1 ~ E = sequence number       (error)
//                        F     = sequence number >= 15 (error)
//           BYTE 2-5: Script ID. (filled in 0's if not available).
// ---------------------------------------------------------------------------
//UCHAR apk_IssuerScriptProcessing3( UCHAR tag, UINT addr_ist )
//{
//UCHAR ist[300];
//UINT  i;
//int   iTempLen;
//int   iCmdLen;
//UINT  iLen;
//UINT  iAddr;
//UCHAR *ptrist;
//UCHAR cnt;
//UCHAR tag2;
//UCHAR script_id[4];
//UCHAR script_sn;
//UCHAR script_result;
//UCHAR flag_exe;
//UCHAR flag_err_format;
//UCHAR flag_ever;
//
//      flag_exe = FALSE;  // reset scripts execution flag
//      flag_ever = FALSE; // reset scripts ever executed flag
//      flag_err_format = FALSE; // script format
//
////    iAddr = ADDR_ISU_SCRIPT_TEMP; // set start address
//      iAddr = addr_ist; // set start address
//
//      apk_ReadRamDataTERM( iAddr, 2, (UCHAR *)&iTempLen ); // total template size
//      if( iTempLen < 8 )
//        return( apkFailed );
//
//      iAddr += 2;
////    apk_ReadRamDataTERM( iAddr, 16, ist ); // read template header
////    ptrist = ist; // pointer to the 1'st [T]
//
////  TL_DispHexWord(0,0,iTempLen);
////  UI_WaitKey();
//
//ISC_ST:
//
//      while( iTempLen > 0 ) // scripts
//           {
//           memset( script_id, 0x00, 4 );
//           script_sn = 0;
//           script_result = 0;
//
//           apk_ReadRamDataTERM( iAddr, 265, ist ); // read template header
//           ptrist = ist; // pointer to the 1'st [T]
//
//           if( iTempLen > 128 ) // PATCH: PBOC2.0, 2006-02-16, 2CO03402
//             {
//             g_isr_len_overrun = *ptrist;
//             apk_UpdateISR( script_id, script_sn, script_result ); // set default ISR = 00 00 00 00 00
//             return( apkFailed );
//             }
//
//           // scan the template Tag 71 or 72
//           if( (*ptrist != 0x71) && (*ptrist != 0x72) )
//             {
//             while( iTempLen > 0 )
//                  {
//                  if( (*ptrist != 0x71) && (*ptrist != 0x72) )
//                    {
//                    ptrist++;
//                    iTempLen--;
//                    iAddr++;
//                    }
//                  else
//                    {
//                    if( iTempLen < 8 )
//                      return( apkFailed );
//                    else
//                      goto ISC_START;
//                    }
//                  }
//             return( apkFailed );
//             }
//
//ISC_START:
//
////  TL_DispHexByte(0,0,*ptrist);
////  TL_DispHexByte(1,0,tag);
////  UI_WaitKey();
//
//           if( *ptrist == tag )
//             {
//             iLen = apk_GetBERLEN( ++ptrist, (UCHAR *)&cnt ); // 71-L
//             ptrist += cnt; // pointer to next [T] (9F18 or 86)
//
////      TL_DispHexWord(0,0,iTempLen);
////      TL_DispHexWord(1,0,iLen);
////      TL_DumpHexData(0,2,16, ptrist);
////      TL_DumpHexData(0,5,16, &ist[iLen+2]);
//
//             if( iLen >= iTempLen ) // PATCH: PBOC2.0, 2006-02-16, 2CO035b (bad length)
//               {
////      TL_DispHexWord(2,0,tag);
//
//               apk_UpdateISR( script_id, script_sn, script_result ); // set default ISR = 00 00 00 00 00
//               return( apkFailed );
//               }
//
//             if( iTempLen > (iLen + 2) )
//               {
//               if( (ist[iLen+2] != 0x71) && (ist[iLen+2] != 0x72) )
//                 {
//                 // 7x length error, skip this template and find next 7x
//                 if( (*ptrist == 0x9F) && (*(ptrist+1) == 0x18) )
//                   {
//                   if( *(ptrist + 2) == 4 )
//                     memmove( script_id, ptrist+3, 4 );
//                   }
//
//                 flag_exe = TRUE;
//                 flag_ever = TRUE;
//                 flag_err_format = TRUE;
//
//                 iAddr += 2;
//                 iTempLen -= 2;
//                 while( iTempLen > 0 )
//                      {
//                      if( (*ptrist != 0x71) && (*ptrist != 0x72) )
//                        {
//                        ptrist++;
//                        iAddr++;
//                        iTempLen--;
//                        }
//                      else // found
//                        goto ISC_2000;
//                      }
//
//                 // PATCH: PBOC2.0, 2006-02-15, 2CO.035.00c-1
//                 apk_ReadRamDataTERM( ADDR_TERM_ISR, 1, (UCHAR *)&cnt ); // get current counter
//                 if( cnt == 0 )
//                   {
//                   apk_UpdateISR( script_id, script_sn, script_result ); // set default ISR = 00 00 00 00 00
////     TL_DispHexByte(3,3,0x88);
////     UI_WaitKey();
//                // return( apkNotReady );
//                   }
//                 return( apkFailed );
//                 }
//               }
//
//             // ScriptID present?
//             if( (*ptrist == 0x9F) && (*(ptrist+1) == 0x18) )
//               {
//               if( *(ptrist+2) == 0x00 ) // PATCH: 2003-09-16, seems "no ID"
//                 {
//                 ptrist += 3; // skip "9F 18 00"
//                 iCmdLen = iLen - 3;
//                 iAddr += (cnt + 1 + 3); // pointer to the 1'st [T=86]
//
//                 goto ISC_1000;
//                 }
//
//               if( *(ptrist+2) != 0x04 )
//                 return( apkFailed ); // illegal ScriptID size
//
//               memmove( script_id, ptrist+3, 4 );
//               script_sn = 0;
//
//               ptrist += 7; // yes, skip "ScriptID" and go to next [Tn]
//               iCmdLen = iLen - 7;
//               iAddr += (cnt + 1 + 7); // pointer to the 1'st [T=86]
//
////  UI_ClearScreen();
////  TL_DispHexByte(0,0,0x66);
////  TL_DumpHexData(0,1,14,ptrist);
//               }
//             else
//               {
//               // PATCH: PBOC2.0, 2006-02-17, 2CO036, neither 9F18 nor 86 then ignore current script
//               if( (*ptrist == 0x9F) && (*(ptrist+2) == 0x04) && (*ptrist != 0x86) )
//                 {
////         UI_ClearScreen();
////         TL_DispHexByte(0,0,0x55);
//                 iTempLen -= (iLen + 1 + cnt);
////         TL_DispHexWord(1,0,iTempLen);
////         UI_WaitKey();
//                 iAddr += (iLen + 1 + cnt);
//
////         apk_ReadRamDataTERM( iAddr, 265, ist ); // read template header
////         TL_DumpHexData(0,0,14,ist);
//                 goto ISC_ST; // parse next one
//                 }
//
//               iCmdLen = iLen;
//               iAddr += (cnt + 1);     // pointer to the 1'st [T=86]
//               }
//ISC_1000:
//             iTempLen -= (iLen + cnt + 1); // remaining template data size
//
////  UI_ClearScreen();
////  TL_DispHexByte(0,0,0x77);
////  TL_DispHexWord(1,0,iTempLen);
////  TL_DispHexWord(2,0,iCmdLen);
////  apk_ReadRamDataTERM( iAddr, 265, ist ); // read one script command
////  TL_DumpHexData(0,3,14, ist);
//
//             // script commands processing: send commands to ICC in sequence
//             while( iCmdLen > 0 )
//                  {
//                  apk_ReadRamDataTERM( iAddr, 265, ist ); // read one script command
//                  ptrist = ist;
//
//                  if( *ptrist == 0x86 )
//                    {
//                    iLen = apk_GetBERLEN( ++ptrist, (UCHAR *)&cnt );
//                    ptrist += cnt; // pointer to next cmd [Vn]
//
//                    script_sn++;
//                    if( script_sn >= 15 )
//                      script_sn = 15;
//
//                    flag_exe = TRUE;
//                    flag_ever = TRUE;
//                    if( apdu_ISSUER_SCRIPT_COMMAND( iLen, ptrist, g_ibuf ) == apiOK )
//                      {
//                      ptrist += iLen;
//
//                      i = g_ibuf[1]*256 + g_ibuf[0]; // size of response APDU
//                      if( i >= 2 )
//                        {
//                        // examine only SW1
//                        switch( g_ibuf[i] )
//                              {
//                              case 0x90:
//                              case 0x62:
//                              case 0x63:
//
//                                   script_result = 2; // successful
//                                   break;
//
//                              case 0x6d: // PATCH: PBOC2.0, 2006-02-15, 2CO.034B
//
//                                   script_result = 1; // failed
//                                   break;
//
//                              default: // 0x69, 0x6a, or others
//
//                                   script_result = 1; // failed
//                                   apk_UpdateISR( script_id, script_sn, script_result );
//
//                                   return( apkFailed );
//                              }
//                        }
//                      else
//                        {
//                        script_result = 1; // failed
//                        apk_UpdateISR( script_id, script_sn, script_result );
//
//                        return( apkFailed );
//                        }
//
//                      iCmdLen -= (iLen + cnt + 1); // 86 Ln Vn
//                      iAddr += (iLen + cnt + 1); // pointer to next [86-Ln-Vn]
//                      }
//                    else
//                      return( apkOutOfService ); // error
//                    }
//                  else // tag 86 is missing (=not performed)
//                    {
//                    script_result = 0; // not specified, PATCH: 2005/05/06
//                    flag_exe = TRUE;
//                    flag_ever = TRUE;       // ever try to execute
//                    flag_err_format = TRUE; // invalid script command tag
//
//                    iLen = apk_GetBERLEN( ++ptrist, (UCHAR *)&cnt );
//                    if( iCmdLen >= (iLen + cnt + 1) )   // PATCH: 2005/05/06
//                      {
//                      iCmdLen -= (iLen + cnt + 1); // 86 Ln Vn
//                      iAddr += (iLen + cnt + 1); // pointer to next [86-Ln-Vn]
//                      }
//                    else
//                      {                      // PATCH: 2005/05/18
//                      if( flag_exe == TRUE ) // 2CJ.194.00, 2CJ.195.00
//                        {
//                        flag_exe = FALSE;
//                        apk_UpdateISR( script_id, script_sn, script_result );
//                        }
//
//                      goto ISC_EXIT; // terminate
//                      }
//                    }
//
//                  } // while( iCmdLen > 0 ) for next command (86)
//ISC_2000:
//             if( flag_exe == TRUE ) // PATCH: 2005/05/06
//               {
//               flag_exe = FALSE;
//               apk_UpdateISR( script_id, script_sn, script_result );
//               }
//             }
//           else
//             {
//
////  TL_DispHexByte(0,0,0x99);
////  TL_DispHexByte(1,0,tag);
//
//             if( tag == 0x71 )
//               tag2 = 0x72;
//             else
//               tag2 = 0x71;
//
////  TL_DispHexByte(2,0,tag2);
////  TL_DispHexByte(3,0,*ptrist);
////  UI_WaitKey();
//
//             if( *ptrist == tag2 )
//               {
//               iLen = apk_GetBERLEN( ++ptrist, (UCHAR *)&cnt );
//
////  TL_DispHexWord(4,0,iLen);
////  TL_DispHexByte(5,0,cnt);
//
//               if( iLen == 0 ) // PATCH: PBOC2.0, 2006-02-17, 2CO035c-1
//                 return( apkNotReady );
//
//               iAddr += (iLen + cnt + 1);
//               apk_ReadRamDataTERM( iAddr, 16, ist ); // read template header
//               ptrist = ist; // pointer to next [T=71 or 72]
//
////  TL_DumpHexData(0,6,7, ptrist);
////  UI_ClearScreen();
//
//               iTempLen -= (iLen + cnt + 1);
//
////  TL_DispHexWord(0,0,iTempLen);
////  UI_WaitKey();
//               }
//             else
//               return( apkFailed ); // invalid script template tag
//             }
//
//           } // while( iTempLen > 0 ) for next script (71 or 72)
//
////    if( flag_exe == TRUE )
////      apk_UpdateISR( script_id, script_sn, script_result );
//
//ISC_EXIT:
//
//      if( flag_err_format == TRUE )
//        return( apkFailed );
//      else
//        {
//        if( flag_ever == TRUE )
//          return( apkOK ); // done
//        else
//          return( apkNotReady ); // PATCH: 2003-08-03
//                                 // script is not performed
//        }
//}
//// ---------------------------------------------------------------------------
//void TC_SelectADF_01( void )
//{
//UCHAR padlen[1];
//UINT  iLen;
//UCHAR rec_len[2];
//UCHAR *ptrobj;
//UCHAR cnt;
//UCHAR fci[] = {
//
//       60, 0x00,
//       0x6F, 56, 0x84, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x03, 0x10, 0x10, 0xA5, 45, 0x87, 0x01,
//       0x01, 0x9F, 0x38, 0x12, 0x9F, 0x1A, 0x02, 0x9F, 0x33, 0x03, 0x9F, 0x40, 0x05, 0x9F, 0x1B, 0x04,
//       0x9F, 0x09, 0x02, 0x9F, 0x35, 0x01, 0x5F, 0x2D, 0x08, 0x65, 0x73, 0x65, 0x6E, 0x66, 0x72, 0x64,
//       0x65, 0x9F, 0x11, 0x01, 0x01, 0xDF, 0x01, 0x03, 0x00, 0x00, 0x00, 0x90, 0x00
//       };
//
//
//       UI_OpenKeyAll();
//       UI_ClearScreen();
//
//       TL_DispHexByte(0,0,0x11);
//
//       ptrobj = apk_FindTag( 0xA5, 0x00, &fci[2] );
//       iLen = apk_GetBERLEN( ptrobj, &cnt );
//       rec_len[0] = iLen & 0x00ff;
//       rec_len[1] = (iLen & 0xff00) >> 8;
//       ptrobj += cnt; // ptr to the 1'st DO of FCI Proprietary Template
//
//       TL_DispHexByte(1,0,apk_ParseLenFCI2( fci, padlen ));
//
//       TL_DispHexByte(2,0,apk_ParseTLV2( &rec_len[0], ptrobj, padlen[0] ));
//       for(;;);
//
//}
//
// ---------------------------------------------------------------------------
//void TC_IssuerScript( void )
//{
//UINT  iAddr;
//UCHAR cnt;
//UCHAR isc[] =
//      {
////    2CO.034.00
////    131 , 0x00,
////    0x71, 0x62, 0x9f, 0x18, 0x04, 0x11, 0x22, 0x33, 0x44,
////    0x86, 0x0d, 0x84, 0x16, 0x00, 0x00, 0x08, 0xaa, 0xbb, 0xcc, 0xdd, 0x11, 0x22, 0x33, 0x44,
////    0x86, 0x0d, 0x84, 0x24, 0x00, 0x00, 0x08, 0xaa, 0x11, 0x22, 0x33, 0x44, 0xbb, 0xcc, 0xdd,
////    0x86, 0x0d, 0x84, 0x16, 0x00, 0x00, 0x08, 0xaa, 0xbb, 0x11, 0x22, 0xcc, 0xdd, 0x33, 0x44,
////    0x86, 0x0d, 0x84, 0x16, 0x00, 0x00, 0x08, 0xaa, 0xbb, 0xcc, 0xdd, 0x11, 0x22, 0x33, 0x44,
////    0x86, 0x0d, 0x84, 0x24, 0x00, 0x00, 0x08, 0xaa, 0x11, 0x22, 0x33, 0x44, 0xbb, 0xcc, 0xdd,
////    0x86, 0x0e, 0x8c, 0x16, 0x00, 0x00, 0x09, 0x8e, 0x07, 0xaa, 0xbb, 0x11, 0x22, 0xcc, 0xdd, 0x33,
////    0x71, 0x1d, 0x9f, 0x18, 0x04, 0x55, 0x66, 0x77, 0x88, 0x86, 0x09, 0x84, 0x24, 0x00, 0x00, 0x04, 0xaa, 0xbb, 0xcc, 0xdd,
////    0x86, 0x09, 0x84, 0x24, 0x00, 0x00, 0x04, 0xaa, 0xbb, 0xcc, 0xdd };
//
////    2CO.035.00b
////    20, 0,
////    0x72, 0x20, 0x9f, 0x18, 0x04, 0x11, 0x22, 0x33, 0x44,
////    0x86, 0x09, 0x84, 0x16, 0x00, 0x00, 0x04, 0xaa, 0xbb, 0xcc, 0xdd };
//
////    2CO.035.00c-1
////    12, 0,
////    0x72, 0x86, 0x09, 0x84, 0x16, 0x00, 0x00, 0x04, 0xaa, 0xbb, 0xcc, 0xdd };
//
////    2CO.035.00c-2
////    14, 0,
////    0x72, 0x0C, 0x72, 0x86, 0x09, 0x84, 0x16, 0x00, 0x00, 0x04, 0xaa, 0xbb, 0xcc, 0xdd };
//
////    2CO.036.00
//      40, 0,
//      0x72, 0x12, 0x9f, 0x20, 0x04, 0x11, 0x22, 0x33, 0x44,
//      0x86, 0x09, 0x84, 0x18, 0x00, 0x00, 0x04, 0xaa, 0xbb, 0xcc, 0xdd,
//      0x72, 0x12, 0x9f, 0x18, 0x04, 0x44, 0x33, 0x22, 0x11,
//      0x86, 0x09, 0x84, 0x18, 0x00, 0x00, 0x04, 0xaa, 0xbb, 0xcc, 0xdd };
//
//
//      UI_OpenKeyAll();
//      UI_ClearScreen();
//
//      apk_ClearRamDataTERM( ADDR_TERM_ISR, 1, 0x00 );
//      apk_ClearRamDataTERM( ADDR_ISU_SCRIPT_TEMP, 2, 0x00 );
//
//      apk_WriteRamDataTERM( ADDR_ISU_SCRIPT_TEMP, 42, isc );
//
////    iAddr = ADDR_ISU_SCRIPT_TEMP;
////    TL_DispHexByte(0,18,apk_IssuerScriptProcessing3( 0x71, iAddr ));
//      iAddr = ADDR_ISU_SCRIPT_TEMP;
//      TL_DispHexByte(1,18,apk_IssuerScriptProcessing3( 0x72, iAddr ));
//
//      apk_ReadRamDataTERM( ADDR_TERM_ISR, 1, (UCHAR *)&cnt ); // get current counter
//      apk_ReadRamDataTERM( ADDR_TERM_ISR, 1+cnt*5, g_obuf );
//      TL_DumpHexData(0,4,1+cnt*5, g_obuf);
//      for(;;);
//}
//
//// ---------------------------------------------------------------------------
//void TC_TcHash( void )
//{
//UCHAR tag1, tag2;
//
////     UI_OpenKeyAll();
////     UI_ClearScreen();
//
//
//
//ST:
//    g_ibuf[0] = 0x11;
//    g_ibuf[1] = 0x22;
//    g_ibuf[2] = 0x33;
//    g_ibuf[3] = 0x44;
//    g_ibuf[4] = 0x55;
//    g_ibuf[5] = 0x66;
//    apk_WriteRamDataTERM( ADDR_TERM_AMT_AUTH_N, 6, g_ibuf );
//    g_ibuf[0] = 0x77;
//    g_ibuf[1] = 0x88;
//    g_ibuf[2] = 0x99;
//    g_ibuf[3] = 0x00;
//    g_ibuf[4] = 0x11;
//    g_ibuf[5] = 0x22;
//    apk_WriteRamDataTERM( ADDR_TERM_AMT_OTHER_N, 6, g_ibuf );
//
//    g_ibuf[0] = 6;
//    g_ibuf[1] = 0;
//    g_ibuf[2] = 0x9f;
//    g_ibuf[3] = 0x02;
//    g_ibuf[4] = 0x06;
//    g_ibuf[5] = 0x9f;
//    g_ibuf[6] = 0x03;
//    g_ibuf[7] = 0x06;
//    apk_WriteRamDataICC( ADDR_ICC_TDOL, 8, g_ibuf );
//
//    tag1 = 0x00;
//    tag2 = 0x98;
//    Build_Special_DOL( tag1, tag2 );
//
//    memset( g_temp, 0x00, 20 );
//    apk_ReadRamDataTERM( ADDR_TERM_TC_SHA1, 20, g_temp );
//    TL_DumpHexData( 0, 4, 20, g_temp );
//    UI_WaitKey();
//    UI_ClearScreen();
//    goto ST;
//    for(;;);
//}
//
//// ---------------------------------------------------------------------------
//
//// ---------------------------------------------------------------------------
//// FUNCTION: The READ RECORD command reads a file record in alinear file.
//// INPUT   : sfi    - short file id. (1..30)
////           recnum - record number.
//// OUTPUT  : recdata - 2L-V, the record data read.
//// REF     : g_dhn_icc
//// RETURN  : apiOK
////           apiFailed
//// ---------------------------------------------------------------------------
//UCHAR apdu_READ_RECORD2( UCHAR sfi, UCHAR recnum, UCHAR *recdata )
//{
//
//UCHAR rec01[] = {
//      46, 0,
//      0x70, 0x2A, 0x61, 0x28, 0x4F, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x03, 0x10, 0x10, 0x50, 0x0A,
//      0x56, 0x49, 0x53, 0x41, 0x43, 0x52, 0x45, 0x44, 0x49, 0x54, 0x87, 0x01, 0x01, 0x9F, 0x12, 0x0D,
//      0x43, 0x52, 0x45, 0x44, 0x49, 0x54, 0x4F, 0x44, 0x45, 0x56, 0x49, 0x53, 0x41, 0x90, 0x00 };
//
//UCHAR rec02[] = {
//      2, 0,
//      0x6a, 0x83 };
//
//UCHAR rec03[] = {
//      66, 0,
//      0x70, 0x3E, 0x5F, 0x20, 0x0F, 0x46, 0x55, 0x4C, 0x4C, 0x20, 0x46, 0x55, 0x4E, 0x43, 0x54,
//      0x49, 0x4F, 0x4E, 0x41, 0x4C, 0x57, 0x11, 0x47, 0x61, 0x73, 0x90, 0x01, 0x01, 0x00, 0x10, 0xD1,
//      0x01, 0x22, 0x01, 0x01, 0x23, 0x45, 0x67, 0x89, 0x9F, 0x1F, 0x16, 0x30, 0x31, 0x30, 0x32, 0x30,
//      0x33, 0x30, 0x34, 0x30, 0x35, 0x30, 0x36, 0x30, 0x37, 0x30, 0x38, 0x30, 0x39, 0x30, 0x41, 0x30,
//      0x42, 0x90, 0x00 };
//
//UCHAR rec04[] = {
//      18, 0,
//      0x70, 0x0E, 0x5A, 0x08, 0x47, 0x61, 0x73, 0x90, 0x01, 0x01, 0x00, 0x10, 0x5F, 0x34, 0x01,
//      0x01, 0x90, 0x00 };
//
//UCHAR rec05[] = {
//      71, 0,
//      0x70, 0x43, 0x8C, 0x06, 0x98, 0x14, 0x95, 0x05, 0x9B, 0x02, 0x8D, 0x19, 0x95, 0x05, 0x9B,
//      0x02, 0x8A, 0x02, 0x9F, 0x02, 0x06, 0x9F, 0x03, 0x06, 0x9F, 0x1A, 0x02, 0x5F, 0x2A, 0x02, 0x9A,
//      0x03, 0x9C, 0x01, 0x9F, 0x37, 0x04, 0x97, 0x06, 0x9F, 0x02, 0x06, 0x9F, 0x03, 0x06, 0x9F, 0x0E,
//      0x05, 0x00, 0x50, 0x88, 0x00, 0x00, 0x9F, 0x0F, 0x05, 0xF0, 0x20, 0x04, 0x98, 0x00, 0x9F, 0x0D,
//      0x05, 0xF0, 0x20, 0x04, 0x00, 0x00, 0x90, 0x00 };
//
//UCHAR rec06[] = {
//      61, 0,
//      0x70, 0x39, 0x5F, 0x25, 0x03, 0x95, 0x07, 0x01, 0x5F, 0x24, 0x03, 0x10, 0x12, 0x31, 0x5F,
//      0x28, 0x02, 0x08, 0x40, 0x9F, 0x07, 0x02, 0xFF, 0xC0, 0x8E, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00,
//      0x00, 0x00, 0x00, 0x41, 0x03, 0x42, 0x03, 0x5E, 0x03, 0x43, 0x03, 0x1F, 0x00, 0x9F, 0x08, 0x02,
//      0x00, 0x8C, 0x5F, 0x30, 0x02, 0x02, 0x01, 0x9F, 0x42, 0x02, 0x08, 0x40, 0x90, 0x00  };
//
//UCHAR rec07[] = {
//      105, 0,
//      0x70, 0x65, 0x8F, 0x01, 0x97, 0x90, 0x60, 0x24, 0x0E, 0x0E, 0xA6, 0xD2, 0x1E, 0x65, 0x52,
//      0xB2, 0xED, 0x3F, 0xAD, 0xC2, 0xF1, 0xD2, 0x80, 0xD1, 0xAD, 0x91, 0x3E, 0x62, 0x2E, 0x2C, 0x35,
//      0x21, 0xAA, 0xDF, 0x2A, 0x47, 0xB3, 0xAC, 0xF6, 0x6B, 0x67, 0x1D, 0x4B, 0x12, 0x36, 0x81, 0x9A,
//      0xD1, 0xB1, 0xFA, 0x9F, 0xA6, 0xAC, 0xDE, 0x38, 0x66, 0x5B, 0x6B, 0xDE, 0x53, 0xC3, 0x80, 0xA1,
//      0x53, 0x16, 0x9A, 0xBA, 0xAB, 0x94, 0x83, 0x90, 0x2F, 0xB7, 0x63, 0xE9, 0xEA, 0xA7, 0xAB, 0x27,
//      0x8A, 0x5D, 0x39, 0xD3, 0xA5, 0x0E, 0x15, 0x98, 0xB8, 0x4C, 0x22, 0x13, 0x9D, 0x43, 0xA7, 0x48,
//      0x6F, 0x71, 0xAA, 0x0E, 0xC3, 0x90, 0x2D, 0x26, 0x90, 0x00 };
//
//UCHAR rec08[] = {
//      30, 0,
//      0x70, 0x1A, 0x9F, 0x32, 0x01, 0x03, 0x92, 0x14, 0xCF, 0xB8, 0xD4, 0x88, 0x5D, 0x96, 0x09,
//      0x67, 0x17, 0x9F, 0x98, 0x2D, 0x42, 0xCE, 0x54, 0xEC, 0xC2, 0x05, 0x46, 0x83, 0x90, 0x00 };
//
//UCHAR rec09[] = {
//      86, 0,
//      0x70, 0x52, 0x93, 0x50, 0x11, 0x0B, 0xB9, 0xDF, 0x2D, 0x21, 0x98, 0x19, 0x06, 0xB2, 0x9A,
//      0x30, 0x14, 0x11, 0xF9, 0xFA, 0x60, 0xCF, 0x49, 0x4D, 0xBA, 0xBA, 0xBF, 0x54, 0xB1, 0x79, 0x7C,
//      0x9C, 0x4B, 0x5D, 0x99, 0xB5, 0xE6, 0x7A, 0xB7, 0x30, 0x49, 0xE7, 0x71, 0xFC, 0x5F, 0xDC, 0x23,
//      0xE5, 0x83, 0x50, 0xB7, 0x81, 0x00, 0x53, 0x24, 0xD3, 0x1D, 0xC8, 0x7A, 0xD0, 0xFB, 0xF6, 0x36,
//      0x73, 0x38, 0x08, 0x05, 0x6D, 0x66, 0x07, 0x46, 0x32, 0x71, 0x1E, 0x7C, 0xBF, 0x14, 0x07, 0x37,
//      0x96, 0xE1, 0xB6, 0x0D, 0x4D, 0x90, 0x00 };
//
//      switch( g_test_flag )
//            {
//            case 0:
//                 memmove( recdata, rec01, sizeof(rec01) );
//                 break;
//
//            case 1:
//                 memmove( recdata, rec02, sizeof(rec02) );
//                 break;
//
//            case 2:
//                 memmove( recdata, rec03, sizeof(rec03) );
//                 break;
//
//            case 3:
//                 memmove( recdata, rec04, sizeof(rec04) );
//                 break;
//
//            case 4:
//                 memmove( recdata, rec05, sizeof(rec05) );
//                 break;
//
//            case 5:
//                 memmove( recdata, rec06, sizeof(rec06) );
//                 break;
//
//            case 6:
//                 memmove( recdata, rec07, sizeof(rec07) );
//                 break;
//
//            case 7:
//                 memmove( recdata, rec08, sizeof(rec08) );
//                 break;
//
//            case 8:
//                 memmove( recdata, rec09, sizeof(rec09) );
//                 break;
//
//            }
//      g_test_flag++;
//      return( apiOK );
//
//
////    c_apdu[0] = 0x05;                 // length of APDU
////    c_apdu[1] = 0x00;                 //
////    c_apdu[2] = 0x00;                     // CLA
////    c_apdu[3] = 0xb2;                     // INS
////    c_apdu[4] = recnum;                   // P1
////    c_apdu[5] = (sfi << 3) | 0x04;        // P2 = reference control parameter
////    c_apdu[6] = 0x00;                     // Le
////
////    return( api_ifm_exchangeAPDU( g_dhn_icc, c_apdu, recdata ) );
//}
//
//// ---------------------------------------------------------------------------
//// FUNCTION: Terminal begins with an explicit selection of the
////           Payment System Environment.
//// INPUT   : none.
//// OUTPUT  : fci - 2L-V type, the file control information.
//// RETURN  : apkOK               (9000, 61La)
////           apkFileInvalidated  (6283) -- application blocked
////           apkFuncNotSupported (6A81) -- card blocked
////           apkFileNotFound     (6A82)
////           apkUnknown          (any other values) -- EMV2000
////           apkFailed
////           apkIncorrect        (mandatory data objects missing or not parse correctly
//// ---------------------------------------------------------------------------
//UCHAR apk_SelectPSE2( UCHAR *fci )
//{
//UCHAR buf[] = {
//      40, 0,
//      0x6F, 0x24, 0x84, 0x0E, 0x31, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44,
//      0x46, 0x30, 0x31, 0xA5, 0x12, 0x88, 0x01, 0x01, 0x5F, 0x2D, 0x08, 0x65, 0x73, 0x65, 0x6E, 0x66,
//      0x72, 0x64, 0x65, 0x9F, 0x11, 0x01, 0x01, 0x90, 0x00 };
//
//      memmove( fci, buf, sizeof(buf) );
//      return( apiOK );
//
//}
//
// ---------------------------------------------------------------------------
// FUNCTION: parse the length of FCI. (constructed data structure)
// INPUT   : lenfic - total length of the record to be parsed. (including SW1 SW2)
//           ptrfci - pointer to the current FCI template (Tag = 6F).
//                    6F-L1-84-L2...A5-L3-...
//                    (1) lenfci - 2 = L1 + 1 + C1
//                    (2) L1 - (1 + L2 + C2) = L3 + 1 +C3
// OUTPUT  : padlen - length of padding data objects.
// RETURN  : TRUE
//           FALSE - invalid data structure.
// ---------------------------------------------------------------------------
//UCHAR apk_ParseLenFCI2( UCHAR *ptrfci, UCHAR *padlen )
//{
//UCHAR i;
//UCHAR c1, c2, c3;
//UINT  iL1, iL2, iL3, iL4;
//UINT  lenfci;
//UCHAR padding;
//UCHAR *ptrobj;
//UCHAR *ptrrec;
//
//      ptrrec = ptrfci;
//      lenfci = ptrfci[1]*256 + ptrfci[0];
//      ptrfci += 2; // pointer to T-L-V
//
//      iL1 = apk_GetBERLEN( ptrfci+1, &c1 );       // FCI Template
//
//      ptrobj = apk_FindTag( 0x84, 0x00, ptrfci ); // DF Name
//      iL2 = apk_GetBERLEN( ptrobj, &c2 );
//
// TL_DispHexWord(0,0,iL1);
// TL_DispHexByte(1,0, c1);
// TL_DispHexWord(2,0,iL2);
// TL_DispHexByte(3,0, c2);
// TL_DumpHexData(0,4, 16, ptrobj);
// UI_WaitKey();
//
//      // check if "A5" is just following "84"
//      iL4 = 0; // length of unexpected data objects btw A5 & 84
//      if( *(ptrobj + iL2 + c2) != 0xA5 )
//        {
//        while( *(ptrobj + iL2 + c2) != 0xA5 )
//             {
//             iL4++;
//             ptrobj++;
//             if( iL4 >= iL1 )
//               {
//               TL_DispHexByte(0,19,0xff);
//               UI_WaitKey();
//               }
//             }
//        }
//
//      ptrobj = apk_FindTag( 0xA5, 0x00, ptrfci ); // FCI Prorietary Template
//      iL3 = apk_GetBERLEN( ptrobj, &c3 );
//
//      // (1) lenfci - 2 = L1 + 1 + C1 or
//      // (2) ISO rigth padding (0x00 or 0xFF)
//
//      if( apk_CheckIsoPadding_Right( ptrrec, padlen ) == FALSE )
//        return( FALSE );
//
////    if( (lenfci - 2) != (iL1 + 1 + c1) )
////      {
////      if( (lenfci - 2) > (iL1 + 1 + c1) )
////        {
////        iL4 = (lenfci - 2) - (iL1 + 1 + c1);
////        padding = *(ptrfci + 2 + iL1);
////
//// TL_DispHexWord(0,0,iL4);
//// TL_DispHexByte(1,0,padding);
//// UI_WaitKey();
////
////        if( (padding != 0x00) && (padding != 0xff) )
////          return( FALSE );
////
////        for( i=0; i<iL4; i++ )
////           {
////           if( *(ptrfci + 2 + iL1 + i) != padding )
////             return( FALSE );
////           }
////        return( TRUE );
////        }
////      else
////        return( FALSE );
////      }
//
//      // L1 - (1 + L2 + C2) = L3 + 1 +C3
//
//      if( (iL1 - (1 + iL2 + c2) - iL4) != (iL3 + 1 + c3) )
//        {
//       if( (lenfci - 2) != (iL1 + 1 + c1) ) // PATCH: PBOC2.0, 2006-02-16, 2CA.099, accept if total length is OK even A5 length is wrong.
//         return( FALSE );
//        }
//
//      return( TRUE );
//}
//

// ---------------------------------------------------------------------------
//UCHAR apk_GenerateAC2( UCHAR type, UCHAR *cdol, UCHAR *ac )
//{
//UINT  iLen;
//UINT  iLen2;
//UINT  iPileLen;
//UCHAR cnt;
//UCHAR *ptrobj;
//UCHAR *ptrtmp;
//UCHAR tlv[261];
//UCHAR result;
//UCHAR len[2];
//UCHAR buf[] = {
//    0x77, 0x22, 0x54, 0x02, 0x01, 0x02, 0x9F, 0x27, 0x01, 0x40, 0x9F, 0x36, 0x02, 0x00, 0x01,
//    0x9F, 0x26, 0x08, 0x47, 0x25, 0xA7, 0x5F, 0xDC, 0xAD, 0x70, 0x5D, 0x9F, 0x10, 0x07, 0x06, 0x01,
//    0x1A, 0x03, 0x90, 0x00, 0x00, 0x90, 0x00 };
//
//      // check excution result
////    if( apdu_GENERATE_AC( type, cdol, ac ) == apiFailed )
////      return( apkFailed );
//
//      memmove(&ac[2], buf, sizeof(buf));
//      ac[0] = 38;
//      ac[1] = 0;
//
//      // check length
//      iLen = ac[1]*256 + ac[0];
//      if( (type & AC_CDA_REQ) == 0 ) // non-CDA process
//        {
//        if( iLen < 15 )
//          return( apkFailed );
//        }
//
//      // check return data format
//      switch( ac[2] )
//            {
//            case 0x80: // primitive
//
//                 if( type & AC_CDA_REQ ) // EMV2000: CDA
//                   return( apkFailed ); // invalid format
//
//            case 0x77: // constructed
//
//                 if( ( (ac[iLen] == 0x90) && (ac[iLen+1] == 0x00) ) ||
//                     (ac[iLen] == 0x61) )
//                   break;
//                 else
//                   {
//                   if( (ac[iLen] == 0x69) && (ac[iLen+1] == 0x85) )
//                     return( apkCondNotSatisfied );
//                   else
//                     return( apkFailed );
//                   }
//
//            default:
//                 return( apkFailed ); // invalid format
//            }
//
//      // PATCH: 2003-06-04, checking for both Tag = 80 & 77
//      iLen2 = apk_GetBERLEN( &ac[3], &cnt ); // actual length of data objects
//      if( (iLen - 2) != (iLen2 + 1 + cnt) )
//        return( apkFailed );
//
//      if( ac[2] == 0x80 ) // primitive?
//        {
//        // save "Cryptogram Information Data" (CID)
//        iLen = 1;
//        apk_WriteRamDataICC( ADDR_ICC_CID, 0x02, (UCHAR *)&iLen );
//        apk_WriteRamDataICC( ADDR_ICC_CID+2, 0x01, &ac[3+cnt] );
//
//        // save "Application Transaction Counter" (ATC)
//        iLen = 2;
//        apk_WriteRamDataICC( ADDR_ICC_ATC, 0x02, (UCHAR *)&iLen );
//        apk_WriteRamDataICC( ADDR_ICC_ATC+2, 0x02, &ac[4+cnt] );
//
//        // save "Application Cryptogram" (AC)
//        iLen = 8;
//        apk_WriteRamDataICC( ADDR_ICC_AC, 0x02, (UCHAR *)&iLen );
//        apk_WriteRamDataICC( ADDR_ICC_AC+2, 0x08, &ac[6+cnt] );
//
//        // save "Issuer Application Data"
//        if( iLen2 > 11 )
//          {
//          iLen2 -= 11;
//          apk_WriteRamDataICC( ADDR_ICC_ISU_AP_DATA, 0x02, (UCHAR *)&iLen2 );
//          apk_WriteRamDataICC( ADDR_ICC_ISU_AP_DATA+2, iLen2, &ac[14+cnt] );
//          }
//        }
//      else // constructed (Tag=77)
//        {
////      iLen2 = apk_GetBERLEN( &ac[3], &cnt ); // template length
////      if( (iLen - 2) != (iLen2 + 1 + cnt) )  // MOVED: 2003-06-04
////        return( apkFailed );
//
//        ptrtmp = &ac[2] + 1 + cnt; // ptr to the 1'st TLV
//        iLen = iLen2;
//
//        // find CID (M)
//        ptrobj = apk_FindTag( 0x9f, 0x27, &ac[2] );
//        if( ptrobj != 0 )
//          {
//          // save "Cryptogram Information Data" (CID)
//      //  iLen = *ptrobj++;
//          iLen2 = apk_GetBERLEN( ptrobj, &cnt );
//          if( iLen2 != 1 )
//            return( apkFailed );
//
//          ptrobj += cnt;
//          apk_WriteRamDataICC( ADDR_ICC_CID, 0x02, (UCHAR *)&iLen2 );
//          apk_WriteRamDataICC( ADDR_ICC_CID+2, iLen2, ptrobj );
//
//          iPileLen = 2 + iLen2 + cnt;
//          }
//        else
//          return( apkFailed );
//
//        // find ATC (M)
//        ptrobj = apk_FindTag( 0x9f, 0x36, &ac[2] );
//        if( ptrobj != 0 )
//          {
//          // save "Application Transaction Counter" (ATC)
//      //  iLen = *ptrobj++;
//          iLen2 = apk_GetBERLEN( ptrobj, &cnt );
//          if( iLen2 != 2 )
//            return( apkFailed );
//
//          ptrobj += cnt;
//          apk_WriteRamDataICC( ADDR_ICC_ATC, 0x02, (UCHAR *)&iLen2 );
//          apk_WriteRamDataICC( ADDR_ICC_ATC+2, iLen2, ptrobj );
//
//          iPileLen += (2 + iLen2 + cnt);
//          }
//        else
//          return( apkFailed );
//
//        // find AC [M]
//        if( (type & AC_CDA_REQ) == 0 ) // non-CDA process
//          {
//          ptrobj = apk_FindTag( 0x9f, 0x26, &ac[2] );
//          if( ptrobj != 0 )
//            {
//            // save "Application Cryptogram" (AC)
//        //  iLen = *ptrobj++;
//            iLen2 = apk_GetBERLEN( ptrobj, &cnt );
//            if( iLen2 != 8 )
//              return( apkFailed );
//
//            ptrobj += cnt;
//            apk_WriteRamDataICC( ADDR_ICC_AC, 0x02, (UCHAR *)&iLen2 );
//            apk_WriteRamDataICC( ADDR_ICC_AC+2, iLen2, ptrobj );
//
//            iPileLen += (2 + iLen2 + cnt);
//            }
//          else
//            return( apkFailed );
//          }
//        else // CDA process
//          {
//          len[0] = 0; // clear flags for AC and SDAD
//          len[1] = 0; //
//
//          ptrobj = apk_FindTag( 0x9f, 0x26, &ac[2] );
//          if( ptrobj != 0 )
//            {
//            // save "Application Cryptogram" (AC)
//            iLen2 = apk_GetBERLEN( ptrobj, &cnt );
//            if( iLen2 != 8 )
//              return( apkFailed );
//            len[0] = 1; // set flag
//
//            ptrobj += cnt;
//            apk_WriteRamDataICC( ADDR_ICC_AC, 0x02, (UCHAR *)&iLen2 );
//            apk_WriteRamDataICC( ADDR_ICC_AC+2, iLen2, ptrobj );
//
//            iPileLen += (2 + iLen2 + cnt);
//            }
//
//          ptrobj = apk_FindTag( 0x9f, 0x4b, &ac[2] );
//          if( ptrobj != 0 )
//            {
//            // save "Signed Dynamic Application Data" (SDAD)
//            iLen2 = apk_GetBERLEN( ptrobj, &cnt );
//            len[1] = 1; // set flag
//
//            ptrobj += cnt;
//            apk_WriteRamDataICC( ADDR_ICC_SIGNED_DAD, 0x02, (UCHAR *)&iLen2 );
//            apk_WriteRamDataICC( ADDR_ICC_SIGNED_DAD+2, iLen2, ptrobj );
//
//            iPileLen += (2 + iLen2 + cnt);
//            }
//
////        if( (len[0] == 0) && (len[1] == 0) ) // both AC & SDAD are absent
////          return( apkFailed );
//
////        if( (len[0] != 0) && (len[1] != 0) ) // both AC & SDAD are present
////          return( apkFailed );
//          }
//
//        // find "Issuer Applicaton Data" (O)
//        ptrobj = apk_FindTag( 0x9f, 0x10, &ac[2] );
//        if( ptrobj != 0 )
//          {
//          // save "Issuer Application Data"
//      //  iLen = *ptrobj++;
//          iLen2 = apk_GetBERLEN( ptrobj, &cnt );
//          if( iLen2 > 32 )
//            return( apkFailed );
//
//          ptrobj += cnt;
//          apk_WriteRamDataICC( ADDR_ICC_ISU_AP_DATA, 0x02, (UCHAR *)&iLen2 );
//          apk_WriteRamDataICC( ADDR_ICC_ISU_AP_DATA+2, iLen2, ptrobj );
//
//          iPileLen += (2 + iLen2 + cnt);
//          }
//
//   TL_DispHexWord( 0,0, iLen );
//   TL_DispHexWord( 1,0, iPileLen );
//   UI_WaitKey();
//
//        if( iLen  > iPileLen ) // any redundancy?
//          {
//   TL_DumpHexData(0,2,7, ptrtmp+iPileLen);
//
//          result = *(ptrtmp+iPileLen); // get first Tag
//          if( apk_CheckWordTag( result ) == FALSE )
//            {
//            iLen2 = apk_GetBERLEN( ptrtmp+1+iPileLen, (UCHAR *)&cnt );
//            iLen2 += ( cnt + 1 );
//            }
//          else
//            {
//            iLen2 = apk_GetBERLEN( ptrtmp+2+iPileLen, (UCHAR *)&cnt );
//            iLen2 += ( cnt + 2 );
//            }
//
//   TL_DispHexWord( 3,0, iLen );
//   TL_DispHexWord( 4,0, iPileLen );
//   TL_DispHexWord( 5,0, iLen2 );
//   UI_WaitKey();
//
//          if( (iLen - iPileLen) == iLen2 )
//            return( apkOK );
//          else
//            return( apkFailed );
//
//          // ----------------------------------------------------------------
//          // scan all "T-L-V"
//          len[0] = iLen & 0x00ff;
//          len[1] = (iLen & 0xff00) >> 8;
//          while(1)
//               {
//               ptrobj = apk_GetBERTLV( len, ptrtmp, tlv );
//
//               if( (len[1]*256 + len[0]) == 0 ) // remaining length
//                 return( apkOK );
//
//               if( ptrobj == 0 ) // legal proprietary TLV
//                 return( apkFailed );
//
//               if( ptrobj == (UCHAR *)-1 ) // illega TLV
//                 return( apkFailed );
//
//               ptrtmp = ptrobj;
//               }
//          }
//        }
//
//      return( apkOK );
//}

//// ---------------------------------------------------------------------------
//void PBOC_TEST()
//{
//
//UCHAR cdol[1];
//UCHAR buf[64];
//
//    apk_GenerateAC2( AC_TC, cdol, buf );

//UCHAR buf[64];
//UCHAR padlen[1];
//UCHAR fci[] = {
//              27, 0,
//              0x6F, 0x17, 0xA5, 0x15, 0x84, 0x05, 0xA0, 0x00, 0x11, 0x11, 0x11, 0x88, 0x01, 0x06, 0xBF,
//              0x0C, 0x08, 0x85, 0x06, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0x90, 0x00
//              };
//
//      apk_ParseLenFCI( fci, padlen );
//      for(;;);
//
//      // ---------------------------------------------------------
//      TL_DumpHexData(0,4,5, g_term_TVR);
//      apk_ReadRamDataTERM( ADDR_TERM_TVR, 5, buf );
//      TL_DumpHexData(0,5,5, buf);
//
//        UI_ClearScreen();
//
//        buf[0] = 0;
//        buf[1] = 0;
//        apk_WriteRamDataICC( ADDR_ICC_TDOL, 254, buf );
//
//        buf[0] = 3;
//        buf[1] = 0;
//        apk_WriteRamDataTERM( ADDR_TERM_TDOL, 254, buf );
//
//        buf[0] = 11;
//        buf[1] = 0;
//
//        buf[2] = 0x9b;
//        buf[3] = 0x02;
//
//        buf[4] = 0x98;
//        buf[5] = 0x14;
//
//        buf[6] = 0x96; // unknown tag
//        buf[7] = 0x10;
//
//        buf[8] = 0xdf; // unknown tag
//        buf[9] = 0x98;
//        buf[10] = 0x14;
//
//        buf[11] = 0x95;
//        buf[12] = 0x05;
//
//        apk_WriteRamDataICC( ADDR_ICC_CDOL1, 13, buf );
//        apk_WriteRamDataICC( ADDR_ICC_CDOL2, 13, buf );
//
//        TL_DispHexByte(0,0,PBOC_CheckTcHash( DOL_CDOL1 ));
//        TL_DumpHexData(0,1,5, g_term_TVR);
//        apk_ReadRamDataTERM( ADDR_TERM_TVR, 5, buf );
//        TL_DumpHexData(0,2,5, buf);
//
//        TL_DispHexByte(4,0,PBOC_CheckTcHash( DOL_CDOL2 ));
//        TL_DumpHexData(0,5,5, g_term_TVR);
//        apk_ReadRamDataTERM( ADDR_TERM_TVR, 5, buf );
//        TL_DumpHexData(0,6,5, buf);
//          for(;;);
//}

// ---------------------------------------------------------------------------
// FUNC  : To get the info of CAPK's in SAM. (fixed 60 bytes info) -> 80 bytes
// INPUT : info - CardScheme0[1]   : 00         RID[5] : 00~04
//                IndexNo0[1]      : 01                : 05
//                Index0[10]       : 02~11             : 06~15
//
//                CardScheme1[1]   : 12         RID[5] : 16~20
//                IndexNo1[1]      : 13                : 21
//                Index1[10]       : 14~23             : 22~31
//
//                CardScheme1[1]   : 24         RID[5] : 32~36
//                IndexNo1[1]      : 25                : 37
//                Index1[10]       : 26~35             : 38~47
//                ..............
//                0xFF
//
//                CardScheme: 0x00=VSDC, 0x01=MCHIP, 0x02=JSMART, 0xFF=end of data.
//                IndexNo   : Number of key index
//                Index     : Key index
// OLD:    sizeof(info) = 60 bytes.  (12*5 payment system)
// NEW:    sizeof(info) = 160 bytes. (16*10 payment system)
//
// OUTPUT: none.
// RETURN: none.
// NOTE  : call this function after api_emvk_InitEMVKernel().
// ---------------------------------------------------------------------------
#ifdef  L2_PBOC20
void  api_emvk_GetPublicKeyInfo( UCHAR *info )
{
UCHAR i,j;
UCHAR cnt;
UCHAR buf[CA_PK_HEADER_LEN];


      memset( info, 0x00, 160 );
      for( i=0; i<MAX_AID_CNT; i++ )
         info[16*i] = 0xff;


//    memset( buf, 0x00, CA_PK_HEADER_LEN );
//    buf[0] = 0xa0;
//    buf[1] = 0x00;
//    buf[2] = 0x00;
//    buf[3] = 0x00;
//    buf[4] = 0x03;
//    buf[5] = 0x11;
//    apk_WriteRamDataKEY( ADDR_CA_PK_01+CA_PK_LEN*0, CA_PK_HEADER_LEN, buf );
//
//    memset( buf, 0x00, CA_PK_HEADER_LEN );
//    buf[0] = 0xa0;
//    buf[1] = 0x00;
//    buf[2] = 0x00;
//    buf[3] = 0x00;
//    buf[4] = 0x04;
//    buf[5] = 0x22;
//    apk_WriteRamDataKEY( ADDR_CA_PK_01+CA_PK_LEN*1, CA_PK_HEADER_LEN, buf );
//
//    memset( buf, 0x00, CA_PK_HEADER_LEN );
//    buf[0] = 0xa0;
//    buf[1] = 0x00;
//    buf[2] = 0x00;
//    buf[3] = 0x00;
//    buf[4] = 0x04;
//    buf[5] = 0x33;
//    apk_WriteRamDataKEY( ADDR_CA_PK_01+CA_PK_LEN*2, CA_PK_HEADER_LEN, buf );
//
//    memset( buf, 0x00, CA_PK_HEADER_LEN );
//    buf[0] = 0x00;
//    buf[1] = 0x00;
//    buf[2] = 0x00;
//    buf[3] = 0x00;
//    buf[4] = 0x00;
//    buf[5] = 0x00;
//    apk_WriteRamDataKEY( ADDR_CA_PK_01+CA_PK_LEN*3, CA_PK_HEADER_LEN, buf );


      for( i=0; i<MAX_KEY_SLOT_NUM; i++ )
         {
         apk_ReadRamDataKEY( ADDR_CA_PK_01+CA_PK_LEN*i, CA_PK_HEADER_LEN, buf );
         if( TL_memcmpc( buf, 0x00, RID_LEN ) != 0 ) // RID=0's ?
           {
           // compare RID
           for( j=0; j<MAX_AID_CNT; j++ )
              {
              if( TL_memcmp( buf, &info[16*j], 5 ) == 0 )
                {
                cnt = info[16*j+5];
                info[16*j+5] += 1; // IndexNo+1
                info[16*j+cnt+6] = buf[5]; // index

                break;
                }
              else
                {
                if( info[16*j] == 0xFF )
                  {
                  memmove( &info[16*j], buf, 5 ); // setup new RID

                  cnt = info[16*j+5];
                  info[16*j+5] += 1; // IndexNo+1
                  info[16*j+cnt+6] = buf[5]; // index

                  break;
                  }
                }
              }
           }
         else
           break;
         }

//SHOWIT:
//   for( i=0; i<10; i++ )
//      {
//      TL_DispHexByte(0,0,i);
//      TL_DumpHexData(0,1,16, &info[i*16]);
//      }
//   goto SHOWIT;

}
#endif
