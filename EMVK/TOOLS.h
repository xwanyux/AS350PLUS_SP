//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : TOOLS.h                                                    **
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
#ifndef _TOOLS_H_
#define _TOOLS_H_

//----------------------------------------------------------------------------
//extern  void  TL_Debug_PutIssuerPKM( UCHAR *buf );
//extern  void  TL_Debug_PutIssuerPKC( UCHAR *buf );
//extern  void  TL_Debug_PutIssuerPKR( UCHAR *buf );
//extern  void  TL_Debug_PutIssuerPKE( UCHAR *buf );
//extern  void  TL_Debug_PutSDTBA( UCHAR *buf );
//extern  void  TL_Debug_PutSSAD( UCHAR *buf );

extern  void  TL_hexb2ascw( UCHAR data, UCHAR *byte_h, UCHAR *byte_l );
extern  void  TL_DispHexByte( UCHAR row, UCHAR col, UCHAR data );
extern  void  TL_DispHexWord( UCHAR row, UCHAR col, UINT data );
extern  void  TL_DumpHexData( UCHAR mode, UCHAR row, UINT length, UCHAR *data );
extern  void  TL_DumpHexData2( UCHAR mode, UCHAR row, UINT length, UCHAR *data );
extern  void  TL_PrintHexData( UCHAR length, UCHAR *data, UCHAR *str );
extern  void  TL_bcd2hex( UCHAR bcdlen, UCHAR *bcd, UCHAR *hex );
extern  UCHAR TL_ascw2bcdb( UCHAR buf[] );
extern  UCHAR TL_ascw2hexb( UCHAR buf[] );
extern  void  TL_asc2bcd( UCHAR bcdlen, UCHAR *bcd, UCHAR *str );
extern  void  TL_hex2asc( UCHAR hexlen, UCHAR *hex, UCHAR *str);
extern  void  TL_bcd2asc( UCHAR bcdlen, UCHAR *bcd, UCHAR *str );
extern  UCHAR TL_itoa( UINT value, UCHAR *abuf );
extern  void  TL_SwapData( UCHAR length, UCHAR *data );
extern  UCHAR TL_bcd_add_bcd( UCHAR bcdlen, UCHAR *bcd1, UCHAR *bcd2 );
extern  void  TL_trim_decimal( UCHAR exp, UCHAR dec, UCHAR *buf );
extern  void  TL_trim_asc( UCHAR method, UCHAR *old, UCHAR *new );
extern  void  TL_insert_thousand_comma( UCHAR *text, UCHAR exp );
extern  UCHAR TL_CNcmp( UCHAR *s1, UCHAR *s2, UCHAR len );
extern  UCHAR TL_CNcmp2( UCHAR *s1, UCHAR *s2, UCHAR len );
extern  void  TL_memmove( UCHAR *des, UCHAR *src, UCHAR len );
extern  int   TL_memcmpc( UCHAR *s1, UCHAR cc, UCHAR len );
extern  int   TL_memcmp( UCHAR *s1, UCHAR *s2, UCHAR len );
extern  UCHAR TL_PushData( UCHAR *stk, UCHAR *top, UCHAR maxstk, UCHAR len, UCHAR *item );
extern  UCHAR TL_PopData( UCHAR *stk, UCHAR *top, UCHAR len, UCHAR *item );
extern  UCHAR TL_isort( UCHAR *a, UCHAR na, UCHAR mask );
extern  UCHAR TL_ListBox( UCHAR start, UCHAR *para, UCHAR *list, UINT tout );
extern  void  TL_WaitTime( UINT tenms );
extern  UCHAR TL_WaitTimeAndKey( UINT tenms );
extern  UCHAR TL_GetNumKey( UINT tout, UCHAR type, UCHAR idle, UCHAR font, UCHAR row, UCHAR len, UCHAR *buf );
extern  UCHAR TL_GetAlphaNumKey( UCHAR dir, UCHAR row, UCHAR col, UCHAR font, UCHAR cnt, UCHAR *data );
extern  UCHAR TL_GetAlphaNumDigits( UCHAR row, UCHAR col, UCHAR font, UCHAR cnt, UCHAR *digits );
extern  void  TL_TDE_GetData( UCHAR index, UCHAR *tde );
extern  UINT  TL_TDE_GetAddr( UCHAR index );
extern  void  TL_IDE_GetData( UCHAR index, UCHAR *tde );
extern  UINT  TL_IDE_GetAddr( UCHAR index );
extern  void  TL_GetDateTime( UCHAR *rtc );
extern  void  TL_SetDateTime( UCHAR *rtc );
extern  UCHAR TL_SetCentury( signed char year );
extern  int   TL_CompareDate( UCHAR *date1, UCHAR *date2 );
extern  UCHAR TL_CheckDate( UCHAR *bdate );
extern  UCHAR TL_CheckDateTime( UCHAR rtc[] );
extern  void  TL_FormatDate( UCHAR o_date[], UCHAR n_date[] );
extern  void  TL_FormatTime( UCHAR o_time[], UCHAR n_time[] );
extern  void  TL_ShowDateTime( UCHAR row, UCHAR col, UCHAR font, UCHAR rtc[] );
extern  UCHAR TL_UpdateReq( UCHAR msgid, UCHAR row, UCHAR pos, UCHAR font );
extern  void  TL_ShowBCD( UCHAR row, UCHAR col, UCHAR font, UCHAR len, UCHAR *bcd );

//extern  UCHAR TL_GetTransLog( UINT recno, UCHAR *log );
//extern  UCHAR TL_PutTransLog( UINT recno, UCHAR *log );
//extern  void  TL_ClearTransLog( void );
extern  void  TL_GetRandomNumber( UCHAR *data );
extern  ULONG TL_ldiv( ULONG numer, ULONG denom );
extern  void  TL_LoadFuncList( UCHAR id, UCHAR list[] );
extern  UCHAR TL_CheckDataType( UCHAR type, UCHAR *data, UCHAR len );
extern  UCHAR TL_VerifyCertificateExpDate( UCHAR *cdate );
extern  UCHAR TL_VerifyTransAmount( UCHAR *amt );

extern  UCHAR TL_OpenAUX( void );
extern  UCHAR TL_CloseAUX( void );
extern  UCHAR TL_ReceiveAUX( UCHAR *data );
extern  UCHAR TL_TransmitAUX( UCHAR *data );


// ---------------------------------------------------------------------------
//extern  void  UI_PrintClear( void );
//extern  UCHAR UI_PrintPutStr( UCHAR length, UCHAR *str );
//extern  UCHAR UI_PrintStatus( UCHAR dhn );
//extern  UCHAR UI_PrintOut( void );
//extern  void  UI_InitLcdChineseFont( void );

//----------------------------------------------------------------------------
#endif
