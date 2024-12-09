//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : 							                                **
//**  PRODUCT  : AS350 Plus						                            **
//**                                                                        **
//**  FILE     : API_TMR.C                                                  **
//**  MODULE   : api_tim_open()				                    **
//**		 api_tim_close()					    **
//**		 api_tim_gettick()					    **
//**		 api_tim_clock()					    **
//**									    **
//**  FUNCTION : API::TIM (Timer Module)				                    **
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
#include "POSAPI.h"
#include "OS_PROCS.h"
//#include "DEV_TMR.h"

extern	void OS_TaskTimer( OS_PST *pPST );
extern	void OS_TaskTimer_EX( OS_PST *pPST );
extern	volatile	UINT32	fPS_Exec;

// ---------------------------------------------------------------------------
// FUNCTION: Enable kernel TIMER1(0), period: 10ms
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TURE  -- OK
//           FALSE -- Failed
// ---------------------------------------------------------------------------
//void OS_TaskTimer( OS_PST *pPST )
//{
//UINT16	iCnt;
//	
//	if( pPST->RFU_0 == 0 )	// psTIM_UNIT=0?
//	  {
//	  pPST->RFU_0 = pPST->RFU_1;  // retrieve unit
//	  
//	  iCnt = (pPST->RFU_2) + (pPST->RFU_3)*256;  // counter++
//	  iCnt++;
//	  pPST->RFU_2 = iCnt & 0x00FF;
//	  pPST->RFU_3 = (iCnt & 0xFF00) >> 4;
//	  }
//	else
//	  pPST->RFU_0 -= 1;  // unit--
//}

// ---------------------------------------------------------------------------
// FUNCTION: To enable the timer device.
// INPUT   : unit
//           Time tick resolution unit, incremented in step of this value.
//	     One unit is 10ms, i.e., time tick is advanced by 10ms if unit=1.
// OUTPUT  : none.
// RETURN  : DeviceHandleNumber
//           apiOutOfService
// ---------------------------------------------------------------------------
UCHAR	api_tim_open( UCHAR unit )
{
OS_PST	pPST;
UCHAR	dhn;

	if( unit != 0 )
	  unit--;

	pPST.Xtime = 0xFFFF;
	pPST.pIsrFunc = (void *)OS_TaskTimer;
	pPST.Signal = SIG_IGNORE;
	pPST.RFU_0 = unit;	// psTIM_UNIT
	pPST.RFU_1 = unit;	// psTIM_UNIT_BAK
	pPST.RFU_2 = 0;	// clear up-counter, psTIM_CNT_L
	pPST.RFU_3 = 0;	//		   , psTIM_CNT_H
	pPST.RFU_4 = 0;
	pPST.RFU_5 = 0;
	
	if( PS_ForkProcess( psDEV_TIM, (OS_PST *)&pPST, (UINT8 *)&dhn ) == TRUE )
	  return( dhn );
	else
	  return( apiOutOfService );

}

// ---------------------------------------------------------------------------
// FUNCTION: To enable the timer device.
// INPUT   : unit
//           Time tick resolution unit, incremented in step of this value.
//	     One unit is 10ms, i.e., time tick is advanced by 10ms if unit=1.
//	     mode	- TIME_MODE_INC (0), up to 0xFFFFFFFF
//			  TIME_MODE_DEC (1), down to 0
//	     ivalue	- initial value.
// OUTPUT  : none.
// RETURN  : DeviceHandleNumber
//           apiOutOfService
// ---------------------------------------------------------------------------
UCHAR	api_tim_open2( UCHAR unit, UCHAR mode, ULONG ivalue )
{
OS_PST	pPST;
UCHAR	dhn;


	if( mode >= 2 )
	  return( apiOutOfService );
	  
	if( unit != 0 )
	  unit--;

	pPST.Xtime = 0xFFFF;
	pPST.pIsrFunc = (void *)OS_TaskTimer_EX;
	pPST.Signal = SIG_IGNORE;
	pPST.RFU_0 = unit;	// psTIM_UNIT
	pPST.RFU_1 = unit;	// psTIM_UNIT_BAK
	pPST.RFU_2 =  ivalue & 0x000000FF;		// reset counter, psTIM_CNT_0
	pPST.RFU_3 = (ivalue & 0x0000FF00) >> 8;	//	        , psTIM_CNT_1
	pPST.RFU_4 = (ivalue & 0x00FF0000) >>16;	// 		, psTIM_CNT_2
	pPST.RFU_5 = (ivalue & 0xFF000000) >>24;	//		, psTIM_CNT_3
	pPST.RFU_6 = mode;	//
	
	if( PS_ForkProcess( psDEV_TIM2, (OS_PST *)&pPST, (UINT8 *)&dhn ) == TRUE )
	  return( dhn );
	else
	  return( apiOutOfService );

}

