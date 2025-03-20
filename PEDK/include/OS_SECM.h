//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : i.MX6UL                                                    **
//**  PRODUCT  : AS350-X6 		                                            **
//**                                                                        **
//**  FILE     : OS_SECM.H						                            **
//**  MODULE   : Declaration of the Secure Memory Mapping.		            **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2022/05/24                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2022 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _DEV_SECM_H_
#define _DEV_SECM_H_

//----------------------------------------------------------------------------
// #include <mml.h>

#include "POSAPI.h"
#include "ANS_TR31_2010.h"
#include "ANS_TR31_2018.h"
#include "ANS_X9143_2022.h"
#include "OS_CAPK.h"


//----------------------------------------------------------------------------
//	Secure Memory	31K BYTES
//----------------------------------------------------------------------------
#define BLOB_OVERHEAD 48
#define	SEC_MEMORY_BASE			load_security_memory()
#define	SEC_MEMORY_SIZE			31*1024

//----------------------------------------------------------------------------
//	System Configuration Area
//	64 bytes
//----------------------------------------------------------------------------
#define	SYS_COLDBOOT_FLAG				    0x00000000
#define	SYS_WARMBOOT_FLAG				    0x12345678

#define	APP_NOT_READY_FLAG				    0x00
#define	APP_READY_FLAG					    0xAA

#define SYS_DISABLE_FLAG				    0x80	// 0x00
#define SYS_ENABLE_FLAG					    0x81	// 0x01

#define SYS_NOT_SUPPORT_FLAG			    0x00
#define SYS_SUPPORT_FLAG				    0x01

#define	ADDR_SYS_RESET_FLAG				    0								    // B4, beginning address
#define	ADDR_APP_CTRL_TBL				    ADDR_SYS_RESET_FLAG+4			    // B32, application control table
#define ADDR_SYS_SRED_STATUS			    ADDR_APP_CTRL_TBL+32			    // B1, SRED status
#define ADDR_SYS_LOAD_IKEK_FLAG			    ADDR_SYS_SRED_STATUS+1			    // B1, Load IKEK flag
#define ADDR_SYS_SETUP_128_TDES_KPK_FLAG	ADDR_SYS_LOAD_IKEK_FLAG+1		    // B1, Setup 128-bits TDES KPK flag
#define ADDR_SYS_SETUP_192_TDES_KPK_FLAG    ADDR_SYS_SETUP_128_TDES_KPK_FLAG+1  // B1, Setup 192-bits TDES KPK flag
#define ADDR_SYS_SETUP_128_AES_KPK_FLAG		ADDR_SYS_SETUP_192_TDES_KPK_FLAG+1	// B1, Setup 128-bits AES KPK flag
#define ADDR_SYS_SETUP_192_AES_KPK_FLAG		ADDR_SYS_SETUP_128_AES_KPK_FLAG+1	// B1, Setup 192-bits AES KPK flag
#define ADDR_SYS_SETUP_256_AES_KPK_FLAG		ADDR_SYS_SETUP_192_AES_KPK_FLAG+1	// B1, Setup 256-bits AES KPK flag
#define	ADDR_SYS_CONF_RFU				    ADDR_SYS_SETUP_256_AES_KPK_FLAG+1	// B21, RFU
#define ADDR_SYS_CONF_END				    ADDR_SYS_CONF_RFU+21

//----------------------------------------------------------------------------
#define	MAX_PED_KEY_MODE		3
#define	PED_KEY_MODE_NULL		0
//#define	PED_KEY_MODE_FIXED		1
#define	PED_KEY_MODE_MS			1
#define	PED_KEY_MODE_DUKPT		2
#define	PED_KEY_MODE_ISO4		3

#define	ADDR_PED_KEY_MODE		ADDR_SYS_CONF_END			// B1, PED_KEY_MODE_XXX
#define	ADDR_PED_KEY_MODE_END	ADDR_PED_KEY_MODE+1

//----------------------------------------------------------------------------
//	ANSI TR-31 (for TDES)
//	Key Block Protection Key (KPK)
//
//	SIZE  : 20 bytes
//	SLOTS : 1
//----------------------------------------------------------------------------
#define	PED_TDES_KEY_PROTECT_KEY_LEN		16
#define	PED_TDES_KEY_PROTECT_KEY_SLOT_LEN	1+16+3			//LEN(1)+KEY(16)+KCV(3)

#define ADDR_PED_TDES_KEY_PROTECT_KEY		ADDR_PED_KEY_MODE_END
#define	ADDR_PED_TDES_KEY_PROTECT_KEY_END	ADDR_PED_TDES_KEY_PROTECT_KEY+PED_TDES_KEY_PROTECT_KEY_SLOT_LEN

//----------------------------------------------------------------------------
//	ANSI X9.143 (for TDES)
//	16-byte Key Block Protection Key (KPK)
//
//	SIZE  : 20 bytes
//	SLOTS : 1
//----------------------------------------------------------------------------
#define	PED_128_TDES_KEY_PROTECT_KEY_LEN		16
#define	PED_128_TDES_KEY_PROTECT_KEY_SLOT_LEN	1+16+3			//LEN(1)+KEY(16)+KCV(3)

#define ADDR_PED_128_TDES_KEY_PROTECT_KEY		ADDR_PED_TDES_KEY_PROTECT_KEY_END
#define	ADDR_PED_128_TDES_KEY_PROTECT_KEY_END	ADDR_PED_128_TDES_KEY_PROTECT_KEY+PED_128_TDES_KEY_PROTECT_KEY_SLOT_LEN

//----------------------------------------------------------------------------
//	ANSI X9.143 (for TDES)
//	24-byte Key Block Protection Key (KPK)
//
//	SIZE  : 28 bytes
//	SLOTS : 1
//----------------------------------------------------------------------------
#define	PED_192_TDES_KEY_PROTECT_KEY_LEN		24
#define	PED_192_TDES_KEY_PROTECT_KEY_SLOT_LEN	1+24+3			//LEN(1)+KEY(24)+KCV(3)

