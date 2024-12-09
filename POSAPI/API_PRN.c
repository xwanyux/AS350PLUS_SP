//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L                                                       **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : API_PRN.C 	                                            **
//**  MODULE   : api_prt_open()				                    **
//**		 api_prt_close()					    **
//**		 api_prt_status()					    **
//**		 api_prt_putstring()					    **
//**		 api_prt_putgraphics()					    **
//**		 api_prt_setlinespace()					    **
//**		 api_prt_setcharspace()					    **
//**		 api_prt_cut()						    **
//**		 api_prt_initfont()					    **
//**									    **
//**  FUNCTION : API::PRT (Printer Module)		    		    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2008/04/22                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2008 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
//#include "bsp_prn.h"
#include "OS_PROCS.h"
#include "POSAPI.h"
#include "DEV_PRN.h"
#include "option.h"
#include "font_type.h"
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#define	FULL_BIG5_CHAR_NUM			13503		// as defined in \PEDK\POSTFUNC.c
#define	ADDR_FULL_BIG5_CODE_TABLE		0x105D0000	//
#define	ADDR_FULL_PRT_FONT2_24X24_BMP		0x105E0000	//
#define	ADDR_FULL_PRT_FONT4_16X16_BMP		0x106D0000	//
// UCHAR	prn_dumpbuffer[1024*1024];
int prn_count=0;
// extern const UCHAR FONT2_ASCII_12x24[];
// extern const UCHAR FONT4_ASCII_8x16[];
extern	struct FONT_PAIR FONT_TABLE[MAX_FONT];
UCHAR		os_DHN_PRN = 0;


