//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : Utilities.                                                 **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : UTILS.H                                                    **
//**  MODULE   : Declaration of related UTILITY functions.		    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2002/12/05                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2002-2011 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _UTILS_H_
#define _UTILS_H_

//----------------------------------------------------------------------------
/*
#define TRUE                    	1
#define FALSE                   	0
*/

#define MAX_DSP_WIDTH                   128
#define MAX_DSP_CHAR                    20+1
#define MAX_DSP_FONT0_COL               19+1
#define MAX_DSP_FONT0_CNT               20+1
#define MAX_DSP_FONT1_COL               14+1
#define MAX_DSP_FONT1_CNT               15+1
#define MAX_DSP_ROW_CNT                 4
#define MAX_AMT_CNT                     12

//#define MAX_DSP_WIDTH                   240
//#define MAX_DSP_CHAR                    20
//#define MAX_DSP_FONT0_COL               29
//#define MAX_DSP_FONT0_CNT               30
//#define MAX_DSP_FONT1_COL               29
//#define MAX_DSP_FONT1_CNT               30
//#define MAX_DSP_ROW_CNT                 4
//#define MAX_AMT_CNT                     12

#define COL_LEFTMOST                    0       // display column position
#define COL_MIDWAY                      1
#define COL_RIGHTMOST                   2

#define KEY_0                           '0'
#define KEY_1                           '1'
#define KEY_2                           '2'
#define KEY_3                           '3'
#define KEY_4                           '4'
#define KEY_5                           '5'
#define KEY_6                           '6'
#define KEY_7                           '7'
#define KEY_8                           '8'
#define KEY_9                           '9'
#define KEY_F1                          'a'     // VOID
#define KEY_F2                          'b'     // ADJUST
#define KEY_F3                          'c'     // SETTLE
#define KEY_F4                          'd'     // FUNC
#define KEY_CANCEL                      'x'
#define KEY_CLEAR                       'n'
#define KEY_ALPHA                       'z'
#define KEY_BACKSPACE                   '#'
#define KEY_OK                          'y'

#define NUM_TYPE_DIGIT                  0x01    // pure digit (RFU)
#define NUM_TYPE_COMMA                  0x02    // insert thousand comma
#define NUM_TYPE_DEC                    0x04    // insert decimal point
#define NUM_TYPE_STAR                   0x08    // special prompt ('*')
#define NUM_TYPE_LEADING_ZERO           0x10    // accept leading '0'

//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
extern	UCHAR	UT_ListBox( UCHAR start, UCHAR *para, UCHAR *list, UINT tout );
extern	void	UT_OpenKeyAll( void );
extern	void	UT_BUZ_Beep1S( void );
extern	void	UT_BUZ_Beep1L( void );
extern	void	UT_ClearScreen(void);
extern	UCHAR	UT_WaitKey(void);
extern	UCHAR	UT_WaitTimeAndKey( UINT tenms );
extern	UCHAR	UT_GetNumKey( UINT tout, UCHAR type, UCHAR idle, UCHAR font, UCHAR row, UCHAR len, UCHAR *buf );
extern	UCHAR	UT_insert_decimal_point( UCHAR type, UCHAR *buf );
extern	UCHAR	UT_bcd_add_bcd( UCHAR bcdlen, UCHAR *bcd1, UCHAR *bcd2 );

//----------------------------------------------------------------------------
#endif
