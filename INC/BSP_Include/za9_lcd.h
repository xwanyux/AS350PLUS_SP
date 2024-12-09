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
#ifndef _ZA9_LCD_H_
#define _ZA9_LCD_H_

#include "za9_defines.h"


/*
 * LCD Registers
 */
#define LCD_CTRL_REG    		(LCD_BASE | 0x000)
#define LCD_RD_REG      		(LCD_BASE | 0x004)
#define LCD_WR_REG      		(LCD_BASE | 0x008)

/*
 * LCD_CTRL_REG bit definitions
 */
#define LCD_EN_ACTIVE_MASK		(0xFF << 24)
#define LCD_EN_INACTIVE_MASK	(0xFF << 16)

#ifdef ZA9_AB_SILICON
	#define LCD_AD_SETUP_MASK		(0x07 << 12)
#else
	#define LCD_AD_SETUP_MASK		(0x07 << 12)
#endif
#define LCD_AD_SETUP_1_HCLK	(0x00 << 12)
#define LCD_AD_SETUP_2_HCLK	(0x01 << 12)
#define LCD_AD_SETUP_3_HCLK	(0x02 << 12)
#define LCD_AD_SETUP_4_HCLK	(0x03 << 12)
#define LCD_AD_SETUP_5_HCLK	(0x04 << 12)
#define LCD_AD_SETUP_6_HCLK	(0x05 << 12)
#define LCD_AD_SETUP_7_HCLK	(0x06 << 12)
#define LCD_AD_SETUP_8_HCLK	(0x07 << 12)
#ifndef ZA9_AB_SILICON
#define LCD_AD_SETUP_9_HCLK	(0x08 << 12)
#define LCD_AD_SETUP_10_HCLK	(0x09 << 12)
#define LCD_AD_SETUP_11_HCLK	(0x0A << 12)
#define LCD_AD_SETUP_12_HCLK	(0x0B << 12)
#define LCD_AD_SETUP_13_HCLK	(0x0C << 12)
#define LCD_AD_SETUP_14_HCLK	(0x0D << 12)
#define LCD_AD_SETUP_15_HCLK	(0x0E << 12)
#define LCD_AD_SETUP_16_HCLK	(0x0F << 12)
#endif

#define LCD_AD_HOLD_MASK		(0x07 << 8)
#define LCD_AD_HOLD_1_HCLK		(0x00 << 8)
#define LCD_AD_HOLD_2_HCLK		(0x01 << 8)
#define LCD_AD_HOLD_3_HCLK		(0x02 << 8)
#define LCD_AD_HOLD_4_HCLK		(0x03 << 8)
#define LCD_AD_HOLD_5_HCLK		(0x04 << 8)
#define LCD_AD_HOLD_6_HCLK		(0x05 << 8)
#define LCD_AD_HOLD_7_HCLK		(0x06 << 8)
#define LCD_AD_HOLD_8_HCLK		(0x07 << 8)
#ifndef ZA9_AB_SILICON
#define LCD_AD_HOLD_9_HCLK		(0x08 << 8)
#define LCD_AD_HOLD_10_HCLK	(0x09 << 8)
#define LCD_AD_HOLD_11_HCLK	(0x0A << 8)
#define LCD_AD_HOLD_12_HCLK	(0x0B << 8)
#define LCD_AD_HOLD_13_HCLK	(0x0C << 8)
#define LCD_AD_HOLD_14_HCLK	(0x0D << 8)
#define LCD_AD_HOLD_15_HCLK	(0x0E << 8)
#define LCD_AD_HOLD_16_HCLK	(0x0F << 8)
#endif

#define LCD_DMA_EN				BIT2
#define LCD_DATA_WIDTH_4		0
#define LCD_DATA_WIDTH_8		BIT1
#define LCD_EN_POL_LO 			0
#define LCD_EN_POL_HI			BIT0


/*
 * LCD_RD_REG and LCD_WR_REG bit definitions
 */
#define LCD_RD_CMD       		BIT9
#define LCD_WR_CMD				BIT9
#define LCD_RS_LO     			0
#define LCD_RS_HI     			BIT8
#define LCD_DATA_MASK			0xFF



#endif

