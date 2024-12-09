//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : 							    **
//**  PRODUCT  : AS350 Plus						    **
//**                                                                        **
//**  FILE     : API_BUZ.C                                                  **
//**  MODULE   : api_buz_open()				                    **
//**		 api_buz_close()					    **
//**		 api_buz_sound()					    **
//**									    **
//**  FUNCTION : API::BUZ (Buzzer Module)				    **
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
//#include "DEV_BUZ.h"


volatile	UINT8	os_DHN_BUZ_5A = 0;
volatile	UINT32	os_BUZ_5A_START = FALSE;

extern	void OS_TaskBuzzer( OS_PST *pPST );

UINT32	os_BUZ_MODE = 1;		// 0x00 = default PCI mode, 0x01 = user-defined mode


// ---------------------------------------------------------------------------
// FUNCTION: To enable the buzzer device.
// INPUT   : sbuf
//	     UCHAR  cycle ;    // repetitive cycles for buzzer ON/OFF period.
//	     UCHAR  buz_on ;   // time for buzzer ON period in 10ms unit.
//	     UCHAR  buz_off ;  // time for buzzer OFF period in 10ms unit.
//	     UINT   buz_freq;  // frequency in Hertz (option)
//	     UCHAR  auto_close;	// auto close after sound = 0x5A
// OUTPUT  : none.
// RETURN  : DeviceHandleNumber
//           apiOutOfService
// ---------------------------------------------------------------------------
UCHAR	api_buz_open( UCHAR *sbuf )
{
OS_PST	pPST;
UCHAR	dhn;


	pPST.Xtime = 0x0000;
	pPST.pIsrFunc = (void *)OS_TaskBuzzer;
	pPST.Signal = SIG_IGNORE;
	pPST.RFU_0 = *sbuf;	// psBUZ_CYCLE
	pPST.RFU_3 = *sbuf++;  // psBUZ_CYCLE_BAK
	pPST.RFU_1 = *sbuf;	// psBUZ_ON
	pPST.RFU_4 = *sbuf++;	// psBUZ_ON_BAK
	pPST.RFU_2 = *sbuf;	// psBUZ_OFF
	pPST.RFU_5 = *sbuf++;	// psBUZ_OFF_BAK

	pPST.RFU_6 = *sbuf++;	// psBUZ_FREQ_L
	pPST.RFU_7 = *sbuf++;	// psBUZ_FREQ_H
	
	pPST.RFU_6 = 0;	// 2016-07-29, set to constant for backward compatible
	pPST.RFU_7 = 0;	//
	
	if( (pPST.RFU_6 == 0) && (pPST.RFU_7 == 0) )
	  {
	  pPST.RFU_6 = 0xC4;	// set default = 2500 Hz
	  pPST.RFU_7 = 0x09;	//
	  }
	
	pPST.RFU_8 = 0;	// psBUZ_START_FLAG (0=not yet start)
	
	if( *sbuf == 0x5A )
	  pPST.RFU_9 = 0x5A;	// psBUZ_AUTO_CLOSE
	else
	  pPST.RFU_9 = 0;
	
	if( PS_ForkProcess( psDEV_BUZ, (OS_PST *)&pPST, (UINT8 *)&dhn ) == TRUE )
	  {
	  if( pPST.RFU_9 == 0x5A )	// 2020-03-03
	    {
	    os_DHN_BUZ_5A = dhn;
	    os_BUZ_5A_START = FALSE;	// 2020-03-04
	    }
	    
	  return( dhn );
	  }
	else
	  return( apiOutOfService );
}

// ---------------------------------------------------------------------------
// FUNCTION: To disable buzzer device.
// INPUT   : dhn
//           The specified device handle number.
//	     0x00 = to close all opened tasks.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_buz_close( UCHAR dhn )
{
//	if( DIAG_IsESX() )
//	  OS_BUZ_TurnOff();
	
	if( PS_NormalCloseDevice( dhn, psDEV_BUZ ) == TRUE )
	  {
//	  if( !DIAG_IsESX() )
	    OS_BUZ_TurnOff();	// turn off buzzer io
	  OS_BUZ_TurnFlag( FALSE );
	  return( apiOK );
	  }
	else
	  return( apiFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate a pattern of beep sound.
// INPUT   : dhn
//           The specified device handle number.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_buz_sound( UCHAR dhn )
{
	if( (dhn != os_DHN_BUZ_5A) && os_BUZ_5A_START )
	  while( os_DHN_BUZ_5A != 0 );
	
	if( dhn == os_DHN_BUZ_5A )
	  os_BUZ_5A_START = TRUE;
	else
	  {
	  if( os_DHN_BUZ_5A && (!os_BUZ_5A_START) )
	    os_DHN_BUZ_5A = 0;	// invalid input parameter for other api_buz_open(), eg. only 3 parameters but the 5'th is possibly 0x5A
	  }
	  
	if( PS_SignalProcess( dhn, SIG_INT, 0xFFFF, NULLPTR ) == TRUE )
	  {
	  OS_BUZ_ResetCnt();
	  
//	  if( !os_BUZ_MODE )
//	    OS_BUZ_TurnFlag( TRUE );
	  
	  return( apiOK );
	  }
	else
	  return( apiFailed );
}
