//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : LPC1788                                                    **
//**  PRODUCT  : AS350	                                                    **
//**                                                                        **
//**  FILE     : DEV_PCD.H                                                  **
//**  MODULE   : Declaration of LCD Module. (GF320240)	                    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2013/04/26                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2013 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _DEV_PCD_H_
#define _DEV_PCD_H_

#include "POSAPI.h"

//----------------------------------------------------------------------------
//	Icon ID
//----------------------------------------------------------------------------
// #define	IID_PCD_EXIT			0	// exit PCD functions
// #define	IID_TAP_LOGO			1
// #define	IID_LED_BLUE			2
// #define	IID_LED_YELLOW			3
// #define	IID_LED_GREEN			4
// #define	IID_LED_RED			5

// #define	IID_LED_ONESHOT			0x80	// for one-shot LED signal control

//----------------------------------------------------------------------------
//	LED Color and Allocation
//----------------------------------------------------------------------------
#define	OFS_PCD_ID_L			0	
#define	OFS_PCD_ID_H			1	
#define	OFS_PCD_BLINK_ON_L		2	
#define	OFS_PCD_BLINK_ON_H		3	
#define	OFS_PCD_BLINK_OFF_L		4	
#define	OFS_PCD_BLINK_OFF_H		5	
// #define	LED_COLOR_OFF			0x6318	// GRAY
#define	LED_COLOR_OFF			0xC0C0C0	// GRAY

// #define	LED_COLOR_BLUE			0x7C00
#define	LED_COLOR_BLUE			0x0000FF
#define	LED_COLOR_YELLOW		0xFFFF00
#define	LED_COLOR_GREEN			0x00FF00
#define	LED_COLOR_RED			0xFF0000

#define	LED_H				16	// height
#define	LED_W				56	// 16	// width
#define	LED_SP				2	// 24	// space btw two LEDs
#define	LED_START_POS			5	// 52

#define	LED_X_START			0
#define	LED_X_END			LED_X_START + LED_H

#define	LED_Y_START_BLUE			LED_START_POS
#define	LED_Y_END_BLUE			 LED_Y_START_BLUE+ LED_W

#define	LED_Y_START_YELLOW		LED_Y_END_BLUE + LED_SP
#define	LED_Y_END_YELLOW			LED_Y_START_YELLOW + LED_W

#define	LED_Y_START_GREEN		LED_Y_END_YELLOW + LED_SP
#define	LED_Y_END_GREEN		LED_Y_START_GREEN + LED_W

#define	LED_Y_START_RED		LED_Y_END_GREEN + LED_SP		
#define	LED_Y_END_RED			LED_Y_START_RED + LED_W

//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
extern	UINT32  PCD_ShowPCD( UINT16 ParaLen, UINT8 *Para );


//----------------------------------------------------------------------------
#endif
