//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2 KERNEL API                                          **
//**  PRODUCT  : AS320-AGR                                                  **
//**                                                                        **
//**  FILE     : L2API_00.C                                                 **
//**  MODULE   : api_emvk_xxx()                                             **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2009/06/04                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2009 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include "POSAPI.h"
#include "GDATA.h"
#include "EMVAPI.h"
#include "EMVAPK.h"
#include "EMVDC.h"
#include "UI.h"
//#include "EMVDCMSG.H"
#include "API_EMVK.H"
#include "EMVCB.H"
#include <string.h>

extern	UCHAR	PED_PutWaveIEK( UCHAR *imek, UCHAR *iaek );
extern	UCHAR	PED_GetWaveIMEK( UCHAR *imek );
extern	UCHAR	PED_GetWaveIAEK( UCHAR *iaek );

//void	api_emvk_SetAmount( UCHAR *amt, UCHAR *sc );

UCHAR	EMVK_CandidateList[MAX_CANDIDATE_NAME_CNT][CANDIDATE_NAME_LEN];

// ---------------------------------------------------------------------------
// FUNC  : Request to do ONLINE transaction.
// INPUT : none.
// OUTPUT: arc - an2, authorization response code. (Tag 8A, an2, fixed 2 bytes - ISO8583 Field_39)
//	   air - an6, Authorisation Identifier Response. (Tag 89, fixed 6 bytes - ISO8583 Field_38)
//         rmsg- the response message of ISO8583 Field_55 from issuer,
//		 containing:
//               LLLVAR, LLL=length of VAR(TLVs) <=255 bytes
//                       VAR=IAD[] + IST[] as defined below.
//                           iad - issuer authentication data.
//                           ist - issuer script template 1 and 2.
// RETURN: emvOK     - online finished.
//         emvFailed - unable to go online.
// NOTE  : This function is to be implemented from AP level.
// ---------------------------------------------------------------------------
//UCHAR	EMV_CB_OnlineProcessing( UCHAR *arc, UCHAR *air, UCHAR *rmsg )
//{
//	return( emvOK );
//}

// ---------------------------------------------------------------------------
// FUNC  : Request to do REFERRAL transaction.
// INPUT : none.
// OUTPUT: none.
// RETURN: emvOK      - approved.
//         emvFailed  - declined.
// NOTE  : This function is to be implemented from AP level.
// ---------------------------------------------------------------------------
//UCHAR	EMV_CB_ReferralProcessing( void )
//{
//	return( emvOK );
//}

// ---------------------------------------------------------------------------
// FUNC  : To find the most recent log entry with the same specified PAN.
// INPUT : pan - the PAN to be compared. (Tag 5A, cn19, fixed 10 bytes)
// OUTPUT: amt - the authorized amount for the found PAN. (Tag 9F02, n12, fixed 6 bytes)
// RETURN: emvOK     - found.
//         emvFailed - not found.
// NOTE  : This function is to be implemented from AP level.
// ---------------------------------------------------------------------------
//UCHAR	EMV_CB_FindMostRecentPAN( UCHAR *pan, UCHAR *amt )
//{
//	return( emvOK );
//}

// ---------------------------------------------------------------------------
// FUNC  : To obtain transaction amount.
// INPUT : none.
// OUTPUT: type - transaction type. (n2, fixed 1 byte, first two digits of ISO8583-Processing Code)
//	   sc   - transaction sequence number. (n8, fixed 4 bytes)
//		  maintained by application program and incremented by one for each transaction.
//         amt  - transaction amount entered. (n12, fixed 6 bytes)
//		  the first 5 bytes are integer part, the last byte is decimal part.
// RETURN: emvOK
//	   emvFailed (aborted)
// NOTE  : This function is to be implemented from AP level.
//	   This function is called if the "Transaction Amount" is not available
//	   before calling "api_emvk_DoTransaction()".
//	   This function shall prompt the "ENTER AMOUNT" to request to enter transaction amount.
// ---------------------------------------------------------------------------
//UCHAR	EMV_CB_GetAmount( UCHAR *amt, UCHAR *sc )
//{
//	return( emvOK );
//}

// ---------------------------------------------------------------------------
// FUNC  : To show a list and request for selecting an application.
// INPUT : list     - a linear fixed length 2-D array (16x18 bytes)
//                    format: LINK[1] LEN1[1] NAME1[16]
//                            LINK[1] LEN2[1] NAME2[16]...
//                            LINK[1] LEN16[1] NAME16[16]
//                            (1) LINK[1]: (reference only)
//                            (2) LEN[1] : acutual length of NAME
//                                         0 = the bottom of list.
//                            (3) NAME[] : contains application label name.
//         list_cnt - number of items in the list.
// OUTPUT: none.
// RETURN  : 0xFF   - if aborted or time out.
//         : others - item number of the selection. (0..n)
// ---------------------------------------------------------------------------
//UCHAR	EMV_CB_SelectApplication( UCHAR list_cnt, UCHAR *list )
//{
//	return( 255 );
//}

// ---------------------------------------------------------------------------
// FUNC  : Show "TRY AGAIN" message in local language. (FONT1, MIDWAY)
// INPUT : none.
// OUTPUT: none.
// RETURN: none.
// NOTE  : This function is to be implemented from AP level.
// ---------------------------------------------------------------------------
//void	EMV_CB_ShowMsg_TRY_AGAIN( void )
//{
//}

// ---------------------------------------------------------------------------
// FUNC  : Show "NOT ACCEPTED" message in local language. (FONT1, MIDWAY)
// INPUT : none.
// OUTPUT: none.
// RETURN: none.
// NOTE  : This function is to be implemented from AP level.
// ---------------------------------------------------------------------------
//void	EMV_CB_ShowMsg_NOT_ACCEPTED( void )
//{
//}

// ---------------------------------------------------------------------------
// FUNC  : Show "PLEASE WAIT" message in local language. (FONT1, MIDWAY)
// INPUT : none.
// OUTPUT: none.
// RETURN: none.
// NOTE  : This function is to be implemented from AP level.
// ---------------------------------------------------------------------------
//void	EMV_CB_ShowMsg_PLEASE_WAIT( void )
//{
//}

// ---------------------------------------------------------------------------
// FUNC  : Show "SELECT" message in local language at the first row. (FONT1, RIGHTMOST)
// INPUT : none.
// OUTPUT: none.
// RETURN: none.
// NOTE  : This function is to be implemented from AP level.
// ---------------------------------------------------------------------------
//void	EMV_CB_ShowMsg_SELECT( void )
//{
//}

// ---------------------------------------------------------------------------
// FUNC  : Show "INCORRECT PIN" message in local language at the first 3 rows. (FONT1)
// INPUT : none.
// OUTPUT: none.
// RETURN: none.
// NOTE  : This function is to be implemented from AP level.
// ---------------------------------------------------------------------------
//void	EMV_CB_ShowMsg_INCORRECT_PIN( void )
//{
//}

// ---------------------------------------------------------------------------
// FUNC  : Show "LAST PIN TRY" message in local language at the first 3 rows. (FONT1)
// INPUT : none.
// OUTPUT: none.
// RETURN: none.
// NOTE  : This function is to be implemented from AP level.
// ---------------------------------------------------------------------------
//void	EMV_CB_ShowMsg_LAST_PIN_TRY( void )
//{
//}
#ifndef PCI_AP
// ---------------------------------------------------------------------------
// FUNC  : Show "TRY AGAIN" message at 4'nd row. (FONT1, MIDWAY)
// INPUT : none.
// OUTPUT: none.
// RETURN: none.
// ---------------------------------------------------------------------------
void	EMV_CallBack_ShowMsg_TRY_AGAIN( void )
{
const	UCHAR msg_TRY_AGAIN[] = {"TRY AGAIN"};


	if( g_user_define == TRUE )
	  EMV_CB_ShowMsg_TRY_AGAIN();
	else
	  UI_PutMsg( 3, COL_MIDWAY, FONT1+attrCLEARWRITE, sizeof(msg_TRY_AGAIN)-1, (UCHAR *)msg_TRY_AGAIN );
}

// ---------------------------------------------------------------------------
// FUNC  : Show "NOT ACCEPTED" message at 4'nd row. (FONT1, MIDWAY)
// INPUT : none.
// OUTPUT: none.
// RETURN: none.
// ---------------------------------------------------------------------------
void	EMV_CallBack_ShowMsg_NOT_ACCEPTED( void )
{
const	UCHAR msg_NOT_ACCEPTED[] = {"NOT ACCEPTED"};


	if( g_user_define == TRUE )
	  EMV_CB_ShowMsg_NOT_ACCEPTED();
	else
	  UI_PutMsg( 3, COL_MIDWAY, FONT1+attrCLEARWRITE, sizeof(msg_NOT_ACCEPTED)-1, (UCHAR *)msg_NOT_ACCEPTED );
}

// ---------------------------------------------------------------------------
// FUNC  : Show "PLEASE WAIT" message at 4'nd row. (FONT1, MIDWAY)
// INPUT : none.
// OUTPUT: none.
// RETURN: none.
// ---------------------------------------------------------------------------
void	EMV_CallBack_ShowMsg_PLEASE_WAIT( void )
{
const	UCHAR msg_PLEASE_WAIT[] = {"PLEASE WAIT"};


	if( g_user_define == TRUE )
	  EMV_CB_ShowMsg_PLEASE_WAIT();
	else
	  UI_PutMsg( 3, COL_MIDWAY, FONT1+attrCLEARWRITE, sizeof(msg_PLEASE_WAIT)-1, (UCHAR *)msg_PLEASE_WAIT );
}

// ---------------------------------------------------------------------------
// FUNC  : Show "SELECT" message at (0,0) row. (FONT1, LEFTMOST)
// INPUT : none.
// OUTPUT: none.
// RETURN: none.
// ---------------------------------------------------------------------------
void	EMV_CallBack_ShowMsg_SELECT( void )
{
const	UCHAR msg_SELECT[] = {"SELECT"};


	if( g_user_define == TRUE )
	  EMV_CB_ShowMsg_SELECT();
	else
	  UI_PutMsg( 0, COL_LEFTMOST, FONT1+attrCLEARWRITE, sizeof(msg_SELECT)-1, (UCHAR *)msg_SELECT );
}

// ---------------------------------------------------------------------------
// FUNC  : To show a list and request for selecting an application.
// INPUT : list     - a linear fixed length 2-D array (16x18 bytes)
//                    format: LINK[1] LEN1[1] NAME1[16]
//                            LINK[1] LEN2[1] NAME2[16]...
//                            LINK[1] LEN16[1] NAME16[16]
//                            (1) LINK[1]: (reference only)
//                            (2) LEN[1] : acutual length of NAME
//                                         0 = the bottom of list.
//                            (3) NAME[] : contains application label name.
//         list_cnt - number of items in the list.
// OUTPUT: none.
// RETURN  : -1     - if aborted or time out.
//         : others - item number of the selection. (0..n)
// ---------------------------------------------------------------------------
UCHAR	EMV_CallBack_SelectApplication( UCHAR list_cnt, UCHAR *list )
{
UCHAR	buffer[8];


	if( g_user_define == TRUE )
	  return( EMV_CB_SelectApplication( list_cnt, list ) );
	else
	  {
	  EMV_CallBack_ShowMsg_SELECT();

	  // wait for attendant's selection for 60 seconds
	  buffer[0] = 2;
	  buffer[1] = 4;
	  buffer[2] = list_cnt;
	  buffer[3] = CANDIDATE_NAME_LEN;
	  buffer[4] = 1;
	  buffer[5] = FONT0;
	  return( TL_ListBox( 0, &buffer[0], list, 60 ) );	// 60 sec timeout
	  }
}

// ---------------------------------------------------------------------------
// FUNC  : To obtain transaction type and amount.
// INPUT : none.
// OUTPUT: amt  - transaction amount. (n12)
//
// RETURN: emvOK
//	   emvFailed (aborted)
// ---------------------------------------------------------------------------
#ifdef	_EMVK_NEW_FUNC_
UCHAR	EMV_CallBack_GetAmount( void )
{
UCHAR	amt[6];
UCHAR	sc[4];


	apk_ReadRamDataTERM( ADDR_TERM_AMT_AUTH_N, 6, amt );
	if( TL_memcmpc( amt, 0x00, 6 ) == 0 )	// amount available?
	  {
	  if( EMV_CB_GetAmount( amt, sc ) !=emvOK )  // "ENTER AMOUNT" ?
	    return( emvFailed ); // aborted
	  }
	  
	// sav "amt" and "sc" to kernel
	api_emvk_SetAmount( amt, sc );
	return( emvOK );
}
#endif

