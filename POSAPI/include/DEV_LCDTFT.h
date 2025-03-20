//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L                                                       **
//**  PRODUCT  : AS350	                                                    **
//**                                                                        **
//**  FILE     : DEV_LCDTFT.H                                               **
//**  MODULE   : Declaration of TFT LCD Module. (GF320240)		    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2013/04/18                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2013 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _DEV_LCDTFT_H_
#define _DEV_LCDTFT_H_

#include "bsp_types.h"
#include "POSAPI.h"
#include "LCDTFTAPI.h"

//----------------------------------------------------------------------------
//#define	LCDTFT_FONT_0					LCDTFT_FONT0	// 16x8  (20 row x 30 col)
//#define	LCDTFT_FONT_1					LCDTFT_FONT1	// 24x12 (13 row x 20 col)

#define	TFTLCD_FONT1_W					12	//14
#define	TFTLCD_FONT1_H					24

#define MAX_LCDTFT_ROW_NO				20	// for 16x8 ASCII font
#define	MAX_LCDTFT_COL_NO				30	// for 16x8 ASCII font
#define MAX_LCDTFT_PIXROW_NO				320	// physical horizontal X-orientation
#define	MAX_LCDTFT_PIXCOL_NO				240	// physical vertical Y-orientation

/*
#define	MAX_STR_IMAGE_BUF_SIZE				1350	// 1024

typedef	struct SPI_LCDTFT_S			// SPI format for TFT LCD parameters
{
	UINT8		DevID;			// device id
	UINT8		CmdID;			// command id
	
	API_LCDTFT_PARA	LcdPara;		// LCD parameters  (fixed 16 bytes)
	
} __attribute__((packed)) SPI_LCDTFT;


typedef	struct SPI_PCDICON_S			// SPI format for PCD icon parameters
{
	UINT8		DevID;			// device id
	UINT8		CmdID;			// command id
	
	API_PCD_ICON	IconPara;		// icon parameters  (fixed 16 bytes)
	
} __attribute__((packed)) SPI_PCDICON;


typedef	struct	LCDTFT_GRAPH_S			// TFT LCD graphics
{
	UINT		ID;			// id number of the built-in icon (0..n)
	UINT		Width;			// image width in dots in horizontal direction
	UINT		Height;			// image height in dots in vertical direction
	ULONG		Size;			// size of the image in bytes
	UCHAR		RGB;			// RGB mode
	
	UCHAR		RFU[5];
} __attribute__((packed)) LCDTFT_GRAPH;


typedef	struct SPI_LCDTFT_GRAPH_S		// SPI format for TFT LCD parameters
{
	UINT8			DevID;		// device id
	UINT8			CmdID;		// command id
	
	LCDTFT_GRAPH		LcdPara;	// LCD parameters  (fixed 16 bytes)
	
} __attribute__((packed)) SPI_LCDTFT_GRAPH;


typedef	struct	LCDTFT_ICON_S			// TFT LCD icon
{
	UINT		ID;			// id number of the built-in icon (0..n)
	UINT		Xleft;			// x positon
	UINT		Ytop;			// y position
	UINT		Method;			// dispaly method
	UINT		Width;
	UINT		Height;
	
	UCHAR		RFU[4];
} __attribute__((packed)) LCDTFT_ICON;


typedef	struct SPI_LCDICON_S			// SPI format for TFT LCD parameters
{
	UINT8			DevID;		// device id
	UINT8			CmdID;		// command id
	
	LCDTFT_ICON		IconPara;	// LCD parameters  (fixed 16 bytes)
	
} __attribute__((packed)) SPI_LCDICON;


typedef	struct SPI_LCDRECT_S			// SPI format for TFT LCD rectangle
{
	UINT8			DevID;		// device id
	UINT8			CmdID;		// command id
	
	API_LCDTFT_RECT		RectPara;	// LCD parameters  (fixed 16 bytes)
	
} __attribute__((packed)) SPI_LCDRECT;


typedef	struct SPI_LCDBL_S			// SPI format for TFT LCD backlight
{
	UINT8			DevID;		// device id
	UINT8			CmdID;		// command id
	
	API_LCDTFT_BACKLIT	RectPara;	// LCD parameters  (fixed 16 bytes)
	
} __attribute__((packed)) SPI_LCDBL;
*/

//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
//extern	UINT32	OS_LCDTFT_ClrScreen( API_LCDTFT_PARA para );
//extern	UINT32	OS_LCDTFT_TransformCharBitmap( UINT32 Height, UINT32 Width, UINT8 *pImageData );
//extern	UINT32	OS_LCDTFT_PutChar( API_LCDTFT_PARA para, OS_FDT *pFdt, UINT8 *pImage );
//extern	UINT32	OS_LCDTFT_ConvertWIN2GUI( UINT16 id, UINT16 CCWdegrees, UCHAR *fg_palette, UCHAR *bg_palette, UINT16 *width, UINT16 *height, UINT8 *winbmp );

extern	UINT32	OS_LCDTFT_SetCharCursor( UINT8 *first, UINT8 font, UINT16 row, UINT16 col, UINT8 height, UINT8 width, UINT8 fwidth, UINT16 *pixrow, UINT16 *pixcol );

//----------------------------------------------------------------------------
#endif
