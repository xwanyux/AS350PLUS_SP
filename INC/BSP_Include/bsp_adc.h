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
#ifndef _BSP_ADC_H_
#define _BSP_ADC_H_

#include "bsp_types.h"
#include "za9_adc.h"
#include "bsp_int.h"
#include "bsp_utils.h"
#include "bsp_gpio.h"
#include "bsp_dma.h"



#define SAMP_BUF_SIZE						512
#define MAX_ADC_CLOCK_FREQ_HZ				600000



typedef enum ADC_MODE_E
{
	ADC_MODE_POLL,									// ADC samples obtained synchronously
	ADC_MODE_IRQ,									// Sample obtianed under interrupt control
	ADC_MODE_DMA									// Samples obtained via DMA
} ADC_MODE;

typedef struct BSP_ADC_S
{
	BSP_SEM						IsIdle;			// IsIdle = 0 once sampling begins
	ADC_MODE						Mode;				// Interrupt, Poll or DMA (default) mode.
	UINT32						AdcCfgReg;		// Controls channel rotation and sampling freq.
	UINT32						NumChannels;	// Number of channels used in sampling rotation
	UINT32						AdcStaReg;		// Watch for ADC_STA_FIFO_OVRFLOW = too slow

	/*
	 * Buffer Management variables.
	 */
	UINT16					 * pSampBuf;		// Buffer used to hold samples
	UINT32						SampBufSize;	// Size of pSampBuf (in 16-bit samples)
	UINT32						MaxSamples;		// Desired number of samples
	UINT32						RxThresh;		// Number of samples before calling pIsrFunc
	UINT16					 * pRd;				// Read pointer into the pSampBuf
	UINT16					 * pWr;				// DMA starting address
	UINT32						Avail;			// Number of samples available in pSampBuf
	UINT32						DmaCount;		// Number of samples being DMAed

	void						(*	pIsrFunc)		// Upper-layer interrupt callback
	(													// callback parameters		
		struct BSP_ADC_S	 * pAdc				// ADC reference
	);
	BSP_INT					 * pInt;				// BSP structure for interrupt control
	BSP_DMA					 * pDma;				// BSP structure for DMA control
} BSP_ADC;

typedef void (* ADC_ISR_FUNC )( BSP_ADC * pAdc ); 



/*
 * Function Prototypes
 */
extern BSP_STATUS BSP_ADC_Init( void );
extern BSP_ADC *  BSP_ADC_Acquire( ADC_ISR_FUNC pIsr );
extern BSP_STATUS BSP_ADC_Release( BSP_ADC * pAdc );
extern BSP_STATUS BSP_ADC_Start( BSP_ADC * pAdc );
extern BSP_STATUS BSP_ADC_Stop( BSP_ADC * pAdc );
extern BSP_STATUS BSP_ADC_Read( BSP_ADC * pAdc, UINT16 * pData, UINT32 * pLen );



#endif
