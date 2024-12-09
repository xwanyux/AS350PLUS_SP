//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS330_mPOS						    **
//**                                                                        **
//**  FILE     : PINPAD.H                                                   **
//**  MODULE   : Declaration of related PINPAD functions.                   **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2018/08/07                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2018 SymLink Corporation. All rights reserved.	    **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _PINPAD_H_
#define _PINPAD_H_

//----------------------------------------------------------------------------
extern  void	PP_show_incorrect_pin( void );
extern  void	PP_show_last_pin_try( void );
extern  void	PP_show_pin_ok( void );
extern  void	PP_show_please_wait( void );
extern  UCHAR	PP_GetPIN( UINT tout, UCHAR *amt );
extern  void	PP_GenPinBlock( UCHAR *pinblock );
extern	UCHAR	PP_GenEncrypedPinBlock( UCHAR *pan, UCHAR *epb, UCHAR *ksn, UCHAR mod, UCHAR idx );

//----------------------------------------------------------------------------
#endif
