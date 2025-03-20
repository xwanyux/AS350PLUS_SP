//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  :                                                            **
//**  PRODUCT  : AS350 PLUS                                                 **
//**                                                                        **
//**  FILE     : OS_FLASH.H                                                 **
//**  MODULE   : Definition of FLASH backup area.	                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2024/12/31                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2024 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _OS_FLASH_H_
#define _OS_FLASH_H_

//----------------------------------------------------------------------------
#include "bsp_types.h"

//----------------------------------------------------------------------------
// 8MB FLASH MAPPING
//
//	-------------------------------------                               
//	107F_FFFF           
//			FLASH DATA	64KB	(for system backup)
//	107F_0000
//	-------------------------------------
// NOTE: the length of any FLASH element shall be even. (word access)
//----------------------------------------------------------------------------
//#ifdef	_FLASH_4MB_
//#define	FLASH_BACKUP_BASE_ADDR		0x103F0000			// 4MB FLASH (64KB)
//#endif

//#ifdef	_FLASH_8MB_
//#define	FLASH_BACKUP_BASE_ADDR		0x107F0000			// 8MB FLASH (64KB)
//#endif

//#ifdef	_FLASH_16MB_
//#define	FLASH_BACKUP_BASE_ADDR		0x10FE0000			// 16MB FLASH (128KB boundary)
//#endif

//#ifdef	_FLASH_32MB_
//#define	FLASH_BACKUP_BASE_ADDR		0x11FE0000			// 32MB FLASH (128KB boundary)
//#endif

#define	FLASH_BACKUP_BASE_ADDR		0x00000000			// 32MB FLASH (128KB boundary) for AS350 PLUS

#define	MAX_FLASH_BACKUP_LEN		64*1024				// 64KB

#define	F_ADDR_PED_ETMK			FLASH_BACKUP_BASE_ADDR		// b20, L(1)+V(16)+KCV(3)
#define	PED_ETMK_LEN			20

#define	F_ADDR_APP_ID			F_ADDR_PED_ETMK + PED_ETMK_LEN	// b1 , application ID, 0=app1, 1=app2
#define	APP_ID_LEN			1
#define	APP_ID_LEN_PAD			2				// padding for even

#define	F_ADDR_TSN			F_ADDR_APP_ID + APP_ID_LEN_PAD	// b16, LEN(1)+TSN(15)
#define	TSN_LEN				16

#define	F_ADDR_ISP_PSW			F_ADDR_TSN + TSN_LEN		// b16, LEN(1)+PSW(12) + PAD(3)
#define	ISP_PSW_LEN			16

//----------------------------------------------------------------------------
//	IMEK & IAEK
//----------------------------------------------------------------------------
#define	F_ADDR_CL_IMEK			F_ADDR_ISP_PSW + ISP_PSW_LEN		// b16, eIMEK(16) = TDES( IMEK, XCSN )
#define	CL_IMEK_LEN			16
#define	F_ADDR_CL_IMEK_KCV		F_ADDR_CL_IMEK + CL_IMEK_LEN		// b8, KCV of IMEK
#define	CL_IMEK_KCV_LEN			8

#define	F_ADDR_IMEK_END			F_ADDR_CL_IMEK_KCV + CL_IMEK_KCV_LEN

#define	F_ADDR_CL_IAEK			F_ADDR_IMEK_END				// b16, eIAEK(16) = TDES( IAEK, XCSN )
#define	CL_IAEK_LEN			16
#define	F_ADDR_CL_IAEK_KCV		F_ADDR_CL_IAEK + CL_IAEK_LEN		// b8, KCV of IAEK
#define	CL_IAEK_KCV_LEN			8

#define	F_ADDR_IAEK_END			F_ADDR_CL_IAEK_KCV + CL_IAEK_KCV_LEN

