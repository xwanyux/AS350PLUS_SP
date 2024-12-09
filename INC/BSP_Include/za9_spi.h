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
#ifndef _ZA9_SPI_H_
#define _ZA9_SPI_H_

/*
 * SPI Register Offsets
 */
#define SPI_DATA_REG			0x000
#define SPI_CTRL_REG			0x004
#define SPI_STA_REG			0x008
#define SPI_MOD_REG			0x00C
#define SPI_DIAG_REG			0x010
#define SPI_BRG_REG			0x014
#define	SPI_DMA_REG			0x018

/*
 * SPI_CTL_REG bit definitions
 */
#define CTL_IRQE   			0x80
#define CTL_STR     			0x40
#define CTL_BIRQ				0x20
#define CTL_PHASE				0x10
#define CTL_CLKPOL			0x08
#define CTL_WOR				0x04
#define CTL_MMEN				0x02
#define CTL_SPIEN				0x01

/*
 * SPI_STA_REG bit definitions
 */
#define STA_IRQ				0x80
#define STA_OVR				0x40
#define STA_COL				0x20
#define STA_ABT				0x10
#define STA_TXST				0x02
#define STA_SLAS				0x01

/*
 * SPI_MOD_REG bit definitions
 */
#define MOD_DIAG				0x40
#define MOD_NUMBITS			0x3C
#define MOD_SSIO				0x02
#define MOD_SSV				0x01

/*
 * SPI_DIAG_REG bit definitions
 */
#define DIAG_SCKEN			0x80
#define DIAG_TCKEN			0x40
#define DIAG_SPISTATE		0x3F

/*
 * SPI_DMA_REG bit definitions
 */
#define RX_DMA_EN				BIT31
#define RX_FIFO_CNT			(BIT26 | BIT25 | BIT24)
#define RX_FIFO_CLR			BIT20
#define RX_FIFO_LEVEL		(BIT17 | BIT16)
#define TX_DMA_EN				BIT15
#define	TX_FIFO_CNT			(BIT10 | BIT9 | BIT8)
#define TX_FIFO_CLR			BIT4
#define TX_FIFO_LEVEL		(BIT1 | BIT0)
#define TX_FIFO_CNT_SHIFT	8
#define RX_FIFO_CNT_SHIFT	24
#define MAX_TX_CNT			0x04
 
#endif 

