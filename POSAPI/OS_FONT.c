//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L (32-bit Platform)                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : OS_FONT.C                                                  **
//**  MODULE   : 			                                    **
//**									    **
//**  FUNCTION : System Font Management.                                    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2007/08/28                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2007 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include "OS_FONT.h"

#include "POSAPI.h"
#include "DEV_LCD.h"

#include <string.h>
//#include "POSAPI.h"	// TEST ONLY

#ifdef	_LCD_ENABLED_
#define	FULL_BIG5_CHAR_NUM			13503		// as defined in \PEDK\POSTFUNC.c
#define	ADDR_FULL_BIG5_CODE_TABLE		0x10D90000	//
#define	ADDR_FULL_PRT_FONT2_24X24_BMP		0x10DA0000	//
#define	ADDR_FULL_PRT_FONT4_16X16_BMP		0x10E90000	//
#endif


//extern	UINT8	FONT4_ASCII_8x16[];
//extern	UINT8	FONT2_ASCII_12x24[];
//extern const UCHAR FONT2_ASCII_12x24[];
//extern	UINT8	const	ASCII_FONT_8X16[4096];
//extern const UCHAR FONT4_ASCII_8x16[];
//extern	UINT8	const	ASCII_FONT_12X24[4608];
extern	unsigned char ext_font816[];

OS_FDT		FDTI[1];			// FDT image
OS_FDT		FDT[MAX_FDT_NO];		// system default FDTs

UINT8	os_LCD_DID  =1;

