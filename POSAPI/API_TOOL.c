//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : 							    **
//**  PRODUCT  : AS350 Plus						    **
//**                                                                        **
//**  FILE     : API_TOOL.C                                                 **
//**  MODULE   : api_tl_itoa()				                    **
//**									    **
//**  FUNCTION : API::TOOL (software Tools Module)			    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2022/12/01                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2022 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include "POSAPI.h"

// ---------------------------------------------------------------------------
// FUNCTION: convert integer(UINT16) to ASCII string.
// INPUT   : value - the integer.
// OUTPUT  : abuf  - the ASCII string.
// RETURN  : the length of the output string in bytes.
// ---------------------------------------------------------------------------
UCHAR api_tl_itoa( UINT value, UCHAR *abuf )
{
UCHAR rem;
UCHAR i,len;
UINT  quo;
UCHAR count;

      i=0;
      len=0;

      if( value < 10 )
        {
        abuf[0] = value | '0';
        return(1);
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

      return(len);
}

// ---------------------------------------------------------------------------
// FUNCTION: convert long to ASCII string.
// INPUT   : value - the long data.
// OUTPUT  : abuf  - the ASCII string.
// RETURN  : the length of the output string in bytes.
// ---------------------------------------------------------------------------
UCHAR	api_tl_ltoa( ULONG value, UCHAR *abuf )
{
UCHAR	rem;
UCHAR	i,len;
ULONG	quo;
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

// ---------------------------------------------------------------------------
// FUNCTION: convert numerical word to DECINAL byte.
// INPUT   : buf - the numerical word. (eg. "12" -> 0x0C)
// OUTPUT  : none.
// RETURN  : the DECINAL value.
// ---------------------------------------------------------------------------
ULONG api_tl_numw2decb( UCHAR *buf )
{
ULONG	value;
	
	value = (buf[0] & 0x0F)*10 + (buf[1] & 0x0F);
	return( value );
}

// ---------------------------------------------------------------------------
// FUNCTION: convert ASCII word to HEX byte.
// INPUT   : buf - the ascii word. (eg. "1A" -> 0x1a)
// OUTPUT  : none.
// RETURN  : the HEX value.
// ---------------------------------------------------------------------------
UCHAR api_tl_ascw2hexb( UCHAR *buf )
{
UCHAR result;
UCHAR hi_byte;
UCHAR lo_byte;

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
// FUNCTION: convert HEX byte to ASCII word.
// INPUT   : data - hex data byte. (0x1A)
// OUTPUT    byte_h - the high byte of ASCII. (0x31)
//           byte_l - the low  byte of ASCII. (0x41)
// RETURN  : none.
// ---------------------------------------------------------------------------
void api_tl_hexb2ascw( UCHAR data, UCHAR *byte_h, UCHAR *byte_l )
{
UCHAR cc, c_hi, c_lo;

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
void api_tl_disphexbyte( UCHAR row, UCHAR col, UCHAR data )
{
UCHAR cc,c_hi,c_lo;
UCHAR sbuf[3], dbuf[3];

      api_tl_hexb2ascw( data, &c_hi, &c_lo );

      sbuf[0]=row;
      sbuf[1]=col;
      sbuf[2]=FONT0;

      dbuf[0]=2;
      dbuf[1]=c_hi;
      dbuf[2]=c_lo;
      api_lcd_putstring(0, sbuf, dbuf);
}

// ---------------------------------------------------------------------------
// FUNCTION: Dispaly a hex value of a word data.
// INPUT   : row  - row position (0..x).
//           col  - col position (0..y).
//           data - the word data.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void api_tl_disphexword( UCHAR row, UCHAR col, UINT data )
{
      api_tl_disphexbyte( row, col,   (UCHAR) ((data & 0x0ff00) >> 8) );
      api_tl_disphexbyte( row, col+2, (UCHAR) (data & 0x00ff) );
}