#define ADDR_PED_192_TDES_KEY_PROTECT_KEY		ADDR_PED_128_TDES_KEY_PROTECT_KEY_END
#define	ADDR_PED_192_TDES_KEY_PROTECT_KEY_END	ADDR_PED_192_TDES_KEY_PROTECT_KEY+PED_192_TDES_KEY_PROTECT_KEY_SLOT_LEN

//----------------------------------------------------------------------------
//	ANSI X9.143 (for AES)
//	Key Block Protection Key (KPK)
//
//	SIZE  : 38 bytes
//	SLOTS : 1
//----------------------------------------------------------------------------
#define	PED_AES_KEY_PROTECT_KEY_LEN			32
#define	PED_AES_KEY_PROTECT_KEY_SLOT_LEN	1+32+5			//LEN(1)+KEY(32)+KCV(5)

#define ADDR_PED_AES_KEY_PROTECT_KEY		ADDR_PED_192_TDES_KEY_PROTECT_KEY_END
#define ADDR_PED_AES_KEY_PROTECT_KEY_END	ADDR_PED_AES_KEY_PROTECT_KEY+PED_AES_KEY_PROTECT_KEY_SLOT_LEN

//----------------------------------------------------------------------------
//	ANSI X9.143 (for AES)
//	16-byte Key Block Protection Key (KPK)
//
//	SIZE  : 22 bytes
//	SLOTS : 1
//----------------------------------------------------------------------------
#define	PED_128_AES_KEY_PROTECT_KEY_LEN			16
#define	PED_128_AES_KEY_PROTECT_KEY_SLOT_LEN	1+16+5			//LEN(1)+KEY(16)+KCV(5)

#define ADDR_PED_128_AES_KEY_PROTECT_KEY		ADDR_PED_AES_KEY_PROTECT_KEY_END
#define ADDR_PED_128_AES_KEY_PROTECT_KEY_END	ADDR_PED_128_AES_KEY_PROTECT_KEY+PED_128_AES_KEY_PROTECT_KEY_SLOT_LEN

//----------------------------------------------------------------------------
//	ANSI X9.143 (for AES)
//	24-byte Key Block Protection Key (KPK)
//
//	SIZE  : 30 bytes
//	SLOTS : 1
//----------------------------------------------------------------------------
#define	PED_192_AES_KEY_PROTECT_KEY_LEN			24
#define	PED_192_AES_KEY_PROTECT_KEY_SLOT_LEN	1+24+5			//LEN(1)+KEY(24)+KCV(5)

#define ADDR_PED_192_AES_KEY_PROTECT_KEY		ADDR_PED_128_AES_KEY_PROTECT_KEY_END
#define ADDR_PED_192_AES_KEY_PROTECT_KEY_END	ADDR_PED_192_AES_KEY_PROTECT_KEY+PED_192_AES_KEY_PROTECT_KEY_SLOT_LEN

//----------------------------------------------------------------------------
//	ANSI X9.143 (for AES)
//	32-byte Key Block Protection Key (KPK)
//
//	SIZE  : 38 bytes
//	SLOTS : 1
//----------------------------------------------------------------------------
#define	PED_256_AES_KEY_PROTECT_KEY_LEN			32
#define	PED_256_AES_KEY_PROTECT_KEY_SLOT_LEN	1+32+5			//LEN(1)+KEY(32)+KCV(5)

#define ADDR_PED_256_AES_KEY_PROTECT_KEY		ADDR_PED_192_AES_KEY_PROTECT_KEY_END
#define ADDR_PED_256_AES_KEY_PROTECT_KEY_END	ADDR_PED_256_AES_KEY_PROTECT_KEY+PED_256_AES_KEY_PROTECT_KEY_SLOT_LEN

//----------------------------------------------------------------------------
//  DUKPT
//	Key Serial Number Register (KSN)
//
//	SIZE  : 10 bytes
//	SLOTS : 1
//  FORMAT: Initial Key Serial Number Register(59 bits) +
//          Encryption Counter(21 bits)
//----------------------------------------------------------------------------
#define KSN_REG_LEN                     10

#define ADDR_KSN_REG                    ADDR_PED_256_AES_KEY_PROTECT_KEY_END

#define ADDR_KSN_REG_END                ADDR_KSN_REG+KSN_REG_LEN

//----------------------------------------------------------------------------
//  DUKPT
//	Future Key Register
//
//	SIZE     : 17 bytes
//	SLOTS    : 21
//	ALGORITHM: TDES
//  FORMAT   : A set of 21 registers, numbered #1 to #21.
//             Future PIN encrypiton key(16 bytes) + LRC(1 byte)
//----------------------------------------------------------------------------
#define MAX_FUTURE_KEY_REG_CNT          21
#define FUTURE_KEY_LEN                  16
#define FUTURE_KEY_SLOT_LEN             17

#define ADDR_FUTURE_KEY_REG             ADDR_KSN_REG_END

#define ADDR_FUTURE_KEY_REG_END         ADDR_FUTURE_KEY_REG+FUTURE_KEY_SLOT_LEN*MAX_FUTURE_KEY_REG_CNT

//----------------------------------------------------------------------------
//  DUKPT
//	MAC Key
//
//	SIZE     : 17 bytes
//	SLOTS    : 1
//	ALGORITHM: TDES
//	FORMAT   : L-V
//----------------------------------------------------------------------------
#define ADDR_MAC_KEY_REG        	ADDR_FUTURE_KEY_REG_END			// b17, L-V
#define	ADDR_MAC_KEY_REG_END		ADDR_MAC_KEY_REG+FUTURE_KEY_SLOT_LEN

//----------------------------------------------------------------------------
//  AES DUKPT
//	SIZE     : 16/24/32 bytes Base Derivation Key (BDK)
//	SLOTS    : 32
//	ALGORITHM: AES
//  FORMAT   : A set of 32 registers
//----------------------------------------------------------------------------
#define	MAX_NUM_REG			                32
#define	MAX_KEYLENGTH			            32

