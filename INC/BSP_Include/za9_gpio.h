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
#ifndef _ZA9_GPIO_H_
#define _ZA9_GPIO_H_

#include "za9_defines.h"


/*
 * GPIO Register Offsets
 */
#define GPIO_EN								0x00
#define GPIO_EN_SET							0x04
#define GPIO_EN_CLR							0x08

#define GPIO_OE								0x0C
#define GPIO_OE_SET							0x10
#define GPIO_OE_CLR							0x14

#define GPIO_OUT								0x18
#define GPIO_OUT_SET							0x1C
#define GPIO_OUT_CLR							0x20

#define GPIO_IN								0x24

#define GPIO_IMOD								0x28
#define GPIO_IPOL								0x2C
#define GPIO_ISEL								0x30

#define GPIO_IEN								0x34
#ifdef ZA9_AB_SILICON
	#define GPIO_IAST							0x38
	#define GPIO_IBST							0x3C
	#define GPIO_ICLR							0x40

	#define GPIO_WKEN							0x44
#else
	#define GPIO_IEN_SET						0x38
	#define GPIO_IEN_CLR						0x3C

	#define GPIO_IAST							0x40
	#define GPIO_IBST							0x44
	#define GPIO_ICLR							0x48

	#define GPIO_WKEN							0x4C
	#define GPIO_WKEN_SET					0x50
	#define GPIO_WKEN_CLR					0x54

	#define GPIO_OPEN_D						0x58
#endif


/*
 * GPIO Port 0 Registers
 */
#define GPIO0_EN								(GPIO0_BASE | GPIO_EN)
#define GPIO0_EN_SET							(GPIO0_BASE | GPIO_EN_SET)
#define GPIO0_EN_CLR							(GPIO0_BASE | GPIO_EN_CLR)

#define GPIO0_OE								(GPIO0_BASE | GPIO_OE)
#define GPIO0_OE_SET							(GPIO0_BASE | GPIO_OE_SET)
#define GPIO0_OE_CLR							(GPIO0_BASE | GPIO_OE_CLR)

#define GPIO0_OUT								(GPIO0_BASE | GPIO_OUT)
#define GPIO0_OUT_SET						(GPIO0_BASE | GPIO_OUT_SET)
#define GPIO0_OUT_CLR						(GPIO0_BASE | GPIO_OUT_CLR)

#define GPIO0_IN								(GPIO0_BASE | GPIO_IN)

#define GPIO0_IMOD							(GPIO0_BASE | GPIO_IMOD)
#define GPIO0_IPOL							(GPIO0_BASE | GPIO_IPOL)
#define GPIO0_ISEL							(GPIO0_BASE | GPIO_ISEL)

#define GPIO0_IEN								(GPIO0_BASE | GPIO_IEN)
#define GPIO0_IEN_SET						(GPIO0_BASE | GPIO_IEN_SET)
#define GPIO0_IEN_CLR						(GPIO0_BASE | GPIO_IEN_CLR)

#define GPIO0_IAST							(GPIO0_BASE | GPIO_IAST)
#define GPIO0_IBST							(GPIO0_BASE | GPIO_IBST)
#define GPIO0_ICLR							(GPIO0_BASE | GPIO_ICLR)

#define GPIO0_WKEN							(GPIO0_BASE | GPIO_WKEN)
#define GPIO0_WKEN_SET						(GPIO0_BASE | GPIO_WKEN_SET)
#define GPIO0_WKEN_CLR						(GPIO0_BASE | GPIO_WKEN_CLR)


/*
 * GPIO Port 1 Registers
 */
#define GPIO1_EN								(GPIO1_BASE | GPIO_EN)
#define GPIO1_EN_SET							(GPIO1_BASE | GPIO_EN_SET)
#define GPIO1_EN_CLR							(GPIO1_BASE | GPIO_EN_CLR)

