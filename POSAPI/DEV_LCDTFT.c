//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L (32-bit Platform)                                     **
//**  PRODUCT  : AS350	                                                    **
//**                                                                        **
//**  FILE     : DEV_LCDTFT.C                                               **
//**  MODULE   : GF320240						    **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2013/04/18                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2013 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include <string.h>
#include "bsp_types.h"
//#include "bsp_mem.h"

#include "OS_FONT.h"
#include "DEV_LCD.h"
#include "DEV_LCDTFT.h"
//#include "DEV_SPI.h"
#include "LCDTFTAPI.h"
//#include "DEV_TSC.h"


extern	UINT8	os_LCDTFT_ATTR;

#if	0
extern	void	RGB_BPP24to16( UINT8 *bpp24, UINT8 *bpp16 );

extern	UINT8	os_LCDTFT_StrImageBuffer[MAX_STR_IMAGE_BUF_SIZE+512];
extern	UINT32	os_LCDTFT_StrImageBufferIndex;
extern	UINT32	os_LCDTFT_StrImageBufferLength;

extern	UINT32	os_HpCopFlag;

//extern	UINT32	os_ADC_InUse;

//extern	UINT32	os_LCDBL_Status;	// defined in DEV_LCD.c


UINT8	os_pImageData2[320*240/8];	// max 320x240 char font

UINT32	os_LCDTFT_BL_FLAG = 0;		// 0=don't care, 1=turn on or off backlight
API_LCDTFT_BACKLIT os_LCDTFT_BL_PARA;	//

UINT32	os_LCDTFT_BL_Status = TRUE;
UINT32	os_LCDTFT_InUse = FALSE;
UINT32	os_LCDTFT_InUse2 = FALSE;
#endif


