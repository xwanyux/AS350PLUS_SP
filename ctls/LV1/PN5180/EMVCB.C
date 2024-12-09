//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2 KERNEL CallBack Functions.                          **
//**  PRODUCT  : AS320	                                                    **
//**                                                                        **
//**  FILE     : EMVCB.C	                                            **
//**  MODULE   :                                                            **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2009/06/10                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2009-2011 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
// NOTE: The following functions are implemented on the APP layer.
//       "MANDATORY" functions shall be implemented for the EMV transaction.
//       "OPTIONAL"  functions could be implemented according to local spec.
//
//----------------------------------------------------------------------------
#include <string.h>

#include "POSAPI.H"
#include "GLOBALVAR.H"

//#include "EMVAPI.H"
//#include "EMVDC.H"


// -------------------------------------------------------------------------------------------------
// FUNC  : (MANDATORY) Request to do ONLINE process.
// INPUT : none.
// OUTPUT: arc - an2, authorization response code. (Tag 8A, an2, fixed 2 bytes - ISO8583 Field_39)
//	   air - an6, Authorisation Identifier Response. (Tag 89, fixed 6 bytes - ISO8583 Field_38)
//         rmsg- the response message of ISO8583 Field_55 from issuer,
//		 containing:
//               LLLVAR, LLL=length of VAR (BCD format, fixed 2 bytes)
//			     eg, (0x01 0x20) = 120 bytes
//                       VAR=IAD[] + IST[] as defined below. (in TLV format, max 255 bytes)
//                           IAD - issuer authentication data. (Tag 91)
//                           IST - issuer script template 1 and 2. (Tag 71 and/or Tag 72)
// RETURN: apiOK     - online finished.
//         apiFailed - unable to go online.
// -------------------------------------------------------------------------------------------------
UCHAR	EMV_CB_OnlineProcessing( UCHAR *arc, UCHAR *air, UCHAR *rmsg )
{
	// NOTE: the following codes are for demo only.
	arc[0] = 0x30;
	arc[1] = 0x30;
	
	air[0] = 0x31;
	air[1] = 0x32;
	air[2] = 0x33;
	air[3] = 0x34;
	air[4] = 0x35;
	air[5] = 0x36;
	
	rmsg[0] = 0;
	rmsg[1] = 0;
	
	return( apiOK );
}

// -------------------------------------------------------------------------------------------------
// FUNC  : (MANDATORY) To find the most recent log entry with the same specified PAN.
// INPUT : pan - the PAN to be compared. (Tag 5A, cn19, fixed 10 bytes)
// OUTPUT: amt - the authorized amount for the found PAN. (Tag 9F02, n12, fixed 6 bytes)
//		 (1) eg, (0x00 0x00 0x00 0x00 0x01 0x25) = $1.25
//		 (2) please clear "amt" to 0s if PAN is not found.
// RETURN: apiOK     - found.
//         apiFailed - not found.
// -------------------------------------------------------------------------------------------------
UCHAR	EMV_CB_FindMostRecentPAN( UCHAR *pan, UCHAR *amt )
{
//20130619 Remove Warning
pan[0]=pan[0];
amt[0]=amt[0];
return apiFailed;
#if	0

	// NOTE: the following codes are for demo only.
//	memset( amt, 0x00, 6 );
//	return( apiFailed );

	// -----------------------------------------------------------------------------------------
UINT  i;
UINT  iLen;
//UCHAR pan[10];
//UCHAR amt[6];
UCHAR log[TX_LOG_LEN];


      if( EMVDC_term_tx_log_cnt != 0 ) // tx log available?
        {
        // get current PAN ("pan" is left-justified & padded with trailing 'F'.
        memset( pan, 0xFF, 10 );
        api_emv_GetDataElement( DE_ICC, ADDR_ICC_AP_PAN, 2, (UCHAR *)&iLen );
        api_emv_GetDataElement( DE_ICC, ADDR_ICC_AP_PAN+2, iLen, pan );

        // scan all logs "backwardly" to find the same PAN

        i = EMVDC_term_tx_log_cnt;
        do{

          EMVDC_GetTransLog( --i, log );

          if( TL_memcmp( pan, &log[LOG_AP_PAN], iLen ) == 0 ) // same PAN?
            {
            memmove( amt, &log[LOG_TX_AMT], 6 ); // get amount value
            break;
            }

          } while( i != 0 );
        }
#endif
}

