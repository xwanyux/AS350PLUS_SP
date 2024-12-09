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
#ifndef _ZA9_PMU_H_
#define _ZA9_PMU_H_

#include "za9_defines.h"



/*
 * PMU Registers
 */
#define PMU_PLL_REG							(PMU_BASE | 0x00)
#define PMU_CLK_REG							(PMU_BASE | 0x04)
#define PMU_CKEN_REG							(PMU_BASE | 0x08)
#define PMU_RST_REG							(PMU_BASE | 0x0C)
#define PMU_ID0_REG							(PMU_BASE | 0x10)
#define PMU_ID1_REG							(PMU_BASE | 0x14)
#define PMU_TRIM_REG							(PMU_BASE | 0x18)
#define PMU_CFG_REG							(PMU_BASE | 0x1C)

#define PMU_OVCK_REG							(PMU_BASE | 0x30)


/*
 * PMU_PLL_REG bit definitions
 */
#define PLL_OUT_DIV_P_MASK					(BIT31 | BIT30 | BIT29 | BIT28)
#define PLL_IN_DIV_N_MASK					(BIT27 | BIT26 | BIT25 | BIT24)
#define PLL_MUL_M_MASK						(BIT23 | BIT22 | BIT21 | BIT20 | BIT19 | BIT18 | BIT17 | BIT16)
#define PLL_LOCK								BIT1
#define PLL_EN									BIT0


/*
 * PMU_CLK_REG bit definitions
 */
#define PMU_SRC_CLK_SEL						BIT30
#define PMU_DAA_CLK_EN						BIT22
#define PMU_SD_CLK_EN						BIT16
#define PMU_HCLK_DIVIDER_MASK				(BIT15 | BIT14 | BIT13 | BIT12 | BIT11 | BIT10 | BIT9 | BIT8)
#define PMU_FCLK_DIVIDER_MASK				(BIT4 | BIT3 | BIT2 | BIT1 | BIT0)


/*
 * PMU_CKEN_REG bit definitions
 */
#define PMU_CPU_CKEN							BIT31
#define PMU_SDRM_CKEN						BIT30
#define PMU_MEMC_CKEN						BIT29
#define PMU_SRAM_ROM_CKEN					BIT28
#define PMU_BRDG_CKEN						BIT27
#define PMU_INTC_CKEN						BIT26
#define PMU_RNG_CKEN							BIT25
#define PMU_HASH_CKEN						BIT24
#define PMU_GPIO2_CKEN						BIT23
#define PMU_GPIO1_CKEN						BIT22
#define PMU_GPIO0_CKEN						BIT21
#define PMU_DMA_CKEN							BIT20
#define PMU_MCR_CKEN							BIT19
#define PMU_ADC_CKEN							BIT18
#define PMU_DAA_CKEN							BIT17
#define PMU_SMC_CKEN							BIT16
#define PMU_SPI1_CKEN						BIT15
#define PMU_SPI0_CKEN						BIT14
#define PMU_LCD_CKEN							BIT13
#define PMU_WDOG_CKEN						BIT12
#define PMU_TMR8_CKEN						BIT11
#define PMU_TMR7_CKEN						BIT10
#define PMU_TMR6_CKEN						BIT9
#define PMU_TMR5_CKEN						BIT8
#define PMU_TMR4_CKEN						BIT7
#define PMU_TMR3_CKEN						BIT6
#define PMU_TMR2_CKEN						BIT5
#define PMU_TMR1_CKEN						BIT4
#define PMU_TMR0_CKEN						BIT3
#define PMU_UART2_CKEN						BIT2
#define PMU_UART1_CKEN						BIT1
#define PMU_UART0_CKEN						BIT0


/*
 * PMU_RST_REG bit definitions
 */
#define PMU_MFG_ACC_DSBL					BIT19
#define PMU_MFG_ACC_STA						BIT18
#define PMU_PERI_RST							BIT17
#define PMU_SOFT_RST							BIT16
#define PMU_SOFT_RST_FLAG					BIT5
#define PMU_TAMP_RST_FLAG					BIT4
#define PMU_WDOG_RST_FLAG					BIT3
#define PMU_RST_PIN_FLAG					BIT2
#define PMU_VOVR_RST_FLAG					BIT1
#define PMU_POR_RST_FLAG					BIT0


/*
 * PMU_ID0_REG bit definitions
 */
#define PMU_CUST_ID_MASK					0xFF000000
#define PMU_SERIAL_NUMBER_MASK			0x00FFFFFF


/*
 * PMU_ID1_REG bit definitions
 */
#define PMU_PART_NUMBER_MASK				(BIT15 | BIT14 | BIT13 | BIT12 | BIT11 | BIT10)
#define PMU_PKG_TYPE_MASK					(BIT9 | BIT8)
#define PMU_VERSION_MASK					0xFF


/*
 * PMU_TRIM_REG bit definitions
 */
#define PMU_TRIM_MASK						0xFFFF0000
#define PMU_CCO_TRIM_MASK					(BIT31 | BIT30 | BIT29 | BIT28 | BIT27)
#define PMU_TEMP_TRIM_MASK					(BIT26 | BIT25 | BIT24)
#define PMU_IBG_TRIM_MASK					(BIT23 | BIT22 | BIT21 | BIT20)
#define PMU_VBG_TRIM_MASK					(BIT19 | BIT18 | BIT17 | BIT16)
#define PMU_NUM_KEYS_MASK					(BIT3 | BIT2 | BIT1 | BIT0)


/*
 * PMU_CFG_REG bit definitions
 */
#define PMU_DAA_PEN 							BIT31
#define PMU_ADC_PEN  						BIT30
#define PMU_MCR2_PEN 						BIT29
#define PMU_MCR1_PEN     					BIT28
#define PMU_MCR0_PEN 						BIT27
#define PMU_MCR_ADC_PEN						BIT26
#define PMU_MCR_VREF_PEN					BIT25
#define PMU_MCR_IREF_PEN					BIT24
#define PMU_GPIO_WAKE						BIT17
#define PMU_RTC_WAKE							BIT16
#define PMU_SDRAM_REMAP						BIT0


/*
 * PMU_OVCK_REG bit definitions
 */
#define PMU_OVCK_LIMIT_MASK				0xFFF00000
#define PMU_OVCK_RST_DSBL					BIT19
#define PMU_OVCK_DET_EN						BIT18
#define PMU_LOCK_RST_DSBL					BIT17
#define PMU_LOCK_FAIL_EN					BIT16
#define PMU_OVCK_FAIL_FLAG					BIT1
#define PMU_LOCK_FAIL_FLAG					BIT0



#endif

