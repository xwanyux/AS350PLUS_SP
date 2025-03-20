//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  :                                                            **
//**  PRODUCT  : AS350	                                                    **
//**                                                                        **
//**  FILE     : LCDTFTAPI.h						    **
//**  MODULE   : Declaration of TFT LCD Module.		    		    **
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



#include "POSAPI.h"


#ifndef _LCDTFT_API_H_
#define _LCDTFT_API_H_

#include "GUI.h"

//----------------------------------------------------------------------------
//		Default Display Font ID
//----------------------------------------------------------------------------
#if	1	// JAMES

#define		LCDTFT_FONT0			FONT0		//
#define		LCDTFT_FONT1			FONT1		//
#define		LCDTFT_FONT2			FONT2		//
#define		LCDTFT_FONT3			FONT13		// printer font   (32x32, 10 row x 20 col, Chinese)
#define		LCDTFT_FONT4			FONT14		// lcdtft  font   (16x16, 20 row x 15 col, Chinese)

#else

#define		LCDTFT_FONT0			FONT10		// printer font 0 (8x16 , 20 row x 30 col, ASCII)
#define		LCDTFT_FONT1			FONT11		// printer font 1 (12x24, 13 row x 20 col, ASCII)
#define		LCDTFT_FONT2			FONT12		// printer font 2 (24x24, 13 row x 20 col, Chinese)
#define		LCDTFT_FONT3			FONT13		// printer font   (32x32, 10 row x 20 col, Chinese)
#define		LCDTFT_FONT4			FONT14		// lcdtft  font   (16x16, 20 row x 15 col, Chinese)

#endif

//----------------------------------------------------------------------------
//		Default Display Font Size
//----------------------------------------------------------------------------
#if	0	// JAMES

#define		TFTLCD_FONT0_W			6
#define		TFTLCD_FONT0_H			8

#define		TFTLCD_FONT1_W			8
#define		TFTLCD_FONT1_H			16

#define		TFTLCD_FONT2_W			24
#define		TFTLCD_FONT2_H			24

#define		TFTLCD_FONT3_W			32
#define		TFTLCD_FONT3_H			32

#define		TFTLCD_FONT4_W			16
#define		TFTLCD_FONT4_H			16

#define		FONT0_W			6
#define		FONT0_H			8

#define		FONT1_W			8
#define		FONT1_H			16

#define		FONT2_W			24
#define		FONT2_H			24

#define		FONT3_W			32
#define		FONT3_H			32

#define		FONT4_W			16
#define		FONT4_H			16

#define		FONT12_W		12
#define		FONT12_H		24

#else

#define		TFTLCD_FONT0_W			8
#define		TFTLCD_FONT0_H			16

#define		TFTLCD_FONT1_W			12
#define		TFTLCD_FONT1_H			24

#define		TFTLCD_FONT2_W			24
#define		TFTLCD_FONT2_H			24

#define		TFTLCD_FONT3_W			32
#define		TFTLCD_FONT3_H			32

#define		TFTLCD_FONT4_W			16
#define		TFTLCD_FONT4_H			16

#define		FONT0_W			8
#define		FONT0_H			16

#define		FONT1_W			12
#define		FONT1_H			24

#define		FONT2_W			24
#define		FONT2_H			24

#define		FONT3_W			32
#define		FONT3_H			32

#define		FONT4_W			16
#define		FONT4_H			16

#define		FONT12_W		12
#define		FONT12_H		24

#endif

//----------------------------------------------------------------------------
//		RGB Mode
//----------------------------------------------------------------------------
#define		RGB_BPP16			0		// 16 bits per pixel (default)
#define		RGB_BPP24			1		// 24 bits per pixel
#define		RGB_BPP8			2		// 8  bits per pixel

//----------------------------------------------------------------------------
//		Built-in Icon ID
//----------------------------------------------------------------------------
#define		IID_PCD_EXIT			0		// exit PCD functions
#define		IID_TAP_LOGO			1
#define		IID_LED_BLUE			2
#define		IID_LED_YELLOW			3
#define		IID_LED_GREEN			4
#define		IID_LED_RED				5

#define		IID_LED_ONESHOT			0x80		// for one-shot LED signal control
//----------------------------------------------------------------------------
//		CCW Degrees
//----------------------------------------------------------------------------
#define		CCW_0				0
#define		CCW_90				90
#define		CCW_180				180
#define		CCW_270				270
//----------------------------------------------------------------------------
//		WIN BMP Method
//----------------------------------------------------------------------------
#define		WINBMP_OW			0x0000		// no method, overwrite
#define		WINBMP_SIGN_FRAME		0x8000		// add 320x120 frame to signature bmp

//----------------------------------------------------------------------------
//		FRAMEBUFFER FLAG
//----------------------------------------------------------------------------
extern UCHAR		IS_FRAMEBUFFER_OPENED;				//checked framebuffer is opened

//----------------------------------------------------------------------------
typedef	struct	API_LCDTFT_PARA_S		// TFT LCD parameters
{
	UINT		Row;			// row or pixel row
	UINT		Col;			// column or pixel column
	UCHAR		Font;			// font id & attribute
	UCHAR		RGB;			// RGB mode
	UCHAR		FG_Palette[3];		// foreground color palette
	UCHAR		FontHeight;		// bitmap font height
	UCHAR		FontWidth;		// bitmap font width
	UCHAR		BG_Palette[3];		// background color palette
	UINT		CCWdegrees;		// degrees of counter clockwise
} __attribute__((packed)) API_LCDTFT_PARA;


