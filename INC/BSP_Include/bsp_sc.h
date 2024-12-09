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
#ifndef _BSP_SC_H_
#define _BSP_SC_H_

#include "bsp_types.h"
#include "za9_sc.h"
#include "bsp_int.h"
#include "bsp_utils.h"
#include "bsp_gpio.h"
#include "bsp_dma.h"


/*
 * Identify each smartcard interface.
 * The single Main card (asynch) is connected smartcard controller SC0.
 * Theere are 4 SIM cards (async) connected to smartcard controller SC1.
 */
#define BSP_SC0_MAIN							0
#define BSP_SC1_SIM1							1
#define BSP_SC1_SIM2							2
#define BSP_SC1_SIM3							3
#define BSP_SC1_SIM4							4
#define BSP_MAX_SC_INTERFACES				5
/*
 * Alternate IDs for the smartcard interfaces
 */
#define BSP_SC_IF_0							(BSP_SC0_MAIN)
#define BSP_SC_IF_1							(BSP_SC1_SIM1)
#define BSP_SC_IF_2							(BSP_SC1_SIM2)
#define BSP_SC_IF_3							(BSP_SC1_SIM3)
#define BSP_SC_IF_4							(BSP_SC1_SIM4)


/*
 * Smartcard addresses used with SPI 
 */
#define SC_SPI_MAIN							0x80
#define SC_SPI_SIM1							0x00
#define SC_SPI_SIM2							0x20
#define SC_SPI_SIM3							0x40
#define SC_SPI_SIM4							0x60


#define MAX_ATR_HISTORICAL_BYTES			16
#define TS_DIRECT_CONVENTION				0x3B
#define TS_INVERSE_CONVENTION				0x3F

#define DEFAULT_SMC_BRR					0x252		// F = 372, D =1, 13.4K bps
#define DEFAULT_FI					372
//#define	DEFAULT_SMC_BRR					0x341		// F = 521, D =1, 9600  bps
//#define	DEFAULT_FI					521

typedef struct BSP_SC_ATR_S
{
	UINT8							TS;
	UINT8							T0;
	UINT8							TA1;
	UINT8							TB1;
	UINT8							TC1;
	UINT8							TD1;
	UINT8							TA2;
	UINT8							TB2;
	UINT8							TC2;
	UINT8							TD2;
	UINT8							TA3;
	UINT8							TB3;
	UINT8							TC3;
	UINT8							HistoricalLen;
	UINT8							Historical[ MAX_ATR_HISTORICAL_BYTES ];
} BSP_SC_ATR;


/*
 * Smartcard States
 */
typedef enum 					SC_STATE_E
{
	SC_STATE_FREE,				// Device not acquired
	SC_STATE_IDLE,				// Device not started
	SC_STATE_NOT_PRESENT,	// Card not inserted
	SC_STATE_PRESENT,			// Card inserted, not powerd
	SC_STATE_READY,			// Ready for command
	SC_STATE_ERROR,			// Card Error reset or remove card

	/*
	 * ATR States
	 */
	SC_STATE_COLD_RESET,
	SC_STATE_WARM_RESET,
	SC_STATE_VALID_COLD_ATR,
	SC_STATE_VALID_WARM_ATR,

	/*
	 * T=0 command processing states
	 */
	SC_STATE_COMMAND,
	SC_STATE_RESPONSE,

	/*
	 * T=1 command processing states
	 */
	SC_STATE_I_RESPONSE,		// Awaiting I frame.
	SC_STATE_R_RESPONSE,		// Awaiting acknowledgement of last I frame fragment.
	SC_STATE_S_RESPONSE		// Awaiting S response
} SC_STATE;