#define ADDR_AES_DUKPT_KEY_TYPE             ADDR_MAC_KEY_REG_END

// UCHAR	IntermediateDerivationKeyRegister[NUM_REG][KEYLENGTH]
#define	ADDR_INT_DERIVATION_KEY_REG		    ADDR_AES_DUKPT_KEY_TYPE+1
#define	ADDR_INT_DERIVATION_KEY_REG_END		ADDR_INT_DERIVATION_KEY_REG+(MAX_NUM_REG*MAX_KEYLENGTH)

// UCHAR	g_DerivationData[16]
//#define	ADDR_DERIVATION_DATA		ADDR_INT_DERIVATION_KEY_REG_END
//#define	ADDR_DERIVATION_DATA_END	ADDR_DERIVATION_DATA+16

// UCHAR	IntermediateDerivationKeyInUse[NUM_REG]
#define ADDR_INT_DERIVATION_KEY_IN_USE      ADDR_INT_DERIVATION_KEY_REG_END
#define ADDR_INT_DERIVATION_KEY_IN_USE_END  ADDR_INT_DERIVATION_KEY_IN_USE+MAX_NUM_REG

// ULONG	g_CurrentDerivationKey
#define ADDR_CUR_DERIVATION_KEY             ADDR_INT_DERIVATION_KEY_IN_USE_END
#define ADDR_CUR_DERIVATION_KEY_END         ADDR_CUR_DERIVATION_KEY+4

// ULONG	g_TransactionCounter
#define ADDR_TRANSACTION_COUNTER            ADDR_CUR_DERIVATION_KEY_END
#define ADDR_TRANSACTION_COUNTER_END        ADDR_TRANSACTION_COUNTER+4

//----------------------------------------------------------------------------
//	Dynamic Key Encryption Key (KEK)
//
//	SIZE     : 17 bytes
//	SLOTS    : 1
//	ALGORITHM: TDES
//	FORMAT   : L-V
//----------------------------------------------------------------------------
//#define	PED_KEK_LEN				16
//#define	PED_KEK_SLOT_LEN		17
//#define PED_DKEK_LEN			16
//#define PED_DKEK_SLOT_LEN		1+16+3							//LEN(1)+KEY(16)+KCV(3)

//#define ADDR_PED_KEK            ADDR_MAC_KEY_REG_END			// b17, L-V, key encryption key
//#define ADDR_PED_IKEK           ADDR_PED_KEK+PED_KEK_SLOT_LEN	// b17, L-V, initial key encryption key
//#define ADDR_PED_DKEK			ADDR_PED_IKEK+PED_KEK_SLOT_LEN	//b20, dynamic key encryption key

//#define ADDR_PED_DKEK_END		ADDR_PED_DKEK+PED_DKEK_SLOT_LEN

//----------------------------------------------------------------------------
//	Master-Session Key
//
//	SIZE  		     : 80 bytes (ANS TR-31 key bundle format)
//	MASTER  KEY SLOTS: 1
//	SESSION KEY SLOTS: 10
//	ALGORITHM	     : TDES
//----------------------------------------------------------------------------
#define	MAX_MKEY_CNT			1										//
#define MAX_SKEY_CNT            10										//
#define	PED_MSKEY_LEN			16										//
//#define PED_MSKEY_SLOT_LEN      KEY_BUNDLE_LEN							//
#define PED_MSKEY_SLOT_LEN      X9143_TDES_KEY_BUNDLE_LEN				//  // ==== [Debug] ====
#define PED_MSKEY_SLOT_LEN2     20										// used for API_PEDS.c

#define	ADDR_PED_MKEY_INDEX		ADDR_TRANSACTION_COUNTER_END			// current master key index  (0)
#define	ADDR_PED_SKEY_INDEX		ADDR_PED_MKEY_INDEX+1					// current session key index (0..9)

#define ADDR_PED_MKEY           ADDR_PED_SKEY_INDEX+1        			// b80, master key (x1)
#define ADDR_PED_MKEY_01        ADDR_PED_MKEY							//

#define ADDR_PED_SKEY_01        ADDR_PED_MKEY_01+PED_MSKEY_SLOT_LEN     // b80, session keys (x10)
#define ADDR_PED_SKEY_02        ADDR_PED_SKEY_01+PED_MSKEY_SLOT_LEN     //
#define ADDR_PED_SKEY_03        ADDR_PED_SKEY_02+PED_MSKEY_SLOT_LEN     //
#define ADDR_PED_SKEY_04        ADDR_PED_SKEY_03+PED_MSKEY_SLOT_LEN     //
#define ADDR_PED_SKEY_05        ADDR_PED_SKEY_04+PED_MSKEY_SLOT_LEN     //
#define ADDR_PED_SKEY_06        ADDR_PED_SKEY_05+PED_MSKEY_SLOT_LEN     //
#define ADDR_PED_SKEY_07        ADDR_PED_SKEY_06+PED_MSKEY_SLOT_LEN     //
#define ADDR_PED_SKEY_08        ADDR_PED_SKEY_07+PED_MSKEY_SLOT_LEN     //
#define ADDR_PED_SKEY_09        ADDR_PED_SKEY_08+PED_MSKEY_SLOT_LEN     //
#define ADDR_PED_SKEY_10        ADDR_PED_SKEY_09+PED_MSKEY_SLOT_LEN     //

#define	ADDR_PED_SKEY_END		ADDR_PED_SKEY_10+PED_MSKEY_SLOT_LEN

//----------------------------------------------------------------------------
//	PEK: PIN Encrption Key
//  PEK Master-Session Key
//	SIZE  		     : 96 or 144 bytes (ANSI X9.143 key bundle format)
//	MASTER  KEY SLOTS: 1
//	SESSION KEY SLOTS: 10
//	ALGORITHM	     : TDES/AES
//----------------------------------------------------------------------------
#define	MAX_PEK_MKEY_CNT			    1										                //
#define MAX_PEK_SKEY_CNT                10										                //
#define	PED_PEK_MSKEY_LEN			    32										                //
#define PED_PEK_TDES_MSKEY_SLOT_LEN     X9143_TDES_KEY_BUNDLE_LEN			                    //
#define PED_PEK_AES_MSKEY_SLOT_LEN      X9143_AES_KEY_BUNDLE_LEN			                    //   

