//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2 KERNEL CallBack Functions.                          **
//**  PRODUCT  : AS320	                                                    **
//**                                                                        **
//**  FILE     : EMVCB.H	                                            **
//**  MODULE   :                                                            **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2009/06/10                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2009-2011 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
#ifndef _EMVCB_H_
#define _EMVCB_H_

//----------------------------------------------------------------------------
extern	UCHAR	EMV_CB_OnlineProcessing( UCHAR *arc, UCHAR *air, UCHAR *rmsg );
extern	UCHAR	EMV_CB_FindMostRecentPAN( UCHAR *pan, UCHAR *amt );
extern	UCHAR	EMV_CB_ReferralProcessing( void );
extern	void	EMV_CB_ShowMsg_ENTER_PIN( void );
extern	void	EMV_CB_ShowMsg_ENTER_PIN_BY_EXT_PINPAD( void );
extern	void	EMV_CB_ShowMsg_PIN_OK( void );
extern	void	EMV_CB_ShowMsg_INCORRECT_PIN( void );
extern	void	EMV_CB_ShowMsg_LAST_PIN_TRY( void );
extern	void	EMV_CB_ShowMsg_PLEASE_WAIT( void );

//----------------------------------------------------------------------------
#endif
