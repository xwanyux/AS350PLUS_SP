//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : 		                                            **
//**  PRODUCT  : AS320                                                      **
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
//**  Copyright(C) 2011 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
// 
// For large data dumping through COM port, the following system functions are useful.
// Please refer to the sample codes in UT_DumpHexData() or UT_DumpHexData2().
//
// On terminal:
// _DEBUGPRINTF_OPEN( ULONG UartPort );
//	Enable the target COM port with default setting - 115200,8,N,1.
//
// _DEBUGPRINTF( char *Msg, ... );
//	Print formatted output to the target COM port.
//	The usage is the same as standard C function "printf()".
//
// On PC:
// Windows HyperTerminal or the similar program can be used as the output device.
//----------------------------------------------------------------------------
#include <string.h>
#include <time.h>
#include "PN5180Define.h"
#include "GlobalVar.h"
#include "POSAPI.h"
#include "UTILS.H"
#include "PN5180Function.h"
#include "LCDTFTAPI.H"
//#include "XIOAPI.h"


extern volatile	ULONG os_SysTimerFreeCnt;

extern UCHAR	apk_EMVCL_GetBERLEN(UCHAR * de, UCHAR * cnt, UINT * optValue);
extern UCHAR	apk_EMVCL_CheckConsTag(UCHAR tag);


UCHAR		ut_dhn_kbd;
UCHAR		ut_dhn_buz;
UCHAR		ut_dhn_lcd;
UCHAR		ut_dhn_prt;
UCHAR		ut_dhn_buz_1l;
UCHAR		ut_dhn_buz_Success;
UCHAR		ut_dhn_buz_Alert;

UCHAR Utils_Global_palette[3] = {0xF0,0xF0,0xFF};

#ifndef _PLATFORM_AS210
UCHAR	UT_ClearRowEx(UCHAR row, UCHAR cnt, UCHAR font, UCHAR *palette)
{
API_LCDTFT_PARA para;
UCHAR	attr = 0;
UCHAR	dhn;


	dhn = ut_dhn_lcd;
	  
	memset( (UCHAR *)&para, 0x00, sizeof(API_LCDTFT_PARA) );
	para.Row = row;
	para.Col = cnt;
	
	switch( font & 0x0F )
	      {
	      case FONT0:
	      
          	   para.Font = LCDTFT_FONT0 + attr;
          	   para.FontHeight = TFTLCD_FONT0_H;
          	   para.FontWidth  = TFTLCD_FONT0_W;
	           break;
	           
	      case FONT1:

          	   para.Font = LCDTFT_FONT1 + attr;
          	   para.FontHeight = TFTLCD_FONT1_H;
          	   para.FontWidth  = TFTLCD_FONT1_W;
	           break;
	           
	      case FONT2:

          	   para.Font = LCDTFT_FONT2 + attr;
          	   para.FontHeight = TFTLCD_FONT2_H;
          	   para.FontWidth  = TFTLCD_FONT2_W;
	           break;
	            
	      default:

          	   para.Font = LCDTFT_FONT0 + attr;
          	   para.FontHeight = TFTLCD_FONT0_H;
          	   para.FontWidth  = TFTLCD_FONT0_W;
	           break;
	      }

	para.RGB = 0;

	if( attr & attrREVERSE )
	  {
	  para.BG_Palette[0] = 0x00;	// black
	  para.BG_Palette[1] = 0x00;
	  para.BG_Palette[2] = 0x00;
	  }
	else
	  {
	  para.BG_Palette[0] = palette[0];	// background palette
	  para.BG_Palette[1] = palette[1];
	  para.BG_Palette[2] = palette[2];
	  }       
	
	return( api_lcdtft_clear( dhn, para ) );
}

// ---------------------------------------------------------------------------
// FUNCTION: Clear whole display screen.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UT_ClearScreenEx(void)
{
API_LCDTFT_PARA		para;


	// Clear screen
	memset( (UCHAR *)&para, 0x00, sizeof(API_LCDTFT_PARA) );
	para.RGB = RGB_BPP16;
	para.Row = 0xFFFF;
	para.BG_Palette[0] = 0xF0; // Light Blue
	para.BG_Palette[1] = 0xF0;
	para.BG_Palette[2] = 0xFF;

	api_lcdtft_clear( ut_dhn_lcd, para );
}
#endif

void UT_Set_LEDSignal(UINT iptID, UINT iptBlkOn, UINT iptBlkOff)
{
	UCHAR			dhnLCD=0;
	API_PCD_ICON	ledSignal;

	ledSignal.ID=iptID;
	ledSignal.BlinkOn=iptBlkOn;
	ledSignal.BlinkOff=iptBlkOff;

	//AS210
//	api_xio_showPCD(ledSignal);

	//AS350
	api_lcdtft_showPCD(dhnLCD, ledSignal);
}

//20130412	copy from tool.c
// ---------------------------------------------------------------------------
// FUNCTION: Compares memory with the specified character.
// INPUT   : s1  - a pointer to the object.
//           cc  - the data byte to be compared.
//           len - length of data to be compared.
// OUTPUT  : none.
// RETURN  : =0  - all s1 = cc.
//           !=0 - not all s1 = cc.
// ---------------------------------------------------------------------------
int UT_memcmpc( UCHAR *s1, UCHAR cc, UCHAR len )
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

//20130412   copy from tool.c , UCHAR TL_SetCentury( signed char year )
// ---------------------------------------------------------------------------
// FUNCTION: set century for the year. (BCD)
//           00~49 = 20YY
//           50~99 = 19YY
// INPUT   : year - 00~99. (bcd)
// OUTPUT  : none.
// RETURN  : century of the year. (bcd)
// ---------------------------------------------------------------------------
UCHAR UT_SetCentury( signed char year )
{
      if( (year >= 0x00) && (year <= 0x49) )
        return( 0x20 ); // 20YY
      else
        return( 0x19 ); // 19YY
}

