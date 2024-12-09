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
#ifndef _BSP_AES_H_
#define _BSP_AES_H_

#include "bsp_types.h"



#define BSP_AES_128_KEY_SIZE_BYTES		16
#define BSP_AES_192_KEY_SIZE_BYTES		24
#define BSP_AES_256_KEY_SIZE_BYTES		32

#define BSP_AES_128_KEY_SIZE_WORDS		((BSP_AES_128_KEY_SIZE_BYTES) >> 2)
#define BSP_AES_192_KEY_SIZE_WORDS		((BSP_AES_192_KEY_SIZE_BYTES) >> 2)
#define BSP_AES_256_KEY_SIZE_WORDS		((BSP_AES_256_KEY_SIZE_BYTES) >> 2)

#define BSP_AES_BLOCK_SIZE_BYTES			16


typedef struct BSP_AES_KEY_S
{
	UINT8					 * pData;
	UINT32					KeyLen;
} BSP_AES_KEY;


/*
 * Function prototypes.
 */
void 
AesInit
( 
	BSP_AES_KEY					 * pKey,
	BSP_HANDLE						hKeyTab
);


BSP_STATUS
AesXform
(
	UINT8							 * pIn,
	UINT8							 * pOut,	
	BSP_BOOL							Decrypt,
	BSP_HANDLE						hKeyTab
);



#endif
