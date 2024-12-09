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
#ifndef _ZA9_TIMER_H_
#define _ZA9_TIMER_H_

/*
 * Timer register offsets
 */
#define TMR_CNT_REG			0x00
#define TMR_CMP_REG			0x04
#define TMR_PWM_REG			0x08
#define TMR_INT_REG			0x0C
#define TMR_CTL_REG			0x10



/*
 * TMR_CNT_REG bit definitions
 */
#define TMR_CNT_MASK			0xFFFF


/*
 * TMR_CMP_REG bit definitions
 */
#define TMR_CMP_MASK			0xFFFF


/*
 * TMR_PWM_REG bit definitions
 */
#define TMR_PWM_MASK			0xFFFF


/*
 * TMR_INT_REG bit definitions
 */
#define TMR_IRQ_CLR			0x01


/*
 * TMR_CTL_REG bit definitions
 */
#define TMR_TEN				0x0080
#define TMR_TPOL				0x0040
#define TMR_TPOL_HI			0x0040
#define TMR_TPOL_LO			0x0000
#define TMR_PRES_MASK		0x0138
#define TMR_MODE_MASK		0x0007

/*
 * Prescale bit definitions
 */
#define TMR_PRES_DIV_1		0x0000
#define TMR_PRES_DIV_2		0x0008
#define TMR_PRES_DIV_4		0x0010
#define TMR_PRES_DIV_8		0x0018
#define TMR_PRES_DIV_16		0x0020
#define TMR_PRES_DIV_32		0x0028
#define TMR_PRES_DIV_64		0x0030
#define TMR_PRES_DIV_128	0x0038
#define TMR_PRES_DIV_256	0x0100
#define TMR_PRES_DIV_512	0x0108
#define TMR_PRES_DIV_1024	0x0110
#define TMR_PRES_DIV_2048	0x0118
#define TMR_PRES_DIV_4096	0x0120


/*
 * Mode bit definitions
 */
#define TMR_MODE_ONE_SHOT	0x00
#define TMR_MODE_RELOAD		0x01
#define TMR_MODE_COUNT		0x02
#define TMR_MODE_PWM			0x03
#define TMR_MODE_CAPTURE	0x04
#define TMR_MODE_COMPARE	0x05
#define TMR_MODE_GATED		0x06
#define TMR_MODE_CAPT_COMP	0x07



#endif 

