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
#ifndef _BSP_KEY_H_
#define _BSP_KEY_H_

#include "bsp_types.h"
#include "bsp_utils.h"
#include "za9_gpio.h"
#include "bsp_tmr.h"


#define BSP_KEY_MAX_ROWS					5
#define BSP_KEY_MAX_COLS					5



/*
 * The Rev A ZA9L Application board has a 4 Row by 5 Col key pad array.
 * The PCB could support a 5x5 key pad array, but currently GPIO Port0 Pin0
 * is used by the Boot ROM to force SW download over UART 0 via JP1.
 * Therefore GPIO0 Port0 Pin0 is not connected to the keypad array and
 * including this GPIO in the ket scan routine will result in phantom
 * key presses on Ro0 depending on whether JP1 is on or off.
 * Therefore when calling the BSP_KEY_Read routine use the following macro
 * to mask keystrokes on Row 0.
 */
#define BSP_KEY_MASK							0x01EF7BDE



/*
 * Key mapping for the 5 Col x 4 Row key pad array on the Rev A ZA9L Application board.
 * The values below identify keys based on ColRow format ie the first key in 
 * Row 2 is BSP_KEY_ROW2_COL0.  The 'orientation' of the key pad is such that
 * Row 0 is not present.
 * Row 1 is immediately below the LCD display and parallel to its length.
 * Col 0 is closest to the power supply connector
 * Col 4 is closest to the ZA9L MCU board.
 */
#define BSP_KEY_ROW0_COL0					0x00100000
#define BSP_KEY_ROW0_COL1					0x00008000
#define BSP_KEY_ROW0_COL2					0x00000400
#define BSP_KEY_ROW0_COL3					0x00000020
#define BSP_KEY_ROW0_COL4					0x00000001

#define BSP_KEY_ROW1_COL0					0x00200000
#define BSP_KEY_ROW1_COL1					0x00010000
#define BSP_KEY_ROW1_COL2					0x00000800
#define BSP_KEY_ROW1_COL3					0x00000040
#define BSP_KEY_ROW1_COL4					0x00000002

#define BSP_KEY_ROW2_COL0					0x01000000
#define BSP_KEY_ROW2_COL1					0x00080000
#define BSP_KEY_ROW2_COL2					0x00004000
#define BSP_KEY_ROW2_COL3					0x00000200
#define BSP_KEY_ROW2_COL4					0x00000010

#define BSP_KEY_ROW3_COL0					0x00800000
#define BSP_KEY_ROW3_COL1					0x00040000
#define BSP_KEY_ROW3_COL2					0x00002000
#define BSP_KEY_ROW3_COL3					0x00000100
#define BSP_KEY_ROW3_COL4					0x00000008

#define BSP_KEY_ROW4_COL0					0x00400000
#define BSP_KEY_ROW4_COL1					0x00020000
#define BSP_KEY_ROW4_COL2					0x00001000
#define BSP_KEY_ROW4_COL3					0x00000080
#define BSP_KEY_ROW4_COL4					0x00000004


#define DEFAULT_SCAN_PERIOD_MS			50

typedef struct				BSP_KEY_S
{
	BSP_SEM						IsIdle;			// If 1, Start has not been called
	UINT32						ScanPeriod_ms;	// ms bewteen key scans					
	UINT32						MissedKeys;		// Counts number of missed key presses
	UINT32						GpioMask;		// GPIO bits required for key scan
	UINT32						RawScan;			// Scan code available in List callback
	void						(*	pIsrFunc)		// Upper-layer callback when key detected
	(													// callback parameters		
		struct BSP_KEY_S   * pKey				// KEY reference
	);		
	BSP_TIMER				 * pTmr;				// HW timer used for Key scan

} BSP_KEY;

typedef void (* KEY_ISR_FUNC )( BSP_KEY * pKey ); 


/*
 * Key scan mapping table
 */
extern UINT32 					KeyTable[4][5];



/*
 * Function Prototypes
 */
extern BSP_STATUS BSP_KEY_Init(    void );
extern UINT32     BSP_KEY_Scan(    UINT32 KeyMask );
extern UINT32     BSP_KEY_Decode(  UINT32 KeyCode );
extern BSP_KEY  * BSP_KEY_Acquire( KEY_ISR_FUNC pKeyFunc );
extern BSP_STATUS BSP_KEY_Release( BSP_KEY * pKey );
extern BSP_STATUS BSP_KEY_Start(   BSP_KEY * pKey );
extern BSP_STATUS BSP_KEY_Stop(    BSP_KEY * pKey );
extern BSP_STATUS BSP_KEY_Read(    BSP_KEY * pKey, UINT32 * pScanCode );



#endif