// ---------------------------------------------------------------------------
// FUNC  : Call back function to do ONLINE processing.
// INPUT : none.
// OUTPUT:
// REF   : g_term_ARC
//         g_isu_authen_data
//         g_isu_script_addr
// RETURN: emvOK
//         emvFailed
// ---------------------------------------------------------------------------
UCHAR EMV_CallBack_OnlineProcessing( void )
{
UINT  iLen;
UINT  iPileLen;
UINT  iAddr;
UINT  *isa;
UCHAR len[4];
UCHAR *ptrobj;
UCHAR *ptrtmp;
UCHAR tlv[261];
UCHAR *ptriad;
UCHAR air[6];   // field_38: Authorisation Identifier Response (approval code)

      // -------------------------------------------------------------------
      // Go Online
      //
      // g_obuf: LLL DATA
      //         LLL = length of response data in Field 55.
      //               attribute & size: BCD, 2 bytes.
      //         DATA= contents of Field 55:
      //               Issuer Script Template 1 (71-L-V)
      //               Issuer Script Template 2 (72-L-V)
      //               Issuer Authentication Data (91-L-V)
      // -------------------------------------------------------------------
      isa = &g_isu_script_addr;    //
      *isa = ADDR_ISU_SCRIPT_TEMP; // to signal kernel where the scripts are stored

      memset( air, 0x00, 6 );
      printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
      if( EMV_CB_OnlineProcessing( g_term_ARC, air, g_obuf ) ==  emvOK )
        {
          printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
        // put Authorisation Code (Tag=89)
        api_emv_PutDataElement( DE_TERM, ADDR_ISU_AUTH_CODE, 6, air );
        printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
        if( (g_obuf[0] == 0) && (g_obuf[1] == 0) ) // no Field 55 data available
          {
          g_isu_authen_data[0] = 0;
          g_isu_authen_data[1] = 0;

          iLen = 0;
          api_emv_PutDataElement( DE_TERM, ADDR_ISU_SCRIPT_TEMP, 2, (UCHAR *)&iLen );

          return( emvOK );
          }
printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
        // (1) concatenate issuer authentication data (91-L-V)
        TL_bcd2hex( 2, &g_obuf[0], len ); // LLL in binary
        TL_SwapData( 4, len );            //
        iPileLen = 0;
        ptriad = g_isu_authen_data;
        ptrtmp = &g_obuf[2];
        ptriad += 2; // ptr to data field
printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
        while(1)
             {
             ptrobj = apk_GetBERTLV( len, ptrtmp, tlv );

             if( (ptrobj == 0) || (ptrobj == (UCHAR *)-1) ) // EOF or invalid
               break; // done

             if( (tlv[0] == 0x00) && (tlv[1] == 0x91) ) // IAD Tag?
               {
               iLen = tlv[3]*256 + tlv[2];
               memmove( ptriad, &tlv[4], iLen ); // concatenate data V
               ptriad += iLen;
               iPileLen += iLen;
               }

             if( (len[1]*256 + len[0]) == 0 ) // remaining length
               break; // done

             ptrtmp = ptrobj; // next
             }
        g_isu_authen_data[0] = iPileLen & 0x00ff;
        g_isu_authen_data[1] = (iPileLen & 0xff00) >> 8;
printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
        // (2) concatenate the scripts (71-L-V or 72-L-V)
        TL_bcd2hex( 2, &g_obuf[0], len ); // LLL in binary
        TL_SwapData( 4, len );            //
        iPileLen = 0;
        ptrtmp = &g_obuf[2];
        iAddr = ADDR_ISU_SCRIPT_TEMP + 2; // ptr to data field
printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
        while(1)
             {
             ptrobj = apk_GetBERTLV( len, ptrtmp, tlv );

             if( (ptrobj == 0) || (ptrobj == (UCHAR *)-1) ) // EOF or invalid
               break; // done

             if( ((tlv[0] == 0x00) && (tlv[1] == 0x71)) ||  // Script Tag 71 or 72?
                 ((tlv[0] == 0x00) && (tlv[1] == 0x72)) )
               {
               iLen = tlv[3]*256 + tlv[2];
               if( (tlv[2] & 0x80) == 0 )
                 {
                 tlv[3] = tlv[2]; // convert from 00 71 L1 L2 V
                 tlv[2] = tlv[1]; //         to   -  -  71 L1 V
                 api_emv_PutDataElement( DE_TERM, iAddr, iLen+2, &tlv[2] );

                 iAddr += iLen + 2;
                 iPileLen += iLen + 2;
                 }
               else
                 {
                 api_emv_PutDataElement( DE_TERM, iAddr, iLen+3, &tlv[1] );

                 iAddr += iLen + 3;
                 iPileLen += iLen + 3;
                 }
               }

             if( (len[1]*256 + len[0]) == 0 ) // remaining length
               break; // done

             ptrtmp = ptrobj; // next
             }printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
        api_emv_PutDataElement( DE_TERM, ADDR_ISU_SCRIPT_TEMP, 2, (UCHAR *)&iPileLen );

//  UI_ClearScreen();
//  api_emv_GetDataElement( DE_TERM, ADDR_ISU_SCRIPT_TEMP, 64, g_ibuf );
//  TL_DumpHexData(0,0,64, g_ibuf);
//  TL_DumpHexData(0,0,18, g_isu_authen_data);

        return( emvOK );
        }
      else
        return( emvFailed ); // unable to go online
}

// ---------------------------------------------------------------------------
// FUNC  : Call back function to do REFERRAL processing.
// INPUT : none.
// OUTPUT:
// RETURN: REF_Approved
//         REF_Declined
//	   REF_ForcedAbort
// ---------------------------------------------------------------------------
UCHAR EMV_CallBack_ReferralProcessing( void )
{
UCHAR result;

//    switch( EMV_CB_ReferralProcessing() )
//          {
//          case emvOK:
//               return( REF_Approved );
//
//          case emvFailed:
//               return( REF_Declined );
//          }

      // PATCH: 2003-09-25
      result = EMV_CB_ReferralProcessing();

      if( api_ifm_present( g_dhn_icc ) != apiReady ) // card present?
        return( REF_ForcedAbort );

      switch( result )
            {
            case emvOK:
                 return( REF_Approved );

            case emvFailed:
                 return( REF_Declined );
            }
}
#endif
// ---------------------------------------------------------------------------
// FUNC  : To get the special ICC data element by using GET_DATA command.
// INPUT : tag1 - the 1'st byte of word tag (0 for single tag).
//         tag2 - the 2'nd byte of word tag.
// OUTPUT: data - 2L-V, the returned data object.
// RETURN: emvOK     - found.
//         emvFailed - not found.
// ---------------------------------------------------------------------------
UCHAR EMVK_GetDataCommand( UCHAR tag1, UCHAR tag2, UCHAR *data )
{
UINT  iLen;
UCHAR buf[64];

      if( apdu_GET_DATA( tag1, tag2, buf ) == apiOK )
        {
        iLen = buf[1]*256 + buf[0];
        if( (buf[2] != tag1) || (buf[3] != tag2) ||
            (buf[iLen] != 0x90) || (buf[iLen+1] != 0x00) )
          return( emvFailed );
        else
          {
          data[0] = buf[4];
          data[1] = 0;
          memmove( &data[2], &buf[5], data[0] );

          return( emvOK );
          }
        }
      else
        return( emvFailed );
}

// ---------------------------------------------------------------------------
// FUNC  : To get the contents of data element (of icc or terminal) by tag.
// INPUT : tag1 - the 1'st byte of word tag (0 for single tag).
//         tag2 - the 2'nd byte of word tag.
// OUTPUT: data - LEN[2] Data[~258], the contents of the specified tag(s).
// RETURN: emvOK     - found.
//         emvFailed - not found.
// ---------------------------------------------------------------------------
UCHAR api_emvk_GetDataElement( UCHAR tag1, UCHAR tag2, UCHAR *data )
{
UINT  i;
UINT  iLen;
UCHAR de[34];
UCHAR index;

      // check PRIVATE data element for CTCB
      //       9F41: Transaction Sequence Counter (n6, EMVK=n8)
      //       9F53: Transaction Category Code (an1) --> created at XCONSTP1.C
      //       DFE1: TC Value for the 2'nd issuance (b8)
      //       DFE2: The 1'st Issuer Script Results (b5) -- CTCB
      //       DF91: The 1'st Issuer Script Results (b5) -- NCCC
      //       DF01: Card Verification Results -- CVR (var.)
      //       DF02: Issuer Script Template (var.)
      //       9F52: Application Default Action -- ADA (b2)
      //       9F56: Issuer Authentication Indicator (b1)
      //       9F55: Geographic Indicator (b1)
      //       9F06: AID (terminal)
      //       91  : Issuer Authentication Data(2L-V)
      //       DF8A: Max number of decimals according to the currency code
      //       DFEE: Terminal Entry Capability

      // --- Transaction Sequence Counter ---
      if( (tag1 == 0x9F) && (tag2 == 0x41) )
        {
        data[0] = 3; // length
        data[1] = 0; //
        apk_ReadRamDataTERM( ADDR_TERM_TX_SC+1, 3, &data[2] ); // ignore MSB

        return( emvOK );
        }

//    if( (tag1 == 0x9F) && (tag2 == 0x53) )    // REMOVED: 2003-08-21
//      {
//      data[0] = 2; // length
//      data[1] = 0; //
//      apk_ReadRamDataTERM( ADDR_TERM_MCC, 2, &data[2] );
//
//      return( emvOK );
//      }

      // --- TC Value ---
      if( (tag1 == 0xDF) && (tag2 == 0xE1) )
        {
        apk_ReadRamDataICC( ADDR_ICC_AC, 10, data );

        return( emvOK );
        }

      // --- Issuer Script Results ---
      if( ((tag1 == 0xDF) && (tag2 == 0xE2)) || ((tag1 == 0xDF) && (tag2 == 0x91)) || ((tag1 == 0x9F) && (tag2 == 0x5B)) )
        {
//      apk_ReadRamDataTERM( ADDR_TERM_ISR, 1, de );
//      apk_ReadRamDataTERM( ADDR_TERM_ISR+1, 5, &data[2] );
//      data[0] = 5;
//      data[1] = 0;
//
//      if( de[0] == 0 )
//        memset( &data[2], 0x00, 5 );
//
//      return( emvOK );

        apk_ReadRamDataTERM( ADDR_TERM_ISR, 1, de );
        iLen = de[0]*5;

        if( iLen == 0 )
          {
          memset( &data[2], 0x00, 5 );
//        iLen = 5;
          iLen = 0; // PATCH: 2005/07/25
          }
        else
          apk_ReadRamDataTERM( ADDR_TERM_ISR+1, iLen, &data[2] );

        data[0] = iLen & 0x00ff;
        data[1] = (iLen & 0xff00) >> 8;

        return( emvOK );
        }

      // --- Card Verification Results ---
      if( (tag1 == 0xDF) && (tag2 == 0x01) )
        {
        // Issuer Application Data (9F10) -- optional
        // Format: Length_Of_Indicator(1)
        //            Derivation_Key_Index(1)
        //            Cryptogram_Version_Number(1)
        //            Card_Verification_Results(n)
        //               Length_Of_Indicator(1)
        //                  CVR(n-1)
        apk_ReadRamDataICC( ADDR_ICC_ISU_AP_DATA, 34, de );
        if( (de[1]*256 + de[0]) != 0 )
          {
          if( de[2] < 3 )               // PATCH: 2010-10-23
            return( emvFailed );        // if len<3, then no CVR

          if( de[2+3] == 0 )
            return( emvFailed );

          data[0] = de[2+3];
          data[1] = 0;
          memmove( &data[2], &de[2+4], data[0] );

          return( emvOK );
          }
        else
          return( emvFailed );
        }

      // --- Issuer Script Template ---
      if( (tag1 == 0xDF) && (tag2 == 0x02) )
        {
        api_emv_GetDataElement( DE_TERM, ADDR_ISU_SCRIPT_TEMP, 2, (UCHAR *)&iLen );
        if( iLen != 0 )
          {
          api_emv_GetDataElement( DE_TERM, ADDR_ISU_SCRIPT_TEMP, iLen+2, data  );
          return( emvOK );
          }
        else
          return( emvFailed );
        }

      // --- Application Default Action ---
      if( (tag1 == 0x9F) && (tag2 == 0x52) )
        return( EMVK_GetDataCommand( tag1, tag2, data ) );

      // --- Issuer Authentication Indicator --
      if( (tag1 == 0x9F) && (tag2 == 0x56) )
        return( EMVK_GetDataCommand( tag1, tag2, data ) );

      // --- Geographic Indicator --
      if( (tag1 == 0x9F) && (tag2 == 0x55) )
        return( EMVK_GetDataCommand( tag1, tag2, data ) );

      // --- Selected AID (terminal) ---
      if( (tag1 == 0x9F) && (tag2 == 0x06) )
        {
        apk_ReadRamDataICC( ADDR_SELECTED_AID, SELECTED_AID_LEN, de ); // LEN(1) + AID(n)
        if( de[0] != 0 )
          {
          data[0] = de[0];
          data[1] = 0;
          memmove( &data[2], &de[1], de[0] );
          return( emvOK );
          }
        else
          return( emvFailed );
        }

      // --- Issuer Authentication Data ---
      if( (tag1 == 0x00) && (tag2 == 0x91) )
        {                                                      // PATCH: 2005-06-10
        apk_ReadRamDataTERM( ADDR_ISU_AUTH_DATA_LEN, 18, de ); // 2L-V(16)
        if( (de[1]*256 + de[0]) != 0 )
          {
          memmove( data, de, de[0]+2 );
          return( emvOK );
          }
        else
          return( emvFailed );
        }

      // --- Max number of decimals according to the currency code ---
      if( (tag1 == 0xDF) && (tag2 == 0x8A) )
        {
        apk_ReadRamDataTERM( ADDR_TERM_MAX_DECIMAL, 3, de ); // 2L-V(1)
        if( (de[1]*256 + de[0]) != 0 )
          {
          memmove( data, de, de[0]+2 );
          return( emvOK );
          }
        else
          return( emvFailed );
        }

      // --- Terminal Entry Capability ---
      if( (tag1 == 0xDF) && (tag2 == 0xEE) )
        {
        apk_ReadRamDataTERM( ADDR_TERM_ENTRY_CAP, 3, de ); // 2L-V(1)
        if( (de[1]*256 + de[0]) != 0 )
          {
          memmove( data, de, de[0]+2 );
          return( emvOK );
          }
        else
          return( emvFailed );
        }

      // --------------------------------------------------------------
      // search from EMV ICC source
      // --------------------------------------------------------------
      index = apk_ScanIDE( tag1, tag2, de );

      i = de[3];  // max length of the data element
      if( i == 0 )
        i = 256;

      if( index != 255 )
        {
        // load data element
        apk_ReadRamDataICC( TL_IDE_GetAddr(index), i+2, data ); // including LL

        return( emvOK );
        }

      // --------------------------------------------------------------
      // search from EMV TERMINAL source
      // --------------------------------------------------------------
      index = Scan_TDE( tag1, tag2, de );

      i = de[3];  // max length of the data element

      if( index != 255 )
        {
        // load data element
        data[0] = i; // length
        data[1] = 0; //
        apk_ReadRamDataTERM( TL_TDE_GetAddr(index), i, &data[2] ); // value

        return( emvOK );
        }
      else
        return( emvFailed );
}

