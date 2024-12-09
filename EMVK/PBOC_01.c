//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : PBOC 2.0 LEVEL 2 Debit/Credit                              **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : PBOC_01.C                                                  **
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
#include "POSAPI.h"
#include "UI.h"


// ---------------------------------------------------------------------------
const UCHAR PBOCmsg_insert_card[] =		{"INSERT CARD"};
const UCHAR PBOCmsg_C_insert_card[] =		{ 0xBD,0xD0, 0xB4,0xA1, 0xA4,0x4A, 0xA5,0x64, 0xA4,0xF9 };
const UCHAR PBOCmsg_remove_card[] =		{"REMOVE CARD"};
const UCHAR PBOCmsg_C_remove_card[] =		{ 0xBD,0xD0, 0xA8,0xFA, 0xA5,0x58, 0xA5,0x64, 0xA4,0xF9 };
const UCHAR PBOCmsg_try_again[] =		{"TRY AGAIN"};
const UCHAR PBOCmsg_C_try_again[] =		{ 0xBD,0xD0, 0xAD,0xAB, 0xB8,0xD5 };
const UCHAR PBOCmsg_not_accepted[] =		{"NOT ACCEPTED"};
const UCHAR PBOCmsg_C_not_accepted[] =		{ 0xA4,0xA3, 0xB1,0xB5, 0xA8,0xFC };
const UCHAR PBOCmsg_please_wait[] =		{"PLEASE WAIT"};
const UCHAR PBOCmsg_C_please_wait[] =		{ 0xBD,0xD0, 0xB5,0xA5, 0xAB,0xDD };

const UCHAR PBOCmsg_select[] =			{"SELECT"};
const UCHAR PBOCmsg_C_select[] =		{ 0xBF,0xEF, 0xBE,0xDC };
const UCHAR PBOCmsg_out_of_service[] =		{"OUT OF SERVICE"};

const UCHAR PBOCmsg_C_verify_ch_license[] =	{ 0xbd,0xd0, 0xae,0xd6, 0xb9,0xef, 0xab,0xf9, 0xa5,0x64, 0xa4,0x48, 0xc3,0xd2, 0xa5,0xf3 };
const UCHAR PBOCmsg_C_license_00[] =		{ 0xa8,0xad, 0xa5,0xf7, 0xc3,0xd2 };
const UCHAR PBOCmsg_C_license_01[] =		{ 0xad,0x78, 0xa9,0x78, 0xc3,0xd2 };
const UCHAR PBOCmsg_C_license_02[] =		{ 0xc5,0x40, 0xb7,0xd3 };
const UCHAR PBOCmsg_C_license_03[] =		{ 0xa4,0x4a, 0xb9,0xd2, 0xc3,0xd2 };
const UCHAR PBOCmsg_C_license_04[] =		{ 0xc1,0x7b, 0xae,0xc9, 0xa8,0xad, 0xa5,0xf7, 0xc3,0xd2 };
const UCHAR PBOCmsg_C_license_05[] =		{ 0xa8,0xe4, 0xa5,0xa6 };
const UCHAR PBOCmsg_C_correct_or_not[] =	{ 0xac,0x4f, 0xa7,0x5f, 0xa5,0xbf, 0xbd,0x54, '(','Y','/','N',')','?' };

const UCHAR PBOCmsg_recno[] =			{"RECORD:"};
const UCHAR PBOCmsg_date[] =			{"DATE:"};
const UCHAR PBOCmsg_time[] =			{"TIME:"};
const UCHAR PBOCmsg_amt_auth[] =		{"AMT AUT:"};
const UCHAR PBOCmsg_amt_other[] =		{"AMT OTH:"};
const UCHAR PBOCmsg_country_code[] =		{"COUNTRY CODE :"};
const UCHAR PBOCmsg_currency_code[] =		{"CURRENCY CODE:"};
const UCHAR PBOCmsg_merchant_name[] =		{"MERCHANT NAME:"};
const UCHAR PBOCmsg_trans_type[] =		{"TRANS TYPE:"};
const UCHAR PBOCmsg_atc[] =			{"ATC:"};
const UCHAR PBOCmsg_more[] =			{"(more)"};
const UCHAR PBOCmsg_next[] =			{"(next)"};


// ---------------------------------------------------------------------------
// FUNCTION: display "INSERT CARD"
//
//           __INSERT CARD__
// ---------------------------------------------------------------------------
void PBOC_DISP_insert_card()
{
      UI_PutMsg( 0, COL_MIDWAY, FONT2+attrCLEARWRITE, 10, (UCHAR *)PBOCmsg_C_insert_card );
      UI_PutMsg( 1, COL_MIDWAY, FONT1+attrCLEARWRITE, 11, (UCHAR *)PBOCmsg_insert_card );
}

