//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : MAX32550 (32-bit Platform)				    **
//**  PRODUCT  : AS330_mPOS						    **
//**                                                                        **
//**  FILE     : DEV_KBD.H                                                  **
//**  MODULE   : Declaration of KEYBOARD Module.	                    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2018/04/20                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2018 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _DEV_KBD_H_
#define _DEV_KBD_H_

//----------------------------------------------------------------------------

#define	KBD_KAT_SIZE			5		// max keyboard allocation table

//----------------------------------------------------------------------------
//	Contactless Reader LEDs
//	BLUE	: P1.14
//	YELLOW	: P1.15
//	GREEN	: P1.16
//	RED	: P1.17
//----------------------------------------------------------------------------
#define	MML_GPIO1_KBD_BL_PIN		18		// EMV_CLK_CUT (P1.18)


#if	0
#define	KBD_KAT_SIZE			5		// max keyboard allocation table
#define	KBD_BUF_SIZE			3		// max key buffer size

#define KBD_SCAN_LINE_MASK		0x000003E0	// xxxx xxxx xxxx xxxx xxxx xx11 111x xxxx (GPIO0 pin5..9)
#define	KBD_SCAN_PATTERN		0xFFFFFFDF	// 1111 1111 1111 1111 1111 1111 1101 1111 (start from pin5)
#define KBD_RETN_LINE_MASK		0x0000001E	// xxxx xxxx xxxx xxxx xxxx xxxx xxx1 111x (GPIO0 pin1..4)
#define KBD_RETN_LINE_MASK2		0x000003E0	// xxxx xxxx xxxx xxxx xxxx xx11 111x xxxx (GPIO0 pin5..9)
#define MAX_KBD_SCAN_LINE		5
#define MAX_KBD_RETN_LINE		4
#define MAX_KBD_SCAN_LINE2		4
#define MAX_KBD_RETN_LINE2		5
#endif


//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
extern	void	OS_KBD_Reset( UINT8 KatValue );
extern	void	OS_KBD_GetKAT( UINT8 *kat );
extern	void	OS_KBD_SetKAT( UINT8 *kat );
extern	UINT32	OS_KBD_Status( UINT32 *ScanCode );
extern	UINT32 	OS_KBD_Init( void );
extern	UINT32	OS_KBD_ScanCodeToAP( UINT32 sc, UINT32 *Table );
extern	UINT32	OS_KBD_GetChar( void );

//----------------------------------------------------------------------------
#endif
