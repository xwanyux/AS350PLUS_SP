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
#ifndef _BSP_GPIO_H_
#define _BSP_GPIO_H_

#include "bsp_types.h"
#include "bsp_utils.h"
#include "za9_gpio.h"


/*
 * ZA9L POS Development Platform GPIO Configuration:
 *
 * Port 0			Primary Function		Dev Kit Use					POR Setting
 * =========		================		=================			===========
 *		Pin 0			GPIO						Boot ROM Download			GPIO
 * 	Pin 1			GPIO						Key Pad - Row 1			GPIO
 *		Pin 2			GPIO						Key Pad - Row 4			GPIO
 * 	Pin 3			GPIO						Key Pad - Row 3			GPIO
 *		Pin 4			GPIO						Key Pad - Row 2			GPIO
 * 	Pin 5			GPIO						Key Pad - Col 4			GPIO
 *		Pin 6			GPIO						Key Pad - Col 3			GPIO
 * 	Pin 7			GPIO						Key Pad - Col 2			GPIO
 *		Pin 8			GPIO						Key Pad - Col 1			GPIO
 * 	Pin 9			GPIO						Key Pad - Col 0			GPIO
 *		Pin 10		GPIO														GPIO
 * 	Pin 11		GPIO														GPIO
 *		Pin 12		GPIO														GPIO
 * 	Pin 13		GPIO														GPIO
 *		Pin 14		GPIO														GPIO
 * 	Pin 15		GPIO														GPIO
 *		Pin 16		nCS[6]													nCS[6]
 * 	Pin 17		nCS[7]													nCS[7]
 *		Pin 18		nCS[8]													nCS[8]
 * 	Pin 19		nCS[9]													nCS[9]
 *		Pin 20		MA[20]					MA[20]						MA[20]
 * 	Pin 21		MA[21]					MA[21]						MA[21]
 *		Pin 22		MA[22]					MA[22]						MA[22]
 * 	Pin 23		MA[23]					MA[23]						MA[23]
 *		Pin 24		nLBA														GPIO
 * 	Pin 25		nBBA														GPIO
 *		Pin 26		Ready														GPIO
 * 	Pin 27		PWM/TCLK[0]				Buzzer						GPIO
 *		Pin 28		PWM/TCLK[1]				LCD_COL_SEL					GPIO
 * 	Pin 29		PWM/TCLK[2]												GPIO
 *		Pin 30		PWM/TCLK[3]												GPIO
 * 	Pin 31		RxD[2]					RxD[2]						GPIO
 *
 * Port 1			Primary Function		Dev Kit Use					POR Setting
 * =========		================		=================			===========
 *		Pin 0			TxD[2]					TxD[2]						GPIO
 * 	Pin 1			nCTS[0]					nCTS[0]						GPIO
 *		Pin 2			nCTS[1]					nCTS[1]						GPIO
 * 	Pin 3			nCTS[2]					nCTS[2]						GPIO
 *		Pin 4			nRTS[0]					nRTS[0]						GPIO
 * 	Pin 5			nRTS[1]					nRTS[1]						GPIO
 *		Pin 6			nRTS[2]					nRTS[2]						GPIO
 * 	Pin 7			nDCD[0]					nDCD[0]						GPIO
 *		Pin 8			nDTR[0]					nDTR[0]						GPIO
 * 	Pin 9			nDSR[0]					nDSR[0]						GPIO
 *		Pin 10		nRI[0]					nRI[0]						GPIO
 * 	Pin 11		MISO[0]													GPIO
 *		Pin 12		MISO[1]													GPIO
 * 	Pin 13		MOSI[0]													GPIO
 *		Pin 14		MOSI[1]													GPIO
 * 	Pin 15		SCK[0]													GPIO
 *		Pin 16		SCK[1]													GPIO
 * 	Pin 17		nSS[0]													GPIO
 *		Pin 18		nSS[1]													GPIO
 * 	Pin 19		SC_CLK[0]												GPIO
 *		Pin 20		SC_CLK[1]												GPIO
 * 	Pin 21		SC_IO[0]													GPIO
 *		Pin 22		SC_IO[1]													GPIO
 * 	Pin 23		SC_nALARM												GPIO
 *		Pin 24		SC_SCK													GPIO
 * 	Pin 25		SC_MOSI													GPIO
 *		Pin 26		SC_MISO													GPIO
 * 	Pin 27		SC_nSS[0]												GPIO
 *		Pin 28		SC_nSS[1]												GPIO
 * 	Pin 29		LCD_E														GPIO
 *		Pin 30		LCD_RS													GPIO
 * 	Pin 31		LCD_RnW													GPIO
 *
 * Port 2			Primary Function		Dev Kit Use					POR Setting	
 * =========		================		=================			===========
 *		Pin 0			LCD_D[0]													GPIO
 * 	Pin 1			LCD_D[1]													GPIO
 *		Pin 2			LCD_D[2]													GPIO
 * 	Pin 3			LCD_D[3]													GPIO
 *		Pin 4			LCD_D[4]													GPIO
 * 	Pin 5			LCD_D[5]													GPIO
 *		Pin 6			LCD_D[6]													GPIO
 * 	Pin 7			LCD_D[7]													GPIO
 *		Pin 8			TxReq														GPIO
 * 	Pin 9			TxAck														GPIO
 *		Pin 10		RxReq														GPIO
 * 	Pin 11		RxAck						LCD_BL_EN					GPIO
 */



