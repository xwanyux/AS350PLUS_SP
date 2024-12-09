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
#ifndef _ZA9_SHA1_H_
#define _ZA9_SHA1_H_

/*
 * SHA1 Hardware mapping
 */
#define SHA_H0_REG 			0xFFFF9000
#define SHA_H1_REG 			0xFFFF9004
#define SHA_H2_REG 			0xFFFF9008
#define SHA_H3_REG 			0xFFFF900C
#define SHA_H4_REG 			0xFFFF9010
#define SHA_DATA_REG 		0xFFFF9014
#define SHA_CTRL_REG 		0xFFFF9018
#define SHA_STA_REG 			0xFFFF901C
#define SHA_WH0_REG 			0xFFFF9020
#define SHA_WH1_REG 			0xFFFF9024
#define SHA_WH2_REG 			0xFFFF9028
#define SHA_WH3_REG 			0xFFFF902C
#define SHA_WH4_REG 			0xFFFF9030

/*
 * SHA_CTRL_REG bit definitions
 */
#ifndef ZA9_AB_SILICON
	#define SHA_LE_EN			0x08
#endif
#define SHA_DMA_EN			0x04
#define SHA_INIT_SEL			0x02
#define SHA_INIT				0x01

/*
 * SHA_STA_REG bit definitions
 */
#define SHA_VALID				0x01



#endif 

