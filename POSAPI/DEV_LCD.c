#include"fbutils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "POSAPI.h"
#include "DEV_LCD.h"
#include "DEV_TSC.h"
#include "DEV_PCD.h"
UINT8	os_pImageData2[LCD_XRES*LCD_YRES/8];	// LCD_YRES,LCD_XRES defined in DEV_LCD.h. Char font Maximun value
API_LCDTFT_GRAPH IDT[GRAPHIC_ID_MAX_ORDER];//store graphic ID 
UCHAR IDorder=0;//order number of graphic's ID
UINT32 graphic_size=0;//order number of graphic's ID
extern uint32_t xres, yres;//declared in fbutils-linux.c
extern int8_t rotation;//declared in fbutils-linux.c
UCHAR OS_LCDTFT_ClrScreen(API_LCDTFT_PARA para){	
	static int colormap;
	UCHAR rotate_bak=0;
	rotate_bak=rotation;
	UCHAR BGpalette[3]={para.BG_Palette[2],para.BG_Palette[1],para.BG_Palette[0]};
	memmove(&colormap,&BGpalette,sizeof(BGpalette));
	rotation=ROTATE_270;
	setcolor(0, colormap);
	fillrect(0, 0, xres , yres ,0);
	rotation=rotate_bak;
	return TRUE;
}

UCHAR OS_LCDTFT_ClrRow(API_LCDTFT_PARA para){
UINT16 Xstart,Ystart,Xend,Yend;
	Xstart=0;
	Ystart=para.Row;
	Xend=240;
	Yend=Ystart+para.Col;
	static int colormap;
	// printf("\033[1;31;40mYstart=%d Yend=%d\033[0m\n",Ystart,Yend);
	UCHAR BGpalette[3]={para.BG_Palette[2],para.BG_Palette[1],para.BG_Palette[0]};
	memmove(&colormap,&BGpalette,sizeof(BGpalette));
	rotation=ROTATE_0;
	setcolor(0, colormap);
	// printf("Xstart=%d\tYstart=%d\tXend=%d\tYend=%d\n",Xstart,Ystart,Xend,Yend);
	fillrect(Xstart, Ystart, Xend, Yend,0);
	return TRUE;
}
UINT32	OS_LCDTFT_PutGraphics( API_LCDTFT_GRAPH graph )
{
	UCHAR IDorder2=IDorder;
	//printf("LCDTFT_PutGraphics\n");
	//printf("IDorder=%d\n",IDorder);
	if(IDorder2>0){
	for(UCHAR i=0;i<IDorder2;i++){//check if ID number input before
		if(IDT[i].ID==graph.ID){
			IDorder=i;
			IDorder2--;
			// printf("IDorder=%d\n",IDorder);
			//memmove(&IDT[i],0x00,sizeof(API_LCDTFT_GRAPH));
			break;
		}
	}
	 }
	if(IDorder>GRAPHIC_ID_MAX_ORDER||graphic_size>GRAPHIC_MAX_SIZE)
	{//if graphic order number exceed 100 or total graphic size exceed 4MB 
		return apiOutOfService;
	}
	
	graphic_size+=graph.Bitmap.YSize*graph.Bitmap.BytesPerLine;
	if(graphic_size>GRAPHIC_MAX_SIZE){//if total graphic size exceed 4MB
		return apiOutOfService;
	}
	memmove(&IDT[IDorder],&graph,sizeof(API_LCDTFT_GRAPH));
	IDorder=IDorder2;
	IDorder++; 
	return apiOK;
}
UINT32 OS_LCDTFT_ShowICON(API_LCDTFT_ICON icon){
	UCHAR IDorder2=IDorder;
	UCHAR result;
	UCHAR rotate_bak=0;
	// printf("OS_LCDTFT_ShowICON\n");
	// printf("IDorder2=%d\n",IDorder2);
	rotate_bak=rotation;
	for(UCHAR i=0;i<IDorder2;i++){
		if(IDT[i].ID==icon.ID)
			IDorder2=i;
		else if(i==IDorder2-1)
			return apiFailed;
	}
	rotation=ROTATE_270;
	// printf("icon.Xleft=%d\n",icon.Xleft);
	// printf("icon.Ytop=%d\n",icon.Ytop);
#ifndef _SCREEN_SIZE_480x320
	if(IDT[IDorder2].RGB==GUI_DRAW_BMP555)
		result=put_graphic_BMP555(icon.Ytop,240-icon.Xleft-icon.Width,icon.Width,icon.Height,(UCHAR*)IDT[IDorder2].Bitmap.pData);
	else
		result=put_graphic(icon.Ytop,240-icon.Xleft-icon.Height,icon.Width,icon.Height,(UCHAR*)IDT[IDorder2].Bitmap.pData);
#else
	if(IDT[IDorder2].RGB==GUI_DRAW_BMP555)
		result=put_graphic_BMP555(icon.Ytop,LCD_XRES-icon.Xleft-icon.Width,icon.Width,icon.Height,(UCHAR*)IDT[IDorder2].Bitmap.pData);
	else
		result=put_graphic(icon.Ytop,LCD_XRES-icon.Xleft-icon.Height,icon.Width,icon.Height,(UCHAR*)IDT[IDorder2].Bitmap.pData);
#endif
	rotation=rotate_bak;
	return result;                                                                  
}                                                                                   
UCHAR OS_LCDTFT_PutStr( UCHAR *data,API_LCDTFT_PARA para,UCHAR fontLen ){
	put_string(para.Col,para.Row , data,  para, fontLen);
	return TRUE;
}
UCHAR OS_LCDTFT_PutBG5Str(UCHAR *data,API_LCDTFT_PARA para,UCHAR fontLen){
	put_Bg5string(para.Col, para.Row, data,  para,fontLen);
	return TRUE;
}
UCHAR OS_LCDTFT_FillRECT(API_LCDTFT_RECT rect){
	static int colormap;
	UCHAR rotate_bak=0;
	rotate_bak=rotation;
	UCHAR BGpalette[3]={rect.Palette[2],rect.Palette[1],rect.Palette[0]};
	rotation=ROTATE_270;
	memmove(&colormap,&BGpalette,sizeof(BGpalette));
	setcolor(0, colormap);
#ifndef _SCREEN_SIZE_480x320
	fillrect( rect.Ystart, 240-rect.Xstart, rect.Yend,240-rect.Xend,0);
#else
	fillrect( rect.Ystart, LCD_XRES-rect.Xstart, rect.Yend,LCD_XRES-rect.Xend,0);
#endif
	rotation=rotate_bak;
}
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
UINT8			data;

