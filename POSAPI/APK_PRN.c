//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L                                                       **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : APK_PRN.C 	                                            **
//**  MODULE   : apk_prn_initfont()			                    **
//**									    **
//**  FUNCTION : APK::PRN (Application Kernel of Printer Module)	    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2008/10/31                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2008 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
//#include <stdio.h>
#include <string.h>
#include "POSAPI.h"
#include "massage.h"
#include "option.h"
#include "charles_base.h"
#include "font_type.h"
#include "printer.h"

extern	struct FONT_PAIR FONT_TABLE[MAX_FONT];
extern	UCHAR	InitDoneFlag[MAX_FONT];

// extern	BSP_LCD	BspLcd;

// ---------------------------------------------------------------------------
// FUNCTION: To initialize the printer driver.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
UINT32	apk_prn_Init( void )
{
UINT32	i;
UINT32	status;


	status = FALSE;

	if( api_prnt_init() == apiOK )	// reset driver
	  {
		memset(FONT_TABLE,0,sizeof(struct FONT)*MAX_FONT);
		for( i=0; i<MAX_FONT; i++ )	// reset font tables
	     {
		    //  FONT_TABLE[i].idel = SEM_B_SIGNAL;
//		     FONT_TABLE[i].ascii.type = USELESS_FONT;
//		     FONT_TABLE[i].other.type = USELESS_FONT;
//		     FONT_TABLE[i].height = 0;
	     }

	  status = TRUE;
	  }

	return( status );
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
UINT32	apk_prn_InitFont( API_PRT_FONT ft )
{
UINT32	status;

	
	status = FALSE;

	if( ft.FontID < MAX_FONT )
	  {
		  InitDoneFlag[ft.FontID]=1;
	  // semAcquire( &FONT_TABLE[ft.FontID].idel );
	  FONT_TABLE[ft.FontID].height			= ft.Height;

	  FONT_TABLE[ft.FontID].SBC.type		= ft.AN_Type;
	  FONT_TABLE[ft.FontID].SBC.width		= ft.AN_Width;
	  FONT_TABLE[ft.FontID].SBC.member_size	= ft.AN_ByteNo;
	  FONT_TABLE[ft.FontID].SBC.bitmap		= ft.AN_BitMap;
	  FONT_TABLE[ft.FontID].SBC.code_size		= ft.AN_CodeNo;
	  FONT_TABLE[ft.FontID].SBC.code_list		= ft.AN_CodeMap;
	  FONT_TABLE[ft.FontID].SBC.member		= ft.AN_CharNo;

	  FONT_TABLE[ft.FontID].MBC.type		= ft.GC_Type;
	  FONT_TABLE[ft.FontID].MBC.width		= ft.GC_Width;
	  FONT_TABLE[ft.FontID].MBC.member_size	= ft.GC_ByteNo;
	  FONT_TABLE[ft.FontID].MBC.bitmap		= ft.GC_BitMap;
	  FONT_TABLE[ft.FontID].MBC.code_size		= ft.GC_CodeNo;
	  FONT_TABLE[ft.FontID].MBC.code_list		= ft.GC_CodeMap;
	  FONT_TABLE[ft.FontID].MBC.member		= ft.GC_CharNo;

	  status = TRUE;
	  }

	return( status );
}

// ---------------------------------------------------------------------------
// FUNCTION: To enable the printer device and reset the interface controller.
// INPUT   : config (RFU) - e.g., settings for darkness, printer type, etc.
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
UINT32	apk_prn_Open( UINT8 type, UINT8 mode )
{
	if( api_prnt_open( type, mode ) == apiOK )
	  {
	  // BSP_WR32( (BspLcd.pLcdCtrl->Base + GPIO_OUT_SET), 0x80000000 );	// LCD_RnW pin (GPIO1[31]) = HI
	  return( TRUE );
	  }
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: To disable the printer device.
// INPUT   : dhn
//	     The specified device handle number.
//	     0x00 = to close all opened tasks.
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
UINT32	apk_prn_Close( void )
{
	if( api_prnt_close( 0 ) == apiOK )
	  {
	  // BSP_WR32( (BspLcd.pLcdCtrl->Base + GPIO_OUT_CLR), 0x80000000 );	// LCD_RnW pin (GPIO1[31]) = LO
	  return( TRUE );
	  }
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: To get the current status of printer.
// INPUT   : none.
// OUTPUT  : dbuf
//	     Printer status byte.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	apk_prn_Status( UINT8 *dbuf )
{
	api_prnt_status( 0, dbuf );
}

// ---------------------------------------------------------------------------
// FUNCTION: To write the given character string to printing device
//	     by the specified font.
// INPUT   : fontid - printer font id.
//	     dbuf
//	     UCHAR  length;        // length of the given string.
//	     UCHAR  data[length];  // character string to be printed.
// RETURN  : TRUE
//	     FALSE (out of system memory allocation, to be re-tried)
// ---------------------------------------------------------------------------
UINT32	apk_prn_PutString( UINT8 fontid, UINT8 *dbuf )
{
	if( api_prnt_putstring( 0, fontid, dbuf ) == apiOK )
	  return( TRUE );
	else
	  return( FALSE );
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
UINT32	apk_prn_PutGraphics( API_GRAPH dim, UINT8 *image )
{
UINT32	width;
UINT32	height;
UINT32	xmove;


	width  = dim.Width / 8;
	height = dim.Height;
	xmove  = dim.Xleft / 8;

	return( API_PRNpicture( image, width, height, xmove ) );
}

// ---------------------------------------------------------------------------
// FUNCTION: To tune the space between two string lines.
// INPUT   : space
//           The unit is in dots.
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
UINT32	apk_prn_SetLineSpace( UINT8 space )
{
	API_prnt_setLineGap( space );
	return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: To get the space setting between two string lines.
// INPUT   : none.
// OUTPUT  : space
//           The unit is in dots.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
UINT32	apk_prn_GetLineSpace( UINT8 *space )
{
	*space = API_prnt_getLineGap();
	return( TRUE );
}