// ---------------------------------------------------------------------------
// FUNCTION: display "REMOVE CARD"
//
//           __REMOVE CARD__
// ---------------------------------------------------------------------------
void PBOC_DISP_remove_card()
{
      UI_PutMsg( 0, COL_MIDWAY, FONT2+attrCLEARWRITE, 10, (UCHAR *)PBOCmsg_C_remove_card );
      UI_PutMsg( 1, COL_MIDWAY, FONT1+attrCLEARWRITE, 11, (UCHAR *)PBOCmsg_remove_card );
}

// ---------------------------------------------------------------------------
// FUNCTION: display "TRY AGAIN"
//
//           ---TRY AGAIN---
// ---------------------------------------------------------------------------
void PBOC_DISP_try_again()
{
      UI_PutMsg( 0, COL_MIDWAY, FONT2+attrCLEARWRITE, 6, (UCHAR *)PBOCmsg_C_try_again );
      UI_PutMsg( 1, COL_MIDWAY, FONT1+attrCLEARWRITE, 9, (UCHAR *)PBOCmsg_try_again );
}

// ---------------------------------------------------------------------------
// FUNCTION: display "NOT ACCEPTED"
//
//           --NOT ACCEPTED-
// ---------------------------------------------------------------------------
void PBOC_DISP_not_accepted()
{
      UI_PutMsg( 0, COL_MIDWAY, FONT2+attrCLEARWRITE, 6, (UCHAR *)PBOCmsg_C_not_accepted );
      UI_PutMsg( 1, COL_MIDWAY, FONT1+attrCLEARWRITE, 12, (UCHAR *)PBOCmsg_not_accepted );
}

// ---------------------------------------------------------------------------
// FUNCTION: display "PLEASE WAIT"
//
//           __PLEASE WAIT__
// ---------------------------------------------------------------------------
void PBOC_DISP_please_wait()
{
      UI_PutMsg( 0, COL_MIDWAY, FONT2+attrCLEARWRITE, 6, (UCHAR *)PBOCmsg_C_please_wait );
      UI_PutMsg( 1, COL_MIDWAY, FONT1+attrCLEARWRITE, 11, (UCHAR *)PBOCmsg_please_wait );
}

// ---------------------------------------------------------------------------
// FUNCTION: display "SELECT"
//
//           SELECT_________
// ---------------------------------------------------------------------------
void PBOC_DISP_select_app()
{
      UI_PutMsg( 0, COL_RIGHTMOST, FONT2, 4, (UCHAR *)PBOCmsg_C_select );
      UI_PutMsg( 0, COL_LEFTMOST, FONT1, 6, (UCHAR *)PBOCmsg_select );
}

// ---------------------------------------------------------------------------
// FUNCTION: display "OUT OF SERVICE"
//
// ---------------------------------------------------------------------------
void PBOC_DISP_out_of_service()
{
      UI_ClearScreen();
//    UI_PutMsg( 0, COL_MIDWAY, FONT2+attrCLEARWRITE, 8, (UCHAR *)PBOCmsg_C_out_of_service );
      UI_PutMsg( 1, COL_MIDWAY, FONT1, 14, (UCHAR *)PBOCmsg_out_of_service );
}

// ---------------------------------------------------------------------------
void PBOC_DISP_verify_ch_license()
{
      UI_PutMsg( 0, COL_LEFTMOST, FONT2+attrCLEARWRITE, 16, (UCHAR *)PBOCmsg_C_verify_ch_license );
}

// ---------------------------------------------------------------------------
void PBOC_DISP_license_00()
{
      UI_PutMsg( 1, COL_LEFTMOST, FONT2+attrCLEARWRITE, 6, (UCHAR *)PBOCmsg_C_license_00 );
}

// ---------------------------------------------------------------------------
void PBOC_DISP_license_01()
{
      UI_PutMsg( 1, COL_LEFTMOST, FONT2+attrCLEARWRITE, 6, (UCHAR *)PBOCmsg_C_license_01 );
}

// ---------------------------------------------------------------------------
void PBOC_DISP_license_02()
{
      UI_PutMsg( 1, COL_LEFTMOST, FONT2+attrCLEARWRITE, 4, (UCHAR *)PBOCmsg_C_license_02 );
}

// ---------------------------------------------------------------------------
void PBOC_DISP_license_03()
{
      UI_PutMsg( 1, COL_LEFTMOST, FONT2+attrCLEARWRITE, 6, (UCHAR *)PBOCmsg_C_license_03 );
}

// ---------------------------------------------------------------------------
void PBOC_DISP_license_04()
{
      UI_PutMsg( 1, COL_LEFTMOST, FONT2+attrCLEARWRITE, 10, (UCHAR *)PBOCmsg_C_license_04 );
}

// ---------------------------------------------------------------------------
void PBOC_DISP_license_05()
{
      UI_PutMsg( 1, COL_LEFTMOST, FONT2+attrCLEARWRITE, 4, (UCHAR *)PBOCmsg_C_license_05 );
}

// ---------------------------------------------------------------------------
void PBOC_DISP_correct_or_not()
{
      UI_PutMsg( 3, COL_LEFTMOST, FONT2+attrCLEARWRITE, 14, (UCHAR *)PBOCmsg_C_correct_or_not );
}