// ---------------------------------------------------------------------------
// FUNC  : To get the contents of data element for the terminal by tag.
// INPUT : tag1 - the 1'st byte of word tag (0 for single tag).
//         tag2 - the 2'nd byte of word tag.
//         index- 0..n.
// OUTPUT: data - LEN[2] Data[~258], the contents of the specified tag(s).
// RETURN: emvOK     - found.
//         emvFailed - not found.
// ---------------------------------------------------------------------------
UCHAR api_emvk_GetTmsDataElement( UCHAR index, UCHAR tag1, UCHAR tag2, UCHAR *data )
{
UINT  iTag;
UINT  iLen;
UCHAR sbuf[64];

//    apk_ReadRamDataTERM( ADDR_TERM_TMS_EMV, 1, sbuf );  // EMV para downloaded?
//    if( sbuf[0] == 0 )
//      return( emvFailed );

      iTag = tag1*256 + tag2;

      switch( iTag )
            {
            case 0x9F06: // AID

                 apk_ReadRamDataTERM( ADDR_TERM_AID_01+index*TERM_AID_LEN, TERM_AID_LEN, sbuf );
                 if( sbuf[0] == 0 )
                   return( emvFailed );
                 else
                   {
                   sbuf[0] -= 1;
                   data[0] = sbuf[0];
                   data[1] = 0;
                   memmove( &data[2], &sbuf[2], sbuf[0] );
                   return( emvOK );
                   }

            case 0xDFEE: // terminal entry capability

                 apk_ReadRamDataTERM( ADDR_TERM_ENTRY_CAP, 3, data );
                 if( data[0] == 0 )
                   return( emvFailed );
                 else
                   return( emvOK );

            case 0x9F35: // terminal type

                 apk_ReadRamDataTERM( ADDR_TERM_TYPE, 1, &data[2] );
                 data[0] = 1;
                 data[1] = 0;
                 return( emvOK );

            case 0x9F33: // terminal capability

                 apk_ReadRamDataTERM( ADDR_TERM_CAP_01+index*TERM_CAP_LEN, TERM_CAP_LEN, &data[2] );
                 data[0] = TERM_CAP_LEN;
                 data[1] = 0;

                 return( emvOK );

            case 0x9F40: // additional terminal capability

                 apk_ReadRamDataTERM( ADDR_TERM_ADD_CAP_01+index*TERM_ADD_CAP_LEN, TERM_ADD_CAP_LEN, &data[2] );
                 data[0] = TERM_ADD_CAP_LEN;
                 data[1] = 0;

                 return( emvOK );

            case 0x9F15: // merchant category code

                 apk_ReadRamDataTERM( ADDR_TERM_MCC_01+index*MCC_LEN, MCC_LEN, &data[2] );
                 data[0] = MCC_LEN;
                 data[1] = 0;

                 return( emvOK );

            case 0x9F53: // transaction category code

                 apk_ReadRamDataTERM( ADDR_TERM_TCC, 1, &data[2] );
                 data[0] = 1;
                 data[1] = 0;
                 return( emvOK );

            case 0x9F1A: // terminal country code

                 apk_ReadRamDataTERM( ADDR_TERM_CNTR_CODE, 2, &data[2] );
                 data[0] = 2;
                 data[1] = 0;
                 return( emvOK );

            case 0x9F09: // application version number

                 apk_ReadRamDataTERM( ADDR_AVN_01+index*AVN_LEN, AVN_LEN, &data[2] );
                 data[0] = AVN_LEN;
                 data[1] = 0;
                 return( emvOK );

            case 0x5F2A: // transaction currency code

                 apk_ReadRamDataTERM( ADDR_TERM_TX_CC_01+index*TX_CC_LEN, TX_CC_LEN, &data[2] );
                 data[0] = TX_CC_LEN;
                 data[1] = 0;
                 return( emvOK );

            case 0x5F36: // transaction currency exponent

                 apk_ReadRamDataTERM( ADDR_TERM_TX_CE_01+index*TX_CE_LEN, TX_CE_LEN, &data[2] );
                 data[0] = TX_CE_LEN;
                 data[1] = 0;
                 return( emvOK );

            case 0x9F1B: // terminal floor limit

                 apk_ReadRamDataTERM( ADDR_TFL_01+index*TFL_LEN, TFL_LEN, &data[2] );
                 data[0] = TFL_LEN;
                 data[1] = 0;
                 return( emvOK );

            case 0xDF89: // max target percentage for BRS

                 apk_ReadRamDataTERM( ADDR_TERM_BRS_MTP_01+index*BRS_MTP_LEN, BRS_MTP_LEN, &data[2] );
                 data[0] = BRS_MTP_LEN;
                 data[1] = 0;
                 return( emvOK );

            case 0xDF88: // target percent for random selection

                 apk_ReadRamDataTERM( ADDR_TERM_RS_TP_01+index*RS_TP_LEN, RS_TP_LEN, &data[2] );
                 data[0] = RS_TP_LEN;
                 data[1] = 0;
                 return( emvOK );

            case 0xDF87: // threshold value for BRS

                 apk_ReadRamDataTERM( ADDR_TERM_BRS_THRESHOLD_01+index*BRS_THRESHOLD_LEN, BRS_THRESHOLD_LEN, &data[2] );
                 data[0] = BRS_THRESHOLD_LEN;
                 data[1] = 0;
                 return( emvOK );

            case 0xDF83: // TAC-default

                 apk_ReadRamDataTERM( ADDR_TERM_TAC_DEFAULT_01+index*TAC_LEN, TAC_LEN, &data[2] );
                 data[0] = TAC_LEN;
                 data[1] = 0;
                 return( emvOK );

            case 0xDF85: // TAC-online

                 apk_ReadRamDataTERM( ADDR_TERM_TAC_ONLINE_01+index*TAC_LEN, TAC_LEN, &data[2] );
                 data[0] = TAC_LEN;
                 data[1] = 0;
                 return( emvOK );

            case 0xDF84: // TAC-denial

                 apk_ReadRamDataTERM( ADDR_TERM_TAC_DENIAL_01+index*TAC_LEN, TAC_LEN, &data[2] );
                 data[0] = TAC_LEN;
                 data[1] = 0;
                 return( emvOK );

            case 0x9F01: // acquirer id

                 apk_ReadRamDataTERM( ADDR_TERM_ACQ_ID_01+index*ACQ_ID_LEN, ACQ_ID_LEN, &data[2] );
                 data[0] = ACQ_ID_LEN;
                 data[1] = 0;
                 return( emvOK );

            case 0xDF8A: // max number of decimal according to the currency code

                 apk_ReadRamDataTERM( ADDR_TERM_MAX_DECIMAL, 3, data );
                 if( data[0] == 0 )
                   return( emvFailed );
                 else
                   return( emvOK );

	    case 0xDF00: // default DDOL
	    
	    	 apk_ReadRamDataTERM( ADDR_TERM_DDOL, 2, (UCHAR *)&iLen );
	    	 if( iLen == 0 )
	    	   return( emvFailed );
	    	 else
	    	   {
	    	   apk_ReadRamDataTERM( ADDR_TERM_DDOL, iLen+2, data );
	    	   return( emvOK );
	    	   }
		 	    	 
	    case 0xDF01: // default TDOL
	    
	    	 apk_ReadRamDataTERM( ADDR_TERM_TDOL, 2, (UCHAR *)&iLen );
	    	 if( iLen == 0 )
	    	   return( emvFailed );
	    	 else
	    	   {
	    	   apk_ReadRamDataTERM( ADDR_TERM_TDOL, iLen+2, data );
	    	   return( emvOK );
	    	   }
	    	   	    	 
            default:
                   return( emvFailed );
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
void  EMVK_SetupCardParameters( void )
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
      apk_ReadRamDataTERM( ADDR_TERM_TX_CC_01+g_selected_aid_index*TX_CC_LEN, TX_CC_LEN, term_aid );
      apk_WriteRamDataTERM( ADDR_TERM_TX_CC, TX_CC_LEN, term_aid );

      // load the "Transaction Currency Exponent"
      apk_ReadRamDataTERM( ADDR_TERM_TX_CE_01+g_selected_aid_index*TX_CE_LEN, TX_CE_LEN, term_aid );
      apk_WriteRamDataTERM( ADDR_TERM_TX_CE, TX_CE_LEN, term_aid );
      g_term_tx_exp = term_aid[0];	// 2010-03-23,	retrieve target transaction currency exponent
      g_term_decimal_point = 0;		//		without decimal point shown

      // load the "Acquirer ID"
      apk_ReadRamDataTERM( ADDR_TERM_ACQ_ID_01+g_selected_aid_index*ACQ_ID_LEN, ACQ_ID_LEN, term_aid );
      apk_WriteRamDataTERM( ADDR_TERM_ACQ_ID, ACQ_ID_LEN, term_aid );

      // load the "Application Version Number"
      apk_ReadRamDataTERM( ADDR_AVN_01+g_selected_aid_index*AVN_LEN, AVN_LEN, term_aid );
      apk_WriteRamDataTERM( ADDR_TERM_AVN, AVN_LEN, term_aid );

      // load the "Merchant Category Code"
      apk_ReadRamDataTERM( ADDR_TERM_MCC_01+g_selected_aid_index*MCC_LEN, MCC_LEN, term_aid );
      apk_WriteRamDataTERM( ADDR_TERM_MCC, MCC_LEN, term_aid );

      // load the "Terminal Floor Limit" & "Flag"
      apk_ReadRamDataTERM( ADDR_TFL_01+TFL_LEN*g_selected_aid_index, TFL_LEN, term_aid );
      apk_WriteRamDataTERM( ADDR_TFL, TFL_LEN, term_aid );  // bcd format

      TL_bcd2hex( TFL_LEN-1, &term_aid[1], selected_aid );   // PATCH: PBOC20, 2006-02-07
      apk_WriteRamDataTERM( ADDR_TERM_FL, 4, selected_aid ); // binary foramt

      apk_ReadRamDataTERM( ADDR_TFL_FLAG_01+g_selected_aid_index, 1, term_aid );
      apk_WriteRamDataTERM( ADDR_TFL_FLAG, 1, term_aid );

      // load the "Terminal Capabilities"
      apk_ReadRamDataTERM( ADDR_TERM_CAP_01+TERM_CAP_LEN*g_selected_aid_index, TERM_CAP_LEN, term_aid );
      apk_WriteRamDataTERM( ADDR_TERM_CAP, TERM_CAP_LEN, term_aid );

      // load the "Additional Terminal Capabilities"
      apk_ReadRamDataTERM( ADDR_TERM_ADD_CAP_01+TERM_ADD_CAP_LEN*g_selected_aid_index, TERM_ADD_CAP_LEN, term_aid );
      apk_WriteRamDataTERM( ADDR_TERM_ADD_CAP, TERM_ADD_CAP_LEN, term_aid );

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

// ---------------------------------------------------------------------------
// FUNC  : Initialize EMV Kernel parameters & keys.
// INPUT : tid - terminal ID, (an8)
// OUTPUT: none.
// RETURN: emvOK
//         emvFailed   - device error.
// ---------------------------------------------------------------------------
//UCHAR api_emvk_InitEMVKernel( UCHAR *tid )
UCHAR api_emvk_InitEMVKernel(void)
{
UINT  i;
UCHAR buf[64];

      g_dhn_pinpad = 0; // pinpad works
      g_dhn_icc = 0;
//    g_dhn_sam = 0;

//    UI_InitLcdChineseFont();
      UI_InitLcdCodeTableFont( 0xF0 ); // load code table 1

//    if( api_emv_RetrievePublicKeyCA() != emvOK )	// 2009-10-23, removed
//      return( emvFailed );

      // --- Terminal Type ---
//    buf[0] = 0x22; // POS terminal
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_TYPE, 1, buf );

      // --- Terminal Capabilities ---
//    buf[0] = 0xE0;
//    buf[1] = 0x20;    // signature, no PIN (org=0xB8)
//    buf[2] = 0xC0;
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_CAP, 3, buf );

      // --- Additional Terminal Capabilities ---
//    buf[0] = 0x60;
//    buf[1] = 0x00;
//    buf[2] = 0xF0;
//    buf[3] = 0xA0;    // org=0xA0
//    buf[4] = 0x01;    // org=0x00
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_ADD_CAP, 5, buf );

      // IFD serial number
//    buf[0] = 'I';
//    buf[1] = 'F';
//    buf[2] = 'D';
//    buf[3] = '0';
//    buf[4] = '0';
//    buf[5] = '0';
//    buf[6] = '0';
//    buf[7] = '1';
      api_sys_info( SID_TerminalSerialNumber, buf );	// LEN[1]+CID[3]+SN[8]
      api_emv_PutDataElement( DE_TERM, ADDR_TERM_IFD_SN, 8, &buf[4] );

      // ----------------------------
      // Application Independent Data
      // ----------------------------

      // --- Terminal Country Code (n3) ---
//    buf[0] = 0x01;
//    buf[1] = 0x58;
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_CNTR_CODE, 2, buf );

      // -----------------------------
      // Application Dependent Data
      // (specified by payment system)
      // -----------------------------

      // --- Application Identifier (AID) ---

      // "VISACREDIT"
//    buf[0] = 0x08;
//    buf[1] = 0x00; // partial matching NOT allowed
//    buf[2] = 0xA0;
//    buf[3] = 0x00;
//    buf[4] = 0x00;
//    buf[5] = 0x00;
//    buf[6] = 0x03;
//    buf[7] = 0x10;
//    buf[8] = 0x10;
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_AID_01, 0x09, buf );

      // "M/CHIP"
//    buf[0] = 0x08;
//    buf[1] = 0x00; // partial matching NOT allowed
//    buf[2] = 0xA0;
//    buf[3] = 0x00;
//    buf[4] = 0x00;
//    buf[5] = 0x00;
//    buf[6] = 0x04;
//    buf[7] = 0x10;
//    buf[8] = 0x10;
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_AID_02, 0x09, buf );

      // "J/Smart"
//    buf[0] = 0x08;
//    buf[1] = 0x00; // partial matching NOT allowed
//    buf[2] = 0xA0;
//    buf[3] = 0x00;
//    buf[4] = 0x00;
//    buf[5] = 0x00;
//    buf[6] = 0x65;
//    buf[7] = 0x10;
//    buf[8] = 0x10;
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_AID_03, 0x09, buf );

      // end of AID list
//    buf[0] = 0x00;
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_AID_04, 0x01, buf );

      // --- Terminal Identification (an8) ---
//    buf[0] = '3';
//    buf[1] = '1';
//    buf[2] = '0';
//    buf[3] = '0';
//    buf[4] = '0';
//    buf[5] = '0';
//    buf[6] = '0';
//    buf[7] = '0';
      //memmove( buf, tid, 8 );
      api_emv_PutDataElement( DE_TERM, ADDR_TERM_TID, 8, buf );

      // --- Transaction Currency Code (n3) ---
//    buf[0] = 0x09; // iso 4217, TWD=901
//    buf[1] = 0x01; //
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_TX_CC, 2, buf );

      // --- Transaction Currency Exponent (n1, decimal point from the right of amt) ---
//    buf[0] = 0x02; // iso 4217, 2 decimal points
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_TX_CE, 1, buf );
//    g_term_tx_exp = 2;
//    g_term_decimal_point = 0;

      // --- default "Language Preference" ---
      buf[0] = 4;
      buf[1] = 0;
      buf[2] = 'z'; // "zh" = Chinese
      buf[3] = 'h';
      buf[4] = 'e'; // "en" = English
      buf[5] = 'n';
      api_emv_PutDataElement( DE_TERM, ADDR_TERM_LANG_PREFER, 6, buf );

      // -----------------------------------------------------------
      // *** The following parameters are for testing only. ***
      //
      // The real settings shall be accessed by CARDs in TMS.
      // -----------------------------------------------------------

      // --- Merchant Category Code --- (9F15)
//    buf[0] = 0x00; // JCB defined = Tag 9F53 (0x1812)
//    buf[1] = 0x00;
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_MCC, 2, buf );

      // --- Transaction Category Code --- (9F53)
//    buf[0] = 'R';  // to be defined (Mchip)
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_TCC, 1, buf );

      // --- Application Version Number (b2) ---
//    buf[0] = 0x00;    // 1.4.0
//    buf[1] = 0x8C;    //
//    api_emv_PutDataElement( DE_TERM, ADDR_AVN_01, AVN_LEN, buf ); // VSDC
//    buf[0] = 0x00;    // 4.0.0
//    buf[1] = 0x04;    //
//    api_emv_PutDataElement( DE_TERM, ADDR_AVN_02, AVN_LEN, buf ); // M/Chip
//    buf[0] = 0x02;    // 2.0.0
//    buf[1] = 0x00;    //
//    api_emv_PutDataElement( DE_TERM, ADDR_AVN_03, AVN_LEN, buf ); // J/Smart

      // --- Default TDOL --- (not used)
//    buf[0] = 0;
//    buf[1] = 0;
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_TDOL, 2, buf );
      
      buf[0] = 0x03;	// 2009-10-23, restored
      buf[1] = 0x00;
      buf[2] = 0x9F;
      buf[3] = 0x02;
      buf[4] = 0x06;
      api_emv_PutDataElement( DE_TERM, ADDR_TERM_TDOL, 5, buf );

      // --- Default DDOL ---
      buf[0] = 0x03; // DDOL length
      buf[1] = 0x00; //
      buf[2] = 0x9F; // the unpredictable number
      buf[3] = 0x37; //
      buf[4] = 0x04; //
      api_emv_PutDataElement( DE_TERM, ADDR_TERM_DDOL, 5, buf );

      // --- Terminal Floor Limit ---
