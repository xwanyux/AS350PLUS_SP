//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L                                                       **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : DEV_PRN.H                                                  **
//**  MODULE   : Declaration of PRINTER Module.		                    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2008/04/22                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2008 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _DEV_PRN_H_
#define _DEV_PRN_H_

//----------------------------------------------------------------------------
#include "bsp_types.h"


//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
extern	UINT32	OS_PRN_Open( UINT8 type, UINT8 mode );
extern	UINT32	OS_PRN_Close( void );
extern	void	OS_PRN_Status( UINT8 *dbuf );
extern	UINT32	OS_PRN_PutString( UINT8 fontid, UINT8 *dbuf );
extern	UINT32	OS_PRN_InitFont( API_PRT_FONT ft );
extern	UINT32	OS_PRN_PutGraphics( API_GRAPH dim, UCHAR *image );
extern	UINT32	OS_PRN_SetLineSpace( UCHAR space );
extern	UINT32	OS_PRN_GetLineSpace( UCHAR *space );

// ---------------------------------------------------------------------------
#endif
