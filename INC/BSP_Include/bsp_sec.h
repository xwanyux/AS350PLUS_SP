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
#ifndef _BSP_SEC_H_
#define _BSP_SEC_H_

#include "bsp_types.h"
#include "za9_sec.h"
#include "za9_pmu.h"
#include "bsp_utils.h"



typedef struct BSP_SEC_S
{
	UINT32						SecCfg1;
	UINT32						SecCfg2;
	UINT32						SecCtrl;
	UINT32						SecSta;
	UINT32						SecDsbl;

	UINT32						PmuRst;
	UINT32						PmuOvck;
} BSP_SEC;


/*
 * Function Prototypes
 */
extern BSP_STATUS BSP_SEC_Init( void );
extern BSP_SEC  * BSP_SEC_Acquire( void );
extern BSP_STATUS BSP_SEC_Release( BSP_SEC * pSec );
extern BSP_STATUS BSP_SEC_Read( BSP_SEC * pSec );
extern BSP_STATUS BSP_SEC_Write( BSP_SEC * pSec );
extern BSP_STATUS BSP_SEC_Unlock( BSP_SEC * pSec );



#endif
