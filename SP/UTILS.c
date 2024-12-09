//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : 		                                            **
//**  PRODUCT  : AS350                                                      **
//**                                                                        **
//**  FILE     : UTILS.C	                                            **
//**  MODULE   : 					                    **
//**									    **
//**  FUNCTION : Useful utilities for testing APIs.			    **
//		 Please refer to the API doc for details.		    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2011/01/04                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2015 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
// 
// For large data dumping through COM port, the following system functions are useful.
// Please refer to the sample codes in UTIL_DumpHexData() or UTIL_DumpHexData2().
//
// On terminal:
// _DEBUGPRINTF_OPEN( ULONG UartPort );
//	Enable the target COM port with default setting - 115200,8,N,1.
//
// _DEBUGPRINTF( char *Msg, ... );
//	Print formatted output to the target COM port.
//	The usage is the same as standard C function "printf()".
//	Max. size of data output is 256 bytes.
//
// On PC:
// Windows HyperTerminal or the similar program can be used as the receiver.
//
//----------------------------------------------------------------------------
#include <string.h>
#include <stdlib.h>

#include "POSAPI.h"
#include "UTILS.h"

// Uncomment the following definition if the printer is supported
//#define	_MOUNT_PRINTER_

UCHAR		util_dhn_kbd;
UCHAR		util_dhn_buz;
UCHAR		util_dhn_lcd;
UCHAR		util_dhn_prt;


// ---------------------------------------------------------------------------
// FUNCTION: Open buzzer for 1 short beep.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UTIL_OpenBuzzer_1S(void)
{
UCHAR	buf[3+2];

	buf[0]=1;
	buf[1]=5;
	buf[2]=5;
	
	buf[3] = 0;	// setup default frequency
	buf[4] = 0;	//
	util_dhn_buz = api_buz_open(buf);
}

// ---------------------------------------------------------------------------
// FUNCTION: Turn on buzzer for 1 short beep.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UTIL_BUZ_Beep1( void )
{
	api_buz_sound( util_dhn_buz );
}

// ---------------------------------------------------------------------------
// FUNCTION: Open keyboard - all keys.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UTIL_OpenKey_ALL( void )
{
UCHAR	buf[5];

	buf[0]=0x0ff;
	buf[1]=0x0ff;
	buf[2]=0x0ff;
	buf[3]=0x0ff;
	buf[4]=0x0ff;
	util_dhn_kbd = api_kbd_open(0, buf);
}

// ---------------------------------------------------------------------------
// FUNCTION: Open keyboard - alpha, numeric keys, cancel, clear, backspace, OK.
// INPUT   : none.
// OUTPUT  : g_dhn_kbd
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UTIL_OpenKeyAlphaNum( void )
{
UCHAR	buf[5];

	buf[0]=0x03c;	// asterisk(*) as alpha key
	buf[1]=0x03c;
	buf[2]=0x03c;
	buf[3]=0x02c;
	buf[4]=0x000;
	util_dhn_kbd = api_kbd_open(0, buf);
}

// ---------------------------------------------------------------------------
// FUNCTION: Wait one key-stroke.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : the key code data.
// ---------------------------------------------------------------------------
UCHAR	UTIL_WaitKey(void)
{
UCHAR	buf[1];

	api_kbd_getchar(util_dhn_kbd, buf);
	UTIL_BUZ_Beep1();
	return(buf[0]);
}

// ---------------------------------------------------------------------------
// FUNCTION: Wait one key-stroke.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : the key code data.
// ---------------------------------------------------------------------------
UCHAR	UTIL_WaitKeyNoSound(void)
{
UCHAR	buf[1];

	api_kbd_getchar(util_dhn_kbd, buf);
	return(buf[0]);
}

// ---------------------------------------------------------------------------
// FUNCTION: Get key status.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiReady
//           apiNotReady
// ---------------------------------------------------------------------------
UCHAR	UTIL_GetKeyStatus(void)
{
UCHAR	buf[1];

	return( api_kbd_status( util_dhn_kbd, buf) );
}

// ---------------------------------------------------------------------------
// FUNCTION: switch alphanumeric keypad.
// INPUT   : key - the latest key code before "ALPHA" key is pressed.
// OUTPUT  : key - the new key code after "ALPHA" key is pressed.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UTIL_SwitchAlphanumericKey( UCHAR *key )
{
UCHAR	i, j;
 
const	UCHAR alpha_tbl[10][4]=
	{{'0',',',';','0'},
	 {'1','Q','Z','.'},
	 {'2','A','B','C'},
	 {'3','D','E','F'},
	 {'4','G','H','I'},
	 {'5','J','K','L'},
	 {'6','M','N','O'},
	 {'7','P','R','S'},
	 {'8','T','U','V'},
	 {'9','W','X','Y'}};
 
 
	for(i=0; i<=9; i++)
	   {
	   for(j=0; j<=3; j++)
	      { // equals any character in alpha_tbl?
	      if( *key == alpha_tbl[i][j] )
	        {                                     
	        *key = alpha_tbl[i][(j+1)%4];
	        return;
	        }
	      }
	   }
}

