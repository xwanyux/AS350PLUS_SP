//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : i.MX6UL				                                    **
//**  PRODUCT  : AS350-X6						                            **
//**                                                                        **
//**  FILE     : OS_LIB.H                                                   **
//**  MODULE   : Declaration of OS Library Module.	                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2022/05/24                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2022 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _OS_LIB_H_
#define _OS_LIB_H_

//----------------------------------------------------------------------------
#include "POSAPI.h"
#include "LCDTFTAPI.h"


//#define MAX_DSP_WIDTH                   320	// 240
//#define MAX_DSP_CHAR                    40+1
//#define MAX_DSP_FONT0_COL               39+1
//#define MAX_DSP_FONT0_CNT               40+1
//#define MAX_DSP_FONT1_COL               25+1
//#define MAX_DSP_FONT1_CNT               26+1
//#define MAX_DSP_ROW_CNT                 13
//#define MAX_AMT_CNT                     12

#define MAX_DSP_WIDTH                   128
#define MAX_DSP_CHAR                    20+1
#define MAX_DSP_FONT0_COL               19+1
#define MAX_DSP_FONT0_CNT               20+1
#define MAX_DSP_FONT1_COL               14+1
#define MAX_DSP_FONT1_CNT               15+1
#define MAX_DSP_ROW_CNT                 4
#define MAX_AMT_CNT                     12

#define NUM_TYPE_DIGIT                  0x01    // pure digit (RFU)
#define NUM_TYPE_COMMA                  0x02    // insert thousand comma
#define NUM_TYPE_DEC                    0x04    // insert decimal point
#define NUM_TYPE_STAR                   0x08    // special prompt ('*')
#define NUM_TYPE_LEADING_ZERO           0x10    // accept leading '0'
#define NUM_TYPE_LEFT_JUSTIFY		0x20	// orientation of entry

#define	COL_LEFTMOST			0
#define	COL_MIDWAY			1
#define	COL_RIGHTMOST			2