//    buf[0] = 0x00;
//    buf[1] = 0x00;
//    buf[2] = 0x00;
//    buf[3] = 0x00; // integer part=0
//    buf[4] = 0x00; //
//    buf[5] = 0x00; // decimal part=.00
//
//    buf[6] = 0x01; // TFL flag is set (TFL present)

      // update all TFLs to the same value (from AID_01 to AID_16)
//    for( i=0; i<MAX_TFL_CNT; i++ )
//       {
//       api_emv_PutDataElement( DE_TERM, ADDR_TFL_01+TFL_LEN*i, TFL_LEN, buf ); // value
//       api_emv_PutDataElement( DE_TERM, ADDR_TFL_FLAG_01+i, 1, &buf[6] ); // flag
//       }

      // --- Terminal Floor Limit (VSDC) ---
//    buf[0] = 0x00;
//    buf[1] = 0x00;
//    buf[2] = 0x00;
//    buf[3] = 0x00; // integer part=00
//    buf[4] = 0x00; //
//    buf[5] = 0x00; // decimal part=.00
//    buf[6] = 0x01; // TFL flag is set (TFL present)
//
//    api_emv_PutDataElement( DE_TERM, ADDR_TFL_01, TFL_LEN, buf ); // value
//    api_emv_PutDataElement( DE_TERM, ADDR_TFL_FLAG_01, 1, &buf[6] ); // flag

      // --- Terminal Floor Limit (M/Chip) ---
//    buf[0] = 0x00;
//    buf[1] = 0x00;
//    buf[2] = 0x00;
//    buf[3] = 0x00; // integer part=00
//    buf[4] = 0x00; //
//    buf[5] = 0x00; // decimal part=.00
//    buf[6] = 0x01; // TFL flag is set (TFL present)
//
//    api_emv_PutDataElement( DE_TERM, ADDR_TFL_02, TFL_LEN, buf ); // value
//    api_emv_PutDataElement( DE_TERM, ADDR_TFL_FLAG_02, 1, &buf[6] ); // flag

      // --- Terminal Floor Limit (J/Smart) ---
//    buf[0] = 0x00;
//    buf[1] = 0x00;
//    buf[2] = 0x00;
//    buf[3] = 0x00; // integer part=00
//    buf[4] = 0x00; //
//    buf[5] = 0x00; // decimal part=.00
//    buf[6] = 0x01; // TFL flag is set (TFL present)
//
//    api_emv_PutDataElement( DE_TERM, ADDR_TFL_03, TFL_LEN, buf ); // value
//    api_emv_PutDataElement( DE_TERM, ADDR_TFL_FLAG_03, 1, &buf[6] ); // flag

      // --- TAC's ---
//    memset( buf, 0x00, 5 );
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_TAC_DEFAULT, 5, buf ); // TAC-Default
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_TAC_DENIAL, 5, buf );  // TAC-Denial
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_TAC_ONLINE, 5, buf );  // TAC-Online

      // --- TAC (VSDC) ---
//    buf[0] = 0xEC; // TAC_Default
//    buf[1] = 0x60; //
//    buf[2] = 0x84; //
//    buf[3] = 0xA8; //
//    buf[4] = 0x00; //
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_TAC_DEFAULT_01, 5, buf );

//    buf[0] = 0x00; // TAC_Denial
//    buf[1] = 0x10; //
//    buf[2] = 0x00; //
//    buf[3] = 0x00; //
//    buf[4] = 0x00; //
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_TAC_DENIAL_01, 5, buf );

//    buf[0] = 0xFC; // TAC_Online
//    buf[1] = 0x68; //
//    buf[2] = 0x84; //
//    buf[3] = 0xF8; //
//    buf[4] = 0xC0; //
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_TAC_ONLINE_01, 5, buf );

      // --- TAC (M/Chip) ---
//    buf[0] = 0xF8; // TAC_Default
//    buf[1] = 0x50; //
//    buf[2] = 0xAC; //
//    buf[3] = 0xA0; //
//    buf[4] = 0x00; //
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_TAC_DEFAULT_02, 5, buf );

//    buf[0] = 0x00; // TAC_Denial
//    buf[1] = 0x00; //
//    buf[2] = 0x00; //
//    buf[3] = 0x00; //
//    buf[4] = 0x00; //
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_TAC_DENIAL_02, 5, buf );

//    buf[0] = 0xF8; // TAC_Online
//    buf[1] = 0x50; //
//    buf[2] = 0xAC; //
//    buf[3] = 0xF8; //
//    buf[4] = 0x00; //
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_TAC_ONLINE_02, 5, buf );

      // --- TAC (J/Smart) ---
//    buf[0] = 0xF8; // TAC_Default
//    buf[1] = 0x60; //
//    buf[2] = 0x24; //
//    buf[3] = 0xA0; //
//    buf[4] = 0x00; //
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_TAC_DEFAULT_03, 5, buf );

//    buf[0] = 0x00; // TAC_Denial
//    buf[1] = 0x10; //
//    buf[2] = 0x00; //
//    buf[3] = 0x00; //
//    buf[4] = 0x00; //
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_TAC_DENIAL_03, 5, buf );

//    buf[0] = 0xF8; // TAC_Online
//    buf[1] = 0x60; //
//    buf[2] = 0xAC; //
//    buf[3] = 0xF8; //
//    buf[4] = 0x00; //
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_TAC_ONLINE_03, 5, buf );

      // --- Target Percentage for Random Selection ---
//    buf[0] = 0x00;
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_RS_TP, 1, buf );

      // --- Max. Target Percentage for Biased Random Selection ---
//    buf[0] = 0x00;
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_BRS_MTP, 1, buf );

      // --- Threshold Value for Biased Random Selection ---
//    buf[0] = 0x00;
//    buf[1] = 0x00;
//    buf[2] = 0x00;
//    buf[3] = 0x00;
//    buf[4] = 0x00; // integer=0
//    buf[5] = 0x00; // decimal=.00
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_BRS_THRESHOLD, BRS_THRESHOLD_LEN, buf );

//    buf[0] = 0x00; // VSDC
//    buf[1] = 0x00;
//    buf[2] = 0x00;
//    buf[3] = 0x00;
//    buf[4] = 0x00; // integer=0
//    buf[5] = 0x00; // decimal=.00
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_BRS_THRESHOLD_01, BRS_THRESHOLD_LEN, buf );

//    buf[0] = 0x00; // M/Chip
//    buf[1] = 0x00;
//    buf[2] = 0x00;
//    buf[3] = 0x00;
//    buf[4] = 0x00; // integer=0
//    buf[5] = 0x00; // decimal=.00
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_BRS_THRESHOLD_02, BRS_THRESHOLD_LEN, buf );

//    buf[0] = 0x00; // J/Smart
//    buf[1] = 0x00;
//    buf[2] = 0x00;
//    buf[3] = 0x00;
//    buf[4] = 0x00; // integer=0
//    buf[5] = 0x00; // decimal=.00
//    api_emv_PutDataElement( DE_TERM, ADDR_TERM_BRS_THRESHOLD_03, BRS_THRESHOLD_LEN, buf );

      return( emvOK );
}

// ---------------------------------------------------------------------------
// FUNC  : Open ICC device.
// INPUT : none.
// OUTPUT: none.
// REF   : g_dhn_icc
//         g_dhn_sam
// RETURN: emvOK
//         emvFailed   - device error.
// ---------------------------------------------------------------------------
UCHAR api_emvk_OpenSessionICC( void )
{
UCHAR	dhn_icc;

#ifdef	USE_RSA_SAM

      // open session
      g_dhn_icc = api_ifm_open( ICC1 ); // target ICC
      g_dhn_sam = api_ifm_open( g_rsa_sam ); // RSA SAM

      if( (g_dhn_icc == apiOutOfService) || (g_dhn_sam == apiOutOfService) )
      if( g_dhn_icc == apiOutOfService )
        return( emvFailed );
      else
        return( emvOK );
#endif

#ifndef	USE_RSA_SAM

      // open session
      dhn_icc = api_ifm_open( ICC1 ); // target ICC
      if( dhn_icc == apiOutOfService )	// PATCH: 2010-10-27
        return( emvFailed );
      else
	{
        g_dhn_icc = dhn_icc;		// PATCH: 2010-10-27
        return( emvOK );
	}
#endif
}

// ---------------------------------------------------------------------------
// FUNC  : Close ICC device.
// INPUT : none.
// OUTPUT: none.
// REF   : g_dhn_icc
//         g_dhn_sam
// RETURN: emvOK
//         emvFailed   - device error.
// ---------------------------------------------------------------------------
UCHAR api_emvk_CloseSessionICC( void )
{
UCHAR result1;
UCHAR result2;

      // clear all related data
      apk_EmptyCandidateList();

#ifdef	USE_RSA_SAM

      // close device
      result1 = api_ifm_close( g_dhn_icc );
      result2 = api_ifm_close( g_dhn_sam );

      if( (result1 == emvOK) && (result2 == emvOK) )
      if( result1 == emvOK )
        return( emvOK );
      else
        {
        api_ifm_close( 0 );
        return( emvFailed );
        }
#endif

#ifndef	USE_RSA_SAM

      // close device
      result1 = api_ifm_close( g_dhn_icc );

      if( result1 == emvOK )
        {
        g_dhn_icc = 0;	// PATCH: 2010-10-27
        return( emvOK );
	}
      else
        {
        api_ifm_close( 0 );
        return( emvFailed );
        }
#endif
}

// ---------------------------------------------------------------------------
// FUNC  : Detect the presence of ICC.
// INPUT : none.
// OUTPUT: none.
// REF   : g_dhn_icc
// RETURN: emvReady    - present.
//         emvNotReady - not present.
//         emvFailed   - device error.
// ---------------------------------------------------------------------------
UCHAR api_emvk_DetectICC( void )
{
      return( api_ifm_present( g_dhn_icc ) );
}

// ---------------------------------------------------------------------------
// FUNC  : Activate ICC contacts and execute ATR.
// INPUT : none.
// OUTPUT: none.
// REF   : g_dhn_icc
//         g_dhn_sam
// RETURN: emvOK
//         emvFailed   - device error.
// ---------------------------------------------------------------------------
UCHAR api_emvk_EnableICC( void )
{
UCHAR atr[260]; // PATCH: 2004-06-29, expand atr buffer from 34 to 260 for abnormal card

#ifdef	USE_RSA_SAM

      // reset RSA-SAM
      if( api_emv_ATR( g_dhn_sam, atr ) != emvOK )
        return( emvFailed );
      else
        {
        // select AID = "A0 00 00 00 00 00 02"
        if( api_emv_SelectSAM() != emvOK )
          return( emvFailed );

        api_emv_CleanSAM();              // garbage collection
        }
#endif

      // reset target ICC
      if( api_emv_ATR( g_dhn_icc, atr ) == emvOK )
        return( emvOK );
      else
        return( emvFailed );
}

// ---------------------------------------------------------------------------
// FUNC  : Deactivate ICC contacts.
// INPUT : none.
// OUTPUT: none.
// REF   : g_dhn_icc
//         g_dhn_sam
// RETURN: emvOK
//         emvFailed   - device error.
// ---------------------------------------------------------------------------
UCHAR api_emvk_DisableICC( void )
{
UCHAR result1;
UCHAR result2;

#ifdef	USE_RSA_SAM

      result1 = api_ifm_deactivate( g_dhn_icc ); // disable all ICC contacts
      result2 = api_ifm_deactivate( g_dhn_sam ); // disable all SAM contacts

      if( (result1 == emvOK) && (result2 == emvOK) )
      if( result1 == emvOK )
        return( emvOK );
      else
        {
        api_ifm_deactivate( 0 );
        return( emvFailed );
        }
#endif

#ifndef	USE_RSA_SAM

      result1 = api_ifm_deactivate( g_dhn_icc ); // disable all ICC contacts

      if( result1 == emvOK )
        return( emvOK );
      else
        {
        api_ifm_deactivate( 0 );
        return( emvFailed );
        }
#endif
}

// ---------------------------------------------------------------------------
// FUNC  : Activate RSA SAM contacts and execute ATR.
// INPUT : none.
// OUTPUT: none.
// REF   : g_dhn_sam
// RETURN: emvOK
//         emvFailed   - device error.
// NOTE  : used when another SAM (eg. CTCB crypto SAM) working together.
// ---------------------------------------------------------------------------
UCHAR api_emvk_EnableSAM( void )
{
UCHAR atr[34];

#ifdef	USE_RSA_SAM

      // reset RSA-SAM
      if( api_emv_ATR( g_dhn_sam, atr ) != emvOK )
        return( emvFailed );
      else
        {
        // select AID = "A0 00 00 00 00 00 02"
        if( api_emv_SelectSAM() != emvOK )
          return( emvFailed );
        else
          return( emvOK );
        }
#endif

#ifndef	USE_RSA_SAM

	return( emvOK );
#endif

}

// ---------------------------------------------------------------------------
// FUNC  : Deactivate RSA SAM contacts.
// INPUT : none.
// OUTPUT: none.
// REF   : g_dhn_sam
// RETURN: emvOK
//         emvFailed   - device error.
// NOTE  : used when another SAM (eg. CTCB crypto SAM) working together.
// ---------------------------------------------------------------------------
UCHAR api_emvk_DisableSAM( void )
{
UCHAR result;

#ifdef	USE_RSA_SAM

      result = api_ifm_deactivate( g_dhn_sam ); // disable all SAM contacts

      if( result == emvOK )
        return( emvOK );
      else
        {
        api_ifm_deactivate( 0 );
        return( emvFailed );
        }
#endif

#ifndef	USE_RSA_SAM

	return( emvOK );
#endif
}

// ---------------------------------------------------------------------------
// FUNC  : Create candidate list for application selection.
// INPUT : none.
// OUTPUT: none.
// RETURN: emvOK
//         emvFailed   - not accepted.
//         emvNotReady - no mutually supported application, VSDC forced to FallBack.
// ---------------------------------------------------------------------------
UCHAR api_emvk_CreateCandidateList( void )
{
	g_occurrence = 0;	// 2012-02-14
	
      // create the candidate list
      return( api_emv_CreateCandidateList_PBOC() );
}

// ---------------------------------------------------------------------------
// FUNC  : Get candidate list for application selection.
// INPUT : none.
// OUTPUT: list     - a linear fixed length 2-D array (16x18 bytes)
//                    format: LINK[1] LEN1[1] NAME1[16]
//                            LINK[1] LEN2[1] NAME2[16]...
//                            LINK[1] LEN16[1] NAME16[16]
//                            (1) LINK[1]: (reference only)
//                            (2) LEN[1] : acutual length of NAME
//                                         0 = the bottom of list.
//                            (3) NAME[] : contains application label name.
//         list_cnt - number of items in the list.
//
// RETURN: emvOK            - list is available.
//         emvAutoSelected  - auto selected by the kernel.
//         emvFailed        - not accepted.
// ---------------------------------------------------------------------------
UCHAR api_emvk_GetCandidateList( UCHAR *list_cnt, UCHAR *list )
{
UCHAR result;

      // get the candidate list
      //
      // format: LINK[1] LEN1[1] NAME1[16]
      //         LINK[1] LEN2[1] NAME2[16]...
      //         LINK[1] LEN16[1] NAME16[16]

      *list_cnt = api_emv_GetCandidateList( list );
      if( *list_cnt == 0 )
        return( emvFailed ); // no any application supported

      // special check for auto-selecting application
//    if( api_emv_AutoSelectApplication( 0, *list_cnt, &result ) == TRUE )
      if( api_emv_AutoSelectApplication( g_occurrence, *list_cnt, &result ) == TRUE )	// 2012-02-14
        return( emvAutoSelected );
      else
        return( emvOK );
}