// ---------------------------------------------------------------------------
// FUNCTION: is LCDTFT in use? (checked by APP thread to avoid resource problem)
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
#if	0
UINT32	IS_LCDTFT_IN_USE( void )
{
	return( FALSE );	// 2022-08-30, do nothing, but SPI in POSAPI shall be updated
	
	if( os_LCDTFT_InUse || os_LCDTFT_InUse2 )
	  return( TRUE );
	else
	  return( FALSE );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: reset M3 system.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	0
void	OS_LCDTFT_ResetSystem( void )
{
SPI_LCDTFT	spilcd;
UINT32	result;

	
	spilcd.DevID = DID_LCD;
	spilcd.CmdID = 0x19;	// CID_LCD_RESET;
//	memmove( (UINT8 *)&spilcd.LcdPara, (UINT8 *)&para, sizeof(API_LCDTFT_PARA) );

	os_LCDTFT_InUse = TRUE;
	
	result = OS_SPI_PutPacket( sizeof(SPI_LCDTFT), (UINT8 *)&spilcd );
	
	BSP_Delay_n_ms(100);
	
	os_LCDTFT_InUse = FALSE;
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To clear the designated display area.
// INPUT   : 
//	     ------ TFT ------	// parameter structure 16 bytes
//	     UINT   pixrow;	// beginning pix row number (x8)
//	     UINT   pixcol;	// total pixel rows to be cleared (x8)
//	     UCHAR  font;	// font id & attribute
//	     UCHAR  rgb;	// RGB mode
//	     UCHAR  FG_palette[3];	// foreground palette (3 bytes)
//	     UCHAR  fontH;	// font height in dots
//	     UCHAR  fontW;	// font width in dots
//	     UCHAR  BG_palette[3];	// background palette (3 bytes)
//	     UCHAR  RFU[3];	// reserved (2 bytes)
//
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
#if	0
UINT32	OS_LCDTFT_ClrScreen( API_LCDTFT_PARA para )
{
SPI_LCDTFT	spilcd;
UINT32	result;

	
//	memset( (UINT8 *)&spilcd, 0x00, sizeof(SPI_LCDTFT) );
	
	spilcd.DevID = DID_LCD;
	spilcd.CmdID = CID_LCD_CLR_SCREEN;
	memmove( (UINT8 *)&spilcd.LcdPara, (UINT8 *)&para, sizeof(API_LCDTFT_PARA) );

#if	0
	spilcd.PixRow = para.PixRow;
	spilcd.PixCol = para.PixCol;
	spilcd.Font = para.Font;
	spilcd.RGB = para.RGB;
	spilcd.Palette[0] = para.Palette[0];
	spilcd.Palette[1] = para.Palette[1];
	spilcd.Palette[2] = para.Palette[2];
	spilcd.FontHeight = para.FontHeight;
	spilcd.FontWidth = para.FontWidth;
#endif
	os_LCDTFT_InUse = TRUE;
	result = OS_SPI_PutPacket( sizeof(SPI_LCDTFT), (UINT8 *)&spilcd );
	
	if( os_HpCopFlag )
//	  BSP_Delay_n_ms(100-60);
//	  BSP_Delay_n_ms(100-75);	// 2016-11-11
	  BSP_Delay_n_ms(100-70);	// 2022-08-04
	else
	  BSP_Delay_n_ms(100);
	
	os_LCDTFT_InUse = FALSE;
	
	return( result );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To clear the designated display area.
// INPUT   : 
//	     ------ TFT ------	// parameter structure 16 bytes
//	     UINT   pixrow;	// beginning pix row number (x8)
//	     UINT   pixcol;	// total pixel rows to be cleared (x8)
//	     UCHAR  font;	// font id & attribute
//	     UCHAR  rgb;	// RGB mode
//	     UCHAR  palette[3];	// palette (3 bytes)
//	     UCHAR  fontH;	// font height in dots
//	     UCHAR  fontW;	// font width in dots
//	     UCHAR  RFU[5];	// reserved (5 bytes)
//
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
#if	0
UINT32	OS_LCDTFT_ClrRow( API_LCDTFT_PARA para )
{
SPI_LCDTFT	spilcd;
UINT32	result;

	
	spilcd.DevID = DID_LCD;
	spilcd.CmdID = CID_LCD_CLR_PIXROW;
	memmove( (UINT8 *)&spilcd.LcdPara, (UINT8 *)&para, sizeof(API_LCDTFT_PARA) );

	os_LCDTFT_InUse = TRUE;
	result = OS_SPI_PutPacket( sizeof(SPI_LCDTFT), (UINT8 *)&spilcd );
	
//	BSP_Delay_n_ms(10);	// 2016-11-11
	BSP_Delay_n_ms(10-8);	// 2016-11-17
	
	os_LCDTFT_InUse = FALSE;
	
	return( result );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To set LCD cursor position according to the specified fond ID at (r,c).
// INPUT   : 
//           row -- row.
//           col -- column.
//           height -- font height.
//           width  -- font width.
//	     fwidth -- original bmp font width.
// OUTPUT  : pixrow -- pixel row number. (X)
//	     pixcol -- pixel col number. (Y)
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
#if	0
//UINT16	x = 0;
//UINT16	y = 0;
//UINT8	offset;
//UINT8	fwidth2;
//UINT8	offset2;
UINT32	OS_LCDTFT_SetCharCursor( UINT8 *first, UINT8 font, UINT8 row, UINT8 col, UINT8 height, UINT8 width, UINT8 fwidth, UINT16 *pixrow, UINT16 *pixcol )
{
UINT16	x = 0;
UINT16	y = 0;
UINT8	offset;
UINT8	fwidth2;
UINT8	offset2;


	*first = 0;

	fwidth2 = fwidth;
	if( fwidth2 % 8 )
	  fwidth2 += (8 - (fwidth % 8 ));

	offset = width;
	if( width > fwidth2 )
	  {
	  while(1)
	       {
	       if( offset <= fwidth2 )
	         break;
	       offset -= fwidth2;
	       }
	  }
	else
	  {
	  offset = width % 8;	// x8 allignment
	  if( offset != 0 )
	    offset = 8 - (width % 8);
	  }
	
	if( (os_LCDTFT_ATTR & LCD_ATTR_XYDOT_MASK) != LCD_ATTR_XYDOT_MASK )	// 2013-09-05
	  {
//	  x = height * row * 8;	// X
	  x = height * row;	// X
	  }
	else
	  x = row*2;		// in 2-pix units
	  
	if( (x + height) > MAX_LCDTFT_PIXROW_NO )
	  return( FALSE );
	
	if( (os_LCDTFT_ATTR & LCD_ATTR_XYDOT_MASK) == LCD_ATTR_YDOT_MASK )	// 2013-09-05
	  {
	  if( (col+width) > MAX_LCDTFT_PIXCOL_NO )
	    return( FALSE );
	    
//	  y = MAX_LCDTFT_PIXCOL_NO - col - width;	// Y
	  if( col == 0 )
	    {
	    if( fwidth % 8 )	// x8 allignment
	      fwidth += (8 - (fwidth % 8));
	    y = MAX_LCDTFT_PIXCOL_NO - fwidth;
	    }
	  else
	    {
	    y = MAX_LCDTFT_PIXCOL_NO - col - width;
	    if( width <= fwidth2 )
	      {
	      if( y >= offset )
	        y -= offset;
	      else
	        {
		*first = offset;
	        }
	      }
	    else
	      y += offset;
	    }
	  }
	else
	  {
///	  if( (font & 0x0F) == LCDTFT_FONT2 )
///	    {
///	    if( (((width/2)*col) + width) > MAX_LCDTFT_PIXCOL_NO )
///	      return( FALSE );
///	    }
///	  else
///	    if( ((width*col) + width) > MAX_LCDTFT_PIXCOL_NO )
///	      return( FALSE );
	  
	  // special check for the left-most boundary
//	  if( first && ((font & 0x0F) == LCDTFT_FONT2) )
//	    y = MAX_LCDTFT_PIXCOL_NO - (width * col) - width/2;
//	  else
//	    y = MAX_LCDTFT_PIXCOL_NO - (width * col) - width;

//	  if( ((font & 0x0F) == LCDTFT_FONT1) || ((font & 0x0F) == LCDTFT_FONT2) )
	  if( (font & 0x0F) >= LCDTFT_FONT2 )
	    y = MAX_LCDTFT_PIXCOL_NO - ((width/2) * col) - (width);
	  else
	    y = MAX_LCDTFT_PIXCOL_NO - (width * col) - width;
	    
	  if( col == 0 )
	    {
	    if( fwidth % 8 )	// x8 allignment
	      fwidth += (8 - (fwidth % 8));
	    y = MAX_LCDTFT_PIXCOL_NO - fwidth;
	    }
	  else
	    {
	    if( width <= fwidth2 )
	      {
	      if( y >= offset )
	        y -= offset;
	      else
	        {
		*first = offset;
	        }
	      }
	    else
	      y += offset;
	    }
	  }
	
	*pixrow = x;
	*pixcol = y;
	
	return( TRUE );
}
#endif

// ---------------------------------------------------------------------------
UINT32	OS_LCDTFT_SetCharCursor( UINT8 *first, UINT8 font, UINT16 row, UINT16 col, UINT8 height, UINT8 width, UINT8 fwidth, UINT16 *pixrow, UINT16 *pixcol )
{
UINT16	x = 0;
UINT16	y = 0;
//UINT8	offset;
//UINT8	fwidth2;
//UINT8	offset2;


	*first = 0;
	
	if( (os_LCDTFT_ATTR & LCD_ATTR_XYDOT_MASK) != LCD_ATTR_XYDOT_MASK )
	  {
	  y = height * row;	// Y
	  }
	else
	  y = row*2;		// in 2-pix units
	  
	if( (y + height) > MAX_LCDTFT_PIXROW_NO )
	  return( FALSE );
	
	if( (os_LCDTFT_ATTR & LCD_ATTR_XYDOT_MASK) == LCD_ATTR_YDOT_MASK )
	  {
	  if( (col+width) > MAX_LCDTFT_PIXCOL_NO )
	    return( FALSE );
	  
	  x = col;
	  }
	else
	  {
	  if( (font & 0x0F) >= LCDTFT_FONT2 )
	    x = (width/2) * col;
	  else
	    x = width * col;
	  }
	
	*pixrow = y;
	*pixcol = x;
	
	return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: To put image by character. (method 1: send right away)
// INPUT   : para
//	     ------ TFT ------	// parameter structure 16 bytes
//	     UINT   row;	// char row
//	     UINT   col;	// char column
//	     UCHAR  font;	// font id & attribute
//	     UCHAR  rgb;	// RGB mode
//	     UCHAR  palette[3];	// palette (3 bytes)
//	     UCHAR  fontH;	// font height in dots
//	     UCHAR  fontW;	// font width in dots
//	     UCHAR  RFU[5];	// reserved (5 bytes)
//
//	     length -- length of image data.
//	     pImage -- image data.
//
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
#if	0
UINT32	OS_LCDTFT_PutChar( API_LCDTFT_PARA para, OS_FDT *pFdt, UINT8 *pImage )
{
UINT32	result;
UINT32	length;
UINT16	pixrow;
UINT16	pixcol;
UINT16	cnt;
UINT8	offset;
UINT8	*pData;
UINT8	*pData2;
UINT8	first;
UINT8	temp[256];


	result = FALSE;
	
	length = pFdt->ByteNo;

	// convert char cursor to pixel cursor
	if( !OS_LCDTFT_SetCharCursor( &first, para.Font, para.Row, para.Col, para.FontHeight, para.FontWidth, pFdt->Width, &pixrow, &pixcol ) )
	  return( result );
	
	if( first )
	  {
	  // up-shift image by "pixcol"
	  memset( temp, 0x00, sizeof(temp) );
//	  first += 2;
	  cnt = (para.FontHeight / 8) * (first);
	  memmove( temp, pImage+cnt, length-cnt );
	  memmove( pImage, temp, length );
	  }
	
	pData = BSP_Malloc( sizeof(SPI_LCDTFT) + length );
	if( pData )
	  {
	  pData2 = pData;
	  
	  // put packet header
	  *pData2++ = DID_LCD;
	  *pData2++ = CID_LCD_PUT_CHAR;
	  	  
	  memmove( pData2, (UINT8 *)&para, sizeof(API_LCDTFT_PARA) );
	  *pData2++ = pixcol & 0x00FF;
	  *pData2++ = (pixcol & 0xFF00) >> 8;
	  *pData2++ = pixrow & 0x00FF;
	  *pData2++ = (pixrow & 0xFF00) >> 8;
	  
	  pData2 +=6;	// ptr to FontWidth
	  if( first == 0 )
	    {
	    offset = pFdt->Width % 8;
	    if( offset != 0 )
	      offset = 8 - (pFdt->Width % 8);
	    }
	  else
	    offset = 0;
	  *pData2++ = pFdt->Width + offset;	// shall be x8 allignment
	  
	  pData2 += (sizeof(API_LCDTFT_PARA) - 11);	// ptr to DATA
	  
	  // put packet data
	  memmove( pData2, pImage, length );
	  
	  os_LCDTFT_InUse = TRUE;
	  if( OS_SPI_PutPacket( sizeof(SPI_LCDTFT)+length, pData ) )
	    result = TRUE;
	    
	  os_LCDTFT_InUse = FALSE;
	    
	  BSP_Free( pData );
	  }

	return( result );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To put image by character. (method 2: store and forward)
// INPUT   : para
//	     ------ TFT ------	// parameter structure 16 bytes
//	     UINT   row;	// char row
//	     UINT   col;	// char column
//	     UCHAR  font;	// font id & attribute
//	     UCHAR  rgb;	// RGB mode
//	     UCHAR  palette[3];	// palette (3 bytes)
//	     UCHAR  fontH;	// font height in dots
//	     UCHAR  fontW;	// font width in dots
//	     UCHAR  RFU[5];	// reserved (5 bytes)
//
//	     length -- length of image data.
//	     pImage -- image data.
//
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
#if	0
UINT32	OS_LCDTFT_PutChar_EX( API_LCDTFT_PARA para, OS_FDT *pFdt, UINT8 *pImage )
{
UINT32	result;
UINT32	length;
UINT32	tlength;
UINT16	pixrow;
UINT16	pixcol;
UINT16	cnt;
UINT8	offset;
UINT8	*pData;
UINT8	*pData2;
UINT8	first;
UINT8	temp[256];


	result = FALSE;
	
	length = pFdt->ByteNo;

	// convert char cursor to pixel cursor
	if( !OS_LCDTFT_SetCharCursor( &first, para.Font, para.Row, para.Col, para.FontHeight, para.FontWidth, pFdt->Width, &pixrow, &pixcol ) )
	  return( result );
	
	if( first )
	  {
	  // up-shift image by "pixcol"
	  memset( temp, 0x00, sizeof(temp) );
//	  first += 2;
	  cnt = (para.FontHeight / 8) * (first);
	  memmove( temp, pImage+cnt, length-cnt );
	  memmove( pImage, temp, length );
	  }
	
	pData = BSP_Malloc( sizeof(SPI_LCDTFT) + length );
	if( pData )
	  {
	  pData2 = pData;
	  
	  // put packet header LEN=(16+length)
	  *pData2++ = (sizeof(API_LCDTFT_PARA)+length) & 0x00FF;
	  *pData2++ = ((sizeof(API_LCDTFT_PARA)+length) & 0xFF00) >> 8;
	  	  
	  memmove( pData2, (UINT8 *)&para, sizeof(API_LCDTFT_PARA) );
	  *pData2++ = pixcol & 0x00FF;
	  *pData2++ = (pixcol & 0xFF00) >> 8;
	  *pData2++ = pixrow & 0x00FF;
	  *pData2++ = (pixrow & 0xFF00) >> 8;
	  
	  pData2 +=6;	// ptr to FontWidth
	  if( first == 0 )
	    {
	    offset = pFdt->Width % 8;
	    if( offset != 0 )
	      offset = 8 - (pFdt->Width % 8);
	    }
	  else
	    offset = 0;
	  *pData2++ = pFdt->Width + offset;	// shall be x8 allignment
	  
	  pData2 += (sizeof(API_LCDTFT_PARA) - 11);	// ptr to DATA
	  
	  // put packet data
	  memmove( pData2, pImage, length );
	  
	  tlength = sizeof(API_LCDTFT_PARA) + length + 2;
	  
	  os_LCDTFT_StrImageBufferLength += tlength;	// total length = PARA + DataLen + 2
	  if( os_LCDTFT_StrImageBufferLength <= sizeof(os_LCDTFT_StrImageBuffer) )
	    {
	    // LEN(2) + PARA(16) + DATA(n)	    
	    memmove( &os_LCDTFT_StrImageBuffer[os_LCDTFT_StrImageBufferIndex], pData, tlength );
	    os_LCDTFT_StrImageBufferIndex += tlength;	// ptr to next position
	    
	    result = TRUE;
	    }
	  
	  BSP_Free( pData );
	  }

	return( result );
}
#endif

// ---------------------------------------------------------------------------
#if	0
UINT32	OS_LCDTFT_PutStr( void )
{
UINT32	result = FALSE;
UINT32	Len1 = 0;
UINT32	Len2 = 0;
UINT32	len;
UINT32	len_lo;
UINT32	len_hi;
UINT8	*pData;
UINT8	*pData2;
UINT8	*pImage;

UINT16	rx_length = 1;
UINT8	status;


	os_LCDTFT_InUse = TRUE;
	
	Len1 = 0;
	pImage = os_LCDTFT_StrImageBuffer;	// [LENx(2) PARAx(16) DATAx(n)] [X] [X]...
	
	if( os_LCDTFT_StrImageBufferLength > MAX_STR_IMAGE_BUF_SIZE )	// COP160 with larger "SSP_RX_BUFFER_SIZE" (NA)
	  {
	  while(1)
	       {
	       // read sub-record length
	       len_lo = *(pImage+0);
	       len_hi = *(pImage+1);
	       len = len_lo + len_hi*256;
	       
	       // max range check
	       if( (Len1 + len + 2) < MAX_STR_IMAGE_BUF_SIZE )
	         {
	         Len1 += (len + 2);	// sub-total length
	         pImage += (len + 2);	// ptr to next record LEN
	         }
	       else
	       	 {
	       	 Len2 = os_LCDTFT_StrImageBufferLength - Len1;
	         break;
	         }
	       }
	  }
	else
	  {
	  result = TRUE;
	  Len2 = os_LCDTFT_StrImageBufferLength;
	  }
	
	// process 1'st part
	if( Len1 )
	  {
	  pData = BSP_Malloc( sizeof(SPI_LCDTFT) + Len1 );
	  if( pData )
	    {
	    pData2 = pData;
	    
	    // put packet header
	    memset( pData2, 0x00, sizeof(SPI_LCDTFT) );
	    
	    *pData2++ = DID_LCD;
	    *pData2++ = CID_LCD_PUT_STR;
	    
	    *pData2++ = Len1 & 0x00FF;		// "Row" as TLEN
	    *pData2++ = (Len1 & 0xFF00) >> 8;	//
	    pData2 += 14;		// ptr to DATA
	    
	    memcpy( pData2, os_LCDTFT_StrImageBuffer, Len1 );
	    
//	    BSP_Delay_n_ms(10+2);
//	    BSP_Delay_n_us(12*Len1);
	    
////	    os_LCDTFT_InUse = TRUE;
//	    if( OS_SPI_PutPacket( sizeof(SPI_LCDTFT) + Len1, pData ) )
	    if( OS_SPI_PutPacket_WR_LCD( sizeof(SPI_LCDTFT) + Len1, pData, rx_length, &status ) )
	      result = TRUE;
	    
	    BSP_Free( pData );
	    
//	    BSP_Delay_n_ms(10);
	    BSP_Delay_n_us(12*Len1);
	    
////	    os_LCDTFT_InUse = FALSE;
	    }
	  }

	if( !result )
////	  return( result );
	  goto EXIT;
	
	result = FALSE;

	// process 2'nd part
	if( Len2 )
	  {
	  pData = BSP_Malloc( sizeof(SPI_LCDTFT) + Len2 );
	  if( pData )
	    {
	    pData2 = pData;
	    
	    // put packet header
	    memset( pData2, 0x00, sizeof(SPI_LCDTFT) );
	    
	    *pData2++ = DID_LCD;
	    *pData2++ = CID_LCD_PUT_STR;
	    
	    *pData2++ = Len2 & 0x00FF;		// "Row" as TLEN
	    *pData2++ = (Len2 & 0xFF00) >> 8;	//
	    pData2 += 14;		// ptr to DATA
	    
	    memcpy( pData2, pImage, Len2 );
	    
////	    os_LCDTFT_InUse = TRUE;
//	    if( OS_SPI_PutPacket( sizeof(SPI_LCDTFT) + Len2, pData ) )
	    if( OS_SPI_PutPacket_WR_LCD( sizeof(SPI_LCDTFT) + Len2, pData, rx_length, &status ) )
	      result = TRUE;
	    
	    BSP_Free( pData );
	    
//	    BSP_Delay_n_ms(10);
//	    BSP_Delay_n_us(12*Len2);
	    
	    if( Len2 < 800 )		// 2022-08-18
	      BSP_Delay_n_us(13*Len2);
	    else
	      BSP_Delay_n_us(20*Len2);
	    
////	    os_LCDTFT_InUse = FALSE;
	    }
	  }

EXIT:
	os_LCDTFT_InUse = FALSE;
	
	return( result );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Transform printer image into AS350 display format. (counter clockwise 90 degrees)
// INPUT   : Height     - height of char font.
//           Width      - width of char font.
//           pImageData - the image data to be transformed.
// OUTPUT  : pImageData - the image data with new orientation.
// RETURN  : TRUE
//	     FALSE - out of spec.
// ---------------------------------------------------------------------------
#if	0
UINT32	OS_LCDTFT_TransformCharBitmap( UINT32 Height, UINT32 Width, UINT8 *pImageData )
{
UINT32	i, j, k, l, m, n;
UINT32	BytesPerRow;
UINT32	BytesPerCol;
UINT32	right_shift;
UINT32	left_shift;
UINT8	c1, c2;
//UINT8	pImageData2[32*32/8];	// max 32x32 char font


	if( (Height*Width)/8 > sizeof(os_pImageData2) )
	  return( FALSE );

	memset( os_pImageData2, 0x00, sizeof(os_pImageData2) );

	BytesPerRow = Width / 8;
	if( Width % 8 )
	  BytesPerRow++;
	BytesPerCol = Height / 8;
	if( Height % 8 )
	  BytesPerCol++;
	
	for( i=0; i<BytesPerRow; i++ )
	   {
	   for( j=0; j<BytesPerCol; j++ )
	      {
	      m = 0;
	      
	      right_shift = 0;
	      
	      for( k=0; k<8; k++ )
	         {
	         left_shift  = 7;
	         c2 = 0;
	         
	         for( l=0; l<8; l++ )
	            {
	            c1 = (pImageData[(j*8*BytesPerRow) + (l*BytesPerRow) + (BytesPerRow-i-1)] >> right_shift) & 0x01;
	            c1 <<= left_shift;
	            c2 |= c1;
	            
	            left_shift--;
	            }
	            
	         os_pImageData2[(BytesPerCol)*m + j + (i*BytesPerCol*8)] = c2;
	         m++;
	         right_shift++;
	         }
	      }
	   }
	   
	memmove( pImageData, os_pImageData2, BytesPerRow*BytesPerCol*8 );
//	memset( os_pImageData2, 0x00, sizeof(os_pImageData2) );
	
	return( TRUE );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To control related graphics appearance for PCD application,
//	     such as TAP ICON, LED ICON...
// INPUT   : dhn
//	     The specified device handle number.
//
//	     icon
//	     ------ TFT ------	// 
//	     UINT   ID;		// id of the built-in icon (0..n)
//	     UINT   BlinkOn;	// time for icon ON period in 10ms units  (0xFFFF=always ON, 0x0000=no blinking)
//	     UINT   BlinkOff;	// time for icon OFF period in 10ms units
// 	     UCHAR  RFU[10];
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
#if	0
UINT32	OS_LCDTFT_ShowPCD( API_PCD_ICON icon )
{
SPI_PCDICON	spipcd;

	
	spipcd.DevID = DID_PCD;
	spipcd.CmdID = CID_PCD_SHOW_PCD;
	memmove( (UINT8 *)&spipcd.IconPara, (UINT8 *)&icon, sizeof(API_PCD_ICON) );

	return( OS_SPI_PutPacket( sizeof(SPI_PCDICON), (UINT8 *)&spipcd ) );
}
#endif

// ---------------------------------------------------------------------------
#if	0
UINT32	OS_LCDTFT_ShowPCD( API_PCD_ICON icon )
{
SPI_PCDICON	spipcd;
UINT16	rx_length = 1;
UINT8	status;
UINT32	result;

	
	spipcd.DevID = DID_PCD;
	spipcd.CmdID = CID_PCD_SHOW_PCD;
	memmove( (UINT8 *)&spipcd.IconPara, (UINT8 *)&icon, sizeof(API_PCD_ICON) );
	
//	return( OS_SPI_PutPacket_WR( sizeof(SPI_PCDICON), (UINT8 *)&spipcd, rx_length, &status ) );
	
	os_LCDTFT_InUse = TRUE;
	result = OS_SPI_PutPacket_WR( sizeof(SPI_PCDICON), (UINT8 *)&spipcd, rx_length, &status );
	os_LCDTFT_InUse = FALSE;
	
	return( result );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To fill a rectangle.
//	     
// INPUT   : dhn
//	     The specified device handle number.
//
//	     para
//	     ------ TFT ------	// 
//	     UINT		Xstart		// id number of the built-in icon (0..n)
//	     UINT		Xend		// start pixel column coordinate at the upper left
//	     UINT		Ystart;		// start pixel row coordinate at the upper left
//	     UINT		Yend;		// method of writing graphics
//	     UCHAR  		rgb;		// RGB mode
//	     UCHAR  		palette[3];	// palette (3 bytes) to fill
//	     UCHAR		RFU[4];
// OUTPUT  : none.
// RETURN  : apiOK
//	     apiFailed
// ---------------------------------------------------------------------------
#if	0
UINT32	OS_LCDTFT_FillRECT( API_LCDTFT_RECT rect )
{
UINT32		result;
SPI_LCDRECT	spirect;
UINT8		bpp24[3];
UINT8		bpp16[2];


//	return( apiOK );
	
	bpp24[0] = rect.Palette[0];
	bpp24[1] = rect.Palette[1];
	bpp24[2] = rect.Palette[2];
	
	if( (rect.Palette[0] == 0xFF) && (rect.Palette[1] == 0xFF) && (rect.Palette[2] == 0xFF) )
	  {
	  bpp16[0] = 0xFF;
	  bpp16[1] = 0xFF;
	  }
	else
	  RGB_BPP24to16( bpp24, bpp16 );
	
	spirect.DevID = DID_LCD;
	spirect.CmdID = CID_LCD_FILL_RECT;
	memmove( (UINT8 *)&spirect.RectPara, (UINT8 *)&rect, sizeof(API_LCDTFT_RECT) );
	
	spirect.RectPara.Palette[0] = bpp16[0];
	spirect.RectPara.Palette[1] = bpp16[1];

	os_LCDTFT_InUse = TRUE;
	result = OS_SPI_PutPacket( sizeof(SPI_LCDRECT), (UINT8 *)&spirect );
	
	BSP_Delay_n_ms( 100 );	// 50
	
	os_LCDTFT_InUse = FALSE;
	
	return( result );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To control backlight for power-saving mode.
//	     
// INPUT   : dhn
//	     The specified device handle number.
//
//	     para
//	     ------ TFT ------	// 
//	     ULONG		Level		// backlight driver level
//	     UCHAR		RFU[12];
// OUTPUT  : none.
// RETURN  : apiOK
//	     apiFailed
// ---------------------------------------------------------------------------
#if	0
UINT32	OS_LCDTFT_BackLight( API_LCDTFT_BACKLIT para )
{
UINT32		result;
SPI_LCDBL	spibl;

	
	spibl.DevID = DID_LCD;
	spibl.CmdID = CID_LCD_BACKLIT;
	memmove( (UINT8 *)&spibl.RectPara, (UINT8 *)&para, sizeof(API_LCDTFT_BACKLIT) );

	result = OS_SPI_PutPacket( sizeof(SPI_LCDBL), (UINT8 *)&spibl );
	
	BSP_Delay_n_ms( 100 );	// 50

	return( result );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To control backlight for power-saving mode.
//	     
// INPUT   : dhn
//	     The specified device handle number.
//
//	     para
//	     ------ TFT ------	// 
//	     ULONG		Level		// backlight driver level
//	     UCHAR		RFU[12];
// OUTPUT  : none.
// RETURN  : apiOK
//	     apiFailed
// ---------------------------------------------------------------------------
#if	0
UINT32	OS_LCDTFT_BackLight_NOW( API_LCDTFT_BACKLIT para )
{
UINT32		result;
SPI_LCDBL	spibl;


//	os_LCDTFT_BL_FLAG = FALSE;
	
//	BSP_Delay_n_ms( 20 );



	if( para.Level )	// turn ON
	  {
	  if( os_LCDTFT_BL_Status == TRUE )		// current status = ON?
	    return( apiOK );
	  else
	    os_LCDTFT_BL_Status = TRUE;
	  }
	else			// turn OFF
	  {
	  if( os_LCDTFT_BL_Status == FALSE )	// current status = ON?
	    return( apiOK );
	  else
	    os_LCDTFT_BL_Status = FALSE;
	  }




	spibl.DevID = DID_LCD;
	spibl.CmdID = CID_LCD_BACKLIT;
	memmove( (UINT8 *)&spibl.RectPara, (UINT8 *)&para, sizeof(API_LCDTFT_BACKLIT) );

	result = OS_SPI_PutPacket( sizeof(SPI_LCDBL), (UINT8 *)&spibl );
	
	BSP_Delay_n_ms( 40 );
	
	return( result );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To control backlight for power-saving mode.
//	     
// INPUT   : dhn
//	     The specified device handle number.
//
//	     para
//	     ------ TFT ------	// 
//	     ULONG		Level		// backlight driver level
//	     UCHAR		RFU[12];
// OUTPUT  : none.
// RETURN  : apiOK
//	     apiFailed
// ---------------------------------------------------------------------------
#if	0
UINT32	OS_LCDTFT_BackLight( API_LCDTFT_BACKLIT para )
{
	if( os_LCDTFT_BL_FLAG )
	  return( apiOK );
	  
	memmove( (UINT8 *)&os_LCDTFT_BL_PARA, (UINT8 *)&para, sizeof(API_LCDTFT_BACKLIT) );
	
	os_LCDTFT_BL_FLAG = TRUE;
	
	return( apiOK );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To control backlight for power-saving mode. (real-time task)
//	     
// INPUT   : dhn
//	     The specified device handle number.
//
//	     para
//	     ------ TFT ------	// 
//	     ULONG		Level		// backlight driver level
//	     UCHAR		RFU[12];
// OUTPUT  : none.
// RETURN  : apiOK
//	     apiFailed
// ---------------------------------------------------------------------------
#if	0
void	OS_LCDTFT_BackLight_Task( void )
{
UINT32		result;
SPI_LCDBL	spibl;

	
	if( !os_LCDTFT_BL_FLAG )
	  return;

	if( os_LCDTFT_InUse || os_LCDTFT_InUse2 )
	  return;

//	BSP_Delay_n_ms( 20 );
	
	spibl.DevID = DID_LCD;
	spibl.CmdID = CID_LCD_BACKLIT;
	memmove( (UINT8 *)&spibl.RectPara, (UINT8 *)&os_LCDTFT_BL_PARA, sizeof(API_LCDTFT_BACKLIT) );

	result = OS_SPI_PutPacket( sizeof(SPI_LCDBL), (UINT8 *)&spibl );
	
	os_LCDTFT_BL_FLAG = FALSE;
	
	BSP_Delay_n_ms( 20 );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To write the given image data to the display at the designated area.
// INPUT   : dhn
//	     The specified device handle number.
//
//	     dim
//	     ------ TFT ------	// 
//	     UINT		ID;		// id number of the built-in icon (0..n)
//	     UINT		Width;		// image width in dots in horizontal direction
//	     UINT		Height;		// image height in dots in vertical direction
//	     ULONG		Size;		// size of the image in bytes (max 320x240x2 = 153600)
//	     UCHAR		RGB;		// RGB mode
//	     UCHAR		RFU[5];
//
//	     image
//	     the image data. (in RGB format)
// 
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
#if	0
UINT32	OS_LCDTFT_PutGraphics( API_LCDTFT_GRAPH dim, UINT8 *pImage )
{
UINT32	result;
UINT32	length;
UINT8	*pData;
UINT8	*pData2;


	result = FALSE;
	
	length = dim.Size;
	
	pData = BSP_Malloc( sizeof(SPI_LCDTFT_GRAPH) + length );
	if( pData )
	  {
	  pData2 = pData;
	  
	  // put packet header
	  *pData2++ = DID_LCD;
	  *pData2++ = CID_LCD_PUT_GRAPHICS;
	  	  
	  memmove( pData2, (UINT8 *)&dim, sizeof(API_LCDTFT_GRAPH) );
	  
	  pData2 += sizeof(API_LCDTFT_GRAPH);		// ptr to DATA
	  
	  // put packet data
	  memmove( pData2, pImage, length );
	  
	  if( OS_SPI_PutPacket( sizeof(SPI_LCDTFT_GRAPH)+length, pData ) )
	    result = TRUE;
	    
	  BSP_Free( pData );
	  }
	  
	return( result );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To write the given image data to the display at the designated area.
// INPUT   : dhn
//	     The specified device handle number.
//
//	     image
//	     ------ TFT ------	// 
//	     UINT		ID;		// id number of the built-in icon (0..n)
//	     UCHAR		RGB;		// RGB mode
//
//	     GUI_BITMAP
//		UINT		XSize;			// width  size in dots
//		UINT		YSize;			// height size in dots
//		UINT		BytesPerLine;		// bytes per line
//		UINT		BitsPerPixel;		// bits  per pixel
//		UCHAR		*pData;			// pointer to picture data
//		UCHAR		*pPal;			// pointer to palette
//		UCHAR		*pMethods;		// pointer to methods
// 
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
#if	0
//UINT8	*pData;
//UINT8	*pData2;
UINT32	OS_LCDTFT_PutGraphics( API_LCDTFT_GRAPH graph )
{
UINT32	result;
UINT32	length;
UINT32	offset = 0;
UINT32	cnt = 0;
UINT32	left_bytes = 0;
UINT32	datalen = 0;
UINT8	*pData;
UINT8	*pData2;
UINT32	bmpSize = 0;


//	BSP_Delay_n_ms(10);

	result = FALSE;
	
	length = (graph.Bitmap.BytesPerLine) * (graph.Bitmap.YSize);	// total size of the image data

        cnt = length / 1024;		// max payload length in one packet = 1024 bytes
        left_bytes = length % 1024;
	
	pData = BSP_Malloc( sizeof(SPI_LCDTFT_GRAPH) + 1024 );
	if( pData )
	  {
START:
//	  pData2 = pData;
	  
	  if( cnt )
	    datalen = 1024;
	  else
	    datalen = left_bytes;
	    
START2:
	  pData2 = pData;
	  
	  // put packet header
	  *pData2++ = DID_LCD;
	  *pData2++ = CID_LCD_PUT_GRAPHICS;

	  // ---------------------------------------------------------------------------
	  //	UINT		ID;		// id number of the built-in icon (0..n)
	  //	UINT		Width;		// image width in dots in horizontal direction
	  //	UINT		Height;		// image height in dots in vertical direction
	  //	ULONG		Size;		// total size of the image in bytes
	  //	UCHAR		RGB;		// RGB mode
	  //	ULONG		Offset;		// offset address (0..x)
	  //	UCHAR		RFU[1];
	  // ---------------------------------------------------------------------------

	  *pData2++ = (graph.ID) & 0x00FF;
	  *pData2++ = ((graph.ID) & 0xFF00) >> 8;
	  
	  *pData2++ = (graph.Bitmap.XSize) & 0x00FF;
	  *pData2++ = ((graph.Bitmap.XSize) & 0xFF00) >> 8;
	  
	  *pData2++ = (graph.Bitmap.YSize) & 0x00FF;
	  *pData2++ = ((graph.Bitmap.YSize) & 0x00FF) >>8;
	  
//	  *pData2++ = datalen & 0x000000FF;
//	  *pData2++ = (datalen & 0x0000FF00) >> 8;
//	  *pData2++ = (datalen & 0x00FF0000) >> 16;
//	  *pData2++ = (datalen & 0xFF000000) >> 24;

	  bmpSize = graph.Bitmap.XSize * graph.Bitmap.YSize * 2;	// 2016-10-28
	  *pData2++ = bmpSize & 0x000000FF;
	  *pData2++ = (bmpSize & 0x0000FF00) >> 8;
	  *pData2++ = (bmpSize & 0x00FF0000) >> 16;
	  *pData2++ = (bmpSize & 0xFF000000) >> 24;
	  
	  *pData2++ = graph.RGB;
	  
	  *pData2++ = offset & 0x000000FF;
	  *pData2++ = (offset & 0x0000FF00) >> 8;
	  *pData2++ = (offset & 0x00FF0000) >> 16;
	  *pData2++ = (offset & 0xFF000000) >> 24;
	  
	  *pData2++ = 0;	// RFU(1)
	  
	  // put packet data
	  memmove( pData2, graph.Bitmap.pData + offset, datalen );
	  
	  os_LCDTFT_InUse = TRUE;
	  if( OS_SPI_PutPacket( sizeof(SPI_LCDTFT_GRAPH)+datalen, pData ) )
	    {
	    os_LCDTFT_InUse = FALSE;
	    result = TRUE;
	    }
	  else
	    {
	    os_LCDTFT_InUse = FALSE;
	    result = FALSE;
	    goto EXIT;
	    }
	    
	  if( cnt == 0 )
	    result = TRUE;	// done
	  else
	    {
	    cnt--;
	    if( cnt == 0 )
	      {
	      if( left_bytes == 0 )
	        {
	        result = TRUE;
	        goto EXIT;
		}
	      else
	        {
	        offset += datalen;	// next packet data
	        
	        datalen = left_bytes;
	        left_bytes = 0;
	        goto START2;
		}
	      }
	      
	    offset += datalen;	// next packet data
	    goto START;
	    }
EXIT:
	  BSP_Free( pData );
	  }

	return( result );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To read the given image data from the display at the designated area.
// INPUT   : dhn
//	     The specified device handle number.
//
//	     dim
//	     ------ TFT ------	// 
//	     UINT		Xleft;		// start pixel column coordinate at the upper left
//	     UINT		Ytop;		// start pixel row coordinate at the upper left
//	     UINT		Width;		// width  size in dots
//	     UINT		Height;		// height size in dots
//	     UCHAR		RGB;		// RGB mode
//	     UCHAR		*pBuf;		// storage for bmp
//
// OUTPUT  :
//	     graph
//	     ------ TFT ------	// 
//	     UINT		ID;		// id number of the built-in icon (always 0)
//	     UCHAR		RGB;		// RGB mode (always RGB_BPP16)
//
//	     GUI_BITMAP
//		UINT		XSize;			// width  size in dots
//		UINT		YSize;			// height size in dots
//		UINT		BytesPerLine;		// bytes per line
//		UINT		BitsPerPixel;		// bits  per pixel
//		UCHAR		*pData;			// pointer to picture data
//		UCHAR		*pPal;			// pointer to palette (RFU)
//		UCHAR		*pMethods;		// pointer to methods (RFU)
//
// RETURN  : apiOK
//	     apiFailed
// ---------------------------------------------------------------------------
#if	0
UINT32	OS_LCDTFT_GetGraphics( API_LCDTFT_GRAPH_DIM dim, API_LCDTFT_GRAPH *graph )
{
	  // ---------------------------------------------------------------------------
	  //	UINT		X;		//
	  //	UINT		Y;		//
	  //	UINT		Width;		// image width in dots in horizontal direction
	  //	UINT		Height;		// image height in dots in vertical direction
	  //	ULONG		Offset;		// offset address (0..x)
	  //	UCHAR		RGB;		// 
	  //	UCHAR		RFU[3];		//
	  // ---------------------------------------------------------------------------
	  
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To transform WIN bmp to SYMLINK raw bmp format.
// INPUT   : winbmp
//	     The Windows format BMP data. (monochrome, black & white)
// OUTPUT  : rawbmp
//	     The raw bmp format. (for LCD)
// RETURN  : NULLPTR
//	     pointer to new raw bmp.
// ---------------------------------------------------------------------------
#if	0
//UINT8	*pImageData;
//UINT8	*pImageData2;
//UINT32	SizeOfImage = 0;
//UINT32	BytesPerRow = 0;

UINT8	*TransformBitmapFile( UINT8 *winbmp, UINT32 *xsize, UINT32 *ysize )
{
UINT32	i, j, k, l, m;
UINT32	right_shift, left_shift;
UINT8	*status = 0;
UINT32	numread;
UINT32	numread2;
UINT32	actualbytes;
UINT32	padding;
UINT32	numwrite;
UINT32	length = 0;
//UINT8	*pImageData;
//UINT8	*pImageData2;
UINT8	*pData;
UINT8	*pData2;
UINT8	data1, data2;
UINT8	c1, c2;

UINT8	*pWinBmp = 0;
UINT8	buffer1[14];
UINT8	buffer2[40];
UINT8	buffer3[4];
UINT8	buffer4[4];
BMPFILEHEADER	*bmpFileHeader = 0;
BMPINFOHEADER	*bmpInfoHeader = 0;
BMPRGBQUAD	*bmpRgbQuad1 = 0;
BMPRGBQUAD	*bmpRgbQuad2 = 0;

//UINT32	SizeOfImage = 0;
//UINT32	BytesPerRow = 0;

UINT8	*pImageData;
UINT8	*pImageData2;
UINT32	SizeOfImage = 0;
UINT32	BytesPerRow = 0;


	bmpFileHeader = (BMPFILEHEADER *)&buffer1;
	bmpInfoHeader = (BMPINFOHEADER *)&buffer2;
	bmpRgbQuad1 = (BMPRGBQUAD *)&buffer3;
	bmpRgbQuad2 = (BMPRGBQUAD *)&buffer4;
	pWinBmp = winbmp;
	
	// parse BMP file header
//	numread = fread( &bmpFileHeader, sizeof( char ), sizeof(BMPFILEHEADER), InputFile );
	numread = sizeof(BMPFILEHEADER);
	memmove( bmpFileHeader, pWinBmp, numread );
	if( numread == sizeof(BMPFILEHEADER) )
	  {
	  length += numread;
	  pWinBmp += numread;
	  
	  if( (bmpFileHeader->bfType == 0x4D42) && // (bmpFileHeader->bfSize == FileSize) && 
	      (bmpFileHeader->bfReserved1 == 0) && (bmpFileHeader->bfReserved2 == 0) )
	    {
	    // parse BMP image header
//	    numread = fread( &bmpInfoHeader, sizeof( char ), sizeof(BMPINFOHEADER), InputFile );
	    numread = sizeof(BMPINFOHEADER);
	    memmove( bmpInfoHeader, pWinBmp, numread );
	    if( numread == sizeof(BMPINFOHEADER) )
	      {
	      length += numread;
	      pWinBmp += numread;
	      
	      if( (bmpInfoHeader->biSize >= 40) && (bmpInfoHeader->biPlanes == 1) &&
	          (bmpInfoHeader->biBitCount == 1) && (bmpInfoHeader->biCompression == 0) && 
	          (bmpInfoHeader->biSizeImage != 0) && (bmpInfoHeader->biClrUsed == 0) && (bmpInfoHeader->biClrImportant == 0) )
	        {
	        // parse BMP RGB Quad
//	        numread  = fread( &bmpRgbQuad1, sizeof( char ), sizeof(bmpRgbQuad1), InputFile );
//	        numread += fread( &bmpRgbQuad2, sizeof( char ), sizeof(bmpRgbQuad2), InputFile );
		numread = sizeof(BMPRGBQUAD);
		memmove( bmpRgbQuad1, pWinBmp, sizeof(BMPRGBQUAD) );
		pWinBmp += sizeof(BMPRGBQUAD);
		memmove( bmpRgbQuad2, pWinBmp, sizeof(BMPRGBQUAD) );
		pWinBmp += sizeof(BMPRGBQUAD);
		numread += sizeof(BMPRGBQUAD);
	        if( numread == sizeof(BMPRGBQUAD)*2 )
	          {
	          length += numread;

	          *xsize = bmpInfoHeader->biWidth;
	          *ysize = bmpInfoHeader->biHeight;
	          
	          SizeOfImage = bmpInfoHeader->biSizeImage;	// size of image data
	          BytesPerRow = bmpInfoHeader->biWidth / 8;	// bytes per row of the graphics (shall be multiple of 4 bytes)
	          
		  padding = BytesPerRow % 4;
		  if( padding )
		    padding = 4 - padding;
		    
//		  printf("\npadding=%d\n", padding );
//		  exit(0);
	          
	          if( ((pImageData  = (UINT8 *)BSP_Malloc( SizeOfImage*sizeof(char)) ) != NULL) &&
	              ((pImageData2 = (UINT8 *)BSP_Malloc( SizeOfImage*sizeof(char)) ) != NULL) )
	            {
	            // read all image data to the working buffer
//	            fseek( InputFile, bmpFileHeader.bfOffBits, SEEK_SET );	// move to the beginning of image data	            
//	            numread = fread( pImageData, sizeof( char ), SizeOfImage, InputFile );
		    pWinBmp = winbmp + bmpFileHeader->bfOffBits;
		    numread = SizeOfImage;
		    memmove( pImageData, pWinBmp, numread );
	            if( numread == SizeOfImage )
	              {
	              // get rid of paddings if available
	              if( padding )
	                {
	                numread2 = numread;
	                actualbytes = 0;
	                pData  = pImageData;
	                pData2 = pImageData2;
	                while( numread2 )
	                     {
	                     memmove( pData2, pData, BytesPerRow );
	                     pData2 += BytesPerRow;
	                     pData += (BytesPerRow + padding);
	                     numread2 -= (BytesPerRow + padding);
	                     actualbytes += (BytesPerRow + padding);
	                     }
	                memmove( pImageData, pImageData2, actualbytes );	// final image data without paddings
	                }
	              
	              // upside down and inverse data
	              if( (bmpRgbQuad1->rgbBlue == 0xFF) && (bmpRgbQuad1->rgbGreen == 0xFF) && (bmpRgbQuad1->rgbRed == 0xFF) )
	                {
	                k = bmpInfoHeader->biHeight - 1;
	                for( i=0; i<(UINT32)(bmpInfoHeader->biHeight/2); i++, k-- )
	                   {
	                   for( j=0; j<BytesPerRow; j++ )
	                      {
	                      data1 = pImageData[i*BytesPerRow+j];	// up data <-> down data
	                      data2 = pImageData[k*BytesPerRow+j];	//
	                                  
	                      pImageData[i*BytesPerRow+j] = data2;
	                      pImageData[k*BytesPerRow+j] = data1;
	                      }
	                   }
	                }
	              else
	                {
	                for( i=0; i<(UINT32)(bmpInfoHeader->biHeight*BytesPerRow); i++ )
	                   {
	                   pImageData[i] = ~pImageData[i];
	                   }
	                
	                k = bmpInfoHeader->biHeight - 1;
	                for( i=0; i<(UINT32)(bmpInfoHeader->biHeight/2); i++, k-- )
	                   {
	                   for( j=0; j<BytesPerRow; j++ )
	                      {
	                      data1 = pImageData[i*BytesPerRow+j];	// up data <-> down data
	                      data2 = pImageData[k*BytesPerRow+j];	//
	                                  
	                      pImageData[i*BytesPerRow+j] = data2;
	                      pImageData[k*BytesPerRow+j] = data1;
	                      }
	                   }
	                }
		         
//		         for( i=0; i< 320; i++ )
//	                    printf( "%2X ", pImageData[i] );    
//	                 printf("\nHere");          	 
//	                 goto DONE;
	                 
#if	0
	              // bits orientation for monochrome DISPLAY
//	              if( option_device == DEV_DISPLAY )
//	                {
	                m = 0;
	                for( i=0; i<(UINT32)(bmpInfoHeader->biHeight/8); i++ )
	                   {
	                   for( j=0; j<BytesPerRow; j++ )
	                      {
	                      right_shift = 7;
	                      
	                      for( k=0; k<8; k++ )
	                         {
	                         left_shift  = 0;
	                         c2 = 0;
	                         
	                         for( l=0; l<8; l++ )
	                            {
	                            c1 = (pImageData[(i*BytesPerRow*8) + j + (l*BytesPerRow)] >> right_shift) & 0x01;
	                            c1 <<= left_shift;
	                            c2 |= c1;
	                            
	                            left_shift++;
	                            }
	                            
	                         pImageData2[m++] = c2;
	                         right_shift--;
	                         }
	                      }
	                   }
//	                }
#endif

#if	1
//	              else	// bits orientation for PRINTER & TFTLCD
//	                {
//	                if( (bmpRgbQuad1.rgbBlue == 0xFF) && (bmpRgbQuad1.rgbGreen == 0xFF) && (bmpRgbQuad1.rgbRed == 0xFF) )
	                  memmove( pImageData2, pImageData, BytesPerRow*bmpInfoHeader->biHeight );
/*	                else
	                  {
	                  // swap bits in byte and repositon bytes in row (left <-> right)
	                  j = BytesPerRow - 1;
	                  for( i=0; i<bmpInfoHeader.biHeight; i++ )
	                     {
	                     for( j=0; j<BytesPerRow/2; j++ )
	                        {
	                        data1 = pImageData[i*BytesPerRow + j];
	                        data2 = pImageData[i*BytesPerRow + (BytesPerRow-1-j)];
	                        
				data1 = ( ((data1 >> 7) & 0x01) | ((data1 >> 5) & 0x02) | ((data1 >> 3) & 0x04) | ((data1 >> 1) & 0x08) |
				          ((data1 << 1) & 0x10) | ((data1 << 3) & 0x20) | ((data1 << 5) & 0x40) | ((data1 << 7) & 0x80) );
				         
				data2 = ( ((data2 >> 7) & 0x01) | ((data2 >> 5) & 0x02) | ((data2 >> 3) & 0x04) | ((data2 >> 1) & 0x08) |
				          ((data2 << 1) & 0x10) | ((data2 << 3) & 0x20) | ((data2 << 5) & 0x40) | ((data2 << 7) & 0x80) );
				
				pImageData2[i*BytesPerRow + j] = data2;
				pImageData2[i*BytesPerRow + (BytesPerRow-1-j)] = data1;
	                	}
	                     }
	                  }
*/	                  
//	                }
#endif
	              
/*	              
	              for( i=0; i< 320; i++ )
	                 printf( "%2X ", pImageData2[i] );
	       
DONE:  
	              fcloseall();
	              free( pImageData );
	              free( pImageData2 );
	              exit(0);
*/


		      // start to output text file (C header file)
//		      status = GenerateOutputFile_C( pImageData2, BytesPerRow*bmpInfoHeader.biHeight, OutputFile );
	              }
	              
	            BSP_Free( pImageData );
//	            BSP_Free( pImageData2 );
	            
	            status = pImageData2;
	            }
	          }
	        }
	      }
	    }
	  }
	  
	return( status );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: convert BPP24 to BPP16.
//		BPP 24: rrrrrrrr gggggggg bbbbbbbb
//		BPP 16: 0bbbbbgg gggrrrrr
// INPUT   : bpp24	- 3 bytes
// OUTPUT  : bpp16	- 2 bytes
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	0
void	RGB_BPP24to16( UINT8 *bpp24, UINT8 *bpp16 )
{
	if( (bpp24[0] == 0xFF) && (bpp24[1] == 0xFF) && (bpp24[2] == 0xFF) )
	  {
	  bpp16[1] = 0xFF;
	  bpp16[0] = 0xFF;
	  }
	else
	  {
	  bpp16[1] = ((bpp24[2] & 0xF8) >> 1) | ((bpp24[1] & 0xC0) >> 6);
	  bpp16[0] = ((bpp24[1] & 0x38) << 2) | ((bpp24[0] & 0xF8) >> 3);
	  }
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To write the given Windows bmp image data to the display at the designated area.
// INPUT   : id
//	     id number of the built-in icon (0..n)
//	     winbmp
//	     The Windows format BMP data. (monochrome, black & white)
// 
// OUTPUT  : none.
// RETURN  : NULLPTR	- failed
//	     pointer to GUI GRAPH structure.
// ---------------------------------------------------------------------------
#if	0
//UINT32			xsize;
//UINT32			ysize;
//UINT32			xsize0;
//UINT32			ysize0;
//UINT32			BytesPerLine;
//UINT32			BmpDim;

UINT32	OS_LCDTFT_ConvertWIN2GUI( UINT16 id, UINT16 CCWdegrees, UCHAR *fg_palette, UCHAR *bg_palette, UINT16 *width, UINT16 *height, UINT8 *winbmp )
{
UINT32	i;
UINT32	j;
UINT8			*pRawBmp = 0;
UINT8			*pRawBmpBak = 0;
UINT8			*pRgbBmp = 0;
UINT8			*pRgbBmpBak = 0;
API_LCDTFT_GRAPH	graph;
GUI_BITMAP		guibmp;
//UINT32			xsize;
//UINT32			ysize;
//UINT32			BytesPerLine;
//UINT32			BmpDim;
UINT8			data;

UINT32			xsize;
UINT32			ysize;
UINT32			xsize0;
UINT32			ysize0;
UINT32			BytesPerLine;
UINT32			BmpDim;

UINT8			bpp24[3];
UINT8			fg_bpp16[2];
UINT8			bg_bpp16[2];


	pRawBmp = TransformBitmapFile( winbmp, &xsize, &ysize );
	if( pRawBmp )
	  {
	  pRawBmpBak = pRawBmp;
	  
	  xsize0 = xsize;
	  ysize0 = ysize;
	  
	  // rotate CCW degrees
	  switch( CCWdegrees )
	        {
	        case CCW_0:
	             
	             *width = ysize0;
	             *height = xsize0;
	             
	             break;
	             
	        case CCW_90:
	        
	             OS_LCDTFT_TransformCharBitmap( ysize, xsize, pRawBmp );
		     
		     *width = xsize0;
		     *height = ysize0;
		     
		     xsize = ysize0;
	             ysize = xsize0;
	             
	             break;
	             
	        case CCW_180:
	             
	             OS_LCDTFT_TransformCharBitmap( ysize, xsize, pRawBmp );
	             OS_LCDTFT_TransformCharBitmap( xsize, ysize, pRawBmp );
	             
	             *width = ysize0;
	             *height = xsize0;
	             
	             xsize = xsize0;
	             ysize = ysize0;
	             
	             break;
	             
	        case CCW_270:

	             OS_LCDTFT_TransformCharBitmap( ysize, xsize, pRawBmp );
	             OS_LCDTFT_TransformCharBitmap( xsize, ysize, pRawBmp );
	             OS_LCDTFT_TransformCharBitmap( ysize, xsize, pRawBmp );
	             
	             *width = xsize0;
	             *height = ysize0;
	             
	             xsize = ysize0;
	             ysize = xsize0;
	             
	             break;
	             
//	        case 360:
//
//	             OS_LCDTFT_TransformCharBitmap( ysize, xsize, pRawBmp );
//	             OS_LCDTFT_TransformCharBitmap( xsize, ysize, pRawBmp );
//	             OS_LCDTFT_TransformCharBitmap( ysize, xsize, pRawBmp );
//	             OS_LCDTFT_TransformCharBitmap( xsize, ysize, pRawBmp );
//	             
//	             xsize = xsize0;
//	             ysize = ysize0;
//	             
//	             break;
	             
	        default:
	        
	             BSP_Free( pRawBmpBak );
	             return( FALSE );             
	        }
	  
	  
//	  OS_LCDTFT_TransformCharBitmap( ysize, xsize, pRawBmp );
	  
	  // expand raw bitmap to RGB bitmap (BMP555)
	  memmove( bpp24, fg_palette, 3 );
	  RGB_BPP24to16( bpp24, fg_bpp16 );
	  memmove( bpp24, bg_palette, 3 );
	  RGB_BPP24to16( bpp24, bg_bpp16 );
	  
	  BytesPerLine = xsize*2;
	  BmpDim = xsize*ysize;
	  pRgbBmp  = (UINT8 *)BSP_Malloc( BmpDim*sizeof(char)*16 );
	  if( pRgbBmp )
	    {
	    pRgbBmpBak = pRgbBmp;
	    
	    for( i=0; i<BmpDim/8; i++ )
	       {
	       data = *pRawBmp++;
	       for( j=0; j<8; j++ )
	          {
	          if( (data << j) & 0x80 )
	            {
	            // foreground
	            
//	            *pRgbBmp++ = 0x00;	// black
//	            *pRgbBmp++ = 0x00;
		    
		    *pRgbBmp++ = fg_bpp16[0];
		    *pRgbBmp++ = fg_bpp16[1];
	            }
	          else
	            {
	            // background
	            
//	            *pRgbBmp++ = 0xFF;	// white
//	            *pRgbBmp++ = 0xFF;

		    *pRgbBmp++ = bg_bpp16[0];
		    *pRgbBmp++ = bg_bpp16[1];
	            }
	          }
	       }
	    }
	 
	  
#if	1
	  graph.ID	= id;
	  graph.RGB	= 0;
	  graph.Bitmap.XSize		= xsize;
	  graph.Bitmap.YSize		= ysize;
	  graph.Bitmap.BytesPerLine	= xsize*2;
	  graph.Bitmap.BitsPerPixel	= 16;
	  graph.Bitmap.pData		= pRgbBmpBak;
	  graph.Bitmap.pPal		= NULL;
	  graph.Bitmap.pMethods	= GUI_DRAW_BMP555;
	  
	  OS_LCDTFT_PutGraphics( graph );
	  
	  BSP_Free( pRgbBmpBak );
	  
	  BSP_Free( pRawBmpBak );
	  
//	  memmove( &graph.Bitmap, &bmtest0124CCW, sizeof(GUI_BITMAP) );
#endif
	  return( TRUE );
	  }
	else
	  return( FALSE );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To control related graphics appearance for PCD application,
//	     such as TAP ICON, LED ICON...
// INPUT   : dhn
//	     The specified device handle number.
//
//	     para
//	     ------ TFT ------	// 
//	     UINT		ID;		// id number of the built-in icon (0..n)
//	     UINT		Xleft;		// start pixel column coordinate at the upper left
//	     UINT		Ytop;		// start pixel row coordinate at the upper left
//	     UINT		Method;		// method of writing graphics
//	     UINT		Width;		//
//	     UINT		Height;		//
//	     UCHAR		RFU[4];		//
// OUTPUT  : none.
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
#if	0
UINT32	OS_LCDTFT_ShowICON( API_LCDTFT_ICON icon )
{
UINT32	result;
UINT32	delay;
SPI_LCDICON	spiicon;

	
	spiicon.DevID	= DID_LCD;
	spiicon.CmdID	= CID_LCD_SHOW_IMAGE;
	memmove( (UINT8 *)&spiicon.IconPara, (UINT8 *)&icon, sizeof(API_LCDTFT_ICON) );
	
	spiicon.IconPara.Xleft	= icon.Ytop;
	spiicon.IconPara.Ytop	= MAX_LCDTFT_PIXCOL_NO - icon.Xleft - icon.Width;

	os_LCDTFT_InUse = TRUE;
	result = OS_SPI_PutPacket( sizeof(SPI_LCDICON), (UINT8 *)&spiicon );
	
//	BSP_Delay_n_ms(100);

	delay = ((icon.Width * icon.Height * 14)/10) + 2000;	// 2016-11-17
	BSP_Delay_n_us( delay );
	
	os_LCDTFT_InUse = FALSE;
	
	return( result );
}
#endif
