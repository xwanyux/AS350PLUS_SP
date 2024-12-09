/*
 * Copyright 2006, ZiLOG Inc.
 * All Rights Reserved
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of ZiLOG Inc., and might
 * contain proprietary, confidential and trade secret information of
 * ZiLOG, our partners and parties from which this code has been licensed.
 * 
 * The contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of ZiLOG Inc.
 */
#ifndef _DMC20434_H_
#define _DMC20434_H_

#include "bsp_lcd.h"


/*
 * The DMC204343 is a 20 character x 4 line display
 */
#define DMC20434_MAX_COLS					20
#define DMC20434_MAX_ROWS					4

/*
 * The following macros identify the display ram address of the first element
 * on each row.  Therefore to position the cursor on Row 3, Col 5 the display
 * ram address is 0x54 + 0x05 = 0x59
 */
#define DMC20434_ROW_0						0x00
#define DMC20434_ROW_1						0x40
#define DMC20434_ROW_2						0x14
#define DMC20434_ROW_3						0x54


/*
 * Supported DMC-20434 commands
 */

// Read Commands
#define DMC_READ_BUSY_ADDR					(LCD_RS_LO | 0x00)
#define DMC_READ_DATA  						(LCD_RS_HI | 0x00)
// Busy bit set while display process command
#define DMC_BUSY								0x80

// Write Commands
#define DMC_CLEAR_HOME     				(LCD_RS_LO | 0x01)
#define DMC_HOME_CURSOR						(LCD_RS_LO | 0x02)
#define DMC_AUTO_INC_ADDR					(LCD_RS_LO | 0x06)
#define DMC_AUTO_DEC_ADDR					(LCD_RS_LO | 0x04)
#define DMC_ON_CURSOR_BLINK				(LCD_RS_LO | 0x0F)
#define DMC_ON_CURSOR						(LCD_RS_LO | 0x0E)
#define DMC_ON_BLINK							(LCD_RS_LO | 0x0D)
#define DMC_ON									(LCD_RS_LO | 0x0C)
#define DMC_OFF								(LCD_RS_LO | 0x08)
#define DMC_SHIFT_RIGHT						(LCD_RS_LO | 0x1C)
#define DMC_SHIFT_LEFT						(LCD_RS_LO | 0x18)
#define DMC_CURSOR_RIGHT					(LCD_RS_LO | 0x14)
#define DMC_CURSOR_LEFT						(LCD_RS_LO | 0x10)
#define DMC_2LINE_5X7						(LCD_RS_LO | 0x38)
#define DMC_1LINE_5X7						(LCD_RS_LO | 0x30)
#define DMC_SET_CGRAM_ADDR					(LCD_RS_LO | 0x40)
#define DMC_SET_DDRAM_ADDR					(LCD_RS_LO | 0x80)
#define DMC_WRITE_DATA						(LCD_RS_HI | 0x00)


/*
 * Macros for reading the display
 */
#define DMC20434_READ_ADDR( pData )		DMC20434_Read( DMC_READ_BUSY_ADDR, (pData) )
#define DMC20434_READ_DATA( pData )		DMC20434_Read( DMC_READ_DATA, (pData) )

/*
 * Macros for issuing commands
 */
#define DMC20434_CLEAR()					DMC20434_Write( DMC_CLEAR_HOME )
#define DMC20434_HOME_CURSOR()			DMC20434_Write( DMC_HOME_CURSOR )
#define DMC20434_AUTO_INC_ADDR()			DMC20434_Write( DMC_AUTO_INC_ADDR )
#define DMC20434_AUTO_DEC_ADDR()			DMC20434_Write( DMC_AUTO_DEC_ADDR )
#define DMC20434_ON_CURSOR_BLINK()		DMC20434_Write( DMC_ON_CURSOR_BLINK )
#define DMC20434_ON_CURSOR()				DMC20434_Write( DMC_ON_CURSOR )
#define DMC20434_ON_BLINK()				DMC20434_Write( DMC_ON_BLINK )
#define DMC20434_ON()						DMC20434_Write( DMC_ON )
#define DMC20434_OFF()						DMC20434_Write( DMC_OFF )
#define DMC20434_SHIFT_RIGHT()			DMC20434_Write( DMC_SHIFT_RIGHT )
#define DMC20434_SHIFT_LEFT()				DMC20434_Write( DMC_SHIFT_LEFT )
#define DMC20434_CURSOR_RIGHT()			DMC20434_Write( DMC_CURSOR_RIGHT )
#define DMC20434_CURSOR_LEFT()			DMC20434_Write( DMC_CURSOR_LEFT )
#define DMC20434_CURSOR_POS(Row, Col)	DMC20434_Write( DMC_SET_DDRAM_ADDR | DDRowOffset[(Row)] + (Col) )
#define DMC20434_CGRAM_POS(Char, Row)	DMC20434_Write( DMC_SET_CGRAM_ADDR | ((Char)<<3) | (Row) )
#define DMC20434_PUT_CHAR(Data)			DMC20434_Write( DMC_WRITE_DATA | (Data) )


extern UINT32 DDRowOffset[ DMC20434_MAX_ROWS ];



/*
 * Function Prototypes
 */
extern BSP_STATUS DMC20434_Write( UINT32 Data );
extern BSP_STATUS DMC20434_Read( UINT32 Addr, UINT8 * pData );
extern BSP_STATUS DMC20434_Write_Data( char * pData, UINT32	Len );
extern BSP_STATUS DMC20434_Init( void );
extern BSP_STATUS DMC20434_BackLight( BSP_BOOL SetHi );
extern BSP_STATUS DMC20434_Terminate( void );



#endif
