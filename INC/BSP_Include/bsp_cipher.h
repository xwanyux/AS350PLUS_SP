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
#ifndef _BSP_CIPHER_H_
#define _BSP_CIPHER_H_

#include "bsp_types.h"
#include "bsp_utils.h"
#include "bsp_aes.h"



/*
 * Supported cipher algorithms.
 */
#define BSP_CPHR_DES							0
#define BSP_CPHR_3DES						1
#define BSP_CPHR_RSA							2
#define BSP_CPHR_AES							3


/*
 * Maximum number of control blocks per cipher.
 */
#define BSP_CPHR_MAX_DES					2
#define BSP_CPHR_MAX_3DES					2
#define BSP_CPHR_MAX_RSA					1
#define BSP_CPHR_MAX_AES					2


/*
 * Cipher transformations
 * The Mode member of the BSP_CPHT structure must be set to one
 * of the following values.
 */
#define BSP_CPHR_MODE_ENCRYPT				0
#define BSP_CPHR_MODE_DECRYPT				1


/*
 * Key sizes.
 * The RSA algorithm can use key sizes of arbitrary length; however, this
 * implementation restricts the key size to 2048 bits.
 */
#define BSP_DES_KEY_SIZE_BYTES			8
#define BSP_3DES_KEY_SIZE_BYTES			24
#define BSP_DES_BLOCK_SIZE_BYTES			8
#define BSP_RSA_KEY_MAX_SIZE_BYTES		256



/*
 * Key data types
 */
typedef struct BSP_DES_KEY_S
{
	UINT8							Data[ BSP_DES_KEY_SIZE_BYTES ];
} BSP_DES_KEY;

typedef struct BSP_3DES_KEY_S
{
	UINT8							Data[ BSP_3DES_KEY_SIZE_BYTES ];	
} BSP_3DES_KEY;

typedef struct BSP_RSA_KEY_S
{
	UINT8						 * pMod;
	UINT32						ModLen;
	UINT8						 * pPubExp;
	UINT32						PubLen;
	UINT8						 * pPrivExp;
	UINT32						PrivLen;
	UINT8						 * pPrime1;
	UINT32						Prime1Len;
	UINT8						 * pPrime2;
	UINT32						Prime2Len;
	UINT8						 * pExp1;
	UINT32						Exp1Len;
	UINT8						 * pExp2;
	UINT32						Exp2Len;
	UINT8						 * pCoef;
	UINT32						CoefLen;
} BSP_RSA_KEY;


typedef union BSP_CPHR_KEY_U
{
	BSP_DES_KEY					Des;
	BSP_3DES_KEY					Des3;
	BSP_RSA_KEY					Rsa;
	BSP_AES_KEY					Aes;
} BSP_CPHR_KEY;


typedef struct BSP_CHR_S
{
	UINT32						Cipher;
	UINT32						Mode;
	BSP_SEM						Avail;
	BSP_SEM						IsIdle;
	BSP_HANDLE					Reserved;
} BSP_CPHR;



/*
 * Function Prototypes
 */
extern BSP_STATUS BSP_CPHR_Init( void );
extern BSP_CPHR * BSP_CPHR_Acquire( UINT32 Cipher, BSP_CPHR_KEY * pKey );
extern BSP_STATUS BSP_CPHR_Release( BSP_CPHR * pCipher );
extern BSP_STATUS BSP_CPHR_Xform( BSP_CPHR * pCipher, UINT8 * pIn, UINT8 * pOut, UINT32 Len );



#endif