// ---------------------------------------------------------------------------
// FUNC  : Final application selection.
// INPUT : list_cnt  - number of items in the list. (1..n)
//         list_no   - number of the selected item in the list (0..n)
//         list_item - the selected candidate in the list.
//                     format: LINK[1] LEN1[1] NAME1[16]
// OUTPUT: none.
// RETURN: emvOK       - selected ok.
//         emvNotReady - cannot be applied.
//         emvFailed   - not accepted.
// ---------------------------------------------------------------------------
UCHAR api_emvk_SelectApplication( UCHAR list_cnt, UCHAR list_no, UCHAR *list_item )
{
UCHAR result;
UCHAR i;


      g_emv_ccl_SW1 = 0;	// 2015-06-04, clear status for "Create Candidate List"
      g_emv_ccl_SW2 = 0;	//
      
      g_candidate_name_index = list_no; // backup the selected item# for later use

      // final selection
      result = api_emv_FinalSelection( &list_item[list_no*CANDIDATE_NAME_LEN] );
//    result = api_emv_FinalSelection( list_item );

      if( result == emvOutOfService )
        return( emvFailed );

      if( result == emvFailed )
        {
        if( list_cnt <= 1 )
          return( emvFailed );
        else
          {
          g_occurrence = 1;	// 2012-02-14
          return( emvNotReady );
          }
        }
      else // target application is confirmed OK
        {
        for( i=0; i<(AMT_INT_SIZE+AMT_DEC_SIZE); i++ )
           g_term_tx_amt[i] = 0x00; // clear tx amt (n10+n2)

        EMVK_SetupCardParameters(); // setup card related parameters

        return( emvOK );
        }
}

// ---------------------------------------------------------------------------
// FUNC  : Final application selection.
// INPUT : list_item - the selected candidate in the list.
//                     format: LINK[1] LEN1[1] NAME1[16]
// OUTPUT: aid	     - format: LEN[1] + AID[n]
//	   api	     - application priority indicator. (0..15, 0=no priority, 1=highest)
// RETURN: emvOK
//         emvFailed
// ---------------------------------------------------------------------------
UCHAR	api_emvk_GetCandidateAID( UCHAR *list_item, UCHAR *aid, UCHAR *api )
{
UCHAR	result;
UCHAR	*ptrobj;
UINT	addr;
UINT	iLen;


      // read AID or DFNAME by using appname.link
      addr = ADDR_CANDIDATE_01 + CANDIDATE_LEN*list_item[0];
      apk_ReadRamDataTERM( addr, CANDIDATE_LEN, g_ibuf );
      
      ptrobj = apk_FindTag( 0x4f, 0x00, g_ibuf );  // Tag=4F (AID)? or
      if( ptrobj != 0 )
        {
        iLen = *ptrobj;
        *aid++ = iLen;	// length
        memmove( aid, ptrobj+1, iLen );
        
        // 2013-10-29, find API
        *api = 0;
        ptrobj = apk_FindTag( 0x87, 0x00, g_ibuf );	// TAG=87 (API)?
        if( ptrobj !=0 )
          *api = *(ptrobj+1) & 0x0F;
        
        result = apiOK;	// AID found
        }
      else
        {
        ptrobj = apk_FindTag( 0x84, 0x00, g_ibuf );  // Tag=84 (DFNAME)
        if( ptrobj !=0 )
          {
          iLen = *ptrobj;
          *aid++ = iLen;	// length
          memmove( aid, ptrobj+1, iLen );
          
          // 2013-10-29, find API
          *api = 0;
          ptrobj = apk_FindTag( 0x87, 0x00, g_ibuf );	// TAG=87 (API)?
          if( ptrobj !=0 )
            *api = *(ptrobj+1) & 0x0F;
        
          result = apiOK;	// DFNAME found
          }
        else
          result = apiFailed;	// DFNAME not found
	}
	
	return( result );
}

// ---------------------------------------------------------------------------
// FUNC  : Inform kernel of the current transaction amount and sequence counter.
// INPUT : amt - current transaction amount. (n12, 6 bytes)
//         sc  - current transaction sequence counter. (n6, 3 bytes)
//               incremented by one after each transaction.
// OUTPUT: none.
// REF   : g_term_tx_amt
// RETURN: none.
// ---------------------------------------------------------------------------
void  api_emvk_SetTransPara( UCHAR *amt, UCHAR *sc )
{
UCHAR buf[4];

      // backup current tx amount bcd format (including integer & decimal)
      memmove( g_term_tx_amt, amt, 6 );

      apk_WriteRamDataTERM( ADDR_TERM_AMT_AUTH_N, 6, amt );  //
      apk_WriteRamDataTERM( ADDR_TERM_TX_AMT, 6, amt );      // including tips & adjustments

      TL_bcd2hex( 5, &amt[1], buf ); // convert to binary format
      apk_WriteRamDataTERM( ADDR_TERM_AMT_AUTH_B, 4, buf );

      // backup current tx sequence counter
      buf[0] = 0x00; // the MSB always = 0
      memmove( &buf[1], sc, 3 );
      api_emv_PutDataElement( DE_TERM, ADDR_TERM_TX_SC, 4, buf );
}

// ---------------------------------------------------------------------------
// FUNC  : Initiate the selected application to get the AIP & AFL.
// INPUT : tx_type - transaction type (eg. TT_GOODS_AND_SERVICE)
// OUTPUT: none.
// RETURN: emvOK
//         emvNotReady - cannot be applied.
//         emvFailed   - not accepted.
// ---------------------------------------------------------------------------
UCHAR api_emvk_InitiateApplication( UCHAR tx_type )
{
UCHAR result;

      // issue "GET PROCESSING OPTIONS" command
      result =  api_emv_InitApplication( tx_type, g_term_tx_amt );
      if( result ==  emvOK )
        return( emvOK );
      else
        {
        if( result == emvNotReady )
          {
          // eliminate the current App from consideration, try another.
          api_emv_RemoveApplication( g_candidate_name_index );
          return( emvNotReady );
          }
        else
          return( emvFailed );
        }
}

// ---------------------------------------------------------------------------
// FUNC  : Terminal reads the files and records indicated in the AFL.
// INPUT : none.
// OUTPUT: none.
// RETURN: emvOK
//         emvFailed   - not accepted.
// ---------------------------------------------------------------------------
UCHAR api_emvk_ReadApplicationData( void )
{
      return( api_emv_ReadApplicationData() );
}

// ---------------------------------------------------------------------------
// FUNC  : Offline data authentication. (SDA, DDA)
// INPUT : none.
// OUTPUT: none.
// RETURN: emvOK
//         emvFailed       - reference only, continue transaction.
//         emvOutOfService - not accepted.
// ---------------------------------------------------------------------------
UCHAR api_emvk_OfflineDataAuthen( void )
{
      return( api_emv_OfflineDataAuthen() );
}

// ---------------------------------------------------------------------------
// FUNC  : to determine the degree of compatibility of the application in
//         the terminal with the application in the ICC and to make any
//         necessary adjustments, including possible rejection of the
//         transaction.
//         (1) AVN: Application Version Number
//         (2) AUC: Application Usage Control
//         (3) AED: Application Effective/Expiration Dates Checking
// INPUT : none.
// OUTPUT: none.
// RETURN: none.
// ---------------------------------------------------------------------------
void  api_emvk_ProcessingRestrictions( void )
{
      api_emv_ProcessingRestrictions();
}

// ---------------------------------------------------------------------------
// FUNC  : Cardholder verification is performed to ensure that the person
//         presenting the ICC is the one to whom the applicaiton in the card
//         was issued.
//         (1) Offline Plaintext PIN.
//         (2) Offline Enciphered PIN.
//         (3) Online PIN.
//         (4) Signature.
// INPUT : ped         - 0 = using internal PINPAD.
//		         1 = using external PINPAD.
//	   algo	       - FX, MS or DUKPT
//	   mode	       - algorithm of PIN block. (effective only for online pin of external pinpad)
//	   index       - key slot index. 	 (effective only for online pin of external pinpad)
//	   tout        - timeout. (PIN entry timeout in seconds)
// OUTPUT  : epb       - LL-V, the enciphered PIN data for online (ISO 9564-1).
//                             if LL=0, no online PIN data.  (2+8)
//	     ksn       - LL-V, key serial number for online. (2+10)
//			       if LL=0, no online KSN data. (DUKPT only)
// REF   : g_term_tx_amt
//	   g_emv_ped_src
//	   g_emv_ped_mode
//	   g_emv_ped_index
// RETURN: emvOK
//         emvFailed       - reference only, continue transaction.
//         emvOutOfService - not accepted.
// ---------------------------------------------------------------------------
UCHAR api_emvk_CardholderVerification( UCHAR ped, UCHAR algo, UCHAR mode, UCHAR index, UCHAR *epb, UCHAR *ksn, UINT tout )	// 2010-03-16
{
UCHAR i;
UCHAR buf1[21];
UCHAR buf2[52];
//UCHAR epb[10];
//UCHAR ksn[12];

	
//	if( ped == 0x00 )		// 2010-03-16
//	  g_emv_ped_src = 0x00;		// internal pinpad
//	else
//	  g_emv_ped_src = 0x01;		// external pinpad

	g_emv_ped_src = ped;		// 2010-03-22, value = 0, 1, 0xFF(out of order)

	g_emv_ped_algo = algo;		// key algorithm   (Fixe, Master/Session, DUKPT)
	g_emv_ped_mode = mode;		// encryption mode (PIN block format)
	g_emv_ped_index = index;	// key slot index  (key slot index)
	  
      // convert AMT bcd to ascii ($NNN)
      TL_bcd2asc( AMT_INT_SIZE+AMT_DEC_SIZE, g_term_tx_amt, buf1 );
      TL_trim_asc( 0, buf1, buf2 );
      i = TL_insert_decimal_point( 0x04, buf2 ); // NNNNN.NN
//    TL_insert_thousand_comma( buf2, i );       // NN,NNN.NN
      TL_trim_decimal( g_term_tx_exp, g_term_decimal_point, buf2 ); // NNNNN

//      buf1[0] = '(';
//      buf1[1] = '$';
//      for( i=0; i<buf2[0]; i++ )
//         buf1[i+2] = buf2[i+1];
//      buf1[i+2] = ')';
//
//      buf1[i+3] = ' ';
//      buf1[i+4] = 'O';
//      buf1[i+5] = 'K';
//      buf1[i+6] = '?';

//      // prepare message ($amount) to be shown on PIN PAD,
//      // which will be confirmed by the cardholder before entering PIN
//      i = buf2[0] + 7;                // length
//      buf2[0] = 3;                    // row
//      buf2[1] = 0;                    // col
//      buf2[2] = FONT0;                // font
//      buf2[3] = i;                    // length
//      memmove( &buf2[4], buf1, i );   // 1'st message

//      buf2[24+0] = 0;                 // row
//      buf2[24+1] = 0;                 // col
//      buf2[24+2] = FONT3;             // font
//      buf2[24+3] = 15;                // length
//      TL_memmove( &buf2[24+4], msg_pin_pad, 15 ); // 2'nd message
//
//      buf2[48+3] = 0;                 // no 3'rd message

      // CVM
      // total amount "$NNNN.NN" to be confirmed on PINPAD
      buf1[0] = buf2[0] + 1;
      buf1[1] = '$';
      for( i=0; i<buf2[0]; i++ )
	 buf1[i+2] = buf2[i+1];

      if( tout == 0 )
        tout = 60;
      //return( api_emv_CardholderVerification( tout, buf1, epb, ksn ) );	// PATCH: 2010-03-12, typing error (buf2->buf1)
      return api_emv_CardholderVerification(tout, buf1, epb, ksn, g_iso_format, g_key_index);
}

// ---------------------------------------------------------------------------
// FUNC  : The TRM is that portion of risk management performed by the
//         terminal to protect the acquirer, issuer, and system from fraud.
//         (1) Floor limit checking.
//         (2) Random transaction selection.
//         (3) Velocity checking.
// INPUT : none.
// OUTPUT: none.
// REF   : g_term_tx_amt
// RETURN: none.
// ---------------------------------------------------------------------------
#ifndef PCI_AP
void  api_emvk_TerminalRiskManagement( void )
{
UINT  i;
UINT  iLen;
UCHAR pan[10];
UCHAR amt[6];
UCHAR log[TX_LOG_LEN];

      // Find the most recent entry the same PAN
      // << This part shall be modified according to the real application. >>

      // get current PAN ("pan" is left-justified & padded with trailing 'F'.
      memset( pan, 0xFF, 10 );
      api_emv_GetDataElement( DE_ICC, ADDR_ICC_AP_PAN, 2, (UCHAR *)&iLen );
      api_emv_GetDataElement( DE_ICC, ADDR_ICC_AP_PAN+2, iLen, pan );

      // scan all logs "backwardly" to find the same PAN
      memset( amt, 0x00, 6 ); // clear amt value
      EMV_CB_FindMostRecentPAN( pan, amt );

      api_emv_TerminalRiskManagement( g_term_tx_amt, g_term_tx_amt, amt );
}
#endif
// ---------------------------------------------------------------------------
// FUNC  : once terminal risk management and application functions related
//         to a normal offline transaction have been completed, the terminal
//         makes the first decision as to whether the transaction should be:
//         (1) Approved Offline   or
//         (2) Declined Offline   or
//         (3) Transmitted Online
// INPUT : online = 1, forced online transaction (e.g. PreAuth).
// OUTPUT: none.
// RETURN: emvOK
//         emvNotReady - reversal is to be done for the current online transaction.
//         emvFailed   - not accepted.
//         emvAborted  - conditions of use not satisfied at 1'st GenAC, goto FallBack.
// ---------------------------------------------------------------------------
UCHAR api_emvk_TerminalActionAnalysis( UCHAR online )
{
UCHAR arc[2];

      return( api_emv_TerminalActionAnalysis2( online, arc ) );
}

// ---------------------------------------------------------------------------
// FUNC  : To check the final status of ICC transaction.
// INPUT : none.
// OUTPUT: arc - the final authorization response code.
//               "Y1" - OFFLINE_APPROVED
//               "Z1" - OFFLINE_DECLINED
//               "Y2" - ARC_ONLINE_APPROVED (NA)
//               "Z2" - ARC_ONLINE_DECLINED (NA)
//               "Y3" - APPROVED_UNABLE_ONLINE
//               "Z3" - ARC_DECLINED_UNABLE_ONLINE
// RETURN: none.
// ---------------------------------------------------------------------------
void  api_emvk_Completion( UCHAR *arc )
{
      api_emv_GetDataElement( DE_TERM, ADDR_TERM_ARC, 2, arc ); // get ARC
}

