//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : i.MX6UL                                                    **
//**  PRODUCT  : AS350-X6                                                   **
//**                                                                        **
//**  FILE     : SRED.C                                                     **
//**  MODULE   :		                                                    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2022/05/24                                                 **
//**  EDITOR   : Tammy														**
//**                                                                        **
//**  Copyright(C) 2022 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
/** Global includes */
#include <config.h>
// #include <errors.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "POSAPI.h"
#include "UTILS.h"

/** Socket includes */	
#include <unistd.h>	
#include <sys/socket.h>	
#include <linux/if_alg.h>	
#include <linux/socket.h>
/** Other includes */
#include "ucl_config.h"
#include "ucl_defs.h"
#include "ucl_retdefs.h"
#include "ucl_types.h"
#include "ucl_sys.h"
#include "ucl_stack.h"

/** Local includes */
#include "ucl_3des_ecb.h"
#include "ucl_3des_cbc.h"
#include "ucl_aes.h"
#include "ucl_rng.h"
#include "ucl_sha512.h"
#include "ucl_hmac_sha256.h"

#include "SRED.h"
#include "OS_SECM.h"
#include "OS_MSG.h"
#include "OS_LIB.h"
#include "OS_PED.h"
#include "DEV_PED.h"
#include "ANS_TR31_2010.h"
#include "ANS_TR31_2018.h"
#include "FPE.h"
#include "SRED_Func.h"
#include "SRED_DBG_Function.h"
#include "PEDKconfig.h"


extern	void	_CLS( void );				// clear screen and reset cursor
extern	void	_PRINTF( const char *fmt, ... );	// output message onto display as "printf()"

extern	UINT8	rtc_dhn;

extern  UCHAR	AES_DUKPT_RequestDataEncryptionKey(UCHAR keyType, UCHAR *workingKey);
extern  UCHAR	AES_DUKPT_RequestMacKey( UCHAR keyType, UCHAR *workingKey );
extern  void    AES_DUKPT_GetKSN(UCHAR *ksn);

UINT32	os_ped_PanDownCnt = 0;
UINT32	os_ped_PanUpCnt = 0;
UINT32  os_ped_PanLife = 0;

UINT8   AccDek_workingKeyType = 0;
UINT8   MAC_workingKeyType = 0;


// ---------------------------------------------------------------------------
// FUNCTION: 3DES ECB mode encryption
// INPUT   : dataIn	 - pointer to the input data
//			 inLen	 - length of the input data
//			 key	 - pointer to the 3DES key
// OUTPUT  : dataOut - pointer to the ciphertext
//           outLen  - ciphertext length
// RETURN  : none.
// NOTE	   : 1.The byte length of plaintext must be a multiple of 8
//           2.Input and Output Data have the same length
//			 3.The key length is 16 or 24 bytes
//			 4.Padding method reference to PKCS #5
// ---------------------------------------------------------------------------
void	SRED_3DesEcb_Encrypt(UINT8 *dataOut, UINT32 *outLen, UINT8 *dataIn, UINT32 inLen, UINT8 *key)
{
	UINT8	ptext[SRED_BUFFER_SIZE];
	UINT32	ptextLen = 0;
	UINT8	ctext[SRED_BUFFER_SIZE];
	UINT8	padNum = 0;
	

	memcpy(ptext, dataIn, inLen);
	ptextLen += inLen;

	//Add N padded bytes of value N to make the input length up to the next exact multiple of 8.
	//Even though the input length is already an exact multiple of 8, it still need to pad 8 bytes of value 0x08.
	padNum = UCL_3DES_BLOCKSIZE - (inLen % UCL_3DES_BLOCKSIZE);
	memset(&ptext[ptextLen], padNum, padNum);
	ptextLen += (UINT32)padNum;

	// ucl_3des_ecb(ctext, ptext, key, ptextLen, UCL_CIPHER_ENCRYPT);
	api_3des_encipher2(ptext, ctext, key, ptextLen);

	memcpy(dataOut, ctext, ptextLen);
	*outLen = ptextLen;

	//Clear sensitive data
	memset(ptext, 0x00, SRED_BUFFER_SIZE);
	memset(ctext, 0x00, SRED_BUFFER_SIZE);
}

// ---------------------------------------------------------------------------
// FUNCTION: 3DES ECB mode decryption
// INPUT   : dataIn	 - pointer to the ciphertext
//			 inLen	 - ciphertext length
//			 key	 - pointer to the 3DES key
// OUTPUT  : dataOut - pointer to the plaintext
//			 outLen  - plaintext length
// RETURN  : none.
// NOTE	   : 1.The byte length of plaintext must be a multiple of 8
//           2.Input and Output Data have the same length
//			 3.The key length is 16 or 24 bytes
//			 4.Padding method reference to PKCS #5
// ---------------------------------------------------------------------------
void	SRED_3DesEcb_Decrypt(UINT8 *dataOut, UINT32	*outLen, UINT8 *dataIn, UINT32 inLen, UINT8 *key)
{
	UINT8	ptext[SRED_BUFFER_SIZE];


	// ucl_3des_ecb(ptext, dataIn, key, inLen, UCL_CIPHER_DECRYPT);
	api_3des_decipher2(ptext, dataIn, key, inLen);

	//After decrypting, read the last character decrypted and strip off that many bytes
	memcpy(dataOut, ptext, (inLen - ptext[inLen - 1]));
	*outLen = inLen - ptext[inLen - 1];

	//Clear sensitive data
	memset(ptext, 0x00, SRED_BUFFER_SIZE);
}

// ---------------------------------------------------------------------------
// FUNCTION: 3DES CBC mode encryption
// INPUT   : dataIn	 - pointer to the input data
//			 inLen	 - length of the input data
//			 key	 - pointer to the 3DES key
//			 iv		 - pointer to the initialization vector
// OUTPUT  : dataOut - pointer to the ciphertext
//           outLen  - length of padded ciphertext
// RETURN  : none.
// NOTE	   : 1.The byte length of plaintext must be a multiple of 8
//           2.Input and Output Data have the same length
//			 3.The key length is 24 bytes
//			 4.Padding method reference to PKCS #5
// ---------------------------------------------------------------------------
void	SRED_3DesCbc_Encrypt(UINT8 *dataOut, UINT32 *outLen, UINT8 *dataIn, UINT32 inLen, UINT8 *key, UINT8 *iv)
{
	UINT8	ptext[SRED_BUFFER_SIZE];
	UINT32	ptextLen = 0;
	UINT8	ctext[SRED_BUFFER_SIZE];
	UINT8	padNum = 0;

	
	memcpy(ptext, dataIn, inLen);
	ptextLen += inLen;

	//Add N padded bytes of value N to make the input length up to the next exact multiple of 8.
	//Even though the input length is already an exact multiple of 8, it still need to pad 8 bytes of value 0x08.
	padNum = UCL_3DES_BLOCKSIZE - (inLen % UCL_3DES_BLOCKSIZE);
	memset(&ptext[ptextLen], padNum, padNum);
	ptextLen += (UINT32)padNum;
	
	PED_CBC_TripleDES(key, iv, ptextLen, ptext, ctext);
	// ucl_3des_cbc(ctext, ptext, key, iv, ptextLen, UCL_CIPHER_ENCRYPT);

	memcpy(dataOut, ctext, ptextLen);
	*outLen = ptextLen;

	//Clear sensitive data
	memset(ptext, 0x00, SRED_BUFFER_SIZE);
	memset(ctext, 0x00, SRED_BUFFER_SIZE);
}

// ---------------------------------------------------------------------------
// FUNCTION: 3DES CBC mode decryption
// INPUT   : dataIn	 - pointer to the ciphertext
//			 inLen	 - ciphertext length
//			 key	 - pointer to the 3DES key
//			 iv		 - pointer to the initialization vector
// OUTPUT  : dataOut - pointer to the plaintext
//			 outLen  - length of unpadded plaintext
// RETURN  : none.
// NOTE	   : 1.The byte length of plaintext must be a multiple of 8
//           2.Input and Output Data have the same length
//			 3.The key length is 24 bytes
//			 4.Padding method reference to PKCS #5
// ---------------------------------------------------------------------------
void	SRED_3DesCbc_Decrypt(UINT8 *dataOut, UINT32	*outLen, UINT8 *dataIn, UINT32 inLen, UINT8 *key, UINT8 *iv)
{
	UINT8	ptext[SRED_BUFFER_SIZE];
	

	PED_CBC_TripleDES2(key, iv, inLen, dataIn, ptext);
	// ucl_3des_cbc(ptext, dataIn, key, iv, inLen, UCL_CIPHER_DECRYPT);

	//After decrypting, read the last character decrypted and strip off that many bytes
	memcpy(dataOut, ptext, (inLen - ptext[inLen - 1]));
	*outLen = inLen - ptext[inLen - 1];

	//Clear sensitive data
	memset(ptext, 0x00, SRED_BUFFER_SIZE);
}

