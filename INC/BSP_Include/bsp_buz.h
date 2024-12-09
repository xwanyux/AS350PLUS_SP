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
#ifndef _BSP_BUZ_H_
#define _BSP_BUZ_H_

#include "bsp_types.h"
#include "bsp_tmr.h"
#include "bsp_gpio.h"


#define MIN_BUZ_FEQ		100
#define MAX_BUZ_FEQ		4000


typedef struct				BSP_BUZ_S
{
	UINT32					Freq;
	BSP_SEM					IsIdle;
} BSP_BUZ;



extern BSP_STATUS BSP_BUZ_Init( void );
extern BSP_BUZ * BSP_BUZ_Acquire( void );
extern BSP_STATUS BSP_BUZ_Release( BSP_BUZ * pBuz );
extern BSP_STATUS BSP_BUZ_Start( BSP_BUZ * pBuz, UINT32 FreqInHz, UINT32 Delay_ms );
extern BSP_STATUS BSP_BUZ_Stop( BSP_BUZ * pBuz );


#endif
