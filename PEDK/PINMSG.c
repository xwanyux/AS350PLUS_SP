//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : 	                                                    **
//**  PRODUCT  : AS330							    **
//**                                                                        **
//**  FILE     : PINMSG.C                                                   **
//**  MODULE   : 							    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2014/04/23                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2003-2018 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include <string.h>

#include "POSAPI.h"
#include "OS_LIB.h"
#include "LCDTFTAPI.h"


// ---------------------------------------------------------------------------
// FUNCTION: display 
//			0123456789012345
//		0	TOTAL	    NNNN
//		1
//		2	ENTER PIN:
//
// INPUT   : amt   - total amount to be confirmed on display. (external)
//                   format: LEN(1) + ASCII(n)
// ---------------------------------------------------------------------------
void	PP_show_enter_pin( UCHAR *amt )
{
const	UCHAR msg_TOTAL[] =	{"TOTAL"};
const	UCHAR msg_ENTER_PIN[] =	{"ENTER PIN:"};


	LIB_LCD_Cls();
	// LIB_LCD_ClearRow_EX( 1, 3, FONT1 );
	
	if( amt[0] != 0 )
	  {
	  LIB_LCD_PutMsg( 0, COL_LEFTMOST, FONT1, sizeof(msg_TOTAL)-1, (UCHAR *)msg_TOTAL );
	  LIB_LCD_PutMsg( 0, COL_RIGHTMOST, FONT1, amt[0], &amt[1] );
	  }
	
	LIB_LCD_PutMsg( 2, COL_LEFTMOST, FONT1, sizeof(msg_ENTER_PIN)-1, (UCHAR *)msg_ENTER_PIN );
}

// -------------------------------------------------------------------------------------------------
// FUNC  : Setup parameters for AS350 internal PIN entry function.
// INPUT : row	   - row number of display for PIN entry.
//	   col	   - beginning column number of display for PIN entry.
//	   palette - fixed 3 bytes palette values of RGB.
// OUTPUT: none.
// RETURN: none.
// NOTE  : This function is to be implemented from AP level.
// -------------------------------------------------------------------------------------------------
void	PP_SetupPinPad( API_LCDTFT_PARA *para )
{
	para->RGB = 0;	// 2013-08-13
	
	para->Row	= 6;
	para->Col	= 2;
	para->Font	= LCDTFT_FONT1+attrCLEARWRITE;
	para->RGB	= RGB_BPP16;
	para->FG_Palette[0]	= 0x00;
	para->FG_Palette[1]	= 0x00;
	para->FG_Palette[2]	= 0x00;
	para->FontHeight	= TFTLCD_FONT1_H;
	para->FontWidth		= TFTLCD_FONT1_W;
	para->BG_Palette[0]	= 0xFF;
	para->BG_Palette[1]	= 0xFF;
	para->BG_Palette[2]	= 0xFF;
	para->CCWdegrees	= 0;
}
