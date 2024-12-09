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
#ifndef _ZA9_ADC_H_
#define _ZA9_ADC_H_

#include "za9_defines.h"



/*
 * ADC Registers
 * ZA9L Errata - ADC_STA_REG does not exist
 */
#define ADC_CFG_REG      		  			(ADC_BASE | 0x00)
//#define ADC_STA_REG      		  			(ADC_BASE | 0x04)
#define ADC_CMD_REG      		  			(ADC_BASE | 0x04)
#define ADC_FIFO_REG     		  			(ADC_BASE | 0x08)
#define ADC_INT_REG      		  			(ADC_BASE | 0x0C)
#define ADC_ISTA_REG     		  			(ADC_BASE | 0x10)



/*
 * ADC Input identifiers
 * These bit patterns are used to select input channels in the sampling rotation
 * and identify the source of sampled data received through the FIFO.
 */
#define ADC_IN_0								0
#define ADC_IN_1								1
#define ADC_IN_2								2
#define ADC_IN_3								3
#define ADC_IN_4								4
#define ADC_IN_5								5
 

/*
 * ADC_CFG_REG bit definitions
 * Set the CFG_xx_MASK bits to the desired ADC_IN_x value
 */
#define ADC_CFG_H_MASK						(BIT31 | BIT30 | BIT29)
#define ADC_CFG_G_MASK						(BIT28 | BIT27 | BIT26)
#define ADC_CFG_F_MASK						(BIT25 | BIT24 | BIT23)
#define ADC_CFG_E_MASK						(BIT22 | BIT21 | BIT20)
#define ADC_CFG_D_MASK						(BIT19 | BIT18 | BIT17)
#define ADC_CFG_C_MASK						(BIT16 | BIT15 | BIT14)
#define ADC_CFG_B_MASK						(BIT13 | BIT12 | BIT11)
#define ADC_CFG_A_MASK						(BIT10 | BIT9 | BIT8)
#define ADC_CFG_H_SHIFT						29
#define ADC_CFG_G_SHIFT						26
#define ADC_CFG_F_SHIFT						23
#define ADC_CFG_E_SHIFT						20
#define ADC_CFG_D_SHIFT						17
#define ADC_CFG_C_SHIFT						14
#define ADC_CFG_B_SHIFT						11
#define ADC_CFG_A_SHIFT						8
#define ADC_CLOCK_DIV_MASK					0xFF


/*
 * ADC_STA_REG bit definitions
 * ZA9L Errata - ADC_STA_REG does not exist
 */
//#define ADC_STA_FIFO_UDRFLOW				BIT7
//#define ADC_STA_FIFO_OVRFLOW				BIT6
//#define ADC_STA_FIFO_EMPTY  				BIT5
//#define ADC_STA_FIFO_FULL   				BIT4
//#define ADC_STA_FIFO_COUNT_MASK			(BIT2 | BIT1 | BIT0)


/*
 * ADC_CMD_REG bit definitions
 */
#define ADC_CMD_ROT_A						(0<<4)
#define ADC_CMD_ROT_AB						(1<<4)
#define ADC_CMD_ROT_ABC						(2<<4)
#define ADC_CMD_ROT_ABCD					(3<<4)
#define ADC_CMD_ROT_ABCDE					(4<<4)
#define ADC_CMD_ROT_ABCDEF					(5<<4)
#define ADC_CMD_ROT_ABCDEFG				(6<<4)
#define ADC_CMD_ROT_ABCDEFGH				(7<<4)
#define ADC_CMD_CONT_SMPL					BIT2
#define ADC_CMD_SNGL_SMPL					BIT1
#define ADC_CMD_ANALOG_RST					BIT0


/*
 * ADC_FIFO_REG bit definitions
 */
#define ADC_FIFO_SMPL_PIN    				(BIT12 | BIT11 | BIT10)
#define ADC_SAMPLE_MASK       			0x3FF


/*
 * ADC_INT_REG bit definitions
 */
#define ADC_INT_GLBL_IE						BIT9
#define ADC_INT_FIFO_LEVEL_IE				BIT8
#define ADC_INT_FIFO_UDRFLOW_IE			BIT7
#define ADC_INT_FIFO_OVRFLOW_IE			BIT6
#define ADC_INT_FIFO_EMPTY_IE				BIT5
#define ADC_INT_FIFO_FULL_IE				BIT4
#define ADC_INT_DMA_REQ_EN					BIT3
#define ADC_INT_FIFO_LEVEL_MASK			0x07
#define ADC_FIFO_LVL_1						0
#define ADC_FIFO_LVL_2						1
#define ADC_FIFO_LVL_3						2
#define ADC_FIFO_LVL_4						3


/*
 * ADC_ISTA_REG bit definitions
 */
#define ADC_ISTA_GLBL						BIT9
#define ADC_ISTA_FIFO_LEVEL				BIT8
#define ADC_ISTA_FIFO_UDRFLOW				BIT7
#define ADC_ISTA_FIFO_OVRFLOW				BIT6
#define ADC_ISTA_FIFO_EMPTY				BIT5
#define ADC_ISTA_FIFO_FULL					BIT4
#define ADC_ISTA_FIFO_COUNT_MASK			(BIT2 | BIT1 | BIT0)


#endif