// ---------------------------------------------------------------------------
// FUNC  : To get SW1 & SW2 after EMVK API command.
//	   Currently only supported by "api_emvk_SelectApplication()".
// INPUT : none.
// OUTPUT: sw1, sw2.
// RETURN: none.
// ---------------------------------------------------------------------------
void	api_emvk_GetStatusBytes( UCHAR *sw1, UCHAR *sw2 )
{
	if( g_emv_ccl_SW1 != 0 )	// 2015-06-04, referenced from CCL
	  {
	  *sw1 = g_emv_ccl_SW1;
	  *sw2 = g_emv_ccl_SW2;
	  }
	else
	  {
	  *sw1 = g_emv_SW1;
	  *sw2 = g_emv_SW2;
	  }
	  
	g_emv_SW1 = 0;
	g_emv_SW2 = 0;
	g_emv_ccl_SW1 = 0;
	g_emv_ccl_SW2 = 0;
}

// ---------------------------------------------------------------------------
// FUNCTION: processing restriction for Application Effective/Expiration Dates
//           Checking.
//           YY=00~49: 20YY
//           YY=50~99: 19YY
// INPUT   : none.
// OUTPUT  : none.
// REF     :
// RETURN  : emvOK       - OK
//           emvNotReady - not effetive
//           emvFailed   - expired
//
// NOTE    : this function is valid after "Read Application Data".
// ---------------------------------------------------------------------------
UCHAR api_emvk_CheckAppDate( void )
{
//    return( apk_ProcessRestrict_AED() );
	
      switch( apk_ProcessRestrict_AED() )	// 2010-11-16
	    {
	    case 0:
	    	 return( emvOK );
	    	 
	    case 1:  // not effective
	    	 return( emvNotReady );
	    	 
	    default: // expired or (expired + not effective)
	         return( emvFailed );
	    }
}

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
void  api_emvk_GetPublicKeyInfo( UCHAR *info )
{
      api_emv_GetPublicKeyInfo( info );
}

// ---------------------------------------------------------------------------
// FUNC  : To setup a new currency code temporarily for the DCC tranaction.
// INPUT : cc = new currency code (n3, 2 bytes), generally it is value of tag 9F42.
// OUTPUT: none.
// RETURN: none.
// ---------------------------------------------------------------------------
void	api_emvk_SetCurrencyCode( UCHAR *cc )
{
	apk_WriteRamDataTERM( ADDR_TERM_TX_CC, TX_CC_LEN, cc );
}

// ---------------------------------------------------------------------------
UCHAR EMVK_FindTag( UCHAR tag1, UCHAR tag2, UCHAR *tlv, UCHAR *value, UCHAR len )
{
UCHAR *ptrobj;
UINT  iLen;
UCHAR cnt;

          memset( value, 0x00, len ); // clear output value

          ptrobj = apk_FindTag( tag1, tag2, tlv );
          if( ptrobj != 0 ) // found
            {
            iLen = apk_GetBERLEN( ptrobj, &cnt ); // length
            ptrobj += cnt; // ptr to V
            memmove( value, ptrobj, iLen ); // get V

            return( TRUE );
            }
          else
            return( FALSE ); // not found
}