//----------------------------------------------------------------------------
//	CAPK (up to 50 key slots, 300 bytes/key, total 15,000 bytes)
//
//      RID                       - 5   bytes
//      INDEX                     - 1   byte
//      EXPONENT LENGTH           - 1   byte  (exponent length in BYTES)
//      MODULUS LENGTH            - 2   byte  (modulus length in BYTES)
//      SHA-1                     - 20  bytes (hash(RID+INDEX+MODULUS+EXPONENT))
//      EXPONENT                  - 3   bytes (2, 3, or 2^16+1)
//      MODULUS                   - 256 bytes (768, 896, 1024, 1152, or 2048 bits)
//      HASH ALGORITHM INDICATOR  - 1   byte
//      PK ALGORITHM INDICATOR    - 1   byte  (PK: public key)
//	EXPIRY DATE		  - 4   bytes (CUP)
//      RFU                       - 6   bytes
//----------------------------------------------------------------------------
#define	CL_CAPK_LEN			300
#define	MAX_CL_CAPK_CNT			50

#define	F_ADDR_CL_CAPK			F_ADDR_IAEK_END
#define	F_ADDR_CL_CAPK_01		F_ADDR_CL_CAPK
#define	F_ADDR_CL_CAPK_02		F_ADDR_CL_CAPK + (CL_CAPK_LEN*1)

#define	F_ADDR_CL_CAPK_END		F_ADDR_CL_CAPK + (CL_CAPK_LEN*MAX_CL_CAPK_CNT)

//----------------------------------------------------------------------------
//	Contactless Reader Parameters
//----------------------------------------------------------------------------
#define	MAX_CL_PARA_SIZE		10*1024
#define	F_ADDR_CL_PARA			F_ADDR_CL_CAPK_END

#define	F_ADDR_CL_PARA_END		F_ADDR_CL_PARA + MAX_CL_PARA_SIZE

//----------------------------------------------------------------------------
//	Contactless Reader Parameters
//----------------------------------------------------------------------------
#define	F_ADDR_BATT_BIAS		F_ADDR_CL_PARA_END+MAX_CL_PARA_SIZE	// b2 , battery BIAS_VALUE(2)
#define	BATT_BIAS_LEN			2

#define	F_ADDR_BATT_BIAS_END		F_ADDR_BATT_BIAS + BATT_BIAS_LEN

//----------------------------------------------------------------------------
//	Flag for TAMPER detection
//	FFFF = ON  (default to detect tamper status)
//	5AA5 = OFF (not to detect tamper status)
//----------------------------------------------------------------------------
#define	FLAG_DETECT_TAMPER_ON		0xFFFF
#define	FLAG_DETECT_TAMPER_OFF		0x5AA5

#define	TAMPER_DETECT_LEN		2
#define	F_ADDR_TAMPER_DETECT		F_ADDR_BATT_BIAS_END

#define	F_ADDR_TAMPER_DETECT_END	F_ADDR_TAMPER_DETECT + TAMPER_DETECT_LEN

//----------------------------------------------------------------------------
//	System Critical Event Log (8 records * 20 bytes)
//
//	(YYMMDDhhmmss) + (Event Source) + (Event ID)
//	(12 bytes)     + (4 bytes)      + ( 4 bytes) = 20 bytes
//----------------------------------------------------------------------------
#define	SYS_EVENT_LOG_LEN		20
#define	F_ADDR_SYS_EVENT_LOG_01		F_ADDR_TAMPER_DETECT_END

#define	SYS_EVENT_TIME_LEN		12
#define	F_ADDR_SYS_EVENT_TIME_01	F_ADDR_TAMPER_DETECT_END

#define	F_ADDR_SYS_EVENT_TIME_01_END	F_ADDR_SYS_EVENT_TIME_01 + SYS_EVENT_TIME_LEN

#define	SYS_EVENT_SRC_LEN		4
#define	F_ADDR_SYS_EVENT_SRC_01		F_ADDR_SYS_EVENT_TIME_01_END