typedef struct BSP_SC_IF_S
{
	SC_STATE						State;			// Smartcard state
	BSP_SEM						Avail;			// Avail = 1 if not in use
	BSP_SEM						IsIdle;			// IsIdle = 0 once timer started
	UINT32						Ifc;				// Identifies the smartcard interface (0-4)
	UINT32						SpiAddr;			// Identifies the smartcard SPI address
	UINT32						Base;				// Base address of associated SC controller

	BSP_SC_ATR					ATR;				// Answer to reset

	/*
	 * Variables used during command processing
	 */
	UINT8 						CmdHdr[5];
	UINT8							Lc;
	UINT8							Le;
	UINT8							Sw1;
	UINT8							Sw2;

	UINT32 						Case;
	UINT8						 * pCmd;
	UINT32						CmdLen;
	UINT8						 * pRsp;
	UINT32						RspLen;
	UINT32						RspMax;

	/*
	 * Protocol Timings
	 */
	UINT32						F;					// Clock Rate conversion factor
	UINT32						D;					// Bit Rate Adjustment factor (1 ETU = F/(D * CardClock))
	UINT32						VPP;				// Programming Voltage (should be 0)
	UINT32						GuardTime;		// Extra Guard Time (in ETUs)
	UINT32						Protocol;		// 0 means T=0; 1 means T=1
	UINT32						WWT;				// Work Waiting Time for T=0
	UINT32						IFS;				// Information Field Size for T=1
	UINT32						CWT;				// Character Waiting Time for T=1
	UINT32						BWT;				// Block Waiting Time for T=1

	/*
	 * T=1 variables
	 */
	UINT8							PCB;				// PCB for next transmit request
	UINT8							LastPCB;			// Last PCB (re)transmitted
	UINT8							TxSeq;			// Sequence number of last frame
	UINT8							RxSeq;			// Expected sequence of ICC response
	UINT32 						InvalidRx;		// Number of consecutive invalid received blocks
	UINT32						TxAcked;			// True after a transmitted I-block is acknowldged
	void						(*	pIsrFunc)		// Upper-layer interrupt callback
	(													// callback parameters		
		struct BSP_SC_IF_S * pScIf				// Smartcard Interface reference
	);
} BSP_SC_IF;

typedef void (* SC_ISR_FUNC )( BSP_SC_IF * pScIf ); 


/*
 * T=0 C-APDU offsets
 */
#define C_APDU_CLA_OFS						0
#define C_APDU_INS_OFS						1
#define C_APDU_P1_OFS						2
#define C_APDU_P2_OFS						3
#define C_APDU_P3_OFS						4

/*
 * T=1 data structures
 */
#define T1_NAD									0x00

#define PCB_BLOCK_TYPE_MASK				0xC0
#define PCB_I_BLOCK							0x00
#define PCB_R_BLOCK							0x80
#define PCB_S_BLOCK							0xC0

#define PCB_I_SEQ								BIT6
#define PCB_I_CHAIN							BIT5

#define PCB_R_SEQ								BIT4
#define PCB_R_ERROR_MASK					0x2F
#define PCB_R_ERROR_NONE					0x00
#define PCB_R_ERROR_EDC_PARITY			0x01
#define PCB_R_ERROR_OTHER					0x02

#define PCB_S_REQUEST						0x00
#define PCB_S_RESPONSE						BIT5
#define PCB_S_CMD_MASK						0x1F
#define PCB_S_CMD_RESYNC					0x00
#define PCB_S_CMD_IFS						0x01
#define PCB_S_CMD_ABORT						0x02
#define PCB_S_CMD_BWT_EXTENSION			0x03

typedef struct					BSP_T1_HDR_S
{
	UINT8							NAD;
	UINT8							PCB;
	UINT8							Len;
} BSP_T1_HDR;



/*
 * Function Prototypes
 */
extern BSP_STATUS  BSP_SC_Init( void );
extern BSP_SC_IF * BSP_SC_Acquire( UINT32 Ifc, SC_ISR_FUNC pIsr );
extern BSP_STATUS  BSP_SC_Release( BSP_SC_IF * pScIf );
extern BSP_STATUS  BSP_SC_Start( BSP_SC_IF * pScIf );
extern BSP_STATUS  BSP_SC_Stop( BSP_SC_IF * pScIf );

extern BSP_STATUS  BSP_SC_Reset( BSP_SC_IF * pScIf );
extern BSP_STATUS BSP_SC_Command( BSP_SC_IF * pSc, UINT8 * pCmd, UINT32 CmdLen, UINT8 * pRsp, UINT32 RspLen );



#endif