#define ADDR_PED_PEK_KEY_TYPE           ADDR_PED_SKEY_END                                       //

#define	ADDR_PED_PEK_MKEY_INDEX		    ADDR_PED_PEK_KEY_TYPE+1					                // current master key index  (0)
#define	ADDR_PED_PEK_SKEY_INDEX		    ADDR_PED_PEK_MKEY_INDEX+1					            // current session key index (0..9)

#define ADDR_PED_PEK_TDES_MKEY          ADDR_PED_PEK_SKEY_INDEX+1        			            // b96, master key (x1)
#define ADDR_PED_PEK_TDES_MKEY_01       ADDR_PED_PEK_TDES_MKEY							        //

#define ADDR_PED_PEK_TDES_SKEY_01       ADDR_PED_PEK_TDES_MKEY_01+PED_PEK_TDES_MSKEY_SLOT_LEN   // b96, session keys (x10)
#define ADDR_PED_PEK_TDES_SKEY_02       ADDR_PED_PEK_TDES_SKEY_01+PED_PEK_TDES_MSKEY_SLOT_LEN   //
#define ADDR_PED_PEK_TDES_SKEY_03       ADDR_PED_PEK_TDES_SKEY_02+PED_PEK_TDES_MSKEY_SLOT_LEN   //
#define ADDR_PED_PEK_TDES_SKEY_04       ADDR_PED_PEK_TDES_SKEY_03+PED_PEK_TDES_MSKEY_SLOT_LEN   //
#define ADDR_PED_PEK_TDES_SKEY_05       ADDR_PED_PEK_TDES_SKEY_04+PED_PEK_TDES_MSKEY_SLOT_LEN   //
#define ADDR_PED_PEK_TDES_SKEY_06       ADDR_PED_PEK_TDES_SKEY_05+PED_PEK_TDES_MSKEY_SLOT_LEN   //
#define ADDR_PED_PEK_TDES_SKEY_07       ADDR_PED_PEK_TDES_SKEY_06+PED_PEK_TDES_MSKEY_SLOT_LEN   //
#define ADDR_PED_PEK_TDES_SKEY_08       ADDR_PED_PEK_TDES_SKEY_07+PED_PEK_TDES_MSKEY_SLOT_LEN   //
#define ADDR_PED_PEK_TDES_SKEY_09       ADDR_PED_PEK_TDES_SKEY_08+PED_PEK_TDES_MSKEY_SLOT_LEN   //
#define ADDR_PED_PEK_TDES_SKEY_10       ADDR_PED_PEK_TDES_SKEY_09+PED_PEK_TDES_MSKEY_SLOT_LEN   //

#define	ADDR_PED_PEK_TDES_SKEY_END		ADDR_PED_PEK_TDES_SKEY_10+PED_PEK_TDES_MSKEY_SLOT_LEN   //

#define ADDR_PED_PEK_AES_MKEY           ADDR_PED_PEK_TDES_SKEY_END+1        			        // b144, master key (x1)
#define ADDR_PED_PEK_AES_MKEY_01        ADDR_PED_PEK_AES_MKEY							        //							        //

#define ADDR_PED_PEK_AES_SKEY_01        ADDR_PED_PEK_AES_MKEY_01+PED_PEK_AES_MSKEY_SLOT_LEN     // b144, session keys (x10)
#define ADDR_PED_PEK_AES_SKEY_02        ADDR_PED_PEK_AES_SKEY_01+PED_PEK_AES_MSKEY_SLOT_LEN     //
#define ADDR_PED_PEK_AES_SKEY_03        ADDR_PED_PEK_AES_SKEY_02+PED_PEK_AES_MSKEY_SLOT_LEN     //
#define ADDR_PED_PEK_AES_SKEY_04        ADDR_PED_PEK_AES_SKEY_03+PED_PEK_AES_MSKEY_SLOT_LEN     //
#define ADDR_PED_PEK_AES_SKEY_05        ADDR_PED_PEK_AES_SKEY_04+PED_PEK_AES_MSKEY_SLOT_LEN     //
#define ADDR_PED_PEK_AES_SKEY_06        ADDR_PED_PEK_AES_SKEY_05+PED_PEK_AES_MSKEY_SLOT_LEN     //
#define ADDR_PED_PEK_AES_SKEY_07        ADDR_PED_PEK_AES_SKEY_06+PED_PEK_AES_MSKEY_SLOT_LEN     //
#define ADDR_PED_PEK_AES_SKEY_08        ADDR_PED_PEK_AES_SKEY_07+PED_PEK_AES_MSKEY_SLOT_LEN     //
#define ADDR_PED_PEK_AES_SKEY_09        ADDR_PED_PEK_AES_SKEY_08+PED_PEK_AES_MSKEY_SLOT_LEN     //
#define ADDR_PED_PEK_AES_SKEY_10        ADDR_PED_PEK_AES_SKEY_09+PED_PEK_AES_MSKEY_SLOT_LEN     //

#define	ADDR_PED_PEK_AES_SKEY_END		ADDR_PED_PEK_AES_SKEY_10+PED_PEK_AES_MSKEY_SLOT_LEN     //

//----------------------------------------------------------------------------
//	Fixed Key
//
//	SIZE 	 : 80 bytes (ANS TR-31 key bundle format)
//	SLOTS	 : 10
//	ALGORITHM: TDES
//----------------------------------------------------------------------------
//#define	MAX_FKEY_CNT			10										//
//#define	PED_FKEY_LEN			16										//
//#define	PED_FKEY_SLOT_LEN		KEY_BUNDLE_LEN							//