// ---------------------------------------------------------------------------
// FUNCTION: AES EBC mode encryption
// INPUT   : dataIn	 - pointer to the input data
//			 inLen	 - length of the input data
//			 key	 - pointer to the AES key
// OUTPUT  : dataOut - pointer to the ciphertext
//           outLen  - length of padded ciphertext
// RETURN  : TRUE
//			 FALSE
// NOTE	   : 1.The byte length of plaintext must be a multiple of 16
//           2.Input and Output Data have the same length
//			 3.The key length is 16, 24, or 32 bytes
//			 4.Padding method reference to PKCS #7
// ---------------------------------------------------------------------------
void    SRED_AesEcb_Encrypt(UINT8 *dataOut, UINT32 *outLen, UINT8 *dataIn, UINT32 inLen, UINT8 *key)
{
    UINT8	result = FALSE;
    UINT8	ptext[SRED_BUFFER_SIZE];
	UINT32	ptextLen = 0;
	UINT8	ctext[SRED_BUFFER_SIZE];
	UINT8	padNum = 0;


    memcpy(ptext, dataIn, inLen);
	ptextLen += inLen;

    //Add N padded bytes of value N to make the input length up to the next exact multiple of 16.
	//Even though the input length is already an exact multiple of 16, it still need to pad 16 bytes of value 0x10.
    padNum = UCL_AES_BLOCKSIZE - (inLen % UCL_AES_BLOCKSIZE);
    memset(&ptext[ptextLen], padNum, padNum);
	ptextLen += (UINT32)padNum;
        
    if(AccDek_workingKeyType == 2)  //AES128
    {
        api_aes_encipher2(ptext, ctext, key, 16, ptextLen);
        result = TRUE;
    }  
    else if(AccDek_workingKeyType == 3)  //AES192
    {
        api_aes_encipher2(ptext, ctext, key, 24, ptextLen);
        result = TRUE;
    }
    else if(AccDek_workingKeyType == 4)  //AES256
    {
        api_aes_encipher2(ptext, ctext, key, 32, ptextLen);
        result = TRUE;
    }
    
    if(result == TRUE)
    {
        memcpy(dataOut, ctext, ptextLen);
	    *outLen = ptextLen;
    }

    //Clear sensitive data
	memset(ptext, 0x00, SRED_BUFFER_SIZE);
	memset(ctext, 0x00, SRED_BUFFER_SIZE);

    return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: AES EBC mode decryption
// INPUT   : dataIn	 - pointer to the input data
//			 inLen	 - length of the input data
//			 key	 - pointer to the AES key
// OUTPUT  : dataOut - pointer to the plaintext
//           outLen  - length of padded plaintext
// RETURN  : TRUE
//			 FALSE
// NOTE	   : 1.The byte length of plaintext must be a multiple of 16
//           2.Input and Output Data have the same length
//			 3.The key length is 16, 24, or 32 bytes
//			 4.Padding method reference to PKCS #7
// ---------------------------------------------------------------------------
void    SRED_AesEcb_Decrypt(UINT8 *dataOut, UINT32 *outLen, UINT8 *dataIn, UINT32 inLen, UINT8 *key)
{
    UINT8	result = FALSE;
    UINT8	ptext[SRED_BUFFER_SIZE];

        
    if(AccDek_workingKeyType == 2)  //AES128
    {
        api_aes_decipher2(ptext, dataIn, key, 16, inLen);
        result = TRUE;
    }  
    else if(AccDek_workingKeyType == 3)  //AES192
    {
        api_aes_decipher2(ptext, dataIn, key, 24, inLen);
        result = TRUE;
    }
    else if(AccDek_workingKeyType == 4)  //AES256
    {
        api_aes_decipher2(ptext, dataIn, key, 32, inLen);
        result = TRUE;
    }
    
    //After decrypting, read the last character decrypted and strip off that many bytes
    memcpy(dataOut, ptext, (inLen - ptext[inLen - 1]));
	*outLen = inLen - ptext[inLen - 1];

    //Clear sensitive data
	memset(ptext, 0x00, SRED_BUFFER_SIZE);

    return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: Retrieve Account Data Encryption Key
// INPUT   : none.
// OUTPUT  : accDek - expanded ACC_DEK (24 bytes)
// RETURN  : TRUE
//			 FALSE
// ---------------------------------------------------------------------------
/*
UINT8	SRED_GetAccDek(UINT8 *accDek)
{
	PED_KEY_BUNDLE	KeyBundle;
	UINT8	keydata[KEY_DATA_LEN + 1]; // L-V
	UINT8	eskey[PED_ACC_DEK_MSKEY_LEN + 1];	// L-V
	UINT8	skey[PED_ACC_DEK_MSKEY_LEN];
	UINT8	mkey[PED_ACC_DEK_MSKEY_LEN + 1];	// L-V
	UINT8	mac8[8];
	UINT8	MkeyIndex;
	UINT8	result = FALSE;

	UINT8	temp[KEY_DATA_LEN + 1];
	UINT8	buf[KEY_BUNDLE_LEN];
	
	
	// get ACC_DEK MKEY bundle
	PED_AccDEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
	OS_SECM_GetData(ADDR_PED_ACC_DEK_MKEY_01 + (MkeyIndex * PED_ACC_DEK_MSKEY_SLOT_LEN), PED_ACC_DEK_MSKEY_SLOT_LEN, buf);

	// verify ACC_DEK MKEY bundle
	if(TR31_VerifyKeyBundle(KEY_BUNDLE_LEN, buf, temp, mac8, (UINT8 *)0))
	{
		// retrieve ACC_DEK MKEY
		if(!TR31_DecryptKeyBundle(mac8, temp, mkey, (UINT8 *)0))	// mkey=ACC_DEK MKEY (as the KBPK for ACC_DEK SKEY)
		{
			result = FALSE;
			goto EXIT;
		}
	}

	// get ACC_DEK ESKEY bundle
	OS_SECM_GetData(ADDR_PED_ACC_DEK_SKEY_01, PED_ACC_DEK_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle);

	// verify ACC_DEK ESKEY bundle
	if(TR31_VerifyKeyBundle(PED_ACC_DEK_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle, keydata, mac8, mkey))
	{
		// check ACC_DEK ESKEY key usage
		if((KeyBundle.Usage[0] == 'D') && (KeyBundle.Usage[1] == '0')) // for DATA encryption?
		{
			// retrieve ACC_DEK ESKEY key
			if(TR31_DecryptKeyBundle(mac8, keydata, eskey, mkey))
			{
				memmove( skey, &eskey[1], eskey[0] );
				
				// get ACC_DEK MKEY bundle
				PED_AccDEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
				OS_SECM_GetData(ADDR_PED_ACC_DEK_MKEY_01 + (MkeyIndex * PED_ACC_DEK_MSKEY_SLOT_LEN), PED_ACC_DEK_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle);

				// verify ACC_DEK MKEY bundle
				if(TR31_VerifyKeyBundle(KEY_BUNDLE_LEN, (UINT8 *)&KeyBundle, keydata, mac8, (UINT8 *)0 ))
				{
					// check ACC_DEK MKEY key usage
					if((KeyBundle.Usage[0] == 'K') && (KeyBundle.Usage[1] == '0')) // for KEY encryption?
					{
						// retrieve ACC_DEK MKEY
//						if(TR31_DecryptKeyBundle(mac8, keydata, mkey))	// mkey=ACC_DEK MKEY
//						{
							// retrieve ACC_DEK SKEY from ACC_DEK MKEY
//							PED_TripleDES2(&mkey[1], eskey[0], &eskey[1], skey);
							
							memmove(accDek, skey, 16);
							memmove(&accDek[16], skey, 8);
							
							result = TRUE;
//						}
					}
				}
			}
		}
	}

EXIT:
	//Clear sensitive data
	memset(keydata, 0x00, sizeof(keydata));
	memset(eskey, 0x00, sizeof(eskey));
	memset(skey, 0x00, sizeof(skey));
	memset(mkey, 0x00, sizeof(mkey));
	
	memset(temp, 0x00, sizeof(temp));
	memset(buf, 0x00, sizeof(buf));

	return result;
}
*/

// ---------------------------------------------------------------------------
// FUNCTION: Retrieve Account Data Encryption Key
// INPUT   : none.
// OUTPUT  : accDek - ACC_DEK (16/24/32 bytes)
// RETURN  : TRUE
//			 FALSE
// ---------------------------------------------------------------------------
UINT8	SRED_GetAccDek(UINT8 *accDek)
{
    X9143_TDES_PED_KEY_BUNDLE   tdesKeyBundle;
    X9143_AES_PED_KEY_BUNDLE    aesKeyBundle; 
    UINT8	tdesKeyData[X9143_TDES_KEY_DATA_LEN + 1]; // L-V
    UINT8	aesKeyData[X9143_AES_KEY_DATA_LEN + 1]; // L-V
    UINT8	pkey[PED_ACC_DEK_MSKEY_LEN + 1];    // L-V
	UINT8	skey[PED_ACC_DEK_MSKEY_LEN];
	UINT8	mkey[PED_ACC_DEK_MSKEY_LEN + 1];	// L-V
	UINT8	mac8[8];
    UINT8   mac16[16];
	UINT8	MkeyIndex;
	UINT8	result = TRUE;
    UINT8	keyScheme;
    UINT8   keyType;
    UINT8   workingKey[32];
    UINT8   ksn[12];
	
	
    //Read key opearation mode
    keyScheme = PED_ReadKeyMode();

    if(keyScheme == PED_KEY_MODE_MS)
    {
        PED_AccDEK_GetKeyType(&keyType);
        
        PED_SetKPKStatus(keyType, 1);

        if(keyType == TDES_192)
        {
            // get ACC_DEK MKEY bundle
            PED_AccDEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
            OS_SECM_GetData(ADDR_PED_ACC_DEK_TDES_MKEY_01 + (MkeyIndex * PED_ACC_DEK_TDES_MSKEY_SLOT_LEN), PED_ACC_DEK_TDES_MSKEY_SLOT_LEN, (UINT8 *)&tdesKeyBundle);

            // verify ACC_DEK MKEY bundle
            if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, (UINT8 *)&tdesKeyBundle, tdesKeyData, mac8, (UINT8 *)0 ))
            {
                // check ACC_DEK MKEY key usage
                if((tdesKeyBundle.Usage[0] == 'K') && (tdesKeyBundle.Usage[1] == '0')) // for KEY encryption?
                {
                    // retrieve ACC_DEK MKEY
                    if(!X9143_DecryptKeyBundle_TDES(mac8, tdesKeyData, mkey, (UINT8 *)0 ))	// mkey=ACC_DEK MKEY (as the KBPK for ACC_DEK SKEY)
                    {
                        // reset flag
                        PED_SetKPKStatus(keyType, 0);

                        result = FALSE;
                        goto EXIT;
                    }
                }
            }

            if(result == TRUE)
            {
                result = FALSE;

                // get ACC_DEK SKEY bundle
                OS_SECM_GetData(ADDR_PED_ACC_DEK_TDES_SKEY_01, PED_ACC_DEK_TDES_MSKEY_SLOT_LEN, (UINT8 *)&tdesKeyBundle);

                // verify ACC_DEK SKEY bundle
                if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, (UINT8 *)&tdesKeyBundle, tdesKeyData, mac8, mkey))
                {
                    // check ACC_DEK SKEY key usage
                    if((tdesKeyBundle.Usage[0] == 'D') && (tdesKeyBundle.Usage[1] == '0')) // for DATA encryption?
                    {
                        // retrieve ACC_DEK SKEY
                        if(X9143_DecryptKeyBundle_TDES(mac8, tdesKeyData, pkey, mkey)) // pkey=ACC_DEK SKEY
                        {
                            // ==== [Debug] ====
                            printf("pkey = ");
                            for(int i = 0 ; i < pkey[0] + 1 ; i++)
                                printf("%02x", pkey[i]);
                            printf("\n");
                            // ==== [Debug] ====

                            memmove(skey, &pkey[1], pkey[0]);

                            memmove(accDek, pkey, pkey[0] + 1);

                            result = TRUE;
                        }
                    }
                }
            }
        }
        else if((keyType == AES_128) || (keyType == AES_192) || (keyType == AES_256))
        {
            // get ACC_DEK MKEY bundle
            PED_AccDEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
            OS_SECM_GetData(ADDR_PED_ACC_DEK_AES_MKEY_01 + (MkeyIndex * PED_ACC_DEK_AES_MSKEY_SLOT_LEN), PED_ACC_DEK_AES_MSKEY_SLOT_LEN, (UINT8 *)&aesKeyBundle);

            // verify ACC_DEK MKEY bundle
            if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, (UINT8 *)&aesKeyBundle, aesKeyData, mac16, (UINT8 *)0 ))
            {
                // check ACC_DEK MKEY key usage
                if((aesKeyBundle.Usage[0] == 'K') && (aesKeyBundle.Usage[1] == '0')) // for KEY encryption?
                {
                    // retrieve ACC_DEK MKEY
                    if(!X9143_DecryptKeyBundle_AES(mac16, aesKeyData, mkey, (UINT8 *)0 ))	// mkey=ACC_DEK MKEY (as the KBPK for ACC_DEK SKEY)
                    {
                        // reset flag
                        PED_SetKPKStatus(keyType, 0);

                        result = FALSE;
                        goto EXIT;
                    }
                }
            }

            if(result == TRUE)
            {
                result = FALSE;

                // get ACC_DEK SKEY bundle
                OS_SECM_GetData(ADDR_PED_ACC_DEK_AES_SKEY_01, PED_ACC_DEK_AES_MSKEY_SLOT_LEN, (UINT8 *)&aesKeyBundle);

                // verify ACC_DEK SKEY bundle
                if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, (UINT8 *)&aesKeyBundle, aesKeyData, mac16, mkey))
                {
                    // check ACC_DEK SKEY key usage
                    if((aesKeyBundle.Usage[0] == 'D') && (aesKeyBundle.Usage[1] == '0')) // for DATA encryption?
                    {
                        // retrieve ACC_DEK SKEY
                        if(X9143_DecryptKeyBundle_AES(mac16, aesKeyData, pkey, mkey)) // pkey=ACC_DEK SKEY
                        {
                            // ==== [Debug] ====
                            printf("pkey = ");
                            for(int i = 0 ; i < pkey[0] + 1 ; i++)
                                printf("%02x", pkey[i]);
                            printf("\n");
                            // ==== [Debug] ====

                            memmove(skey, &pkey[1], pkey[0]);

                            memmove(accDek, pkey, pkey[0] + 1);

                            result = TRUE;
                        }
                    }
                }
            }
        }

        // reset flag
        PED_SetKPKStatus(keyType, 0);
    }
    else if(keyScheme == PED_KEY_MODE_DUKPT)
    {
        OS_SECM_GetData(ADDR_PED_ACC_DEK_AES_DUKPT_WORKING_KEY, PED_ACC_DEK_AES_DUKPT_WORKING_KEY_LEN, accDek);
        
        if(accDek[0] != 0)  //Working key already exists
        {
            result = TRUE;
            goto EXIT;
        }

        PED_AES_DUKPT_GetKeyType(&keyType);
        SRED_AES_DUKPT_setWorkingKeyType(0);

        if(keyType == AES_128)
        {
            if(!((AccDek_workingKeyType == 1) || (AccDek_workingKeyType == 2)))
            {
                //Working key type is not TDES-192, or AES-128
                return FALSE;
            }
        }
        else if(keyType == AES_192)
        {
            if(!((AccDek_workingKeyType == 1) || (AccDek_workingKeyType == 2) || (AccDek_workingKeyType == 3)))
            {
                //Working key type is not TDES-192, AES-128, or AES-192
                return FALSE;
            }
        }
        else if(keyType == AES_256)
        {
            if(!((AccDek_workingKeyType == 1) || (AccDek_workingKeyType == 2) || (AccDek_workingKeyType == 3) || (AccDek_workingKeyType == 4)))
            {
                //Working key type is not TDES-192, AES-128, AES-192, or AES-256
                return FALSE;
            }
        }

        if(AES_DUKPT_RequestDataEncryptionKey(AccDek_workingKeyType, workingKey))
        {
            if(AccDek_workingKeyType == 1) //TDES-192
            {
                accDek[0] = 24;
                memmove(&accDek[1], workingKey, 24);
            }
            else if(AccDek_workingKeyType == 2)    //AES-128
            {
                accDek[0] = 16;
                memmove(&accDek[1], workingKey, 16);
            }
            else if(AccDek_workingKeyType == 3)    //AES-192
            {
                accDek[0] = 24;
                memmove(&accDek[1], workingKey, 24);
            }
            else if(AccDek_workingKeyType == 4)    //AES-256
            {
                accDek[0] = 32;
                memmove(&accDek[1], workingKey, 32);
            }

            OS_SECM_PutData(ADDR_PED_ACC_DEK_AES_DUKPT_WORKING_KEY, PED_ACC_DEK_AES_DUKPT_WORKING_KEY_LEN, accDek);

            AES_DUKPT_GetKSN(ksn);
            OS_SECM_PutData(ADDR_PED_ACC_DEK_CURRENT_KSN, 12, ksn);
            SRED_DBG_Put_String(33, (UINT8*)"==== Current KSN for ACC_DEK ====");	// ==== [Debug] ====
            SRED_DBG_Put_Hex(12, ksn);	// ==== [Debug] ====
        }
    }
    else
        return FALSE;

