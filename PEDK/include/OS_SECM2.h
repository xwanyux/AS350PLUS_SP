//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : 							    **
//**  PRODUCT  : AS350 PLUS						    **
//**                                                                        **
//**  FILE     : DEV_SECM2.H						    **
//**  MODULE   : Declaration of the 2'nd Secure Memory Mapping.		    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2024/12/30                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2024 SymLink Corporation. All rights reserved.	    **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
//#ifndef _DEV_SECM2_H_
//#define _DEV_SECM2_H_

//----------------------------------------------------------------------------
//#include "bsp_types.h"
//#include "POSAPI.h"

//----------------------------------------------------------------------------
//#define	ADDR_SECM_LAST		0			// last address of SECM (NOTE: to be defined in OS_SECM.h)
#define ADDR_SECM2_START        ADDR_FLASH_BACKUP_LAST  // beginning address



//----------------------------------------------------------------------------
//	Master-Session Key
//
//	SIZE  		 : 20 bytes, L(1)+V(16)+KCV(3)
//	MASTER  KEY SLOTS: 20
//	SESSION KEY SLOTS: 20
//	ALGORITHM	 : TDES
//----------------------------------------------------------------------------
#define	MAX_MKEY2_CNT			10+10			// 2015-04-16, change cnt from 10 to 20
#define MAX_SKEY2_CNT			10+10			//
#define	PED_MSKEY2_LEN			16			//
#define PED_MSKEY2_SLOT_LEN		17			// L(1)+V(16)
#define PED_MSKEY2_SLOT_LEN2		20			// L(1)+V(16)+KCV(3)

#define	ADDR_PED_MKEY2_INDEX		ADDR_SECM2_START			// current master key index  (0..9)
#define	ADDR_PED_SKEY2_INDEX		ADDR_PED_MKEY2_INDEX+1			// current session key index (0..9)

#define ADDR_PED_MKEY2                  ADDR_PED_SKEY2_INDEX+1        		// b20, master key
#define ADDR_PED_MKEY2_01               ADDR_PED_MKEY2				//
#define	ADDR_PED_MKEY2_02		ADDR_PED_MKEY2_01+PED_MSKEY2_SLOT_LEN2	// #2~10 RFU
#define ADDR_PED_MKEY2_03		ADDR_PED_MKEY2_02+PED_MSKEY2_SLOT_LEN2  //
#define ADDR_PED_MKEY2_04		ADDR_PED_MKEY2_03+PED_MSKEY2_SLOT_LEN2  //
#define ADDR_PED_MKEY2_05		ADDR_PED_MKEY2_04+PED_MSKEY2_SLOT_LEN2  //
#define ADDR_PED_MKEY2_06		ADDR_PED_MKEY2_05+PED_MSKEY2_SLOT_LEN2  //
#define ADDR_PED_MKEY2_07		ADDR_PED_MKEY2_06+PED_MSKEY2_SLOT_LEN2  //
#define ADDR_PED_MKEY2_08		ADDR_PED_MKEY2_07+PED_MSKEY2_SLOT_LEN2  //
#define ADDR_PED_MKEY2_09		ADDR_PED_MKEY2_08+PED_MSKEY2_SLOT_LEN2  //
#define ADDR_PED_MKEY2_10		ADDR_PED_MKEY2_09+PED_MSKEY2_SLOT_LEN2  //

#define ADDR_PED_MKEY2_11		ADDR_PED_MKEY2_10+PED_MSKEY2_SLOT_LEN2	// 2015-04-16
#define ADDR_PED_MKEY2_12		ADDR_PED_MKEY2_11+PED_MSKEY2_SLOT_LEN2	//
#define ADDR_PED_MKEY2_13		ADDR_PED_MKEY2_12+PED_MSKEY2_SLOT_LEN2	//
#define ADDR_PED_MKEY2_14		ADDR_PED_MKEY2_13+PED_MSKEY2_SLOT_LEN2	//
#define ADDR_PED_MKEY2_15		ADDR_PED_MKEY2_14+PED_MSKEY2_SLOT_LEN2	//
#define ADDR_PED_MKEY2_16		ADDR_PED_MKEY2_15+PED_MSKEY2_SLOT_LEN2	//
#define ADDR_PED_MKEY2_17		ADDR_PED_MKEY2_16+PED_MSKEY2_SLOT_LEN2	//
#define ADDR_PED_MKEY2_18		ADDR_PED_MKEY2_17+PED_MSKEY2_SLOT_LEN2	//
#define ADDR_PED_MKEY2_19		ADDR_PED_MKEY2_18+PED_MSKEY2_SLOT_LEN2	//
#define ADDR_PED_MKEY2_20		ADDR_PED_MKEY2_19+PED_MSKEY2_SLOT_LEN2	//

