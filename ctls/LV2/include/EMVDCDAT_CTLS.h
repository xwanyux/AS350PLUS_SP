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
#ifndef _EMVDCDAT_H_
#define _EMVDCDAT_H_

#include "POSAPI.h"

//----------------------------------------------------------------------------
extern	UCHAR	EMVDC_beep_on;
extern	UCHAR   EMVDC_ibuf[512];
extern	UCHAR   EMVDC_obuf[512];
extern	UCHAR   EMVDC_temp[512];
extern	UCHAR   EMVDC_dhn_kbd;
extern	UCHAR   EMVDC_dhn_buz_1s;
extern	UCHAR   EMVDC_dhn_buz_1l;
extern	UCHAR   EMVDC_dhn_icc;
extern	UCHAR   EMVDC_dhn_aux;
extern	UCHAR   EMVDC_dhn_pinpad;
extern	UCHAR   EMVDC_dhn_msr;
extern	UCHAR	EMVDC_term_tx_exp;
extern	UCHAR	EMVDC_term_decimal_point;
extern	UCHAR	EMVDC_term_tx_type;
extern	UCHAR	EMVDC_forced_online;
extern	UCHAR	EMVDC_term_tx_amt[6];
extern	UCHAR   EMVDC_term_ARC[2];
extern	UCHAR	EMVDC_pos_entry_mode;
extern	UCHAR	EMVDC_tx_date[3];
extern	UCHAR	EMVDC_tx_time[3];
extern	UCHAR	EMVDC_expire_date[3];
extern	UCHAR	EMVDC_cardholder_name[28];
extern	UCHAR	EMVDC_ap_pan[12];
extern	UCHAR	EMVDC_epb[10];
extern	UCHAR	EMVDC_ksn[12];

extern	UCHAR	EMVDC_trans_type;

//----------------------------------------------------------------------------
#endif
