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
//**  Copyright(C) 2009 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
// NOTE: The MANDATORY functions must be implemented for EMV transaction.
//
//----------------------------------------------------------------------------
#ifndef _EMVCB_H_
#define _EMVCB_H_

//----------------------------------------------------------------------------
extern	UCHAR	EMV_CB_OnlineProcessing( UCHAR *arc, UCHAR *air, UCHAR *rmsg );
extern	UCHAR	EMV_CB_ReferralProcessing( void );
extern	UCHAR	EMV_CB_FindMostRecentPAN( UCHAR *pan, UCHAR *amt );
extern	UCHAR	EMV_CB_GetAmount( UCHAR *amt, UCHAR *sc );
extern	UCHAR	EMV_CB_SelectApplication( UCHAR list_cnt, UCHAR *list );
extern	void	EMV_CB_ShowMsg_TRY_AGAIN( void );
extern	void	EMV_CB_ShowMsg_NOT_ACCEPTED( void );
extern	void	EMV_CB_ShowMsg_PLEASE_WAIT( void );
extern	void	EMV_CB_ShowMsg_SELECT( void );
extern	void	EMV_CB_ShowMsg_INCORRECT_PIN( void );
extern	void	EMV_CB_ShowMsg_LAST_PIN_TRY( void );

//----------------------------------------------------------------------------
#endif
