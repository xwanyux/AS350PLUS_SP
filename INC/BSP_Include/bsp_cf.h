/*
 * Copyright 2007, ZiLOG Inc.
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
#ifndef _BSP_CF_H_
#define _BSP_CF_H_

#include "bsp_types.h"
#include "za9_cf.h"
#include "bsp_gpio.h"


/* 
 * different modes of interfacing a CF storage card with the host 
 */
 
#define CF_TRUE_IDE_MODE			0
#define	CF_MEMORY_MODE				0
#define	CF_IO_CONTIGUOUS_MODE		1
#define	CF_IO_PRIMARY_MODE			2
#define	CF_IO_SECONDARY_MODE		3

/*
 * Size of each sector 
 */ 
 
#define CF_SECTOR_SIZE_BYTES		512
#define CF_SECTOR_SIZE_HWORDS		256

/*
 * Min and Max values for Sector, Cylinder and Head
 */ 
#define CF_MIN_SECTOR				0
#define CF_MAX_SECTOR				1
#define CF_MIN_CYLINDER				0
#define CF_MAX_CYLINDER				1
#define CF_MIN_HEAD					0
#define CF_MAX_HEAD					1

/* 
 * Addressing Modes 
 */
 
#define CF_LBA_ADDMODE				1
#define CF_CHS_ADDMODE				0


/*
 * Diagnostic Codes, returned in Error task file register after executing the diagnostic command
 */

#define NO_ERROR_DETECTED					0x01
#define FORMATTER_DEVICE_ERROR				0x02
#define SECTOR_BUFFER_ERROR					0x03
#define ECC_CIRCUITRY_ERROR					0x04
#define CONTROLLING_MICROPROCESSOR_ERROR	0x05
#define SLAVE_ERROR_IN_TIDE_MODE			0x8F


/*
 * Compact Flash Storage Commands 
 */

#define CF_EXECUTE_DRIVER_DIAGNOSTIC			0x90
#define CF_ERASE_SECTORS						0xC0
#define CF_IDENTIFY_DEVICE						0xEC
#define CF_READ_SECTORS_1						0x20
#define CF_READ_SECTORS_2						0x21
#define CF_READ_VERIFY_SECTORS_1				0x40
#define CF_READ_VERIFY_SECTORS_2				0x41
#define CF_WRITE_SECTORS_1						0x30
#define CF_WRITE_SECTORS_2						0x31
#define CF_WRITE_SECTORS_WITHOUT_ERASE			0x38
#define CF_WRITE_VERIFY							0x3C



/*
 * Structure for controlling CF card 
 */	

typedef struct BSP_CF_S
{

	BSP_SEM			IsIdle;				// True if the device is idle otherwise False
	
	UINT8			CFOperatingMode;	// 0, Memory mode, 1 for Contiguous I/O mode, 
										// 2 for Primary I/O, 3 for 	Secondary I/O	
	
	UINT8			CFAddrMode;			// 1 for LBA mode and 0 for CHS mode of addressing 
	
	UINT8			*pRegTaskFile;		// Pointer to the Task File Registers base address
	
	UINT8			*pData;				// Pointer to the data

	UINT32			dSize;				// Size of the data to be processed 
	
	UINT8			SectorCount;		// Number of sectors on which the R/W/E operation has to be done
	
	UINT8			Sector;				// Sector Number
	
	UINT16			Cylinder;			// Cylinder Number MSB represents the value for Cylinder_High Parameter Reg
										// and LSB represents the value for Cylinder_Low Parameter Reg										
	UINT8			Head;				// Head Number 
	
	UINT16			CFIdentificationData[256]; // CF Indentification information unformatted

	UINT8			ManufName[41];		// Manufacturer name
	
	UINT16			DefNumCyl;			// Default number of cylinders

	UINT16			DefNumHds;			// Default number of heads

	UINT16			DefNumSectorsPerTrack;		// Default number of sectors per track
	
	UINT16			DefNumSectorsPerCardHigh;		// Default number of sectors per card high integer
		
	UINT16			DefNumSectorsPerCardLow;		// Default number of sectors per card low integer

	UINT16			CurNumCyl;			// Current number of cylinders

	UINT16			CurNumHds;			// Current number of heads

	UINT16			CurNumSectorsPerTrack;		// Current number of sectors per track
	
	UINT16			CurNumSectorsPerCardHigh;		// Current number of sectors per card high integer
		
	UINT16			CurNumSectorsPerCardLow;		// Current number of sectors per card low integer
	
	BSP_GPIO		*pCfscGpio;			// Pointer to GPIO structure 
		
}BSP_CF;



/*
 *	CF Storage Card APIs
 */

/*
 * Function to Initialize the CF device
 */
BSP_STATUS	BSP_CF_Init(void);

/*
 * Function to Acquire the CF device
 */
BSP_CF*		BSP_CF_Acquire(void);

/*
 * Function to Release the CF device 
 */
BSP_STATUS	BSP_CF_Release(BSP_CF *);

/*
 * Function to Stop the CF device
 */
BSP_STATUS  BSP_CF_Stop(BSP_CF *);

/*
 * Function to start the CF device
 */
BSP_STATUS	BSP_CF_Start(BSP_CF *);

/*
 * Function to Soft Reset CF device 
 */
BSP_STATUS	BSP_CF_SoftReset(BSP_CF *);

/*
 * Function to switch between different modes of CF operation
 */
BSP_STATUS  BSP_CF_ModeSwitch(BSP_CF *,UINT8 Mode);

/*
 * Function to read a sector of data from CF
 */ 
BSP_STATUS  BSP_CF_Read(BSP_CF *, UINT8 Sector, UINT16 Cylinder,UINT8 Head,UINT8* pData,UINT32 *pLen);

/*
 * Function to write a sector of data to CF
 */ 
BSP_STATUS  BSP_CF_Write(BSP_CF *, UINT8 Sector, UINT16 Cylinder,UINT8 Head,UINT8* pData,UINT32 *pLen);

/*
 * Function to erase a sector of data from CF
 */ 
BSP_STATUS  BSP_CF_Erase(BSP_CF *, UINT8 SectorCount, UINT8 Sector, UINT16 Cylinder,UINT8 Head);

/*
 * Function to identify the device
 */ 
BSP_STATUS  BSP_CF_IdentifyDevice(BSP_CF *);


#endif
