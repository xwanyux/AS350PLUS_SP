//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : i.MX6UL				                                    **
//**  PRODUCT  : AS350-X6						                            **
//**                                                                        **
//**  FILE     : POST.C		                                                **
//**  MODULE   : 							                                **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2023/08/02                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2023 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include <stdio.h>

#include "POSAPI.h"


// ---------------------------------------------------------------------------
void	POST_AutoSetMAC( void )
{
UINT8	mac_b[6];	// 12 34 56 78 9A BC
UINT8	mac_s[17+1];	// 12:23:56:78:9A:BC
UINT8	cmd_if_down[]	= {"ifconfig eth0 down"};
UINT8	cmd_if_up[]	= {"ifconfig eth0 up"};
UINT8	cmd_if_hw[]	= {"ifconfig eth0 hw ether "};
UINT8	buffer[128];
FILE	*file_cmmd;


//	api_sys_genMAC( mac_b, mac_s );		// using SYMLINK OUI (6C 15 24 Dx xx xx)
	api_sys_genMAC_PSEUDO( mac_b, mac_s );	// using PSEUDO  OUI (00 F0 FF xx xx xx)
	mac_s[17] = 0;

	file_cmmd = popen( cmd_if_down, "r" );
	pclose(file_cmmd);
	
	memset( buffer, 0x00, sizeof(buffer) );
	sprintf( buffer, "%s%s", cmd_if_hw, mac_s );
	file_cmmd = popen( buffer, "r" );
	pclose(file_cmmd);
	
	file_cmmd = popen( cmd_if_up, "r" );
	pclose(file_cmmd);
}

#if 0
/* Scheduler includes. */
//#define configUSE_MUTEXES                       1
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/** Global includes */
#include <config.h>
#include <errors.h>
/** Other includes */
#include <mml_gcr.h>
#include <mml_gpio.h>
#include <mml_uart.h>
/** Local includes */
#include <private.h>
#include <printf_lite.h>

#include "bsp_mem.h"

#include "PEDKconfig.h"

#include "POSAPI.h"
#include "OS_MSG.h"
#include "OS_LIB.h"
#include "OS_FONT.h"
#include "OS_PROCS.h"
#include "OS_SECM.h"
#include "OS_SECS.h"

#include "DEV_LCD.h"
#include "DEV_LED.h"
#include "DEV_KBD.h"
#include "DEV_TMR.h"
#include "DEV_BUZ.h"
//#include "DEV_PMU.h"
//#include "DEV_BT.h"
//#include "DEV_BCR.h"
//#include "DEV_USB.h"
#include "DEV_RTC.h"

#include "LCDTFTAPI.h"

#include "MPU.h"


extern	void	POST_DiagnosticTest( void );
extern	void	APP_main( void );


UINT8	post_dhn_buz1 = 0;
UINT8	post_dhn_buz2 = 0;



// ---------------------------------------------------------------------------
//#ifdef	_KBD_ENABLED_
static void vDiagTestTask( void *pvParameters )
{
	for(;;)
	{
		POST_DiagnosticTest();
	}
}
//#endif

// ---------------------------------------------------------------------------
static void vAppMainTask( void *pvParameters )
{    
	for(;;)
	{
		APP_main();
	}
}

// ---------------------------------------------------------------------------
//	FW:   AS330-PED-01-FW-V1.0
//	AP:   AS330-PED-01-AP-V1.0
//	SRED: AS330-SRED-V1.0
// ---------------------------------------------------------------------------
void	POST_ShowPEDVersions( void )
{
#ifdef	_LCD_ENABLED_
UINT8	const msg_FWV[] = {" FW  :AS330-PED-01-FW-V1.0"};
UINT8	const msg_APV[] = {" AP  :AS330-PED-01-AP-V1.0"};
UINT8	const msg_SRV[] = {" SRED:AS330-SRED-V1.0     "};


	LIB_LCD_ClearRow( 2, 1, FONT0 );
	
	LIB_LCD_Puts( 2, 0, FONT1, sizeof(msg_FWV)-1, (UINT8 *)msg_FWV );
	LIB_LCD_Puts( 3, 0, FONT1, sizeof(msg_APV)-1, (UINT8 *)msg_APV );
	
	if( OS_SECM_VerifySredStatus() == TRUE )
	  LIB_LCD_Puts( 4, 0, FONT1, sizeof(msg_SRV)-1, (UINT8 *)msg_SRV );
	
	LIB_WaitTime(300);
#else
	return;	// TBD
#endif
}

// ---------------------------------------------------------------------------
// FUNCTION: main procedure of POST.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
int	POST_main( void )
{
int	err;
UINT8	sbuf[8];


	BSP_MEM_Init();			// init BSP heap
	
//	OS_USB_Init();			// init USB device
	
//	OS_PMU_Init();			// init PMU

#ifdef	_LCD_ENABLED_
	OS_FONT_Init( LCD_DID_TFT );	// new TFT LCD font
		
	OS_LCD_Init();			// init LCD
#endif

#ifdef	_OUTPUT_BOOT_MSG_
	LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_INIT), (UINT8 *)os_msg_INIT );	// INIT...
