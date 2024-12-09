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
#ifndef _BSP_SSD_H_
#define _BSP_SSD_H_

#include "bsp_types.h"
#include "bsp_int.h"
#include "za9_daa.h"



typedef struct BSP_SSD_S
{
	UINT32						IsrCount;		// Counts number of times the low level ISR runs
	void						(*	pIsrFunc)		// Upper layer interrupt callback
	(
		struct BSP_SSD_S 	 * pSsd 				// Callback parameters		
	);
	BSP_INT					 * pInt;
} BSP_SSD;

typedef void (* SSD_ISR_FUNC )( BSP_SSD * pSsd ); 



/*
 * Function Prototypes
 */
extern BSP_STATUS BSP_SSD_Init( void );
extern BSP_SSD * BSP_SSD_Acquire( SSD_ISR_FUNC pIsr );
extern BSP_STATUS BSP_SSD_Release( BSP_SSD * pSsd );

extern void BSP_SSD_EnableInt( void );
extern void BSP_SSD_DisableInt( void );

#ifdef BSP_OS_IS_LINUX
	#define BSP_SSD_EnableInt()		enable_irq( INTNUM_SSD );
	#define BSP_SSD_DisableInt()		disable_irq( INTNUM_SSD );
#else
	#define BSP_SSD_EnableInt()		BSP_WR32( INTC_ESET_REG, INT_MASK_SSD );
	#define BSP_SSD_DisableInt()		BSP_WR32( INTC_ECLR_REG, INT_MASK_SSD );
#endif



#endif

