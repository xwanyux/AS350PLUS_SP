//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L (32-bit Platform)                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : DEV_PRN.C                                                  **
//**  MODULE   : 							    **
//**                                                                        **
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
//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
#include <stdarg.h>

#include "bsp_types.h"
//#include "bsp_prn.h"
#include "POSAPI.h"
extern UINT32	apk_prn_Init( void );
extern UINT32	apk_prn_Open( UINT8 type, UINT8 mode );
extern UINT32	apk_prn_Close( void );
extern void		apk_prn_Status( UINT8 *dbuf );
extern UINT32	apk_prn_PutString( UINT8 fontid, UINT8 *dbuf );
extern UINT32	apk_prn_PutGraphics( API_GRAPH dim, UINT8 *image );
extern UINT32	apk_prn_SetLineSpace( UINT8 space );
extern UINT32	apk_prn_GetLineSpace( UINT8 *space );
extern UINT32	apk_prn_InitFont( API_PRT_FONT ft );
// ---------------------------------------------------------------------------
// FUNCTION: Initialize printer module, must be called at POST.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_PRN_Init( void )
{
	apk_prn_Init();
}

// ---------------------------------------------------------------------------
// FUNCTION: To gain access of printer interface driver.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
UINT32	OS_PRN_Open( UCHAR type, UCHAR mode )
{
	return( apk_prn_Open( type, mode ) );
}

// ---------------------------------------------------------------------------
// FUNCTION: To deactivate the specified printer interface driver's state machine.
// INPUT   : pPrn -- pointer to the BSP_PRN structure.
// OUTPUT  : none.
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
UINT32	OS_PRN_Close( void )
{
	return( apk_prn_Close() );
}

// ---------------------------------------------------------------------------
// FUNCTION: To get the current status of printer.
// INPUT   : none.
// OUTPUT  : dbuf
//	     Printer status byte.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_PRN_Status( UINT8 *dbuf )
{
	apk_prn_Status( dbuf );
}

// ---------------------------------------------------------------------------
// FUNCTION: To write the given character string to printing device 
//	     by the specified font.
// INPUT   : pPrn -- pointer to the BSP_PRN structure. (RFU)
//
//	     fontid - printer font id.
//	     	      APS_FONT_8x8
//		      APS_FONT_12x10
//                    APS_FONT_7x8
//		      APS_FONT_8x14
//		      SII_FONT_8x16
//		      SII_FONT_12x24
//
//	     dbuf
//	     UCHAR  length;        // length of the given string.
//	     UCHAR  data[length];  // character string to be printed.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
UINT32	OS_PRN_PutString( UINT8 fontid, UINT8 *dbuf )
{
	return( apk_prn_PutString( fontid, dbuf ) );
}

// ---------------------------------------------------------------------------
// FUNCTION: To write the given image data to printing device
//	     by the specified displacement.
// INPUT   : dhn
//	     The specified device handle number.
//
//	     dim
//	     The dimension of the graphics to be printed.
//	     ULONG   xmove;         // horizontal displacement in dots. (x8)
//	     ULONG   ymove ;        // vertical displacement in dots. (RFU)
//	     ULONG   width ;        // image width in dots in the horizontal direction. (x8)
//	     ULONG   height ;       // image height in dots  in the vertical direction. (x8)
//	     ULONG   mode ;         // image data mode.
//
//	     dbuf
//	     image data.
//
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
UINT32	OS_PRN_PutGraphics( API_GRAPH dim, UCHAR *image )
{
	return( apk_prn_PutGraphics( dim, image ) );
}

// ---------------------------------------------------------------------------
// FUNCTION: To tune the space between two string lines.
// INPUT   : space
//           The unit is in dots.
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
UINT32	OS_PRN_SetLineSpace( UCHAR space )
{
	return( apk_prn_SetLineSpace( space ) );
}

// ---------------------------------------------------------------------------
// FUNCTION: To get the space setting between two string lines.
// INPUT   : none.
// OUTPUT  : space
//           The unit is in dots.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
UINT32	OS_PRN_GetLineSpace( UCHAR *space )
{
	return( apk_prn_GetLineSpace( space ) );
}

// ---------------------------------------------------------------------------
// FUNCTION: To initialize the external bitmap fonts for printer.
//	     The specified font should be downloaded to system memory prior to 
//	     initialization.
// INPUT   : ft
// 	     The "API_PRN_FONT" struture.
//	     Normal Fond ID: FONT10, FONT11, FONT12 ...
// OUTPUT  : none.
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
UINT32	OS_PRN_InitFont( API_PRT_FONT ft )
{
	return( apk_prn_InitFont( ft ) );
}
