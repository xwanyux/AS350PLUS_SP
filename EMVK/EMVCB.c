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
//**  Copyright(C) 2009 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
// NOTE: The MANDATORY functions must be implemented for EMV transaction.
//       The OPTIONAL functions must be implemented only if "UserDefinedMsg = TURE" 
//	 in the api_emvk_InitKernel( UCHAR *tid, UCHAR UserDefinedMsg ).
//
//----------------------------------------------------------------------------
#include "POSAPI.h"




// -------------------------------------------------------------------------------------------------
// FUNC  : (MANDATORY) Request to do ONLINE transaction.
// INPUT : none.
// OUTPUT: arc - an2, authorization response code. (Tag 8A, an2, fixed 2 bytes - ISO8583 Field_39)
//	   air - an6, Authorisation Identifier Response. (Tag 89, fixed 6 bytes - ISO8583 Field_38)
//         rmsg- the response message of ISO8583 Field_55 from issuer,
//		 containing:
//               LLLVAR, LLL=length of VAR(TLVs) <=255 bytes
//                       VAR=IAD[] + IST[] as defined below.
//                           iad - issuer authentication data.
//                           ist - issuer script template 1 and 2.
// RETURN: apiOK     - online finished.
//         apiFailed - unable to go online.
// NOTE  : This function is to be implemented from AP level.
// -------------------------------------------------------------------------------------------------
#if 0
UCHAR	EMV_CB_OnlineProcessing( UCHAR *arc, UCHAR *air, UCHAR *rmsg )
{
	return( apiFailed );
}
#endif

// -------------------------------------------------------------------------------------------------
// FUNC  : (MANDATORY) Request to do REFERRAL transaction.
// INPUT : none.
// OUTPUT: none.
// RETURN: apiOK      - approved.
//         apiFailed  - declined.
// NOTE  : This function is to be implemented from AP level.
// -------------------------------------------------------------------------------------------------
#if 0
UCHAR	EMV_CB_ReferralProcessing( void )
{
	return( apiFailed );
}
#endif

// -------------------------------------------------------------------------------------------------
// FUNC  : (MANDATORY) To find the most recent log entry with the same specified PAN.
// INPUT : pan - the PAN to be compared. (Tag 5A, cn19, fixed 10 bytes)
// OUTPUT: amt - the authorized amount for the found PAN. (Tag 9F02, n12, fixed 6 bytes)
// RETURN: apiOK     - found.
//         apiFailed - not found.
// NOTE  : This function is to be implemented from AP level.
// -------------------------------------------------------------------------------------------------
#if 0
UCHAR	EMV_CB_FindMostRecentPAN( UCHAR *pan, UCHAR *amt )
{
	return( apiOK );
}
#endif
// -------------------------------------------------------------------------------------------------
// FUNC  : (MANDATORY) To obtain transaction amount.
// INPUT : none.
// OUTPUT: type - transaction type. (n2, fixed 1 byte, first two digits of ISO8583-Processing Code)
//	   sc   - transaction sequence number. (n8, fixed 4 bytes)
//		  maintained by application program and incremented by one for each transaction.
//         amt  - transaction amount entered. (n12, fixed 6 bytes)
//		  the first 5 bytes are integer part, the last byte is decimal part.
// RETURN: apiOK
//	   apiFailed (aborted)
// NOTE  : This function is to be implemented from AP level.
//	   This function is called if the "Transaction Amount" is not available
//	   before calling "api_emvk_DoTransaction()".
//	   This function shall prompt the "ENTER AMOUNT" to request to enter transaction amount.
// -------------------------------------------------------------------------------------------------
#if 0
UCHAR	EMV_CB_GetAmount( UCHAR *amt, UCHAR *sc )
{
	return( apiFailed );
}
#endif

// -------------------------------------------------------------------------------------------------
// FUNC  : (OPTIONAL) To show a list and request for selecting an application.
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
// -------------------------------------------------------------------------------------------------
#if 1
UCHAR	EMV_CB_SelectApplication( UCHAR list_cnt, UCHAR *list )
{
	return( 255 );
}
#endif

// -------------------------------------------------------------------------------------------------
// FUNC  : (OPTIONAL) Show "TRY AGAIN" message in local language. (FONT1, MIDWAY)
// INPUT : none.
// OUTPUT: none.
// RETURN: none.
// NOTE  : This function is to be implemented from AP level.
// -------------------------------------------------------------------------------------------------
#if 1
void	EMV_CB_ShowMsg_TRY_AGAIN( void )
{
}
#endif

// -------------------------------------------------------------------------------------------------
// FUNC  : (OPTIONAL) Show "NOT ACCEPTED" message in local language. (FONT1, MIDWAY)
// INPUT : none.
// OUTPUT: none.
// RETURN: none.
// NOTE  : This function is to be implemented from AP level.
// -------------------------------------------------------------------------------------------------
#if 1
void	EMV_CB_ShowMsg_NOT_ACCEPTED( void )
{
}
#endif

// -------------------------------------------------------------------------------------------------
// FUNC  : (OPTIONAL) Show "PLEASE WAIT" message in local language. (FONT1, MIDWAY)
// INPUT : none.
// OUTPUT: none.
// RETURN: none.
// NOTE  : This function is to be implemented from AP level.
// -------------------------------------------------------------------------------------------------
#if 0
void	EMV_CB_ShowMsg_PLEASE_WAIT( void )
{
}
#endif

// -------------------------------------------------------------------------------------------------
// FUNC  : (OPTIONAL) Show "SELECT" message in local language at the first row. (FONT1, RIGHTMOST)
// INPUT : none.
// OUTPUT: none.
// RETURN: none.
// NOTE  : This function is to be implemented from AP level.
// -------------------------------------------------------------------------------------------------
#if 1
void	EMV_CB_ShowMsg_SELECT( void )
{
}
#endif
// -------------------------------------------------------------------------------------------------
// FUNC  : (OPTIONAL) Show "INCORRECT PIN" message in local language at the first 3 rows. (FONT1)
// INPUT : none.
// OUTPUT: none.
// RETURN: none.
// NOTE  : This function is to be implemented from AP level.
// -------------------------------------------------------------------------------------------------
#if 0
void	EMV_CB_ShowMsg_INCORRECT_PIN( void )
{
}
#endif 0

// -------------------------------------------------------------------------------------------------
// FUNC  : (OPTIONAL) Show "LAST PIN TRY" message in local language at the first 3 rows. (FONT1)
// INPUT : none.
// OUTPUT: none.
// RETURN: none.
// NOTE  : This function is to be implemented from AP level.
// -------------------------------------------------------------------------------------------------
#if 0
void	EMV_CB_ShowMsg_LAST_PIN_TRY( void )
{
}
#endif
// -------------------------------------------------------------------------------------------------