/*
 * GPIO Port numbers
 */
#define GPIO_PORT_0							0x00
#define GPIO_PORT_1							0x01
#define GPIO_PORT_2							0x02
#define NUM_GPIO_PORTS						0x03


/*
 * GPIO Modes
 * Primary => GPIO pin is being used for primary function
 */
#define GPIO_MODE_PRIMARY					0x00
#define GPIO_MODE_INPUT						0x11
#define GPIO_MODE_OUTPUT					0x21
#define GPIO_MODE_IO							0x31

/*
 * GPIO Interrupt Mode bit definitions
 */
#ifdef ZA9_AB_SILICON
	#define GPIO_INT_EDGE					0x00
	#define GPIO_INT_LEVEL					0x01
#else
	#define GPIO_INT_EDGE					0x01
	#define GPIO_INT_LEVEL					0x00
#endif
#define GPIO_INT_FALLING					0x00
#define GPIO_INT_RISING						0x02
#define GPIO_INT_A							0x00
#define GPIO_INT_B							0x04
#define GPIO_INT_DISABLE					0x00
#define GPIO_INT_ENABLE						0x08

/*
 * GPIO Interrupt Modes
 */
#define GPIO_INT_MODE_NONE					(GPIO_INT_DISABLE)

#define GPIO_INT_MODE_A_0_LEVEL			(GPIO_INT_ENABLE | GPIO_INT_A | GPIO_INT_FALLING | GPIO_INT_LEVEL)
#define GPIO_INT_MODE_A_0_EDGE			(GPIO_INT_ENABLE | GPIO_INT_A | GPIO_INT_FALLING | GPIO_INT_EDGE)
#define GPIO_INT_MODE_A_1_LEVEL			(GPIO_INT_ENABLE | GPIO_INT_A | GPIO_INT_RISING  | GPIO_INT_LEVEL)
#define GPIO_INT_MODE_A_1_EDGE			(GPIO_INT_ENABLE | GPIO_INT_A | GPIO_INT_RISING  | GPIO_INT_EDGE)

#define GPIO_INT_MODE_B_0_LEVEL			(GPIO_INT_ENABLE | GPIO_INT_B | GPIO_INT_FALLING | GPIO_INT_LEVEL)
#define GPIO_INT_MODE_B_0_EDGE			(GPIO_INT_ENABLE | GPIO_INT_B | GPIO_INT_FALLING | GPIO_INT_EDGE)
#define GPIO_INT_MODE_B_1_LEVEL			(GPIO_INT_ENABLE | GPIO_INT_B | GPIO_INT_RISING  | GPIO_INT_LEVEL)
#define GPIO_INT_MODE_B_1_EDGE			(GPIO_INT_ENABLE | GPIO_INT_B | GPIO_INT_RISING  | GPIO_INT_EDGE)

