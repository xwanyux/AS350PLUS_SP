//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  :							                                **
//**  PRODUCT  : AS350-X6							                        **
//**                                                                        **
//**  FILE     : ANS_TR31.H                                                 **
//**  MODULE   : Declaration of ANSI TR-31 Module.	                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2022/05/24                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2008-2022 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _ANS_TR31_2010_H_
#define _ANS_TR31_2010_H_

//----------------------------------------------------------------------------
#include "POSAPI.h"


//----------------------------------------------------------------------------
//      Key Usage (defines the type of the key)
//----------------------------------------------------------------------------
#define	KU_BDK			0x4230		// B0, Base Derivation Key
#define	KU_CVK			0x4330		// C0, Card Verification Key
#define	KU_DATA_ENC		0x4430		// D0, Data Encryption
#define	KU_ICMK_AC		0x4530		// E0, EMV/Chip Card Master Key: Application Cryptograms
#define	KU_ICMK_SMC		0x4531		// E1, EMV/Chip Card Master Key: Secure Messaging for Confidentiality
#define	KU_ICMK_SMI		0x4532		// E2, EMV/Chip Card Master Key: Secure Messaging for Integrity
#define	KU_ICMK_DAC		0x4533		// E3, EMV/Chip Card Master Key: Data Authentication Code
#define	KU_ICMK_DNUM		0x4534		// E4, EMV/Chip Card Master Key: Dynamic Numbers
#define	KU_ICMK_CPERSO		0x4535		// E5, EMV/Chip Card Master Key: Card Personalization
#define	KU_ICMK_OTHER		0x4536		// E6, EMV/Chip Card Master Key: Other
#define	KU_IV			0x4930		// I0, Initial Vector
#define	KU_KEY_ENCRYPT		0x4B30		// K0, Key Encryption or Wrapping
#define	KU_MAC_ALG1_ISO16609	0x4D30		// M0, ISO 16609 MAC Algorithm 1 (TDES)
#define	KU_MAC_ALG1_ISO97971	0x4D31		// M1, ISO 9797-1 MAC Algorithm 1
#define	KU_MAC_ALG2_ISO97971	0x4D32		// M2, ISO 9797-1 MAC Algorithm 2
#define	KU_MAC_ALG3_ISO97971	0x4D33		// M3, ISO 9797-1 MAC Algorithm 3
#define	KU_MAC_ALG4_ISO97971	0x4D34		// M4, ISO 9797-1 MAC Algorithm 4
#define	KU_MAC_ALG5_ISO97971	0x4D35		// M5, ISO 9797-1 MAC Algorithm 5
#define	KU_PIN_ENCRYPT		0x5030		// P0, PIN Encryption
#define	KU_PV_KPV		0x5630		// V0, PIN Verification - KPV, other algorithm
#define	KU_PV_IBM3624		0x5631		// V1, PIN Verification - IBM 3624
#define	KU_PV_VISA_PVV		0x5632		// V2, PIN Verification - VISA PVV

//----------------------------------------------------------------------------
//      Key Algorithm (defines what algorithms can be used with this key)
//----------------------------------------------------------------------------
#define	ALG_AES			0x41		// 'A'
#define	ALG_DEA			0x44		// 'D'
#define	ALG_ECC			0x45		// 'E'
#define	ALG_SHA1		0x48		// 'H'
#define	ALG_RSA			0x52		// 'R'
#define	ALG_DSA			0x53		// 'S', Digital Signature Algorithm
#define	ALG_TDES		0x54		// 'T', Triple DEA

//----------------------------------------------------------------------------
//      Mode of Use (defines the operation the key can perform)
//----------------------------------------------------------------------------
#define	MOU_ENC_DEC		0x42		// 'B', Both encrypt & decrypt
#define	MOU_MAC_CAL		0x43		// 'C', MAC calculate (generate & verify)
#define	MOU_DEC_ONLY		0x44		// 'D', Decrypt only
#define	MOU_ENC_ONLY		0x45		// 'E', Encrypt only
#define	MOU_MAC_GEN_ONLY	0x47		// 'G', MAC generate only
#define	MOU_NA			0x4E		// 'N', No special restrictions or not applicable
#define	MOU_SIGN_ONLY		0x53		// 'S', Signature only
#define	MOU_MAC_VER_ONLY	0x56		// 'V', MAC verify only

//----------------------------------------------------------------------------
//      Key Version Number (first character)
//----------------------------------------------------------------------------
#define	KVN_NOT_USED		0x30		// "00", not used for the key
#define	KVN_COMPONENT		0x63		// "cX", the value carried in this key block is a component of a key.

//----------------------------------------------------------------------------
//      Exportability
//----------------------------------------------------------------------------
#define	KEY_EXPORT		0x45		// 'E', Exportable under trusted key (MFK or KEK in key block format)
#define	KEY_NON_EXPORT		0x4E		// 'N', Non-Exportable
#define	KEY_SENSITIVE		0x53		// 'S', Sensitive, Exportable under trusted key (MFK or KEK in key block format)

//----------------------------------------------------------------------------
//      Key Block Header (KBH)
//----------------------------------------------------------------------------
#define	KEY_BUNDLE_LEN		80			// whole key bundle
#define	KEY_DATA_LEN		24			// KeyLen(2) + Key(16) + Padding(6)

typedef	struct	PED_KBH_S
{
	UINT8			VersionID[1];		// key block version id
	UINT8			BlockLen[4];		// key block length
	UINT8			Usage[2];		// key usage
	UINT8			Algorithm[1];		// algorithm
	UINT8			ModeOfUse[1];		// mode of use
	UINT8			VersionNo[2];		// key version number
	UINT8			Exportability[1];	// exportability
	UINT8			OptionBlocks[2];	// number of optional blocks
	UINT8			RFU[2];			// reserved field
} PED_KBH;

typedef	struct	PED_KEY_BUNDLE_S			// ASCII format
{
	UINT8			VersionID[1];		// key block version id
	UINT8			BlockLen[4];		// key block length
	UINT8			Usage[2];		// key usage
	UINT8			Algorithm[1];		// algorithm
	UINT8			ModeOfUse[1];		// mode of use
	UINT8			VersionNo[2];		// key version number
	UINT8			Exportability[1];	// exportability
	UINT8			OptionBlocks[2];	// number of optional blocks
	UINT8			RFU[2];			// reserved field
	
	UINT8			KeyLen[4];		// key length
	UINT8			Key[32];		// key data
	UINT8			Padding[12];		// random padding
	UINT8			MAC[16];		// MAC values after truncation	(TR-31 2010)
	
} PED_KEY_BUNDLE;

typedef	struct	PED_KEY_DATA_S				// HEX format
{	
	UINT8			KeyLen[2];		// key length
	UINT8			Key[16];		// key data
	UINT8			Padding[6];		// random padding
	UINT8			MAC[8];			// MAC values after truncation	(TR-31 2010)
	
} PED_KEY_DATA;

//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
extern	UINT32	TR31_VerifyKeyBundle( UINT16 Length, UINT8 *KeyBundle, UINT8 *eKeyData, UINT8 *mac, UINT8 *KpkSrc );
extern	UINT32	TR31_DecryptKeyBundle( UINT8 *icv, UINT8 *eKeyData, UINT8 *key, UINT8 *KpkSrc );

//----------------------------------------------------------------------------
#endif
