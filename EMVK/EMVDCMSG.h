//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMVDC DEMO APP                                             **
//**  PRODUCT  : AS330	                                                    **
//**                                                                        **
//**  FILE     : EMVDCMSG.H                                                 **
//**  MODULE   : Declaration of related message string.                     **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2018/08/08                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2008-2018 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _EMVDCMSG_H_
#define _EMVDCMSG_H_


//----------------------------------------------------------------------------
//      Standard Message
//----------------------------------------------------------------------------

extern const UCHAR stdmsg_amount[];
extern const UCHAR stdmsg_C_amount[];
extern const UCHAR stdmsg_amount_ok[];
extern const UCHAR stdmsg_approved[];
extern const UCHAR stdmsg_C_approved[];
extern const UCHAR stdmsg_call_your_bank[];
extern const UCHAR stdmsg_C_call_your_bank[];
extern const UCHAR stdmsg_cancel_or_enter[];
extern const UCHAR stdmsg_card_error[];
extern const UCHAR stdmsg_C_card_error[];
extern const UCHAR stdmsg_declined[];
extern const UCHAR stdmsg_C_declined[];
extern const UCHAR stdmsg_enter_amount[];
extern const UCHAR stdmsg_C_enter_amount[];
extern const UCHAR stdmsg_enter_pin[];
extern const UCHAR stdmsg_C_enter_pin[];
extern const UCHAR stdmsg_C_pin[];
extern const UCHAR stdmsg_incorrect_pin[];
extern const UCHAR stdmsg_C_incorrect_pin[];
extern const UCHAR stdmsg_insert_card[];
extern const UCHAR stdmsg_C_insert_card[];
extern const UCHAR stdmsg_not_accepted[];
extern const UCHAR stdmsg_C_not_accepted[];
extern const UCHAR stdmsg_pin_ok[];
extern const UCHAR stdmsg_C_pin_ok[];
extern const UCHAR stdmsg_please_wait[];
extern const UCHAR stdmsg_C_please_wait[];
extern const UCHAR stdmsg_processing_error[];
extern const UCHAR stdmsg_C_processing_error[];
extern const UCHAR stdmsg_remove_card[];
extern const UCHAR stdmsg_C_remove_card[];
extern const UCHAR stdmsg_use_chip_reader[];
extern const UCHAR stdmsg_C_use_chip_reader[];
extern const UCHAR stdmsg_use_mag_stripe[];
extern const UCHAR stdmsg_try_again[];
extern const UCHAR stdmsg_C_try_again[];
extern const UCHAR stdmsg_last_pin_try[];
extern const UCHAR stdmsg_C_last_pin_try[];

extern const UCHAR stdmsg_terminated[];
extern const UCHAR stdmsg_not_allowed[];

//----------------------------------------------------------------------------
//      General Message
//----------------------------------------------------------------------------
extern const UCHAR msg_version[];
extern const UCHAR msg_emvdc_demo_app[];
extern const UCHAR msg_select[];
extern const UCHAR msg_C_select[];
extern const UCHAR msg_application[];
extern const UCHAR msg_config[];
extern const UCHAR msg_start[];
extern const UCHAR msg_b_ok[];
extern const UCHAR msg_yes_or_no[];
extern const UCHAR msg_cancel_or_enter[];
extern const UCHAR msg_sale[];
extern const UCHAR msg_C_sale[];
extern const UCHAR msg_auth[];
extern const UCHAR msg_C_auth[];
extern const UCHAR msg_pin_pad[];
extern const UCHAR msg_pan[];
extern const UCHAR msg_C_pan[];
extern const UCHAR msg_1approve_2decline[];
extern const UCHAR msg_approve[];
extern const UCHAR msg_decline[];
extern const UCHAR msg_C_1approve[];
extern const UCHAR msg_C_2decline[];
extern const UCHAR msg_auth_code[];
extern const UCHAR msg_C_auth_code[];
extern const UCHAR msg_online[];
extern const UCHAR msg_C_online[];
extern const UCHAR msg_reversal[];
extern const UCHAR msg_C_reversal[];
extern const UCHAR msg_1online[];
extern const UCHAR msg_2decline[];
extern const UCHAR msg_2decline_3accept[];
extern const UCHAR msg_processing[];
extern const UCHAR msg_C_processing[];
extern const UCHAR msg_trans_log_full[];
extern const UCHAR msg_total_rec[];
extern const UCHAR msg_out_of_service[];
extern const UCHAR msg_printing[];
extern const UCHAR msg_C_printing[];
extern const UCHAR msg_swipe_card[];
extern const UCHAR msg_C_swipe_card[];
extern const UCHAR msg_load_key_ok[];
extern const UCHAR msg_load_key_failed[];
extern const UCHAR msg_1clear_2details[];
extern const UCHAR msg_print_log_details[];
extern const UCHAR msg_enter_tx_sc[];
extern const UCHAR msg_record_not_found[];
extern const UCHAR msg_1term_2icc[];
extern const UCHAR msg_batchno[];
extern const UCHAR msg_goods[];
extern const UCHAR msg_services[];
extern const UCHAR msg_bdc_uploaded[];
extern const UCHAR msg_press_ok_to_finish[];
extern const UCHAR msg_rid[];

extern	const UCHAR msg_setup_printer[];
extern	const UCHAR msg_setup_odc[];
extern	const UCHAR msg_setup_autotest[];
extern	const UCHAR msg_setup_exception_file[];
extern	const UCHAR msg_on[];
extern	const UCHAR msg_off[];

extern const UCHAR msg_asi[];
extern const UCHAR msg_aid[];
extern const UCHAR msg_null_asi[];
extern const UCHAR msg_null_aid[];
extern const UCHAR msg_0delete_1update[];
extern const UCHAR msg_pls_select_aid[];
extern const UCHAR msg_okstart[];
extern const UCHAR msg_1goods_2services[];

//----------------------------------------------------------------------------
//      EMV D/C Functon List
//----------------------------------------------------------------------------
extern const UCHAR EMVDC_Func_Table[];
extern const UCHAR TAC_Func_Table[];
extern const UCHAR REF_Func_Table[];
extern const UCHAR SALE_Func_Table[];
extern const UCHAR TRANS_TYPE_Func_Table[];

extern const UCHAR msg_null_date[];
extern const UCHAR msg_confid_range[];
extern const UCHAR msg_q_update[];
extern const UCHAR msg_q_clear[];
extern const UCHAR msg_q_print[];
extern const UCHAR msg_C_q_print[];
extern const UCHAR msg_null_atc[];
extern const UCHAR msg_pls_select_item[];

//----------------------------------------------------------------------------
//      Chinese Bitmap Font for Display (16x16)
//----------------------------------------------------------------------------
extern const UCHAR LCD_ChineseFontCode[];
extern const UCHAR LCD_ChineseFontBitmap[];

//----------------------------------------------------------------------------
//#pragma memory=default
//----------------------------------------------------------------------------
#endif