EXIT:
	//Clear sensitive data
    memset(tdesKeyData, 0x00, sizeof(tdesKeyData));
    memset(aesKeyData, 0x00, sizeof(aesKeyData));
    memset(pkey, 0x00, sizeof(pkey));
	memset(skey, 0x00, sizeof(skey));
	memset(mkey, 0x00, sizeof(mkey));
    memset(workingKey, 0x00, sizeof(workingKey));

	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: Retrieve Format-Preserving Encryption Key
// INPUT   : none.
// OUTPUT  : FPEKey - FPE Key
// RETURN  : TRUE
//			 FALSE
// ---------------------------------------------------------------------------
/*
UINT8	SRED_GetFPEKey(UINT8 *FPEKey)
{
	PED_KEY_BUNDLE2	KeyBundle;
	UINT8	keydata[KEY_DATA_LEN2 + 1]; // L-V
	UINT8	eskey[PED_FPE_KEY_LEN + 1];	// L-V
	UINT8	skey[PED_FPE_KEY_LEN];
	UINT8	mkey[PED_FPE_KEY_LEN + 1];	// L-V
	UINT8	mac16[16];
	UINT8	MkeyIndex;
	UINT8	result = FALSE;


	// get FPE Key bundle
	OS_SECM_GetData(ADDR_PED_FPE_KEY, PED_FPE_KEY_SLOT_LEN, (UINT8 *)&KeyBundle);

	// verify FPE Key bundle
	if(TR31_VerifyKeyBundle_AES(PED_FPE_KEY_SLOT_LEN, (UINT8 *)&KeyBundle, keydata, mac16))
	{
		// check FPE Key key usage
		if((KeyBundle.Usage[0] == 'D') && (KeyBundle.Usage[1] == '0')) // for DATA encryption?
		{
			// retrieve FPE Key
			if(TR31_DecryptKeyBundle_AES(mac16, keydata, eskey))
			{
				memmove(skey, &eskey[1], eskey[0]);
				memmove(FPEKey, skey, PED_FPE_KEY_LEN);

				result = TRUE;
			}

			// // retrieve FPE ESKEY key (SKEY encrypted by MKEY)
			// if(TR31_DecryptKeyBundle_AES(mac16, keydata, eskey))
			// {
			// 	// get FPE MKEY bundle
			// 	PED_FPE_ReadMKeyIndex((UINT8 *)&MkeyIndex);
			// 	OS_SECM_GetData(ADDR_PED_FPE_KEY_MKEY_01 + (MkeyIndex * PED_FPE_KEY_MSKEY_SLOT_LEN), PED_FPE_KEY_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle);

			// 	// verify FPE MKEY bundle
			// 	if(TR31_VerifyKeyBundle_AES(KEY_BUNDLE_LEN2, (UINT8 *)&KeyBundle, keydata, mac16))
			// 	{
			// 		// check FPE MKEY key usage
			// 		if((KeyBundle.Usage[0] == 'K') && (KeyBundle.Usage[1] == '0')) // for KEY encryption?
			// 		{
			// 			// retrieve FPE MKEY
			// 			if(TR31_DecryptKeyBundle_AES(mac16, keydata, mkey))	// mkey=FPE MKEY
			// 			{
			// 				// retrieve FPE SKEY from FPE MKEY
			// 				api_aes_decipher(skey, &eskey[1], &mkey[1], eskey[0]);

			// 				memmove(FPEKey, skey, PED_FPE_KEY_LEN);

			// 				result = TRUE;
			// 			}
			// 		}
			// 	}
			// }
		}
	}

	//Clear sensitive data
	memset(keydata, 0x00, sizeof(keydata));
	memset(eskey, 0x00, sizeof(eskey));
	memset(skey, 0x00, sizeof(skey));
	memset(mkey, 0x00, sizeof(mkey));

	return result;
}
*/

// ---------------------------------------------------------------------------
// FUNCTION: Retrieve Format-Preserving Encryption Key
// INPUT   : none.
// OUTPUT  : FPEKey - FPE Key
// RETURN  : TRUE
//			 FALSE
// ---------------------------------------------------------------------------
UINT8	SRED_GetFPEKey(UINT8 *FPEKey)
{
    X9143_AES_PED_KEY_BUNDLE KeyBundle;
    UINT8   keydata[X9143_AES_KEY_DATA_LEN + 1];    // L-V
    UINT8   pkey[PED_FPE_KEY_MSKEY_LEN + 1];    // L-V
	UINT8	skey[PED_FPE_KEY_MSKEY_LEN];
	UINT8	mkey[PED_FPE_KEY_MSKEY_LEN + 1];	// L-V
	UINT8	mac16[16];
	UINT8	MkeyIndex;
	UINT8	result = TRUE;
    UINT8	keyScheme;
    UINT8   keyType;
    UINT8   key[PED_FPE_KEY_AES_DUKPT_WORKING_KEY_LEN];    // L-V
    UINT8   workingKey[16];
    UINT8   ksn[12];


    //Read key opearation mode
    keyScheme = PED_ReadKeyMode();

    if(keyScheme == PED_KEY_MODE_MS)
    {
        PED_FPE_GetKeyType(&keyType);
        PED_SetKPKStatus(keyType, 1);

        if(keyType == AES_128)
        {
            // get FPE Key bundle
	        OS_SECM_GetData(ADDR_PED_FPE_KEY_AES_MKEY, PED_FPE_KEY_AES_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle);

            // verify FPE MKEY bundle
            if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, (UINT8 *)&KeyBundle, keydata, mac16, (UINT8 *)0 ))
            {
                // check FPE MKEY key usage
                if((KeyBundle.Usage[0] == 'K') && (KeyBundle.Usage[1] == '0')) // for KEY encryption?
                {
                    // retrieve FPE MKEY
                    if(!X9143_DecryptKeyBundle_AES(mac16, keydata, mkey, (UINT8 *)0 ))	// mkey=FPE Key MKEY (as the KBPK for FPE Key SKEY)
                    {
                        // reset flag
                        PED_SetKPKStatus(keyType, 0);

                        result = FALSE;
                        goto EXIT;
                    }
                }
            }

            if(result == TRUE)
            {
                result = FALSE;

                // get FPE SKEY bundle
                OS_SECM_GetData(ADDR_PED_FPE_KEY_AES_SKEY_01, PED_FPE_KEY_AES_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle);

                // verify FPE SKEY bundle
                if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, (UINT8 *)&KeyBundle, keydata, mac16, mkey))
                {
                    // check FPE SKEY key usage
                    if((KeyBundle.Usage[0] == 'D') && (KeyBundle.Usage[1] == '0')) // for DATA encryption?
                    {
                        // retrieve FPE SKEY
                        if(X9143_DecryptKeyBundle_AES(mac16, keydata, pkey, mkey)) // pkey=ACC_DEK SKEY
                        {
                            // ==== [Debug] ====
                            printf("pkey = ");
                            for(int i = 0 ; i < pkey[0] + 1 ; i++)
                                printf("%02x", pkey[i]);
                            printf("\n");
                            // ==== [Debug] ====

                            memmove(skey, &pkey[1], pkey[0]);

                            memmove(FPEKey, &pkey[1], pkey[0]);

	    			        result = TRUE;
                        }
                    }
                }
            }
        }

        // reset flag
        PED_SetKPKStatus(keyType, 0);
    }
    else if(keyScheme == PED_KEY_MODE_DUKPT)
    {
        OS_SECM_GetData(ADDR_PED_FPE_KEY_AES_DUKPT_WORKING_KEY, PED_FPE_KEY_AES_DUKPT_WORKING_KEY_LEN, key);
        
        if(key[0] != 0)  //Working key already exists
        {
            memmove(FPEKey, &key[1], key[0]);

            result = TRUE;
            goto EXIT;
        }

        if(AES_DUKPT_RequestDataEncryptionKey(2, workingKey))   //AES128
        {
            memmove(FPEKey, workingKey, 16);

            key[0] = 16;
            memmove(&key[1], workingKey, 16);
            OS_SECM_PutData(ADDR_PED_FPE_KEY_AES_DUKPT_WORKING_KEY, PED_FPE_KEY_AES_DUKPT_WORKING_KEY_LEN, key);

            AES_DUKPT_GetKSN(ksn);
            OS_SECM_PutData(ADDR_PED_FPE_KEY_CURRENT_KSN, 12, ksn);
            SRED_DBG_Put_String(33, (UINT8*)"==== Current KSN for FPE Key ====");	// ==== [Debug] ====
            SRED_DBG_Put_Hex(12, ksn);	// ==== [Debug] ====
        }
        else
            result = FALSE;
    }
    else
        return FALSE;