// -------------------------------------------------------------------------------------------------
// FUNC  : (OPTIONAL) Request to do REFERRAL (call bank) process.
// INPUT : none.
// OUTPUT: none.
// RETURN: apiOK      - approved.
//         apiFailed  - declined.
// -------------------------------------------------------------------------------------------------
UCHAR	EMV_CB_ReferralProcessing( void )
{
	// NOTE: the following codes are for demo only.
	return( apiFailed );
}

// -------------------------------------------------------------------------------------------------
// FUNC  : (OPTIONAL) Show "ENTER PIN" message in local language.
// INPUT : none.
// OUTPUT: none.
// RETURN: none.
// NOTE  : The default prompts are in English as shown below. (ROW 0~2)
//	   ROW 3 is controlled by the PIN pad system.
//
//		 0123456789012345 (COLUMN)
//		+----------------+
//	(ROW) 0 |TOTAL		 |
//	      1 |	     $amt|
//	      2 |ENTER PIN:	 |
//	      3 |****		 |
//		+----------------+
// -------------------------------------------------------------------------------------------------
void	EMV_CB_ShowMsg_ENTER_PIN( void )
{
}

// -------------------------------------------------------------------------------------------------
// FUNC  : (OPTIONAL) Show "ENTER PIN" message in local language to request PIN entry
//	              from the external PIN pad.
// INPUT : none.
// OUTPUT: none.
// RETURN: none.
// NOTE  : The default prompts are in English as shown below. (ROW 0~3)
//
//		 0123456789012345 (COLUMN)
//		+----------------+
//	(ROW) 0 |		 |
//	      1 |	     	 |
//	      2 |		 |
//	      3 |   ENTER PIN	 |
//		+----------------+
// -------------------------------------------------------------------------------------------------
void	EMV_CB_ShowMsg_ENTER_PIN_BY_EXT_PINPAD( void )
{
}

// -------------------------------------------------------------------------------------------------
// FUNC  : (OPTIONAL) Show "PIN OK" message in local language.
// INPUT : none.
// OUTPUT: none.
// RETURN: none.
// NOTE  : The default prompts are in English as shown below. (ROW 0~3)
//
//		 0123456789012345 (COLUMN)
//		+----------------+
//	(ROW) 0 |		 |
//	      1 |	     	 |
//	      2 |		 |
//	      3 |     PIN OK	 |
//		+----------------+
// -------------------------------------------------------------------------------------------------
void	EMV_CB_ShowMsg_PIN_OK( void )
{
}

// -------------------------------------------------------------------------------------------------
// FUNC  : (OPTIONAL) Show "INCORRECT PIN" message in local language.
// INPUT : none.
// OUTPUT: none.
// RETURN: none.
// NOTE  : The default prompts are in English as shown below. (ROW 0~3)
//
//		 0123456789012345 (COLUMN)
//		+----------------+
//	(ROW) 0 |		 |
//	      1 |	     	 |
//	      2 |		 |
//	      3 | INCORRECT PIN	 |
//		+----------------+
// -------------------------------------------------------------------------------------------------
void	EMV_CB_ShowMsg_INCORRECT_PIN( void )
{
}

// -------------------------------------------------------------------------------------------------
// FUNC  : (OPTIONAL) Show "LAST PIN TRY" message in local language.
// INPUT : none.
// OUTPUT: none.
// RETURN: none.
// NOTE  : The default prompts are in English as shown below. (ROW 0~3)
//
//		 0123456789012345 (COLUMN)
//		+----------------+
//	(ROW) 0 |		 |
//	      1 |	     	 |
//	      2 |		 |
//	      3 |  LAST PIN TRY	 |
//		+----------------+
// -------------------------------------------------------------------------------------------------
void	EMV_CB_ShowMsg_LAST_PIN_TRY( void )
{
}

// -------------------------------------------------------------------------------------------------
// FUNC  : (OPTIONAL) Show "PLEASE WAIT" message in local language.
// INPUT : none.
// OUTPUT: none.
// RETURN: none.
// NOTE  : The default prompts are in English as shown below. (ROW 0~3)
//
//		 0123456789012345 (COLUMN)
//		+----------------+
//	(ROW) 0 |		 |
//	      1 |	     	 |
//	      2 |		 |
//	      3 |  PLEASE WAIT	 |
//		+----------------+
// -------------------------------------------------------------------------------------------------
void	EMV_CB_ShowMsg_PLEASE_WAIT( void )
{
}

// -------------------------------------------------------------------------------------------------
