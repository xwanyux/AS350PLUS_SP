//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : 							                                **
//**  PRODUCT  : AS350-X6							                        **
//**                                                                        **
//**  FILE     : ANS_X9143_2022.C					                        **
//**  MODULE   : 			   				                                **
//**                                                                        **
//**  FUNCTION : Key Bundle Management					                    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2023/04/10                                                 **
//**  EDITOR   : Tammy Tsai                                                **
//**                                                                        **
//**  Copyright(C) 2008-2023 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
// 
//----------------------------------------------------------------------------
#include <stdlib.h>
#include <string.h>
#include "OS_SECM.h"
#include "OS_PED.h"
#include "DEV_PED.h"
#include "ANS_X9143_2022.h"


// ---------------------------------------------------------------------------
// FUNCTION: To generate Key Block Encryption Key,
//	         derived from Key Block Protection Key (Kb) according to ANSI X9.143.
// INPUT   : k1     - CMAC subkey.
//	         KpkSrc - source of the Key Block Protection Key. (L-V)
//			          using primitive KPK if NULLPTR.
// OUTPUT  : Kbe    - Key Block Encryption Key. (16/24 bytes)
// RETURN  : TRUE	- success.
//           FALSE	- failure. (Key Block Protection Key is absent)
// ---------------------------------------------------------------------------
UINT32  X9143_GenKeyBlockEncryptKey_TDES(UINT8 *k1, UINT8 *Kbe, UINT8 *KpkSrc)
{
	UINT32  i;
	UINT32	status;
	UINT8	Kb[PED_192_TDES_KEY_PROTECT_KEY_SLOT_LEN];
	UINT8	kdid_16B[8] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80};	// key derivation input data
    UINT8	kdid_24B[8] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0xC0};	// key derivation input data
	UINT8	temp[8];


	status = FALSE;

	// retrieve Key Block Protection Key
    if(PED_VerifyKPKStatus(TDES_128) == TRUE)
    {
        if(KpkSrc == NULLPTR)
            OS_SECM_GetData(ADDR_PED_128_TDES_KEY_PROTECT_KEY, PED_128_TDES_KEY_PROTECT_KEY_SLOT_LEN, Kb);
        else
            memmove(Kb, KpkSrc, PED_128_TDES_KEY_PROTECT_KEY_LEN + 1);
    }    
    else if(PED_VerifyKPKStatus(TDES_192) == TRUE)
    {
        if(KpkSrc == NULLPTR)
            OS_SECM_GetData(ADDR_PED_192_TDES_KEY_PROTECT_KEY, PED_192_TDES_KEY_PROTECT_KEY_SLOT_LEN, Kb);
        else
            memmove(Kb, KpkSrc, PED_192_TDES_KEY_PROTECT_KEY_LEN + 1);
    }

	if(Kb[0] == PED_128_TDES_KEY_PROTECT_KEY_LEN)
	{
		for(i = 0 ; i < 8 ; i++)
			temp[i] = kdid_16B[i] ^ k1[i];

		PED_TripleDES(&Kb[1], 8, temp, Kbe);	//  first 8 bytes of the Key Block Encryption Key

		kdid_16B[0] += 1;	// counter++

		for(i = 0 ; i < 8 ; i++)
			temp[i] = kdid_16B[i] ^ k1[i];

		PED_TripleDES(&Kb[1], 8, temp, &Kbe[8]);	//  second 8 bytes of the Key Block Encryption Key

		status = TRUE;
	}
    else if(Kb[0] == PED_192_TDES_KEY_PROTECT_KEY_LEN)
	{
		for(i = 0 ; i < 8 ; i++)
			temp[i] = kdid_24B[i] ^ k1[i];

        api_3des_encipher(temp, Kbe, &Kb[1]);   //  first 8 bytes of the Key Block Encryption Key

		kdid_24B[0] += 1;	// counter++

		for(i = 0 ; i < 8 ; i++)
			temp[i] = kdid_24B[i] ^ k1[i];

        api_3des_encipher(temp, &Kbe[8], &Kb[1]);   //  second 8 bytes of the Key Block Encryption Key

        kdid_24B[0] += 1;	// counter++

		for(i = 0 ; i < 8 ; i++)
			temp[i] = kdid_24B[i] ^ k1[i];

        api_3des_encipher(temp, &Kbe[16], &Kb[1]);  //  third 8 bytes of the Key Block Encryption Key

		status = TRUE;
	}

	// clear sensitive data
	memset(Kb, 0x00, sizeof(Kb));

	return status;
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate Key Block MAC Key,
//	         derived from Key Block Protection Key (Kb) according to ANSI X9.143.
// INPUT   : k1     - CMAC subkey.
//	         KpkSrc - source of the Key Block Protection Key. (L-V)
//			          using primitive KPK if NULLPTR.
// OUTPUT  : Kbm    - Key Block MAC Key.        (16/24 bytes)
// RETURN  : TRUE	- success.
//           FALSE	- failure. (Key Block Protection Key is absent)
// ---------------------------------------------------------------------------
UINT32  X9143_GenKeyBlockMacKey_TDES(UINT8 *k1, UINT8 *Kbm, UINT8 *KpkSrc)
{
	UINT32  i;
	UINT32	status;
	UINT8	Kb[PED_192_TDES_KEY_PROTECT_KEY_SLOT_LEN];
	UINT8	kdid_16B[8] = {0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80};	// key derivation input data
    UINT8	kdid_24B[8] = {0x01, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0xC0};	// key derivation input data
	UINT8	temp[8];


	status = FALSE;

	// retrieve Key Block Protection Key
    if(PED_VerifyKPKStatus(TDES_128) == TRUE)
    {
        if(KpkSrc == NULLPTR)
            OS_SECM_GetData(ADDR_PED_128_TDES_KEY_PROTECT_KEY, PED_128_TDES_KEY_PROTECT_KEY_SLOT_LEN, Kb);
        else
            memmove(Kb, KpkSrc, PED_128_TDES_KEY_PROTECT_KEY_LEN + 1);
    }    
    else if(PED_VerifyKPKStatus(TDES_192) == TRUE)
    {
        if(KpkSrc == NULLPTR)
            OS_SECM_GetData(ADDR_PED_192_TDES_KEY_PROTECT_KEY, PED_192_TDES_KEY_PROTECT_KEY_SLOT_LEN, Kb);
        else
            memmove(Kb, KpkSrc, PED_192_TDES_KEY_PROTECT_KEY_LEN + 1);
    }

	if(Kb[0] == PED_128_TDES_KEY_PROTECT_KEY_LEN)
	{
		for(i = 0 ; i < 8 ; i++)
			temp[i] = kdid_16B[i] ^ k1[i];

		PED_TripleDES(&Kb[1], 8, temp, Kbm);	//  first 8 bytes of the Key Block MAC Key

		kdid_16B[0] += 1;	// counter++

		for(i = 0 ; i < 8 ; i++)
			temp[i] = kdid_16B[i] ^ k1[i];

		PED_TripleDES(&Kb[1], 8, temp, &Kbm[8]);	//  second 8 bytes of the Key Block MAC Key

		status = TRUE;
	}
    else if(Kb[0] == PED_192_TDES_KEY_PROTECT_KEY_LEN)
	{
		for(i = 0 ; i < 8 ; i++)
			temp[i] = kdid_24B[i] ^ k1[i];

        api_3des_encipher(temp, Kbm, &Kb[1]);   //  first 8 bytes of the Key Block MAC Key

		kdid_24B[0] += 1;	// counter++

		for(i = 0 ; i < 8 ; i++)
			temp[i] = kdid_24B[i] ^ k1[i];

        api_3des_encipher(temp, &Kbm[8], &Kb[1]);   //  second 8 bytes of the Key Block MAC Key

        kdid_24B[0] += 1;	// counter++

		for(i = 0 ; i < 8 ; i++)
			temp[i] = kdid_24B[i] ^ k1[i];

        api_3des_encipher(temp, &Kbm[16], &Kb[1]);  //  third 8 bytes of the Key Block MAC Key

		status = TRUE;
	}

	// clear sensitive data
	memset(Kb, 0x00, sizeof(Kb));

	return status;
}