#define	F_ADDR_SYS_EVENT_SRC_01_END	F_ADDR_SYS_EVENT_SRC_01 + SYS_EVENT_SRC_LEN

#define	CODE_SYS_EVENT_ID_LEN		4
#define	F_ADDR_SYS_EVENT_ID_01		F_ADDR_SYS_EVENT_SRC_01_END

#define	F_ADDR_SYS_EVENT_ID_01_END	F_ADDR_SYS_EVENT_ID_01 + CODE_SYS_EVENT_ID_LEN

#define	F_ADDR_SYS_EVENT_LOG_01_END	F_ADDR_SYS_EVENT_ID_01_END

//----------------------------------------------------------------------------
#define	F_ADDR_SYS_EVENT_LOG_02		F_ADDR_SYS_EVENT_LOG_01 + SYS_EVENT_LOG_LEN
#define	F_ADDR_SYS_EVENT_LOG_03		F_ADDR_SYS_EVENT_LOG_02 + SYS_EVENT_LOG_LEN
#define	F_ADDR_SYS_EVENT_LOG_04		F_ADDR_SYS_EVENT_LOG_03 + SYS_EVENT_LOG_LEN
#define	F_ADDR_SYS_EVENT_LOG_05		F_ADDR_SYS_EVENT_LOG_04 + SYS_EVENT_LOG_LEN
#define	F_ADDR_SYS_EVENT_LOG_06		F_ADDR_SYS_EVENT_LOG_05 + SYS_EVENT_LOG_LEN
#define	F_ADDR_SYS_EVENT_LOG_07		F_ADDR_SYS_EVENT_LOG_06 + SYS_EVENT_LOG_LEN
#define	F_ADDR_SYS_EVENT_LOG_08		F_ADDR_SYS_EVENT_LOG_07 + SYS_EVENT_LOG_LEN

#define	F_ADDR_SYS_EVENT_LOG_END	F_ADDR_SYS_EVENT_LOG_08 + SYS_EVENT_LOG_LEN

//----------------------------------------------------------------------------
//	Flag for BIOS FUNCTION 97 reference.
//	FFFF = OFF (not to refer to F97, that is to set default "XT" state.
//	ABCD = ON  (must refer to F97 for setting "XT")
//----------------------------------------------------------------------------
#define	FLAG_REF_F97_OFF		0xFFFF
#define	FLAG_REF_F97_ON			0xABCD

#define	REF_F97_LEN			2
#define	F_ADDR_REF_F97			F_ADDR_TAMPER_DETECT_END

#define	F_ADDR_REF_F97_END		F_ADDR_REF_F97 + REF_F97_LEN

//----------------------------------------------------------------------------
#define	F_ADDR_PED_ETMK_01		F_ADDR_REF_F97_END			// 2014-06-24, for NCCC multiple TMK backup
#define	F_ADDR_PED_ETMK_02		F_ADDR_PED_ETMK_01 + PED_ETMK_LEN
#define	F_ADDR_PED_ETMK_03		F_ADDR_PED_ETMK_02 + PED_ETMK_LEN
#define	F_ADDR_PED_ETMK_04		F_ADDR_PED_ETMK_03 + PED_ETMK_LEN
#define	F_ADDR_PED_ETMK_05		F_ADDR_PED_ETMK_04 + PED_ETMK_LEN
#define	F_ADDR_PED_ETMK_06		F_ADDR_PED_ETMK_05 + PED_ETMK_LEN
#define	F_ADDR_PED_ETMK_07		F_ADDR_PED_ETMK_06 + PED_ETMK_LEN
#define	F_ADDR_PED_ETMK_08		F_ADDR_PED_ETMK_07 + PED_ETMK_LEN
#define	F_ADDR_PED_ETMK_09		F_ADDR_PED_ETMK_08 + PED_ETMK_LEN
#define	F_ADDR_PED_ETMK_10		F_ADDR_PED_ETMK_09 + PED_ETMK_LEN

