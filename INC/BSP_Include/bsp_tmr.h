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
 
2008/10/15		1.�ק�BSP_TMR_Acquire() �s�W�ﶵBSP_SMALL_TIMER BSP_LARGE_TIMER
 				2.add function : TMR_getAvailStatus(), TMR_getCount()
 
 
 */
#ifndef _BSP_TMR_H_
#define _BSP_TMR_H_

#include "bsp_types.h"
#include "za9_tmr.h"
#include "bsp_int.h"
#include "bsp_utils.h"
#include "bsp_gpio.h"



#define BSP_MAX_TIMERS		15
#define BSP_ANY_TIMER		(BSP_MAX_TIMERS)
#define BSP_SMALL_TIMER			10
#define BSP_LARGE_TIMER			11
			


typedef struct BSP_TIMER_S
{
	BSP_SEM						Avail;			// Avail = 1 if not in use
	BSP_SEM						IsIdle;			// IsIdle = 0 once timer started
	UINT32						Timer;			// Identifies HW Timer number (0 - 8)
	UINT32						Base;				// Base adress of timer HW registers
	UINT32						Gpio;				// Mask for GPIO port control
	UINT32						IntChannel;		// Interrupt Channel number
	UINT32						Count;			// Initial/ current counter
	UINT32						Compare;			// Reload Threshold
	UINT32						PwmTrigger;		// PWM Threshold
	UINT32						Control;			// Timer control
	void						(*	pIsrFunc)		// Upper-layer interrupt callback
	(													// callback parameters		
		struct BSP_TIMER_S * pTimer			// Timer Port reference
	);		
	BSP_INT					 * pInt;				// BSP structure for interrupt control
	BSP_GPIO					 * pGpio;			// BSP structure for GPIO control
} BSP_TIMER;


typedef void (* TMR_ISR_FUNC )( BSP_TIMER * pTimer ); 


/*
 * GPIO pins available for Timer function:
 * After reset, the PWM output/ Count input pins default to GPIO operation.
 * If those modes of operatoin are not required, then the user must clear the
 * corresponding bits of the Gpio member of the Timer structure.
 */
#define TMR0_PWM_CLK			0x08000000
#define TMR1_PWM_CLK			0x10000000
#define TMR2_PWM_CLK			0x20000000
#define TMR3_PWM_CLK			0x40000000


/*
 * Function Prototypes
 */
extern BSP_STATUS  BSP_TMR_Init( void );
extern BSP_TIMER * BSP_TMR_Acquire( UINT32 TimerNum, TMR_ISR_FUNC pLisr );
extern BSP_STATUS  BSP_TMR_Release( BSP_TIMER * pTimer );
extern BSP_STATUS  BSP_TMR_Start( BSP_TIMER * pTimer );
extern BSP_STATUS  BSP_TMR_Stop( BSP_TIMER * pTimer );
extern inline unsigned short TMR_getAvailStatus();
extern inline unsigned long TMR_getCount(BSP_TIMER *pTmr);

#endif
