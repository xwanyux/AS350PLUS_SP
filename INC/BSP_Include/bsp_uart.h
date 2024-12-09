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
#ifndef _BSP_UART_H_
#define _BSP_UART_H_


#include "za9_uart.h"
#include "bsp_types.h"
#include "bsp_int.h"
#include "bsp_dma.h"
#include "bsp_utils.h"
#include "bsp_gpio.h"



#define UART_BUFF_SIZE 		512


#define UART_MODE_POLL		1
#define UART_MODE_IRQ		2
#define UART_MODE_DMA		3

#define BSP_MAX_UARTS		3


#define UART0_BASE			0xFFFE0000
#define UART1_BASE			0xFFFE1000
#define UART2_BASE			0xFFFE2000

#define UART_FLOW_NONE		0
#define UART_FLOW_RTS_CTS	1
#define UART_FLOW_XON_XOFF	2

#define UART_PARITY_NONE	0
#define UART_PARITY_ODD		1
#define UART_PARITY_EVEN	2

#define UART_LCTL_EPS		0x10
#define UART_LCTL_PEN		0x08



/*
 * GPIO pins available for Uart function:
 * After reset, the only uart pins available are Rx0, Rx1, Tx0 and Tx1.  All other
 * pins will be set to secondary function (GPIO).  RTS and CTS are available on
 * all 3 Uarts, but DCD, DTR, DTR, DSR and RI are only available on Uart0.
 * The GpioConfig member of the BSP_UART structure contains a bit mask of pins
 * that will be swicthed from GPIO to UART mode when the uart is activated.
 * Therefore if handshking is not required, or these pins are needed for GPIO
 * operation, the caller must clear the corresponding bits in the GpioConfig
 * member of the BSP_UART structure before activating the Corresponding Uart.
 */
#define UART_0_CTS			0x00000002
#define UART_0_RTS			0x00000010
#define UART_0_DCD			0x00000080
#define UART_0_DTR			0x00000100
#define UART_0_DSR			0x00000200
#define UART_0_RI 			0x00000400

#define UART_1_CTS			0x00000004
#define UART_1_RTS			0x00000020

#define UART_2_RX 			0x80000000
#define UART_2_TX 			0x00000001
#define UART_2_CTS			0x00000008
#define UART_2_RTS			0x00000040


/*
 * Macros for specifying which UART to use. 
 */
#define BSP_UART_0         0
#define BSP_UART_1         1
#define BSP_UART_2         2



typedef struct BSP_UART_S
{
	BSP_SEM						AcquireSem;
	BSP_SEM						StartSem;
   UINT32          			Mode;
   UINT32          			Base;
	UINT32						UartNum;
	UINT32						IntChannel;
	UINT32						GpioConfig;
	BSP_GPIO					 * pGpio0;
	BSP_GPIO					 * pGpio1;

	/*
	 * Uart Line Settings
	 */
	UINT32						Baud;
	UINT32						DataBits;
	UINT32						StopBits;
	UINT32						Parity;

	/*
	 * Flow Control
	 */
	UINT32						ModemStatus;
	UINT32						FlowControl;
	BSP_BOOL						RxFlowOn;
	BSP_BOOL						TxFlowOn;

	 /*
	  * Error Counters
	  */
	UINT32						BreakErrors;
   UINT32      			   FramingErrors;
   UINT32   			      ParityErrors;
	UINT32						OverrunErrors;
	UINT32						RxSpillErrors;

	/*
	 * Access control variables
	 */
	BSP_SEM						TxSem;
	BSP_SEM						RxSem;

	/*
	 * IRQ driven Tx variables
	 */
	UINT32						TxCount;
	UINT8						 * pTxData;
	UINT32						TxMaxWrite;
	UINT32						RxCallLevel;
	BSP_BOOL						TxCallCntrl;

	/*
	 * Buffer Management variables.
	 */
	UINT8						 * pRxBuffer;	// Used in DMA and IRQ mode
	UINT8						 * pTxBuffer;	// Only used in DMA mode
	UINT32						BufferSize;
	UINT8						 * pRd;		// Read pointer into the Rx Buffer
	UINT8						 * pWr;		// DMA starting address
	UINT32						Count;	// Max DMA byte count
	UINT32						Avail;	// Number of unread bytes in buffer
	UINT32						Read;		// Number of bytes already read

	/*
	 * DMA Mode variables
	 */
	BSP_DMA					 * pTxDma;
	BSP_DMA					 * pRxDma;

	void						(*	pIsrFunc)		// Upper-layer interrupt callback
	(													// callback parameters		
		struct BSP_UART_S  * pUart				// UART Port reference
	);		
	BSP_INT					 * pInt;
} BSP_UART;

typedef void (* UART_ISR_FUNC )( BSP_UART * pUart ); 



/*
 * Function Prototypes
 */
extern BSP_STATUS BSP_UART_Init( void );
extern BSP_UART * BSP_UART_Acquire( UINT32 UartNum, UART_ISR_FUNC pIsr );
extern BSP_STATUS BSP_UART_Release( BSP_UART * pUart );
extern BSP_STATUS BSP_UART_SetBaud(BSP_UART * pUart, UINT32 Baud );
extern BSP_STATUS BSP_UART_Start( BSP_UART * pUart );
extern BSP_STATUS BSP_UART_Stop( BSP_UART * pUart );
extern BSP_STATUS BSP_UART_SetModemControl( BSP_UART * pUart, UINT32 Flags );
extern BSP_STATUS BSP_UART_GetModemStatus(	BSP_UART	* pUart );

extern UINT32 BSP_UART_GetRxAvail( BSP_UART * pUart );
extern UINT32 BSP_UART_Read( BSP_UART *uart, UINT8 * pBuf, UINT32 Max );
extern BSP_STATUS BSP_UART_Write( BSP_UART * pUart, UINT8 * pData, UINT32 Len );



#endif