// ---------------------------------------------------------------------------
// FUNCTION: To derive subkeys from key block protection key according to
//	         X9.143.
// INPUT   : srckey    - source key. (16/24 bytes)
//           srckeyLen - length of source key.
// OUTPUT  : k1	- subkey K1. (8 bytes)
//	         k2 - subkey K2. (8 bytes)
// RETURN  : TRUE	- success.
//           FALSE	- failure. (Key Block Protection Key is absent)
// ---------------------------------------------------------------------------
UINT32  X9143_CMAC_DeriveSubKey_TDES(UINT8 *srckey, UINT8 srckeyLen, UINT8 *k1, UINT8 *k2)
{
    UINT32  i;
    UINT32  status;
    UINT8   idata[8];
    UINT8   odata[8];
    UINT8   key_s[8];
    UINT8   key_k1[8];
    UINT8   R64[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1B};
    UINT8   carry = 0;
    UINT8   data = 0;


    status = TRUE;

    // derive S
    memset(idata, 0x00, 8);

    //PED_TripleDES(srckey, 8, idata, odata); // odata = S
    if(srckeyLen == 16) // 16-bit key
        PED_TripleDES(srckey, 8, idata, odata); // odata = S
    else    // 24-bit key
        api_3des_encipher(idata, odata, srckey);    // odata = S
    
    memmove(key_s, odata, 8);

    // derive K1
    carry = 0;
    for(i = 0 ; i < 8 ; i++) // S << 1
    {
        data = key_s[7 - i];
        key_s[7 - i] = (data << 1);
        key_s[7 - i] |= carry; // carry of the previous byte

        if(data & 0x80) // LSB for the next byte
            carry = 1;
        else
            carry = 0;
    }

    // if MSB of S = 1, then K1 = (S << 1) XOR R64
    // else 		   K1 = (S << 1)

    if(odata[0] & 0x80)
    {
        for(i = 0 ; i < 8 ; i++)
            key_s[i] ^= R64[i];
    }

    memmove(k1, key_s, 8); // K1

    memmove(odata, key_s, 8);
    memmove(key_k1, key_s, 8);

    // derive K2
    carry = 0;
    for(i = 0 ; i < 8 ; i++) // K1 << 1
    {
        data = key_k1[7 - i];
        key_k1[7 - i] = (data << 1);
        key_k1[7 - i] |= carry; // carry of the previous byte

        if (data & 0x80) // LSB for the next byte
            carry = 1;
        else
            carry = 0;
    }

    // if MSB of K1 = 1, then K2 = (K1 << 1) XOR R64
    // else 		    K2 = (K1 << 1)

    if(odata[0] & 0x80)
    {
        for(i = 0 ; i < 8 ; i++)
            key_k1[i] ^= R64[i];
    }

    memmove(k2, key_k1, 8); // K2

    return status;
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate MAC value by using X9.143 CMAC algorithm.
// INPUT   : kbm    - Key Block MAC Key used to encrypt. (16/24-byte)
//	         km1    - Subkey of the kbm. (8-byte)
//	         icv    - initial chain value. (8-byte)
//           len    - length of data.
//           data   - data to be authenticated.
// OUTPUT  : mac    - the MAC value. (8-byte)
//           icv    - next icv values.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
void    X9143_GenMAC_CMAC_TDES(UINT8 *kbm, UINT8 *km1, UINT8 *icv, UINT16 len, UINT8 *data, UINT8 *mac)
{
    UINT16  i, j, k;
    UINT16  cnt;
    UINT16  left_bytes;
    UINT8   ctext[8];
    UINT8   ptext[8];


    cnt = len / 8;
    left_bytes = len % 8;
    i = 0;

    if(cnt)
    {
        for(i = 0 ; i < cnt ; i++)
        {
            // XOR( ICV, D1 )
            for(j = 0 ; j < 8 ; j++)
                ptext[j] = icv[j] ^ data[i * 8 + j];

            /*
            // DES( ptext, key(L) )
            PED_des_encipher(ptext, ctext, kbm);

            // -DES( ctext, key(R) )
            PED_des_decipher(ptext, ctext, &kbm[8]);

            // DES( ptext, key(L) )
            PED_des_encipher(ptext, ctext, kbm);
            */
            
            api_3des_encipher2(ptext, ctext, kbm, 8);
            
            memmove(icv, ctext, 8);

            if(i == (cnt - 2))
            {
                for(k = 0 ; k < 8 ; k++)
                    icv[k] ^= km1[k];
            }
        }
    }

    memmove(mac, ctext, 8);
}

// ---------------------------------------------------------------------------
// FUNCTION: To verify the encrypted key bundle format according to ANSI X9.143.
// INPUT   : KeyBundle - encrypted key block compliant to ANSI X9.143 ASCII format.
//           Length    - size of bytes of the key bundle.
//	         KpkSrc    - source of the Key Block Protection Key. (L-V)
//			             using primitive KPK if NULLPTR.
// OUTPUT  : eKeyData  - encrypted key data in HEX format. (L-V)
//			 eKeyData(32) = TDES((Len(2) + Key(16) + Key Padding(8) + Block Padding(6)), Kbe)
//                        = TDES((Len(2) + Key(24) + Key Padding(0) + Block Padding(6)), Kbe)
//	         mac       - MAC(8)
// RETURN  : TRUE	   - valid format
//           FALSE	   - invalid format
// ---------------------------------------------------------------------------
UINT32  X9143_VerifyKeyBundle_TDES(UINT16 Length, UINT8 *KeyBundle, UINT8 *eKeyData, UINT8 *mac, UINT8 *KpkSrc)
{
    UINT16  i;
    UINT32  status;
    UINT16  len;
    UINT8   temp[80];
    UINT8   Kbm[24]; // key block MAC key
    UINT8   Kbe[24]; // key block ENC key
    UINT8   key[64];
    UINT8   icv[8];
    UINT8   K1[8];
    UINT8   K2[8];
    UINT8   Kb[PED_192_TDES_KEY_PROTECT_KEY_SLOT_LEN];
    X9143_TDES_PED_KEY_BUNDLE keyblock;
    X9143_TDES_PED_KEY_DATA keydata;
    UINT8   xkey[24];

    
    status = FALSE;

    memmove((UINT8 *)&keyblock, KeyBundle, sizeof(keyblock));

    // Make sure the length of the key block matches the contents of bytes 1-4
    memmove(temp, keyblock.BlockLen, sizeof(keyblock.BlockLen));
    temp[sizeof(keyblock.BlockLen)] = 0x00;
    len = atoi(temp);

    if((Length <= 96) && (Length == len) && (keyblock.VersionID[0] == 'B') &&
        (keyblock.OptionBlocks[0] == '0') && (keyblock.OptionBlocks[1] == '0'))
    {
        // convert ASCII to HEX for the key data (LEN KEY PADDING MAC)
        for(i = 0 ; i < (Length - 16) ; i++)
            temp[i] = LIB_ascw2hexb(&keyblock.KeyLen[i * 2]);
        memmove((UINT8 *)&keydata, temp, (Length - 16) / 2);

        // retrieve Key Block Protection Key
        if(PED_VerifyKPKStatus(TDES_128) == TRUE)
        {
            if(KpkSrc == NULLPTR)
                OS_SECM_GetData(ADDR_PED_128_TDES_KEY_PROTECT_KEY, PED_128_TDES_KEY_PROTECT_KEY_SLOT_LEN, Kb);
            else
                memmove(Kb, KpkSrc, PED_128_TDES_KEY_PROTECT_KEY_LEN + 1);

            if(Kb[0] != PED_128_TDES_KEY_PROTECT_KEY_LEN)
                return FALSE;
        }    
        else if(PED_VerifyKPKStatus(TDES_192) == TRUE)
        {
            if(KpkSrc == NULLPTR)
                OS_SECM_GetData(ADDR_PED_192_TDES_KEY_PROTECT_KEY, PED_192_TDES_KEY_PROTECT_KEY_SLOT_LEN, Kb);
            else
                memmove(Kb, KpkSrc, PED_192_TDES_KEY_PROTECT_KEY_LEN + 1);
            
            if(Kb[0] != PED_192_TDES_KEY_PROTECT_KEY_LEN)
                return FALSE;
        }

        // CMAC Subkey (K1 & K2) Derivation from the Key Block Protection Key (Kb)
        X9143_CMAC_DeriveSubKey_TDES(&Kb[1], Kb[0], K1, K2);

        // derive Key Block MAC Key (Kbm) from Key Block Protection Key (Kb)
        if(X9143_GenKeyBlockMacKey_TDES(K1, Kbm, KpkSrc))
        {
            if(X9143_GenKeyBlockEncryptKey_TDES(K1, Kbe, KpkSrc))
            {
                // decrypt the key bundle [KEY_LENGTH(2) + KEY(16) + KEY PADDING(8) + BLOCK PADDING(6)] with IV=MAC, or
                // decrypt the key bundle [KEY_LENGTH(2) + KEY(24) + KEY PADDING(0) + BLOCK PADDING(6)] with IV=MAC
                memmove(temp, &keydata.KeyLen[0], ((Length - 16) / 2) - 8);
                memmove(icv, keydata.MAC, 8);
                if(Kb[0] == PED_128_TDES_KEY_PROTECT_KEY_LEN)
                    PED_CBC_TripleDES2(Kbe, icv, ((Length - 16) / 2) - 8, temp, key);
                else if(Kb[0] == PED_192_TDES_KEY_PROTECT_KEY_LEN)
                    api_3des_cbc_decipher(temp, key, Kbe, Kb[0], ((Length - 16) / 2) - 8, icv, 8);

                // CMAC Subkey (KM1 & KM2) Derivation from the Key Block MAC Key (Kbm)
                X9143_CMAC_DeriveSubKey_TDES(Kbm, Kb[0], K1, K2); // KM1 = K1

                // verify CBC MAC for the key bundle [KBH(16) + KEY_LENGTH(2) + KEY(16) + KEY PADDING(8) + BLOCK PADDING(6)], or
                // the key bundle [KBH(16) + KEY_LENGTH(2) + KEY(24) + KEY PADDING(0) + BLOCK PADDING(6)]
                memmove(temp, (UINT8 *)&keyblock, 16);            // KBH(16)
                memmove(&temp[16], key, ((Length - 16) / 2) - 8); // KEY_LENGTH(2) + KEY(16/24) + KEY PADDING(8/0) + BLOCK PADDING(6)

                memset(icv, 0x00, 8);
                if(Kb[0] == PED_128_TDES_KEY_PROTECT_KEY_LEN)
                {
                    memmove(xkey, Kbm, 16);
                    memmove(&xkey[16], Kbm, 8);
                    X9143_GenMAC_CMAC_TDES(xkey, K1, icv, 16 + sizeof(keydata) - 8, temp, mac);
                }
                else if(Kb[0] == PED_192_TDES_KEY_PROTECT_KEY_LEN)
                    X9143_GenMAC_CMAC_TDES(Kbm, K1, icv, 16 + sizeof(keydata) - 8, temp, mac);
            }

            if(LIB_memcmp(keydata.MAC, mac, 8) == 0) // MAC OK?
            {
                eKeyData[0] = ((Length - 16) / 2) - 8;
                memmove(&eKeyData[1], &keydata.KeyLen[0], eKeyData[0]);
                status = TRUE;
            }
        }
    }

    // clear sensitive data
    memset(temp, 0x00, sizeof(temp));
    memset(Kbm, 0x00, sizeof(Kbm));
    memset(Kbe, 0x00, sizeof(Kbe));
    memset(Kb, 0x00, sizeof(Kb));
    memset(K1, 0x00, sizeof(K1));
    memset(K2, 0x00, sizeof(K2));
    memset(key, 0x00, sizeof(key));
    memset(xkey, 0x00, sizeof(xkey));
    memset((UINT8 *)&keyblock, 0x00, sizeof(keyblock));
    memset((UINT8 *)&keydata, 0x00, sizeof(keydata));

    return status;
}

// ---------------------------------------------------------------------------
// FUNCTION: To restore the plaintext Key Block Encryption Key from key bundle
//	         according to ANSI X9.143.
// INPUT   : eKeyData - encrypted key data in HEX format. (L-V)
//			 eKeyData(32) = TDES((Len(2) + Key(16) + Key Padding(8) + Block Padding(6)), Kbe)
//                        = TDES((Len(2) + Key(24) + Key Padding(0) + Block Padding(6)), Kbe)
//           icv	  - initial chain vector. (8 bytes)
//	         KpkSrc   - source of the Key Block Protection Key. (L-V)
//			            using primitive KPK if NULLPTR.
// OUTPUT  : key	  - target plaintext key. (L-V)
// RETURN  : TRUE	  - valid format
//           FALSE	  - invalid format
// ---------------------------------------------------------------------------
UINT32  X9143_DecryptKeyBundle_TDES(UINT8 *icv, UINT8 *eKeyData, UINT8 *key, UINT8 *KpkSrc)
{
    UINT32  status;
    UINT8   Kbe[24]; // key block encryption key
    UINT8   keydata[32];
    UINT8   iv[8];
    UINT8   K1[8];
    UINT8   K2[8];
    UINT8   Kb[PED_192_TDES_KEY_PROTECT_KEY_SLOT_LEN];
    UINT16  len;


    status = FALSE;

    // retrieve Key Block Protection Key
    if(PED_VerifyKPKStatus(TDES_128) == TRUE)
    {
        if(KpkSrc == NULLPTR)
            OS_SECM_GetData(ADDR_PED_128_TDES_KEY_PROTECT_KEY, PED_128_TDES_KEY_PROTECT_KEY_SLOT_LEN, Kb);
        else
            memmove(Kb, KpkSrc, PED_128_TDES_KEY_PROTECT_KEY_LEN + 1);

        if(Kb[0] != PED_128_TDES_KEY_PROTECT_KEY_LEN)
            return FALSE;
    }    
    else if(PED_VerifyKPKStatus(TDES_192) == TRUE)
    {
        if(KpkSrc == NULLPTR)
            OS_SECM_GetData(ADDR_PED_192_TDES_KEY_PROTECT_KEY, PED_192_TDES_KEY_PROTECT_KEY_SLOT_LEN, Kb);
        else
            memmove(Kb, KpkSrc, PED_192_TDES_KEY_PROTECT_KEY_LEN + 1);

        if(Kb[0] != PED_192_TDES_KEY_PROTECT_KEY_LEN)
            return FALSE;
    }

    // CMAC Subkey (K1 & K2) Derivation from the Key Block Protection Key (Kb)
    X9143_CMAC_DeriveSubKey_TDES(&Kb[1], Kb[0], K1, K2);

    memmove(iv, icv, 8);

    if(X9143_GenKeyBlockEncryptKey_TDES(K1, Kbe, KpkSrc))
    {
        if(Kb[0] == PED_128_TDES_KEY_PROTECT_KEY_LEN)
            PED_CBC_TripleDES2(Kbe, iv, eKeyData[0], &eKeyData[1], keydata);
        else if(Kb[0] == PED_192_TDES_KEY_PROTECT_KEY_LEN)
            api_3des_cbc_decipher(&eKeyData[1], keydata, Kbe, Kb[0], eKeyData[0], iv, 8);

        len = (keydata[0] * 256 + keydata[1]) / 8; // key length in bytes
        if(((len == 16) || (len == 24)) && (Kb[0] >= len))
        {
            key[0] = len;
            memmove(&key[1], &keydata[2], len);

            status = TRUE;
        }
    }

    // clear sensitive data
    memset(Kbe, 0x00, sizeof(Kbe));
    memset(keydata, 0x00, sizeof(keydata));
    memset(Kb, 0x00, sizeof(Kb));
    memset(K1, 0x00, sizeof(K1));
    memset(K2, 0x00, sizeof(K2));

    return status;
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate Key Block Encryption Key,
//	         derived from Key Block Protection Key (Kb) according to ANSI X9.143.
// INPUT   : k2  - CMAC subkey. (16 bytes)
//	         KpkSrc - source of the Key Block Protection Key. (L-V)
//			          using primitive KPK if NULLPTR.
// OUTPUT  : Kbe - Key Block Encryption Key. (16/24/32 bytes)
// RETURN  : TRUE	- success.
//           FALSE	- failure. (Key Block Protection Key is absent)
// ---------------------------------------------------------------------------
UINT32	X9143_GenKeyBlockEncryptKey_AES(UINT8 *k2, UINT8 *Kbe, UINT8 *KpkSrc)
{
    UINT32	i;
    UINT32	status;
    UINT8	Kb[PED_256_AES_KEY_PROTECT_KEY_SLOT_LEN];
    UINT8	kdid_16B[16] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x80,
                             0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };	// key derivation input data
    UINT8	kdid_24B[16] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0xC0,
                             0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };	// key derivation input data
    UINT8	kdid_32B[16] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x04, 0x01, 0x00,
                             0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };	// key derivation input data
    UINT8	temp[16];


    status = FALSE;

    // retrieve Key Block Protection Key for AES
    if(PED_VerifyKPKStatus(AES_128) == TRUE)
    {
        if(KpkSrc == NULLPTR)
            OS_SECM_GetData(ADDR_PED_128_AES_KEY_PROTECT_KEY, PED_128_AES_KEY_PROTECT_KEY_SLOT_LEN, Kb);
        else
            memmove(Kb, KpkSrc, PED_128_AES_KEY_PROTECT_KEY_LEN + 1);
    }    
    else if(PED_VerifyKPKStatus(AES_192) == TRUE)
    {
        if(KpkSrc == NULLPTR)
            OS_SECM_GetData(ADDR_PED_192_AES_KEY_PROTECT_KEY, PED_192_AES_KEY_PROTECT_KEY_SLOT_LEN, Kb);
        else
            memmove(Kb, KpkSrc, PED_192_AES_KEY_PROTECT_KEY_LEN + 1);
    }
    else if(PED_VerifyKPKStatus(AES_256) == TRUE)
    {
        if(KpkSrc == NULLPTR)
            OS_SECM_GetData(ADDR_PED_256_AES_KEY_PROTECT_KEY, PED_256_AES_KEY_PROTECT_KEY_SLOT_LEN, Kb);
        else
            memmove(Kb, KpkSrc, PED_256_AES_KEY_PROTECT_KEY_LEN + 1);
    }

    if(Kb[0] == PED_128_AES_KEY_PROTECT_KEY_LEN)
    {
        for(i = 0 ; i < 16 ; i++)
            temp[i] = kdid_16B[i] ^ k2[i];

        api_aes_encipher(temp, Kbe, &Kb[1], 16); // first 16 bytes of the Key Block Encryption Key

        status = TRUE;
    }
    else if(Kb[0] == PED_192_AES_KEY_PROTECT_KEY_LEN)
    {
        for(i = 0 ; i < 16 ; i++)
            temp[i] = kdid_24B[i] ^ k2[i];

        api_aes_encipher(temp, Kbe, &Kb[1], 24); // first 16 bytes of the Key Block Encryption Key

        kdid_24B[0] += 1; // counter++

        for(i = 0 ; i < 16 ; i++)
            temp[i] = kdid_24B[i] ^ k2[i];

        api_aes_encipher(temp, &Kbe[16], &Kb[1], 24); // second 16 bytes of the Key Block Encryption Key

        status = TRUE;
    }
    else if(Kb[0] == PED_256_AES_KEY_PROTECT_KEY_LEN)
    {
        for(i = 0 ; i < 16 ; i++)
            temp[i] = kdid_32B[i] ^ k2[i];

        api_aes_encipher(temp, Kbe, &Kb[1], 32); // first 16 bytes of the Key Block Encryption Key

        kdid_32B[0] += 1; // counter++

        for(i = 0 ; i < 16 ; i++)
            temp[i] = kdid_32B[i] ^ k2[i];

        api_aes_encipher(temp, &Kbe[16], &Kb[1], 32); // second 16 bytes of the Key Block Encryption Key

        status = TRUE;
    }

    // clear sensitive data
    memset(Kb, 0x00, sizeof(Kb));

    return status;
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate Key Block MAC Key,
//	         derived from Key Block Protection Key (Kb) according to ANSI X.9143.
// INPUT   : k2  - CMAC subkey. (16 bytes)
//	         KpkSrc - source of the Key Block Protection Key. (L-V)
//			          using primitive KPK if NULLPTR.
// OUTPUT  : Kbm - Key Block MAC Key. (16/24/32 bytes)
// RETURN  : TRUE	- success.
//           FALSE	- failure. (Key Block Protection Key is absent)
// ---------------------------------------------------------------------------
UINT32	X9143_GenKeyBlockMacKey_AES(UINT8 *k2, UINT8 *Kbm, UINT8 *KpkSrc)
{
    UINT32	i;
    UINT32	status;
    UINT8	Kb[PED_256_AES_KEY_PROTECT_KEY_SLOT_LEN];
    UINT8	kdid_16B[16] = { 0x01, 0x00, 0x01, 0x00, 0x00, 0x02, 0x00, 0x80,
                             0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };	// key derivation input data
    UINT8	kdid_24B[16] = { 0x01, 0x00, 0x01, 0x00, 0x00, 0x03, 0x00, 0xC0,
                             0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };	// key derivation input data
    UINT8	kdid_32B[16] = { 0x01, 0x00, 0x01, 0x00, 0x00, 0x04, 0x01, 0x00,
                             0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };	// key derivation input data
    UINT8	temp[16];


    status = FALSE;

    // retrieve Key Block Protection Key for AES
    if(PED_VerifyKPKStatus(AES_128) == TRUE)
    {
        if(KpkSrc == NULLPTR)
            OS_SECM_GetData(ADDR_PED_128_AES_KEY_PROTECT_KEY, PED_128_AES_KEY_PROTECT_KEY_SLOT_LEN, Kb);
        else
            memmove(Kb, KpkSrc, PED_128_AES_KEY_PROTECT_KEY_LEN + 1);
    }    
    else if(PED_VerifyKPKStatus(AES_192) == TRUE)
    {
        if(KpkSrc == NULLPTR)
            OS_SECM_GetData(ADDR_PED_192_AES_KEY_PROTECT_KEY, PED_192_AES_KEY_PROTECT_KEY_SLOT_LEN, Kb);
        else
            memmove(Kb, KpkSrc, PED_192_AES_KEY_PROTECT_KEY_LEN + 1);
    }
    else if(PED_VerifyKPKStatus(AES_256) == TRUE)
    {
        if(KpkSrc == NULLPTR)
            OS_SECM_GetData(ADDR_PED_256_AES_KEY_PROTECT_KEY, PED_256_AES_KEY_PROTECT_KEY_SLOT_LEN, Kb);
        else
            memmove(Kb, KpkSrc, PED_256_AES_KEY_PROTECT_KEY_LEN + 1);
    }

    if(Kb[0] == PED_128_AES_KEY_PROTECT_KEY_LEN)
    {
        for(i = 0 ; i < 16 ; i++)
            temp[i] = kdid_16B[i] ^ k2[i];

        api_aes_encipher(temp, Kbm, &Kb[1], 16); // first 16 bytes of the Key Block Encryption Key

        status = TRUE;
    }
    else if(Kb[0] == PED_192_AES_KEY_PROTECT_KEY_LEN)
    {
        for(i = 0 ; i < 16 ; i++)
            temp[i] = kdid_24B[i] ^ k2[i];

        api_aes_encipher(temp, Kbm, &Kb[1], 24); // first 16 bytes of the Key Block Encryption Key

        kdid_24B[0] += 1; // counter++

        for(i = 0 ; i < 16 ; i++)
            temp[i] = kdid_24B[i] ^ k2[i];

        api_aes_encipher(temp, &Kbm[16], &Kb[1], 24); // second 16 bytes of the Key Block Encryption Key

        status = TRUE;
    }
    else if(Kb[0] == PED_256_AES_KEY_PROTECT_KEY_LEN)
    {
        for(i = 0 ; i < 16 ; i++)
            temp[i] = kdid_32B[i] ^ k2[i];

        api_aes_encipher(temp, Kbm, &Kb[1], 32); // first 16 bytes of the Key Block Encryption Key

        kdid_32B[0] += 1; // counter++

        for(i = 0 ; i < 16 ; i++)
            temp[i] = kdid_32B[i] ^ k2[i];

        api_aes_encipher(temp, &Kbm[16], &Kb[1], 32); // second 16 bytes of the Key Block Encryption Key

        status = TRUE;
    }

    // clear sensitive data
    memset(Kb, 0x00, sizeof(Kb));

    return status;
}