// ---------------------------------------------------------------------------
// FUNCTION: Open LCD.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UTIL_OpenDisplay( void )
{
	util_dhn_lcd = api_lcd_open( 0 );
}

// ---------------------------------------------------------------------------
// FUNCTION: Clear whole display screen.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UTIL_ClearScreen(void)
{
UCHAR	buf[3];

	buf[0]=-1;
	buf[1]=0;
	buf[2]=FONT1;
	api_lcd_clear(util_dhn_lcd, buf);
}

// ---------------------------------------------------------------------------
// FUNCTION: Clear whole display screen.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UTIL_BlackenScreen(void)
{
UCHAR	buf[3];

	buf[0]=-1;
	buf[1]=0;
	buf[2]=FONT1+attrREVERSE;
	api_lcd_clear(util_dhn_lcd, buf);
}

// ---------------------------------------------------------------------------
// FUNCTION: Clear display row.
// INPUT   : row  - row number, 0..N.
//           cnt  - number of rows to be cleared.
//           font - font id & attribute.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UTIL_ClearRow(UCHAR row, UCHAR cnt, UCHAR font)
{
UCHAR	buf[3];

	buf[0]=row;
	buf[1]=cnt;
	buf[2]=font;
	api_lcd_clear(util_dhn_lcd, buf);
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
void	UTIL_PutChar(UCHAR row, UCHAR col, UCHAR font, UCHAR data)
{
UCHAR	sbuf[3];
UCHAR	dbuf[2];

	sbuf[0]=row;
	sbuf[1]=col;
	sbuf[2]=font;
	dbuf[0]=1;
	dbuf[1]=data;
	api_lcd_putstring(util_dhn_lcd, sbuf, dbuf);
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
void	UTIL_PutStr(UCHAR row, UCHAR col, UCHAR font, UCHAR len, UCHAR *msg)
{
UCHAR	sbuf[3];
UCHAR	dbuf[64];

	sbuf[0]=row;
	sbuf[1]=col;
	sbuf[2]=font;
	dbuf[0]=len;
	memmove(&dbuf[1], msg, len);
	api_lcd_putstring(util_dhn_lcd, sbuf, dbuf);
}

// ---------------------------------------------------------------------------
// FUNCTION: convert HEX byte to ASCII word.
// INPUT   : data - hex data byte. (0x1A)
// OUTPUT    byte_h - the high byte of ASCII. (0x31)
//           byte_l - the low  byte of ASCII. (0x41)
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UTIL_hexb2ascw( UCHAR data, UCHAR *byte_h, UCHAR *byte_l )
{
UCHAR	cc, c_hi, c_lo;

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
// FUNCTION: Dispaly a hex value of a byte data.
// INPUT   : row  - row position (0..x).
//           col  - col position (0..y).
//           data - the byte data.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UTIL_DispHexByte( UCHAR row, UCHAR col, UCHAR data )
{
UCHAR	c_hi, c_lo;
UCHAR	sbuf[3], dbuf[3];

	UTIL_hexb2ascw( data, &c_hi, &c_lo );
	
	sbuf[0]=row;
	sbuf[1]=col;
	sbuf[2]=FONT0;
	
	dbuf[0]=2;
	dbuf[1]=c_hi;
	dbuf[2]=c_lo;
	api_lcd_putstring(util_dhn_lcd, sbuf, dbuf);
}

// ---------------------------------------------------------------------------
// FUNCTION: Dispaly a hex value of a word data.
// INPUT   : row  - row position (0..x).
//           col  - col position (0..y).
//           data - the word data.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UTIL_DispHexWord( UCHAR row, UCHAR col, UINT data )
{
	UTIL_DispHexByte( row, col,   (UCHAR) ((data & 0x0ff00) >> 8) );
	UTIL_DispHexByte( row, col+2, (UCHAR) (data & 0x00ff) );
}

// ---------------------------------------------------------------------------
// FUNCTION: Dump a number of hex bytes to the display. (separated by space)
//	     It will wait for key press if more than one screen data to be shown.
//           format:
//           01234567890123456789
//           xx xx xx xx xx xx xx
// INPUT   : mode - 0x00=binary, 0x01=text for DISPLAY.
//		    0x80=binary, 0x81=text for COM port.
//         : row  - the beginning row number.
//           len  - length of data.
//           data - the hex data.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UTIL_DumpHexData( UCHAR mode, UCHAR row, UINT length, UCHAR *data )
{
UINT	i;
UCHAR	col;
UCHAR	st_row;
UCHAR	c_hi, c_lo;
UCHAR	buf[3];


	if( mode & 0x80 )
	  {
	  if( length == 0 )
	    return;
	  
	  st_row = 0;
	  col = 0;
	  
	  for( i=0; i<length; i++ )
	     {
	     if( mode == 0x80 )
	       {
	       UTIL_hexb2ascw( data[i], &c_hi, &c_lo );
	       buf[0] = c_hi;
	       buf[1] = c_lo;
	       buf[3] = 0;
//	       _DEBUGPRINTF( "%s", buf );
//	       _DEBUGPRINTF( "%c", 0x20 );
	       }
	     else
	       {
//	       _DEBUGPRINTF( "%c", data[i] );
//	       _DEBUGPRINTF( "%c", 0x20 );
	       }
	     
	     col += 3;
	     if(col >= 48)	// 16 bytes per line
	       {
//	       _DEBUGPRINTF( "\r\n" );
	       
	       col = 0;
	       st_row++;
	       if(st_row >=8)
	         {
	         st_row = 0;
	         
//	         _DEBUGPRINTF( "       more...\r\n" );
	         UTIL_WaitKey();
	         }
	       }
	     }
	  
//	  _DEBUGPRINTF( "\r\n\r\n" );
	  }
	else
	  {
	  if( (length == 0) || (row > 7) )
	    return;
	  
	  st_row = row;
	  col=0;
	  
	  UTIL_ClearRow( st_row, 8-st_row, FONT0 );
	  
	  for(i=0; i<length; i++)
	     {
	     if( mode == 0 )
	       UTIL_DispHexByte( row, col, data[i] );
	     else
	       {
	       UTIL_PutChar( row, col, FONT0, data[i]);
	       UTIL_PutChar( row, col+1, FONT0, 0x20);
	       }
	  
	     col += 3;
	     if(col >= 19)
	       {
	       col = 0;
	       row++;
	       if(row >=8)
	         {
	         row = st_row;
	         UTIL_WaitKey();
	         UTIL_ClearRow( st_row, 8-st_row, FONT0 );
	         }
	       }
	     }
	  
	  UTIL_WaitKey();
	  }
}

// ---------------------------------------------------------------------------
// FUNCTION: Dump a number of hex byte value to display. (continuous)
//           format:
//           01234567890123456789
//           xx xx xx xx xx xx xx
// INPUT   : mode - 0x00=binary, 0x01=text for DISPLAY.
//		    0x80=binary, 0x81=text for COM port.
//         : row  - the beginning row number.
//           len  - length of data.
//           data - the hex data.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UTIL_DumpHexData2( UCHAR mode, UCHAR row, UINT length, UCHAR *data )
{
UINT	i;
UCHAR	col;
UCHAR	st_row;
UCHAR	maxcol;
UCHAR	c_hi, c_lo;
UCHAR	buf[3];


	if( mode & 0x80 )
	  {
	  if( length == 0 )
	    return;
	  
	  st_row = 0;
	  col = 0;
	  
	  for( i=0; i<length; i++ )
	     {
	     if( mode == 0x80 )
	       {
	       UTIL_hexb2ascw( data[i], &c_hi, &c_lo );
	       buf[0] = c_hi;
	       buf[1] = c_lo;
	       buf[3] = 0;
//	       _DEBUGPRINTF( "%s", buf );
	       
	       col += 2;
	       maxcol = 47;
	       }
	     else
	       {
//	       _DEBUGPRINTF( "%c", data[i] );
	       
	       col += 1;
	       maxcol = 48;
	       }
	     
	     if(col >= maxcol)	// 16 bytes per line
	       {
//	       _DEBUGPRINTF( "\r\n" );
	       
	       col = 0;
	       st_row++;
	       if(st_row >=8)
	         {
	         st_row = 0;
	         
//	         _DEBUGPRINTF( "       more...\r\n" );
	         UTIL_WaitKey();
	         }
	       }
	     }
	  
//	  _DEBUGPRINTF( "\r\n\r\n" );
	  }
	else
	  {
	  if( (length == 0) || (row > 7) )
	    return;
	  
	  st_row = row;
	  col=0;
	  
	  UTIL_ClearRow( st_row, 8-st_row, FONT0 );
	  
	  for(i=0; i<length; i++)
	     {
	     if( mode == 0 )
	       {
	       UTIL_DispHexByte( row, col, data[i] );
	       
	       col += 2;
	       maxcol = 20;
	       }
	     else
	       {
	       UTIL_PutChar( row, col, FONT0, data[i]);
	  
	       col += 1;
	       maxcol = 21;
	       }
	  
	     if(col >= maxcol)
	       {
	       col = 0;
	       row++;
	       if(row >=8)
	         {
	         row = st_row;
	         UTIL_WaitKey();
	         UTIL_ClearRow( st_row, 8-st_row, FONT0 );
	         }
	       }
	     }
	  
	  UTIL_WaitKey();
	  }
}

// ---------------------------------------------------------------------------
// FUNCTION: Wait for N*10ms.
// INPUT   : ten_ms - unit in step of ten mini-second. (1 sec = 100 unit)
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UTIL_WaitTime( UINT tenms )
{
UCHAR	dhn_tim;
UINT	tick1, tick2;

	dhn_tim = api_tim_open(1); // 1 tick unit = 10 ms
	api_tim_gettick( dhn_tim, (UCHAR *)&tick1 );
	
	do{
	  api_tim_gettick( dhn_tim, (UCHAR *)&tick2 );
	  } while( tick2 - tick1 < tenms );
	
	api_tim_close( dhn_tim );
}

// ---------------------------------------------------------------------------
// FUNCTION: Wait for N*10ms or exit if any key is pressed.
// INPUT   : ten_ms - unit in step of ten mini-second. (1 sec = 100 unit)
// OUTPUT  : none.
// RETURN  : 0xFF   = time is up without any key pressed.
//           others = the key code pressed within timeout.
// ---------------------------------------------------------------------------
UCHAR	UTIL_WaitTimeAndKey( UINT tenms )
{
UCHAR	dhn_tim;
UINT 	tick1, tick2;
UCHAR	buf[1];

	dhn_tim = api_tim_open(1); // 1 tick unit = 10 ms
	api_tim_gettick( dhn_tim, (UCHAR *)&tick1 );
	
	do{
	  api_tim_gettick( dhn_tim, (UCHAR *)&tick2 );
	
	  if( api_kbd_status( util_dhn_kbd, buf) == apiReady )
	    {
	    api_tim_close( dhn_tim );
	    return( UTIL_WaitKey() );
	    }
	
	  } while( tick2 - tick1 < tenms );
	
	api_tim_close( dhn_tim );
	return( 0xFF );
}

// ---------------------------------------------------------------------------
// FUNCTION: Open printer.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#ifdef	_MOUNT_PRINTER_
void	UTIL_OpenPrinter(void)
{
	util_dhn_prt = api_prt_open( prtThermal, 0 );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Close printer.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#ifdef	_MOUNT_PRINTER_
void	UTIL_ClosePrinter(void)
{
	api_prt_close( util_dhn_prt );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Print one string with default ASCII FONT0.
// INPUT   : FormFeed     - TRUE if the last character of the string is a form-feed (0x0C).
//	     str          - the string to be printed, format: LEN(1)+STR(n)
// OUTPUT  : status       - final status of printer.
// RETURN  : TRUE  - OK
//	     FALSE - exception.
// ---------------------------------------------------------------------------
#ifdef	_MOUNT_PRINTER_
UCHAR	UTIL_PrintString( UCHAR FormFeed, UCHAR FontID, UCHAR *str, UCHAR *status )
{
UCHAR	result;


	do{
	  result = api_prt_putstring( util_dhn_prt, FontID, str );
	  if( result == apiNotReady )
	    {
	    api_prt_status( util_dhn_prt, status );
	    if( (*status == prtPaperEmpty) || (*status == prtBufferFull) )
	      break;	// exception occurs
	    }
	  } while( result != apiOK );	// waiting for the end of process
	       
	if( result != apiOK )
	  return( FALSE );
	else
	  {
	  if( FormFeed )
	    {
	    do{
	      // additional checking for "prtComplete" if form feed character is there.
	      api_prt_status( util_dhn_prt, status );
	      } while( (*status != prtComplete) && (*status != prtPaperEmpty) && (*status != prtBufferFull) );
	    }
	  
	  return( TRUE );
	  }
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Print one graphic bitmap.
// INPUT   : dim          - dimension of the bitmap.
//	     bmp	  - the bitmap to be printed.
// OUTPUT  : status       - final status of printer.
// RETURN  : TRUE  - OK
//	     FALSE - exception.
// ---------------------------------------------------------------------------
#ifdef	_MOUNT_PRINTER_
UCHAR	UTIL_PrintGraphics( API_GRAPH dim, UCHAR *bmp,  UCHAR *status )
{
UCHAR	result;


	do{
	  result = api_prt_putgraphics( util_dhn_prt, dim, bmp );
	  if( result == apiNotReady )
	    {
	    api_prt_status( util_dhn_prt, status );
	    if( (*status == prtPaperEmpty) || (*status == prtBufferFull) )
	      break;
	    }
	  } while( result != apiOK );	// waiting for the end of process
	       
	if( result == apiOK )
	  return( TRUE );
	else
	  return( FALSE );
}

#endif

// ---------------------------------------------------------------------------
// FUNCTION: Get keyin number for item selection.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : -1 if "CANCEL" key is pressed.
//	     other values for the keyin number.
// ---------------------------------------------------------------------------
int	UTIL_GetKeyInNumber( UCHAR row, UCHAR font )
{
int	result;
UCHAR	FirstEntry = TRUE;
UCHAR	keyin;
UCHAR	keybuf[8];
UCHAR	index;


	memset( keybuf, 0x00, sizeof(keybuf) );
	index = 0;
	result = -1;
	
	while(1)
	     {
	     keyin = UTIL_WaitKey();
	     if( FirstEntry )
	       {
	       FirstEntry = FALSE;
	       UTIL_ClearRow( row, 1, font );
	       }
	     
	     if( keyin == 'x' )	// CANCEL
	       return(-1);
	     
	     if( keyin == 'y' )	// ENTER
	       break;

	     if( keyin == 'n' )	// CLEAR
	       {
	       UTIL_ClearRow( row, 1, font );
	       memset( keybuf, 0x00, sizeof(keybuf) );
	       index = 0;
	       
	       continue;
	       }
	     
	     if( index >= 2 )	// check max digits
	       continue;
	     
	     if( (keyin >= '0') && (keyin <= '9') )
	       {
	       UTIL_PutChar( row, index, font, keyin );
	       keybuf[index++] = keyin;
	       }
	     }
	
	if( index != 0 )
	  {
	  // convert ASCII string to an integer
	  keybuf[index] = '\0';
	  result = atoi( (char *)keybuf );
	  }
	else
	  result = 0;
	
	return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: Get keyin string.
// INPUT   : row
//	     font
// OUTPUT  : str - string.
// RETURN  : -1 if "CANCEL" key is pressed.
//	     other values for the string length.
// ---------------------------------------------------------------------------
int	UTIL_GetKeyInString( UCHAR row, UCHAR col, UCHAR font, UCHAR *str )
{
int	result;
UCHAR	FirstEntry = TRUE;
UCHAR	keyin;
UCHAR	keybuf[32];
UCHAR	index;
UCHAR	rr;
UCHAR	cc;


	rr = row;
	cc = col;

	memset( keybuf, 0x00, sizeof(keybuf) );
	index = 0;
	result = -1;

	while(1)
	     {
	     keyin = UTIL_WaitKey();
	     if( FirstEntry )
	       {
	       FirstEntry = FALSE;
	       UTIL_ClearRow( row, 1, font );
	       }
	     
	     if( keyin == 'x' )	// CANCEL
	       return(-1);
	     
	     if( keyin == 'y' )	// ENTER
	       break;

	     if( keyin == 'n' )	// CLEAR
	       {
	       UTIL_ClearRow( row, 1, font );
	       memset( keybuf, 0x00, sizeof(keybuf) );
	       index = 0;
	       
	       cc = col;
	       
	       continue;
	       }
	     
	     if( index >= sizeof(keybuf) )	// check max digits
	       continue;
	     
	     if( (keyin >= '0') && (keyin <= '9') )
	       {
	       UTIL_PutChar( row, cc++, font, keyin );
	       keybuf[index++] = keyin;
	       }
	     }
	
	if( index != 0 )
	  {
	  result = index;
	  memmove( str, keybuf, index );
	  }
	else
	  result = 0;
	
	return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: convert integer to ASCII string.
// INPUT   : value - the integer.
// OUTPUT  : abuf  - the ASCII string.
// RETURN  : the length of the output string in bytes.
// ---------------------------------------------------------------------------
UCHAR	UTIL_itoa( UINT value, UCHAR *abuf )
{
UCHAR	rem;
UCHAR	i,len;
UINT	quo;
UCHAR	count;

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
