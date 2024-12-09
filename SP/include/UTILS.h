//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  :                                                            **
//**  PRODUCT  : AS330	                                                    **
//**                                                                        **
//**  FILE     : UTILS.H						    **
//**  MODULE   : Declaration of TEST Module.		    		    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2018/05/08                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2018 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _UTILS_H_
#define _UTILS_H_

//----------------------------------------------------------------------------
#define		TRUE		1
#define		FALSE		0

#define		KEY_0                           '0'
#define		KEY_1                           '1'
#define		KEY_2                           '2'
#define		KEY_3                           '3'
#define		KEY_4                           '4'
#define		KEY_5                           '5'
#define		KEY_6                           '6'
#define		KEY_7                           '7'
#define		KEY_8                           '8'
#define		KEY_9                           '9'
#define		KEY_F1                          'a'	// NA
#define		KEY_F2                          'b'	// NA
#define		KEY_F3                          'c'	// NA
#define		KEY_F4                          'd'	// NA
#define		KEY_CANCEL                      'x'
#define		KEY_CLEAR                       'n'
#define		KEY_ALPHA                       '*'
#define		KEY_BACKSPACE                   '#'
#define		KEY_OK                          'y'

//----------------------------------------------------------------------------
extern	UCHAR		util_dhn_kbd;
extern	UCHAR		util_dhn_buz;
extern	UCHAR		util_dhn_lcd;
extern	UCHAR		util_dhn_prt;

//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
void	UTIL_OpenBuzzer_1S( void );
void	UTIL_BUZ_Beep1( void );
void	UTIL_OpenKey_ALL( void );
void	UTIL_OpenKeyAlphaNum( void );
UCHAR	UTIL_WaitKey( void );
UCHAR	UTIL_GetKeyStatus( void );
void	UTIL_SwitchAlphanumericKey( UCHAR *key );
void	UTIL_OpenDisplay( void );
void	UTIL_ClearScreen( void );
void	UTIL_BlackenScreen( void );
void	UTIL_ClearRow( UCHAR row, UCHAR cnt, UCHAR font );
void	UTIL_PutChar( UCHAR row, UCHAR col, UCHAR font, UCHAR data );
void	UTIL_PutStr( UCHAR row, UCHAR col, UCHAR font, UCHAR len, UCHAR *msg );
void	UTIL_hexb2ascw( UCHAR data, UCHAR *byte_h, UCHAR *byte_l );
void	UTIL_DispHexByte( UCHAR row, UCHAR col, UCHAR data );
void	UTIL_DispHexWord( UCHAR row, UCHAR col, UINT data );
void	UTIL_DumpHexData( UCHAR mode, UCHAR row, UINT length, UCHAR *data );
void	UTIL_DumpHexData2( UCHAR mode, UCHAR row, UINT length, UCHAR *data );
void	UTIL_WaitTime( UINT tenms );
UCHAR	UTIL_WaitTimeAndKey( UINT tenms );
void	UTIL_OpenPrinter(void);
void	UTIL_ClosePrinter(void);
UCHAR	UTIL_PrintString( UCHAR FormFeed, UCHAR FontID, UCHAR *str, UCHAR *status );
//UCHAR	UTIL_PrintGraphics( API_GRAPH dim, UCHAR *bmp,  UCHAR *status );
int	UTIL_GetKeyInNumber( UCHAR row, UCHAR font );
int	UTIL_GetKeyInString( UCHAR row, UCHAR col, UCHAR font, UCHAR *str );
UCHAR	UTIL_itoa( UINT value, UCHAR *abuf );

extern	ULONG	_DEBUGPRINTF_OPEN( ULONG UartPort );
extern	ULONG	_DEBUGPRINTF_CLOSE( void );
extern	void	_DEBUGPRINTF( char *Msg, ... );

//----------------------------------------------------------------------------
#endif



