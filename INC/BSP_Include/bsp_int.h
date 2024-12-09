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
#ifndef _BSP_INT_H_
#define _BSP_INT_H_

#include "za9_int.h"
#include "za9_pmu.h"
#include "bsp_types.h"



#define BSP_MAX_INT_CHANNELS				32

/*
 * BSP Interrupt Masks
 */
#define BSP_IRQ_MASK							0x80
#define BSP_FIQ_MASK							0x40
#define BSP_INT_MASK							0xC0


typedef void 			   (* BSP_ISR_FUNC )( UINT32 Channel );

typedef struct					BSP_INT_S
{
	BSP_BOOL						InUse;
	UINT32						Channel;
	UINT32						Mask;
	BSP_ISR_FUNC				pIsrCB;
} BSP_INT;




BSP_STATUS BSP_INT_Init( void );
BSP_INT *  BSP_INT_Acquire( UINT32 Channel, BSP_ISR_FUNC pIsr );
BSP_STATUS BSP_INT_Release( BSP_INT * pInt );


void BSP_INT_Mask( BSP_INT * pInt );
void BSP_INT_Unmask( BSP_INT * pInt );
#ifdef BSP_OS_IS_LINUX
	#define BSP_INT_Mask( pInt ) 		disable_irq(pInt->Channel)
	#define BSP_INT_Unmask( pInt )	enable_irq(pInt->Channel)
#else
	#define BSP_INT_Mask( pInt ) 		BSP_WR32( INTC_ECLR_REG, (pInt->Mask) )
	#define BSP_INT_Unmask( pInt )	BSP_WR32( INTC_ESET_REG, (pInt->Mask) )
#endif

#define BSP_INT_Start( pInt )		BSP_INT_Unmsk( pInt )
#define BSP_INT_Stop( pInt )		BSP_INT_Mask( pInt )

/*
 * These routines can be used to disable/ enable IRQ and FIQ interrupts
 */
UINT32 BSP_DisableInterrupts( UINT32 Mask );
UINT32 BSP_EnableInterrupts( 	UINT32 Mask );
UINT32 BSP_RestoreInterrupts( UINT32 IntState );

void OsIntInit( void );
BSP_STATUS OsIntAcquire( UINT32 Channel, BSP_ISR_FUNC pLisr );
BSP_STATUS OsIntRelease( BSP_INT * pInt );



#endif