EXIT:
	//Clear sensitive data
	memset(keydata, 0x00, sizeof(keydata));
    memset(pkey, 0x00, sizeof(pkey));
	memset(skey, 0x00, sizeof(skey));
	memset(mkey, 0x00, sizeof(mkey));
    memset(key, 0x00, sizeof(key));
    memset(workingKey, 0x00, sizeof(workingKey));

	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: Mutual authentication (ISO/IEC 9798-2)
//			 Three pass authentication
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
/*UINT8	SRED_MutualAuthentication(void)
{
	UINT8	result = FALSE;
	UINT8	macKey[16] = {0x65, 0x28, 0x25, 0x3A, 0x69, 0x0B, 0xB6, 0x11, 0x8B, 0xE3, 0x5F, 0xBA, 0xA9, 0x87, 0x68, 0xC9};
	UINT8	dek[16] = {0xFD, 0x92, 0xB2, 0xDF, 0xEE, 0xEB, 0xDF, 0x7C, 0x6B, 0xE3, 0x83, 0x71, 0x00, 0x76, 0xA5, 0xDB};
	UINT8	extendDek[24];
	UINT8	H_ranNum[8];
	UINT8	T_ranNum[8];
	UINT8	ptext[SRED_BUFFER_SIZE];
	UINT32	ptextLen = 0;
	UINT8	ctext[SRED_BUFFER_SIZE];
	UINT32	ctextLen = 0;
	UINT8	mac[UCL_SHA256_HASHSIZE];
	UINT8	rcvMac[UCL_SHA256_HASHSIZE];
	UINT8	sndToken[SRED_BUFFER_SIZE];	//2L-V
	UINT8	rcvToken[SRED_BUFFER_SIZE];	//2L-V
	UINT16	length = 0;
	UINT8	sbuf[8];
	UINT8	dbuf[16];
	UINT8	secretKey[16];

	
	//Mutual Authentication - Three pass authentication (ISO IEC 9798-2)
	//+--------+	            	  RB || Text1				      +--------+
	//|        |<-----------------------------------------------------|        |
	//|        |   TokenAB = Text3 || eKAB (RA || RB || B || Text2)   |        |
	//|   A    |----------------------------------------------------->|   B    |
	//|        |   TokenBA = Text5 || eKAB (RB || RA || Text4)        |        |
	//|        |<-----------------------------------------------------|        |
	//+--------+		                                              +--------+

	//Generate 16 bytes MAC key
	//ucl_rng_read(macKey, 16, UCL_RAND_DEFAULT);

	//Generate 16 bytes account data encryption key
	//ucl_rng_read(dek, PED_ACC_DEK_LEN, UCL_RAND_DEFAULT);

	//Expanded to 24 bytes DEK
	memcpy(extendDek, dek, 16);
	memcpy(&extendDek[16], dek, 8);

	if(LIB_OpenAUX(COM0, auxDLL, COM_9600) == FALSE)
	{
		return FALSE;
	}

	//Initiate mutual authentication
	sbuf[0] = 1;
	sbuf[1] = 0;
	sbuf[2] = 0xC1;	//command
	
	if(LIB_TransmitAUX(sbuf) == TRUE)
	{
		//Step 1 : Host generates a random number RB and send it and, optionally, a text field Text1 to Terminal
	
		if(LIB_ReceiveAUX(dbuf) == TRUE)
		{
			length = dbuf[0] + dbuf[1] * 256;

			if((length != 0x09) || (dbuf[2] != 0xC2))
			{
				result = FALSE;
				goto EXIT;
			}

			memcpy(H_ranNum, &dbuf[3], (length-1));

			//Step 2 : Terminal generates a random number RA, and sends TokenAB to Host

			ucl_rng_read(T_ranNum, 8, UCL_RAND_DEFAULT);

			memcpy(ptext, T_ranNum, 8);
			ptextLen += 8;

			memcpy(&ptext[ptextLen], H_ranNum, 8);
			ptextLen += 8;

			SRED_3DesEcb_Encrypt(ctext, &ctextLen, ptext, ptextLen, extendDek);

			ucl_hmac_sha256(mac, UCL_SHA256_HASHSIZE, ctext, ctextLen, macKey, 16);

			memset(sndToken, 0x00, SRED_BUFFER_SIZE);
			length = 0;

			memcpy(&sndToken[3], ctext, ctextLen);
			length += ctextLen;

			memcpy(&sndToken[3 + length], mac, UCL_SHA256_HASHSIZE);
			length += UCL_SHA256_HASHSIZE;

			sndToken[0] = 1 + length;	//cmd + data
			sndToken[1] = 0;
			sndToken[2] = 0xC3;	//command

			if(LIB_TransmitAUX(sndToken) == TRUE)
			{
				//Step 3 : Host verifies TokenAB

				//Step 4 : Host generates and sends TokenBA to Terminal

				if(LIB_ReceiveAUX(rcvToken) == TRUE)
				{
					//Step 5 : Terminal verifies TokenBA and then checking that the random number RB received from Host

					length = rcvToken[0] + rcvToken[1] * 256;

					if((length != 0x39) || (rcvToken[2] != 0xC4))
					{
						result = FALSE;
						goto EXIT;
					}

					memcpy(ctext, &rcvToken[3], (length - 1 - UCL_SHA256_HASHSIZE));
					ctextLen = length - 1 - UCL_SHA256_HASHSIZE;

					memcpy(rcvMac, &rcvToken[3+ctextLen], UCL_SHA256_HASHSIZE);

					ucl_hmac_sha256(mac, UCL_SHA256_HASHSIZE, ctext, ctextLen, macKey, 16);
					if(LIB_memcmp(mac, rcvMac, UCL_SHA256_HASHSIZE) == 0)	//MAC match
					{
						SRED_3DesEcb_Decrypt(ptext, &ptextLen, ctext, ctextLen, extendDek);
		
						//Check random number RB received from Host
						if(LIB_memcmp(H_ranNum, ptext, 8) == 0)
						{
							//secretKey(16) = RB[5:8] + RA[1:4] + RB[1:4] + RA[5:8]
							memcpy(&secretKey[0], &H_ranNum[4], 4);
							memcpy(&secretKey[4], &T_ranNum[0], 4);
							memcpy(&secretKey[8], &H_ranNum[0], 4);
							memcpy(&secretKey[12], &T_ranNum[4],4);
							OS_SECM_PutData(ADDR_PED_SECRET_KEY, PED_SECRET_KEY_LEN, secretKey);

							LIB_LCD_Cls();
							LIB_LCD_Puts(1, 0, FONT0 + attrCLEARWRITE, 20, (UINT8*)"Valid Authentication");
							LIB_WaitTimeAndKey(300);

							result = TRUE;
						}
						else
						{
							LIB_LCD_Cls();
							LIB_LCD_Puts(1, 0, FONT0 + attrCLEARWRITE, 22, (UINT8*)"Invalid Authentication");
							LIB_WaitTimeAndKey(300);

							result = FALSE;
						}
					}
					else
					{
						LIB_LCD_Cls();
						LIB_LCD_Puts(1, 0, FONT0 + attrCLEARWRITE, 16, (UINT8*)"MAC is incorrect");
						LIB_WaitTimeAndKey(300);

						result = FALSE;
					}
				}			
			}	
		}	
	}
	
EXIT:	
	LIB_CloseAUX();

	//Clear sensitive data
	memset(macKey, 0x00, sizeof(macKey));
	memset(dek, 0x00, sizeof(dek));
	memset(extendDek, 0x00, sizeof(extendDek));
	memset(H_ranNum, 0x00, sizeof(H_ranNum));
	memset(T_ranNum, 0x00, sizeof(T_ranNum));
	memset(ptext, 0x00, sizeof(ptext));
	memset(ctext, 0x00, sizeof(ctext));
	memset(mac, 0x00, sizeof(mac));
	memset(rcvMac, 0x00, sizeof(rcvMac));
	memset(sndToken, 0x00, sizeof(sndToken));
	memset(rcvToken, 0x00, sizeof(rcvToken));
	memset(dbuf, 0x00, sizeof(dbuf));
	memset(secretKey, 0x00, sizeof(secretKey));

	return result;
}*/

