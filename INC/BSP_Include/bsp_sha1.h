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
#ifndef _BSP_SHA1_H_
#define _BSP_SHA1_H_

#include "bsp_types.h"
#include "za9_sha1.h"

#define SHA1_HASH_SIZE_BYTES           20
#define SHA1_BLOCK_SIZE_BYTES          64
#define SHA_MAX_MSG_LEN						0x10000000

typedef struct BSP_SHA
{
	UINT8								Buffer[ SHA1_BLOCK_SIZE_BYTES ];
	UINT32							Index;
	UINT32							MsgLength;
} BSP_SHA;



/*
 * Function Prototypes
 */
extern BSP_STATUS BSP_SHA_Init( void );
extern BSP_SHA * BSP_SHA_Acquire( void );
extern BSP_STATUS BSP_SHA_Release( BSP_SHA * pSha );
extern BSP_STATUS BSP_SHA_Update( BSP_SHA * pSha, UINT8 * pData, UINT32 Length );
extern BSP_STATUS BSP_SHA_Final( BSP_SHA * pSha, UINT8 * pOutput, UINT32 Length );



#endif
