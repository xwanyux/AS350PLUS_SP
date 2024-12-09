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
#ifndef _ZA9_RTC_H_
#define _ZA9_RTC_H_

#include "za9_defines.h"



/*
 * WDT Registers
 */
#define RTC_SEC_REG  		   			(RTC_BASE | 0x00)
#define RTC_MIN_REG  		   			(RTC_BASE | 0x04)
#define RTC_HRS_REG  		   			(RTC_BASE | 0x08)
#define RTC_DOM_REG  		   			(RTC_BASE | 0x0C)
#define RTC_MON_REG  		   			(RTC_BASE | 0x10)
#define RTC_YR_REG   		   			(RTC_BASE | 0x14)
#define RTC_TIME_REG 		   			(RTC_BASE | 0x18)
#define RTC_ASEC_REG  		   			(RTC_BASE | 0x1C)
#define RTC_AMIN_REG  		   			(RTC_BASE | 0x20)
#define RTC_AHRS_REG  		   			(RTC_BASE | 0x24)
#define RTC_ADOM_REG  		   			(RTC_BASE | 0x28)
#define RTC_AMON_REG  		   			(RTC_BASE | 0x2C)
#define RTC_AYR_REG  		   			(RTC_BASE | 0x30)
#define RTC_ACTRL_REG  		   			(RTC_BASE | 0x34)
#define RTC_CTRL_REG  		   			(RTC_BASE | 0x38)



/*
 * RTC_SEC_REG
 * RTC_ASEC_REG bit definitions
 */
#define RTC_SEC_MASK							(BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0)

/*
 * RTC_MIN_REG
 * RTC_AMIN_REG bit definitions
 */
#define RTC_MIN_MASK							(BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0)

/*
 * RTC_HRS_REG
 * RTC_AHRS_REG bit definitions
 */
#define RTC_HRS_MASK							(BIT4 | BIT3 | BIT2 | BIT1 | BIT0)

/*
 * RTC_DOM_REG
 * RTC_ADOM_REG bit definitions
 */
#define RTC_DOM_MASK							(BIT4 | BIT3 | BIT2 | BIT1 | BIT0)

/*
 * RTC_MON_REG
 * RTC_AMON_REG bit definitions
 */
#define RTC_MON_MASK							(BIT3 | BIT2 | BIT1 | BIT0)

/*
 * RTC_YR_REG
 * RTC_AYR_REG bit definitions
 * Reading this register gives the current offset from the Base Year (1900).
 * The Maximum value is 255 corresponding to the year 2155
 */
#define RTC_BASE_YEAR						1900
#define RTC_YR_MASK							(BIT7 | BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0)

/*
 * RTC_TIME_REG bit definitions
 */
#define RTC_TIME_SEC_MASK					(RTC_SEC_MASK << 0)
#define RTC_TIME_MIN_MASK					(RTC_MIN_MASK << 6)
#define RTC_TIME_HRS_MASK					(RTC_HRS_MASK << 12)
#define RTC_TIME_DOM_MASK					(RTC_DOM_MASK << 17)
#define RTC_TIME_MON_MASK					(RTC_MON_MASK << 22)
#define RTC_TIME_YR_MASK					(BIT31 | BIT30 | BIT29 | BIT28 | BIT 27 | BIT26)

/*
 * RTC_ACTRL_REG bit definitions
 */
#define RTC_ASEC_EN							BIT0
#define RTC_AMIN_EN							BIT1
#define RTC_AHRS_EN							BIT2
#define RTC_ADOM_EN							BIT3
#define RTC_AMON_EN							BIT4
#define RTC_AYR_EN							BIT5

/*
 * RTC_CTRL_REG bit definitions
 */
#define RTC_ALM_EN							BIT0
#define RTC_ALM_STA							BIT1



#endif