#define GPIO1_OE								(GPIO1_BASE | GPIO_OE)
#define GPIO1_OE_SET							(GPIO1_BASE | GPIO_OE_SET)
#define GPIO1_OE_CLR							(GPIO1_BASE | GPIO_OE_CLR)

#define GPIO1_OUT								(GPIO1_BASE | GPIO_OUT)
#define GPIO1_OUT_SET						(GPIO1_BASE | GPIO_OUT_SET)
#define GPIO1_OUT_CLR						(GPIO1_BASE | GPIO_OUT_CLR)

#define GPIO1_IN								(GPIO1_BASE | GPIO_IN)

#define GPIO1_IMOD							(GPIO1_BASE | GPIO_IMOD)
#define GPIO1_IPOL							(GPIO1_BASE | GPIO_IPOL)
#define GPIO1_ISEL							(GPIO1_BASE | GPIO_ISEL)

#define GPIO1_IEN								(GPIO1_BASE | GPIO_IEN)
#define GPIO1_IEN_SET						(GPIO1_BASE | GPIO_IEN_SET)
#define GPIO1_IEN_CLR						(GPIO1_BASE | GPIO_IEN_CLR)

#define GPIO1_IAST							(GPIO1_BASE | GPIO_IAST)
#define GPIO1_IBST							(GPIO1_BASE | GPIO_IBST)
#define GPIO1_ICLR							(GPIO1_BASE | GPIO_ICLR)

#define GPIO1_WKEN							(GPIO1_BASE | GPIO_WKEN)
#define GPIO1_WKEN_SET						(GPIO1_BASE | GPIO_WKEN_SET)
#define GPIO1_WKEN_CLR						(GPIO1_BASE | GPIO_WKEN_CLR)



/*
 * GPIO Port 2 Registers
 */
#define GPIO2_EN								(GPIO2_BASE | GPIO_EN)
#define GPIO2_EN_SET							(GPIO2_BASE | GPIO_EN_SET)
#define GPIO2_EN_CLR							(GPIO2_BASE | GPIO_EN_CLR)

#define GPIO2_OE								(GPIO2_BASE | GPIO_OE)
#define GPIO2_OE_SET							(GPIO2_BASE | GPIO_OE_SET)
#define GPIO2_OE_CLR							(GPIO2_BASE | GPIO_OE_CLR)

#define GPIO2_OUT								(GPIO2_BASE | GPIO_OUT)
#define GPIO2_OUT_SET						(GPIO2_BASE | GPIO_OUT_SET)
#define GPIO2_OUT_CLR						(GPIO2_BASE | GPIO_OUT_CLR)

#define GPIO2_IN								(GPIO2_BASE | GPIO_IN)

#define GPIO2_IMOD							(GPIO2_BASE | GPIO_IMOD)
#define GPIO2_IPOL							(GPIO2_BASE | GPIO_IPOL)
#define GPIO2_ISEL							(GPIO2_BASE | GPIO_ISEL)

#define GPIO2_IEN								(GPIO2_BASE | GPIO_IEN)
// *** ZA9L ERATA - not implemented	#define GPIO2_IEN_SET						(GPIO2_BASE | GPIO_IEN_SET)
// *** ZA9L ERATA - not implemented	#define GPIO2_IEN_CLR						(GPIO2_BASE | GPIO_IEN_CLR)

#define GPIO2_IAST							(GPIO2_BASE | GPIO_IAST)
#define GPIO2_IBST							(GPIO2_BASE | GPIO_IBST)
#define GPIO2_ICLR							(GPIO2_BASE | GPIO_ICLR)

#define GPIO2_WKEN							(GPIO2_BASE | GPIO_WKEN)
// *** ZA9L ERATA - not implemented	#define GPIO2_WKEN_SET						(GPIO2_BASE | GPIO_WKEN_SET)
// *** ZA9L ERATA - not implemented	#define GPIO2_WKEN_CLR						(GPIO2_BASE | GPIO_WKEN_CLR)



#endif
