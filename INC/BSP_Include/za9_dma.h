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
#ifndef _ZA9_DMA_H_
#define _ZA9_DMA_H_



/*
 * DMA registers 
 */
#define DMA_CTRL_REG      	(DMAC_BASE|0x000)
#define DMA_ISTA_REG       (DMAC_BASE|0x004)


/*
 * Base address for configuration of each channel
 */
#define DMA_CH0            	(DMAC_BASE|0x100)
#define DMA_CH1            	(DMAC_BASE|0x120)
#define DMA_CH2            	(DMAC_BASE|0x140)
#define DMA_CH3            	(DMAC_BASE|0x160)
#define DMA_CH4            	(DMAC_BASE|0x180)
#define DMA_CH5            	(DMAC_BASE|0x1A0)
#define DMA_CH6            	(DMAC_BASE|0x1C0)
#define DMA_CH7            	(DMAC_BASE|0x1E0)


/*
 * DMA Register offsets (from DMA_CHx base)
 */
#define DMA_CFG_REG       		0x00
#define DMA_STA_REG       		0x04
#define DMA_SRC_REG     		0x08
#define DMA_DST_REG       		0x0C
#define DMA_CNT_REG       		0x10
#define DMA_SRLD_REG      		0x14
#define DMA_DRLD_REG      		0x18
#define DMA_CRLD_REG      		0x1C  


/*
 * DMA_CFG_N_REG register bit fields
 */
#define CTZ_IEN               BIT31
#define EN_STA_IEN            BIT30
#define DST_AUTO_INC          BIT22
#define SRC_AUTO_INC          BIT18
#define DSTWIDTH_FIELD_BYTE	(0<<20)
#define DSTWIDTH_FIELD_HWORD  (1<<20)
#define DSTWIDTH_FIELD_WORD   (2<<20)
#define SRCWIDTH_FIELD_BYTE   (0<<16)
#define SRCWIDTH_FIELD_HWORD  (1<<16)
#define SRCWIDTH_FIELD_WORD  	(2<<16)

#define DMA_PS_SEL_MASK			(BIT15 | BIT14)
#define DMA_PS_SEL_DISABLE		(0<<14)
#define DMA_PS_SEL_256			(1<<14)
#define DMA_PS_SEL_64K			(2<<14)
#define DMA_PS_SEL_16M			(3<<14)

#define DMA_TO_SEL_MASK			(BIT13 | BIT12 | BIT11)
#define DMA_TO_SEL_3_4			(0<<11)
#define DMA_TO_SEL_7_8			(1<<11)
#define DMA_TO_SEL_15_16		(2<<11)
#define DMA_TO_SEL_31_32		(3<<11)
#define DMA_TO_SEL_63_64		(4<<11)
#define DMA_TO_SEL_127_128		(5<<11)
#define DMA_TO_SEL_255_256		(6<<11)
#define DMA_TO_SEL_511_512		(7<<11)

#define DMA_REQ_WAIT				BIT10

#define DMA_CH_EN           	BIT0

/*
 * DMA_STA_N_REG register bit fields
 */
#define DMA_STA_TO				BIT6
#define DMA_STA_OVFL				BIT5
#define DMA_STA_BUS_ERR			BIT4
#define DMA_STA_RLOAD			BIT3
#define DMA_STA_CTZ				BIT2
#define DMA_STA_PEND				BIT1
#define DMA_STA_EN				BIT0

/*
 * DMA_CRLD_N_REG reload enable bit
 */
#ifdef ZA9_AB_SILICON
	#define DMA_RLD_EN          BIT16
#else
	#define DMA_RLD_EN          BIT31
#endif



#endif 