//#define	ADDR_PED_FKEY           ADDR_PED_SKEY_END						// b80, fixed key
//#define	ADDR_PED_FKEY_01		ADDR_PED_FKEY
//#define	ADDR_PED_FKEY_02		ADDR_PED_FKEY_01+PED_FKEY_SLOT_LEN
//#define	ADDR_PED_FKEY_03		ADDR_PED_FKEY_02+PED_FKEY_SLOT_LEN
//#define	ADDR_PED_FKEY_04		ADDR_PED_FKEY_03+PED_FKEY_SLOT_LEN
//#define	ADDR_PED_FKEY_05		ADDR_PED_FKEY_04+PED_FKEY_SLOT_LEN
//#define	ADDR_PED_FKEY_06		ADDR_PED_FKEY_05+PED_FKEY_SLOT_LEN
//#define	ADDR_PED_FKEY_07		ADDR_PED_FKEY_06+PED_FKEY_SLOT_LEN
//#define	ADDR_PED_FKEY_08		ADDR_PED_FKEY_07+PED_FKEY_SLOT_LEN
//#define	ADDR_PED_FKEY_09		ADDR_PED_FKEY_08+PED_FKEY_SLOT_LEN
//#define	ADDR_PED_FKEY_10		ADDR_PED_FKEY_09+PED_FKEY_SLOT_LEN

//#define	ADDR_PED_FKEY_END		ADDR_PED_FKEY_10+PED_FKEY_SLOT_LEN

//----------------------------------------------------------------------------
//	PIN Data
//----------------------------------------------------------------------------
#define PED_PIN_LEN                     16								// actual 4..12 digits
#define PED_PIN_SLOT_LEN                17								// L(1)+V(16)

#define ADDR_PED_PIN                    ADDR_PED_PEK_AES_SKEY_END		// b17, L-V

#define ADDR_PED_PIN_END                ADDR_PED_PIN+PED_PIN_SLOT_LEN

//----------------------------------------------------------------------------
//	Password Management
//----------------------------------------------------------------------------
#define MAX_PED_PSW_CNT			3
#define MIN_PED_PSW_LEN			7		// 2014-10-15
#define	MAX_PED_PSW_LEN			12
#define	PED_ADMIN_PSW_LEN		12
#define	PED_USER_PSW_LEN		12
#define	PED_PSW_SLOT_LEN		13

#define	ADDR_PED_ADMIN_PSW		ADDR_PED_PIN_END
#define	ADDR_PED_ADMIN_PSW1		ADDR_PED_PIN_END						// b13, L-V, administration password
#define	ADDR_PED_ADMIN_PSW2		ADDR_PED_ADMIN_PSW1+PED_PSW_SLOT_LEN	// b13, L-V, administration password

#define	ADDR_PED_USER_PSW		ADDR_PED_ADMIN_PSW2+PED_PSW_SLOT_LEN	// b13, L-V, user password

#define	ADDR_PED_DSS_PSW1		ADDR_PED_USER_PSW+PED_PSW_SLOT_LEN		// b13, L-V, user password (RFU)
#define	ADDR_PED_DSS_PSW2		ADDR_PED_DSS_PSW1+PED_PSW_SLOT_LEN		// b13, L-V, user password (RFU)

//SRED password
#define ADDR_PED_SRED_PSW1		ADDR_PED_DSS_PSW2+PED_PSW_SLOT_LEN		// b13, L-V, user password (RFU)
#define ADDR_PED_SRED_PSW2		ADDR_PED_SRED_PSW1+PED_PSW_SLOT_LEN		// b13, L-V, user password (RFU)

#define ADDR_PED_PSW_END		ADDR_PED_SRED_PSW2+PED_PSW_SLOT_LEN

//----------------------------------------------------------------------------
//	Terminal Parameters
//----------------------------------------------------------------------------
#define	PED_TERM_ID_LEN			8

#define	ADDR_PED_TERM_ID		ADDR_PED_PSW_END
#define	ADDR_PED_TERM_ID_END	ADDR_PED_TERM_ID+PED_TERM_ID_LEN

//----------------------------------------------------------------------------
//	ACC_DEK : Account Data-Encryption Key
//	ACC_DEK Master-Session Key
//	SIZE  		     : 96 or 144 bytes (ANSI X9.143 key bundle format)
//	MASTER  KEY SLOTS: 1
//	SESSION KEY SLOTS: 1
//	ALGORITHM	     : TDES/AES
//----------------------------------------------------------------------------
#define	PED_ACC_DEK_LEN					            24													                            // ACC_DEK session key (16) + ACC_DEK session key (8)
#define	MAX_ACC_DEK_MKEY_CNT			            1													                            //
#define MAX_ACC_DEK_SKEY_CNT			            1													                            //
//#define	PED_ACC_DEK_MSKEY_LEN			16													                                    //
#define	PED_ACC_DEK_MSKEY_LEN			            32													                            //
//#define PED_ACC_DEK_MSKEY_SLOT_LEN      KEY_BUNDLE_LEN										                                    //
#define PED_ACC_DEK_MSKEY_SLOT_LEN                  X9143_TDES_KEY_BUNDLE_LEN							                            //
#define PED_ACC_DEK_TDES_MSKEY_SLOT_LEN             X9143_TDES_KEY_BUNDLE_LEN							                            //
#define PED_ACC_DEK_AES_MSKEY_SLOT_LEN              X9143_AES_KEY_BUNDLE_LEN							                            //
#define PED_ACC_DEK_AES_DUKPT_WORKING_KEY_LEN       1+MAX_KEYLENGTH                                                                 //LEN(1)+KEY(32)

#define ADDR_PED_AccDEK_KEY_TYPE                    ADDR_PED_TERM_ID_END                                                            //

#define	ADDR_PED_ACC_DEK_MKEY_INDEX		            ADDR_PED_AccDEK_KEY_TYPE+1								                        // current master key index  (0)
#define	ADDR_PED_ACC_DEK_SKEY_INDEX		            ADDR_PED_ACC_DEK_MKEY_INDEX+1						                            // current session key index (0)

#define ADDR_PED_ACC_DEK_MKEY			            ADDR_PED_ACC_DEK_SKEY_INDEX+1        				                            // b80, master key (x1)
#define ADDR_PED_ACC_DEK_MKEY_01		            ADDR_PED_ACC_DEK_MKEY								                            //

