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
#ifndef _BSP_MCR_H_
#define _BSP_MCR_H_

#include "bsp_types.h"
#include "za9_mcr.h"
#include "bsp_int.h"
#include "bsp_dma.h"



/*
 * Track 1 Notes:
 * 210 bpi, 7 bit data (includes 1 ODD parity bit), max of 79 alphanumeric chars, RO
 * To convert raw data to ASCII add 0x20 to each 6-bit data byte.
 * Start sentinel:	0x25
 * End sentinel:		0x3F
 */

/*
 * Track 2 Notes:
 * 75 bpi, 5 bit data (includes 1 ODD parity bit), max of 40 numeric chars, RO
 * To convert raw data to ASCII add 0x30 to each 4-bit data byte.
 * Start sentinel:	0x3B
 * End sentinel:		0x3F
 */

/*
 * Track 3 Notes:
 * 210 bpi, 7 bit data (includes 1 ODD parity bit), max of 107 numeric chars, RW
 * To convert raw data to ASCII add 0x20 to each 6-bit data byte.
 * Start sentinel:	0x25
 * End sentinel:		0x3F
 */

/*
 * The size of the track data must be large enough to hold all characters from
 * all 3 tracks, including sentinels and LRC but not leading and trailing zeros.
 */
#define MAX_TRACK_DATA_BYTES				512


/*
 * At 210 bpi, a 3 3/8" magnetic stripe should contain approx 709 bits.
 * Allow for oversized tracks.  The worse case number of samples required 
 * is 2x the worse case number of bits per track.
 */
#define MAX_TRACK_BITS						1024
#define MAX_TRACK_SAMPLES					(MAX_TRACK_BITS * 2)

#define SIX_BIT_CHAR_WIDTH      			7
#define FOUR_BIT_CHAR_WIDTH     			5

#define MCR_END_SENTINEL					0x3F

#define NUM_MCR_TRACKS						3


/*
 * MCR operationsl modes
 */
#define MCR_MODE_DMA							1
#define MCR_MODE_INT							2
#define MCR_MODE_BYPASS						3


#define MCR_SAMPLE_FREQ_HZ					250000
#define MCR_DEFAULT_DCO						0x800
#define MCR_DEFAULT_THRESH_POS			100	// 38, 2010-05-03
#define MCR_DEFAULT_THRESH_NEG			100	// 38, 2010-05-03


/*
 * MCR Buffer
 * This buffer holds raw samples from all 3 tracks.
 * A maximum of 2560 samples can be stored in the buffer.
 * This buffer would need to be increased if card with 3 tracks
 * encoded at 210 bpi are used (non standard). 
 */
#define MCR_BUF_SIZE_BYTES					10240


/*
 * Track Status codes
 */
typedef enum
{
	MCR_SUCCESS,
	MCR_NO_SENTINEL,
	MCR_BAD_PARITY,
	MCR_BAD_LRC,
	MCR_TOO_FEW_SAMPLES,
	MCR_SWIPE_TOO_FAST,
	MCR_SWIPE_T00_SLOW
} MCR_STATUS_E;

typedef struct				MCR_TRACK_INFO_S
{
	MCR_STATUS_E			Status;
	UINT32					Count;
	UINT32					Speed_cmps;
	UINT32					Clip;

	UINT32					CountD;
	UINT32					MinD;
	UINT32					MaxD;
	UINT32					AveD;

	UINT32					InitBT;
	UINT32					FinalBT;
	UINT32					BTCount;
	UINT32					MinBT;
	UINT32					MaxBT;
	UINT32					AveBT;

	UINT32					MidCount;
	UINT32					MergeCount;

	UINT32					OneCount;
	UINT32					MinOne;
	UINT32					MaxOne;
	UINT32					AveOne;

	UINT32					ZeroCount;
	UINT32					MinZero;
	UINT32					MaxZero;
	UINT32 					AveZero;
} MCR_TRACK_INFO;

typedef struct				BSP_MCR_S
{
	UINT32					Mode;
	UINT32					Control;
	volatile UINT32		IntStatus;
	UINT32					Timing;
	UINT32					Adc;
	UINT32					DCOffset[NUM_MCR_TRACKS];
	UINT32					PosThresh[NUM_MCR_TRACKS];
	UINT32					NegThresh[NUM_MCR_TRACKS];

	UINT8					 * pBuf;									// Buffer to hold raw samples
	/*
	 * Variables used in MCT_INT_MODE
	 */
	UINT32				 * pIntBuf;								// Location in pBuf for next sample
	UINT32					SampleCount;						// Number of samples in pBuf

	UINT8					 * pTrackData[NUM_MCR_TRACKS];	// Array of pointers to track data (in ASCII)
	UINT32					TrackLen[NUM_MCR_TRACKS];		// Number of bytes of data on each track

	BSP_DMA				 * pDma;
	BSP_INT 				 * pMcrInt;
	void						(*	pIsrFunc)		// Upper-layer interrupt callback
	(													// callback parameters		
		struct BSP_MCR_S * pMcr					// MCR reference
	);		

	MCR_TRACK_INFO			Info[NUM_MCR_TRACKS];			// Debug information
} BSP_MCR;

typedef void (* MCR_ISR_FUNC )( BSP_MCR * pMcr ); 



extern BSP_STATUS BSP_MCR_Init( void );
extern BSP_MCR * BSP_MCR_Acquire( MCR_ISR_FUNC pIsr );
extern BSP_STATUS BSP_MCR_Release( BSP_MCR * pMcr );
extern BSP_STATUS BSP_MCR_Start( BSP_MCR * pMcr );
extern BSP_STATUS BSP_MCR_Stop( BSP_MCR * pMcr );
extern BSP_STATUS BSP_MCR_Read( BSP_MCR * pMcr );



#endif