//#define		LCDTFT_FONT0			FONT4		// printer font 4 (8x16 , 20 row x 30 col
//#define		LCDTFT_FONT1			FONT2		// printer font 2 (12x24, 13 row x 20 col)

		
// ---------------------------------------------------------------------------
// FUNCTION: To build the system default FDT 0 & 1.
// INPUT   : devid - device id.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_FONT_Init( UINT8 devid )
{
UINT32	i;
API_LCD_FONT	lcdft;

	
	// os_LCD_DID = devid;
	
	for( i=0; i<MAX_FDT_NO; i++ )
	   FDT[i].FontID = 0x00;

	//if( devid == LCD_DID_STN )
#if	0	// JAMES
	  // Init FONT0 (8x6)
	  FDT[FONT0].FontID = FONT_0;
	  FDT[FONT0].ByteNo = FONT6X8_BMP_LEN;
	  FDT[FONT0].Width = 6;
	  FDT[FONT0].Height = 8;
	  FDT[FONT0].BmpStAddr = (UINT8 *)&ASCII_FONT_6X8[0];
	  FDT[FONT0].Cross = 10;
	  
	  // Init FONT1 (16x8)
	  FDT[FONT1].FontID = FONT_1;
	  FDT[FONT1].ByteNo = FONT8X16_BMP_LEN;
	  FDT[FONT1].Width = 8;
	  FDT[FONT1].Height = 16;
	  #ifdef _build_DSS_
	  FDT[FONT1].BmpStAddr = (UINT8 *)&ext_font816[0];
	  #else
	  FDT[FONT1].BmpStAddr = (UINT8 *)&ASCII_FONT_8X16[0];
	  #endif
	  FDT[FONT1].Cross = 8;
	  

	  // Init FONT2
	  FDT[FONT2].FontID = FONT_2;
	  FDT[FONT2].ByteNo = FONT24X24_BMP_LEN;
	  FDT[FONT2].Width  = FONT2_W;
	  FDT[FONT2].Height = FONT2_H;
	  FDT[FONT2].CodStAddr  = (UINT8 *)0;
	  FDT[FONT2].CodEndAddr = (UINT8 *)0;
	  FDT[FONT2].BmpStAddr  = (UINT8 *)0;
	  FDT[FONT2].BmpEndAddr = (UINT8 *)0;
	  FDT[FONT2].Cross = 0;
	//   printf("FONT2=%d  FONT2_W=%d  FONT2_H=%d",FONT2,FONT2_W,FONT2_H);
	  // Init FONT4
	  FDT[FONT4].FontID = FONT4;
	  FDT[FONT4].ByteNo = FONT16X16_BMP_LEN;
	  FDT[FONT4].Width = FONT4_W;
	  FDT[FONT4].Height = FONT4_H;
	  FDT[FONT4].CodStAddr  = (UINT8 *)0;
	  FDT[FONT4].CodEndAddr = (UINT8 *)0;
	  FDT[FONT4].BmpStAddr  = (UINT8 *)0;
	  FDT[FONT4].BmpEndAddr = (UINT8 *)0;
	  FDT[FONT4].Cross = 0;

	  // Init FONT12
	  FDT[FONT12].FontID = FONT12;
	  FDT[FONT12].ByteNo = FONT12X24_BMP_LEN;
	  FDT[FONT12].Width = FONT12_W;
	  FDT[FONT12].Height = FONT12_H;
	  FDT[FONT12].BmpStAddr  = (UINT8 *)&ASCII_FONT_12X24[0];
	  FDT[FONT12].Cross = 0;
#else
	  // Init FONT0 (8x16)
	  FDT[FONT0].FontID = FONT_0;
	  FDT[FONT0].ByteNo = FONT8X16_BMP_LEN;
	  FDT[FONT0].Width = 8;
	  FDT[FONT0].Height = 16;
	  #ifdef _build_DSS_
	  FDT[FONT0].BmpStAddr = (UINT8 *)&ext_font816[0];
	  #else
	  FDT[FONT0].BmpStAddr = (UINT8 *)&ASCII_FONT_8X16[0];
	  #endif
	  FDT[FONT0].Cross = 0;
	  
	  // Init FONT1 (12x24)
	  FDT[FONT1].FontID = FONT_1;
	  FDT[FONT1].ByteNo = FONT12X24_BMP_LEN;
	  FDT[FONT1].Width = FONT12_W;
	  FDT[FONT1].Height = FONT12_H;
	  FDT[FONT1].BmpStAddr  = (UINT8 *)&ASCII_FONT_12X24[0];
	  FDT[FONT1].Cross = 0;
	  
	  // Init FONT10 (8x16)
	  FDT[FONT10].FontID = FONT_10;
	  FDT[FONT10].ByteNo = FONT8X16_BMP_LEN;
	  FDT[FONT10].Width = 8;
	  FDT[FONT10].Height = 16;
	  #ifdef _build_DSS_
	  FDT[FONT10].BmpStAddr = (UINT8 *)&ext_font816[0];
	  #else
	  FDT[FONT10].BmpStAddr = (UINT8 *)&ASCII_FONT_8X16[0];
	  #endif
	  FDT[FONT10].Cross = 0;
	  
	  // Init FONT11 (12x24)
	  FDT[FONT11].FontID = FONT_11;
	  FDT[FONT11].ByteNo = FONT12X24_BMP_LEN;
	  FDT[FONT11].Width = FONT12_W;
	  FDT[FONT11].Height = FONT12_H;
	  FDT[FONT11].BmpStAddr  = (UINT8 *)&ASCII_FONT_12X24[0];
	  FDT[FONT11].Cross = 0;
	  
	  
	// setup default TFT LCD Chinese font (FONT12, 24X24)
	memset( &lcdft, 0, sizeof(API_LCD_FONT) );

        lcdft.FontID = FONT2; // set to FONT2
        lcdft.ByteNo = 72;
        lcdft.Width  = 24;
        lcdft.Height = 24;
        lcdft.codPageNo  = 0x00;  // RFU
        lcdft.codStAddr  = 0x00;	//(UCHAR *)ADDR_FULL_BIG5_CODE_TABLE;
        lcdft.codEndAddr = 0x00;	//(UCHAR *)ADDR_FULL_BIG5_CODE_TABLE + 2*FULL_BIG5_CHAR_NUM - 1;
        lcdft.bmpPageNo  = 0x00;  // RFU
        lcdft.bmpStAddr  = 0x00;	//(UCHAR *)ADDR_FULL_PRT_FONT2_24X24_BMP;
        lcdft.bmpEndAddr = 0x00;	//(UCHAR *)ADDR_FULL_PRT_FONT2_24X24_BMP + 72*FULL_BIG5_CHAR_NUM - 1;
        
	api_lcd_initfont( lcdft );
#endif

/*
#ifndef	_LCD_ENABLED_

	//if( devid == LCD_DID_TFT )
	if( 1 )
	  {
	  // Init FONT0 (8x16)
	  FDT[LCDTFT_FONT0].FontID = LCDTFT_FONT0;		// printer FONT4
	  FDT[LCDTFT_FONT0].ByteNo = FONT8X16_BMP_LEN;
	  FDT[LCDTFT_FONT0].Width = 8;
	  FDT[LCDTFT_FONT0].Height = 16;
	  //FDT[LCDTFT_FONT0].BmpStAddr = (UINT8 *)&FONT4_ASCII_8x16[0];
	  FDT[LCDTFT_FONT0].BmpStAddr = (UINT8 *)&ASCII_FONT_8X16[0];
	  FDT[LCDTFT_FONT0].Cross = 0;
	  
	  // Init FONT1 (12x24)
	  FDT[LCDTFT_FONT1].FontID = LCDTFT_FONT1;		// printer FONT2
	  FDT[LCDTFT_FONT1].ByteNo = FONT12X24_BMP_LEN;
	  FDT[LCDTFT_FONT1].Width = 12;
	  FDT[LCDTFT_FONT1].Height = 24;
	  //FDT[LCDTFT_FONT1].BmpStAddr = (UINT8 *)&FONT2_ASCII_12x24[0];
	  FDT[LCDTFT_FONT1].BmpStAddr = (UINT8 *)&ASCII_FONT_12X24[0];
	  FDT[LCDTFT_FONT1].Cross = 0;
	  }
#endif*/

	//UseBootLoader_110();	// 2012-09-10, swich to BootLoader cipher lib
}

