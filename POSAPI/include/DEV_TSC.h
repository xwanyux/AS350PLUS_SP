//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L                                                       **
//**  PRODUCT  : AS350	                                                    **
//**                                                                        **
//**  FILE     : DEV_TSC.H	                                            **
//**  MODULE   : Declaration of TSC Module.				    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2013/08/06                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2013 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _DEV_TSC_H_
#define _DEV_TSC_H_

#include "bsp_types.h"
#include "POSAPI.h"
#include "API_TSC.h"

//----------------------------------------------------------------------------
#define	BYTE					UINT8
#define	WORD					UINT16
#define	DWORD					UINT32
#define	LONG					long int

//----------------------------------------------------------------------------
//	Sign Pad Status
//----------------------------------------------------------------------------
#define	SIGNPAD_STATUS_REVERSE			0x08

#define	SIGNPAD_STATUS_NOT_READY		0xA0
#define	SIGNPAD_STATUS_READY			0xA1
#define	SIGNPAD_STATUS_READY_REVERSE		0xA1 + SIGNPAD_STATUS_REVERSE
#define	SIGNPAD_STATUS_ROTATE			0xA2
#define	SIGNPAD_STATUS_CLEAR			0xA3
#define	SIGNPAD_STATUS_TIMEOUT			0xA4
#define	SIGNPAD_STATUS_EXIT			0xA5

//----------------------------------------------------------------------------
typedef	struct SPI_TSC_S			// SPI format for TSC parameters
{
	UINT8		DevID;			// device id
	UINT8		CmdID;			// command id
	
	API_TSC_PARA	TscPara;		// TSC parameters  (fixed 16 bytes)
	
} __attribute__((packed)) SPI_TSC;


typedef struct	BMPFILEHEADER_S {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} __attribute__((packed)) BMPFILEHEADER;


typedef struct	BMPINFOHEADER_S{
        DWORD   biSize;
        LONG    biWidth;
        LONG    biHeight;
        WORD    biPlanes;
        WORD    biBitCount;
        DWORD   biCompression;
        DWORD   biSizeImage;
        LONG    biXPelsPerMeter;
        LONG    biYPelsPerMeter;
        DWORD   biClrUsed;
        DWORD   biClrImportant;
} __attribute__((packed)) BMPINFOHEADER;


typedef struct	BMPRGBQUAD_S {
        BYTE    rgbBlue;
        BYTE    rgbGreen;
        BYTE    rgbRed;
        BYTE    rgbReserved;
} __attribute__((packed)) BMPRGBQUAD;




#if	0
//----------------------------------------------------------------------------
#define	TFTLCD_FONT1_W					12	//14
#define	TFTLCD_FONT1_H					24

#define MAX_LCDTFT_ROW_NO				20	// for 16x8 ASCII font
#define	MAX_LCDTFT_COL_NO				30	// for 16x8 ASCII font
#define MAX_LCDTFT_PIXROW_NO				320	// physical horizontal X-orientation
#define	MAX_LCDTFT_PIXCOL_NO				240	// physical vertical Y-orientation

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



//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
extern	UINT32	OS_LCDTFT_ClrScreen( API_LCDTFT_PARA para );
extern	UINT32	OS_LCDTFT_TransformCharBitmap( UINT8 Height, UINT8 Width, UINT8 *pImageData );
extern	UINT32	OS_LCDTFT_PutChar( API_LCDTFT_PARA para, OS_FDT *pFdt, UINT8 *pImage );


#endif
extern	UINT32	OS_TSC_Status( API_TSC_PARA para, UCHAR *status );
extern	UINT32	OS_TSC_SignPad( API_TSC_PARA para, UINT8 *status, UINT8 orient );
extern	UINT32	OS_TSC_ShowSignPad( API_TSC_PARA tscpara, UINT32 orient, UINT8 *palette );
extern	UINT32	OS_TSC_ShowRotateButton( UINT32 flag, UINT32 orient );
extern	UINT32	OS_TSC_ClearSignpad( API_TSC_PARA para,UINT32 orient );
extern	UINT32	TSC_Convert2WinBMP( ULONG rawWidth, ULONG rawHeight, ULONG rawLen, UCHAR *rawBmp, ULONG *winLen, UCHAR *winBmp );
extern	UINT32	OS_TSC_Close();
extern	UINT32	OS_TSC_Open();
extern	UINT32	OS_TSC_GetSign( API_TSC_PARA para, UINT8 *status, UINT8 *sign, ULONG *length );
//----------------------------------------------------------------------------
#endif