// ---------------------------------------------------------------------------
// FUNCTION: Perform authenticated encryption by Encrypt-then-MAC (ISO/IEC 19772)
// INPUT   : pedKey   - PED Key (Fixed Key or Master/Session key or DUKPT)
//			 keyIndex - key index used to encrypt data
//           ap       - MAC algorithm and padding method (MAC_ALGx + MAC_PADx)
//			 mode     - encrypt/decrypt procedure
//			 s		  - Starting Variable
//			 dataIn   - pointer to the plaintext (for encryption procedure)
//						encrypted data string (for decryption procedure)
//           inLen    - length of plaintext (for encryption procedure)
//						length of encrypted data string (for decryption procedure)
// OUTPUT  : dataOut  - encrypted/decrypted data string (for encryption/decryption procedure)
//           outLen	  - length of encrypted data string sent by originator
//						length of decrypted data string received by receipient
// RETURN  : TRUE  - no error occurred
//           FALSE - invalid input or output
// ---------------------------------------------------------------------------
UINT8	SRED_EncryptThenMAC(UINT8 pedKey, UINT8 keyIndex, UINT8 ap, UINT8 mode, UINT8 *s, UINT8 *dataIn, UINT32 inLen, UINT8 *dataOut, UINT32 *outLen)
{
	UINT8	ptext[SRED_BUFFER_SIZE];
	UINT32	ptextLen = 0;
	UINT8	ctext[SRED_BUFFER_SIZE];
	UINT32	ctextLen = 0;
	UINT8	data[SRED_BUFFER_SIZE];
	UINT8	dek[24];	//account data encryption key
	UINT8	iv[8] = {0};	//zero initialization vector
	UINT8	mac[8];	//that is Tag specified in ISO/IEC 19772
	UINT8	rcvMac[8];	//received MAC 
	UINT8	ksn[10];	//key serial number (MUST be 10 bytes)
	UINT8	result = apiFailed;


	//Retrieve ACC_DEK
	if(SRED_GetAccDek(dek) == FALSE)
	{
		return FALSE;
	}

	SRED_DBG_Put_String(29, (UINT8*)"==== Data encryption key ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(PED_ACC_DEK_LEN, dek);	// ==== [Debug] ====

	if(mode == UCL_CIPHER_ENCRYPT)
	{
		//Encrypt data
		SRED_3DesCbc_Encrypt(ctext, &ctextLen, dataIn, inLen, dek, s);

		SRED_DBG_Put_String(23, (UINT8*)"==== Data string D ====");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(inLen, dataIn);	// ==== [Debug] ====
		SRED_DBG_Put_String(25, (UINT8*)"==== C' = encrypt(D) ====");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(ctextLen, ctext);	// ==== [Debug] ====

		//Concatenate Starting Variable and ciphertext
		memcpy(data, s, 8);
		memcpy(&data[8], ctext, ctextLen);

		SRED_DBG_Put_String(15, (UINT8*)"==== S||C' ====");	// ==== [Debug] ====
		SRED_DBG_Put_Hex((8 + ctextLen), data);	// ==== [Debug] ====

		//MAC computation
		switch(pedKey)
		{
			case 0x00:
#if 0
				result = PED_FXKEY_GenMAC(ap, keyIndex, iv, (8 + ctextLen), data, mac);
#endif
				break;

			case 0x01:
				result = PED_MSKEY_GenMAC(ap, keyIndex, iv, (8 + ctextLen), data, mac);
				break;

			case 0x02:
				result = PED_DUKPT_GenMAC(ap, iv, (8 + ctextLen), data, mac, ksn);
				break;
		}
	
		if(result == apiFailed)
		{
			return FALSE;
		}

		SRED_DBG_Put_String(21, (UINT8*)"==== MAC value T ====");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(8, mac);	// ==== [Debug] ====

		//Concatenate the encrypted data with MAC as the encrypted data string to be output
		memcpy(dataOut, ctext, ctextLen);
		memcpy(&dataOut[ctextLen], mac, 8);
		*outLen = ctextLen + 8;

		SRED_DBG_Put_String(19, (UINT8*)"==== C = C'||T ====");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(*outLen, dataOut);	// ==== [Debug] ====

		return TRUE;
	}
	else if(mode == UCL_CIPHER_DECRYPT)
	{
		if(inLen < 8)
		{
			UTIL_ClearScreen();
			UTIL_PutStr(0, 0, FONT0, 36, (UINT8*)"Invalid for the length of ciphertext");
			UTIL_WaitKey();
			return FALSE;
		}

		memcpy(ctext, dataIn, (inLen - 8));
		ctextLen = inLen - 8;
		memcpy(rcvMac, &dataIn[inLen - 8], 8);

		SRED_DBG_Put_String(12, (UINT8*)"==== C' ====");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(ctextLen, ctext);	// ==== [Debug] ====
		SRED_DBG_Put_String(21, (UINT8*)"==== MAC value T ====");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(8, rcvMac);	// ==== [Debug] ====
		SRED_DBG_Put_String(19, (UINT8*)"==== C = C'||T ====");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(inLen, dataIn);	// ==== [Debug] ====

		//Concatenate Starting Variable and ciphertext
		memcpy(data, s, 8);
		memcpy(&data[8], ctext, ctextLen);

		SRED_DBG_Put_String(15, (UINT8*)"==== S||C' ====");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(8 + ctextLen, data);	// ==== [Debug] ====

		//MAC computation
		switch(pedKey)
		{
			case 0x00:
#if 0
				result = PED_FXKEY_GenMAC(ap, keyIndex, iv, (8 + ctextLen), data, mac);
#endif
				break;

			case 0x01:
				result = PED_MSKEY_GenMAC(ap, keyIndex, iv, (8 + ctextLen), data, mac);
				break;

			case 0x02:
				result = PED_DUKPT_GenMAC(ap, iv, (8 + ctextLen), data, mac, ksn);
				break;
		}

		if(result == apiFailed)
		{
			return FALSE;
		}

		SRED_DBG_Put_String(22, (UINT8*)"==== MAC value T' ====");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(8, mac);	// ==== [Debug] ====

		//Check MAC
		if(LIB_memcmp(rcvMac, mac, 8) != 0)
		{
			UTIL_ClearScreen();
			UTIL_PutStr(0, 0, FONT0, 25, (UINT8*)"Invalid for the MAC value");
			UTIL_WaitKey();
			return FALSE;
		}

		//Decrypt data
		SRED_3DesCbc_Decrypt(ptext, &ptextLen, ctext, ctextLen, dek, s);

		memcpy(dataOut, ptext, ptextLen);
		*outLen = ptextLen;

		SRED_DBG_Put_String(11, (UINT8*)"==== D ====");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(*outLen, dataOut);	// ==== [Debug] ====

		UTIL_ClearScreen();
		UTIL_PutStr(0, 0, FONT0, 5, (UINT8*)"Valid");
		UTIL_WaitKey();
		return TRUE;
	}

	//Clear sensitive data
	memset(dek, 0x00, sizeof(dek));
	memset(ctext, 0x00, sizeof(ctext));
	memset(data, 0x00, sizeof(data));
	memset(mac, 0x00, sizeof(mac));
	memset(rcvMac, 0x00, sizeof(rcvMac));

	return FALSE;
}

// ---------------------------------------------------------------------------
// FUNCTION: Data Origin Authentication
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	SRED_DataOriginAuthentication(void)
{
	UINT8	pan[19];
	UINT8	panLen = 0;
	UINT8	buffer[6];
	UINT8	start;
	UINT8	select;
	UINT8	index = 0xFF;
	UINT8	macAlgo = 0;
	UINT8	padding = 0;
	UINT8	ap;	//MAC_ALGx + MAC_PADx
	UINT8	s[8];	//Starting Variable
	UINT8	sbuf[100];
	UINT32	sbufLen = 0;
	UINT8	dbuf[100];
	UINT32	dbufLen = 0;


	//Assume that PAN is the plaintext
	LIB_LCD_Puts(0, 0, FONT0, 17, (UINT8*)"Please enter PAN.");
	SRED_ManualPanKeyEntry(pan, &panLen);
	LIB_LCD_Cls();

	start = 0;
	LIB_LCD_Cls();
	LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_SELECT), (UINT8 *)os_msg_SELECT );
	buffer[0] = 1;	//Starting row number
    buffer[1] = 5;	//Max lcd row cnt
    buffer[2] = 3;	//List items
    buffer[3] = 18;	//Item length
    buffer[4] = 0;	//Offset of LEN field in item
    buffer[5] = FONT0;
	select = LIB_ListBox(start, &buffer[0], (UINT8 *)&os_list_KEYS[0], 30);	//Wait for selection (T.O.=30sec)

	if(select == 0xFF)	//Aborted
	{
		LIB_OpenKeyAll();
		return;
	}
	else if(select != 0x02)	//Enter the key index except DUKPT
	{
		LIB_LCD_Cls();
		LIB_LCD_Puts(0, 0, FONT0, 27, (UINT8*)"Please enter the key index.");
		index = UTIL_GetKeyInNumber(1, FONT0);	//Key index start from 0
	}

	LIB_LCD_Cls();
	LIB_LCD_Puts(0, 0, FONT0, 31, (UINT8*)"Please select the MAC algorithm");
	LIB_LCD_Puts(1, 0, FONT0, 35, (UINT8*)"1. ISO 16609 MAC algorithm 1 (TDEA)");
	LIB_LCD_Puts(2, 0, FONT0, 29, (UINT8*)"2. ISO 9797-1 MAC Algorithm 1");
	LIB_LCD_Puts(3, 0, FONT0, 29, (UINT8*)"3. ISO 9797-1 MAC Algorithm 3");
	switch(UTIL_GetKeyInNumber(4, FONT0))
	{
		case 0x01:
			macAlgo = MAC_ISO16609;
			break;

		case 0x02:
			macAlgo = MAC_ALG1;
			break;

		case 0x03:
			macAlgo = MAC_ALG3;
			break;
	}

	LIB_LCD_Cls();
	LIB_LCD_Puts(0, 0, FONT0, 33, (UINT8*)"Please select the padding method.");
	LIB_LCD_Puts(1, 0, FONT0, 19, (UINT8*)"1. Padding Method 1");
	LIB_LCD_Puts(2, 0, FONT0, 19, (UINT8*)"2. Padding Method 2");
	LIB_LCD_Puts(3, 0, FONT0, 19, (UINT8*)"3. Padding Method 3");
	switch(UTIL_GetKeyInNumber(4, FONT0))
	{
		case 0x01:
			padding = MAC_PAD1;
			break;

		case 0x02:
			padding = MAC_PAD2;
			break;

		case 0x03:
			padding = MAC_PAD3;
			break;
	}

	ap = macAlgo | padding;

	SRED_DBG_Put_String(23, (UINT8*)"==== MAC Algorithm ====");	// ==== [Debug] ====
	SRED_DBG_Put_UCHAR(macAlgo+'0');	// ==== [Debug] ====
	SRED_DBG_Put_String(24, (UINT8*)"==== Padding Method ====");	// ==== [Debug] ====
	SRED_DBG_Put_UCHAR((padding >> 4) + '0');	// ==== [Debug] ====

	//Generate the Starting Variable at random
	api_sys_random(s);

	SRED_DBG_Put_String(29, (UINT8*)"==== Starting Variable S ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(8, s);	// ==== [Debug] ====

	//Encryption procedure (on the originating end)
	SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====
	SRED_DBG_Put_String(30, (UINT8*)"**** Encryption procedure ****");	// ==== [Debug] ====
	SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====
	SRED_EncryptThenMAC(select, index, ap, UCL_CIPHER_ENCRYPT, s, pan, (UINT32)panLen, sbuf, &sbufLen);

	//Decryption procedure (on the receiving end)
	SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====
	SRED_DBG_Put_String(30, (UINT8*)"**** Decryption procedure ****");	// ==== [Debug] ====
	SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====
	SRED_EncryptThenMAC(select, index, ap, UCL_CIPHER_DECRYPT, s, sbuf, sbufLen, dbuf, &dbufLen);

	//Clear sensitive data
	memset(pan, 0x00, sizeof(pan));
	memset(s, 0x00, sizeof(s));
	memset(sbuf, 0x00, sizeof(sbuf));
	memset(dbuf, 0x00, sizeof(dbuf));
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate surrogate PAN values according to NIST SP 800-38G
// INPUT   : pan       - PAN value
//			 panLen    - length of PAN
// OUTPUT  : surrogate - a surrogate PAN values for sensitive middle digits
// RETURN  : TRUE
//			 FALSE
// ---------------------------------------------------------------------------
UINT8	SRED_GenerateSurrogatePAN(UINT8 *pan, UINT8 panLen, UINT8 *surrogate)
{
    UINT8   dek[PED_ACC_DEK_MSKEY_LEN + 1]; // L-V
    UINT8   FPEKey[PED_FPE_KEY_MSKEY_LEN];
	UINT8	tweak[10];
	UINT8	tLen = 0;
	UINT8	middleDigits[9];
	UINT8	middleLen = 0;
	UINT8	ptext[9];
	UINT8	i;


	//Retrieve ACC_DEK
	if(SRED_GetAccDek(dek) == FALSE)
	{
		return FALSE;
	}
	
	//Retrieve FPE Key
	if(SRED_GetFPEKey(FPEKey) == FALSE)
	{
		return FALSE;
	}

	//The leading six digits and the trailing four digits are used as the tweak
	memcpy(tweak, pan, 6);
	tLen += 6;
	memcpy(&tweak[6], &pan[panLen - 4], 4);
	tLen += 4;

	//Sensitive data
	memcpy(middleDigits, &pan[6], (panLen - 10));
	middleLen = panLen - 10;

	//Convert ASCII to numeral string
	for(i = 0 ; i < middleLen ; i++)
	{
		ptext[i] = middleDigits[i] - '0';
	}

	SRED_DBG_Put_String(53, (UINT8*)"-------------------- FF1.Encrypt --------------------");	// ==== [Debug] ====
	//The other ten digits are the tweak for the encryption of the midddle digits
	FPE_FF1_Encrypt(10, FPEKey, tweak, tLen, ptext, middleLen, surrogate);

	//SRED_DBG_Put_String(53, (UINT8*)"-------------------- FF1.Decrypt --------------------");	// ==== [Debug] ====
	//FPE_FF1_Decrypt(10, FPEKey, tweak, tLen, surrogate, middleLen, ptext);	// ==== [Debug] ====

	//Clear sensitive data
	memset(dek, 0x00, sizeof(dek));
	memset(FPEKey, 0x00, sizeof(FPEKey));
	memset(tweak, 0x00, sizeof(tweak));
	memset(ptext, 0x00, sizeof(ptext));
	
	return TRUE;
}

// ---------------------------------------------------------------------------
// FUNCTION: Generate salt
// INPUT   : none.
// OUTPUT  : salt - a randomly generated salt
// RETURN  : none.
// ---------------------------------------------------------------------------
void	SRED_GetSalt(UINT8 *salt)
{
	api_sys_random_len(salt, PED_SALT_LEN);

	OS_SECM_PutData(ADDR_PED_SALT, PED_SALT_LEN, salt);
}

// ---------------------------------------------------------------------------
// FUNCTION: Reset the PAN encrypting interval 
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// NOTE    : This function must be called before encryptin PAN.
// ---------------------------------------------------------------------------
void	SRED_ResetEncryptLimit(void)
{
	os_ped_PanDownCnt = PED_PAN_ENCRYPT_TOUT;

	OS_SECM_ClearData(ADDR_PED_PAN_DATA, PED_PAN_DATA_SLOT_LEN, 0x00);
}

// ---------------------------------------------------------------------------
// FUNCTION: Erase Secure Data when the transaction is completed
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	SRED_EraseSecureData(void)
{
	//Clear Salt
	OS_SECM_ClearData(ADDR_PED_SALT, PED_SALT_LEN, 0x00);

	//Clear PAN data
	OS_SECM_ClearData(ADDR_PED_PAN_DATA, PED_PAN_DATA_SLOT_LEN, 0x00);

	//Clear Account data
	OS_SECM_ClearData(ADDR_PED_ACC_DATA, PED_ACC_DATA_LEN, 0x00);
}

// ---------------------------------------------------------------------------
// FUNCTION: To get MAC key by using Fixed key algorithm
// INPUT   : index  - fixed key index
// OUTPUT  : macKey - MAC key
// RETURN  : TRUE
//			 FALSE
// ---------------------------------------------------------------------------
#if	0
UINT8	SRED_FXKEY_GetMacKey(UINT8 index, UINT8	*macKey)
{
	PED_KEY_BUNDLE	KeyBundle;
	UINT8	keydata[KEY_DATA_LEN + 1]; //L-V
	UINT8	fkey[PED_FKEY_LEN + 1];	//L-V
	UINT8	result = FALSE;
	UINT8	mac8[8];


	PED_InUse(TRUE);

	//Check Fixed key index
	if(index < MAX_FKEY_CNT)
	{
		//Get Fixed key bundle
		OS_SECM_GetData(ADDR_PED_FKEY_01 + (index*PED_FKEY_SLOT_LEN), PED_FKEY_SLOT_LEN, (UINT8 *)&KeyBundle);

		//Verify Fixed key bundle
		if(TR31_VerifyKeyBundle(PED_FKEY_SLOT_LEN, (UINT8 *)&KeyBundle, keydata, mac8))
		{
			//Check key usage
			if((KeyBundle.Usage[0] == 'D') && (KeyBundle.Usage[1] == '0')) //for DATA encryption?
			{
				//Retrieve Fixed key
				if(TR31_DecryptKeyBundle(mac8, keydata, fkey))
					result = TRUE;

				memcpy(macKey, &fkey[1], PED_FKEY_LEN);

				SRED_DBG_Put_String(19, (UINT8*)"==== Fixed Key ====");	// ==== [Debug] ====
				SRED_DBG_Put_Hex(PED_FKEY_LEN, &fkey[1]);	// ==== [Debug] ====
			}
		}

		//Clear sensitive data
		memset(fkey, 0x00, sizeof(fkey));	//Clear KEY data
		memset((UINT8 *)&KeyBundle, 0x00, sizeof(KeyBundle));
		memset(keydata, 0x00, sizeof(keydata));
	}

	PED_InUse(FALSE);

	return result;
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To generate MAC key by using Master/Session key algorithm
// INPUT   : index  - session key index
// OUTPUT  : macKey - MAC key
// RETURN  : TRUE
//			 FALSE
// ---------------------------------------------------------------------------
/*
UINT8	SRED_MSKEY_GetMacKey(UINT8 index, UINT8	*macKey)
{
	PED_KEY_BUNDLE	KeyBundle;
	UINT8	keydata[KEY_DATA_LEN + 1];	//L-V
	UINT8	eskey[PED_MSKEY_LEN + 1];	//L-V
	UINT8	skey[PED_MSKEY_LEN + 1];	//L-V
	UINT8	mkey[PED_MSKEY_LEN + 1];	//L-V
	UINT8	MkeyIndex;
	UINT8	result = FALSE;
	UINT8	mac8[8];

	UINT8	temp[KEY_DATA_LEN + 1];
	UINT8	buf[KEY_BUNDLE_LEN];


	PED_InUse(TRUE);

	//Check SKEY index
	if(index < MAX_SKEY_CNT)
	{
		// get MKEY bundle
		PED_ReadMKeyIndex((UINT8 *)&MkeyIndex);
		OS_SECM_GetData(ADDR_PED_MKEY_01 + (MkeyIndex * PED_MSKEY_SLOT_LEN), PED_MSKEY_SLOT_LEN, buf);

		// verify MKEY bundle
		if(TR31_VerifyKeyBundle(KEY_BUNDLE_LEN, buf, temp, mac8, (UINT8 *)0))
		{
			// retrieve MKEY
			if(!TR31_DecryptKeyBundle(mac8, temp, mkey, (UINT8 *)0))	// mkey=MKEY (as the KBPK for SKEY)
			{
				result = FALSE;
				goto EXIT;
			}
		}

		//Get ESKEY bundle
		OS_SECM_GetData(ADDR_PED_SKEY_01 + (index*PED_MSKEY_SLOT_LEN), PED_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle);

		//Verify ESKEY bundle
		if(TR31_VerifyKeyBundle(PED_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle, keydata, mac8, mkey))
		{
			//Check ESKEY key usage
			if((KeyBundle.Usage[0] == 'D') && (KeyBundle.Usage[1] == '0')) //for DATA encryption?
			{
				//Retrieve ESKEY key (SKEY encrypted by MKEY)
				if(TR31_DecryptKeyBundle(mac8, keydata, eskey, mkey))
				{
					memmove( skey, &eskey[1], eskey[0] );
					
					//Get MKEY bundle
					PED_ReadMKeyIndex((UINT8 *)&MkeyIndex);
					OS_SECM_GetData(ADDR_PED_MKEY_01 + (MkeyIndex*PED_MSKEY_SLOT_LEN), PED_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle);

					//Verify MKEY bundle
					if(TR31_VerifyKeyBundle(PED_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle, keydata, mac8, (UINT8 *)0 ))
					{
						//Check MKEY key usage
						if((KeyBundle.Usage[0] == 'K') && (KeyBundle.Usage[1] == '0')) // for KEY encryption?
						{
							//Retrieve MKEY
//							if(TR31_DecryptKeyBundle(mac8, keydata, mkey))
//							{
								//Retrieve SKEY from MKEY
//								PED_TripleDES2(&mkey[1], eskey[0], &eskey[1], skey);

								memcpy(macKey, skey, PED_MSKEY_LEN);

								SRED_DBG_Put_String(21, (UINT8*)"==== Session Key ====");	// ==== [Debug] ====
								SRED_DBG_Put_Hex(PED_MSKEY_LEN, skey);	// ==== [Debug] ====

								result = TRUE;
//							}
						}
					}
				}
			}
		}

EXIT:
		//Clear sensitive data
		memset(mkey, 0x00, sizeof(mkey));	//Clear KEY data
		memset(skey, 0x00, sizeof(skey));	//Clear KEY data
		memset((UINT8 *)&KeyBundle, 0x00, sizeof(KeyBundle));
		memset(keydata, 0x00, sizeof(keydata));
		memset(eskey, 0x00, sizeof(eskey));
		
		memset(temp, 0x00, sizeof(temp));
		memset(buf, 0x00, sizeof(buf));
	}

	PED_InUse(FALSE);

	return result;
}
*/

// ---------------------------------------------------------------------------
// FUNCTION: To generate MAC key by using Master/Session key algorithm
// INPUT   : index  - session key index
// OUTPUT  : macKey - MAC key
//           keyLen - length of MAC key
// RETURN  : TRUE
//			 FALSE
// ---------------------------------------------------------------------------
UINT8	SRED_MSKEY_GetMacKey(UINT8 index, UINT8	*macKey, UINT8 *keyLen)
{
    UINT8	skey[PED_PEK_MSKEY_LEN+1];	// L-V
    UINT8	mkey[PED_PEK_MSKEY_LEN+1];	// L-V
    X9143_TDES_PED_KEY_BUNDLE   tdesKeyBundle;
    X9143_AES_PED_KEY_BUNDLE    aesKeyBundle; 
    UINT8	keydata[X9143_AES_KEY_DATA_LEN + 1]; // L-V
    UINT8	MkeyIndex;
    UINT8	result = TRUE;
    UINT8	mac8[8];
    UINT8   mac16[16];
    UINT8   keyType;


	PED_InUse(TRUE);

	// check PEK SKEY index
    if(index < MAX_PEK_SKEY_CNT)
	{
        PED_PEK_GetKeyType(&keyType);
        PED_SetKPKStatus(keyType, 1);

        if((keyType == TDES_128) || (keyType == TDES_192))
        {
            // get PEK MKEY bundle
            PED_PEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
            OS_SECM_GetData(ADDR_PED_PEK_TDES_MKEY_01 + (MkeyIndex * PED_PEK_TDES_MSKEY_SLOT_LEN), PED_PEK_TDES_MSKEY_SLOT_LEN, (UINT8 *)&tdesKeyBundle);

            // verify PEK MKEY bundle
            if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, (UINT8 *)&tdesKeyBundle, keydata, mac8, (UINT8 *)0 ))
            {
                // check PEK MKEY key usage
                if((tdesKeyBundle.Usage[0] == 'K') && (tdesKeyBundle.Usage[1] == '0')) // for KEY encryption?
                {
                    // retrieve PEK MKEY
                    if(!X9143_DecryptKeyBundle_TDES(mac8, keydata, mkey, (UINT8 *)0 ))	// mkey=PEK MKEY (as the KBPK for SKEY)
                    {
                        // reset flag
                        PED_SetKPKStatus(keyType, 0);

                        result = FALSE;
                        goto EXIT;
                    }
                }
            }

            if(result == TRUE)
            {
                result = FALSE;

                // get PEK SKEY bundle
                OS_SECM_GetData(ADDR_PED_PEK_TDES_SKEY_01 + (index * PED_PEK_TDES_MSKEY_SLOT_LEN), PED_PEK_TDES_MSKEY_SLOT_LEN, (UINT8 *)&tdesKeyBundle);

                // verify PEK SKEY bundle
                if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, (UINT8 *)&tdesKeyBundle, keydata, mac8, mkey))
                {
                    // check PEK SKEY key usage
                    if((tdesKeyBundle.Usage[0] == 'D') && (tdesKeyBundle.Usage[1] == '0')) // for DATA encryption?
                    {
                        // retrieve PEK SKEY
                        if(X9143_DecryptKeyBundle_TDES(mac8, keydata, skey, mkey)) // skey=PEK SKEY
                        {
                            // ==== [Debug] ====
                            printf("skey = ");
                            for(int i = 0 ; i < skey[0] + 1 ; i++)
                                printf("%02x", skey[i]);
                            printf("\n");
                            // ==== [Debug] ====

                            memcpy(macKey, &skey[1], skey[0]);
                            *keyLen = skey[0];

							SRED_DBG_Put_String(21, (UINT8*)"==== Session Key ====");	// ==== [Debug] ====
							SRED_DBG_Put_Hex(skey[0], &skey[1]);	// ==== [Debug] ====

                            result = TRUE;
                        }
                    }
                }
            }
        }
        else if((keyType == AES_128) || (keyType == AES_192) || (keyType == AES_256))
        {
            // get PEK MKEY bundle
            PED_PEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
            OS_SECM_GetData(ADDR_PED_PEK_AES_MKEY_01 + (MkeyIndex * PED_PEK_AES_MSKEY_SLOT_LEN), PED_PEK_AES_MSKEY_SLOT_LEN, (UINT8 *)&aesKeyBundle);

            // verify PEK MKEY bundle
            if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, (UINT8 *)&aesKeyBundle, keydata, mac16, (UINT8 *)0 ))
            {
                // check MKEY key usage
                if((aesKeyBundle.Usage[0] == 'K') && (aesKeyBundle.Usage[1] == '0')) // for KEY encryption?
                {
                    // retrieve PEK MKEY
                    if(!X9143_DecryptKeyBundle_AES(mac16, keydata, mkey, (UINT8 *)0 ))	// mkey=PEK MKEY (as the KBPK for SKEY)
                    {
                        // reset flag
                        PED_SetKPKStatus(keyType, 0);

                        result = FALSE;
                        goto EXIT;
                    }
                }
            }

            if(result == TRUE)
            {
                result = FALSE;

                // get PEK SKEY bundle
                OS_SECM_GetData(ADDR_PED_PEK_AES_SKEY_01 + (index * PED_PEK_AES_MSKEY_SLOT_LEN), PED_PEK_AES_MSKEY_SLOT_LEN, (UINT8 *)&aesKeyBundle);

                // verify PEK SKEY bundle
                if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, (UINT8 *)&aesKeyBundle, keydata, mac16, mkey))
                {
                    // check PEK SKEY key usage
                    if((aesKeyBundle.Usage[0] == 'D') && (aesKeyBundle.Usage[1] == '0')) // for DATA encryption?
                    {
                        // retrieve PEK SKEY
                        if(X9143_DecryptKeyBundle_AES(mac16, keydata, skey, mkey)) // skey=PEK SKEY
                        {
                            // ==== [Debug] ====
                            printf("skey = ");
                            for(int i = 0 ; i < skey[0] + 1 ; i++)
                                printf("%02x", skey[i]);
                            printf("\n");
                            // ==== [Debug] ====

                            memcpy(macKey, &skey[1], skey[0]);
                            *keyLen = skey[0];

							SRED_DBG_Put_String(21, (UINT8*)"==== Session Key ====");	// ==== [Debug] ====
							SRED_DBG_Put_Hex(skey[0], &skey[1]);	// ==== [Debug] ====

                            result = TRUE;
                        }
                    }
                }
            }
        }

        // reset flag
        PED_SetKPKStatus(keyType, 0);

