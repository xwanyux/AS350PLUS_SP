//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : i.MX6UL                                       				**
//**  PRODUCT  : AS350-X6						                            **
//**                                                                        **
//**  FILE     : SRED_FUNC.C                                                **
//**  MODULE   : 							                                **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2022/05/24                                                 **
//**  EDITOR   : Tammy							                            **
//**                                                                        **
//**  Copyright(C) 2022 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "POSAPI.h"
#include "UTILS.h"
#include "OS_LIB.h"
#include "OS_SECM.h"
#include "DEV_PED.h"

#include <ucl/ucl_defs.h>
#include <ucl/ucl_types.h>
#include <ucl/ucl_rng.h>
#include <ucl/ucl_hmac_sha256.h>

#include "SRED.h"
#include "FPE.h"
#include "SRED_DBG_Function.h"

#include "MPU.h"
#include "PEDKconfig.h"


extern	UINT32	os_ped_PanDownCnt;
extern	UINT32	os_ped_PanUpCnt;
extern  UINT32  os_ped_PanLife;

extern  UINT8   AccDek_workingKeyType;


// ---------------------------------------------------------------------------
// FUNCTION: Verify the SRED status by checking "SYS_ENABLE_FLAG".
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  -- ready.
//           FALSE -- not ready.
// ---------------------------------------------------------------------------
UINT32	SRED_VerifySredStatus(void)
{
	UINT8	status;


	OS_SECM_GetData(ADDR_SYS_SRED_STATUS, 1, &status);

	if(status == SYS_ENABLE_FLAG)
		return TRUE;
	else
		return FALSE;
}