//20130412   copy from tool.c , UCHAR TL_VerifyCertificateExpDate( UCHAR *cdate )
// ---------------------------------------------------------------------------
// FUNCTION: verify the expiration date for the specified certicicate.
// INPUT   : cdate - MMYY (n4, 2 bytes).
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - expired.
// ---------------------------------------------------------------------------
UCHAR UT_VerifyCertificateExpDate( UCHAR *cdate )
{
UINT  i, j;
UCHAR temp[20];

      UT_GetDateTime( temp ); // get current date (temp[0..11]="YYMMDDhhmmss")
      
      temp[13] = ((temp[0] & 0x0f) << 4) + (temp[1] & 0x0f); // convert to BCD
      temp[12] = UT_SetCentury( temp[13] );

      i = temp[12]*256 + temp[13];   // today year CCYY

      temp[14] = UT_SetCentury( *(cdate+1) );
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


//20130412   copy from tool.c , UCHAR TL_CNcmp( UCHAR *s1, UCHAR *s2, UCHAR len )
// ---------------------------------------------------------------------------
// FUNCTION: convert BCD data to ASCII string.
// INPUT   : bcdlen - size of bcd buffer. (in bytes)
//           bcd    - right-justified bcd data with leading zeros.
// OUTPUT  : str    - L-V, the ascii string.
// RETURN  : the BCD value.
// ---------------------------------------------------------------------------
void UT_bcd2asc( UCHAR bcdlen, UCHAR *bcd, UCHAR *str )
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

//20130412   copy from tool.c , UCHAR TL_CNcmp( UCHAR *s1, UCHAR *s2, UCHAR len )
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
UCHAR UT_CNcmp( UCHAR *s1, UCHAR *s2, UCHAR len )
{
UCHAR i;
UCHAR buf1[21];
UCHAR buf2[21];

      UT_bcd2asc( len, s1, buf1 );
      UT_bcd2asc( len, s2, buf2 );

      for( i=0; i<buf1[0]; i++ ) // based on the length of "s1"
         {
         if( buf1[i+1] != buf2[i+1] )
           return( FALSE );
         }
      return( TRUE );
}

//20130412   copy from tool.c , UCHAR TL_CNcmp2( UCHAR *s1, UCHAR *s2, UCHAR len )
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
UCHAR UT_CNcmp2( UCHAR *s1, UCHAR *s2, UCHAR len )
{
UCHAR i;
UCHAR buf1[21];
UCHAR buf2[21];

      UT_bcd2asc( len, s1, buf1 ); // recovered PAN
      UT_bcd2asc( len, s2, buf2 ); // original  PAN

      if( buf1[0] != buf2[0] ) // check size of digits
        return( FALSE );

      for( i=0; i<buf1[0]; i++ ) // check value of digits
         {
         if( buf1[i+1] != buf2[i+1] )
           return( FALSE );
         }
      return( TRUE );
}

//20130311 atoi

int atoi(UCHAR *s)
{	
	int temp=0;
		while(*s>='0'&&*s<='9')
		{
			temp=10*temp+(*s-'0');
			++s;
		}
		return temp;
}



//20130311 modified from TOOL.c - TL_memcmp(UCHAR *s1, UCHAR *s2, UCHAR len)

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
int	 UT_memcmp( UCHAR *s1, UCHAR *s2, UCHAR len )
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

//20130311 modified from TOOL.c - TL_CompareDate(UCHAR *date1, UCHAR *date2)

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
int  UT_CompareDate( UCHAR *date1, UCHAR *date2 )
{

     return( UT_memcmp( date1, date2, 4 ) );

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


//20130311 modified from TOOL.c - TL_CheckDateTime(UCHAR rtc[])
//20130621 modify check month start from rtc[3]
// ---------------------------------------------------------------------------
// FUNCTION: verify date & time.
// INPUT   : rtc - LEN[1] DATA[n].
// OUTPUT  : none.
// RETURN  : TRUE.
//           FALSE.
// ---------------------------------------------------------------------------

UCHAR UT_CheckDateTime( UCHAR rtc[] )
{
UCHAR buffer[2];
int   data;

      if( rtc[0] != 12 )
        return( FALSE );

      buffer[2] = 0;

      // check month
//      buffer[0] = rtc[0];
//      buffer[1] = rtc[1];
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

//20130311 modified from TOOL.c - UT_GetDateTime(UCHAR rtc[])

// ---------------------------------------------------------------------------
// FUNCTION: get date & time.
// INPUT   : none.
// OUTPUT  : rtc - YYMMDDhhmmss in printable ASCII format.
// RETURN  : none.
// ---------------------------------------------------------------------------

void UT_GetDateTime( UCHAR *rtc )
{
     api_rtc_getdatetime( 0, rtc );
}

/*
*	20130321	Modify UINT year; to int year;
*	20130321	Modify cptr = timer; to cptr = (UCHAR*)timer;
*	20130321	Modify temp[3] = 0; to temp[2] = 0;
*/
time_t	UT_time( time_t *timer )
{
UCHAR	abuf[12];
UCHAR	temp[3];
struct	tm ltime;
//UINT	year;
int		year;
UINT	mon;
UINT	day;
UINT	hour;
UINT	min;
UINT	sec;
ULONG	tseconds;
UCHAR	*cptr;

	
	api_rtc_getdatetime( 0, abuf );		// read local date & time
	
	// convert ASCII to binary
	temp[0] = abuf[0];
	temp[1] = abuf[1];
//	temp[3] = 0;
	temp[2] = 0;
	year = atoi( temp );
	if( (year >= 0x00) && (year <= 0x49) )
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
	
//	tseconds -= (28800+356);	// GMT-8

	if( timer != NULL )
	  {
//	  cptr = timer;
	  cptr = (UCHAR*)timer;

	  *cptr++ = tseconds & 0x000000FF;
	  *cptr++ = (tseconds & 0x0000FF00) >> 8;
	  *cptr++ = (tseconds & 0x00FF0000) >> 16;
	  *cptr++ = (tseconds & 0xFF000000) >> 24;
	  }
	
	return( tseconds );
}

/*
*	20130318	Rename from TL_ascw2bcdb
*/
// ---------------------------------------------------------------------------
// FUNCTION: convert ASCII word to BCD byte.
// INPUT   : buf - the ascii word.
// OUTPUT  : none.
// RETURN  : the BCD value.
// ---------------------------------------------------------------------------
UCHAR UT_ascw2bcdb( UCHAR buf[] )
{
      return( ((buf[0] & 0x0f) << 4) + (buf[1] & 0x0f) );
}

/*
*	20130318	Rename from TL_asc2bcd
*/
// ---------------------------------------------------------------------------
// FUNCTION: convert ASCII string to BCD data.
// INPUT   : str    - L-V, the ascii string.
//           bcdlen - size of bcd buffer.
// OUTPUT  : bcd - right-justified bcd data with leading zeros.
// RETURN  : the BCD value.
// ---------------------------------------------------------------------------
void UT_asc2bcd( UCHAR bcdlen, UCHAR *bcd, UCHAR *str )
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
         bcd[bcdidx--] = UT_ascw2bcdb( buf );

         len -= 2;
         }

      if( remainder != 0 )
        {
        buf[1] = *(str+len);
        buf[0] = '0';
        bcd[bcdidx] = UT_ascw2bcdb( buf );
        }
}

/*
*	20130320	Rename from TL_bcd2hex
*/
// ---------------------------------------------------------------------------
// FUNCTION: convert BCD data to HEX data.
// INPUT   : bcdlen - size of bcd data in bytes. (max. 5)
//           bcd    - the bcd data.          (eg,"46 60" )
// OUTPUT    hex    - fixed 4-byte hex data. (eg,"00 00 12 34" = 0x1234)
// RETURN  : none.
// ---------------------------------------------------------------------------
void UT_bcd2hex( UCHAR bcdlen, UCHAR *bcd, UCHAR *hex )
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
// FUNCTION: Open buzzer for 1 short beep.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UT_OpenBuzzer_1S(void)
{
UCHAR	buf[3];

	buf[0]=1;
	buf[1]=5;
	buf[2]=1;
	ut_dhn_buz = api_buz_open(buf);
}

void	UT_OpenBuzzer_1L(void)
{
	UCHAR	buf[5]={0};

	buf[0]=1;
	buf[1]=15;
	buf[2]=1;
	
	ut_dhn_buz_1l = api_buz_open(buf);
}

void	UT_OpenBuzzer_Success(void)
{
	UCHAR	buf[5]={0};

	buf[0]=1;
	buf[1]=50;
	buf[2]=1;
	buf[3]=0xDC;
	buf[4]=0x05;
	
	ut_dhn_buz_Success=api_buz_open(buf);
}

void	UT_OpenBuzzer_Alert(void)
{
	UCHAR	buf[5]={0};

	buf[0]=2;
	buf[1]=20;
	buf[2]=20;
	buf[3]=0xEE;
	buf[4]=0x02;
	
	ut_dhn_buz_Alert=api_buz_open(buf);
}

// ---------------------------------------------------------------------------
// FUNCTION: Turn on buzzer for 1 short beep.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UT_BUZ_Beep1( void )
{
	api_buz_sound( ut_dhn_buz );
}

// ---------------------------------------------------------------------------
// FUNCTION: Turn on buzzer for 1 long beep.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UT_BUZ_Beep1L( void )
{
	  api_buz_sound( ut_dhn_buz_1l );
}

void UT_BUZ_Success(void)
{
	api_buz_sound(ut_dhn_buz_Success);
}

void UT_BUZ_Alert(void)
{
	api_buz_sound(ut_dhn_buz_Alert);
}

// ---------------------------------------------------------------------------
// FUNCTION: Open keyboard - all keys.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UT_OpenKey_ALL( void )
{
UCHAR	buf[5];

	buf[0]=0x0ff;
	buf[1]=0x0ff;
	buf[2]=0x0ff;
	buf[3]=0x0ff;
	buf[4]=0x0ff;
	ut_dhn_kbd = api_kbd_open(0, buf);
}

// ---------------------------------------------------------------------------
// FUNCTION: Open keyboard - all keys.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UT_OpenKeyAll( void )
{
UCHAR	buf[5];

	buf[0]=0x0ff;
	buf[1]=0x0ff;
	buf[2]=0x0ff;
	buf[3]=0x0ff;
	buf[4]=0x0ff;
	ut_dhn_kbd = api_kbd_open(0, buf);
}

// ---------------------------------------------------------------------------
// FUNCTION: Open keyboard - alpha, numeric keys, cancel, clear, backspace, OK.
// INPUT   : none.
// OUTPUT  : g_dhn_kbd
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UT_OpenKeyAlphaNum()
{
UCHAR	buf[5];

	buf[0]=0x01c;
	buf[1]=0x03c;
	buf[2]=0x03c;
	buf[3]=0x03c;
	buf[4]=0x000;
	ut_dhn_kbd = api_kbd_open(0, buf);
}

// ---------------------------------------------------------------------------
// FUNCTION: Open keyboard - numeric keys, cancel, clear, backspace, OK.
// INPUT   : none.
// OUTPUT  : g_dhn_kbd
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UT_OpenKeyNum()
{
UCHAR	buf[5];

	buf[0]=0x01c;
	buf[1]=0x03c;
	buf[2]=0x03c;
	buf[3]=0x02c;
	buf[4]=0x000;
	ut_dhn_kbd = api_kbd_open(0, buf);
}

// ---------------------------------------------------------------------------
// FUNCTION: Wait one key-stroke.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : the key code data.
// ---------------------------------------------------------------------------
UCHAR	UT_WaitKey(void)
{
UCHAR	buf[1];

	api_kbd_getchar(ut_dhn_kbd, buf);
	api_buz_sound(ut_dhn_buz); // 1 short beep
	return(buf[0]);
}

// ---------------------------------------------------------------------------
// FUNCTION: Get key status.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiReady
//           apiNotReady
// ---------------------------------------------------------------------------
UCHAR	UT_GetKeyStatus(void)
{
UCHAR	buf[1];

	return( api_kbd_status( ut_dhn_kbd, buf) );
}

// ---------------------------------------------------------------------------
// FUNCTION: switch alphanumeric keypad.
// INPUT   : key - the latest key code before "ALPHA" key is pressed.
// OUTPUT  : key - the new key code after "ALPHA" key is pressed.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UT_SwitchAlphanumericKey( UCHAR *key )
{
//UCHAR	i, j, k;	For Remove Warning
UCHAR	i, j;

const	UCHAR alpha_tbl[10][4]=
	{{'0','0','0','0'},
	 {'1','Q','Z','1'},
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
void	UT_OpenDisplay( void )
{
	ut_dhn_lcd = api_lcd_open( 0 );
}

// ---------------------------------------------------------------------------
// FUNCTION: Clear whole display screen.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UT_ClearScreen(void)
{
UCHAR	buf[3];

	buf[0]=-1;
	buf[1]=0;
	buf[2]=FONT1;
	api_lcd_clear(ut_dhn_lcd, buf);
}

// ---------------------------------------------------------------------------
// FUNCTION: Clear whole display screen.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UT_BlackenScreen(void)
{
UCHAR	buf[3];

	buf[0]=-1;
	buf[1]=0;
	buf[2]=FONT1+attrREVERSE;
	api_lcd_clear(ut_dhn_lcd, buf);
}


// ---------------------------------------------------------------------------
// FUNCTION: Clear display row.
// INPUT   : row  - row number, 0..N.
//           cnt  - number of rows to be cleared.
//           font - font id & attribute.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UT_ClearRow(UCHAR row, UCHAR cnt, UCHAR font)
{
#ifdef _SCREEN_SIZE_128x64

UCHAR	buf[3];

	buf[0]=row;
	buf[1]=cnt;
	buf[2]=font;
	api_lcd_clear(ut_dhn_lcd, buf);

#else

API_LCDTFT_PARA para;
UCHAR	attr = 0;
UCHAR	dhn;


	dhn = ut_dhn_lcd;
	  
	memset( (UCHAR *)&para, 0x00, sizeof(API_LCDTFT_PARA) );
	para.Row = row;
	para.Col = cnt;
	
	switch( font & 0x0F )
	      {
	      case FONT0:
	      
          	   para.Font = LCDTFT_FONT0 + attr;
          	   para.FontHeight = TFTLCD_FONT0_H;
          	   para.FontWidth  = TFTLCD_FONT0_W;
	           break;
	           
	      case FONT1:

          	   para.Font = LCDTFT_FONT1 + attr;
          	   para.FontHeight = TFTLCD_FONT1_H;
          	   para.FontWidth  = TFTLCD_FONT1_W;
	           break;
	           
	      case FONT2:

          	   para.Font = LCDTFT_FONT2 + attr;
          	   para.FontHeight = TFTLCD_FONT2_H;
          	   para.FontWidth  = TFTLCD_FONT2_W;
	           break;
	            
	      default:

          	   para.Font = LCDTFT_FONT0 + attr;
          	   para.FontHeight = TFTLCD_FONT0_H;
          	   para.FontWidth  = TFTLCD_FONT0_W;
	           break;
	      }

	para.RGB = 0;

	if( attr & attrREVERSE )
	  {
	  para.BG_Palette[0] = 0x00;	// black
	  para.BG_Palette[1] = 0x00;
	  para.BG_Palette[2] = 0x00;
	  }
	else
	  {
	  para.BG_Palette[0] = Utils_Global_palette[0];
	  para.BG_Palette[1] = Utils_Global_palette[1];
	  para.BG_Palette[2] = Utils_Global_palette[2];
	  }       
	
	api_lcdtft_clear( dhn, para );

#endif
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
void	UT_PutChar(UCHAR row, UCHAR col, UCHAR font, UCHAR data)
{
UCHAR	sbuf[3];
UCHAR	dbuf[2];

	sbuf[0]=row;
	sbuf[1]=col;
	sbuf[2]=font;
	dbuf[0]=1;
	dbuf[1]=data;
	api_lcd_putstring(ut_dhn_lcd, sbuf, dbuf);
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
void	UT_PutStr(UCHAR row, UCHAR col, UCHAR font, UCHAR len, UCHAR *msg)
{
#ifdef _SCREEN_SIZE_128x64
UCHAR	sbuf[3];
UCHAR	dbuf[23];

	sbuf[0]=row;
	sbuf[1]=col;
	sbuf[2]=font;
	dbuf[0]=len;
	memmove(&dbuf[1], msg, len);
	api_lcd_putstring(ut_dhn_lcd, sbuf, dbuf);

#else

//UCHAR	sbuf[3];
UCHAR	dbuf[64] = {0};

API_LCDTFT_PARA para;
memset( (UCHAR *)&para, 0x00, sizeof(API_LCDTFT_PARA) );

	//sbuf[0]=row;
	//sbuf[1]=col;
	//sbuf[2]=font;
	dbuf[0]=len;
	memmove(&dbuf[1], msg, len);

	para.RGB = RGB_BPP16;
	para.Row = row;
	para.Col = col;

	if(font == FONT2)
	{
		para.Font = LCDTFT_FONT2;
		para.FontHeight = TFTLCD_FONT2_H;
		para.FontWidth = TFTLCD_FONT2_W;
	}
	else if(font == FONT1)
	{
		para.Font = LCDTFT_FONT1;
		para.FontHeight = TFTLCD_FONT1_H;
		para.FontWidth = TFTLCD_FONT1_W;
	}
	else if(font == FONT0)
	{
		para.Font = LCDTFT_FONT0;
		para.FontHeight = TFTLCD_FONT0_H;
		para.FontWidth = TFTLCD_FONT0_W;
	}
	else	// Set Default
	{
		para.Font = LCDTFT_FONT1;
		para.FontHeight = TFTLCD_FONT1_H;
		para.FontWidth = TFTLCD_FONT1_W;
	}

	para.FG_Palette[0] = 0x00;		// Black
	para.FG_Palette[1] = 0x00;
	para.FG_Palette[2] = 0x00;
	
	para.BG_Palette[0] = Utils_Global_palette[0];
	para.BG_Palette[1] = Utils_Global_palette[1];
	para.BG_Palette[2] = Utils_Global_palette[2];
	
	api_lcdtft_putstring(ut_dhn_lcd,para,dbuf);

#endif
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
void	UT_PutMsg(UCHAR row, UCHAR pos, UCHAR font, UCHAR len, UCHAR *msg)
{
UCHAR	sbuf[3];
UCHAR	dbuf[MAX_DSP_CHAR+1];
UCHAR	fontid;
UCHAR	col = 0;

	fontid = font & 0x0f;
	
	switch( fontid )
	      {
	      case FONT0:
	           col = MAX_DSP_WIDTH - len*6;
	           break;
	
	      case FONT1:
	      case FONT2:
	      case FONT3:
	           col = MAX_DSP_WIDTH - len*8;
	           break;
	      }
	
	switch( pos )
	      {
	      case COL_LEFTMOST:
	           col = 0;
	
	           break;
	
	      case COL_MIDWAY:
	
	           col /= 2;
//	           col /= 8; // convert to bytes
	
//                 if( fontid != FONT1 )
//                   {
                   if( col != 0 )
                     col--;
//                   }
	           font += attrPIXCOLUMN;
	
	           break;
	
	      case COL_RIGHTMOST:
	      
//	      	   col /= 8; // convert to bytes
	
	           if( row != 0 )
	             col--;
	           font += attrPIXCOLUMN;
	
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
// FUNCTION: convert HEX byte to ASCII word.
// INPUT   : data - hex data byte. (0x1A)
// OUTPUT    byte_h - the high byte of ASCII. (0x31)
//           byte_l - the low  byte of ASCII. (0x41)
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UT_hexb2ascw( UCHAR data, UCHAR *byte_h, UCHAR *byte_l )
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
// FUNCTION: swap the position of data (MSB to LSB)
// INPUT   : length - length of data bytes.
//           data   - the data.
// OUTPUT  : data   - the swapped data.
// RETURN  : none.
// ---------------------------------------------------------------------------
void UT_SwapData( UCHAR length, UCHAR *data )
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
UCHAR UT_bcd_add_bcd( UCHAR bcdlen, UCHAR *bcd1, UCHAR *bcd2 )
{
UCHAR i;
UCHAR digit1_h, digit1_l;
UCHAR digit2_h, digit2_l;
UCHAR carry;

      UT_SwapData( bcdlen, bcd1 ); // convert to LSB-MSB hex format
      UT_SwapData( bcdlen, bcd2 ); //

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
           digit1_l = 0;
           digit1_h += 1;
           }

         digit1_h += digit2_h;
         if( digit1_h > 9 )
           {
           digit1_h = 0;
           carry = 1; // carry for next higher digit
           }
         else
           carry = 0;

         bcd1[i] = (digit1_h << 4) + digit1_l;
         }

      UT_SwapData( bcdlen, bcd1 ); // return to "n" format

      if( carry == 1 )
        return( TRUE );
      else
        return( FALSE );
}


// ---------------------------------------------------------------------------
// FUNCTION: Dispaly a hex value of a byte data.
// INPUT   : row  - row position (0..x).
//           col  - col position (0..y).
//           data - the byte data.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UT_DispHexByte( UCHAR row, UCHAR col, UCHAR data )
{
//UCHAR	cc,c_hi,c_lo;	For Remove Warning
UCHAR	c_hi,c_lo;
UCHAR	sbuf[3], dbuf[3];

	UT_hexb2ascw( data, &c_hi, &c_lo );
	
	sbuf[0]=row;
	sbuf[1]=col;
	sbuf[2]=FONT0;
	
	dbuf[0]=2;
	dbuf[1]=c_hi;
	dbuf[2]=c_lo;
	api_lcd_putstring(ut_dhn_lcd, sbuf, dbuf);
}

// ---------------------------------------------------------------------------
// FUNCTION: Dispaly a hex value of a word data.
// INPUT   : row  - row position (0..x).
//           col  - col position (0..y).
//           data - the word data.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UT_DispHexWord( UCHAR row, UCHAR col, UINT data )
{
	UT_DispHexByte( row, col,   (UCHAR) ((data & 0x0ff00) >> 8) );
	UT_DispHexByte( row, col+2, (UCHAR) (data & 0x00ff) );
}


//	Copy From UT_DumpHexData
//	INPUT   : mode - 0x00=binary, 0x01=text for DISPLAY.
void	UT_DumpHex( UCHAR mode, UCHAR row, UINT length, UCHAR *data )
{
	UINT	i;
	UCHAR	col;
	UCHAR	st_row;


	if( (length == 0) || (row > 7) )
		return;
	  
	st_row = row;
	col=0;
	  
	UT_ClearRow( st_row, 8-st_row, FONT0 );
	  
	for(i=0; i<length; i++)
	{
		if( mode == 0 )
			UT_DispHexByte( row, col, data[i] );
		else
		{
			UT_PutChar( row, col, FONT0, data[i]);
			UT_PutChar( row, col+1, FONT0, 0x20);
		}
	  
		col += 3;
		if(col >= 19)
		{
			col = 0;
			row++;
			
			if(row >=8)
			{
				row = st_row;
				UT_WaitKey();
				UT_ClearRow( st_row, 8-st_row, FONT0 );
			}
		}
	}
	  
	UT_WaitKey();
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
void	UT_DumpHexData( UCHAR mode, UCHAR row, UINT length, UCHAR *data )
{
//	Original Code
/*
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
	       UT_hexb2ascw( data[i], &c_hi, &c_lo );
	       buf[0] = c_hi;
	       buf[1] = c_lo;
	       buf[3] = 0;
	       _DEBUGPRINTF( "%s", buf );
	       _DEBUGPRINTF( "%c", 0x20 );
	       }
	     else
	       {
	       _DEBUGPRINTF( "%c", data[i] );
	       _DEBUGPRINTF( "%c", 0x20 );
	       }
	     
	     col += 3;
	     if(col >= 48)	// 16 bytes per line
	       {
	       _DEBUGPRINTF( "\r\n" );
	       
	       col = 0;
	       st_row++;
	       if(st_row >=8)
	         {
	         st_row = 0;
	         
	         _DEBUGPRINTF( "       more...\r\n" );
	         UT_WaitKey();
	         }
	       }
	     }
	  
	  _DEBUGPRINTF( "\r\n\r\n" );
	  }
	else
	  {
	  if( (length == 0) || (row > 7) )
	    return;
	  
	  st_row = row;
	  col=0;
	  
	  UT_ClearRow( st_row, 8-st_row, FONT0 );
	  
	  for(i=0; i<length; i++)
	     {
	     if( mode == 0 )
	       UT_DispHexByte( row, col, data[i] );
	     else
	       {
	       UT_PutChar( row, col, FONT0, data[i]);
	       UT_PutChar( row, col+1, FONT0, 0x20);
	       }
	  
	     col += 3;
	     if(col >= 19)
	       {
	       col = 0;
	       row++;
	       if(row >=8)
	         {
	         row = st_row;
	         UT_WaitKey();
	         UT_ClearRow( st_row, 8-st_row, FONT0 );
	         }
	       }
	     }
	  
	  UT_WaitKey();
	  }
*/

//	For Remove Warning
	mode=mode;
	row=row;
	length=length;
	data=data;
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
void	UT_DumpHexData2( UCHAR mode, UCHAR row, UINT length, UCHAR *data )
{
//Original Code
/*
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
	       UT_hexb2ascw( data[i], &c_hi, &c_lo );
	       buf[0] = c_hi;
	       buf[1] = c_lo;
	       buf[3] = 0;
	       _DEBUGPRINTF( "%s", buf );
	       
	       col += 2;
	       maxcol = 47;
	       }
	     else
	       {
	       _DEBUGPRINTF( "%c", data[i] );
	       
	       col += 1;
	       maxcol = 48;
	       }
	     
	     if(col >= maxcol)	// 16 bytes per line
	       {
	       _DEBUGPRINTF( "\r\n" );
	       
	       col = 0;
	       st_row++;
	       if(st_row >=8)
	         {
	         st_row = 0;
	         
	         _DEBUGPRINTF( "       more...\r\n" );
	         UT_WaitKey();
	         }
	       }
	     }
	  
	  _DEBUGPRINTF( "\r\n\r\n" );
	  }
	else
	  {
	  if( (length == 0) || (row > 7) )
	    return;
	  
	  st_row = row;
	  col=0;
	  
	  UT_ClearRow( st_row, 8-st_row, FONT0 );
	  
	  for(i=0; i<length; i++)
	     {
	     if( mode == 0 )
	       {
	       UT_DispHexByte( row, col, data[i] );
	       
	       col += 2;
	       maxcol = 20;
	       }
	     else
	       {
	       UT_PutChar( row, col, FONT0, data[i]);
	  
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
	         UT_WaitKey();
	         UT_ClearRow( st_row, 8-st_row, FONT0 );
	         }
	       }
	     }
	  
	  UT_WaitKey();
	  }
*/

//	For Remove Warning
	mode=mode;
	row=row;
	length=length;
	data=data;
}

// ---------------------------------------------------------------------------
// FUNCTION: Wait for N*10ms.
// INPUT   : ten_ms - unit in step of ten mini-second. (1 sec = 100 unit)
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UT_WaitTime( UINT tenms )
{
/*
UCHAR	dhn_tim;
UINT	tick1, tick2;

	dhn_tim = api_tim_open(1); // 1 tick unit = 10 ms
	api_tim_gettick( dhn_tim, (UCHAR *)&tick1 );
	
	do{
	  api_tim_gettick( dhn_tim, (UCHAR *)&tick2 );
	  } while( tick2 - tick1 < tenms );
	
	api_tim_close( dhn_tim );
*/

ULONG	tick1, tick2;

	//Timer Setting
	os_SysTimerFreeCnt=0;
	tick1=os_SysTimerFreeCnt;
	
	do{
	  	//Get Timer Value
		tick2=os_SysTimerFreeCnt;
	} while((tick2 - tick1) < tenms);
}

void UT_Wait(ULONG timUnit)
{
	UCHAR	dhn;

	dhn = api_tim2_open( timUnit );

	if (dhn == apiOutOfService)
	{
		return;
	}
	
	while( api_tim2_status( dhn ) != apiReady );
	api_tim2_close( dhn );
}

// ---------------------------------------------------------------------------
// FUNCTION: Wait for N*10ms or exit if any key is pressed.
// INPUT   : ten_ms - unit in step of ten mini-second. (1 sec = 100 unit)
// OUTPUT  : none.
// RETURN  : 0xFF   = time is up without any key pressed.
//           others = the key code pressed within timeout.
// ---------------------------------------------------------------------------
UCHAR	UT_WaitTimeAndKey( UINT tenms )
{
UCHAR	dhn_tim;
UINT 	tick1, tick2;
UCHAR	buf[1];

	dhn_tim = api_tim_open(1); // 1 tick unit = 10 ms
	api_tim_gettick( dhn_tim, (UCHAR *)&tick1 );
	
	do{
	  api_tim_gettick( dhn_tim, (UCHAR *)&tick2 );
	
	  if( api_kbd_status( ut_dhn_kbd, buf) == apiReady )
	    {
	    api_tim_close( dhn_tim );
	    return( UT_WaitKey() );
	    }
	
	  } while( tick2 - tick1 < tenms );
	
	api_tim_close( dhn_tim );
	return( 0xFF );
}

// ---------------------------------------------------------------------------
// FUNCTION: Open keyboard - UP, DOWN, ENTER, CANCEL.
// INPUT   : none.
// OUTPUT  : g_dhn_kbd
// RETURN  : none.
// ---------------------------------------------------------------------------
void UT_OpenKeySelect()
{
UCHAR buf[5];

      buf[0]=0x020;
      buf[1]=0x000;
      buf[2]=0x020;
      buf[3]=0x024;
      buf[4]=0x000;
      ut_dhn_kbd = api_kbd_open(0, buf);
}

// ---------------------------------------------------------------------------
// FUNCTION: Open printer.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UT_OpenPrinter(void)
{
	ut_dhn_prt = api_prt_open( prtThermal, 0 );
}

// ---------------------------------------------------------------------------
// FUNCTION: Close printer.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	UT_ClosePrinter(void)
{
	api_prt_close( ut_dhn_prt );
}

// ---------------------------------------------------------------------------
// FUNCTION: Print one string with default ASCII FONT0.
// INPUT   : FormFeed     - TRUE if the last character of the string is a form-feed (0x0C).
//	     str          - the string to be printed, format: LEN(1)+STR(n)
// OUTPUT  : status       - final status of printer.
// RETURN  : TRUE  - OK
//	     FALSE - exception.
// ---------------------------------------------------------------------------
UCHAR	UT_PrintString( UCHAR FormFeed, UCHAR FontID, UCHAR *str, UCHAR *status )
{
UCHAR	result;


	do{
	  result = api_prt_putstring( ut_dhn_prt, FontID, str );
	  if( result == apiNotReady )
	    {
	    api_prt_status( ut_dhn_prt, status );
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
	      api_prt_status( ut_dhn_prt, status );
	      } while( (*status != prtComplete) && (*status != prtPaperEmpty) && (*status != prtBufferFull) );
	    }
	  
	  return( TRUE );
	  }
}

// ---------------------------------------------------------------------------
// FUNCTION: Print one graphic bitmap.
// INPUT   : dim          - dimension of the bitmap.
//	     bmp	  - the bitmap to be printed.
// OUTPUT  : status       - final status of printer.
// RETURN  : TRUE  - OK
//	     FALSE - exception.
// ---------------------------------------------------------------------------
UCHAR	UT_PrintGraphics( API_GRAPH dim, UCHAR *bmp,  UCHAR *status )
{
UCHAR	result;


	do{
	  result = api_prt_putgraphics( ut_dhn_prt, dim, bmp );
	  if( result == apiNotReady )
	    {
	    api_prt_status( ut_dhn_prt, status );
	    if( (*status == prtPaperEmpty) || (*status == prtBufferFull) )
	      break;
	    }
	  } while( result != apiOK );	// waiting for the end of process
	       
	if( result == apiOK )
	  return( TRUE );
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: Get keyin number for item selection.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : -1 if "CANCEL" key is pressed.
//	     other values for the keyin number.
// ---------------------------------------------------------------------------
int	UT_GetKeyInNumber( UCHAR row, UCHAR font )
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
	     keyin = UT_WaitKey();
	     if( FirstEntry )
	       {
	       FirstEntry = FALSE;
	       UT_ClearRow( row, 1, font );
	       }
	     
	     if( keyin == 'x' )	// CANCEL
	       return(-1);
	     
	     if( keyin == 'y' )	// ENTER
	       break;

	     if( keyin == 'n' )	// CLEAR
	       {
	       UT_ClearRow( row, 1, font );
	       memset( keybuf, 0x00, sizeof(keybuf) );
	       index = 0;
	       
	       continue;
	       }
	     
	     if( index >= 2 )	// check max digits
	       continue;
	     
	     if( (keyin >= '0') && (keyin <= '9') )
	       {
	       UT_PutChar( row, index, font, keyin );
	       keybuf[index++] = keyin;
	       }
	     }
	
	if( index != 0 )
	  {
	  // convert ASCII string to an integer
	  keybuf[index] = '\0';
	  result = atoi( keybuf );
	  }
	else
	  result = 0;
	
	return( result );

}

UCHAR UT_GetKeyInAscii( UCHAR row, UCHAR font , UCHAR digLimit, UCHAR *ascNum)
{
	int		result;
	UCHAR	FirstEntry = TRUE;
	UCHAR	keyin;
	UCHAR	keybuf[16];
	UCHAR	index;

	memset( keybuf, 0x00, sizeof(keybuf) );
	index = 0;
	result = -1;
	
	while(1)
	{
		keyin = UT_WaitKey();
		if( FirstEntry )
	       {
	       	FirstEntry = FALSE;
	       	UT_ClearRow( row, 1, font );
	       }
	     
		if( keyin == 'x' )	// CANCEL
	       	return FAIL;
	     
		if( keyin == 'y' )	// ENTER
			break;

		if( keyin == 'n' )	// CLEAR
	       {
			UT_ClearRow( row, 1, font );
			memset( keybuf, 0x00, sizeof(keybuf) );
			index = 0;
		       
		       continue;
	       }

		if (keyin == 'z')	//ALPHA
		{
			UT_ClearRow(row, 1, font);
			UT_SwitchAlphanumericKey(&keybuf[index-1]);
			UT_PutStr(row, 0, font, index, keybuf);
		}
		
		if( index >= digLimit)	// check max digits
			continue;
	     
		if( (keyin >= '0') && (keyin <= '9') )
	       {
			UT_PutChar( row, index, font, keyin );
			keybuf[index++] = keyin;
	       }
	}
	
	if( index != 0 )
		memcpy(ascNum, keybuf, digLimit);
	else
		result = 0;

	return SUCCESS;
}



//Copy from AGR
UCHAR UT_GetKey(void)
{
	UCHAR keyin;

	if(!api_kbd_status(ut_dhn_kbd, &keyin))
	{
		api_kbd_getchar(ut_dhn_kbd, &keyin);
		return keyin;
	}
	else
		return FAIL;
}


//// ---------------------------------------------------------------------------
//// FUNCTION: show digit string with right-justified. (max 20 digits)
//// INPUT   : type - bit 7 6 5 4 3 2 1 0
////                              | | | |_ pure digits (NNNNNN)
////                              | | |___ with sperator (NN,NNN)
////                              | |_____ with decimal point (NNN.NN) cf. currency exponent.
////                              |_______ special prompt (eg. ****) cf.
////           buf  - L-V, the char string.
//// OUTPUT  : buf  - L-V, with the decimal point if necessary.
//// REF     : EMVDC_term_tx_exp, EMVDC_term_decimal_point.
//// RETURN  : the transaction exponent value (0,2 or 3)
//// ---------------------------------------------------------------------------
//UCHAR UT_insert_decimal_point( UCHAR type, UCHAR *buf )
//{
////20130423 Remark UCHAR i; for Removing Warning
////UCHAR i;
//UCHAR exp;
//UCHAR data[30];
//UCHAR len1;
//UCHAR len2;
//
//      // get tranaction currency exponent
////    api_emv_GetDataElement( DE_TERM, ADDR_TERM_TX_CE, 1, exp );
//      if( EMVDC_term_decimal_point ) // show decimal point?
//        exp = EMVDC_term_tx_exp;
//      else
//        exp = 0;
//
//      // with decimal attribute & exponent ? (the exponent will be 2 or 3)
//      if( (type & 0x04) && (exp != 0) )
//        {
//        len1 = buf[0];
//
//        if( len1 == 0 )
//          {
//          // set to default empty value '0'
//          len1 = 1;
//          buf[0] = 1;
//          buf[1] = '0';
//          }
//
//        // arrange decimal point
//        // case 1: the char length < exponent (1 -> 0.01)
//        // case 2: the char length = exponent (12 -> 0.12)
//        // case 3: the char length > exponent (123 -> 1.23)
//        if( len1 <= exp )
//          {
//          if( len1 == exp )
//            len2 = 0;
//          else
//            {
//            len2 = exp - len1;
//            memset( &data[3], 0x30, len2 );
//            }
//
//          data[1] = '0';
//          data[2] = '.';
//          memmove( &data[3+len2], &buf[1], len1 );
//          data[0] = len1 + len2 + 2;
//          }
//        else
//          {
//          len2 = len1 - exp; // length of integer part
//          memmove( &data[1], &buf[1], len2 ); // set integer part
//          data[1+len2] = '.';
//          memmove( &data[2+len2], &buf[1+len2], exp ); // set decimal part
//          data[0] = len1 + 1;
//          }
//
//        memmove( buf, data, data[0]+1 );
//        }
//
////    TL_trim_asc( 0, buf, data );     // to trim the case: 00000.00 to 0.00
////    memmove( buf, data, data[0]+1 );
//
//      return( exp );
//}



//// ---------------------------------------------------------------------------
//// FUNCTION: show digit string with right-justified. (max 20 digits)
//// INPUT   : row  - row position to show.
////           buf  - L-V, the digit string.
////           type -
////           idle - character to be shown when idle. (eg. '0','-'...)
//// OUTPUT  : none.
//// RETURN  : none.
//// ---------------------------------------------------------------------------
//void UT_ShowKeyIn( UCHAR type, UCHAR idle, UCHAR font, UCHAR row, UCHAR *buf )
//{
//UCHAR i;
//UCHAR len;
//UCHAR index;
//UCHAR max_dsp_cnt;
//UCHAR newbuf[30];
//
//      memmove( newbuf, buf, buf[0]+1 );
//
//      UT_insert_decimal_point( type, newbuf ); // put decimal point if necessary
//
//      UT_ClearRow( row, 1, font );
//
//      if( (font & 0x0f) == FONT0 )
//        max_dsp_cnt = MAX_DSP_FONT0_CNT;
//      else
//        max_dsp_cnt = MAX_DSP_FONT1_CNT;
//
//      // check non-zero idle prompt
//      if( (buf[0] == 0) && (idle != '0') )
//        {
//        newbuf[1] = idle;
//        UT_PutStr( row, max_dsp_cnt-1, font, 1, &newbuf[1] );
//        return;
//        }
//
//      if( newbuf[0] == 0 ) // no data in buffer, show '0'
//        {
//        newbuf[1] = idle;
//        UT_PutStr( row, max_dsp_cnt-1, font, 1, &newbuf[1] );
//        }
//      else
//        { // check special prompt
//        if( (type & NUM_TYPE_STAR) != 0 )
//          {
//          type &= 0xf7;
//
//          for( i=0; i<newbuf[0]; i++ )
//             newbuf[i+1] = '*';
//          }
//
//        if( (type & NUM_TYPE_COMMA) == 0 )
//          { // NNNNNNNNNNNNNNNN...N
//SHOW_NORM:
//          if( (max_dsp_cnt-newbuf[0]) > 0 )
//            UT_PutStr( row, max_dsp_cnt-newbuf[0], font, newbuf[0], &newbuf[1] );
//          else
//            {
//            index = newbuf[0]-max_dsp_cnt + 1;
//            UT_PutStr( row, 0, font, max_dsp_cnt, &newbuf[index] );
//            }
//          }  //  9   6   3   0   7   4
//        else // NN,NNN,NNN,NNN,NNN,NNN,NNN
//          {
//          len = newbuf[0];
//          if( len < 4 )
//            goto SHOW_NORM;
//          // to be implemented!
//          }
//        }
//}


//// ---------------------------------------------------------------------------
//// FUNCTION: get keys & show numeric digits.
//// INPUT   : tout - time out before ENTER key pressed. (in seconds, 0=always)
////           row  - row position to show.
////           len  - max. length allowed.
////           type - bit 7 6 5 4 3 2 1 0
////                            | | | | |_ pure digits (NNNNNN)
////                            | | | |___ with sperator (NN,NNN)
////                            | | |_____ with decimal point (NNN.NN) cf. currency exponent.
////                            | |_______ special prompt (eg. ****) cf.
////                            |_________ accept leading '0' (eg. "0123", rather "123")
////
////           idle - character to be shown when idle. (eg. '0','-'...)
//// OUTPUT  : buf - the final result. (sizeof(buf) = len+1)
////                 format: LEN[1] DIGIT[n]
//// REF     :
//// RETURN  : TRUE  = confirmed. (by entering OK)
////           FALSE = aborted.   (by entering CANCEL)
////           -1    = timeout.
//// ---------------------------------------------------------------------------
//UCHAR UT_GetNumKey( UINT tout, UCHAR type, UCHAR idle, UCHAR font, UCHAR row, UCHAR len, UCHAR *buf )
//{
//UCHAR i;
//UCHAR key;
//UCHAR dhn_tim;
//UINT  tick1, tick2;
//
//      UT_OpenKeyNum(); // enable numeric keypad
//
//      dhn_tim = api_tim_open(100); // time tick = 1sec
//      api_tim_gettick( dhn_tim, (UCHAR *)&tick1 );
//
//KEYIN_CLEAR:
//
//      for( i=0; i<=len; i++ )
//         buf[i] = 0;  // clear output buffer
//
//      i = 1;
//      buf[0] = 0;
//      key = 0;
//
//      while(1)
//           {
//           if(i == 1)
//             UT_ShowKeyIn( type, idle, font, row, buf );
//KEYIN_WAIT:
//           if( tout != 0 )
//             {
//             // wait key
//             do{
//               // check timeout
//               api_tim_gettick( dhn_tim, (UCHAR *)&tick2 );
//               if( (tick2 - tick1) >= tout )
//                 {
//                 api_tim_close( dhn_tim );
//                 return( -1 );
//                 }
//               } while( UT_GetKeyStatus() == apiNotReady );
//             }
//           key = UT_WaitKey();
//
//           switch(key)
//                 {
//                 case 'x': // cancel
//                      for( i=0; i<=len; i++ )
//                         buf[i] = 0;
//
//                      api_tim_close( dhn_tim );
//                      return( FALSE );
//					  
////20130423 Remark KEYIN_NO: for Removing Warning
////KEYIN_NO:
//                 case 'n': // clear
//                      UT_ClearRow(row, 1, FONT1);
//                      i = 1;
//                      buf[0] = 0;
//                      goto KEYIN_CLEAR;
//
////20130423 Remark KEYIN_YES: for Removing Warning
////KEYIN_YES:
//                 case 'y': // ok
//                //    if( i == 1 )
//                //      continue;
//
//                      if( buf[0] == 0 )
//                        {
//                        buf[0] = 1;
//                        buf[1] = '0';
//                        }
//
//                      api_tim_close( dhn_tim );
//                      return( TRUE );
//
//                 case '#': // backspace
//                      if( i != 1 )
//                        {
//                        buf[--i] = 0;
//                        buf[0] -= 1;
//                        UT_ShowKeyIn( type, idle, font, row, buf );
//                        continue;
//                        }
//                      else
//                        goto KEYIN_CLEAR;
//
//                 default: // '0'~'9'
//                      if( i > len ) // overflow?
//                        continue;
//
//                //    if( (type & NUM_TYPE_STAR) == 0 )
//                      if( (type & NUM_TYPE_LEADING_ZERO) == 0 )
//                        {
//                        if( (key == '0') && (buf[0] == 0) )
//                          {
//                          buf[0] = 1;
//                          buf[1] = '0';
//                          UT_ShowKeyIn( type, idle, font, row, buf );
//                          buf[0] = 0;
//                          goto KEYIN_WAIT;
//                          }
//                        }
//
//                      buf[0] = i; // length of current keyin
//
//                //    if( (type & 0x08) != 0 ) // check special prompt
//                //      {
//                //      type &= 0xf7;
//                //      key = '*';
//                //      }
//
//                      buf[i++] = key;
//                      UT_ShowKeyIn( type, idle, font, row, buf );
//
//                } // switch(key)
//          } // while(1)
//}

//// ---------------------------------------------------------------------------
//// FUNCTION: open AUX device for communicaiton. (9600, 8, n, 1)
//// INPUT   : port - port number.
//// OUTPUT  : none.
//// REF     : g_dhn_aux
//// RETURN  : TRUE  - OK
////           FALSE - device error.
//// ---------------------------------------------------------------------------
//UCHAR UT_OpenAUX( UCHAR port )
//{
//API_AUX	pAux;
//
//      pAux.Mode = auxSOH;
//      pAux.Baud = COM_115200 + COM_CHR8 + COM_NOPARITY + COM_STOP1;
//      pAux.Tob = 100;	// 1 sec
//      pAux.Tor = 200;	// 2 sec
//      pAux.Acks = 1;	//
//      pAux.Resend = 0;	// no retry
//      EMVDC_dhn_aux = api_aux_open( port, pAux );
//
//      if( (EMVDC_dhn_aux == apiOutOfLink) || (EMVDC_dhn_aux == apiOutOfService) )
//        {
//        api_aux_close(0);
//        return( FALSE );
//        }
//      else
//        return( TRUE );
//}
//
//// ---------------------------------------------------------------------------
//// FUNCTION: close AUX device.
//// INPUT   : none.
//// OUTPUT  : none.
//// REF     : g_dhn_aux
//// RETURN  : TRUE  - OK
////           FALSE - device error.
//// ---------------------------------------------------------------------------
//UCHAR UT_CloseAUX( void )
//{
//UCHAR result;
//
//      UT_WaitTime(10);	// PATCH: 2008-12-01, wait for end of sending ACK to HOST
//
//      result = api_aux_close( EMVDC_dhn_aux );
//      if( result != apiOK )
//        {
//        api_aux_close(0);
//        return( FALSE );
//        }
//      else
//        return( TRUE );
//}
//
//// ---------------------------------------------------------------------------
//// FUNCTION: check data availability from AUX port.
//// INPUT   : none.
//// OUTPUT  : none.
//// RETURN  : TRUE  - data ready.
////           FALSE - device error or timeout before data ready.
//// ---------------------------------------------------------------------------
//UCHAR	UT_CheckAUX( void )
//{
//UCHAR	buf[8];
//
//	if( api_aux_rxready( EMVDC_dhn_aux, buf ) == apiReady )
//	  return( TRUE );
//	else
//	  return( FALSE );
//}
//
//// ---------------------------------------------------------------------------
//// FUNCTION: receive data from AUX port.
//// INPUT   : none.
//// OUTPUT  : data - 2L-V, the data receivd.
//// REF     : g_dhn_aux
//// RETURN  : TRUE  - data ready.
////           FALSE - device error or timeout before data ready.
//// ---------------------------------------------------------------------------
//UCHAR UT_DataReceiveAUX( UCHAR *data )
//{
//UCHAR dhn_tim;
//UINT  tick1, tick2;
//
//      dhn_tim = api_tim_open(100); // time tick = 1sec
//      api_tim_gettick( dhn_tim, (UCHAR *)&tick1 );
//
//      do{
//        api_tim_gettick( dhn_tim, (UCHAR *)&tick2 );
//
//        if( api_aux_rxready( EMVDC_dhn_aux, data ) == apiReady )
//          {
//          api_aux_rxstring( EMVDC_dhn_aux, data );
//
//          api_tim_close( dhn_tim );
//          return( TRUE );
//          }
//        } while( (tick2 - tick1) <= 15 );
//
//      api_tim_close( dhn_tim );
//      return( FALSE );
//}
//
//
//// ---------------------------------------------------------------------------
//// FUNCTION: receive data from AUX port.
//// INPUT   : none.
//// OUTPUT  : data - 2L-V, the data receivd.
//// REF     : g_dhn_aux
//// RETURN  : TRUE  - data ready.
////           FALSE - device error or timeout before data ready.
//// ---------------------------------------------------------------------------
//UCHAR UT_ReceiveAUX( UCHAR *data )
//{
//UCHAR dhn_tim;
//UINT  tick1, tick2;
//
//      dhn_tim = api_tim_open(100); // time tick = 1sec
//      api_tim_gettick( dhn_tim, (UCHAR *)&tick1 );
//
//      do{
//        api_tim_gettick( dhn_tim, (UCHAR *)&tick2 );
//
//        if( api_aux_rxready( EMVDC_dhn_aux, data ) == apiReady )
//          {
//          api_aux_rxstring( EMVDC_dhn_aux, data );
//
//          api_tim_close( dhn_tim );
//          return( TRUE );
//          }
//        } while( (tick2 - tick1) <= 5 );
//
//      api_tim_close( dhn_tim );
//      return( FALSE );
//}
//
//// ---------------------------------------------------------------------------
//// FUNCTION: transmit data to AUX port.
//// INPUT   : data - 2L-V, the data to be transmitted.
//// OUTPUT  : none.
//// REF     : g_dhn_aux
//// RETURN  : TRUE  - data ready.
////           FALSE - device error.
//// ---------------------------------------------------------------------------
//UCHAR UT_TransmitAUX( UCHAR *data )
//{
//UCHAR dhn_tim;
//UINT  tick1, tick2;
//UCHAR result;
//
//      dhn_tim = api_tim_open(100); // time tick = 1sec
//      api_tim_gettick( dhn_tim, (UCHAR *)&tick1 );
//
//      do{
//        api_tim_gettick( dhn_tim, (UCHAR *)&tick2 );
//
//        if( api_aux_txready( EMVDC_dhn_aux ) == apiReady )
//          {
//
//          api_aux_txstring( EMVDC_dhn_aux, data );
//
//          // wait for ACK from HOST
//          do{
//            api_tim_gettick( dhn_tim, (UCHAR *)&tick2 );
//
//            result = api_aux_txready( EMVDC_dhn_aux );
//            if( result == apiFailed)
//              break;
//
//            if( result == apiReady )
//              {
//              api_tim_close( dhn_tim );
//              return( TRUE );
//              }
//            } while( (tick2 - tick1) <= 3 );
//
//          break;
//          }
//        } while( (tick2 - tick1) <= 3 );
//
//      api_tim_close( dhn_tim );
//      return( FALSE );
//}


// ---------------------------------------------------------------------------
// FUNCTION: convert integer to ASCII string.
// INPUT   : value - the integer.
// OUTPUT  : abuf  - the ASCII string.
// RETURN  : the length of the output string in bytes.
// ---------------------------------------------------------------------------
UCHAR	UT_itoa( UINT value, UCHAR *abuf )
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


void UT_1hexto3asc(UCHAR data,UCHAR * ascout)
{
	UCHAR CC = 0;

	CC = data;

	ascout[0] = (CC/100) + 0x30;

	CC = CC%100;
	
	ascout[1] = (CC/10) + 0x30;

	CC = CC%10;

	ascout[2] = CC + 0x30;	
}


UCHAR UT_bcdcmp(UCHAR *bcdOne, UCHAR *bcdTwo, UCHAR cmpLen)
{	//Copy from AS320AGR
	UCHAR idx=0;

	for (idx=0; idx<cmpLen; idx++, bcdOne++, bcdTwo++)
	{
		if ((*bcdOne) > (*bcdTwo))
			return 1;		//bcdOne Larger
		else if ((*bcdOne) < (*bcdTwo))
			return 2;		//bcdTwo Larger
	}

	return 0;				//Equal
}

UINT UT_pow(UCHAR numBase, UCHAR numPow)
{
	UCHAR i=0;
	UINT numTemp=1;

	if (numPow != 0)
	{
		for (i=0; i<numPow; i++)
			numTemp*=numBase;
	}
	
	return numTemp;
}

UCHAR UT_SubSplit(UCHAR data)
{
	 return ( data > 9) ? (data-9+'@') : (data+'0');
}

void UT_Split(UCHAR *des, UCHAR *src, char pair_len)
{
	UCHAR cnt;

	for (cnt=0; cnt < pair_len; cnt++, src++)
	{
		*des++ = UT_SubSplit((*src & 0xF0) >> 4);
		*des++ = UT_SubSplit(*src & 0x0F);
	}
}

UCHAR UT_SubCompress(UCHAR data)
{
	UCHAR factor;

	if ((data>='0') && (data<='?'))
		factor = '0';
	else if ((data>='A') && (data<='F'))
		factor = '7';
	else
		factor = FALSE;

	return ( factor ? (data - factor) : 0x0F);
}

void UT_Compress(UCHAR *des, UCHAR *src, UCHAR pair_len)
{
	UCHAR len;

	for (len=0; len < pair_len; len++)
	{
		*des = UT_SubCompress(*src++) << 4;
		*des++ += UT_SubCompress(*src++);
	}
}

ULONG UT_ASC2INT(UCHAR *iptAscii)
{
	ULONG	tmpInt=0;
	UCHAR	idxNum=0;
	ULONG	idxBit=1;

	for (idxNum=0; idxNum<10; idxNum++)
	{
		tmpInt+=(iptAscii[9-idxNum]&0x0F)*idxBit;
		idxBit*=10;
	}

	return tmpInt;
}

void UT_INT2ASC(ULONG iptInt, UCHAR *optAscii)
{
	ULONG	tmpInt;
	UCHAR	idxNum=0;

	memset(optAscii, '0', 10);
	tmpInt=iptInt;

	for (idxNum=0; idxNum<10; idxNum++)
	{
		optAscii[9-idxNum]+=(tmpInt % 10);
		tmpInt/=10;

		if (tmpInt == 0)
			break;
	}
}

ULONG UT_UCHAR2ULONG(UCHAR lenUCHAR, UCHAR *iptUCHAR)
{
	ULONG	tmpValue=0;	//Temporary Value
	ULONG	idxValue=0;	//Index Value
	UCHAR	idxNum=0;	//Index Number

	idxValue=0x01000000;//Set Default Value(16,777,216)

	for (idxNum=0; idxNum<lenUCHAR; idxNum++)
	{
		tmpValue+=idxValue*iptUCHAR[idxNum];
		idxValue>>=8;
	}

	return tmpValue;
}

/*
*	20130321	Modify from UT_time
*
*	Input: 12 Bytes Ascii [YYMMDDHHMMSS]
*/
ULONG UT_Get_UnixTime(UCHAR *iptDatetime)	
{
	UCHAR	temp[3]={0};
	struct	tm ltime;
	int		year;
	UINT	mon;
	UINT	day;
	UINT	hour;
	UINT	min;
	UINT	sec;
	ULONG	tseconds;
	
	// convert ASCII to binary
	temp[0] = iptDatetime[0];
	temp[1] = iptDatetime[1];

	year = atoi( temp );
	if( (year >= 0x00) && (year <= 0x49) )
	  year += 2000;
	else
	  year += 1900;
	
	temp[0] = iptDatetime[2];
	temp[1] = iptDatetime[3];
	mon = atoi( temp );
	
	temp[0] = iptDatetime[4];
	temp[1] = iptDatetime[5];
	day = atoi( temp );
	
	temp[0] = iptDatetime[6];
	temp[1] = iptDatetime[7];
	hour = atoi( temp );
	
	temp[0] = iptDatetime[8];
	temp[1] = iptDatetime[9];
	min = atoi( temp );
	
	temp[0] = iptDatetime[10];
	temp[1] = iptDatetime[11];
	sec = atoi( temp );
	
	ltime.tm_year = year-1900;
	ltime.tm_mon = mon - 1;
	ltime.tm_mday = day;
	ltime.tm_hour = hour;
	ltime.tm_min = min;
	ltime.tm_sec = sec;
	tseconds = (ULONG)mktime( &ltime );	// total seconds elapsed since 1970

	return tseconds;
}

UCHAR UT_Get_TLVLengthOfT(UCHAR *iptDataOfT, UCHAR *optLenOfT)
{
	UINT	cntIdx=0;

	if (iptDataOfT[0] != 0)
	{
		for (cntIdx=0; cntIdx < 256; cntIdx++)
		{
			if (cntIdx == 0)
			{
				if ((iptDataOfT[0] & 0x1F) != 0x1F)	//First Byte & No subsequent bytes
				{
					optLenOfT[0]=1;
					return SUCCESS;
				}
			}
			else
			{
				if ((iptDataOfT[cntIdx] & 0x80) == 0x00)	//Last tag byte
				{
					optLenOfT[0]=cntIdx+1;
					return SUCCESS;
				}
			}
		}
	}

	optLenOfT[0]=0;
	return FAIL;
}

//UCHAR UT_Get_TLVLengthOfL(UCHAR *iptDataOfL, UCHAR *optLenOfL)
//{
//	UCHAR	rspCode=0;
//	UINT	lenOfV=0;
//
//	rspCode=apk_EMVCL_GetBERLEN(iptDataOfL, optLenOfL, &lenOfV);
//
//	return rspCode;
//}
//
//UCHAR UT_Get_TLVLengthOfV(UCHAR *iptDataOfL, UINT *optLenOfV)
//{
//	UCHAR	rspCode=0;
//	UCHAR	lenOfL=0;
//
//	rspCode=apk_EMVCL_GetBERLEN(iptDataOfL, &lenOfL, optLenOfV);
//
//	return rspCode;
//}
//
//UCHAR UT_Get_TLVLength(UCHAR *iptDataOfT, UCHAR *optLenOfT, UCHAR *optLenOfL, UINT *optLenOfV)
//{
//	UCHAR	rspCode=0;
//
//	rspCode=UT_Get_TLVLengthOfT(iptDataOfT, optLenOfT);
//	if (rspCode == SUCCESS)
//	{
//		rspCode=apk_EMVCL_GetBERLEN(iptDataOfT+optLenOfT[0], optLenOfL, optLenOfV);
//		if (rspCode == SUCCESS)
//			return SUCCESS;
//	}
//
//	return FAIL;
//}

UCHAR UT_Set_TagLength(UINT iptLength, UCHAR *optLength)
{
	if (iptLength < 128)
	{
		optLength[0]=iptLength;

		return 1;
	}

	if (iptLength < 256)
	{
		optLength[0]=0x81;
		optLength[1]=(iptLength & 0x00FF);

		return 2;
	}

	optLength[0]=0x82;
	optLength[1]=(iptLength & 0xFF00) >> 8;
	optLength[2]=(iptLength & 0x00FF);

	return 3;
}

UCHAR UT_Search(
	UCHAR	keyStrSize,	//Size of Key String
	UCHAR	*keyString,	//Pointer of Key String
	UCHAR	*srhString,	//Pointer of Search String
	ULONG	recNumber,	//Number of Record
	UCHAR	recSize,	//Size of One Record
	ULONG	*optIndex)	//Output Index
{
	ULONG	idxNumber=0;

	for (idxNumber=0; idxNumber<recNumber; idxNumber++)
	{
		if (!memcmp(keyString, &srhString[idxNumber*recSize], keyStrSize))
		{
			optIndex[0]=idxNumber;
			return SUCCESS;
		}
	}

	return FAIL;	//Not Found
}

UCHAR UT_Search_Record(
	UCHAR	keyStrSize,	//Size of Key String
	UCHAR	*keyString,	//Pointer of Key String
	UCHAR	*srhString,	//Pointer of Search String
	UINT	recNumber,	//Number of Record
	UCHAR	recSize,	//Size of One Record
	UINT	*optIndex)	//Output Index
{
	UINT	smlIndex=0;	//Index of Small Value
	UINT	midIndex=0;	//Index of Middle Value
	UINT	lrgIndex=0;	//Index of Large Value
	ULONG	keyValue=0;	//Key Value Convert from Key String
	ULONG	srhValue=0;	//Search Value Convert from Search String


	if (keyStrSize > 4)
		return FAIL;

	lrgIndex=recNumber-1;

	keyValue=UT_UCHAR2ULONG(keyStrSize, keyString);

	while (smlIndex <= lrgIndex)
	{
		midIndex=(smlIndex+lrgIndex)/2;

		srhValue=UT_UCHAR2ULONG(keyStrSize, &srhString[midIndex*recSize]);

		if (srhValue == keyValue)
		{
			optIndex[0]=midIndex;
			return SUCCESS;		//Found
		}
		else if (srhValue > keyValue)
		{
			if (midIndex == 0)
				return FAIL;	//Not Found
			else
				lrgIndex=midIndex-1;
		}
		else //(srhValue < keyValue)
			smlIndex=midIndex+1;
	}

	return FAIL;	//Not Found
}

//UCHAR *UT_Search_TLV(UCHAR *iptTag, UINT tlvNumber, UCHAR *tlvData, UINT *optIndex)
//{
//	UCHAR	rspCode=0;
//	UCHAR	lenOfT=0;
//	UCHAR	lenOfL=0;
//	UINT	lenOfV=0;
//	UINT	idxNum=0;
//
//	for (idxNum=0; idxNum<tlvNumber; idxNum++)
//	{
//		//Get T Length
//		rspCode=UT_Get_TLVLengthOfT(tlvData, &lenOfT);
//		if (rspCode == FAIL)
//			return NULLPTR;
//
//		//Compare with Input Tag
//		rspCode=memcmp(iptTag, tlvData, lenOfT);
//		if (rspCode == 0)
//		{
//			optIndex[0]=idxNum;
//			return tlvData;
//		}
//		else
//		{
//			//Get L Length
//			rspCode=UT_Get_TLVLengthOfL(tlvData+lenOfT, &lenOfL);
//			if (rspCode == FAIL)
//				return NULLPTR;
//
//			//Get V Length
//			rspCode=UT_Get_TLVLengthOfV(tlvData+lenOfT, &lenOfV);
//			if (rspCode == FAIL)
//				return NULLPTR;
//
//			//Point to Next Tag
//			tlvData+=(lenOfT+lenOfL+lenOfV);
//		}
//	}
//
//	return NULLPTR;
//}

UCHAR *UT_Search_ListItem(UCHAR *iptTag, UINT lstNumber, UCHAR *lstData, UINT *optIndex)
{
	UCHAR	rspCode=0;
	UCHAR	lenOfT=0;
	UINT	idxNum=0;

	for (idxNum=0; idxNum<lstNumber; idxNum++)
	{
		//Get List Tag Length
		rspCode=UT_Get_TLVLengthOfT(lstData, &lenOfT);

		//Compare with Input Tag
		rspCode=memcmp(iptTag, lstData, lenOfT);
		if (rspCode == 0)
		{
			optIndex[0]=idxNum;
			return lstData;
		}
		else
			lstData+=lenOfT;	//Point to Next List Item
	}

	return NULLPTR;
}

UCHAR UT_Check_SW12(UCHAR *datSW, UINT chkSW)
{
	if ((datSW[0] == ((chkSW & 0xFF00) >> 8)) && (datSW[1] == (chkSW & 0x00FF)))
		return TRUE;

	return FALSE;
}

UCHAR UT_Check_PrimitiveTag(UCHAR *iptTag)
{
	//[Ref. EMV 4.3 Book 3 Annex B Rules for BER-TLV Data Objects]
	if ((iptTag[0] & 0x20) == 0)
		return TRUE;

	return FALSE;
}

UCHAR UT_Check_PrivateClassTag(UCHAR *iptTag)
{
	//[Ref. EMV 4.3 Book 3 Annex B Rules for BER-TLV Data Objects]
	if ((iptTag[0] & 0xC0) == 0xC0)
		return TRUE;

	return FALSE;
}

//UCHAR *UT_Find_Tag(UCHAR *iptTag, UINT iptLen, UCHAR *iptData)
//{
//	UCHAR	flgConstructed=FALSE;	//Flag of Constructed Tag
//	UCHAR	lenOfT=0;				//Length of TLV-T
//	UCHAR	lenOfL=0;				//Length of TLV-L
//	UINT	lenOfV=0;				//Length of TLV-V
//	UCHAR	iptTagLen=0;			//Length of iptTag
//	UINT	parLen=0;				//Length of Parse Length
//	UCHAR	*ptrData=NULLPTR;		//Pointer of datBuffer
//	UCHAR	rspCode=FALSE;
//
//	//Get Input Tag Length
//	rspCode=UT_Get_TLVLengthOfT(iptTag, &iptTagLen);
//	if (rspCode == FALSE)
//		return NULLPTR;
//
//	//Set Pointer
//	ptrData=iptData;
//
//	do {
//		//Get TLV Length
//		rspCode=UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
//		if (rspCode == FAIL)
//			return NULLPTR;
//
//		//Match Tag
//		if (iptTagLen == lenOfT)
//		{
//			if (!memcmp(iptTag, ptrData, iptTagLen))
//				return ptrData;
//		}
//
//		//Check Constructed Tag
//		flgConstructed=!UT_Check_PrimitiveTag(ptrData);
//		if ((flgConstructed == TRUE) || (ptrData[0] == 0x80))	//Exception: Tag 80
//		{
//			//Skip T+L to Next Tag
//			ptrData+=(lenOfT+lenOfL);
//			parLen+=(lenOfT+lenOfL);
//		}
//		else
//		{
//			//Skip T+L+V to Next Tag
//			ptrData+=(lenOfT+lenOfL+lenOfV);
//			parLen+=(lenOfT+lenOfL+lenOfV);
//		}
//	} while (parLen < iptLen);
//
//	return NULLPTR;
//}


UCHAR UT_min(UCHAR numA, UCHAR numB)
{
	if (numA < numB)
		return numA;

	return numB;
}

UCHAR UT_Tx_AUX(UCHAR auxDHN, UCHAR *sndData)
{
	if (api_aux_txready(auxDHN) == apiReady)
	{
		api_aux_txstring(auxDHN, sndData);

		while (api_aux_txready(auxDHN) != apiReady);
	}
	else
		return FAIL;

	return SUCCESS;
}

UCHAR UT_Rx_AUX(UCHAR auxDHN, UCHAR *rcvData, UINT rcvTimeout)
{
	UCHAR	dhnTmr;
	UINT	tik1, tik2;
	UCHAR	rspCode=0;

	dhnTmr=api_tim_open(1);
	
	api_tim_gettick(dhnTmr, (UCHAR*)&tik1);

	do {
		rspCode=api_aux_rxready(auxDHN, rcvData);
		if (rspCode == apiOK)
		{
			api_aux_rxstring(auxDHN, rcvData);
			break;
		}

		api_tim_gettick(dhnTmr, (UCHAR*)&tik2);
	} while ((tik2 - tik1) < rcvTimeout);
	
	api_tim_close(dhnTmr);

	if (rspCode == apiOK)
		rspCode=SUCCESS;
	else
		rspCode=0xFF;	//Timeout

	return rspCode;
}

UCHAR UT_Open_AUX(UCHAR auxPort, UINT bauRate, UINT parTOB, UINT parTOR)
{
	API_AUX	sbuf;
	UCHAR	auxDHN;

	sbuf.Mode	=auxBYPASS;	
	sbuf.Baud	=bauRate+COM_CHR8+COM_NOPARITY+COM_STOP1;
	sbuf.Tob	=parTOB;
	sbuf.Tor	=parTOR;
	sbuf.Acks	=0;
	sbuf.Resend	=0;
	
	auxDHN=api_aux_open(auxPort, sbuf);
	return auxDHN;
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
UCHAR UT_ListBox( UCHAR start, UCHAR *para, UCHAR *list, UINT tout )
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
UINT  tick1, tick2;
UCHAR font;

      UT_OpenKeySelect();

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
              UT_PutChar( i, 0, font, 0x20 ); // clear prefix cursor mark
              UT_PutStr( i, 2, font+attrCLEARWRITE, list[list_start*len+ofs], &list[list_start*len+ofs+1] );
              list_start++;
              }
           UT_PutChar( cursor_pos, 0, font, 0x3e );  // show cursor mark

           if( items > 1 )
             {
             if( user_select == 0 )
               UT_PutChar( 3, 20, FONT0, 0x94 );    // show down arrow
             //UT_PutChar( 3, 20, FONT0, 0x1F );    // show down arrow
             else
               {
               if( user_select == (items - 1) )
                 UT_PutChar( 3, 20, FONT0, 0x93 );  // show up arrow
               //UT_PutChar( 3, 20, FONT0, 0x1E );  // show up arrow
               else
                 UT_PutChar( 3, 20, FONT0, 0x92 );  // show up/down arrow
               //UT_PutChar( 2, 20, FONT0, 0x1E );  // show up/down arrow
               //UT_PutChar( 3, 20, FONT0, 0x1F );  // show up/down arrow
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
               } while( UT_GetKeyStatus() == apiNotReady );
             }
           key = UT_WaitKey();

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
                      UT_OpenKeyAll();

                      return( user_select );

                 case 'x': // CANCEL
                      api_tim_close( dhn_tim );
                      UT_OpenKeyAll();

                      return( -1 );

                 default:
                      goto LB_WAIT_KEY;
                 }

           } //  while(1)
}

void UT_Put_Hex(UCHAR iptLen, UCHAR * iptData)
{
	UCHAR	idxNum=0;
	UCHAR	bytAdjustment=0;
	UCHAR	bytNum=0;
	UCHAR	colNum=0;

#ifdef _SCREEN_SIZE_128x64
	bytNum=8;
	colNum=8;
#elif defined _SCREEN_SIZE_320x240
	bytNum=12;
	colNum=15;
#else
	bytNum=10;
	colNum=20;
#endif

	if (iptLen > (bytNum*colNum))
	{
		return;
	}

	UT_ClearScreen();

	for (idxNum=0; idxNum < iptLen; idxNum++)
	{
		bytAdjustment=0;

#ifdef _SCREEN_SIZE_128x64
		if ((idxNum%8) > 4)
		{
			bytAdjustment=((idxNum%8) == 7)?(2):(1);
		}
#endif

		UT_DispHexByte((idxNum/bytNum), ((idxNum%bytNum)*3-bytAdjustment), iptData[idxNum]);
	}
}
