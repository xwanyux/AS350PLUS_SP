//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : MAX32550 (32-bit Platform)				    **
//**  PRODUCT  : AS330_mPOS						    **
//**                                                                        **
//**  FILE     : API_LCD.C 	                                            **
//**  MODULE   : api_lcd_open()				                    **
//**		 api_lcd_close()					    **
//**		 api_lcd_clear()					    **
//**		 api_lcd_putstring()					    **
//**		 api_lcd_putgraphics()					    **
//**		 api_lcd_getgraphics()					    **
//**									    **
//**  FUNCTION : API::LCD (Liquid Crystal Display Module)		    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2018/04/18                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2018 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include <string.h>

#include "POSAPI.h"

#include "OS_PROCS.h"
#include "OS_FONT.h"

#include "DEV_LCD.h"
#include "bsp_gpio.h"
// #include "DEV_LCDTFT.h"
// #include "LCDTFTAPI.h"


UCHAR		os_DHN_LCD = 0;
UCHAR		os_LCD_ATTR = 0;

//#define _LCD_ENABLED_

//extern	Bmp_t LogoPic;	// defined in "main.c"

//UCHAR		os_LCD_ConvertFont = 0;		// flag for font conversion

// ---------------------------------------------------------------------------
// FUNCTION: To check if DHN matched.
// INPUT   : dhn
// OUTPUT  : none.
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
UCHAR LCD_CheckDHN( UCHAR dhn )
{
	if( (dhn == 0) || (dhn == os_DHN_LCD) )
	  return( TRUE );
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: To enable the display device, which is enabled by system default.
// INPUT   : deviceid (RFU)
//	     The device ID, there are two basic types of display are used in POS
//	     application, the operator's display and the customer's display.
//	     0x00 - default monochrome STN LCD
//	     0x01 - TFT color LCD (GF320x240)
// OUTPUT  : none.
// RETURN  : DeviceHandleNo
//	     apiOutOfService
// NOTE    : Prior to calling this function, OS_LCD_Init() shall be called first.
// ---------------------------------------------------------------------------
#if	1
UCHAR api_lcd_open( UCHAR deviceid )
{
#ifdef	_LCD_ENABLED_

	if( os_DHN_LCD != 0 )	// already opened?
	  return( apiOutOfService );
	
//	OS_LCD_Start( os_pLcd );
	  
	os_DHN_LCD = api_lcdtft_open( deviceid);
	printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$os_DHN_LCD =%d\n",os_DHN_LCD);
	return( os_DHN_LCD );
	// return( api_lcdtft_open( deviceid) );
#else

	return( api_lcdtft_open( deviceid) );
#endif
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To disable the display device, no more message can be shown on the screen.
// INPUT   : dhn
//	     The specified device handle number.
//	     0x00 = to close all opened tasks.
// OUTPUT  : none.
// RETURN  : apiOK
//	     apiFailed
// ---------------------------------------------------------------------------
#if	1
UCHAR api_lcd_close( UCHAR dhn )
{
#ifdef	_LCD_ENABLED_

	if( LCD_CheckDHN( dhn ) == TRUE )
	  {
	  os_DHN_LCD = 0;	
	  return( apiOK );
	  }
	else
	  return( apiFailed );
#else

	return( api_lcdtft_close( dhn ) );
#endif
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To clear the designated display area.
// INPUT   : dhn
//	     The specified device handle number.
//	     sbuf
//	     UCHAR  row ;      // start row position.
//	     UCHAR  count ;    // total rows to be cleared.
//	     UCHAR  font ;     // font ID & attribute.
// OUTPUT  : none.
// RETURN  : apiOK
//	     apiFailed
// NOTE    : If the "start row position" is -1, it means to clear the whole screen,
//           only the "font attribute" is effective.
// ---------------------------------------------------------------------------
UCHAR	api_lcd_clear( UCHAR dhn, UCHAR *sbuf )
{
UCHAR	row;
UCHAR	cnt;
UCHAR	font;
UCHAR	fid;
ULONG	i;
ULONG	hbyte;
ULONG	tcnt;
OS_FDT	*pFdt;


API_LCDTFT_PARA para;
UCHAR	attr;


	row = sbuf[0];
	cnt = sbuf[1];
	font = sbuf[2];	// attr + fid
	attr = font & 0xF0;
	  
	if( row == 255 )	// clear whole screen?
	  {
	  memset( (UINT8 *)&para, 0x00, sizeof(API_LCDTFT_PARA) );
	  para.Row = 0xFFFF;
	  
	  if( attr & LCD_ATTR_REVERSE_MASK )
	    {
	    para.BG_Palette[0] = 0x00;	// black
	    para.BG_Palette[1] = 0x00;
	    para.BG_Palette[2] = 0x00;
	    }
	  else
	    {
	    para.BG_Palette[0] = 0xFF;	// white
	    para.BG_Palette[1] = 0xFF;
	    para.BG_Palette[2] = 0xFF;
	    }
	  
	  return( api_lcdtft_clear( dhn, para ) );
	  }
	else
	  {
	  memset( (UINT8 *)&para, 0x00, sizeof(API_LCDTFT_PARA) );
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

	        case FONT3:

            	     para.Font = LCDTFT_FONT3 + attr;
            	     para.FontHeight = TFTLCD_FONT3_H;
            	     para.FontWidth  = TFTLCD_FONT3_W;
	             break;
	              
	        default:

            	     para.Font = LCDTFT_FONT0 + attr;
            	     para.FontHeight = TFTLCD_FONT0_H;
            	     para.FontWidth  = TFTLCD_FONT0_W;
	             break;
	        }
	  
//          if( (font & 0x0F) == FONT0 )
//            {
//            para.Font = LCDTFT_FONT0 + attr;
//            para.FontHeight = TFTLCD_FONT0_H;
//            para.FontWidth  = TFTLCD_FONT0_W;
//            }
//          else
//            {
//            para.Font = LCDTFT_FONT1 + attr;
//            para.FontHeight = TFTLCD_FONT1_H;
//            para.FontWidth  = TFTLCD_FONT1_W;
//            }

	para.RGB = 0;	// 2013-08-13

	  if( attr & LCD_ATTR_REVERSE_MASK )
	    {
	    para.BG_Palette[0] = 0x00;	// black
	    para.BG_Palette[1] = 0x00;
	    para.BG_Palette[2] = 0x00;
	    }
	  else
	    {
	    para.BG_Palette[0] = 0xFF;	// white
	    para.BG_Palette[1] = 0xFF;
	    para.BG_Palette[2] = 0xFF;
	    }       
	  
//	  lite_printf("\r\nRow=%x", para.Row);
//	  lite_printf("\r\nCol=%x", para.Col);
//	  lite_printf("\r\nFontID=%x", para.Font);
//	  lite_printf("\r\nFontHeight=%x", para.FontHeight);
//	  lite_printf("\r\nFontWidth=%x", para.FontWidth);
//	  lite_printf("\r\nBG0=%x", para.BG_Palette[0] );
//	  lite_printf("\r\nBG1=%x", para.BG_Palette[1] );
//	  lite_printf("\r\nBG2=%x", para.BG_Palette[2] );
	  
	  return( api_lcdtft_clear( dhn, para ) );
	  }
}

// ---------------------------------------------------------------------------
// FUNCTION: To write the given string to the display at the specified position.
// INPUT   : dhn
//	     The specified device handle number.
//	     sbuf
//	     UCHAR  row ;      // start row position of the cursor.
//	     UCHAR  col ;      // start column position of the cursor.
//	     UCHAR  font ;     // font ID & attribute.
//	     dbuf
//	     UCHAR  length;        // length of the given string.
//	     UCHAR  data[length];  // character string to be displayed.
// OUTPUT  : none.
// RETURN  : apiOK
//	     apiFailed
// ---------------------------------------------------------------------------
UCHAR api_lcd_putstring( UCHAR dhn, UCHAR *sbuf, UCHAR *dbuf )
{
UCHAR	row;
UCHAR	col;
UCHAR	font;
UCHAR	fid;
UCHAR	*pData;
UCHAR	*pImage;
ULONG	i;
ULONG	len;
ULONG	tcnt = 0;
OS_FDT	*pFdt = 0;
OS_FDT	*pFdt1 = 0;


#ifdef	_LCD_ENABLED_

	if( LCD_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );
	  
	len = *dbuf++;
	pData = dbuf;
	
	if( len == 0 )
	  return( apiFailed );	// invalid data length
	  
	row = sbuf[0];
	col = sbuf[1];
	font = sbuf[2];		// attr + fid
	fid = font & 0x0F;	// fid
	
	if( font & LCD_ATTR_CLEARWRITE_MASK )	// clear before writing?
	  {
	  pFdt = OS_FONT_GetFdtAddr( fid );
	  tcnt = (pFdt->Height) / 8;	// font height in bytes = total bytes
	  
	  row *= tcnt;			// page=row*height
	  
	  os_LCD_ATTR = 0;		// reset attribute
	  for( i=0; i<tcnt; i++ )
	     {
	     OS_LCD_ClrRow( row, 0 );	// clear row with null pattern
	     row++;
	     }
	  
	  row = sbuf[0];	// retrieve row number
	  }

	os_LCD_ATTR = font & 0xF0;
	
	if( fid >= FONT2 )
	  pFdt1 = OS_FONT_GetFdtAddr( FONT1 );	// load FONT1 first then FONT2 for FDTI settings
	  
	pFdt = OS_FONT_GetFdtAddr( fid );
	pImage = os_LCD_ImageBuffer;
	  
	switch( fid )
	      {
//	      case FONT0:
//	        
//	           tcnt = len * FONT6X8_BMP_LEN;
//	             
//	           // fill image buffer with bitmap data char by char for FONT0
//	  	   for( i=0; i<len; i++ )
//	              pImage = OS_FONT_LoadFont01Bmp( pFdt, *pData++, pImage );
//	                
//	           break;

	      case FONT0:	             
	      case FONT1:
	        
//	           tcnt = len * FONT8X16_BMP_LEN;
		   tcnt = len * pFdt->ByteNo;
	             
	           // fill image buffer with bitmap data char by char for FONT1
	  	   for( i=0; i<len; i++ )
	              pImage = OS_FONT_LoadFont01Bmp( pFdt, *pData++, pImage );
	                
	           break;
	      
	      case FONT2:	// double byte code only, FONT2 (Chinese)
	      	        
	           tcnt = OS_LCD_HalfFontPutString( pFdt1, pFdt, len, pData, pImage );
	           
	           break;
	           
	      default:		// PATCH: 2012-04-05, single byte code only, FONT3~5

		   tcnt = len * pFdt->ByteNo;
		   		   
	           // fill image buffer with bitmap data char by char for FONTx
	  	   for( i=0; i<len; i++ )
	              pImage = OS_FONT_LoadFontXBmp( pFdt, *pData++, pImage );
	           
	           break;
	      }
	
	// Output all string images
	OS_LCD_WriteCharImage( fid, row, col, tcnt, os_LCD_ImageBuffer );
	
	return( apiOK );

#else

API_LCDTFT_PARA para;
UCHAR	attr;

	
	memset( (UCHAR *)&para, 0x00, sizeof(API_LCDTFT_PARA) );
	
	row = sbuf[0];
	col = sbuf[1];
	font = sbuf[2];		// attr + fid
	fid = font & 0x0F;	// fid
	attr = font & 0xF0;
	// printf("row=%d  col=%d  attr=0x%02x  font=&d\n",row,col,attr, font);
	para.Row = row;
	para.Col = col;

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

	      case FONT3:

          	   para.Font = LCDTFT_FONT3 + attr;
          	   para.FontHeight = TFTLCD_FONT3_H;
          	   para.FontWidth  = TFTLCD_FONT3_W;
	           break;

	      default:
	      
            	   para.Font = LCDTFT_FONT0 + attr;
            	   para.FontHeight = TFTLCD_FONT0_H;
            	   para.FontWidth  = TFTLCD_FONT0_W;
	           break;
	      }

//	if( (font & 0x0F) == FONT0 )
//	  {
//	  para.Font = LCDTFT_FONT0 + attr;
//	  para.FontHeight = TFTLCD_FONT0_H;
//	  para.FontWidth  = TFTLCD_FONT0_W;
//	  }
//	else
//	  {
//	  para.Font = LCDTFT_FONT1 + attr;
//	  para.FontHeight = TFTLCD_FONT1_H;
//	  para.FontWidth  = TFTLCD_FONT1_W;
//	  }
	
	//printf("F=%d  H=%d  W=0x%02x\n",para.Font, para.FontHeight, para.FontWidth);
	
	para.RGB = 0;	// 2013-08-13
	
	para.FG_Palette[0] = 0x00;
	para.FG_Palette[1] = 0x00;
	para.FG_Palette[2] = 0x00;
	
	para.BG_Palette[0] = 0xff;
	para.BG_Palette[1] = 0xff;
	para.BG_Palette[2] = 0xff;
	
	return( api_lcdtft_putstring( dhn, para, dbuf ) );

#endif
}





#if	0	// <===
// ---------------------------------------------------------------------------
// FUNCTION: To write the given image data to the display at the designated area.
// INPUT   : dhn
//	     The specified device handle number.
//	     sbuf
//	     ULONG  xleft ;     // start pixel column coordinate at the upper left.
//	     ULONG  ytop ;      // start pixel row coordinate at the upper left.
//	     ULONG  width ;     // image width in dots in horizontal direction.
//	     ULONG  height ;    // image height in dots in vertical direction.
//	     ULONG  method ;    // method of writing graphics.
//	     dbuf -- the image data.
// OUTPUT  : none.
// RETURN  : apiOK
//	     apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_lcd_putgraphics( UCHAR dhn, API_GRAPH dim, UCHAR *Image )
{
UINT32	xs, ys;

	
	if( LCD_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );

 	xs = dim.Ytop;
 	ys = 240-dim.Width-dim.Xleft;
	LogoPic.H_Size = dim.Height;
	LogoPic.V_Size = dim.Width;

	LogoPic.pPicStream = (UCHAR *)Image;
	LCD_LoadPic( LCD_PANEL_UPPER, xs, ys, &LogoPic, 0x00 );
	
	return( apiOK );
}

// ---------------------------------------------------------------------------
// FUNCTION: To read the given image data from the display at the designated area.
// INPUT   : dhn
//	     The specified device handle number.
//	     sbuf
//	     UCHAR  xleft ;     // start pixel column coordinate at the upper left.
//	     UCHAR  ytop ;      // start pixel row coordinate at the upper left.
//	     UCHAR  width ;     // image width in dots in horizontal direction.
//	     UCHAR  height ;    // image height in dots in vertical direction.
//	     UCHAR  method ;    // method of writing graphics.
// OUTPUT  : dbuf
//	     UINT   length ;       // length of image data read.
//           UCHAR  data[length] ; // image data read.
// OUTPUT  : none.
// RETURN  : apiOK
//	     apiFailed
// ---------------------------------------------------------------------------
#if	0
UCHAR api_lcd_getgraphics( UCHAR dhn, API_GRAPH dim, UCHAR *Image )
{
#ifdef	_LCD_ENABLED_

ULONG	i;
ULONG	j;
ULONG	cnt;
UCHAR	rdata = 0;
ULONG	x, y;
ULONG	method;
	
	if( LCD_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );
	
	os_LCD_ATTR = 0;	// clr attribute
	
	cnt = (dim.Height >> 3) + (dim.Height % 8);	// row count (1 row = 8 dots)
	
	x = dim.Ytop;
	method = dim.Method;
	for( i=0; i<cnt; i++ )
	   {
	   y = dim.Xleft;
	   
	   for( j=0; j<dim.Width; j++ )
	      OS_LCD_ReadModifyWrite( 1, x, y++, method, rdata, Image++ );
	   
	   x += 8;
	   }

	return( apiOK );
#endif

	return( apiFailed );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To initialize the external bitmap fonts for application program.
//	     The specified font should be downloaded to system memory prior to 
//	     initialization.
// INPUT   : ft
// 	     The "FONT" struture.
//	     Normal Fond ID: FONT2, FONT3...
//	     Special Fond ID (for EMV L2 application perferred name)
//	     FondID: 0xFF - reset to system default FONT0 & FONT1.
//		     0xF0 - setup ISO 8859 code table to FONT0.
//		     0xF1 - setup ISO 8859 code table to FONT1.
//		     0xFn - setup ISO 8859 code table to FONTn.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
#if	1
UCHAR api_lcd_initfont( API_LCD_FONT ft  )
{
UINT8	fid;
OS_FDT	*pFdt;

	fid = ft.FontID;
	if( (fid & 0xF0) == 0xF0 )	// font = 0xFn? (ISO special font)
	  {
	  fid &= 0x0F;
	  
	  if( fid > MAX_FDT_NO )
	    return( apiFailed );
	  
	  if( fid == 0xFF )
	    {
	    OS_FONT_Init( LCD_DID_STN );	// restore system default FONT0 & FONT1
	    return( apiOK );
	    }
	  }
	else
	  {
	  if( (fid > MAX_FDT_ID) || (fid < MIN_FDT_ID) )
	    return( apiFailed ); // invalid fond id	    
	  }
	
	pFdt = OS_FONT_GetFdtAddr( fid );	// calculate target FDT
	
	pFdt->FontID = ft.FontID;		// copy settings to target FDT
	pFdt->ByteNo = ft.ByteNo;
	pFdt->Width = ft.Width;
	pFdt->Height = ft.Height;
	pFdt->CodStAddr = ft.codStAddr;
	pFdt->CodEndAddr = ft.codEndAddr;
	pFdt->BmpStAddr = ft.bmpStAddr;
	pFdt->BmpEndAddr = ft.bmpEndAddr;
	
	return( apiOK );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To convert printer font into display font.
// INPUT   : flag - 0 = disable font conversion.
//		    1 = enabled font conversion.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	0
void	api_lcd_convertfont( UCHAR flag )
{
	os_LCD_ConvertFont = flag;
}
#endif

#endif	// <===
