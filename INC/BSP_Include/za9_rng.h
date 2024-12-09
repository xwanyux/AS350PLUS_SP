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
#ifndef _ZA9_RNG_H_
#define _ZA9_RNG_H_

/*
 * RNG Hardware mapping
 */
#define RNG_DATA_REG			0xFFFFA000
#define RNG_CTRL_REG			0xFFFFA004

/*
 * RNG_CTRL_REG bit definitions
 */
#define RNG_IRQ    			0x40
#define RNG_GRNT     		0x20
#define RNG_IRQ_CLR			0x08
#define RNG_IRQ_EN			0x04
#define RNG_REQ				0x02



#endif 

