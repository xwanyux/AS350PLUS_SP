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
#ifndef _ZA9_WDT_H_
#define _ZA9_WDT_H_

#include "za9_defines.h"


/*
 * WDT Registers
 */
#define WDT_CTRL_REG    		(WDT_BASE | 0x000)
#define WDT_RR_REG      		(WDT_BASE | 0x004)


/*
 * WDT_CTRL_REG bit definitions
 */
#define WDT_RST_EN          				BIT10
#define WDT_INT_EN          				BIT9
#define WDT_EN              				BIT8
#define WDT_RESET_MASK      				(0xF << 4)
#define WDT_RESET_PERIOD_2_POW_16		(0xF << 4)
#define WDT_RESET_PERIOD_2_POW_17		(0xE << 4)
#define WDT_RESET_PERIOD_2_POW_18		(0xD << 4)
#define WDT_RESET_PERIOD_2_POW_19		(0xC << 4)
#define WDT_RESET_PERIOD_2_POW_20		(0xB << 4)
#define WDT_RESET_PERIOD_2_POW_21		(0xA << 4)
#define WDT_RESET_PERIOD_2_POW_22		(0x9 << 4)
#define WDT_RESET_PERIOD_2_POW_23		(0x8 << 4)
#define WDT_RESET_PERIOD_2_POW_24		(0x7 << 4)
#define WDT_RESET_PERIOD_2_POW_25		(0x6 << 4)
#define WDT_RESET_PERIOD_2_POW_26		(0x5 << 4)
#define WDT_RESET_PERIOD_2_POW_27		(0x4 << 4)
#define WDT_RESET_PERIOD_2_POW_28		(0x3 << 4)
#define WDT_RESET_PERIOD_2_POW_29		(0x2 << 4)
#define WDT_RESET_PERIOD_2_POW_30		(0x1 << 4)
#define WDT_RESET_PERIOD_2_POW_31		(0x0 << 4)
#define WDT_INT_PERIOD_MASK      		(0xF << 0)
#define WDT_INT_PERIOD_2_POW_16			(0xF << 0)
#define WDT_INT_PERIOD_2_POW_17			(0xE << 0)
#define WDT_INT_PERIOD_2_POW_18			(0xD << 0)
#define WDT_INT_PERIOD_2_POW_19			(0xC << 0)
#define WDT_INT_PERIOD_2_POW_20			(0xB << 0)
#define WDT_INT_PERIOD_2_POW_21			(0xA << 0)
#define WDT_INT_PERIOD_2_POW_22			(0x9 << 0)
#define WDT_INT_PERIOD_2_POW_23			(0x8 << 0)
#define WDT_INT_PERIOD_2_POW_24			(0x7 << 0)
#define WDT_INT_PERIOD_2_POW_25			(0x6 << 0)
#define WDT_INT_PERIOD_2_POW_26			(0x5 << 0)
#define WDT_INT_PERIOD_2_POW_27			(0x4 << 0)
#define WDT_INT_PERIOD_2_POW_28			(0x3 << 0)
#define WDT_INT_PERIOD_2_POW_29			(0x2 << 0)
#define WDT_INT_PERIOD_2_POW_30			(0x1 << 0)
#define WDT_INT_PERIOD_2_POW_31			(0x0 << 0)


/*
 * WDT_RR_REG bit definitions
 */
#define WDT_RR_MASK     		0xFF
#define WDT_RR_FIRST      		0xA5
#define WDT_RR_SECOND			0x5A



#endif

