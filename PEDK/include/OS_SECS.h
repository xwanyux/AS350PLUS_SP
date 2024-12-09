//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  :							    **
//**  PRODUCT  : AS330 		                                            **
//**                                                                        **
//**  FILE     : OS_SECS.H						    **
//**  MODULE   : Declaration of the Secure Sub-system.			    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2018/07/24                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2018 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _DEV_SECS_H_
#define _DEV_SECS_H_

//----------------------------------------------------------------------------
#include <mml.h>

#include "POSAPI.h"


//----------------------------------------------------------------------------
//#define	_DEBUG_SECS_		// uncomment if UART output is needed for debug purpose

extern	ULONG	_DEBUGPRINTF_OPEN( ULONG UartPort );
extern	ULONG	_DEBUGPRINTF_CLOSE( void );
extern	void	_DEBUGPRINTF( char *Msg, ... );



//----------------------------------------------------------------------------
/* Define register address */
/* Bits Fields */
#define ENABLE_ALL_EXTS	 	0x1F
#define LOCK_REG	 	0x80000000
#define ENABLE_ALL_INTS		0x00000007
#define MINUS_50		0x10000
#define PLUS_110		0x20000


/* others IP registers */

/*AES*/
#define AES_BASE 		0x40005000
#define AES_AESC_C		AES_BASE + 0x0
#define AESUKEYR0 		AES_BASE + 0x24
#define AESUKEYR1 		AES_BASE + 0x28
#define AESUKEYR2 		AES_BASE + 0x2C
#define AESUKEYR3 		AES_BASE + 0x30
#define AESDIN0 		AES_BASE + 0x04
#define AESDIN1 		AES_BASE + 0x08
#define AESDIN2 		AES_BASE + 0x0C
#define AESDIN3 		AES_BASE + 0x10
#define AESDOUT0 		AES_BASE + 0x14
#define AESDOUT1 		AES_BASE + 0x18
#define AESDOUT2 		AES_BASE + 0x1C
#define AESDOUT3 		AES_BASE + 0x20

#define START_UAES_ENC		0x101  // Start AES encryption by using 128-bit user key
#define START_SAES_ENC		0x01  // Start AES encryption by using 128-bit secure key

//----------------------------------------------------------------------------
#define IDLE			0
#define ARMED			1
#define TRIGGERED		2


//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
extern	void	OS_SEC_Setup( void );
extern	UINT32	OS_SEC_Status( UINT8 *status );
extern	void	OS_SEC_CheckTamperStatus( void );
extern	void	OS_SEC_ReadAlarmReg(UINT32 *value);

//----------------------------------------------------------------------------
#endif
