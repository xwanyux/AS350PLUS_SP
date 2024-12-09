//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L                                                       **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : OS_FLASH.H                                                 **
//**  MODULE   : Definition of FLASH backup area.	                    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2009/10/02                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2009 SymLink Corporation. All rights reserved.           **
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
#ifdef	_FLASH_4MB_
#define	FLASH_BACKUP_BASE_ADDR		0x103F0000			// 4MB FLASH (64KB)
#endif

#ifdef	_FLASH_8MB_
#define	FLASH_BACKUP_BASE_ADDR		0x107F0000			// 8MB FLASH (64KB)
#endif

#ifdef	_FLASH_16MB_
#define	FLASH_BACKUP_BASE_ADDR		0x10FE0000			// 16MB FLASH (128KB boundary)
#endif

#ifdef	_FLASH_32MB_
#define	FLASH_BACKUP_BASE_ADDR		0x11FE0000			// 32MB FLASH (128KB boundary)
#endif

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
//	Flag for TAMPER detection
//	FFFF = ON  (default to detect tamper status)
//	5AA5 = OFF (not to detect tamper status)
//----------------------------------------------------------------------------
#define	FLAG_DETECT_TAMPER_ON		0xFFFF
#define	FLAG_DETECT_TAMPER_OFF		0x5AA5

#define	TAMPER_DETECT_LEN		2
#define	F_ADDR_TAMPER_DETECT		F_ADDR_CL_PARA_END

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
#define	F_ADDR_REF_F97			F_ADDR_SYS_EVENT_LOG_END

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
//	Function Prototypes
//----------------------------------------------------------------------------
extern	UINT32	FLASH_EraseSector( UINT32 SectorBase );
extern	UINT32	FLASH_WriteData( void *BaseAddr, void *pData, UINT32 Len );
extern	void	FLASH_ReadData( void *BaseAddr, void *pData, UINT32 Len );

extern	UINT32	OS_FLS_PutData( UINT32 addr, UINT32 len, UINT8 *data );
extern	void	OS_FLS_GetData( UINT32 addr, UINT32 len, UINT8 *data );

//----------------------------------------------------------------------------
#endif