// ---------------------------------------------------------------------------
// FUNCTION: To derive subkeys from key block protection key according to
//	         X9.143.
// INPUT   : srckey    - source key. (16/24/32 bytes)
//           srckeyLen - length of source key.
// OUTPUT  : k1	- subkey K1. (16 bytes)
//	         k2 - subkey K2. (16 bytes)
// RETURN  : TRUE	- success.
//           FALSE	- failure. (Key Block Protection Key is absent)
// ---------------------------------------------------------------------------
UINT32	X9143_CMAC_DeriveSubKey_AES(UINT8 *srckey, UINT8 srckeyLen, UINT8 *k1, UINT8 *k2)
{
    UINT32	i;
    UINT32	status;
    UINT8	idata[16];
    UINT8	odata[16];
    UINT8	key_s[16];
    UINT8	key_k1[16];
    UINT8	R128[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87 };
    UINT8	carry = 0;
    UINT8	data = 0;


    status = TRUE;

    // derive S
    memset(idata, 0x00, 16);
    api_aes_encipher(idata, odata, srckey, srckeyLen); // odata = S(16)
    memmove(key_s, odata, 16);

    // derive K1
    carry = 0;
    for(i = 0 ; i < 16 ; i++) // S << 1
    {
        data = key_s[15 - i];
        key_s[15 - i] = (data << 1);
        key_s[15 - i] |= carry; // carry of the previous byte

        if(data & 0x80) // LSB for the next byte
            carry = 1;
        else
            carry = 0;
    }

    // if MSB of S = 1, then K1 = (S << 1) XOR R128
    // else 		 K1 = (S << 1)

    if(odata[0] & 0x80)
    {
        for(i = 0 ; i < 16 ; i++)
            key_s[i] ^= R128[i];
    }

    memmove(k1, key_s, 16); // K1(16)

    memmove(odata, key_s, 16);
    memmove(key_k1, key_s, 16);

    // derive K2
    carry = 0;
    for(i = 0 ; i < 16 ; i++) // K1 << 1
    {
        data = key_k1[15 - i];
        key_k1[15 - i] = (data << 1);
        key_k1[15 - i] |= carry; // carry of the previous byte

        if(data & 0x80) // LSB for the next byte
            carry = 1;
        else
            carry = 0;
    }

    // if MSB of K1 = 1, then K2 = (K1 << 1) XOR R128
    // else 		  K2 = (K1 << 1)

    if(odata[0] & 0x80)
    {
        for(i = 0 ; i < 16 ; i++)
            key_k1[i] ^= R128[i];
    }

    memmove(k2, key_k1, 16); // K2(16)

    return status;
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate MAC value by using X9.143 CMAC algorithm.
// INPUT   : kbm    - Key Block MAC Key used to encrypt. (16/24/32-byte)
//           keyLen - length of Key Block MAC Key.
//	         km1    - Subkey of the kbm. (16-byte)
//	         icv    - initial chain value. (16-byte)
//           len    - length of data.
//           data   - data to be authenticated.
// OUTPUT  : mac    - the MAC value. (16-byte)
//           icv    - next icv values.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
void    X9143_GenMAC_CMAC_AES(UINT8 *kbm, UINT8 keyLen, UINT8 *km1, UINT8 *icv, UINT16 len, UINT8 *data, UINT8 *mac)
{
    UINT16	i, j, k;
    UINT16	cnt;
    UINT16	left_bytes;
    UINT8	ctext[16];
    UINT8	ptext[16];


    cnt = len / 16;
    left_bytes = len % 16;
    i = 0;

    if(cnt)
    {
        for(i = 0 ; i < cnt ; i++)
        {
            // XOR( ICV, D1 )
            for(j = 0 ; j < 16 ; j++)
                ptext[j] = icv[j] ^ data[i * 16 + j];

            // ECB-ENC (AES)
            api_aes_encipher(ptext, ctext, kbm, keyLen);

            memmove(icv, ctext, 16);

            if(i == (cnt - 2))
            {
                for(k = 0 ; k < 16 ; k++)
                    icv[k] ^= km1[k];
            }
        }
    }

    memmove(mac, ctext, 16);
}

// ---------------------------------------------------------------------------
// FUNCTION: To verify the encrypted key bundle format according to ANSI X9.143.
// INPUT   : KeyBundle - encrypted key block compliant to ANSI X9.143 ASCII format.
//           Length    - size of bytes of the key bundle.
//	         KpkSrc    - source of the Key Block Protection Key. (L-V)
//			             using primitive KPK if NULLPTR.
// OUTPUT  : eKeyData  - encrypted key data in HEX format. (L-V)
//			 eKeyData(48) = AES((Len(2) + Key(16) + Key Padding(16) + Block Padding(14)), Kbe)
//                        = AES((Len(2) + Key(24) + Key Padding(8) + Block Padding(14)), Kbe)
//                        = AES((Len(2) + Key(32) + Key Padding(0) + Block Padding(14)), Kbe)
//                        = AES((Len(2) + IKSN(12) + Key Padding(20) + Block Padding(14)), Kbe)
//	         mac       - MAC(16)
// RETURN  : TRUE	   - valid format
//           FALSE	   - invalid format
// ---------------------------------------------------------------------------
UINT32  X9143_VerifyKeyBundle_AES(UINT16 Length, UINT8 *KeyBundle, UINT8 *eKeyData, UINT8 *mac, UINT8 *KpkSrc)
{
    UINT16	i;
    UINT32	status;
    UINT16	len;
    UINT8	temp[128];
    UINT8	Kbm[32];	// key block MAC key
    UINT8	Kbe[32];	// key block ENC key
    UINT8	key[48];
    UINT8	icv[16];
    UINT8	K1[16];
    UINT8	K2[16];
    UINT8	Kb[PED_256_AES_KEY_PROTECT_KEY_SLOT_LEN];
    X9143_AES_PED_KEY_BUNDLE    keyblock;
    X9143_AES_PED_KEY_DATA	    keydata;


    status = FALSE;

    memmove((UINT8 *)&keyblock, KeyBundle, sizeof(keyblock));

    // Make sure the length of the key block matches the contents of bytes 1-4
    memmove(temp, keyblock.BlockLen, sizeof(keyblock.BlockLen));
    temp[sizeof(keyblock.BlockLen)] = 0x00;
    len = atoi(temp);

    if((Length <= 144) && (Length == len) && (keyblock.VersionID[0] == 'D') &&
        (keyblock.OptionBlocks[0] == '0') && (keyblock.OptionBlocks[1] == '0'))
    {
        // convert ASCII to HEX for the key data (LEN KEY PADDING MAC)
        for(i = 0 ; i < (Length - 16) ; i++)
            temp[i] = LIB_ascw2hexb(&keyblock.KeyLen[i * 2]);
        memmove((UINT8 *)&keydata, temp, (Length - 16) / 2);

        // retrieve Key Block Protection Key
        if(PED_VerifyKPKStatus(AES_128) == TRUE)
        {
            if(KpkSrc == NULLPTR)
                OS_SECM_GetData(ADDR_PED_128_AES_KEY_PROTECT_KEY, PED_128_AES_KEY_PROTECT_KEY_SLOT_LEN, Kb);
            else
                memmove(Kb, KpkSrc, PED_128_AES_KEY_PROTECT_KEY_LEN + 1);

            if(Kb[0] != PED_128_AES_KEY_PROTECT_KEY_LEN)
                return FALSE;
        }    
        else if(PED_VerifyKPKStatus(AES_192) == TRUE)
        {
            if(KpkSrc == NULLPTR)
                OS_SECM_GetData(ADDR_PED_192_AES_KEY_PROTECT_KEY, PED_192_AES_KEY_PROTECT_KEY_SLOT_LEN, Kb);
            else
                memmove(Kb, KpkSrc, PED_192_AES_KEY_PROTECT_KEY_LEN + 1);

            if(Kb[0] != PED_192_AES_KEY_PROTECT_KEY_LEN)
                return FALSE;
        }
        else if(PED_VerifyKPKStatus(AES_256) == TRUE)
        {
            if(KpkSrc == NULLPTR)
                OS_SECM_GetData(ADDR_PED_256_AES_KEY_PROTECT_KEY, PED_256_AES_KEY_PROTECT_KEY_SLOT_LEN, Kb);
            else
                memmove(Kb, KpkSrc, PED_256_AES_KEY_PROTECT_KEY_LEN + 1);

            if(Kb[0] != PED_256_AES_KEY_PROTECT_KEY_LEN)
                return FALSE;
        }

        // CMAC Subkey (K1 & K2) Derivation from the Key Block Protection Key (Kb)
        X9143_CMAC_DeriveSubKey_AES(&Kb[1], Kb[0], K1, K2);

        // derive Key Block MAC Key (Kbm) from Key Block Protection Key (Kb)
        if(X9143_GenKeyBlockMacKey_AES(K2, Kbm, KpkSrc))
        {
            if(X9143_GenKeyBlockEncryptKey_AES(K2, Kbe, KpkSrc))
            {
                // decrypt the key bundle [KEY_LENGTH(2) + KEY(16) + KEY PADDING(16) + BLOCK PADDING(14)] with IV=MAC,
                // decrypt the key bundle [KEY_LENGTH(2) + KEY(24) + KEY PADDING(8) + BLOCK PADDING(14)] with IV=MAC, or
                // decrypt the key bundle [KEY_LENGTH(2) + KEY(32) + KEY PADDING(0) + BLOCK PADDING(14)] with IV=MAC
                memmove(temp, &keydata.KeyLen[0], ((Length - 16) / 2) - 16);
                memmove(icv, keydata.MAC, 16);
                PED_CBC_AES2(Kbe, Kb[0], icv, ((Length - 16) / 2) - 16, temp, key);

                // CMAC Subkey (KM1 & KM2) Derivation from the Key Block MAC Key (Kbm)
                X9143_CMAC_DeriveSubKey_AES(Kbm, Kb[0], K1, K2); // KM1 = K1

                // verify CBC MAC for the key bundle [KBH(16) + KEY_LENGTH(2) + KEY(16) + KEY PADDING(16) + BLOCK PADDING(14)],
                // the key bundle [KBH(16) + KEY_LENGTH(2) + KEY(24) + KEY PADDING(8) + BLOCK PADDING(14)], or
                // the key bundle [KBH(16) + KEY_LENGTH(2) + KEY(32) + KEY PADDING(0) + BLOCK PADDING(14)]
                memmove(temp, (UINT8 *)&keyblock, 16);             // KBH(16)
                memmove(&temp[16], key, ((Length - 16) / 2) - 16); // KEY_LENGTH(2) + KEY(16/24/32) + KEY PADDING(16/8/0) + BLOCK PADDING(14)

                memset(icv, 0x00, 16);

                X9143_GenMAC_CMAC_AES(Kbm, Kb[0], K1, icv, 16 + sizeof(keydata) - 16, temp, mac);
            }

            if(LIB_memcmp(keydata.MAC, mac, 16) == 0) // MAC OK?
            {
                eKeyData[0] = ((Length - 16) / 2) - 16;
                memmove(&eKeyData[1], &keydata.KeyLen[0], eKeyData[0]);
                status = TRUE;
            }
        }
    }

    // clear sensitive data
    memset(temp, 0x00, sizeof(temp));
    memset(Kbm, 0x00, sizeof(Kbm));
    memset(Kbe, 0x00, sizeof(Kbe));
    memset(Kb, 0x00, sizeof(Kb));
    memset(K1, 0x00, sizeof(K1));
    memset(K2, 0x00, sizeof(K2));
    memset(key, 0x00, sizeof(key));
    memset((UINT8 *)&keyblock, 0x00, sizeof(keyblock));
    memset((UINT8 *)&keydata, 0x00, sizeof(keydata));

    return status;
}

// ---------------------------------------------------------------------------
// FUNCTION: To restore the plaintext Key Block Encryption Key from key bundle
//	         according to ANSI X9.143.
// INPUT   : eKeyData	- encrypted key data in HEX format. (L-V)
//			 eKeyData(48) = AES((Len(2) + Key(16) + Key Padding(16) + Block Padding(14)), Kbe)
//                        = AES((Len(2) + Key(24) + Key Padding(8) + Block Padding(14)), Kbe)
//                        = AES((Len(2) + Key(32) + Key Padding(0) + Block Padding(14)), Kbe)
//                        = AES((Len(2) + IKSN(12) + Key Padding(20) + Block Padding(14)), Kbe)
//           icv	- initial chain vector. (16 bytes)
//	         KpkSrc - source of the Key Block Protection Key. (L-V)
//			          using primitive KPK if NULLPTR.
// OUTPUT  : key	- target plaintext key. (L-V)
// RETURN  : TRUE	- valid format
//           FALSE	- invalid format
// ---------------------------------------------------------------------------
UINT32	X9143_DecryptKeyBundle_AES(UINT8 *icv, UINT8 *eKeyData, UINT8 *key, UINT8 *KpkSrc)
{
    UINT32	status;
    UINT8	Kbe[32];	// key block encryption key
    UINT8	keydata[48];
    UINT8	iv[16];
    UINT8	K1[16];
    UINT8	K2[16];
    UINT8	Kb[PED_256_AES_KEY_PROTECT_KEY_SLOT_LEN];
    UINT16	len;


    status = FALSE;

    // retrieve Key Block Protection Key
    if(PED_VerifyKPKStatus(AES_128) == TRUE)
    {
        if(KpkSrc == NULLPTR)
            OS_SECM_GetData(ADDR_PED_128_AES_KEY_PROTECT_KEY, PED_128_AES_KEY_PROTECT_KEY_SLOT_LEN, Kb);
        else
            memmove(Kb, KpkSrc, PED_128_AES_KEY_PROTECT_KEY_LEN + 1);

        if(Kb[0] != PED_128_AES_KEY_PROTECT_KEY_LEN)
            return FALSE;
    }    
    else if(PED_VerifyKPKStatus(AES_192) == TRUE)
    {
        if(KpkSrc == NULLPTR)
            OS_SECM_GetData(ADDR_PED_192_AES_KEY_PROTECT_KEY, PED_192_AES_KEY_PROTECT_KEY_SLOT_LEN, Kb);
        else
            memmove(Kb, KpkSrc, PED_192_AES_KEY_PROTECT_KEY_LEN + 1);

        if(Kb[0] != PED_192_AES_KEY_PROTECT_KEY_LEN)
            return FALSE;
    }
    else if(PED_VerifyKPKStatus(AES_256) == TRUE)
    {
        if(KpkSrc == NULLPTR)
            OS_SECM_GetData(ADDR_PED_256_AES_KEY_PROTECT_KEY, PED_256_AES_KEY_PROTECT_KEY_SLOT_LEN, Kb);
        else
            memmove(Kb, KpkSrc, PED_256_AES_KEY_PROTECT_KEY_LEN + 1);

        if(Kb[0] != PED_256_AES_KEY_PROTECT_KEY_LEN)
            return FALSE;
    }

    // CMAC Subkey (K1 & K2) Derivation from the Key Block Protection Key (Kb)
    X9143_CMAC_DeriveSubKey_AES(&Kb[1], Kb[0], K1, K2);

    memmove(iv, icv, 16);

    if(X9143_GenKeyBlockEncryptKey_AES(K2, Kbe, KpkSrc))
    {
        PED_CBC_AES2(Kbe, Kb[0], iv, eKeyData[0], &eKeyData[1], keydata);

        len = (keydata[0] * 256 + keydata[1]) / 8; // key length in bytes
        //len may be 12 bytes if AES DUKPT IKSN store in keydata array
        if (((len == 12) || (len == 16) || (len == 24) || (len == 32)) && (Kb[0] >= len))
        {
            key[0] = len;
            memmove(&key[1], &keydata[2], len);

            status = TRUE;
        }
    }

    // clear sensitive data
    memset(Kbe, 0x00, sizeof(Kbe));
    memset(keydata, 0x00, sizeof(keydata));
    memset(Kb, 0x00, sizeof(Kb));
    memset(K1, 0x00, sizeof(K1));
    memset(K2, 0x00, sizeof(K2));

    return status;
}