EXIT:
		//Clear sensitive data
		memset(mkey, 0x00, sizeof(mkey));	//Clear KEY data
		memset(skey, 0x00, sizeof(skey));	//Clear KEY data
		memset((UINT8 *)&tdesKeyBundle, 0x00, sizeof(tdesKeyBundle));
        memset((UINT8 *)&aesKeyBundle, 0x00, sizeof(aesKeyBundle));
		memset(keydata, 0x00, sizeof(keydata));
	}

	PED_InUse(FALSE);

	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate MAC key by using DUKPT algorithm
// INPUT   : none.
// OUTPUT  : ksn    - key serial number (10 bytes) 
//			 macKey - MAC key
// RETURN  : TRUE
//			 FALSE
// NOTE    : The "DUKPT_RequestPinEntry()" shall be called prior to calling 
//           this function, the MAC key is updated over there.
// ---------------------------------------------------------------------------
UINT8	SRED_DUKPT_GetMacKey(UINT8 *ksn, UINT8 *macKey)
{
	UINT8	mac[8];
	UINT8	result = FALSE;


	PED_InUse(TRUE);

	if(PED_VerifyKeyStatus_DUKPT())
	{
		memset(macKey, 0x00, FUTURE_KEY_LEN);
		if(DUKPT_RequestPinEntry(macKey, ksn, mac))	//Retrieve current ksn and new a next future key
		{						//The "macKey" & "mac" are null parameters
			OS_SECM_GetData(ADDR_MAC_KEY_REG + 1, FUTURE_KEY_LEN, macKey);	//Retrieve real current MAC key

			SRED_DBG_Put_String(23, (UINT8*)"==== DUKPT MAC Key ====");	// ==== [Debug] ====
			SRED_DBG_Put_Hex(FUTURE_KEY_LEN, macKey);	// ==== [Debug] ====

			result = TRUE;
		}
		else
		{
			result = FALSE;
		}
			
		OS_SECM_ClearData(ADDR_MAC_KEY_REG, FUTURE_KEY_SLOT_LEN, 0x00);	//Clear KEY data
	}

	PED_InUse(FALSE);

	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate MAC key by using AES DUKPT algorithm
// INPUT   : keyType - working key type.
//			           0: _2TDEA_
//			           1: _3TDEA_
//			           2: _AES128_
//			           3: _AES192_
//			           4: _AES256_
// OUTPUT  : ksn    - key serial number (12 bytes) 
//			 macKey - MAC key
//           keyLen - length of MAC key
// RETURN  : TRUE
//			 FALSE
// NOTE    : The "DUKPT_RequestPinEntry()" shall be called prior to calling 
//           this function, the MAC key is updated over there.
// ---------------------------------------------------------------------------
UINT8	SRED_AES_DUKPT_GetMacKey(UINT8 *ksn, UINT8 *macKey, UINT8 *keyLen)
{
    UINT8   keyType;
    UINT8	mackey[32];
	UINT8	mac[8];
	UINT8	result = FALSE;


	PED_InUse(TRUE);

    if(PED_VerifyKeyStatus_AES_DUKPT())
    {
        PED_AES_DUKPT_GetKeyType(&keyType);
        SRED_AES_DUKPT_setWorkingKeyType(1);

        if(keyType == AES_128)
        {
            if(!((MAC_workingKeyType == 0) || (MAC_workingKeyType == 1) || (MAC_workingKeyType == 2)))
            {
                //Working key type is not TDES-128, TDES-192, or AES-128
                return FALSE;
            }
        }
        else if(keyType == AES_192)
        {
            if(!((MAC_workingKeyType == 0) || (MAC_workingKeyType == 1) || (MAC_workingKeyType == 2) || (MAC_workingKeyType == 3)))
            {
                //Working key type is not TDES-128, TDES-192, AES-128, or AES-192
                return FALSE;
            }
        }

        memset(mackey, 0x00, sizeof(mackey));
        if(AES_DUKPT_RequestMacKey(MAC_workingKeyType, mackey))
        {
            SRED_DBG_Put_String(27, (UINT8*)"==== AES DUKPT MAC Key ====");	// ==== [Debug] ====
            
            if((MAC_workingKeyType == 0) || (MAC_workingKeyType == 2))    //TDES-128 or AES-128
            {
                *keyLen = 16;
                SRED_DBG_Put_Hex(16, macKey);	// ==== [Debug] ====
            }
            else if((MAC_workingKeyType == 1) || (MAC_workingKeyType == 3))   //TDES-192 or AES-192
            {
                *keyLen = 24;
                SRED_DBG_Put_Hex(24, macKey);	// ==== [Debug] ====
            }
            else if(MAC_workingKeyType == 4)   //AES-256
            {
                *keyLen = 32;
                SRED_DBG_Put_Hex(32, macKey);	// ==== [Debug] ====
            }

            AES_DUKPT_GetKSN(ksn);

            SRED_DBG_Put_String(33, (UINT8*)"==== Current KSN for MAC Key ====");	// ==== [Debug] ====
            SRED_DBG_Put_Hex(12, ksn);	// ==== [Debug] ====

            result = TRUE;
        }
    }

	PED_InUse(FALSE);

	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: Get length of T
// INPUT   : iptDataOfT - tag
// OUTPUT  : optLenOfT - length of T
// RETURN  : TRUE
//			 FALSE
// ---------------------------------------------------------------------------
UINT8	SRED_Get_TLVLengthOfT(UINT8 *iptDataOfT, UINT8 *optLenOfT)
{
	UINT16	cntIdx = 0;


	if (iptDataOfT[0] != 0)
	{
		for (cntIdx=0; cntIdx < 256; cntIdx++)
		{
			if (cntIdx == 0)
			{
				if ((iptDataOfT[0] & 0x1F) != 0x1F)	//First Byte & No subsequent bytes
				{
					optLenOfT[0]=1;
					return TRUE;
				}
			}
			else
			{
				if ((iptDataOfT[cntIdx] & 0x80) == 0x00)	//Last tag byte
				{
					optLenOfT[0]=cntIdx+1;
					return TRUE;
				}
			}
		}
	}

	optLenOfT[0]=0;
	return FALSE;
}

// ---------------------------------------------------------------------------
// FUNCTION: Split process
// INPUT   : data
// OUTPUT  : none.
// RETURN  : a byte of ASCII string
// ---------------------------------------------------------------------------
UINT8	SRED_SubSplit(UINT8 data)
{
	 return ( data > 9) ? (data-9+'@') : (data+'0');
}

// ---------------------------------------------------------------------------
// FUNCTION: Convert BCD data to ASCII string
// INPUT   : src - BCD data
//			 pair_len - length of ASCII string
// OUTPUT  : des - ASCII string
// RETURN  : none.
// ---------------------------------------------------------------------------
void	SRED_Split(UINT8 *des, UINT8 *src, char pair_len)
{
	UINT8 cnt;


	for (cnt=0; cnt < pair_len; cnt++, src++)
	{
		*des++ = SRED_SubSplit((*src & 0xF0) >> 4);
		*des++ = SRED_SubSplit(*src & 0x0F);
	}
}

// ---------------------------------------------------------------------------
// FUNCTION: compress process
// INPUT   : data
// OUTPUT  : none.
// RETURN  : the half byte of BCD data
// ---------------------------------------------------------------------------
UINT8	SRED_SubCompress(UINT8 data)
{
	UINT8 factor;


	if ((data>='0') && (data<='?'))
		factor = '0';
	else if ((data>='A') && (data<='F'))
		factor = '7';
	else
		factor = FALSE;

	return ( factor ? (data - factor) : 0x0F);
}

// ---------------------------------------------------------------------------
// FUNCTION: Convert ASCII string to BCD data
// INPUT   : src - ASCII string
//			 pair_len - length of BCD data
// OUTPUT  : des - BCD data
// RETURN  : none.
// ---------------------------------------------------------------------------
void	SRED_Compress(UINT8 *des, UINT8 *src, UINT8 pair_len)
{
	UINT8 len;


	for (len=0; len < pair_len; len++)
	{
		*des = SRED_SubCompress(*src++) << 4;
		*des++ += SRED_SubCompress(*src++);
	}
}

// ---------------------------------------------------------------------------
// FUNCTION: Generate surrogate PAN values and output outside of the device
//			 according to PCI DSS Requirements 3.4
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//			 FALSE
// ---------------------------------------------------------------------------
UINT8	SRED_OutputPAN(void)
{
	UINT8	pan[19];
	UINT8	panLen = 0;
	int		select;
	UINT8	dataOut[SRED_BUFFER_SIZE];
	UINT8	length = 0;


	//Enter PAN
	LIB_LCD_Puts(0, 0, FONT0, 17, (UINT8*)"Please enter PAN.");
	SRED_ManualPanKeyEntry(pan, &panLen);
	LIB_LCD_Cls();

	//Encrypt PAN data and write it to NVSRAM
	if(SRED_Func_StorePAN(pan, panLen, 1, 0) == FALSE)
	{
		LIB_LCD_Cls();
		LIB_LCD_PutMsg(1, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING_ERR), (UINT8 *)os_msg_PROCESSING_ERR);
		LIB_WaitTimeAndKey(300);
		return FALSE;
	}

	LIB_LCD_Cls();
	LIB_LCD_PutMsg(1, COL_RIGHTMOST, FONT1 + attrCLEARWRITE, sizeof(os_msg_WRITE_OK), (UINT8 *)os_msg_WRITE_OK);
	LIB_WaitTimeAndKey(300);

	//Get PAN data
	while(1)
	{
		LIB_LCD_Cls();
		LIB_LCD_Puts(0, 0, FONT0, 37, (UINT8*)"Select the output format of PAN data.");
		LIB_LCD_Puts(1, 0, FONT0, 21, (UINT8*)"1. Plaintext PAN data");
		LIB_LCD_Puts(2, 0, FONT0, 36, (UINT8*)"2. Encrypted sensitive middle digits");
		LIB_LCD_Puts(3, 0, FONT0, 22, (UINT8*)"3. Ciphertext PAN data");
		select = UTIL_GetKeyInNumber(4, FONT0);

		if(select == -1)
		{
			break;
		}
		
		if(SRED_Func_GetPAN((select - 1), dataOut, &length) == FALSE)
		{
			LIB_LCD_Cls();
			LIB_LCD_PutMsg(1, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING_ERR), (UINT8 *)os_msg_PROCESSING_ERR);
			LIB_WaitTimeAndKey(300);
			return FALSE;
		}

		LIB_LCD_Cls();
		LIB_LCD_PutMsg(1, COL_RIGHTMOST, FONT1 + attrCLEARWRITE, sizeof(os_msg_READ_OK), (UINT8*)os_msg_READ_OK);
		LIB_WaitTimeAndKey(300);

		LIB_LCD_Cls();
		LIB_LCD_Puts(0, 0, FONT0, 8, (UINT8*)"PAN data");
		LIB_DumpHexData(0, 1, length, dataOut);
	}
	
	//Clear sensitive data
	memset(pan, 0x00, sizeof(pan));
	memset(dataOut, 0x00, sizeof(dataOut));
	
	return TRUE;
}

