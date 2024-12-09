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
#ifndef _BSP_UTILS_H_
#define _BSP_UTILS_H_

#include "bsp_types.h"

typedef BSP_BOOL BSP_SEM;



/*
 * bsp_clock.c
 */
extern UINT32 BSP_GetClock( BSP_BOOL GetHclk, UINT32 Scale );


/*
 * bsp_delay.c
 */
extern void BSP_Delay_n_ms( 	UINT32 Count );


/*
 * bsp_endian.c
 */
extern UINT32 BSP_EndianReverse( UINT32 Data );


/*
 * bsp_sem.c
 */
extern void BSP_SemCreate( BSP_SEM	* pSem );
extern BSP_BOOL BSP_SemAcquire( BSP_SEM	* pSem );
extern BSP_BOOL BSP_SemRelease( BSP_SEM * pSem );
#define BSP_SemCreate( pSem ) BSP_SemRelease( (pSem) )


/*
 * bsp_init.c
 */
extern void BSP_Halt( void );



#endif
