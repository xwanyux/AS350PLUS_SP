//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS330_mPOS						    **
//**                                                                        **
//**  FILE     : REVISION.H                                                 **
//**  MODULE   : Revision of product.			                    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2018/08/02                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2018 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _GDATAEX_H_
#define _GDATAEX_H_

#include "POSAPI.h"

extern UCHAR	g_kernel;
extern UCHAR	g_user_define;
//----------------------------------------------------------------------------
extern UCHAR    g_ibuf[];
extern UCHAR    g_obuf[];
extern UCHAR    g_temp[];

extern UCHAR    g_dhn_kbd;
extern UCHAR    g_dhn_buz_1s;
extern UCHAR    g_dhn_buz_1l;
extern UCHAR    g_dhn_icc;
extern UCHAR    g_dhn_sam;
extern UCHAR    g_dhn_aux;
extern UCHAR    g_dhn_pinpad;
extern UCHAR    g_dhn_msr;

extern UCHAR    g_beep_on;

extern UCHAR    g_candidate_list_head;
extern UCHAR    g_candidate_list_tail;
extern UCHAR    g_candidate_list_index;
extern UCHAR    g_candidate_name_index;
extern UCHAR    g_selected_aid_index;
extern UCHAR    g_api[];

extern UCHAR    g_icc_AIP[];

extern UCHAR    g_term_TVR[];
extern UCHAR    g_term_TSI[];
extern UCHAR    g_term_tx_amt[];
extern UCHAR    g_term_tx_exp;
extern UCHAR    g_term_decimal_point;
extern UCHAR    g_term_tx_type;
extern UCHAR    g_term_CVMR[];
//extern UCHAR    g_term_tx_seqno[];
extern UINT     g_term_tx_log_cnt;
extern UCHAR    g_term_mode_offline;

extern UCHAR    g_term_ARC[];
extern UCHAR    g_isu_authen_data[];
//extern UCHAR    g_isu_script_temp[];
extern UINT     g_isu_script_addr;
extern UCHAR    g_online_complete;
extern UCHAR    g_forced_online;

extern UINT     g_prt_que_head;
extern UINT     g_prt_que_tail;

extern UINT     g_key_fid;
extern UCHAR    g_capk_cnt;
extern UCHAR    g_rsa_sam;

//extern UCHAR    g_cdol_updn;

extern UCHAR    g_test_read_app_data;           // DEBUG USE ONLY
extern UCHAR    g_test_flag;                    // DEBUG USE ONLY
extern UCHAR    g_pre_auth;
extern UCHAR    g_isr_len_overrun;
extern UCHAR    g_fallback;

extern UCHAR	g_iso_format;
//extern UINT	    g_iso_format;
extern UCHAR	g_key_index;
extern UCHAR    g_mac_key_index;

extern UCHAR    g_pse;

extern	UCHAR	g_emv_ped_src;
extern	UCHAR	g_emv_ped_algo;
extern	UCHAR	g_emv_ped_mode;
extern	UCHAR	g_emv_ped_index;
extern	UCHAR	g_emv_ped_type;

extern	UCHAR	g_occurrence;

extern	UCHAR	g_emv_SW1;
extern	UCHAR	g_emv_SW2;

extern	UCHAR	g_emv_ccl_SW1;
extern	UCHAR	g_emv_ccl_SW2;

//----------------------------------------------------------------------------
#endif
