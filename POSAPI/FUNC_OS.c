//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : MAX32550 (32-bit Platform)				    **
//**  PRODUCT  : AS330_mPOS						    **
//**                                                                        **
//**  FILE     : FUNC_OS.C						    **
//**  MODULE   :							    **
//**									    **
//**  FUNCTION : OS::xxx (OS function call)				    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2019/01/15                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2019 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
//#include "bsp_types.h"
//#include "bsp_wdt.h"
//#include "za9_pmu.h"
#include "DEV_MSR.h"

#include "POSAPI.h"


extern	ULONG	OS_KBD_Status( UINT32 *ScanCode );		// DEV_KBD.c
extern	UCHAR	Check_Present_SAM(UCHAR sam_id);		// DEF_IFM.c
extern	void	OS_MSR_Status( UINT8 action, UINT8 *dbuf );	// DEV_MSR.c

extern	ULONG	os_SysTimerFreeCnt;	// bsp_timer.c

ULONG	os_KbdEventFlag = 0;
ULONG	os_ScEventFlag = 0;
ULONG	os_MsrEventFlag = 0;


// ---------------------------------------------------------------------------
// FUNCTION: Get the value of system global variable "os_SysTimerFreeCnt".
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : value.
// ---------------------------------------------------------------------------
ULONG	OS_GET_SysTimerFreeCnt( void )
{
	return( os_SysTimerFreeCnt );
}

// ---------------------------------------------------------------------------
// FUNCTION: Set the value of system global variable "os_SysTimerFreeCnt".
// INPUT   : value to be set.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_SET_SysTimerFreeCnt( ULONG value )
{
	os_SysTimerFreeCnt = value;
}

// ---------------------------------------------------------------------------
// FUNCTION: Get the value of system global variable "os_KbdEventFlag".
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : value.
// ---------------------------------------------------------------------------
ULONG	OS_GET_KbdEventFlag( void )
{
UCHAR	sc ;

	if( os_KbdEventFlag )
	  return( TRUE );
	  
	if( KBD_Status(&sc) )
	  {
	  os_KbdEventFlag = 1;
	  return( TRUE );
	  }
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: Set the value of system global variable "os_KbdEventFlag".
// INPUT   : value to be set.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_SET_KbdEventFlag( ULONG value )
{
	os_KbdEventFlag = value;
}

// ---------------------------------------------------------------------------
// FUNCTION: Get the value of system global variable "os_ScEventFlag".
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : value.
// ---------------------------------------------------------------------------
ULONG	OS_GET_ScEventFlag( void )
{
	if( os_ScEventFlag )
	  return( TRUE );

	if( Check_Present_SAM(1) )
	  {
	  os_ScEventFlag = 1;
	  return( TRUE );
	  }
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: Set the value of system global variable "os_ScEventFlag".
// INPUT   : value to be set.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_SET_ScEventFlag( ULONG value )
{
	os_ScEventFlag = value;
}

// ---------------------------------------------------------------------------
// FUNCTION: Get the value of system global variable "os_MsrEventFlag".
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : value.
// ---------------------------------------------------------------------------
ULONG	OS_GET_MsrEventFlag( void )
{
UCHAR	dbuf[8];


	if( os_MsrEventFlag )
	  return( TRUE );
	  
	OS_MSR_Status( 1, dbuf );
	if( dbuf[0] == TRK_STATUS_SWIPED )
	  {
	  os_MsrEventFlag = 1;
	  return( TRUE );
	  }
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: Set the value of system global variable "os_MsrEventFlag".
// INPUT   : value to be set.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_SET_MsrEventFlag( ULONG value )
{
	os_MsrEventFlag = value;
}





#if	0
// ---------------------------------------------------------------------------
// FUNCTION: To setup WDT resources,
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : NULLPTR		-- failed
//	     WDT pointer	-- OK
// ---------------------------------------------------------------------------
BSP_WDT	*OS_WDT_Acquire( void )
{
BSP_WDT	*pWdt;


	pWdt = &BspWdt;

	/*
	 * The timer has been acquired.
	 */
	pWdt->pIsrFunc = NULLPTR;
	pWdt->pInt = BSP_INT_Acquire( INTNUM_WDT, NULLPTR );
	if( pWdt->pInt == NULLPTR )
	  {
	  BSP_WDT_Release( pWdt );
	  pWdt = NULLPTR;
	  }

	return( pWdt );
}

// ---------------------------------------------------------------------------
// FUNCTION: To enable system WDT (Watch Dog Timer).
//	     Default Settings (in BSP_WDT_Init())
//		Interrupt Period = 0.745 seconds
//		Reset     Period = 1.49  seconds
//
// INPUT   : pWdt - reference to BSP_WDT structure.
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
UINT32	OS_WDT_Start( BSP_WDT *pWdt )
{
UINT32	Div;


	Div = 5;	// RST_PERIOD = 2^26 / 90M = 0.75s 
			// INT_PERIOD = 2^25 / 90M = 0.37s
	pWdt->Control  = WDT_RST_EN
		       | WDT_INT_EN
		       | WDT_EN
		       | (Div << 4)
		       | (Div + 1);

	WdtAvailable = FALSE;
	if( BSP_WDT_Start( pWdt ) == BSP_SUCCESS )
	  return( TRUE );
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: To clear WDT interrupt.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_WDT_ClearInterrupt( void )
{
	/*
	 * Clear the WDT interrupt
	 */
	BSP_WR32( WDT_RR_REG, WDT_RR_FIRST );
	BSP_WR32( WDT_RR_REG, WDT_RR_SECOND );
}

// ---------------------------------------------------------------------------
// FUNCTION: Read MCU serial number.
// INPUT   : bpSNLen	- max length of MCU serail number read.
// OUTPUT  : bpMcuSN	- MCU serial number.
//	     bpSNLen	- actual length of MCU serial number.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
UINT32	OS_MCU_ReadSN( UINT8 *bpMcuSN, UINT8 *bpSNLen )
{
UINT32	result = FALSE;
UINT32	data;


	if( *bpSNLen >= 4 )
	  {
	  data = BSP_RD32(PMU_ID0_REG);
	  *(bpMcuSN+0) =  data & 0x000000FF;
	  *(bpMcuSN+1) = (data & 0x0000FF00) >> 8;
	  *(bpMcuSN+2) = (data & 0x00FF0000) >> 16;
	  *(bpMcuSN+3) = (data & 0xFF000000) >> 24;
	  
	  *bpSNLen = 4;
	  result = TRUE;
	  }
	  
	return( result );
}
#endif
