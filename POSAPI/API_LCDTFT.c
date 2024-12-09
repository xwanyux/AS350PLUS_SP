#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "fbutils.h"
#include "OS_FONT.h"
#include "POSAPI.h"
#include "ExtIODev.h"
#include "DEV_LCD.h"
#include "bsp_gpio.h"
#include "bsp_types.h"

UCHAR SIGNFRAME_WINBMP[] = {
#include "sign-frame.h"
};
extern void backlight_control_LCDTFT(UCHAR operation,UCHAR bright); 
extern UCHAR		SYS_brightness;
UCHAR		os_DHN_LCDTFT = 0;
UCHAR		os_LCDTFT_ATTR = 0;

UCHAR		os_LCDTFT_WorkBuffer[320*240/8];	// max 320x240 monochrome graph
UCHAR		IS_FRAMEBUFFER_OPENED=0;				//checked framebuffer is opened
UCHAR		os_LCD_ConvertFont=0;
UINT32		os_LCDTFT_InUse2;
ULONG		os_HpCopFlag;
// ---------------------------------------------------------------------------
// FUNCTION: To check if DHN matched.
// INPUT   : dhn
// OUTPUT  : none.
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
UCHAR	LCDTFT_CheckDHN( UCHAR dhn )
{
	if( (dhn == 0) || (dhn == os_DHN_LCDTFT) )
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
UCHAR	api_lcdtft_open( UCHAR deviceid )
{
int result;
	OS_FONT_Init(0);
	if( os_DHN_LCDTFT != 0 )	// already opened?
	  return( os_DHN_LCDTFT );
	
	BSP_IO_Init();
	UCHAR *tx=malloc(sizeof(UCHAR)*4);
	UCHAR *rx=malloc(sizeof(tx));
	IS_FRAMEBUFFER_OPENED=open_framebuffer();
	setcolor(0, 0xffffff);
	fillrect(0, 0, xres - 1, yres - 1,0);
	//enable extension IO hardware chip select mode and enter byte mode
	// *tx=0x40;
	// *(tx+1)=0x0B;//IOCON
	// *(tx+2)=0xA8;//BANK=1,SEQOP=1,HAEN=1
	// SPI_Transfer(tx,rx,0,EXTIO,4);
	printf("SYS_brightness=%d\n",SYS_brightness);
	backlight_control_LCDTFT(1,SYS_brightness);
	free(tx);
	free(rx);
	
	  
	os_DHN_LCDTFT = 4;
	return( os_DHN_LCDTFT );
}
// ---------------------------------------------------------------------------
// FUNCTION: To disable the display device, no more message can be shown on the screen.
// INPUT   : dhn
//	     The specified device handle number.
//	     0x00 = to close all opened tasks.
// OUTPUT  : none.
// RETURN  : apiOK
//	     apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_lcdtft_close( UCHAR dhn )
{
	if( LCDTFT_CheckDHN( dhn ) == TRUE )
	  {
	  os_DHN_LCDTFT = 0;	
	  return( apiOK );
	  }
	else
	  return( apiFailed );
}
// ---------------------------------------------------------------------------
// FUNCTION: To clear the designated display area.
// INPUT   : dhn
//	     The specified device handle number.
//
//	     para
//	     ------ TFT ------	// parameter structure 16 bytes
//	     UINT   row;	// beginning row number
//	     UINT   col;	// total pixel rows to be cleared (x8)
//	     UCHAR  font;	// font id & attribute
//	     UCHAR  rgb;	// RGB mode
//	     UCHAR  FG_palette[3];	// foreground palette (3 bytes)
//	     UCHAR  fontH;	// font height in dots
//	     UCHAR  fontW;	// font width in dots
//	     UCHAR  BG_palette[3];	// background palette (3 bytes)
//	     UCHAR  RFU[2];	// reserved (2 bytes)
//
// OUTPUT  : none.
// RETURN  : apiOK
//	     apiFailed
// NOTE    : If the "start row position" is -1, it means to clear the whole screen,
//           only the "font attribute" is effective.
// ---------------------------------------------------------------------------
//UCHAR	api_lcdtft_clear( UCHAR dhn, UCHAR *sbuf )
UCHAR	api_lcdtft_clear( UCHAR dhn, API_LCDTFT_PARA para )
{
UINT	row;
UINT	cnt;
UCHAR	font;
UCHAR	fid;
OS_FDT	*pFdt;
API_LCDTFT_PARA para2;
//API_LCDTFT_PARA para;

	//memmove( &para, sbuf, sizeof(API_LCDTFT_PARA) );
	
	if(!IS_FRAMEBUFFER_OPENED){//if framebuffer not opened.
		return( apiDeviceNotOpen );
	}
	if( LCDTFT_CheckDHN( dhn ) == TRUE )
	{
	  row = para.Row;
	  cnt = para.Col;
	  font = para.Font;	// attr + fid
	  fid = font & 0x0F;	// fid
	  if( (row != 0xFFFF) && (cnt == 0) ) // count = 0?
	    return( apiOK );	// do nothing
	    
	  if( row == 0xFFFF )	// clear whole screen?
	    {
		while(os_LCDTFT_InUse2);//20210911 add by west. Wait untill another process done of framebuffer manipulating
		os_LCDTFT_InUse2 = TRUE;	
	    if( OS_LCDTFT_ClrScreen( para ) )
		{
			os_LCDTFT_InUse2 = FALSE;
	      return( apiOK );
	    }
		else
		{
			os_LCDTFT_InUse2 = FALSE;
	      return( apiFailed );
	    }
		}

	  OS_FONT_Init(fid);
	  pFdt = OS_FONT_GetFdtAddr( fid );
	  memmove( (UINT *)&para2, (UINT *)&para, sizeof(API_LCDTFT_PARA) );

#if	1	// row++ for X6 PCI only (shifted down 1 row)
	  para.Row += 1;
	  para2.Row += 1;
#endif

	  if( !(font & LCD_ATTR_XYDOT_MASK) )
	    {
			para2.Row = para.Row * pFdt->Height;
			para2.Col = para.Col * pFdt->Height;
			if(para2.Row>320)
				para2.Row = para.Row;
			if(para2.Row+para2.Col>320)
				para2.Col = para.Col;				
	    }
		while(os_LCDTFT_InUse2);//20210911 add by west. Wait untill another process done of framebuffer manipulating
	  os_LCDTFT_InUse2 = TRUE;
	  rotation=ROTATE_0;
	//   printf("^^Row=%d\tCol=%d\n",para.Row,para.Col);
	  if( OS_LCDTFT_ClrRow( para2 ) )
	    {
			os_LCDTFT_InUse2 = FALSE;
	    return( apiOK );
	    }
	  else
	    {
	    os_LCDTFT_InUse2 = FALSE;
	    return( apiFailed );
	    }     
	}
	else
	  return( apiFailed );
}

//UCHAR	api_lcdtft_putstring( UCHAR dhn, UCHAR *sbuf, UCHAR *dbuf )
UCHAR api_lcdtft_putstring( UCHAR dhn, API_LCDTFT_PARA para, UCHAR *dbuf )
{
UINT	row;
UINT	col;
UCHAR	font;
UCHAR	fid;
UCHAR	data;
UCHAR	*pData;
ULONG	len;

UCHAR	attr = 0;
OS_FDT	*pFdt = 0;
API_LCDTFT_PARA para2;
//API_LCDTFT_PARA para;
	//memmove( &para, sbuf, sizeof(API_LCDTFT_PARA) );
	if( LCDTFT_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );
	//len = sizeof(dbuf);
	len = *dbuf++;
	// printf("API TFT len=%d  ",len);
	pData = dbuf;
	if( len == 0 )
	  return( apiFailed );	// invalid data length
	row = para.Row;
	col = para.Col;
	font = para.Font;	// attr + fid
	fid = font & 0x0F;	// fid
	attr = font & 0xF0;	// attr
	memmove( (UINT *)&para2, (UINT *)&para, sizeof(API_LCDTFT_PARA) );

#if	1	// row++ for X6 PCI only (shifted down 1 row)
	para2.Row += 1;
#endif

	// printf("font=0x%02x\n",font);
	os_LCDTFT_ATTR = font & 0xF0;
	// printf("para.FontHeight=%d  para.FontWidth=%d\n",para2.FontHeight,para2.FontWidth);
	pFdt = OS_FONT_GetFdtAddr( fid );
	para2.Font=		pFdt->FontID;
	//len=			pFdt->ByteNo;		
	para2.FontHeight=pFdt->Height;
	para2.FontWidth=pFdt->Width;
	// printf("pFdt->Height=%d  pFdt->Width=%d",pFdt->Height,pFdt->Width);
	// printf("para.FontHeight=%d  para.FontWidth=%d\n",para2.FontHeight,para2.FontWidth);
	if(os_LCDTFT_ATTR<LCD_ATTR_XYDOT_MASK){//if pixel cursor not enable
	// printf("os_LCDTFT_ATTR<LCD_ATTR_XYDOT_MASK\n");
		para2.Row=para2.Row*(pFdt->Height);
		para2.Col=para2.Col*(pFdt->Width);
		
	}
	else
		os_LCDTFT_ATTR-=LCD_ATTR_XYDOT_MASK;
		
	if(!IS_FRAMEBUFFER_OPENED){//if framebuffer not opened.
		return( apiDeviceNotOpen );
	}
	
	if( os_LCDTFT_ATTR >= LCD_ATTR_CLEARWRITE_MASK )	// clear before writing?
	  {	  
	  memmove( (UINT *)&para, (UINT *)&para2, sizeof(API_LCDTFT_PARA) );
	  para2.Col=pFdt->Height;
	  while(os_LCDTFT_InUse2);//20210911 add by west. Wait untill another process done of framebuffer manipulating
	  os_LCDTFT_InUse2 = TRUE;
	  rotation=ROTATE_0;
	  OS_LCDTFT_ClrRow( para2 );
	//   para2.Col=para.Col;
	  memmove( (UINT *)&para2, (UINT *)&para, sizeof(API_LCDTFT_PARA) );
	  os_LCDTFT_InUse2 = FALSE;
	  os_LCDTFT_ATTR-=LCD_ATTR_CLEARWRITE_MASK;
	  }
	if( os_LCDTFT_ATTR == LCD_ATTR_REVERSE_MASK )
	  {
	  data = para2.BG_Palette[0];
	  para2.BG_Palette[0] = para2.FG_Palette[0];
	  para2.FG_Palette[0] = data;

	  data = para2.BG_Palette[1];
	  para2.BG_Palette[1] = para2.FG_Palette[1];
	  para2.FG_Palette[1] = data;
	  
	  data = para2.BG_Palette[2];
	  para2.BG_Palette[2] = para2.FG_Palette[2];
	  para2.FG_Palette[2] = data;
	  }
	// if(fid==FONT_0)
	
	os_HpCopFlag=1;
	while(os_LCDTFT_InUse2);//20210911 add by west. Wait untill another process done of framebuffer manipulating
	os_LCDTFT_InUse2 = TRUE;
	if( os_HpCopFlag ){
	  if( (*pData)>0x9F || para2.Font==FONT2 || para2.Font==FONT4 || para2.Font==FONT12)
	    {
			// printf("para2.Font=%d\n",para2.Font);
			// printf("para.FontHeight=%d  para.FontWidth=%d\n",para2.FontHeight,para2.FontWidth);
			rotation=ROTATE_0;//int8_t rotation from fbutils-linux.c
			OS_LCDTFT_PutBG5Str(pData,para2,len);
		}
	  else{
		  rotation=ROTATE_0;//int8_t rotation from fbutils-linux.c
		  OS_LCDTFT_PutStr( pData, para2,len );
	  }
	}
	os_LCDTFT_InUse2 = FALSE;
	// printf("API printstring done\n");
	return( apiOK );
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
// RETURN  : apiOK
//	     apiFailed
// ---------------------------------------------------------------------------
//UCHAR	api_lcdtft_putgraphics( UCHAR dhn, API_LCDTFT_GRAPH *sbuf )
UCHAR	api_lcdtft_putgraphics( UCHAR dhn, API_LCDTFT_GRAPH graph)
{
	//API_LCDTFT_GRAPH graph;
	//memmove( &graph, sbuf, sizeof(API_LCDTFT_GRAPH) );
	if( LCDTFT_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );
	  while(os_LCDTFT_InUse2);//20210911 add by west. Wait untill another process done of framebuffer manipulating
	  os_LCDTFT_InUse2 = TRUE;
	OS_LCDTFT_PutGraphics( graph );
	os_LCDTFT_InUse2 = FALSE;
	return( apiOK );
}
//UCHAR	api_lcdtft_showICON( UCHAR dhn, API_LCDTFT_ICON *sbuf )
UCHAR   api_lcdtft_showICON( UCHAR dhn, API_LCDTFT_ICON icon )
{
//API_LCDTFT_ICON icon;


	//memmove( &icon, sbuf, sizeof(API_LCDTFT_ICON) );
	if( LCDTFT_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );
	
	if(!IS_FRAMEBUFFER_OPENED){//if framebuffer not opened.
		return( apiDeviceNotOpen );
	}
	while(os_LCDTFT_InUse2);//20210911 add by west. Wait untill another process done of framebuffer manipulating
	os_LCDTFT_InUse2 = TRUE;
	OS_LCDTFT_ShowICON( icon );  
	os_LCDTFT_InUse2 = FALSE;
	return( apiOK );
}
// ---------------------------------------------------------------------------
// FUNCTION: To control related graphics appearance for PCD application,
//	     such as TAP ICON, LED ICON...
// INPUT   : dhn
//	     The specified device handle number.
//
//	     para
//	     ------ TFT ------	// 
//	     UINT   ID;		// id of the built-in icon (0..n)
//	     UINT   BlinkOn;	// time for icon ON period in 10ms units  (0xFFFF=always ON, 0x0000=no blinking)
//	     UINT   BlinkOff;	// time for icon OFF period in 10ms units
// 	     UCHAR  RFU[10];
// OUTPUT  : none.
// RETURN  : apiOK
//	     apiFailed
// NOTE    : PCD kernel shall handle the LEDs while running contactless app.
// ---------------------------------------------------------------------------
UCHAR	api_lcdtft_showPCD( UCHAR dhn, API_PCD_ICON icon )
{
	if( LCDTFT_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );
	
	if( OS_LCDTFT_ShowPCD( icon ) )
	{
	  return( apiOK );
	}
	else
	  return( apiFailed );
}
// ---------------------------------------------------------------------------
// FUNCTION: To write the given Windows bmp image data to the display at the designated area.
// INPUT   : dhn
//	     The specified device handle number.
//
//	     bmp
//	     The Windows format BMP data. (monochrome, black & white)
//
//	     degrees
//	     CCW_0, CCW_90, CCW_180, CCW_270
// OUTPUT  : none.
// RETURN  : apiOK
//	     apiFailed
// ---------------------------------------------------------------------------
//UCHAR	api_lcdtft_putwinbmp( UCHAR dhn, UCHAR *sbuf, UCHAR *winbmp )
UCHAR	api_lcdtft_putwinbmp( UCHAR dhn, API_LCDTFT_WINBMP *bmppara, UCHAR *winbmp )
{
ULONG	i;
UINT	id;
UINT	CCWdegrees;
UINT	g_width = 0;
UINT	g_height = 0;
UCHAR	fg_palette[3];
UCHAR	bg_palette[3];
UINT	method;
//API_LCDTFT_WINBMP *bmppara;
API_LCDTFT_RECT rect;


	//bmppara = (API_LCDTFT_WINBMP *)sbuf;

	id = bmppara->ID;
	CCWdegrees = bmppara->CCWdegrees;

	if( LCDTFT_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );
	
	memmove( fg_palette, bmppara->FG_Palette, 3 );
	memmove( bg_palette, bmppara->BG_Palette, 3 );
	memmove( os_LCDTFT_WorkBuffer, winbmp, sizeof(os_LCDTFT_WorkBuffer) );
	method = bmppara->Method;
	if( method & WINBMP_SIGN_FRAME )
	  {
		for( i=0; i<sizeof(SIGNFRAME_WINBMP)-62; i++ )
	     os_LCDTFT_WorkBuffer[i+62] &= ~SIGNFRAME_WINBMP[i+62];
	  }
	  while(os_LCDTFT_InUse2);//20210911 add by west. Wait untill another process done of framebuffer manipulating
	  os_LCDTFT_InUse2 = TRUE;
	if( OS_LCDTFT_ConvertWIN2GUI( id, CCWdegrees, fg_palette, bg_palette, &g_width, &g_height, os_LCDTFT_WorkBuffer ) )
	  {
		  os_LCDTFT_InUse2 = FALSE;
	  bmppara->Width[0] = g_width;
	  bmppara->Height[0] = g_height;
	  
	  memset( os_LCDTFT_WorkBuffer, 0x00, sizeof(os_LCDTFT_WorkBuffer) );
	  return( apiOK );
	  }
	else
	  {
		  os_LCDTFT_InUse2 = FALSE;
	  memset( os_LCDTFT_WorkBuffer, 0x00, sizeof(os_LCDTFT_WorkBuffer) );
	  return( apiFailed );
	  }
}
//UCHAR	api_lcdtft_fillRECT( UCHAR dhn, UCHAR *sbuf )
UCHAR api_lcdtft_fillRECT( UCHAR dhn, API_LCDTFT_RECT rect)
{
//API_LCDTFT_RECT rect;

	//memmove( &rect, sbuf, sizeof(API_LCDTFT_RECT) );
	
	if( LCDTFT_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );
	while(os_LCDTFT_InUse2);//20210911 add by west. Wait untill another process done of framebuffer manipulating
	os_LCDTFT_InUse2 = TRUE;  
	OS_LCDTFT_FillRECT( rect );
	os_LCDTFT_InUse2 = FALSE;
	return( apiOK );
}
//UCHAR	SIGNPAD_lcdtft_putstring( UCHAR dhn, UCHAR *sbuf, UCHAR *dbuf, UINT CCWdegrees )
UCHAR  SIGNPAD_lcdtft_putstring( UCHAR dhn, API_LCDTFT_PARA para, UCHAR *dbuf, UINT CCWdegrees )
{
UINT	row;
UINT	col;
UCHAR	font;
UCHAR	fid;
UCHAR	data;
UCHAR	*pData;
ULONG	len;
UINT	ccw;
UCHAR	attr = 0;
OS_FDT	*pFdt = 0;
API_LCDTFT_PARA para2;

//API_LCDTFT_PARA para;

	//memmove( &para, sbuf, sizeof(API_LCDTFT_PARA) );
	if( LCDTFT_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );
  
	len = *dbuf++;
	pData = dbuf;
	if( len == 0 )
	  return( apiFailed );	// invalid data length
	
	ccw = CCWdegrees;	
	row = para.Row;
	col = para.Col;
	font = para.Font;	// attr + fid
	fid = font & 0x0F;	// fid
	attr = font & 0xF0;	// attr
	memmove( (UINT *)&para2, (UINT *)&para, sizeof(API_LCDTFT_PARA) );
	OS_FONT_Init(fid);
	os_LCDTFT_ATTR = font & 0xF0;
	pFdt = OS_FONT_GetFdtAddr( fid );
	para2.Font=		pFdt->FontID;
	//len=			pFdt->ByteNo;
	para2.FontHeight=pFdt->Height;
	para2.FontWidth=pFdt->Width;
	
	if(!(os_LCDTFT_ATTR==LCD_ATTR_XYDOT_MASK)){//if pixel cursor not enable
		para2.Row=para2.Row*(pFdt->Height);
		para2.Col=para2.Col*(pFdt->Width);
	}
	
	if(!IS_FRAMEBUFFER_OPENED){//if framebuffer not opened.
		return( apiDeviceNotOpen );
	}
	
	if( font & LCD_ATTR_CLEARWRITE_MASK )	// clear before writing?
	  {	  
		  while(os_LCDTFT_InUse2);//20210911 add by west. Wait untill another process done of framebuffer manipulating
	  os_LCDTFT_InUse2 = TRUE;
		if( ccw == CCW_90 )
			rotation=ROTATE_90;
		else if( ccw == CCW_270 )
			rotation=ROTATE_270;
		else
			return( apiFailed );
		
	  OS_LCDTFT_ClrRow( para2 );
	  os_LCDTFT_InUse2 = FALSE;
	  }
	if( font & LCD_ATTR_REVERSE_MASK )
	  {
	  data = para2.BG_Palette[0];
	  para2.BG_Palette[0] = para2.FG_Palette[0];
	  para2.FG_Palette[0] = data;

	  data = para2.BG_Palette[1];
	  para2.BG_Palette[1] = para2.FG_Palette[1];
	  para2.FG_Palette[1] = data;
	  
	  data = para2.BG_Palette[2];
	  para2.BG_Palette[2] = para2.FG_Palette[2];
	  para2.FG_Palette[2] = data;
	  }

	 while(os_LCDTFT_InUse2);//20210911 add by west. Wait untill another process done of framebuffer manipulating
	os_LCDTFT_InUse2 = TRUE;
	os_HpCopFlag=1;
	if( os_HpCopFlag ){
	  //FONT2 and FONT4 are using BIG5 display method
	  if( fid > FONT_12 )
	    {
			os_LCDTFT_InUse2 = FALSE;
			return( apiFailed );
		}
	  else if( ccw == CCW_90 )
	  {
		  rotation=ROTATE_90;
		  ((fid == FONT_2)||(fid == FONT_4))?OS_LCDTFT_PutBG5Str( pData, para2,len ):OS_LCDTFT_PutStr( pData, para2,len );
	  }
	  else if( ccw == CCW_270 )
	  {
		  rotation=ROTATE_270;
		  ((fid == FONT_2)||(fid == FONT_4))?OS_LCDTFT_PutBG5Str( pData, para2,len ):OS_LCDTFT_PutStr( pData, para2,len );
	  }
	  else{
		  os_LCDTFT_InUse2 = FALSE;
			return( apiFailed );
	  }
	}
	os_LCDTFT_InUse2 = FALSE;
	return( apiOK );
}

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
	// printf("@@fid=%d\n",fid);
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
UCHAR api_lcdtft_initfont( API_LCD_FONT ft  )
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
void	api_lcd_convertfont( UCHAR flag )
{
	os_LCD_ConvertFont = flag;
}