UINT32			xsize;
UINT32			ysize;
UINT32			xsize0;
UINT32			ysize0;
UINT32			BytesPerLine;
UINT32			BmpDim;

UINT8			bpp24[3];
UINT8			fg_bpp24[3];
UINT8			bg_bpp24[3];
	pRawBmp = TransformBitmapFile( winbmp, &xsize, &ysize ); 
	if( pRawBmp )
	  {
	  pRawBmpBak = pRawBmp;
	  
	  xsize0 = xsize;
	  ysize0 = ysize;
	  // printf("xsize0=%d ysize0=%d\n",xsize0,ysize0);
	  
	  // rotate CCW degrees
	  switch( CCWdegrees )
	        {
	        case CCW_0:
	             
	             *width = xsize0;
	             *height = ysize0;
	             
	             break;
	             
	        case CCW_270:
	        
	             OS_LCDTFT_TransformCharBitmap( ysize, xsize, pRawBmp );
		     
				// *width = xsize0;
				// *height = ysize0;
				*width = ysize0;
	            *height = xsize0;
				xsize = ysize0;
	            ysize = xsize0;
	             
	             break;
	             
	        case CCW_180:
	             
	             OS_LCDTFT_TransformCharBitmap( ysize, xsize, pRawBmp );
	             OS_LCDTFT_TransformCharBitmap( xsize, ysize, pRawBmp );
	             
	             *width = xsize0;
	             *height = ysize0;
	             
	             xsize = ysize0;
	             ysize = xsize0;
	             
	             break;
	             
	        case CCW_90:

	             OS_LCDTFT_TransformCharBitmap( ysize, xsize, pRawBmp );
	             OS_LCDTFT_TransformCharBitmap( xsize, ysize, pRawBmp );
	             OS_LCDTFT_TransformCharBitmap( ysize, xsize, pRawBmp );
	             
	             // *width = xsize0;
	             // *height = ysize0;
	             *width = ysize0;
	             *height = xsize0;
				 
	             xsize = ysize0;
	             ysize = xsize0;
	             
	             break;
	             	             
	        default:
	        
	             free( pRawBmpBak );
	             return( FALSE );             
	        }

	  //fill pixel with 24bit color	
	  fg_bpp24[0]=*fg_palette;
	  fg_bpp24[1]=*(fg_palette+1);
	  fg_bpp24[2]=*(fg_palette+2);
	  bg_bpp24[0]=*bg_palette;
	  bg_bpp24[1]=*(bg_palette+1);
	  bg_bpp24[2]=*(bg_palette+2);
	  BytesPerLine = xsize*3;
	  BmpDim = xsize*ysize;
	  pRgbBmp  = malloc( BmpDim*3 );
	  if( pRgbBmp )
	    {
	    pRgbBmpBak = pRgbBmp;
	    
	    for( i=0; i<BmpDim/8; i++ )
	       {
			//printf("i=%ld pRawBmp=%02x\n",i,*pRawBmp);
	       data = *pRawBmp++;
	       for( j=0; j<8; j++ )
	          {
	          if( !((data << j) & 0x80) )
	            {
	            // foreground
	            
//	            *pRgbBmp++ = 0x00;	// black
//	            *pRgbBmp++ = 0x00;
		    
		    *pRgbBmp++ = fg_bpp24[0];
		    *pRgbBmp++ = fg_bpp24[1];
		    *pRgbBmp++ = fg_bpp24[2];
	            }
	          else
	            {
	            // background
	            
//	            *pRgbBmp++ = 0xFF;	// white
//	            *pRgbBmp++ = 0xFF;

		    *pRgbBmp++ = bg_bpp24[0];
		    *pRgbBmp++ = bg_bpp24[1];
		    *pRgbBmp++ = bg_bpp24[2];
	            }
	          }
	       }
	    }
	  graph.ID	= id;
	  graph.RGB	= 1;
	  graph.Bitmap.XSize		= xsize;
	  graph.Bitmap.YSize		= ysize;
	  graph.Bitmap.BytesPerLine	= xsize*3;//24 bit
	  graph.Bitmap.BitsPerPixel	= 24;
	  graph.Bitmap.pData		= pRgbBmpBak;
	  graph.Bitmap.pPal		= NULL;
	  graph.Bitmap.pMethods	= GUI_DRAW_BMP555;
	  OS_LCDTFT_PutGraphics( graph );
	  
	  //free( pRgbBmpBak );
	  
	  //free( pRawBmpBak );
	  
	  return( TRUE );
	  }
	else
	  return( FALSE );
}
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
		    
	          
	          pImageData  = malloc( SizeOfImage*sizeof(char));
	          pImageData2 = malloc( SizeOfImage*sizeof(char));
			  
	        // read all image data to the working buffer 
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
	              if(   ((bmpRgbQuad1->rgbBlue == 0xFF) && (bmpRgbQuad1->rgbGreen == 0xFF) && (bmpRgbQuad1->rgbRed == 0xFF))||
						((bmpRgbQuad2->rgbBlue == 0xFF) && (bmpRgbQuad2->rgbGreen == 0xFF) && (bmpRgbQuad2->rgbRed == 0xFF))
				    )
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
					//20200930:comment out because bmp data will be broken with it	
					/*
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
					   */
	              }
	              
	            free( pImageData2 );
	            
	            status = pImageData;
	            
	          }
	        }
	      }
	    }
	  }	  
	return( status );
}
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
UINT32	OS_LCDTFT_ShowPCD( API_PCD_ICON icon )
{
UINT8	*Para;
UINT32	result;
UINT16 ParaLen=16;
	Para=&icon;
	OS_EnableTimer1();
	result=PCD_ShowPCD( ParaLen, Para );
	
	return( result );
}