/*
 * GPIO Wake Modes
 * If enabled, the GPIO controller will assert the PMU Wake pin on either a 
 * falling or rising edge detected on the GPIO pin.  The edge polarity is 
 * shared between the Interrupt and Wake modes.
 */
#define GPIO_WAKE								TRUE
#define GPIO_WAKE_DISABLE					FALSE



typedef struct BSP_GPIO_S
{
	BSP_SEM						Avail;		// If 0, GPIO structure has already been allocated
	BSP_SEM						IsIdle;		// If 1, GPIO Start has not been called
	UINT32						Base;			// Base address of GPIO port
	UINT32						Mask;			// Bitmask of GPIO pins which identically configured
	UINT32						Mode;			// GPIO operating mode
	UINT32						IntMode;		// GPIO interrupt mode
	BSP_BOOL						Wake;			// GPIO Wake control (same polarity as Int Mode)
	UINT32						Data;
	void						(*	pIsrFunc)	// Upper-layer interrupt callback
	(												// callback parameters
		struct BSP_GPIO_S * pGpio			// GPIO reference
	);		
} BSP_GPIO;


typedef void (* GPIO_ISR_FUNC )( BSP_GPIO * pGpio ); 



/*
 * Function Prototypes
 */
extern BSP_STATUS BSP_GPIO_Init( UINT32 NumGpio );
extern BSP_GPIO * BSP_GPIO_Acquire( UINT32 Port, UINT32 PinMask );
extern BSP_STATUS BSP_GPIO_Release( BSP_GPIO * pGpio );
extern BSP_STATUS BSP_GPIO_Start( BSP_GPIO * pGpio );
extern BSP_STATUS BSP_GPIO_Stop( BSP_GPIO * pGpio );


/*
 * Macros for modifying groups of GPIO pins
 */
#define BSP_GPIO_READ( pGpio )			((pGpio)->Data = BSP_RD32( (pGpio)->Base | GPIO_IN ) & (pGpio)->Mask)
#define BSP_GPIO_OUT( pGpio )				BSP_WR32( (pGpio)->Base | GPIO_OUT, BSP_RD32((pGpio)->Base | GPIO_OUT)& ~(pGpio)->Mask | ((pGpio)->Mask &(pGpio)->Data))
#define BSP_GPIO_OUT_0( pGpio )			BSP_WR32( (pGpio)->Base | GPIO_OUT_CLR, (pGpio)->Mask )
#define BSP_GPIO_OUT_1( pGpio )			BSP_WR32( (pGpio)->Base | GPIO_OUT_SET, (pGpio)->Mask )
#define BSP_GPIO_OE_CLR( pGpio )			BSP_WR32( (pGpio)->Base | GPIO_OE_CLR, (pGpio)->Mask )
#define BSP_GPIO_OE_SET( pGpio )			BSP_WR32( (pGpio)->Base | GPIO_OE_SET, (pGpio)->Mask )

#define BSP_GPIO_OE_CLR_DATA( pGpio )	BSP_WR32( (pGpio)->Base | GPIO_OE_CLR, (pGpio)->Mask & (pGpio)->Data )
#define BSP_GPIO_OE_SET_DATA( pGpio )	BSP_WR32( (pGpio)->Base | GPIO_OE_SET, (pGpio)->Mask & (pGpio)->Data )

#ifndef ZA9_AB_SILICON
	#define BSP_GPIO_IE_CLR( pGpio )		BSP_WR32( (pGpio)->Base | GPIO_IEN_CLR, (pGpio)->Mask )
	#define BSP_GPIO_IE_SET( pGpio )		BSP_WR32( (pGpio)->Base | GPIO_IEN_SET, (pGpio)->Mask )
#endif

#endif