#define ADDR_PED_SKEY2_01		ADDR_PED_MKEY2_20+PED_MSKEY2_SLOT_LEN2  // b72, session keys (x10)
#define ADDR_PED_SKEY2_02		ADDR_PED_SKEY2_01+PED_MSKEY2_SLOT_LEN   //
#define ADDR_PED_SKEY2_03		ADDR_PED_SKEY2_02+PED_MSKEY2_SLOT_LEN   //
#define ADDR_PED_SKEY2_04		ADDR_PED_SKEY2_03+PED_MSKEY2_SLOT_LEN   //
#define ADDR_PED_SKEY2_05		ADDR_PED_SKEY2_04+PED_MSKEY2_SLOT_LEN   //
#define ADDR_PED_SKEY2_06		ADDR_PED_SKEY2_05+PED_MSKEY2_SLOT_LEN   //
#define ADDR_PED_SKEY2_07		ADDR_PED_SKEY2_06+PED_MSKEY2_SLOT_LEN   //
#define ADDR_PED_SKEY2_08		ADDR_PED_SKEY2_07+PED_MSKEY2_SLOT_LEN   //
#define ADDR_PED_SKEY2_09		ADDR_PED_SKEY2_08+PED_MSKEY2_SLOT_LEN   //
#define ADDR_PED_SKEY2_10		ADDR_PED_SKEY2_09+PED_MSKEY2_SLOT_LEN   //

#define ADDR_PED_SKEY2_11		ADDR_PED_SKEY2_10+PED_MSKEY2_SLOT_LEN	// 2015-04-16
#define ADDR_PED_SKEY2_12		ADDR_PED_SKEY2_11+PED_MSKEY2_SLOT_LEN	//
#define ADDR_PED_SKEY2_13		ADDR_PED_SKEY2_12+PED_MSKEY2_SLOT_LEN	//
#define ADDR_PED_SKEY2_14		ADDR_PED_SKEY2_13+PED_MSKEY2_SLOT_LEN	//
#define ADDR_PED_SKEY2_15		ADDR_PED_SKEY2_14+PED_MSKEY2_SLOT_LEN	//
#define ADDR_PED_SKEY2_16		ADDR_PED_SKEY2_15+PED_MSKEY2_SLOT_LEN	//
#define ADDR_PED_SKEY2_17		ADDR_PED_SKEY2_16+PED_MSKEY2_SLOT_LEN	//
#define ADDR_PED_SKEY2_18		ADDR_PED_SKEY2_17+PED_MSKEY2_SLOT_LEN	//
#define ADDR_PED_SKEY2_19		ADDR_PED_SKEY2_18+PED_MSKEY2_SLOT_LEN	//
#define ADDR_PED_SKEY2_20		ADDR_PED_SKEY2_19+PED_MSKEY2_SLOT_LEN	//

#define	ADDR_PED_SKEY2_END		ADDR_PED_SKEY2_20+PED_MSKEY2_SLOT_LEN

//----------------------------------------------------------------------------
//      PIN Data (for PEDS API only)
//----------------------------------------------------------------------------
#define PEDS_PIN_LEN			16			// actual 4..12 digits
#define PEDS_PIN_SLOT_LEN		17			// L(1)+V(16)

#define ADDR_PEDS_PIN			ADDR_PED_SKEY2_END	// b17, L-V

#define ADDR_PEDS_PIN_END		ADDR_PEDS_PIN+PEDS_PIN_SLOT_LEN

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//#endif