// ---------------------------------------------------------------------------
// FUNCTION: Get FDT start address by FID, and copy the table to FDI_Image.
// INPUT   : fid -- font ID.
// OUTPUT  : none.
// RETURN  : pointer to the target FDT.
// ---------------------------------------------------------------------------
OS_FDT	*OS_FONT_GetFdtAddr( UINT8 fid )
{
OS_FDT	*pFdt;
	
	pFdt = &FDT[fid];
//	memmove( FDTI, pFdt, FDT_IMAGE_LEN );
	// printf("@@fid=%d pFdt->Width=%d pFdt->Height=%d\n",fid,pFdt->Width,pFdt->Height);
	FDTI[0].FontID = pFdt->FontID;		// 2012-04-05
	FDTI[0].ByteNo = pFdt->ByteNo;
	FDTI[0].Width  = pFdt->Width;
	FDTI[0].Height = pFdt->Height;
	FDTI[0].CodStAddr  = pFdt->CodStAddr;
	FDTI[0].CodEndAddr = pFdt->CodEndAddr;
	FDTI[0].BmpStAddr  = pFdt->BmpStAddr;
	FDTI[0].BmpEndAddr = pFdt->BmpEndAddr;
	FDTI[0].Cross = pFdt->Cross;
	FDTI[0].RFU   = pFdt->RFU;
	
	return( pFdt );
}

// ---------------------------------------------------------------------------
// FUNCTION: Get & load bitmap image of font 0/1.
// INPUT   : pFdt -- pointer to the target FDT.
//           code -- char code.
//           pBuf -- image storage buffer.
// OUTPUT  : none.
// RETURN  : pointer to next position of image storage buffer.
// ---------------------------------------------------------------------------
UINT8	*OS_FONT_LoadFont01Bmp( OS_FDT *pFdt, UINT8 code, UINT8 *pBuf )
{
UINT32	i;
UINT8	ByteNo;
UINT8	*pBmpAddr;
	
	ByteNo = pFdt->ByteNo;

//	if( os_LCD_ATTR & LCD_ATTR_ISO )	// using ISO 8859 code defintions? (0x00..0xFF, defined in EMV L2)
//	  {
//	  pBmpAddr = pFdt->BmpStAddr + (code * ByteNo);	// ptr to target image address of the "code"
//	  for( i=0; i<ByteNo; i++ )
//	     *pBuf++ = *pBmpAddr++;
//	     
//	  return( pBuf );
//	  }

//	if( code > ASCII_ST_CHAR )		// printable ascii char?
//	  {
//	  if( code >= DOLLAR_SIGN_ST_CHAR )	// special dollar sign char?
//	    code -= 0x10;
//	      
//	  code -= ASCII_ST_CHAR;		// image offset
//	  pBmpAddr = pFdt->BmpStAddr + (code * ByteNo);	// ptr to target image address of the "code"
//	    
//	  for( i=0; i<ByteNo; i++ )
//	     *pBuf++ = *pBmpAddr++;
//	  }
//	else	// for non-printable ascii char 0x00..0x20, not supported, fill with NULL data
//	  {
//	  for( i=0; i<ByteNo; i++ )
//	     *pBuf++ = 0x00;
//	  }

	pBmpAddr = pFdt->BmpStAddr + (code * ByteNo);	// ptr to target image address of the "code"
	for( i=0; i<ByteNo; i++ )
	   *pBuf++ = *pBmpAddr++;
	  
	return( pBuf );
}

