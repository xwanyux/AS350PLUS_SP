
//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : TOOL.C                                                     **
//**  MODULE   : Common tools.                                              **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2002/12/04                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2002-2007 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//HISTORY: 2008-dec-25 charles
//				1 modify TL_CheckDate()
//				2 add TL_bcd2dec()
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include <string.h>

#include "POSAPI.h"
#include "EMVDC.h"
#include "EMVAPI.h"
#include "GDATAEX.h"
#include "UI.h"
#include "EMVCONST.H"
// #ifdef _EMVDCMSG_H_
#include "EMVDCMSG.h"
// #endif
// ---------------------------------------------------------------------------
// FUNCTION: Put ISSUER Public Key modulus to memory.
// INPUT   : buf - 2L-V, to store issuer PKC.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
//void TL_Debug_PutIssuerPKM( UCHAR *buf )
//{
//      buf[0] = 112;
//      buf[1] = 0;
//
//      memmove( &buf[2], ISSUER_Public_Key, 112 );
//}

// ---------------------------------------------------------------------------
// FUNCTION: Put ISSUER Public Key Certificate to memory.
// INPUT   : buf - 2L-V, to store issuer PKC.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
//void TL_Debug_PutIssuerPKC( UCHAR *buf )
//{
//      buf[0] = 112;
//      buf[1] = 0;
//
//      memmove( &buf[2], ISSUER_Public_Key_Certificate, 112 );
//}

// ---------------------------------------------------------------------------
// FUNCTION: Put ISSUER Public Key Remainder to memory.
// INPUT   : buf - 2L-V, to store issuer PKR.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
//void TL_Debug_PutIssuerPKR( UCHAR *buf )
//{
//      buf[0] = 20;
//      buf[1] = 0;
//
//      memmove( &buf[2], ISSUER_Public_Key_Remainder, 20 );
//}

// ---------------------------------------------------------------------------
// FUNCTION: Put ISSUER Public Key Exponent to memory.
// INPUT   : buf - 2L-V, to store issuer PKE.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
//void TL_Debug_PutIssuerPKE( UCHAR *buf )
//{
//      buf[0] = 1;
//      buf[1] = 0;
//      buf[2] = 1; // exponent=2, 3, or 65537
//}

// ---------------------------------------------------------------------------
// FUNCTION: Put the Static Data To Be Authenticated.
// INPUT   : buf - 2L-V.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
//void TL_Debug_PutSDTBA( UCHAR *buf )
//{
//      buf[0] = 90+75;
//      buf[1] = 0;
//
//      memmove( &buf[2], ISSUER_StaticDataToBeAuthen, 90+75 );
//}

// ---------------------------------------------------------------------------
// FUNCTION: Put ISSUER Public Key Exponent to memory.
// INPUT   : buf - 2L-V, to store issuer PKE.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
//void TL_Debug_PutSSAD( UCHAR *buf )
//{
//UINT  i;
//
//      buf[0] = 112;
//      buf[1] = 0;
//
//      for( i=0; i< 112; i++ )
//         buf[i+2] = i;
//}

// ---------------------------------------------------------------------------
// FUNCTION: convert HEX byte to ASCII word.
// INPUT   : data - hex data byte. (0x1A)
// OUTPUT    byte_h - the high byte of ASCII. (0x31)
//           byte_l - the low  byte of ASCII. (0x41)
// RETURN  : none.
// ---------------------------------------------------------------------------
void TL_hexb2ascw( UCHAR data, UCHAR *byte_h, UCHAR *byte_l )
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
void TL_DispHexByte( UCHAR row, UCHAR col, UCHAR data )
{
UCHAR cc,c_hi,c_lo;
UCHAR sbuf[3], dbuf[3];

//    cc = data;
//    data &= 0x0f;
//    if (data <= 9)
//       data += '0';
//    else
//       {
//       data -= 10;
//       data += 'A';
//       }
//    c_lo=data;
//
//    data = cc;
//    data &= 0x0f0;
//    data >>=  4;
//    if (data <= 9)
//       data += '0';
//    else
//       {
//       data -= 10;
//       data += 'A';
//       }
//    c_hi=data;

      TL_hexb2ascw( data, &c_hi, &c_lo );

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
void TL_DispHexWord( UCHAR row, UCHAR col, UINT data )
{
      TL_DispHexByte( row, col,   (UCHAR) ((data & 0x0ff00) >> 8) );
      TL_DispHexByte( row, col+2, (UCHAR) (data & 0x00ff) );
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
void TL_DumpHexData( UCHAR mode, UCHAR row, UINT length, UCHAR *data )
{
UINT  i;
UCHAR col;
UCHAR st_row;

      if( (length == 0) || (row > 7) )
        return;

      st_row = row;
      col=0;

      UI_ClearRow( st_row, 8-st_row, FONT0 );

      for(i=0; i<length; i++)
         {
         if( mode == 0 )
           TL_DispHexByte( row, col, data[i] );
         else
           {
           UI_PutChar( row, col, FONT0, data[i]);
           UI_PutChar( row, col+1, FONT0, 0x20);
           }

         col += 3;
         if(col >= 19)
           {
           col = 0;
           row++;
           if(row >=8)
             {
             row = st_row;
             UI_WaitKey();
             UI_ClearRow( st_row, 8-st_row, FONT0 );
             }
           }
         }

      UI_WaitKey();
}

// ---------------------------------------------------------------------------
// FUNCTION: Dump a number of hex byte value to display. (ROW 0..6)
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
void TL_DumpHexData2( UCHAR mode, UCHAR row, UINT length, UCHAR *data )
{
UINT  i;
UCHAR col;
UCHAR st_row;

      if( (length == 0) || (row > 6) )
        return;

      st_row = row;
      col=0;

      UI_ClearRow( st_row, 7-st_row, FONT0 );

      for(i=0; i<length; i++)
         {
         if( mode == 0 )
           TL_DispHexByte( row, col, data[i] );
         else
           {
           UI_PutChar( row, col, FONT0, data[i]);
           UI_PutChar( row, col+1, FONT0, 0x20);
           }

         col += 3;
         if(col >= 19)
           {
           col = 0;
           row++;
           if(row >=7)
             {
             row = st_row;
             UI_WaitKey();
             UI_ClearRow( st_row, 7-st_row, FONT0 );
             }
           }
         }

      UI_WaitKey();
}
// ---------------------------------------------------------------------------
// FUNCTION: convert BCD data to DEC data.
// INPUT   : bcdlen - size of bcd data in bytes. (max. 5)
//           bcd    - the bcd data.          (eg,"46 60" ) 46 in bcd[0] 60 in bcd[1]
// OUTPUT    none.
// RETURN  : unsigned long number
//2008-dec-25 charles
// ---------------------------------------------------------------------------
ULONG TL_bcd2Dec( UCHAR bcdlen, UCHAR *bcd )
{
	UCHAR i;
	UCHAR data;
	UCHAR data_h;
	UCHAR data_l;
	ULONG power = 1;
	ULONG result = 0;
	UCHAR *ptrdata;
    if( (bcdlen == 0) || (bcdlen > 6) )
    	return 0; // out of range
    for(i=bcdlen;i>0;i--)
    {
    	result += (bcd[i-1]&0xf)*power;
    	result += ((bcd[i-1]>>4)&0xf)*power*10;
    	power*=100;
    }
//    ptrdata = (UCHAR *)&result;
//    for( i=0; i<4; i++ )
//        dec[i] = *(ptrdata+3-i);
    return result;
}
// ---------------------------------------------------------------------------
// FUNCTION: convert BCD data to HEX data.
// INPUT   : bcdlen - size of bcd data in bytes. (max. 5)
//           bcd    - the bcd data.          (eg,"46 60" )
// OUTPUT    hex    - fixed 4-byte hex data. (eg,"00 00 12 34" = 0x1234)
// RETURN  : none.
// ---------------------------------------------------------------------------
void TL_bcd2hex( UCHAR bcdlen, UCHAR *bcd, UCHAR *hex )
{
UCHAR i;
UCHAR data;
UCHAR data_h;
UCHAR data_l;
ULONG power[10];
ULONG result;
UCHAR *ptrdata;

      if( (bcdlen == 0) || (bcdlen > 6) )
        return; // out of range

      power[9] = 1000000000;
      power[8] = 100000000;
      power[7] = 10000000;
      power[6] = 1000000;
      power[5] = 100000;
      power[4] = 10000;
      power[3] = 1000;
      power[2] = 100;
      power[1] = 10;
      power[0] = 1;

      // convert to a long type data
      result = 0;
      for( i=0; i<bcdlen; i++ )
         {
         data = *bcd++;
         data_l = data & 0x0f;
         result += power[(bcdlen-1-i)*2] * data_l;
         data_h = ( data & 0xf0 ) >> 4;
         result += power[(bcdlen-1-i)*2+1] * data_h;
         }

      // convert to "b4" format
      ptrdata = (UCHAR *)&result;
      for( i=0; i<4; i++ )
         hex[i] = *(ptrdata+3-i);
}

// ---------------------------------------------------------------------------
// FUNCTION: convert ASCII word to BCD byte.
// INPUT   : buf - the ascii word.
// OUTPUT  : none.
// RETURN  : the BCD value.
// ---------------------------------------------------------------------------
UCHAR TL_ascw2bcdb( UCHAR buf[] )
{
      return( ((buf[0] & 0x0f) << 4) + (buf[1] & 0x0f) );
}

// ---------------------------------------------------------------------------
// FUNCTION: convert ASCII word to HEX byte.
// INPUT   : buf - the ascii word. (eg. "1A" -> 0x1a)
// OUTPUT  : none.
// RETURN  : the HEX value.
// ---------------------------------------------------------------------------
UCHAR TL_ascw2hexb( UCHAR buf[] )
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
// FUNCTION: convert ASCII string to BCD data.
// INPUT   : str    - L-V, the ascii string.
//           bcdlen - size of bcd buffer.
// OUTPUT  : bcd - right-justified bcd data with leading zeros.
// RETURN  : the BCD value.
// ---------------------------------------------------------------------------
void TL_asc2bcd( UCHAR bcdlen, UCHAR *bcd, UCHAR *str )
{
UCHAR i;
UCHAR len;
UCHAR cnt;
UCHAR remainder;
UCHAR buf[2];
UCHAR bcdidx;

      // clear to zeros
      for( i=0; i<bcdlen; i++ )
         bcd[i] = 0x00;

      bcdidx = bcdlen - 1;

      len = *str;

      cnt = len/2;
      remainder = len - cnt*2;

      for( i=0; i<cnt; i++ )
         {
         buf[1] = *(str+len);
         buf[0] = *(str+len-1);
         bcd[bcdidx--] = TL_ascw2bcdb( buf );

         len -= 2;
         }

      if( remainder != 0 )
        {
        buf[1] = *(str+len);
        buf[0] = '0';
        bcd[bcdidx] = TL_ascw2bcdb( buf );
        }
}

// ---------------------------------------------------------------------------
// FUNCTION: convert HEX data to ASCII string. (0x1A2B -> "1A2B")
// INPUT   : hexlen - size of hex buffer. (in bytes)
//           hex    - hex data.
// OUTPUT  : str    - L-V, the ascii string.
// RETURN  : the BCD value.
// ---------------------------------------------------------------------------
void TL_hex2asc( UCHAR hexlen, UCHAR *hex, UCHAR *str)
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
         TL_hexb2ascw( *hex++, &hi_byte, &lo_byte );
         str[j++] = hi_byte;
         str[j++] = lo_byte;
         }

      str[0] = j-1; // actual string length
}

