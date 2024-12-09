//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  :                                                            **
//**  PRODUCT  : AS350	                                                    **
//**                                                                        **
//**  FILE     : GUI.H						    	    **
//**  MODULE   : Declaration of GUI.			    		    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2013/07/31                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2013 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _GUI_H_
#define _GUI_H_
#include "POSAPI.h"
//----------------------------------------------------------------------------
#define	GUI_DRAW_BMP555				0
#define	GUI_DRAW_BMP24				1

//----------------------------------------------------------------------------
typedef	struct	GUI_BITMAP_S			// GUI Bitmap
{
	UINT		XSize;			// width  size in dots
	UINT		YSize;			// height size in dots
	UINT		BytesPerLine;		// bytes per line
	UINT		BitsPerPixel;		// bits  per pixel
	UCHAR		*pData;			// pointer to picture data
	UCHAR		*pPal;			// pointer to palette
	UCHAR		*pMethods;		// pointer to methods
	
} __attribute__((packed)) GUI_BITMAP;

//----------------------------------------------------------------------------
#endif
