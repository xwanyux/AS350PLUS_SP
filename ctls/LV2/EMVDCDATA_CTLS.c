//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMVDC DEMO APP                                             **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : EMVDCDAT.H                                                 **
//**  MODULE   : Declaration of the global variables.	                    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2007/10/11                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2011 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include "POSAPI.h"

UCHAR	EMVDC_beep_on = 1;
//UCHAR   EMVDC_ibuf[512];
//UCHAR   EMVDC_obuf[512];
//UCHAR   EMVDC_temp[512];
UCHAR   EMVDC_dhn_kbd = 0;
UCHAR   EMVDC_dhn_buz_1s = 0;
UCHAR   EMVDC_dhn_buz_1l = 0;
UCHAR   EMVDC_dhn_icc = 0;
UCHAR   EMVDC_dhn_aux =0;
UCHAR   EMVDC_dhn_pinpad = 0;
UCHAR   EMVDC_dhn_msr = 0;
UCHAR	EMVDC_term_tx_exp = 0;				// transaction currency exponent
UCHAR	EMVDC_term_decimal_point = 0;			// flag to show decimal point
UCHAR	EMVDC_term_tx_type = 0;				// transaction type
UCHAR	EMVDC_forced_online = 0;			// forced online flag
UCHAR   EMVDC_term_tx_amt[6];				// transaction amount (int(n10)+dec(n2), 6-byte)
UCHAR   EMVDC_term_ARC[2];				// authorization response code
UCHAR	EMVDC_pos_entry_mode;				//
UCHAR	EMVDC_tx_date[3];				// transaction date
UCHAR	EMVDC_tx_time[3];				// transaction time
UCHAR	EMVDC_expire_date[3];				// expiry date
UCHAR	EMVDC_cardholder_name[28];			// LL-V
UCHAR	EMVDC_ap_pan[12];				// LL-V, primary account number
UCHAR	EMVDC_epb[10];					// LL-V, encrypted pin block
UCHAR	EMVDC_ksn[12];					// LL-V, key serial number for DUKPT

UCHAR	EMVDC_trans_type = 0;				// transaction type: 0=GOODS & SERVICES, 1=CASH, 9=CASHBACK

//20130423 amount, cashback amount
UCHAR	AmountAuth[6] = {0};		//amount authorized
UCHAR	AmountOther[6] = {0};		//amount other