// ---------------------------------------------------------------------------
// FUNCTION: Get & load bitmap image of font 3~5.
// INPUT   : pFdt -- pointer to the target FDT.
//           code -- char code.
//           pBuf -- image storage buffer.
// OUTPUT  : none.
// RETURN  : pointer to next position of image storage buffer.
// ---------------------------------------------------------------------------
UINT8	*OS_FONT_LoadFontXBmp( OS_FDT *pFdt, UINT8 code, UINT8 *pBuf )
{
UINT32	i;
UINT32	flag;
UINT8	ByteNo;
UINT8	*pBmpAddr;
UINT8	*pCodAddr;
UINT8	*pCodEndAddr;

	
	ByteNo = pFdt->ByteNo;

	// 2012-04-05, locate the char code offset to the CodStAddr
	pCodAddr = pFdt->CodStAddr;	// start addr of code table
	pCodEndAddr = pFdt->CodEndAddr;	// end   addr of code table
	
	flag = 0; // flag for "found"
	for( i=0; i<256; i++ )
	   {
	   if( pCodAddr > pCodEndAddr )
	     break;	// end of search
	   
	   if( *pCodAddr == code )
	     {
	     flag = 1;	// found
	     break;
	     }
	     
	   pCodAddr++;	// next
	   }

	if( flag )
	  {
	  pBmpAddr = pFdt->BmpStAddr + (i * ByteNo);	// ptr to target image address of the "code"
	  for( i=0; i<ByteNo; i++ )
	     *pBuf++ = *pBmpAddr++;
	  }
	else
	  {
	  // fill with space pattern if not found
	  for( i=0; i<ByteNo; i++ )
	     *pBuf++ = 0x00;
	  }
	  
	return( pBuf );
}

