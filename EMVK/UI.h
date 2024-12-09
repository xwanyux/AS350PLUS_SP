
//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : UI.H                                                       **
//**  MODULE   : Declaration of related UTILITY functions. (in page1)       **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2002/12/05                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2002-2007 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _UI_H_
#define _UI_H_

//----------------------------------------------------------------------------
#define TRUE                    	1
#define FALSE                   	0

#define MAX_DSP_WIDTH                   128
#define MAX_DSP_CHAR                    20+1
#define MAX_DSP_FONT0_COL               19+1
#define MAX_DSP_FONT0_CNT               20+1
#define MAX_DSP_FONT1_COL               14+1
#define MAX_DSP_FONT1_CNT               15+1
#define MAX_DSP_ROW_CNT                 4
#define MAX_AMT_CNT                     12

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

//#define AMT_INT_SIZE                    5       // amount, integer part (n10)
//#define AMT_DEC_SIZE                    1       // amount, decimal part (n2)

#define NUM_TYPE_DIGIT                  0x01    // pure digit (RFU)
#define NUM_TYPE_COMMA                  0x02    // insert thousand comma
#define NUM_TYPE_DEC                    0x04    // insert decimal point
#define NUM_TYPE_STAR                   0x08    // special prompt ('*')
#define NUM_TYPE_LEADING_ZERO           0x10    // accept leading '0'

//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
extern  void  UI_ClearScreen(void);
extern  void  UI_ClearRow(UCHAR row, UCHAR cnt, UCHAR font);
extern  void  UI_PutChar(BYTE row, BYTE col, BYTE font, BYTE data);
extern  void  UI_PutStr(BYTE row, BYTE col, BYTE font, BYTE len, BYTE *msg);
extern  void  UI_PutMsg(BYTE row, BYTE pos, BYTE font, BYTE len, BYTE *msg);
extern  UCHAR UI_OpenBuzzer1S(void);
extern  UCHAR UI_OpenBuzzer1L(void);
extern  void  UI_Beep(UCHAR dhn);
extern  void  UI_OpenKeyAll(void);
extern  void  UI_CloseKeyAll(void);
extern  void  UI_OpenKey_OK_CANCEL( void );
extern  UCHAR UI_WaitKey(void);

extern  UCHAR UI_GetKeyStatus(void);
extern  void  UI_PrintClear( void );
extern  UCHAR UI_PrintPutStr( UCHAR length, UCHAR *str );
extern  UCHAR UI_PrintStatus( UCHAR dhn );
extern  UCHAR UI_PrintOut( void );
extern  void  UI_InitLcdChineseFont( UINT num, UCHAR *code, UCHAR *bmp );
extern  void  UI_InitLcdCodeTableFont( UCHAR fontid );

//----------------------------------------------------------------------------
#endif
