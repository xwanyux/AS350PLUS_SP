//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : 							    **
//**  PRODUCT  : AS350 Plus						    **
//**                                                                        **
//**  FILE     : DEV_TMR.C                                                  **
//**  MODULE   : OS_Timer1Int()						    **
//**             OS_EnableTimer1()			                    **
//**		 OS_TaskTimer()						    **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2022/11/30                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2022 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
//#include "bsp_gpio.h"
#include "bsp_tmr.h"
#include "OS_PROCS.h"
//#include "DEV_TMR.h"
//#include "DEV_AUX.h"
//#include "DEV_KBD.h"
//#include "DEV_PMU.h"

#if	0
extern	void	OS_PED_Task( void );
extern	void	OS_KBD_BL_Task( void );
extern	void	OS_PMU_SwitchScan( void );
extern	void	OS_PMU_TimeTick_Task( void );
extern	void	OS_LCD_BL_Task( void );
extern	void	OS_PMU_BatteryCharge( void );
extern	void	OS_DHCP_Lease_Time_Task( void );

extern	UINT32	DIAG_IsES3( void );

volatile	UINT32	os_SysTimerFreeCnt = 0;
volatile	UINT32	os_SysTimerFreeCnt_IFM = 0;	// 2017-09-27
volatile	UINT32	os_SysTimerFreeCnt_LTE = 0;	// 2020-10-30
#endif


// ---------------------------------------------------------------------------
// FUNCTION: Timer1(0) interrupt service routine.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	0
void OS_Timer1Int( void )
{
	
	os_SysTimerFreeCnt++;		// 2013-07-25
	os_SysTimerFreeCnt_IFM++;	// 2017-09-27
	os_SysTimerFreeCnt_LTE++;	// 2020-10-30
	
	PS_ExecProcess();
	
	OS_KBD_Scan();			// normal keypad scan
//	OS_KBD_SecureScan();		// secure keypad scan
	
	OS_AUX_FlowControl();		// AUX flow control
	
	OS_PED_Task();			// PED task
	
	OS_KBD_BL_Task();		// KBD backlight

	if( DIAG_IsESX() && !DIAG_IsES3() )		// 2021-05-24, skip if S3 or ES34
	  OS_PMU_SwitchScan();		// scan AGM power switch
	
	OS_PMU_TimeTick_Task();		//
	
	OS_LCD_BL_Task();
	
	OS_PMU_BatteryCharge();
	
	OS_DHCP_Lease_Time_Task();	// LAN.DHCP
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Enable kernel TIMER1(0), period: 10ms
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TURE  -- OK
//           FALSE -- Failed
// ---------------------------------------------------------------------------
#if	0
BSP_BOOL OS_EnableTimer1( void )
{
BSP_TIMER	* pTmr;

//	pTmr = BSP_TMR_Acquire( 1, (void *)OS_Timer1Int );	// register timer 1
	pTmr = BSP_TMR_Acquire( BSP_ANY_TIMER, (void *)OS_Timer1Int );	// register timer 1
	
	if( pTmr == NULLPTR )
	  return( FALSE );
	
	pTmr->Count    = 1;
	pTmr->Compare  = BSP_GetClock( TRUE, 2048 );
	pTmr->Compare *= 10; // 10ms;
	pTmr->Compare /= 1000;
	pTmr->Control  = TMR_PRES_DIV_2048 | TMR_TEN | TMR_MODE_RELOAD;

	if( BSP_TMR_Start( pTmr ) == BSP_SUCCESS )	// start timer
	  return( TRUE );
	else
	  return( FALSE );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Kernel task for device TIMER.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void OS_TaskTimer( OS_PST *pPST )
{
UINT16	iCnt;
	
	if( pPST->RFU_0 == 0 )	// psTIM_UNIT=0?
	  {
	  pPST->RFU_0 = pPST->RFU_1;  // retrieve unit
	  
	  iCnt = (pPST->RFU_2) + (pPST->RFU_3)*256;  // counter++
	  iCnt++;
	  pPST->RFU_2 = iCnt & 0x00FF;
	  pPST->RFU_3 = (iCnt & 0xFF00) >> 8;
	  }
	else
	  pPST->RFU_0 -= 1;  // unit--
}

// ---------------------------------------------------------------------------
// FUNCTION: Kernel task for device TIMER.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void OS_TaskTimer_EX( OS_PST *pPST )
{
UINT32	iCnt;

	
	if( pPST->RFU_0 == 0 )	// psTIM_UNIT=0?
	  {
	  pPST->RFU_0 = pPST->RFU_1;  // retrieve unit
	  
	  iCnt = (pPST->RFU_2) + (pPST->RFU_3)*0x100 + (pPST->RFU_4)*0x10000 + (pPST->RFU_5)*0x1000000;	// current counter
	  
	  if( pPST->RFU_6 == 0 )	// MODE = up count?
	    {
	    if( iCnt != 0xFFFFFFFF )
	      {
	      iCnt++;
	      pPST->RFU_2 = iCnt & 0x000000FF;
	      pPST->RFU_3 = (iCnt & 0x0000FF00) >> 8;
	      pPST->RFU_4 = (iCnt & 0x00FF0000) >> 16;
	      pPST->RFU_5 = (iCnt & 0xFF000000) >> 24;
	      }
	    }
	  else	// down count
	    {
	    if( iCnt != 0 )
	      {
	      iCnt--;
	      pPST->RFU_2 = iCnt & 0x000000FF;
	      pPST->RFU_3 = (iCnt & 0x0000FF00) >> 8;
	      pPST->RFU_4 = (iCnt & 0x00FF0000) >> 16;
	      pPST->RFU_5 = (iCnt & 0xFF000000) >> 24;
	      }
	    }
	  }
	else
	  pPST->RFU_0 -= 1;  // unit--
}
