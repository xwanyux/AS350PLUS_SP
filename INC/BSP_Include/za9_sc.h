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
#ifndef _ZA9_SMC_H_
#define _ZA9_SMC_H_

#include "za9_defines.h"



/*
 * SmartCard (SMC) Registers shared by both controllers
 */
#define SMC_INT_STAT_REG		  			(SMC_BASE | 0x00)
#define SMC_INT_MASK_REG	   			(SMC_BASE | 0x04)
#define SMC_VCC_CFG_REG		   			(SMC_BASE | 0x08)
#define SMC_CLK_DIV_REG		   			(SMC_BASE | 0x0C)
#define SMC_DET_CMD_REG		   			(SMC_BASE | 0x10)
#define SMC_SPI_DATA_REG	   			(SMC_BASE | 0x14)
#define SMC_SPICLKDIV_REG	   			(SMC_BASE | 0x18)
#define SMC_SCSTATUS_REG	   			(SMC_BASE | 0x1C)
#define SMC_SPIDMASEL_REG	   			(SMC_BASE | 0x24)


/*
 * Smart Card Controller register offsets
 */
#define SMC_RBR_THR							0x00
#define SMC_IER								0x04
#define SMC_IIR								0x08
#define SMC_LCR								0x0C
#define SMC_BRR								0x10
#define SMC_LSR								0x14
#define SMC_GR									0x18
#define SMC_GTIME								0x1C
#define SMC_NUM_REP							0x20
#define SMC_REV_DLY							0x24
#define SMC_CLK_DIV							0x28
#define SMC_NUM_TX							0x2C
#define SMC_NUM_PE							0x30
#define SMC_NUM_RX							0x34
#define SMC_PARITY							0x38

#define SMC_COMMAND							0x40
#define SMC_INT_STAT							0x44
#define SMC_INT_EN							0x48
#define SMC_TO_EN								0x4C
#define SMC_RST_LEN							0x60
#define SMC_ATR_MIN_DLY						0x64
#define SMC_ATR_MAX_DLY						0x68
#define SMC_ATR_MAX_LEN						0x6C
#define SMC_WWT_TO							0x70
#define SMC_CWT_TO							0x74
#define SMC_BWT_TO							0x78
#define SMC_BGT_TO							0x7C
#define SMC_DEAC_DLY							0x80


/*
 * SmartCard0 Registers
 */
#define SMC0_BASE								(SMC_BASE | 0x100)
#define SMC0_RBR_THR_REG					(SMC0_BASE | SMC_RBR_THR)
#define SMC0_IER_REG							(SMC0_BASE | SMC_IER)
#define SMC0_LCR_REG							(SMC0_BASE | SMC_LCR)
#define SMC0_BRR_REG							(SMC0_BASE | SMC_BRR)
#define SMC0_LSR_REG							(SMC0_BASE | SMC_LSR)
#define SMC0_GR_REG							(SMC0_BASE | SMC_GR)
#define SMC0_GTIME_REG						(SMC0_BASE | SMC_GTIME)
#define SMC0_NUM_REP_REG					(SMC0_BASE | SMC_NUM_REP)
#define SMC0_REV_DLY_REG					(SMC0_BASE | SMC_REV_DLY)
#define SMC0_CLK_DIV_REG					(SMC0_BASE | SMC_CLK_DIV)
#define SMC0_NUM_TX_REG						(SMC0_BASE | SMC_NUM_TX)
#define SMC0_NUM_PE_REG						(SMC0_BASE | SMC_NUM_PE)
#define SMC0_NUM_RX_REG						(SMC0_BASE | SMC_NUM_RX)
#define SMC0_PARITY_REG						(SMC0_BASE | SMC_PARITY)

