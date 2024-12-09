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
#ifndef _BSP_RTC_H_
#define _BSP_RTC_H_

#include "bsp_types.h"
#include "bsp_utils.h"
#include "bsp_int.h"
#include "za9_rtc.h"


typedef struct					RTC_TIME_S
{
	UINT32						Sec;
	UINT32						Min;
	UINT32						Hrs;
	UINT32						DOM;
	UINT32						Mon;
	UINT32						Yr;
} RTC_TIME;

typedef struct					BSP_RTC_S
{
	UINT32						Ctrl;
	UINT32						AlarmCtrl;

	/*
	 * BSP-specific variables.
	 */
	void						(*	pIsrFunc)		// Upper-layer interrupt callback
	(													// callback parameters		
		struct BSP_RTC_S   * pRtc				// RTC reference
	);		
	BSP_INT					 * pInt;
} BSP_RTC;

typedef void (* RTC_ISR_FUNC )( BSP_RTC * pRtc ); 




/*
 * Function Prototypes
 */
extern BSP_STATUS BSP_RTC_Init( void );
extern BSP_RTC *  BSP_RTC_Acquire( RTC_ISR_FUNC pLisr );
extern BSP_STATUS BSP_RTC_Release( BSP_RTC * pRtc );

extern BSP_STATUS BSP_RTC_GetTime(     BSP_RTC * pRtc, RTC_TIME * pTime );
extern BSP_STATUS BSP_RTC_SetTime(     BSP_RTC * pRtc, RTC_TIME * pTime );
extern BSP_STATUS BSP_RTC_GetAlarm(    BSP_RTC * pRtc, RTC_TIME * pTime );
extern BSP_STATUS BSP_RTC_SetAlarm(    BSP_RTC * pRtc, RTC_TIME * pTime );
extern BSP_STATUS BSP_RTC_CancelAlarm( BSP_RTC * pRtc );



#endif
