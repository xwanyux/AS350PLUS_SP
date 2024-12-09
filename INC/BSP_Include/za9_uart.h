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
#ifndef _ZA9_UART_H_
#define _ZA9_UART_H_



/*
 * UART register offsets
 */
#define UART_REG_BRG_L 		0x00000000  // baud rate gen L
#define UART_REG_BRG_H 		0x00000004  // baud rate gen H
#define UART_REG_THR   		0x00000000  // transmit holding register 
#define UART_REG_RBR   		0x00000000  // receive buffer register
#define UART_REG_IER   		0x00000004  // interrupt enable register
#define UART_REG_IIR   		0x00000008  // interrupt identification register
#define UART_REG_FCTL  		0x00000008  // fifo control register
#define UART_REG_LCTL  		0x0000000C  // line control register
#define UART_REG_MCTL  		0x00000010  // modem control register
#define UART_REG_LSR   		0x00000014  // line status register
#define UART_REG_MSR   		0x00000018  // modem status register
#define UART_REG_SPR   		0x0000001C  // scratch pad register
#define UART_REG_IRCTL 		0x00000020  // IR control register (new)



/*
 * UART_REG_IER bit definitions
 */
#define RIE  					0x01
#define TIE  					0x02
#define LSIE 					0x04
#define MIIE 					0x08
#define TCIE 					0x10


/*
 * UART_REG_IIR bit definitions
 */
#define IIR_INT_STATUS_MSK	0x0E
#define IIR_RX_INT         0x04
#define IIR_TX_INT         0x02
#define IIR_RXTO_INT       0x0C
#define IIR_MODEM_STATUS   0x00
#define IIR_LINE_STATUS    0x06


/*
 * UART_REG_FCTL bit definitions
 */
#define FCTL_TRG1      		0x00
#define FCTL_TRG4      		0x40
#define FCTL_TRG8      		0x80
#define FCTL_TRG14     		0xC0
#define FCTL_CLRTXF    		0x04
#define FCTL_CLRRXF    		0x02
#define FCTL_FIFOEN    		0x01


/*
 * UART_REG_LCTL bit definitions
 */
#define LCTL_PARITY_NONE	0x00
#define LCTL_PARITY_EVEN   0x18
#define LCTL_PARITY_ODD    0x08

#define LCTL_DATA_BITS_5	0x00
#define LCTL_DATA_BITS_6   0x01
#define LCTL_DATA_BITS_7   0x02
#define LCTL_DATA_BITS_8   0x03

#define LCTL_STOP_BITS_1   0x00
#define LCTL_STOP_BITS_2   0x04


/*
 * UART_REG_MCTL bit definitions
 */
#define MCTL_RTS    			(1<<1)
#define MCTL_DTR    			(1<<0)
#define MCTL_OUT1   			(1<<2)
#define MCTL_OUT2   			(1<<3)
#define MCTL_LOOP	  			(1<<4)


/*
 * UART_REG_LSR bit definitions
 */
#define LSR_ERR     			0x80
#define LSR_TEMT    			0x40
#define LSR_THRE    			0x20
#define LSR_BI      			0x10
#define LSR_FE      			0x08
#define LSR_PE      			0x04
#define LSR_OE      			0x02
#define LSR_DR      			0x01


/*
 * UART_REG_LSR bit definitions
 */
#define MSR_DELTA_MSK 		(MSR_DCTS | MSR_DDSR | MSR_TERI | MSR_DDCD)
#define MSR_DCTS				(1<<0)
#define MSR_DDSR				(1<<1)
#define MSR_TERI				(1<<2)
#define MSR_DDCD				(1<<3)
#define MSR_CTS				(1<<4)
#define MSR_DSR				(1<<5)
#define MSR_RI					(1<<6)
#define MSR_DCD				(1<<7)



#endif 