#endif
	OS_LED_Init();			// init LED

	OS_KBD_Init();			// init secure KBD

#ifdef	_OUTPUT_BOOT_MSG_
	LIB_LCD_Puts( 0, sizeof(os_msg_INIT), FONT0, sizeof("BLE")-1, (UINT8 *)"BLE" );
#endif
//	OS_BT_Init();			// init BT module
	
	PS_ClearAllProcess();		// reset system process
	
	OS_EnableTimer1();		// activate system timer

	if( OS_BUZ_Init() == TRUE )	// init BUZZER
	  {
	  memset( sbuf, 0x00, sizeof(sbuf) );
	  
	  sbuf[0] = 1;
	  sbuf[1] = 5;
	  sbuf[2] = 5;
	  post_dhn_buz1 = api_buz_open( sbuf );	// sound for OK
	  
	  sbuf[0] = 2;
	  sbuf[1] = 10;
	  sbuf[2] = 10;
	  post_dhn_buz2 = api_buz_open( sbuf );	// sound for ERROR
	  }

#ifdef	_OUTPUT_BOOT_MSG_
	LIB_LCD_Puts( 0, sizeof(os_msg_INIT), FONT0, sizeof("BCR")-1, (UINT8 *)"BCR" );
#endif
//	OS_BCR_Init();			// init 2D BarCode Reader

#ifdef	_OUTPUT_BOOT_MSG_
	LIB_LCD_Puts( 0, sizeof(os_msg_INIT), FONT0, sizeof("SEM")-1, (UINT8 *)"SEM" );
#endif
//	OS_SECM_Init();			// init NVSRAM

#ifdef	_OUTPUT_BOOT_MSG_
	LIB_LCD_Puts( 0, sizeof(os_msg_INIT), FONT0, sizeof("RTC")-1, (UINT8 *)"RTC" );
#endif
#ifdef	_SECS_ENABLED_
	OS_SEC_Setup();			// setup Security sub-system
	OS_SEC_CheckTamperStatus();
#endif

#ifdef	_OUTPUT_BOOT_MSG_
	LIB_LCD_Puts( 0, sizeof(os_msg_INIT), FONT0, sizeof("RTC")-1, (UINT8 *)"RTC" );
#endif
	OS_RTC_Init();			// init RTC
	
	OS_SECM_Init();			// init NVSRAM

#ifdef	_OUTPUT_BOOT_MSG_
	LIB_LCD_Puts( 0, sizeof(os_msg_INIT), FONT0, sizeof("UCL")-1, (UINT8 *)"UCL" );
#endif
	api_sys_initUCL();		// init UCL

#ifdef	_OUTPUT_BOOT_MSG_
	LIB_LCD_Puts( 0, sizeof(os_msg_INIT), FONT0, sizeof("OK ")-1, (UINT8 *)"OK " );
	BSP_Delay_n_ms(1000);
#endif

	PED_ResetPinPad();		// 2022-02-10, reset PINPAD

	// === FreeRTOS DEMO =============================
	// priority level: (max (configMAX_PRIORITIES - 1)
	// Low priority numbers denote low priority tasks.
	//
	//	0: IDLE task
	//	1: SYSTEM/APP task
	//	2: BLE task
	//	3: USB task
	//	4: (RFU)
	//	
	// ===============================================
#ifdef	_FreeRTOS_

//#ifdef	_KBD_ENABLED_
	if( LIB_GetKeyStatus() == apiReady )
	  {
//	  if( LIB_WaitKey() == 'x' )	// press CANCEL key at POST?
	  if( LIB_WaitKey() == 'z' )	// press FUNCTION key at POST?
	    {
	    // *** SYSTEM mode ***
#ifdef	_MPU_ENABLED_
	    err=xTaskCreate( vDiagTestTask, "SYS", 1024, NULL, 1 | portPRIVILEGE_BIT, NULL );	// stack size = 1024*4, priority=1 (privileged mode)
#else
	    err=xTaskCreate( vDiagTestTask, "SYS", 1024, NULL, 1, NULL );	// stack size = 1024*4, priority=1 ((user mode)
#endif
	    if(err!=pdPASS) while(1);

	    /* Start the scheduler. */
	    vTaskStartScheduler();
	    /* Will only get here if there was insufficient heap to start the scheduler. */
	    }
	  }
//#endif	// _KBD_ENABLED_

	// === Show Firmware, APP and SRED version number ===
#ifndef	_DEBUG_SECS_DISPLAY_
	POST_ShowPEDVersions();
#endif

	// *** APP mode ***
	api_buz_close( post_dhn_buz1 );
	api_buz_close( post_dhn_buz2 );
	post_dhn_buz1 = 0;
	post_dhn_buz2 = 0;
	    
	err=xTaskCreate( vAppMainTask, "APP", 1024*3, NULL, 1, NULL );	// stack size = (1024*3)*4, priority=1 (user mode)
	if(err!=pdPASS) while(1);

	/* Start the scheduler. */
	vTaskStartScheduler();
	/* Will only get here if there was insufficient heap to start the scheduler. */
#else
	TASK_DIAG_test();	// Test ONLY for non-RTOS mode
#endif
	// === End of FreeRTOS ===

	return( 0 );
}

#endif