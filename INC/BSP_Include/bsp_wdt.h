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
#ifndef _BSP_WDT_H_
#define _BSP_WDT_H_

#include "bsp_types.h"
#include "za9_wdt.h"
#include "bsp_int.h"
#include "bsp_utils.h"



typedef struct BSP_WDT_S
{
	UINT32						Control;			// WDT Control Register
	void						(*	pIsrFunc)		// Upper-layer interrupt callback
	(													// callback parameters		
		struct BSP_WDT_S   * pWdt				// WDT reference
	);		
	BSP_INT					 * pInt;				// BSP structure for interrupt control
} BSP_WDT;


typedef void (* WDT_ISR_FUNC )( BSP_WDT * pWdt ); 


/*
 * Function Prototypes
 */
extern BSP_STATUS BSP_WDT_Init( void );
extern BSP_WDT  * BSP_WDT_Acquire( WDT_ISR_FUNC pIsr );
extern BSP_STATUS BSP_WDT_Release( BSP_WDT * pWdt );
extern BSP_STATUS BSP_WDT_Start( BSP_WDT * pWdt );
extern BSP_STATUS BSP_WDT_Stop( BSP_WDT * pWdt );



#endif
