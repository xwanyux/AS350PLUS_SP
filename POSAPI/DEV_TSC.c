//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : AS350+                                     **
//**  PRODUCT  : AS350	                                                    **
//**                                                                        **
//**  FILE     : DEV_TSC.C 	                                            **
//**  MODULE   : Touch Screen Controller.				    **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2020/10/16                                                 **
//**  EDITOR   : West Chu                                                **
//**                                                                        **
//**  Copyright(C) 2013 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "bsp_types.h"
#include "bsp_gpio.h"
#include "DEV_LCD.h"
#include "DEV_TSC.h"
#include "ExtIODev.h"
#include "fbutils.h"
#include "LCDTFTAPI.h"
#include "POSAPI.h"
//from LINE.c
extern void	putpixel( int x, int y );
extern void	fast_line( int x1, int y1, int x2, int y2 );
//button image
extern const GUI_BITMAP bmbutton_BLANK_ROTATE;
extern const GUI_BITMAP bmbutton_OK_180;
extern const GUI_BITMAP bmbutton_OK;
extern const GUI_BITMAP bmbutton_CLEAR_180;
extern const GUI_BITMAP bmbutton_CLEAR;
extern const GUI_BITMAP bmbutton_ROTATE_180;
extern const GUI_BITMAP bmbutton_ROTATE;
extern const GUI_BITMAP bmtext_SIGNHERE_180;
extern const GUI_BITMAP bmtext_SIGNHERE;
extern const GUI_BITMAP bmbutton_CANCEL_180;
extern const GUI_BITMAP bmbutton_CANCEL;
UCHAR SIGNFRAME[] = {
#include "Invert_sign_frame.h"
};
//variable
UINT16 lastxpos,lastypos;
UINT16 xpos_bak,ypos_bak;
UCHAR PRESSorSLIDE=0;
UCHAR PRESSorLEAVE=0;//finger press or leave the LCD.press down=1, finger leave=0;
UCHAR FIRSTsignFLAG=0;//if LCD pressed before signpad function called,signpad function can't get press signal(event.code 330).
UCHAR ValidCode=0;//Capacitive touch driver will keep send message, this variable used to tag if these messages are useful.
UINT8	TSC_SignRawBuffer[320*120/8];
UCHAR resetIRQ=0;
int TSC_fd=0;
UINT16 TSC_timerdhn=0;
UINT16 SignPadTime;
// ---------------------------------------------------------------------------
UINT16	os_SIGNPAD_GID = 0;		// beginning graphic id

#define	_SIGNPAD_PUTG_			1	// marked if support new putgraphic method

#define	ID_BUT_OK_180			0
#define	ID_BUT_OK			1

#define	ID_BUT_CLEAR_180		2
#define	ID_BUT_CLEAR			3

#define	ID_BUT_ROTATE_180		4
#define	ID_BUT_ROTATE			5
#define	ID_BUT_ROTATE_BLANK		6

#define	ID_BUT_CANCEL_180		7
#define	ID_BUT_CANCEL			8

#define	ID_TXT_SIGNHERE_180		9
#define	ID_TXT_SIGNHERE			10

