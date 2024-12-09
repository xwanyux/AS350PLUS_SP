//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  :							    **
//**  PRODUCT  : AS330 		                                            **
//**                                                                        **
//**  FILE     : PEDKconfig.H						    **
//**  MODULE   : Define configuration of PEDK.				    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2018/08/19                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2018 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _PEDK_CONFIG_H_
#define _PEDK_CONFIG_H_

//----------------------------------------------------------------------------
#define	_FreeRTOS_			// defined when supporting FreeRTOS (POST.c)

#define	_SECS_ENABLED_			// enable security sub-system (POST.c)
#define	_DEBUG_SECS_			// debug only, report security sub-system status (OS_SECS.c)

//#define	_UNLOCK_			// unlock the security monitor

//#define	_DEBUG_SECS_DISPLAY_		// output tamper status onto display
//#define	_DEBUG_SECS_DISPLAY_DETAILS_	// output tamper status onto display, including setup process

//#define	_OUTPUT_BOOT_MSG_		// debug only, report self-test status (POST.c)
//#define	_TR31_DEBUG_			// debug only, (ANS_TR31_2010.c)
#define	_USE_UART_PORT_			// using UART instead of USB (OS_LIB.c)
//#define	_WDT_RESET_ENABLED_		// using WDT to reset system (OS_LIB.c)
//#define	_USE_IRAM_PARA_			// using MCU internal RAM as local stack (OS_PED1.c)
//#define	_BOOT_FROM_POST0_		// (NA) (OS_PED2.c)
//#define	_CAPK_IN_SRAM_			// keep CAPKs in external SRAM (OS_PEDK5.c)

//#define	_SAM_ENABLED_			// enable SAM slots (POSTFUNC.c)
//#define	_DEBUG_AES_			// show details for PED_GenEncPinBlock_ISO4()
#define	_OUTPUT_KMS_HINT_		// output KMS hints to COM port

//#define	_DEBUG_SECS_PERIODIC_OUTPUT_	// output tamper status periodically (for lab testing purpose)
//#define	_OUTPUT_TO_UART_		// output SEC details to UART
//#define	_OUTPUT_TO_LCD_			// output SEC details to LCD

//#define _DEBUG_SEMAPHORE_           // output debug message for controlling shared memory between SP and AP

//----------------------------------------------------------------------------
#endif