// ---------------------------------------------------------------------------
// FUNCTION: Get & load bitmap image of font 3~5.
// INPUT   : pFdt -- pointer to the target FDT.
//           code -- unicode (1 to 4 bytes), according to pFdt.RFU.
//		     format: UTF-32BE (big endian)
//           pBuf -- image storage buffer.
// OUTPUT  : none.
// RETURN  : pointer to next position of image storage buffer.
// ---------------------------------------------------------------------------
UINT8	*OS_FONT_LoadFontXBmp_UTF( OS_FDT *pFdt, UINT8 *code, UINT8 *pBuf )
{
UINT32	i = 0;
UINT32	flag;
UINT8	ByteNo;
UINT8	*pBmpAddr;
UINT8	*pCodAddr;
UINT8	*pCodEndAddr;
UINT8	*pBufBak;
UINT32	code1;
UINT32	code2;

	
	pBufBak = pBuf;
	ByteNo = pFdt->ByteNo;

	// 2012-04-05, locate the char code offset to the CodStAddr
	pCodAddr = pFdt->CodStAddr;	// start addr of code table
	pCodEndAddr = pFdt->CodEndAddr;	// end   addr of code table

	switch( pFdt->RFU )
	      {
	      case 1:
	      	
	           code1 = code[0];
	           break;
	      	   
	      case 2:
	      	
	           code1 = code[0]*0x100 + code[1];
	           break;
	           
	      case 3:
	           
	           code1 = code[0]*0x10000 + code[1]*0x100 + code[2];
	           break;
	           
	      case 4:
	           
	           code1 = code[0]*0x1000000 + code[1]*0x10000 + code[2]*0x100 + code[3];
	           break;
	      
	      default:

	           code1 = *code;
	           break;
	      }

	flag = 0; // flag for "found"
//	for( i=0; i<256; i++ )
	while(1)
	   {
	   if( pCodAddr > pCodEndAddr )
	     break;	// end of search

	   switch( pFdt->RFU )
	         {
	         case 1:
	         	
	              code2 = pCodAddr[0];
	              pCodAddr += 1;
	              break;
	         	   
	         case 2:
	         	
	              code2 = pCodAddr[0]*0x100 + pCodAddr[1];
	              pCodAddr += 2;
	              break;
	              
	         case 3:
	              
	              code2 = pCodAddr[0]*0x10000 + pCodAddr[1]*0x100 + pCodAddr[2];
	              pCodAddr += 3;
	              break;
	              
	         case 4:
	              
	              code2 = pCodAddr[0]*0x1000000 + pCodAddr[1]*0x10000 + pCodAddr[2]*0x100 + pCodAddr[3];
	              pCodAddr += 4;
	              break;
	         
	         default:

	              code2 = *pCodAddr;
	              pCodAddr += 1;
	              break;
	         }
	      
	   if( code1 == code2 )
	     {
	     flag = 1;	// found
	     break;
	     }
	     
	   i++;
	   }

	if( flag )
	  {
	  pBmpAddr = pFdt->BmpStAddr + (i * ByteNo);	// ptr to target image address of the "code"
	  for( i=0; i<ByteNo; i++ )
	     *pBuf++ = *pBmpAddr++;

//	  if( !os_LCD_ConvertFont )	// not using printer font?
//	    OS_FONT_TransformCharBitmap( pFdt->Height, pFdt->Width, pBufBak );	// converted to printer font format when using LCD font directley
	  }
	else
	  {
	  // fill with space pattern if not found
	  for( i=0; i<ByteNo; i++ )
	     *pBuf++ = 0x00;
	  }
	  
	return( pBuf );
}

// ---------------------------------------------------------------------------
int compare( UINT8 *arg1, UINT8 *arg2 )
{
UINT16	code1;
UINT16	code2;

	code1 = *arg1 * 256 + *(arg1+1);
	code2 = *arg2 * 256 + *(arg2+1);

   	if( code1 == code2 )
   	  return( 0 );
   	
   	if( code1 > code2 )
   	  return( 1 );
   	else
   	  return( -1 );
}

// ---------------------------------------------------------------------------
// FUNCTION: Calculate and get the start bitmap address of the FONT2 by using
//	     binary search algorithm.
// INPUT   : pFdt   -- pointer to the target FDT.
//           HiByte -- 1'st byte of the 2-byte char.
//           LoByte -- 2'nd byte of the 2-byte char.
//           pBuf   -- image storage buffer.
// OUTPUT  : none.
// RETURN  : pointer to target bitmap address.
//	     NULLPOTR -- not found.
// ---------------------------------------------------------------------------
UINT8	*FONT_GetFont2BmpAddr( OS_FDT *pFdt, UINT8 HiByte, UINT8 LoByte )
{
char	*result;
UINT32	item;
UINT8	*target;
UINT8	*source;
UINT8	code[2];


	code[0] = HiByte;
	code[1] = LoByte;
	result = (char *)bsearch( (UINT8 *)&code[0], (UINT8 *)pFdt->CodStAddr, (int)((pFdt->CodEndAddr - pFdt->CodStAddr + 1)/2),
				2, (int (*)(const void*, const void*))compare );
	
//	source = pFdt->CodStAddr;
//	DEBUGPRINTF("\r\nSoruce=%x ", source );
//	target = (UINT8 *)result;
//	DEBUGPRINTF("\r\nTarget=%x ", target );
//	item = (target - source) / 2;
			
	if( result )
	  {
	  source = pFdt->CodStAddr;
	  target = (UINT8 *)result;
	  item = (target - source) / 2;
	  
	  target = pFdt->BmpStAddr + (item * (pFdt->ByteNo));
	  
	  return( target );	// found
	  }
	else
	  //return( NULLPTR );	// not found
	  return( 0 );	// not found
}

