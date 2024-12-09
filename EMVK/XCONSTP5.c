//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS330	                                                    **
//**                                                                        **
//**  FILE     : XCONSTP5.C                                                 **
//**  MODULE   : Declaration of related message string.                     **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2018/08/08                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2018 SymLink Corporation. All rights reserved.	    **
//**                                                                        **
//****************************************************************************
//============================================================================
//============================================================================
//
//----------------------------------------------------------------------------
#include "POSAPI.h"
#include "EMVAPI.h"

//#pragma memory=constseg(P5_XCONST)

//----------------------------------------------------------------------------
//                                        012345678901234567890
const UCHAR msg_asi[] =                 {"ASI="};
const UCHAR msg_aid[] =                 {"AID="};
const UCHAR msg_null_asi[] =            {"__"};
const UCHAR msg_null_aid[] =            {"__ -- -- -- -- -- --"};
const UCHAR msg_0delete_1update[] =     {"0-DELETE  1-UPDATE"};
const UCHAR msg_pls_select_aid[] =      {"PLEASE SELECT AID"};
const UCHAR msg_okstart[] =             {"PRESS [OK] TO START"};
//const UCHAR msg_goods[] =               {"GOODS"};
//const UCHAR msg_services[] =            {"SERVICES"};
const UCHAR msg_1goods_2services[] =    {"1-GOODS   2-SERVICES"};

const UCHAR msg_recno[] =               {"RECORD:"};
const UCHAR msg_date[] =                {"DATE:"};
const UCHAR msg_time[] =                {"TIME:"};
const UCHAR msg_amt_auth[] =            {"AMT AUT:"};
const UCHAR msg_amt_other[] =           {"AMT OTH:"};
const UCHAR msg_country_code[] =        {"COUNTRY CODE :"};
const UCHAR msg_currency_code[] =       {"CURRENCY CODE:"};
const UCHAR msg_merchant_name[] =       {"MERCHANT NAME:"};
const UCHAR msg_trans_type[] =          {"TRANS TYPE:"};
const UCHAR msg_atc[] =                 {"ATC:"};
const UCHAR msg_more[] =                {"(more)"};
const UCHAR msg_next[] =                {"(next)"};

//----------------------------------------------------------------------------
//#pragma memory=default

