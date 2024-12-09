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
#ifndef _BSP_PRN_H_
#define _BSP_PRN_H_

#include "za9_defines.h"
#include "bsp_types.h"
#include "bsp_utils.h"

#define SPI1_CTL_REG  						(SPI1_BASE + SPI_CTRL_REG)
#define SPI1_STA_REG  						(SPI1_BASE + SPI_STA_REG)
#define SPI1_BRG_REG  						(SPI1_BASE + SPI_BRG_REG)

#define MEMC_CFG_7_REG 						0xFFFF8030
#define MEMC_TIM_7_REG 						0xFFFF8058

#define PRN_ABORT_TEMP						0x3FF
#define PRN_MIN_DARKNESS					0x00
#define PRN_MAX_DARKNESS					0x0F
#define PRN_DEF_DARKNESS					0x0C



typedef struct					BSP_PRN_S
{
	UINT32						CtrlReg;

	BSP_SEM			    		IsIdle;
	BSP_SEM						CanPrint;

	UINT32						DeacStrobe;

	/*
	 * Set darkness between 0 (lightest) and 15 (darkest)
	 */
	UINT32						Darkness;
	/*
	 * During printing, the print head temperature is monitored via the ADC (10-bit).
	 * The MinTemp and MaxTemp values record the highest and lowest print
	 * head measurements.  Printing will be aborted if the a reading >= AbortTemp
	 * is detected.
	 */
	UINT16						MinTemp;
	UINT16						MaxTemp;
	UINT16						AbortTemp;
} BSP_PRN;



extern BSP_STATUS BSP_PRN_Init( void );
extern BSP_PRN* BSP_PRN_Acquire( void );
extern BSP_STATUS BSP_PRN_Release(   BSP_PRN *pPrn );
extern BSP_STATUS BSP_PRN_Write(     BSP_PRN *pPrn, UINT16 *pData, UINT32 NumRows );
extern BSP_STATUS BSP_PRN_Start(     BSP_PRN *pPrn );
extern BSP_STATUS BSP_PRN_Stop(      BSP_PRN *pPrn );
extern BSP_STATUS BSP_PRN_PaperFeed( BSP_PRN *pPrn, UINT32 Lines );



#endif
