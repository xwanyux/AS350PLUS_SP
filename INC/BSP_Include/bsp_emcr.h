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
 
#ifndef __EMCR_H_
#define __EMCR_H_

#include "za9_defines.h"
#include "bsp_types.h"
#include "bsp_utils.h"
#include "bsp_gpio.h"
#include "bsp_spi.h"
#include "bsp_cipher.h"



#define NUM_EMCR_TRACKS						3
#define MAX_EMCR_RAWDATABUF_SIZE			512
#define MAX_EMCR_TRACKDATABUF_SIZE		120


typedef enum BSP_EMCR_STATUS_E
{
	EMCR_SUCCESS					= 0x0000,
	EMCR_FAILURE					= 0x0001,
	EMCR_BAD_PARAMETER			= 0x0002,
	EMCR_TIMEOUT					= 0x0004,
	EMCR_BAD_INPUT_PARAMETER	= 0x0008,
	EMCR_RESOURCE_UNAVAILABLE	= 0x0010,
	EMCR_HOST_KEY_NOT_VALID		= 0x0020,
	EMCR_KEYLOCKED					= 0x0040,
	EMCR_TRACK_LRC_ERROR			= 0x0080,
	EMCR_TRACK_PARITY_ERROR		= 0x0100,	
	EMCR_EXTRNAL_AUTHEN_NEEDED	= 0x0200,
	EMCR_START_SENTINEL_ERROR	= 0x0400
}BSP_EMCR_STATUS;

	
typedef struct 			BSP_EMCR_COMMAND_S
{
	UINT8						CommandData[32];
	UINT8						CommandLen;
	UINT8						ResponseData[32];
	UINT8						ResponseLen;
	BSP_EMCR_STATUS 		Status;
}BSP_EMCR_COMMAND;


typedef struct				BSP_EMCR_S
{
	BSP_SEM			    	IsIdle;
	BSP_SPI				 * pSpi;
		
	// CS8 to determine the card swipe
	BSP_GPIO 			 * pGpio;	
	UINT8					 * pRawDataBuf;
	UINT8					 * pTrackData[NUM_EMCR_TRACKS];
	UINT32					TrackLen[NUM_EMCR_TRACKS];	
	UINT32					TrackStatus[NUM_EMCR_TRACKS];
	void					(*	pIsrFunc)
	(										
		struct BSP_EMCR_S * pEmcr
	);		
	UINT8 					DeviceKey[24];
	UINT8						DevSerialNum[8];
	UINT8						SoftWareId[11];
} BSP_EMCR;

typedef void (* EMCR_ISR_FUNC )( BSP_EMCR * pEmcr ); 




extern BSP_STATUS	BSP_EMCR_Init( 	void );
extern BSP_EMCR*  BSP_EMCR_Acquire( UINT32 SpiNum, EMCR_ISR_FUNC pIsr );
extern BSP_STATUS BSP_EMCR_Release( BSP_EMCR *pEmcr );
extern BSP_STATUS BSP_EMCR_Read( 	BSP_EMCR *pEmcr );
extern BSP_STATUS BSP_EMCR_Start( 	BSP_EMCR *pEmcr );
extern BSP_STATUS BSP_EMCR_Stop( 	BSP_EMCR *pEmcr );

extern BSP_EMCR_STATUS 	BSP_EMCR_LoadDevKey( 				BSP_EMCR *pEmcr, UINT8 * pKey16 );
extern BSP_EMCR_STATUS	BSP_EMCR_VerifyDeviceKey( 			BSP_EMCR *pEmcr );
extern BSP_EMCR_STATUS	BSP_EMCR_Reset( 						BSP_EMCR *pEmcr );
extern BSP_EMCR_STATUS 	BSP_EMCR_SetEncryptionProperty( 	BSP_EMCR *pEmcr, BSP_BOOL EncryptOn );
extern BSP_EMCR_STATUS 	BSP_EMCR_SetKeyLockProperty( 		BSP_EMCR *pEmcr, BSP_BOOL LockOn );
extern BSP_EMCR_STATUS	BSP_EMCR_ExternalAuthentication( BSP_EMCR *pEmcr );
extern BSP_EMCR_STATUS	BSP_EMCR_SaveNonVolatileData( 	BSP_EMCR	*pEmcr );
extern BSP_EMCR_STATUS 	BSP_EMCR_SetDevSerialNum( 			BSP_EMCR *pEmcr, UINT8 * pSerial8 );
extern BSP_EMCR_STATUS 	BSP_EMCR_GetDevSerialNum( 			BSP_EMCR	*pEmcr );
extern BSP_EMCR_STATUS	BSP_EMCR_GetSoftwareId( 			BSP_EMCR	*pEmcr );



#endif