// ---------------------------------------------------------------------------
// FUNCTION: To enable the timer device.
// INPUT   : unit
//           Time tick resolution unit, incremented in step of this value.
//	     One unit is 10ms, i.e., time tick is advanced by 10ms if unit=1.
//	     mode	- TIME_MODE_INC (0), up to 0xFFFFFFFF
//			  TIME_MODE_DEC (1), down to 0
//	     ivalue	- initial value.
// OUTPUT  : none.
// RETURN  : DeviceHandleNumber
//           apiOutOfService
// ---------------------------------------------------------------------------
UCHAR api_tim_open3( UCHAR unit, UCHAR mode, ULONG ivalue )
{
OS_PST	pPST;
UCHAR	dhn;


	if( mode >= 2 )
	  return( apiOutOfService );
	  
	if( unit != 0 )
	  unit--;

	pPST.Xtime = 0xFFFF;
	pPST.pIsrFunc = (void *)OS_TaskTimer_EX;
	pPST.Signal = SIG_IGNORE;
	pPST.RFU_0 = unit;	// psTIM_UNIT
	pPST.RFU_1 = unit;	// psTIM_UNIT_BAK
	pPST.RFU_2 =  ivalue & 0x000000FF;		// reset counter, psTIM_CNT_0
	pPST.RFU_3 = (ivalue & 0x0000FF00) >> 8;	//	        , psTIM_CNT_1
	pPST.RFU_4 = (ivalue & 0x00FF0000) >>16;	// 		, psTIM_CNT_2
	pPST.RFU_5 = (ivalue & 0xFF000000) >>24;	//		, psTIM_CNT_3
	pPST.RFU_6 = mode;	//
	
	if( PS_ForkProcess( psDEV_TIM3, (OS_PST *)&pPST, (UINT8 *)&dhn ) == TRUE )
	  return( dhn );
	else
	  return( apiOutOfService );

}