#define SMC0_COMMAND_REG					(SMC0_BASE | SMC_COMMAND)
#define SMC0_INT_STAT_REG					(SMC0_BASE | SMC_INT_STAT)
#define SMC0_INT_EN_REG						(SMC0_BASE | SMC_INT_EN)
#define SMC0_TO_EN_REG						(SMC0_BASE | SMC_TO_EN)
#define SMC0_RST_LEN_REG					(SMC0_BASE | SMC_RST_LEN)
#define SMC0_ATR_MIN_DLY_REG				(SMC0_BASE | SMC_ATR_MIN_DLY)
#define SMC0_ATR_MAX_DLY_REG				(SMC0_BASE | SMC_ATR_MAX_DLY)
#define SMC0_ATR_MAX_LEN_REG				(SMC0_BASE | SMC_ATR_MAX_LEN)
#define SMC0_WWT_TO_REG						(SMC0_BASE | SMC_WWT_TO)
#define SMC0_CWT_TO_REG						(SMC0_BASE | SMC_CWT_TO)
#define SMC0_BWT_TO_REG						(SMC0_BASE | SMC_BWT_TO)
#define SMC0_BGT_TO_REG						(SMC0_BASE | SMC_BGT_TO)
#define SMC0_DEAC_DLY_REG					(SMC0_BASE | SMC_DEAC_DLY)


/*
 * SmartCard1 Registers
 */
#define SMC1_BASE								(SMC_BASE | 0x200)
#define SMC1_RBR_THR_REG					(SMC1_BASE | SMC_RBR_THR)
#define SMC1_IER_REG							(SMC1_BASE | SMC_IER)
#define SMC1_LCR_REG							(SMC1_BASE | SMC_LCR)
#define SMC1_BRR_REG							(SMC1_BASE | SMC_BRR)
#define SMC1_LSR_REG							(SMC1_BASE | SMC_LSR)
#define SMC1_GR_REG							(SMC1_BASE | SMC_GR)
#define SMC1_GTIME_REG						(SMC1_BASE | SMC_GTIME)
#define SMC1_NUM_REP_REG					(SMC1_BASE | SMC_NUM_REP)
#define SMC1_REV_DLY_REG					(SMC1_BASE | SMC_REV_DLY)
#define SMC1_CLK_DIV_REG					(SMC1_BASE | SMC_CLK_DIV)
#define SMC1_NUM_TX_REG						(SMC1_BASE | SMC_NUM_TX)
#define SMC1_NUM_PE_REG						(SMC1_BASE | SMC_NUM_PE)
#define SMC1_NUM_RX_REG						(SMC1_BASE | SMC_NUM_RX)
#define SMC1_PARITY_REG						(SMC1_BASE | SMC_PARITY)

#define SMC1_COMMAND_REG					(SMC1_BASE | SMC_COMMAND)
#define SMC1_INT_STAT_REG					(SMC1_BASE | SMC_INT_STAT)
#define SMC1_INT_EN_REG						(SMC1_BASE | SMC_INT_EN)
#define SMC1_TO_EN_REG						(SMC1_BASE | SMC_TO_EN)
#define SMC1_RST_LEN_REG					(SMC1_BASE | SMC_RST_LEN)
#define SMC1_ATR_MIN_DLY_REG				(SMC1_BASE | SMC_ATR_MIN_DLY)
#define SMC1_ATR_MAX_DLY_REG				(SMC1_BASE | SMC_ATR_MAX_DLY)
#define SMC1_ATR_MAX_LEN_REG				(SMC1_BASE | SMC_ATR_MAX_LEN)
#define SMC1_WWT_TO_REG						(SMC1_BASE | SMC_WWT_TO)
#define SMC1_CWT_TO_REG						(SMC1_BASE | SMC_CWT_TO)
#define SMC1_BWT_TO_REG						(SMC1_BASE | SMC_BWT_TO)
#define SMC1_BGT_TO_REG						(SMC1_BASE | SMC_BGT_TO)
#define SMC1_DEAC_DLY_REG					(SMC1_BASE | SMC_DEAC_DLY)
#define SMC1_SIM_SEL_REG					(SMC1_BASE | 0x84)



/*
 * SMC_INT_STAT_REG bit definitions
 */
#define SMC_ALRM_TRIG						BIT4
#define SMC_ALRM								BIT3
#define SMC_SC1_INT							BIT2
#define SMC_SC0_INT							BIT1
#define SMC_SPI_INT							BIT0

/*
 * SMC_INT_MASK_REG bit definitions
 */
#define SMC_ALRM_TRIG_MASK					BIT4
#define SMC_ALRM_MASK						BIT3
#define SMC_SPI_INT_MASK					BIT0