#define ADDR_PED_ACC_DEK_SKEY_01                    ADDR_PED_ACC_DEK_MKEY_01+PED_ACC_DEK_MSKEY_SLOT_LEN	                            // b80, session keys (x1)
#define	ADDR_PED_ACC_DEK_SKEY_END		            ADDR_PED_ACC_DEK_SKEY_01+PED_ACC_DEK_MSKEY_SLOT_LEN                             //

#define ADDR_PED_ACC_DEK_TDES_MKEY		            ADDR_PED_ACC_DEK_SKEY_END+1        				                                // b96, master key (x1)
#define ADDR_PED_ACC_DEK_TDES_MKEY_01	            ADDR_PED_ACC_DEK_TDES_MKEY                                                      //

#define ADDR_PED_ACC_DEK_TDES_SKEY_01               ADDR_PED_ACC_DEK_TDES_MKEY_01+PED_ACC_DEK_TDES_MSKEY_SLOT_LEN	                // b96, session keys (x1)
#define	ADDR_PED_ACC_DEK_TDES_SKEY_END	            ADDR_PED_ACC_DEK_TDES_SKEY_01+PED_ACC_DEK_TDES_MSKEY_SLOT_LEN                   //

#define ADDR_PED_ACC_DEK_AES_MKEY		            ADDR_PED_ACC_DEK_TDES_SKEY_END+1        				                        // b144, master key (x1)
#define ADDR_PED_ACC_DEK_AES_MKEY_01	            ADDR_PED_ACC_DEK_AES_MKEY                                                       //

#define ADDR_PED_ACC_DEK_AES_SKEY_01                ADDR_PED_ACC_DEK_AES_MKEY_01+PED_ACC_DEK_AES_MSKEY_SLOT_LEN	                    // b144, session keys (x1)
#define	ADDR_PED_ACC_DEK_AES_SKEY_END	            ADDR_PED_ACC_DEK_AES_SKEY_01+PED_ACC_DEK_AES_MSKEY_SLOT_LEN                     //

#define ADDR_PED_ACC_DEK_AES_DUKPT_WORKING_KEY      ADDR_PED_ACC_DEK_AES_SKEY_END
#define ADDR_PED_ACC_DEK_AES_DUKPT_WORKING_KEY_END  ADDR_PED_ACC_DEK_AES_DUKPT_WORKING_KEY+PED_ACC_DEK_AES_DUKPT_WORKING_KEY_LEN    //LEN(1)+KEY(32)
#define ADDR_PED_ACC_DEK_CURRENT_KSN                ADDR_PED_ACC_DEK_AES_DUKPT_WORKING_KEY_END                                      //
#define ADDR_PED_ACC_DEK_CURRENT_KSN_END            ADDR_PED_ACC_DEK_CURRENT_KSN+12                                                 //

//----------------------------------------------------------------------------
//	Secret Key
//	DESCRIPTION : A data encryption key generated by mutual authentication is 
//				  used for remote key loading
//----------------------------------------------------------------------------
//#define PED_SECRET_KEY_LEN			16
//#define ADDR_PED_SECRET_KEY			ADDR_PED_ACC_DEK_END
//#define ADDR_PED_SECRET_KEY_END		ADDR_PED_SECRET_KEY + PED_SECRET_KEY_LEN

//----------------------------------------------------------------------------
//	Salt
//----------------------------------------------------------------------------
#define PED_SALT_LEN				16

#define ADDR_PED_SALT				ADDR_PED_ACC_DEK_CURRENT_KSN_END
#define ADDR_PED_SALT_END			ADDR_PED_SALT + PED_SALT_LEN

//----------------------------------------------------------------------------
//	PAN Data
//----------------------------------------------------------------------------
#define PED_PAN_DATA_LEN			64	//plaintext PAN(12~19 digits), 24/32 bytes ciphertext PAN with 32 bytes MAC
#define PED_PAN_DATA_SLOT_LEN		65	//L-V

#define ADDR_PED_PAN_DATA			ADDR_PED_SALT_END
#define ADDR_PED_PAN_DATA_END		ADDR_PED_PAN_DATA + PED_PAN_DATA_SLOT_LEN

//----------------------------------------------------------------------------
//	Account Data
//----------------------------------------------------------------------------
#define PED_ACC_DATA_LEN			562
#define LENGTH_TAG57				24	//Track 2 Equivalent Data
#define LENGTH_TAG5A				32	//Application Primary Account Number (PAN)
#define LENGTH_TAG9F1F				255	//Track 1 Discretionary Data
#define LENGTH_TAG9F20				255	//Track 2 Discretionary Data

#define ADDR_PED_ACC_DATA			ADDR_PED_PAN_DATA_END
#define ADDR_TAG57_L				ADDR_PED_ACC_DATA
#define ADDR_TAG57_V				ADDR_TAG57_L + 1
#define ADDR_TAG5A_L				ADDR_TAG57_V + LENGTH_TAG57
#define ADDR_TAG5A_V				ADDR_TAG5A_L + 1
#define ADDR_TAG9F1F_L				ADDR_TAG5A_V + LENGTH_TAG5A
#define ADDR_TAG9F1F_V				ADDR_TAG9F1F_L + 1
#define ADDR_TAG9F20_L				ADDR_TAG9F1F_V + LENGTH_TAG9F1F
#define ADDR_TAG9F20_V				ADDR_TAG9F20_L + 1
#define ADDR_PED_ACC_DATA_END		ADDR_TAG9F20_V + LENGTH_TAG9F20

