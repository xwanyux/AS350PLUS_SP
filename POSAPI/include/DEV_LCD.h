#ifndef _DEV_LCD_H_
#define _DEV_LCD_H_


#include <stdio.h>
#include <stdlib.h>
#include "POSAPI.h"
#include "DEV_LCD.h"
#include "LCDTFTAPI.h"
#include "fbutils.h"

#define GW12864B_ACTIVE_NS					1000		// new 1000ns, old: 472
#define GW12864B_INACTIVE_NS					800		// new 1500ns, old: 2936
#define GW12864B_SETUP_NS					147		//
#define GW12864B_HOLD_NS					30		// new 40ns, old: 11

#define	LCD_SEGMENT_LEN						64

#define	MAX_LCD_X_NO						64
#define MAX_LCD_ROW_NO						8
#define	MAX_LCD_COL_NO						21

#define	LCD_STATUS_BUSY						0x80
#define	LCD_STATUS_OFF						0x20
#define	LCD_STATUS_RESET					0x10

#define	LCD_CMD_SET_X_ADDR					(LCD_RS_LO | 0xB8)
#define	LCD_CMD_SET_Y_ADDR					(LCD_RS_LO | 0x40)
#define	LCD_CMD_SET_START_LINE					(LCD_RS_LO | 0xC0)
#define	LCD_CMD_OFF						(LCD_RS_LO | 0x3E)
#define	LCD_CMD_ON						(LCD_RS_LO | 0x3F)

#define	LCD_ATTR_REVERSE_MASK					0x10
#define	LCD_ATTR_CLEARWRITE_MASK				0x20
#define	LCD_ATTR_YDOT_MASK					0x40
#define	LCD_ATTR_XYDOT_MASK					0x40
#define	LCD_ATTR_ISO						0x80

#define	BMP_OW							0x00
#define	BMP_OR							0x01
#define	BMP_AND							0x02
#define	BMP_XOR							0x03

#define	LCD_DID_STN						0x00
#define	LCD_DID_TFT						0x01
#define	LCD_DID_TSC						0x02
#ifndef _SCREEN_SIZE_480x320
	#define	LCD_XRES						240
	#define	LCD_YRES						320
	#define	ROTATE_0						3
	#define	ROTATE_90						2
	#define	ROTATE_180						1
	#define	ROTATE_270						0
#else
	#define	LCD_XRES						320
	#define	LCD_YRES						480
	#define	ROTATE_0						0
	#define	ROTATE_90						3
	#define	ROTATE_180						2
	#define	ROTATE_270						1
#endif
#define	GRAPHIC_ID_MAX_ORDER						100
#define	GRAPHIC_MAX_SIZE						4194304//4MB
typedef	struct	LCDTFT_GRAPH_S			// TFT LCD graphics
{
	UINT		ID;			// id number of the built-in icon (0..n)
	UINT		Width;			// image width in dots in horizontal direction
	UINT		Height;			// image height in dots in vertical direction
	ULONG		Size;			// size of the image in bytes
	UCHAR		RGB;			// RGB mode
	
	UCHAR		RFU[5];
} __attribute__((packed)) LCDTFT_GRAPH;
typedef	struct	LCDTFT_ICON_S			// TFT LCD icon
{
	UINT		ID;			// id number of the built-in icon (0..n)
	UINT		Xleft;			// x positon
	UINT		Ytop;			// y position
	UINT		Method;			// dispaly method
	UINT		Width;
	UINT		Height;
	
	UCHAR		RFU[4];
} __attribute__((packed)) LCDTFT_ICON;
UINT32 OS_LCDTFT_ShowICON(API_LCDTFT_ICON icon);
UINT32 OS_LCDTFT_PutGraphics( API_LCDTFT_GRAPH graph );
UCHAR OS_LCDTFT_FillRECT(API_LCDTFT_RECT rect);
UCHAR OS_LCDTFT_ClrScreen(API_LCDTFT_PARA para);
UCHAR OS_LCDTFT_ClrRow(API_LCDTFT_PARA para);
UCHAR OS_LCDTFT_PutStr( UCHAR *data,API_LCDTFT_PARA para,UCHAR fontLen );
UCHAR OS_LCDTFT_PutBG5Str(UCHAR *data,API_LCDTFT_PARA para,UCHAR fontLen);
UINT8 *TransformBitmapFile( UINT8 *winbmp, UINT32 *xsize, UINT32 *ysize );
UINT32 OS_LCDTFT_ConvertWIN2GUI( UINT16 id, UINT16 CCWdegrees, UCHAR *fg_palette, UCHAR *bg_palette, UINT16 *width, UINT16 *height, UINT8 *winbmp );
UINT32 OS_LCDTFT_TransformCharBitmap( UINT32 Height, UINT32 Width, UINT8 *pImageData );
UINT32	OS_LCDTFT_ShowPCD( API_PCD_ICON icon );
#endif