/*
 * SMC_VCC_CFG_REG bit definitions
 */
#define SMC_SC1_SIM4_0V						(0<<8)
#define SMC_SC1_SIM4_1_8V					(1<<8)
#define SMC_SC1_SIM4_3V						(2<<8)
#define SMC_SC1_SIM4_5V						(3<<8)

#define SMC_SC1_SIM3_0V						(0<<6)
#define SMC_SC1_SIM3_1_8V					(1<<6)
#define SMC_SC1_SIM3_3V						(2<<6)
#define SMC_SC1_SIM3_5V						(3<<6)

#define SMC_SC1_SIM2_0V						(0<<4)
#define SMC_SC1_SIM2_1_8V					(1<<4)
#define SMC_SC1_SIM2_3V						(2<<4)
#define SMC_SC1_SIM2_5V						(3<<4)

#define SMC_SC1_SIM1_0V						(0<<2)
#define SMC_SC1_SIM1_1_8V					(1<<2)
#define SMC_SC1_SIM1_3V						(2<<2)
#define SMC_SC1_SIM1_5V						(3<<2)

#define SMC_SC0_MAIN_0V						(0<<0)
#define SMC_SC0_MAIN_1_8V					(1<<0)
#define SMC_SC0_MAIN_3V						(2<<0)
#define SMC_SC0_MAIN_5V						(3<<0)


/*
 * SMC_CLK_DIV_REG bit definitions
 */
#define SMC_SC1_SIM4_NO_CLOCK				(0<<8)
#define SMC_SC1_SIM4_CLK_DIV_1			(1<<8)
#define SMC_SC1_SIM4_CLK_DIV_2			(2<<8)
#define SMC_SC1_SIM4_CLK_DIV_4			(3<<8)

#define SMC_SC1_SIM3_NO_CLOCK				(0<<6)
#define SMC_SC1_SIM3_CLK_DIV_1			(1<<6)
#define SMC_SC1_SIM3_CLK_DIV_2			(2<<6)
#define SMC_SC1_SIM3_CLK_DIV_4			(3<<6)

#define SMC_SC1_SIM2_NO_CLOCK				(0<<4)
#define SMC_SC1_SIM2_CLK_DIV_1			(1<<4)
#define SMC_SC1_SIM2_CLK_DIV_2			(2<<4)
#define SMC_SC1_SIM2_CLK_DIV_4			(3<<4)

#define SMC_SC1_SIM1_NO_CLK				(0<<2)
#define SMC_SC1_SIM1_CLK_DIV_1			(1<<2)
#define SMC_SC1_SIM1_CLK_DIV_2			(2<<2)
#define SMC_SC1_SIM1_CLK_DIV_4			(3<<2)

#define SMC_SC0_MAIN_NO_CLOCK				(0<<0)
#define SMC_SC0_MAIN_CLK_DIV_1			(1<<0)
#define SMC_SC0_MAIN_CLK_DIV_2			(2<<0)
#define SMC_SC0_MAIN_CLK_DIV_4			(3<<0)


/*
 * SMC_DET_CMD_REG bit definitions
 */
#define SMC_DSC1_SIM4						BIT4
#define SMC_DSC1_SIM3						BIT3
#define SMC_DSC1_SIM2						BIT2
#define SMC_DSC1_SIM1						BIT1
#define SMC_DSC0_MAIN						BIT0


/*
 * SMC_SPI_DATA_REG bit definitions
 */
#define SMC_BP_DONE							BIT8
#define SMC_SPI_DATA_MASK					0xFF


/*
 * SMC_SPICLKDIV_REG bit definitions
 */
#define SMC_SPI_CLK_DIV_MASK				0x0F


/*
 * SMC_SCSTATUS_REG bit definitions
 */
#define SMC_SIM4_RESET						BIT9
#define SMC_SIM4_VCC							BIT8
#define SMC_SIM3_RESET						BIT7
#define SMC_SIM3_VCC							BIT6
#define SMC_SIM2_RESET						BIT5
#define SMC_SIM2_VCC							BIT4
#define SMC_SIM1_RESET						BIT3
#define SMC_SIM1_VCC							BIT2
#define SMC_MAIN_RESET						BIT1
#define SMC_MAIN_VCC							BIT0