//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
extern	void	LIB_LCD_Cls( void );
extern	void	LIB_LCD_Putc(UINT8 row, UINT8 col, UINT8 font, UINT8 data);
extern	void	LIB_LCD_Puts( UINT8 row, UINT8 col, UINT8 font, UINT8 len, UINT8 *str );
extern	void	LIB_LCD_PutMsg(UINT8 row, UINT8 pos, UINT8 font, UINT8 len, UINT8 *msg);
extern	void	LIB_BUZ_Open(void);
extern	void	LIB_BUZ_Close(void);
extern	void	LIB_BUZ_Beep1( void );
extern	void	LIB_BUZ_IO_Beep1( void );
extern	void	LIB_BUZ_Beep2( void );
extern	void	LIB_BUZ_IO_Beep2( void );
extern	void	LIB_OpenKeyAll( void );
extern	void	LIB_OpenKeyNum( void );
extern	UINT8	LIB_GetKeyStatus( void );
extern	UINT8	LIB_WaitKey( void );
extern	UINT8	LIB_WaitMuteKey( void );
extern	UINT8	LIB_WaitTimeAndKey( UINT16 tenms );
extern	void	LIB_FlushKeys( void );
extern	UINT32	LIB_GetNumKey( UINT16 tout, UINT8 type, UINT8 idle, UINT8 font, UINT8 row, UINT8 len, UINT8 *buf );
extern	UINT8	GetAlphaNumKey(UINT8 dir, UINT8 row, UINT8 col, UINT8 font, UINT8 cnt, UINT8 *data);
extern	UINT8	GetAlphaNumKey2(UINT16 tout, UINT8 dir, UINT8 row, UINT8 col, UINT8 font, UINT8 cnt, UINT8 *data);
extern	UINT32	LIB_GetAlphaNumDigits( UINT8 row, UINT8 col, UINT8 font, UINT8 cnt, UINT8 *digits );
extern	UINT32	LIB_GetAlphaNumDigits2(UINT16 tout, UINT8 row, UINT8 col, UINT8 font, UINT8 cnt, UINT8 *digits);
extern	UINT32	LIB_EnterPassWord( UINT32 maxtry, UINT8 row, UINT8 *psw );
extern	UINT32	LIB_EnterPassWordLen( UINT32 maxtry, UINT8 row, UINT8 *psw );
extern	UINT32	LIB_EnterPassWordLen2( UINT32 maxtry, UINT8 row, UINT8 *psw, UINT8 entry );
extern	UINT32	LIB_WaitKeyYesNo( UINT8 row, UINT8 col );
extern	UINT32	LIB_WaitKeyMsgYesNo( UINT8 row, UINT8 col, UINT8 len, UINT8 *msg );
extern	UINT32	LIB_WaitKeyMsgYesNoTO( UINT8 row, UINT8 col, UINT8 len, UINT8 *msg );
extern	void	LIB_DispHexByte( UINT8 row, UINT8 col, UINT8 data );
extern	void	LIB_DispHexWord( UINT8 row, UINT8 col, UINT16 data );
extern	void	LIB_DispHexLong( UINT8 row, UINT8 col, UINT32 data );
extern	void	LIB_LCD_ClearRow(UINT8 row, UINT8 cnt, UINT8 font);
extern	void	LIB_LCD_ClearRow_EX(UINT8 row, UINT8 cnt, UINT8 font);
extern	void	LIB_WaitTime( UINT16 tenms );
extern	void	LIB_DumpHexData( UINT8 mode, UINT8 row, UINT16 length, UINT8 *data );
extern	void	LIB_DumpHexData2( UINT8 mode, UINT8 row, UINT16 length, UINT8 *data );
extern	void	LIB_DumpHexDataTO( UINT8 mode, UINT8 row, UINT16 length, UINT8 *data );
extern	UINT8	LIB_ListBox( UINT8 start, UINT8 *para, UINT8 *list, UINT16 tout );
extern	UINT8	LIB_OpenAUX( UINT8 port, UINT8 mode, UINT16 baud );
extern	UINT8	LIB_CloseAUX( void );
extern  UINT8   LIB_ReceiveReadyAUX(UINT8 *data);
extern	UINT8	LIB_ReceiveAUX( UINT8 *data );
extern	UINT8	LIB_TransmitAUX( UINT8 *data );
extern	UINT8	LIB_TransmitAUX_C( UINT8 c );
extern	UINT8	LIB_itoa( UINT16 value, UINT8 *abuf );
extern	UINT8	LIB_ltoa( UINT32 value, UINT8 *abuf );
extern	void	LIB_bcd2asc( UINT8 bcdlen, UINT8 *bcd, UINT8 *str );
extern	UINT8	LIB_ascw2hexb( UINT8 buf[] );
extern	int	LIB_memcmp( UINT8 *s1, UINT8 *s2, UINT32 len );
extern	int	LIB_memcmpc( UINT8 *s1, UINT8 cc, UINT32 len );
extern	void	LIB_ResetSystem( void );
extern	void	LIB_BUZ_TamperedBeeps( UCHAR state );
extern	void    LIB_BUZ_PromptForEntering(void);
extern	void    LIB_BUZ_OkBeep(void);
extern	void    LIB_BUZ_ErrorBeep(void);
extern	void    LIB_LED_BlinkPeriodically(void);
extern	void    LIB_LED_BlinkOn(void);
extern	void    LIB_LED_BlinkOff(void);
extern	void    LIB_LED_BlinkStyle(UINT8 style);
extern	void	LIB_LCDTFT_PutStr( API_LCDTFT_PARA para, UINT8 len, UINT8 *msg );
extern	void	LIB_LCDTFT_ClearRow( API_LCDTFT_PARA para );
extern	int	LIB_memcmp( UINT8 *s1, UINT8 *s2, UINT32 len );  // modify by Wayne change form UINT16 -> UINT32
extern	int	LIB_memcmpc( UINT8 *s1, UINT8 cc, UINT32 len );  // modify by Wayne change form UINT16 -> UINT32

extern	void	BSP_Delay_n_ms( UINT32 ms );

extern	void	LIB_BUZ_MPU_MemManage( void );
extern	void	LIB_BUZ_MPU_HardFault( void );
extern	void	LIB_BUZ_StackOverflow( void );

//----------------------------------------------------------------------------
#endif