#define	F_ADDR_PED_ETMK_11		F_ADDR_PED_ETMK_10 + PED_ETMK_LEN	// 2014-08-20, extend key slots up to 20
#define	F_ADDR_PED_ETMK_12		F_ADDR_PED_ETMK_11 + PED_ETMK_LEN
#define	F_ADDR_PED_ETMK_13		F_ADDR_PED_ETMK_12 + PED_ETMK_LEN
#define	F_ADDR_PED_ETMK_14		F_ADDR_PED_ETMK_13 + PED_ETMK_LEN
#define	F_ADDR_PED_ETMK_15		F_ADDR_PED_ETMK_14 + PED_ETMK_LEN
#define	F_ADDR_PED_ETMK_16		F_ADDR_PED_ETMK_15 + PED_ETMK_LEN
#define	F_ADDR_PED_ETMK_17		F_ADDR_PED_ETMK_16 + PED_ETMK_LEN
#define	F_ADDR_PED_ETMK_18		F_ADDR_PED_ETMK_17 + PED_ETMK_LEN
#define	F_ADDR_PED_ETMK_19		F_ADDR_PED_ETMK_18 + PED_ETMK_LEN
#define	F_ADDR_PED_ETMK_20		F_ADDR_PED_ETMK_19 + PED_ETMK_LEN

#define	F_ADDR_PED_ETMK_END		F_ADDR_PED_ETMK_20 + PED_ETMK_LEN

//----------------------------------------------------------------------------
//	BIOS Release Date/Time
//----------------------------------------------------------------------------
#define	BIOS_RELEASE_DATE_LEN		14				// DATA(14)

#define	F_ADDR_BIOS_RELEASE_DATE	F_ADDR_PED_ETMK_END		// 2015-02-09, YYYYMMDDhhmmss
#define	F_ADDR_BIOS_RELEASE_DATE_END	F_ADDR_BIOS_RELEASE_DATE + BIOS_RELEASE_DATE_LEN

//----------------------------------------------------------------------------
//	Global Variables Backup Area
//----------------------------------------------------------------------------
#define	DssTelNum_LEN			24
#define	DssRemoteIP_LEN			24
#define	DssRemotePort_LEN		24
#define	DssPort_LEN			24
#define	DSSGV_LEN			24*4

#define	F_ADDR_DSSGV			F_ADDR_BIOS_RELEASE_DATE_END

#define	F_ADDR_DssTelNum		F_ADDR_DSSGV
#define	F_ADDR_DssRemoteIP		F_ADDR_DssTelNum + DssTelNum_LEN
#define	F_ADDR_DssRemotePort		F_ADDR_DssRemoteIP + DssRemoteIP_LEN
#define	F_ADDR_DssPort			F_ADDR_DssRemotePort + DssRemotePort_LEN

#define	F_ADDR_DSSGV_END		F_ADDR_DssPort + DssPort_LEN

//----------------------------------------------------------------------------
//	Contactless Reader Data (in WORDs)
//----------------------------------------------------------------------------
#define	MAX_CL_DATA_INIT_STATE_SIZE	2
#define	F_ADDR_CL_DATA_INIT_STATE	F_ADDR_DSSGV_END		// if INIT_STATE = 0xFFFF, clear all DATA = 0s and INIT_STATE = 0x1234

#define	F_ADDR_CL_DATA_INIT_STATE_END	F_ADDR_CL_DATA_INIT_STATE + MAX_CL_DATA_INIT_STATE_SIZE

//----------------------------------------------------------------------------
#define	MAX_CL_DATA_SIZE		1*1024		// 512 words
#define	F_ADDR_CL_DATA			F_ADDR_CL_DATA_INIT_STATE_END

#define	F_ADDR_CL_DATA_END		F_ADDR_CL_DATA + MAX_CL_DATA_SIZE

