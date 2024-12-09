/*
 * private.h --
 *
 * ----------------------------------------------------------------------------
 * Copyright (c) 2012, Maxim Integrated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY MAXIM INTEGRATED ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* [INTERNAL] ------------------------------------------------------------------
 * Created on: Feb 06, 2012
 * Author: Yann G. (yann.gaude@maximintegrated.com)
 *
 * ---- Subversion keywords (need to set the keyword property)
 * $Revision:: $: Revision of last commit
 * $Author:: $: Author of last commit
 * $Date:: $: Date of last commit
 * [/INTERNAL] -------------------------------------------------------------- */

#ifndef _PRIVATE_H_
#define _PRIVATE_H_

/** Global includes */
#include <config.h>
/** Other includes */
/** Local includes */

#define	K_BLINK_LED_MAX_COUNT					0xffff
 #ifdef _SLOW_FLICKING_
#define K_BLINK_LED_ON_MAX_COUNT    ( 29 * K_BLINK_LED_MAX_COUNT )
#define K_BLINK_LED_OFF_MAX_COUNT   ( 31 * K_BLINK_LED_MAX_COUNT )
 #else
#define K_BLINK_LED_ON_MAX_COUNT    ( 5 * K_BLINK_LED_MAX_COUNT )
#define K_BLINK_LED_OFF_MAX_COUNT   ( 3 * K_BLINK_LED_MAX_COUNT )
 #endif /* _SLOW_FLICKING_ */

#define	MML_GPIO0_LED_PIN			21	// LED
//#define	MML_GPIO1_LCD_BL_EN_PIN			30	// LCD_BL_EN


/* Structures *****************************************************************/
typedef struct
{
	/** System frequency */
	unsigned int				system;
	/** AHB frequency */
	unsigned int				ahb;
	/** APB frequency */
	unsigned int				apb;

} t_cobra_frequencies;


/******************************************************************************/
int blink_led(void);

/** This functions writes a character data to the UART0 port */
int uart_write_char(const char c);

__COBRA_OPT_DEBUG__ void my_default_nmi_(void);

#endif /* _PRIVATE_H_ */

/******************************************************************************/
/* EOF */