//----------------------------------------------------------------------------
//	ISO4_KEY : AES KEY (128 bits) for ISO9564-1:2017 format 4
//	ISO4_KEY Master-Session Key
//	SIZE  		     : 112 bytes (ANS TR-31 key bundle format)
//	MASTER  KEY SLOTS: 1
//	SESSION KEY SLOTS: 1
//	ALGORITHM	     : AES
//----------------------------------------------------------------------------
#define PED_ISO4_KEY_LEN				16
//#define	MAX_ISO4_KEY_MKEY_CNT			1														//
#define MAX_ISO4_KEY_SKEY_CNT			1														//
#define	PED_ISO4_KEY_MSKEY_LEN			16														//
#define PED_ISO4_KEY_MSKEY_SLOT_LEN		KEY_BUNDLE_LEN2											//
//#define PED_ISO4_KEY_SLOT_LEN			KEY_BUNDLE_LEN2
#define PED_ISO4_KEY_SLOT_LEN			X9143_AES_KEY_BUNDLE_LEN    // ==== [Debug] ====

//#define	ADDR_PED_ISO4_KEY_MKEY_INDEX	ADDR_PED_ACC_DATA_END									// current master key index  (0)
//#define	ADDR_PED_ISO4_KEY_SKEY_INDEX	ADDR_PED_ISO4_KEY_MKEY_INDEX+1							// current session key index (0)

//#define ADDR_PED_ISO4_KEY_MKEY			ADDR_PED_ISO4_KEY_SKEY_INDEX+1        					// b112, master key (x1)
//#define ADDR_PED_ISO4_KEY_MKEY_01		ADDR_PED_ISO4_KEY_MKEY									//

//#define ADDR_PED_ISO4_KEY_SKEY_01       ADDR_PED_ISO4_KEY_MKEY_01+PED_ISO4_KEY_MSKEY_SLOT_LEN	// b112, session keys (x1)
//#define	ADDR_PED_ISO4_KEY_SKEY_END		ADDR_PED_ISO4_KEY_SKEY_01+PED_ISO4_KEY_MSKEY_SLOT_LEN

#define ADDR_PED_ISO4_KEY				ADDR_PED_ACC_DATA_END									// b112, ISO4_KEY
#define	ADDR_PED_ISO4_KEY_END			ADDR_PED_ISO4_KEY + PED_ISO4_KEY_SLOT_LEN

//----------------------------------------------------------------------------
//	FPE Key : Format-Preserving Encryption Key
//	FPE Master-Session Key
//	SIZE  		     : 144 bytes (ANSI X9.143 key bundle format)
//	MASTER  KEY SLOTS: 1
//	SESSION KEY SLOTS: 1
//	ALGORITHM	     : AES
//----------------------------------------------------------------------------
#define PED_FPE_KEY_LEN					            16
#define	MAX_FPE_KEY_MKEY_CNT			            1														                        //
#define MAX_FPE_KEY_SKEY_CNT			            1														                        //
#define	PED_FPE_KEY_MSKEY_LEN			            16														                        //
#define PED_FPE_KEY_MSKEY_SLOT_LEN	                X9143_AES_KEY_BUNDLE_LEN											            //
#define PED_FPE_KEY_AES_MSKEY_SLOT_LEN	            X9143_AES_KEY_BUNDLE_LEN											            //
#define PED_FPE_KEY_SLOT_LEN			            KEY_BUNDLE_LEN2
#define PED_FPE_KEY_AES_DUKPT_WORKING_KEY_LEN       1+16                                                                            //LEN(1)+KEY(16)

#define ADDR_PED_FPE_KEY_TYPE                       ADDR_PED_ACC_DATA_END                                                           //

#define	ADDR_PED_FPE_KEY_MKEY_INDEX		            ADDR_PED_FPE_KEY_TYPE+1								                            // current master key index  (0)
#define	ADDR_PED_FPE_KEY_SKEY_INDEX		            ADDR_PED_FPE_KEY_MKEY_INDEX+1							                        // current session key index (0)

//#define ADDR_PED_FPE_KEY_MKEY			ADDR_PED_FPE_KEY_SKEY_INDEX+1        					// b112, master key (x1)
//#define ADDR_PED_FPE_KEY_MKEY_01		ADDR_PED_FPE_KEY_MKEY									//

//#define ADDR_PED_FPE_KEY_SKEY_01		ADDR_PED_FPE_KEY_MKEY_01+PED_FPE_KEY_MSKEY_SLOT_LEN		// b112, session keys (x1)
//#define	ADDR_PED_FPE_KEY_SKEY_END		ADDR_PED_FPE_KEY_SKEY_01+PED_FPE_KEY_MSKEY_SLOT_LEN

#define ADDR_PED_FPE_KEY_AES_MKEY		            ADDR_PED_FPE_KEY_SKEY_INDEX+1        					                        // b144, master key (x1)
#define ADDR_PED_FPE_KEY_AES_MKEY_01	            ADDR_PED_FPE_KEY_AES_MKEY                                                       //

#define ADDR_PED_FPE_KEY_AES_SKEY_01	            ADDR_PED_FPE_KEY_AES_MKEY_01+PED_FPE_KEY_AES_MSKEY_SLOT_LEN		                // b144, session keys (x1)
#define	ADDR_PED_FPE_KEY_AES_SKEY_END	            ADDR_PED_FPE_KEY_AES_SKEY_01+PED_FPE_KEY_AES_MSKEY_SLOT_LEN                     //

#define ADDR_PED_FPE_KEY				            ADDR_PED_ISO4_KEY_END                                                           //
#define	ADDR_PED_FPE_KEY_END			            ADDR_PED_FPE_KEY + PED_FPE_KEY_SLOT_LEN                                         //

#define ADDR_PED_FPE_KEY_AES_DUKPT_WORKING_KEY      ADDR_PED_FPE_KEY_AES_SKEY_END
#define ADDR_PED_FPE_KEY_AES_DUKPT_WORKING_KEY_END  ADDR_PED_FPE_KEY_AES_DUKPT_WORKING_KEY+PED_FPE_KEY_AES_DUKPT_WORKING_KEY_LEN    //LEN(1)+KEY(16)
#define ADDR_PED_FPE_KEY_CURRENT_KSN                ADDR_PED_FPE_KEY_AES_DUKPT_WORKING_KEY_END                                      //
#define ADDR_PED_FPE_KEY_CURRENT_KSN_END            ADDR_PED_FPE_KEY_CURRENT_KSN+12                                                 //