// ---------------------------------------------------------------------------
// FUNCTION: Enable the recipient of data to verify the identity of the data
//			 originator
// INPUT   : keyScheme - key opearation mode
//						 01 = FIXED
//						 02 = MASTER/SESSION
//						 03 = DUKPT
//			 index	   - key index
//			 mode      - algorithm and padding method
//					     MAC_ALGx + MAC_PADx
//			 dataIn    - the account data to be encrypted
//			 inLen	   - length of account data
// OUTPUT  : dataOut   - encrypted data concatenated with MAC value and the
//					     Starting Variable
//			 outLen    - length of encrypted data concatenated with MAC value
//                       and the Starting Variable
// RETURN  : TRUE
//			 FALSE
// ---------------------------------------------------------------------------
UINT8	SRED_Func_UseDOA(UINT8 keyScheme, UINT8 index, UINT8 mode, UINT8 *dataIn, UINT32 inLen, UINT8 *dataOut, UINT32 *outLen)
{
	UINT8	buffer[6];
	UINT8	s[8];	//Starting Variable
	UINT8	encryptedStr[SRED_BUFFER_SIZE];	//Encrypted Data String + Tag
	UINT32  length = 0;
	UINT8	result = FALSE;
	

	SRED_DBG_Put_String(40, (UINT8*)"==== MAC Algorithm & Padding Method ====");	// ==== [Debug] ====
	SRED_DBG_Put_UCHAR(mode);	// ==== [Debug] ====

	//Generate the Starting Variable at random
	api_sys_random(s);

	SRED_DBG_Put_String(29, (UINT8*)"==== Starting Variable S ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(8, s);	// ==== [Debug] ====

	//Encryption procedure (on the originating end)
	SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====
	SRED_DBG_Put_String(30, (UINT8*)"**** Encryption procedure ****");	// ==== [Debug] ====
	SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====
	result = SRED_EncryptThenMAC(keyScheme, index, mode, UCL_CIPHER_ENCRYPT, s, dataIn, inLen, encryptedStr, &length);

	//The encrypted data adjoining MAC is outputted from device, together with the Starting Variable
	//+------------+-----+-------------------+
	//| Ciphertext | MAC | Starting Variable |
	//+------------+-----+-------------------+
	memmove(dataOut, encryptedStr, length);
	*outLen += length;
	memmove(&dataOut[length], s, 8);
	*outLen += 8;

	//Clear sensitive data
	memset(s, 0x00, sizeof(s));
	memset(encryptedStr, 0x00, sizeof(0x00));

	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: Encrypt PAN data and write it to NVSRAM
// INPUT   : pan - the PAN data to be encrypted and stored
//			 length - length of PAN data
//			 keyScheme - key opearation mode
//						 01 = FIXED
//						 02 = MASTER/SESSION
//						 03 = DUKPT
//			 index	   - key index
// OUTPUT  : none.
// RETURN  : TRUE
//			 FALSE
// NOTE	   : Limiting the rate at which the device will encrypt PANs.
//			 Min encrypt PIN interval = 30 sec
// ---------------------------------------------------------------------------
UINT8	SRED_Func_StorePAN(UINT8 *pan, UINT8 length, UINT8 keyScheme, UINT8 index)
{
	UINT8	rspCode = FALSE;
	UINT8	surrogate[19];
	UINT8	data[19];
    UINT8   dek[PED_ACC_DEK_MSKEY_LEN + 1]; // L-V
	UINT8	ctext[SRED_BUFFER_SIZE];
	UINT32	ctextLen = 0;
	UINT8	salt[PED_SALT_LEN];
	UINT8	extendedData[32 + 16];	//pan length + salt length
	UINT32	extendedLen = 0;
	UINT8	i;
	UINT8	ksn[12];
	UINT8	result;
	UINT8	macKey[16];
	UINT8	hash[UCL_SHA256_HASHSIZE];
	UINT8	proData[SRED_BUFFER_SIZE];
	UINT32	proLen = 0;
	UINT8	dataOfL = 0;
	UINT8	L = 0;	// ==== [Debug] ====
	UINT8	V[LENGTH_TAG5A];	// ==== [Debug] ====
    UINT8   keyLen = 0;


	//The protected data consists of ciphertext and MAC
	//+------------------+-----------------+------------------+------------------------+
	//| First six digits | Surrogate value | Last four digits |          MAC           |
	//+------------------+-----------------+------------------+------------------------+
	//|<---                   Encrypted                   --->|<--- Hash with salt --->|


	//Make sure system timer is active  
	os_ped_PanUpCnt = 0;
	LIB_WaitTime(2);
	while(os_ped_PanUpCnt == 0);

	//Check get PAN interval
	if(os_ped_PanDownCnt)
	{
		return FALSE;
	}

	//Enable the device to encrypt PAN data
	SRED_ResetEncryptLimit();

	//-------------------- Encryption --------------------

	//Generate a surrogate PAN values for sensitive middle digits
	result = SRED_GenerateSurrogatePAN(pan, length, surrogate);
	if(result == FALSE)
	{
		rspCode = FALSE;
		goto EXIT;
	}

	SRED_DBG_Put_String(22, (UINT8*)"==== Original PAN ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(length, pan);	// ==== [Debug] ====

	//The data that should be encrypted consists of first six digits, middle surrogate value and last four digits
	memmove(data, pan, 6);	//First six digits
	memmove(&data[6], surrogate, (length - 10));	//Sensitive middle digits
	memmove(&data[length - 4], &pan[length - 4], 4);	//Last four digits

	SRED_DBG_Put_String(23, (UINT8*)"==== Surrogate PAN ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(length, data);	// ==== [Debug] ====

	//Retrieve ACC_DEK
	if(SRED_GetAccDek(dek) == FALSE)
	{
		rspCode = FALSE;
		goto EXIT;
	}

	SRED_DBG_Put_String(29, (UINT8*)"==== Data encryption key ====");	// ==== [Debug] ====
    SRED_DBG_Put_Hex(dek[0], &dek[1]);  // ==== [Debug] ====

	//Encrypt the PAN data after the surrogate PAN value is generated by device
    if((PED_ReadKeyMode() == PED_KEY_MODE_MS) ||
       ((PED_ReadKeyMode() == PED_KEY_MODE_DUKPT) && (AccDek_workingKeyType == 1))) //TDES-192
        SRED_3DesEcb_Encrypt(ctext, &ctextLen, data, length, &dek[1]);
    else if((PED_ReadKeyMode() == PED_KEY_MODE_DUKPT) && (AccDek_workingKeyType >= 2))  //AES-128, AES-192, or AES-256
        SRED_AesEcb_Encrypt(ctext, &ctextLen, data, length, &dek[1]);
    else
    {
        rspCode = FALSE;
		goto EXIT;
    }

	SRED_DBG_Put_String(20, (UINT8*)"==== Ciphertext ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(ctextLen, ctext);	// ==== [Debug] ====

	//Write ciphertext and the length of ciphertext to Tag 5A L-V in NVSRAM
	dataOfL = ctextLen & 0x000000FF;
	OS_SECM_PutData(ADDR_TAG5A_L, 1, &dataOfL);
	OS_SECM_PutData(ADDR_TAG5A_V, ctextLen, ctext);

	OS_SECM_GetData(ADDR_TAG5A_L, 1, &L);	// ==== [Debug] ====
	SRED_DBG_Put_String(17, (UINT8*)"==== Tag5A_L ====");	// ==== [Debug] ====
	SRED_DBG_Put_UCHAR(L);	// ==== [Debug] ====
	OS_SECM_GetData(ADDR_TAG5A_V, L, V);	// ==== [Debug] ====
	SRED_DBG_Put_String(17, (UINT8*)"==== Tag5A_V ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(L, V);	// ==== [Debug] ====

	memmove(&proData[1], ctext, ctextLen);
	proLen += ctextLen;

	//-------------------- HASH --------------------

	//Salt is appended to the PAN
	//PAN : [0]   [1]   [2] ...... [length-1]
	//Salt :   [0]   [1]   [2] ...... [PED_SALT_LEN-1]
	SRED_GetSalt(salt);

	SRED_DBG_Put_String(14, (UINT8*)"==== Salt ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(PED_SALT_LEN, salt);	// ==== [Debug] ====

	for(i = 0 ; i < ctextLen ; i++)	//ciphertext length >= salt length
	{
		extendedData[extendedLen++] = ctext[i];
		if(i < PED_SALT_LEN)
		{
			extendedData[extendedLen++] = salt[i];
		}
	}

	if(ctextLen < PED_SALT_LEN)	//ciphertext length < salt length
	{
		memmove(&extendedData[extendedLen], &salt[i], (PED_SALT_LEN - i));
		extendedLen += PED_SALT_LEN - i;
	}

	SRED_DBG_Put_String(23, (UINT8*)"==== PAN with salt ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(extendedLen, extendedData);	// ==== [Debug] ====
    
	//Get MAC Key
	switch(keyScheme)
	{
		case 0x01:
			result = SRED_MSKEY_GetMacKey(index, macKey, &keyLen);
			break;

		case 0x02:
			// result = SRED_DUKPT_GetMacKey(ksn, macKey);
            result = SRED_AES_DUKPT_GetMacKey(ksn, macKey, &keyLen);
			break;

		default:
			result = FALSE;
			break;
	}

	if(result == FALSE)
	{
		rspCode = FALSE;
		goto EXIT;
	}
	
	//The hashed value is the hash of the salt value appended to the PAN
	// ucl_hmac_sha256(hash, UCL_SHA256_HASHSIZE, extendedData, extendedLen, macKey, 16);
	api_HMAC_SHA256_encipher(extendedData, extendedLen, hash, UCL_SHA256_HASHSIZE, macKey, keyLen);

	SRED_DBG_Put_String(21, (UINT8*)"==== HMAC SHA256 ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(UCL_SHA256_HASHSIZE, hash);	// ==== [Debug] ====
	
	memmove(&proData[proLen + 1], hash, UCL_SHA256_HASHSIZE);
	proLen += UCL_SHA256_HASHSIZE;
	proData[0] = proLen;

	SRED_DBG_Put_String(24, (UINT8*)"==== Protected data ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(proData[0], &proData[1]);	// ==== [Debug] ====

	//Write the protected data (L-V) to NVSRAM
	OS_SECM_PutData(ADDR_PED_PAN_DATA, proData[0] + 1, proData);

	os_ped_PanDownCnt = PED_PAN_ENCRYPT_TOUT;	//Setup min encrypt PAN interval = 30sec

	rspCode = TRUE;

EXIT:
	//Clear sensitive data
	memset(surrogate, 0x00, 19);
	memset(data, 0x00, 19);
	memset(dek, 0x00, PED_ACC_DEK_MSKEY_LEN + 1);
	memset(ctext, 0x00, SRED_BUFFER_SIZE);
	memset(salt, 0x00, PED_SALT_LEN);
	memset(extendedData, 0x00, 40);
	memset(macKey, 0x00, 16);
	memset(hash, 0x00, UCL_SHA256_HASHSIZE);
	memset(proData, 0x00, SRED_BUFFER_SIZE);

	return rspCode;
}

// ---------------------------------------------------------------------------
// FUNCTION: Get PAN data from NVSRAM
//			 SRED is enabled - 1. only first six and last four digits are 
//								  plaintext
//							   2. the whole PAN data is encrypted
//			 SRED is disabled - plaintext PAN data 
// INPUT   : mode   - the PAN data format what you want to get
//					  0 = plaintext PAN data
//					  1 = only first six and last four digits are plaintext
//					  2 = the whole PAN data is encrypted
// OUTPUT  : data   - PAN data
//			 length - length of PAN data
// RETURN  : TRUE
//			 FALSE
// ---------------------------------------------------------------------------
UINT8	SRED_Func_GetPAN(UINT8 mode, UINT8 *data, UINT8 *length)
{
	UINT8	rspCode = FALSE;
	UINT8	ctext[SRED_BUFFER_SIZE];
	UINT32	ctextLen = 0;
	UINT8	ptext[SRED_BUFFER_SIZE];
	UINT32	ptextLen = 0;
    UINT8   dek[PED_ACC_DEK_MSKEY_LEN + 1]; // L-V
	UINT8	FPEKey[PED_FPE_KEY_MSKEY_LEN];
	UINT8	tweak[10];
	UINT8	middleDigits[9];
	UINT8	middleLen = 0;
	UINT8	i;
	UINT8	surrogate[9];
	UINT8	ascData[20];
	UINT8	ascDataLen = 0;
	

	if((mode == 0) && (OS_SECM_VerifySredStatus() == FALSE))	//Plaintext PAN data
	{
		OS_SECM_GetData(ADDR_TAG5A_L, 1, &ctextLen);
		OS_SECM_GetData(ADDR_TAG5A_V, ctextLen, ctext);

		SRED_DBG_Put_String(20, (UINT8*)"==== Ciphertext ====");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(ctextLen, ctext);	// ==== [Debug] ====

		//Retrieve ACC_DEK
		if(SRED_GetAccDek(dek) == FALSE)
		{
			rspCode = FALSE;
			goto EXIT;
		}

		SRED_DBG_Put_String(29, (UINT8*)"==== Data encryption key ====");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(dek[0], &dek[1]);	// ==== [Debug] ====

		//Decrypt the whole encrypted PAN data
        if((PED_ReadKeyMode() == PED_KEY_MODE_MS) ||
        ((PED_ReadKeyMode() == PED_KEY_MODE_DUKPT) && (AccDek_workingKeyType == 1))) //TDES-192
            SRED_3DesEcb_Decrypt(ptext, &ptextLen, ctext, (UINT32)ctextLen, &dek[1]);
        else if((PED_ReadKeyMode() == PED_KEY_MODE_DUKPT) && (AccDek_workingKeyType >= 2))  //AES-128, AES-192, or AES-256
            SRED_AesEcb_Decrypt(ptext, &ptextLen, ctext, (UINT32)ctextLen, &dek[1]);
        else
        {
            rspCode = FALSE;
            goto EXIT;
        }

		SRED_DBG_Put_String(19, (UINT8*)"==== Plaintext ====");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(ptextLen, ptext);	// ==== [Debug] ====

		//-------------------- Get plaintext middle digits --------------------

		//The leading six digits and the trailing four digits are used as the tweak
		memmove(tweak, ptext, 6);
		memmove(&tweak[6], &ptext[ptextLen - 4], 4);

		SRED_DBG_Put_String(15, (UINT8*)"==== Tweak ====");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(10, tweak);	// ==== [Debug] ====
		
		//Encrypted middle digits
		memmove(surrogate, &ptext[6], (ptextLen - 10));
		middleLen = ptextLen - 10;

		SRED_DBG_Put_String(33, (UINT8*)"==== Encrypted middle digits ====");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(middleLen, surrogate);	// ==== [Debug] ====

		//Retrieve FPE Key
		if(SRED_GetFPEKey(FPEKey) == FALSE)
		{
			rspCode = FALSE;
			goto EXIT;
		}

		SRED_DBG_Put_String(17, (UINT8 *)"==== FPE key ====");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(PED_FPE_KEY_LEN, FPEKey);	// ==== [Debug] ====

		//Decrypt sensitive middle digits
		SRED_DBG_Put_String(53, (UINT8*)"-------------------- FF1.Decrypt --------------------");	// ==== [Debug] ====
		FPE_FF1_Decrypt(10, FPEKey, tweak, 10, surrogate, (UINT16)middleLen, middleDigits);

		//First six digits
		memmove(ascData, ptext, 6);

		//Convert numeral string to ASCII
		for(i = 0 ; i < middleLen ; i++)
		{
			//Middle digits
			ascData[i + 6] = middleDigits[i] + '0';
		}

		//Last four digits
		memmove(&ascData[6 + middleLen], &ptext[ptextLen - 4], 4);
		ascDataLen = ptextLen;

		SRED_DBG_Put_String(34, (UINT8*)"==== PAN data in ASCII format ====");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(ascDataLen, ascData);	// ==== [Debug] ====

		if((ascDataLen % 2) != 0)
		{
			*length = (ascDataLen + 1) / 2;
		}
		else
		{
			*length = ascDataLen / 2;
		}

		//Convert ASCII string to BCD data
		SRED_Compress(data, ascData, *length);

		SRED_DBG_Put_String(32, (UINT8*)"==== PAN data in BCD format ====");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(*length, data);	// ==== [Debug] ====

		rspCode = TRUE;
	}
	else if((mode == 1) && (OS_SECM_VerifySredStatus() == TRUE))	//Only first six and last four digits are plaintext
	{
		OS_SECM_GetData(ADDR_TAG5A_L, 1, &ctextLen);
		OS_SECM_GetData(ADDR_TAG5A_V, ctextLen, ctext);

		//Retrieve ACC_DEK
		if(SRED_GetAccDek(dek) == FALSE)
		{
			rspCode = FALSE;
			goto EXIT;
		}

		SRED_DBG_Put_String(29, (UINT8*)"==== Data encryption key ====");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(dek[0], &dek[1]);	// ==== [Debug] ====

        if((PED_ReadKeyMode() == PED_KEY_MODE_MS) ||
           ((PED_ReadKeyMode() == PED_KEY_MODE_DUKPT) && (AccDek_workingKeyType == 1))) //TDES-192
            SRED_3DesEcb_Decrypt(data, (UINT32*)length, ctext, (UINT32)ctextLen, &dek[1]);
        else if((PED_ReadKeyMode() == PED_KEY_MODE_DUKPT) && (AccDek_workingKeyType >= 2))  //AES-128, AES-192, or AES-256
            SRED_AesEcb_Decrypt(data, (UINT32*)length, ctext, (UINT32)ctextLen, &dek[1]);
        else
        {
            rspCode = FALSE;
            goto EXIT;
        }

		SRED_DBG_Put_String(18, (UINT8*)"==== PAN data ====");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(*length, data);	// ==== [Debug] ====

		rspCode = TRUE;
	}
	else if((mode == 2) && (OS_SECM_VerifySredStatus() == TRUE))	//The whole PAN data is encrypted
	{
		OS_SECM_GetData(ADDR_TAG5A_L, 1, length);
		OS_SECM_GetData(ADDR_TAG5A_V, *length, data);

		SRED_DBG_Put_String(18, (UINT8*)"==== PAN data ====");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(*length, data);	// ==== [Debug] ====
		
		rspCode = TRUE;
	}

EXIT:
	//Clear sensitive data
	memset(ctext, 0x00, SRED_BUFFER_SIZE);
	memset(ptext, 0x00, SRED_BUFFER_SIZE);
	memset(dek, 0x00, PED_ACC_DEK_MSKEY_LEN + 1);
	memset(middleDigits, 0x00, 9);
	memset(tweak, 0x00, 10);
	memset(surrogate, 0x00, 19);
	memset(ascData, 0x00, 20);
	
	return rspCode;
}

// ---------------------------------------------------------------------------
// FUNCTION: Write data element to NVSRAM
// INPUT   : iptTag    - T of TLV
//			 iptLen    - L of TLV
//			 iptData   - V of TLV
//			 keyScheme - key opearation mode
//						 01 = FIXED
//						 02 = MASTER/SESSION
//						 03 = DUKPT
//			 index	   - key index
// OUTPUT  : none.
// RETURN  : TRUE
//			 FALSE
// ---------------------------------------------------------------------------
UINT8	SRED_Func_StoreDataElement(UINT8 *iptTag, UINT8 iptLen, UINT8 *iptData, UINT8 keyScheme, UINT8 index)
{
	UINT8	rspCode = FALSE;
	UINT8	lenOfT = 0;
	UINT8	dek[PED_ACC_DEK_MSKEY_LEN + 1]; // L-V
	UINT8	ctext[SRED_BUFFER_SIZE];
	UINT32	ctextLen = 0;
	UINT8	dataOfL = 0;
	UINT8	ascPAN[20];
	UINT8	PANLen = 0;
	UINT8	i;
	UINT8	count = 0;
	

    // clear AES DUKPT working key and KSN before storing data element
    OS_SECM_ClearData(ADDR_PED_ACC_DEK_AES_DUKPT_WORKING_KEY, PED_ACC_DEK_AES_DUKPT_WORKING_KEY_LEN, 0x00);
    OS_SECM_ClearData(ADDR_PED_ACC_DEK_CURRENT_KSN, 12, 0x00);
    OS_SECM_ClearData(ADDR_PED_FPE_KEY_AES_DUKPT_WORKING_KEY, PED_FPE_KEY_AES_DUKPT_WORKING_KEY_LEN, 0x00);
    OS_SECM_ClearData(ADDR_PED_FPE_KEY_CURRENT_KSN, 12, 0x00);

	SRED_Get_TLVLengthOfT(iptTag, &lenOfT);

	if(lenOfT == 1)
	{
		if(*iptTag == 0x57)	//Track 2 Equivalent Data
		{
			//Retrieve ACC_DEK
			if(SRED_GetAccDek(dek) == FALSE)
			{
				rspCode = FALSE;
				goto EXIT;
			}

			SRED_DBG_Put_String(29, (UINT8*)"==== Data encryption key ====");	// ==== [Debug] ====
			SRED_DBG_Put_Hex(dek[0], &dek[1]);	// ==== [Debug] ====
			
			SRED_DBG_Put_String(19, (UINT8*)"==== Plaintext ====");	// ==== [Debug] ====
			SRED_DBG_Put_Hex(iptLen, iptData);	// ==== [Debug] ====

			//Encrypt Track 2 Equivalent Data
            if((PED_ReadKeyMode() == PED_KEY_MODE_MS) ||
               ((PED_ReadKeyMode() == PED_KEY_MODE_DUKPT) && (AccDek_workingKeyType == 1))) //TDES-192
                SRED_3DesEcb_Encrypt(ctext, &ctextLen, iptData, iptLen, &dek[1]);
            else if((PED_ReadKeyMode() == PED_KEY_MODE_DUKPT) && (AccDek_workingKeyType >= 2))  //AES-128, AES-192, or AES-256
                SRED_AesEcb_Encrypt(ctext, &ctextLen, iptData, iptLen, &dek[1]);
            else
            {
                rspCode = FALSE;
                goto EXIT;
            }

			SRED_DBG_Put_String(20, (UINT8*)"==== Ciphertext ====");	// ==== [Debug] ====
			SRED_DBG_Put_Hex(ctextLen, ctext);	// ==== [Debug] ====

			dataOfL = ctextLen & 0x000000FF;
			SRED_DBG_Put_String(19, (UINT8*)"==== Data of L ====");	// ==== [Debug] ====
			SRED_DBG_Put_UCHAR(dataOfL);	// ==== [Debug] ====

			OS_SECM_PutData(ADDR_TAG57_L, 1, &dataOfL);
			OS_SECM_PutData(ADDR_TAG57_V, ctextLen, ctext);

			rspCode = TRUE;
		}
		else if(*iptTag == 0x5A)	//Application Primary Account Number (PAN)
		{
			SRED_DBG_Put_String(27, (UINT8*)"==== PAN in BCD format ====");	// ==== [Debug] ====
			SRED_DBG_Put_Hex(iptLen, iptData);	// ==== [Debug] ====

			//Convert BCD data to ASCII string
			SRED_Split(ascPAN, iptData, iptLen);
			SRED_DBG_Put_String(29, (UINT8*)"==== PAN in ASCII format ====");	// ==== [Debug] ====
			SRED_DBG_Put_Hex((2 * iptLen), ascPAN);	// ==== [Debug] ====

			//Remove padding 'F'
			for(i = (2 * iptLen - 1) ; i >= 0 ; i--)
			{
				if(ascPAN[i] == 'F')
				{
					count++;
				}
				else
				{
					break;
				}
			}

			PANLen = 2 * iptLen - count;
            
			rspCode = SRED_Func_StorePAN(ascPAN, PANLen, keyScheme, index);
		}
	}
	else if(lenOfT == 2)
	{
		if((iptTag[0] == 0x9F) && (iptTag[1] == 0x1F))	//Track 1 Discretionary Data
		{
			//Retrieve ACC_DEK
			if(SRED_GetAccDek(dek) == FALSE)
			{
				rspCode = FALSE;
				goto EXIT;
			}

			SRED_DBG_Put_String(29, (UINT8*)"==== Data encryption key ====");	// ==== [Debug] ====
			SRED_DBG_Put_Hex(dek[0], &dek[1]);	// ==== [Debug] ====
			
			SRED_DBG_Put_String(19, (UINT8*)"==== Plaintext ====");	// ==== [Debug] ====
			SRED_DBG_Put_Hex(iptLen, iptData);	// ==== [Debug] ====

			//Encrypt Track 1 Discretionary Data
            if((PED_ReadKeyMode() == PED_KEY_MODE_MS) ||
               ((PED_ReadKeyMode() == PED_KEY_MODE_DUKPT) && (AccDek_workingKeyType == 1))) //TDES-192
                SRED_3DesEcb_Encrypt(ctext, &ctextLen, iptData, iptLen, &dek[1]);
            else if((PED_ReadKeyMode() == PED_KEY_MODE_DUKPT) && (AccDek_workingKeyType >= 2))  //AES-128, AES-192, or AES-256
                SRED_AesEcb_Encrypt(ctext, &ctextLen, iptData, iptLen, &dek[1]);
            else
            {
                rspCode = FALSE;
                goto EXIT;
            }

			SRED_DBG_Put_String(20, (UINT8*)"==== Ciphertext ====");	// ==== [Debug] ====
			SRED_DBG_Put_Hex(ctextLen, ctext);	// ==== [Debug] ====

			dataOfL = ctextLen & 0x000000FF;
			SRED_DBG_Put_String(19, (UINT8*)"==== Data of L ====");	// ==== [Debug] ====
			SRED_DBG_Put_UCHAR(dataOfL);	// ==== [Debug] ====

			OS_SECM_PutData(ADDR_TAG9F1F_L, 1, &dataOfL);
			OS_SECM_PutData(ADDR_TAG9F1F_V, ctextLen, ctext);

			rspCode = TRUE;
		}
		else if((iptTag[0] == 0x9F) && (iptTag[1] == 0x20))	//Track 2 Discretionary Data
		{
			//Retrieve ACC_DEK
			if(SRED_GetAccDek(dek) == FALSE)
			{
				rspCode = FALSE;
				goto EXIT;
			}

			SRED_DBG_Put_String(29, (UINT8*)"==== Data encryption key ====");	// ==== [Debug] ====
			SRED_DBG_Put_Hex(dek[0], &dek[1]);	// ==== [Debug] ====
			
			SRED_DBG_Put_String(19, (UINT8*)"==== Plaintext ====");	// ==== [Debug] ====
			SRED_DBG_Put_Hex(iptLen, iptData);	// ==== [Debug] ====

			//Encrypt Track 2 Discretionary Data
            if((PED_ReadKeyMode() == PED_KEY_MODE_MS) ||
               ((PED_ReadKeyMode() == PED_KEY_MODE_DUKPT) && (AccDek_workingKeyType == 1))) //TDES-192
                SRED_3DesEcb_Encrypt(ctext, &ctextLen, iptData, iptLen, &dek[1]);
            else if((PED_ReadKeyMode() == PED_KEY_MODE_DUKPT) && (AccDek_workingKeyType >= 2))  //AES-128, AES-192, or AES-256
                SRED_AesEcb_Encrypt(ctext, &ctextLen, iptData, iptLen, &dek[1]);
            else
            {
                rspCode = FALSE;
                goto EXIT;
            }

			SRED_DBG_Put_String(20, (UINT8*)"==== Ciphertext ====");	// ==== [Debug] ====
			SRED_DBG_Put_Hex(ctextLen, ctext);	// ==== [Debug] ====

			dataOfL = ctextLen & 0x000000FF;
			SRED_DBG_Put_String(19, (UINT8*)"==== Data of L ====");	// ==== [Debug] ====
			SRED_DBG_Put_UCHAR(dataOfL);	// ==== [Debug] ====

			OS_SECM_PutData(ADDR_TAG9F20_L, 1, &dataOfL);
			OS_SECM_PutData(ADDR_TAG9F20_V, ctextLen, ctext);

			rspCode = TRUE;
		}
	}

EXIT:
	//Clear sensitive data
	memset(dek, 0x00, PED_ACC_DEK_MSKEY_LEN + 1);
	memset(ctext, 0x00, SRED_BUFFER_SIZE);
	memset(ascPAN, 0x00, 20);

	return rspCode;
}

// ---------------------------------------------------------------------------
// FUNCTION: To get data element
// INPUT   : mode   - the PAN data format what you want to get
//					  0	   = plaintext PAN data
//					  1    = only first six and last four digits are plaintext
//					  2    = the whole PAN data is encrypted
//					  0xFF = not applicable for access Tag 57, Tag 9F1F, and Tag 9F20
//			 iptTag - T of TLV
// OUTPUT  : optData - V of TLV
//			 optLen - L of TLV
// RETURN  : TRUE
//			 FALSE
// ---------------------------------------------------------------------------
UINT8	SRED_Func_GetDataElement(UINT8 mode, UINT8 *iptTag, UINT8 *optData, UINT8 *optLen)
{
	UINT8	rspCode = FALSE;
	UINT8	lenOfT = 0;
    UINT8   dek[PED_ACC_DEK_MSKEY_LEN + 1]; // L-V
	UINT8	ctext[SRED_BUFFER_SIZE];
	UINT32	ctextLen = 0;
	

	SRED_Get_TLVLengthOfT(iptTag, &lenOfT);

	if(lenOfT == 1)
	{
		if(*iptTag == 0x57)	//Track 2 Equivalent Data
		{
			//Retrieve encrypted Tag 57 L-V
			OS_SECM_GetData(ADDR_TAG57_L, 1, &ctextLen);
			OS_SECM_GetData(ADDR_TAG57_V, ctextLen, ctext);

			SRED_DBG_Put_String(20, (UINT8*)"==== Ciphertext ====");	// ==== [Debug] ====
			SRED_DBG_Put_Hex(ctextLen, ctext);	// ==== [Debug] ====
			
			if(OS_SECM_VerifySredStatus() == TRUE)	//Output ciphertext
			{
				memmove(optData, ctext, ctextLen);
				*optLen = ctextLen;
			}
			else  //Output plaintext
			{
				//Retrieve ACC_DEK
				if(SRED_GetAccDek(dek) == FALSE)
				{
					rspCode = FALSE;
					goto EXIT;
				}

				SRED_DBG_Put_String(29, (UINT8*)"==== Data encryption key ====");	// ==== [Debug] ====
				SRED_DBG_Put_Hex(dek[0], &dek[1]);	// ==== [Debug] ====

                if((PED_ReadKeyMode() == PED_KEY_MODE_MS) ||
                   ((PED_ReadKeyMode() == PED_KEY_MODE_DUKPT) && (AccDek_workingKeyType == 1))) //TDES-192
                    SRED_3DesEcb_Decrypt(optData, optLen, ctext, (UINT32)ctextLen, &dek[1]);
                else if((PED_ReadKeyMode() == PED_KEY_MODE_DUKPT) && (AccDek_workingKeyType >= 2))  //AES-128, AES-192, or AES-256
                    SRED_AesEcb_Decrypt(optData, optLen, ctext, (UINT32)ctextLen, &dek[1]);
                else
                {
                    rspCode = FALSE;
                    goto EXIT;
                }

				SRED_DBG_Put_String(19, (UINT8*)"==== Plaintext ====");	// ==== [Debug] ====
				SRED_DBG_Put_Hex(*optLen, optData);	// ==== [Debug] ====
			}

			rspCode = TRUE;
		}
		else if(*iptTag == 0x5A)	//Application Primary Account Number (PAN)
		{
			rspCode = SRED_Func_GetPAN(mode, optData, optLen);
		}
	}
	else if(lenOfT == 2)
	{
		if((iptTag[0] == 0x9F) && (iptTag[1] == 0x1F))	//Track 1 Discretionary Data
		{
			//Retrieve encrypted Tag 9F1F L-V
			OS_SECM_GetData(ADDR_TAG9F1F_L, 1, &ctextLen);
			OS_SECM_GetData(ADDR_TAG9F1F_V, ctextLen, ctext);

			SRED_DBG_Put_String(20, (UINT8*)"==== Ciphertext ====");	// ==== [Debug] ====
			SRED_DBG_Put_Hex(ctextLen, ctext);	// ==== [Debug] ====
			
			if(OS_SECM_VerifySredStatus() == TRUE)	//Output ciphertext
			{
				memmove(optData, ctext, ctextLen);
				*optLen = ctextLen;
			}
			else  //Output plaintext
			{
				//Retrieve ACC_DEK
				if(SRED_GetAccDek(dek) == FALSE)
				{
					rspCode = FALSE;
					goto EXIT;
				}

				SRED_DBG_Put_String(29, (UINT8*)"==== Data encryption key ====");	// ==== [Debug] ====
				SRED_DBG_Put_Hex(dek[0], &dek[1]);	// ==== [Debug] ====

                if((PED_ReadKeyMode() == PED_KEY_MODE_MS) ||
                   ((PED_ReadKeyMode() == PED_KEY_MODE_DUKPT) && (AccDek_workingKeyType == 1))) //TDES-192
                    SRED_3DesEcb_Decrypt(optData, optLen, ctext, (UINT32)ctextLen, &dek[1]);
                else if((PED_ReadKeyMode() == PED_KEY_MODE_DUKPT) && (AccDek_workingKeyType >= 2))  //AES-128, AES-192, or AES-256
                    SRED_AesEcb_Decrypt(optData, optLen, ctext, (UINT32)ctextLen, &dek[1]);
                else
                {
                    rspCode = FALSE;
                    goto EXIT;
                }

				SRED_DBG_Put_String(19, (UINT8*)"==== Plaintext ====");	// ==== [Debug] ====
				SRED_DBG_Put_Hex(*optLen, optData);	// ==== [Debug] ====
			}

			rspCode = TRUE;
		}
		else if((iptTag[0] == 0x9F) && (iptTag[1] == 0x20))	//Track 2 Discretionary Data
		{
			//Retrieve encrypted Tag 9F20 L-V
			OS_SECM_GetData(ADDR_TAG9F20_L, 1, &ctextLen);
			OS_SECM_GetData(ADDR_TAG9F20_V, ctextLen, ctext);

			SRED_DBG_Put_String(20, (UINT8*)"==== Ciphertext ====");	// ==== [Debug] ====
			SRED_DBG_Put_Hex(ctextLen, ctext);	// ==== [Debug] ====
			
			if(OS_SECM_VerifySredStatus() == TRUE)	//Output ciphertext
			{
				memmove(optData, ctext, ctextLen);
				*optLen = ctextLen;
			}
			else  //Output plaintext
			{
				//Retrieve ACC_DEK
				if(SRED_GetAccDek(dek) == FALSE)
				{
					rspCode = FALSE;
					goto EXIT;
				}

				SRED_DBG_Put_String(29, (UINT8*)"==== Data encryption key ====");	// ==== [Debug] ====
				SRED_DBG_Put_Hex(dek[0], &dek[1]);	// ==== [Debug] ====

                if((PED_ReadKeyMode() == PED_KEY_MODE_MS) ||
                   ((PED_ReadKeyMode() == PED_KEY_MODE_DUKPT) && (AccDek_workingKeyType == 1))) //TDES-192
                    SRED_3DesEcb_Decrypt(optData, optLen, ctext, (UINT32)ctextLen, &dek[1]);
                else if((PED_ReadKeyMode() == PED_KEY_MODE_DUKPT) && (AccDek_workingKeyType >= 2))  //AES-128, AES-192, or AES-256
                    SRED_AesEcb_Decrypt(optData, optLen, ctext, (UINT32)ctextLen, &dek[1]);
                else
                {
                    rspCode = FALSE;
                    goto EXIT;
                }

				SRED_DBG_Put_String(19, (UINT8*)"==== Plaintext ====");	// ==== [Debug] ====
				SRED_DBG_Put_Hex(*optLen, optData);	// ==== [Debug] ====
			}

			rspCode = TRUE;
		}
	}

EXIT:
	//Clear sensitive data
	memset(dek, 0x00, PED_ACC_DEK_MSKEY_LEN);
	memset(ctext, 0x00, SRED_BUFFER_SIZE);

	return rspCode;
}

// ---------------------------------------------------------------------------
// FUNCTION: Encrypt PAN data
// INPUT   : dataIn  - the PAN to be encrypted
//			 inLen   - length of PAN
// OUTPUT  : dataOut - encrypted PAN
//			 outLen  - length of encrypted PAN
// RETURN  : TRUE
//			 FALSE
// NOTE	   : Limiting the rate at which the device will encrypt PANs.
//			 Min encrypt PIN interval = 30 sec
// ---------------------------------------------------------------------------
UINT8	SRED_Func_EncryptPAN(UINT8 *dataIn, UINT8 inLen, UINT8 *dataOut, UINT8 *outLen)
{
	UINT8	rspCode = FALSE;
	UINT8	surrogate[19];
	UINT8	data[19];
	UINT8	dek[PED_ACC_DEK_LEN];
	UINT8	result;


	//+------------------+-----------------+------------------+
	//| First six digits | Surrogate value | Last four digits |
	//+------------------+-----------------+------------------+
	//|<---                   Encrypted                   --->|

	//Generate a surrogate PAN values for sensitive middle digits
	result = SRED_GenerateSurrogatePAN(dataIn, inLen, surrogate);
	if(result == FALSE)
	{
		rspCode = FALSE;
		goto EXIT;
	}

	SRED_DBG_Put_String(22, (UINT8 *)"==== Original PAN ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(inLen, dataIn);	// ==== [Debug] ====

	//The data that should be encrypted consists of first six digits, middle surrogate value and last four digits
	memmove(data, dataIn, 6);	//First six digits
	memmove(&data[6], surrogate, (inLen - 10));	//Sensitive middle digits
	memmove(&data[inLen - 4], &dataIn[inLen - 4], 4);	//Last four digits

	SRED_DBG_Put_String(23, (UINT8 *)"==== Surrogate PAN ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(inLen, data);	// ==== [Debug] ====

	//Retrieve ACC_DEK
	if(SRED_GetAccDek(dek) == FALSE)
	{
		rspCode = FALSE;
		goto EXIT;
	}

	SRED_DBG_Put_String(29, (UINT8 *)"==== Data encryption key ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(PED_ACC_DEK_LEN, dek);	// ==== [Debug] ====

	//Encrypt the PAN data after the surrogate PAN value is generated by device
	SRED_3DesEcb_Encrypt(dataOut, outLen, data, inLen, dek);

	SRED_DBG_Put_String(20, (UINT8 *)"==== Ciphertext ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(outLen, dataOut);	// ==== [Debug] ====

	rspCode = TRUE;

EXIT:
	//Clear sensitive data
	memset(surrogate, 0x00, 19);
	memset(data, 0x00, 19);
	memset(dek, 0x00, PED_ACC_DEK_LEN);

	return rspCode;
}

// ---------------------------------------------------------------------------
// FUNCTION: Decrypt PAN data
// INPUT   : dataIn  - the encrypted PAN to be decrypted
//			 inLen   - length of encrypted PAN
// OUTPUT  : dataOut - plaintext PAN
//			 outLen  - length of plaintext PAN
// RETURN  : TRUE
//			 FALSE
// NOTE	   : Limiting the rate at which the device will encrypt PANs.
//			 Min encrypt PIN interval = 30 sec
// ---------------------------------------------------------------------------
UINT8	SRED_Func_DecryptPAN(UINT8 *dataIn, UINT8 inLen, UINT8 *dataOut, UINT8 *outLen)
{
	UINT8	rspCode = FALSE;
	UINT8	ptext[SRED_BUFFER_SIZE];
	UINT32	ptextLen = 0;
	UINT8	accDek[PED_ACC_DEK_MSKEY_LEN];
	UINT8	FPEKey[PED_FPE_KEY_MSKEY_LEN];
	UINT8	tweak[10];
	UINT8	middleDigits[9];
	UINT8	middleLen = 0;
	UINT8	i;
	UINT8	surrogate[9];
	UINT8	ascData[20];
	UINT8	ascDataLen = 0;


	//+------------------+-----------------+------------------+
	//| First six digits | Surrogate value | Last four digits |
	//+------------------+-----------------+------------------+
	//|<---                   Encrypted                   --->|

	SRED_DBG_Put_String(20, (UINT8 *)"==== Ciphertext ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(inLen, dataIn);	// ==== [Debug] ====

	//Retrieve ACC_DEK
	if(SRED_GetAccDek(accDek) == FALSE)
	{
		rspCode = FALSE;
		goto EXIT;
	}

	SRED_DBG_Put_String(29, (UINT8 *)"==== Data encryption key ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(PED_ACC_DEK_LEN, accDek);	// ==== [Debug] ====

	//Retrieve FPE Key
	if(SRED_GetFPEKey(FPEKey) == FALSE)
	{
		rspCode = FALSE;
		goto EXIT;
	}

	SRED_DBG_Put_String(17, (UINT8 *)"==== FPE key ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(PED_FPE_KEY_LEN, FPEKey);	// ==== [Debug] ====

	//Decrypt the whole encrypted PAN data
	SRED_3DesEcb_Decrypt(ptext, &ptextLen, dataIn, (UINT32)inLen, accDek);

	SRED_DBG_Put_String(19, (UINT8 *)"==== Plaintext ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(ptextLen, ptext);	// ==== [Debug] ====

	//-------------------- Get plaintext middle digits --------------------

	//The leading six digits and the trailing four digits are used as the tweak
	memmove(tweak, ptext, 6);
	memmove(&tweak[6], &ptext[ptextLen - 4], 4);

	SRED_DBG_Put_String(15, (UINT8 *)"==== Tweak ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(10, tweak);	// ==== [Debug] ====

	//Encrypted middle digits
	memmove(surrogate, &ptext[6], (ptextLen - 10));
	middleLen = ptextLen - 10;

	SRED_DBG_Put_String(33, (UINT8 *)"==== Encrypted middle digits ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(middleLen, surrogate);	// ==== [Debug] ====

	//Decrypt sensitive middle digits
	SRED_DBG_Put_String(53, (UINT8 *)"-------------------- FF1.Decrypt --------------------");	// ==== [Debug] ====
	FPE_FF1_Decrypt(10, FPEKey, tweak, 10, surrogate, (UINT16)middleLen, middleDigits);

	//First six digits
	memmove(ascData, ptext, 6);

	//Convert numeral string to ASCII
	for(i = 0 ; i < middleLen ; i++)
	{
		//Middle digits
		ascData[i + 6] = middleDigits[i] + '0';
	}

	//Last four digits
	memmove(&ascData[6 + middleLen], &ptext[ptextLen - 4], 4);
	ascDataLen = ptextLen;

	SRED_DBG_Put_String(34, (UINT8 *)"==== PAN data in ASCII format ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(ascDataLen, ascData);	// ==== [Debug] ====

	memmove(dataOut, ascData, ascDataLen);
	*outLen = ascDataLen;

	rspCode = TRUE;

EXIT:
	//Clear sensitive data
	memset(ptext, 0x00, SRED_BUFFER_SIZE);
	memset(accDek, 0x00, PED_ACC_DEK_MSKEY_LEN);
	memset(FPEKey, 0x00, PED_FPE_KEY_MSKEY_LEN);
	memset(middleDigits, 0x00, 9);
	memset(tweak, 0x00, 10);
	memset(surrogate, 0x00, 19);
	memset(ascData, 0x00, 20);

	return rspCode;
}

// ---------------------------------------------------------------------------
// FUNCTION: Get current KSN of ACC_DEK
// INPUT   : none.
// OUTPUT  : ksn - fixed 12 bytes (96 bits)
// RETURN  : none.
// ---------------------------------------------------------------------------
void    SRED_Func_AccDEK_GetCurrentKSN(UINT8 *ksn)
{
    OS_SECM_GetData(ADDR_PED_ACC_DEK_CURRENT_KSN, 12, ksn);
}

// ---------------------------------------------------------------------------
// FUNCTION: Get current KSN of FPE Key
// INPUT   : none.
// OUTPUT  : ksn - fixed 12 bytes (96 bits)
// RETURN  : none.
// ---------------------------------------------------------------------------
void    SRED_Func_FPE_GetCurrentKSN(UINT8 *ksn)
{
    OS_SECM_GetData(ADDR_PED_FPE_KEY_CURRENT_KSN, 12, ksn);
}

// ---------------------------------------------------------------------------
// FUNCTION: save data element to PAGE SRAM. (PAN)
// INPUT   : source  - data source (only ICC)
//         : address - begin address to write.
//           length  - length of the data element.
//           data    - the data element to be saved.
// OUTPUT  : none.
// RETURN  : emvOK
//           emvFailed
// ---------------------------------------------------------------------------
UINT8   SRED_Func_emv_PutDataElement(UINT8 index, UINT32 address, UINT32 length, UINT8 *data)
{
    UINT8   KeyScheme;
    UINT8   Tag;


    api_sram_write(address, length, data);

    os_ped_PanLife = PED_PAN_LIFE_TOUT;

    KeyScheme = PED_ReadKeyMode();
    Tag = 0x5A;	// PAN
    if(SRED_Func_StoreDataElement(&Tag, length, data, KeyScheme, index) == TRUE)
        return apiOK;
    else
        return apiFailed;
}

// ---------------------------------------------------------------------------
// FUNCTION: get data element from PAGE SRAM. (PAN)
// INPUT   : source  - data source (only ICC).
//         : address - begin address to write.
//           length  - length of the data element.
// OUTPUT  : data    - the data element read.
// RETURN  : emvOK
//           emvFailed
// ---------------------------------------------------------------------------
UINT8	SRED_Func_emv_GetDataElement(UINT32 address, UINT32 length, UINT8 *data)
{
    UINT32  i;
    UINT32  len;
    UINT32  len2;
    UINT8   Tag;
    UINT8   buf[10];
    

    if(!SRED_VerifySredStatus())
        api_sram_read(address, length, data);
    else
    {
        Tag = 0x5A;	// PAN		
	    SRED_Func_GetDataElement(1, &Tag, data, &len);

        memset(buf, 0xff, sizeof(buf));
        len2 = len / 2;
        for(i = 0 ; i < len2 ; i++)
            buf[i] = ((data[i * 2] & 0x0F) << 4) | (data[i * 2 + 1] & 0x0F);

        if(len & 1)
        {
            buf[i] = ((data[i * 2] & 0x0F) << 4) | 0x0F;
            len2++;
        }

        memmove(data, buf, sizeof(buf));
    }
}
