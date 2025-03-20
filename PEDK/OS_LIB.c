//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : i.MX6UL                                                    **
//**  PRODUCT  : AS350-X6                                                   **
//**                                                                        **
//**  FILE     : OS_LIB.C                                                   **
//**  MODULE   : LIB_xxx()		   				                            **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2022/05/24                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2022 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
// 
//----------------------------------------------------------------------------
#include <time.h>

#include "bsp_uart.h"
//#include "bsp_wdt.h"
//#include "bsp_gpio.h"
//#include "za9_pmu.h"
#include "POSAPI.h"
#include "OS_LIB.h"
#include "OS_POST.h"

#include "LCDTFTAPI.h"

UINT8	post_dhn_buz1;
UINT8	post_dhn_buz2;
UINT8	post_dhn_kbd;
UINT8	post_dhn_aux;

extern	UINT8	os_LCD_DID;
//#define	TFTLCD_FONT0_W			9
//#define	TFTLCD_FONT0_H			16
//#define	TFTLCD_FONT1_W			14
//#define	TFTLCD_FONT1_H			24


// ---------------------------------------------------------------------------
// FUNCTION: Clear the whole screen.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	LIB_LCD_Cls( void )
{
UINT8	sbuf[3];
API_LCDTFT_PARA para;


	sbuf[0] = 255;	// row
	sbuf[1] = 0;	// cnt
	sbuf[2] = FONT0;
	// api_lcd_clear( 0, sbuf );

// #ifdef	_BOTH_LCD_ENABLED_

	if( os_LCD_DID )
	  {
	  memset( (UINT8 *)&para, 0x00, sizeof(API_LCDTFT_PARA) );
	  para.Row = 0xFFFF;
	  para.BG_Palette[0] = 0xFF;	// white
	  para.BG_Palette[1] = 0xFF;
	  para.BG_Palette[2] = 0xFF;
	  
	  api_lcdtft_clear( 0, para );
	  }
// #endif
}

// ---------------------------------------------------------------------------
// FUNCTION: Clear display row. (used only when PIN ENTRY)
// INPUT   : row  - row number, 0..N.
//           cnt  - number of rows to be cleared.
//           font - font id & attribute.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	LIB_LCD_ClearRow_EX(UINT8 row, UINT8 cnt, UINT8 font)
{
API_LCDTFT_PARA para;


	if( os_LCD_DID )
	  {
	  memset( (UINT8 *)&para, 0x00, sizeof(API_LCDTFT_PARA) );
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
            
	  para.BG_Palette[0] = 0xF0;	// RGB
	  para.BG_Palette[1] = 0xF0;
	  para.BG_Palette[2] = 0xFF;
	  
	  api_lcdtft_clear( 0, para );
	  }
}

