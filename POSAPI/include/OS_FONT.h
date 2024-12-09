//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L                                                       **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : OS_FONT.H	                                            **
//**  MODULE   : Declaration of system FONT module. 		            **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2007/07/17                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2007 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _OS_FONT_H_
#define _OS_FONT_H_
#include "POSAPI.h"
//----------------------------------------------------------------------------

extern	UINT8		ASCII_FONT_6X8[2048];
extern	UINT8		ASCII_FONT_8X16[4096];
extern	UINT8		ASCII_FONT_12X24[];

//----------------------------------------------------------------------------

#define	FDT_IMAGE_LEN					16
#define MAX_FDT_NO					20			// 10
#define	MAX_FDT_ID					MAX_FDT_NO - 1
#define	MIN_FDT_ID					2			//
#define	FONT6X8_BMP_LEN					8			// 6x8
#define	FONT8X16_BMP_LEN				16			// 8x16
#define	FONT16X16_BMP_LEN				32			// 16x16
#define	FONT12X24_BMP_LEN				48			// 12x24
#define	FONT24X24_BMP_LEN				72			// 24x24

#define	FONT_0						0			// font ID=0, system default 6x8 ascii
#define	FONT_1						1			// font ID=1, system default 8x16 ascii
#define	FONT_2						2			// font ID=2, AP 24x24 Chinese font
#define	FONT_3						3			//
#define	FONT_4						4			//
#define	FONT_5						5			//
#define	FONT_6						6			//
#define	FONT_7						7			//
#define	FONT_8						8			//
#define	FONT_9						9			//
#define	FONT_10						10			// font ID=10, printer default 24x24 ascii
#define	FONT_11						11			//
#define	FONT_12						12			//
#define	FONT_13						13			//
#define	FONT_14						14			//
#define	FONT_15						15			//

#define	BMP_FONT_ST_CHAR				0xA0			// min start hi byte code for 2-byte char
#define	ASCII_ST_CHAR					0x20			// beginning printable ascii char
#define	DOLLAR_SIGN_ST_CHAR				0x90			// special dollar sign

#define	LCD_ATTR_XYDOT_MASK				0x40			// to identify attribute of XYDOT

typedef struct OS_FDT_S					// Font Descriptor Table, size=16 bytes
{
	UINT8			FontID;			// font id
	UINT8			ByteNo;			// no. of bytes of one char image
	UINT8			Width;			//
	UINT8			Height;			//
	UINT8			*CodStAddr;		// char code table
	UINT8			*CodEndAddr;		//
	UINT8			*BmpStAddr;		// char image table
	UINT8			*BmpEndAddr;		//
	UINT8			Cross;			//
	UINT8			RFU;			//
} OS_FDT;


//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
extern	void	OS_FONT_Init( UINT8 devid );
extern	OS_FDT	*OS_FONT_GetFdtAddr( UINT8 fid );
extern	UINT8	*OS_FONT_LoadFont01Bmp( OS_FDT *pFdt, UINT8 code, UINT8 *pBuf );
extern	UINT8	*OS_FONT_LoadFont2Bmp( OS_FDT *pFdt, UINT8 HiByte, UINT8 LoByte, UINT8 *pBuf );
extern	UINT8	*OS_FONT_LoadFontXBmp( OS_FDT *pFdt, UINT8 code, UINT8 *pBuf );

//----------------------------------------------------------------------------
#endif
