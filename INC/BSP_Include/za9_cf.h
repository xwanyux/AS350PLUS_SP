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
#ifndef _ZA9_CF_H_
#define _ZA9_CF_H_


#ifdef BSP_OS_IS_LINUX

#define BSP_WR8( Reg, Data )	( (*((volatile UINT8 *)((Reg) - 0x01000000)) ) = (UINT8)(Data) )
#define BSP_RD8( Reg )        ( *((volatile UINT8 *)((Reg) - 0x01000000)) )

/* Memory mode Task file base address */
#define ZA9L_MEMORY_MODE					0xD1000800

/* True IDE mode Task file base address */
#define ZA9L_TRUE_IDE_MODE					0xD1000800

/* I/O Contigous mode Task file base address */
#define ZA9L_IO_CONT_MODE					0xD1000000

/* I/O Primary mode Task file base address */
#define	ZA9L_IO_PRI_MODE					0xD10001F0	

/* I/O Secondary mode Task file base address */
#define	ZA9L_IO_SEC_MODE					0xD1000170

#else

#define BSP_WR8( Reg, Data )	( (*((volatile UINT8 *)(Reg)) ) = (UINT8)(Data) )
#define BSP_RD8( Reg )        ( *((volatile UINT8 *)(Reg)) )

/* Memory mode Task file base address */
#define ZA9L_MEMORY_MODE					0x16000800

/* True IDE mode Task file base address */
#define ZA9L_TRUE_IDE_MODE					0x16000800

/* I/O Contigous mode Task file base address */
#define ZA9L_IO_CONT_MODE					0x16000000

/* I/O Primary mode Task file base address */
#define	ZA9L_IO_PRI_MODE					0x160001F0	

/* I/O Secondary mode Task file base address */
#define	ZA9L_IO_SEC_MODE					0x16000170

#endif



/* Configuration Registers for CS6 for Compact Flash Card */
#define MEMC_CFG_6_REG 						0xFFFF802C
#define MEMC_TIM_6_REG 						0xFFFF8054


/* Task File Registers in Memory mode of CF operation */
#define	CF_MEM_DATA_REG				(ZA9L_MEMORY_MODE | 0x00)
#define	CF_MEM_ERR_REG				(ZA9L_MEMORY_MODE | 0x01)
#define	CF_MEM_FET_REG				(ZA9L_MEMORY_MODE | 0x01)
#define CF_MEM_SEC_CNT_REG			(ZA9L_MEMORY_MODE | 0x02)
#define CF_MEM_SEC_NUM_REG			(ZA9L_MEMORY_MODE | 0x03)
#define CF_MEM_CYL_LOW_REG			(ZA9L_MEMORY_MODE | 0x04)
#define CF_MEM_CYL_HIGH_REG			(ZA9L_MEMORY_MODE | 0x05)
#define CF_MEM_DRIVE_HEAD_REG		(ZA9L_MEMORY_MODE | 0x06)
#define CF_MEM_STATUS_REG			(ZA9L_MEMORY_MODE | 0x07)
#define CF_MEM_COMMAND_REG			(ZA9L_MEMORY_MODE | 0x07)
#define CF_MEM_ALT_STATUS_REG		(ZA9L_MEMORY_MODE | 0x0E)
#define CF_MEM_DEVICE_CNTL_REG      (ZA9L_MEMORY_MODE | 0x0E)
#define CF_MEM_CARD_ADDR_REG		(ZA9L_MEMORY_MODE | 0x0F)


/* 
 * CF_MEM_ERR_REG, Status Register Bit Deffinitions 
 */

/*
 * This bit is set when a Bad Block is detected. This bit is also set when 
 * an interface CRC error is detected in True IDE Ultra DMA modes of operation
 */

#define CF_BAD_BLOCK_CRCERROR_DTCTD			BIT7

/*
 * This bit is set when an Uncorrectable Error is encountered
 */
#define CF_UNCORRECTABLE_ERROR				BIT6

/*
 * This bit is set when the requested sector ID is an error or cannot be found
 */ 

#define CF_IDNF								BIT4

/*
 * This bit is set if the command has been aborted because of a CompactFlash Storage Card status 
 * condition: (Not Ready, Write Fault, etc) or when an invalid command has been issued
 */

#define CF_ABORT							BIT2

/*
 *	This bit is set in case of a general error
 */
 
#define CF_AMNF								BIT0
 