// ---------------------------------------------------------------------------
// FUNCTION: Clear display row.
// INPUT   : row  - row number, 0..N.
//           cnt  - number of rows to be cleared.
//           font - font id & attribute.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	LIB_LCD_ClearRow(UINT8 row, UINT8 cnt, UINT8 font)
{
UINT8	buf[3];
API_LCDTFT_PARA para;


	buf[0]=row;
	buf[1]=cnt;
	buf[2]=font;
	// api_lcd_clear(0, buf);

// #ifdef	_BOTH_LCD_ENABLED_

	if( os_LCD_DID )
	  {
	  memset( (UINT8 *)&para, 0x00, sizeof(API_LCDTFT_PARA) );
	  para.Row = row;
	  para.Col = cnt;
	  
          if( (font & 0x0F) == FONT0 )
            {
            para.Font = FONT0;
            para.FontHeight = FONT0_H;
            para.FontWidth  = FONT0_W;
            }
          else if((font & 0x0F) == FONT1)
            {
            para.Font = FONT1;
            para.FontHeight = FONT1_H;
            para.FontWidth  = FONT1_W;
            }
          else
          {
            para.Font = FONT2;
            para.FontHeight = FONT2_H;
            para.FontWidth  = FONT2_W;
          }
            
	  para.BG_Palette[0] = 0xFF;	// white
	  para.BG_Palette[1] = 0xFF;
	  para.BG_Palette[2] = 0xFF;
	  
	  api_lcdtft_clear( 0, para );
	  }
// #endif
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
void LIB_LCD_Putc(UINT8 row, UINT8 col, UINT8 font, UINT8 data)
{
// UINT8 sbuf[3];
UINT8 dbuf[2];
API_LCDTFT_PARA para;


      // sbuf[0]=row;
      // sbuf[1]=col;
      // sbuf[2]=font;
      dbuf[0]=1;
      dbuf[1]=data;
      // api_lcd_putstring(0, sbuf, dbuf);

// #ifdef	_BOTH_LCD_ENABLED_

      if( os_LCD_DID )
        {
        para.Row = row;
        para.Col = col;
        if( (font & 0x0F) == FONT0 )
          {
          para.Font = FONT0;
          para.FontHeight = FONT0_H;
          para.FontWidth  = FONT0_W;
          }
        else
          {
          para.Font = FONT1;
          para.FontHeight = FONT1_H;
          para.FontWidth  = FONT1_W;
          }
        
        para.FG_Palette[0] = 0x00;
	para.FG_Palette[1] = 0x00;
	para.FG_Palette[2] = 0x00;

	para.BG_Palette[0] = 0xff;
	para.BG_Palette[1] = 0xff;
	para.BG_Palette[2] = 0xff;
	
        api_lcdtft_putstring( 0, para, dbuf );
        }
// #endif
}

// ---------------------------------------------------------------------------
// FUNCTION: Display string.
// INPUT   : row  -- row no.
//           col  -- col no.
//           font -- fontid & attribute
//           len  -- length
//           str  -- string
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	LIB_LCD_Puts( UINT8 row, UINT8 col, UINT8 font, UINT8 len, UINT8 *str )
{
#if	0	// JAMES

UINT8	sbuf[3];
UINT8	dbuf[32];
API_LCDTFT_PARA para;


// 	sbuf[0] = row;
// 	sbuf[1] = col;
// 	sbuf[2] = font;
	dbuf[0] = len;
	memmove( &dbuf[1], str, len );
// 	api_lcd_putstring( 0, sbuf, dbuf );

// #ifdef	_BOTH_LCD_ENABLED_

      if( os_LCD_DID )
        {
        para.Row = row;
        para.Col = col;
        para.Font = font;
        if( (font & 0x0F) == FONT0 )
          {
          para.FontHeight = FONT0_H;
          para.FontWidth  = FONT0_W;
          }
        else if((font & 0x0F) == FONT1)
          {
          para.FontHeight = FONT1_H;
          para.FontWidth  = FONT1_W;
          }
        else if((font & 0x0F) == FONT2)
          {
          para.FontHeight = FONT2_H;
          para.FontWidth  = FONT2_W;
          }
        else
          {
          para.FontHeight = FONT4_H;
          para.FontWidth  = FONT4_W;
          }
        para.FG_Palette[0] = 0x00;
	para.FG_Palette[1] = 0x00;
	para.FG_Palette[2] = 0x00;

	para.BG_Palette[0] = 0xff;
	para.BG_Palette[1] = 0xff;
	para.BG_Palette[2] = 0xff;
	// printf("fid=%d\n",font);
        api_lcdtft_putstring( 0, para, dbuf );
        }
// #endif

#else

UINT8	sbuf[3];
UINT8	dbuf[32];
//API_LCDTFT_PARA para;


	sbuf[0] = row;
	sbuf[1] = col;
	sbuf[2] = font;
	
	if( (strlen(str)+1) == len )	// sizeof(str) = strlen(str)+1
	  len--;
	  
	dbuf[0] = len;
	memmove( &dbuf[1], str, len );
	api_lcd_putstring( 0, sbuf, dbuf );

#endif
}

// ---------------------------------------------------------------------------
// FUNCTION: Display a string on TFTLCD.
// INPUT   : para - parameters of string to be shown on TFTLCD.
//           len  - length of string.
//           msg  - the string to be displayed.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	LIB_LCDTFT_PutStr( API_LCDTFT_PARA para, UINT8 len, UINT8 *msg )
//void	LIB_LCDTFT_PutStr( UCHAR *sbuf, UINT8 len, UINT8 *msg )
{
UCHAR	dbuf[MAX_DSP_CHAR+1];

	
	dbuf[0]=len;
	memmove(&dbuf[1], msg, len);
	
	api_lcdtft_putstring( 0, para, dbuf );
//	api_lcdtft_putstring( 0, sbuf, dbuf );
}

// ---------------------------------------------------------------------------
// FUNCTION: Clear a string on one of the TFTLCD row.
// INPUT   : para - parameters of string to be cleared on TFTLCD.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	LIB_LCDTFT_ClearRow( API_LCDTFT_PARA para )
//void	LIB_LCDTFT_ClearRow( UCHAR *sbuf )
{	
//	api_lcdtft_clear( 0, sbuf );
	api_lcdtft_clear( 0, para );
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
void	LIB_LCD_PutMsg(UINT8 row, UINT8 pos, UINT8 font, UINT8 len, UINT8 *msg)
{
UCHAR sbuf[3];
UCHAR dbuf[MAX_DSP_CHAR+1];
UCHAR fontid;
UCHAR fontno;
UCHAR col = 0;
API_LCDTFT_PARA para;
//API_LCDTFT_PARA para;
      
      

      fontid = font & 0x0f;
      
      // if( fontid == FONT0 )
      //   fontno = LCDTFT_FONT0;
      // else
      //   fontno = LCDTFT_FONT1;
      
      dbuf[0]=len;
      memmove(&dbuf[1], msg, len);
      // font=(font&0xf0)|fontid;
//#ifdef	_BOTH_LCD_ENABLED_


      switch( fontid )
            {
            case FONT0:
//               col = MAX_DSP_WIDTH - len*6;
                 col = 240 - len*FONT0_W;
                 break;

            case FONT1:
            case FONT2:
            case FONT3:
//               col = MAX_DSP_WIDTH - len*8;
		 col = 240 - len*FONT1_W;
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
		  col /= FONT0_W;
		else
		  col /= FONT1_W;

//               if( col != 0 )
//                 col--;
//               font += attrPIXCOLUMN;

                 break;

            case COL_RIGHTMOST:
            
		if( fontid == FONT0 )
		  col /= FONT0_W;
		else
		  col /= FONT1_W;

                 if( col != 0 )
                   col--;
		
//               font += attrPIXCOLUMN;

                 break;
            }

      sbuf[0]=row;
      sbuf[1]=col;
      sbuf[2]=font;
      dbuf[0]=len;
      para.Font = font;
      para.Row = row;
      para.Col = col;
      para.FG_Palette[0] = 0x00;
	    para.FG_Palette[1] = 0x00;
	    para.FG_Palette[2] = 0x00;

	    para.BG_Palette[0] = 0xff;
	    para.BG_Palette[1] = 0xff;
	    para.BG_Palette[2] = 0xff;
      memmove(&dbuf[1], msg, len);
      api_lcdtft_putstring( 0, para, dbuf );
      // api_lcd_putstring(0, sbuf, dbuf);
}

// ---------------------------------------------------------------------------
#if	0
void	LIB_LCD_PutMsg_EX(UINT8 row, UINT8 pos, UINT8 font, UINT8 len, UINT8 *msg)
{
UINT8 sbuf[3];
UINT8 dbuf[MAX_DSP_CHAR+1];
UINT8 fontid;
UINT8 fontno;
UINT8 col = 0;
UINT8 attr;
//API_LCDTFT_PARA para;
API_LCDTFT_PARA para[1];


      fontid = font & 0x0f;
      attr = font & 0xf0;
      
      if( fontid == FONT0 )
        fontno = LCDTFT_FONT0;
      else
        fontno = LCDTFT_FONT1;
      
      dbuf[0]=len;
      memmove(&dbuf[1], msg, len);

#if	0
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
#endif

      if( os_LCD_DID )
        {
        switch( pos )
              {
              case COL_LEFTMOST:
                   col = 0 + 4;
        
                   break;
        
              case COL_MIDWAY:
              
              	   col = (20 - len)/2;
              
#if	0
                   col /= 2;

		   if( fontid == FONT0 )
		     col /= TFTLCD_FONT0_W;
		   else
		     col /= TFTLCD_FONT1_W;
#endif

//		   col = 240 - len*TFTLCD_FONT1_W;
//		   col /= 2;
//		   
//		   row = (row*TFTLCD_FONT1_H)/2;
//		   
//		   attr |= attrPIXCURSOR;
               
                   break;
        
              case COL_RIGHTMOST:
              
	  	  if( fontid == FONT0 )
	  	    col /= TFTLCD_FONT0_W;
	  	  else
	  	    col /= TFTLCD_FONT1_W;
        
                  if( col != 0 )
                    col--;
	  	
//                 font += attrPIXCOLUMN;
        
                   break;
              }

        para->Row = row;
        para->Col = col;
        if( (font & 0x0F) == FONT0 )
          {
          para->Font = LCDTFT_FONT0;
          para->FontHeight = TFTLCD_FONT0_H;
          para->FontWidth  = TFTLCD_FONT0_W;
          }
        else
          {
          para->Font = LCDTFT_FONT1;
          para->FontHeight = TFTLCD_FONT1_H;
          para->FontWidth  = TFTLCD_FONT1_W;
          }
        
        para->Font |= attr;
        
        para->FG_Palette[0] = 0x00;
	para->FG_Palette[1] = 0x00;
	para->FG_Palette[2] = 0x00;

	para->BG_Palette[0] = 0xF0;
	para->BG_Palette[1] = 0xF0;
	para->BG_Palette[2] = 0xFF;
	
        api_lcdtft_putstring( 0, (UCHAR *)para, dbuf );
        }
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Open buzzer.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	LIB_BUZ_Open( void )
{
  const UCHAR onebeep_setup[]   = {1,5,1,0,0};
  const UCHAR longbeep_setup[]   = {1,60,1,0,0};
  //if(post_dhn_buz1==0)
	  post_dhn_buz1=api_buz_open((UCHAR *)onebeep_setup);
  //if(post_dhn_buz2==0)
	  post_dhn_buz2=api_buz_open((UCHAR *)longbeep_setup);
}

// ---------------------------------------------------------------------------
// FUNCTION: Close buzzer.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	LIB_BUZ_Close( void )
{
  api_buz_close(post_dhn_buz1);
  api_buz_close(post_dhn_buz2);
  //post_dhn_buz1=0;
  //post_dhn_buz2=0;
}

// ---------------------------------------------------------------------------
// FUNCTION: Turn on buzzer for 1 short beep.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	LIB_BUZ_Beep1( void )
{
//	return;
	
	api_buz_sound( post_dhn_buz1 );
}

// ---------------------------------------------------------------------------
// FUNCTION: Turn on buzzer for 2 long beep.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	LIB_BUZ_Beep2( void )
{
//	return;
	
	api_buz_sound( post_dhn_buz2 );
}

// ---------------------------------------------------------------------------
// FUNCTION: Warning beeps for tampered conditions. (3 short beeps periodically)
// INPUT   : state	- 1=event at POST
//			        - 2=event at RUNTIME
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	LIB_BUZ_TamperedBeeps( UCHAR state )    //Modified by Tammy
{
    UINT32  i;


    if(state == 2)
    {
        OS_BUZ_TurnOn();
    }
    else
    {
        for(i = 0 ; i < 4 ; i++)
        {
            OS_BUZ_TurnOn();
	
            DelayMS(20);
            
            OS_BUZ_TurnOff();

            DelayMS(20);
        }
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: Warning beeps for entering.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void    LIB_BUZ_PromptForEntering(void)
{
    UINT8	i;


    LIB_WaitTime(100);

    for(i = 0 ; i < 3 ; i++)
    {
        LIB_BUZ_Beep1();
        LIB_WaitTime(100);
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: Ok beeps.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void    LIB_BUZ_OkBeep(void)
{
    UINT8	buz_dhn;
    UINT8   sbuf[5];

    
    sbuf[0] = 2;
    sbuf[1] = 5;
    sbuf[2] = 5;
    buz_dhn = api_buz_open(sbuf);

    // api_buz_mode(1);

    LIB_WaitTime(100);

    if(buz_dhn != apiOutOfService)
        api_buz_sound(buz_dhn);

    LIB_WaitTime(100);

    // api_buz_mode(0);

    api_buz_close(buz_dhn);
}

// ---------------------------------------------------------------------------
// FUNCTION: Error beeps.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void    LIB_BUZ_ErrorBeep(void)
{
    UINT8	buz_dhn;
    UINT8   sbuf[5];


    sbuf[0] = 1;
    sbuf[1] = 80;
    sbuf[2] = 1;
    buz_dhn = api_buz_open(sbuf);

    // api_buz_mode(1);

    LIB_WaitTime(100);

    if(buz_dhn != apiOutOfService)
        api_buz_sound(buz_dhn);

    LIB_WaitTime(100);

    // api_buz_mode(0);

    api_buz_close(buz_dhn);
}

// ---------------------------------------------------------------------------
// FUNCTION: LED blinks periodically.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void    LIB_LED_BlinkPeriodically(void)
{
    API_PCD_ICON icon;
    icon.ID = IID_LED_GREEN;
    icon.BlinkOn = 20;
    icon.BlinkOff = 20;

    // OS_LED_ShowPCD(icon);
    // LIB_WaitTime(10);
}

// ---------------------------------------------------------------------------
// FUNCTION: LED blinks on.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void    LIB_LED_BlinkOn(void)
{
    API_PCD_ICON icon;
    icon.ID = IID_LED_GREEN;
    icon.BlinkOn = 0xFFFF;

    // OS_LED_ShowPCD(icon);
    // LIB_WaitTime(10);
}

// ---------------------------------------------------------------------------
// FUNCTION: LED blinks off.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void    LIB_LED_BlinkOff(void)
{
    API_PCD_ICON icon;
    icon.ID = IID_LED_GREEN;
    icon.BlinkOn = 0x0000;

    // OS_LED_ShowPCD(icon);
    // LIB_WaitTime(10);
}
// ---------------------------------------------------------------------------
// FUNCTION: Open keyboard - all keys.
// INPUT   : none.
// OUTPUT  : post_dhn_kbd
// RETURN  : none.
// ---------------------------------------------------------------------------
void LIB_OpenKeyAll( void )
{
UINT8 buf[5];

      buf[0]=0x0ff;
      buf[1]=0x0ff;
      buf[2]=0x0ff;
      buf[3]=0x0ff;
      buf[4]=0x0ff;
      post_dhn_kbd = api_kbd_open(0, buf);
}

// ---------------------------------------------------------------------------
// FUNCTION: Open keyboard - UP, DOWN, ENTER, CANCEL.
// INPUT   : none.
// OUTPUT  : g_dhn_kbd
// RETURN  : none.
// ---------------------------------------------------------------------------
void LIB_OpenKeySelect()
{
UINT8 buf[5];

      buf[0]=0x020;
      buf[1]=0x000;
      buf[2]=0x020;
      buf[3]=0x024;
      buf[4]=0x000;
      post_dhn_kbd = api_kbd_open(0, buf);
}

// ---------------------------------------------------------------------------
// FUNCTION: Open keyboard - numeric keys, cancel, clear, backspace, OK.
// INPUT   : none.
// OUTPUT  : post_dhn_kbd
// RETURN  : none.
// ---------------------------------------------------------------------------
void LIB_OpenKeyNum( void )
{
UINT8 buf[5];

      buf[0]=0x03c;	// * 7 4 1
      buf[1]=0x03c;
      buf[2]=0x03c;
      buf[3]=0x02c;
      buf[4]=0x000;
      post_dhn_kbd = api_kbd_open(0, buf);
}

// ---------------------------------------------------------------------------
// FUNCTION: Open keyboard - alpha, numeric keys, cancel, clear, backspace, OK.
// INPUT   : none.
// OUTPUT  : g_dhn_kbd
// RETURN  : none.
// ---------------------------------------------------------------------------
void LIB_OpenKeyAlphaNum()
{
UCHAR buf[5];

      buf[0]=0x3c;	// 0x01c;
      buf[1]=0x03c;
      buf[2]=0x03c;
      buf[3]=0x03c;
      buf[4]=0x000;
      post_dhn_kbd = api_kbd_open(0, buf);
}

// ---------------------------------------------------------------------------
// FUNCTION: Get key status.
// INPUT   : post_dhn_kbd
// OUTPUT  : none.
// RETURN  : apiReady
//           apiNotReady
// ---------------------------------------------------------------------------
UINT8	LIB_GetKeyStatus( void )
{
UINT8	buf[1];

	return( api_kbd_status( post_dhn_kbd, buf) );
}

// ---------------------------------------------------------------------------
// FUNCTION: Wait key.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : app key code.
// ---------------------------------------------------------------------------
UINT8	LIB_WaitKey( void )
{
UINT8	key[1];

	api_kbd_getchar( post_dhn_kbd, key );
	LIB_BUZ_Beep1();
	
	return( key[0] );
}

// ---------------------------------------------------------------------------
// FUNCTION: Wait key. (without beep after key depressed)
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : app key code.
// ---------------------------------------------------------------------------
UINT8	LIB_WaitMuteKey( void )
{
UINT8	key[1];

	api_kbd_getchar( post_dhn_kbd, key );
	
	return( key[0] );
}

// ---------------------------------------------------------------------------
// FUNCTION: Wait for N*10ms or exit if any key is pressed.
// INPUT   : ten_ms - unit in step of ten mini-second. (1 sec = 100 unit)
// OUTPUT  : none.
// RETURN  : -1     = time is up without any key pressed.
//           others = the key code pressed within timeout.
// ---------------------------------------------------------------------------
UINT8	LIB_WaitTimeAndKey( UINT16 tenms )
{
UINT8	dhn_tim;
UINT16 	tick1=0, tick2=0;
UINT8	buf[1];

      dhn_tim = api_tim_open(1); // 1 tick unit = 10 ms
      api_tim_gettick( dhn_tim, (UCHAR *)&tick1 );

      do{
        api_tim_gettick( dhn_tim, (UCHAR *)&tick2 );

        if( api_kbd_status( post_dhn_kbd, buf) == apiReady )
          {
          api_tim_close( dhn_tim );
          return( LIB_WaitKey() );
          }

        } while( tick2 - tick1 < tenms );

      api_tim_close( dhn_tim );
      return( -1 );
}

// ---------------------------------------------------------------------------
// FUNCTION: show digit string with right-justified. (max 20 digits)
// INPUT   : row  - row position to show.
//           buf  - L-V, the digit string.
//           type -
//           idle - character to be shown when idle. (eg. '0','-'...)
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void  LIB_ShowKeyIn( UINT8 type, UINT8 idle, UINT8 font, UINT8 row, UINT8 *buf )
{
UINT8 i;
UINT8 len;
UINT8 index;
UINT8 max_dsp_cnt;
UINT8 newbuf[30];

      memmove( newbuf, buf, buf[0]+1 );

//    TL_insert_decimal_point( type, newbuf ); // put decimal point if necessary

      LIB_LCD_ClearRow( row, 1, font );

      if( (font & 0x0f) == FONT0 )
        max_dsp_cnt = MAX_DSP_FONT0_CNT;
      else
        max_dsp_cnt = MAX_DSP_FONT1_CNT;

      // check non-zero idle prompt
      if( (buf[0] == 0) && (idle != '0') )
        {
        newbuf[1] = idle;
        if( (type & NUM_TYPE_LEFT_JUSTIFY) == 0 )
          LIB_LCD_Puts( row, max_dsp_cnt-1, font, 1, &newbuf[1] );
        else
          LIB_LCD_Puts( row, 0, font, 1, &newbuf[1] );
        return;
        }

      if( newbuf[0] == 0 ) // no data in buffer, show '0'
        {
        newbuf[1] = idle;
        if( (type & NUM_TYPE_LEFT_JUSTIFY) == 0 )
          LIB_LCD_Puts( row, max_dsp_cnt-1, font, 1, &newbuf[1] );
        else
          LIB_LCD_Puts( row, 0, font, 1, &newbuf[1] );
        }
      else
        { // check special prompt
        if( (type & NUM_TYPE_STAR) != 0 )
          {
          type &= 0xf7;

          for( i=0; i<newbuf[0]; i++ )
             newbuf[i+1] = '*';
          }

        if( (type & NUM_TYPE_COMMA) == 0 )
          { // NNNNNNNNNNNNNNNN...N
SHOW_NORM:
          if( (max_dsp_cnt-newbuf[0]) > 0 )
            {
            if( (type & NUM_TYPE_LEFT_JUSTIFY) == 0 )
              LIB_LCD_Puts( row, max_dsp_cnt-newbuf[0], font, newbuf[0], &newbuf[1] );
            else
              LIB_LCD_Puts( row, 0, font, newbuf[0], &newbuf[1] );
            }
          else
            {
            index = newbuf[0]-max_dsp_cnt + 1;
            LIB_LCD_Puts( row, 0, font, max_dsp_cnt, &newbuf[index] );
            }
          }  //  9   6   3   0   7   4
        else // NN,NNN,NNN,NNN,NNN,NNN,NNN
          {
          len = newbuf[0];
          if( len < 4 )
            goto SHOW_NORM;
          // to be implemented!
          }
        }
}

// ---------------------------------------------------------------------------
// FUNCTION: get keys & show numeric digits.
// INPUT   : tout - time out before ENTER key pressed. (in seconds, 0=always)
//           row  - row position to show.
//           len  - max. length allowed.
//           type - bit 7 6 5 4 3 2 1 0
//                            | | | | |_ pure digits (NNNNNN)
//                            | | | |___ with sperator (NN,NNN)
//                            | | |_____ with decimal point (NNN.NN) cf. currency exponent.
//                            | |_______ special prompt (eg. ****) cf.
//                            |_________ accept leading '0' (eg. "0123", rather "123")
//
//           idle - character to be shown when idle. (eg. '0','-'...)
// OUTPUT  : buf - the final result. (sizeof(buf) = len+1)
//                 format: LEN[1] DIGIT[n]
// REF     :
// RETURN  : TRUE  = confirmed. (by entering OK)
//           FALSE = aborted.   (by entering CANCEL)
//           -1    = timeout.
// ---------------------------------------------------------------------------
UINT32 LIB_GetNumKey( UINT16 tout, UINT8 type, UINT8 idle, UINT8 font, UINT8 row, UINT8 len, UINT8 *buf )
{
UINT8	i;
UINT8	key;
UINT8	dhn_tim;
UINT16	tick1=0, tick2=0;

      LIB_OpenKeyNum(); // enable numeric keypad

      dhn_tim = api_tim_open(100); // time tick = 1sec
      api_tim_gettick( dhn_tim, (UCHAR *)&tick1 );

KEYIN_CLEAR:

      for( i=0; i<=len; i++ )
         buf[i] = 0;  // clear output buffer

      i = 1;
      buf[0] = 0;
      key = 0;

      while(1)
           {
           if(i == 1)
             LIB_ShowKeyIn( type, idle, font, row, buf );
KEYIN_WAIT:
           if( tout != 0 )
             {
             // wait key
             do{
               // check timeout
               api_tim_gettick( dhn_tim, (UCHAR *)&tick2 );
               if( (tick2 - tick1) >= tout )
                 {
                 api_tim_close( dhn_tim );
                 return( -1 );
                 }
               } while( LIB_GetKeyStatus() == apiNotReady );
             }
           key = LIB_WaitKey();

           switch(key)
                 {
                 case 'x': // cancel
                      for( i=0; i<=len; i++ )
                         buf[i] = 0;

                      api_tim_close( dhn_tim );
                      return( FALSE );
KEYIN_NO:
                 case 'n': // clear
                      LIB_LCD_ClearRow(row, 1, FONT1);
                      i = 1;
                      buf[0] = 0;
                      goto KEYIN_CLEAR;
KEYIN_YES:
                 case 'y': // ok
                //    if( i == 1 )
                //      continue;

                //    if( buf[0] == 0 )	// 2009-07-23
                //      {
                //      buf[0] = 1;
                //      buf[1] = '0';
                //      }

                      api_tim_close( dhn_tim );
                      return( TRUE );

                 case '#': // backspace
                      if( i != 1 )
                        {
                        buf[--i] = 0;
                        buf[0] -= 1;
                        LIB_ShowKeyIn( type, idle, font, row, buf );
                        continue;
                        }
                      else
                        goto KEYIN_CLEAR;
		
		case '*': // decimal point
		     key = '.';
		
                 default: // '0'~'9'
                      if( i > len ) // overflow?
                        continue;

                //    if( (type & NUM_TYPE_STAR) == 0 )
                      if( (type & NUM_TYPE_LEADING_ZERO) == 0 )
                        {
                        if( (key == '0') && (buf[0] == 0) )
                          {
                          buf[0] = 1;
                          buf[1] = '0';
                          LIB_ShowKeyIn( type, idle, font, row, buf );
                          buf[0] = 0;
                          goto KEYIN_WAIT;
                          }
                        }

                      buf[0] = i; // length of current keyin

                //    if( (type & 0x08) != 0 ) // check special prompt
                //      {
                //      type &= 0xf7;
                //      key = '*';
                //      }

                      buf[i++] = key;
                      LIB_ShowKeyIn( type, idle, font, row, buf );

                } // switch(key)
          } // while(1)
}

// ---------------------------------------------------------------------------
// FUNCTION: get key & show one alphanumeric digit.
// INPUT   : cnt  - wait for number of digits captured.
//           dir  - 0=left to right, 1=right to left.
// OUTPUT  : data - KEYS('0'-'9', 'A'-'Z', OK, <-, ->)
// RETURN  : TRUE  ( OK, <-, -> )
//           FALSE ( CANCEL or CLEAR ).
// ---------------------------------------------------------------------------
UINT8   GetAlphaNumKey( UINT8 dir, UINT8 row, UINT8 col, UINT8 font, UINT8 cnt, UINT8 *data )
{
UINT8 buf[21];
UINT8 key;
UINT8 pre_key;
UINT8 alpha_cnt;
UINT8 alpha_flag;
UINT8 old_col;
UINT8 *ptrdata;
UINT8 *ptrbuf;
UINT8 key_cnt;

const UINT8 keypad[10][4]= {
                        {'0', '0', '0', '0'},
                        {'1', '1', '1', '1'},
                        {'A', 'B', 'C', '2'},
                        {'D', 'E', 'F', '3'},
                        {'4', '4', '4', '4'},
                        {'5', '5', '5', '5'},
                        {'6', '6', '6', '6'},
                        {'7', '7', '7', '7'},
                        {'8', '8', '8', '8'},
                        {'9', '9', '9', '9'}
                       };


      LIB_OpenKeyAlphaNum(); // enable numeric keypad

      ptrbuf = data;
      pre_key = 0;
      alpha_cnt = 0;
      alpha_flag = 0;
      key_cnt = 0;

      while(1)
           {
           key = LIB_WaitKey();
           switch( key )
                 {
                 case KEY_CANCEL:

                      *ptrbuf = key;
                      return( FALSE );

                 case KEY_CLEAR:

                      *ptrbuf = key;
                      return( FALSE );

                 case KEY_ENTER:

                      return( TRUE );

                 case KEY_ALPHA:

                      if( pre_key == KEY_BACKSPACE )
                        break;

                      alpha_flag = 1;

                      if( pre_key != 0 )
                        {
                        key = keypad[pre_key-'0'][alpha_cnt++];
                        if( alpha_cnt == 4 )
                          alpha_cnt = 0;

                        if( dir == 0 )
                          {
                          old_col = col - 1;
                          LIB_LCD_Putc( row, old_col, font, key );
                          }

                        ptrdata = data;
                        *(ptrdata-1) = key;

                        if( dir != 0 )
                          {
                          memmove( &buf[1], ptrbuf, key_cnt );
                          buf[0] = key_cnt;
                          LIB_ShowKeyIn( NUM_TYPE_LEADING_ZERO+NUM_TYPE_LEFT_JUSTIFY, '_', font, row, buf );
                          }
                        }
                        break;

                 case KEY_BACKSPACE:

//                    if( dir == 0 )
//                      return( TRUE );

                      if( key_cnt != 0 )
                        {
                        memmove( &buf[1], ptrbuf, key_cnt );
                        key_cnt--;
                        cnt++;
                        *(--data) = 0;
                        if( col )
                          col--;
                        }
                      buf[0] = key_cnt;
//                    LIB_DumpHexData( 0, 4, 7, buf );
                      LIB_ShowKeyIn( NUM_TYPE_LEADING_ZERO+NUM_TYPE_LEFT_JUSTIFY, '_', font, row, buf );

                      pre_key = key;

                      break;

                 default: // 0~9

                      if( cnt == 0 )
                        break;
                      cnt--;

                      if( alpha_flag == 1 )
                        {
                        alpha_cnt = 0;
                        alpha_flag = 0;
                        }

                      if( dir == 0 )
                        LIB_LCD_Putc( row, col++, font, key );

                      *data++ = key;
                      pre_key = key;
                      key_cnt++;

                      if( dir != 0 )
                        {
                        memmove( &buf[1], ptrbuf, key_cnt );
                        buf[0] = key_cnt;
                        LIB_ShowKeyIn( NUM_TYPE_LEADING_ZERO+NUM_TYPE_LEFT_JUSTIFY, '_', font, row, buf );
                        }
                 }
           }
}

// ---------------------------------------------------------------------------
// FUNCTION: get key & show one alphanumeric digit.
// INPUT   : tout - time out before ENTER key pressed. (in seconds, 0=always)
//           cnt  - wait for number of digits captured.
//           dir  - 0=left to right, 1=right to left.
// OUTPUT  : data - KEYS('0'-'9', 'A'-'Z', OK, <-, ->)
// RETURN  : TRUE  ( OK, <-, -> )
//           FALSE ( CANCEL or CLEAR ).
//           -1    = timeout.
// NOTE    : show special prompt ('*')
// ---------------------------------------------------------------------------
UINT8   GetAlphaNumKey2(UINT16 tout, UINT8 dir, UINT8 row, UINT8 col, UINT8 font, UINT8 cnt, UINT8 *data)
{
    UINT8	dhn_tim;
    UINT16	tick1, tick2;
    UINT8   buf[21];
    UINT8   key;
    UINT8   pre_key;
    UINT8   alpha_cnt;
    UINT8   alpha_flag;
    UINT8   old_col;
    UINT8   *ptrdata;
    UINT8   *ptrbuf;
    UINT8   key_cnt;

    const UINT8 keypad[10][4] = {
                            {'0', '0', '0', '0'},
                            {'1', '1', '1', '1'},
                            {'A', 'B', 'C', '2'},
                            {'D', 'E', 'F', '3'},
                            {'4', '4', '4', '4'},
                            {'5', '5', '5', '5'},
                            {'6', '6', '6', '6'},
                            {'7', '7', '7', '7'},
                            {'8', '8', '8', '8'},
                            {'9', '9', '9', '9'}
    };


    LIB_OpenKeyAlphaNum(); // enable numeric keypad

    dhn_tim = api_tim_open(100);	// time tick = 1sec
    api_tim_gettick(dhn_tim, (UINT8 *)&tick1);

    ptrbuf = data;
    pre_key = 0;
    alpha_cnt = 0;
    alpha_flag = 0;
    key_cnt = 0;

    while(1)
    {
        do
        {
            api_tim_gettick(dhn_tim, (UINT8 *)&tick2);
            if((tick2 - tick1) >= tout)
            {
                api_tim_close(dhn_tim);
                return -1;
            }
        } while(LIB_GetKeyStatus() == apiNotReady);

        key = LIB_WaitKey();
        switch(key)
        {
            case KEY_CANCEL:

                *ptrbuf = key;
                api_tim_close(dhn_tim);
                return(FALSE);

            case KEY_CLEAR:

                *ptrbuf = key;
                api_tim_close(dhn_tim);
                return(FALSE);

            case KEY_ENTER:

                api_tim_close(dhn_tim);
                return(TRUE);

            case KEY_ALPHA:

                if(pre_key == KEY_BACKSPACE)
                    break;

                alpha_flag = 1;

                if(pre_key != 0)
                {
                    key = keypad[pre_key - '0'][alpha_cnt++];
                    if(alpha_cnt == 4)
                        alpha_cnt = 0;

                    if(dir == 0)
                    {
                        old_col = col - 1;
                        LIB_LCD_Putc(row, old_col, font, key);

                        LIB_WaitTime(20);
                        memmove(&buf[1], ptrbuf, key_cnt);
                        buf[0] = key_cnt;
                        LIB_ShowKeyIn(NUM_TYPE_STAR + NUM_TYPE_LEADING_ZERO + NUM_TYPE_LEFT_JUSTIFY, '_', font, row, buf);
                    }

                    ptrdata = data;
                    *(ptrdata - 1) = key;

                    if(dir != 0)
                    {
                        memmove(&buf[1], ptrbuf, key_cnt);
                        buf[0] = key_cnt;
                        LIB_ShowKeyIn(NUM_TYPE_STAR + NUM_TYPE_LEADING_ZERO + NUM_TYPE_LEFT_JUSTIFY, '_', font, row, buf);
                    }
                }
                break;

            case KEY_BACKSPACE:

                if(key_cnt != 0)
                {
                    memmove(&buf[1], ptrbuf, key_cnt);
                    key_cnt--;
                    cnt++;
                    *(--data) = 0;
                    if(col)
                        col--;
                }
                buf[0] = key_cnt;
                
                LIB_ShowKeyIn(NUM_TYPE_STAR + NUM_TYPE_LEADING_ZERO + NUM_TYPE_LEFT_JUSTIFY, '_', font, row, buf);

                pre_key = key;

                break;

            default: // 0~9

                if(cnt == 0)
                    break;
                cnt--;

                if(alpha_flag == 1)
                {
                    alpha_cnt = 0;
                    alpha_flag = 0;
                }

                *data++ = key;
                pre_key = key;
                key_cnt++;

                if(dir == 0)
                {
                    LIB_LCD_Putc(row, col++, font, key);

                    LIB_WaitTime(20);
                    memmove(&buf[1], ptrbuf, key_cnt);
                    buf[0] = key_cnt;
                    LIB_ShowKeyIn(NUM_TYPE_STAR + NUM_TYPE_LEADING_ZERO + NUM_TYPE_LEFT_JUSTIFY, '_', font, row, buf);
                }

                if(dir != 0)
                {
                    memmove(&buf[1], ptrbuf, key_cnt);
                    buf[0] = key_cnt;
                    LIB_ShowKeyIn(NUM_TYPE_STAR + NUM_TYPE_LEADING_ZERO + NUM_TYPE_LEFT_JUSTIFY, '_', font, row, buf);
                }
        }
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: wait for alphanumeric digits.
// INPUT   : cnt    - max number of digits captured.
// OUTPUT  : digits - L-V, the captured digits.
// RETURN  : TRUE  - confirmed.
//           FALSE - aborted.
// ---------------------------------------------------------------------------
UINT32  LIB_GetAlphaNumDigits( UINT8 row, UINT8 col, UINT8 font, UINT8 cnt, UINT8 *digits )
{
UINT8 i;
UINT8 result;
UINT8 column;
UINT8 buf[MAX_DSP_CHAR+1];

      column = col;
      digits[0] = 0;

      while(1)
           {
           for( i=0; i<MAX_DSP_CHAR+1; i++ )
              buf[i] = 0;
           LIB_ShowKeyIn( NUM_TYPE_LEADING_ZERO+NUM_TYPE_LEFT_JUSTIFY, '_', font, row, buf );

           // wait & show key
           result = GetAlphaNumKey( 0, row, column, font, cnt, buf );
           if( result == FALSE )
             {
             if( buf[0] == KEY_CLEAR )
               {
               LIB_LCD_ClearRow( row, 1, font );

               digits[0] = 0;
               continue;
               }
             else
               {
               digits[0] = 0;
               return( FALSE ); // aborted
               }
             }
           else
             {
             if( buf[0] == 0 )
               continue;
             else
               {
               for( i=0; i<MAX_DSP_CHAR; i++ )
                  {
                  if( buf[i] != 0 )
                    digits[i+1] = buf[i];
                  else
                    break;
                  }

               digits[0] = i; // final length
               return( TRUE ); // confirmed
               }
             }
           }
}

// ---------------------------------------------------------------------------
// FUNCTION: wait for alphanumeric digits.
// INPUT   : tout - time out before ENTER key pressed. (in seconds, 0=always)
//           cnt    - max number of digits captured.
// OUTPUT  : digits - L-V, the captured digits.
// RETURN  : TRUE  - confirmed.
//           FALSE - aborted.
// NOTE    : show special prompt ('*')
// ---------------------------------------------------------------------------
UINT32  LIB_GetAlphaNumDigits2(UINT16 tout, UINT8 row, UINT8 col, UINT8 font, UINT8 cnt, UINT8 *digits)
{
    UINT8 i;
    UINT8 result;
    UINT8 column;
    UINT8 buf[MAX_DSP_CHAR + 1];

    column = col;
    digits[0] = 0;

    while(1)
    {
        for(i = 0; i < MAX_DSP_CHAR + 1; i++)
            buf[i] = 0;
        LIB_ShowKeyIn(NUM_TYPE_LEADING_ZERO + NUM_TYPE_LEFT_JUSTIFY, '_', font, row, buf);

        // wait & show key
        result = GetAlphaNumKey2(tout, 0, row, column, font, cnt, buf);
        if(result == FALSE)
        {
            if(buf[0] == KEY_CLEAR)
            {
                LIB_LCD_ClearRow(row, 1, font);

                digits[0] = 0;
                continue;
            }
            else
            {
                digits[0] = 0;
                return(FALSE); // aborted
            }
        }
        else if(result == 0xff)
            return(FALSE);  // time out
        else
        {
            if(buf[0] == 0)
                continue;
            else
            {
                for(i = 0; i < MAX_DSP_CHAR; i++)
                {
                    if(buf[i] != 0)
                        digits[i + 1] = buf[i];
                    else
                        break;
                }

                digits[0] = i; // final length
                return(TRUE); // confirmed
            }
        }
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: Enter and check user password.
// INPUT   : row -    row cursor position to show "PASSWORD:".
//	     maxtry - max counter to be tried.
//           psw -    the correct password string. (STRz)
// OUTPUT  : none.
// RETURN  : TRUE  - YES
//           FALSE - NO
//	     -1    - out of retry
// ---------------------------------------------------------------------------
UINT32	LIB_EnterPassWord( UINT32 maxtry, UINT8 row, UINT8 *psw )
{
UINT32	i,j;
UINT32	result = 0;
UINT8	buf[16];
UINT8	len;
UINT8	msg_PASSWORD[] = {"PASSWORD:"};


	LIB_LCD_Puts( row, 0, FONT0, sizeof(msg_PASSWORD)-1, msg_PASSWORD );
	for( i=0; i<maxtry; i++ )
	   {
	   if( LIB_GetNumKey( 30, NUM_TYPE_STAR+NUM_TYPE_LEADING_ZERO+NUM_TYPE_LEFT_JUSTIFY, '_', FONT0, 2, 12, buf ) != TRUE )
	     {
	     result = -1;	// aborted
	     break;
	     }
	   
//	   LIB_LCD_ClearRow( row, 2, FONT0 );	// clear two rows before return
	   
	   len = buf[0];
	   buf[len+1] = 0x00;

	   if( strcmp( psw, &buf[1] ) == 0 )
	     {
	     result = TRUE;
	     break;
	     }
	   else
	     {
	     result = FALSE;
//	     break;
	     }
	   }
	   
	// over retrial counter, may erase all secure message here before returning to caller
	LIB_LCD_ClearRow( row, 2, FONT0 );	// clear two rows before return
	LIB_OpenKeyAll();
	return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: Enter and check user password with length.
// INPUT   : row -    row cursor position to show "PASSWORD:".
//	         maxtry - max counter to be tried.
//           psw -    the correct password string. (L-V)
// OUTPUT  : none.
// RETURN  : TRUE  - YES
//           FALSE - NO
//	         -1    - out of retry
// ---------------------------------------------------------------------------
UINT32	LIB_EnterPassWordLen( UINT32 maxtry, UINT8 row, UINT8 *psw )
{
UINT32	i,j;
UINT32	result = 0;
UINT8	buf[16];
UINT8	len;
UINT8	msg_PASSWORD[] = {"PASSWORD:"};


	LIB_LCD_Puts( row, 0, FONT0, sizeof(msg_PASSWORD)-1, msg_PASSWORD );
	for( i=0; i<maxtry; i++ )
	   {
	   if( LIB_GetNumKey( 30, NUM_TYPE_STAR+NUM_TYPE_LEADING_ZERO, '_', FONT0, 2, 12, buf ) != TRUE )
	     {
	     result = -1;	// aborted
	     break;
	     }
	   
//	   LIB_LCD_ClearRow( row, 2, FONT0 );	// clear two rows before return
	   
	   len = buf[0];
//	   buf[len+1] = 0x00;
	   if( strncmp( psw, buf, len+1 ) == 0 ) // compare L+V
	     {
	     result = TRUE;
	     break;
	     }
	   else
	     {
	     result = FALSE;
//	     break;
	     }
	   }
	   
	// over retrial counter, may erase all secure message here before returning to caller
	LIB_LCD_ClearRow( row, 2, FONT0 );	// clear two rows before return
	LIB_OpenKeyAll();
	return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: Enter and check user password with length.
// INPUT   : row -    row cursor position to show "PASSWORD:".
//	         maxtry - max counter to be tried.
//           psw -    the correct password string. (L-V)
//	         entry  - 0=1'st, 1=2'nd
// OUTPUT  : none.
// RETURN  : TRUE  - YES
//           FALSE - NO
//	         255   - out of retry
// ---------------------------------------------------------------------------
UINT32	LIB_EnterPassWordLen2(UINT32 maxtry, UINT8 row, UINT8 *psw, UINT8 entry)
{
    UINT32	i;
    UINT32	result = 0;
    UINT8	buf[16];
    UINT8	len;
    UINT8	msg_PASSWORD1[] = {"PASSWORD1:"};
    UINT8	msg_PASSWORD2[] = {"PASSWORD2:"};


    if(entry == 0)
        LIB_LCD_Puts(row, 0, FONT0 + attrCLEARWRITE, sizeof(msg_PASSWORD1) - 1, msg_PASSWORD1);
    else
        LIB_LCD_Puts(row, 0, FONT0 + attrCLEARWRITE, sizeof(msg_PASSWORD2) - 1, msg_PASSWORD2);

    for(i = 0; i < maxtry; i++)
    {
        if(LIB_GetNumKey(30, NUM_TYPE_STAR + NUM_TYPE_LEADING_ZERO, '_', FONT0, 2, 12, buf) != TRUE)
        {
            result = 255;	// aborted
            break;
        }

        len = buf[0];
        if(LIB_memcmp(psw, buf, len + 1) == 0)	// 2014-07-27, compare L+V
        {
            result = TRUE;
            break;
        }
        else
        {
            result = FALSE;
        }
    }

    // over retrial counter, may erase all secure message here before returning to caller
    LIB_LCD_ClearRow(row, 2, FONT0);	// clear two rows before return
    LIB_OpenKeyAll();

    // clear sensitive data
	memset(buf, 0x00, sizeof(buf));
    
    return(result);
}

// ---------------------------------------------------------------------------
// FUNCTION: Wait for decision by pressing YES or NO key.
//           Show "(Y/N)?" message at (r,c) and wait for key stroke.
// INPUT   : row - row cursor position.
//	     col - column cursor position.
// OUTPUT  : none.
// RETURN  : TRUE  - YES
//           FALSE - NO
// ---------------------------------------------------------------------------
UINT32	LIB_WaitKeyYesNo( UINT8 row, UINT8 col )
{
UINT8	msg_Q_YES_NO[] = {"(Y/N)?"};
UINT8	key;

	LIB_LCD_Puts( row, col, FONT0, sizeof(msg_Q_YES_NO)-1, msg_Q_YES_NO );
	while(1)
	     {
	     key = LIB_WaitKey();
	     LIB_BUZ_Beep1();
	     if( key == 'y' )	// ENTER
	       return( TRUE );
	     if( (key == 'x') || (key == 'n') )  // CANCEL or CLEAR
	       return( FALSE );
	     }
}

// ---------------------------------------------------------------------------
// FUNCTION: Wait for decision by pressing YES or NO key.
//           Show "Message(Y/N)?" message at (r,c) and wait for key stroke.
// INPUT   : row - row cursor position.
//	     col - column cursor position.
//           msg - message shown to user.
// OUTPUT  : none.
// RETURN  : TRUE  - YES
//           FALSE - NO
// ---------------------------------------------------------------------------
UINT32	LIB_WaitKeyMsgYesNo( UINT8 row, UINT8 col, UINT8 len, UINT8 *msg )
{
UINT8	key;

	LIB_LCD_Puts( row, col, FONT0, len, msg );
	while(1)
	     {
	     key = LIB_WaitKey();
	     LIB_BUZ_Beep1();
	     if( key == 'y' )	// ENTER
	       return( TRUE );
	     if( (key == 'x') || (key == 'n') )  // CANCEL or CLEAR
	       return( FALSE );
	     }
}

// ---------------------------------------------------------------------------
// FUNCTION: Wait for decision by pressing YES or NO key. (with key-waiting timeout)
//           Show "Message(Y/N)?" message at (r,c) and wait for key stroke.
// INPUT   : row - row cursor position.
//	     col - column cursor position.
//           msg - message shown to user.
// OUTPUT  : none.
// RETURN  : TRUE  - YES
//           FALSE - NO
// ---------------------------------------------------------------------------
UINT32	LIB_WaitKeyMsgYesNoTO(UINT8 row, UINT8 col, UINT8 len, UINT8 *msg)
{
    UINT8	key;

    LIB_LCD_Puts(row, col, FONT0, len, msg);
    while(1)
    {
        key = LIB_WaitTimeAndKey(3000);
        LIB_BUZ_Beep1();
        if(key == 'y')	// ENTER
        {
            LIB_LCD_ClearRow(row, 1, FONT0);
            return(TRUE);
        }
        if((key == 'x') || (key == 'n') || (key == 255))  // CANCEL, CLEAR or timeout
            return(FALSE);
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: convert HEX byte to ASCII word.
// INPUT   : data - hex data byte. (0x1A)
// OUTPUT    byte_h - the high byte of ASCII. (0x31)
//           byte_l - the low  byte of ASCII. (0x41)
// RETURN  : none.
// ---------------------------------------------------------------------------
void	LIB_hexb2ascw( UINT8 data, UINT8 *byte_h, UINT8 *byte_l )
{
UINT8 cc, c_hi, c_lo;

      cc = data;
      data &= 0x0f;
      if (data <= 9)
         data += '0';
      else
         {
         data -= 10;
         data += 'A';
         }
      c_lo=data;

      data = cc;
      data &= 0x0f0;
      data >>=  4;
      if (data <= 9)
         data += '0';
      else
         {
         data -= 10;
         data += 'A';
         }
      c_hi=data;

      *byte_h = c_hi;
      *byte_l = c_lo;
}

// ---------------------------------------------------------------------------
// FUNCTION: convert HEX data to ASCII string. (0x1A2B -> "1A2B")
// INPUT   : hexlen - size of hex buffer. (in bytes)
//           hex    - hex data.
// OUTPUT  : str    - L-V, the ascii string.
// RETURN  : the BCD value.
// ---------------------------------------------------------------------------
void	LIB_hex2asc( UCHAR hexlen, UCHAR *hex, UCHAR *str)
{
UCHAR i, j;
UCHAR lo_byte;
UCHAR hi_byte;

      if( hexlen == 0 )
        {
        str[0] = 0;
        return;
        }

      j = 1;
      for( i=0; i<hexlen; i++ )
         {
         LIB_hexb2ascw( *hex++, &hi_byte, &lo_byte );
         str[j++] = hi_byte;
         str[j++] = lo_byte;
         }

      str[0] = j-1; // actual string length
}

// ---------------------------------------------------------------------------
// FUNCTION: Dispaly a hex value of a byte data.
// INPUT   : row  - row position (0..x).
//           col  - col position (0..y).
//           data - the byte data.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	LIB_DispHexByte( UINT8 row, UINT8 col, UINT8 data )
{
UINT8 cc,c_hi,c_lo;
UINT8 sbuf[3], dbuf[3];

      LIB_hexb2ascw( data, &c_hi, &c_lo );

      sbuf[0]=row;
      sbuf[1]=col;
      sbuf[2]=FONT0;

      dbuf[0]=2;
      dbuf[1]=c_hi;
      dbuf[2]=c_lo;
//    api_lcd_putstring(0, sbuf, dbuf);
      LIB_LCD_Puts( row, col, sbuf[2], dbuf[0], &dbuf[1] );
}

// ---------------------------------------------------------------------------
// FUNCTION: Dispaly a hex value of a word data.
// INPUT   : row  - row position (0..x).
//           col  - col position (0..y).
//           data - the word data.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	LIB_DispHexWord( UINT8 row, UINT8 col, UINT16 data )
{
      LIB_DispHexByte( row, col,   (UINT8) ((data & 0x0ff00) >> 8) );
      LIB_DispHexByte( row, col+2, (UINT8) (data & 0x00ff) );
}

// ---------------------------------------------------------------------------
// FUNCTION: Dispaly a hex value of a double word data.
// INPUT   : row  - row position (0..x).
//           col  - col position (0..y).
//           data - the word data.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	LIB_DispHexLong( UINT8 row, UINT8 col, UINT32 data )
{
      LIB_DispHexByte( row, col,   (UINT8) ((data & 0xff000000) >> 24) );
      LIB_DispHexByte( row, col+2, (UINT8) ((data & 0x00ff0000) >> 16) );
      LIB_DispHexByte( row, col+4, (UINT8) ((data & 0x0000ff00) >>  8) );
      LIB_DispHexByte( row, col+6, (UINT8) ( data & 0x000000ff) );
}

// ---------------------------------------------------------------------------
// FUNCTION: Wait for N*10ms.
// INPUT   : ten_ms - unit in step of ten mini-second. (1 sec = 100 unit)
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	LIB_WaitTime( UINT16 tenms )
{
UINT8	dhn_tim;
UINT16	tick1=0, tick2=0;

      dhn_tim = api_tim_open(1); // 1 tick unit = 10 ms
      api_tim_gettick( dhn_tim, (UCHAR *)&tick1 );

      do{
        api_tim_gettick( dhn_tim, (UCHAR *)&tick2 );
        } while( tick2 - tick1 < tenms );

      api_tim_close( dhn_tim );
}

// ---------------------------------------------------------------------------
// FUNCTION: Dump a number of hex byte value to display.
//           format:
//           01234567890123456789
//           xx xx xx xx xx xx xx
// INPUT   : mode - 0=binary, 1=text.
//         : row  - the beginning row number.
//           len  - length of data.
//           data - the hex data.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	LIB_DumpHexData( UINT8 mode, UINT8 row, UINT16 length, UINT8 *data )
{
UINT16	i;
UINT8	col;
UINT8	st_row;
UINT8	maxcol;

      if( (length == 0) || (row > 7) )
        return;

      st_row = row;
      col=0;

      LIB_LCD_ClearRow( st_row, 8-st_row, FONT0 );

      for(i=0; i<length; i++)
         {
         if( mode == 0 )
           {
           LIB_DispHexByte( row, col, data[i] );
           
           col += 3;
	   maxcol = 19;
           }
         else
           {
           LIB_LCD_Putc( row, col, FONT0, data[i]);
//           LIB_LCD_Putc( row, col+1, FONT0, 0x20);

	   col += 1;
	   maxcol = 21;
           }

         if(col >= maxcol)
//         col += 3;
           {
           col = 0;
           row++;
           if(row >=8)
             {
             row = st_row;
             LIB_WaitKey();
             LIB_LCD_ClearRow( st_row, 8-st_row, FONT0 );
             }
           }
         }

      LIB_WaitKey();
}

// ---------------------------------------------------------------------------
// FUNCTION: Dump a number of hex byte value to display. (with key-waiting timeout)
//           format:
//           01234567890123456789
//           xx xx xx xx xx xx xx
// INPUT   : mode - 0=binary, 1=text.
//         : row  - the beginning row number.
//           len  - length of data.
//           data - the hex data.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	LIB_DumpHexDataTO(UINT8 mode, UINT8 row, UINT16 length, UINT8 *data)
{
    UINT16	i;
    UINT8	col;
    UINT8	st_row;
    UINT8	maxcol;
    UINT8	maxrow = 20 - 5;	// 8


    if((length == 0) || (row > maxrow))
        return;

    st_row = row;
    col = 0;

    LIB_LCD_ClearRow(st_row, maxrow - st_row, FONT0);

    for(i = 0; i < length; i++)
    {
        if(mode == 0)
        {
            LIB_DispHexByte(row, col, data[i]);

            col += 3;
            maxcol = 19;
        }
        else
        {
            LIB_LCD_Putc(row, col, FONT0, data[i]);
            //           LIB_LCD_Putc( row, col+1, FONT0, 0x20);

            col += 1;
            maxcol = 21;
        }

        //         col += 3;
        if(col >= maxcol)
        {
            col = 0;
            row++;
            if(row >= maxrow)
            {
                row = st_row;
                LIB_WaitTimeAndKey(1500);
                LIB_LCD_ClearRow(st_row, maxrow - st_row, FONT0);
            }
        }
    }

    LIB_WaitTimeAndKey(1500);
}

// ---------------------------------------------------------------------------
// FUNCTION: Display & wait to Select an item from the specified List.
// INPUT   : para - the parameters.
//                  byte 00 = start row number.
//                       01 = max number of rows of the display.
//                       02 = number of items in list.
//                       03 = length of an item in byte.
//                       04 = offset address of length field in an item.
//                       05 = font id. (FONT0 or FONT2)
//           list - the list, format: XXXX LEN[1] ITEM[n] YYYY
//                  where: LEN[1] & ITEM[n] are mandatory, length <= 16.
//           tout - time out in second. (0=always)
//           start- start cursor position. (0..N)
// OUTPUT  : none.
// RETURN  : -1     - if aborted or time out.
//         : others - item number of the selection. (0..n)
// UI-KEY  : keys allowed - down/up arrow, OK (confirmed), Cancel (aborted).
// UI-DISP : > ITEM_NAME1
//             ITEM_NAME2
// ---------------------------------------------------------------------------
UINT8	LIB_ListBox( UINT8 start, UINT8 *para, UINT8 *list, UINT16 tout )
{
UINT8 i;
UINT8 user_select; // user's selection
UINT8 list_start;  // menu start index
UINT8 cursor_pos;  // cursor position
UINT8 list_bottom;
UINT8 key;
UINT8 row;
UINT8 max_row;
UINT8 items;
UINT8 len;
UINT8 ofs;
UINT8 dhn_tim;
UINT16	tick1=0, tick2=0;
UINT8 font;

      LIB_OpenKeySelect();

      dhn_tim = api_tim_open(100); // time tick = 1sec
      api_tim_gettick( dhn_tim, (UCHAR *)&tick1 );

      row = para[0];
      max_row = para[1];
      items = para[2];
      len = para[3];
      ofs = para[4];
      font = para[5];

      user_select = start; // start item #

      if( items < (max_row - row) )
        list_bottom = items + row;
      else
        list_bottom = max_row;
      while(1)
           {
           if( user_select < (max_row - row) )
             list_start = 0;
           else
             list_start = user_select - max_row + 1 + row;

           cursor_pos = user_select - list_start + row;

           for( i=row; i<list_bottom; i++ )
              {
              LIB_LCD_Putc( i, 0, font, 0x20 ); // clear prefix cursor mark
              LIB_LCD_Puts( i, 2, font+attrCLEARWRITE, list[list_start*len+ofs], &list[list_start*len+ofs+1] );
              list_start++;
              }
           LIB_LCD_Putc( cursor_pos, 0, font, 0x3e );  // show cursor mark

           if( items > 1 )
             {
             if( user_select == 0 )
               LIB_LCD_Putc( max_row-1, 19, FONT0, 0x94 );    // show down arrow
             else
               {
               if( user_select == (items - 1) )
                 LIB_LCD_Putc( max_row-1, 19, FONT0, 0x93 );  // show up arrow
               else
                 LIB_LCD_Putc( max_row-1, 19, FONT0, 0x92 );  // show up/down arrow
               }
             }
LB_WAIT_KEY:
           // wait key
           if( tout != 0 )
             {
             do{
               // check timeout
               api_tim_gettick( dhn_tim, (UCHAR *)&tick2 );
               if( (tick2 - tick1) >= tout )
                 {
                 api_tim_close( dhn_tim );
                 return( -1 );
                 }
               } while( LIB_GetKeyStatus() == apiNotReady );
             }
           key = LIB_WaitKey();

           switch(key)
                 {
                 case '*': // DOWN
                      if( user_select < (items - 1) )
                         user_select+=1;
                      break;

                 case '#': // UP
                      if( user_select > 0 )
                        user_select-=1;
                      break;

                 case 'y': // ENTER
                      api_tim_close( dhn_tim );
                      LIB_OpenKeyAll();

                      return( user_select );

                 case 'x': // CANCEL
                      api_tim_close( dhn_tim );
                      LIB_OpenKeyAll();

                      return( -1 );

                 defaut:
                      goto LB_WAIT_KEY;
                 }

           } //  while(1)
}

// ---------------------------------------------------------------------------
// FUNCTION: open AUX device for communicaiton. (9600, 8, n, 1)
// INPUT   : port
//	     mode
// OUTPUT  : none.
// REF     : g_dhn_aux
// RETURN  : TRUE  - OK
//           FALSE - device error.
// ---------------------------------------------------------------------------
#if 0
UINT8	LIB_OpenAUX( UINT8 port, UINT8 mode )
{
//API_AUX	pAux;
API_AUX	pAux[1];

	pAux->Mode = mode;	// default: auxDLL
	pAux->Baud = COM_9600 + COM_CHR8 + COM_NOPARITY + COM_STOP1;
	pAux->Tob = 100;	// 1 second
	pAux->Tor = 300;	// 3 seconds
	pAux->Acks = 0;
	pAux->Resend = 0;
	//post_dhn_aux = api_aux_open( port, (UINT8 *)pAux );
  post_dhn_aux = api_aux_open( port, pAux[0] );

      if( (post_dhn_aux == apiOutOfLink) || (post_dhn_aux == apiOutOfService) )
        {
        api_aux_close(0);
        return( FALSE );
        }
      else
        return( TRUE );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: open AUX device for communicaiton.
// INPUT   : port
//	         mode
//	         baud = COM_xxxx
// OUTPUT  : none.
// REF     : g_dhn_aux
// RETURN  : TRUE  - OK
//           FALSE - device error.
// ---------------------------------------------------------------------------
UINT8	LIB_OpenAUX(UINT8 port, UINT8 mode, UINT16 baud)
{
    API_AUX	pAux;

    LIB_WaitTime(50);

    pAux.Mode = mode;	// default: auxDLL
    pAux.Baud = baud + COM_CHR8 + COM_NOPARITY + COM_STOP1;
    pAux.Tob = 100;	// 1 second
    pAux.Tor = 300;	// 3 seconds
    if(mode != auxBYPASS)
        pAux.Acks = 1;
    else
        pAux.Acks = 0;
    pAux.Resend = 0;
    post_dhn_aux = api_aux_open(port, pAux);

    //if((post_dhn_aux == apiOutOfLink) || (post_dhn_aux == apiOutOfService))
    if(post_dhn_aux == apiOutOfService)
    {
        api_aux_close(0);
        return(FALSE);
    }
    else
        return(TRUE);
}

// ---------------------------------------------------------------------------
// FUNCTION: close AUX device.
// INPUT   : none.
// OUTPUT  : none.
// REF     : g_dhn_aux
// RETURN  : TRUE  - OK
//           FALSE - device error.
// ---------------------------------------------------------------------------
UINT8	LIB_CloseAUX( void )
{
UINT8 result;

      result = api_aux_close( post_dhn_aux );
      if( result != apiOK )
        {
        api_aux_close(0);
        return( FALSE );
        }
      else
        return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: To determine whether data is ready for input from AUX port.
// INPUT   : none.
// OUTPUT  : data - length of the received data string.
// REF     : g_dhn_aux
// RETURN  : TRUE  - data ready.
//           FALSE - device error or timeout before data ready.
// ---------------------------------------------------------------------------
UINT8   LIB_ReceiveReadyAUX(UINT8 *data)
{
    UINT8	dhn_tim;
    UINT16	tick1 = 0, tick2 = 0;

    dhn_tim = api_tim_open(100); // time tick = 1sec
    api_tim_gettick(dhn_tim, (UCHAR *)&tick1);

    do
    {
        api_tim_gettick(dhn_tim, (UCHAR *)&tick2);

        if(api_aux_rxready(post_dhn_aux, data) == apiReady)
        {
            api_tim_close(dhn_tim);
            return TRUE;
        }
    }while((tick2 - tick1) <= 10); // timeout 10s

    api_tim_close(dhn_tim);
    return FALSE;
}

// ---------------------------------------------------------------------------
// FUNCTION: receive data from AUX port.
// INPUT   : none.
// OUTPUT  : data - 2L-V, the data receivd.
// REF     : g_dhn_aux
// RETURN  : TRUE  - data ready.
//           FALSE - device error or timeout before data ready.
// ---------------------------------------------------------------------------
UINT8	LIB_ReceiveAUX( UINT8 *data )
{
UINT8	dhn_tim;
UINT16	tick1=0, tick2=0;

      dhn_tim = api_tim_open(100); // time tick = 1sec
      api_tim_gettick( dhn_tim, (UCHAR *)&tick1 );

      do{
        api_tim_gettick( dhn_tim, (UCHAR *)&tick2 );

        if( api_aux_rxready( post_dhn_aux, data ) == apiReady )
          {
            
          api_aux_rxstring( post_dhn_aux, data );
          // printf("LIB_ReceiveAUX api_aux_rxstring\n");
          api_tim_close( dhn_tim );
          // printf("return( TRUE )\n");
          return( TRUE );
          }
        } while( (tick2 - tick1) <= 10 ); // timeout 10s

      api_tim_close( dhn_tim );
      return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: transmit data to AUX port.
// INPUT   : data - 2L-V, the data to be transmitted.
// OUTPUT  : none.
// REF     : g_dhn_aux
// RETURN  : TRUE  - data ready.
//           FALSE - device error.
// ---------------------------------------------------------------------------
UINT8	LIB_TransmitAUX( UINT8 *data )
{
UINT8	dhn_tim;
UINT16	tick1=0, tick2=0;
UINT8	result;

      dhn_tim = api_tim_open(100); // time tick = 1sec
      api_tim_gettick( dhn_tim, (UCHAR *)&tick1 );

      do{
        api_tim_gettick( dhn_tim, (UCHAR *)&tick2 );

              printf("LIB_TransmitAUX txready\n");
          api_aux_txstring( post_dhn_aux, data );

          // wait for ACK from HOST
          do{
            api_tim_gettick( dhn_tim, (UCHAR *)&tick2 );

            result = api_aux_txready( post_dhn_aux );
            if( result == apiFailed)
              break;

            if( result == apiReady )
              {
              api_tim_close( dhn_tim );
              return( TRUE );
              }
            } while( (tick2 - tick1) <= 3 );

        
        } while( (tick2 - tick1) <= 3 );

      api_tim_close( dhn_tim );
      return( FALSE );
}
// ---------------------------------------------------------------------------
// FUNCTION: manualy send ack.
// INPUT   : none.
// OUTPUT  : data - 2L-V, the data receivd.
// REF     : g_dhn_aux
// RETURN  : TRUE  - data ready.
//           FALSE - device error or timeout before data ready.
// ---------------------------------------------------------------------------
UINT8	LIB_AUXsendACK(UINT8  acknum)
{
    if(BSP_UART_SendAck( post_dhn_aux, acknum))
      return( TRUE );
    else
      return( FALSE );
}
// ---------------------------------------------------------------------------
// FUNCTION: convert integer to ASCII string.
// INPUT   : value - the integer.
// OUTPUT  : abuf  - the ASCII string.
// RETURN  : the length of the output string in bytes.
// ---------------------------------------------------------------------------
UINT8	LIB_itoa( UINT16 value, UINT8 *abuf )
{
UINT8	rem;
UINT8	i,len;
UINT16	quo;
UINT8	count;

      i=0;
      len=0;

      if( value < 10 )
        {
        abuf[0] = value | '0';
        return (1);
        }

      do{
        quo = value / 10;
        rem = value % 10;
        rem |= '0';
        abuf[i++] = rem;
        len++;
        value = quo;
        }while( quo >= 10 );
      abuf[i++] = quo | '0';
      len++;

      //  re-position

      count = len / 2;

      for(i=0; i<count; i++)
         {
         rem = abuf[i];
         abuf[i] = abuf[len-i-1];
         abuf[len-i-1] = rem;
         }

      return len;
}

// ---------------------------------------------------------------------------
// FUNCTION: convert long to ASCII string.
// INPUT   : value - the long data.
// OUTPUT  : abuf  - the ASCII string.
// RETURN  : the length of the output string in bytes.
// ---------------------------------------------------------------------------
UINT8	LIB_ltoa( UINT32 value, UINT8 *abuf )
{
UINT8	rem;
UINT8	i,len;
UINT32	quo;
UINT8	count;

      i=0;
      len=0;

      if( value < 10 )
        {
        abuf[0] = value | '0';
        return (1);
        }

      do{
        quo = value / 10;
        rem = value % 10;
        rem |= '0';
        abuf[i++] = rem;
        len++;
        value = quo;
        }while( quo >= 10 );
      abuf[i++] = quo | '0';
      len++;

      //  re-position

      count = len / 2;

      for(i=0; i<count; i++)
         {
         rem = abuf[i];
         abuf[i] = abuf[len-i-1];
         abuf[len-i-1] = rem;
         }

      return len;
}

// ---------------------------------------------------------------------------
// FUNCTION: convert BCD data to ASCII string.
// INPUT   : bcdlen - size of bcd buffer. (in bytes)
//           bcd    - right-justified bcd data with leading zeros.
// OUTPUT  : str    - L-V, the ascii string.
// RETURN  : the BCD value.
// ---------------------------------------------------------------------------
void	LIB_bcd2asc( UINT8 bcdlen, UINT8 *bcd, UINT8 *str )
{
UINT8 i, j;
UINT8 lo_byte;
UINT8 hi_byte;

      if( bcdlen == 0 )
        {
        str[0] = 0;
        return;
        }

      j=1;
      for( i=0; i<bcdlen; i++ )
         {
         hi_byte = (bcd[i] & 0xf0) >> 4;
         lo_byte = bcd[i] & 0x0f;

         if( hi_byte == 0x0f ) // "cn" padding? (eg. 0x12 0xFF)
           break;

         if( lo_byte != 0x0f ) // "cn" padding? (eg. 0x12 0x3F)
           {
           str[j++] = hi_byte | 0x30;
           str[j++] = lo_byte | 0x30;
           }
         else
           {
           str[j++] = hi_byte | 0x30; // the last digit
           break;
           }
         }

      str[0] = j-1; // actual string length
}

// ---------------------------------------------------------------------------
// FUNCTION: convert ASCII word to HEX byte.
// INPUT   : buf - the ascii word. (eg. "1A" -> 0x1a)
// OUTPUT  : none.
// RETURN  : the HEX value.
// ---------------------------------------------------------------------------
UINT8	LIB_ascw2hexb( UINT8 buf[] )
{
UINT8	result;
UINT8	hi_byte;
UINT8	lo_byte;

      hi_byte = buf[0];
      lo_byte = buf[1];

      if( hi_byte <= '9' )
        hi_byte &= 0x0f;
      else
        {
        hi_byte -= 'A';
        hi_byte += 10;
        }
      hi_byte <<= 4;

      if( lo_byte <= '9' )
        lo_byte &= 0x0f;
      else
        {
        lo_byte -= 'A';
        lo_byte += 10;
        }

      return( hi_byte | lo_byte );
}

// ---------------------------------------------------------------------------
// FUNCTION: Compares memory. (MSB-LSB)
// INPUT   : s1  - a pointer to the first object.
//           s2  - a pointer to the second object.
//           len - length of data to be compared in the object.
// OUTPUT  : none.
// RETURN  : >0 - s1 > s2
//           =0 - s1 = s2
//           <0 - s1 < s2
// ---------------------------------------------------------------------------
#if	0
int	LIB_memcmp( UINT8 *s1, UINT8 *s2, UINT32 len )
{
UINT32	i;
UINT8	data1, data2;

      for( i=0; i<len; i++ )
         {
         data1 = *s1++;
         data2 = *s2++;
         if( data1 > data2 )
           return(1);
         else
           {
           if( data1 < data2 )
             return(-1);
           }
         }
      return(0);
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Compares memory. (MSB-LSB), new algorithm to prevent timing attacks.
// INPUT   : s1  - a pointer to the first object.
//           s2  - a pointer to the second object.
//           len - length of data to be compared in the object.
// OUTPUT  : none.
// RETURN  : 
//           0 if s1 = s2
//           otherwise s1 != s2
// ---------------------------------------------------------------------------
int	LIB_memcmp( UINT8 *s1, UINT8 *s2, UINT32 len )
{
UINT32	i;
UINT8	*ss1 = (UINT8 *)s1;
UINT8	*ss2 = (UINT8 *)s2;
int	result = 0;


	for( i=0; i<len; i++ )		// 2014-07-27
	   {
	   result |= ss1[i] ^ ss2[i];
	   }
	   
	return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: Compares memory with the specified character.
// INPUT   : s1  - a pointer to the object.
//           cc  - the data byte to be compared.
//           len - length of data to be compared.
// OUTPUT  : none.
// RETURN  : =0  - all s1 = cc.
//           !=0 - not all s1 = cc.
// ---------------------------------------------------------------------------
int	LIB_memcmpc( UINT8 *s1, UINT8 cc, UINT32 len )
{
UINT32	i;

      if( len == 0 )
        return(-1);

      for( i=0; i<len; i++ )
         {
         if( *s1++ != cc )
           return(-1);
         }
      return(0);
}

// ---------------------------------------------------------------------------
// FUNCTION: Reset system.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	LIB_ResetSystem( void )
{
#if	0

#if	0	// Warm Reset
	// memory remap
	BSP_WR32( PMU_CFG_REG,  BSP_RD32( PMU_CFG_REG ) & 0xFFFFFFFE );

	// soft reset
	BSP_WR32( PMU_RST_REG,  BSP_RD32( PMU_RST_REG ) | 0x00010000 );
#endif


BSP_WDT		*pWdt;
BSP_GPIO	*pHwRst;
UINT32		Temp;
UINT32		Div;

	
	// Trun off INT & BUZZER
	BSP_DisableInterrupts( BSP_INT_MASK );
	
	OS_BUZ_TurnOff();
	
	// 2014-03-12, Hardware Reset by power down
	// GPIO0[14]	-> HI
	
	pHwRst = BSP_GPIO_Acquire( GPIO_PORT_0, BIT14 );
	if( pHwRst )
	  {
	  pHwRst->Mode     = GPIO_MODE_OUTPUT;
	  pHwRst->IntMode  = GPIO_INT_DISABLE;
	  pHwRst->Wake     = GPIO_WAKE_DISABLE;
	  pHwRst->Data     = 0;
	  pHwRst->pIsrFunc = NULLPTR;
	  
	  BSP_GPIO_Start( pHwRst );
	  
	  BSP_GPIO_OUT_1( pHwRst );	// start to reset...
	  }

	BSP_Delay_n_ms(500);

	// 2013-12-27, new method of reset (watch dog reset)
	//	WDT
	//	Interrupt Period = 0.745 seconds
	//	Reset     Period = 1.49  seconds

	pWdt = BSP_WDT_Acquire( NULLPTR );

	Temp = BSP_GetClock( TRUE, 1 );
	for( Div = 17; Div<31; Div++ )
	{
		if( (Temp >> Div) == 0 )
		{
			break;
		}
	}
	Div = 31 - Div;

	pWdt->Control = WDT_RST_EN
		       | WDT_INT_EN
		       | WDT_EN
		       | (Div << 4)	// default Div = 4
		       | (Div + 1);

	BSP_WDT_Start( pWdt );

	while(1);
	
#endif
  api_sys_reset( 0 );
}
void	LIB_BUZ_MPU_MemManage( void )
{
	// OS_LED_Blink(5);
}

// ---------------------------------------------------------------------------
void	LIB_BUZ_MPU_HardFault( void )
{
	// OS_LED_Blink(6);
}

// ---------------------------------------------------------------------------
// FUNCTION: convert RTC to UNIX time.
// INPUT   : abuf - YYMMDDhhmmss (in ASCII)
// OUTPUT  : none.
// RETURN  : unix time in seconds. (since 1970)
// ---------------------------------------------------------------------------
#if	0
UINT32	LIB_UnixTime( UINT8 *abuf )
{
struct	tm ltime;

UINT8	temp[3];
UINT32	year;
UINT32	mon;
UINT32	day;
UINT32	hour;
UINT32	min;
UINT32	sec;
UINT32	tseconds;

	
	// convert ASCII to binary
	temp[0] = abuf[0];
	temp[1] = abuf[1];
	temp[2] = 0;
	year = atoi( temp );
	if( year <= 0x49 )
	  year += 2000;
	else
	  year += 1900;
	
	temp[0] = abuf[2];
	temp[1] = abuf[3];
	mon = atoi( temp );
	
	temp[0] = abuf[4];
	temp[1] = abuf[5];
	day = atoi( temp );
	
	temp[0] = abuf[6];
	temp[1] = abuf[7];
	hour = atoi( temp );
	
	temp[0] = abuf[8];
	temp[1] = abuf[9];
	min = atoi( temp );
	
	temp[0] = abuf[10];
	temp[1] = abuf[11];
	sec = atoi( temp );
	
	ltime.tm_year = year-1900;
	ltime.tm_mon = mon - 1;
	ltime.tm_mday = day;
	ltime.tm_hour = hour;
	ltime.tm_min = min;
	ltime.tm_sec = sec;
	tseconds = (ULONG)mktime( &ltime );	// total seconds elapsed since 1970
	
	return( tseconds );
}
#endif

// ---------------------------------------------------------------------------
void	DelayMS( UINT32 dly)
{
volatile UINT32	i = 0;

	for ( ; dly > 0; dly--)
		for (i = 0; i < 16000*30; i++);
}
