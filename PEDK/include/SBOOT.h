//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  :							    **
//**  PRODUCT  : AS330							    **
//**                                                                        **
//**  FILE     : SBOOT.H	                                            **
//**  MODULE   : Declaration of SBOOT.	                    		    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2018/10/16                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2018 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _SBOOT_H_
#define _SBOOT_H_

//----------------------------------------------------------------------------
#include "bsp_types.h"


//----------------------------------------------------------------------------
//	Signed File Format
//	PHI		(32)
//	BIN FILE	(N)
//	ECDSA		(64)
//	RSA2048		(256)
//----------------------------------------------------------------------------
typedef struct	ECB_PIH_S
{						// External Control Block
	UINT8		m_SYNC_PATTERN[8];	// 48 49 53 57 45 44 47 44 "HISWEDGD"
	UINT32		m_FORMAT_VER_NUM;	// 
	UINT32		m_RAW_BIN_LOAD_ADDR;	// 
	UINT8		m_RAW_BIN_LEN[4];	// (N) big-endian
	UINT8		m_RAW_BIN_JUMP_ADDR[4];	// 
	UINT32		m_SLA_ARG_SIZE;		//
	UINT32		m_APP_VER_NUM;		//

} __attribute__((packed)) ECB_PIH;


#define	FLASH_SECTOR_SIZE			1024*4		// 4KB
#define	FLASH_MAX_SIZE				0x100000	// 1MB

#define	HASH_BLOCK_SIZE				1024
#define	SIGNED_HASH_SIZE			256
#define	DIGEST_SIZE				32
#define	M3_HASH_BLOCK_SIZE			64*1

#define	FLASH_BASE				MML_MEM_FLASH_BASE
#define	APP1_FLASH_ST_ADDRESS			FLASH_BASE+0x20000	// 1002_0000~100F_FFFF (1MB-128KB)
#define	ADDR_APP1				APP1_FLASH_ST_ADDRESS
#define	ADDR_APP1_RUN				APP1_FLASH_ST_ADDRESS + 0x20

#define	ADDR_LAST_SECTOR			FLASH_BASE+(FLASH_MAX_SIZE - FLASH_SECTOR_SIZE)
#define	ADDR_SECALM_VALUE			FLASH_BASE+(FLASH_MAX_SIZE - 4)


extern	void	SB_FLASH_ReadData( void *BaseAddr, void *pData, UINT32 Len );
extern	UINT32	SB_FindEcb( UINT32 StartAddr, UINT32 MaxSize, UINT32 *RunAddr, UINT32 *Size );


//----------------------------------------------------------------------------
#endif