//----------------------------------------------------------------------------
//	CAPK
//
//----------------------------------------------------------------------------
#define	ADDR_PED_CAPK			ADDR_PED_FPE_KEY_CURRENT_KSN_END
#define	ADDR_PED_CAPK_END		ADDR_PED_CAPK + (CAPK_KEY_SLOT_LEN*MAX_CAPK_CNT)

//----------------------------------------------------------------------------
//	Secret Information :
//----------------------------------------------------------------------------
#define EDC_PRV_KEY_SLOT_LEN     2048

#define ADDR_EDC_PRV_KEY                ADDR_PED_CAPK_END
#define ADDR_DEVICE_AUTH_STATUS         ADDR_EDC_PRV_KEY + EDC_PRV_KEY_SLOT_LEN // 2L-V
// #define ADDR_DEVICE_AUTH_STATUS         ADDR_PED_CAPK_END
#define ADDR_DEVICE_AUTH_DATE_TIME      ADDR_DEVICE_AUTH_STATUS + 1
#define ADDR_DEVICE_AUTH_DATE_TIME_END  ADDR_DEVICE_AUTH_DATE_TIME + 12

//----------------------------------------------------------------------------
//	Open Protocol : SSL
//----------------------------------------------------------------------------
#define CA_CERTIFICATE_SLOT_LEN     2048
#define CLIENT_CERTIFICATE_SLOT_LEN 5120
#define CLIENT_PRV_KEY_SLOT_LEN     2048

#define ADDR_CA_CERTIFICATE         ADDR_DEVICE_AUTH_DATE_TIME_END                         // 2L-V
#define ADDR_CLIENT_CERTIFICATE     ADDR_CA_CERTIFICATE + CA_CERTIFICATE_SLOT_LEN          // 2L-V
#define ADDR_CLIENT_PRV_KEY         ADDR_CLIENT_CERTIFICATE + CLIENT_CERTIFICATE_SLOT_LEN  // 2L-V
#define ADDR_CLIENT_PRV_KEY_END     ADDR_CLIENT_PRV_KEY + CLIENT_PRV_KEY_SLOT_LEN          // 2L-V

//----------------------------------------------------------------------------
//	DHCP
//	0xFFFF or 0x0000	= disable DHCP
//	0x0001			    = enable DHCP
//----------------------------------------------------------------------------
#define	DHCP_FLAG_ENABLE		0x0001
#define	DHCP_FLAG_DISABLE		0x0000
#define	DHCP_FLAG_LEN			2

#define	ADDR_DHCP_FLAG			ADDR_CLIENT_PRV_KEY_END
#define	ADDR_DHCP_FLAG_END		ADDR_DHCP_FLAG + DHCP_FLAG_LEN

//----------------------------------------------------------------------------
//	File Structure
//----------------------------------------------------------------------------
typedef struct FILE_INFO_S
{
    char    *fileName;
    UINT16  fileSize;
} FILE_INFO;

//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
extern  void    getSecureFileName(UINT32 address, char *fileName);
extern  void    init_secure_file();
extern  UINT8*  load_secure_file(char *fileName);
extern  UINT8   update_secure_file(char *fileName, UINT8 *dataPtr);

extern	void	init_secure_memory();
extern	UINT8*	load_security_memory();
extern	UINT8	update_security_memory(UINT8 *ptr_sm);

extern	UINT32	OS_SECM_VerifySredStatus(void);
extern	UINT32	OS_SECM_SetSredStatus(UINT8 flag);

extern	UINT32	OS_SECM_Init( void );

extern	UINT32	OS_SECM_VerifyBootStatus( void );
extern	UINT32	OS_SECM_SetWarmBootStatus( void );
extern	UINT32	OS_SECM_ResetBootStatus( void );
extern	UINT32	OS_SECM_VerifyAppStatus( void );
extern	UINT32	OS_SECM_SetAppStatus( void );
extern	UINT32	OS_SECM_ResetAppStatus( void );
extern	void	OS_SECM_PutData( UINT32 address, UINT32 length, UINT8 *data );
extern	void	OS_SECM_GetData( UINT32 address, UINT32 length, UINT8 *data );
extern	void	OS_SECM_ClearData( UINT32 address, UINT32 length, UINT8 pattern );
extern  void    OS_SECM_ClearAllSecureFile(void);

extern  void    OS_SECM_PutIntDerivationKeyReg(UINT32 address, UINT32 length, UINT8 *data);
extern  void	OS_SECM_GetIntDerivationKeyReg(UINT32 address, UINT32 length, UINT8 *data);
extern  void	OS_SECM_ClearIntDerivationKeyReg(UINT32 address, UINT32 length, UINT8 pattern);

extern  void    OS_SECM_PutIntDerivationKeyInUse(UINT32 address, UINT32 length, UINT8 *data);
extern  void	OS_SECM_GetIntDerivationKeyInUse(UINT32 address, UINT32 length, UINT8 *data);
extern  void	OS_SECM_ClearIntDerivationKeyInUse(UINT32 address, UINT32 length, UINT8 pattern);

extern	void	OS_SECM_GetKeyData( UINT32 address, UINT32 length, UINT8 *data );
extern	void	OS_SECM_ClearKeyData( UINT32 address, UINT32 length, UINT8 pattern );
extern	void	OS_SECM_PutKeyData( UINT32 address, UINT32 length, UINT8 *data );

extern  void	OS_SECM_PutEdcPrvKey(UINT32 address, UINT32 length, UINT8 *data);
extern  void	OS_SECM_GetEdcPrvKey(UINT32 address, UINT32 length, UINT8 *data);

extern  void	OS_SECM_GetCaCert(UINT32 address, UINT32 length, UINT8 *data);
extern  void	OS_SECM_GetClientCert(UINT32 address, UINT32 length, UINT8 *data);
extern  void	OS_SECM_GetClientPrvKey(UINT32 address, UINT32 length, UINT8 *data);

//----------------------------------------------------------------------------
#endif