// ---------------------------------------------------------------------------
// FUNCTION: Transform printer image into display format.
// INPUT   : Height     - height of char font.
//           Width      - width of char font.
//           pImageData - the image data to be transformed.
// OUTPUT  : pImageData - the image data with new orientation.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	0
void	OS_FONT_TransformCharBitmap( UCHAR Height, UCHAR Width, UCHAR *pImageData )
{
UINT32	i, j, k, l, m, n;
UINT32	BytesPerRow;
UINT32	right_shift;
UINT32	left_shift;
UINT8	c1, c2;
UINT8	pImageData2[128];	// 32x32 char font


	BytesPerRow = Width / 8;
	for( i=0; i<(Height/8); i++ )
	   {
	   m = 0;
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
	            
	         pImageData2[(Height/8)*m + i] = c2;
	         m++;
	         right_shift--;
	         }
	      }
	   }
	   
	memmove( pImageData, pImageData2, Width*(Height/8) );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Get & load bitmap image of font 2 (e.g., the Chinese char).
// INPUT   : pFdt   -- pointer to the target FDT.
//           HiByte -- 1'st byte of the 2-byte char.
//           LoByte -- 2'nd byte of the 2-byte char.
//           pBuf   -- image storage buffer.
// OUTPUT  : none.
// RETURN  : pointer to next position of image storage buffer.
// ---------------------------------------------------------------------------
UINT8	*OS_FONT_LoadFont2Bmp( OS_FDT *pFdt, UINT8 HiByte, UINT8 LoByte, UINT8 *pBuf )
{
UINT8	*pBmp;
	
	pBmp = FONT_GetFont2BmpAddr( pFdt, HiByte, LoByte );
	if( pBmp )
	  {
	  memmove( pBuf, pBmp, pFdt->ByteNo );	// found, fill with bitmap data
#if	0
	  if( os_LCD_ConvertFont )		// 2012-03-09, bitmap transformation (prt to lcd)
	    OS_FONT_TransformCharBitmap( pFdt->Height, pFdt->Width, pBuf );
#endif
	  }
	else
	  memset( pBuf, 0x00, pFdt->ByteNo );	// not found, fill with NULL data
	  
	pBuf += (pFdt->ByteNo);
	
	return( pBuf );
}

// ---------------------------------------------------------------------------
/*
void	TestFont2(void)
{
OS_FDT	*pFdt;
UINT8	*pBmp;

API_FONT	ft;

	ft.FontID = FONT2;
	ft.ByteNo = 32;
	ft.Width = 16;
	ft.Height = 16;
	ft.codStAddr = (UINT8 *)FONT2_CODE;
	ft.codEndAddr = (UINT8 *)FONT2_CODE + 2*3 - 1;
	ft.bmpStAddr = (UINT8 *)FONT2_BMP;
	ft.bmpEndAddr = (UINT8 *)FONT2_BMP + 32*3 - 1;
	api_sys_initfont( ft );

//	pFdt = OS_FONT_GetFdtAddr( FONT_2 );
//	pBmp = FONT_GetFont2BmpAddr( pFdt, 0xA1, 0x40 );
//	if( pBmp )
//	  DEBUGPRINTF("\r\nA140 Found: %x\r\n", *pBmp );
//	else
//	  DEBUGPRINTF("\r\nA140 Not Found\r\n");
//	pBmp = FONT_GetFont2BmpAddr( pFdt, 0xA1, 0x41 );
//	if( pBmp )
//	  DEBUGPRINTF("\r\nA141 Found: %x\r\n", *pBmp );
//	else
//	  DEBUGPRINTF("\r\nA141 Not Found\r\n");
//	pBmp = FONT_GetFont2BmpAddr( pFdt, 0xA1, 0x42 );
//	if( pBmp )
//	  DEBUGPRINTF("\r\nA142 Found: %x\r\n", *pBmp );
//	else
//	  DEBUGPRINTF("\r\nA142 Not Found\r\n");
//	pBmp = FONT_GetFont2BmpAddr( pFdt, 0xA1, 0x43 );
//	if( pBmp )
//	  DEBUGPRINTF("\r\nA143 Found: %x\r\n", *pBmp );
//	else
//	  DEBUGPRINTF("\r\nA143 Not Found\r\n");
}
*/