// ---------------------------------------------------------------------------
// FUNC  : To put TMS EMV parameters to EMV kernel. (_SRAM_PAGE_)
// INPUT : buf  - where the parameters are stored in SRAM page.
//                UCHAR page   : page number.
//                ULONG address: beginning address.
//
//                Data structure: (max 16KB)
//                LEN1[2] PARA1[n1]   <-- 1'st record
//                LEN2[2] PARA2[n2]   <-- 2'nd record
//                .................
//                LENn[2] PARAn[nn]   <-- the last record
//                0x00 0x00           <-- end of record
//
//                Where:
//                PARA = AID + parameters
//                       all data elements are represented by T-L-V format.
//
//                       TAG    LENGTH  NAME
//                       ----   ------  ----------------------------------
//                       9F06   5~16    AID (b)
//                       5F2A   2       Transaction Currency Code (n3)
//                       5F36   1       Transaction Currency Exponent (n1)
//                       9F01   6       Acquirer Identifier (n6~11)
//                       9F09   2       Application Version Number (b)
//                       9F15   2       Merchant Category Code (n4)
//                       9F1A   2       Terminal Country Code (n3)
//                       9F1B   4       Terminal Floor Limit (b)				- OLD NCCC TMS
//		***	 9F1B	6	Terminal Floor Limit (n12)					- NEW NCCC TMS
//                       9F33   3       Terminal Capabilities (b)
//                       9F35   1       Terminal Type (n2)
//                       9F40   5       Additional Terminal Capabilities (b)
//                       9F53   1       Transaction Category Code (an)
//                       DF83   5       Terminal Action Code - Default (b)
//                       DF84   5       Terminal Action Code - Denial (b)
//                       DF85   5       Terminal Action Code - Online (b)
//                       DF87   4       Threshold Value for Biased Random Selection (b)		- OLD NCCC TMS
//		***	 DF87   6       Threshold Value for Biased Random Selection (n12)		- NEW NCCC TMS
//                       DF88   2       Target Percentage for Random Selection (n2)		- OLD NCCC TMS
//		*** 	 DF88   1       Target Percentage for Random Selection (n2)			- NEW NCCC TMS
//			 DF89   2       Max Target Percentage for Biased Random Selection (n2)	- OLD NCCC TMS
//		***	 DF89   1       Max Target Percentage for Biased Random Selection (n2)		- NEW NCCC TMS
//                       DF8A   1       Max number of decimals according to the currency code (RFU)
//                       DFEE   1       Terminal Entry Capability (RFU)
//              ***      DF01   var.    Default TDOL (b)						- NEW NCCC TMS
//              ***      DF00   var.    Default DDOL (b)						- NEW NCCC TMS
//
// OUTPUT: none.
// RETURN: emvOK
//         emvFailed  - out of memory.
//         emvAborted - invalid record format. (no AID)
//
// NOTE  : This function is called once by AP after EMV parameters are downloaded
//         from TMS to terminal.
//         Current Version: Fixed as follows:
//         1'ST REC: VSDC   (A0 00 00 00 03 10 10)
//         2'ND REC: MCHIP  (A0 00 00 00 04 10 10)
//         3'RD REC: JSMART (A0 00 00 00 65 10 10)
// ---------------------------------------------------------------------------
UCHAR api_emvk_PutTmsPARA( UCHAR *buf )
{
UCHAR sbuf[256];
API_SRAM ibuf;//2009-02-06_Richard:Adding for passing parameter to api_sram_PageRead
UCHAR dbuf[256];
ULONG lAddr;
UINT  iLen, iLen2;
UCHAR *ptrobj;
UCHAR cnt;
UCHAR para_cnt;
UCHAR exp;
ULONG lValue;
UCHAR i;
UCHAR rid[5];


//	TL_DumpHexData( 0, 0, 5, buf );		//////

      // clear all related EMV parameters in terminal
      // AID, ...

      para_cnt = 0;

      lAddr = buf[1] + buf[2]*256; // the start address

GetRecord:

      if( para_cnt >= MAX_AID_CNT )
        return( emvFailed );
      // get record LEN[2]
      sbuf[0] = buf[0];                 // page no.
      sbuf[1] = lAddr & 0x00ff;         // addr of length
      sbuf[2] = (lAddr & 0xff00) >> 8;  //
      sbuf[3] = 0x00;                   //
      sbuf[4] = 0x00;                   //
      sbuf[5] = 2;       // length
      sbuf[6] = 0;       //
      
      //2009-02-06_Richard: Adding definition of 'ibuf'
      ibuf.StPage = buf[0];             // page no.
      ibuf.StAddr = lAddr;     // addr of length
      ibuf.Len = 2; // length
      
      api_sram_PageRead( ibuf, dbuf );//2009-02-06_Ricahrd: 1. Modifying "api_ram_read" to "api_sram_PageRead"
      //api_ram_read( sbuf, dbuf );   //                    2. Modifying 'sbuf' to  'ibuf'          
      if(dbuf[0]==0x30 && dbuf[1]==0x30)
      {
        dbuf[0]=0;
        dbuf[1]=0;
      }
        
      iLen = dbuf[0] + dbuf[1]*256; // get one record length
      if( iLen > (sizeof(dbuf)-2) )
        return( emvAborted );
      if( iLen != 0 )
        {
        sbuf[0] = buf[0];                     // page no.
        sbuf[1] = (lAddr+2) & 0x00ff;         // addr of data
        sbuf[2] = ((lAddr+2) & 0xff00) >> 8;  //
        sbuf[3] = 0x00;                   //
        sbuf[4] = 0x00;                   //
        sbuf[5] = iLen & 0x00ff;          // length
        sbuf[6] = (iLen & 0xff00) >> 8;   //
        
        //2009-02-06_Richard: Adding definition of 'ibuf'
        ibuf.StPage = buf[0];                 // page no.
        ibuf.StAddr = lAddr+2;     // addr of data
        ibuf.Len = iLen;// length
                
        api_sram_PageRead( ibuf, &dbuf[4] );//2009-02-06_Ricahrd: Modifying "api_ram_read" to "api_sram_PageRead"
        //api_ram_read( sbuf, &dbuf[4] );   // get one record data
        
//	TL_DispHexWord( 0, 0, iLen );			/////
//	TL_DumpHexData( 0, 1, iLen, &dbuf[4] );		/////

        // foramt to: 70 82 LL LL [TLV's]
        dbuf[0] = 0x70;
        dbuf[1] = 0x82;
        dbuf[2] = sbuf[6];
        dbuf[3] = sbuf[5];

	memset( rid, 0x00, sizeof(rid) );	// 2014-06-05

        // 1. AID
        ptrobj = apk_FindTag( 0x9F, 0x06, dbuf );
        if( ptrobj != 0 )
          {
          iLen2 = apk_GetBERLEN( ptrobj, &cnt ); // length of AID
          ptrobj += cnt; // ptr to V[AID]

//          if( (iLen2 > 5) && (iLen2 <17) )
//            sbuf[1] = 0x00; // partial matching is not allowed
//          else
//            {
//            if( iLen2 == 5 )
//              sbuf[1] = 0x01; // partial matching is allowed
//            else
//              return( emvAborted );
//            }

	  // PATCH: 2011-07-19
	  if( (iLen2 > 4) && (iLen2 <17) )	// 5..16
	    sbuf[1] = 0x01; // default: partial matching is allowed for all valid AIDs
	  else
	    return( emvAborted );
          sbuf[0] = iLen2 + 1;
          memmove( &sbuf[2], ptrobj, iLen2 );
          api_emv_PutDataElement( DE_TERM, ADDR_TERM_AID_01+TERM_AID_LEN*para_cnt, iLen2+2, sbuf );

	  memmove( rid, ptrobj, sizeof(rid) );	// 2014-06-05, backup RID (first 5 bytes of AID) for later checking

          if( para_cnt < (MAX_AID_CNT-1) )
            {
            sbuf[0] = 0; // put end of AID mark
            api_emv_PutDataElement( DE_TERM, ADDR_TERM_AID_01+TERM_AID_LEN*(para_cnt+1), 1, sbuf );
            }

RecStart:
          // 2. Transaction Currency Code (Tag=5F2A)
          EMVK_FindTag( 0x5F, 0x2A, dbuf, sbuf, TX_CC_LEN );
          api_emv_PutDataElement( DE_TERM, ADDR_TERM_TX_CC_01+TX_CC_LEN*para_cnt, TX_CC_LEN, sbuf );

          // 3. Transaction Currency Exponent (Tag=5F36)
          if( EMVK_FindTag( 0x5F, 0x36, dbuf, sbuf, TX_CE_LEN ) == FALSE )
            sbuf[0] = 0x02;	// PATCH: 2010-03-19
          api_emv_PutDataElement( DE_TERM, ADDR_TERM_TX_CE_01+TX_CE_LEN*para_cnt, TX_CE_LEN, sbuf );
          exp = sbuf[0];
          g_term_tx_exp = sbuf[0];
          g_term_decimal_point = 0; // number of display decimals = 0

          // 4. Acquirer Identifier (Tag=9F01)
          EMVK_FindTag( 0x9F, 0x01, dbuf, sbuf, ACQ_ID_LEN );
          api_emv_PutDataElement( DE_TERM, ADDR_TERM_ACQ_ID_01+ACQ_ID_LEN*para_cnt, ACQ_ID_LEN, sbuf );

          // 5. Application Version Number (Tag=9F09)
          if( EMVK_FindTag( 0x9F, 0x09, dbuf, sbuf, AVN_LEN ) == FALSE )
            {
#if	0
            // PATCH: 2010-10-23, set default AVN for EMV2000 if not found
            switch( para_cnt )
                  {
                  case 0: // VSDC
                       sbuf[0] = 0x00;
                       sbuf[1] = 0x8C;
                       break;

                  case 1: // MCHIP
                       sbuf[0] = 0x00;
                       sbuf[1] = 0x04;
                       break;

                  case 2: // JSMART
                       sbuf[0] = 0x02;
                       sbuf[1] = 0x00;
                       break;

                  case 3: // AEIPS (RFU)
                       break;
                       
                  case 4: // 2013-10-31, UICC
                       sbuf[0] = 0x00;
                       sbuf[1] = 0x20;
                       break;
                  }
#endif
	    // 2014-05-06, assign default AVN according to RID(5)
	    sbuf[0] = 0;
	    sbuf[1] = 0;
	    
	    if( (rid[0] == 0xA0) && (rid[1] == 0x00) && (rid[2] == 0x00) && (rid[3] == 0x00) && (rid[4] == 0x03) ) // VSDC
	      {
	      sbuf[0] = 0x00;
	      sbuf[1] = 0x8C;
	      goto SETUP_AVN;
	      }

	    if( (rid[0] == 0xA0) && (rid[1] == 0x00) && (rid[2] == 0x00) && (rid[3] == 0x00) && (rid[4] == 0x04) ) // MCHIP
	      {
	      sbuf[0] = 0x00;
//	      sbuf[1] = 0x04;
	      sbuf[1] = 0x02;	// 2017-11-29, for MTIP Test
	      goto SETUP_AVN;
	      }

	    if( (rid[0] == 0xA0) && (rid[1] == 0x00) && (rid[2] == 0x00) && (rid[3] == 0x00) && (rid[4] == 0x65) ) // JSMART
	      {
	      sbuf[0] = 0x02;
	      sbuf[1] = 0x00;
	      goto SETUP_AVN;
	      }

	    if( (rid[0] == 0xA0) && (rid[1] == 0x00) && (rid[2] == 0x00) && (rid[3] == 0x00) && (rid[4] == 0x25) ) // AMEX, 2018-03-13
	      {
	      sbuf[0] = 0x00;
	      sbuf[1] = 0x01;
	      goto SETUP_AVN;
	      }

	    if( (rid[0] == 0xA0) && (rid[1] == 0x00) && (rid[2] == 0x00) && (rid[3] == 0x03) && (rid[4] == 0x33) ) // UICC
	      {
	      sbuf[0] = 0x00;
	      sbuf[1] = 0x20;
	      goto SETUP_AVN;
	      }
	      
	    if( (rid[0] == 0xA0) && (rid[1] == 0x00) && (rid[2] == 0x00) && (rid[3] == 0x01) && (rid[4] == 0x52) ) // DISCOVER, 2018-03-13
	      {
	      sbuf[0] = 0x01;
	      sbuf[1] = 0x00;
	      goto SETUP_AVN;
	      }

            }
SETUP_AVN:
          api_emv_PutDataElement( DE_TERM, ADDR_AVN_01+AVN_LEN*para_cnt, AVN_LEN, sbuf );

          // 6. Merchant Category Code (Tag=9F15)
          EMVK_FindTag( 0x9F, 0x15, dbuf, sbuf, MCC_LEN );
          api_emv_PutDataElement( DE_TERM, ADDR_TERM_MCC_01+MCC_LEN*para_cnt, MCC_LEN, sbuf );

          // 7. Terminal Country Code (Tag=9F1A)
          if( EMVK_FindTag( 0x9F, 0x1A, dbuf, sbuf, 2 ) == TRUE )
            api_emv_PutDataElement( DE_TERM, ADDR_TERM_CNTR_CODE, 2, sbuf );

          // 8. Terminal Floor Limit (Tag=9F1B)	- OLD NCCC TMS
//          if( EMVK_FindTag( 0x9F, 0x1B, dbuf, sbuf, TFL_LEN-2 ) == TRUE )
//            {
//            lValue = sbuf[3] + 0x100L*sbuf[2] + 0x10000L*sbuf[1] + 0x1000000L*sbuf[0];
//            sbuf[0] = api_tl_ltoa( lValue, &sbuf[1] );//2009-02-06_Ricahrd: Modifying 'TL_ltoa()' to '(api_tl_ltoa)' 
//            //sbuf[0] = TL_ltoa( lValue, &sbuf[1] );//2009-02-06_Ricahrd: Modifying 'TL_ltoa()' to '(api_tl_ltoa)' 
//            cnt = sbuf[0];
//
//            TL_asc2bcd( 6, &sbuf[100], sbuf );
//
//            api_emv_PutDataElement( DE_TERM, ADDR_TFL_01+TFL_LEN*para_cnt, TFL_LEN, &sbuf[100] );
//            sbuf[0] = 1; // TFL is set
//            }
//          else
//            sbuf[0] = 0; // TFL is not set
//          api_emv_PutDataElement( DE_TERM, ADDR_TFL_FLAG_01+1*para_cnt, 1, sbuf ); // flag

          // 8. Terminal Floor Limit (Tag=9F1B)	- NEW NCCC TMS
	  if( EMVK_FindTag( 0x9F, 0x1B, dbuf, sbuf, TFL_LEN ) == TRUE )
	    {
	    api_emv_PutDataElement( DE_TERM, ADDR_TFL_01+TFL_LEN*para_cnt, TFL_LEN, sbuf );
	    sbuf[0] = 1; // TFL is set
	    }
	  else
	     sbuf[0] = 0; // TFL is not set
	  api_emv_PutDataElement( DE_TERM, ADDR_TFL_FLAG_01+1*para_cnt, 1, sbuf ); // flag

          // 9. Terminal Capabilities (Tag=9F33)
          EMVK_FindTag( 0x9F, 0x33, dbuf, sbuf, TERM_CAP_LEN );
          api_emv_PutDataElement( DE_TERM, ADDR_TERM_CAP_01+TERM_CAP_LEN*para_cnt, TERM_CAP_LEN, sbuf );

          // 10. Terminal Type (Tag=9F35)
          if( EMVK_FindTag( 0x9F, 0x35, dbuf, sbuf, 1 ) == TRUE )
            api_emv_PutDataElement( DE_TERM, ADDR_TERM_TYPE, 1, sbuf );

          // 11. Additional Terminal Capabilities (Tag=9F40)
          if( EMVK_FindTag( 0x9F, 0x40, dbuf, sbuf, 5 ) == TRUE )
            api_emv_PutDataElement( DE_TERM, ADDR_TERM_ADD_CAP_01+TERM_ADD_CAP_LEN*para_cnt, TERM_ADD_CAP_LEN, sbuf );

          // 12. Transaction Category Code (Tag=9F53)
          if( EMVK_FindTag( 0x9F, 0x53, dbuf, sbuf, 1 ) == TRUE )
            api_emv_PutDataElement( DE_TERM, ADDR_TERM_TCC, 1, sbuf );

          // 13. Terminal Action Code - Default (Tag=DF83)
          EMVK_FindTag( 0xDF, 0x83, dbuf, sbuf, TAC_LEN );
          api_emv_PutDataElement( DE_TERM, ADDR_TERM_TAC_DEFAULT_01+TAC_LEN*para_cnt, TAC_LEN, sbuf );

          // 14. Terminal Action Code - Denial (Tag=DF84)
          EMVK_FindTag( 0xDF, 0x84, dbuf, sbuf, TAC_LEN );
          api_emv_PutDataElement( DE_TERM, ADDR_TERM_TAC_DENIAL_01+TAC_LEN*para_cnt, TAC_LEN, sbuf );

          // 15. Terminal Action Code - Online (Tag=DF85)
          EMVK_FindTag( 0xDF, 0x85, dbuf, sbuf, TAC_LEN );
          api_emv_PutDataElement( DE_TERM, ADDR_TERM_TAC_ONLINE_01+TAC_LEN*para_cnt, TAC_LEN, sbuf );

          // 16. Threshold Value for BRS (Tag=DF87)	- OLD NCCC TMS
//          EMVK_FindTag( 0xDF, 0x87, dbuf, sbuf, BRS_THRESHOLD_LEN );
//
//          // convert HEX to BCD with decimal point (b4 -> n12)
//          lValue = sbuf[3] + 0x100L*sbuf[2] + 0x10000L*sbuf[1] + 0x1000000L*sbuf[0];
//          sbuf[0] = api_tl_ltoa( lValue, &sbuf[1] );//2009-02-06_Ricahrd: Modifying 'TL_ltoa()' to '(api_tl_ltoa)' 
//          //sbuf[0] = TL_ltoa( lValue, &sbuf[1] );
//          cnt = sbuf[0];
//
//          TL_asc2bcd( 6, &sbuf[100], sbuf );
//
//          api_emv_PutDataElement( DE_TERM, ADDR_TERM_BRS_THRESHOLD_01+BRS_THRESHOLD_LEN*para_cnt, BRS_THRESHOLD_LEN, &sbuf[100] );

	 // 16. Threshold Value for BRS (Tag=DF87)	- NEW NCCC TMS
	 EMVK_FindTag( 0xDF, 0x87, dbuf, sbuf, BRS_THRESHOLD_LEN );
	 api_emv_PutDataElement( DE_TERM, ADDR_TERM_BRS_THRESHOLD_01+BRS_THRESHOLD_LEN*para_cnt, BRS_THRESHOLD_LEN, sbuf );

          // 17. Target Percentage for RS (Tag=DF88)	- OLD NCCC TMS
////        EMVK_FindTag( 0xDF, 0x88, dbuf, sbuf, RS_TP_LEN );
////        api_emv_PutDataElement( DE_TERM, ADDR_TERM_RS_TP_01+RS_TP_LEN*para_cnt, RS_TP_LEN, &sbuf[1] );
//
//          EMVK_FindTag( 0xDF, 0x88, dbuf, sbuf, 2 );
//          sbuf[0] = TL_itoa( sbuf[0]*256+sbuf[1], &sbuf[1] );
//          TL_asc2bcd( 2, &sbuf[100], sbuf );
//
//          api_emv_PutDataElement( DE_TERM, ADDR_TERM_RS_TP_01+RS_TP_LEN*para_cnt, RS_TP_LEN, &sbuf[101] );
          
          // 17. Target Percentage for RS (Tag=DF88)	- NEW NCCC TMS
          EMVK_FindTag( 0xDF, 0x88, dbuf, sbuf, RS_TP_LEN );
          api_emv_PutDataElement( DE_TERM, ADDR_TERM_RS_TP_01+RS_TP_LEN*para_cnt, RS_TP_LEN, sbuf );

          // 18. Max Target Percentage for BRS (Tag=DF89)	- OLD NCCC TMS
////        EMVK_FindTag( 0xDF, 0x89, dbuf, sbuf, BRS_MTP_LEN );
////        api_emv_PutDataElement( DE_TERM, ADDR_TERM_BRS_MTP_01+BRS_MTP_LEN*para_cnt, BRS_MTP_LEN, &sbuf[1] );
//
//          EMVK_FindTag( 0xDF, 0x89, dbuf, sbuf, 2 );
//          sbuf[0] = TL_itoa( sbuf[0]*256+sbuf[1], &sbuf[1] );
//          TL_asc2bcd( 2, &sbuf[100], sbuf );
//
//          api_emv_PutDataElement( DE_TERM, ADDR_TERM_BRS_MTP_01+BRS_MTP_LEN*para_cnt, BRS_MTP_LEN, &sbuf[101] );

	  // 18. Max Target Percentage for BRS (Tag=DF89)	- NEW NCCC TMS
	  EMVK_FindTag( 0xDF, 0x89, dbuf, sbuf, BRS_MTP_LEN );
	  api_emv_PutDataElement( DE_TERM, ADDR_TERM_BRS_MTP_01+BRS_MTP_LEN*para_cnt, BRS_MTP_LEN, sbuf );

          // 19. ?? Max Number of decimals according to the currency code (Tag=DF8A)
          iLen2 = 0;
          api_emv_PutDataElement( DE_TERM, ADDR_TERM_MAX_DECIMAL, 2, (UCHAR *)&iLen2 );

          if( EMVK_FindTag( 0xDF, 0x8A, dbuf, sbuf, 1 ) == TRUE )
            {
            iLen2 = 1;
            api_emv_PutDataElement( DE_TERM, ADDR_TERM_MAX_DECIMAL, 2, (UCHAR *)&iLen2 );
            api_emv_PutDataElement( DE_TERM, ADDR_TERM_MAX_DECIMAL+2, 1, sbuf );
            }

          // 20. ?? Terminal Entry Capability (Tag=DFEE) -- RFU
          iLen2 = 0;
          api_emv_PutDataElement( DE_TERM, ADDR_TERM_ENTRY_CAP, 2, (UCHAR *)&iLen2 );

          if( EMVK_FindTag( 0xDF, 0xEE, dbuf, sbuf, 1 ) == TRUE )
            {
            iLen2 = 1;
            api_emv_PutDataElement( DE_TERM, ADDR_TERM_ENTRY_CAP, 2, (UCHAR *)&iLen2 );
            api_emv_PutDataElement( DE_TERM, ADDR_TERM_ENTRY_CAP+2, 1, sbuf );
            }

	  // 21. default DDOL (Tag=DF00)	// 2010-04-09
	  ptrobj = apk_FindTag( 0xDF, 0x00, dbuf );
	  if( ptrobj != 0 )
            {
            iLen2 = apk_GetBERLEN( ptrobj, &cnt ); // length of DDOL
            ptrobj += cnt; // ptr to V[AID]
            
            api_emv_PutDataElement( DE_TERM, ADDR_TERM_DDOL, 2, (UCHAR *)&iLen2 );
            api_emv_PutDataElement( DE_TERM, ADDR_TERM_DDOL+2, iLen2, ptrobj );	// 2010-04-13, typo
            }

	  // 22. default TDOL (Tag=DF01)	// 2010-04-09
	  ptrobj = apk_FindTag( 0xDF, 0x01, dbuf );
	  if( ptrobj != 0 )
            {
            iLen2 = apk_GetBERLEN( ptrobj, &cnt ); // length of TDOL
            ptrobj += cnt; // ptr to V[AID]
            
            api_emv_PutDataElement( DE_TERM, ADDR_TERM_TDOL, 2, (UCHAR *)&iLen2 );
            api_emv_PutDataElement( DE_TERM, ADDR_TERM_TDOL+2, iLen2, ptrobj );	// 2010-04-13, typo
            }
            
          // --------------------------------------------------------------
          lAddr += (iLen + 2); // ptr to next record
          para_cnt++;
          goto GetRecord;
          }
        else
          {
          switch( para_cnt )
                {
                case 0: // VSDC
                     sbuf[0] = 0x08;
                     sbuf[1] = 0x01;	// 2011-07-19, partial matching is allowed
                     sbuf[2] = 0xA0;
                     sbuf[3] = 0x00;
                     sbuf[4] = 0x00;
                     sbuf[5] = 0x00;
                     sbuf[6] = 0x03;
                     sbuf[7] = 0x10;
                     sbuf[8] = 0x10;
                     break;

                case 1: // MCHIP
                     sbuf[0] = 0x08;
                     sbuf[1] = 0x01;	// 2011-07-19, partial matching is allowed
                     sbuf[2] = 0xA0;
                     sbuf[3] = 0x00;
                     sbuf[4] = 0x00;
                     sbuf[5] = 0x00;
                     sbuf[6] = 0x04;
                     sbuf[7] = 0x10;
                     sbuf[8] = 0x10;
                     break;

                case 2: // JSMART
                     sbuf[0] = 0x08;
                     sbuf[1] = 0x01;	// 2011-07-19, partial matching is allowed
                     sbuf[2] = 0xA0;
                     sbuf[3] = 0x00;
                     sbuf[4] = 0x00;
                     sbuf[5] = 0x00;
                     sbuf[6] = 0x65;
                     sbuf[7] = 0x10;
                     sbuf[8] = 0x10;
                     break;

                case 3: // AMEX
                     sbuf[0] = 0x08;
                     sbuf[1] = 0x01;	// 2011-07-19, partial matching is allowed
                     sbuf[2] = 0xA0;
                     sbuf[3] = 0x00;
                     sbuf[4] = 0x00;
                     sbuf[5] = 0x00;
                     sbuf[6] = 0x25;
                     sbuf[7] = 0x10;
                     sbuf[8] = 0x10;
                     break;

		case 4:	// UICC
		     break;
		     
		case 5:	// DISCOVER
                     sbuf[0] = 0x08;
                     sbuf[1] = 0x01;	// 2018-03-13, partial matching is allowed
                     sbuf[2] = 0xA0;
                     sbuf[3] = 0x00;
                     sbuf[4] = 0x00;
                     sbuf[5] = 0x01;
                     sbuf[6] = 0x52;
                     sbuf[7] = 0x30;
                     sbuf[8] = 0x10;
		     break;
		     
                default:
                     return( emvAborted );
                }
          api_emv_PutDataElement( DE_TERM, ADDR_TERM_AID_01+TERM_AID_LEN*para_cnt, 9, sbuf );
          sbuf[0] = 0;
          api_emv_PutDataElement( DE_TERM, ADDR_TERM_AID_01+TERM_AID_LEN*(para_cnt+1), 1, sbuf );
          goto RecStart;
          }
        }
      else
        return( emvOK ); // done

}

