//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS330_mPOS						    **
//**                                                                        **
//**  FILE     : XCONSTP5.H                                                 **
//**  MODULE   : Declaration of related message string.                     **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2014/06/17                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2014 SymLink Corporation. All rights reserved.	    **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _XCONSTP5_H_
#define _XCONSTP5_H_

//#pragma memory=constseg(P5_XCONST)

//----------------------------------------------------------------------------
extern const UCHAR msg_asi[];
extern const UCHAR msg_aid[];
extern const UCHAR msg_null_asi[];
extern const UCHAR msg_null_aid[];
extern const UCHAR msg_0delete_1update[];
extern const UCHAR msg_pls_select_aid[];
extern const UCHAR msg_okstart[];
//extern const UCHAR msg_goods[];
//extern const UCHAR msg_service[];
extern const UCHAR msg_1goods_2services[];

extern const UCHAR msg_recno[];
extern const UCHAR msg_date[];
extern const UCHAR msg_time[];
extern const UCHAR msg_amt_auth[];
extern const UCHAR msg_amt_other[];
extern const UCHAR msg_country_code[];
extern const UCHAR msg_currency_code[];
extern const UCHAR msg_merchant_name[];
extern const UCHAR msg_trans_type[];
extern const UCHAR msg_atc[];
extern const UCHAR msg_more[];
extern const UCHAR msg_next[];

//----------------------------------------------------------------------------
//#pragma memory=default
//----------------------------------------------------------------------------
#endif
