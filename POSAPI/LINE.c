//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : SignPad	                                            **
//**  PRODUCT  : AS350                                                      **
//**                                                                        **
//**  FILE     : LINE.C		                                            **
//**  MODULE   : 					                    **
//**									    **
//**  FUNCTION : Draw a line.						    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2013/08/28                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2013 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
//#include <stdio.h>
//#include <math.h>
#include <stdlib.h>
#include "DEV_LCD.h"


// ---------------------------------------------------------------------------
// FUNCTION: Draw a pixel.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	putpixel( int x, int y )
{	
	API_LCDTFT_RECT rect;
	rect.Xstart=x;	
	rect.Xend=x+1;	
	rect.Ystart=y;	
	rect.Yend=y+1;	
	rect.Palette[0]=0;	
	rect.Palette[1]=0;	
	rect.Palette[2]=0;	
	OS_LCDTFT_FillRECT(rect);
	//2x2
	// fillrect(x, y, x+1, y+1,0);
}

// ---------------------------------------------------------------------------
// FUNCTION: Sign function.
// INPUT   : n
// OUTPUT  : none.
// RETURN  : -1, 0, 1
// ---------------------------------------------------------------------------
int	sgn( int n )
{
	if( n < 0 )
	  return( -1 );
	else
	  {
	  if( n > 0 )
	    return( 1 );
	  else
	    return( 0 );
	  }
}

// ---------------------------------------------------------------------------
// FUNCTION: Draw a line with normal algorithm. (SLOW)
// INPUT   : (x0, y0), (x1, y1)
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	0
void	slow_line( int x0, int y0, int x1, int y1 )
{
int	x, y;

	
	// Draw degenerate line (pixel)
	
	if( x0 == x1 && y0 == y1 )	// draw a signle pixel
	  putpixel( x0, y0 );
	
	// Draw lines with dx > dy
	
	else if( abs(x1 - x0) >= abs(y1 - y0) )
	       {	// swap end point
	       if( x1 < x0 )
	         {
	         x = x1;
	         y = y1;
	         x1 = x0;
	         y1 = y0;
	         x0 = x;
	         y0 = y;
	         }
	         
	       for( x=x0; x<=x1; x++ )
	          {
	          y = y0 + ((x - x0)*(long)(y1 -y0))/(x1 - x0);
	          putpixel( x, y );
	          }
	       }
	     
	     // Draw lines with dy > dx
	     
	     else
	       {	// swap end points
	       if( y1 < y0 )
	         {
	         x = x1;
	         y = y1;
	         x1 = x0;
	         y1 = y0;
	         x0 = x;
	         y0 = y;
	         }
	       
	       for( y=y0; y<=y1; y++ )
	          {
	          x = x0 + ((y - y0)*(long)(x1 - x0))/(y1 - y0);
	          putpixel( x, y );
	          }
	       }
	
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Draw a line with J.E. Bresenham's algorithm. (FAST)
// INPUT   : (x1, y1), (x2, y2)
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	fast_line( int x1, int y1, int x2, int y2 )
{
int	n;
int	deltax, deltay;
int	sgndeltax, sgndeltay;
int	deltaxabs, deltayabs;
int	x, y;
int	drawx, drawy; 


	//__disable_interrupt();

	deltax = x2 - x1; 
	deltay = y2 - y1; 
	deltaxabs = abs(deltax); 
	deltayabs = abs(deltay); 	
	// if(deltaxabs>20||deltayabs>20)
		// return;
	sgndeltax = sgn(deltax); 
	sgndeltay = sgn(deltay); 
	x = deltayabs >> 1; 
	y = deltaxabs >> 1; 
	drawx = x1; 
	drawy = y1; 

	putpixel(drawx, drawy); 

	if(deltaxabs >= deltayabs)
	  {
	  for( n = 0; n < deltaxabs; n++ )
	     {
	     y += deltayabs; 
	     if( y >= deltaxabs )
	       {
	       y -= deltaxabs;
	       drawy += sgndeltay;
	       }
	     drawx += sgndeltax; 
	     putpixel(drawx, drawy); 
	     } 
	  }
	else
	  { 
	  for( n = 0; n < deltayabs; n++ )
	     {
	     x += deltaxabs; 
	     if( x >= deltayabs )
	       {
	       x -= deltayabs; 
	       drawx += sgndeltax;
	       }
	     drawy += sgndeltay; 
	     putpixel(drawx, drawy); 
	     }
	  }

	//__enable_interrupt();
}
