/*
 * Copyright 2007, ZiLOG Inc.
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
#ifndef _BSP_PICO_H_
#define _BSP_PICO_H_

#include "bsp_gpio.h"
#include "bsp_spi.h"


#undef PICO_DEBUG


/*
 * PicoRead Page 0 command bits
 */
#define PICO_CMD_ACCESS_P5		0x80
#define PICO_CMD_NO_RESET		0x40
#define PICO_CMD_RESET			0x00
#define PICO_CMD_READ			0x20
#define PICO_CMD_WRITE			0x00
#define PICO_CMD_PAGE_1			(1<<2)
#define PICO_CMD_PAGE_2			(2<<2)
#define PICO_CMD_PAGE_3			(3<<2)
#define PICO_CMD_PAGE_4			(4<<2)
#define PICO_CMD_PAGE_5			(5<<2)
#define PICO_CMD_PAGE_6			(6<<2)
#define PICO_CMD_PAGE_7			(7<<2)
#define PICO_CMD_RF_ON			0x02


/*
 * PicoRead Page 5 command bits
 */
#define PICO_PG5_ACCESS_P5		0x80
#define PICO_PG5_RX				0x40
#define PICO_PG5_TX				0x00
#define PICO_PG5_USE_PG_1		(0<<4)
#define PICO_PG5_USE_PG_2		(1<<4)
#define PICO_PG5_USE_PG_3		(2<<4)
#define PICO_PG5_USE_PG_4		(3<<4)
#define PICO_PG5_CRC_EN			0x08
#define PICO_PG5_EXT				0x01


/*
 * Command bytes used when transmitting PicoPass commands
 */
#define PICOPASS_ACTALL			0x0A
#define PICOPASS_ACT				0x8E
#define PICOPASS_IDENTIFY		0x0C
#define PICOPASS_SELECT			0x81
#define PICOPASS_HALT			0x00



/*
 * Default PicoPass timeout (in us)
 */
#define PICOPASS_RX_TIMEOUT	1000

/*
 * Default picoread buffer size
 */
#define PICO_BUF_SIZE			260


/*
 * Pico driver operating modes
 */
#define PICO_MODE_IRQ			0x00
#define PICO_MODE_POLL			0x01

/*
 * Pico driver state
 */
#define PICO_STATE_IDLE			0x00
#define PICO_STATE_BUSY			0x01
#define PICO_STATE_TRANSMIT	0x02
#define PICO_STATE_RECEIVE		0x03

/*
 * Pico driver callback events
 */
#define PICO_EVENT_TXC			0x01
#define PICO_EVENT_RX			0x02
#define PICO_EVENT_RX_TO		0x03
#define PICO_EVENT_RX_ERROR	0x04
#define PICO_EVENT_RX_OVFL		0x05
#define PICO_EVENT_TX_TO		0x06
#define PICO_EVENT_TX_ERROR	0x07



/*
 * Structures for accessing Picoread registers
 */
typedef UINT8				PICO_PROTOCOL_CFG[5];
typedef UINT8				PICO_TX_RX_CFG[2];
typedef UINT8				PICO_RF_CFG[8];
typedef UINT8				PICO_TEST_CFG[2];


typedef struct				BSP_PICO_S
{
	UINT32					State;			// Picoread state
	UINT32					Event;			// valid on callback
	UINT32					Mode;				// IRQ or POLL
	PICO_TX_RX_CFG			Page5Tx;
	PICO_TX_RX_CFG			Page5Rx;
	UINT32					Timeout;			// in microseconds (approx)
	UINT32					RxBufSize;
	UINT8					 * pRxData;
	UINT8					 * pTxData;
	UINT32					Len;				// Length of Rx/ Tx data
	void						(*	pIsrFunc)
	(
		struct BSP_PICO_S * pPico
	);
} BSP_PICO;

typedef void (* PICO_ISR_FUNC )( BSP_PICO * pPico ); 


/*
 * Function Prototypes
 */
BSP_STATUS BSP_PICO_Init( void );
BSP_PICO * BSP_PICO_Acquire( UINT32 SpiNum, PICO_ISR_FUNC pIsr );
BSP_STATUS BSP_PICO_Start( BSP_PICO * pPico );
BSP_STATUS BSP_PICO_Stop( BSP_PICO * pPico );
BSP_STATUS BSP_PICO_Release( BSP_PICO * pPico );
BSP_STATUS BSP_PICO_Write( BSP_PICO * pPico, UINT8 * pData, UINT32 Len );
UINT32 BSP_PICO_Read( BSP_PICO * pPico, UINT8 * pData, UINT32 Len );
BSP_STATUS BSP_PICO_Command( BSP_PICO * pPico, UINT32 Command, UINT8 * pData, UINT32 Len );



#endif