/*
 * SMC_SPIDMASEL_REG bit definitions
 */
#define SMC_CSR								BIT5
#define SMC_CSM								BIT4
#define SMC_SC1_RX							BIT3
#define SMC_SC0_RX							BIT2
#define SMC_SC1_TX							BIT1
#define SMC_SC0_TX							BIT0


/*
 * SMC_RBR_THR bit definitions
 */
#define SMC_RBR_THR_MASK					0xFF


/*
 * SMC_IER bit definitions
 */
#define SMC_LS_EN								BIT2
#define SMC_THRE_EN							BIT1
#define SMC_DR_EN								BIT0


/*
 * SMC_IIR bit definitions
 */
#define SMC_IIR_MASK							(BIT2 | BIT1)
#define SMC_IIR_THRE							BIT1
#define SMC_IIR_DR							BIT2
#define SMC_IIR_LS							(BIT2 | BIT1)
#define SMC_IIR_NO_INT						BIT0


/*
 * SMC_LCR bit definitions
 */
#define SMC_LCR_CNV_DIRECT					0
#define SMC_LCR_CNV_INVERSE				BIT7
#define SMC_LCR_REP_IEN						BIT6
#define SMC_LCR_PAR_STKY					BIT5
#define SMC_LCR_PAR_EVEN					BIT4
#define SMC_LCR_PAR_ODD						0
#define SMC_LCR_PAR_EN						BIT3
#define SMC_LCR_STOP_1						0
#define SMC_LCR_STOP_GUARD					BIT2
#define SMC_LCR_DATA_MASK					(BIT1 | BIT0)
#define SMC_LCR_DATA_5						0
#define SMC_LCR_DATA_6						BIT0
#define SMC_LCR_DATA_7						BIT1
#define SMC_LCR_DATA_8						(BIT1 | BIT0)


/*
 * SMC_BRR bit definitions
 */
#define SMC_BRR_TUNE_MASK					0x3FF0
#define SMC_BRR_FINE_MASK					0x000F


/*
 * SMC_LSR bit definitions
 */
#define SMC_LSR_TEMT							BIT6
#define SMC_LSR_THRE							BIT5
#define SMC_LSR_FE							BIT#
#define SMC_LSR_PE							BIT2
#define SMC_LSR_OE							BIT1
#define SMC_LSR_DR							BIT0


/*
 * SMC_GR bit definitions
 */
#define SMC_GR_WWT_LATE						0
#define SMC_GR_WWT_EARLY					BIT7
#define SMC_GR_T1_EN							BIT6
#define SMC_GR_RST_REP_CNT					BIT5
#define SMC_GR_AUTO_RX_TO_TX				BIT4
#define SMC_GR_AUTO_TX_TO_RX				BIT3
#define SMC_GR_AUTO_SWT						BIT2
#define SMC_GR_SW_RST						BIT1
#define SMC_GR_DIR_TX						0
#define SMC_GR_DIR_RX						BIT0


/*
 * SMC_GTIME bit definitions
 */
#define SMC_GTIME_MASK						0xFF


/*
 * SMC_NUM_REP bit definitions
 */
#define SMC_NUM_REP_MASK					0x0F


/*
 * SMC_REV_DLY bit definitions
 */
#define SMC_REV_DLY_MASK					0xFF


/*
 * SMC_CLK_DIV bit definitions
 */
#define SMC_CLK_DIV_IF_MASK				0xFF00
#define SMC_CLK_DIV_CARD_MASK				0x00FF


/*
 * SMC_NUM_TX bit definitions
 */
#define SMC_NUM_TX_MASK						0xFFFF


/*
 * SMC_NUM_PE bit definitions
 */
#define SMC_NUM_PE_MASK						0xFFFF


/*
 * SMC_NUM_RX bit definitions
 */
#define SMC_NUM_RX_MASK						0xFFFF


/*
 * SMC_PARITY bit definitions
 */
#define SMC_PARITY_AUTO_EN					BIT8
#define SMC_PARITY_DATA_MASK				0xFF


