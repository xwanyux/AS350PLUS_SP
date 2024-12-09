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
#ifndef _BSP_SD_H_
#define _BSP_SD_H_



#include "bsp_gpio.h"
#include "bsp_spi.h"
#include "bsp_types.h"
#include "bsp_uart.h"
#include "bsp_utils.h"
#include "bsp_mem.h"
#include "bsp_tmr.h"




enum {
	SD_INIT_TIMEOUT 		= 600,			
	SD_COMMAND_TIMEOUT		= 500, 			
	SD_PROG_TIMEOUT			= 1000,			
	BUSY_BLOCK_LEN 			= 1,			
	BUSY_BLOCK_LEN_SHORT	= 16,			
	SD_SECTOR_SIZE			= 512,			 
	SD_PRE_CMD_ZEROS		= 4
};

// Internal error codes
enum {
	ERR_SPI_TIMEOUT			= 0xF1,
	ERR_SD_TIMEOUT			= 0xF2,
	ERR_UNKNOWN_TOK			= 0xF3
};

// return values from functions
enum {
	RVAL_OK					= 0,
	RVAL_ERROR				= 1,
	RVAL_CRITICAL				= 2
};

// Format R1(b) response tokens (1 byte long)
enum {
	BUSY_TOKEN				= 0x00,
	R1_OK					= 0x00,
	R1_IDLE_STATE			= 0x01,
	R1_ERASE_STATE			= 0x02,
	R1_ILLEGAL_COMMAND		= 0x04,
	R1_COM_CRC_ERROR		= 0x08,
	R1_ERASE_SEQ_ERROR		= 0x10,
	R1_ADDRESS_ERROR		= 0x20,
	R1_PARAMETER_ERROR		= 0x40
};

// Format R2 response tokens (2 bytes long, first is same as R1 responses)
enum {
	R2_OK					= 0x00,
	R2_CARD_LOCKED			= 0x01,
	R2_WP_ERASE_SKIP		= 0x02,
	R2_LOCK_UNLOCK_CMD_FAIL	= 0x02,
	R2_ERROR				= 0x04,
	R2_CC_ERROR				= 0x08,
	R2_CARD_ECC_FAILED		= 0x10,
	R2_WP_VIOLATION			= 0x20,
	R2_ERASE_PARAM			= 0x40,
	R2_OUT_OF_RANGE			= 0x80,
	R2_CSD_OVERWRITE		= 0x80
};


// Data response tokens
enum {
	DR_MASK					= 0x0F,
	DR_ACCEPTED				= 0x05,
	DR_CRC_ERROR			= 0x0B,
	DR_WRITE_ERROR			= 0x0D
} ;


enum {
	SBT_S_BLOCK_READ		= 0xFE,
	SBT_M_BLOCK_READ		= 0xFE,
	SBT_S_BLOCK_WRITE		= 0xFE,
	SBT_M_BLOCK_WRITE 		= 0xFC,
	STT_M_BLOCK_WRITE		= 0xFD
} ;

// Data error tokens (1 byte long)
enum {
	DE_ERROR				= 0x01,
	DE_CC_ERROR				= 0x02,
	DE_CARD_ECC_FAILED		= 0x04,
	DE_OUT_OF_RANGE			= 0x08,
	DE_CARD_IS_LOCKED		= 0x10
};

// MMC/SD SPI mode commands
enum {
	GO_IDLE_STATE			= 0,
	SEND_OP_COND			= 1,
	SEND_CSD				= 9,
	SEND_CID				= 10,
	STOP_TRANSMISSION		= 12,
	SEND_STATUS				= 13,
	SET_BLOCKLEN			= 16,
	READ_SINGLE_BLOCK		= 17,
	READ_MULTIPLE_BLOCK		= 18,
	WRITE_BLOCK				= 24,
	WRITE_MULTIPLE_BLOCK	= 25,
	ERASE_START_ADDRESS		= 32,
	ERASE_END_ADDRESS		= 33,
	ERASE					= 38,
	SD_SEND_OP_COND			= 41,
	APP_CMD					= 55
};

	
struct cid_str {
	unsigned int		manfid;
	char				prod_name[8];
	unsigned int		serial;
	unsigned short		oemid;
	unsigned short		year;
	unsigned char		hwrev;
	unsigned char		fwrev;
	unsigned char		month;
};

struct csd_str {					
	unsigned char		mmca_vsn;	
	unsigned short		cmdclass;	
	unsigned short		tacc_clks;	
	unsigned int		tacc_ns;		
	unsigned int		max_dtr;		
	unsigned int		read_blkbits;	
	unsigned int		capacity;
};







typedef struct					BSP_SD_S
{


	BSP_SEM			    			IsIdle;
	UINT8						raw_CSD[18];
	UINT8						raw_CID[18];
	struct cid_str					CID;
	struct csd_str					CSD;
	BSP_SPI						*pSDSpi;
	BSP_GPIO					*pSDGpioCD;
	BSP_GPIO					*pSDGpioWP;
	BSP_TIMER					*pSDTimer;
	UINT8						IsSD_Present;

	
} BSP_SD;



extern BSP_STATUS BSP_SD_Init( void );
extern BSP_SD* BSP_SD_Acquire( void );
extern BSP_STATUS BSP_SD_Release(BSP_SD *pSD );
extern UINT16 BSP_SD_Mult_Write(BSP_SD *pSD,UINT32 BlockAddress,UINT8 *pData,UINT32 NumSector );
extern UINT16 BSP_SD_Mult_Read(BSP_SD *pSD,UINT32 BlockAddress,UINT8 *pData,UINT32 NumSector );
extern UINT16 BSP_SD_Write(BSP_SD *pSD,UINT32 BlockAddress,UINT8 *pData);
extern UINT16 BSP_SD_Read(BSP_SD *pSD,UINT32 BlockAddress,UINT8 *pData);
extern BSP_STATUS BSP_SD_Start(BSP_SD *pSD );
extern BSP_STATUS BSP_SD_Stop(BSP_SD *pSD );
extern UINT16 BSP_SD_Erase( BSP_SD *pSD, UINT32 BlockAddress_From,UINT32 BlockAddress_To );


#endif