struct input_event TSCevent[15];
struct timeval timeout;
fd_set set;
// ---------------------------------------------------------------------------
// FUNCTION: To activate SignPad function.
// INPUT   : dhn
//	     The specified device handle number.
//	     para	- parameters.
// OUTPUT  : status	- status byte.
//			  0: not available (abort or timeout)
//			  1: available	   (sign confirmed)
//	     amt	- amount string to be shown on the SignPad
//			  format: LEN(1) + STR(n), max 20 digits.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
UINT32	OS_TSC_SignPad( API_TSC_PARA para, UINT8 *status, UINT8 orient )
{
SPI_TSC		spitsc;
API_TSC_PARA	tscpara;
UINT32	result = TRUE;
UINT16	rx_length;
UINT32	length;
UINT32	retry;
UINT8	temp[64];
UINT8	timerdbuf = 0;
	if( para.RFU[0] == SIGN_STATUS_PROCESSING )
	  goto WAIT_STATUS;

	

	memmove( &temp[2], (UINT8 *)&para, sizeof(API_TSC_PARA) );
	temp[2+14] = orient;
	// result = OS_SPI_PutPacket( sizeof(SPI_TSC), temp );
	
	
	// wait for end of sign pad function...
WAIT_STATUS:

	// memset( (UCHAR *)&tscpara, 0x00, sizeof( API_TSC_PARA ) );
	memmove( (UCHAR *)&tscpara, (UINT8 *)&para, sizeof( API_TSC_PARA ) );
	// tscpara.ID	= FID_TSC_STATUS_SIGNPAD;
	// tscpara.X	= 0;
	// tscpara.Y	= 0;
	// tscpara.Width	= 0;
	// tscpara.Height	= 0;
	// tscpara.RxLen = 1;
	// tscpara.fd = para.fd;
	while(1)
	     {
	     //BSP_Delay_n_ms(100);
	     api_tim_gettick( TSC_timerdhn, &timerdbuf );
		 SignPadTime+=timerdbuf;
		 timerdbuf = 0;
	     *status = 0;
	     OS_TSC_Status( tscpara, status );
		 if(SignPadTime>para.Timeout)
			 *status=SIGNPAD_STATUS_TIMEOUT;
		 
		 if((*status == SIGNPAD_STATUS_CLEAR)||(*status == SIGNPAD_STATUS_EXIT))
			 FIRSTsignFLAG=0;
	     if( (*status == SIGNPAD_STATUS_NOT_READY) || (*status == SIGNPAD_STATUS_READY) || 
	         (*status == SIGNPAD_STATUS_ROTATE) || (*status == SIGNPAD_STATUS_READY_REVERSE) ||
	         (*status == SIGNPAD_STATUS_CLEAR) || (*status == SIGNPAD_STATUS_TIMEOUT) || (*status == SIGNPAD_STATUS_EXIT) )
	       {
	       	
	       retry = 3;
	       while(retry--)
	            {
RESET_PINPAD:
		    
	            // reset PINPAD status
	            tscpara.ID	= FID_TSC_STATUS_SIGNPAD;
	            tscpara.X	= 0xFFFF;
	            tscpara.Y	= 0xFFFF;
	            temp[0] = 0;
	            OS_TSC_Status( tscpara, temp );
	            if( (temp[0] == SIGNPAD_STATUS_NOT_READY) || (temp[0] == SIGNPAD_STATUS_READY) || 
	                (temp[0] == SIGNPAD_STATUS_ROTATE) || (temp[0] == SIGNPAD_STATUS_READY_REVERSE) ||
	                (temp[0] == SIGNPAD_STATUS_CLEAR) || (temp[0] == SIGNPAD_STATUS_TIMEOUT) || (temp[0] == SIGNPAD_STATUS_EXIT) )
					
		      continue;
	            else
	              goto EXIT;
	            }
	       }
	     
	     *status = SIGN_STATUS_PROCESSING;
	     break;
	     }

EXIT:
	return( result );
}
// ---------------------------------------------------------------------------
// FUNCTION: To get XY axis position of TSC.
// INPUT   : xpos	- parameters.
// OUTPUT  : status	- status byte.
//			  0: not available (no event)
//			  1: available     (event occurs)
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
#ifdef TSC_RESIST
UCHAR res_count;
UINT  res_xposavg=0,res_yposavg=0;
UINT  res_xposmid[10],res_yposmid[10];
UINT  res_postemp;
#endif
UINT32 TSC_Status(UINT16* xpos,UINT16* ypos,API_TSC_PARA para)
{
// UINT16 i=0,rd,xpos_bak,ypos_bak;
UINT16 i=0,rd;
UCHAR tx[4];
UCHAR rx[4];
UCHAR data;
#ifdef TSC_RESIST
long res_xpos=0,res_ypos=0;
long xcale=5547;
long xymix=113;
long xoffset=-1299240;
long yxmix=86;
long yscale=4403;
long yoffset=-1260224;
long scaler=65536;
#endif
int rv,fd=para.fd;
	FD_ZERO(&set); /* clear the set */
	FD_SET(fd, &set); /* add our file descriptor to the set */
	timeout.tv_sec = 0;
	timeout.tv_usec = 500;//0.5ms timeout
	rv = select(fd+1, &set, NULL, NULL, &timeout);
	
	if (FD_ISSET(fd, &set))
        {
		  rd=read(fd,&TSCevent, sizeof(TSCevent));//read Keypad Status
		}
	if(rv == -1 || rv == 0 ){//select() timeout?????¯error
	#ifdef TSC_RESIST
	//if touch screen use resistive LCD panel,do nothing
	#else
		//update interrupt pins status
		tx[0]=0x45;
		tx[1]=0x08;//INTCAPA
		tx[2]=0x00;
		SPI_Transfer(tx,rx,0,EXTIO,4);
	#endif	
		
		return(FALSE);		
	}
	else{
		tx[0]=0x45;
		tx[1]=0x09;//GPIOA
		tx[2]=0x00;
		SPI_Transfer(tx,rx,0,EXTIO,4);
		//å¦????select()æ²?timeout????????³NULL
		if (rd < (int) sizeof(struct input_event)) {
				printf("expected %d bytes, got %d\n", (int) sizeof(struct input_event), rd);
				perror("\nevtest: error reading");
				return(FALSE);
			}
		
		if((rx[2]&0x80)>0)
		{//power button not pressed
			data=0;
			bsp_shm_ButtPressed(1,&data);
		}			
		else
		{//power button pressed
			data=1;
			bsp_shm_ButtPressed(1,&data);
		}
		#ifdef TSC_RESIST
		//if touch screen use resistive LCD panel,do nothing
		if(res_count==0)
			PRESSorSLIDE= 0;
		#else
		PRESSorSLIDE= 0;
		#endif
		for (i = 0; i < rd / sizeof(struct input_event); i++) 
		{
		#ifdef TSC_RESIST
			if(TSCevent[i].code==330){//check if first press
				if(TSCevent[i].value)
					PRESSorSLIDE= 1;
				else
				{
					PRESSorSLIDE= 0;
					res_yposavg=0;
					res_xposavg=0;
					res_count=0;
					return(FALSE);
				}
				}			
			if(TSCevent[i].code==0){//get x axis position
				res_xpos= TSCevent[i].value;
				}
			if(TSCevent[i].code==1){//get y axis position
				res_ypos= TSCevent[i].value;	
				}
			if((res_xpos>0)&&(res_ypos>0))
			{
				ypos_bak=(res_xpos*xcale+res_ypos*xymix+xoffset)/scaler;
				xpos_bak=(res_ypos*yscale+res_xpos*yxmix+yoffset)/scaler;
				xpos_bak=240-xpos_bak;
				res_xposavg+=xpos_bak;
				res_yposavg+=ypos_bak;
				res_count++;
				xpos_bak=0;
				ypos_bak=0;
			}
			if(res_count>2)
			{
				xpos_bak=res_xposavg/res_count;
				ypos_bak=res_yposavg/res_count;
				res_yposavg=0;
				res_xposavg=0;
				res_count=0;
			}	
		#else
			if(TSCevent[i].code==330){//check if first press
				PRESSorSLIDE= 1;
				ValidCode=1;
				}
			if(TSCevent[i].code==57)
			{
				// printf("TSCevent[i].code==57\n");
				if(TSCevent[i].value>0)
				{
					
					PRESSorLEAVE= 1;//finger press
				}					
				else
				{
					
					PRESSorLEAVE= 0;
				}	
				ValidCode=1;
			}
			if(TSCevent[i].code==53){//get x axis position
				xpos_bak= TSCevent[i].value;
				ValidCode=1;
				}
			else if(TSCevent[i].code==54){//get y axis position
				ypos_bak= TSCevent[i].value;	
				ValidCode=1;
				}
			else
			{
				PRESSorSLIDE= 1;
			}
		#endif	
		}
		// if(ValidCode==0)
		// 	return(FALSE);
		ValidCode=0;//initial variable
		resetIRQ=0;
		//button can't be pressed with slide gesture
		// printf("xpos_bak= %d ypos_bak= %d\n", xpos_bak, ypos_bak);
		if(para.ID==FID_TSC_STATUS_BUTTON)
		{
			// if(PRESSorSLIDE>0)
			// {
				if((xpos_bak>0) && (ypos_bak>0)&&(xpos_bak<241) && (ypos_bak<321))
				{
					*xpos=ypos_bak;
					*ypos=xpos_bak;
					return(TRUE);			
				}
			// }
			// else
				return FALSE;
		}
		else
		{
			if((xpos_bak>0) && (ypos_bak>0)&&(xpos_bak<241) && (ypos_bak<321))
				{			
					*xpos=xpos_bak;
					*ypos=ypos_bak;
					return(TRUE);			
				}
			return FALSE;
		}
		
			
	}
  	
}
// ---------------------------------------------------------------------------
// FUNCTION: To get status of TSC.
// INPUT   : para	- parameters.
// OUTPUT  : status	- status byte.
//			  0: not available (no event)
//			  1: available     (event occurs)
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
UINT32	OS_TSC_Status( API_TSC_PARA para, UCHAR *status )
{
UINT16 Xstart;
UINT16 Xend;
UINT16 Ystart;
UINT16 Yend;
UINT16 xpos;
UINT16 ypos;
UCHAR	orient;
API_LCDTFT_RECT rect;
	//if read x,y position
	if(TSC_Status(&xpos,&ypos,para)==FALSE)
		return(FALSE);

	SignPadTime=0;
	orient=para.RFU[1];
	Xstart=para.Y-1;
	Ystart=para.X;
	Xend=Xstart+para.Height;
	Yend=Ystart+para.Width;
		
	
	if(para.ID==FID_TSC_STATUS_BUTTON)
	{
	// 	printf("Xstart=%d\t",Xstart);
	// printf("Ystart=%d\t",Ystart);
	// printf("Xend=%d\t",Xend);
	// printf("Yend=%d\n",Yend);
	// printf("xpos=%d\t",xpos);
	// printf("ypos=%d\n",ypos);
		FIRSTsignFLAG=0;
		//if x,y position are in range of given values
		if(
			xpos>Xstart	&&
			xpos<Xend	&&
			ypos>Ystart	&&
			ypos<Yend	
		)
		{
			*status=1;
			xpos_bak=0;
			ypos_bak=0;
		}			
		else
			*status=0;
	
		return(TRUE);
	}
	if(para.ID==FID_TSC_SIGNPAD||para.ID==FID_TSC_STATUS_SIGNPAD)
	{//press in signpad area
		
		if(orient==1)
		{
			Xstart=240-para.Height-para.Y-1;
			Ystart=para.X;
			Xend=Xstart+para.Height;
			Yend=Ystart+para.Width;
		}
		if(
			xpos>Xstart	&&
			xpos<Xend	&&
			ypos>Ystart	&&
			ypos<Yend	
		)
		{	
			
			// if(PRESSorSLIDE==1||FIRSTsignFLAG==0)
			if(FIRSTsignFLAG==0)
			{
				lastxpos=xpos;
				lastypos=ypos;
				// if(FIRSTsignFLAG==0)
				if(PRESSorLEAVE==1)
				{
					//hide TXT_SIGNHERE
					if( orient == 0 )
						rect.Xstart	= 104 - 16;
					else
						rect.Xstart	= 104 + 16;
					
					rect.Xend	= rect.Xstart+32;
					rect.Ystart	= 112;
					rect.Yend	= 112+96;
					rect.Palette[0]	= 0xff;
					rect.Palette[1]	= 0xff;
					rect.Palette[2]	= 0xff;
					//api_lcdtft_fillRECT( 0, (UINT8 *)&rect );
					// Wayne 20/10/28
					api_lcdtft_fillRECT( 0, rect );
					
					//hide rotate button
					OS_TSC_ShowRotateButton(FALSE,orient);
				}
				
			}
			// fillrect(ypos-1, xpos-1, ypos+1, xpos+1,0);
			//fast_line( lastxpos, lastypos, xpos, ypos,Xstart,Xend,Ystart,Yend );
			
			if(PRESSorLEAVE==1)//if finger first press LCD panel, initial previous x y position
			{		
				// printf("PRESSorLEAVE==0\n");		
				lastxpos=xpos;
				lastypos=ypos;
			}

			fast_line( lastxpos, lastypos, xpos, ypos );
			lastxpos=xpos;
			lastypos=ypos;
			FIRSTsignFLAG=1;
			PRESSorLEAVE=0;//initial variable to prevent from initaling x y position every times
			*status=SIGN_STATUS_PROCESSING;
		}
		else//press button
		{
	// 		printf("Xstart=%d\t",Xstart);
	// printf("Ystart=%d\t",Ystart);
	// printf("Xend=%d\t",Xend);
	// printf("Yend=%d\n",Yend);
	// printf("xpos=%d\t",xpos);
	// printf("ypos=%d\n",ypos);
	// printf("PRESSorSLIDE=%d\n",PRESSorSLIDE);
			PRESSorSLIDE=1;
			if(PRESSorSLIDE==1)//if press event,not slide behavior
			{
				//button OK
				if(orient == 0)
				{
					Xstart=0;
					Ystart=8;
					Xend=Xstart+32;
					Yend=64;
				}
				else
				{
					Xstart=240-32;
					Ystart=320-(8+64);
					Xend=Xstart+32;
					Yend=320;
				}
				if(
					xpos>Xstart	&&
					xpos<Xend	&&
					ypos>Ystart	&&
					ypos<Yend
				)
					*status=SIGNPAD_STATUS_READY;
				
				//button CLEAR
				if(orient == 0)
				{
					Xstart=0;
					Ystart=8+64+32;
					Xend=Xstart+32;
					Yend=Ystart+64;
				}
				else
				{
					Xstart=240-32;
					Ystart=320-(8+64+32+64);
					Xend=240;
					Yend=320-(8+64+32);
				}
				if(
					xpos>Xstart	&&
					xpos<Xend	&&
					ypos>Ystart	&&
					ypos<Yend
				)
				{
					*status=SIGNPAD_STATUS_CLEAR;
				}
				//button CANCEL
				if(orient == 0)
				{
					Xstart=0;
					Ystart=8+64+32+64+40+32+8;
					Xend=Xstart+32;
					Yend=Ystart+64;
				}
				else
				{
					Xstart=240-32;
					Ystart=320-(8+64+32+64+40+32+8+64);
					Xend=240;
					Yend=320-(8+64+32+64+40+32+8);
				}
				if(
					xpos>Xstart	&&
					xpos<Xend	&&
					ypos>Ystart	&&
					ypos<Yend
				)
					*status=SIGNPAD_STATUS_EXIT;
				//button ROTATE
				if(orient == 0)
				{
					Xstart=0;
					Ystart=8+64+8+64+8+64+32+32-72;
					Xend=Xstart+32;
					Yend=Ystart+32;
				}
				else
				{
					Xstart=240-32;
					Ystart=320-(8+64+8+64+8+64+32+32+32-72);
					Xend=240;
					Yend=320-(8+64+8+64+8+64+32+32-72);
				}
				if(
					xpos>Xstart	&&
					xpos<Xend	&&
					ypos>Ystart	&&
					ypos<Yend
				)
					*status=SIGNPAD_STATUS_ROTATE;
					
				
			}
		}
	}
	
}