// ---------------------------------------------------------------------------
// FUNCTION: set ASI(Application Selection Indicator) of AID.
// INPUT   : aid  - L-V (V=RID+PIX).
//	     flag - flag for ASI partial match.
//		    0: partial match is not allowed. (exact match)
//		    1: partial match is allowed.     (partial match)
// OUTPUT  : none.
// RETURN  : emvOK
//           emvFailed (AID not found or invalid flag)
// NOTE    : this function should be called after AIDs have been setup.
//           eg, after "api_emvk_PutTmsPARA()".
// ---------------------------------------------------------------------------
UCHAR	api_emvk_SetASI( UCHAR *aid, UCHAR flag )
{
//	if( (apk_SetASI( aid, flag ) == apkExactMatch) && ((flag == 0) || (flag == 1)) )
	if( (apk_SetASI( aid, flag ) == apkExactMatch) && ((flag == 0) || (flag == 1) || (flag == 0x80)) )	// 2020-04-09, ASI=0x80: support PSE partial, non-PSE exact
	  return( emvOK );
	else
	  return( emvFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: retrieve the selected public key header (29 bytes).
//           RID[5], INDEX[1], EXP_LEN[1], MOD_LEN[2], SHA1[20]
// INPUT   : none.
// OUTPUT  : header - CAPK header.
// RETURN  : none.
// ---------------------------------------------------------------------------
void  EMVK_GetPublicKeyHeader( UINT index, UCHAR *header )
{
      index--;
      api_emv_GetDataElement( DE_KEY, ADDR_CA_PK_01+CA_PK_LEN*index, CA_PK_HEADER_LEN, header );
}

// ---------------------------------------------------------------------------
// FUNCTION: load public key data to the selected key slot.
// INPUT   : length - length of key data.
//           data   - the related key data.
//           hval   - the hash value to be compared.
//
//                HASH = SHA1( RID + INDEX + MODULUS + EXPONENT )
//                HASH check is skipped if all the values are 0xFF.
//
// OUTPUT  : none.
// REF     : g_dhn_sam
// RETURN  : emvOK
//           emvFailed
// ---------------------------------------------------------------------------
UCHAR EMVK_CheckPublicKeySHA1( UINT length, UCHAR *data, UCHAR *hval )
{
UCHAR hash[20];

      if( TL_memcmpc( hval, 0xff, 20 ) == 0 ) // skip checking HASH?
        return( emvOK );

      if( apk_HASH( SHA1, length, data, hash ) == apkOK )
        {
        if( TL_memcmp( hash, hval, 20 ) == 0 )
          return( emvOK );
        }

      return( emvFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: compare new key data with the existent key headers.
// INPUT   : data - the CAPK header data. (total 29 bytes)
//                  RID(5) + INDEX(1) + EXPLEN(1) + MODLEN(2) + SHA1(20)
// OUTPUT  : none.
// REF     : g_dhn_sam
// RETURN  : emvOK      -- new key.
//           emvFailed  -- not a new key.
// ---------------------------------------------------------------------------
UCHAR EMVK_CheckPublicKeyHeader( UCHAR *data )
{
UCHAR i;
UCHAR head[CA_PK_HEADER_LEN];

      for( i=0; i<MAX_KEY_SLOT_NUM; i++ )
         {
         api_emv_GetDataElement( DE_KEY, ADDR_CA_PK_01+CA_PK_LEN*i, CA_PK_HEADER_LEN, head );

         if( TL_memcmpc( head, 0x00, RID_LEN ) == 0 )
           break; // end of record

         if( TL_memcmp( head, data, CA_PK_HEADER_LEN ) == 0 )
           return( emvFailed );
         }

      return( emvOK );
}

// ---------------------------------------------------------------------------
// FUNCTION: clear all CAPK header in the terminal config memory.
// INPUT   : data - the CAPK header data. (total 29 bytes)
//                  RID(5) + INDEX(1) + EXPLEN(1) + MODLEN(2) + SHA1(20)
// OUTPUT  : none.
// REF     : g_dhn_sam
// RETURN  : none.
// ---------------------------------------------------------------------------
void  EMVK_ClearPublicKeyHeader( void )
{
UCHAR i;
UCHAR head[CA_PK_HEADER_LEN];

      memset( head, 0x00, CA_PK_HEADER_LEN );

      for( i=0; i<MAX_KEY_SLOT_NUM; i++ )
         api_emv_PutDataElement( DE_KEY, ADDR_CA_PK_01+CA_PK_LEN*i, CA_PK_HEADER_LEN, head );
}

// ---------------------------------------------------------------------------
// FUNC  : To put new CA Public Keys to EMV SAM slot. (_SRAM_PAGE_)
// INPUT : buf  - where the parameters are stored in SRAM page.
//                UCHAR page   : page number.
//                ULONG address: beginning address.
//
//                Data structure: (max 16KB)
//                LEN1[2] CAPK1[n1]   <-- 1'st record
//                LEN2[2] CAPK2[n2]   <-- 2'nd record
//                .................
//                LENn[2] CAPKn[nn]   <-- the last record
//                0x00 0x00           <-- end of record
//
//                Where:
//                CAPK = CardScheme[1]    +
//                       TestFlag[1]      +
//                       KeyIndex[1]      +
//                       ExpoLen[1]       +
//                       ExpoValue[~3]    +
//                       KeyLen[1]        +
//                       KeyValue[~248]   +
//                       KeyHash[20]
//
// OUTPUT: none.
// RETURN: emvOK
//         emvFailed       - out of memory.
//         emvAborted      - invalid key data (eg. HASH error).
//         emvOutOfService - device error.
//
// NOTE  : 1. This function is called once by AP after EMV CAPK's are downloaded
//            from TMS to terminal.
//         2. The SAM session must have been opened in advance.
// ---------------------------------------------------------------------------
UCHAR api_emvk_PutTmsCAPK( UCHAR *buf )
{
UINT  i, j;
UINT  iFID;
UINT  iLen;
UINT  iRecLen;
ULONG lAddr;
UCHAR modlen;
UCHAR explen;
UCHAR result;
UCHAR sbuf[258];
UCHAR dbuf[280];
UCHAR rid[5];

API_SRAM pSram;


      // 1. Clear all current CAPKs
      EMVK_ClearPublicKeyHeader();
      lAddr = buf[1] + buf[2]*256; // the start page address
      iFID = 1;                    // init FID offset

      // ---------------------------------------------------------------
      // 2. Scan all key FIDs to find or create a free key slot
ScanFID:

      // get record LEN[2]     
      pSram.StPage = buf[0];		// page no.
      pSram.StAddr = lAddr;		// addr
      pSram.Len = 2;			// length
      
      api_sram_PageRead( pSram, dbuf );	// get LEN(2)
      if((dbuf[0]==0x30)&&(dbuf[1]==0x30))
      {
        dbuf[0]=0;
        dbuf[1]=0;
      }
      iRecLen = dbuf[0] + dbuf[1]*256; // get record length - LEN[2]

      if( iRecLen == 0 ) // end of record
        {
        result = emvOK;
        goto PutCapkExit;
        }
      lAddr += 2; // ptr to CAPK[]
      
      pSram.StPage = buf[0];		// page no.
      pSram.StAddr = lAddr;		// addr
      pSram.Len = iRecLen;		// length
      api_sram_PageRead( pSram, dbuf );	// get CAPK[n]
      // determine RID by the Card Scheme
      switch( dbuf[0] ) // Card Scheme
            {
            case 0: // VSDC

                 rid[0] = 0xA0; // RID[5]
                 rid[1] = 0x00; //
                 rid[2] = 0x00; //
                 rid[3] = 0x00; //
                 rid[4] = 0x03; //

                 break;

            case 1: // MCHIP

                 rid[0] = 0xA0; // RID[5]
                 rid[1] = 0x00; //
                 rid[2] = 0x00; //
                 rid[3] = 0x00; //
                 rid[4] = 0x04; //

                 break;

            case 2: // JSMART

                 rid[0] = 0xA0; // RID[5]
                 rid[1] = 0x00; //
                 rid[2] = 0x00; //
                 rid[3] = 0x00; //
                 rid[4] = 0x65; //

                 break;

            case 3: // AMEX

                 rid[0] = 0xA0; // RID[5]
                 rid[1] = 0x00; //
                 rid[2] = 0x00; //
                 rid[3] = 0x00; //
                 rid[4] = 0x25; //

                 break;

	    case 4: // UICC
	    
                 rid[0] = 0xA0; // RID[5]
                 rid[1] = 0x00; //
                 rid[2] = 0x00; //
                 rid[3] = 0x03; //
                 rid[4] = 0x33; //

                 break;

	    case 5: // DISCOVER, 2018-05-21
	    
                 rid[0] = 0xA0; // RID[5]
                 rid[1] = 0x00; //
                 rid[2] = 0x00; //
                 rid[3] = 0x01; //
                 rid[4] = 0x52; //

                 break;

            default: // not suppored

                 result = emvAborted;
                 goto PutCapkExit;
            }
      // prepare HASH input data (RID + INDEX + MODULUS + EXPONENT)
      memmove( sbuf, rid, 5 ); // RID
      iLen = 5;

      sbuf[5] = dbuf[2]; // INDEX
      iLen += 1;
      explen = dbuf[3];  // exp length

      j = dbuf[3] + 4;   // mod length
     
      modlen = dbuf[j];  //

      if( (modlen == 0) || (modlen > 248) || (explen == 0) || (explen > 3) )
        {
        result = emvAborted;
        goto PutCapkExit;
        }
      
      j = j + modlen + 1; // SHA1
      memmove( &sbuf[6], &dbuf[5+explen], modlen ); // MODULUS
      iLen += modlen;
      memmove( &sbuf[6+modlen], &dbuf[4], explen ); // EXPONENT
      iLen += explen;

      if( (iRecLen - (5+explen+modlen)) == 20 )
        {
        // check HASH values
        if( EMVK_CheckPublicKeySHA1( iLen, sbuf, &dbuf[j] ) != emvOK )
          {
          result = emvAborted; // invalid hash values
          goto PutCapkExit;
          }
        }

      // look up the existent CAPK header (RID + INDEX + EXPLEN + MODLEN + SHA1)
      sbuf[6] = explen;
      sbuf[7] = 0x00;
      sbuf[8] = modlen;
      
      memmove( &sbuf[9], &dbuf[j], 20 );

      if( EMVK_CheckPublicKeyHeader( sbuf ) == emvFailed ) // same CAPK found?
        goto NextCapk;

      // find an empty key slot for the new CAPK
      for(  ; iFID<(MAX_KEY_SLOT_NUM+1); iFID++ )
         {
         EMVK_GetPublicKeyHeader( iFID, sbuf );

         // select public key (Select_Pub)
         if( TL_memcmpc( sbuf, 0x00, CA_PK_HEADER_LEN ) == 0 ) // file not found?
           {
           break; // new one slot is created
           }
         else // old key slot is found, check its status (free or deleted)
           {
           if( TL_memcmpc( sbuf, 0x00, 5 ) == 0 ) // free?
             break;
           if( TL_memcmpc( sbuf, 0xff, 5 ) == 0 ) // deleted?
             break;
           }
         } // for(iFID)

      if( iFID >= (MAX_KEY_SLOT_NUM+1) )
        {
        result = emvFailed;
        goto PutCapkExit;
        }

      // final selection
      // 3. Save the new CAPK to SRAM

      // load HEADER[29] + EXPONENT[1 or 3] & MODULUS[]
      memmove( sbuf, rid, 5 ); // RID

      sbuf[5] = dbuf[2]; // index
      sbuf[6] = dbuf[3]; // exp length
      explen = dbuf[3];  //

      j = dbuf[3] + 4;   // mod length
      sbuf[7] = 0x00;    //
      sbuf[8] = dbuf[j]; //
      modlen = dbuf[j];  //

      j = j + modlen + 1; // ptr to SHA1
      memmove( &sbuf[9], &dbuf[j], 20 ); // real HASH value

      memmove( &sbuf[29], &dbuf[4], explen ); // exp

      // save HEADER+EXP
      apk_WriteRamDataKEY( ADDR_CA_PK_01+CA_PK_LEN*(iFID-1), 29+explen, sbuf );
      memmove( sbuf, &dbuf[5+explen], modlen ); // load MODULUS

      // save MODULUS
      apk_WriteRamDataKEY( ADDR_CA_PK_01+CA_PK_LEN*(iFID-1)+(29+explen), modlen, sbuf );
NextCapk:

      iFID++;
      lAddr += iRecLen;
      goto ScanFID; // next record

PutCapkExit:

      return( result );
}

// ---------------------------------------------------------------------------
// FUNC  : save IMEK & IAEK keys to SAM.
// INPUT : IMEK, IAEK - the key value. (L-V)
// OUTPUT: none.
// RETURN: emvOK
//         emvFailed
//         emvOutOfService
// ---------------------------------------------------------------------------
UCHAR	api_emvk_PutWaveIEK( UCHAR *imek, UCHAR *iaek )
{
	if( PED_PutWaveIEK( imek, iaek ) == apiOK )
	  return( emvOK );
	else
	  return( emvFailed );
}

// ---------------------------------------------------------------------------
// FUNC  : get IMEK & IAEK keys from SAM.
// INPUT : none.
// OUTPUT: IMEK, IAEK - the key value. (L-V)
// RETURN: emvOK
//         emvFailed
//         emvOutOfService
// ---------------------------------------------------------------------------
UCHAR	api_emvk_GetWaveIEK( void )
{
	// to be implemented
	return( emvOutOfService );
}

// ---------------------------------------------------------------------------
// FUNC  : get IMEK from SRAM.
// INPUT : none.
// OUTPUT: IMEK - the key value. (L-V)
// RETURN: emvOK
//         emvFailed
// ---------------------------------------------------------------------------
UCHAR	api_emvk_GetWaveIMEK( UCHAR *imek )
{
	if( PED_GetWaveIMEK( imek ) == apiOK )
	  return( emvOK );
	else
	  return( emvFailed );
}

// ---------------------------------------------------------------------------
// FUNC  : get IMEK from SRAM.
// INPUT : none.
// OUTPUT: IMEK - the key value. (L-V)
// RETURN: emvOK
//         emvFailed
// ---------------------------------------------------------------------------
UCHAR	api_emvk_GetWaveIAEK( UCHAR *iaek )
{
	if( PED_GetWaveIAEK( iaek ) == apiOK )
	  return( emvOK );
	else
	  return( emvFailed );
}

// ---------------------------------------------------------------------------
// FUNC  : get PIN type. (offline or online)
// INPUT : none.
// OUTPUT: none.
// RETURN: 0 = offline PIN
//         1 = online  PIN
// ---------------------------------------------------------------------------
UCHAR	api_emvk_GetPinType( void )
{
	return( g_emv_ped_type );
}

// ---------------------------------------------------------------------------
// FUNCTION: increment "Transaction Sequence Counter" by one.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	api_emvk_IncTransSequenceCounter( void )
{
UCHAR	buf[16];


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
// FUNC  : set COM port for external pin pad device.
// INPUT : port
// OUTPUT: 
// RETURN: emvOK
//         emvFailed (invalid port number)
// NOTE  : This function should be called before api_emvk_CardholderVerification().
// ---------------------------------------------------------------------------
UCHAR	api_emvk_SetPinPadPort( UCHAR port )
{
//	return( api_ped_SetPinPadPort( port ) );	// 2016-10-11
	return( emvOK );
}

