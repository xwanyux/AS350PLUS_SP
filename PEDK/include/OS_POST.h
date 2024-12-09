//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : MAX32550 (32-bit Platform)				    **
//**  PRODUCT  : AS330_mPOS						    **
//**                                                                        **
//**  FILE     : OS_POST.H                                                  **
//**  MODULE   : Declaration of POST Module.	                    	    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2018/05/15                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2018 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _OS_POST_H_
#define _OS_POST_H_

//----------------------------------------------------------------------------
#include "POSAPI.h"

//----------------------------------------------------------------------------
#define	OP_MODE_POST				0		// power on self test
#define	OP_MODE_DIAG				1		// diagnostic test
#define	OP_MODE_MFG				2		// manufacturing test
#define	OP_MODE_BIN				3		// burn-in test
#define	OP_MODE_DRET				4		// data retention
#define	OP_MODE_LOCK				5		// undefined
#define	OP_MODE_QUICK_MFG			0x82		// press "CLEAR" key at power-on

#define	KEY_FUNC				'd'
#define	KEY_CANCEL				'x'
#define	KEY_CLEAR				'n'
#define	KEY_ALPHA				'*'		// 'z'
#define	KEY_ENTER				'y'
#define	KEY_BACKSPACE				'#'

//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
extern	void	POST_TestDIAG( void );
extern	void	POST_TestMFG( void );
extern	void	POST_TestBIN( void );
extern	void	POST_TestDRET( void );

//----------------------------------------------------------------------------
#endif
