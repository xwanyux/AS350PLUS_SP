//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : PBOC.H                                                     **
//**  MODULE   : Declaration of related message string.                     **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2009/02/03                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2009 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _PBOC_H_
#define _PBOC_H_


//----------------------------------------------------------------------------
//      Message Definitions
//----------------------------------------------------------------------------
extern	const UCHAR PBOCmsg_insert_card[];
extern	const UCHAR PBOCmsg_C_insert_card[];
extern	const UCHAR PBOCmsg_remove_card[];
extern	const UCHAR PBOCmsg_C_remove_card[];
extern	const UCHAR PBOCmsg_try_again[];
extern	const UCHAR PBOCmsg_C_try_again[];
extern	const UCHAR PBOCmsg_not_accepted[];
extern	const UCHAR PBOCmsg_C_not_accepted[];
extern	const UCHAR PBOCmsg_please_wait[];
extern	const UCHAR PBOCmsg_C_please_wait[];

extern	const UCHAR PBOCmsg_select[];
extern	const UCHAR PBOCmsg_C_select[];
extern	const UCHAR PBOCmsg_out_of_service[];
                                        
extern	const UCHAR PBOCmsg_C_verify_ch_license[];
extern	const UCHAR PBOCmsg_C_license_00[];
extern	const UCHAR PBOCmsg_C_license_01[];
extern	const UCHAR PBOCmsg_C_license_02[];
extern	const UCHAR PBOCmsg_C_license_03[];
extern	const UCHAR PBOCmsg_C_license_04[];
extern	const UCHAR PBOCmsg_C_license_05[];
extern	const UCHAR PBOCmsg_C_correct_or_not[];
                                        
extern	const UCHAR PBOCmsg_recno[];
extern	const UCHAR PBOCmsg_date[];
extern	const UCHAR PBOCmsg_time[];
extern	const UCHAR PBOCmsg_amt_auth[];
extern	const UCHAR PBOCmsg_amt_other[];
extern	const UCHAR PBOCmsg_country_code[];
extern	const UCHAR PBOCmsg_currency_code[];
extern	const UCHAR PBOCmsg_merchant_name[];
extern	const UCHAR PBOCmsg_trans_type[];
extern	const UCHAR PBOCmsg_atc[];
extern	const UCHAR PBOCmsg_more[];
extern	const UCHAR PBOCmsg_next[];

//---------------------------------------------------------------------------- 
//      Prototype Declarations                                                    
//----------------------------------------------------------------------------
extern	void PBOC_DISP_insert_card();
extern	void PBOC_DISP_remove_card();
extern	void PBOC_DISP_try_again();
extern	void PBOC_DISP_not_accepted();
extern	void PBOC_DISP_please_wait();
extern	void PBOC_DISP_select_app();
extern	void PBOC_DISP_out_of_service();
extern	void PBOC_DISP_verify_ch_license();
extern	void PBOC_DISP_license_00();
extern	void PBOC_DISP_license_01();
extern	void PBOC_DISP_license_02();
extern	void PBOC_DISP_license_03();
extern	void PBOC_DISP_license_04();
extern	void PBOC_DISP_license_05();
extern	void PBOC_DISP_correct_or_not();

//----------------------------------------------------------------------------
#endif
