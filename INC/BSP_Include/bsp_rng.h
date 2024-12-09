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
#ifndef _BSP_RNG_H_
#define _BSP_RNG_H_

#include "bsp_types.h"
#include "za9_rng.h"
#include "za9_pmu.h"


typedef	BSP_HANDLE BSP_RNG;


/*
 * Function Prototypes
 */
extern BSP_STATUS BSP_RNG_Init( void );

extern BSP_RNG * BSP_RNG_Acquire( void );
extern BSP_STATUS BSP_RNG_Release( BSP_RNG * pRng );

extern UINT32 BSP_RNG_GenWord( BSP_RNG * pRng );
extern UINT32 BSP_RNG_GenVector(	BSP_RNG * pRng, UINT8 * pVector,	UINT32 LengthMask );
extern BSP_STATUS BSP_RNG_GenVectorLen(	BSP_RNG * pRng, UINT8 * pVector,	UINT32 Length );



#endif