// ---------------------------------------------------------------------------
// FUNCTION: Encrypt PAN and write to NVSRAM
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
/*UINT8	SRED_EncryptPan(void)
{
	UINT8	key[PED_ACC_DEK_LEN];
	UINT8	pan[19];
	UINT8	panLen = 0;
	UINT8	encryptedPan[24];
	UINT32	encryptedPanLen = 0;


	//Retrieve ACC_DEK
	if(SRED_GetAccDek(key) == FALSE)
	{
		return FALSE;
	}

	//Enter PAN
	LIB_LCD_Puts(0, 0, FONT0, 17, (UINT8*)"Please enter PAN.");
	SRED_ManualPanKeyEntry(pan, &panLen);
	LIB_LCD_Cls();

	//Encrypt PAN
	SRED_3DesEcb_Encrypt(encryptedPan, &encryptedPanLen, pan, (UINT32)panLen, key);

	SRED_DBG_Put_String(13, (UINT8*)"==== PAN ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(panLen, pan);	// ==== [Debug] ====
	SRED_DBG_Put_String(13, (UINT8*)"==== Key ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(24, key);	// ==== [Debug] ====
	SRED_DBG_Put_String(23, (UINT8*)"==== Encrypted PAN ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(encryptedPanLen, encryptedPan);	// ==== [Debug] ====

	//Write encrypted PAN to NVSRAM
	OS_SECM_PutData(ADDR_PED_ACC_DATA, encryptedPanLen, encryptedPan);

	//Clear sensitive data
	memset(key, 0x00, sizeof(key));
	memset(pan, 0x00, sizeof(pan));
	memset(encryptedPan, 0x00, sizeof(encryptedPan));

	return TRUE;
}*/