/*
 * CF_MEM_DRIVE_HEAD_REG, used to select the drive and head, bit deffinitions are as below
 * BIT7 and BIT5 should be 1 for backward compatibility. 
 */

/*
 * When this bit is set LBA, Logical Block Addressing mode is used, When this bit is cleared 
 * CHS Cylinder/Head/Sector addressing mode is used
 */

#define	CF_LBA_ADDRESSING					BIT6

/*
 * When this bit is '0' card (driver) 0 is be selected, when this bit is '1' card (drive) 1 is
 * selected. Setting this bit to 1 is obsolete and should not be used
 */
#define CF_DRV_NUM							BIT4

/* 
 * When operating in CHS mode this is bit 3 of the head number, It is Bit 27 in the LBA mode
 */

#define CF_HS3								BIT3
/* 
 * When operating in CHS mode this is bit 2 of the head number, It is Bit 26 in the LBA mode
 */
#define CF_HS2								BIT2
/* 
 * When operating in CHS mode this is bit 1 of the head number, It is Bit 25 in the LBA mode
 */
#define CF_HS1								BIT1
/* 
 * When operating in CHS mode this is bit 0 of the head number, It is Bit 24 in the LBA mode
 */
#define CF_HS0								BIT0



/* 
 * CF_MEM_STATUS_REG, Status Register Bit Deffinitions 
 */

/*
 * The busy bit is set when the CF Storage Card has access to the command buffer and registers
 * and the host is locked out from accessing the command register and buffer. No other bits in  
 * this register are valid when this bit is set to 1.
 */
#define CF_STAT_BUSY								BIT7

/*
 * RDY bit when set indicates that the CF storage card is capable of performing
 * CF Storage Card operations. 
 */ 
#define CF_STAT_READY							BIT6

/*
 * This bit is set when the write fault has occurred 
 */ 
#define	CF_STAT_DWF								BIT5

/*
 * This bit is set when the Compact Flash Storage Card is ready
 */ 
#define	CF_STAT_DSC								BIT4

/*	
 * DRQ bit in the status register will be set when the CF Storage Card requires that information
 * be transferred either to or from the host through the Data register 
 */  
#define	CF_STAT_DRQ								BIT3												


/*
 *	This bit is set to one when a correctable data error has occured and the data has been corrected. This
 *  condition does not terminate a multi-sector read operation.
 */

#define CF_STAT_CORR							BIT2

/*
 * This bit is set when the previous command has ended in some type of error. The bits in error register contain some 
 * additional information describing the error. 
 */
 
 #define CF_STAT_ERR							BIT0
 
 
/*
 * CF_MEM_DEVICE_CNTL_REG, Device control register bit deffinitions
 * BIT7 to BIT3 and BIT0 are ignored by the CF Storage card and should be set to 0  
 */
 
/*
 * Set this bit to 1 inorder to force the CF Card to perform an AT Disk controller Soft Reset
 * operation. This does not change the PCMCIA PC Card Configuration Registers as a hardware Reset
 * does. The card remains in Reset unitl this bit is cleared.
 */

#define CF_SOFT_RESET				BIT2
 
/*
 * When This bit is 0, Interrupts from the Compact Flash Storage card are enabled and when it
 * set to 1 the interrupts are dissabled
 */
#define CF_IRQ_ENBL					BIT1							


#ifdef BSP_OS_IS_LINUX

/* Attribute Memory Registers */

#define ZA9L_ATTR_MEMORY					0xD0F01200

#else

/* Attribute Memory Registers */
#define ZA9L_ATTR_MEMORY					0x16000200

#endif


#define	ZA9L_ATTR_CONFIG_OPTN_REG			(ZA9L_ATTR_MEMORY + 0x00)

#define ZA9L_ATTR_CARD_CONFIG_STAT_REG		(ZA9L_ATTR_MEMORY + 0x02)		// Not used for CF cards
		
#define ZA9L_ATTR_PIN_REPL_REG				(ZA9L_ATTR_MEMORY + 0x04)		// Not used for CF cards
	
#define ZA9L_ATTR_SOCK_CPY_REG				(ZA9L_ATTR_MEMORY + 0x06)		// Not used for CF cards


/* ZA9L_ATTR_CONFIG_OPTN_REG bit deffinitions */

#define	CF_HARD_RESET						BIT7

#define CF_LEVEL_IRQ						BIT6

#define CF_MODE_CONFIG						BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0


#endif

