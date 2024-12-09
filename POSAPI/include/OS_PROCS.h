//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L                                                       **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : OS_PROCS.H                                                 **
//**  MODULE   : Declaration of related Process Scheduler.                  **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2007/07/17                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2007 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _OS_PROCS_H_
#define _OS_PROCS_H_

//----------------------------------------------------------------------------
#include "bsp_types.h"

//----------------------------------------------------------------------------
#define MAX_PST_NO					16+16		// max no. of PST table
//#define PST_LEN					16		// length of a PST table
#define PST_CHKSUM_LEN					9		// size of chksum for PST
#define MAX_DID_NO					8		// max no. with the same device id in processing

#define PS_END						0x00		// end of process state
#define PS_RUNNING					0x01		// process in running
#define PS_SLEEPING					0x02		// process in sleeping
#define PS_KILLED					0xFF		// process was killed

#define SIG_IGNORE					0x00		// ignore the occurrence of the signal
#define SIG_EXIT					0x01		// quit process
#define SIG_TIMER					0x02		// used as timer
#define SIG_INT						0x03		// wake up process

typedef struct OS_PST_S
{
	UINT8			State;					// process state
	UINT8			PID;					// process id
	UINT16			Xtime;					// execution time
	void			(*pIsrFunc)(struct OS_PST_S *pPST);	// target callback function
	UINT8			Signal;					// process signal
	UINT8			LRC;					// XOR(State..Signal)
	UINT8			RFU_0;					// Reserved field 0
	UINT8			RFU_1;					//                1
	UINT8			RFU_2;					//                2
	UINT8			RFU_3;					//                3
	UINT8			RFU_4;					//                4
	UINT8			RFU_5;					//                5
	
	UINT8			RFU_6;			// 2016-03-16, for buz_freq
	UINT8			RFU_7;			//
	UINT8			RFU_8;			//
	UINT8			RFU_9;			//
} __attribute__((packed)) OS_PST;

#define PST_LEN		sizeof( OS_PST )

//typedef void (* OS_CALLBACK_FUNC )	(OS_PST *pPST);

//----------------------------------------------------------------------------
//	Device ID used in PST
//----------------------------------------------------------------------------
#define	psDEV_TIM					0x00		//
#define	psDEV_AUX					0x08		//
#define	psDEV_MDM					0x10		//
#define	psDEV_LAN					0x18		//
#define	psDEV_PRN					0x20		//
#define	psDEV_MSR					0x28		//
#define	psDEV_BUZ					0x30		//
#define	psDEV_KBD					0x38		//
#define	psDEV_LCD					0x40		//
#define	psDEV_RTC					0x48		//
#define	psDEV_TIM2					0x50		// ECC
#define	psDEV_TIM3					0x58		// ECC
#define	psDEV_SCR					0x60		// 60=ICC, 61=SAM1, 62=SAM2...
#define	psDEV_LTE					0x68
#define	psDEV_WIFI					0x70
#define	psDEV_BLE					0x78
#define	psDEV_SYS					0x80
#define	psDEV_TSC					0x88
#define	psDEV_SECM					0x90
#define	psDEV_SRAM					0x98
#define	psDEV_SRED					0xA0
#define	psDEV_CPHR					0xA8
#define	psDEV_OSSRAM				0xB0
#define	psDEV_PED					0xB8
#define	psDEV_FLS					0xC0
#define	psDEV_MEM					0xC8
#define	psDEV_NXP					0xD0
#define psDEV_TLS                   0xE0
#define	psDEV_SPI					0xF0
#define	psDEV_FS					0xF1		// for FLASH & SD
#define	psDEV_PMU					0xF2		// for PMU
#define	psDEV_OS					0xF3		// for FUNC_OS.c

//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
extern	void 	 PS_ClearAllProcess( void );
extern	BSP_BOOL PS_CheckLRC( UINT8 *pPST );
extern	void 	 PS_RenewLRC( UINT8 *pPST );
extern	void 	 PS_UpKillProcess( UINT8 *pPST, UINT32 index );
extern	void     PS_KillProcess( UINT8 pid );
extern	OS_PST	 *PS_SeekProcess( UINT8 pid );
extern	void     PS_ExecProcess( void );
extern	BSP_BOOL PS_ForkProcess( UINT8 devid, OS_PST *pPPST, UINT8 *dhn );
extern	BSP_BOOL PS_SignalProcess( UINT8 pid, UINT8 signal, UINT16 delay, OS_PST *pIsr );
extern	void     PS_CloseAllPID( UINT8 pid );
extern	BSP_BOOL PS_NormalCloseDevice( UINT8 dhn, UINT8 pid );

//----------------------------------------------------------------------------
#endif