// ---------------------------------------------------------------------------
// FUNCTION: convert BCD data to ASCII string.
// INPUT   : bcdlen - size of bcd buffer. (in bytes)
//           bcd    - right-justified bcd data with leading zeros.
// OUTPUT  : str    - L-V, the ascii string.
// RETURN  : the BCD value.
// ---------------------------------------------------------------------------
void TL_bcd2asc( UCHAR bcdlen, UCHAR *bcd, UCHAR *str )
{
UCHAR i, j;
UCHAR lo_byte;
UCHAR hi_byte;

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
// FUNCTION: convert integer to ASCII string.
// INPUT   : value - the integer.
// OUTPUT  : abuf  - the ASCII string.
// RETURN  : the length of the output string in bytes.
// ---------------------------------------------------------------------------
UCHAR TL_itoa( UINT value, UCHAR *abuf )
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
// FUNCTION: Dump a number of hex byte value to the specified buffer.
//           format:
//           01234567890123456789
//           xx xx xx xx xx xx xx
// INPUT   : length - length of hex data.
//           data   - the hex data;
// OUTPUT  : str    - the converted string.
// RETURN  : none.
// ---------------------------------------------------------------------------
void TL_PrintHexData( UCHAR length, UCHAR *data, UCHAR *str )
{
UCHAR i;
UCHAR hi_byte;
UCHAR lo_byte;

      for( i=0; i<length; i++ )
         {
         TL_hexb2ascw( data[i], &hi_byte, &lo_byte );
         *str++ = hi_byte;
         *str++ = lo_byte;
         *str++ = 0x20;
         }
}

// ---------------------------------------------------------------------------
// FUNCTION: swap the position of data (MSB to LSB)
// INPUT   : length - length of data bytes.
//           data   - the data.
// OUTPUT  : data   - the swapped data.
// RETURN  : none.
// ---------------------------------------------------------------------------
void TL_SwapData( UCHAR length, UCHAR *data )
{
UCHAR i, j;
UCHAR cnt;
UCHAR temp;

      if( length < 2 )
        return;

      cnt = length/2;

      i = 0;
      j = length-1;
      do{
        temp = data[i];
        data[i] = data[j];
        data[j] = temp;

        i++;
        j--;
        cnt--;
        } while( cnt != 0 );
}

// ---------------------------------------------------------------------------
// FUNCTION: bcd1 = bcd1 + bcd2. ("n" format, "00 01 23" = 123)
// INPUT   : bcdlen - length of bcd in bytes. (both must be the same length)
//           bcd1 = 1'st bcd.
//           bcd2 = 2'nd bcd.
// OUTPUT  : bcd1 = the addition of the two bcds in bcd format.
// RETURN  : TRUE  - carry occurred.
//           FLASE - no carry occurred.
// NOTE    : 99 + 01 = 00 (carry occurred)
// ---------------------------------------------------------------------------
UCHAR TL_bcd_add_bcd( UCHAR bcdlen, UCHAR *bcd1, UCHAR *bcd2 )
{
UCHAR i;
UCHAR digit1_h, digit1_l;
UCHAR digit2_h, digit2_l;
UCHAR carry;

      TL_SwapData( bcdlen, bcd1 ); // convert to LSB-MSB hex format
      TL_SwapData( bcdlen, bcd2 ); //

      carry = 0;
      for( i=0; i<bcdlen; i++ )
         {
         digit1_h = (bcd1[i] & 0xf0) >> 4;
         digit1_l = bcd1[i] & 0x0f;

         digit2_h = (bcd2[i] & 0xf0) >> 4;
         digit2_l = bcd2[i] & 0x0f;

         digit1_l += (digit2_l + carry);
         if( digit1_l > 9 )
           {
//         digit1_l = 0;
	   digit1_l -= 10;	// PATCH: 2013-06-25
           digit1_h += 1;
           }

         digit1_h += digit2_h;
         if( digit1_h > 9 )
           {
//         digit1_h = 0;
	   digit1_h -= 10;	// PATCH: 2013-06-25
           carry = 1; // carry for next higher digit
           }
         else
           carry = 0;

         bcd1[i] = (digit1_h << 4) + digit1_l;
         }

      TL_SwapData( bcdlen, bcd1 ); // return to "n" format

      if( carry == 1 )
        return( TRUE );
      else
        return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: trim decimal point.
// INPUT   : exp    - the currency expoent.
//           dec    - the decimal point flag. (1=to show decimal point)
//           buf    - L-V, the ascii data.
// OUTPUT  : buf    - L-V, the new ascii data.
// RETURN  : none.
// ---------------------------------------------------------------------------
void TL_trim_decimal( UCHAR exp, UCHAR dec, UCHAR *buf )
{
     if( (exp != 0) && (dec == 0) ) // imply decimal but show only integer part
       {
       if( buf[0] >= exp ) // PATCH: 2006-12-26, fixup for amount = '0'
         buf[0] -= exp; // ignore decimal part
       }
}

// ---------------------------------------------------------------------------
// FUNCTION: trim ascii data.
// INPUT   : old    - L-V, the original data.
//           method - 0=trim leading zero.
//                    1=trim trailing zero.
// OUTPUT  : new - L-V, the trimed data.
// RETURN  : the BCD value.
// ---------------------------------------------------------------------------
//
void TL_trim_asc( UCHAR method, UCHAR *old, UCHAR *new )
{
UCHAR len;

      len = *old++;

      while( (*old == '0') && (len != 0) )
           {
           old++;
           len--;
           }

      if( *old == '.' ) // decimal point (0.xx)
        {
        old--;
        *old = '0';
        len++;
        }

      if( len == 0 )
        {
        *new++ = 1;
        *new = '0'; // only 1 zero left
        }
      else
        {
        *new++ = len;
        memmove( new, old, len );
        }

//    *new++ = len;
//    if( len != 0 )
//      memmove( new, old, len );
}

// ---------------------------------------------------------------------------
// FUNCTION: show digit string with right-justified. (max 20 digits)
// INPUT   : type - bit 7 6 5 4 3 2 1 0
//                              | | | |_ pure digits (NNNNNN)
//                              | | |___ with sperator (NN,NNN)
//                              | |_____ with decimal point (NNN.NN) cf. currency exponent.
//                              |_______ special prompt (eg. ****) cf.
//           buf  - L-V, the char string.
// OUTPUT  : buf  - L-V, with the decimal point if necessary.
// REF     : g_term_tx_exp, g_term_decimal_point.
// RETURN  : the transaction exponent value (0,2 or 3)
// ---------------------------------------------------------------------------
UCHAR TL_insert_decimal_point( UCHAR type, UCHAR *buf )
{
UCHAR i;
UCHAR exp;
UCHAR data[30];
UCHAR len1;
UCHAR len2;

      // get tranaction currency exponent
//    api_emv_GetDataElement( DE_TERM, ADDR_TERM_TX_CE, 1, exp );
      if( g_term_decimal_point ) // show decimal point?
        exp = g_term_tx_exp;
      else
        exp = 0;

      // with decimal attribute & exponent ? (the exponent will be 2 or 3)
      if( (type & 0x04) && (exp != 0) )
        {
        len1 = buf[0];

        if( len1 == 0 )
          {
          // set to default empty value '0'
          len1 = 1;
          buf[0] = 1;
          buf[1] = '0';
          }

        // arrange decimal point
        // case 1: the char length < exponent (1 -> 0.01)
        // case 2: the char length = exponent (12 -> 0.12)
        // case 3: the char length > exponent (123 -> 1.23)
        if( len1 <= exp )
          {
          if( len1 == exp )
            len2 = 0;
          else
            {
            len2 = exp - len1;
            memset( &data[3], 0x30, len2 );
            }

          data[1] = '0';
          data[2] = '.';
          memmove( &data[3+len2], &buf[1], len1 );
          data[0] = len1 + len2 + 2;
          }
        else
          {
          len2 = len1 - exp; // length of integer part
          memmove( &data[1], &buf[1], len2 ); // set integer part
          data[1+len2] = '.';
          memmove( &data[2+len2], &buf[1+len2], exp ); // set decimal part
          data[0] = len1 + 1;
          }

        memmove( buf, data, data[0]+1 );
        }

//    TL_trim_asc( 0, buf, data );     // to trim the case: 00000.00 to 0.00
//    memmove( buf, data, data[0]+1 );

      return( exp );
}

// ---------------------------------------------------------------------------
// FUNCTION: insert ',' to the money text.
// INPUT   : text - L-V, the text to be inserted. (NNNN or NNNNNN)
//           exp  - decimal point exponent. (0,2,3)
// OUTPUT  : text - L-V, the text with commas inserted. (N,NNN or N,NNN.NN)
// RETURN  : the BCD value.
// ---------------------------------------------------------------------------
void TL_insert_thousand_comma( UCHAR *text, UCHAR exp )
{
UCHAR i, j;
UCHAR len;
UCHAR buf[17]; // L NNN,NNN,NNN,NNN
UCHAR *old;
//UCHAR exp; // decimal exponent value

//    exp = TL_insert_decimal_point( 0x04, text );

      for( i=0; i<16; i++ )
         buf[i+1] = '0';

      buf[4-exp]  = ',';
      buf[8-exp]  = ',';
      buf[12-exp] = ',';

      if( exp != 0 )
        exp++; // including char 'dot'

      len = text[0];
      j=0;
      for( i=0; i<len; i++ )
         {
         if( (j == (3+exp)) || (j == (7+exp)) || (j == (11+exp)) )
           j++;

         if( exp == 0 )
           buf[15-j] = text[len-i];
         else
           buf[16-j] = text[len-i];

         j++;
         }

      if( exp == 0 )
        buf[0] = 15;
      else
        buf[0] = 16;

      old = &buf[0];
      len = *old++;

      while( (*old == '0') || (*old == ',') )
           {
           old++;
           len--;
           }

      if( *old == '.' )
        {
        old--;
        len++;
        *old = '0';
        }

      *text++ = len;
      if( len != 0 )
        memmove( text, old, len );
}

// ---------------------------------------------------------------------------
// FUNCTION: Compares memory with "cn" data type. (padded with 'F')
// INPUT   : s1  - a pointer to the first object.
//           s2  - a pointer to the second object.
//           len - min length of data bytes be compared in the object. (1 byte = 2 digits)
//           eg. 12 34 5F : 12 34 56 , cnt=5 digits, result = equal. (PAN)
// OUTPUT  : none.
// RETURN  : TURE  (equal)
//           FLASE (not equal)
// ---------------------------------------------------------------------------
UCHAR TL_CNcmp( UCHAR *s1, UCHAR *s2, UCHAR len )
{
UCHAR i;
UCHAR buf1[21];
UCHAR buf2[21];

      TL_bcd2asc( len, s1, buf1 );
      TL_bcd2asc( len, s2, buf2 );

      for( i=0; i<buf1[0]; i++ ) // based on the length of "s1"
         {
         if( buf1[i+1] != buf2[i+1] )
           return( FALSE );
         }
      return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: Compares memory with "cn" data type. (padded with 'F')
// INPUT   : s1  - a pointer to the first object.
//           s2  - a pointer to the second object.
//           len - max length of data in bytes to be compared in the object. (1 byte = 2 digits)
//           eg. 12 34 5F : 12 34 56 , 5:6, result = NOT equal. (PAN)
// OUTPUT  : none.
// RETURN  : TURE  (equal) - both length and value are equal.
//           FLASE (not equal)
// ---------------------------------------------------------------------------
UCHAR TL_CNcmp2( UCHAR *s1, UCHAR *s2, UCHAR len )
{
UCHAR i;
UCHAR buf1[21];
UCHAR buf2[21];

      TL_bcd2asc( len, s1, buf1 ); // recovered PAN
      TL_bcd2asc( len, s2, buf2 ); // original  PAN

      if( buf1[0] != buf2[0] ) // check size of digits
        return( FALSE );

      for( i=0; i<buf1[0]; i++ ) // check value of digits
         {
         if( buf1[i+1] != buf2[i+1] )
           return( FALSE );
         }
      return( TRUE );
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
int TL_memcmpc( UCHAR *s1, UCHAR cc, UCHAR len )
{
UCHAR i;

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
// FUNCTION: Compares memory. (MSB-LSB)
// INPUT   : s1  - a pointer to the first object.
//           s2  - a pointer to the second object.
//           len - length of data to be compared in the object.
// OUTPUT  : none.
// RETURN  : >0 - s1 > s2
//           =0 - s1 = s2
//           <0 - s1 < s2
// ---------------------------------------------------------------------------
int TL_memcmp( UCHAR *s1, UCHAR *s2, UCHAR len )
{
UCHAR i;
UCHAR data1, data2;

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

// ---------------------------------------------------------------------------
// FUNCTION: move memmory data form cross-bank access.
// INPUT   : des - the destination address.
//           src - the source address.
//                 it must be resided in the same bank as this function is.
//           len - length of data to be moved.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void TL_memmove( UCHAR *des, UCHAR *src, UCHAR len )
{
    memmove( des, src, len );
}

// ---------------------------------------------------------------------------
// FUNCTION: PUSH an ITEM onto a stack array.
// INPUT   : stk    - the linear stack array.
//           top    - pointer to the available position.
//           maxstk - max number of elements can be held in stack.
//           len    - size of data elements.
//           item   - the elements to be pushed.
// OUTPUT  : none.
// RETURN  : TURE  = OK
//           FLASE = stack overflow
// ---------------------------------------------------------------------------
UCHAR TL_PushData( UCHAR *stk, UCHAR *top, UCHAR maxstk, UCHAR len, UCHAR *item )
{
UCHAR i;
UCHAR *ptrstk;
UCHAR cnt;

      if( *top == maxstk )
        return( FALSE );

      cnt = *top;
      ptrstk = stk + cnt*len;

      for( i=0; i<len; i++ )
         *ptrstk++ = *item++;

      *top = cnt + 1;

      return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: Delete the top element of stack and assign it to a item.
// INPUT   : stk    - the linear stack array.
//           top    - pointer to the available position.
//           maxstk - max number of elements can be held in stack.
//           len    - size of data elements.
// OUTPUT  : item   - the elements to be pushed.
// RETURN  : TURE  = OK
//           FLASE = stack underflow
// ---------------------------------------------------------------------------
UCHAR TL_PopData( UCHAR *stk, UCHAR *top, UCHAR len, UCHAR *item )
{
UCHAR i;
UCHAR *ptrstk;
UCHAR cnt;

      if( *top == 0 )
        return( FALSE );

      cnt = *top;
      ptrstk = stk + cnt*len;

      for( i=0; i<len; i++ )
         *item++ = *(--ptrstk);

      *top = cnt - 1;

      return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: Insertion sort.
// INPUT   : a    - a character array.
//           na   - number of array.
//           mask - bits mask. (0xff=comparing all bits)
// OUTPUT  : a  - the sorted array in accending order.
// RETURN  : none.
// ---------------------------------------------------------------------------
void TL_isort( UCHAR *a, UCHAR na, UCHAR mask )
{
int   i, j;
UCHAR temp1, temp2;

      if( na < 2 )
        return;

      for( i=1; i<na; i++ )
         {
         temp1 = a[i] & mask;
         temp2 = a[i];

         j = i - 1;
         while( (j>=0) && (temp1 < (a[j] & mask)) )
              {
              a[j+1] = a[j];
              j = j - 1;
              }
         a[j+1] = temp2;
         }
}

// ---------------------------------------------------------------------------
// FUNCTION: Display & wait to Select an item from the specified List.
// INPUT   : para - the parameters.
//                  byte 00 = start row number.
//                       01 = end row number.
//                       02 = number of items in list.
//                       03 = length of an item in byte.
//                       04 = offset address of length field in an item.
//           list - the list, format: XXXX LEN[1] ITEM[n] YYYY
//                  where: LEN[1] & ITEM[n] are mandatory, length <= 16.
// OUTPUT  : none.
// RETURN  : -1     - if aborted.
//         : others - item number of the selection. (0..n)
// UI-KEY  : keys allowed - down/up arrow, OK (confirmed), Cancel (aborted).
// UI-DISP : > ITEM_NAME1
//             ITEM_NAME2
// ---------------------------------------------------------------------------
//UCHAR TL_SelectList( UCHAR *para, UCHAR *list )
//{
//UCHAR i;
//UCHAR row_top;
//UCHAR row_btm;
//UCHAR cnt;
//UCHAR len;
//UCHAR ofs;
//UCHAR win_top;
//UCHAR win_btm;
//UCHAR win_size;
//UCHAR item_no;
//UCHAR index;
//UCHAR select;
//
//      row_top = para[0];
//      row_btm = para[1];
//      cnt = para[2];
//      len = para[3];
//      ofs = para[4];
//
//      if( ( cnt == 0 ) || ( cnt == 255 ) )
//        return( -1 );
//
//      // init
//      win_size = row_btm - row_top + 1; // window frame height
//      win_top = 0;            // the 1'st item number in window
////    win_btm = win_size - 1; // the last item number in window
//      select = 0;             // the selected item number
//
//      while(1)
//           {
//           // show the items within the "window" frame
//           UI_ClearRow( row_top, win_size, FONT0 ); // clear frame
//
//           item_no = win_top;
//           cnt = para[2];
//
//           for( i=0; i<win_size; i++ )
//              {
//              if( cnt == 0 )
//                break;
//
//              index = len*item_no + ofs;
//              UI_PutStr( row_top+i, 2, FONT0, list[index], &list[index+1] );
//              item_no++;
//              cnt--;
//              }
//
//           // wait key
//           switch( UI_WaitKey() )
//                 {
//                 case '*': // down arrow
//                      if( win_top < para[2] )
//                        win_top++;
//                      break;
//
//                 case '#': // up arrow
//                      if( win_top != 0 )
//                        win_top--;
//                      break;
//
//                 case 'x': // cancel
//                      return( -1 );
//
//                 case 'y': // OK
//                      return( select );
//                 }
//           } // while(1)
//}

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
UCHAR TL_ListBox( UCHAR start, UCHAR *para, UCHAR *list, UINT tout )
{
UCHAR i;
UCHAR user_select; // user's selection
UCHAR list_start;  // menu start index
UCHAR cursor_pos;  // cursor position
UCHAR list_bottom;
UCHAR key;
UCHAR row;
UCHAR max_row;
UCHAR items;
UCHAR len;
UCHAR ofs;
UCHAR dhn_tim;
UINT  tick1=0, tick2=0;
UCHAR font;

      UI_OpenKeySelect();

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

// if(g_test_flag == 1)
//   {
//   TL_DispHexByte(0,0,list_start);
//   TL_DispHexByte(1,0,list_bottom);
//   TL_DispHexByte(2,0,cursor_pos);
//   TL_DispHexByte(3,0,row);
//   TL_DispHexByte(4,0,items);
//   UI_WaitKey();
//   }

           for( i=row; i<list_bottom; i++ )
              {
              UI_PutChar( i, 0, font, 0x20 ); // clear prefix cursor mark
              UI_PutStr( i, 2, font+attrCLEARWRITE, list[list_start*len+ofs], &list[list_start*len+ofs+1] );
              list_start++;
              }
           UI_PutChar( cursor_pos, 0, font, 0x3e );  // show cursor mark

           if( items > 1 )
             {
             if( user_select == 0 )
            // UI_PutChar( 3, 19, FONT0, 0x94 );    // show down arrow
               UI_PutChar( 3, 20, FONT0, 0x94 );    // show down arrow
             else
               {
               if( user_select == (items - 1) )
             //  UI_PutChar( 3, 19, FONT0, 0x93 );  // show up arrow
                 UI_PutChar( 3, 20, FONT0, 0x93 );  // show up arrow
               else
             //  UI_PutChar( 3, 19, FONT0, 0x92 );  // show up/down arrow
                 UI_PutChar( 3, 20, FONT0, 0x92 );  // show up/down arrow
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
               } while( UI_GetKeyStatus() == apiNotReady );
             }
           key = UI_WaitKey();

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
                      UI_OpenKeyAll();

                      return( user_select );

                 case 'x': // CANCEL
                      api_tim_close( dhn_tim );
                      UI_OpenKeyAll();

                      return( -1 );

                 defaut:
                      goto LB_WAIT_KEY;
                 }

           } //  while(1)
}

// ---------------------------------------------------------------------------
// FUNCTION: Wait for N*10ms.
// INPUT   : ten_ms - unit in step of ten mini-second. (1 sec = 100 unit)
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void TL_WaitTime( UINT tenms )
{
UCHAR dhn_tim;
UINT  tick1=0, tick2=0;

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
// RETURN  : -1     = time is up without any key pressed.
//           others = the key code pressed within timeout.
// ---------------------------------------------------------------------------
UCHAR TL_WaitTimeAndKey( UINT tenms )
{
UCHAR dhn_tim;
UINT  tick1=0, tick2=0;
UCHAR buf[1];

      dhn_tim = api_tim_open(1); // 1 tick unit = 10 ms
      api_tim_gettick( dhn_tim, (UCHAR *)&tick1 );

      do{
        api_tim_gettick( dhn_tim, (UCHAR *)&tick2 );

        if( api_kbd_status( g_dhn_kbd, buf) == apiReady )
          {
          api_tim_close( dhn_tim );
          return( UI_WaitKey() );
          }

        } while( tick2 - tick1 < tenms );

      api_tim_close( dhn_tim );
      return( -1 );
}

// ---------------------------------------------------------------------------
// FUNCTION: get amount.
// INPUT   : none.
// OUTPUT  : amt - the amount (n12).
// REF     :
// RETURN  : TRUE  = confirmed. (by entering OK)
//           FALSE = aborted.   (by entering CANCEL)
// ---------------------------------------------------------------------------
//UCHAR TL_GetAmount( UCHAR *amt )
//{
//UCHAR i;
//char  j;                        // 0123456789ABCDE
//UCHAR keyin_buf[MAX_AMT_CNT];   // NNN,NNN,NNN,NNN
//UCHAR keyin_len;
//UCHAR show_len;
//UCHAR digit_cnt;
//UCHAR keyin;
//UCHAR col_ofs;
////UCHAR currency_sign;
//
//      UI_OpenKeyNum(); // enable numeric keypad
//
//GET_AMT_ST:
//
//      // reset keyin buf
//      keyin_buf[0] = '0';
//      keyin_len = 0;
//      show_len = 1;
////    currency_sign = '$';
//
//      do{
//        digit_cnt = 1;
//        col_ofs = 0;
//        UI_ClearRow( 1, 1, FONT1 );
//
//        show_len = keyin_len;
//        if( show_len == 0 )
//          show_len = 1;
//
//        // show current amt
//        j = MAX_AMT_CNT - 1;
//        for( i=0; i<show_len; i++ )
//           {
//      //   if( (digit_cnt == 3) || (digit_cnt == 6) || (digit_cnt == 9) )
//      //     {
//      //     UI_PutChar( 1, MAX_DSP_FONT1_COL-digit_cnt, FONT1, ',' );
//      //     col_ofs++;
//      //     }
//
//           UI_PutChar( 1, MAX_DSP_FONT1_COL - i - col_ofs, FONT1, keyin_buf[j] );
//           digit_cnt++;
//           j--;
//           }
//
//GET_AMT_WAIT_KEY:
//
//        keyin = UI_WaitKey();
//        switch( keyin )
//              {
//              case 'x': // cancel
//                   return( FALSE );
//
//              case 'n': // clear
//                   goto GET_AMT_ST;
//
//              case 'y': // OK
//                   if( keyin_len == 0 )
//                     goto GET_AMT_ST;
//
//                   // pack keyin buf to BCD format
//
//
//              default:  // '0'~'9'
//                   if( keyin_len < MAX_AMT_CNT )
//                     {
//                     keyin_buf[MAX_AMT_CNT - keyin_len - 1] = keyin;
//                     keyin_len++;
//                     }
//                   else
//                     goto GET_AMT_WAIT_KEY;
//              }
//
//        } while(1);
//}
//
// ---------------------------------------------------------------------------
// FUNCTION: show digit string with right-justified. (max 20 digits)
// INPUT   : row  - row position to show.
//           buf  - L-V, the digit string.
//           type -
//           idle - character to be shown when idle. (eg. '0','-'...)
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void ShowKeyIn( UCHAR type, UCHAR idle, UCHAR font, UCHAR row, UCHAR *buf )
{
UCHAR i;
UCHAR len;
UCHAR index;
UCHAR max_dsp_cnt;
UCHAR newbuf[30];

      memmove( newbuf, buf, buf[0]+1 );

      TL_insert_decimal_point( type, newbuf ); // put decimal point if necessary

      UI_ClearRow( row, 1, font );

      if( (font & 0x0f) == FONT0 )
        max_dsp_cnt = MAX_DSP_FONT0_CNT;
      else
        max_dsp_cnt = MAX_DSP_FONT1_CNT;

      // check non-zero idle prompt
      if( (buf[0] == 0) && (idle != '0') )
        {
        newbuf[1] = idle;
        UI_PutStr( row, max_dsp_cnt-1, font, 1, &newbuf[1] );
        return;
        }

      if( newbuf[0] == 0 ) // no data in buffer, show '0'
        {
        newbuf[1] = idle;
        UI_PutStr( row, max_dsp_cnt-1, font, 1, &newbuf[1] );
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
            UI_PutStr( row, max_dsp_cnt-newbuf[0], font, newbuf[0], &newbuf[1] );
          else
            {
            index = newbuf[0]-max_dsp_cnt + 1;
            UI_PutStr( row, 0, font, max_dsp_cnt, &newbuf[index] );
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
UCHAR TL_GetNumKey( UINT tout, UCHAR type, UCHAR idle, UCHAR font, UCHAR row, UCHAR len, UCHAR *buf )
{
UCHAR i;
UCHAR key;
UCHAR dhn_tim;
UINT  tick1=0, tick2=0;

      UI_OpenKeyNum(); // enable numeric keypad

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
             ShowKeyIn( type, idle, font, row, buf );
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
               } while( UI_GetKeyStatus() == apiNotReady );
             }
           key = UI_WaitKey();

           switch(key)
                 {
                 case 'x': // cancel
                      for( i=0; i<=len; i++ )
                         buf[i] = 0;

                      api_tim_close( dhn_tim );
                      return( FALSE );
KEYIN_NO:
                 case 'n': // clear
                      UI_ClearRow(row, 1, FONT1);
                      i = 1;
                      buf[0] = 0;
                      goto KEYIN_CLEAR;
KEYIN_YES:
                 case 'y': // ok
                //    if( i == 1 )
                //      continue;

                      if( buf[0] == 0 )
                        {
                        buf[0] = 1;
                        buf[1] = '0';
                        }

                      api_tim_close( dhn_tim );
                      return( TRUE );

                 case '#': // backspace
                      if( i != 1 )
                        {
                        buf[--i] = 0;
                        buf[0] -= 1;
                        ShowKeyIn( type, idle, font, row, buf );
                        continue;
                        }
                      else
                        goto KEYIN_CLEAR;

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
                          ShowKeyIn( type, idle, font, row, buf );
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
                      ShowKeyIn( type, idle, font, row, buf );

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
UCHAR TL_GetAlphaNumKey( UCHAR dir, UCHAR row, UCHAR col, UCHAR font, UCHAR cnt, UCHAR *data )
{
UCHAR buf[21];
UCHAR key;
UCHAR pre_key;
UCHAR alpha_cnt;
UCHAR alpha_flag;
UCHAR old_col;
UCHAR *ptrdata;
UCHAR *ptrbuf;
UCHAR key_cnt;

const UCHAR keypad[10][4]= {
                        {'0', '0', '0', '0'},
                        {'Q', 'Z', '1', '1'},
                        {'A', 'B', 'C', '2'},
                        {'D', 'E', 'F', '3'},
                        {'G', 'H', 'I', '4'},
                        {'J', 'K', 'L', '5'},
                        {'M', 'N', 'O', '6'},
                        {'P', 'R', 'S', '7'},
                        {'T', 'U', 'V', '8'},
                        {'W', 'X', 'Y', '9'}
                       };


      UI_OpenKeyAlphaNum(); // enable numeric keypad

      ptrbuf = data;
      pre_key = 0;
      alpha_cnt = 0;
      alpha_flag = 0;
      key_cnt = 0;

      while(1)
           {
           key = UI_WaitKey();
           switch( key )
                 {
                 case KEY_CANCEL:

                      *ptrbuf = key;
                      return( FALSE );

                 case KEY_CLEAR:

                      *ptrbuf = key;
                      return( FALSE );

                 case KEY_OK:

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
                          UI_PutChar( row, old_col, font, key );
                          }

                        ptrdata = data;
                        *(ptrdata-1) = key;

                        if( dir != 0 )
                          {
                          memmove( &buf[1], ptrbuf, key_cnt );
                          buf[0] = key_cnt;
                          ShowKeyIn( 0, '_', font, row, buf );
                          }
                        }
                        break;

                 case KEY_BACKSPACE:

                      if( dir == 0 )
                        return( TRUE );

                      if( key_cnt != 0 )
                        {
                        memmove( &buf[1], ptrbuf, key_cnt );
                        key_cnt--;
                        cnt++;
                        *(--data) = 0;
                        }
                      buf[0] = key_cnt;
                      ShowKeyIn( 0, '_', font, row, buf );

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
                        UI_PutChar( row, col++, font, key );

                      *data++ = key;
                      pre_key = key;
                      key_cnt++;

                      if( dir != 0 )
                        {
                        memmove( &buf[1], ptrbuf, key_cnt );
                        buf[0] = key_cnt;
                        ShowKeyIn( 0, '_', font, row, buf );
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
UCHAR TL_GetAlphaNumDigits( UCHAR row, UCHAR col, UCHAR font, UCHAR cnt, UCHAR *digits )
{
UCHAR i;
UCHAR result;
UCHAR column;
UCHAR buf[MAX_DSP_CHAR+1];

      column = col;
      digits[0] = 0;

      while(1)
           {
           for( i=0; i<MAX_DSP_CHAR+1; i++ )
              buf[i] = 0;
           ShowKeyIn( 0, '_', font, row, buf );
      //   UI_PutChar( row, col, font, '_' ); // idle char

           // wait & show key
           result = TL_GetAlphaNumKey( 1, row, column, font, cnt, buf );
           if( result == FALSE )
             {
             if( buf[0] == KEY_CLEAR )
               {
               UI_ClearRow( row, 1, font );

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
// FUNCTION: load one TDE data element.
// INPUT   : index - record number. (0..N)
// OUTPUT  : tde   - the data element values.
// RETURN  : none.
// ---------------------------------------------------------------------------
void TL_TDE_GetData( UCHAR index, UCHAR *tde )
{
UCHAR *ptrtag;
UCHAR i;

      ptrtag = (UCHAR *)&TDE_Tags_Table[0] + 4*index;

      for( i=0; i<4; i++ )
         *tde++ = *ptrtag++;
}

// ---------------------------------------------------------------------------
// FUNCTION: load one IDE data element.
// INPUT   : index - record number. (0..N)
// OUTPUT  : tde   - the data element values.
// RETURN  : none.
// ---------------------------------------------------------------------------
void TL_IDE_GetData( UCHAR index, UCHAR *tde )
{
UCHAR *ptrtag;
UCHAR i;

      ptrtag = (UCHAR *)&IDE_Tags_Table[0] + 4*index;

      for( i=0; i<4; i++ )
         *tde++ = *ptrtag++;
}

// ---------------------------------------------------------------------------
// FUNCTION: load one TDE address.
// INPUT   : index - record number. (0..N)
// OUTPUT  : addr  - the data element address.
// RETURN  : none.
// ---------------------------------------------------------------------------
UINT TL_TDE_GetAddr( UCHAR index )
{
      return( TDE_Addr_Table[index] );
}

// ---------------------------------------------------------------------------
// FUNCTION: load one TDE address.
// INPUT   : index - record number. (0..N)
// OUTPUT  : addr  - the data element address.
// RETURN  : none.
// ---------------------------------------------------------------------------
UINT TL_IDE_GetAddr( UCHAR index )
{
      return( IDE_Addr_Table[index] );
}

// ---------------------------------------------------------------------------
// FUNCTION: get date & time.
// INPUT   : none.
// OUTPUT  : rtc - YYMMDDhhmmss in printable ASCII format.
// RETURN  : none.
// ---------------------------------------------------------------------------
void TL_GetDateTime( UCHAR *rtc )
{
     api_rtc_getdatetime( 0, rtc );
}

// ---------------------------------------------------------------------------
// FUNCTION: set date & time.
// INPUT   : rtc - YYMMDDhhmmss in printable ASCII format.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void TL_SetDateTime( UCHAR *rtc )
{
     api_rtc_setdatetime( 0, rtc );
}

// ---------------------------------------------------------------------------
// FUNCTION: set century for the year. (BCD)
//           00~49 = 20YY
//           50~99 = 19YY
// INPUT   : year - 00~99. (bcd)
// OUTPUT  : none.
// RETURN  : century of the year. (bcd)
// ---------------------------------------------------------------------------
UCHAR TL_SetCentury( signed char year )
{
      if( (year >= 0x00) && (year <= 0x49) )
        return( 0x20 ); // 20YY
      else
        return( 0x19 ); // 19YY
}

// ---------------------------------------------------------------------------
// FUNCTION: compare dates.
// INPUT   : date1 - CCYYMMDD, BCD, n4
//           date2 - CCYYMMDD, BCD, n4
//                   CC = 19 or 20
// OUTPUT  : none.
// RETURN  : > 0: date1 > date2
//           = 0: date1 = date2
//           < 0: date1 < date2
// ---------------------------------------------------------------------------
int  TL_CompareDate( UCHAR *date1, UCHAR *date2 )
{

     return( TL_memcmp( date1, date2, 4 ) );

//   if( (date1[0] == date2[0]) && (date1[1] == date2[1]) &&
//       (date1[2] == date2[2]) && (date1[3] == date2[3] )
//     return( 0 );
//
//   if( date1[0] > date2[0] )
//     return( 1 );
//
//   if( date1[1] > date2[1] )
//     return( 1 );
//
//   if( date1[2] > date2[2] )
//     return( 1 );
//
//   return( -1 );

}

// ---------------------------------------------------------------------------
// FUNCTION: check leap year.
// INPUT   : year = calendar year.
// OUTPUT  : none.
// RETURN  : TRUE.
//           FALSE.
// ---------------------------------------------------------------------------
UCHAR	TL_CheckLeapYear( UINT year )
{
	if( ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0) )
	  return( TRUE );  
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
//2008-dec-25	Charles
// FUNCTION: verify date.
// INPUT   : bdate - YYMMDD. (3 bytes in bcd format)
// OUTPUT  : none.
// RETURN  : TRUE.
//           FALSE.
// ---------------------------------------------------------------------------
UCHAR TL_CheckDate( UCHAR *bdate )
{
UCHAR yy, mm, dd;
UCHAR cc;
UINT  year;
UCHAR bcd_year[2] = {0,0};
UCHAR RESPONSE = TRUE;


      bcd_year[1] = *bdate++;
      mm = *bdate++;
      dd = *bdate;
      if(dd==0)
    	  RESPONSE =( FALSE );
      else
      {
      switch(mm)
      {
		  //dd == 31
		  case 0x01:
		  case 0x03:
		  case 0x05:
		  case 0x07:
		  case 0x08:
		  case 0x10:
		  case 0x12:
			  if(dd>0x31)
				  RESPONSE =( FALSE );
			  break;
		  //dd == 30
		  case 0x4:
		  case 0x6:
		  case 0x9:
		  case 0x11:
			  if(dd>0x30)
				  RESPONSE =( FALSE );
			  break;
		  case 0x02:
			  if(dd==0x29)
			  {

				  bcd_year[0] = TL_SetCentury( (signed char)bcd_year[1] );
				  year = TL_bcd2Dec(2,bcd_year);
				  
				  RESPONSE = TL_CheckLeapYear( year );	// PATCH: 2009-01-31
//				  if(year%4!=0)//���O�|�~
//					  RESPONSE =( FALSE );

			  }
			  else if(dd>0x29)
				  RESPONSE =( FALSE );
			  break;
		  default:
			  RESPONSE =( FALSE );
			  break;
      }
      }
/*

      // month: 01-12
      if( (mm == 0x00) || (mm > 0x12) )
        return( FALSE );

      // day  : 01-31
      if( (dd == 0x00) || (dd > 0x31) )
        return( FALSE );

//    return( TRUE );

      // ---------------------------------------------------------
      // Leap Year (RFU) PATCH: 2006-10-07
      // One of the following conditions is met, it's a leap year.
      // 1. Divisible by 4 and not divisible by 100
      // 2. Divisible by 4 and divisible by 400
      // ---------------------------------------------------------

      if( mm == 0x02 )
        {
        if( dd > 0x29 )
          return( FALSE );

        cc = TL_SetCentury( (signed char)yy );
        year = cc*256 + yy;

        if( (year % 100) == 0 )
          {
          if( (year % 400) == 0 )
            {
            if( dd > 0x29 )
              return( FALSE );
            }
          }
        else
          {
          if( (year % 4) == 0 )
            {
            if( dd > 0x29 )
              return( FALSE );
            }
          else
            {
            if( dd > 0x28 )
              return( FALSE );
            }
          }
        }
*/

      return RESPONSE;
}

// ---------------------------------------------------------------------------
// FUNCTION: verify date & time.
// INPUT   : rtc - LEN[1] DATA[n].
// OUTPUT  : none.
// RETURN  : TRUE.
//           FALSE.
// ---------------------------------------------------------------------------
UCHAR TL_CheckDateTime( UCHAR rtc[] )
{
UCHAR buffer[3];
int   data;

      if( rtc[0] != 12 )
        return( FALSE );

      buffer[2] = 0;

      // check month
      buffer[0] = rtc[3];
      buffer[1] = rtc[4];
      data = atoi( buffer );
      if( (data == 0) || (data > 12) )
        return( FALSE );

      // check day
      buffer[0] = rtc[5];
      buffer[1] = rtc[6];
      data = atoi( buffer );
      if( (data == 0) || (data > 31) )
        return( FALSE );

      // check hour
      buffer[0] = rtc[7];
      buffer[1] = rtc[8];
      data = atoi( buffer );
      if( data > 24 )
        return( FALSE );

      // check minute
      buffer[0] = rtc[9];
      buffer[1] = rtc[10];
      data = atoi( buffer );
      if( data > 59 )
        return( FALSE );

      // check second
      buffer[0] = rtc[11];
      buffer[1] = rtc[12];
      data = atoi( buffer );
      if( data > 59 )
        return( FALSE );

      return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: format date to a new presentation.
// INPUT   : o_date - original date (YYMMDD)
// OUTPUT  : n_date - formatted date (YY/MM/DD)
// RETURN  : none.
// ---------------------------------------------------------------------------
void TL_FormatDate( UCHAR o_date[], UCHAR n_date[] )
{
      n_date[0] = o_date[0];
      n_date[1] = o_date[1];
      n_date[2] = '/';
      n_date[3] = o_date[2];
      n_date[4] = o_date[3];
      n_date[5] = '/';
      n_date[6] = o_date[4];
      n_date[7] = o_date[5];
}

// ---------------------------------------------------------------------------
// FUNCTION: format time to a new presentation.
// INPUT   : o_time - original date (HHMMSS)
// OUTPUT  : n_time - formatted date (HH:MM:SS)
// RETURN  : none.
// ---------------------------------------------------------------------------
void TL_FormatTime( UCHAR o_time[], UCHAR n_time[] )
{
      n_time[0] = o_time[0];
      n_time[1] = o_time[1];
      n_time[2] = ':';
      n_time[3] = o_time[2];
      n_time[4] = o_time[3];
      n_time[5] = ':';
      n_time[6] = o_time[4];
      n_time[7] = o_time[5];
}

// ---------------------------------------------------------------------------
// FUNCTION: display date & time.
//           YY/MM/DD hh:mm
// INPUT   : rtc - YYMMDDhhmmss in printable ASCII format.
//           row - row position.
//           col - column position.
//           font- font id & attr.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void TL_ShowDateTime( UCHAR row, UCHAR col, UCHAR font, UCHAR rtc[] )
{
UCHAR buffer[17];

      buffer[0] = rtc[0];
      buffer[1] = rtc[1];
      buffer[2] = '/';
      buffer[3] = rtc[2];
      buffer[4] = rtc[3];
      buffer[5] = '/';
      buffer[6] = rtc[4];
      buffer[7] = rtc[5];
      buffer[8] = ' ';
      buffer[9] = rtc[6];
      buffer[10]= rtc[7];
      buffer[11]= ':';
      buffer[12]= rtc[8];
      buffer[13]= rtc[9];
      buffer[14]= ':';
      buffer[15]= rtc[10];
      buffer[16]= rtc[11];
      UI_PutStr( row, col, font, 17, buffer );
}

// ---------------------------------------------------------------------------
// FUNCTION: display date & time.
//           YY/MM/DD hh:mm
// INPUT   : rtc   - YYMMDDhhmmss in printable ASCII format.
//           row   - row position.
//           pos   - column position.
//           font  - font id & attr.
//           msgid - message id, 0=msg_q_update
//                               1=msg_yes_or_no
//                               2=msg_q_clear
//                               3=msg_q_print
//                               4=msg_cancel_or_enter
//                              99=no message
// OUTPUT  : none.
// RETURN  : TRUE  - to be updated.
//           FALSE - not to be update.
// ---------------------------------------------------------------------------
#ifdef _EMVDCMSG_H_
UCHAR TL_UpdateReq( UCHAR msgid, UCHAR row, UCHAR pos, UCHAR font )
{
UCHAR key;

      switch( msgid )
            {
            case 0:

                 UI_PutMsg( row, pos, font, 13, (UCHAR *)msg_q_update );
                 break;

            case 1:

                 UI_PutMsg( row, pos, font, 6, (UCHAR *)msg_yes_or_no );
                 break;

            case 2:

                 UI_PutMsg( row, pos, font, 12, (UCHAR *)msg_q_clear );
                 break;

            case 3:

                 UI_PutMsg( 0, COL_MIDWAY, FONT2+attrCLEARWRITE, 5, (UCHAR *)msg_C_q_print );
                 UI_PutMsg( row, pos, font, 20, (UCHAR *)msg_q_print );
                 break;

            case 4:

                 UI_PutMsg( row, pos, font, 17, (UCHAR *)msg_cancel_or_enter );
                 break;

            case 99:
                 break;
            }

      while(1)
           {
           key = UI_WaitKey();
           if( key == KEY_OK )
             return( TRUE );
           else
             {
             if( key == KEY_CANCEL )
               return( FALSE );
             }
           }
}

#endif
// ---------------------------------------------------------------------------
// FUNCTION: convert the BCD data to ASCII character and show it.
// INPUT   : row  - row#.
//           col  - col#.
//           font - font id.
//           len  - length of bcd data.
//           bcd  - the bcd data.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void TL_ShowBCD( UCHAR row, UCHAR col, UCHAR font, UCHAR len, UCHAR *bcd )
{
UCHAR text[21]; // L-V

      TL_bcd2asc( len, bcd, text );
      UI_PutStr( row, col, font, text[0], &text[1] );
}

// ---------------------------------------------------------------------------
// FUNCTION: read the specified transaction log (fixed length = TX_LOG_LEN)
// INPUT   : recno - record number of the specified log. (0..N)
// OUTPUT  : log   - the log data read. (fixed length)
// REF     : g_term_tx_log_cnt
// RETURN  : TRUE  - OK.
//           FALSE - record not found.
// ---------------------------------------------------------------------------
UCHAR TL_GetTransLog( UINT recno, UCHAR *log )
{
API_SRAM pSram;
//UCHAR sbuf[7];
ULONG address;

     address = TX_LOG_001 + TX_LOG_LEN*recno; // target address

     pSram.StPage = SRAM_PAGE_LOG;
     pSram.StAddr = address;
     pSram.Len = TX_LOG_LEN;
     api_sram_PageRead( pSram, log );

//      sbuf[0] = SRAM_PAGE_LOG;
//      sbuf[1] = address & 0x000000ff;
//      sbuf[2] = (address & 0x0000ff00) >> 8;
//      sbuf[3] = (address & 0x00ff0000) >> 16;
//      sbuf[4] = (address & 0xff000000) >> 24;
//      sbuf[5] = TX_LOG_LEN;
//      sbuf[6] = 0;
//      api_ram_read( sbuf, log );

     return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: write the specified transaction log (fixed length = TX_LOG_LEN)
// INPUT   : recno - record number of the specified log. (0..N)
//           log   - the log data to be written.
// OUTPUT  : none.
// REF     : g_term_tx_log_cnt
// RETURN  : TRUE  - OK.
//           FALSE - out of memory.
// ---------------------------------------------------------------------------
UCHAR TL_PutTransLog( UINT recno, UCHAR *log )
{
API_SRAM pSram;
//UCHAR sbuf[7];
ULONG address;

     if( g_term_tx_log_cnt > MAX_TX_LOG_CNT )
       return( FALSE ); // out of memory

     address = TX_LOG_001 + TX_LOG_LEN*recno; // target address

     pSram.StPage = SRAM_PAGE_LOG;
     pSram.StAddr = address;
     pSram.Len = TX_LOG_LEN;
     api_sram_PageWrite( pSram, log );

//      sbuf[0] = SRAM_PAGE_LOG;
//      sbuf[1] = address & 0x000000ff;
//      sbuf[2] = (address & 0x0000ff00) >> 8;
//      sbuf[3] = (address & 0x00ff0000) >> 16;
//      sbuf[4] = (address & 0xff000000) >> 24;
//      sbuf[5] = TX_LOG_LEN;
//      sbuf[6] = 0;
//      api_ram_write( sbuf, log );

     g_term_tx_log_cnt++;

     return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: clear transaction logs.
// INPUT   : none.
// OUTPUT  : none.
// REF     : g_term_tx_log_cnt
// RETURN  : none.
// ---------------------------------------------------------------------------
void TL_ClearTransLog( void )
{
    g_term_tx_log_cnt = 0; // reset counter
}

// ---------------------------------------------------------------------------
// FUNCTION: generate a random number (8-byte hex value).
// INPUT   : none.
// OUTPUT  : data - the random number
// RETURN  : none.
// ---------------------------------------------------------------------------
void TL_GetRandomNumber( UCHAR *data )
{
      api_sys_random( data );
}

// ---------------------------------------------------------------------------
// FUNCTION: long integer division.
// INPUT   : numer - numerator.
//           denom - denominator.
//           numer >= denom
// OUTPUT  : none.
// RETURN  : the quotation. (eg. 1.4 -> 1, 1.5 -> 2)
// ---------------------------------------------------------------------------
ULONG TL_ldiv( ULONG numer, ULONG denom )
{
ULONG quo1;
ULONG data;

      quo1 = numer / denom;
      data = quo1 * denom;

      if( quo1 == data )
        return( quo1 );

      if( (data - quo1) >= (numer/2) )
        return( quo1 + 1 );
      else
        return( quo1 );
}

// ---------------------------------------------------------------------------
// FUNCTION: load NAME list for FUNCTION key processing.
// INPUT   : id   - 0: EMVDC_Func_Table[]
//                  1: TAC_Func_Table[]
//                  2: REF_Func_Table[]
//                  3: SALE_Func_Table[]
// OUTPUT  : list - array for the list.
// RETURN  : none.
// ---------------------------------------------------------------------------

void TL_LoadFuncList( UCHAR id, UCHAR list[] )
{
      switch( id )
            {
            case 0:
                 memmove( list, EMVDC_Func_Table, MAX_EMVDC_FUNC_CNT*EMVDC_FUNC_LEN );
                 break;

            case 1:
                 memmove( list, TAC_Func_Table, MAX_TAC_FUNC_CNT*TAC_FUNC_LEN );
                 break;

            case 2:
                 memmove( list, REF_Func_Table, 2*EMVDC_FUNC_LEN );
                 break;

            case 3:
                 memmove( list, SALE_Func_Table, 2*EMVDC_FUNC_LEN );
                 break;
            }
}


// ---------------------------------------------------------------------------
// FUNCTION: validate data type.
// INPUT   : type - the data type. (DEF_xxx)
//           data - the data to be verified.
//           len  - length of data.
// OUTPUT  : none.
// RETURN  : TRUE  - OK.
//           FALSE - illegal data type.
// ---------------------------------------------------------------------------
UCHAR TL_CheckDataType( UCHAR type, UCHAR *data, UCHAR len )
{
UCHAR i;
UCHAR cc;

      switch( type )
            {
            case DEF_AN: // '0'-'9', 'A'-'Z', 'a'-'z'

                 for( i=0; i<len; i++ )
                    {
                    cc = *data++;
                    if( ((cc >= '0') && (cc <= '9')) ||
                        ((cc >= 'A') && (cc <= 'Z')) ||
                        ((cc >= 'a') && (cc <= 'z')) )
                      continue;
                    else
                      return( FALSE );
                    }
                 return( TRUE );

            default:
                 return( FALSE );
            }
}

// ---------------------------------------------------------------------------
// FUNCTION: verify the expiration date for the specified certicicate.
// INPUT   : cdate - MMYY (n4, 2 bytes).
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - expired.
// ---------------------------------------------------------------------------
UCHAR TL_VerifyCertificateExpDate( UCHAR *cdate )
{
UINT  i, j;
UCHAR temp[20];

      TL_GetDateTime( temp ); // get current date (temp[0..11]="YYMMDDhhmmss")

//    temp[12] = TL_SetCentury( temp[0] );
      temp[12] = TL_SetCentury( TL_ascw2bcdb(&temp[0]) );	// 2015-03-03
      temp[13] = ((temp[0] & 0x0f) << 4) + (temp[1] & 0x0f); // convert to BCD
      i = temp[12]*256 + temp[13];   // today year CCYY

      temp[14] = TL_SetCentury( *(cdate+1) );
      j = temp[14]*256 + *(cdate+1); // expiry year CCYY

      if( j < i )  // compare Year
        return( FALSE );

      if( j == i ) // compare Month
        {
        temp[12] = ((temp[2] & 0x0f) << 4) + (temp[3] & 0x0f); // convert to BCD
        if( *cdate < temp[12] ) // compare month
          return( FALSE );
        }

      return(TRUE);
}

// ---------------------------------------------------------------------------
// FUNCTION: verify the transaction amount is under ceiling limit.
//           max. amount = "42 94 96 72 95" (0xFF FF FF FF)
//           decimal point is implied. (0, 1, 2, or 3)
// INPUT   : amt - the current transaction amount. (excluding adjustment)
// OUTPUT  : none.
// REF     : g_term_tx_exp
// RETURN  : TRUE  - OK.
//           FALSE - overflow.
// ---------------------------------------------------------------------------
UCHAR TL_VerifyTransAmount( UCHAR *amt )
{
UCHAR buf[6];

//    switch( g_term_tx_exp ) // decimal point position (iso 4217)
//          {
//          case 0: // 4294967295
//               buf[0] = 0x42;
//               buf[1] = 0x94;
//               buf[2] = 0x96;
//               buf[3] = 0x72;
//               buf[4] = 0x95;
//               break;
//
//          case 1: // 429496729.0
//               buf[0] = 0x04;
//               buf[1] = 0x29;
//               buf[2] = 0x49;
//               buf[3] = 0x67;
//               buf[4] = 0x29;
//               break;
//
//          case 2: // 42949672.00
//               buf[0] = 0x00;
//               buf[1] = 0x42;
//               buf[2] = 0x94;
//               buf[3] = 0x96;
//               buf[4] = 0x72;
//               break;
//
//          case 3: // 4294967.000
//               buf[0] = 0x00;
//               buf[1] = 0x04;
//               buf[2] = 0x29;
//               buf[3] = 0x49;
//               buf[4] = 0x67;
//               break;
//          }

      buf[0] = 0x00;
      buf[1] = 0x42;
      buf[2] = 0x94;
      buf[3] = 0x96;
      buf[4] = 0x72;
      buf[5] = 0x95;

      if( TL_memcmp( amt, buf, 6 ) > 0 )
        return( FALSE );
      else
        return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: open AUX device for communicaiton. (9600, 8, n, 1)
// INPUT   : none.
// OUTPUT  : none.
// REF     : g_dhn_aux
// RETURN  : TRUE  - OK
//           FALSE - device error.
// ---------------------------------------------------------------------------
UCHAR TL_OpenAUX( void )
{
API_AUX	pAux;

      pAux.Mode = auxDLL;
      pAux.Baud = COM_9600 + COM_CHR8 + COM_NOPARITY + COM_STOP1;
      pAux.Tob = 100;	// 1 sec
      pAux.Tor = 200;	// 2 sec
      pAux.Acks = 1;	//
      pAux.Resend = 0;	// no retry
      g_dhn_aux = api_aux_open( COM1, pAux );	// 'COM2' is to be changed if necessary

      if((g_dhn_aux == apiOutOfService) )
        {
        api_aux_close(0);
        return( FALSE );
        }
      else
        return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: close AUX device.
// INPUT   : none.
// OUTPUT  : none.
// REF     : g_dhn_aux
// RETURN  : TRUE  - OK
//           FALSE - device error.
// ---------------------------------------------------------------------------
UCHAR TL_CloseAUX( void )
{
UCHAR result;

      TL_WaitTime(10);	// PATCH: 2008-12-01, wait for end of sending ACK to HOST

      result = api_aux_close( g_dhn_aux );
      if( result != apiOK )
        {
        api_aux_close(0);
        return( FALSE );
        }
      else
        return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: receive data from AUX port.
// INPUT   : none.
// OUTPUT  : data - 2L-V, the data receivd.
// REF     : g_dhn_aux
// RETURN  : TRUE  - data ready.
//           FALSE - device error or timeout before data ready.
// ---------------------------------------------------------------------------
UCHAR TL_ReceiveAUX( UCHAR *data )
{
UCHAR dhn_tim;
UINT  tick1=0, tick2=0;

      dhn_tim = api_tim_open(100); // time tick = 1sec
      api_tim_gettick( dhn_tim, (UCHAR *)&tick1 );

      do{
        api_tim_gettick( dhn_tim, (UCHAR *)&tick2 );

        if( api_aux_rxready( g_dhn_aux, data ) == apiReady )
          {
          api_aux_rxstring( g_dhn_aux, data );

          api_tim_close( dhn_tim );
          return( TRUE );
          }
        } while( (tick2 - tick1) <= 3 );

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
UCHAR TL_TransmitAUX( UCHAR *data )
{
UCHAR dhn_tim;
UINT  tick1=0, tick2=0;
UCHAR result;

      dhn_tim = api_tim_open(100); // time tick = 1sec
      api_tim_gettick( dhn_tim, (UCHAR *)&tick1 );

      do{
        api_tim_gettick( dhn_tim, (UCHAR *)&tick2 );

        if( api_aux_txready( g_dhn_aux ) == apiReady )
          {

          api_aux_txstring( g_dhn_aux, data );

          // wait for ACK from HOST
          do{
            api_tim_gettick( dhn_tim, (UCHAR *)&tick2 );

            result = api_aux_txready( g_dhn_aux );
            if( result == apiFailed)
              break;

            if( result == apiReady )
              {
              api_tim_close( dhn_tim );
              return( TRUE );
              }
            } while( (tick2 - tick1) <= 3 );

          break;
          }
        } while( (tick2 - tick1) <= 3 );

      api_tim_close( dhn_tim );
      return( FALSE );
}

