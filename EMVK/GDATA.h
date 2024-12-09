
//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS350							    **
//**                                                                        **
//**  FILE     : GDATA.H                                                    **
//**  MODULE   : Global data definition.                                    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2014/06/17                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2014 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _GDATA_H_
#define _GDATA_H_
UCHAR   g_ibuf[300];
UCHAR   g_obuf[300];
UCHAR   g_temp[320];
UCHAR	g_kernel = 0;			// KERNEL_EMV or KERNEL_PBOC	
UCHAR	g_user_define;			// user defined message
UCHAR   g_dhn_kbd;
UCHAR   g_dhn_buz_1s;
UCHAR   g_dhn_buz_1l;
UCHAR   g_dhn_icc = 0xE0;
UCHAR   g_dhn_sam;
UCHAR   g_dhn_aux;
UCHAR   g_dhn_pinpad;
UCHAR   g_dhn_msr;

UCHAR   g_beep_on;                      // 1=turn on buzzer

UCHAR   g_candidate_list_head;
UCHAR   g_candidate_list_tail;
UCHAR   g_candidate_list_index;
UCHAR   g_candidate_name_index;
UCHAR   g_selected_aid_index;
UCHAR   g_api[16];                      // cf. MAX_CANDIDATE_NAME_CNT

UCHAR   g_icc_AIP[2];                   // application interchange profile

UCHAR   g_term_TVR[5];                  // Terminal Verification Results
UCHAR   g_term_TSI[2];                  // Transaction Status Information
UCHAR   g_term_tx_amt[6];               // transaction amount (int(n10)+dec(n2), 6-byte),
                                        // may or may not including tips & adjustments
UCHAR   g_term_tx_exp;                  // transaction exponent (decimal point)
UCHAR   g_term_decimal_point;           // output decimal point or not, if yes, = g_term_tx_exp
UCHAR   g_term_tx_type;                 // transaction type (iso processing code)
UCHAR   g_term_CVMR[3];                 // CVM Rresults
//UCHAR   g_term_tx_seqno[4];             // n8, transaction sequence number
UINT    g_term_tx_log_cnt;              // 0x0000 = no transaction log
UCHAR   g_term_mode_offline;            // 1=terminal only supports offline tx

UCHAR   g_term_ARC[2];                  // an2, authorization response code
UCHAR   g_isu_authen_data[18];          // 2L-V (b16), issuer authentication data
//UCHAR   g_isu_script_temp[274];         // T-L-V, issuer script template (size will be changed)
                                        //        71-L-L-9F18-L-V[4]-86-L-L-V[261]
UINT    g_isu_script_addr;              // storage address of the issuer scripts
UCHAR   g_online_complete;              // online transaction flag
UCHAR   g_forced_online;                // forced transaction online flag

UINT    g_prt_que_head;                 // printer queue buffer head pointer
UINT    g_prt_que_tail;                 //                      tail pointer

UINT    g_key_fid;                      // file id for RSA key (80xx)
UCHAR   g_capk_cnt;                     // total number of CA public keys in use
UCHAR   g_rsa_sam;                      // position of the target RSA SAM (SAM1 or SAM6)

//UCHAR   g_cdol_updn;                    // 1=CDOL contains unpredictable number (9F37)

                                        // -----------------------------------
UCHAR   g_test_read_app_data;           // DEBUG USE ONLY
UCHAR   g_test_flag;                    // DEBUG USE ONLY
UCHAR   g_pre_auth;                     //
UCHAR   g_isr_len_overrun;
UCHAR   g_fallback;

UCHAR	g_iso_format = 0;
//UINT	g_iso_format = 0;
UCHAR	g_key_index = 0;
UCHAR   g_mac_key_index = 0;

UCHAR   g_pse;

UCHAR	g_emv_ped_src = 0;		// 0 = internal PINPAD, 1 = external PINPAD
UCHAR	g_emv_ped_algo = 0;		// key algorithm
UCHAR	g_emv_ped_mode = 0;		// encryption mode
UCHAR	g_emv_ped_index = 0;		// key slot index
UCHAR	g_emv_ped_type = 0;		// 2011-03-27, 0 = offline pin, 1 = online pin.

UCHAR	g_occurrence = 0;		// 2012-02-14 for re-select application process

UCHAR	g_emv_SW1 = 0;			// 2014-05-15
UCHAR	g_emv_SW2 = 0;			//

UCHAR	g_emv_ccl_SW1 = 0;		// 2015-06-04, for "Create Candidate List" process
UCHAR	g_emv_ccl_SW2 = 0;		//
#endif