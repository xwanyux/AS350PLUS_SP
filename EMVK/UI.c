
//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : UI.C                                                       **
//**  MODULE   : User Interface Functions.                                  **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2002/12/04                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2002-2007 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include <string.h>

#include "POSAPI.h"
#include "EMVAPI.h"
#include "GDATAEX.h"
#include "ISO88591.H"
#include "UI.h"

#include "LCDTFTAPI.h"

//#define	TFTLCD_FONT0_W			9
//#define	TFTLCD_FONT0_H			16
//#define	TFTLCD_FONT1_W			14
//#define	TFTLCD_FONT1_H			24

// ---------------------------------------------------------------------------
// FUNCTION: Clear whole display screen.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void UI_ClearScreen(void)
{
UCHAR buf[3];

      buf[0]=-1;
      buf[1]=0;
      buf[2]=FONT1;
      api_lcd_clear(0, buf);
}

// ---------------------------------------------------------------------------
// FUNCTION: Clear display row.
// INPUT   : row  - row number, 0..N.
//           cnt  - number of rows to be cleared.
//           font - font id & attribute.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void UI_ClearRow(UCHAR row, UCHAR cnt, UCHAR font)
{
UCHAR buf[3];

      buf[0]=row;
      buf[1]=cnt;
      buf[2]=font;
      api_lcd_clear(0, buf);
}

// ---------------------------------------------------------------------------
// FUNCTION: Clear display row. (used only when PIN ENTRY)
// INPUT   : row  - row number, 0..N.
//           cnt  - number of rows to be cleared.
//           font - font id & attribute.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UI_ClearRow_EX(UCHAR row, UCHAR cnt, UCHAR font)
{
API_LCDTFT_PARA para;


	memset( (UCHAR *)&para, 0x00, sizeof(API_LCDTFT_PARA) );
#ifndef PCI_AP
	EMV_CB_SetupPinPad( &para );	// get PIN pad settings for PIN entry function from APP layer
#endif
//	if( os_LCD_DID )
//	  {
//	  memset( (UCHAR *)&para, 0x00, sizeof(API_LCDTFT_PARA) );
	  para.Row = row;
	  para.Col = cnt;
	  
          if( (font & 0x0F) == FONT0 )
            {
            para.Font = LCDTFT_FONT0;
            para.FontHeight = TFTLCD_FONT0_H;
            para.FontWidth  = TFTLCD_FONT0_W;
            }
          else
            {
            para.Font = LCDTFT_FONT1;
            para.FontHeight = TFTLCD_FONT1_H;
            para.FontWidth  = TFTLCD_FONT1_W;
            }
            
//	  para.BG_Palette[0] = 0xF0;	// RGB
//	  para.BG_Palette[1] = 0xF0;
//	  para.BG_Palette[2] = 0xFF;
	  
	  api_lcdtft_clear( 0, para );
//	  }
}

// ---------------------------------------------------------------------------
// FUNCTION: Display a character.
// INPUT   : row  - row number, 0..N.
//           col  - column number, 0..N.
//           font - font id & attribute.
//           data - the character to be displayed.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void UI_PutChar(UCHAR row, UCHAR col, UCHAR font, UCHAR data)
{
UCHAR sbuf[3];
UCHAR dbuf[2];

      sbuf[0]=row;
      sbuf[1]=col;
      sbuf[2]=font;
      dbuf[0]=1;
      dbuf[1]=data;
      api_lcd_putstring(0, sbuf, dbuf);
}

// ---------------------------------------------------------------------------
// FUNCTION: Display a string.
// INPUT   : row  - row number, 0..N.
//           col  - column number, 0..N.
//           font - font id & attribute.
//           len  - length of string.
//           msg  - the string to be displayed.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void UI_PutStr(UCHAR row, UCHAR col, UCHAR font, UCHAR len, UCHAR *msg)
{
UCHAR sbuf[3];
UCHAR dbuf[MAX_DSP_CHAR+1];

      sbuf[0]=row;
      sbuf[1]=col;
      sbuf[2]=font;
      dbuf[0]=len;
      memmove(&dbuf[1], msg, len);
      api_lcd_putstring(0, sbuf, dbuf);
}

