//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : L2API_11.C                                                 **
//**  MODULE   : api_emv_TAA_Denial()                                       **
//**             api_emv_TAA_Online()                                       **
//**             api_emv_TAA_Default()                                      **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2003/01/29                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  History  :                                                            **
//**	         2009/03/04 Richard: EMV4.2a bug fixed                      **
//**                                                                        **
//**                                                                        **
//**  Copyright(C) 2003-2007 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include "POSAPI.h"
#include "EMVDC.h"
#include "GDATAEX.h"
#include "EMVAPI.h"
#include "EMVAPK.h"

// ---------------------------------------------------------------------------
// FUNCTION: increment "Transaction Sequence Counter" by one.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void api_emv_IncTransSequenceCounter( void )
{
UCHAR buf[16];

     // Transaction Sequence Counter
     api_emv_GetDataElement( DE_TERM, ADDR_TERM_TX_SC, 4, buf );
     buf[4] = 0x00;                          // cnt++
     buf[5] = 0x00;                          //
     buf[6] = 0x00;                          //
     buf[7] = 0x01;                          //
     TL_bcd_add_bcd( 4, &buf[0], &buf[4] );  //
     api_emv_PutDataElement( DE_TERM, ADDR_TERM_TX_SC, 4, buf );
}

// ---------------------------------------------------------------------------
// FUNCTION: calculate the checksum of the EMV Kernel.
//           by adding kernel codes byte by byte. (EMVKAPI1,2,3)
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : the checksum.
// ---------------------------------------------------------------------------
//UINT api_emv_KernelCheckSum( void )
//{
//UINT  i, j, k;
//UINT  iSum;
//UINT  iAddr;
//UCHAR sbuf[7];
//
//
////    iSum = 0;
////    for( i=0; i<3; i++ ) // EMVKAPI N, N+1, N+2
////       {
////       iAddr = 0;
////
////       for( j=0; j<128; j++ ) // 128*256=32KB
////          {
////          sbuf[0] = i + 0x01; // from ROM page 1
////          sbuf[1] = iAddr & 0x00ff;
////          sbuf[2] = (iAddr & 0xff00) >> 8;
////          sbuf[3] = 0x00;
////          sbuf[4] = 0x00;
////          sbuf[5] = 0x80; // length=256 bytes
////          sbuf[6] = 0x00; //
////          api_ram_read( sbuf, g_ibuf );
////
////          // check sum
////          for( k=0; k<256; k++ )
////             iSum += (UINT)g_ibuf[k];
////
////          iAddr += 256;
////          }
////       }
//
//      iSum = 0x853E;
//
//      return( iSum );
//}

// ---------------------------------------------------------------------------
// FUNCTION: check if Unpredictable Number contained in CDOL1.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - yes.
//           FALSE - no.
// ---------------------------------------------------------------------------
UCHAR Check_UnpredictNumInCDOL1( void )
{
UCHAR buf[254];

      // get CDOL1
      apk_ReadRamDataICC( ADDR_ICC_CDOL1, 254, buf );
      buf[1] = buf[0];
      buf[0] = 0x81;
      return( Check_UnpredictNumInCDOL( buf ) );
}

// ---------------------------------------------------------------------------
// FUNCTION: check if Unpredictable Number contained in CDOL2.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - yes.
//           FALSE - no.
// ---------------------------------------------------------------------------
UCHAR Check_UnpredictNumInCDOL2( void )
{
UCHAR buf[254];

      // get CDOL2
      apk_ReadRamDataICC( ADDR_ICC_CDOL2, 254, buf );
      buf[1] = buf[0];
      buf[0] = 0x81;
      return( Check_UnpredictNumInCDOL( buf ) );
}