//----------------------------------------------------------------------------
//	FISC TMK STATE
//	Fixed 16 bytes (free format defined by AP layer)
//----------------------------------------------------------------------------
#define	FISC_TMK_STATE_LEN		16
#define	F_ADDR_FISC_TMK_STATE		F_ADDR_CL_DATA_END

#define	F_ADDR_FISC_TMK_STATE_END	F_ADDR_FISC_TMK_STATE + FISC_TMK_STATE_LEN

//----------------------------------------------------------------------------
//	NAND Flash Initial BBT (Bad Block Table) Flag
//
//	0xFFFF	= iBBT is not avaliable, to get it from NAND Flash.
//	0x1234	= iBBT is available, to get it from "F_ADDR_NAND_FLASH_BBT".
//----------------------------------------------------------------------------
#define	NAND_FLASH_BBT_FLAG_LEN		2

#define	F_ADDR_NAND_FLASH_BBT_FLAG	F_ADDR_FISC_TMK_STATE_END
#define	F_ADDR_NAND_FLASH_BBT_FLAG_END	F_ADDR_NAND_FLASH_BBT_FLAG + NAND_FLASH_BBT_FLAG_LEN

//----------------------------------------------------------------------------
//	NAND Flash iBBT (Initial Bad Block Table)
//
//	128 Bytes for 1024 blocks. (1 bit = 1 block)
//	BYTE 0	bit 0 = block 0
//	BYTE 0	bit 1 = block 1
//	...
//	BYTE 127 bit 7 = block 1023
//----------------------------------------------------------------------------
#define	NAND_FLASH_BBT_LEN		128

#define	F_ADDR_NAND_FLASH_BBT		F_ADDR_NAND_FLASH_BBT_FLAG_END
#define	F_ADDR_NAND_FLASH_BBT_END	F_ADDR_NAND_FLASH_BBT + NAND_FLASH_BBT_LEN

//----------------------------------------------------------------------------
//	ECC TSN (removed 2018/01/25)
//----------------------------------------------------------------------------
//#define	TSN_ECC_LEN			4
//
//#define	F_ADDR_TSN_ECC			F_ADDR_NAND_FLASH_BBT_END
//#define	F_ADDR_TSN_ECC_END		F_ADDR_TSN_ECC + TSN_ECC_LEN

//----------------------------------------------------------------------------
//	ECC SECM Restore
//	ADDR_EFC_KEY_SLOT_01 <->	FLASH
//----------------------------------------------------------------------------
#define	ECC_SECM_LEN			2048

#define	F_ADDR_ECC_SECM			F_ADDR_NAND_FLASH_BBT_END
#define	F_ADDR_ECC_SECM_END		F_ADDR_ECC_SECM + ECC_SECM_LEN

//----------------------------------------------------------------------------
//	EDC_UID (Unique ID, 8 digits)
//		used to replace the UID of MCU
//----------------------------------------------------------------------------
#define	UID_LEN				16		// b16, LEN(1)+UID(15)

#define	F_ADDR_UID			F_ADDR_ECC_SECM_END
#define	F_ADDR_UID_END			F_ADDR_UID + UID_LEN

//----------------------------------------------------------------------------
//	2'nd Secure Memory
//----------------------------------------------------------------------------
#define ADDR_FLASH_BACKUP_LAST  F_ADDR_UID_END
#include "OS_SECM2.h"			// 2'nd secure memory for PEDS API


//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
extern	UINT32	FLASH_EraseSector( UINT32 SectorBase );
extern	UINT32	FLASH_WriteData( void *BaseAddr, void *pData, UINT32 Len );
extern	void	FLASH_ReadData( void *BaseAddr, void *pData, UINT32 Len );

extern	UINT32	OS_FLS_PutData( UINT32 addr, UINT32 len, UINT8 *data );
extern	void	OS_FLS_GetData( UINT32 addr, UINT32 len, UINT8 *data );

//----------------------------------------------------------------------------
#endif
