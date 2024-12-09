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
#ifndef _ZA9_MCR_H_
#define _ZA9_MCR_H_

#include "za9_defines.h"


/*
 * MCR Registers
 */
#define MCR_CTRL_REG    		(MCR_BASE | 0x000)
#define MCR_INT_REG     		(MCR_BASE | 0x004)
#define MCR_TMR_REG     		(MCR_BASE | 0x008)
#define MCR_FIFO_REG    		(MCR_BASE | 0x00C)
#define MCR_ADC_REG     		(MCR_BASE | 0x010)
#define MCR0_DCO_REG    		(MCR_BASE | 0x014)
#define MCR1_DCO_REG    		(MCR_BASE | 0x018)
#define MCR2_DCO_REG    		(MCR_BASE | 0x01C)
#define MCR0_THRS_REG   		(MCR_BASE | 0x020)
#define MCR1_THRS_REG   		(MCR_BASE | 0x024)
#define MCR2_THRS_REG   		(MCR_BASE | 0x028)
#define MCR_AUX_ADC_REG  		(MCR_BASE | 0x02C)

/*
 * MCR_CTRL_REG bit definitions
 */
#define MCR_FIFO_COUNT_MASK	(BIT27 | BIT26 | BIT25 | BIT24)
#define CARD_TO_INT_EN      	BIT23
#define AUX_ADC_INT_EN      	BIT22
#define FIFO_UDF_INT_EN     	BIT21
#define FIFO_OVF_INT_EN     	BIT20
#define FIFO_LVL_INT_EN     	BIT19
#define MCR2_TO_INT_EN			BIT18
#define MCR1_TO_INT_EN			BIT17
#define MCR0_TO_INT_EN			BIT16
#define MCR_BYPASS				BIT15
#define TRK2_ADC_MODE			BIT12
#define TRK2_PEAK_MODE      	0
#define TRK1_ADC_MODE			BIT11
#define TRK1_PEAK_MODE      	0
#define TRK0_ADC_MODE			BIT10
#define TRK0_PEAK_MODE      	0
#define MCR_SW_RST				BIT9
#define TRSH_MODE_DIV16     	(3 << 7)
#define TRSH_MODE_DIV8      	(2 << 7)
#define TRSH_MODE_DIV4      	(1 << 7)
#define TRSH_MODE_STATIC    	(0 << 7)
#define MCR_DMA_EN          	BIT6
#define MCR_FIFO_LVL_8			(7 << 3)
#define MCR_FIFO_LVL_7			(6 << 3)
#define MCR_FIFO_LVL_6			(5 << 3)
#define MCR_FIFO_LVL_5			(4 << 3)
#define MCR_FIFO_LVL_4			(3 << 3)
#define MCR_FIFO_LVL_3			(2 << 3)
#define MCR_FIFO_LVL_2			(1 << 3)
#define MCR_FIFO_LVL_1			(0 << 3)
#define MCR2_EN					BIT2
#define MCR1_EN					BIT1
#define MCR0_EN					BIT0


/*
 * MCR_INT_REG bit definitions
 */
#define MCR_GLBL_TO_ISTA		BIT23
#define MCR_AUX_ADC_ISTA		BIT22
#define MCR_UDFLO_ISTA			BIT21
#define MCR_OVFLO_ISTA			BIT20
#define MCR_LVL_ISTA				BIT19
#define MCR_MCR2_TO_ISTA		BIT18
#define MCR_MCR1_TO_ISTA		BIT17
#define MCR_MCR0_TO_ISTA		BIT16
#define MCT_INT_STA				BIT0


/*
 * MCR_TMR_REG bit definitions
 */
#define MCR_MIN_DELTA_MASK		(0xFFF << 16)
#define MCR_MAX_DELTA_MASK		(0xFFF << 0)


/*
 * MCR_FIFO_REG bit definitions
 * Note on AA silicon the track numbers are incorrect if multiple tracks enabled.
 */
#define MCR_PEAK_AMP_MASK		(0xFFF << 16)
#define MCR_TO						BIT15
#define MCR_POL					BIT14
#define MCR_FIFO_TRK_MASK		(3 << 12)
#define MCR_FIFO_TRK2			(2 << 12)
#define MCR_FIFO_TRK1			(1 << 12)
#define MCR_FIFO_TRK0			(0 << 12)
#define MCR_DELTA_ADC_MASK		(0xFFF << 0)


/*
 * MCR_ADC_REG bit definitions
 */
#define MCR_REF_CAL_MASK		(7 << 13)
#define MCR_REF_CAL_500			(0 << 13)
#define MCR_REF_CAL_400			(1 << 13)
#define MCR_REF_CAL_300			(2 << 13)
#define MCR_REF_CAL_250			(3 << 13)
#define MCR_REF_CAL_200			(4 << 13)
#define MCR_REF_CAL_150			(5 << 13)
#define MCR_REF_CAL_100			(6 << 13)
#define MCR_REF_CAL_050			(7 << 13)
#define MCR_ADC_RST				BIT12
#define MCR_ADC_CLK_DIV_MASK	0xFFF


/*
 * MCR2_DCO_REG
 * MCR1_DCO_REG bit definitions
 * MCR0_DCO_REG
 */
#define MCR_DC_OFFSET_MASK		0xFFF


/*
 * MCR2_THRS_REG
 * MCR1_THRS_REG bit definitions
 * MCR0_THRS_REG
 */
#define MCR_NEG_THRESH_MASK	(0xFFF << 16)
#define MCR_POS_THRESH_MASK	(0xFFF << 0)


/*
 * MCR_AUX_ADC_REG bit definitions
 */
#define MCR_AUX_ADC_TRK2		(2 << 16)
#define MCR_AUX_ADC_TRK1		(1 << 16)
#define MCR_AUX_ADC_TRK0		(0 << 16)
#define MCR_AUX_ADC_NEW_SAMP	BIT15
#define MCR_AUX_ADC_OVFL		BIT14
#define MCR_AUX_ADC_SAMP_MASK 0xFFF



#endif