// ---------------------------------------------------------------------------
// FUNCTION: To show or clear ROTATE button.
// INPUT   : flag	- TRUE, show it
//			- FALSE, clear it
//	     orient	- TRUE/FALSE
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
UINT32	OS_TSC_ShowRotateButton( UINT32 flag, UINT32 orient )
{
UINT8	dhn_lcd = 0;
API_LCDTFT_ICON		gicon[1];
API_LCDTFT_GRAPH	graph[1];
	// button_ROTATE
#ifdef	_SIGNPAD_PUTG_
	graph->ID	= 0;
	graph->RGB	= 0;
	if( orient == 0 )
	  {
	  if( flag )
	    memmove( &graph->Bitmap, &bmbutton_ROTATE_180, sizeof(GUI_BITMAP) );
	  else
	    memmove( &graph->Bitmap, &bmbutton_BLANK_ROTATE, sizeof(GUI_BITMAP) );
	  }
	else
	  {
	  if( flag )
	    memmove( &graph->Bitmap, &bmbutton_ROTATE, sizeof(GUI_BITMAP) );
	  else
	    memmove( &graph->Bitmap, &bmbutton_BLANK_ROTATE, sizeof(GUI_BITMAP) );
	  }
	//api_lcdtft_putgraphics( dhn_lcd, graph );
	// Wayne 20/10/28
	api_lcdtft_putgraphics( dhn_lcd, graph[0] );
	

#endif

	memset( (UCHAR *)gicon, 0x00, sizeof(gicon) );

#ifndef	_SIGNPAD_PUTG_
	if( orient == 0 )
	  {
	  if( flag )
	    gicon->ID = os_SIGNPAD_GID + ID_BUT_ROTATE_180;
	  else
	    gicon->ID = os_SIGNPAD_GID + ID_BUT_ROTATE_BLANK;
	  }
	else
	  {
	  if( flag )
	    gicon->ID = os_SIGNPAD_GID + ID_BUT_ROTATE;
	  else
	    gicon->ID = os_SIGNPAD_GID + ID_BUT_ROTATE_BLANK;
	  }
#else  
	gicon->ID	= 0;
#endif

	if( orient == 0 )
	  {
	  gicon->Xleft	= 0;
	  gicon->Ytop	= 8+64+8+64+8+64+32+32-72;		// 2015-05-22
	  }
	else
	  {
	  gicon->Xleft	= 240-32;
	  gicon->Ytop	= 320-(8+64+8+64+8+64+32+32+32-72);	// 2015-05-22
      }
	gicon->Method	= 0;
	gicon->Width	= 32;
	gicon->Height	= 32;			// 32x32
	//api_lcdtft_showICON( dhn_lcd, gicon );
	// Wayne 20/10/28
	api_lcdtft_showICON( dhn_lcd, gicon[0] );

	return( TRUE );
}
// ---------------------------------------------------------------------------
// FUNCTION: To upload the pictures of sign pad in advance for better performance.
//	     This function is called at POST.
// INPUT   : dhn
//	     The specified device handle number.
//	     para	- parameters.
// OUTPUT  : status	- status byte.
//			  0: not available (abort or timeout)
//			  1: available	   (sign confirmed)
//	     amt	- amount string to be shown on the SignPad
//			  format: LEN(1) + STR(n), max 20 digits.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
UINT32	OS_TSC_PreloadSignPadPictures( UINT16 StartId )
{
API_LCDTFT_ICON		gicon[1];
API_LCDTFT_GRAPH	graph[1];


	os_SIGNPAD_GID = StartId;
	
	memset( (UINT8 *)graph, 0x00, sizeof(API_LCDTFT_GRAPH) );
	
	// button_OK
	graph->ID	= StartId+ID_BUT_OK_180;
	memmove( &graph->Bitmap, &bmbutton_OK_180, sizeof(GUI_BITMAP) );
	if( api_lcdtft_putgraphics( 0, graph[0] ) != apiOK )
	  return( FALSE );

	graph->ID	= StartId+ID_BUT_OK;
	memmove( &graph->Bitmap, &bmbutton_OK, sizeof(GUI_BITMAP) );
	if( api_lcdtft_putgraphics( 0, graph[0] ) != apiOK )
	  return( FALSE );
	
	// button_CLEAR
	graph->ID	= StartId+ID_BUT_CLEAR_180;
	memmove( &graph->Bitmap, &bmbutton_CLEAR_180, sizeof(GUI_BITMAP) );
	if( api_lcdtft_putgraphics( 0, graph[0] ) != apiOK )
	  return( FALSE );

	graph->ID	= StartId+ID_BUT_CLEAR;
	memmove( &graph->Bitmap, &bmbutton_CLEAR, sizeof(GUI_BITMAP) );
	if( api_lcdtft_putgraphics( 0, graph[0] ) != apiOK )
	  return( FALSE );

	// button_ROTATE & BLANK
	graph->ID	= StartId+ID_BUT_ROTATE_180;
	memmove( &graph->Bitmap, &bmbutton_ROTATE_180, sizeof(GUI_BITMAP) );
	if( api_lcdtft_putgraphics( 0, graph[0] ) != apiOK )
	  return( FALSE );
	
	graph->ID	= StartId+ID_BUT_ROTATE;
	memmove( &graph->Bitmap, &bmbutton_ROTATE, sizeof(GUI_BITMAP) );
	if( api_lcdtft_putgraphics( 0, graph[0] ) != apiOK )
	  return( FALSE );
	
	graph->ID	= StartId+ID_BUT_ROTATE_BLANK;
	memmove( &graph->Bitmap, &bmbutton_BLANK_ROTATE, sizeof(GUI_BITMAP) );
	if( api_lcdtft_putgraphics( 0, graph[0] ) != apiOK )
	  return( FALSE );
	
	// text_SIGNHERE
	graph->ID	= StartId+ID_TXT_SIGNHERE_180;
	memmove( &graph->Bitmap, &bmtext_SIGNHERE_180, sizeof(GUI_BITMAP) );
	if( api_lcdtft_putgraphics( 0, graph[0] ) != apiOK )
	  return( FALSE );
	
	graph->ID	= StartId+ID_TXT_SIGNHERE;
	memmove( &graph->Bitmap, &bmtext_SIGNHERE, sizeof(GUI_BITMAP) );
	if( api_lcdtft_putgraphics( 0, graph[0] ) != apiOK )
	  return( FALSE );

	// button_CANCEL
	graph->ID	= StartId+ID_BUT_CANCEL_180;
	memmove( &graph->Bitmap, &bmbutton_CANCEL_180, sizeof(GUI_BITMAP) );
	if( api_lcdtft_putgraphics( 0, graph[0] ) != apiOK )
	  return( FALSE );

	graph->ID	= StartId+ID_BUT_CANCEL;
	memmove( &graph->Bitmap, &bmbutton_CANCEL, sizeof(GUI_BITMAP) );
	if( api_lcdtft_putgraphics( 0, graph[0] ) != apiOK )
	  return( FALSE );
	  
	return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: To show sign pad graphics.
// SOLUTION 1:
//		|<--	    320 dots	     -->|
//		+-------------------------------+ --
//		|				| 40 dots  (button or msg area)
//		+-------------------------------+ --
//		|				| 160 dots (sign area 316*160)
//		|				|
//		+-------------------------------+ --
//		|				| 40 dots  (button or msg area)
//		+-------------------------------+ --
//
// SOLUTION 2:
//		|<--	    320 dots	     -->|
//		+-------------------------------+ --
//		|				| 40 dots  (button or msg area)
//		+-------------------------------+ --
//		|				| 120 dots (sign area 316*120)
//		|				|
//		+-------------------------------+ --
//		|				| 80 dots  (button or msg area)
//		+-------------------------------+ --
//
// INPUT   : orient	- 0x00 = right-handed (default)
//			  0x01 = left-handed
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
//API_LCDTFT_RECT		rect;
UINT32	OS_TSC_ShowSignPad( API_TSC_PARA tscpara, UINT32 orient, UINT8 *palette )
{
UINT8	dhn_lcd = 0;
API_LCDTFT_PARA 	para[1];
API_LCDTFT_RECT		rect[1];
API_LCDTFT_ICON		gicon[1];
API_LCDTFT_GRAPH	graph[1];
//	BSP_Delay_n_ms(100);
//	goto EXIT;
	printf("OS_TSC_ShowSignPad\n");
#if	0
	// clear screen
	memset( (UCHAR *)&para, 0x00, sizeof(API_LCDTFT_PARA) );
	para.RGB = 0;
	para.Row = 0xFFFF;
	para.BG_Palette[0] = 255;	// white (FF FF FF) or orange (FF A5 00)
	para.BG_Palette[1] = 208;
	para.BG_Palette[2] = 139;
	api_lcdtft_clear( dhn_lcd, para );
#endif


	memset( (UCHAR *)rect, 0x00, sizeof(API_LCDTFT_RECT) );
	
	if( orient == 0 )
	  {
	  rect->Xstart	= 0;
	  rect->Xend	= 40+120+1;
	  rect->Ystart	= 0;
	  rect->Yend	= 319;
	  rect->Palette[0]	= palette[0];
	  rect->Palette[1]	= palette[1];
	  rect->Palette[2]	= palette[2];
	  //api_lcdtft_fillRECT( dhn_lcd, (UINT8 *)rect );
	  // Wayne 20/10/28
	  api_lcdtft_fillRECT( dhn_lcd, rect[0] );
	  }
	else
	  {
	  rect->Xstart	= 80;
	  rect->Xend	= 240;
	  rect->Ystart	= 0;
	  rect->Yend	= 319;
	  rect->Palette[0]	= palette[0];
	  rect->Palette[1]	= palette[1];
	  rect->Palette[2]	= palette[2];
	  //api_lcdtft_fillRECT( dhn_lcd, (UINT8 *)rect );
	   // Wayne 20/10/28
	  api_lcdtft_fillRECT( dhn_lcd, rect[0] );
	  }

	// button_OK
#ifdef	_SIGNPAD_PUTG_
	graph->ID	= 0;
	graph->RGB	= 0;
	if( orient == 0 )
	  memmove( &graph->Bitmap, &bmbutton_OK_180, sizeof(GUI_BITMAP) );
	else
	  memmove( &graph->Bitmap, &bmbutton_OK, sizeof(GUI_BITMAP) );
	//api_lcdtft_putgraphics( dhn_lcd, graph );
	// Wayne 20/10/28
	api_lcdtft_putgraphics( dhn_lcd, graph[0] );
#endif

	memset( (UCHAR *)gicon, 0x00, sizeof(gicon) );
	
#ifndef	_SIGNPAD_PUTG_
	if( orient == 0 )
	  gicon->ID = os_SIGNPAD_GID + ID_BUT_OK_180;
	else
	  gicon->ID = os_SIGNPAD_GID + ID_BUT_OK;
#else  
	gicon->ID	= 0;
#endif
	if( orient == 0 )
	  {
	  gicon->Xleft	= 0;
	  gicon->Ytop	= 8;
	  }
	else
	  {
	  gicon->Xleft	= 240-32;
	  gicon->Ytop	= 320-(8+64);
	  }
	gicon->Method	= 0;
	gicon->Width	= 32;
	gicon->Height	= 64;	
	//api_lcdtft_showICON( dhn_lcd, gicon );
	// Wayne
	api_lcdtft_showICON( dhn_lcd, gicon[0] );
		// button_CANCEL, 2015-05-22 for CTBC
#ifdef	_SIGNPAD_PUTG_
	graph->ID	= 0;
	graph->RGB	= 0;
	if( orient == 0 )
	  memmove( &graph->Bitmap, &bmbutton_CANCEL_180, sizeof(GUI_BITMAP) );
	else
	  memmove( &graph->Bitmap, &bmbutton_CANCEL, sizeof(GUI_BITMAP) );
	api_lcdtft_putgraphics( dhn_lcd, graph[0] );
#endif

	memset( (UCHAR *)gicon, 0x00, sizeof(gicon) );

#ifndef	_SIGNPAD_PUTG_
	if( orient == 0 )
	  gicon->ID = os_SIGNPAD_GID + ID_BUT_CANCEL_180;
	else
	  gicon->ID = os_SIGNPAD_GID + ID_BUT_CANCEL;
#else  
	gicon->ID	= 0;
#endif
	if( orient == 0 )
	  {
	  gicon->Xleft	= 0;
	  gicon->Ytop	= 8+64+32+64+40+32+8;
	  }
	else
	  {
	  gicon->Xleft	= 240-32;
	  gicon->Ytop	= 320-(8+64+32+64+40+32+8+64);
	  }
	gicon->Method	= 0;
	gicon->Width	= 32;
	gicon->Height	= 64;
	api_lcdtft_showICON( dhn_lcd, gicon[0] );
	// button_CLEAR
#ifdef	_SIGNPAD_PUTG_
	graph->ID	= 0;
	graph->RGB	= 0;
	if( orient == 0 )
	  memmove( &graph->Bitmap, &bmbutton_CLEAR_180, sizeof(GUI_BITMAP) );
	else
	  memmove( &graph->Bitmap, &bmbutton_CLEAR, sizeof(GUI_BITMAP) );
	api_lcdtft_putgraphics( dhn_lcd, graph[0] );
#endif

	memset( (UCHAR *)gicon, 0x00, sizeof(gicon) );

#ifndef	_SIGNPAD_PUTG_
	if( orient == 0 )
	  gicon->ID = os_SIGNPAD_GID + ID_BUT_CLEAR_180;
	else
	  gicon->ID = os_SIGNPAD_GID + ID_BUT_CLEAR;
#else  
	gicon->ID	= 0;
#endif
	if( orient == 0 )
	  {
	  gicon->Xleft	= 0;
//	  gicon.Ytop	= 8+64+8;
	  gicon->Ytop	= 8+64+32;	// 2014-12-15
	  }
	else
	  {
	  gicon->Xleft	= 240-32;
//	  gicon.Ytop	= 320-(8+64+8+64);
	  gicon->Ytop	= 320-(8+64+32+64);	// 2014-12-15
	  }
	gicon->Method	= 0;
	gicon->Width	= 32;
	gicon->Height	= 64;
	api_lcdtft_showICON( dhn_lcd, gicon[0] );

#if	0
	// button_ROTATE
	graph.ID	= 0;
	graph.RGB	= 0;
	if( orient == 0 )
	  memmove( &graph.Bitmap, &bmbutton_ROTATE_180, sizeof(GUI_BITMAP) );
	else
	  memmove( &graph.Bitmap, &bmbutton_ROTATE, sizeof(GUI_BITMAP) );
	api_lcdtft_putgraphics( dhn_lcd, graph );
	
	memset( (UCHAR *)&gicon, 0x00, sizeof(gicon) );
	gicon.ID	= 0;
	if( orient == 0 )
	  {
	  gicon.Xleft	= 0;
	  gicon.Ytop	= 8+64+8+64+8+64+32+32;	// 32x32
	  }
	else
	  {
	  gicon.Xleft	= 240-32;
//	  gicon.Ytop	= 8+64+8+64+8+64+32;	// 64x32
	  gicon.Ytop	= 320-(8+64+8+64+8+64+32+32+32);	// 32x32
          }
	gicon.Method	= 0;
	gicon.Width	= 32;
//	gicon.Height	= 64;			// 64x32
	gicon.Height	= 32;			// 32x32
	api_lcdtft_showICON( dhn_lcd, gicon );
#endif

#if	0
	// text_AMOUNT
	graph.ID	= 0;
	graph.RGB	= 0;
	memmove( &graph.Bitmap, &bmtext_AMOUNT, sizeof(GUI_BITMAP) );
	api_lcdtft_putgraphics( dhn_lcd, graph );
	
	memset( (UCHAR *)&gicon, 0x00, sizeof(gicon) );
	gicon.ID	= 0;
	gicon.Xleft	= 0;
	gicon.Ytop	= 320-48;
	gicon.Method	= 0;
	gicon.Width	= 32;
	gicon.Height	= 48;
	api_lcdtft_showICON( dhn_lcd, gicon );
#endif

#if	0
	// text_NTD123
	graph.ID	= 0;
	graph.RGB	= 0;
	if( orient == 0 )
	  memmove( &graph.Bitmap, &bmtext_NTD123_180, sizeof(GUI_BITMAP) );
	else
	  memmove( &graph.Bitmap, &bmtext_NTD123, sizeof(GUI_BITMAP) );
	api_lcdtft_putgraphics( dhn_lcd, graph );
	
	memset( (UCHAR *)&gicon, 0x00, sizeof(gicon) );
	gicon.ID	= 0;
	gicon.ID	= 0;
	if( orient == 0 )
	  {
	  gicon.Xleft	= 240-32;
	  gicon.Ytop	= 320-88;
	  }
	else
	  {
	  gicon.Xleft	= 0;
	  gicon.Ytop	= 0;
	  }
	gicon.Method	= 0;
	gicon.Width	= 24;
	gicon.Height	= 88;
	api_lcdtft_showICON( dhn_lcd, gicon );
#endif

	// fill rectangle
	memset( (UCHAR *)&rect, 0x00, sizeof(API_LCDTFT_RECT) );
#if	0	// fixed para
	rect.Xstart	= 40-2;		//32+2;
//	rect.Xend	= 200+2;	//206; SOLUTION 1
	rect.Xend	= 200+2-40;	//206; SOLUTION 2
	rect.Ystart	= 1;
	rect.Yend	= 318;
	rect.Palette[0]	= 0;
	rect.Palette[1]	= 0;
	rect.Palette[2]	= 0;
#endif
#if	1	// variable para
	if( orient == 0 )
	  {
	  rect->Xstart	= tscpara.Y-2;
	  rect->Xend	= tscpara.Y+tscpara.Height+2-1;
	  rect->Ystart	= 1;
	  rect->Yend	= tscpara.Width-2;
	  rect->Palette[0]	= 0;
	  rect->Palette[1]	= 0;
	  rect->Palette[2]	= 0;
	  }
	else
	  {
	  rect->Xstart	= 240-tscpara.Y-tscpara.Height-2;
	  rect->Xend	= rect->Xstart+tscpara.Height+2+1;
	  rect->Ystart	= 1;
	  rect->Yend	= tscpara.Width-2;
	  rect->Palette[0]	= 0;
	  rect->Palette[1]	= 0;
	  rect->Palette[2]	= 0; 
	  }
#endif
	//api_lcdtft_fillRECT( dhn_lcd, (UINT8 *)rect );	// fill black
	// Wayne
	api_lcdtft_fillRECT( dhn_lcd, rect[0] );	// fill black

#if	0	// fixed para
	rect.Xstart	= 40-2+2;	//32+2+2;
//	rect.Xend	= 200+2-2;	//206-2; SOLUTION 1
	rect.Xend	= 200+2-2-40;	//206-2; SOLUTION 2
	rect.Ystart	= 1+2;
	rect.Yend	= 318-2;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0xFF;
	rect.Palette[2]	= 0xFF;
#endif
#if	1	// variable para
	if( orient == 0 )
	  {
	  rect->Xstart	= tscpara.Y-2+2;
	  rect->Xend	= tscpara.Y+tscpara.Height+2-2-1;
	  rect->Ystart	= 1+2;
	  rect->Yend	= tscpara.Width-2-2;
	  rect->Palette[0]	= 0xFF;
	  rect->Palette[1]	= 0xFF;
	  rect->Palette[2]	= 0xFF;
	  }
	else
	  {
	  rect->Xstart	= 240-tscpara.Y-tscpara.Height-2+2;
	  rect->Xend	= rect->Xstart+tscpara.Height-2+1;
	  rect->Ystart	= 1+2;
	  rect->Yend	= tscpara.Width-2-2;
	  rect->Palette[0]	= 0xFF;
	  rect->Palette[1]	= 0xFF;
	  rect->Palette[2]	= 0xFF;  
	  }
#endif
	//api_lcdtft_fillRECT( dhn_lcd, (UINT8 *)rect );	// fill white
	// Wayne
	api_lcdtft_fillRECT( dhn_lcd, rect[0] );	// fill black

	// text_SIGNHERE
#ifdef	_SIGNPAD_PUTG_
	graph->ID	= 0;
	graph->RGB	= 0;
	if( orient == 0 )
	  memmove( &graph->Bitmap, &bmtext_SIGNHERE_180, sizeof(GUI_BITMAP) );
	else
	  memmove( &graph->Bitmap, &bmtext_SIGNHERE, sizeof(GUI_BITMAP) );
	api_lcdtft_putgraphics( dhn_lcd, graph[0] );
#endif

	memset( (UCHAR *)gicon, 0x00, sizeof(gicon) );

#ifndef	_SIGNPAD_PUTG_
	if( orient == 0 )
	  gicon->ID = os_SIGNPAD_GID + ID_TXT_SIGNHERE_180;
	else
	  gicon->ID = os_SIGNPAD_GID + ID_TXT_SIGNHERE;
#else  
	gicon->ID	= 0;
#endif

//	gicon.Xleft	= 104;
	if( orient == 0 )
	  gicon->Xleft	= 104 - 16;
	else
	  gicon->Xleft	= 104 + 16;
	gicon->Ytop	= 112;
	gicon->Method	= 0;
	gicon->Width	= 32;
	gicon->Height	= 96;
	api_lcdtft_showICON( dhn_lcd, gicon[0] );
	
#if	0
	// button_ROTATE
	graph.ID	= 0;
	graph.RGB	= 0;
	if( orient == 0 )
	  memmove( &graph.Bitmap, &bmbutton_ROTATE_180, sizeof(GUI_BITMAP) );
	else
	  memmove( &graph.Bitmap, &bmbutton_ROTATE, sizeof(GUI_BITMAP) );
	api_lcdtft_putgraphics( dhn_lcd, graph );
	
	memset( (UCHAR *)&gicon, 0x00, sizeof(gicon) );
	gicon.ID	= 0;
	if( orient == 0 )
	  {
	  gicon.Xleft	= 0;
	  gicon.Ytop	= 8+64+8+64+8+64+32+32;	// 32x32
	  }
	else
	  {
	  gicon.Xleft	= 240-32;
//	  gicon.Ytop	= 8+64+8+64+8+64+32;	// 64x32
	  gicon.Ytop	= 320-(8+64+8+64+8+64+32+32+32);	// 32x32
          }
	gicon.Method	= 0;
	gicon.Width	= 32;
//	gicon.Height	= 64;			// 64x32
	gicon.Height	= 32;			// 32x32
	api_lcdtft_showICON( dhn_lcd, gicon );
#endif

EXIT:
	OS_TSC_ShowRotateButton( TRUE, orient );

//	BSP_Delay_n_ms(100);

	return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: To clear sign area.
// INPUT   : para	- parameters.
//			 orient
// OUTPUT  : none
//			 
//			 
// RETURN  : TRUE
//	     	 FALSE
// ---------------------------------------------------------------------------
UINT32	OS_TSC_ClearSignpad( API_TSC_PARA para,UINT32 orient )
{
	
UINT8	dhn_lcd = 0;
API_LCDTFT_RECT		rect[1];
	if( orient == 0 )
	  {
	  rect->Xstart	= para.Y-2+2;
	  rect->Xend	= para.Y+para.Height+2-2-1;
	  rect->Ystart	= 1+2;
	  rect->Yend	= para.Width-2-2;
	  rect->Palette[0]	= 0xFF;
	  rect->Palette[1]	= 0xFF;
	  rect->Palette[2]	= 0xFF;
	  }
	else
	  {
	  rect->Xstart	= 240-para.Y-para.Height-2+2;
	  rect->Xend	= rect->Xstart+para.Height-2+1;
	  rect->Ystart	= 1+2;
	  rect->Yend	= para.Width-2-2;
	  rect->Palette[0]	= 0xFF;
	  rect->Palette[1]	= 0xFF;
	  rect->Palette[2]	= 0xFF;  
	  }
	//api_lcdtft_fillRECT( dhn_lcd, (UINT8 *)rect );	// fill white
	// Wayne
	api_lcdtft_fillRECT( dhn_lcd, rect[0] );	 
}

// ---------------------------------------------------------------------------
// FUNCTION: To get image data of the signature.
// INPUT   : dhn
//	     The specified device handle number.
//	     para	- parameters.
// OUTPUT  : status	- status byte.
//			  0: not available (no data)
//			  1: available	   (data available)
//	     sign	- image data of the signature (fixed 128x64 bitmap)
//			  available only when "status" = 1.
//	     length	- length of the image data.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
UINT32	OS_TSC_GetSign( API_TSC_PARA para, UINT8 *status, UINT8 *sign, ULONG *length )
{
UINT8	*pBuf = 0;
UINT8	*pBuf2 = 0;
UINT8	*pBufBak = 0;
UINT8	orient = 0;
UINT32	i;
UINT32	result = FALSE;
UINT32	PayloadSize = 1024;
UINT8	temp[8];
UINT16	Xstart;
UINT16	Xend;
UINT16	Ystart;
UINT16	Yend;
API_LCDTFT_RECT rect;
	#if 1
	if(para.RFU[1] == 0)
	{
		Xstart=para.Y+41;//40+ 1 sign frame
		Ystart=para.X;
		Xend=Xstart+para.Height;
		Yend=Ystart+para.Width;
	}
	else
	{
		Xstart=240-para.Height-para.Y-40;
		Ystart=para.X;
		Xend=Xstart+para.Height;
		Yend=Ystart+para.Width;
	}
	// printf("\033[1;31;40mpara.RFU[1]=%d Xstart=%d Ystart=%d Xend=%d Yend=%d \033[0m\n",para.RFU[1],Xstart,Ystart,Xend,Yend);
	#endif
	#if 0
	if(para.RFU[1] == 0)
	{
		Xstart=para.X;
		Ystart=para.Y;
		Xend=Ystart+para.Width;
		Yend=Xstart+para.Height;
	}
	else
	{
		Xstart=para.X;
		Ystart=240-para.Height-para.Y;
		Xend=Ystart+para.Width;
		Yend=Xstart+para.Height;
	}
	#endif
	*status = 0;
	//if haven't touched or slided in sign area
	if(FIRSTsignFLAG==0)
	{
		//20210915 add by West. if have not signed anything, clear sign area and continue get sign process
		if( para.RFU[1] == 0 )
			rect.Xstart	= 104 - 16;
		else
			rect.Xstart	= 104 + 16;
		
		rect.Xend	= rect.Xstart+32;
		rect.Ystart	= 112;
		rect.Yend	= 112+96;
		rect.Palette[0]	= 0xff;
		rect.Palette[1]	= 0xff;
		rect.Palette[2]	= 0xff;
		api_lcdtft_fillRECT( 0, rect );
	}
	FIRSTsignFLAG=0;
	if( para.RFU[0] == 0 )
	  pBuf = sign;
	else
	  {
	  pBuf = malloc( sizeof(BMPFILEHEADER) + sizeof(BMPINFOHEADER) + sizeof(BMPRGBQUAD)*2 + para.RxLen );
	  pBuf2= malloc( (para.Width*para.Height)/8 );	// 2015-04-07, for RFU[0]=2
	  if( !pBuf )
	    return( FALSE );
	  if( !pBuf2 )
	    {
	    free( pBuf );
	    return( FALSE );
	    }
	    
	  memset( pBuf, 0x00, sizeof(BMPFILEHEADER) + sizeof(BMPINFOHEADER) + sizeof(BMPRGBQUAD)*2 + para.RxLen );
	  }
	  printf("before getpixelvalue");
	pBufBak = pBuf;	// pointer backup
	rotation=0;//20210915 add by west to make sure orientation when every time getsign
	getpixelvalue(Ystart,Xstart,Yend,Xend,sign);
	// printf("sign[]=\n");
	
	// printf("\n");
	if( para.RFU[1] == 0 )	// reverse direction? (CCW_180)
	  {
	  OS_LCDTFT_TransformCharBitmap( para.Height, para.Width, sign );
	  OS_LCDTFT_TransformCharBitmap( para.Width, para.Height, sign );
	  }
	for( i=0; i<sizeof(SIGNFRAME); i++ )
	{
	     sign[i] |= ~SIGNFRAME[i];	
	}  
	memmove(pBuf,sign,sizeof(BMPFILEHEADER) + sizeof(BMPINFOHEADER) + sizeof(BMPRGBQUAD)*2 + para.RxLen);
	
	if( para.RFU[0] == 1 )	// convert to WINDOWS BMP?
	  {
	  result = TSC_Convert2WinBMP( para.Width, para.Height, para.RxLen, pBuf, length, sign );
	  free( pBufBak );
	  free( pBuf2 );	// 2015-04-01
	  }
	else if( para.RFU[0] == 2 )  // 2015-04-01, append fixed RawData[320x120] or RawData[120x240] to WinBmp, "length" is effective for WinBmp only
	  {	    
	    memmove( pBuf2, pBufBak, (para.Width*para.Height)/8 );	// backup RawData
	    result = TSC_Convert2WinBMP( para.Width, para.Height, para.RxLen, pBufBak, length, sign );
		
		//modified 20210118 reverse 1 and 0 for turn bitmap format from lcd to printer
		for(i=0;i<((para.Width*para.Height)/8);i++)
			pBuf2[i]=~pBuf2[i];
	    memmove( &sign[*length], pBuf2, (para.Width*para.Height)/8 );	// append RawData to WinBmp
	    
	    free( pBufBak );
	    free( pBuf2 );	// 2015-04-01	    
	  }
	printf("\033[1;31;40mresult=%d\033[0m\n",result);  
	return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: Convert bitmap format from SYMLINK (raw format) to WINDOWS (standard bmp format).
// INPUT   : rawlen - length of raw bmp.
//	     rawbmp - raw bmp.
//	     width  - raw bmp width.
//	     height - raw bmp height.
// OUTPUT  : winlen - length of windows bmp.
//	     winbmp - windows bmp.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
UINT32	TSC_Convert2WinBMP( ULONG rawWidth, ULONG rawHeight, ULONG rawLen, UCHAR *rawBmp, ULONG *winLen, UCHAR *winBmp )
{
UINT32	i, j, k;
UINT32	result = FALSE;
UINT32	BytesPerRow = 0;
UINT8	*bmpPixData = 0;
UINT8	data1, data2;
UINT8	buffer1[14];
UINT8	buffer2[40];
UINT8	buffer3[4];
UINT8	buffer4[4];
BMPFILEHEADER	*bmpFileHeader = 0;
BMPINFOHEADER	*bmpInfoHeader = 0;
BMPRGBQUAD	*bmpRgbQuad1 = 0;
BMPRGBQUAD	*bmpRgbQuad2 = 0;


	if( rawLen > sizeof(TSC_SignRawBuffer) )
	  return( FALSE );
	memset( TSC_SignRawBuffer, 0x00, sizeof(TSC_SignRawBuffer) );
	
	*winLen = 0;
//	bmpFileHeader = (BMPFILEHEADER *)BSP_Malloc( sizeof(BMPFILEHEADER) );
	bmpFileHeader = (BMPFILEHEADER *)&buffer1;
	
	if( bmpFileHeader )
	  {
	  bmpFileHeader->bfType		= 0x4D42;	// "BM"
	  bmpFileHeader->bfSize		= sizeof(BMPFILEHEADER) + sizeof(BMPINFOHEADER) + sizeof(BMPRGBQUAD)*2 + rawLen;
	  bmpFileHeader->bfReserved1	= 0;
	  bmpFileHeader->bfReserved2	= 0;
	  bmpFileHeader->bfOffBits	= 0x3E;
	  
	  memmove( winBmp, bmpFileHeader, sizeof(BMPFILEHEADER) );
	  winBmp += sizeof(BMPFILEHEADER);
	  *winLen += sizeof(BMPFILEHEADER);
	  
//	  bmpInfoHeader = (BMPINFOHEADER *)BSP_Malloc( sizeof(BMPINFOHEADER) );
	  bmpInfoHeader = (BMPINFOHEADER *)&buffer2;
	  if( bmpInfoHeader )
	    {
	    bmpInfoHeader->biSize	= 40;
	    bmpInfoHeader->biWidth	= rawWidth;
	    bmpInfoHeader->biHeight	= rawHeight;
	    bmpInfoHeader->biPlanes	= 1;
	    bmpInfoHeader->biBitCount	= 1;
	    bmpInfoHeader->biCompression	= 0;
	    bmpInfoHeader->biSizeImage	= rawLen;
	    bmpInfoHeader->biXPelsPerMeter	= 0x0B40;
	    bmpInfoHeader->biYPelsPerMeter	= 0x0B40;
	    bmpInfoHeader->biClrUsed		= 0;
	    bmpInfoHeader->biClrImportant	= 0;
	    
	    memmove( winBmp, bmpInfoHeader, sizeof(BMPINFOHEADER) );
	    winBmp += sizeof(BMPINFOHEADER);
	    *winLen += sizeof(BMPINFOHEADER);
	    
//	    bmpRgbQuad1 = (BMPRGBQUAD *)BSP_Malloc( sizeof(BMPRGBQUAD) );
	    bmpRgbQuad1 = (BMPRGBQUAD *)&buffer3;
	    if( bmpRgbQuad1 )
	      {
	      bmpRgbQuad1->rgbBlue	= 0;
	      bmpRgbQuad1->rgbGreen	= 0;
	      bmpRgbQuad1->rgbRed	= 0;
	      bmpRgbQuad1->rgbReserved	= 0;
	      
	      memmove( winBmp, bmpRgbQuad1, sizeof(BMPRGBQUAD) );
	      winBmp += sizeof(BMPRGBQUAD);
	      *winLen += sizeof(BMPRGBQUAD);
	      
//	      bmpRgbQuad2 = (BMPRGBQUAD *)BSP_Malloc( sizeof(BMPRGBQUAD) );
	      bmpRgbQuad2 = (BMPRGBQUAD *)&buffer4;
	      if( bmpRgbQuad2 )
	        {
	        bmpRgbQuad2->rgbBlue	= 0xFF;
	        bmpRgbQuad2->rgbGreen	= 0xFF;
	        bmpRgbQuad2->rgbRed	= 0xFF;
	        bmpRgbQuad2->rgbReserved	= 0;
	        
	        memmove( winBmp, bmpRgbQuad2, sizeof(BMPRGBQUAD) );
	        winBmp += sizeof(BMPRGBQUAD);
	        *winLen += sizeof(BMPRGBQUAD);
	        
	        result = TRUE;
	        }
	      }
	    }
	  }
	if( result )
	  {
	  // upside down and inverse the raw data
	  result = FALSE;
	  BytesPerRow = bmpInfoHeader->biWidth / 8;	// bytes per row of the graphics (shall be multiple of 4 bytes)
	  
//	  bmpPixData = (UINT8 *)BSP_Malloc( sizeof(rawLen) );
	  bmpPixData = (UINT8 *)&TSC_SignRawBuffer;
	  if( bmpPixData )
	    {
	    k = bmpInfoHeader->biHeight - 1;
	    for( i=0; i<(UINT32)(bmpInfoHeader->biHeight/2); i++, k-- )
	       {
	       for( j=0; j<BytesPerRow; j++ )
	         {
	         data1 = rawBmp[i*BytesPerRow+j];	// up data <-> down data
	         data2 = rawBmp[k*BytesPerRow+j];	//
	         
			 //output color will reversed
	         // bmpPixData[i*BytesPerRow+j] = ~data2;
	         // bmpPixData[k*BytesPerRow+j] = ~data1;
			 
			 bmpPixData[i*BytesPerRow+j] = data2;
	         bmpPixData[k*BytesPerRow+j] = data1;
	         }
	       }
	    	    
//	    for( i=0; i<rawLen; i++ )
//	       bmpPixData[rawLen-1-i] = ~(*rawBmp++);
	       
	    memmove( winBmp, bmpPixData, rawLen );
	    *winLen += rawLen;
	    
	    result = TRUE;
	    }
	  }
	
//	if( bmpFileHeader )
//	  BSP_Free( bmpFileHeader );
//	
//	if( bmpInfoHeader )
//	  BSP_Free( bmpInfoHeader );
//	  
//	if( bmpRgbQuad1 )
//	  BSP_Free( bmpRgbQuad1 );
//	
//	if( bmpRgbQuad2 )
//	  BSP_Free( bmpRgbQuad2 );
//	  
//	if( bmpPixData )
//	  BSP_Free( bmpPixData );
	
	return( result );
}

#if	0
// ---------------------------------------------------------------------------
// FUNCTION: To get image data of the signature.
// INPUT   : dhn
//	     The specified device handle number.
//	     para	- parameters.
// OUTPUT  : status	- status byte.
//			  0: not available (no data)
//			  1: available	   (data available)
//	     sign	- image data of the signature (fixed 128x64 bitmap)
//			  available only when "status" = 1.
//	     length	- length of the image data.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------



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
UINT32	OS_LCDTFT_ClrRow( API_LCDTFT_PARA para )
{
SPI_LCDTFT	spilcd;
UINT32	result;

	
	spilcd.DevID = DID_LCD;
	spilcd.CmdID = CID_LCD_CLR_PIXROW;
	memmove( (UINT8 *)&spilcd.LcdPara, (UINT8 *)&para, sizeof(API_LCDTFT_PARA) );

	result = OS_SPI_PutPacket( sizeof(SPI_LCDTFT), (UINT8 *)&spilcd );
	
	BSP_Delay_n_ms(10);
	
	return( result );
}

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
	  
//	x = height * row * 8;	// X
	x = height * row;	// X
	if( x >= MAX_LCDTFT_PIXROW_NO )
	  return( FALSE );
	
	if( os_LCDTFT_ATTR & LCD_ATTR_YDOT_MASK )
	  {
	  if( (col+width) >= MAX_LCDTFT_PIXCOL_NO )
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

// ---------------------------------------------------------------------------
// FUNCTION: To 
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
	  
	  if( OS_SPI_PutPacket( sizeof(SPI_LCDTFT)+length, pData ) )
	    result = TRUE;
	    
	  BSP_Free( pData );
	  }

	return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: Transform printer image into AS350 display format. (counter clockwise 90 degrees)
// INPUT   : Height     - height of char font.
//           Width      - width of char font.
//           pImageData - the image data to be transformed.
// OUTPUT  : pImageData - the image data with new orientation.
// RETURN  : TRUE
//	     FALSE - out of spec.
// ---------------------------------------------------------------------------
UINT32	OS_LCDTFT_TransformCharBitmap( UINT8 Height, UINT8 Width, UINT8 *pImageData )
{
UINT32	i, j, k, l, m, n;
UINT32	BytesPerRow;
UINT32	BytesPerCol;
UINT32	right_shift;
UINT32	left_shift;
UINT8	c1, c2;
UINT8	pImageData2[128];	// max 32x32 char font


	if( (Height*Width)/8 > sizeof(pImageData2) )
	  return( FALSE );

	memset( pImageData2, 0x00, sizeof(pImageData2) );

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
	            
	         pImageData2[(BytesPerCol)*m + j + (i*BytesPerCol*8)] = c2;
	         m++;
	         right_shift++;
	         }
	      }
	   }
	   
	memmove( pImageData, pImageData2, BytesPerRow*BytesPerCol*8 );
	
	return( TRUE );
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
SPI_PCDICON	spipcd;

	
	spipcd.DevID = DID_PCD;
	spipcd.CmdID = CID_PCD_SHOW_PCD;
	memmove( (UINT8 *)&spipcd.IconPara, (UINT8 *)&icon, sizeof(API_PCD_ICON) );

	return( OS_SPI_PutPacket( sizeof(SPI_PCDICON), (UINT8 *)&spipcd ) );
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
	  
	  *pData2++ = datalen & 0x000000FF;
	  *pData2++ = (datalen & 0x0000FF00) >> 8;
	  *pData2++ = (datalen & 0x00FF0000) >> 16;
	  *pData2++ = (datalen & 0xFF000000) >> 24;
	  
	  *pData2++ = graph.RGB;
	  
	  *pData2++ = offset & 0x000000FF;
	  *pData2++ = (offset & 0x0000FF00) >> 8;
	  *pData2++ = (offset & 0x00FF0000) >> 16;
	  *pData2++ = (offset & 0xFF000000) >> 24;
	  
	  *pData2++ = 0;	// RFU(1)
	  
	  // put packet data
	  memmove( pData2, graph.Bitmap.pData + offset, datalen );
	  
	  if( OS_SPI_PutPacket( sizeof(SPI_LCDTFT_GRAPH)+datalen, pData ) )
	    result = TRUE;
	  else
	    {
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
#if 0
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
UINT32	OS_LCDTFT_ShowICON( API_LCDTFT_ICON icon )
{
UINT32	result;
SPI_LCDICON	spiicon;

	
	spiicon.DevID	= DID_LCD;
	spiicon.CmdID	= CID_LCD_SHOW_IMAGE;
	memmove( (UINT8 *)&spiicon.IconPara, (UINT8 *)&icon, sizeof(API_LCDTFT_ICON) );
	
	spiicon.IconPara.Xleft	= icon.Ytop;
	spiicon.IconPara.Ytop	= MAX_LCDTFT_PIXCOL_NO - icon.Xleft - icon.Width;

	result = OS_SPI_PutPacket( sizeof(SPI_LCDICON), (UINT8 *)&spiicon );
	
	BSP_Delay_n_ms(100);
	return( result );
}




#endif
// ---------------------------------------------------------------------------
// FUNCTION: To open TSC device.
// INPUT   : 
//	     ------ TFT ------	// parameter structure 16 bytes
//	     UINT   ID;		// 
//	     UINT   X;		// 
//	     UINT   Y;		// 
//	     UINT   Width;	//
//	     UINT   Height;	//
//	     UINT   Timeout;	//
//	     UCHAR  RFU[4];	// reserved (4 bytes)
//
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
void TSC_ExtIOInitialize(UCHAR dev_stage)
{
	if(dev_stage==DEV_OPEN){
		//add mirror property
		UCHAR tx[4];
		UCHAR rx[4];
		tx[0]=0x44;
		tx[1]=0x05;//IOCON
		tx[2]=0xE8;//BANK=1,MIRROR=1,SEQOP=1,HAEN=1
		SPI_Transfer(tx,rx,0,EXTIO,4);
		
		//get which interrupt pin enabled
		tx[0]=0x45;
		tx[1]=0x02;//GPINTENA
		tx[2]=0x00;
		SPI_Transfer(tx,rx,0,EXTIO,4);
		tx[2]=rx[2];
		tx[2]|=0x40;//make bit7 to 1
		tx[0]=0x44;
		SPI_Transfer(tx,rx,0,EXTIO,4);
		
		//INTCON configs same as GPINTEN
		tx[1]=0x04;//INTCONA
		tx[2]=0x40;//make bit 7 to 1
		SPI_Transfer(tx,rx,0,EXTIO,4);
		
		
		//get interrupt pins base status
		tx[0]=0x45;
		tx[1]=0x03;//DEFVALA
		tx[2]=0x00;
		SPI_Transfer(tx,rx,0,EXTIO,4);
		tx[2]=rx[2];
		tx[2]|=0x40;//make bit 7 to 1
		tx[0]=0x44;
		SPI_Transfer(tx,rx,0,EXTIO,4);
	}
	else{
		//add mirror property
		UCHAR tx[4];
		UCHAR rx[4];
		tx[0]=0x44;
		tx[1]=0x05;//IOCON
		tx[2]=0xE8;//BANK=1,MIRROR=1,SEQOP=1,HAEN=1
		SPI_Transfer(tx,rx,0,EXTIO,4);
		
		//get which interrupt pin enabled
		tx[0]=0x45;
		tx[1]=0x02;//GPINTENA
		tx[2]=0x00;
		SPI_Transfer(tx,rx,0,EXTIO,4);
		tx[2]=rx[2];
		tx[2]&=~0x40;//make bit 7 to 0
		tx[0]=0x44;
		SPI_Transfer(tx,rx,0,EXTIO,4);
		
		//get INTCON configs
		tx[0]=0x45;
		tx[1]=0x04;//INTCONA
		tx[2]=0;
		SPI_Transfer(tx,rx,0,EXTIO,4);
		tx[2]=rx[2];
		tx[2]&=~0x40;//make bit 7 to 0
		tx[0]=0x44;
		SPI_Transfer(tx,rx,0,EXTIO,4);
		
		//get interrupt pins base status
		tx[0]=0x45;
		tx[1]=0x03;//DEFVALA
		tx[2]=0x00;
		SPI_Transfer(tx,rx,0,EXTIO,4);
		tx[2]=rx[2];
		tx[2]&=~0x40;//make bit 7 to 0
		tx[0]=0x44;
		SPI_Transfer(tx,rx,0,EXTIO,4);
	}
}
// ---------------------------------------------------------------------------
// FUNCTION: To open TSC device.
// INPUT   : none. 
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
UINT32	OS_TSC_Open()
{
UINT32 os_DHN_TSC;
UCHAR data=1;
UCHAR *SHM_data=NULL;
TSC_ExtIOInitialize(DEV_OPEN);
	if(TSC_fd<1){
		bsp_shm_acquire(SHM_data);//power button stuff
		bsp_shm_IrqOccupied(1,&data);
		TSC_fd = open("/dev/input/event2", O_RDWR);
		if(TSC_fd==-1)
			return( FALSE );
	}
	if(TSC_timerdhn==0)
	{
		TSC_timerdhn=api_tim_open( 1 );//10ms timer
		if(TSC_timerdhn==apiOutOfService)
			return( FALSE );
	}
	setcolor(0, 0x000000);
	rotation=0;
	os_DHN_TSC=TSC_fd;
	ypos_bak=0;
	xpos_bak=0;
	return( os_DHN_TSC );
}
// ---------------------------------------------------------------------------
// FUNCTION: To close TSC device.
// INPUT   : 
//	     ------ TFT ------	// parameter structure 16 bytes
//	     UINT   ID;		// 
//	     UINT   X;		// 
//	     UINT   Y;		// 
//	     UINT   Width;	//
//	     UINT   Height;	//
//	     UINT   Timeout;	//
//	     UCHAR  RFU[4];	// reserved (4 bytes)
//
// OUTPUT  : none.
// RETURN  : TRUE
//	     	 FALSE
// ---------------------------------------------------------------------------
UINT32	OS_TSC_Close()
{
UCHAR data;
	data=0;
	if(TSC_fd<3)
	{
		printf("TSC_fd=%d\n",TSC_fd);
		return(FALSE);		
	}
	if(close(TSC_fd)<0)
		return(FALSE); 
	
	bsp_shm_IrqOccupied(1,&data);//release irq event flag
	TSC_fd = 0;
	api_tim_close( TSC_timerdhn );
	TSC_ExtIOInitialize(DEV_CLOSE);
	ypos_bak=0;
	xpos_bak=0;
	return(TRUE); 
}