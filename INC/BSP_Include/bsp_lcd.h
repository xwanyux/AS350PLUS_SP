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
#ifndef _BSP_LCD_H_
#define _BSP_LCD_H_

#include "bsp_types.h"
#include "bsp_utils.h"
#include "za9_lcd.h"
#include "za9_pmu.h"
#include "bsp_gpio.h"



typedef struct					BSP_LCD_S
{
	BSP_SEM						IsIdle;
	UINT32						Control;
	BSP_GPIO					 * pBackLight;
	BSP_GPIO					 * pColSel;
   BSP_GPIO					 * pLcdCtrl;
	BSP_GPIO					 * pLcdData;
} BSP_LCD;



/*
 * Function Prototypes
 */
extern BSP_STATUS BSP_LCD_Init( void );
extern BSP_LCD *  BSP_LCD_Acquire( void );
extern BSP_STATUS BSP_LCD_Start( BSP_LCD * pLcd );
extern BSP_STATUS BSP_LCD_Stop( BSP_LCD * pLcd );
extern BSP_STATUS BSP_LCD_Release( BSP_LCD * pLcd );

extern BSP_STATUS BSP_LCD_Write( BSP_LCD * pLcd, UINT32	Data );
extern UINT32 BSP_LCD_Read( BSP_LCD * pLcd, UINT32 Addr );


extern BSP_STATUS BSP_LCD_BackLight( BSP_LCD * pLcd, BSP_BOOL SetHi );
extern BSP_STATUS BSP_LCD_ColSel( BSP_LCD * pLcd, BSP_BOOL SetHi );



#endif