// ---------------------------------------------------------------------------
// FUNCTION: To disable the system timer service.
// INPUT   : dhn
//           The specified device handle number.
//	     0x00 = to close all opened tasks.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_tim_close( UCHAR dhn )
{
	fPS_Exec = TRUE;
	
	if( PS_NormalCloseDevice( dhn, psDEV_TIM ) == TRUE )
	  return( apiOK );
	else
	  return( apiFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: To disable the system timer service.
// INPUT   : dhn
//           The specified device handle number.
//	     0x00 = to close all opened tasks.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_tim_close2( UCHAR dhn )
{
	return( api_tim_close( dhn ) );
}

// ---------------------------------------------------------------------------
// FUNCTION: To disable the system timer service.
// INPUT   : dhn
//           The specified device handle number.
//	     0x00 = to close all opened tasks.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_tim_close3( UCHAR dhn )
{
	return( api_tim_close( dhn ) );
}

// ---------------------------------------------------------------------------
// FUNCTION: To get the contents of tick counter from the specified timer.
// INPUT   : dhn
//           The specified device handle number.
// OUTPUT  : dbuf
//	     UINT  counter  // current 16-bit up-counter of the specified timer.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_tim_gettick( UCHAR dhn, UCHAR *dbuf )
{
OS_PST	*pPST;
UCHAR	temp[2];

	pPST = PS_SeekProcess( dhn );
	
	if( pPST != NULLPTR )
	  {
#if	1
	  *dbuf++ = pPST->RFU_2;  // psTIM_CNT_L
	  *dbuf = pPST->RFU_3;    // psTIM_CNT_H
#else
	  temp[0] = pPST->RFU_2;  // psTIM_CNT_L
	  temp[1] = pPST->RFU_3;    // psTIM_CNT_H
	  memmove( dbuf, temp, 2 );
	  
	  printf("\ntick_l=%x", temp[0]);
	  printf("  tick_h=%x\n", temp[1]);
#endif
	  return( apiOK );
	  }
	else
	  return( apiFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: To get the contents of tick counter from the specified timer.
// INPUT   : dhn
//           The specified device handle number.
// OUTPUT  : dbuf
//	     ULONG  counter  // current 32-bit up-counter of the specified timer.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_tim_gettick2( UCHAR dhn, UCHAR *dbuf )
{
OS_PST	*pPST;

	pPST = PS_SeekProcess( dhn );
	
	if( pPST != NULLPTR )
	  {
	  *dbuf++ = pPST->RFU_2; 	// psTIM_CNT_0
	  *dbuf++ = pPST->RFU_3;	// psTIM_CNT_1
	  *dbuf++ = pPST->RFU_4;	// psTIM_CNT_2
	  *dbuf   = pPST->RFU_5;	// psTIM_CNT_3
	  
	  return( apiOK );
	  }
	else
	  return( apiFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: To get the contents of tick counter from the specified timer.
// INPUT   : dhn
//           The specified device handle number.
// OUTPUT  : dbuf
//	     ULONG  counter  // current 32-bit up-counter of the specified timer.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_tim_gettick3( UCHAR dhn, UCHAR *dbuf )
{
	return( api_tim_gettick2( dhn, dbuf ) );
}

// ---------------------------------------------------------------------------
// FUNCTION: To enable a timer clock process for processing the timeout task
//	     of the specified event.
// INPUT   : unit (RFU)
//                Time tick resolution unit, incremented in step of this value.
//	          One unit is 10ms, i.e., time tick is advanced by 10ms if unit=1.
//           pTask
//                the process to be executed at every 10ms time tick.
//	     tick1 (down-counter in 10ms unit)
//	          eg. tob -- inter-byte timeout for receiving character string.
//           tick2 (down-counter in 10ms unit)
//	          eg. tor -- timeout for waiting response.
// OUTPUT  : none.
// RETURN  : DeviceHandleNumber
//           apiOutOfService
// NOTE    : 1. The thread shall be closed by caller process - pTask.
//           2. This function is reserved for internal use only.
// ---------------------------------------------------------------------------
#if	0
UCHAR api_tim_clock( UINT tick1, UINT tick2, void *pTask )
{
static	OS_PST	*pPST;
UCHAR	dhn;

//	if( unit != 0 )
//	  unit--;

	pPST->Xtime = 0xFFFF;
	pPST->pIsrFunc = pTask;			// task
	pPST->Signal = SIG_IGNORE;		// signal
	pPST->RFU_0 = tick1 & 0x00FF;		// eg. TOB_L
	pPST->RFU_1 = (tick1 & 0xFF00) >> 8;	//     TOB_H
	pPST->RFU_2 = tick2 & 0x00FF;		// eg. TOR_L
	pPST->RFU_3 = (tick2 & 0xFF00) >> 8;	//     TOR_H
	pPST->RFU_4 = 0;			// DHN
	pPST->RFU_5 = 0;
	
	if( PS_ForkProcess( psDEV_TIM, pPST, (UINT8 *)&dhn ) == TRUE )
	  {
	  pPST->RFU_4 = dhn;	// backup DHN for self-closed function
	  return( dhn );
	  }
	else
	  return( apiOutOfService );

}
#endif