/*
 * SMC_COMMAND bit definitions
 */
#define SMC_CMD_ATR_TIM_DISABLE			BIT7
#define SMC_CMD_SEND_CLK					BIT6
#define SMC_CMD_TO_EN						BIT5
#define SMC_CMD_ATR_END						BIT4
#define SMC_CMD_STOP_CARD_CLOCK			BIT3
#define SMC_CMD_DEAC_CARD					BIT2
#define SMC_CMD_RESET_CARD					BIT1
#define SMC_CMD_POWER_CARD					BIT0


/*
 * SMC_INT_STAT bit definitions
 */
#define SMC_INT_STA_ATR_OVERLAP			BIT15
#define SMC_INT_STA_NB_RX					BIT14
#define SMC_INT_STA_NB_TX					BIT13
#define SMC_INT_STA_BGT_ERROR				BIT12
#define SMC_INT_STA_BWT_ERROR				BIT11
#define SMC_INT_STA_CWT_ERROR				BIT10
#define SMC_INT_STA_WWT_ERROR				BIT9
#define SMC_INT_STA_PHY_ERROR				BIT8
#define SMC_INT_STA_PARITY_ERROR			BIT7
#define SMC_INT_STA_FRAMING_ERROR		BIT6
#define SMC_INT_STA_OVERRUN_ERROR		BIT5
#define SMC_INT_STA_TX_HOLD_EMPTY		BIT4
#define SMC_INT_STA_TX_EMPTY				BIT3
#define SMC_INT_STA_RX_AVAIL				BIT2
#define SMC_INT_STA_CARD_REMOVED			BIT1
#define SMC_INT_STA_CARD_INSERTED		BIT0


/*
 * SMC_INT_EN bit definitions
 */
#define SMC_INT_EN_NUM_RX					BIT14
#define SMC_INT_EN_NUM_TX					BIT13
#define SMC_INT_EN_BGT_ERROR				BIT12
#define SMC_INT_EN_BWT_ERROR				BIT11
#define SMC_INT_EN_CWT_ERROR				BIT10
#define SMC_INT_EN_WWT_ERROR				BIT9
#define SMC_INT_EN_PHY_ERROR				BIT8
#define SMC_INT_EN_CARD_REMOVED			BIT1
#define SMC_INT_EN_CARD_INSERTED			BIT0


/*
 * SMC_TO_EN bit definitions
 */
#define SMC_TO_MASTER_INT_EN				BIT7
#define SMC_TO_EN_BGT						BIT3
#define SMC_TO_EN_BWT						BIT2
#define SMC_TO_EN_CWT						BIT1
#define SMC_TO_EN_WWT						BIT0


/*
 * SMC_RST_LEN bit definitions
 */
#define SMC_RST_LEN_MASK					0xFFFF


/*
 * SMC_ATR_MIN_DLY bit definitions
 */
#define SMC_ATR_MIN_DLY_MASK				0x3FFFF


/*
 * SMC_ATR_MAX_DLY bit definitions
 */
#define SMC_ATR_MAX_DLY_MASK				0x3FFFF


/*
 * SMC_ATR_MAX_LEN bit definitions
 */
#define SMC_ATR_MAX_LEN_MASK				0x3FFFF


/*
 * SMC_WWT_TO bit definitions
 */
#define SMC_WWT_TO_MASK						0x3FFFFF


/*
 * SMC_CWT_TO bit definitions
 */
#define SMC_CWT_TO_MASK						0x3FFFFF


/*
 * SMC_BWT_TO bit definitions
 */
#define SMC_BWT_TO_MASK						0x3FFFFFF


/*
 * SMC_BGT_TO bit definitions
 */
#define SMC_BGT_TO_MASK						0xFFFF


/*
 * SMC_DEAC_DLY bit definitions
 */
#define SMC_DEAC_DLY_MASK					0x3FFFFFF


/*
 * SMC1_SIM_SEL_REG bit definitions
 */
#define SMC_SIM_SEL_1						0x00
#define SMC_SIM_SEL_2						0x01
#define SMC_SIM_SEL_3						0x10
#define SMC_SIM_SEL_4						0x11



#endif