// ---------------------------------------------------------------------------
// FUNCTION: To check if DHN matched.
// INPUT   : dhn
// OUTPUT  : none.
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
UCHAR	PRN_CheckDHN( UCHAR dhn )
{

	if( (dhn == 0) || (dhn == os_DHN_PRN) )
	  return( TRUE );
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: To enable the printer device and reset the interface controller.
// INPUT   : config (RFU) - e.g., settings for darkness, printer type, etc.
// OUTPUT  : none.
// RETURN  : DeviceHandleNo
//           apiOutOfService
// ---------------------------------------------------------------------------
UCHAR	api_prt_open( UCHAR type, UCHAR mode )
{
	
	if( os_DHN_PRN != 0 )	// already opened?
	  return( apiOutOfService );
	
	  if( OS_PRN_Open( type, mode ) == FALSE )
	    return( apiOutOfService );
	
	os_DHN_PRN = psDEV_PRN + 0x80;
	return( os_DHN_PRN );
}

// ---------------------------------------------------------------------------
// FUNCTION: To disable the printer device.
// INPUT   : dhn
//	     The specified device handle number.
//	     0x00 = to close all opened tasks.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_prt_close( UCHAR dhn )
{
UCHAR	status;

	status = apiFailed;
	
	if( os_DHN_PRN != 0 )
		if( PRN_CheckDHN( dhn ) == TRUE )
	  		if( OS_PRN_Close() )
	    	{
	   			os_DHN_PRN = 0;
	    		status = apiOK;
	    	}

	return( status );
}

// ---------------------------------------------------------------------------
// FUNCTION: To get the printer status.
// INPUT   : dhn
//	     The specified device handle number.
// OUTPUT  : dbuf
//	     Printer status byte.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_prt_status( UCHAR dhn, UCHAR *dbuf )
{
	if( PRN_CheckDHN( dhn ) == TRUE )
	  {
	  OS_PRN_Status( dbuf );
	  
	  return( apiOK );
	  }
	else
	  return( apiFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: To write the given character string to printing device 
//	     by the specified font.
// INPUT   : dhn
//	     The specified device handle number.
//	     fontid - printer font id.
//	     	      APS_FONT_8x8
//		      APS_FONT_12x10
//                    APS_FONT_7x8
//		      APS_FONT_8x14
//		      SII_FONT_8x16
//		      SII_FONT_12x24
//	     sbuf
//	     UCHAR  length;        // length of the given string.
//	     UCHAR  data[length];  // character string to be printed.
//
// OUTPUT  : none.
// RETURN  : apiOK
//	     apiFailed
//	     apiNotReady (out of system memory allocation, to be re-tried)
// ---------------------------------------------------------------------------
UCHAR	api_prt_putstring( UCHAR dhn, UCHAR fontid, UCHAR *sbuf )
{	
int i;
	if( PRN_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );

	if( OS_PRN_PutString( fontid, sbuf ) == TRUE )
	{
		// printf("fontid=%d\n",fontid);
		// memset(&prn_dumpbuffer[prn_count++],0x99,1);
		// memset(&prn_dumpbuffer[prn_count++],fontid,1);
		// for(i=0;i<sbuf[0];i++)
			// prn_dumpbuffer[prn_count++]=sbuf[i];
	  return( apiOK );
	}
	else
	  return( apiNotReady );
}

// ---------------------------------------------------------------------------
// FUNCTION: To write the given image data to printing device
//	     by the specified displacement.
// INPUT   : dhn
//	     The specified device handle number.
//
//	     dim
//	     The dimension of the graphics to be printed.
//	     ULONG   xmove;         // horizontal displacement in dots. (x8)
//	     ULONG   ymove ;        // vertical displacement in dots. (RFU)
//	     ULONG   width ;        // image width in dots in the horizontal direction. (x8)
//	     ULONG   height ;       // image height in dots  in the vertical direction. (x8)
//	     ULONG   mode ;         // image data mode.
//
//	     dbuf
//	     image data.
//
// OUTPUT  : none.
// RETURN  : apiOK
//	     apiFailed
//	     apiNotReady (out of system memory allocation, to be re-tried)
// ---------------------------------------------------------------------------
//UCHAR	api_prt_putgraphics( UCHAR dhn, UCHAR *sbuf, UCHAR *image )
UCHAR	api_prt_putgraphics( UCHAR dhn, API_GRAPH dim, UCHAR *image )
{
	if( PRN_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );
	// printf("dim.xmove=%ld\n",dim.Xleft);
	// printf("dim.ymove=%ld\n",dim.Ytop);
	// printf("dim.width=%ld\n",dim.Width);
	// printf("dim.height=%ld\n",dim.Height);
	// printf("dim.mode=%ld\n",dim.Method);
	
	// memset(&prn_dumpbuffer[prn_count++],0x97,1);	
	// for(i=0;i<sizeof(API_GRAPH);i++)
		// prn_dumpbuffer[prn_count++]=pdim[i];
		
	// memset(&prn_dumpbuffer[prn_count++],0x98,1);	
	// for(i=0;i<dim.Height*dim.Width/8;i++)
		// prn_dumpbuffer[prn_count++]=image[i];
	if( OS_PRN_PutGraphics( dim, image ) == TRUE )
	  return( apiOK );
	  
	else
	  return( apiNotReady );	// PATCH: 2009-11-19
}

// ---------------------------------------------------------------------------
// FUNCTION: To tune the space between two string lines.
// INPUT   : dhn
//	     The specified device handle number.
//	     space
//           The unit is in dots.
// OUTPUT  : none.
// RETURN  : apiOK
//	     apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_prt_setlinespace( UCHAR dhn, UCHAR space )
{
	if( PRN_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );
	  
	if( OS_PRN_SetLineSpace( space ) == TRUE )
	  return( apiOK );
	else
	  return( apiFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: To get the space setting between two string lines.
// INPUT   : dhn
//	     The specified device handle number.
//	     space
//           The unit is in dots.
// OUTPUT  : none.
// RETURN  : apiOK
//	     apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_prt_getlinespace( UCHAR dhn, UCHAR *space )
{
	if( PRN_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );
	  
	OS_PRN_GetLineSpace( space );
	
	return( apiOK );
}

// ---------------------------------------------------------------------------
// FUNCTION: To tune the space between two characters.
// INPUT   : dhn
//	     The specified device handle number.
//	     space
//           The unit is in dots. (default = 0)
// OUTPUT  : none.
// RETURN  : apiOK
//	     apiFailed
// ---------------------------------------------------------------------------
//UCHAR	api_prt_setcharspace( UCHAR dhn, UCHAR space )
//{
//	if( PRN_CheckDHN( dhn ) == FALSE )
//	  return( apiFailed );
//	  
//	OS_PRN_SetCharSpace( os_pPrn, space );
//	
//	return( apiOK );
//}

// ---------------------------------------------------------------------------
// FUNCTION: To use cutter to cut the paper off.
// INPUT   : dhn
//	     The specified device handle number.
// OUTPUT  : none.
// RETURN  : apiOK
//	     apiFailed
// ---------------------------------------------------------------------------
//UCHAR	api_prt_cut( UCHAR dhn )
//{
//	return( apiOK );
//}

// ---------------------------------------------------------------------------
// FUNCTION: To initialize the external bitmap fonts for printer.
//	     The specified font should be downloaded to system memory prior to 
//	     initialization.
// INPUT   : ft
// 	     The "API_PRT_FONT" struture.
//	     Normal Fond ID: FONT10, FONT11, FONT12 ...
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
API_PRT_FONT ftt;
//UCHAR	api_prt_initfont( UCHAR *sbuf  )
UCHAR	api_prt_initfont( API_PRT_FONT ft  )
{
//API_PRT_FONT ft;

	ftt = ft;
	//memmove( &ftt, sbuf, sizeof(API_PRT_FONT) );
	
	if( OS_PRN_InitFont( ftt ) )
	  return( apiOK );
	else
	  return( apiFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: To return the bitmap address of the built-in Chinese FONT.
// INPUT   : fontid - font id.
// OUTPUT  : ft     - pointer to font structure.
// RETURN  : apiOK
//	     apiFailed
// ---------------------------------------------------------------------------
//UCHAR	api_prt_getfont( UCHAR fontid, UCHAR *dbuf  )
UCHAR	api_prt_getfont( UCHAR fontid, API_PRT_FONT *ft  )
{
UCHAR	result = apiOK;
//API_PRT_FONT *ft = 0;
printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);

	//ft = (API_PRT_FONT *)dbuf;
	
	switch( fontid )
	      {
		  case FONT0:	// 24x24
	      
	      	   ft->FontID		= FONT0;
	           ft->Height		= 24;
	           
	           ft->AN_Type		= FT_ASCII;
	           ft->AN_CodeNo	= 1;
	           ft->AN_ByteNo	= 2*24;
	           ft->AN_CharNo	= 96;
	           ft->AN_Width		= 12;
	           // ft->AN_BitMap	= (UCHAR *)FONT2_ASCII_12x24;
	           ft->AN_CodeMap	= (UCHAR *)0;
	           
	           
	           break;	  
	      case FONT2:	// 24x24
	      printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
	      	   ft->FontID		= FONT2;
	           ft->Height		= 24;
	           printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
	           ft->AN_Type		= FT_ASCII;
	           ft->AN_CodeNo	= 1;
	           ft->AN_ByteNo	= 2*24;
	           ft->AN_CharNo	= 96;
	           ft->AN_Width		= 12;
	           // ft->AN_BitMap	= (UCHAR *)FONT2_ASCII_12x24;
			   printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
	           ft->AN_CodeMap	= (UCHAR *)0;
	           printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
		   		ft->GC_Type		= FT_BIG5;
		   		ft->GC_CodeNo	= 2;
		   		ft->GC_ByteNo	= 3*24;
		   		// ft->GC_CharNo	= FULL_BIG5_CHAR_NUM;
		   		ft->GC_Width		= 24;
		   		// ft->GC_BitMap	= (UCHAR *)ADDR_FULL_PRT_FONT2_24X24_BMP;
		   		// ft->GC_CodeMap	= (UCHAR *)ADDR_FULL_BIG5_CODE_TABLE;
	           printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
	           break;
	           
	      case FONT4:	// 16x16
	      
		   ft->FontID		= FONT4;
		   ft->Height		= 16;
		   
		   ft->AN_Type		= FT_ASCII;
		   ft->AN_CodeNo	= 1;
		   ft->AN_ByteNo	= 1*16;
		   ft->AN_CharNo	= 96;
		   ft->AN_Width		= 8;
		   // ft->AN_BitMap	= (UCHAR *)FONT4_ASCII_8x16;	// system default
		   ft->AN_CodeMap	= (UCHAR *)0;
        	   
		   ft->GC_Type = FT_BIG5;
		   ft->GC_CodeNo = 2;
		   ft->GC_ByteNo = 2*16;
		   ft->GC_CharNo = FULL_BIG5_CHAR_NUM;
		   ft->GC_Width = 16;
		//    ft->GC_BitMap = (UCHAR *)ADDR_FULL_PRT_FONT4_16X16_BMP;
		//    ft->GC_CodeMap = (UCHAR *)ADDR_FULL_BIG5_CODE_TABLE;
	        
	           break;
	      
	      default:
	           if(fontid>MAX_FONT)
			   {
				   result = apiFailed;
				   break;
			   }
		   ft->FontID		= fontid;
		   ft->Height		= FONT_TABLE[fontid].height;
		   
		   ft->AN_Type		= FONT_TABLE[fontid].SBC.type;
		   ft->AN_CodeNo	= FONT_TABLE[fontid].SBC.code_size;
		   ft->AN_ByteNo	= FONT_TABLE[fontid].SBC.member_size;
		   ft->AN_CharNo	= FONT_TABLE[fontid].SBC.member;
		   ft->AN_Width		= FONT_TABLE[fontid].SBC.width;
		   ft->AN_CodeMap	= FONT_TABLE[fontid].SBC.code_list;
        	   
		   ft->GC_Type 		= FONT_TABLE[fontid].MBC.type;
		   ft->GC_CodeNo	= FONT_TABLE[fontid].MBC.code_size;
		   ft->GC_ByteNo	= FONT_TABLE[fontid].MBC.member_size;
		   ft->GC_CharNo	= FONT_TABLE[fontid].MBC.member;
		   ft->GC_Width 	= FONT_TABLE[fontid].MBC.width;
		   ft->GC_BitMap	= FONT_TABLE[fontid].MBC.bitmap;
		   ft->GC_CodeMap	= FONT_TABLE[fontid].MBC.code_list;
	           break;
	      }
	      
	return( result );
}