// ---------------------------------------------------------------------------
// FUNCTION: Enter the PAN manually
// INPUT   : none.
// OUTPUT  : pan    - Manually entered card number
//			 panLen - the length of PAN
// RETURN  : TRUE
//			 FALSE
// ---------------------------------------------------------------------------
UINT8	SRED_ManualPanKeyEntry(UINT8 *pan, UINT8 *panLen)
{
	UINT8	result = FALSE;
	UINT8	ch;
	UINT8	buf[20] = {0};


	while(1)
	{
		ch = LIB_WaitKey();
		if(ch == 'y')	//ENTER
		{
			result = TRUE;
			break;
		}
		else if(ch == 'x')	//CANCEL
		{
			goto EXIT;
		}
		else if(ch == 'n')	//CLEAR
		{
			UTIL_ClearRow(1, 1, FONT1);
			buf[0] = 0;
		}
		else if(ch == '#')	//BACKSPACE
		{
			if(buf[0] > 0)
			{
				buf[0]--;
				UTIL_ClearRow(1, 1, FONT1);
				UTIL_PutStr(1, 0, FONT1, buf[0], &buf[1]);
			}
		}
		else if((ch >= '0') && (ch <= '9'))
		{
			if(buf[0] < 19)
			{
				buf[0]++;
				buf[buf[0]] = ch;
				UTIL_ClearRow(1, 1, FONT1);
				UTIL_PutStr(1, 0, FONT1, buf[0], &buf[1]);
			}
			else
			{
				break;
			}
		}
	}

	memcpy(pan, &buf[1], buf[0]);
	*panLen = buf[0];

EXIT:
	//Clear sensitive data
	memset(buf, 0x00, sizeof(buf));

	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: setup AES DUKPT derived working key for MAC generation.
// INPUT   : mode - setting mode.
//                  0: set working key of ACC_DEK
//                  1: set working key of MAC key
// OUTPUT  : none.
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
UINT8	SRED_AES_DUKPT_setWorkingKeyType(UINT8 mode)
{
    UINT8   msg_setAccDekKeyType[] = {"SET ACC_DEK KEY TYPE"};
    UINT8   msg_setMacKeyType[] = {"SET MAC KEY TYPE"};
    UINT8	msg_2TDES[] = {"0-2TDEA"};
    UINT8	msg_3TDEA[] = {"1-3TDEA"};
    UINT8	msg_AES128[] = {"2-AES128"};
    UINT8	msg_AES192[] = {"3-AES192"};
    UINT8	msg_AES256[] = {"4-AES256"};
    UINT8   result = TRUE;
    

    LIB_LCD_Cls();

    if(mode == 0)
    {
        LIB_LCD_PutMsg(0, COL_LEFTMOST, FONT1, strlen(msg_setAccDekKeyType), (UINT8 *)msg_setAccDekKeyType);
        LIB_DispHexByte(1, 0, AccDek_workingKeyType);
        
        LIB_LCD_PutMsg(2, COL_LEFTMOST, FONT1, strlen(msg_3TDEA), (UINT8 *)msg_3TDEA);
        LIB_LCD_PutMsg(3, COL_LEFTMOST, FONT1, strlen(msg_AES128), (UINT8 *)msg_AES128);
        LIB_LCD_PutMsg(4, COL_LEFTMOST, FONT1, strlen(msg_AES192), (UINT8 *)msg_AES192);
        LIB_LCD_PutMsg(5, COL_LEFTMOST, FONT1, strlen(msg_AES256), (UINT8 *)msg_AES256);
    }
    else
    {
        LIB_LCD_PutMsg(0, COL_LEFTMOST, FONT1, strlen(msg_setMacKeyType), (UINT8 *)msg_setMacKeyType);
        LIB_DispHexByte(1, 0, MAC_workingKeyType);

        LIB_LCD_PutMsg(2, COL_LEFTMOST, FONT1, strlen(msg_2TDES), (UINT8 *)msg_2TDES);
        LIB_LCD_PutMsg(3, COL_LEFTMOST, FONT1, strlen(msg_3TDEA), (UINT8 *)msg_3TDEA);
        LIB_LCD_PutMsg(4, COL_LEFTMOST, FONT1, strlen(msg_AES128), (UINT8 *)msg_AES128);
        LIB_LCD_PutMsg(5, COL_LEFTMOST, FONT1, strlen(msg_AES192), (UINT8 *)msg_AES192);
        LIB_LCD_PutMsg(6, COL_LEFTMOST, FONT1, strlen(msg_AES256), (UINT8 *)msg_AES256);
    }

    switch(LIB_WaitKey())
    {
        case 'x':
            result = FALSE;
            break;

        case '0':   //TDES-128
            if(mode == 0)
                result = FALSE;
            else
                MAC_workingKeyType = 0;
            
            break;

        case '1':   //TDES-192
            if(mode == 0)
                AccDek_workingKeyType = 1;
            else
                MAC_workingKeyType = 1;
            
            break;

        case '2':   //AES-128
            if(mode == 0)
                AccDek_workingKeyType = 2;
            else
                MAC_workingKeyType = 2;
            
            break;

        case '3':   //AES-192
            if(mode == 0)
                AccDek_workingKeyType = 3;
            else
                MAC_workingKeyType = 3;
            
            break;

        case '4':   //AES-256
            if(mode == 0)
                AccDek_workingKeyType = 4;
            else
                MAC_workingKeyType = 4;
            
            break;

        default:
            result = FALSE;
            break;
    }

    if(mode == 0)
        LIB_DispHexByte(1, 0, AccDek_workingKeyType);
    else
        LIB_DispHexByte(1, 0, MAC_workingKeyType);

    LIB_WaitKey();

    return result;
}