// ---------------------------------------------------------------------------
// FUNCTION: Display a string with position justified.
// INPUT   : pos  - 0=leftmost.  (LEFTMOST)
//                  1=midway.    (MIDWAY)
//                  2=rightmost. (RIGHTMOST)
//           row  - row number, 0..N.
//           font - font id & attribute.
//           len  - length of string.
//           msg  - the string to be displayed.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void UI_PutMsg(UCHAR row, UCHAR pos, UCHAR font, UCHAR len, UCHAR *msg)
{
UCHAR sbuf[3];
UCHAR dbuf[MAX_DSP_CHAR+1];
UCHAR fontid;
UCHAR fontno;
UCHAR col = 0;
//API_LCDTFT_PARA para;


      fontid = font & 0x0f;
      
//      if( fontid == FONT0 )
//        fontno = LCDTFT_FONT0;
//      else
//        fontno = LCDTFT_FONT1;
      
      dbuf[0]=len;
      memmove(&dbuf[1], msg, len);

//#ifdef	_BOTH_LCD_ENABLED_

      switch( fontid )
            {
            case FONT0:
//               col = MAX_DSP_WIDTH - len*6;
                 col = 240 - len*TFTLCD_FONT0_W;
                 break;

            case FONT1:
            case FONT2:
            case FONT3:
//               col = MAX_DSP_WIDTH - len*8;
		 col = 240 - len*TFTLCD_FONT1_W;
                 break;

//          case FONT2:
//               col = 122 - len*16;
//               break;
            }

//    if( fontid == FONT0 )
//      col = 122 - len*6;
//    else
//      col = 122 - len*16;

      switch( pos )
            {
            case COL_LEFTMOST:
                 col = 0;

                 break;

            case COL_MIDWAY:

                 col /= 2;

		if( fontid == FONT0 )
		  col /= TFTLCD_FONT0_W;
		else
		  col /= TFTLCD_FONT1_W;

//               if( col != 0 )
//                 col--;
//               font += attrPIXCOLUMN;

                 break;

            case COL_RIGHTMOST:
            
		if( fontid == FONT0 )
		  col /= TFTLCD_FONT0_W;
		else
		  col /= TFTLCD_FONT1_W;

                 if( col != 0 )
                   col--;
		
//               font += attrPIXCOLUMN;

                 break;
            }

      sbuf[0]=row;
      sbuf[1]=col;
      sbuf[2]=font;
      dbuf[0]=len;
      memmove(&dbuf[1], msg, len);
      api_lcd_putstring(0, sbuf, dbuf);
}

// ---------------------------------------------------------------------------
// FUNCTION: Open buzzer for 1 short beep.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : device handle number.
// ---------------------------------------------------------------------------
UCHAR UI_OpenBuzzer1S(void)
{
UCHAR buf[6];
      memset(buf,0,sizeof(buf));
      buf[0]=1;
      buf[1]=5;
      buf[2]=1;
      return(api_buz_open(buf));
}

// ---------------------------------------------------------------------------
// FUNCTION: Open buzzer for 1 long beep. (for error)
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : device handle number.
// ---------------------------------------------------------------------------
UCHAR UI_OpenBuzzer1L(void)
{
UCHAR buf[6];
      memset(buf,0,sizeof(buf));
      buf[0]=1;
      buf[1]=10;
      buf[2]=1;
      return(api_buz_open(buf));
}

// ---------------------------------------------------------------------------
// FUNCTION: beep.
// INPUT   :
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void UI_Beep(UCHAR dhn)
{
      if( g_beep_on == 1 )
        api_buz_sound(dhn);
}

// ---------------------------------------------------------------------------
// FUNCTION: beep.
// INPUT   :
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void UI_BeepLong( void )
{
      if( g_beep_on == 1 )
        api_buz_sound( g_dhn_buz_1l );
}

// ---------------------------------------------------------------------------
// FUNCTION: Open keyboard - all keys.
// INPUT   : none.
// OUTPUT  : g_dhn_kbd
// RETURN  : none.
// ---------------------------------------------------------------------------
void UI_OpenKeyAll()
{
UCHAR buf[5];

      buf[0]=0x0ff;
      buf[1]=0x0ff;
      buf[2]=0x0ff;
      buf[3]=0x0ff;
      buf[4]=0x0ff;
      g_dhn_kbd = api_kbd_open(0, buf);
}

