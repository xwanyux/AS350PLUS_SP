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
#ifndef _ZA9_INT_H_
#define _ZA9_INT_H_

#include "za9_defines.h"


/*
 * INTC Registers
 */
#define INTC_EN_REG				(INTC_BASE | 0x0000)
#define INTC_ESET_REG			(INTC_BASE | 0x0004)
#define INTC_ECLR_REG			(INTC_BASE | 0x0008)
#define INTC_DFLT_REG			(INTC_BASE | 0x000C)
#define INTC_ISTA_REG			(INTC_BASE | 0x0010)
#define INTC_RSTA_REG			(INTC_BASE | 0x0014)
#define INTC_IDBG_REG			(INTC_BASE | 0x0018)
#define INTC_FDBG_REG			(INTC_BASE | 0x001C)
#define INTC_SW_INT_REG			(INTC_BASE | 0x0020)
#define INTC_SW_INT_SET_REG	(INTC_BASE | 0x0024)
#define INTC_SW_INT_CLR_REG	(INTC_BASE | 0x0028)
#define INTC_VEC_N_REG			(INTC_BASE | 0x0080)
#define INTC_CFG_N_REG			(INTC_BASE | 0x0100)
#define INTC_IVEC_REG			(INTC_BASE | 0x0F00)
#define INTC_FVEC_REG			(INTC_BASE | 0x0F04)
#define INTC_IEND_REG			(INTC_BASE | 0x0F08)
#define INTC_FEND_REG			(INTC_BASE | 0x0F0C)



/*
 * CFG_N_REG bit definitions
 */
#define INTC_FIRQ_MASK		0x08
#define INTC_FIQ				0x08
#define INTC_IRQ    			0x00
#define INTC_PRIO_MASK		0x07
#define INTC_PRIO_0			0x00
#define INTC_PRIO_1			0x01
#define INTC_PRIO_2			0x02
#define INTC_PRIO_3			0x03
#define INTC_PRIO_4			0x04
#define INTC_PRIO_5			0x05
#define INTC_PRIO_6			0x06
#define INTC_PRIO_7			0x07


/*
 * Interrupt Channel Numbers
 */
#define INTNUM_TIMER6   	0
#define INTNUM_WDT      	1
#define INTNUM_UART0    	2
#define INTNUM_TIMER0   	3
#define INTNUM_TIMER1   	4
#define INTNUM_TIMER2   	5
#define INTNUM_TIMER3   	6
#define INTNUM_TIMER4   	7
#define INTNUM_TIMER5   	8
#define INTNUM_SMC0     	9
#define INTNUM_SMC1     	10
#define INTNUM_SMC2     	11
#define INTNUM_GPIO0A   	12
#define INTNUM_GPIO0B   	13
#define INTNUM_GPIO1A   	14
#define INTNUM_GPIO1B   	15
#define INTNUM_GPIO2A   	16
#define INTNUM_GPIO2B   	17
#define INTNUM_DMAC     	18
#define INTNUM_MCR      	19
#define INTNUM_DAA      	20
#define INTNUM_SSD      	21
#define INTNUM_ADC      	22
#define INTNUM_SPI0     	23
#define INTNUM_SPI1     	24
#define INTNUM_UART1    	25
#define INTNUM_UART2    	26
#define INTNUM_HASH     	27
#define INTNUM_RNG      	28
#define INTNUM_RTC      	29
#define INTNUM_TIMER7   	30
#define INTNUM_TIMER8   	31



/*
 * Interrupt controller interrupt sources bit masks
 */
#define INT_MASK_ADC    	(1<<INTNUM_ADC)
#define INT_MASK_DMAC   	(1<<INTNUM_DMAC)
#define INT_MASK_GPIO0A 	(1<<INTNUM_GPIO0A)
#define INT_MASK_GPIO0B 	(1<<INTNUM_GPIO0B)
#define INT_MASK_GPIO1A 	(1<<INTNUM_GPIO1A)
#define INT_MASK_GPIO1B 	(1<<INTNUM_GPIO1B)
#define INT_MASK_GPIO2A 	(1<<INTNUM_GPIO2A)
#define INT_MASK_GPIO2B 	(1<<INTNUM_GPIO2B)
#define INT_MASK_MCR    	(1<<INTNUM_MCR)
#define INT_MASK_RTC    	(1<<INTNUM_RTC)
#define INT_MASK_RNG    	(1<<INTNUM_RNG)
#define INT_MASK_SPI0   	(1<<INTNUM_SPI0)
#define INT_MASK_SPT    	(1<<INTNUM_SPT)
#define INT_MASK_SMC0   	(1<<INTNUM_SMC0)
#define INT_MASK_SMC1   	(1<<INTNUM_SMC1)
#define INT_MASK_SMC2   	(1<<INTNUM_SMC2)
#define INT_MASK_TIMER0 	(1<<INTNUM_TIMER0)
#define INT_MASK_TIMER1 	(1<<INTNUM_TIMER1)
#define INT_MASK_TIMER2 	(1<<INTNUM_TIMER2)
#define INT_MASK_TIMER3 	(1<<INTNUM_TIMER3)
#define INT_MASK_TIMER4 	(1<<INTNUM_TIMER4)
#define INT_MASK_TIMER5 	(1<<INTNUM_TIMER5)
#define INT_MASK_UART0  	(1<<INTNUM_UART0)
#define INT_MASK_UART1  	(1<<INTNUM_UART1)
#define INT_MASK_UART2  	(1<<INTNUM_UART2)
#define INT_MASK_WDT    	(1<<INTNUM_WDT)
#define INT_MASK_DAA    	(1<<INTNUM_DAA)
#define INT_MASK_SSD    	(1<<INTNUM_SSD)



#endif 