// ---------------------------------------------------------------------------
// FUNCTION: build CDOL1.
// INPUT   : ptrobj - pointer to orignal CDOL. (berLEN, T1-L1, T2-L2,...)
// OUTPUT  : cdol - (L-V), (V=data elements)
//                  L = total length of the concatenated data elements.
//                  concatenation of all related data elements of the DOL type.
// REF     : g_ibuf
//           g_obuf
// RETURN  : TRUE  - OK
//           FALSE - something wrong in TLV format.
// ---------------------------------------------------------------------------
UCHAR Build_CDOL1( UCHAR *cdol )
{
UCHAR length[1];
UCHAR buf[254];
UINT  iLen;
UCHAR result;

      // get CDOL1
      apk_ReadRamDataICC( ADDR_ICC_CDOL1, 254, buf );
      buf[1] = buf[0];
      buf[0] = 0x81;

      // concatenate DO
      result = api_emv_ConcatenateDOL( DOL_CDOL1, buf, cdol );

      // backup CDOL1 concatenation results
      iLen = cdol[0];
      apk_WriteRamDataICC( ADDR_ICC_CDOL1_CATS, 2, (UCHAR *)&iLen );
      apk_WriteRamDataICC( ADDR_ICC_CDOL1_CATS+2, iLen, &cdol[1] );

      if( result == emvOK )
        return( TRUE );
      else
        return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: build CDOL2.
// INPUT   : ptrobj - pointer to orignal CDOL. (berLEN, T1-L1, T2-L2,...)
// OUTPUT  : cdol - (L-V), (V=data elements)
//                  L = total length of the concatenated data elements.
//                  concatenation of all related data elements of the DOL type.
// REF     : g_ibuf
//           g_obuf
// RETURN  : TRUE  - OK
//           FALSE - something wrong in TLV format.
// ---------------------------------------------------------------------------
UCHAR Build_CDOL2( UCHAR *cdol )
{
UCHAR length[1];
UCHAR buf[254];
UINT  iLen;
UCHAR result;

      // get CDOL2
      apk_ReadRamDataICC( ADDR_ICC_CDOL2, 254, buf );
      buf[1] = buf[0];
      buf[0] = 0x81;

      // concatenate DO
      result = api_emv_ConcatenateDOL( DOL_CDOL2, buf, cdol );

      // backup CDOL2 concatenation results
      iLen = cdol[0];
      apk_WriteRamDataICC( ADDR_ICC_CDOL2_CATS, 2, (UCHAR *)&iLen );
      apk_WriteRamDataICC( ADDR_ICC_CDOL2_CATS+2, iLen, &cdol[1] );

      if( result == emvOK )
        return( TRUE );
      else
        return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: load target TAC's from the selected AID.
// INPUT   : none.
// OUTPUT  : none.
// REF     : g_selected_aid_index
// RETURN  : none.
// ---------------------------------------------------------------------------
//void  TAA_LoadTAC( void )
//{
//UCHAR tac[5];
//
//      // load the target "TAC_Default" for the selected AID
//      apk_ReadRamDataTERM( ADDR_TERM_TAC_DEFAULT_01+TAC_LEN*g_selected_aid_index, TAC_LEN, tac );
//      apk_WriteRamDataTERM( ADDR_TERM_TAC_DEFAULT, TAC_LEN, tac );
//
//      // load the target "TAC_Denial" for the selected AID
//      apk_ReadRamDataTERM( ADDR_TERM_TAC_DENIAL_01+TAC_LEN*g_selected_aid_index, TAC_LEN, tac );
//      apk_WriteRamDataTERM( ADDR_TERM_TAC_DENIAL, TAC_LEN, tac );
//
//      // load the target "TAC_Online" for the selected AID
//      apk_ReadRamDataTERM( ADDR_TERM_TAC_ONLINE_01+TAC_LEN*g_selected_aid_index, TAC_LEN, tac );
//      apk_WriteRamDataTERM( ADDR_TERM_TAC_ONLINE, TAC_LEN, tac );
//}

// ---------------------------------------------------------------------------
// FUNCTION: for each bit in the TVR that has a value of 1, the terminal shall
//           check the corresponding bits in the IAC and TAC.
// INPUT   : iac - the IAC code. (5 bytes)
//           tac - the TAC code. (5 bytes)
// OUTPUT  : none.
// REF     : g_term_TVR[0..4]
// RETURN  : TRUE  - bits matched. (issuer or acquirer wishes to do the action)
//           FALSE - not matched.
// ---------------------------------------------------------------------------
UCHAR TAA_CheckBitsTVR( UCHAR *iac, UCHAR *tac )
{
UCHAR i;
UCHAR byteno;
UCHAR bitno;

      for( i=0; i<40; i++ ) // from "msb" to "lsb"
         {
         byteno = i / 8;
         bitno = (i % 8);

         if( (g_term_TVR[byteno] << bitno) & 0x80  ) // TVR bit value = '1'
           {
           // check the corresponding bit in IAC and TAC
           if( ( (iac[byteno] << bitno) & 0x80 ) || ( (tac[byteno] << bitno) & 0x80 ) )
             return( TRUE );
           }
         }

      return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: terminal action analysis for Action Code - DENIAL.
// INPUT   : none.
// OUTPUT  : none.
// REF     :
// RETURN  : emvOK     (meet condition)
//           emvFailed (not meet condition)
// ---------------------------------------------------------------------------
UCHAR api_emv_TAA_Denial( void )
{
UCHAR i;
UINT  iLen;
UCHAR iac[5];
UCHAR tac[5];

      // IAC-Denial present?
      api_emv_GetDataElement( DE_ICC, ADDR_ICC_IAC_DENIAL, 2, (UCHAR *)&iLen );
      if( iLen == 0 )
        {
        // not present, set default IAC-Denial bits = 0s
        for( i=0; i<5; i++ )
           iac[i] = 0x00;
        api_emv_PutDataElement( DE_ICC, ADDR_ICC_IAC_DENIAL+2, 5, iac );
        }
      else
        api_emv_GetDataElement( DE_ICC, ADDR_ICC_IAC_DENIAL+2, 5, iac );

      // get TAC-Denial
      api_emv_GetDataElement( DE_TERM, ADDR_TERM_TAC_DENIAL, 5, tac );

      // check corresponding bits in both TVR and IAC or TAC
      if( TAA_CheckBitsTVR( iac, tac ) == TRUE )
        return( emvOK );
      else
        return( emvFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: terminal action analysis for Action Code - ONLINE.
// INPUT   : none.
// OUTPUT  : none.
// REF     :
// RETURN  : emvOK     (meet condition)
//           emvFailed (not meet condition)
// ---------------------------------------------------------------------------
UCHAR api_emv_TAA_Online( void )
{
UCHAR i;
UINT  iLen;
UCHAR iac[5];
UCHAR tac[5];

      // IAC-Online present?
      api_emv_GetDataElement( DE_ICC, ADDR_ICC_IAC_ONLINE, 2, (UCHAR *)&iLen );
      if( iLen == 0 )
        {
        // not present, set default IAC-Online bits = 1s
        for( i=0; i<5; i++ )
           iac[i] = 0xff;
        api_emv_PutDataElement( DE_ICC, ADDR_ICC_IAC_ONLINE+2, 5, iac );
        }
      else
        api_emv_GetDataElement( DE_ICC, ADDR_ICC_IAC_ONLINE+2, 5, iac );

      // get TAC-Online
      api_emv_GetDataElement( DE_TERM, ADDR_TERM_TAC_ONLINE, 5, tac );

      // check corresponding bits in both TVR and IAC or TAC
      if( TAA_CheckBitsTVR( iac, tac ) == TRUE )
        return( emvOK );
      else
        return( emvFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: terminal action analysis for Action Code - DEFAULT.
// INPUT   : none.
// OUTPUT  : none.
// REF     :
// RETURN  : emvOK     (meet condition)
//           emvFailed (not meet condition)
// ---------------------------------------------------------------------------
UCHAR api_emv_TAA_Default( void )
{
UCHAR i;
UINT  iLen;
UCHAR iac[5];
UCHAR tac[5];

      // IAC-Default present?
      api_emv_GetDataElement( DE_ICC, ADDR_ICC_IAC_DEFAULT, 2, (UCHAR *)&iLen );
      if( iLen == 0 )
        {
        // not present, set default IAC-Default bits = 1s
        for( i=0; i<5; i++ )
           iac[i] = 0xff;
        api_emv_PutDataElement( DE_ICC, ADDR_ICC_IAC_DEFAULT+2, 5, iac );
        }
      else
        api_emv_GetDataElement( DE_ICC, ADDR_ICC_IAC_DEFAULT+2, 5, iac );

      // get TAC-Default
      api_emv_GetDataElement( DE_TERM, ADDR_TERM_TAC_DEFAULT, 5, tac );

      // check corresponding bits in both TVR and IAC or TAC
      if( TAA_CheckBitsTVR( iac, tac ) == TRUE )
        return( emvOK );
      else
        return( emvFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: set ADDR_TERM_ARC to the specified value.
// INPUT   : arc - the authorization response code.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void TAA_SetARC( UINT arc )
{
UCHAR buf[2];

      buf[0] = (arc & 0xff00) >> 8;
      buf[1] = arc & 0x00ff;
      api_emv_PutDataElement( DE_TERM, ADDR_TERM_ARC, 2, buf );
}

// ---------------------------------------------------------------------------
// FUNCTION: send financial cofirm message to issuer for reporting ISR.
// INPUT   : p1  - TC or AAC.
//           arc - final ARC.
// OUTPUT  : none.
// REF     : g_isu_script_addr
//           g_online_complete
// RETURN  : TRUE  - done.
//           FALSE - transmission failed.
// ---------------------------------------------------------------------------
/*
UCHAR TAA_FinancialConfirm( UINT arc, UCHAR p1 )
{
UCHAR isr[2+5*16]; // 2L-V
UCHAR cnt;
UINT  iLen;

      if( (g_online_complete == TRUE) && (arc == ARC_APPROVED_UNABLE_ONLINE) ) // 2003-04-10
        {
        // send financial request for ODC (online data capture)
        EMVDC_OnlineProcessing( MSGID_FINANCIAL_REQ, p1, g_term_ARC, isr, (UINT *)&g_isu_script_addr );
        }

      apk_ReadRamDataTERM( ADDR_TERM_ISR, 1, (UCHAR *)&cnt );
      if( (g_online_complete == FALSE) || (cnt == 0) )
        return( TRUE );

      iLen = cnt*5;
      isr[0] = iLen & 0x00ff;
      isr[1] = (iLen & 0xff00) >> 8;
      apk_ReadRamDataTERM( ADDR_TERM_ISR+1, iLen, &isr[2] );

// TL_DumpHexData(0,0,7, isr);

      // financial confirm for ISR
      apk_ReadRamDataICC( ADDR_ICC_CID+2, 0x01, (UCHAR *)&cnt );
//    if( EMVDC_OnlineProcessing( MSGID_FINANCIAL_CONFIRM, p1, g_term_ARC, isr, (UINT *)&g_isu_script_addr ) == ONL_Completed ) // completed online?
      if( EMVDC_OnlineProcessing( MSGID_FINANCIAL_CONFIRM, cnt, g_term_ARC, isr, (UINT *)&g_isu_script_addr ) == ONL_Completed ) // completed online?
        {
//      apk_ClearRamDataTERM( ADDR_TERM_ISR, 1, 0x00 );  // 2006-10-11, removed for Last BDC Upload function
        return( TRUE );
        }
      else
        return( FALSE );
}
*/

// ---------------------------------------------------------------------------
// FUNCTION: process issuer scripts (TAG=72) after the final generate AC command.
//           excuting only for "ONLINE" transaction completion.
// INPUT   : none.
// OUTPUT  : none.
// REF     : g_isu_script_addr
//           g_online_complete
// RETURN  : none.
// ---------------------------------------------------------------------------
void TAA_COMPLETION_ISP2( void )
{
UINT  iLen;
UCHAR result;

      if( g_isr_len_overrun != 0 )  // PATCH: PBOC2.0, 2006-02-16, 2CO03402
        return; // ever overrun

      if( g_online_complete == FALSE )
        return;

      apk_ReadRamDataTERM( g_isu_script_addr, 2, (UCHAR *)&iLen );
      if( iLen == 0 )
        return;

      // after issuing the final GENERATE AC command
//    if( apk_IssuerScriptProcessing( 0x72, g_isu_script_temp ) != apkOK )

      result = apk_IssuerScriptProcessing2( 0x72, g_isu_script_addr );
      if( result == apkNotReady ) // PATCH: 2003-08-03
        return;                   // script is not performed

      if( result != apkOK )
        {
        g_term_TVR[4] |= TVR4_SP_FAILED_AFTER_FAC;
        api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR+4, 1, &g_term_TVR[4] );
        }

      g_term_TSI[0] |= TSI0_SCRIPT_PROCESS_PERFORMED;
      api_emv_PutDataElement( DE_TERM, ADDR_TERM_TSI, 1, &g_term_TSI[0] );
}

// ---------------------------------------------------------------------------
// FUNCTION: set CDA failure in TVR.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void TAA_set_CDA_fail_in_TVR( void )
{
      g_term_TVR[0] |= TVR0_OFFLINE_CDA_FAILED; // set CDA failure in TVR
      api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR, 1, &g_term_TVR[0] );
}

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

// ---------------------------------------------------------------------------
// FUNCTION: Check TC Hash (Tag 98 in CDOL1).
// INPUT   : type = DOL_CDOL1 or DOL_CDOL2
// OUTPUT  : none.
// REF     : g_ibuf
// RETURN  : TRUE  - TC Hash Value Tage is in CDOL.
//           FALSE - not found.
// NOTE    : Call this function prior to building CDOL1 and CDOL2.
// ---------------------------------------------------------------------------
UCHAR TAA_CheckTcHash( UCHAR type )
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
// FUNCTION: once terminal risk management and application functions related
//           to a normal offline transaction have been completed, the terminal
//           makes the first decision as to whether the transaction should be
//           one of the following decisions:
//           (1) Approved Offline     <-- TC
//           (2) Declined Offline     <-- AAC
//           (3) Transmitted Online   <-- ARQC
//           (4) Forced online by merchant.
// INPUT   : online - TRUE =forced transaction online.
//                    FALSE=normal decision.
// OUTPUT  : arc - the authorization response code. (an2)
// REF     : g_term_ARC
//           g_isu_authen_data
//           g_isu_script_temp
//           g_online_complete
// RETURN  : emvOK       (TAA has finished)
//           emvFailed   (ICC logic error or service not allowed, terminate the transaction)
//           emvNotReady (RFU, TAA has not finished, ie. Voice Referral is required.)
//	     emvAborted  (1'st GenAC condition not satisfied, goto FallBack according to ETEC)
//           emvAborted  (RFU, for TA only, will be replaced with emvFailed)
// ---------------------------------------------------------------------------
UCHAR api_emv_TerminalActionAnalysis2( UCHAR online, UCHAR arc[] )
{
UINT  i;
UCHAR p1;
UCHAR buf1[300];	// org 254
UCHAR buf2[300];	// org 254
UCHAR auth_code[6];
UCHAR cid;
UINT  iARC;
UINT  iLen;
UCHAR result;
UCHAR flag_irefer; // referral from issuer (online result)
UCHAR flag_online_retry;
UCHAR flag_reversal;
UCHAR online_retry;
UCHAR flag_reversal2;
UCHAR cda;

      g_isr_len_overrun = 0;

      if( CDA_CheckCondition() == TRUE ) // EMV2000: if TRUE to perform CDA
        cda = AC_CDA_REQ;
      else
        cda = 0;

//    TAA_LoadTAC();

      g_online_complete = FALSE;
      flag_irefer = FALSE;

      flag_online_retry = FALSE;
      online_retry = 1;

      flag_reversal = FALSE;
      flag_reversal2= FALSE;

      // setup current transaction Date & Time

      // PATCH: PBOC2.0, 2006-02-21, move get_date_time prior to GET_PROCESSING_OPTIONS
//    TL_GetDateTime( &buf1[1] );
//
//    buf1[0]=6;
//    TL_asc2bcd( 3, buf2, buf1 );     // convert to BCD, buf2: YYMMDD
//    apk_WriteRamDataTERM( ADDR_TERM_TX_DATE, 3, buf2 );
//
//    buf1[6]=6;
//    TL_asc2bcd( 3, buf2, &buf1[6] ); // convert to BCD, buf2: HHMMSS
//    apk_WriteRamDataTERM( ADDR_TERM_TX_TIME, 3, buf2 );

      iARC = 0;
      g_term_ARC[0] = 0;
      g_term_ARC[1] = 0;
// printf("online =%d\n",online);
      // forced online
      if( online == TRUE )
        {
        g_term_TVR[3] |= TVR3_MERCHANT_FORCED_TX_ONLINE;
        api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR+3, 1, &g_term_TVR[3] );

	// 2011-08-03, ADVT test case 4, shall be declined at 1'st issuance.
//      p1 = AC_ARQC;
//      goto FIRST_ISSUANCE;
        }

TAA_DENIAL:

      TAA_CheckTcHash( DOL_CDOL1 );	// 2017-01-05

      // Action Code: DENIAL
      if( api_emv_TAA_Denial() == emvOK )
        {
        // temp set ARC="Z3" (declined when unable to go online)
        iARC = ARC_DECLINED_UNABLE_ONLINE;
        TAA_SetARC( ARC_OFFLINE_DECLINED );

        p1 = AC_AAC;
        cda = 0;
        
        goto FIRST_ISSUANCE;
        }

      // if terminal is for any reason unable to process online,
      // or only supports offline mode,
      // then go to DEFAULT case. (ref: g_term_mode_offline=1)

TAA_ONLINE:

      // Action Code: ONLINE
      if( api_emv_TAA_Online() == emvOK )
        {
        p1 = AC_ARQC;
        goto FIRST_ISSUANCE;
        }
      else
        {
        // set ARC="Y1" (offline approved)
        iARC = ARC_OFFLINE_APPROVED;
        TAA_SetARC( ARC_OFFLINE_APPROVED );

        p1 = AC_TC;
        goto FIRST_ISSUANCE;
        }

TAA_DEFAULT:

      // Action Code: DEFAULT
      if( api_emv_TAA_Default() == emvOK )
        {
        // if CVM=signature & CVR=always then accept it as an approval (REMOVED: 2003-09-17)
//      if( (g_term_CVMR[0] == CVR_SIGN_PAPER) && (g_term_CVMR[1] != CVC_CASH_OR_CASHBACK) && (g_term_CVMR[2] == CVMR_UNKNOWN) )
//        goto TAA_DEFAULT2;
//
//      ** This is the EMV ERRATA test case "2CJ.093.00", the modified process shall be as follows, PATCH: 2003-09-25
//      ** Condition: Unable to go online.
//      if( (g_term_CVMR[0] & 0xBF) == CVR_SIGN_PAPER) && (g_term_CVMR[2] == CVMR_UNKNOWN) )
//        {
//        if( (g_term_CVMR[1] == CVC_ALWAYS) || (g_term_CVMR[1] == CVC_AMT_UNDER_X) || (g_term_CVMR[1] == CVC_AMT_OVER_Y) )
//          goto TAA_DEFAULT2;
//        }

        // set ARC="Z3" (declined when unable to go online)
        iARC = ARC_DECLINED_UNABLE_ONLINE;
        TAA_SetARC( ARC_DECLINED_UNABLE_ONLINE ); // 2003-03-28

        cda = 0;
        p1 = AC_AAC;
        goto SECOND_ISSUANCE;
        }
      else
        {
TAA_DEFAULT2:

        // set ARC="Y3" (approved when unable to go online)
        iARC = ARC_APPROVED_UNABLE_ONLINE;
        TAA_SetARC( ARC_APPROVED_UNABLE_ONLINE ); // 2003-03-28

        p1 = AC_TC;
        goto SECOND_ISSUANCE;
        }

      //---------------------------------------------------------------------
      //        1'ST ISSUANCE
      //---------------------------------------------------------------------
FIRST_ISSUANCE:
/*20081230_Richard: EMV 4.2a, 2CC.133.01~02, CDA flow change
 *

      // EMV2000: CDA - check unpredictable number in CDOL1?
      if( Check_UnpredictNumInCDOL1() == FALSE )
        {
        if( cda != 0 )
          {
          cda = 0;
          TAA_set_CDA_fail_in_TVR();

          p1 = AC_AAC;
          }
        }
*/
      TAA_CheckTcHash( DOL_CDOL1 ); // PATCH: PBOC2.0, 2006-02-18

      if( Build_CDOL1( buf1 ) == FALSE ) // build CDOL1
        return( emvFailed );             // PATCH: 2005/08/09
        
/*
      if( apk_GenerateAC( p1+cda, buf1, buf2 ) != apkOK )
        {
        if( cda != 0 )
          {
          TAA_set_CDA_fail_in_TVR();
          return( emvAborted );         // PATCH: 2006/09/30, TA only
          }

        return( emvFailed );
        }
*/
 //20090302_Richard: EMV 4.2a, 2CC.135.00, mandatory data objects are not present in response to generate AC
      switch( apk_GenerateAC( p1+cda, buf1, buf2 )  )
             {
              case apkCondNotSatisfied:	// 2003-11-10, ETEC: FallBack
              	   return( emvAborted );
              	   
              case apkMObjMissing:
                   return( emvFailed );

	      case apkOK:
		   break;
		   
	      default:
	          
		  if( cda != 0 )
                    {
                    TAA_set_CDA_fail_in_TVR();
              //    return( emvAborted );         // PATCH: 2006/09/30, TA only
                    }

                   return( emvFailed );
 	     }

      g_term_TSI[0] |= TSI0_CRM_PERFORMED;
      api_emv_PutDataElement( DE_TERM, ADDR_TERM_TSI, 1, &g_term_TSI[0] );

      // get "Cryptogram Information Data"
      api_emv_GetDataElement( DE_ICC, ADDR_ICC_CID+2, 1, (UCHAR *)&cid );

      // PATCH: PBOC2.0, 2006-02-14, according EMV SPEC bulletin #42, no more support ICC AAR
      if( (cid & CID_AC_MASK) == AC_AAR )
        return( emvFailed );

      // check "Advice Code"
      if( (cid & CID_ADVICE_REQUIRED_MASK) || (cid & CID_ADVICE_CODE_MASK) )
        {
        switch( cid & CID_ADVICE_CODE_MASK )
              {
              case AVC_SERVICE_NOT_ALLOWED:

                   return( emvFailed );

              default:
                   break;
              }
        }
      // EMV2000: CDA - check CID of 1'st issuance
      if( cda != 0 )
        {
        switch( cid & CID_AC_MASK )
              {
              case AC_AAC:

                   cda = 0;
                   /*20081230_Richard: EMV 4.2a, 2CC.134.03, 2CC.134.07(1), CDA flow change
                    *
                   TAA_set_CDA_fail_in_TVR();
                   */
                   goto COMPLETION_AAC_AAC;

/*20090304_Richard: EMV 4.2a, 2CC.122.02, Return AAR

              case AC_AAR:

                   api_emv_GetDataElement( DE_ICC, ADDR_ICC_SIGNED_DAD, 2, (UCHAR *)&iLen );
                   if( iLen != 0 ) // signed DAD in response?
                     {
                     cda = 0;
                     TAA_set_CDA_fail_in_TVR();
                     goto COMPLETION_AAC_AAC;
                     }

                   break; // go on for performing referral processing
*/
              case AC_TC:

                   api_emv_GetDataElement( DE_ICC, ADDR_ICC_SIGNED_DAD, 2, (UCHAR *)&iLen );

                   if( iLen == 0 ) // signed DAD in response?
                     {
                     cda = 0;
                     TAA_set_CDA_fail_in_TVR();

//                   p1 = AC_AAC;
//                   iARC = ARC_OFFLINE_DECLINED;
//                   goto SECOND_ISSUANCE;

                     goto COMPLETION_AAC_AAC; // PATCH: PBOC20, 2006-02-08
                     }

                   result = api_emv_OfflineCDA( 1, buf2 );
                   if( result == emvAborted )
                     {
                     cda = 0;
                     TAA_set_CDA_fail_in_TVR();

                     p1 = AC_AAC;
                     iARC = ARC_OFFLINE_DECLINED;
                     goto SECOND_ISSUANCE;
                     }

                   if( result != emvOK ) // recovery OK?
                     {
                     cda = 0;
                     TAA_set_CDA_fail_in_TVR();

                //   return( emvAborted ); // TA only
                     return( emvFailed );  // real value
                     }
                   else
                     {
                     break; // perform approval processing
                     }

              case AC_ARQC:

                   api_emv_GetDataElement( DE_ICC, ADDR_ICC_SIGNED_DAD, 2, (UCHAR *)&iLen );
                   if( iLen == 0 ) // signed DAD in response?
                     {
                     cda = 0;
                     TAA_set_CDA_fail_in_TVR();

                     p1 = AC_AAC;
                     iARC = ARC_OFFLINE_DECLINED;
                     goto SECOND_ISSUANCE;
                     }

                   result = api_emv_OfflineCDA( 1, buf2 );
                   if( result == emvAborted )
                     {
                     cda = 0;
                     TAA_set_CDA_fail_in_TVR();

                //   return( emvAborted ); // TA only
                     return( emvFailed );  // real value
                     }

                   if( result != emvOK ) // recovery OK?
                     {
                     cda = 0;
                     TAA_set_CDA_fail_in_TVR();

                     p1 = AC_AAC;
                     iARC = ARC_OFFLINE_DECLINED;
                     goto SECOND_ISSUANCE;
                     }
                   else
                     {
                     break; // perform approval processing
                     }
                //20090304_Richard: EMV 4.2a, 2CC.122.02, Return AAR
                default :

                     return ( emvFailed );
              }
        }
      // check earlier processing
      switch( p1 )
            {
            case AC_AAC:

                 goto COMPLETION_AAC;

            case AC_ARQC:

                 goto COMPLETION_ARQC;

            case AC_TC:

		 if( online == TRUE )		// PATCH: 2011-08-03
		   goto COMPLETION_ARQC;	// forced to go online
		   
                 goto COMPLETION_TC;

            default:

                 return( emvFailed);
            }

      //---------------------------------------------------------------------
      //        2'ND ISSUANCE
      //---------------------------------------------------------------------
SECOND_ISSUANCE:
/*20081226_Richard: EMV 4.2a, 2CC.133.01~02, CDA flow change
 *
      // EMV2000: CDA - check unpredictable number in CDOL2?
      if( Check_UnpredictNumInCDOL2() == FALSE )
        {
        p1 &= AC_CDA_REQ_MASK; // remove request CDA in P1

        if( cda != 0 )
          {
          cda = 0;
          TAA_set_CDA_fail_in_TVR();

          p1 = AC_AAC;
          }
        }
*/
      TAA_CheckTcHash( DOL_CDOL2 ); // PATCH: PBOC2.0, 2006-02-18
      if( Build_CDOL2( buf1 ) == FALSE ) // build CDOL2
        return( emvFailed );             // PATCH: 2005/08/09
/*
      if( apk_GenerateAC( p1+cda, buf1, buf2 ) != apkOK )
        {
        if( (g_online_complete == TRUE) && (flag_reversal == FALSE) )
          {
          if( cda != 0 )                // PATCH: 2006/10/01
            TAA_set_CDA_fail_in_TVR();

          // reversal of the current online-approved transaction
          EMVDC_OnlineProcessing( MSGID_REVERSAL_REQ, p1, g_term_ARC, g_isu_authen_data, (UINT *)&g_isu_script_addr );
          g_term_ARC[0] = 0;
          g_term_ARC[1] = 0;
          }

        if( cda != 0 )
          return( emvAborted );         // PATCH: 2006/09/30, TA only

        return( emvFailed );
        }
*/

       //20090302_Richard: EMV 4.2a, 2CC.135.00, mandatory data objects are not present in response to generate AC
       switch( apk_GenerateAC( p1+cda, buf1, buf2 )  )
           {
             case apkMObjMissing:
               return( emvFailed );

	     case apkOK:
	       break;

	     default:
	      if( (g_online_complete == TRUE) && (flag_reversal == FALSE) )
                {
                  if( cda != 0 )                // PATCH: 2006/10/01
                   TAA_set_CDA_fail_in_TVR();

                  // reversal of the current online-approved transaction
//                EMVDC_OnlineProcessing( MSGID_REVERSAL_REQ, p1, g_term_ARC, g_isu_authen_data, (UINT *)&g_isu_script_addr );
                  g_term_ARC[0] = 0;
                  g_term_ARC[1] = 0;
                  
                  return( emvNotReady );
                }

             if( cda != 0 )
               return( emvAborted );         // PATCH: 2006/09/30, TA only

               return( emvFailed );
 	   }
      // get "Cryptogram Information Data"
      api_emv_GetDataElement( DE_ICC, ADDR_ICC_CID+2, 1, (UCHAR *)&cid );

      // PATCH: PBOC2.0, 2006-02-14, according EMV SPEC bulletin #42, no more support ICC AAR
      if( (cid & CID_AC_MASK) == AC_AAR )
        {
        iARC = ARC_OFFLINE_DECLINED;
        goto COMPLETION_END2;
        }

      // PATCH: 2006/10/01, EMV2000 v4.1a -- 2CM.042.01
      // check "Advice Code"
      if( (cid & CID_ADVICE_REQUIRED_MASK) || (cid & CID_ADVICE_CODE_MASK) )
        {
        switch( cid & CID_ADVICE_CODE_MASK )
              {
              case AVC_SERVICE_NOT_ALLOWED:

//                 if( (g_online_complete == TRUE) && (flag_reversal == FALSE) )
//                   {
//                   // reversal of the current online-approved transaction
//                   EMVDC_OnlineProcessing( MSGID_REVERSAL_REQ, p1, g_term_ARC, g_isu_authen_data, (UINT *)&g_isu_script_addr );
//                   }

                   arc[0] = g_term_ARC[0];
                   arc[1] = g_term_ARC[1];
                   api_emv_PutDataElement( DE_TERM, ADDR_TERM_ARC, 2, g_term_ARC );

                   g_term_ARC[0] = 0;
                   g_term_ARC[1] = 0;

                   return( emvFailed );

              default:
                   break;
              }
        }

      // EMV2000: CDA - check CID of 2'st issuance
      if( cda != 0 )
        {
        switch( cid & CID_AC_MASK )
              {
              case AC_TC:

                   if( api_emv_OfflineCDA( 2, buf2 ) != emvOK ) // recovery OK?
                     {
                     cda = 0;
                     TAA_set_CDA_fail_in_TVR();

//                   if( (g_online_complete == TRUE) && (flag_reversal == FALSE) )
//                     {
//                     // reversal of the current online-approved transaction
//                     EMVDC_OnlineProcessing( MSGID_REVERSAL_REQ, p1, g_term_ARC, g_isu_authen_data, (UINT *)&g_isu_script_addr );
//                     }

                     goto COMPLETION_EXIT2;
                     }

                   break;

              case AC_AAC:

                   cda = 0;
                   /*20081226_Richard: EMV 4.2a, 2CC.134.07(2), 2CC.134.13~14, CDA flow change
                    *
                   TAA_set_CDA_fail_in_TVR();
                   */
                   break;
              }
        }

      // check earlier processing
      switch( p1 )
            {
            case AC_AAC:

                 goto COMPLETION2_AAC;

            case AC_TC:

                 goto COMPLETION2_TC;

            default:

                 return( emvFailed );
            }

      //---------------------------------------------------------------------
      //        COMPLETION PROCESSING
      //
      //        Primitive Data Object  : 80-L-[Value] SW1 SW2
      //        Constructed Data Object: 77-L-[...Data...] SW1 SW2
      //
      //        Returned Values/Data Objects Format:
      //        ----    --------------------------------  --------------
      //        Tag     Value                             Presence
      //        ----    --------------------------------  --------------
      //    [1] 9F27    Cryptogram Information Data       M  (b1)
      //    [2] 9F36    Application Trans. Counter (ATC)  M  (b2)
      //    [3] 9F26    Application Cryptogram (AC)       M  (b8)
      //    [4] 9F10    Issuer Application Data           O  (up to b32)
      //        ----    --------------------------------  --------------
      //
      //---------------------------------------------------------------------

COMPLETION_AAC:

      switch( cid & CID_AC_MASK )
            {
            case AC_AAC:  // CAA: reject

COMPLETION_AAC_AAC:

                 // set ARC="Z1" (offline declined)
                 iARC = ARC_OFFLINE_DECLINED;

                 goto COMPLETION_END;

            default://EMV 42a Charles 2009-03-06 2ck.017.00
               return( emvFailed ); // icc logic error, terminate transaction.
//                 iARC = ARC_OFFLINE_DECLINED;
//                 return( emvOK );
            }

      // ---------------------------------------------------------------------
COMPLETION_ARQC:

      if( online == TRUE )
        {
        online = FALSE;
        p1 = AC_AAR;
        goto COMPLETION_ARQC_ARQC;
        }
      switch( cid & CID_AC_MASK )
            {
            case AC_AAC:  // CAA: reject

                 goto COMPLETION_AAC_AAC;

            case AC_AAR:  // CAA: refer

COMPLETION_ARQC_AAR:

                 iARC = 0; // indicating ARC is from external procedure
#ifdef PCI_AP
                      switch( EMVDC_ReferralProcessing( auth_code ) )
#else                    
		                  switch( EMV_CallBack_ReferralProcessing() )
#endif                      
                       {
                       case REF_Approved:      // approved by referral

                            if( flag_irefer == TRUE ) // issuer referral
                              {
                        //    api_emv_PutDataElement( DE_TERM, ADDR_TERM_ARC, 2, auth_code );

                        //    iARC = 0x5934; // Y4: a special code for issuer referral approval
                        
                              auth_code[0] = 'Y'; // PATCH: 2003-07-07
                              auth_code[1] = '4'; //    Y4: a special code for issuer referral approval

                              p1 = AC_TC;
                              }
                            else // card referral (CREFER-MA)
                              {
                              auth_code[0] = 'Y';
                              auth_code[1] = '2';
                              api_emv_PutDataElement( DE_TERM, ADDR_TERM_ARC, 2, auth_code );

                              p1 = AC_TC;
                              }

                            goto SECOND_ISSUANCE;

                       case REF_Declined:      // declined by referral

                            if( flag_irefer == TRUE ) // issuer referral (CREFER-MA)
                              {
                        //    api_emv_PutDataElement( DE_TERM, ADDR_TERM_ARC, 2, auth_code );

                              // PATCH: 2003-07-10
                              //        retrieve original issuer response code
                              api_emv_GetDataElement( DE_TERM, ADDR_TERM_ARC, 2, auth_code );

                              p1 = AC_AAC;
                              }
                            else // card referral (CREFER-MA)
                              {
                              auth_code[0] = 'Z';
                              auth_code[1] = '2';
                              api_emv_PutDataElement( DE_TERM, ADDR_TERM_ARC, 2, auth_code );

                        //    p1 = AC_TC;
                              p1 = AC_AAC;
                              }

                            goto SECOND_ISSUANCE;

                       case REF_ForcedOnline:  // forced-online by attendant

                            p1 = AC_AAR;
                            goto COMPLETION_ARQC_ARQC;

                       case REF_ForcedDecline: // forced-decline by attendant

                            p1 = AC_AAC;
                            goto SECOND_ISSUANCE;
                        //  return( emvFailed );

                       case REF_ForcedAccept:  // forced-acceptance by attendant

                            p1 = AC_TC;
                            goto SECOND_ISSUANCE;

                       default:

                            return( emvFailed );
                       }

            case AC_ARQC: // CAA: online

COMPLETION_ARQC_ARQC:

                 // ------------------------------------------------------------------------------
                 // 1. terminal transmits online request to issuer through acquier (CALL-BACK)
                 // 2. terminal receives authorization response
                 // 3. IF   terminal cannot complete online
                 //         THEN go to IAC-DEFAULT
                 //    ELSE IF   "Issuer Authen Data" in response AND "AIP" supports Issuer Authen
                 //               THEN  terminal issues "EXTERNAL AUTHEN" command
                 //         ELSE  goto COMPLETION_ARC
                 // NOTE: a call-back function will be implemented externally.
                 // ------------------------------------------------------------------------------
                 
#ifdef PCI_AP
              if( EMVDC_OnlineProcessing( MSGID_FINANCIAL_REQ, p1, g_term_ARC, g_isu_authen_data, (UINT *)&g_isu_script_addr ) == ONL_Completed ) // completed online?
#else		 
              if( EMV_CallBack_OnlineProcessing() == emvOK )
#endif              
                   {
//                 i = g_term_ARC[0]*256 + g_term_ARC[1];
//                 /*
//                  * 20090305_Richard: EMV 4.2a, 2CO.030.00.02
//                  * Add recognizing issuer authentication respose code (IARC) less then 0x3030 as error IARC
//                  */
//                 if( (i == 0) || (i > ISU_ARC_MAX) || (i < ISU_ARC_APPROVED) ) // not present or incorrect
//                   flag_online_retry = TRUE;

                   // PATCH: PBOC2.0, 2006-02-15, 2CO.030
// 2006-09-20      if( (i != 0x3030) && (i != 0x3031) && (i != 0x3032) && (i != 0x3035) && (i != 0x3130) && (i != 0x3131) )
//                   goto TAA_DEFAULT;

        //         i = g_term_ARC[0]*256 + g_term_ARC[1];
        //
        //         if( (i == 0) || (i > ISU_ARC_MAX) || (i < ISU_ARC_APPROVED) ) // not present or incorrect
        //           {
        //           flag_online_retry = TRUE;
        //
        //           // repeat financial request again
        //           if( EMVDC_OnlineProcessing( MSGID_FINANCIAL_REQ, p1, g_term_ARC, g_isu_authen_data, (UINT *)&g_isu_script_addr ) == ONL_Completed ) // completed online?
        //             {
        //             i = g_term_ARC[0]*256 + g_term_ARC[1];
        //
        //             if( (i == 0) || (i > ISU_ARC_MAX) || (i < ISU_ARC_APPROVED) ) // not present or incorrect
        //               goto COMPLETION_ARC;
        //             }
        //           else // completed online has failed
        //             goto TAA_DEFAULT;
        //           }

                   g_online_complete = TRUE;
                   api_emv_PutDataElement( DE_TERM, ADDR_TERM_ARC, 2, g_term_ARC );

                   iLen = g_isu_authen_data[1]*256 + g_isu_authen_data[0];
                   if( iLen != 0 )
                    { // PATCH: 2003-10-02, add LEN(2) for ISU_AUTH_DATA(n)
                       // PATCH: 2005-06-10, move length of ISU_AUTH_DATA to ISU_AUTH_DATA_LEN
                       //                    Fixup GEN_AC requesting ISU_AUTH_DATA bug
                       //                    Problem: the length of ISU_AUTH_DATA will be inserted prior to real ARPC
                       //                    Example: if ISU_AUTH_DATA is 0x11~0x88,0x30,0x30, then CDOL2 will be 0x0A,0x00, 0x11-0x88
                       //                             other than 0x11-0x88,0x30,0x30, this will cause ARPC verification failed by ICC.
                     apk_WriteRamDataTERM( ADDR_ISU_AUTH_DATA_LEN, 2, (UCHAR *)&iLen ); // saving 2L
                     apk_WriteRamDataTERM( ADDR_ISU_AUTH_DATA, iLen, &g_isu_authen_data[2] ); // saving IAD[8~16]
                   }

                   if( (flag_online_retry == TRUE) && (online_retry != 0) )
                     goto COMPLETION_ARC;

                   if( iLen && (g_icc_AIP[0] & AIP0_ISSUER_AUTHEN) )
                     {
                     result = apk_ExternalAuthen( g_isu_authen_data );

                     if( (result == apkOutOfService) || (result == apkCondNotSatisfied) )
                       {
                       // reversal of the current online-approved transaction
//                     EMVDC_OnlineProcessing( MSGID_REVERSAL_REQ, p1, g_term_ARC, g_isu_authen_data, (UINT *)&g_isu_script_addr );
                       g_term_ARC[0] = 0;
                       g_term_ARC[1] = 0;

//                     return( emvFailed ); // terminate transaction
		       return( emvNotReady ); // real, need CALL-BANK
                       }

                     if( result != apkOK )
                       {
                       g_term_TVR[4] |= TVR4_ISSUER_AUTHEN_UNSUCCESS;
                       api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR+4, 1, &g_term_TVR[4] );
                       }

                     g_term_TSI[0] |= TSI0_ISSUER_AUTHEN_PERFORMED;
                     api_emv_PutDataElement( DE_TERM, ADDR_TERM_TSI, 1, &g_term_TSI[0] );
                     }

                // if( (flag_online_retry == TRUE) && (online_retry != 0) )
                //   goto COMPLETION_ARC;

                   // issuer scripts present?
                // if( (g_isu_script_temp[1]*256 + g_isu_script_temp[0]) != 0 )
                   apk_ReadRamDataTERM( g_isu_script_addr, 2, (UCHAR *)&iLen );
                   if( iLen != 0 )
                     goto COMPLETION_ISP1;
                   else
                     goto COMPLETION_ARC;
                   }
                 else // completed online has failed
                   {
                   goto TAA_DEFAULT;
                   }

            default://EMV 42a Charles 2009-03-06 2ck.017.00
               return( emvFailed ); // icc logic error, terminate transaction.

        //       return( emvFailed ); // icc logic error, terminate transaction.

                 iARC = ARC_OFFLINE_DECLINED;
                 return( emvOK );
            }

      // ---------------------------------------------------------------------
COMPLETION_TC:
      switch( cid & CID_AC_MASK )
            {
            case AC_AAC:  // CAA: reject

                 goto COMPLETION_AAC_AAC;

            case AC_AAR:  // CAA: refer

                 goto COMPLETION_ARQC_AAR;

            case AC_ARQC: // CAA: online

                 goto COMPLETION_ARQC_ARQC;

            case AC_TC:   // CAA: offline approved

                 goto COMPLETION_END;

            default:

        //       return( emvFailed ); // icc logic error, terminate transaction.

                 iARC = ARC_OFFLINE_DECLINED;
                 return( emvOK );
            }

      // ---------------------------------------------------------------------
      //                Issuer Script Processing (TAG 71)
      // ---------------------------------------------------------------------
COMPLETION_ISP1:

      // prior to issuing the final GENERATE AC command
//    if( apk_IssuerScriptProcessing( 0x71, g_isu_script_temp ) != apkOK )

      result = apk_IssuerScriptProcessing2( 0x71, g_isu_script_addr );

      if( result == apkNotReady ) // PATCH: 2003-08-03
        goto COMPLETION_ARC;      // script is not performed

      if( result != apkOK )
        {
        g_term_TVR[4] |= TVR4_SP_FAILED_BEFORE_FAC;
        api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR+4, 1, &g_term_TVR[4] );
        }

      g_term_TSI[0] |= TSI0_SCRIPT_PROCESS_PERFORMED;
      api_emv_PutDataElement( DE_TERM, ADDR_TERM_TSI, 1, &g_term_TSI[0] );

      goto COMPLETION_ARC;

      // ---------------------------------------------------------------------
      //                Issuer Script Processing (TAG 72)
      // ---------------------------------------------------------------------
//COMPLETION_ISP2:
//
//      apk_ReadRamDataTERM( g_isu_script_addr, 2, (UCHAR *)&iLen );
//      if( iLen == 0 )
//        goto COMPLETION_END2;
//
//      // after issuing the final GENERATE AC command
////    if( apk_IssuerScriptProcessing( 0x72, g_isu_script_temp ) != apkOK )
//      if( apk_IssuerScriptProcessing2( 0x72, g_isu_script_addr ) != apkOK )
//        {
//        g_term_TVR[4] |= TVR4_SP_FAILED_BEFORE_FAC;
//        api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR+4, 1, &g_term_TVR[4] );
//        }
//
//      g_term_TSI[0] |= TSI0_SCRIPT_PROCESS_PERFORMED;
//      api_emv_PutDataElement( DE_TERM, ADDR_TERM_TSI, 1, &g_term_TSI[0] );
//
//      goto COMPLETION_END2;

      // ---------------------------------------------------------------------
COMPLETION2_AAC:
      switch( cid & CID_AC_MASK )
            {
            case AC_AAC:  // CAA: reject

                 goto COMPLETION_END;

            case AC_TC:   // CAA: approved

                 if( p1 == AC_AAC )     // PATCH: 2005/05/09
                   goto COMPLETION_END; // treated as an AAC

                 if( iARC == ARC_DECLINED_UNABLE_ONLINE )
                   {
                   iARC = ARC_APPROVED_UNABLE_ONLINE;
                   goto COMPLETION_END;
                   }
                 else
                   {
                // COMPLETION_ISP2();
                // return( emvFailed );

                   goto COMPLETION_END; // treated as an AAC
                   }

            default:

        //       COMPLETION_ISP2();
        //       return( emvFailed ); // icc logic error, terminate transaction.

                 goto COMPLETION_END; // treated as an AAC
            }

      // ---------------------------------------------------------------------
COMPLETION2_TC:
      switch( cid & CID_AC_MASK )
            {
            case AC_AAC:  // CAA: reject

COMP2_TC_AAC:
                 // case 1: Y2 -> Z2
                 // case 2: Y3 -> Z3
                 // case 3: as a result of EXTERNAL PROCEDURE (eg. referral)

                 if( iARC == ARC_ONLINE_APPROVED )
                   {
                   iARC = ARC_ONLINE_DECLINED;

                   // reversal of the current online-approved transaction
//                 EMVDC_OnlineProcessing( MSGID_REVERSAL_REQ, p1, g_term_ARC, g_isu_authen_data, (UINT *)&g_isu_script_addr );
                   flag_reversal2 = TRUE;
                   }
                 else
                   {
                   if( iARC == ARC_APPROVED_UNABLE_ONLINE )
                     iARC = ARC_DECLINED_UNABLE_ONLINE;
                   }

                 if( flag_irefer == TRUE )
                   {
                   // PATCH: 2005-01-21
                   if( (auth_code[0] == 'Y') && ( auth_code[1] == '4') )
                     goto COMPLETION_END2;

                   // PATCH: 2003-07-10
                   //        retrieve original issuer response code
                   api_emv_GetDataElement( DE_TERM, ADDR_TERM_ARC, 2, auth_code );
                   }

                 goto COMPLETION_END;

            case AC_TC:   // CAA: offline approved

                 goto COMPLETION_END;

            default:

        //       COMPLETION_ISP2();
        //       return( emvFailed ); // icc logic error, terminate transaction.

                 goto COMP2_TC_AAC; // treated as an AAC
            }

      // ---------------------------------------------------------------------
      //                Authorization Response Code (from Issuer)
      // ---------------------------------------------------------------------
COMPLETION_ARC:

      i = g_term_ARC[0]*256 + g_term_ARC[1];
      switch( i ) // ARC code from issuer
            {
            case ISU_ARC_APPROVED:

                 // set ARC="Y2" (online approved)
                 iARC = ARC_ONLINE_APPROVED;
        //       TAA_SetARC( ARC_ONLINE_APPROVED ); // 2003-03-28

                 p1 = AC_TC;  // to approve
                 break;

            case ISU_ARC_REFERRAL1:
            case ISU_ARC_REFERRAL2:

                 // "Call Bank"
                 api_emv_PutDataElement( DE_TERM, ADDR_TERM_ARC, 2, g_term_ARC );
                 flag_irefer = TRUE;

                 goto COMPLETION_ARQC_AAR;

            default:

                 if( (flag_online_retry == TRUE) && (online_retry != 0) )
                   {
                   online_retry = 0;
                   goto COMPLETION_ARQC_ARQC;
                   }
                 /*
                  * 20090305_Richard: EMV 4.2a, 2CO.030.00.02
                  * Add recognizing issuer authentication respose code (IARC) less then 0x3030 as error IARC
                  */
                 if( (i == 0) || (i > ISU_ARC_MAX) || (i < ISU_ARC_APPROVED) ) // not present or incorrect
                   {
                   // reversal of the current online-approved transaction
//                 EMVDC_OnlineProcessing( MSGID_REVERSAL_REQ, p1, g_term_ARC, g_isu_authen_data, &g_isu_script_addr );

                   flag_reversal = TRUE;
                   }

                 if( flag_online_retry == TRUE ) // ever online retry?
                   {
                   flag_online_retry = FALSE;

                   goto TAA_DEFAULT; // yes, unable to go online
                   }
                 else
                   {
                   // set ARC="Z2" (online declined)
                   iARC = ARC_ONLINE_DECLINED;
           //      TAA_SetARC( ARC_ONLINE_DECLINED ); // 2003-03-28
                   }

                 cda = 0;
                 p1 = AC_AAC; // to decline
            }

//    if( iLen == ISU_ARC_APPROVED )
//      {
//      // set ARC="Y2" (online approved)
//      iARC = ARC_ONLINE_APPROVED;
//
//      p1 = AC_TC;  // to approve
//      }
//    else
//      {
//      // set ARC="Z2" (online declined)
//      iARC = ARC_ONLINE_DECLINED;
//
//      p1 = AC_AAC; // to decline
//      }

      goto SECOND_ISSUANCE;

      // ---------------------------------------------------------------------
COMPLETION_END:

//  UI_ClearScreen();
//  TL_DispHexWord(0,0,iARC);
//  UI_WaitKey();
//  TL_DumpHexData(0,0,2,auth_code);

      TAA_COMPLETION_ISP2(); // check for issuer script TAG=72

//    TAA_FinancialConfirm( iARC, p1 );

COMPLETION_END2:
      // update final ARC

      if( (iARC == ARC_ONLINE_APPROVED) || (iARC == ARC_ONLINE_DECLINED) )
        {
        api_emv_PutDataElement( DE_TERM, ADDR_TERM_ARC, 2, g_term_ARC );

        g_term_ARC[0] = (iARC & 0xff00) >> 8;
        g_term_ARC[1] = iARC & 0x00ff;
        arc[0] = g_term_ARC[0];
        arc[1] = g_term_ARC[1];

        if( flag_reversal2 == TRUE )
          return( emvNotReady );
        else
          return( emvOK );
        }

//      if( iARC == 0x5934 ) // Y4 ?
//        {
//        arc[0] = 'Y';
//        arc[1] = '4';
//        g_term_ARC[0] = 'Y';
//        g_term_ARC[1] = '4';
//
//        return( emvOK );
//        }

      if( iARC != 0 )
        {
        g_term_ARC[0] = (iARC & 0xff00) >> 8;
        g_term_ARC[1] = iARC & 0x00ff;
        }
      else
        {
        g_term_ARC[0] = auth_code[0];
        g_term_ARC[1] = auth_code[1];
        }

//    g_term_TSI[0] |= TSI0_CRM_PERFORMED;
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_TSI, 1, &g_term_TSI[0] );

//COMPLETION_EXIT:

      arc[0] = g_term_ARC[0];
      arc[1] = g_term_ARC[1];
      api_emv_PutDataElement( DE_TERM, ADDR_TERM_ARC, 2, g_term_ARC );

      return( emvOK );

      // ---------------------------------------------------------------
      // CDA recovery failed after the 2'nd issuance
      // ---------------------------------------------------------------
COMPLETION_EXIT2:

      arc[0] = g_term_ARC[0];
      arc[1] = g_term_ARC[1];
      api_emv_PutDataElement( DE_TERM, ADDR_TERM_ARC, 2, g_term_ARC );

      g_term_ARC[0] = 0;
      g_term_ARC[1] = 0;

   // return( emvAborted ); // TA only
      return( emvFailed );  // real value
}