// ---------------------------------------------------------------------------
// FUNCTION: close keyboard - all keys.
// INPUT   : none.
// OUTPUT  : g_dhn_kbd
// RETURN  : none.
// ---------------------------------------------------------------------------
void UI_CloseKeyAll()
{
UCHAR buf[5];

      buf[0]=0x000;
      buf[1]=0x000;
      buf[2]=0x000;
      buf[3]=0x000;
      buf[4]=0x000;
      g_dhn_kbd = api_kbd_open(0, buf);
}

// ---------------------------------------------------------------------------
// FUNCTION: Open keyboard - numeric keys, cancel, clear, backspace, OK.
// INPUT   : none.
// OUTPUT  : g_dhn_kbd
// RETURN  : none.
// ---------------------------------------------------------------------------
void UI_OpenKeyNum()
{
UCHAR buf[5];

      buf[0]=0x01c;
      buf[1]=0x03c;
      buf[2]=0x03c;
      buf[3]=0x02c;
      buf[4]=0x000;
      g_dhn_kbd = api_kbd_open(0, buf);
}

// ---------------------------------------------------------------------------
// FUNCTION: Open keyboard - alpha, numeric keys, cancel, clear, backspace, OK.
// INPUT   : none.
// OUTPUT  : g_dhn_kbd
// RETURN  : none.
// ---------------------------------------------------------------------------
void UI_OpenKeyAlphaNum()
{
UCHAR buf[5];

//    buf[0]=0x01c;
      buf[0]=0x03c;	// replace ALPHA with '*' for AS350
      buf[1]=0x03c;
      buf[2]=0x03c;
      buf[3]=0x03c;
      buf[4]=0x000;
      g_dhn_kbd = api_kbd_open(0, buf);
}

// ---------------------------------------------------------------------------
// FUNCTION: Open keyboard - CANCEL, OK.
// INPUT   : none.
// OUTPUT  : g_dhn_kbd
// RETURN  : none.
// ---------------------------------------------------------------------------
void UI_OpenKey_OK_CANCEL()
{
UCHAR buf[5];

      buf[0]=0x000;
      buf[1]=0x000;
      buf[2]=0x000;
      buf[3]=0x024;
      buf[4]=0x000;
      g_dhn_kbd = api_kbd_open(0, buf);
}

// ---------------------------------------------------------------------------
// FUNCTION: Open keyboard - UP, DOWN, ENTER, CANCEL.
// INPUT   : none.
// OUTPUT  : g_dhn_kbd
// RETURN  : none.
// ---------------------------------------------------------------------------
void UI_OpenKeySelect()
{
UCHAR buf[5];

      buf[0]=0x020;
      buf[1]=0x000;
      buf[2]=0x020;
      buf[3]=0x024;
      buf[4]=0x000;
      g_dhn_kbd = api_kbd_open(0, buf);
}

// ---------------------------------------------------------------------------
// FUNCTION: Wait key-stroke.
// INPUT   : g_dhn_kbd
// OUTPUT  : none.
// RETURN  : the key code data.
// ---------------------------------------------------------------------------
UCHAR UI_WaitKey(void)
{
UCHAR buf[1];

      api_kbd_getchar(g_dhn_kbd, buf);
      if( g_beep_on == 1 )
        api_buz_sound(g_dhn_buz_1s); // 1 short beep
      return(buf[0]);
}

// ---------------------------------------------------------------------------
// FUNCTION: Get key status.
// INPUT   : g_dhn_kbd
// OUTPUT  : none.
// RETURN  : apiReady
//           apiNotReady
// ---------------------------------------------------------------------------
UCHAR UI_GetKeyStatus(void)
{
UCHAR buf[1];

      return( api_kbd_status( g_dhn_kbd, buf) );
}

