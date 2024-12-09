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
#ifndef _BSP_DMA_H_
#define _BSP_DMA_H_

#include "bsp_types.h"
#include "bsp_int.h"
#include "za9_dma.h"
#include "za9_pmu.h"
#include "bsp_utils.h"


/*
 * Dma Channel number identifies
 */
#define DMA_CHANNEL_0					0
#define DMA_CHANNEL_1					1
#define DMA_CHANNEL_2					2
#define DMA_CHANNEL_3					3
#define DMA_CHANNEL_4					4
#define DMA_CHANNEL_5					5
#define DMA_CHANNEL_6					6
#define DMA_CHANNEL_7					7
#define ANY_DMA_CHANNEL					0xFF

#define NUM_DMA_CHANNELS				8



typedef struct 				DMAC_CONFIG_S
{
	UINT32						CfgReg;
	UINT32						SrcReg;
	UINT32						DstReg;
	UINT32						CntReg;
	UINT32						SrcRldReg;
	UINT32						DstRldReg;
	UINT32						CntRldReg;

	UINT32						StatusReg;
} DMA_CONFIG;


typedef struct BSP_DMA_S
{
	BSP_SEM						IsAvail;			// If TRUE, channel can be acquired
	BSP_SEM						IsIdle;			// If TRUE, no current DMA operation in progress
	UINT32						Channel;			// Identifies HW channel number (0 - 7)
	UINT32						Base;				// Base address of DMA registers
	DMA_CONFIG					Config;			// ZA9-speciifc channel configuration
	UINT32						IsrCount;		// Counts number of times the low level ISR runs
	void						(*	pIsrCB)			// Upper-layer interrupt callback
	(													// callback parameters		
		struct BSP_DMA_S   * pDma 				// DMA Port reference
	);		
} BSP_DMA;


typedef void (* DMA_ISR_FUNC )( BSP_DMA * pDma ); 




BSP_STATUS BSP_DMA_Init( void );

BSP_DMA * BSP_DMA_Acquire( UINT32 Channel, DMA_ISR_FUNC pLisr );
BSP_STATUS BSP_DMA_Start( BSP_DMA * pDma );
BSP_STATUS BSP_DMA_Reload( BSP_DMA * pDma );
BSP_STATUS BSP_DMA_Stop( BSP_DMA * pDma );
BSP_STATUS BSP_DMA_Resume( BSP_DMA * pDma );
BSP_STATUS BSP_DMA_Release( BSP_DMA * pDma );
 


#endif

