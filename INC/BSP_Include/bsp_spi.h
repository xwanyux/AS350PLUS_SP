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
#ifndef _BSP_SPI_H_
#define _BSP_SPI_H_


#include "za9_spi.h"
#include "bsp_types.h"
#include "bsp_int.h"
#include "bsp_dma.h"
#include "bsp_utils.h"



#define BSP_MAX_SPIS		2

/*
 * Macros for specifying which SPI to use. 
 */

#define BSP_SPI_0         0
#define BSP_SPI_1         1


/*
 * Macros for SPIs mode of operation.
 */

#define SPI_SLAVE		  0
#define SPI_MASTER		  1

/*
 * Macros for SPIs data transfer operation mode.
 */

#define SPI_POLL	  0
#define SPI_IRQ		  1
#define SPI_DMA		  2

/*
 * GPIO pin definitions for SPI0 and SPI1
 */


#define SPI_0_MISO		BIT11
#define SPI_1_MISO		BIT12
#define SPI_0_MOSI		BIT13
#define SPI_1_MOSI		BIT14
#define SPI_0_SCK		BIT15
#define SPI_1_SCK		BIT16
#define SPI_0_SS		BIT17
#define SPI_1_SS		BIT18


/*
 * NUMBITS values
 */

#define NUMBITS_16		 (0x00 << 2)
#define NUMBITS_15		 (0x0F << 2)
#define NUMBITS_14		 (0x0E << 2)
#define NUMBITS_13		 (0x0D << 2)
#define NUMBITS_12		 (0x0C << 2)
#define NUMBITS_11		 (0x0B << 2)
#define NUMBITS_10		 (0x0A << 2)
#define NUMBITS_09		 (0x09 << 2)
#define NUMBITS_08		 (0x08 << 2)
#define NUMBITS_07		 (0x07 << 2)
#define NUMBITS_06		 (0x06 << 2)
#define NUMBITS_05		 (0x05 << 2)
#define NUMBITS_04		 (0x04 << 2)
#define NUMBITS_03		 (0x03 << 2)
#define NUMBITS_02		 (0x02 << 2)
#define NUMBITS_01		 (0x01 << 2)


#define RX_FIFO_LEVEL_4		 (0x03 << 16)
#define RX_FIFO_LEVEL_3		 (0x02 << 16)
#define RX_FIFO_LEVEL_2		 (0x01 << 16)
#define RX_FIFO_LEVEL_1		 (0x00 << 16)

#define TX_FIFO_LEVEL_4		  0x03 
#define TX_FIFO_LEVEL_3		  0x02
#define TX_FIFO_LEVEL_2		  0x01
#define TX_FIFO_LEVEL_1		  0x00



/*
 *	SPI Structure Definition	
 */

typedef struct BSP_SPI_S
{
	/*
	 *  SPI Register variables
	 */
	
	BSP_SEM						AcquireSem;
	BSP_SEM						StartSem;
	
	UINT32						Mode;		// Master = 1, Slave = 0
	UINT32						DataTxMode;	// Poll = 0, IRQ = 1 and DMA = 2
	UINT32						Base;		// Base address of the SPI device
	UINT32						DataReg;        // SPI DATA REG    
	UINT32						CtlReg;		// SPI_CTL_REG
	UINT32						StatusReg;	// SPI_STA_REG
	UINT32						ModReg;		// SPI_MOD_REG
	UINT32						DiagReg;	// SPI_DIAG_STA_REG
	UINT32						BrgReg;		// SPI_BRG_REG
	UINT32						DmaReg;		// SPI_DMA_REG
	UINT32						IntChannel;				
	UINT32						SpiNum;
	BSP_GPIO					* pGpio0;
	BSP_GPIO					* pGpio1;
	UINT32						GpioConfig;
	UINT32						GpioSS;
	UINT32						PmuSpiBit;	// PMU clock enable registers bit for SPI

	/*
         * IRQ transfer status variables
         */
	
	UINT32						IrqTxState;	// =1 -> Transfer in progress
									// =0 -> No Data Transfer is in progress
	/*
	 * IRQ driven Tx variables
	 */
	UINT32						TxCount;
	UINT16						* pTxData;
	UINT32						TxMaxWrite;
	UINT32						RxCallLevel;
	BSP_BOOL					TxCallCntrl;

	 /*
	  * Error Counters
	  */
	UINT32						OverrunErrors;
	UINT32						RxSpillErrors;


	
	/*
	 * Access control variables
	 */
	BSP_SEM						TxSem;
	BSP_SEM						RxSem;

	BSP_BOOL					TxFlowOn;
	BSP_BOOL					RxFlowOn;

	/*
	 * Buffer Management variables.
	 */

	UINT16						 * pRxBuffer;	// Used in DMA and IRQ mode
	UINT16						 * pTxBuffer;	// Only used in DMA mode
	UINT32						   BufferSize;
	UINT16						 * pRd;		// Read pointer into the Rx Buffer
	UINT16						 * pWr;		// DMA starting address
	UINT32						   Count;		// Max DMA byte count
	UINT32						   Avail;		// Number of unread bytes in buffer
	UINT32						   Read;		// Number of bytes already read

	/*
	 * DMA Mode variables
	*/
		 
	BSP_DMA					 * pTxDma;
	BSP_DMA					 * pRxDma;	
	
	
	void					(*	pIsrFunc)	// Upper-layer interrupt callback
	(								// callback parameters		
		struct BSP_SPI_S  * pSpi				// SPI Port reference
	);		
	
	BSP_INT					 * pInt;


} BSP_SPI;

typedef void (* SPI_ISR_FUNC )( BSP_SPI * pSpi ); 

/*
 * Function Prototypes
 */
extern BSP_STATUS BSP_SPI_Init ( void );
extern BSP_SPI * BSP_SPI_Acquire ( UINT32 SpiNum, SPI_ISR_FUNC pIsr );
extern BSP_STATUS BSP_SPI_Release ( BSP_SPI * pSpi );
extern BSP_STATUS BSP_SPI_Start ( BSP_SPI *pSpi);
extern BSP_STATUS BSP_SPI_Stop ( BSP_SPI * pSpi );
extern BSP_STATUS BSP_SPI_Write ( BSP_SPI * pSpi, UINT16 *pTxData, UINT32 Len );
extern UINT32	  BSP_SPI_Read ( BSP_SPI * pSpi, UINT16 *pRxData, UINT32 Len );
//extern UINT32     BSP_SPI_Read ( BSP_SPI * pSpi );
extern BSP_STATUS BSP_SPI_SetBaud ( BSP_SPI * pSpi, UINT32 NewBaud );
extern BSP_STATUS BSP_SPI_SSAssert ( BSP_SPI * pSpi);
extern BSP_STATUS BSP_SPI_SSDeAssert ( BSP_SPI * pSpi);

#endif