// ---------------------------------------------------------------------------
// FUNCTION: clear all data in the printer queue.
// INPUT   : none.
// OUTPUT  : none.
// REF     : g_prt_que_head
//           g_prt_que_tail
// RETURN  : none.
// ---------------------------------------------------------------------------
//void UI_PrintClear( void )
//{
//      g_prt_que_head = ADDR_PRINTER_QUE;
//      g_prt_que_tail = ADDR_PRINTER_QUE;
//}
//
//// ---------------------------------------------------------------------------
//// FUNCTION: put string to printer queue.
//// INPUT   : length - size of string.
////           str    - the string data.
//// OUTPUT  : none.
//// REF     : g_prt_que_head
////           g_prt_que_tail
//// RETURN  : TRUE  - OK.
////           FALSE - buffer full.
//// ---------------------------------------------------------------------------
//UCHAR UI_PrintPutStr( UCHAR length, UCHAR *str )
//{
//#ifdef	PLATFORM_16BIT
//
//UCHAR sbuf[7];
//
//      if( (g_prt_que_tail + length) > ADDR_PRINTER_QUE_END )
//        return( FALSE );
//
//      sbuf[0] = SRAM_PAGE_WORK;
//      sbuf[1] = g_prt_que_tail & 0x00ff;
//      sbuf[2] = (g_prt_que_tail & 0xff00) >> 8;
//      sbuf[3] = 0x00;
//      sbuf[4] = 0x00;
//      sbuf[5] = length & 0x00ff;
//      sbuf[6] = (length & 0xff00) >> 8;
//      api_ram_write( sbuf, str );
//
//      g_prt_que_tail += length;
//
//      return( TRUE );
//
//#endif
//
//#ifndef	PLATFORM_16BIT
//
//API_SRAM pSram;
//
//	if( (g_prt_que_tail + length) > ADDR_PRINTER_QUE_END )
//        return( FALSE );
//
//	pSram.StPage = SRAM_PAGE_WORK;
//	pSram.StAddr = g_prt_que_tail;
//	pSram.Len = length;
//	api_sram_PageWrite( pSram, str );
//
//	g_prt_que_tail += length;
//
//	return( TRUE );
//
//#endif
//}
//
//// ---------------------------------------------------------------------------
//// FUNCTION: check printer status.
//// INPUT   : dhn - device handle number.
//// OUTPUT  : none.
//// RETURN  : TRUE  - ready.
////           FALSE - device error.
//// ---------------------------------------------------------------------------
//UCHAR UI_PrintStatus( UCHAR dhn )
//{
////#ifdef	PRT_ENABLED
//
//UCHAR result;
//UCHAR status[1];
//UCHAR retry;
//
//      retry = 6;
//      while(1)
//           {
//           result = api_prt_status( dhn, status );
//
////         TL_DispHexByte(1,0,result);
////         TL_DispHexByte(2,0,status[0]);
////         UI_WaitKey();
//
//           switch( result )
//                 {
//                 case apiOK:
//
//                      if( (status[0] == prtPaperEmpty)|| (status[0] == prtComplete) )
//                        return( TRUE );
//
//                      break;
//
//                 case apiFailed:
//
//                      if( retry == 0 )
//                        return( FALSE );
//
//                      retry--;
//                      break;
//
//                 case apiNotReady:
//
//                      break;
//
//                 case apiOutOfLink:
//
//                      return( FALSE );
//                 }
//           }
////#endif
//
////#ifndef	PRT_ENABLED
////	return( TRUE );
////#endif
//}
//
//// ---------------------------------------------------------------------------
//// FUNCTION: print out the printer queue data.
//// INPUT   : none.
//// OUTPUT  : none.
//// REF     : g_prt_que_head
////           g_prt_que_tail
//// RETURN  : TRUE  - OK.
////           FALSE - device error.
//// ---------------------------------------------------------------------------
//UCHAR UI_PrintOut( void )
//{
////#ifdef	PRT_ENABLED
//
//UCHAR dhn_prt;
//UCHAR sbuf[7];
//UCHAR buf[201];
//UINT  iLen;
//UCHAR result = TRUE;
//
//      dhn_prt = api_prt_open( prtThermal, 0 ); // open printer device
//      if( (dhn_prt == apiOutOfLink) || (dhn_prt == apiOutOfService ) )
//        return( FALSE );
//
//      while( (g_prt_que_tail - g_prt_que_head) > 200 )
//      	   {
//#ifdef	PLATFORM_16BIT
//    	     sbuf[0] = SRAM_PAGE_WORK;
//    	      sbuf[1] = g_prt_que_head & 0x00ff;
//    	      sbuf[2] = (g_prt_que_head & 0xff00) >> 8;
//    	      sbuf[3] = 0x00;
//    	      sbuf[4] = 0x00;
//    	      sbuf[5] = 200;
//    	      sbuf[6] = 0;
//    	      api_ram_read( sbuf, &buf[1] );
//#endif
//
//#ifndef	PLATFORM_16BIT
//
//    	      API_SRAM pSram;
//
//    	      pSram.StPage = SRAM_PAGE_WORK;
//    	      pSram.StAddr = g_prt_que_head;
//    	      pSram.Len = 200;
//    	      api_sram_PageRead( pSram, &buf[1] );
//#endif
//
////           if( UI_PrintStatus( dhn_prt ) == FALSE )
////             return( FALSE );
//
//    	      buf[0] = 200;
//api_putstring_retry:
//    	      if( api_prt_putstring( dhn_prt, FONT0, buf ) != apiOK )
//    	        {
//    	        //return( FALSE );
//    	   	   NosSleep(5);
//    	   	   goto api_putstring_retry;
//    	        }
//    	   g_prt_que_head += 200;
//      	   }
//
//      iLen = g_prt_que_tail - g_prt_que_head;
//      if( iLen != 0 )
//        {
//#ifdef	PLATFORM_16BIT
//        sbuf[0] = SRAM_PAGE_WORK;
//        sbuf[1] = g_prt_que_head & 0x00ff;
//        sbuf[2] = (g_prt_que_head & 0xff00) >> 8;
//        sbuf[3] = 0x00;
//        sbuf[4] = 0x00;
//        sbuf[5] = (UCHAR)iLen;
//        sbuf[6] = 0;
//        api_ram_read( sbuf, &buf[1] );
//#endif
//
//#ifndef	PLATFORM_16BIT
//
//API_SRAM pSram;
//
//	pSram.StPage = SRAM_PAGE_WORK;
//	pSram.StAddr = g_prt_que_head;
//	pSram.Len = iLen;
//	api_sram_PageRead( pSram, &buf[1] );
//
//#endif
//
////      if( UI_PrintStatus( dhn_prt ) == FALSE )
////        result = FALSE;
//
//        buf[0] = (UCHAR)iLen;
//        if( api_prt_putstring( dhn_prt, FONT0, buf ) != apiOK )
//          {
//          result = FALSE;
//          goto END;
//          }
//        }
//
//      if( UI_PrintStatus( dhn_prt ) == FALSE ) // wait for end of printing
//        result = FALSE;
//END:
////    g_prt_que_head = ADDR_PRINTER_QUE; // reset head pointer
//      UI_PrintClear();
//      api_prt_close( dhn_prt );          // close printer device
//
////#endif
//      return( result );
//}
//
// ---------------------------------------------------------------------------
// FUNCTION: initialize chinese font for display.
// INPUT   : code - address of Big5 code table.
//	     bmp  - address of Big5 bitmap table.
//	     num  - number of Bit5 characters.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void UI_InitLcdChineseFont( UINT num, UCHAR *code, UCHAR *bmp )
{
API_LCD_FONT ft;

        ft.FontID = FONT2; // set to FONT3 for formal PINPAD release
        ft.ByteNo = 32;
        ft.Width  = 16;
        ft.Height = 16;
        ft.codPageNo  = 0x81;  // in FLASH page 1
        ft.codStAddr  = code;
        ft.codEndAddr = code + 2*num - 1;	// 57+20 Chinese char
        ft.bmpPageNo  = 0x81;  // in FLASH page 1
        ft.bmpStAddr  = bmp;
        ft.bmpEndAddr = bmp + 32*num - 1;	// 57+20 Chinese char

        api_lcd_initfont(ft);
}

// ---------------------------------------------------------------------------
// FUNCTION: initialize CODE TABLE 1 font for "Application Preferred Name".
// INPUT   : fontid - 0xFF = reset to system default FONT (0 & 1)
//                    0xF0 = setup ISO code table to FONT 0
//                    0xF1 = setup ISO code table to FONT 1
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void UI_InitLcdCodeTableFont( UCHAR fontid )
{
API_LCD_FONT ft;

        ft.FontID = fontid;
        ft.ByteNo = 6;
        ft.Width  = 6;
        ft.Height = 8;
        ft.codPageNo  = 0x83;  // in FLASH page 3
        ft.codStAddr  = (UCHAR *)CODE_TABLE_01;               // not used
        ft.codEndAddr = (UCHAR *)CODE_TABLE_01 + 6*256 - 1;   // not used
        ft.bmpPageNo  = 0x83;  // in FLASH page 3
        ft.bmpStAddr  = (UCHAR *)CODE_TABLE_01;
        ft.bmpEndAddr = (UCHAR *)CODE_TABLE_01 + 6*256 - 1;   // 256 char

        api_lcd_initfont(ft);
}

// ---------------------------------------------------------------------------