typedef	struct	API_LCDTFT_GRAPH_S		// TFT LCD graphics
{
	UINT		ID;			// id number of the built-in icon (0..n)
	UCHAR		RGB;			// RGB mode
	
	GUI_BITMAP	Bitmap;			// GUI bitmap
	
} API_LCDTFT_GRAPH;


typedef	struct	API_LCDTFT_ICON_S		// TFT LCD built-in icons for display
{
	UINT		ID;			// id number of the built-in icon (0..n)
	UINT		Xleft;			// start pixel column coordinate at the upper left
	UINT		Ytop;			// start pixel row coordinate at the upper left
	UINT		Method;			// method of writing graphics
	UINT		Width;			// image width in dots in horizontal direction
	UINT		Height;			// image height in dots in vertical direction
	
	UCHAR		RFU[4];
} __attribute__((packed)) API_LCDTFT_ICON;


typedef	struct	API_LCDTFT_WIMBMP_S		// TFT LCD Windows monochrome BMP
{
	UINT		ID;			// id number of the built-in icon (0..n)
	UINT		CCWdegrees;		// degrees of counter clockwise
	UINT		Width[1];		// image width in dots in horizontal direction
	UINT		Height[1];		// image height in dots in vertical direction
	UINT		Method;			// method of writing graphics
	UCHAR		FG_Palette[3];		// foreground color palette
	UCHAR		BG_Palette[3];		// background color palette
} __attribute__((packed)) API_LCDTFT_WINBMP;


typedef	struct	API_LCDTFT_RECT_S		// TFT LCD rectangle for display
{
	UINT		Xstart;			// start pixel X coordinate
	UINT		Xend;			// end   pixel X coordinate
	UINT		Ystart;			// start pixel Y coordinate
	UINT		Yend;			// end   pixel Y coordinate
	UCHAR		RGB;			// RGB mode
	UCHAR		Palette[3];		// color palette to fill
	
	UCHAR		RFU[4];
} __attribute__((packed)) API_LCDTFT_RECT;


typedef	struct	API_PCD_ICON_S			// TFT LCD built-in icons for PCD
{
	UINT		ID;			// id number of the built-in icon (0..n)
	UINT		BlinkOn;		// time for icon ON period in 10ms units  (0xFFFF=always ON, 0x0000=always OFF)
	UINT		BlinkOff;		// time for icon OFF period in 10ms units
	
	UCHAR		RFU[10];
} __attribute__((packed)) API_PCD_ICON;


typedef	struct	API_LCDTFT_BACKLIT_S		// TFT LCD backlight for display
{
	ULONG		Level;			// backlight driver level
	
	UCHAR		RFU[12];
} __attribute__((packed)) API_LCDTFT_BACKLIT;


typedef	struct	API_LCDTFT_GRAPH_DIM_S		// TFT LCD graphics dimension
{
	UINT		Xleft;			// start pixel column coordinate at the upper left
	UINT		Ytop;			// start pixel row coordinate at the upper left
	UINT		Width;			// image width in dots in horizontal direction
	UINT		Height;			// image height in dots in vertical direction
	UCHAR		RGB;			// RGB mode
	UCHAR		*pBuf;			// storage for bmp
	
} API_LCDTFT_GRAPH_DIM;


//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
// extern	UCHAR	api_lcdtft_open( UCHAR deviceid, UCHAR rgbmode );
// extern	UCHAR	api_lcdtft_close( UCHAR dhn );
// extern	UCHAR	api_lcdtft_clear( UCHAR dhn, UCHAR *sbuf );
// extern	UCHAR	api_lcdtft_putstring( UCHAR dhn, UCHAR *sbuf, UCHAR *dbuf );
// extern	UCHAR	api_lcdtft_showPCD( UCHAR dhn, UCHAR *sbuf );
// extern	UCHAR	api_lcdtft_putgraphics( UCHAR dhn, API_LCDTFT_GRAPH *sbuf );
// extern	UCHAR	api_lcdtft_showICON( UCHAR dhn, API_LCDTFT_ICON *sbuf );
// extern	UCHAR	api_lcdtft_putwinbmp( UCHAR dhn, UCHAR *sbuf, UCHAR *winbmp );
// extern	UCHAR	api_lcdtft_fillRECT( UCHAR dhn, UCHAR *sbuf );
// extern	UCHAR	SIGNPAD_lcdtft_putstring( UCHAR dhn, UCHAR *sbuf, UCHAR *dbuf, UINT CCWdegrees );

extern	UCHAR	api_lcdtft_open( UCHAR deviceid );
extern	UCHAR	api_lcdtft_close( UCHAR dhn );
extern	UCHAR	api_lcdtft_clear( UCHAR dhn, API_LCDTFT_PARA para );
extern  UCHAR   api_lcdtft_putstring( UCHAR dhn, API_LCDTFT_PARA para, UCHAR *dbuf );
extern 	UCHAR   api_lcdtft_showPCD(UCHAR dhn, API_PCD_ICON icon);
extern	UCHAR	api_lcdtft_putgraphics( UCHAR dhn, API_LCDTFT_GRAPH graph);
extern	UCHAR	api_lcdtft_showICON( UCHAR dhn, API_LCDTFT_ICON icon );
extern	UCHAR	api_lcdtft_putwinbmp( UCHAR dhn, API_LCDTFT_WINBMP *bmppara, UCHAR *winbmp );
extern	UCHAR	api_lcdtft_fillRECT( UCHAR dhn, API_LCDTFT_RECT rect);
extern	UCHAR	api_lcdtft_initfont( API_LCD_FONT ft  );
extern   UCHAR  SIGNPAD_lcdtft_putstring( UCHAR dhn, API_LCDTFT_PARA para, UCHAR *dbuf, UINT CCWdegrees );

//----------------------------------------------------------------------------
#endif


