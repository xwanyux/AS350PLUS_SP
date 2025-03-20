//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : i.MX6UL							                        **
//**  PRODUCT  : AS350-X6							                        **
//**                                                                        **
//**  FILE     : OS_PED1.C						                            **
//**  MODULE   : 			   				                                **
//**                                                                        **
//**  FUNCTION : OS::PED1 (PCI PED Secure Function Module)		            **
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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>

//#include <ucl/ucl_defs.h>
//#include <ucl/ucl_types.h>
//#include <ucl/ucl_3des_ecb.h>

#include <ucl/ucl_config.h>
#include <ucl/ucl_defs.h>
#include <ucl/ucl_retdefs.h>
#include <ucl/ucl_types.h>
#include <ucl/ucl_sys.h>
#include <ucl/ucl_stack.h>
#include <ucl/ucl_sha1.h>
#include <ucl/ucl_sha256.h>
#include <ucl/ucl_info.h>

#include <ucl/ucl_des.h>
#include <ucl/ucl_des_ecb.h>
#include <ucl/ucl_des_cbc.h>
#include <ucl/ucl_des_cbc_mac.h>
#include <ucl/ucl_des_ofb.h>
#include <ucl/ucl_des_cfb.h>
#include <ucl/ucl_3des_ecb.h>
#include <ucl/ucl_data_conv.h>

#include "PEDKconfig.h"

#include "POSAPI.h"
#include "OS_MSG.h"
#include "OS_LIB.h"
#include "OS_PED.h"
#include "OS_SECM.h"
#include "OS_FLASH.h"
//#include "OS_CAPK.h"
#include "ANS_TR31_2010.h"
#include "ANS_TR31_2018.h"
#include "ANS_X9143_2022.h"
//#include "SRED.h"

#include "MPU.h"

extern	UINT32	TR31_CMAC_DeriveSubKey_AES(UINT8 *srckey, UINT8 *k1, UINT8 *k2);
extern  UINT32	X9143_CMAC_DeriveSubKey_AES(UINT8 *srckey, UINT8 srckeyLen, UINT8 *k1, UINT8 *k2);
extern  void    SecretInfo_Sign(FILE *prvKeyFilePtr, UINT8 *data, UINT16 dataLen, UINT8 *signature, UINT16 *signatureLen);
extern  void    SecretInfo_Verify(FILE *pubKeyFilePtr, UINT8 *data, UINT16 dataLen, UINT8 *signature, UINT16 signatureLen, UINT8 *isAuthenticated);
extern  void    SecretInfo_writeAuditTrail(UINT8 authStatus);


// ---------------------------------------------------------------------------
// FUNCTION: Download Initial Key Encryption Key (IKEK).
// INPUT   : tid  - terminal id. (8 bytes)
// OUTPUT  : none.
// RETURN  : apiOK
//	     apiFailed
// ---------------------------------------------------------------------------
#if 0
UINT8	PED_LoadInitialKeyEncryptionKey(UINT8 *tid)
{
	UINT8	kek[16];
	UINT8	temp[32];
	UINT8	buf1[32];
	UINT8	buf2[40];
	UINT8	key[16];

	UINT8	rnd_b[8];
	UINT8	rnd_r[8];
	UINT8	ernd[8];
	UINT8	result;


	// Terminal Serial Number (TSN)
	api_sys_info(SID_TerminalSerialNumber, temp);
	memmove(buf2, &temp[4], 8);

	// Terminal ID (TID)
	memmove(&buf2[8], tid, 8);

	// Random Number (RND_B)
	api_sys_random(rnd_b);
	memmove(&buf2[16], rnd_b, 8);

	memmove(&buf1[3], buf2, 24);
	buf1[0] = 25;
	buf1[1] = 0;
	buf1[2] = KMC_DOWNLOAD_IKEK;	// command

	// init communication port
	if(LIB_OpenAUX(COM0, auxDLL, COM_9600) == FALSE)
	{
		// Clear sensitive data
		return(apiFailed); // device error
	}

	result = apiFailed;

	// send CMD + TSN(8) + TID(8) + RND_B(8) to HOST for download key
	if(LIB_TransmitAUX(buf1) == TRUE)
	{
		if(LIB_ReceiveAUX(buf2) == TRUE)
		{
			if((buf2[0] == 9) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
			{
				// recover, check, and save RND_R(8)
				memmove(rnd_r, &buf2[3], 8);

				buf1[0] = PED_KEK_LEN;
				memcpy(&buf1[1], &rnd_r[4], 4);	//RND_R[5:8]
				memcpy(&buf1[5], &rnd_b[0], 4);	//RND_B[1:4]
				memcpy(&buf1[9], &rnd_r[0], 4);	//RND_R[1:4]
				memcpy(&buf1[13], &rnd_b[4], 4);	//RND_B[5:8]
				OS_SECM_PutData(ADDR_PED_KEK, PED_KEK_SLOT_LEN, buf1);
				memmove(kek, &buf1[1], PED_KEK_LEN);

				result = apiOK;
			}
		}
	}
	
	// Download Initial Key Encryption Key with KCV
	if(result == apiOK)
	{
		// Terminal Serial Number (TSN)
		api_sys_info(SID_TerminalSerialNumber, temp);
		memmove(&buf1[3], &temp[4], 8);

		// Terminal ID (TID)
		memmove(&buf1[3 + 8], tid, 8);

		buf1[0] = 17;
		buf1[1] = 0;
		buf1[2] = KMC_DOWNLOAD_IKEK | KMC_NEXT; // command

		result = apiFailed;

		if(LIB_TransmitAUX(buf1) == TRUE)
		{
			if(LIB_ReceiveAUX(buf2) == TRUE)
			{				
				if((buf2[0] == 20) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
				{
					// Verify encrypted Initial Key Encryption Key (EIKEK): LEN(1) + EIKEK(16) + KCV(3)
					memmove(temp, &buf2[3], PED_KEK_LEN);

					PED_TripleDES2(kek, PED_KEK_LEN, temp, buf1);	// buf1 = IKEK = ~TDES(EIKEK, KEK)

					memset(ernd, 0x00, 8);
					PED_TripleDES(buf1, 8, ernd, temp);		// temp = KCV = TDES(NULL, IKEK)

					if(LIB_memcmp(temp, &buf2[3 + 16], 3) == 0)	// KCV OK?
					{
						// Save IKEK : LEN(1) + IKEK(16)
						buf2[0] = PED_KEK_LEN;			// LEN
						memmove(&buf2[1], buf1, PED_KEK_LEN);	// IKEK
						OS_SECM_PutData(ADDR_PED_IKEK, PED_KEK_SLOT_LEN, buf2);

						result = apiOK;	// done
					}
				}
			}
		}
	}

	LIB_CloseAUX();

	// clear sensitive data
	memset(kek, 0x00, sizeof(kek));
	memset(temp, 0x00, sizeof(temp));
	memset(buf1, 0x00, sizeof(buf1));
	memset(buf2, 0x00, sizeof(buf2));
	memset(key, 0x00, sizeof(key));
	memset(rnd_b, 0x00, sizeof(rnd_b));
	memset(rnd_r, 0x00, sizeof(rnd_r));

	return(result);
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To verify ISO4_KEY for ISO format 4 app by KCV.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//	         FALSE
// ---------------------------------------------------------------------------
UINT8	PED_VerifyISO4KEY(void)
{
	PED_KEY_BUNDLE2	KeyBundle;
	UINT8	keydata[KEY_DATA_LEN2 + 1]; // L-V
	UINT8	mac16[16];
	UINT8	result = FALSE;


	// get ISO4_KEY ESKEY bundle
	OS_SECM_GetData(ADDR_PED_ISO4_KEY, PED_ISO4_KEY_SLOT_LEN, (UINT8 *)&KeyBundle);

	// verify ISO4_KEY ESKEY bundle
	if(TR31_VerifyKeyBundle_AES(PED_ISO4_KEY_SLOT_LEN, (UINT8 *)&KeyBundle, keydata, mac16))
		result = TRUE;

	// clear sensitive data
	memset(keydata, 0x00, sizeof(keydata));

	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: To verify ACC_DEK.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//	         FALSE
// ---------------------------------------------------------------------------
UINT32	PED_VerifyAccDEK(void)
{
	PED_KEY_BUNDLE	KeyBundle;
	UINT8	keydata[KEY_DATA_LEN + 1]; // L-V
	UINT8	mac8[8];
	UINT8	result = FALSE;

	UINT8	MkeyIndex;
	UINT8	temp[KEY_DATA_LEN + 1];
	UINT8	buf[KEY_BUNDLE_LEN];
	UINT8	mkey[PED_ACC_DEK_MSKEY_LEN + 1];
	

	// get ACC_DEK MKEY bundle
	PED_AccDEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
	OS_SECM_GetData(ADDR_PED_ACC_DEK_MKEY_01 + (MkeyIndex * PED_ACC_DEK_MSKEY_SLOT_LEN), PED_ACC_DEK_MSKEY_SLOT_LEN, buf);
	
	// verify ACC_DEK MKEY bundle
	if( TR31_VerifyKeyBundle(KEY_BUNDLE_LEN, buf, temp, mac8, (UINT8 *)0 ) )
	  {
	  // retrieve ACC_DEK MKEY
	  if( !TR31_DecryptKeyBundle(mac8, temp, mkey, (UINT8 *)0 ) )	// mkey=ACC_DEK MKEY (as the KBPK for SKEY)
	    return( FALSE );
	  }

	// get ACC_DEK ESKEY bundle
	OS_SECM_GetData(ADDR_PED_ACC_DEK_SKEY_01, PED_ACC_DEK_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle);

	// verify ACC_DEK ESKEY bundle
	if(TR31_VerifyKeyBundle(PED_ACC_DEK_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle, keydata, mac8, mkey))
		result = TRUE;

	// clear sensitive data
	memset(keydata, 0x00, sizeof(keydata));
	
	memset(temp, 0x00, sizeof(temp));
	memset(buf, 0x00, sizeof(buf));
	memset(mkey, 0x00, sizeof(mkey));

	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: To verify FPE Key.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//	         FALSE
// ---------------------------------------------------------------------------
UINT8	PED_VerifyFPEKey(void)
{
	PED_KEY_BUNDLE2	KeyBundle;
	UINT8	keydata[KEY_DATA_LEN2 + 1]; // L-V
	UINT8	mac16[16];
	UINT8	result = FALSE;


	// get FPE Key ESKEY bundle
	OS_SECM_GetData(ADDR_PED_FPE_KEY, PED_FPE_KEY_SLOT_LEN, (UINT8 *)&KeyBundle);

	// verify FPE Key ESKEY bundle
	if(TR31_VerifyKeyBundle_AES(PED_FPE_KEY_SLOT_LEN, (UINT8 *)&KeyBundle, keydata, mac16))
		result = TRUE;

	// clear sensitive data
	memset(keydata, 0x00, sizeof(keydata));

	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: Setup default KEK (Initial Key Encryption Key).
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	0	// 2019-01-11,removed.
void	PED_InitIKEK( void )
{
#ifdef	_USE_IRAM_PARA_
// 2014-11-24, allocate sensitive data buffer on MCU internal SRAM
#define	BUF_SIZE_IKEK		PED_KEK_SLOT_LEN
#define	BUF_ADDR_IKEK		0x0000F000	// pointer to MCU internal SRAM

UINT8	*buf = (UINT8 *)BUF_ADDR_IKEK;
#else
UINT8	buf[PED_KEK_SLOT_LEN];
#endif


	buf[0] = PED_KEK_LEN;
	PED_GetIKEK( &buf[1] );
      
	OS_SECM_PutData( ADDR_PED_IKEK, PED_KEK_SLOT_LEN, buf );
      
      // PATCH: 2009-04-07, clear sensitive data
#ifdef	_USE_IRAM_PARA_
	memset( buf, 0x00, sizeof(UINT8)*BUF_SIZE_IKEK );
#else
	memset( buf, 0x00, sizeof(buf) );
#endif
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Erase IKEK (Initial Key Encryption Key).
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	0
void	PED_EraseIKEK( void )
{
//UINT8	temp[PED_KEK_SLOT_LEN];
//
//	memset( temp, 0x00, PED_KEK_SLOT_LEN );
	
	OS_SECM_ClearData( ADDR_PED_IKEK, PED_KEK_SLOT_LEN, 0x00 );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Erase DKEK (Dynamic Key Encryption Key).
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	0
void	PED_EraseDKEK(void)
{	
	OS_SECM_ClearData(ADDR_PED_DKEK, PED_DKEK_SLOT_LEN, 0x00);
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Erase KEK (Key Encryption Key).
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_EraseKEK( void )
{	
//	OS_SECM_ClearData( ADDR_PED_KEK, PED_KEK_SLOT_LEN, 0x00 );
}

// ---------------------------------------------------------------------------
// FUNCTION: Erase KPK (Key Block Protection Key).
// INPUT   : type - 0 = erase both KPK.
//                  1 = erase KPK used for TDES key.
//                  2 = erase KPK used for AES key.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_EraseKPK(UINT8 type)
{	
	switch(type)
	{
		case 0:
			OS_SECM_ClearData(ADDR_PED_TDES_KEY_PROTECT_KEY, PED_TDES_KEY_PROTECT_KEY_SLOT_LEN, 0x00);
			OS_SECM_ClearData(ADDR_PED_AES_KEY_PROTECT_KEY, PED_AES_KEY_PROTECT_KEY_SLOT_LEN, 0x00);
			break;

		case 1:
			OS_SECM_ClearData(ADDR_PED_TDES_KEY_PROTECT_KEY, PED_TDES_KEY_PROTECT_KEY_SLOT_LEN, 0x00);
			break;

		case 2:
			OS_SECM_ClearData(ADDR_PED_AES_KEY_PROTECT_KEY, PED_AES_KEY_PROTECT_KEY_SLOT_LEN, 0x00);
			break;
	}
}

// ---------------------------------------------------------------------------
// FUNCTION: Erase KPK (Key Block Protection Key).
// INPUT   : type - 0x00 = erase all KPK.
//                  0x01 = erase KPK used for 128-bit TDES key.
//                  0x02 = erase KPK used for 192-bit TDES key.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_TDES_EraseKPK(UINT8 type)
{	
	switch(type)
	{
        case 0:
            OS_SECM_ClearData(ADDR_PED_128_TDES_KEY_PROTECT_KEY, PED_128_TDES_KEY_PROTECT_KEY_SLOT_LEN, 0x00);
            PED_SetKPKStatus(TDES_128, 0);

            OS_SECM_ClearData(ADDR_PED_192_TDES_KEY_PROTECT_KEY, PED_192_TDES_KEY_PROTECT_KEY_SLOT_LEN, 0x00);
            PED_SetKPKStatus(TDES_192, 0);
            break;

        case TDES_128:
            OS_SECM_ClearData(ADDR_PED_128_TDES_KEY_PROTECT_KEY, PED_128_TDES_KEY_PROTECT_KEY_SLOT_LEN, 0x00);
            PED_SetKPKStatus(TDES_128, 0);
            break;

        case TDES_192:
            OS_SECM_ClearData(ADDR_PED_192_TDES_KEY_PROTECT_KEY, PED_192_TDES_KEY_PROTECT_KEY_SLOT_LEN, 0x00);
            PED_SetKPKStatus(TDES_192, 0);
            break;
	}
}

// ---------------------------------------------------------------------------
// FUNCTION: Erase KPK (Key Block Protection Key).
// INPUT   : type - 0x00 = erase all KPK.
//                  0x03 = erase KPK used for 128-bit AES key.
//                  0x04 = erase KPK used for 192-bit AES key.
//                  0x05 = erase KPK used for 256-bit AES key.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_AES_EraseKPK(UINT8 type)
{	
	switch(type)
	{
        case 0:
			OS_SECM_ClearData(ADDR_PED_128_AES_KEY_PROTECT_KEY, PED_128_AES_KEY_PROTECT_KEY_SLOT_LEN, 0x00);
            PED_SetKPKStatus(AES_128, 0);

            OS_SECM_ClearData(ADDR_PED_192_AES_KEY_PROTECT_KEY, PED_192_AES_KEY_PROTECT_KEY_SLOT_LEN, 0x00);
            PED_SetKPKStatus(AES_192, 0);

            OS_SECM_ClearData(ADDR_PED_256_AES_KEY_PROTECT_KEY, PED_256_AES_KEY_PROTECT_KEY_SLOT_LEN, 0x00);
            PED_SetKPKStatus(AES_256, 0);
            break;

        case AES_128:
            OS_SECM_ClearData(ADDR_PED_128_AES_KEY_PROTECT_KEY, PED_128_AES_KEY_PROTECT_KEY_SLOT_LEN, 0x00);
            PED_SetKPKStatus(AES_128, 0);
            break;

        case AES_192:
            OS_SECM_ClearData(ADDR_PED_192_AES_KEY_PROTECT_KEY, PED_192_AES_KEY_PROTECT_KEY_SLOT_LEN, 0x00);
            PED_SetKPKStatus(AES_192, 0);
            break;

        case AES_256:
            OS_SECM_ClearData(ADDR_PED_256_AES_KEY_PROTECT_KEY, PED_256_AES_KEY_PROTECT_KEY_SLOT_LEN, 0x00);
            PED_SetKPKStatus(AES_256, 0);
            break;
	}
}

// ---------------------------------------------------------------------------
// FUNCTION: Erase Master Keys.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_EraseMasterKey( void )
{
UINT32	i;

	for( i=0; i<MAX_MKEY_CNT; i++ )
	   OS_SECM_ClearData( ADDR_PED_MKEY_01 + i*PED_MSKEY_SLOT_LEN, PED_MSKEY_SLOT_LEN, 0x00 );
}

// ---------------------------------------------------------------------------
// FUNCTION: Erase Session Keys.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_EraseSessionKey( void )
{
UINT32	i;

	for( i=0; i<MAX_SKEY_CNT; i++ )
	   OS_SECM_ClearData( ADDR_PED_SKEY_01 + i*PED_MSKEY_SLOT_LEN, PED_MSKEY_SLOT_LEN, 0x00 );
}

// ---------------------------------------------------------------------------
// FUNCTION: Erase Master Session Keys.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_EraseMSK( void )
{
	PED_EraseMasterKey();
	PED_EraseSessionKey();
}

// ---------------------------------------------------------------------------
// FUNCTION: Erase PEK.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_ErasePEK(void)
{
	UINT32	i;

    for(i = 0 ; i < MAX_PEK_MKEY_CNT ; i++)
    {
        // clear TDES Master Key
        OS_SECM_ClearData(ADDR_PED_PEK_TDES_MKEY_01 + i * PED_PEK_TDES_MSKEY_SLOT_LEN, PED_PEK_TDES_MSKEY_SLOT_LEN, 0x00);
        // clear AES Master Key
        OS_SECM_ClearData(ADDR_PED_PEK_AES_MKEY_01 + i * PED_PEK_AES_MSKEY_SLOT_LEN, PED_PEK_AES_MSKEY_SLOT_LEN, 0x00);
    }

    for(i = 0 ; i < MAX_PEK_SKEY_CNT ; i++)
    {
        // clear TDES Session Key
        OS_SECM_ClearData(ADDR_PED_PEK_AES_SKEY_01 + i * PED_PEK_TDES_MSKEY_SLOT_LEN, PED_PEK_TDES_MSKEY_SLOT_LEN, 0x00);
        // clear AES Session Key
        OS_SECM_ClearData(ADDR_PED_PEK_AES_SKEY_01 + i * PED_PEK_AES_MSKEY_SLOT_LEN, PED_PEK_AES_MSKEY_SLOT_LEN, 0x00);
    }

    PED_PEK_SetKeyType(0);
}

// ---------------------------------------------------------------------------
// FUNCTION: Erase DUKPT Keys.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_EraseDUKPT( void )
{
UINT32	i;

	
	OS_SECM_ClearData( ADDR_KSN_REG, KSN_REG_LEN, 0x00 );
	
	for( i=0; i<MAX_FUTURE_KEY_REG_CNT; i++ )
	   OS_SECM_ClearData( ADDR_FUTURE_KEY_REG + (i*FUTURE_KEY_SLOT_LEN), FUTURE_KEY_SLOT_LEN, 0x00 );
}

// ---------------------------------------------------------------------------
// FUNCTION: Erase AES DUKPT Keys.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_EraseAesDUKPT(void)
{
    UINT32 i;


    for(i = 0 ; i < MAX_NUM_REG ; i++)
    //    OS_SECM_ClearData(ADDR_INT_DERIVATION_KEY_REG + (i * MAX_KEYLENGTH), MAX_KEYLENGTH, 0x00);
       OS_SECM_ClearIntDerivationKeyReg(i * MAX_KEYLENGTH, MAX_KEYLENGTH, 0x00);
    
    PED_AES_DUKPT_SetKeyType(0);
}

// ---------------------------------------------------------------------------
// FUNCTION: Erase Fixed Keys.
// INPUT   : index - key slot index. (00..nn), 0xFF = erase all.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	0
void	PED_EraseFXKEY( UINT8 index )
{
UINT32	i;

	if( index == 0xFF )
	  {
	  for( i=0; i<MAX_FKEY_CNT; i++ )
	     OS_SECM_ClearData( ADDR_PED_FKEY_01 + (i*PED_FKEY_SLOT_LEN), PED_FKEY_SLOT_LEN, 0x00 );
	  }
	else
	  { 
	  if( index < MAX_FKEY_CNT )
	    OS_SECM_ClearData( ADDR_PED_FKEY_01 + (index*PED_FKEY_SLOT_LEN), PED_FKEY_SLOT_LEN, 0x00 );
	  }
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Erase ISO4_KEY Master Keys.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if 0
void	PED_EraseISO4MasterKey(void)
{
	UINT32	i;

	for(i = 0; i < MAX_ISO4_KEY_MKEY_CNT; i++)
		OS_SECM_ClearData(ADDR_PED_ISO4_KEY_MKEY_01 + i * PED_ISO4_KEY_MSKEY_SLOT_LEN, PED_ISO4_KEY_MSKEY_SLOT_LEN, 0x00);
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Erase ISO4_KEY Session Keys.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if 0
void	PED_EraseISO4SessionKey(void)
{
	UINT32	i;

	for(i = 0; i < MAX_ISO4_KEY_SKEY_CNT; i++)
		OS_SECM_ClearData(ADDR_PED_ISO4_KEY_SKEY_01 + i * PED_ISO4_KEY_MSKEY_SLOT_LEN, PED_ISO4_KEY_MSKEY_SLOT_LEN, 0x00);
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Erase ISO4_KEY.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_EraseISO4KEY( void )
{
#if 0
	PED_EraseISO4MasterKey();
	PED_EraseISO4SessionKey();
#endif

	OS_SECM_ClearData(ADDR_PED_ISO4_KEY, PED_ISO4_KEY_SLOT_LEN, 0x00);
}

// ---------------------------------------------------------------------------
// FUNCTION: Erase ACC_DEK Master Keys.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_EraseAccDEKMasterKey(void)
{
	UINT32	i;

	for(i = 0; i < MAX_ACC_DEK_MKEY_CNT; i++)
		OS_SECM_ClearData(ADDR_PED_ACC_DEK_MKEY_01 + i * PED_ACC_DEK_MSKEY_SLOT_LEN, PED_ACC_DEK_MSKEY_SLOT_LEN, 0x00);
}

// ---------------------------------------------------------------------------
// FUNCTION: Erase ACC_DEK Session Keys.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_EraseAccDEKSessionKey(void)
{
	UINT32	i;

	for(i = 0; i < MAX_ACC_DEK_SKEY_CNT; i++)
		OS_SECM_ClearData(ADDR_PED_ACC_DEK_SKEY_01 + i * PED_ACC_DEK_MSKEY_SLOT_LEN, PED_ACC_DEK_MSKEY_SLOT_LEN, 0x00);
}

// ---------------------------------------------------------------------------
// FUNCTION: Erase ACC_DEK.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_EraseAccDEK(void)
{
#if 0
	PED_EraseAccDEKMasterKey();
	PED_EraseAccDEKSessionKey();
#endif

    UINT32	i;

    for(i = 0 ; i < MAX_ACC_DEK_MKEY_CNT ; i++)
    {
        // clear TDES Master Key
        OS_SECM_ClearData(ADDR_PED_ACC_DEK_TDES_MKEY_01 + i * PED_ACC_DEK_TDES_MSKEY_SLOT_LEN, PED_ACC_DEK_TDES_MSKEY_SLOT_LEN, 0x00);
        // clear AES Master Key
        OS_SECM_ClearData(ADDR_PED_ACC_DEK_AES_MKEY_01 + i * PED_ACC_DEK_AES_MSKEY_SLOT_LEN, PED_ACC_DEK_AES_MSKEY_SLOT_LEN, 0x00);
    }

    for(i = 0 ; i < MAX_ACC_DEK_SKEY_CNT ; i++)
    {
        // clear TDES Session Key
        OS_SECM_ClearData(ADDR_PED_ACC_DEK_AES_SKEY_01 + i * PED_ACC_DEK_TDES_MSKEY_SLOT_LEN, PED_ACC_DEK_TDES_MSKEY_SLOT_LEN, 0x00);
        // clear AES Session Key
        OS_SECM_ClearData(ADDR_PED_ACC_DEK_AES_SKEY_01 + i * PED_ACC_DEK_AES_MSKEY_SLOT_LEN, PED_ACC_DEK_AES_MSKEY_SLOT_LEN, 0x00);
    }

    PED_AccDEK_SetKeyType(0);

    // clear AES DUKPT working key of ACC_DEK
    OS_SECM_ClearData(ADDR_PED_ACC_DEK_AES_DUKPT_WORKING_KEY, PED_ACC_DEK_AES_DUKPT_WORKING_KEY_LEN, 0x00);
    // clear AES DUKPT KSN of ACC_DEK
    OS_SECM_ClearData(ADDR_PED_ACC_DEK_CURRENT_KSN, 12, 0x00);
}

// ---------------------------------------------------------------------------
// FUNCTION: Erase FPE Master Keys.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if 0
void	PED_EraseFPEMasterKey(void)
{
	UINT32	i;

	for(i = 0; i < MAX_FPE_KEY_MKEY_CNT; i++)
		OS_SECM_ClearData(ADDR_PED_FPE_KEY_MKEY_01 + i * PED_FPE_KEY_MSKEY_SLOT_LEN, PED_FPE_KEY_MSKEY_SLOT_LEN, 0x00);
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Erase FPE Session Keys.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if 0
void	PED_EraseFPESessionKey(void)
{
	UINT32	i;

	for(i = 0; i < MAX_FPE_KEY_SKEY_CNT; i++)
		OS_SECM_ClearData(ADDR_PED_FPE_KEY_SKEY_01 + i * PED_FPE_KEY_MSKEY_SLOT_LEN, PED_FPE_KEY_MSKEY_SLOT_LEN, 0x00);
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Erase FPE Key.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_EraseFPEKEY(void)
{
#if 0
	PED_EraseFPEMasterKey();
	PED_EraseFPESessionKey();
#endif

	UINT32	i;

    for(i = 0 ; i < MAX_FPE_KEY_MKEY_CNT ; i++)
		OS_SECM_ClearData(ADDR_PED_FPE_KEY_AES_MKEY_01 + i * PED_FPE_KEY_AES_MSKEY_SLOT_LEN, PED_FPE_KEY_AES_MSKEY_SLOT_LEN, 0x00);

    for(i = 0 ; i < MAX_FPE_KEY_SKEY_CNT ; i++)
		OS_SECM_ClearData(ADDR_PED_FPE_KEY_AES_SKEY_01 + i * PED_FPE_KEY_AES_MSKEY_SLOT_LEN, PED_FPE_KEY_AES_MSKEY_SLOT_LEN, 0x00);
    
    PED_FPE_SetKeyType(0);
}

// ---------------------------------------------------------------------------
// FUNCTION: Erase administrator mode password.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_EraseAdminPSW( void )
{
//	OS_SECM_ClearData( ADDR_PED_ADMIN_PSW, 1, MIN_PED_PSW_LEN );
//	OS_SECM_ClearData( ADDR_PED_ADMIN_PSW+1, MIN_PED_PSW_LEN, '0' );

	OS_SECM_ClearData( ADDR_PED_ADMIN_PSW1, PED_PSW_SLOT_LEN, 0x00 );
	OS_SECM_ClearData( ADDR_PED_ADMIN_PSW2, PED_PSW_SLOT_LEN, 0x00 );
}

// ---------------------------------------------------------------------------
// FUNCTION: Erase user mode password.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_EraseUserPSW( void )
{
	OS_SECM_ClearData( ADDR_PED_USER_PSW, 1, MIN_PED_PSW_LEN );
	OS_SECM_ClearData( ADDR_PED_USER_PSW+1, MIN_PED_PSW_LEN, '0' );
}

// ---------------------------------------------------------------------------
// FUNCTION: Write key opearation mode.
// INPUT   : mode - PED_KEY_MODE_XXX.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_WriteKeyMode( UINT8 mode )
{
	OS_SECM_PutData( ADDR_PED_KEY_MODE, 1, (UINT8 *)&mode );
}

// ---------------------------------------------------------------------------
// FUNCTION: Read key opearation mode.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : PED_KEY_MODE_xxx.
// ---------------------------------------------------------------------------
UINT8	PED_ReadKeyMode( void )
{
UINT8	mode;
	
	PED_InUse( TRUE );
	OS_SECM_GetData( ADDR_PED_KEY_MODE, 1, (UINT8 *)&mode );
	PED_InUse( FALSE );
	
	return( mode );
}

// ---------------------------------------------------------------------------
// FUNCTION: Setup current master key index.
// INPUT   : index - current master key index.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_WriteMKeyIndex( UINT8 index )
{
	OS_SECM_PutData( ADDR_PED_MKEY_INDEX, 1, (UINT8 *)&index );
}

// ---------------------------------------------------------------------------
// FUNCTION: Get current master key index.
// INPUT   : none.
// OUTPUT  : index - current master key index.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_ReadMKeyIndex( UINT8 *index )
{
//	OS_SECM_GetData( ADDR_PED_MKEY_INDEX, 1, index );
	*index = 0;	// always the 1'st one
}

// ---------------------------------------------------------------------------
// FUNCTION: Setup current PEK master key index.
// INPUT   : index - current master key index.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_PEK_WriteMKeyIndex(UINT8 index)
{
	OS_SECM_PutData( ADDR_PED_PEK_MKEY_INDEX, 1, (UINT8 *)&index );
}

// ---------------------------------------------------------------------------
// FUNCTION: Get current PEK master key index.
// INPUT   : none.
// OUTPUT  : index - current master key index.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_PEK_ReadMKeyIndex(UINT8 *index)
{
//	OS_SECM_GetData( ADDR_PED_PEK_MKEY_INDEX, 1, index );
	*index = 0;	// always the 1'st one
}

// ---------------------------------------------------------------------------
// FUNCTION: Setup current ISO4_KEY master key index.
// INPUT   : index - current master key index.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if 0
void	PED_ISO4_WriteMKeyIndex(UINT8 index)
{
	OS_SECM_PutData(ADDR_PED_ISO4_KEY_MKEY_INDEX, 1, (UINT8 *)&index);
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Get current ISO4_KEY master key index.
// INPUT   : none.
// OUTPUT  : index - current master key index.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if 0
void	PED_ISO4_ReadMKeyIndex(UINT8 *index)
{
	//	OS_SECM_GetData( ADDR_PED_ISO4_KEY_MKEY_INDEX, 1, index );
	*index = 0;	// always the 1'st one
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Setup current ACC_DEK master key index.
// INPUT   : index - current master key index.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_AccDEK_WriteMKeyIndex(UINT8 index)
{
	OS_SECM_PutData(ADDR_PED_ACC_DEK_MKEY_INDEX, 1, (UINT8 *)&index);
}

// ---------------------------------------------------------------------------
// FUNCTION: Get current ACC_DEK master key index.
// INPUT   : none.
// OUTPUT  : index - current master key index.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_AccDEK_ReadMKeyIndex(UINT8 *index)
{
	//	OS_SECM_GetData( ADDR_PED_ACC_DEK_MKEY_INDEX, 1, index );
	*index = 0;	// always the 1'st one
}

// ---------------------------------------------------------------------------
// FUNCTION: Setup current FPE master key index.
// INPUT   : index - current master key index.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_FPE_WriteMKeyIndex(UINT8 index)
{
	OS_SECM_PutData(ADDR_PED_FPE_KEY_MKEY_INDEX, 1, (UINT8 *)&index);
}

// ---------------------------------------------------------------------------
// FUNCTION: Get current FPE master key index.
// INPUT   : none.
// OUTPUT  : index - current master key index.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_FPE_ReadMKeyIndex(UINT8 *index)
{
	//	OS_SECM_GetData( ADDR_PED_FPE_KEY_MKEY_INDEX, 1, index );
	*index = 0;	// always the 1'st one
}

// ---------------------------------------------------------------------------
// FUNCTION: Setup current PEK key type.
// INPUT   : keyType - key length and algorithm.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_PEK_SetKeyType(UINT8 keyType)
{
	OS_SECM_PutData(ADDR_PED_PEK_KEY_TYPE, 1, (UINT8 *)&keyType);
}

// ---------------------------------------------------------------------------
// FUNCTION: Get current PEK key type.
// INPUT   : none.
// OUTPUT  : keyType - key length and algorithm.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_PEK_GetKeyType(UINT8 *keyType)
{
	OS_SECM_GetData(ADDR_PED_PEK_KEY_TYPE, 1, keyType);
}

// ---------------------------------------------------------------------------
// FUNCTION: Setup current AccDEK key type.
// INPUT   : keyType - key length and algorithm.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_AccDEK_SetKeyType(UINT8 keyType)
{
	OS_SECM_PutData(ADDR_PED_AccDEK_KEY_TYPE, 1, (UINT8 *)&keyType);
}

// ---------------------------------------------------------------------------
// FUNCTION: Get current AccDEK key type.
// INPUT   : none.
// OUTPUT  : keyType - key length and algorithm.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_AccDEK_GetKeyType(UINT8 *keyType)
{
	OS_SECM_GetData(ADDR_PED_AccDEK_KEY_TYPE, 1, keyType);
}

// ---------------------------------------------------------------------------
// FUNCTION: Setup current FPE key type.
// INPUT   : keyType - key length and algorithm.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_FPE_SetKeyType(UINT8 keyType)
{
	OS_SECM_PutData(ADDR_PED_FPE_KEY_TYPE, 1, (UINT8 *)&keyType);
}

// ---------------------------------------------------------------------------
// FUNCTION: Get current FPE key type.
// INPUT   : none.
// OUTPUT  : keyType - key length and algorithm.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_FPE_GetKeyType(UINT8 *keyType)
{
	OS_SECM_GetData(ADDR_PED_FPE_KEY_TYPE, 1, keyType);
}

// ---------------------------------------------------------------------------
// FUNCTION: Setup current AES DUKPT key type.
// INPUT   : keyType - key length and algorithm.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_AES_DUKPT_SetKeyType(UINT8 keyType)
{
	OS_SECM_PutData(ADDR_AES_DUKPT_KEY_TYPE, 1, (UINT8 *)&keyType);
}

// ---------------------------------------------------------------------------
// FUNCTION: Get current AES DUKPT key type.
// INPUT   : none.
// OUTPUT  : keyType - key length and algorithm.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_AES_DUKPT_GetKeyType(UINT8 *keyType)
{
	OS_SECM_GetData(ADDR_AES_DUKPT_KEY_TYPE, 1, keyType);
}

// ---------------------------------------------------------------------------
// FUNCTION: Erase all sensitive data in secure SRAM.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_EraseSensitiveData( void )
{
	//PED_EraseIKEK();
	//PED_EraseDKEK();
	//PED_EraseKEK();
	PED_EraseKPK(0);
	PED_EraseMSK();
	PED_EraseDUKPT();
//	PED_EraseFXKEY(0xFF);
	PED_EraseCAPK();
	PED_EraseISO4KEY();
	PED_EraseAccDEK();
	PED_EraseFPEKEY();
	
	PED_EraseAdminPSW();
	PED_EraseUserPSW();
	
	PED_WriteKeyMode( PED_KEY_MODE_NULL );
	PED_WriteMKeyIndex( 0 );
}

// ---------------------------------------------------------------------------
// FUNCTION: Erase all sensitive keys in secure SRAM.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_EraseSensitiveKeys( void )
{
	//if(PED_VerifyIkekStatus() == FALSE)
	//{
	//	PED_EraseIKEK();
	//}

	//PED_EraseDKEK();
	//PED_EraseKEK();

	//if(PED_VerifyKPKStatus(1) == FALSE)
	//{
	//	PED_EraseKPK(1);
	//}

	//if(PED_VerifyKPKStatus(5) == FALSE)
	//{
	//	PED_EraseKPK(2);
	//}

    PED_TDES_EraseKPK(0);
    PED_AES_EraseKPK(0);

//	PED_EraseMSK();
//	PED_EraseDUKPT();
    PED_EraseAesDUKPT();
//	PED_EraseFXKEY(0xFF);
//	PED_EraseCAPK();
//	PED_EraseISO4KEY();
    PED_ErasePEK();
	PED_EraseAccDEK();
	PED_EraseFPEKEY();
	
//	PED_EraseAdminPSW();
//	PED_EraseUserPSW();
//	
//	PED_WriteKeyMode( PED_KEY_MODE_NULL );
//	PED_WriteMKeyIndex( 0 );
}

// ---------------------------------------------------------------------------
// FUNCTION: To encipher plaintext data (pIn) to enciphered data (pOut) by DES.
// INPUT   : pIn  -- the plaintext data. (8-byte)
//           pKey  -- the DES key.       (8-byte)
// OUTPUT  : pOut -- the ciphered data.  (8-byte)
// RETURN  : apiOK
//           apiFailed
// NOTE    : for kernel used only.
// ---------------------------------------------------------------------------
UINT32	PED_des_encipher( UINT8 *pIn, UINT8 *pOut, UINT8 *pKey )
{
	return( api_des_encipher( pIn, pOut, pKey ) );
}

// ---------------------------------------------------------------------------
// FUNCTION: To decipher enciphered data (pIn) to plaintext data (pOut) by DES.
// INPUT   : pIn  -- the ciphered data.  (8-byte)
//           pKey -- the DES key.        (8-byte)
// OUTPUT  : pOut -- the plaintext data. (8-byte)
// RETURN  : apiOK
//           apiFailed
// NOTE    : for kernel used only.
// ---------------------------------------------------------------------------
UINT32	PED_des_decipher( UINT8 *pOut, UINT8 *pIn, UINT8 *pKey )
{
	return( api_des_decipher( pOut, pIn, pKey ) );
}

// ---------------------------------------------------------------------------
// FUNCTION: Encription of 3DES.
// INPUT   : key --   3DES key. (16 bytes)
//           len --   length of input data. (8N, N=1...)
//           idata -- input data.
// OUTPUT  : odata -- output data.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_TripleDES( UINT8 *key, UINT8 len, UINT8 *idata, UINT8 *odata )
{
UINT8	i, j;
UINT8	cnt;
UINT8	xkey[24];	// expanded to 24-bit key (L-R-L)
UINT8	ibuf[8];
UINT8	obuf[8];

      memmove( &xkey[0], key, 16 );	// L-R
      memmove( &xkey[16], key, 8 );	// L

      cnt = len / 8;

      for( i=0; i<cnt; i++ )
         {
         for( j=0; j<8; j++ )
            ibuf[j] = *idata++;
            
         api_3des_encipher( ibuf, obuf, xkey );
            
         for( j=0; j<8; j++ )
            *odata++ = obuf[j];
         }
         
	// PATCH: 2009-04-07, clear sensitive data
	memset( xkey, 0x00, sizeof(xkey) );
    memset(ibuf, 0x00, sizeof(ibuf));
    memset(obuf, 0x00, sizeof(obuf));
}

// ---------------------------------------------------------------------------
// FUNCTION: Decription of 3DES.
// INPUT   : key --   3DES key. (16 bytes)
//           len --   length of input data. (8N, N=1...)
//           idata -- input data.
// OUTPUT  : odata -- output data.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_TripleDES2( UINT8 *key, UINT8 len, UINT8 *idata, UINT8 *odata )
{
UINT8	i, j;
UINT8	cnt;
UINT8	xkey[24];	// expanded to 24-bit key (L-R-L)
UINT8	ibuf[8];
UINT8	obuf[8];

      memmove( &xkey[0], key, 16 );	// L-R
      memmove( &xkey[16], key, 8 );	// L

      cnt = len / 8;

      for( i=0; i<cnt; i++ )
         {
         for( j=0; j<8; j++ )
            ibuf[j] = *idata++;
            
         api_3des_decipher( obuf, ibuf, xkey );
            
         for( j=0; j<8; j++ )
            *odata++ = obuf[j];
         }
         
	// PATCH: 2009-04-07, clear sensitive data
	memset( xkey, 0x00, sizeof(xkey) );
    memset(ibuf, 0x00, sizeof(ibuf));
    memset(obuf, 0x00, sizeof(obuf));
}

// ---------------------------------------------------------------------------
// FUNCTION: Encryption of 3DES. (CBC mode)
// INPUT   : key --   3DES key. (16 bytes)
//           len --   length of input data. (8N, N=1...)
//           idata -- input data.
//	     icv   -- initial chain vector.
// OUTPUT  : odata -- output data.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_CBC_TripleDES( UINT8 *key, UINT8 *icv, UINT8 len, UINT8 *idata, UINT8 *odata )
{
UINT16	i, j;
UINT16	cnt;
UINT8	ctext[8];
UINT8	ptext[8];


        cnt = len / 8;

        if( cnt )
          {
          for( i=0; i<cnt; i++ )
             {
             // XOR( ICV, D1 )
             for( j=0; j<8; j++ )
                ptext[j] = icv[j] ^ idata[i*8+j];

             // DES( ptext, key(L) )
             PED_des_encipher( ptext, ctext, key );
             
             // -DES( ctext, key(R) )
             PED_des_decipher( ptext, ctext, &key[8] );

             // DES( ptext, key(L) )
             //PED_des_encipher( ptext, ctext, key );
             PED_des_encipher( ptext, ctext, &key[16] );
             
             for( j=0; j<8; j++ )
                {
                *odata++ = ctext[j];
                icv[j] = ctext[j];
        	}
             }
          }
        
        // clear sensitive data
	    memset(ctext, 0x00, sizeof(ctext));
        memset(ptext, 0x00, sizeof(ptext));
}

// ---------------------------------------------------------------------------
// FUNCTION: Decription of 3DES. (CBC mode)
// INPUT   : key --   3DES key. (16 bytes)
//           len --   length of input data. (8N, N=1...)
//           idata -- input data.
//	     icv   -- initial chain vector.
// OUTPUT  : odata -- output data.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_CBC_TripleDES2( UINT8 *key, UINT8 *icv, UINT8 len, UINT8 *idata, UINT8 *odata )
{
UINT16	i, j;
UINT16	cnt;
UINT8	ctext[8];
UINT8	ptext[8];


        cnt = len / 8;

        if( cnt )
          {
          for( i=0; i<cnt; i++ )
             {  
             memmove( ctext, &idata[i*8], 8 );

             // -DES( ctext, key(L) )
             PED_des_decipher( ptext, ctext, key );
             
             // DES( ptext, key(R) )
             PED_des_encipher( ptext, ctext, &key[8] );

             // -DES( ctext, key(L) )
             PED_des_decipher( ptext, ctext, key );

             // XOR( ICV, D1 )
             for( j=0; j<8; j++ )
                {
                *odata++ = icv[j] ^ ptext[j];
                icv[j] = idata[i*8+j];
                }
             }
          }
        
        // clear sensitive data
	    memset(ctext, 0x00, sizeof(ctext));
        memset(ptext, 0x00, sizeof(ptext));
}

// ---------------------------------------------------------------------------
// FUNCTION: Decription of AES. (CBC mode)
// INPUT   : key    -- AES key. (16/24/32 bytes)
//           keyLen -- length of AES key
//           len    -- length of input data. (16*N, N=1...)
//           idata  -- input data.
//			 icv    -- initial chain vector.
// OUTPUT  : odata  -- output data.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_CBC_AES2(UINT8 *key, UINT8 keyLen, UINT8 *icv, UINT8 len, UINT8 *idata, UINT8 *odata)
{
	UINT16	i, j;
	UINT16	cnt;
	UINT8	ctext[16];
	UINT8	ptext[16];


	cnt = len / 16;

	if(cnt)
	{
		for(i = 0; i < cnt; i++)
		{
			memmove(ctext, &idata[i * 16], 16);

			// -AES
			api_aes_decipher(ptext, ctext, key, keyLen);

			// XOR( ICV, D1 )
			for(j = 0; j < 16; j++)
			{
				*odata++ = icv[j] ^ ptext[j];
				icv[j] = idata[i * 16 + j];
			}
		}
	}

    // clear sensitive data
	memset(ctext, 0x00, sizeof(ctext));
    memset(ptext, 0x00, sizeof(ptext));
}

// ---------------------------------------------------------------------------
// FUNCTION: Download Dynamic Key Encryption Key (DKEK)
// INPUT   : tid  - terminal id. (8 bytes)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
//
//           TERM                         HOST
//           --------------               -------------
//			 CMD+TSN(8)+TID(8)   ->
//								 <-		  CMD+EDKEK(16)+KCV(3)
//
//			 where: EDKEK(16) = TDES(DKEK,DIKEK), DKEK = Dynamic Key Encryption Key
//				    DIKEK(24) = Derived Initial Key Encryption Key
//							  = TSN[1]+IKEK[1:2]+TSN[2]+IKEK[3:4]+
//							    TSN[3]+IKEK[5:6]+TSN[4]+IKEK[7:8]+
//								TSN[5]+IKEK[9:10]+TSN[6]+IKEK[11:12]+
//								TSN[7]+IKEK[13:14]+TSN[8]+IKEK[15:16]
// NOTE    : This is the first command before downloading Key Block Protection Key.
// ---------------------------------------------------------------------------
#if	0
UINT8	PED_LoadDynamicKEK(UINT8 *tid)
{
	UINT8	ikek[16];
	UINT8	dikek[24];
	UINT8	temp[32];
	UINT8	buf1[32];
	UINT8	buf2[40];
	UINT8	result;
	UINT8	ernd[8];

	// Terminal Serial Number (TSN)
	api_sys_info(SID_TerminalSerialNumber, temp);
	memmove(buf2, &temp[4], 8);

	// Terminal ID (TID)
	memmove(&buf2[8], tid, 8);

	// Retrieve IKEK
	OS_SECM_GetData(ADDR_PED_IKEK + 1, PED_KEK_LEN, ikek);	// L-V(16), ignore LEN field

	//Derived IKEK (24) = TSN[1] + IKEK[1:2] + TSN[2] + IKEK[3:4] +
	//					  TSN[3] + IKEK[5:6] + TSN[4] + IKEK[7:8] +
	//					  TSN[5] + IKEK[9:10] + TSN[6] + IKEK[11:12] +
	//					  TSN[7] + IKEK[13:14] + TSN[8] + IKEK[15:16]
	dikek[0] = temp[4];
	memcpy(&dikek[1], &ikek[0], 2);
	dikek[3] = temp[5];
	memcpy(&dikek[4], &ikek[2], 2);
	dikek[6] = temp[6];
	memcpy(&dikek[7], &ikek[4], 2);
	dikek[9] = temp[7];
	memcpy(&dikek[10], &ikek[6], 2);
	dikek[12] = temp[8];
	memcpy(&dikek[13], &ikek[8], 2);
	dikek[15] = temp[9];
	memcpy(&dikek[16], &ikek[10], 2);
	dikek[18] = temp[10];
	memcpy(&dikek[19], &ikek[12], 2);
	dikek[21] = temp[11];
	memcpy(&dikek[22], &ikek[14], 2);

	memmove(&buf1[3], buf2, 16);
	buf1[0] = 17;
	buf1[1] = 0;
	buf1[2] = KMC_DOWNLOAD_DYNAMIC_KEK;	// command

	// init communication port
//	if(LIB_OpenAUX(COM0, auxDLL, COM_9600) == FALSE)
//	{
//		memset(ikek, 0x00, sizeof(ikek));
//		return(apiFailed); // device error
//	}

	result = apiFailed;

	// send CMD + TSN(8) + TID(8) to HOST for download key
	if(LIB_TransmitAUX(buf1) == TRUE)
	{
		if(LIB_ReceiveAUX(buf2) == TRUE)
		{
			// Download Dynamic Key Encryption Key (DKEK) with KCV
			if((buf2[0] == 20) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
			{
				// Verify encrypted Dynamic Key Encryption Key (EDKEK): LEN(1) + EDKEK(16) + KCV(3)
				memmove(temp, &buf2[3], PED_DKEK_LEN);

				api_3des_decipher2( buf1, temp, dikek, PED_DKEK_LEN );
				//	  LIB_DumpHexData( 0, 0, PED_DKEK_LEN, buf1 );

				memset(ernd, 0x00, 8);
				PED_TripleDES(buf1, 8, ernd, temp);		// temp = KCV = TDES(NULL, DKEK)

				if(LIB_memcmp(temp, &buf2[3 + 16], 3) == 0)	// KCV OK?
				{
					// Save DKEK : LEN(1) + DKEK(16) + KCV(3)
					buf2[0] = PED_DKEK_LEN;			// LEN
					memmove(&buf2[1], buf1, PED_DKEK_LEN);	// DKEK
					memmove(&buf2[1 + PED_DKEK_LEN], temp, 3);	// KCV
					OS_SECM_PutData(ADDR_PED_DKEK, PED_DKEK_SLOT_LEN, buf2);
					//		LIB_DumpHexData( 0, 0, PED_DKEK_SLOT_LEN, buf2 );

					result = apiOK;	// done
				}
			}
		}
	}

//	LIB_CloseAUX();

	//Clear sensitive data
	memset(ikek, 0x00, sizeof(ikek));
	memset(dikek, 0x00, sizeof(dikek));
	memset(temp, 0x00, sizeof(temp));
	memset(buf1, 0x00, sizeof(buf1));
	memset(buf2, 0x00, sizeof(buf2));

	return result;
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Generate and Download Key Block Protection Key (KPK).
// INPUT   : tid  - terminal id. (8 bytes)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
//
//           TERM                         HOST
//           --------------               -------------
//           CMD+TSN(8)+TID(8)+ERND(8) ->
//                                     <- CMD+CRYPTOGRAM
//	     CMD+TSN(8)+TID(8)	       ->
//				       <- CMD+EKPK(16)+KCV(3)
//
//           where: ERND(8)  = TDES( RND_B(8), IKEK )
//                  RND_B(8) = 8-byte random number, generated by EDC
//                  CRYPTOGRAM(24) = TDES( KEK(16)+RND_R(8), IKEK )
//                  RND_R(8) = 8-byte random number, generated by KMS
//                  KEK(16)  = RND_R[5:8]+RND_B[1:4]+RND_R[1:4]+RND_B[5:8]
//		    EKPK(16) = TDES(KPK, KEK), KPK = Key Block Protection Key (ANSI TR-31)
//		    KCV(3)   = Key Check Value of KPK = TDES(NULL(8), KPK)
// NOTE    : This is the first command before key download.
// ---------------------------------------------------------------------------
#if	0
UINT8	PED_LoadKeyBlockProtectionKey(UINT8 *tid)
{
#ifdef	_USE_IRAM_PARA_
	// 2014-11-24, allocate sensitive data buffer on MCU internal SRAM
#define	IKEK_SIZE_LKPK		16
#define	IKEK_ADDR_LKPK		0x0000F000	// pointer to MCU internal SRAM
#define	TEMP_SIZE_LKPK		32
#define	TEMP_ADDR_LKPK		IKEK_ADDR_LKPK+IKEK_SIZE_LKPK
#define	BUF1_SIZE_LKPK		32
#define	BUF1_ADDR_LKPK		TEMP_ADDR_LKPK+TEMP_SIZE_LKPK
#define	BUF2_SIZE_LKPK		40
#define	BUF2_ADDR_LKPK		BUF1_ADDR_LKPK+BUF1_SIZE_LKPK
#define	KEY_SIZE_LKPK		16
#define	KEY_ADDR_LKPK		BUF2_ADDR_LKPK+BUF2_SIZE_LKPK

	UINT8	*ikek = (UINT8 *)IKEK_ADDR_LKPK;
	UINT8	*temp = (UINT8 *)TEMP_ADDR_LKPK;
	UINT8	*buf1 = (UINT8 *)BUF1_ADDR_LKPK;
	UINT8	*buf2 = (UINT8 *)BUF2_ADDR_LKPK;
	UINT8	*key = (UINT8 *)KEY_ADDR_LKPK;
#else
	UINT8	kek[16];
	UINT8	dkek[16];
	UINT8	temp[32];
	UINT8	buf1[32];
	UINT8	buf2[40];
	UINT8	key[16];
#endif

	UINT8	rnd_b[8];
	UINT8	ernd[8];
	UINT8	result;


	// Terminal Serial Number (TSN)
	api_sys_info(SID_TerminalSerialNumber, temp);
	memmove(buf2, &temp[4], 8);

	// Terminal ID (TID)
	memmove(&buf2[8], tid, 8);

	// Retrieve DKEK
	OS_SECM_GetData(ADDR_PED_DKEK + 1, PED_DKEK_LEN, dkek);	// L-V(16), ignore LEN field

	// Random Number (RND_B)
	api_sys_random(rnd_b);

	// ERND = TDES(RND_B, DKEK)
	PED_TripleDES(dkek, 8, rnd_b, ernd);
	memmove(&buf2[16], ernd, 8);

	memmove(&buf1[3], buf2, 24);
	buf1[0] = 25;
	buf1[1] = 0;
	buf1[2] = KMC_DOWNLOAD_KEY_PROTECT_KEY;	// command

	// init communication port
//	if(LIB_OpenAUX(COM0, auxDLL, COM_9600) == FALSE)
//	{
//		// PATCH: 2009-04-07, clear sensitive data
//#ifdef	_USE_IRAM_PARA_
//		memset(ikek, 0x00, sizeof(UINT8)*IKEK_SIZE_LKPK);
//#else
//		memset(dkek, 0x00, sizeof(dkek));
//#endif
//		return(apiFailed); // device error
//	}

	result = apiFailed;

	// send CMD + TSN(8) + TID(8) + ERND(8) to HOST for download key
	if(LIB_TransmitAUX(buf1) == TRUE)
	{
		if(LIB_ReceiveAUX(buf2) == TRUE)
		{
			if((buf2[0] == 25) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
			{
				// recover, check, and save KEK(16)
				memmove(temp, &buf2[3], 24);

				PED_TripleDES2(dkek, 24, temp, buf2);
				memmove(&buf1[1], buf2, 24);

				//      LIB_DumpHexData(0,0,24, buf2);

				if((LIB_memcmp(&buf1[1], &buf1[21], 4) != 0) ||
					(LIB_memcmp(&buf1[9], &buf1[17], 4) != 0) ||
					(LIB_memcmp(&buf1[5], &rnd_b[0], 4) != 0) ||
					(LIB_memcmp(&buf1[13], &rnd_b[4], 4) != 0))
					result = apiFailed;
				else
				{
					buf1[0] = PED_KEK_LEN;
					OS_SECM_PutData(ADDR_PED_KEK, PED_KEK_SLOT_LEN, buf1);
					//	      LIB_DumpHexData( 0, 0, PED_KEK_LEN+1, buf1 );
					memmove(kek, &buf1[1], PED_KEK_LEN);
					result = apiOK;
				}
			}
		}
	}

	// Download encrypted Key Block Protection Key with KCV
	if(result == apiOK)
	{
		// Terminal Serial Number (TSN)
		api_sys_info(SID_TerminalSerialNumber, temp);
		memmove(&buf1[3], &temp[4], 8);

		// Terminal ID (TID)
		memmove(&buf1[3 + 8], tid, 8);

		buf1[0] = 17;
		buf1[1] = 0;
		buf1[2] = KMC_DOWNLOAD_KEY_PROTECT_KEY | KMC_NEXT; // command

		result = apiFailed;

		if(LIB_TransmitAUX(buf1) == TRUE)
		{
			if(LIB_ReceiveAUX(buf2) == TRUE)
			{
				if((buf2[0] == 20) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
				{
					// Verify encrypted Key Block Protection Key (EKPK): LEN(1) + EKPK(16) + KCV(3)
					memmove(temp, &buf2[3], PED_TDES_KEY_PROTECT_KEY_LEN);

					PED_TripleDES2(kek, PED_TDES_KEY_PROTECT_KEY_LEN, temp, buf1);	// buf1 = KPK = ~TDES(EKB, KEK)
	  //	      LIB_DumpHexData( 0, 0, PED_TDES_KEY_PROTECT_KEY_LEN, buf1 );

					memset(ernd, 0x00, 8);
					PED_TripleDES(buf1, 8, ernd, temp);		// temp = KCV = TDES(NULL, KPK)

					if(LIB_memcmp(temp, &buf2[3 + 16], 3) == 0)	// KCV OK?
					{
						// Save KPK : LEN(1) + KPK(16) + KCV(3)
						buf2[0] = PED_TDES_KEY_PROTECT_KEY_LEN;			// LEN
						memmove(&buf2[1], buf1, PED_TDES_KEY_PROTECT_KEY_LEN);	// KPK
						memmove(&buf2[1 + PED_TDES_KEY_PROTECT_KEY_LEN], temp, 3);	// KCV
						OS_SECM_PutData(ADDR_PED_TDES_KEY_PROTECT_KEY, PED_TDES_KEY_PROTECT_KEY_SLOT_LEN, buf2);
						//		LIB_DumpHexData( 0, 0, PED_TDES_KEY_PROTECT_KEY_SLOT_LEN, buf2 );

						result = apiOK;	// done
					}
				}
			}
		}
	}

//	LIB_CloseAUX();

	// PATHC: 2009-04-07, clear sensitive data
#ifdef	_USE_IRAM_PARA_
	memset(ikek, 0x00, sizeof(UINT8)*IKEK_SIZE_LKPK);
	memset(temp, 0x00, sizeof(UINT8)*TEMP_SIZE_LKPK);
	memset(buf1, 0x00, sizeof(UINT8)*BUF1_SIZE_LKPK);
	memset(buf2, 0x00, sizeof(UINT8)*BUF2_SIZE_LKPK);
	memset(key, 0x00, sizeof(UINT8)*KEY_SIZE_LKPK);
#else
	memset(kek, 0x00, sizeof(kek));
	memset(dkek, 0x00, sizeof(dkek));
	memset(temp, 0x00, sizeof(temp));
	memset(buf1, 0x00, sizeof(buf1));
	memset(buf2, 0x00, sizeof(buf2));
	memset(key, 0x00, sizeof(key));
#endif

	return(result);
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: load DUKPT initial keys.
// INPUT   : tid - terminal id. (8-byte string)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
//
//           TERM                  HOST
//           --------------        -------------
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+EIPEK(80)
//	         CMD+TSN(8)+TID(8) ->
//			                <-	   CMD+IKSN(10)
//
//           where: EIPEK(80)	= EKB( IPEK(16), KPK )
//		            IPEK(16)	= Initial DUKPT Key
//		            IKSN(10)	= Initial Key Serial Number
// ---------------------------------------------------------------------------
UINT8	PED_LoadDukptInitKey(UINT8 *tid)
{
    UINT8	eipek[X9143_TDES_KEY_BUNDLE_LEN];	// encrypted initial PIN encryption key // ==== [Debug] ====
	//UINT8	eipek[KEY_BUNDLE_LEN];	// encrypted initial PIN encryption key
	UINT8	iksn[10];		// plaintext initial key serial number
	UINT8	temp[32];
	UINT8	buf1[32];
//	UINT8	buf2[128];
	UINT8	buf2[255];		// 2022-06-06
	UINT8	result;


	// Terminal Serial Number (TSN)
	api_sys_info(SID_TerminalSerialNumber, temp);
	memmove(buf2, &temp[4], 8);

	// Terminal ID (TID)
	memmove(&buf2[8], tid, 8);

	memmove(&buf1[3], buf2, 16);
	buf1[0] = 17;
	buf1[1] = 0;
	buf1[2] = KMC_DOWNLOAD_DUKPT_IPEK; // command

	// send CMD + TSN(8) + TID(8) to HOST for EIPEK(80)
	result = apiFailed;

	if(LIB_TransmitAUX(buf1) == TRUE)
	{
		if(LIB_ReceiveAUX(buf2) == TRUE)
		{
			//if((buf2[0] == (KEY_BUNDLE_LEN + 1)) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
            if((buf2[0] == (X9143_TDES_KEY_BUNDLE_LEN + 1)) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
			{
				//memmove(eipek, &buf2[3], KEY_BUNDLE_LEN);
                memmove(eipek, &buf2[3], X9143_TDES_KEY_BUNDLE_LEN);
				result = apiOK;
			}
		}
	}

	if(result == apiOK)
	{
		LIB_WaitTime(50);

		// Terminal Serial Number (TSN)
		api_sys_info(SID_TerminalSerialNumber, temp);
		memmove(buf2, &temp[4], 8);

		// Terminal ID (TID)
		memmove(&buf2[8], tid, 8);

		memmove(&buf1[3], buf2, 16);
		buf1[0] = 17;
		buf1[1] = 0;
		buf1[2] = KMC_DOWNLOAD_DUKPT_IKSN; // command

		// send CMD + TSN(8) + TID(8) to HOST for IKSN(10)
		result = apiFailed;

		if(LIB_TransmitAUX(buf1) == TRUE)
		{
			if(LIB_ReceiveAUX(buf2) == TRUE)
			{
				if((buf2[0] == 11) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
				{
					memmove(iksn, &buf2[3], 10);
					result = apiOK;
				}
			}
		}
	}

	if(result == apiOK)
	{
		// setup DUKPT keys
		result = PED_SetDUKPT(1, eipek, iksn);
	}

	if(result == apiFailed)
	{
		// erase KEK
		PED_EraseKEK();
	}
	
	memset(eipek, 0x00, sizeof(eipek));
	memset(iksn, 0x00, sizeof(iksn));
	memset(temp, 0x00, sizeof(temp));
	memset(buf1, 0x00, sizeof(buf1));
	memset(buf2, 0x00, sizeof(buf2));

	return(result);
}

// ---------------------------------------------------------------------------
// FUNCTION: load AES DUKPT initial keys.
// INPUT   : tid - terminal id. (8-byte string)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
//
//           TERM                  HOST
//           --------------        -------------
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+EIPEK(144)
//	         CMD+TSN(8)+TID(8) ->
//			                <-	   CMD+EIKSN(144)
//
//           where: EIPEK(144)	= EKB( IPEK(16/24/32), KPK )
//		            IPEK(16/24/32)	= Initial DUKPT Key
//                  EIKSN(144)	= EKB( IKSN(12), KPK )
//		            IKSN(12)	= Initial Key Serial Number
// ---------------------------------------------------------------------------
UINT8	PED_LoadAesDukptInitKey(UINT8 *tid)
{
    UINT8	eipek[X9143_AES_KEY_BUNDLE_LEN];	// encrypted initial PIN encryption key
	// UINT8	iksn[12];		// plaintext initial key serial number
    UINT8   eiksn[X9143_AES_KEY_BUNDLE_LEN];    // encrypted initial key serial number
	UINT8	temp[32];
	UINT8	buf1[32];
//	UINT8	buf2[128];
	UINT8	buf2[255];		// 2022-06-06
    UINT8   buf3[255];
    UINT8   dataLen;
	UINT8	result;


	// Terminal Serial Number (TSN)
	api_sys_info(SID_TerminalSerialNumber, temp);
	memmove(buf2, &temp[4], 8);

	// Terminal ID (TID)
	memmove(&buf2[8], tid, 8);

	memmove(&buf1[3], buf2, 16);
	buf1[0] = 17;
	buf1[1] = 0;
    buf1[2] = KMC_DOWNLOAD_AES_DUKPT_IPEK; // command

	// send CMD + TSN(8) + TID(8) to HOST for EIPEK(144)
	result = apiFailed;

	if(LIB_TransmitAUX(buf1) == TRUE)
	{
        if(LIB_ReceiveReadyAUX(buf3) == TRUE)
        {
            dataLen = buf3[0] + buf3[1] * 256;
            if(dataLen <= sizeof(buf2))
            {
                if(LIB_ReceiveAUX(buf2) == TRUE)
                {
                    if((buf2[0] == (X9143_AES_KEY_BUNDLE_LEN + 1)) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
                    {
                        memmove(eipek, &buf2[3], X9143_AES_KEY_BUNDLE_LEN);
                        result = apiOK;
                    }
                }
            }
        }
	}

	if(result == apiOK)
	{
		LIB_WaitTime(50);

		// Terminal Serial Number (TSN)
		api_sys_info(SID_TerminalSerialNumber, temp);
		memmove(buf2, &temp[4], 8);

		// Terminal ID (TID)
		memmove(&buf2[8], tid, 8);

		memmove(&buf1[3], buf2, 16);
		buf1[0] = 17;
		buf1[1] = 0;
        buf1[2] = KMC_DOWNLOAD_AES_DUKPT_IKSN; // command

		// send CMD + TSN(8) + TID(8) to HOST for IKSN(12)
		result = apiFailed;

		if(LIB_TransmitAUX(buf1) == TRUE)
		{
            if(LIB_ReceiveReadyAUX(buf3) == TRUE)
            {
                dataLen = buf3[0] + buf3[1] * 256;
                if(dataLen <= sizeof(buf2))
                {
                    if(LIB_ReceiveAUX(buf2) == TRUE)
                    {
                        // if((buf2[0] == 13) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
                        if((buf2[0] == (X9143_AES_KEY_BUNDLE_LEN + 1)) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
                        {
                            // memmove(iksn, &buf2[3], 12);
                            memmove(eiksn, &buf2[3], X9143_AES_KEY_BUNDLE_LEN);
                            result = apiOK;
                        }
                    }
                }
            }
		}
	}

	if(result == apiOK)
	{
		// setup DUKPT keys
		// result = PED_SetAESDUKPT(eipek, iksn);
        result = PED_SetAESDUKPT(eipek, eiksn);
	}

	if(result == apiOK)
	{
		if(PED_VerifyKPKStatus(AES_128) == TRUE)
        {
            PED_AES_DUKPT_SetKeyType(AES_128);
            PED_SetKPKStatus(AES_128, 0);
        }
        else if(PED_VerifyKPKStatus(AES_192) == TRUE)
        {
            PED_AES_DUKPT_SetKeyType(AES_192);
            PED_SetKPKStatus(AES_192, 0);
        }
        else if(PED_VerifyKPKStatus(AES_256) == TRUE)
        {
            PED_AES_DUKPT_SetKeyType(AES_256);
            PED_SetKPKStatus(AES_256, 0);
        }
	}
	
	memset(eipek, 0x00, sizeof(eipek));
	// memset(iksn, 0x00, sizeof(iksn));
    memset(eiksn, 0x00, sizeof(eiksn));
	memset(temp, 0x00, sizeof(temp));
	memset(buf1, 0x00, sizeof(buf1));
	memset(buf2, 0x00, sizeof(buf2));
    memset(buf3, 0x00, sizeof(buf3));

	return(result);
}

// ---------------------------------------------------------------------------
// FUNCTION: load Master/Session keys.
// INPUT   : tid - terminal id. (8-byte string)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
//
// Master Key
//           TERM                  HOST
//           --------------        -------------
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+EMK(80)
//           where: EMK(80) = EKB( MKEY(16), KPK )
//
// Session Keys
//           TERM                  HOST
//           --------------        -------------
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+NN+ESK(80)
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+NN+ESK(80)
//           ...
//           where: ESK(80)   = EKB( ESKEY(16), KPK )
//                  ESKEY(16) = TDES( SKEY(16), MKEY )
//                  NN        = Key Slot Number (0x01..nn)
// ---------------------------------------------------------------------------
UINT8	PED_LoadMasterSessionKey(UINT8 *tid)
{
	UINT8	temp[32];
	UINT8	buf1[32];
//	UINT8	buf2[PED_MSKEY_SLOT_LEN + 16];
	UINT8	buf2[255];	// 2022-06-06
	UINT8	nextblk;
	UINT8	result;
	UINT8	index;


	// Terminal Serial Number (TSN)
	api_sys_info(SID_TerminalSerialNumber, temp);
	memmove(buf2, &temp[4], 8);

	// Terminal ID (TID)
	memmove(&buf2[8], tid, 8);

	// --- Master Key ---
	memmove(&buf1[3], buf2, 16);
	buf1[0] = 17;
	buf1[1] = 0;
	buf1[2] = KMC_DOWNLOAD_MASTER_KEY; // command

	// send CMD + TSN(8) + TID(8) to HOST for download key
	result = apiFailed;
	index = 1; // default TMK slot

	if(LIB_TransmitAUX(buf1) == TRUE)
	{
		if(LIB_ReceiveAUX(buf2) == TRUE)
		{
			if((buf2[0] == (PED_MSKEY_SLOT_LEN + 1)) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
			{
			// verify & save key bundle
  //	      LIB_DumpHexData( 0, 0, PED_MSKEY_SLOT_LEN, &buf2[3] );
				result = PED_SetMasterKey(index - 1, PED_MSKEY_SLOT_LEN, &buf2[3]);
			}
		}
	}

	LIB_WaitTime(50);

	// --- Session Key ---
//    if( result == apiOK )
//		PED_EraseSessionKey();	// erase current keys

	nextblk = 0x00;

	while(result == apiOK)
	{
		// Terminal Serial Number (TSN)
		api_sys_info(SID_TerminalSerialNumber, temp);
		memmove(buf2, &temp[4], 8);

		// Terminal ID (TID)
		memmove(&buf2[8], tid, 8);

		memmove(&buf1[3], buf2, 16);
		buf1[0] = 17;
		buf1[1] = 0;
		buf1[2] = KMC_DOWNLOAD_SESSION_KEY | nextblk; // command

		// send CMD + TSN(8) + TID(8) to HOST for download key
		result = apiFailed;

		if(LIB_TransmitAUX(buf1) == TRUE)
		{
			if(LIB_ReceiveAUX(buf2) == TRUE)
			{
				if((buf2[0] == 1) && (buf2[1] == 0x00) && (buf2[2] == buf1[2])) // EOF?
				{
					result = apiOK;
					break;
				}

				if((buf2[0] == (PED_MSKEY_SLOT_LEN + 2)) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
				{
					index = buf2[3]; // key index
					if((index == 0) || (index > MAX_SKEY_CNT))
						break;

					// verify & save key bundle
					result = PED_SetSessionKey(index - 1, PED_MSKEY_SLOT_LEN, &buf2[4]);
					if(result == apiFailed)
						break;

					nextblk = KMC_NEXT;
				}
			}
		}
	} // while(1)

	memset(temp, 0x00, sizeof(temp));
	memset(buf1, 0x00, sizeof(buf1));
	memset(buf2, 0x00, sizeof(buf2));
	
	return(result);
}

// ---------------------------------------------------------------------------
// FUNCTION: load Master/Session keys for PIN Encryption Key (PEK).
// INPUT   : tid - terminal id. (8-byte string)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
//
// Master Key
//           TERM                  HOST
//           --------------        -------------
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+EMK(96/144)
//           where: EMK(96)  = EKB( MKEY(16/24), KPK ), if TDES key bundle is received
//                  EMK(144) = EKB( MKEY(16/24/32), KPK ), if AES key bundle is received
//
// Session Keys
//           TERM                  HOST
//           --------------        -------------
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+NN+ESK(96/144)
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+NN+ESK(96/144)
//           ...
//           where: ESK(96)   = EKB( ESKEY(16/24), MKEY as KPK ), if TDES key bundle is received
//                  ESK(144)  = EKB( ESKEY(16/24/32), MKEY as KPK ), if AES key bundle is received
//                  NN        = Key Slot Number (0x01..nn)
// ---------------------------------------------------------------------------
UINT8	PED_LoadPEKMasterSessionKey(UINT8 *tid)
{
	UINT8	temp[32];
	UINT8	buf1[32];
//	UINT8	buf2[PED_MSKEY_SLOT_LEN + 16];
	UINT8	buf2[255];	// 2022-06-06
    UINT8   buf3[255];
    UINT8   dataLen;
	UINT8	nextblk;
	UINT8	result;
	UINT8	index;
    UINT8   kpkAlgorithm = "";


	// Terminal Serial Number (TSN)
	api_sys_info(SID_TerminalSerialNumber, temp);
	memmove(buf2, &temp[4], 8);

	// Terminal ID (TID)
	memmove(&buf2[8], tid, 8);

	// --- Master Key ---
	memmove(&buf1[3], buf2, 16);
	buf1[0] = 17;
	buf1[1] = 0;
	buf1[2] = KMC_DOWNLOAD_PEK_MASTER_KEY; // command

	// send CMD + TSN(8) + TID(8) to HOST for download key
	result = apiFailed;
	index = 1; // default TMK slot

	if(LIB_TransmitAUX(buf1) == TRUE)
	{
        if(LIB_ReceiveReadyAUX(buf3) == TRUE)
        {
            dataLen = buf3[0] + buf3[1] * 256;
            if(dataLen <= sizeof(buf2))
            {
                if(LIB_ReceiveAUX(buf2) == TRUE)
                {
                    if((buf2[0] == (PED_PEK_TDES_MSKEY_SLOT_LEN + 1)) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
                    {
                        kpkAlgorithm = 'T';

                        // verify & save key bundle
                        result = PED_PEK_SetMasterKey(index - 1, PED_PEK_TDES_MSKEY_SLOT_LEN, &buf2[3]);
                    }
                    else if((buf2[0] == (PED_PEK_AES_MSKEY_SLOT_LEN + 1)) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
                    {
                        kpkAlgorithm = 'A';

                        // verify & save key bundle
                        result = PED_PEK_SetMasterKey(index - 1, PED_PEK_AES_MSKEY_SLOT_LEN, &buf2[3]);
                    }
                }
            }
        }
	}

	LIB_WaitTime(50);

	// --- Session Key ---
//    if( result == apiOK )
//		PED_EraseSessionKey();	// erase current keys

	nextblk = 0x00;

	while(result == apiOK)
	{
		// Terminal Serial Number (TSN)
		api_sys_info(SID_TerminalSerialNumber, temp);
		memmove(buf2, &temp[4], 8);

		// Terminal ID (TID)
		memmove(&buf2[8], tid, 8);

		memmove(&buf1[3], buf2, 16);
		buf1[0] = 17;
		buf1[1] = 0;
		buf1[2] = KMC_DOWNLOAD_PEK_SESSION_KEY | nextblk; // command

		// send CMD + TSN(8) + TID(8) to HOST for download key
		result = apiFailed;

		if(LIB_TransmitAUX(buf1) == TRUE)
		{
            if(LIB_ReceiveReadyAUX(buf3) == TRUE)
            {
                dataLen = buf3[0] + buf3[1] * 256;
                if(dataLen <= sizeof(buf2))
                {
                    if(LIB_ReceiveAUX(buf2) == TRUE)
                    {
                        if((buf2[0] == 1) && (buf2[1] == 0x00) && (buf2[2] == buf1[2])) // EOF?
                        {
                            result = apiOK;
                            break;
                        }

                        if((buf2[0] == (PED_PEK_TDES_MSKEY_SLOT_LEN + 2)) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
                        {
                            index = buf2[3]; // key index
                            if((index == 0) || (index > MAX_PEK_SKEY_CNT))
                                break;

                            // verify & save key bundle
                            result = PED_PEK_SetSessionKey(index - 1, PED_PEK_TDES_MSKEY_SLOT_LEN, &buf2[4]);
                            if(result == apiFailed)
                                break;

                            nextblk = KMC_NEXT;
                        }
                        else if((buf2[0] == (PED_PEK_AES_MSKEY_SLOT_LEN + 2)) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
                        {
                            index = buf2[3]; // key index
                            if((index == 0) || (index > MAX_PEK_SKEY_CNT))
                                break;

                            // verify & save key bundle
                            result = PED_PEK_SetSessionKey(index - 1, PED_PEK_AES_MSKEY_SLOT_LEN, &buf2[4]);
                            if(result == apiFailed)
                                break;

                            nextblk = KMC_NEXT;
                        }
                    }
                }
            }
		}
	} // while(1)

    if((result == apiOK) && (kpkAlgorithm == 'T'))
    {
        if(PED_VerifyKPKStatus(TDES_128) == TRUE)
        {
            PED_PEK_SetKeyType(TDES_128);
            PED_SetKPKStatus(TDES_128, 0);
        }
        else if(PED_VerifyKPKStatus(TDES_192) == TRUE)
        {
            PED_PEK_SetKeyType(TDES_192);
            PED_SetKPKStatus(TDES_192, 0);
        }
    }
    else if((result == apiOK) && (kpkAlgorithm == 'A'))
    {
        if(PED_VerifyKPKStatus(AES_128) == TRUE)
        {
            PED_PEK_SetKeyType(AES_128);
            PED_SetKPKStatus(AES_128, 0);
        }
        else if(PED_VerifyKPKStatus(AES_192) == TRUE)
        {
            PED_PEK_SetKeyType(AES_192);
            PED_SetKPKStatus(AES_192, 0);
        }
        else if(PED_VerifyKPKStatus(AES_256) == TRUE)
        {
            PED_PEK_SetKeyType(AES_256);
            PED_SetKPKStatus(AES_256, 0);
        }
    }

	memset(temp, 0x00, sizeof(temp));
	memset(buf1, 0x00, sizeof(buf1));
	memset(buf2, 0x00, sizeof(buf2));
    memset(buf3, 0x00, sizeof(buf3));
	
	return(result);
}

// ---------------------------------------------------------------------------
// FUNCTION: load Master/Session keys for ISO format 4 app.
// INPUT   : tid - terminal id. (8-byte string)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
//
// Master Key
//           TERM                  HOST
//           --------------        -------------
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+EMK(112)
//           where: EMK(112) = EKB( MKEY(16), KPK )
//
// Session Keys
//           TERM                  HOST
//           --------------        -------------
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+NN+ESK(112)
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+NN+ESK(112)
//           ...
//           where: ESK(112)  = EKB( ESKEY(16), KPK )
//                  ESKEY(16) = TDES( SKEY(16), MKEY )
//                  NN        = Key Slot Number (0x01..nn)
// ---------------------------------------------------------------------------
#if 0
UINT8	PED_LoadISO4MasterSessionKey(UINT8 *tid)
{
	UINT8	temp[32];
	UINT8	buf1[32];
	UINT8	buf2[PED_ISO4_KEY_MSKEY_SLOT_LEN + 16];
	UINT8	nextblk;
	UINT8	result;
	UINT8	index;


	// Terminal Serial Number (TSN)
	api_sys_info(SID_TerminalSerialNumber, temp);
	memmove(buf2, &temp[4], 8);

	// Terminal ID (TID)
	memmove(&buf2[8], tid, 8);

	// --- Master Key ---
	memmove(&buf1[3], buf2, 16);
	buf1[0] = 17;
	buf1[1] = 0;
	buf1[2] = KMC_DOWNLOAD_ISO4_MASTER_KEY; // command

	// send CMD + TSN(8) + TID(8) to HOST for download key
	result = apiFailed;
	index = 1; // default TMK slot

	if(LIB_TransmitAUX(buf1) == TRUE)
	{
		if(LIB_ReceiveAUX(buf2) == TRUE)
		{
			if((buf2[0] == (PED_ISO4_KEY_MSKEY_SLOT_LEN + 1)) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
			{
				// verify & save key bundle
	  //	      LIB_DumpHexData( 0, 0, PED_ISO4_KEY_MSKEY_SLOT_LEN, &buf2[3] );
				result = PED_ISO4_SetMasterKey(index - 1, PED_ISO4_KEY_MSKEY_SLOT_LEN, &buf2[3]);
			}
		}
	}

	LIB_WaitTime(50);

	// --- Session Key ---
	nextblk = 0x00;

	while(result == apiOK)
	{
		// Terminal Serial Number (TSN)
		api_sys_info(SID_TerminalSerialNumber, temp);
		memmove(buf2, &temp[4], 8);

		// Terminal ID (TID)
		memmove(&buf2[8], tid, 8);

		memmove(&buf1[3], buf2, 16);
		buf1[0] = 17;
		buf1[1] = 0;
		buf1[2] = KMC_DOWNLOAD_ISO4_SESSION_KEY | nextblk; // command

		// send CMD + TSN(8) + TID(8) to HOST for download key
		result = apiFailed;

		if(LIB_TransmitAUX(buf1) == TRUE)
		{
			if(LIB_ReceiveAUX(buf2) == TRUE)
			{
				if((buf2[0] == 1) && (buf2[1] == 0x00) && (buf2[2] == buf1[2])) // EOF?
				{
					result = apiOK;
					break;
				}

				if((buf2[0] == (PED_ISO4_KEY_MSKEY_SLOT_LEN + 2)) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
				{
					index = buf2[3]; // key index
					if((index == 0) || (index > MAX_ISO4_KEY_SKEY_CNT))
						break;
					
					// verify & save key bundle
					result = PED_ISO4_SetSessionKey(index - 1, PED_ISO4_KEY_MSKEY_SLOT_LEN, &buf2[4]);
					if(result == apiFailed)
						break;

					nextblk = KMC_NEXT;
				}
			}
		}
	} // while(1)

	memset(temp, 0x00, sizeof(temp));
	memset(buf1, 0x00, sizeof(buf1));
	memset(buf2, 0x00, sizeof(buf2));
	
	return(result);
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: load ISO4_KEY for ISO9564-1:2017 format 4.
// INPUT   : tid - terminal id. (8-byte string)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
//
// Session Keys
//           TERM                  HOST
//           --------------        -------------
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+NN+ESK(112)
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+NN+ESK(112)
//           ...
//           where: ESK(112)  = EKB( SKEY(16), KPK )
//                  NN        = Key Slot Number (0x01..nn)
// ---------------------------------------------------------------------------
UINT8	PED_LoadISO4KEY(UINT8 *tid)
{
	UINT8	temp[32];
	UINT8	buf1[32];
//	UINT8	buf2[PED_ISO4_KEY_MSKEY_SLOT_LEN + 16];
	UINT8	buf2[255];	// 2022-06-06
	UINT8	result;


	// Terminal Serial Number (TSN)
	api_sys_info(SID_TerminalSerialNumber, temp);
	memmove(buf2, &temp[4], 8);

	// Terminal ID (TID)
	memmove(&buf2[8], tid, 8);

	memmove(&buf1[3], buf2, 16);
	buf1[0] = 17;
	buf1[1] = 0;
	buf1[2] = KMC_DOWNLOAD_ISO4_KEY; // command

	// send CMD + TSN(8) + TID(8) to HOST for download key
	result = apiFailed;

	if(LIB_TransmitAUX(buf1) == TRUE)
	{
		if(LIB_ReceiveAUX(buf2) == TRUE)
		{
			if((buf2[0] == (PED_ISO4_KEY_SLOT_LEN + 1)) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
			{
				// verify & save key bundle
				result = PED_SetISO4KEY(PED_ISO4_KEY_SLOT_LEN, &buf2[3]);
			}
		}
	}

	// clear sensitive data
	memset(temp, 0x00, sizeof(temp));
	memset(buf1, 0x00, sizeof(buf1));
	memset(buf2, 0x00, sizeof(buf2));
	
	return(result);
}

// ---------------------------------------------------------------------------
// FUNCTION: load ACC_DEK Master/Session keys.
// INPUT   : tid - terminal id. (8-byte string)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
//
// Master Key
//           TERM                  HOST
//           --------------        -------------
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+EMK(80)
//           where: EMK(80) = EKB( MKEY(16), KPK )
//
// Session Keys
//           TERM                  HOST
//           --------------        -------------
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+NN+ESK(80)
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+NN+ESK(80)
//           ...
//           where: ESK(80)   = EKB( ESKEY(16), KPK )
//                  ESKEY(16) = TDES( SKEY(16), MKEY )
//                  NN        = Key Slot Number (0x01..nn)
// ---------------------------------------------------------------------------
/*
UINT8	PED_LoadAccDEKMasterSessionKey(UINT8 *tid)
{
	UINT8	temp[32];
	UINT8	buf1[32];
//	UINT8	buf2[PED_MSKEY_SLOT_LEN + 16];
	UINT8	buf2[255];	// 2022-06-06
	UINT8	nextblk;
	UINT8	result;
	UINT8	index;


	// Terminal Serial Number (TSN)
	api_sys_info(SID_TerminalSerialNumber, temp);
	memmove(buf2, &temp[4], 8);

	// Terminal ID (TID)
	memmove(&buf2[8], tid, 8);

	// --- Master Key ---
	memmove(&buf1[3], buf2, 16);
	buf1[0] = 17;
	buf1[1] = 0;
	buf1[2] = KMC_DOWNLOAD_ACC_DEK_MASTER_KEY; // command

	// send CMD + TSN(8) + TID(8) to HOST for download key
	result = apiFailed;
	index = 1; // default TMK slot

	if(LIB_TransmitAUX(buf1) == TRUE)
	{
		if(LIB_ReceiveAUX(buf2) == TRUE)
		{
			if((buf2[0] == (PED_ACC_DEK_MSKEY_SLOT_LEN + 1)) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
			{
				// verify & save key bundle
	  //	      LIB_DumpHexData( 0, 0, PED_ACC_DEK_MSKEY_SLOT_LEN, &buf2[3] );
				result = PED_AccDEK_SetMasterKey(index - 1, PED_ACC_DEK_MSKEY_SLOT_LEN, &buf2[3]);
			}
		}
	}

	LIB_WaitTime(50);

	// --- Session Key ---
	nextblk = 0x00;

	while(result == apiOK)
	{
		// Terminal Serial Number (TSN)
		api_sys_info(SID_TerminalSerialNumber, temp);
		memmove(buf2, &temp[4], 8);

		// Terminal ID (TID)
		memmove(&buf2[8], tid, 8);

		memmove(&buf1[3], buf2, 16);
		buf1[0] = 17;
		buf1[1] = 0;
		buf1[2] = KMC_DOWNLOAD_ACC_DEK_SESSION_KEY | nextblk; // command

		// send CMD + TSN(8) + TID(8) to HOST for download key
		result = apiFailed;

		if(LIB_TransmitAUX(buf1) == TRUE)
		{
			if(LIB_ReceiveAUX(buf2) == TRUE)
			{
				if((buf2[0] == 1) && (buf2[1] == 0x00) && (buf2[2] == buf1[2])) // EOF?
				{
					result = apiOK;
					break;
				}

				if((buf2[0] == (PED_ACC_DEK_MSKEY_SLOT_LEN + 2)) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
				{
					index = buf2[3]; // key index
					if((index == 0) || (index > MAX_ACC_DEK_SKEY_CNT))
						break;

					// verify & save key bundle
					result = PED_AccDEK_SetSessionKey(index - 1, PED_ACC_DEK_MSKEY_SLOT_LEN, &buf2[4]);
					if(result == apiFailed)
						break;

					nextblk = KMC_NEXT;
				}
			}
		}
	} // while(1)

	memset(temp, 0x00, sizeof(temp));
	memset(buf1, 0x00, sizeof(buf1));
	memset(buf2, 0x00, sizeof(buf2));
	
	return(result);
}
*/

// ---------------------------------------------------------------------------
// FUNCTION: load ACC_DEK Master/Session keys.
// INPUT   : tid - terminal id. (8-byte string)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
//
// Master Key
//           TERM                  HOST
//           --------------        -------------
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+EMK(96/144)
//           where: EMK(96) = EKB( MKEY(24), KPK ), if TDES key bundle is received
//                  EMK(144) = EKB( MKEY(16/24/32), KPK ), if AES key bundle is received
//
// Session Keys
//           TERM                  HOST
//           --------------        -------------
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+NN+ESK(96/144)
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+NN+ESK(96/144)
//           ...
//           where: ESK(96)   = EKB( ESKEY(16/24), MKEY as KPK ), if TDES key bundle is received
//                  ESK(144)  = EKB( ESKEY(16/24/32), MKEY as KPK ), if AES key bundle is received
//                  NN        = Key Slot Number (0x01..nn)
// ---------------------------------------------------------------------------
UINT8	PED_LoadAccDEKMasterSessionKey(UINT8 *tid)
{
	UINT8	temp[32];
	UINT8	buf1[32];
//	UINT8	buf2[PED_MSKEY_SLOT_LEN + 16];
	UINT8	buf2[255];	// 2022-06-06
    UINT8   buf3[255];
    UINT8   dataLen;
	UINT8	nextblk;
	UINT8	result;
	UINT8	index;
    UINT8   kpkAlgorithm = "";


	// Terminal Serial Number (TSN)
	api_sys_info(SID_TerminalSerialNumber, temp);
	memmove(buf2, &temp[4], 8);

	// Terminal ID (TID)
	memmove(&buf2[8], tid, 8);

	// --- Master Key ---
	memmove(&buf1[3], buf2, 16);
	buf1[0] = 17;
	buf1[1] = 0;
	buf1[2] = KMC_DOWNLOAD_ACC_DEK_MASTER_KEY; // command

	// send CMD + TSN(8) + TID(8) to HOST for download key
	result = apiFailed;
	index = 1; // default TMK slot

	if(LIB_TransmitAUX(buf1) == TRUE)
	{
        if(LIB_ReceiveReadyAUX(buf3) == TRUE)
        {
            dataLen = buf3[0] + buf3[1] * 256;
            if(dataLen <= sizeof(buf2))
            {
                if(LIB_ReceiveAUX(buf2) == TRUE)
                {
                    if((buf2[0] == (PED_ACC_DEK_TDES_MSKEY_SLOT_LEN + 1)) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
                    {
                        kpkAlgorithm = 'T';

                        // verify & save key bundle
            //	      LIB_DumpHexData( 0, 0, PED_ACC_DEK_TDES_MSKEY_SLOT_LEN, &buf2[3] );
                        result = PED_AccDEK_SetMasterKey(index - 1, PED_ACC_DEK_TDES_MSKEY_SLOT_LEN, &buf2[3]);
                    }
                    else if((buf2[0] == (PED_ACC_DEK_AES_MSKEY_SLOT_LEN + 1)) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
                    {
                        kpkAlgorithm = 'A';

                        // verify & save key bundle
            //	      LIB_DumpHexData( 0, 0, PED_ACC_DEK_AES_MSKEY_SLOT_LEN, &buf2[3] );
                        result = PED_AccDEK_SetMasterKey(index - 1, PED_ACC_DEK_AES_MSKEY_SLOT_LEN, &buf2[3]);
                    }
                }
            }
        }
	}

	LIB_WaitTime(50);

	// --- Session Key ---
	nextblk = 0x00;

	while(result == apiOK)
	{
		// Terminal Serial Number (TSN)
		api_sys_info(SID_TerminalSerialNumber, temp);
		memmove(buf2, &temp[4], 8);

		// Terminal ID (TID)
		memmove(&buf2[8], tid, 8);

		memmove(&buf1[3], buf2, 16);
		buf1[0] = 17;
		buf1[1] = 0;
		buf1[2] = KMC_DOWNLOAD_ACC_DEK_SESSION_KEY | nextblk; // command

		// send CMD + TSN(8) + TID(8) to HOST for download key
		result = apiFailed;

		if(LIB_TransmitAUX(buf1) == TRUE)
		{
            if(LIB_ReceiveReadyAUX(buf3) == TRUE)
            {
                dataLen = buf3[0] + buf3[1] * 256;
                if(dataLen <= sizeof(buf2))
                {
                    if(LIB_ReceiveAUX(buf2) == TRUE)
                    {
                        if((buf2[0] == 1) && (buf2[1] == 0x00) && (buf2[2] == buf1[2])) // EOF?
                        {
                            result = apiOK;
                            break;
                        }

                        if((buf2[0] == (PED_ACC_DEK_TDES_MSKEY_SLOT_LEN + 2)) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
                        {
                            index = buf2[3]; // key index
                            if((index == 0) || (index > MAX_ACC_DEK_SKEY_CNT))
                                break;

                            // verify & save key bundle
                            result = PED_AccDEK_SetSessionKey(index - 1, PED_ACC_DEK_TDES_MSKEY_SLOT_LEN, &buf2[4]);
                            if(result == apiFailed)
                                break;

                            nextblk = KMC_NEXT;
                        }
                        else if((buf2[0] == (PED_ACC_DEK_AES_MSKEY_SLOT_LEN + 2)) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
                        {
                            index = buf2[3]; // key index
                            if((index == 0) || (index > MAX_ACC_DEK_SKEY_CNT))
                                break;

                            // verify & save key bundle
                            result = PED_AccDEK_SetSessionKey(index - 1, PED_ACC_DEK_AES_MSKEY_SLOT_LEN, &buf2[4]);
                            if(result == apiFailed)
                                break;

                            nextblk = KMC_NEXT;
                        }
                    }
                }
            }
		}
	} // while(1)

    if((result == apiOK) && (kpkAlgorithm == 'T'))
    {
        PED_AccDEK_SetKeyType(TDES_192);
        PED_SetKPKStatus(TDES_192, 0);
    }
    else if((result == apiOK) && (kpkAlgorithm == 'A'))
    {
        if(PED_VerifyKPKStatus(AES_128) == TRUE)
        {
            PED_AccDEK_SetKeyType(AES_128);
            PED_SetKPKStatus(AES_128, 0);
        }
        else if(PED_VerifyKPKStatus(AES_192) == TRUE)
        {
            PED_AccDEK_SetKeyType(AES_192);
            PED_SetKPKStatus(AES_192, 0);
        }
        else if(PED_VerifyKPKStatus(AES_256) == TRUE)
        {
            PED_AccDEK_SetKeyType(AES_256);
            PED_SetKPKStatus(AES_256, 0);
        }
    }
    
	memset(temp, 0x00, sizeof(temp));
	memset(buf1, 0x00, sizeof(buf1));
	memset(buf2, 0x00, sizeof(buf2));
    memset(buf3, 0x00, sizeof(buf3));
	
	return(result);
}

// ---------------------------------------------------------------------------
// FUNCTION: load Master/Session keys for Format-Preserving Encryption Key.
// INPUT   : tid - terminal id. (8-byte string)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
//
// Master Key
//           TERM                  HOST
//           --------------        -------------
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+EMK(112)
//           where: EMK(112) = EKB( MKEY(16), KPK )
//
// Session Keys
//           TERM                  HOST
//           --------------        -------------
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+NN+ESK(112)
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+NN+ESK(112)
//           ...
//           where: ESK(112)  = EKB( ESKEY(16), KPK )
//                  ESKEY(16) = TDES( SKEY(16), MKEY )
//                  NN        = Key Slot Number (0x01..nn)
// ---------------------------------------------------------------------------
/*
UINT8	PED_LoadFPEMasterSessionKey(UINT8 *tid)
{
	UINT8	temp[32];
	UINT8	buf1[32];
	UINT8	buf2[PED_FPE_KEY_MSKEY_SLOT_LEN + 16];
	UINT8	nextblk;
	UINT8	result;
	UINT8	index;


	// Terminal Serial Number (TSN)
	api_sys_info(SID_TerminalSerialNumber, temp);
	memmove(buf2, &temp[4], 8);

	// Terminal ID (TID)
	memmove(&buf2[8], tid, 8);

	// --- Master Key ---
	memmove(&buf1[3], buf2, 16);
	buf1[0] = 17;
	buf1[1] = 0;
	buf1[2] = KMC_DOWNLOAD_FPE_MASTER_KEY; // command

	// send CMD + TSN(8) + TID(8) to HOST for download key
	result = apiFailed;
	index = 1; // default TMK slot

	if(LIB_TransmitAUX(buf1) == TRUE)
	{
		if(LIB_ReceiveAUX(buf2) == TRUE)
		{
			if((buf2[0] == (PED_FPE_KEY_MSKEY_SLOT_LEN + 1)) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
			{
				// verify & save key bundle
	  //	      LIB_DumpHexData( 0, 0, PED_FPE_KEY_MSKEY_SLOT_LEN, &buf2[3] );
				result = PED_FPE_SetMasterKey(index - 1, PED_FPE_KEY_MSKEY_SLOT_LEN, &buf2[3]);
			}
		}
	}

	LIB_WaitTime(50);

	// --- Session Key ---
	nextblk = 0x00;

	while(result == apiOK)
	{
		// Terminal Serial Number (TSN)
		api_sys_info(SID_TerminalSerialNumber, temp);
		memmove(buf2, &temp[4], 8);

		// Terminal ID (TID)
		memmove(&buf2[8], tid, 8);

		memmove(&buf1[3], buf2, 16);
		buf1[0] = 17;
		buf1[1] = 0;
		buf1[2] = KMC_DOWNLOAD_FPE_SESSION_KEY | nextblk; // command

		// send CMD + TSN(8) + TID(8) to HOST for download key
		result = apiFailed;

		if(LIB_TransmitAUX(buf1) == TRUE)
		{
			if(LIB_ReceiveAUX(buf2) == TRUE)
			{
				if((buf2[0] == 1) && (buf2[1] == 0x00) && (buf2[2] == buf1[2])) // EOF?
				{
					result = apiOK;
					break;
				}

				if((buf2[0] == (PED_FPE_KEY_MSKEY_SLOT_LEN + 2)) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
				{
					index = buf2[3]; // key index
					if((index == 0) || (index > MAX_FPE_KEY_SKEY_CNT))
						break;

					// verify & save key bundle
					result = PED_FPE_SetSessionKey(index - 1, PED_FPE_KEY_MSKEY_SLOT_LEN, &buf2[4]);
					if(result == apiFailed)
						break;

					nextblk = KMC_NEXT;
				}
			}
		}
	} // while(1)

	// clear sensitive data
	memset(temp, 0x00, sizeof(temp));
	memset(buf1, 0x00, sizeof(buf1));
	memset(buf2, 0x00, sizeof(buf2));
	
	return(result);
}
*/

// ---------------------------------------------------------------------------
// FUNCTION: load Master/Session keys for Format-Preserving Encryption Key.
// INPUT   : tid - terminal id. (8-byte string)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
//
// Master Key
//           TERM                  HOST
//           --------------        -------------
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+EMK(144)
//           where: EMK(144) = EKB( MKEY(16), KPK )
//
// Session Keys
//           TERM                  HOST
//           --------------        -------------
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+NN+ESK(144)
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+NN+ESK(144)
//           ...
//           where: ESK(144)  = EKB( ESKEY(16), MKEY as KPK )
//                  NN        = Key Slot Number (0x01..nn)
// ---------------------------------------------------------------------------
UINT8	PED_LoadFPEMasterSessionKey(UINT8 *tid)
{
	UINT8	temp[32];
	UINT8	buf1[32];
	//UINT8	buf2[PED_FPE_KEY_MSKEY_SLOT_LEN + 16];
    UINT8	buf2[255];
    UINT8   buf3[255];
    UINT8   dataLen;
	UINT8	nextblk;
	UINT8	result;
	UINT8	index;


	// Terminal Serial Number (TSN)
	api_sys_info(SID_TerminalSerialNumber, temp);
	memmove(buf2, &temp[4], 8);

	// Terminal ID (TID)
	memmove(&buf2[8], tid, 8);

	// --- Master Key ---
	memmove(&buf1[3], buf2, 16);
	buf1[0] = 17;
	buf1[1] = 0;
	buf1[2] = KMC_DOWNLOAD_FPE_MASTER_KEY; // command

	// send CMD + TSN(8) + TID(8) to HOST for download key
	result = apiFailed;
	index = 1; // default TMK slot

	if(LIB_TransmitAUX(buf1) == TRUE)
	{
        if(LIB_ReceiveReadyAUX(buf3) == TRUE)
        {
            dataLen = buf3[0] + buf3[1] * 256;
            if(dataLen <= sizeof(buf2))
            {
                if(LIB_ReceiveAUX(buf2) == TRUE)
                {
                    if((buf2[0] == (PED_FPE_KEY_AES_MSKEY_SLOT_LEN + 1)) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
                    {
                        // verify & save key bundle
            //	      LIB_DumpHexData( 0, 0, PED_FPE_KEY_MSKEY_SLOT_LEN, &buf2[3] );
                        result = PED_FPE_SetMasterKey(index - 1, PED_FPE_KEY_AES_MSKEY_SLOT_LEN, &buf2[3]);
                    }
                }
            }
        }
	}

	LIB_WaitTime(50);

	// --- Session Key ---
	nextblk = 0x00;

	while(result == apiOK)
	{
		// Terminal Serial Number (TSN)
		api_sys_info(SID_TerminalSerialNumber, temp);
		memmove(buf2, &temp[4], 8);

		// Terminal ID (TID)
		memmove(&buf2[8], tid, 8);

		memmove(&buf1[3], buf2, 16);
		buf1[0] = 17;
		buf1[1] = 0;
		buf1[2] = KMC_DOWNLOAD_FPE_SESSION_KEY | nextblk; // command

		// send CMD + TSN(8) + TID(8) to HOST for download key
		result = apiFailed;

		if(LIB_TransmitAUX(buf1) == TRUE)
		{
            if(LIB_ReceiveReadyAUX(buf3) == TRUE)
            {
                dataLen = buf3[0] + buf3[1] * 256;
                if(dataLen <= sizeof(buf2))
                {
                    if(LIB_ReceiveAUX(buf2) == TRUE)
                    {
                        if((buf2[0] == 1) && (buf2[1] == 0x00) && (buf2[2] == buf1[2])) // EOF?
                        {
                            result = apiOK;
                            break;
                        }

                        if((buf2[0] == (PED_FPE_KEY_AES_MSKEY_SLOT_LEN + 2)) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
                        {
                            index = buf2[3]; // key index
                            if((index == 0) || (index > MAX_FPE_KEY_SKEY_CNT))
                                break;

                            // verify & save key bundle
                            result = PED_FPE_SetSessionKey(index - 1, PED_FPE_KEY_AES_MSKEY_SLOT_LEN, &buf2[4]);
                            if(result == apiFailed)
                                break;

                            nextblk = KMC_NEXT;
                        }
                    }
                }
            }
		}
	} // while(1)

    if(result == apiOK)
    {
        PED_FPE_SetKeyType(AES_128);
        PED_SetKPKStatus(AES_128, 0);
    }

	// clear sensitive data
	memset(temp, 0x00, sizeof(temp));
	memset(buf1, 0x00, sizeof(buf1));
	memset(buf2, 0x00, sizeof(buf2));
    memset(buf3, 0x00, sizeof(buf3));
	
	return(result);
}

// ---------------------------------------------------------------------------
// FUNCTION: load Format-Preserving Encryption Key (FPE Key).
// INPUT   : tid - terminal id. (8-byte string)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
//
// Session Keys
//           TERM                  HOST
//           --------------        -------------
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+NN+ESK(112)
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+NN+ESK(112)
//           ...
//           where: ESK(112)  = EKB( SKEY(16), KPK )
//                  NN        = Key Slot Number (0x01..nn)
// ---------------------------------------------------------------------------
UINT8	PED_LoadFPEKey(UINT8 *tid)
{
	UINT8	temp[32];
	UINT8	buf1[32];
//	UINT8	buf2[PED_FPE_KEY_MSKEY_SLOT_LEN + 16];
	UINT8	buf2[255];	// 2022-06-06
	UINT8	result;


	// Terminal Serial Number (TSN)
	api_sys_info(SID_TerminalSerialNumber, temp);
	memmove(buf2, &temp[4], 8);

	// Terminal ID (TID)
	memmove(&buf2[8], tid, 8);

	memmove(&buf1[3], buf2, 16);
	buf1[0] = 17;
	buf1[1] = 0;
	buf1[2] = KMC_DOWNLOAD_FPE_KEY; // command

	// send CMD + TSN(8) + TID(8) to HOST for download key
	result = apiFailed;

	if(LIB_TransmitAUX(buf1) == TRUE)
	{
		if(LIB_ReceiveAUX(buf2) == TRUE)
		{
			if((buf2[0] == (PED_FPE_KEY_SLOT_LEN + 1)) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
			{
				// verify & save key bundle
				result = PED_SetFPEKey(PED_FPE_KEY_SLOT_LEN, &buf2[3]);
			}
		}
	}

	// clear sensitive data
	memset(temp, 0x00, sizeof(temp));
	memset(buf1, 0x00, sizeof(buf1));
	memset(buf2, 0x00, sizeof(buf2));
	
	return(result);
}

// ---------------------------------------------------------------------------
// FUNCTION: load Fixed key.
// INPUT   : tid - terminal id. (8-byte string)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
//
//           TERM                  HOST
//           --------------        -------------
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+NN+EFK(72)
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+NN+EFK(72)
//	     ...
//           where: EFK(72) = EKB( FKEY(16), KPK )
//                  NN        = Key Slot Number (0x01..nn)
// ---------------------------------------------------------------------------
#if	0
UINT8	PED_LoadFixedKey(UINT8 *tid)
{
#ifdef	_USE_IRAM_PARA_
	// 2014-11-24, allocate sensitive data buffer on MCU internal SRAM
#define	TEMP_SIZE_LFXK		32
#define	TEMP_ADDR_LFXK		0x0000F000	// pointer to MCU internal SRAM
#define	BUF1_SIZE_LFXK		32
#define	BUF1_ADDR_LFXK		TEMP_ADDR_LFXK+TEMP_SIZE_LFXK
#define	BUF2_SIZE_LFXK		PED_FKEY_SLOT_LEN+8
#define	BUF2_ADDR_LFXK		BUF1_ADDR_LFXK+BUF1_SIZE_LFXK

	UINT8	*temp = (UINT8 *)TEMP_ADDR_LFXK;
	UINT8	*buf1 = (UINT8 *)BUF1_ADDR_LFXK;
	UINT8	*buf2 = (UINT8 *)BUF2_ADDR_LFXK;
#else
	UINT8	temp[32];
	UINT8	buf1[32];
	UINT8	buf2[PED_FKEY_SLOT_LEN + 16];
#endif

	UINT8	result;
	UINT8	index;
	UINT8	nextblk;


	// init communication port
	if(LIB_OpenAUX(COM0, auxDLL, COM_9600) == FALSE)
		return(apiFailed); // device error

//	PED_EraseFXKEY( 0xFF ); // erase current keys

	result = apiOK;
	nextblk = 0x00;

	while(result == apiOK)
	{
		// Terminal Serial Number (TSN)
		api_sys_info(SID_TerminalSerialNumber, temp);
		memmove(buf2, &temp[4], 8);

		// Terminal ID (TID)
		memmove(&buf2[8], tid, 8);

		// --- Fixed Key ---
		memmove(&buf1[3], buf2, 16);
		buf1[0] = 17;
		buf1[1] = 0;
		buf1[2] = KMC_DOWNLOAD_FIXED_KEY | nextblk; // command

		// send CMD + TSN(8) + TID(8) to HOST for download key        
		result = apiFailed;

		if(LIB_TransmitAUX(buf1) == TRUE)
		{
			if(LIB_ReceiveAUX(buf2) == TRUE)
			{
				if((buf2[0] == 1) && (buf2[1] == 0x00) && (buf2[2] == buf1[2])) // EOF?
				{
					result = apiOK;
					break;
				}				

				if((buf2[0] == (PED_FKEY_SLOT_LEN + 2)) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]))
				{
					index = buf2[3]; // check key index
					if((index == 0) || (index > MAX_FKEY_CNT))
						break;

					// verify & save key bundle
 //	           LIB_DumpHexData( 0, 0, PED_FKEY_SLOT_LEN, &buf2[4] );
					result = PED_SetFixedKey(index - 1, PED_FKEY_SLOT_LEN, &buf2[4]);
					if(result == apiFailed)
						break;

					nextblk = KMC_NEXT;
				}
			}
		}
	}

	if(result == apiFailed)
	{
		// erase KEK
		PED_EraseKEK();
	}

	LIB_CloseAUX();

	// PATCH: 2009-04-07, clear sensitive data
#ifdef	_USE_IRAM_PARA_
	memset(temp, 0x00, sizeof(UINT8)*TEMP_SIZE_LFXK);
	memset(buf1, 0x00, sizeof(UINT8)*BUF1_SIZE_LFXK);
	memset(buf2, 0x00, sizeof(UINT8)*BUF2_SIZE_LFXK);
#else
	memset(temp, 0x00, sizeof(temp));
	memset(buf1, 0x00, sizeof(buf1));
	memset(buf2, 0x00, sizeof(buf2));
#endif
	return(result);
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: load CA public key from host simulator.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
//
//           TERM                  HOST
//           --------------        -------------
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+LEN(2)+CAPK(1) ...
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD+LEN(2)+CAPK(n) ...
//           CMD+TSN(8)+TID(8) ->
//                          <-     CMD
//
//           Note : If LEN(2) = 0, then the process is terminated.
//		     LEN(2) = HI-LO
//           Where:
//           CAPK = RID[5]	         +
//                  TestFlag[1]      +
//                  Index[1]         +
//                  ExpLen[1]        +
//                  Exp[~3]          +
//                  ModLen[1]        +
//                  Mod[~248]        +
//                  Hash[20]		// HASH=SHA1(RID+INDEX+MOD+EXP)
//
//	     CAPK Structure in SRAM (fixed 300 bytes per key slot)
//		-------------------------	-------------
//		Key Components			    Size in bytes
//		-------------------------	-------------
//		RID				            5
//		INDEX				        1
//		EXPONENT LENGTH			    1
//		MODULUS LENGTH			    2
//		HASH				        20
//		EXPONENT			        3
//		MODULUS				        256
//		HASH ALGORITHM INDICATOR	1
//		PK ALGORITHM INDICATOR		1
//		RFU				            10
//		-------------------------	-------------
//
// ---------------------------------------------------------------------------
UINT8	PED_LoadCaPublicKey(UINT8 *tid)
{
	UINT8	temp[CAPK_KEY_SLOT_LEN];
	UINT8	capk[CAPK_KEY_SLOT_LEN];
	UINT8	buf1[32];
	UINT8	buf2[32];
    UINT8   buf3[CAPK_KEY_SLOT_LEN];
    UINT8   dataLen;
	UINT8	result;
	UINT32	iLen;
	UINT32	iAddr;
	UINT8	nextblk;
	UINT32	nextframe;
	UINT32	offset;
	UINT32	capklen;
	UINT32	KeyReady;
	UINT8	explen;
	UINT8	modlen;
	UINT8	*pExp;
	UINT8	*pMod;
	UINT8	*pHash;
	UINT32	index;


	// Terminal Serial Number (TSN)
	api_sys_info(SID_TerminalSerialNumber, temp);
	memmove(buf2, &temp[4], 8);

	// Terminal ID (TID)
	memmove(&buf2[8], tid, 8);

	// --- CAPK ---

	iAddr = 0x00;
	nextblk = 0x00;
	result = apiOK;

	nextframe = 0;
	offset = 0;
	KeyReady = 0;
	index = 0;

	PED_EraseCAPK();  // erase current CAPKs

	while(result == apiOK)
	{
		result = apiFailed;

		memmove(&buf1[3], buf2, 16);
		buf1[0] = 17;
		buf1[1] = 0;
		buf1[2] = KMC_DOWNLOAD_CAPK | nextblk; // command

		// send CMD + TSN(8) + TID(8) to HOST for download key
		iLen = 0;
		if(LIB_TransmitAUX(buf1) == TRUE)
		{
            if(LIB_ReceiveReadyAUX(buf3) == TRUE)
            {
                dataLen = buf3[0] + buf3[1] * 256;
                if(dataLen <= sizeof(temp))
                {
                    if(LIB_ReceiveAUX(temp) == TRUE)
                    {
                        if(temp[2] == buf1[2]) // check command code
                        {
                            iLen = temp[0] + temp[1] * 256;

                            // save CAPK record to SRAM KEY SLOT
                            if(iLen > 1)
                            {
                                if(nextframe == 0)
                                {
                                    memset(capk, 0x00, sizeof(capk));
                                    memmove(capk, &temp[5], iLen - 3);	// without CMD(1)+LEN(2)

                                    capklen = temp[3] * 256 + temp[4];	// size of capk components
                                    if(iLen != (capklen + 3))
                                    {
                                        offset = iLen - 3;
                                        nextframe = 1;	// the capk is devided into two frames
                                    }
                                    else
                                        KeyReady = 1;
                                }
                                else
                                {
                                    memmove(&capk[offset], &temp[3], iLen - 1);	// without CMD(1)

                                    offset = 0;
                                    nextframe = 0;
                                    KeyReady = 1;
                                }

                                if(KeyReady == 1)
                                {
                                    KeyReady = 0;

                                    // verify HASH value and store it to SRAM CAPK KEY SLOT
                                    // HASH=SHA1(RID+INDEX+MOD+EXP)

                                    explen = capk[7];	// exponent length
                                    pExp = &capk[8];		// pointer to exponent

                                    if(explen == 1)	// exponent = 2 or 3
                                    {
                                        modlen = capk[9];
                                        pMod = &capk[10];
                                        pHash = &capk[10 + modlen];
                                    }
                                    else			// exponent = 2^16+1 (0x010001)
                                    {
                                        modlen = capk[11];
                                        pMod = &capk[12];
                                        pHash = &capk[12 + modlen];
                                    }

                                    // write formated CAPK to key slot
                                    memset(temp, 0x00, sizeof(capk));
                                    memmove(&temp[ADDR_CAPK_RID], capk, 5);			// RID(5)
                                    memmove(&temp[ADDR_CAPK_INDEX], &capk[6], 1);		// index(1)
                                    memmove(&temp[ADDR_CAPK_EXP_LEN], (UINT8 *)&explen, 1);	// exp length(1)
                                    memmove(&temp[ADDR_CAPK_MOD_LEN + 1], (UINT8 *)&modlen, 1);	// mod length(2) H-L
                                    memmove(&temp[ADDR_CAPK_HASH], pHash, 20);			// SHA1(20)
                                    memmove(&temp[ADDR_CAPK_EXP], pExp, explen);			// exp(n)
                                    memmove(&temp[ADDR_CAPK_EXP + explen], pMod, modlen);		// mod(n)
        //                       memmove( &temp[ADDR_CAPK_MOD], pMod, modlen );			// mod(n)
        //                       temp[ADDR_CAPK_HASH_AI] = 1;					// hash algorithm indicator (default)
        //                       temp[ADDR_CAPK_PK_AI] = 1;					// public key algorithm indicator (default)

                                    if(PED_SetCAPK(index, temp) == apiOK)
                                        index++;
                                    else
                                        break;	// invalid HASH value, terminate process
                                }
                            }
                            else
                            {
                                if(iLen == 1) // end of record?
                                    result = apiOK;	

                                break;
                            }

                            nextblk = KMC_NEXT;
                            result = apiOK;
                        }
                    }
                }
            }
		}
	} // while()

	LIB_CloseAUX();

    // clear sensitive data
	memset(temp, 0x00, sizeof(temp));
    memset(capk, 0x00, sizeof(capk));
    memset(buf1, 0x00, sizeof(buf1));
    memset(buf2, 0x00, sizeof(buf2));
    memset(buf3, 0x00, sizeof(buf3));

	return(result);
}

// ---------------------------------------------------------------------------
// FUNCTION: ADMIN - Load PED Key.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK  - OK
//           apiFailed - ERROR
// ---------------------------------------------------------------------------
#if	0
UINT8	PED_ADMIN_LoadPedKey(void)
{
	UINT8	buffer[32];
	UINT8	tid[9];
	UINT8	result = 0;
	UINT8	status = 0;
	UINT8	start;
	//UINT32	data;	
	UINT32	cancel = 0;
	UINT32	len;
	UINT32	keyloaded;


	LIB_LCD_Cls();
	LIB_LCD_Puts(0, 0, FONT0, sizeof(os_msg_LOAD_PED_KEY), (UINT8 *)os_msg_LOAD_PED_KEY);

	// Request TID(8)
	LIB_LCD_Puts(1, 0, FONT0, sizeof(os_msg_TERM_ID), (UINT8 *)os_msg_TERM_ID);

	// read current TID	// PATCH: 2009-11-11
	OS_SECM_GetData(ADDR_PED_TERM_ID, PED_TERM_ID_LEN, &tid[1]);
	tid[0] = PED_TERM_ID_LEN;

	if((tid[1] < '0') || (tid[1] > '9') || (tid[2] < '0') || (tid[2] > '9') ||
		(tid[3] < '0') || (tid[3] > '9') || (tid[4] < '0') || (tid[4] > '9') ||
		(tid[5] < '0') || (tid[5] > '9') || (tid[6] < '0') || (tid[6] > '9') ||
		(tid[7] < '0') || (tid[7] > '9') || (tid[8] < '0') || (tid[8] > '9'))
	{
		// set default TID to TSN if illegal tid values
		api_sys_info(SID_TerminalSerialNumber, buffer);
		memmove(&tid[1], &buffer[4], 8);
	}

	// show TID
	LIB_LCD_Puts(1, 13, FONT0, PED_TERM_ID_LEN, &tid[1]);

	if(LIB_GetNumKey(0, NUM_TYPE_DIGIT + NUM_TYPE_LEADING_ZERO, '_', FONT0, 2, 8, buffer) == FALSE)
		return(apiOK); // aborted

	if(!((buffer[0] == 1) && (buffer[1] == '0')))
	{
		if(buffer[0] != 8)
		{
			memset(&tid[1], 0x30, 8);
			len = PED_TERM_ID_LEN - buffer[0] + 1;
			memmove(&tid[len], &buffer[1], buffer[0]);
		}
		else
			memmove(&tid[1], &buffer[1], buffer[0]);

		// save new TID
		OS_SECM_PutData(ADDR_PED_TERM_ID, PED_TERM_ID_LEN, &tid[1]);
	}

//	if( (tid[0] == 1) && (tid[1] == '0') )	// set tid="00000000" if not entered
//	  {
//	  tid[0] = 8;
//	  memset( &tid[1], 0x30, 8 );
//	  }
//	else
//	  {
//	  if( tid[0] != 8 )  // sizeof(tid) must be 8 if entered
//	    return( apiOK );
//	  }


	// erase all keys
//	PED_EraseSensitiveKeys(); // PATCH: 2008-12-25


	// --- Generate dynamic KEK ---  
	LIB_LCD_Cls();
	LIB_LCD_PutMsg(1, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

	if(PED_VerifyIkekStatus() == FALSE)
	{
		// setup built-in IKEK
	//	PED_InitIKEK();
		return( apiFailed );	// 2019-01-11
	}

	// reset flag
	PED_SetIkekStatus(0);

	if(PED_LoadDynamicKEK(&tid[1]) == apiFailed)	//download Dynamic Key Encryption key(DKEK)
	{
		LIB_LCD_PutMsg(1, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING_ERR), (UINT8 *)os_msg_PROCESSING_ERR);
		LIB_WaitTimeAndKey(300);

		return(apiFailed);
	}

	LIB_WaitTime(100);

	if(PED_LoadKeyBlockProtectionKey(&tid[1]) == apiFailed)	// generate dynamic KEK & Key Block Protection Key
	{
		LIB_LCD_PutMsg(1, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING_ERR), (UINT8 *)os_msg_PROCESSING_ERR);
		LIB_WaitTimeAndKey(300);

		return(apiFailed);
	}

	// --- Generate PED keys ---
	start = 0;
	keyloaded = FALSE;	// 2014-10-27
	while(1)
	{
		LIB_LCD_Cls();
		LIB_LCD_Puts(0, 0, FONT0, sizeof(os_msg_SELECT), (UINT8 *)os_msg_SELECT);

		buffer[0] = 1;	// starting row number
		buffer[1] = 5;	// max lcd row cnt
		buffer[2] = 4;	// list items
		buffer[3] = 18;	// item length
		buffer[4] = 0;	// offset of LEN field in item
		buffer[5] = FONT0;
		result = LIB_ListBox(start, &buffer[0], (UINT8 *)&os_list_KEYS[0], 30); // wait for selection (T.O.=30sec)

		switch(result)
		{
			case 0xff: // aborted

//                      PED_EraseIKEK();	// PATCH: 2008-12-25
//                      PED_EraseKEK();	//
//                      PED_EraseKPK();	//

				return(apiOK);

			case 0x00: // Fixed Key

				if(keyloaded)	// 2014-10-27, not accept more than one key scheme
				{
					cancel = TRUE;
					start = result;
					break;
				}

				LIB_LCD_Cls();
				LIB_LCD_PutMsg(0, COL_LEFTMOST, FONT0, sizeof(os_msg_FKEY), (UINT8 *)os_msg_FKEY);

				cancel = FALSE;
				if(LIB_WaitKeyMsgYesNo(3, COL_LEFTMOST, sizeof(os_msg_Q_UPDATE), (UINT8 *)os_msg_Q_UPDATE) == TRUE)
				{
					LIB_LCD_PutMsg(1, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

					// --- Load Key ---
					status = PED_LoadFixedKey(&tid[1]);
					if(status == apiOK)	// 2014-10-27
						keyloaded = TRUE;
				}
				else
					cancel = TRUE;

				break;

			case 0x01: // Master Session Key

				if(keyloaded)	// 2014-10-27, not accept more than one key scheme
				{
					cancel = TRUE;
					start = result;
					break;
				}

				LIB_LCD_Cls();
				LIB_LCD_PutMsg(0, COL_LEFTMOST, FONT0, sizeof(os_msg_MSKEY), (UINT8 *)os_msg_MSKEY);

				cancel = FALSE;
				if(LIB_WaitKeyMsgYesNo(3, COL_LEFTMOST, sizeof(os_msg_Q_UPDATE), (UINT8 *)os_msg_Q_UPDATE) == TRUE)
				{
					LIB_LCD_PutMsg(1, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

					// --- Load Key ---
					status = PED_LoadMasterSessionKey(&tid[1]);
					if(status == apiOK)	// 2014-10-27
						keyloaded = TRUE;
				}
				else
					cancel = TRUE;

				break;

			case 0x02: // DUKPT Initial Key

				if(keyloaded)	// 2014-10-27, not accept more than one key scheme
				{
					cancel = TRUE;
					start = result;
					break;
				}

				LIB_LCD_Cls();
				LIB_LCD_PutMsg(0, COL_LEFTMOST, FONT0, sizeof(os_msg_DUKPT), (UINT8 *)os_msg_DUKPT);

				cancel = FALSE;
				if(LIB_WaitKeyMsgYesNo(3, COL_LEFTMOST, sizeof(os_msg_Q_UPDATE), (UINT8 *)os_msg_Q_UPDATE) == TRUE)
				{
					LIB_LCD_PutMsg(1, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

					// --- Load Key ---
					status = PED_LoadDukptInitKey(&tid[1]);
					if(status == apiOK)	// 2014-10-27
						keyloaded = TRUE;
				}
				else
					cancel = TRUE;

				break;

			case 0x03: // CAPK

				LIB_LCD_Cls();
				LIB_LCD_PutMsg(0, COL_LEFTMOST, FONT0, sizeof(os_msg_CAPK), (UINT8 *)os_msg_CAPK);

				cancel = FALSE;
				if(LIB_WaitKeyMsgYesNo(3, COL_LEFTMOST, sizeof(os_msg_Q_UPDATE), (UINT8 *)os_msg_Q_UPDATE) == TRUE)
				{
					LIB_LCD_PutMsg(1, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

					// --- Load Key ---
					status = PED_LoadCaPublicKey(&tid[1]);
				}
				else
					cancel = TRUE;

				break;
		}

		if(cancel)
			continue;

		if(status == apiOK)
			LIB_LCD_PutMsg(1, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_OK), (UINT8 *)os_msg_LOAD_KEY_OK);
		else
		{
			LIB_LCD_PutMsg(1, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);

			// erase all keys
			PED_EraseSensitiveKeys(); // PATCH: 2008-12-25
		}

		LIB_WaitKey();	// wait for any key stroke to exit
		start = result;

		if(status == apiFailed)
			return(apiFailed);

//	     return( status );
	} // while(1)

}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: ADMIN - Delete Fixed Key.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
//UINT8	PED_DeleteFixedKey( void )
//{
//	
//	PED_EraseFXKEY( 0xFF );
//	return( apiOK );
//}

// ---------------------------------------------------------------------------
// FUNCTION: ADMIN - Delete Master/Session Key.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
//UINT8	PED_DeleteMasterSessionKey( void )
//{
//	PED_EraseMSK();
//	return( apiOK );
//}

// ---------------------------------------------------------------------------
// FUNCTION: ADMIN - Delete DUKPT Key.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
//UINT8	PED_DeleteDUKPT( void )
//{
//	PED_EraseDUKPT();
//	return( apiOK );
//}

// ---------------------------------------------------------------------------
// FUNCTION: ADMIN - Delete CAPK.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
#if	0
UINT8	PED_DeleteCAPK( void )
{
	PED_EraseCAPK();
	return( apiOK );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: ADMIN - Delete PED Key.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK     - OK
//           apiFailed - ERROR
// ---------------------------------------------------------------------------
/*
UINT8	PED_ADMIN_DeletePedKey( void )
{
	UINT8	result;
	UINT8	status = 0;
	UINT8	start;
	UINT32	cancel = 0;
	UINT8	buffer[16];


	LIB_LCD_Cls();
	LIB_LCD_Puts(0, 0, FONT1, sizeof(os_msg_DEL_PED_KEY), (UINT8 *)os_msg_DEL_PED_KEY);

	// --- Delete PED keys ---
	start = 0;
	while(1)
	{
		LIB_LCD_Cls();
		LIB_LCD_Puts(0, 0, FONT1, sizeof(os_msg_SELECT), (UINT8 *)os_msg_SELECT);

		buffer[0] = 1;	// starting row number
		buffer[1] = 7;	// max lcd row cnt
		buffer[2] = 6;	// list items
		buffer[3] = 18;	// item length
		buffer[4] = 0;	// offset of LEN field in item
		buffer[5] = FONT1;
		result = LIB_ListBox(start, &buffer[0], (UINT8 *)&os_list_KEYS[0], 30); // wait for selection (T.O.=30sec)

		status = apiFailed;
		switch(result)
		{
			case 0xff: // aborted

				return(apiOK);

			case 0x00: // Master Session Key

				LIB_LCD_Cls();
				LIB_LCD_PutMsg(0, COL_LEFTMOST, FONT1, sizeof(os_msg_MSKEY), (UINT8 *)os_msg_MSKEY);

				cancel = FALSE;
				
				if(LIB_WaitKeyMsgYesNoTO(3, COL_LEFTMOST, sizeof(os_msg_Q_DELETE), (UINT8 *)os_msg_Q_DELETE) == TRUE)
				{
					LIB_LCD_PutMsg(1, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

					// --- Delete Master/Session Key ---
					PED_EraseMSK();
					status = apiOK;
				}
				else
					cancel = TRUE;

				break;

			case 0x01: // DUKPT Initial Key

				LIB_LCD_Cls();
				LIB_LCD_PutMsg(0, COL_LEFTMOST, FONT1, sizeof(os_msg_DUKPT), (UINT8 *)os_msg_DUKPT);

				cancel = FALSE;
				
				if(LIB_WaitKeyMsgYesNoTO(3, COL_LEFTMOST, sizeof(os_msg_Q_DELETE), (UINT8 *)os_msg_Q_DELETE) == TRUE)
				{
					LIB_LCD_PutMsg(1, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

					// --- Delete DUKPT ---
					PED_EraseDUKPT();
					status = apiOK;
				}
				else
					cancel = TRUE;

				break;

			case 0x02: // ISO4_KEY

				LIB_LCD_Cls();
				LIB_LCD_PutMsg(0, COL_LEFTMOST, FONT1, sizeof(os_msg_ISO4KEY), (UINT8 *)os_msg_ISO4KEY);

				cancel = FALSE;

				if(LIB_WaitKeyMsgYesNoTO(3, COL_LEFTMOST, sizeof(os_msg_Q_DELETE), (UINT8 *)os_msg_Q_DELETE) == TRUE)
				{
					LIB_LCD_PutMsg(1, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

					// --- Delete ISO4_KEY ---
					PED_EraseISO4KEY();
					status = apiOK;
				}
				else
					cancel = TRUE;

				break;

			case 0x03: // ACC_DEK

				LIB_LCD_Cls();
				LIB_LCD_PutMsg(0, COL_LEFTMOST, FONT1, sizeof(os_msg_ACCDEK), (UINT8 *)os_msg_ACCDEK);

				cancel = FALSE;

				if(LIB_WaitKeyMsgYesNoTO(3, COL_LEFTMOST, sizeof(os_msg_Q_DELETE), (UINT8 *)os_msg_Q_DELETE) == TRUE)
				{
					LIB_LCD_PutMsg(1, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

					// --- Delete ACC_DEK ---
					PED_EraseAccDEK();
					status = apiOK;
				}
				else
					cancel = TRUE;

				break;

			case 0x04: // FPE KEY

				LIB_LCD_Cls();
				LIB_LCD_PutMsg(0, COL_LEFTMOST, FONT1, sizeof(os_msg_FPEKEY), (UINT8 *)os_msg_FPEKEY);

				cancel = FALSE;

				if(LIB_WaitKeyMsgYesNoTO(3, COL_LEFTMOST, sizeof(os_msg_Q_DELETE), (UINT8 *)os_msg_Q_DELETE) == TRUE)
				{
					LIB_LCD_PutMsg(1, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

					// --- Delete FPE Key ---
					PED_EraseFPEKEY();
					status = apiOK;
				}
				else
					cancel = TRUE;

				break;

			case 0x05: // CAPK

				LIB_LCD_Cls();
				LIB_LCD_PutMsg(0, COL_LEFTMOST, FONT1, sizeof(os_msg_CAPK), (UINT8 *)os_msg_CAPK);

				cancel = FALSE;
				
				if(LIB_WaitKeyMsgYesNoTO(3, COL_LEFTMOST, sizeof(os_msg_Q_DELETE), (UINT8 *)os_msg_Q_DELETE) == TRUE)
				{
					LIB_LCD_PutMsg(1, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

					// --- Delete CAPK ---
					PED_EraseCAPK();
					status = apiOK;
				}
				else
					cancel = TRUE;

				break;
		}

		start = result;
		if(cancel)
			continue;

		if(status == apiOK)
			LIB_LCD_PutMsg(3, COL_RIGHTMOST, FONT1 + attrCLEARWRITE, sizeof(os_msg_OK), (UINT8 *)os_msg_OK);
		else
			LIB_LCD_PutMsg(3, COL_RIGHTMOST, FONT1 + attrCLEARWRITE, sizeof(os_msg_ERROR), (UINT8 *)os_msg_ERROR);
		LIB_WaitTimeAndKey(300);

		return(status);
	}
}
*/

// ---------------------------------------------------------------------------
// FUNCTION: ADMIN - Delete PED Key.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK     - OK
//           apiFailed - ERROR
// ---------------------------------------------------------------------------
UINT8	PED_ADMIN_DeletePedKey( void )
{
	UINT8	result;
	UINT8	status = 0;
	UINT8	start;
	UINT32	cancel = 0;
	UINT8	buffer[16];


	LIB_LCD_Cls();
	LIB_LCD_Puts(0, 0, FONT1, sizeof(os_msg_DEL_PED_KEY), (UINT8 *)os_msg_DEL_PED_KEY);

	// --- Delete PED keys ---
	start = 0;
	while(1)
	{
		LIB_LCD_Cls();
		LIB_LCD_Puts(0, 0, FONT1, sizeof(os_msg_SELECT), (UINT8 *)os_msg_SELECT);

		buffer[0] = 1;	// starting row number
		buffer[1] = 6;	// max lcd row cnt
		buffer[2] = 5;	// list items
		buffer[3] = 20;	// item length
		buffer[4] = 0;	// offset of LEN field in item
		buffer[5] = FONT1;
		result = LIB_ListBox(start, &buffer[0], (UINT8 *)&os_list_KEYS[0], 30); // wait for selection (T.O.=30sec)

		status = apiFailed;
		switch(result)
		{
			case 0xff: // aborted

				return(apiOK);

			case 0x00: // PEK

				LIB_LCD_Cls();
				LIB_LCD_PutMsg(0, COL_LEFTMOST, FONT1, sizeof(os_msg_PEK), (UINT8 *)os_msg_PEK);

				cancel = FALSE;
				
				if(LIB_WaitKeyMsgYesNoTO(3, COL_LEFTMOST, sizeof(os_msg_Q_DELETE), (UINT8 *)os_msg_Q_DELETE) == TRUE)
				{
					LIB_LCD_PutMsg(1, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

					// --- Delete PEK ---
					PED_ErasePEK();
					status = apiOK;
				}
				else
					cancel = TRUE;

				break;

			case 0x01: // ACC_DEK

				LIB_LCD_Cls();
				LIB_LCD_PutMsg(0, COL_LEFTMOST, FONT1, sizeof(os_msg_ACCDEK), (UINT8 *)os_msg_ACCDEK);

				cancel = FALSE;

				if(LIB_WaitKeyMsgYesNoTO(3, COL_LEFTMOST, sizeof(os_msg_Q_DELETE), (UINT8 *)os_msg_Q_DELETE) == TRUE)
				{
					LIB_LCD_PutMsg(1, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

					// --- Delete ACC_DEK ---
					PED_EraseAccDEK();
					status = apiOK;
				}
				else
					cancel = TRUE;

				break;

			case 0x02: // FPE KEY

				LIB_LCD_Cls();
				LIB_LCD_PutMsg(0, COL_LEFTMOST, FONT1, sizeof(os_msg_FPEKEY), (UINT8 *)os_msg_FPEKEY);

				cancel = FALSE;

				if(LIB_WaitKeyMsgYesNoTO(3, COL_LEFTMOST, sizeof(os_msg_Q_DELETE), (UINT8 *)os_msg_Q_DELETE) == TRUE)
				{
					LIB_LCD_PutMsg(1, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

					// --- Delete FPE Key ---
					PED_EraseFPEKEY();
					status = apiOK;
				}
				else
					cancel = TRUE;

				break;
            
            case 0x03: // AES DUKPT Key

				LIB_LCD_Cls();
				LIB_LCD_PutMsg(0, COL_LEFTMOST, FONT1, sizeof(os_msg_AES_DUKPT), (UINT8 *)os_msg_AES_DUKPT);

				cancel = FALSE;
				
				if(LIB_WaitKeyMsgYesNoTO(3, COL_LEFTMOST, sizeof(os_msg_Q_DELETE), (UINT8 *)os_msg_Q_DELETE) == TRUE)
				{
					LIB_LCD_PutMsg(1, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

					// --- Delete AES DUKPT ---
					PED_EraseAesDUKPT();
					status = apiOK;
				}
				else
					cancel = TRUE;

				break;

			case 0x04: // CAPK

				LIB_LCD_Cls();
				LIB_LCD_PutMsg(0, COL_LEFTMOST, FONT1, sizeof(os_msg_CAPK), (UINT8 *)os_msg_CAPK);

				cancel = FALSE;
				
				if(LIB_WaitKeyMsgYesNoTO(3, COL_LEFTMOST, sizeof(os_msg_Q_DELETE), (UINT8 *)os_msg_Q_DELETE) == TRUE)
				{
					LIB_LCD_PutMsg(1, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

					// --- Delete CAPK ---
					PED_EraseCAPK();
					status = apiOK;
				}
				else
					cancel = TRUE;

				break;
		}

		start = result;
		if(cancel)
			continue;

		if(status == apiOK)
			LIB_LCD_PutMsg(3, COL_RIGHTMOST, FONT1 + attrCLEARWRITE, sizeof(os_msg_OK), (UINT8 *)os_msg_OK);
		else
			LIB_LCD_PutMsg(3, COL_RIGHTMOST, FONT1 + attrCLEARWRITE, sizeof(os_msg_ERROR), (UINT8 *)os_msg_ERROR);
		LIB_WaitTimeAndKey(300);

        // clear sensitive data
	    memset(buffer, 0x00, sizeof(buffer));

		return(status);
	}
}

// ---------------------------------------------------------------------------
// FUNCTION: ADMIN - Setup Key Block Protection Key (KPK).
// INPUT   : type - 1 = KPK used for TDES key.
//                  2 = KPK used for AES key.
// OUTPUT  : none.
// RETURN  : apiOK  - OK
//           apiFailed - ERROR
// ---------------------------------------------------------------------------
UINT8	PED_ADMIN_SetKeyBlockProtectionKey(UINT8 type)
{
	UINT8	buf[1 + 65];	// LEN[1] DIGIT[n]
	UINT8	i;
	UINT8	result = apiOK;
	UINT8	tdesKpk1[16 + 8];
	UINT8	tdesKpk2[16 + 8];
	UINT8	tdesKpk[16 + 8];
	UINT8	aesKpk1[32];
	UINT8	aesKpk2[32];
	UINT8	aesKpk[32];
	UINT8	k1[16];
	UINT8	k2[16];
	UINT8	kcv[16];
	UINT8	data[16];
    UINT8	temp[X9143_TDES_KEY_DATA_LEN + 1];  // ==== [Debug] ====
	UINT8	buffer[X9143_TDES_KEY_BUNDLE_LEN];  // ==== [Debug] ====
	//UINT8	temp[KEY_DATA_LEN + 1];
	//UINT8	buffer[KEY_BUNDLE_LEN];
	UINT8	pkey1[PED_MSKEY_LEN + 1];
	UINT8	pkey2[PED_MSKEY_LEN + 1];
	UINT8	mkey[PED_MSKEY_LEN + 1];
	UINT8	kpk[16 + 8];
	UINT8	mac8[8];
	UINT8	MkeyIndex;
	UINT8	j;

	UINT8	msg_ruler[] = {"1234567890123456"};
	UINT8	msg_KPK1[] = {"KPK1:"};
	UINT8	msg_KPK2[] = {"KPK2:"};
	UINT8	msg_KCV1[] = {"KCV1="};
	UINT8	msg_KCV2[] = {"KCV2="};


	// erase KPK
	PED_EraseKPK(type);
	PED_SetKPKStatus(type, 0);

	if(type == 1)	// enter 16 bytes KPK for TDES key
	{
		LIB_LCD_Cls();
		LIB_LCD_Puts(0, 0, FONT1, sizeof(os_msg_SET_TDES_KPK), (UINT8 *)os_msg_SET_TDES_KPK);

SET_TDES_KPK1:
		// --- KPK1 ---
		LIB_LCD_ClearRow(1, 7, FONT1);
		LIB_LCD_Puts(1, 0, FONT1, sizeof(msg_KPK1), (UINT8 *)msg_KPK1);
		LIB_LCD_Puts(2, 0, FONT1, sizeof(msg_ruler), (UINT8 *)msg_ruler);
		
#if 1
		//Test key component 1
		for(i = 0 ; i < 16 ; i++)	// ==== [Debug] ====
		{
			tdesKpk1[i] = 0x98;
		}
#else
		// byte 1 ~ byte 8
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(60, 3, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					tdesKpk1[i] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}

		// byte 9 ~ byte 16
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(60, 4, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					tdesKpk1[i + 8] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}
#endif

		// enforce KPK1 != 0
		memmove(tdesKpk, tdesKpk1, PED_TDES_KEY_PROTECT_KEY_LEN);
		for(i = 0 ; i < PED_TDES_KEY_PROTECT_KEY_LEN ; i++)
			tdesKpk[i] &= 0xFE;	// ignore parity bit of input KEY

		if(LIB_memcmpc(tdesKpk, 0x00, PED_TDES_KEY_PROTECT_KEY_LEN) == 0)
		{
			result = apiFailed;	// not accepted
			goto EXIT;
		}

		// KCV1 = TDES(8*0x00, KPK1)
		LIB_LCD_Puts(6, 0, FONT1, sizeof(msg_KCV1), (UINT8 *)msg_KCV1);

		memset(buf, 0x00, sizeof(buf));
		memmove(&tdesKpk1[16], &tdesKpk1[0], 8);
		api_3des_encipher(buf, kcv, tdesKpk1);
		
		LIB_DispHexByte(6, 5, kcv[0]);
		LIB_DispHexByte(6, 8, kcv[1]);
		LIB_DispHexByte(6, 11, kcv[2]);

		if(!LIB_WaitKeyYesNo(7, 0))
			goto SET_TDES_KPK1;

SET_TDES_KPK2:
		// --- KPK2 ---
		LIB_LCD_ClearRow(1, 7, FONT1);
		LIB_LCD_Puts(1, 0, FONT1, sizeof(msg_KPK2), (UINT8 *)msg_KPK2);
		LIB_LCD_Puts(2, 0, FONT1, sizeof(msg_ruler), (UINT8 *)msg_ruler);
		
#if 1
		//Test key component 2
		for(i = 0 ; i < 16 ; i++)	// ==== [Debug] ====
		{
			tdesKpk2[i] = 0x87;
		}
#else
		// byte 1 ~ byte 8
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(60, 3, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					tdesKpk2[i] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}

		// byte 9 ~ byte 16
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(60, 4, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					tdesKpk2[i + 8] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}
#endif

		// enforce KPK2 != 0
		memmove(tdesKpk, tdesKpk2, PED_TDES_KEY_PROTECT_KEY_LEN);
		for(i = 0 ; i < PED_TDES_KEY_PROTECT_KEY_LEN ; i++)
			tdesKpk[i] &= 0xFE;	// ignore parity bit of input KEY

		if(LIB_memcmpc(tdesKpk, 0x00, PED_TDES_KEY_PROTECT_KEY_LEN) == 0)
		{
			result = apiFailed;	// not accepted
			goto EXIT;
		}

		// KCV2 = TDES(8*0x00, KPK2)
		LIB_LCD_Puts(6, 0, FONT1, sizeof(msg_KCV2), (UINT8 *)msg_KCV2);

		memset(buf, 0x00, sizeof(buf));
		memmove(&tdesKpk2[16], &tdesKpk2[0], 8);
		api_3des_encipher(buf, kcv, tdesKpk2);
		
		LIB_DispHexByte(6, 5, kcv[0]);
		LIB_DispHexByte(6, 8, kcv[1]);
		LIB_DispHexByte(6, 11, kcv[2]);

		if(!LIB_WaitKeyYesNo(7, 0))
			goto SET_TDES_KPK2;

		// final KPK = KPK1 xor KPK2
		for(i = 0 ; i < PED_TDES_KEY_PROTECT_KEY_LEN ; i++)
			tdesKpk[i] = tdesKpk1[i] ^ tdesKpk2[i];

		memmove(kpk, tdesKpk, PED_TDES_KEY_PROTECT_KEY_LEN);

		for(i = 0 ; i < PED_TDES_KEY_PROTECT_KEY_LEN ; i++)
			kpk[i] &= 0xFE;	// ignore parity bit of KPK

		// ================== KPK is compared with Master Key ==================
		// get MKEY bundle
		PED_ReadMKeyIndex((UINT8 *)&MkeyIndex);
		OS_SECM_GetData(ADDR_PED_MKEY_01 + (MkeyIndex * PED_MSKEY_SLOT_LEN), PED_MSKEY_SLOT_LEN, buffer);

		// verify MKEY bundle
		//if(TR31_VerifyKeyBundle(KEY_BUNDLE_LEN, buffer, temp, mac8, (UINT8 *)0))
        if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, buffer, temp, mac8, (UINT8 *)0))
		{
			// retrieve MKEY
			//if(TR31_DecryptKeyBundle(mac8, temp, pkey1, (UINT8 *)0))	// pkey1=MKEY
            if(X9143_DecryptKeyBundle_TDES(mac8, temp, pkey1, (UINT8 *)0))	// pkey1=MKEY
			{
				memmove(mkey, pkey1, sizeof(mkey));

				for(i = 0 ; i < pkey1[0] ; i++)
					pkey1[i + 1] &= 0xFE;	// ignore parity bit of MKEY

				if(LIB_memcmp(kpk, &pkey1[1], pkey1[0]) == 0)  // compared with MKEY
				{
					result = apiFailed;	// not accepted
					goto EXIT;
				}
			}
		}

		// ================== KPK is compared with Session Key ==================
		for(i = 0; i < MAX_SKEY_CNT; i++)
		{
			OS_SECM_GetData(ADDR_PED_SKEY_01 + (i * PED_MSKEY_SLOT_LEN), PED_MSKEY_SLOT_LEN, buffer);
			if(TR31_VerifyKeyBundle(KEY_BUNDLE_LEN, buffer, temp, mac8, mkey))	// mkey as the KBPK for SKEY
			{
				if(TR31_DecryptKeyBundle(mac8, temp, pkey2, mkey))	// pkey2=SKEY, mkey as the KBPK for SKEY
				{
					for(j = 0; j < pkey2[0]; j++)
						pkey2[j + 1] &= 0xFE; // ignore parity bit of SKEY

					if(LIB_memcmp(kpk, &pkey2[1], pkey2[0]) == 0) // compared with SKEY
					{
						result = apiFailed;	// not accepted
						goto EXIT;
					}
				}
			}
		}

		// ================== KPK is compared with ACC_DEK Master Key ==================
		// get ACC_DEK MKEY bundle
		PED_AccDEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
		OS_SECM_GetData(ADDR_PED_ACC_DEK_MKEY_01 + (MkeyIndex * PED_ACC_DEK_MSKEY_SLOT_LEN), PED_ACC_DEK_MSKEY_SLOT_LEN, buffer);

		// verify ACC_DEK MKEY bundle
		if(TR31_VerifyKeyBundle(KEY_BUNDLE_LEN, buffer, temp, mac8, (UINT8 *)0))
		{
			// retrieve ACC_DEK MKEY
			if(TR31_DecryptKeyBundle(mac8, temp, pkey1, (UINT8 *)0))	// pkey1=ACC_DEK MKEY
			{
				memmove(mkey, pkey1, sizeof(mkey));

				for(i = 0 ; i < pkey1[0] ; i++)
					pkey1[i + 1] &= 0xFE;	// ignore parity bit of ACC_DEK MKEY

				if(LIB_memcmp(kpk, &pkey1[1], pkey1[0]) == 0)  // compared with ACC_DEK MKEY
				{
					result = apiFailed;	// not accepted
					goto EXIT;
				}
			}
		}

		// ================== KPK is compared with ACC_DEK Session Key ==================
		// get ACC_DEK SKEY bundle
		OS_SECM_GetData(ADDR_PED_ACC_DEK_SKEY_01, PED_ACC_DEK_MSKEY_SLOT_LEN, buffer);

		// verify ACC_DEK SKEY bundle
		if(TR31_VerifyKeyBundle(KEY_BUNDLE_LEN, buffer, temp, mac8, mkey))	// mkey as the KBPK for ACC_DEK SKEY
		{
			// retrieve ACC_DEK ESKEY
			if(TR31_DecryptKeyBundle(mac8, temp, pkey2, mkey))	// pkey2=ACC_DEK SKEY, mkey as the KBPK for ACC_DEK SKEY
			{
				for(i = 0 ; i < pkey2[0] ; i++)
					pkey2[i + 1] &= 0xFE;	 // ignore parity bit of ACC_DEK SKEY

				if(LIB_memcmp(kpk, &pkey2[1], pkey2[0]) == 0)	// compared with ACC_DEK SKEY
				{
					result = apiFailed;	// not accepted
					goto EXIT;
				}
			}
		}

		//final KCV of KPK = TDES(8*0x00, KPK)
		memset(buf, 0x00, sizeof(buf));
		memmove(&tdesKpk[16], &tdesKpk[0], 8);
		api_3des_encipher(buf, kcv, tdesKpk);

		// Save KPK : LEN(1) + KPK(16) + KCV(3)
		memset(buf, 0x00, sizeof(buf));
		buf[0] = PED_TDES_KEY_PROTECT_KEY_LEN;			// LEN
		memmove(&buf[1], tdesKpk, PED_TDES_KEY_PROTECT_KEY_LEN);	// KPK
		memmove(&buf[1 + PED_TDES_KEY_PROTECT_KEY_LEN], kcv, 3);	// KCV

		OS_SECM_PutData(ADDR_PED_TDES_KEY_PROTECT_KEY, PED_TDES_KEY_PROTECT_KEY_SLOT_LEN, buf);
	}
	else if(type == 2)	// enter 32 bytes KPK for AES key
	{
		LIB_LCD_Cls();
		LIB_LCD_Puts(0, 0, FONT1, sizeof(os_msg_SET_AES_KPK), (UINT8 *)os_msg_SET_AES_KPK);

SET_AES_KPK1:
		// --- KPK1 ---
		LIB_LCD_ClearRow(1, 9, FONT1);
		LIB_LCD_Puts(1, 0, FONT1, sizeof(msg_KPK1), (UINT8 *)msg_KPK1);
		LIB_LCD_Puts(2, 0, FONT1, sizeof(msg_ruler), (UINT8 *)msg_ruler);
		
#if 1
		//Test key component 1
		for(i = 0 ; i < 32 ; i++)	// ==== [Debug] ====
		{
			aesKpk1[i] = 0x98;
		}
#else
		// byte 1 ~ byte 8
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(120, 3, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					aesKpk1[i] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}

		// byte 9 ~ byte 16
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(120, 4, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					aesKpk1[i + 8] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}

		// byte 17 ~ byte 24
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(120, 5, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					aesKpk1[i + 16] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}

		// byte 25 ~ byte 32
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(120, 6, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					aesKpk1[i + 24] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}
#endif

		// enforce KPK1 != 0
		memmove(aesKpk, aesKpk1, PED_AES_KEY_PROTECT_KEY_LEN);
		for(i = 0 ; i < PED_AES_KEY_PROTECT_KEY_LEN ; i++)
			aesKpk[i] &= 0xFE;	// ignore parity bit of input KEY

		if(LIB_memcmpc(aesKpk, 0x00, PED_AES_KEY_PROTECT_KEY_LEN) == 0)
		{
			result = apiFailed;	// not accepted
			goto EXIT;
		}

		// the KCV is calculated by MACing an all-zero block using the CMAC algorithm as specified in ISO 9797-1 (see also NIST SP 800-38B)
		// KCV1 = leftmost 5 bytes of AES(16*0x00 XOR K1, KPK1)
		LIB_LCD_Puts(8, 0, FONT1, sizeof(msg_KCV1), (UINT8 *)msg_KCV1);

		TR31_CMAC_DeriveSubKey_AES(aesKpk1, k1, k2);

		memset(data, 0x00, sizeof(data));
		for(i = 0 ; i < 16 ; i++)
			data[i] ^= k1[i];

		api_aes_encipher(data, kcv, aesKpk1, 32);

		LIB_DispHexByte(8, 5, kcv[0]);
		LIB_DispHexByte(8, 8, kcv[1]);
		LIB_DispHexByte(8, 11, kcv[2]);
		LIB_DispHexByte(8, 14, kcv[3]);
		LIB_DispHexByte(8, 17, kcv[4]);

		if(!LIB_WaitKeyYesNo(9, 0))
			goto SET_AES_KPK1;

SET_AES_KPK2:
		// --- KPK2 ---
		LIB_LCD_ClearRow(1, 9, FONT1);
		LIB_LCD_Puts(1, 0, FONT1, sizeof(msg_KPK2), (UINT8 *)msg_KPK2);
		LIB_LCD_Puts(2, 0, FONT1, sizeof(msg_ruler), (UINT8 *)msg_ruler);
		
#if 1
		//Test key component 2
		for(i = 0 ; i < 32 ; i++)	// ==== [Debug] ====
		{
			aesKpk2[i] = 0x87;
		}
#else
		// byte 1 ~ byte 8
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(120, 3, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					aesKpk2[i] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}

		// byte 9 ~ byte 16
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(120, 4, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					aesKpk2[i + 8] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}

		// byte 17 ~ byte 24
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(120, 5, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					aesKpk2[i + 16] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}

		// byte 25 ~ byte 32
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(120, 6, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					aesKpk2[i + 24] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}
#endif

		// enforce KPK2 != 0
		memmove(aesKpk, aesKpk2, PED_AES_KEY_PROTECT_KEY_LEN);
		for(i = 0 ; i < PED_AES_KEY_PROTECT_KEY_LEN ; i++)
			aesKpk[i] &= 0xFE;	// ignore parity bit of input KEY

		if(LIB_memcmpc(aesKpk, 0x00, PED_AES_KEY_PROTECT_KEY_LEN) == 0)
		{
			result = apiFailed;	// not accepted
			goto EXIT;
		}

		// the KCV is calculated by MACing an all-zero block using the CMAC algorithm as specified in ISO 9797-1 (see also NIST SP 800-38B)
		// KCV2 = leftmost 5 bytes of AES(16*0x00 XOR K1, KPK2)
		LIB_LCD_Puts(8, 0, FONT1, sizeof(msg_KCV2), (UINT8 *)msg_KCV2);

		TR31_CMAC_DeriveSubKey_AES(aesKpk2, k1, k2);

		memset(data, 0x00, sizeof(data));
		for(i = 0 ; i < 16 ; i++)
			data[i] ^= k1[i];

		api_aes_encipher(data, kcv, aesKpk2, 32);

		LIB_DispHexByte(8, 5, kcv[0]);
		LIB_DispHexByte(8, 8, kcv[1]);
		LIB_DispHexByte(8, 11, kcv[2]);
		LIB_DispHexByte(8, 14, kcv[3]);
		LIB_DispHexByte(8, 17, kcv[4]);

		if(!LIB_WaitKeyYesNo(9, 0))
			goto SET_AES_KPK2;

		// final KPK = KPK1 xor KPK2
		for(i = 0 ; i < PED_AES_KEY_PROTECT_KEY_LEN ; i++)
			aesKpk[i] = aesKpk1[i] ^ aesKpk2[i];

		// the KCV is calculated by MACing an all-zero block using the CMAC algorithm as specified in ISO 9797-1 (see also NIST SP 800-38B)
		// KCV = leftmost 5 bytes of AES(16*0x00 XOR K1, KPK)
		TR31_CMAC_DeriveSubKey_AES(aesKpk, k1, k2);

		memset(data, 0x00, sizeof(data));
		for(i = 0 ; i < 16 ; i++)
			data[i] ^= k1[i];

		api_aes_encipher(data, kcv, aesKpk, 32);

		// Save KPK : LEN(1) + KPK(32) + KCV(5)
		memset(buf, 0x00, sizeof(buf));
		buf[0] = PED_AES_KEY_PROTECT_KEY_LEN;			// LEN
		memmove(&buf[1], aesKpk, PED_AES_KEY_PROTECT_KEY_LEN);	// KPK
		memmove(&buf[1 + PED_AES_KEY_PROTECT_KEY_LEN], kcv, 5);	// KCV

		OS_SECM_PutData(ADDR_PED_AES_KEY_PROTECT_KEY, PED_AES_KEY_PROTECT_KEY_SLOT_LEN, buf);
	}

	// KPK setup ok
	PED_SetKPKStatus(type, 1);

EXIT:
	// clear sensitive data
	memset(buf, 0x00, sizeof(buf));
	memset(tdesKpk1, 0x00, sizeof(tdesKpk1));
	memset(tdesKpk2, 0x00, sizeof(tdesKpk2));
	memset(tdesKpk, 0x00, sizeof(tdesKpk));
	memset(aesKpk1, 0x00, sizeof(aesKpk1));
	memset(aesKpk2, 0x00, sizeof(aesKpk2));
	memset(aesKpk, 0x00, sizeof(aesKpk));
	memset(k1, 0x00, sizeof(k1));
	memset(k2, 0x00, sizeof(k2));
	memset(kcv, 0x00, sizeof(kcv));
	memset(data, 0x00, sizeof(data));
    memset(temp, 0x00, sizeof(temp));
    memset(buffer, 0x00, sizeof(buffer));
    memset(pkey1, 0x00, sizeof(pkey1));
    memset(pkey2, 0x00, sizeof(pkey2));
    memset(mkey, 0x00, sizeof(mkey));
    memset(kpk, 0x00, sizeof(kpk));
    memset(mac8, 0x00, sizeof(mac8));

	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: ADMIN - Setup TDES Key Block Protection Key (KPK).
// INPUT   : type - 0x01 = 128-bit TDES KPK.
//                  0x02 = 192-bit TDES KPK.
// OUTPUT  : none.
// RETURN  : apiOK  - OK
//           apiFailed - ERROR
// ---------------------------------------------------------------------------
UINT8	PED_ADMIN_SetTDESKeyBlockProtectionKey(UINT8 type)
{
    UINT8	buf[1 + 24 + 3];	// LEN[1] DIGIT[n]
	UINT8	i;
	UINT8	result = apiOK;
    UINT8	tdesKpk1[24];
	UINT8	tdesKpk2[24];
	UINT8	tdesKpk[24];
	UINT8	kcv[16];
	UINT8	data[16];
    UINT8	temp[X9143_TDES_KEY_DATA_LEN + 1];
	UINT8	buffer[X9143_TDES_KEY_BUNDLE_LEN];

    UINT8	pkey1[24 + 1];
	UINT8	pkey2[24 + 1];
	UINT8	mkey[24 + 1];
	UINT8	kpk[24];
	UINT8	mac8[8];
	UINT8	MkeyIndex;
	UINT8	j;

    UINT8	msg_ruler[] = {"1234567890123456"};
	UINT8	msg_KPK1[] = {"KPK1:"};
	UINT8	msg_KPK2[] = {"KPK2:"};
	UINT8	msg_KCV1[] = {"KCV1="};
	UINT8	msg_KCV2[] = {"KCV2="};

    // erase KPK
    PED_TDES_EraseKPK(type);

    if(type == TDES_128)	// enter 128-bit KPK for TDES key
	{
		LIB_LCD_Cls();
		LIB_LCD_Puts(0, 0, FONT1, sizeof(os_msg_SET_TDES_KPK), (UINT8 *)os_msg_SET_TDES_KPK);

SET_128_TDES_KPK1:
		// --- KPK1 ---
		LIB_LCD_ClearRow(1, 7, FONT1);
		LIB_LCD_Puts(1, 0, FONT1, sizeof(msg_KPK1), (UINT8 *)msg_KPK1);
		LIB_LCD_Puts(2, 0, FONT1, sizeof(msg_ruler), (UINT8 *)msg_ruler);
		
#if 0
		//Test key component 1
		for(i = 0 ; i < 16 ; i++)
		{
			tdesKpk1[i] = 0x98;
		}
#else
		// byte 1 ~ byte 8
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(60, 3, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					tdesKpk1[i] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}

		// byte 9 ~ byte 16
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(60, 4, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					tdesKpk1[i + 8] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}
#endif

		// enforce KPK1 != 0
		memmove(tdesKpk, tdesKpk1, PED_128_TDES_KEY_PROTECT_KEY_LEN);
		for(i = 0 ; i < PED_128_TDES_KEY_PROTECT_KEY_LEN ; i++)
			tdesKpk[i] &= 0xFE;	// ignore parity bit of input KEY

		if(LIB_memcmpc(tdesKpk, 0x00, PED_128_TDES_KEY_PROTECT_KEY_LEN) == 0)
		{
			result = apiFailed;	// not accepted
			goto EXIT;
		}

		// KCV1 = TDES(8*0x00, KPK1)
		LIB_LCD_Puts(6, 0, FONT1, sizeof(msg_KCV1), (UINT8 *)msg_KCV1);

		memset(buf, 0x00, sizeof(buf));
		memmove(&tdesKpk1[16], &tdesKpk1[0], 8);
		api_3des_encipher(buf, kcv, tdesKpk1);
		
		LIB_DispHexByte(6, 5, kcv[0]);
		LIB_DispHexByte(6, 8, kcv[1]);
		LIB_DispHexByte(6, 11, kcv[2]);

		if(!LIB_WaitKeyYesNo(7, 0))
			goto SET_128_TDES_KPK1;

SET_128_TDES_KPK2:
		// --- KPK2 ---
		LIB_LCD_ClearRow(1, 7, FONT1);
		LIB_LCD_Puts(1, 0, FONT1, sizeof(msg_KPK2), (UINT8 *)msg_KPK2);
		LIB_LCD_Puts(2, 0, FONT1, sizeof(msg_ruler), (UINT8 *)msg_ruler);
		
#if 0
		//Test key component 2
		for(i = 0 ; i < 16 ; i++)
		{
			tdesKpk2[i] = 0x87;
		}
#else
		// byte 1 ~ byte 8
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(60, 3, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					tdesKpk2[i] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}

		// byte 9 ~ byte 16
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(60, 4, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					tdesKpk2[i + 8] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}
#endif

		// enforce KPK2 != 0
		memmove(tdesKpk, tdesKpk2, PED_128_TDES_KEY_PROTECT_KEY_LEN);
		for(i = 0 ; i < PED_128_TDES_KEY_PROTECT_KEY_LEN ; i++)
			tdesKpk[i] &= 0xFE;	// ignore parity bit of input KEY

		if(LIB_memcmpc(tdesKpk, 0x00, PED_128_TDES_KEY_PROTECT_KEY_LEN) == 0)
		{
			result = apiFailed;	// not accepted
			goto EXIT;
		}

		// KCV2 = TDES(8*0x00, KPK2)
		LIB_LCD_Puts(6, 0, FONT1, sizeof(msg_KCV2), (UINT8 *)msg_KCV2);

		memset(buf, 0x00, sizeof(buf));
		memmove(&tdesKpk2[16], &tdesKpk2[0], 8);
		api_3des_encipher(buf, kcv, tdesKpk2);
		
		LIB_DispHexByte(6, 5, kcv[0]);
		LIB_DispHexByte(6, 8, kcv[1]);
		LIB_DispHexByte(6, 11, kcv[2]);

		if(!LIB_WaitKeyYesNo(7, 0))
			goto SET_128_TDES_KPK2;

		// final KPK = KPK1 xor KPK2
		for(i = 0 ; i < PED_128_TDES_KEY_PROTECT_KEY_LEN ; i++)
			tdesKpk[i] = tdesKpk1[i] ^ tdesKpk2[i];

		memmove(kpk, tdesKpk, PED_128_TDES_KEY_PROTECT_KEY_LEN);

		for(i = 0 ; i < PED_128_TDES_KEY_PROTECT_KEY_LEN ; i++)
			kpk[i] &= 0xFE;	// ignore parity bit of KPK

		// ================== KPK is compared with PEK Master Key ==================
		// get PEK MKEY bundle
		PED_PEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
		OS_SECM_GetData(ADDR_PED_PEK_TDES_MKEY_01 + (MkeyIndex * PED_PEK_TDES_MSKEY_SLOT_LEN), PED_PEK_TDES_MSKEY_SLOT_LEN, buffer);

		// verify PEK MKEY bundle
        if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, buffer, temp, mac8, (UINT8 *)0))
		{
			// retrieve PEK MKEY
            if(X9143_DecryptKeyBundle_TDES(mac8, temp, pkey1, (UINT8 *)0))	// pkey1=PEK MKEY
			{
                if(pkey1[0] != 16)
                    goto SAVE_128_KPK;
                
				memmove(mkey, pkey1, sizeof(mkey));

				for(i = 0 ; i < pkey1[0] ; i++)
					pkey1[i + 1] &= 0xFE;	// ignore parity bit of PEK MKEY

				if(LIB_memcmp(kpk, &pkey1[1], pkey1[0]) == 0)  // compared with PEK MKEY
				{
					result = apiFailed;	// not accepted
					goto EXIT;
				}
			}
		}

		// ================== KPK is compared with PEK Session Key ==================
		for(i = 0 ; i < MAX_PEK_SKEY_CNT ; i++)
		{
			OS_SECM_GetData(ADDR_PED_PEK_TDES_SKEY_01 + (i * PED_PEK_TDES_MSKEY_SLOT_LEN), PED_PEK_TDES_MSKEY_SLOT_LEN, buffer);
			if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, buffer, temp, mac8, mkey))	// mkey as the KBPK for PEK SKEY
			{
				if(X9143_DecryptKeyBundle_TDES(mac8, temp, pkey2, mkey))	// pkey2=SKEY, mkey as the KBPK for PEK SKEY
				{
					for(j = 0 ; j < pkey2[0] ; j++)
						pkey2[j + 1] &= 0xFE; // ignore parity bit of PEK SKEY

					if(LIB_memcmp(kpk, &pkey2[1], pkey2[0]) == 0) // compared with PEK SKEY
					{
						result = apiFailed;	// not accepted
						goto EXIT;
					}
				}
			}
		}

SAVE_128_KPK:
		//final KCV of KPK = TDES(8*0x00, KPK)
		memset(buf, 0x00, sizeof(buf));
		memmove(&tdesKpk[16], &tdesKpk[0], 8);
		api_3des_encipher(buf, kcv, tdesKpk);

		// Save KPK : LEN(1) + KPK(16) + KCV(3)
		memset(buf, 0x00, sizeof(buf));
		buf[0] = PED_128_TDES_KEY_PROTECT_KEY_LEN;			// LEN
		memmove(&buf[1], tdesKpk, PED_128_TDES_KEY_PROTECT_KEY_LEN);	// KPK
		memmove(&buf[1 + PED_128_TDES_KEY_PROTECT_KEY_LEN], kcv, 3);	// KCV

		OS_SECM_PutData(ADDR_PED_128_TDES_KEY_PROTECT_KEY, PED_128_TDES_KEY_PROTECT_KEY_SLOT_LEN, buf);
	}
    else if(type == TDES_192)	// enter 192-bit KPK for TDES key
	{
		LIB_LCD_Cls();
		LIB_LCD_Puts(0, 0, FONT1, sizeof(os_msg_SET_TDES_KPK), (UINT8 *)os_msg_SET_TDES_KPK);

SET_192_TDES_KPK1:
		// --- KPK1 ---
		LIB_LCD_ClearRow(1, 8, FONT1);
		LIB_LCD_Puts(1, 0, FONT1, sizeof(msg_KPK1), (UINT8 *)msg_KPK1);
		LIB_LCD_Puts(2, 0, FONT1, sizeof(msg_ruler), (UINT8 *)msg_ruler);
		
#if 0
		//Test key component 1
		for(i = 0 ; i < 24 ; i++)
		{
			tdesKpk1[i] = 0x98;
		}
#else
		// byte 1 ~ byte 8
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(60, 3, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					tdesKpk1[i] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}

		// byte 9 ~ byte 16
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(60, 4, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					tdesKpk1[i + 8] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}

        // byte 17 ~ byte 24
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(60, 5, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					tdesKpk1[i + 16] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}
#endif

		// enforce KPK1 != 0
		memmove(tdesKpk, tdesKpk1, PED_192_TDES_KEY_PROTECT_KEY_LEN);
		for(i = 0 ; i < PED_192_TDES_KEY_PROTECT_KEY_LEN ; i++)
			tdesKpk[i] &= 0xFE;	// ignore parity bit of input KEY

		if(LIB_memcmpc(tdesKpk, 0x00, PED_192_TDES_KEY_PROTECT_KEY_LEN) == 0)
		{
			result = apiFailed;	// not accepted
			goto EXIT;
		}

		// KCV1 = TDES(8*0x00, KPK1)
		LIB_LCD_Puts(7, 0, FONT1, sizeof(msg_KCV1), (UINT8 *)msg_KCV1);

		memset(buf, 0x00, sizeof(buf));
		api_3des_encipher(buf, kcv, tdesKpk1);
		
		LIB_DispHexByte(7, 5, kcv[0]);
		LIB_DispHexByte(7, 8, kcv[1]);
		LIB_DispHexByte(7, 11, kcv[2]);

		if(!LIB_WaitKeyYesNo(8, 0))
			goto SET_192_TDES_KPK1;

SET_192_TDES_KPK2:
		// --- KPK2 ---
		LIB_LCD_ClearRow(1, 8, FONT1);
		LIB_LCD_Puts(1, 0, FONT1, sizeof(msg_KPK2), (UINT8 *)msg_KPK2);
		LIB_LCD_Puts(2, 0, FONT1, sizeof(msg_ruler), (UINT8 *)msg_ruler);
		
#if 0
		//Test key component 2
		for(i = 0 ; i < 24 ; i++)
		{
			tdesKpk2[i] = 0x87;
		}
#else
		// byte 1 ~ byte 8
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(60, 3, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					tdesKpk2[i] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}

		// byte 9 ~ byte 16
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(60, 4, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					tdesKpk2[i + 8] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}

        // byte 17 ~ byte 24
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(60, 5, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					tdesKpk2[i + 16] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}
#endif

		// enforce KPK2 != 0
		memmove(tdesKpk, tdesKpk2, PED_192_TDES_KEY_PROTECT_KEY_LEN);
		for(i = 0 ; i < PED_192_TDES_KEY_PROTECT_KEY_LEN ; i++)
			tdesKpk[i] &= 0xFE;	// ignore parity bit of input KEY

		if(LIB_memcmpc(tdesKpk, 0x00, PED_192_TDES_KEY_PROTECT_KEY_LEN) == 0)
		{
			result = apiFailed;	// not accepted
			goto EXIT;
		}

		// KCV2 = TDES(8*0x00, KPK2)
		LIB_LCD_Puts(7, 0, FONT1, sizeof(msg_KCV2), (UINT8 *)msg_KCV2);

		memset(buf, 0x00, sizeof(buf));
		api_3des_encipher(buf, kcv, tdesKpk2);
		
		LIB_DispHexByte(7, 5, kcv[0]);
		LIB_DispHexByte(7, 8, kcv[1]);
		LIB_DispHexByte(7, 11, kcv[2]);

		if(!LIB_WaitKeyYesNo(8, 0))
			goto SET_192_TDES_KPK2;

		// final KPK = KPK1 xor KPK2
		for(i = 0 ; i < PED_192_TDES_KEY_PROTECT_KEY_LEN ; i++)
			tdesKpk[i] = tdesKpk1[i] ^ tdesKpk2[i];

		memmove(kpk, tdesKpk, PED_192_TDES_KEY_PROTECT_KEY_LEN);

		for(i = 0 ; i < PED_192_TDES_KEY_PROTECT_KEY_LEN ; i++)
			kpk[i] &= 0xFE;	// ignore parity bit of KPK

COMPARE_WITH_192_PEK_MKSK:
		// ================== KPK is compared with PEK Master Key ==================
		// get PEK MKEY bundle
		PED_PEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
		OS_SECM_GetData(ADDR_PED_PEK_TDES_MKEY_01 + (MkeyIndex * PED_PEK_TDES_MSKEY_SLOT_LEN), PED_PEK_TDES_MSKEY_SLOT_LEN, buffer);

		// verify PEK MKEY bundle
        if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, buffer, temp, mac8, (UINT8 *)0))
		{
			// retrieve PEK MKEY
            if(X9143_DecryptKeyBundle_TDES(mac8, temp, pkey1, (UINT8 *)0))	// pkey1=PEK MKEY
			{
                if(pkey1[0] != 24)  // no need to compare with PEK
                    goto COMPARE_WITH_192_ACCDEK_MKSK;
                
				memmove(mkey, pkey1, sizeof(mkey));

				for(i = 0 ; i < pkey1[0] ; i++)
					pkey1[i + 1] &= 0xFE;	// ignore parity bit of PEK MKEY

				if(LIB_memcmp(kpk, &pkey1[1], pkey1[0]) == 0)  // compared with PEK MKEY
				{
					result = apiFailed;	// not accepted
					goto EXIT;
				}
			}
		}

		// ================== KPK is compared with PEK Session Key ==================
		for(i = 0 ; i < MAX_PEK_SKEY_CNT ; i++)
		{
			OS_SECM_GetData(ADDR_PED_PEK_TDES_SKEY_01 + (i * PED_PEK_TDES_MSKEY_SLOT_LEN), PED_PEK_TDES_MSKEY_SLOT_LEN, buffer);
			if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, buffer, temp, mac8, mkey))	// mkey as the KBPK for PEK SKEY
			{
				if(X9143_DecryptKeyBundle_TDES(mac8, temp, pkey2, mkey))	// pkey2=SKEY, mkey as the KBPK for PEK SKEY
				{
					for(j = 0 ; j < pkey2[0] ; j++)
						pkey2[j + 1] &= 0xFE; // ignore parity bit of PEK SKEY

					if(LIB_memcmp(kpk, &pkey2[1], pkey2[0]) == 0) // compared with PEK SKEY
					{
						result = apiFailed;	// not accepted
						goto EXIT;
					}
				}
			}
		}

COMPARE_WITH_192_ACCDEK_MKSK:
		// ================== KPK is compared with ACC_DEK Master Key ==================
		// get ACC_DEK MKEY bundle
		PED_AccDEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
		OS_SECM_GetData(ADDR_PED_ACC_DEK_TDES_MKEY_01 + (MkeyIndex * PED_ACC_DEK_TDES_MSKEY_SLOT_LEN), PED_ACC_DEK_TDES_MSKEY_SLOT_LEN, buffer);

		// verify ACC_DEK MKEY bundle
		if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, buffer, temp, mac8, (UINT8 *)0))
		{
			// retrieve ACC_DEK MKEY
			if(X9143_DecryptKeyBundle_TDES(mac8, temp, pkey1, (UINT8 *)0))	// pkey1=ACC_DEK MKEY
			{
                if(pkey1[0] != 24)  // no need to compare with ACC_DEK
                    goto SAVE_192_KPK;

				memmove(mkey, pkey1, sizeof(mkey));

				for(i = 0 ; i < pkey1[0] ; i++)
					pkey1[i + 1] &= 0xFE;	// ignore parity bit of ACC_DEK MKEY

				if(LIB_memcmp(kpk, &pkey1[1], pkey1[0]) == 0)  // compared with ACC_DEK MKEY
				{
					result = apiFailed;	// not accepted
					goto EXIT;
				}
			}
		}

		// ================== KPK is compared with ACC_DEK Session Key ==================
		// get ACC_DEK SKEY bundle
		OS_SECM_GetData(ADDR_PED_ACC_DEK_TDES_SKEY_01, PED_ACC_DEK_TDES_MSKEY_SLOT_LEN, buffer);

		// verify ACC_DEK SKEY bundle
		if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, buffer, temp, mac8, mkey))	// mkey as the KBPK for ACC_DEK SKEY
		{
			// retrieve ACC_DEK ESKEY
			if(X9143_DecryptKeyBundle_TDES(mac8, temp, pkey2, mkey))	// pkey2=ACC_DEK SKEY, mkey as the KBPK for ACC_DEK SKEY
			{
				for(i = 0 ; i < pkey2[0] ; i++)
					pkey2[i + 1] &= 0xFE;	 // ignore parity bit of ACC_DEK SKEY

				if(LIB_memcmp(kpk, &pkey2[1], pkey2[0]) == 0)	// compared with ACC_DEK SKEY
				{
					result = apiFailed;	// not accepted
					goto EXIT;
				}
			}
		}

SAVE_192_KPK:
		//final KCV of KPK = TDES(8*0x00, KPK)
		memset(buf, 0x00, sizeof(buf));
		api_3des_encipher(buf, kcv, tdesKpk);

		// Save KPK : LEN(1) + KPK(24) + KCV(3)
		memset(buf, 0x00, sizeof(buf));
		buf[0] = PED_192_TDES_KEY_PROTECT_KEY_LEN;			// LEN
		memmove(&buf[1], tdesKpk, PED_192_TDES_KEY_PROTECT_KEY_LEN);	// KPK
		memmove(&buf[1 + PED_192_TDES_KEY_PROTECT_KEY_LEN], kcv, 3);	// KCV

		OS_SECM_PutData(ADDR_PED_192_TDES_KEY_PROTECT_KEY, PED_192_TDES_KEY_PROTECT_KEY_SLOT_LEN, buf);
	}

    // KPK setup ok
	PED_SetKPKStatus(type, 1);

EXIT:
	// clear sensitive data
	memset(buf, 0x00, sizeof(buf));
	memset(tdesKpk1, 0x00, sizeof(tdesKpk1));
	memset(tdesKpk2, 0x00, sizeof(tdesKpk2));
	memset(tdesKpk, 0x00, sizeof(tdesKpk));
	memset(kcv, 0x00, sizeof(kcv));
	memset(data, 0x00, sizeof(data));
    memset(temp, 0x00, sizeof(temp));
    memset(buffer, 0x00, sizeof(buffer));
    memset(pkey1, 0x00, sizeof(pkey1));
    memset(pkey2, 0x00, sizeof(pkey2));
    memset(mkey, 0x00, sizeof(mkey));
    memset(kpk, 0x00, sizeof(kpk));
    memset(mac8, 0x00, sizeof(mac8));

	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: ADMIN - Setup AES Key Block Protection Key (KPK).
// INPUT   : type - 0x03 = 128-bit AES KPK.
//                  0x04 = 192-bit AES KPK.
//                  0x05 = 256-bit AES KPK.
// OUTPUT  : none.
// RETURN  : apiOK  - OK
//           apiFailed - ERROR
// ---------------------------------------------------------------------------
UINT8	PED_ADMIN_SetAESKeyBlockProtectionKey(UINT8 type)
{
    UINT8	buf[1 + 32 + 5];	// LEN[1] DIGIT[n]
	UINT8	i;
	UINT8	result = apiOK;
    UINT8	aesKpk1[32];
	UINT8	aesKpk2[32];
	UINT8	aesKpk[32];
    UINT8	k1[16];
	UINT8	k2[16];
	UINT8	kcv[16];
	UINT8	data[16];
    UINT8	temp[X9143_AES_KEY_DATA_LEN + 1];
	UINT8	buffer[X9143_AES_KEY_BUNDLE_LEN];

    UINT8	pkey1[32 + 1];
	UINT8	pkey2[32 + 1];
	UINT8	mkey[32 + 1];
	UINT8	mac16[16];
	UINT8	MkeyIndex;
	UINT8	j;

    UINT8	msg_ruler[] = {"1234567890123456"};
	UINT8	msg_KPK1[] = {"KPK1:"};
	UINT8	msg_KPK2[] = {"KPK2:"};
	UINT8	msg_KCV1[] = {"KCV1="};
	UINT8	msg_KCV2[] = {"KCV2="};

    // erase KPK
	//PED_AES_EraseKPK(0);
    PED_AES_EraseKPK(type);

    if(type == AES_128)	// enter 128-bit KPK for AES key
	{
		LIB_LCD_Cls();
		LIB_LCD_Puts(0, 0, FONT1, sizeof(os_msg_SET_AES_KPK), (UINT8 *)os_msg_SET_AES_KPK);

SET_128_AES_KPK1:
		// --- KPK1 ---
		LIB_LCD_ClearRow(1, 7, FONT1);
		LIB_LCD_Puts(1, 0, FONT1, sizeof(msg_KPK1), (UINT8 *)msg_KPK1);
		LIB_LCD_Puts(2, 0, FONT1, sizeof(msg_ruler), (UINT8 *)msg_ruler);
		
#if 0
		//Test key component 1
		for(i = 0 ; i < 16 ; i++)
		{
			aesKpk1[i] = 0x98;
		}
#else
		// byte 1 ~ byte 8
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(60, 3, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					aesKpk1[i] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}

		// byte 9 ~ byte 16
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(60, 4, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					aesKpk1[i + 8] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}
#endif

		// enforce KPK1 != 0
        if(LIB_memcmpc(aesKpk1, 0x00, PED_128_AES_KEY_PROTECT_KEY_LEN) == 0)
		{
			result = apiFailed;	// not accepted
			goto EXIT;
		}

		// the KCV is calculated by MACing an all-zero block using the CMAC algorithm as specified in ISO 9797-1 (see also NIST SP 800-38B)
		// KCV1 = leftmost 5 bytes of AES(16*0x00 XOR K1, KPK1)
		LIB_LCD_Puts(6, 0, FONT1, sizeof(msg_KCV1), (UINT8 *)msg_KCV1);

        X9143_CMAC_DeriveSubKey_AES(aesKpk1, PED_128_AES_KEY_PROTECT_KEY_LEN, k1, k2);

		memset(data, 0x00, sizeof(data));
		for(i = 0 ; i < 16 ; i++)
			data[i] ^= k1[i];

		api_aes_encipher(data, kcv, aesKpk1, 16);
		
		LIB_DispHexByte(6, 5, kcv[0]);
		LIB_DispHexByte(6, 8, kcv[1]);
		LIB_DispHexByte(6, 11, kcv[2]);
        LIB_DispHexByte(6, 14, kcv[3]);
		LIB_DispHexByte(6, 17, kcv[4]);

		if(!LIB_WaitKeyYesNo(7, 0))
			goto SET_128_AES_KPK1;

SET_128_AES_KPK2:
		// --- KPK2 ---
		LIB_LCD_ClearRow(1, 7, FONT1);
		LIB_LCD_Puts(1, 0, FONT1, sizeof(msg_KPK2), (UINT8 *)msg_KPK2);
		LIB_LCD_Puts(2, 0, FONT1, sizeof(msg_ruler), (UINT8 *)msg_ruler);
		
#if 0
		//Test key component 2
		for(i = 0 ; i < 16 ; i++)
		{
			aesKpk2[i] = 0x87;
		}
#else
		// byte 1 ~ byte 8
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(60, 3, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					aesKpk2[i] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}

		// byte 9 ~ byte 16
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(60, 4, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					aesKpk2[i + 8] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}
#endif

		// enforce KPK2 != 0
        if(LIB_memcmpc(aesKpk2, 0x00, PED_128_AES_KEY_PROTECT_KEY_LEN) == 0)
		{
			result = apiFailed;	// not accepted
			goto EXIT;
		}

		// the KCV is calculated by MACing an all-zero block using the CMAC algorithm as specified in ISO 9797-1 (see also NIST SP 800-38B)
		// KCV2 = leftmost 5 bytes of AES(16*0x00 XOR K1, KPK2)
		LIB_LCD_Puts(6, 0, FONT1, sizeof(msg_KCV2), (UINT8 *)msg_KCV2);

        X9143_CMAC_DeriveSubKey_AES(aesKpk2, PED_128_AES_KEY_PROTECT_KEY_LEN, k1, k2);

        memset(data, 0x00, sizeof(data));
		for(i = 0 ; i < 16 ; i++)
			data[i] ^= k1[i];

		api_aes_encipher(data, kcv, aesKpk2, 16);
		
		LIB_DispHexByte(6, 5, kcv[0]);
		LIB_DispHexByte(6, 8, kcv[1]);
		LIB_DispHexByte(6, 11, kcv[2]);
        LIB_DispHexByte(6, 14, kcv[3]);
		LIB_DispHexByte(6, 17, kcv[4]);

		if(!LIB_WaitKeyYesNo(7, 0))
			goto SET_128_AES_KPK2;

		// final KPK = KPK1 xor KPK2
		for(i = 0 ; i < PED_128_AES_KEY_PROTECT_KEY_LEN ; i++)
			aesKpk[i] = aesKpk1[i] ^ aesKpk2[i];

COMPARE_WITH_128_PEK_MKSK:
		// ================== KPK is compared with PEK Master Key ==================
		// get PEK MKEY bundle
		PED_PEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
		OS_SECM_GetData(ADDR_PED_PEK_AES_MKEY_01 + (MkeyIndex * PED_PEK_AES_MSKEY_SLOT_LEN), PED_PEK_AES_MSKEY_SLOT_LEN, buffer);

		// verify PEK MKEY bundle
        if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, buffer, temp, mac16, (UINT8 *)0))
		{
			// retrieve PEK MKEY
            if(X9143_DecryptKeyBundle_AES(mac16, temp, pkey1, (UINT8 *)0))	// pkey1=PEK MKEY
			{
                if(pkey1[0] != 16)
                    goto COMPARE_WITH_128_ACCDEK_MKSK;
                
				memmove(mkey, pkey1, sizeof(mkey));

				if(LIB_memcmp(aesKpk, &pkey1[1], pkey1[0]) == 0)  // compared with PEK MKEY
				{
					result = apiFailed;	// not accepted
					goto EXIT;
				}
			}
		}

		// ================== KPK is compared with PEK Session Key ==================
		for(i = 0 ; i < MAX_PEK_SKEY_CNT ; i++)
		{
            // get PEK SKEY bundle
			OS_SECM_GetData(ADDR_PED_PEK_AES_SKEY_01 + (i * PED_PEK_AES_MSKEY_SLOT_LEN), PED_PEK_AES_MSKEY_SLOT_LEN, buffer);
			
            // verify PEK SKEY bundle
            if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, buffer, temp, mac16, mkey))	// mkey as the KBPK for PEK SKEY
			{
                // retrieve PEK SKEY
				if(X9143_DecryptKeyBundle_AES(mac16, temp, pkey2, mkey))	// pkey2=SKEY, mkey as the KBPK for PEK SKEY
				{
					if(LIB_memcmp(aesKpk, &pkey2[1], pkey2[0]) == 0) // compared with PEK SKEY
					{
						result = apiFailed;	// not accepted
						goto EXIT;
					}
				}
			}
		}

COMPARE_WITH_128_ACCDEK_MKSK:
		// ================== KPK is compared with ACC_DEK Master Key ==================
		// get ACC_DEK MKEY bundle
		PED_AccDEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
		OS_SECM_GetData(ADDR_PED_ACC_DEK_AES_MKEY_01 + (MkeyIndex * PED_ACC_DEK_AES_MSKEY_SLOT_LEN), PED_ACC_DEK_AES_MSKEY_SLOT_LEN, buffer);

		// verify ACC_DEK MKEY bundle
		if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, buffer, temp, mac16, (UINT8 *)0))
		{
			// retrieve ACC_DEK MKEY
			if(X9143_DecryptKeyBundle_AES(mac16, temp, pkey1, (UINT8 *)0))	// pkey1=ACC_DEK MKEY
			{
                if(pkey1[0] != 16)  // no need to compare with ACC_DEK
                    goto COMPARE_WITH_128_FPE_MKSK;

				memmove(mkey, pkey1, sizeof(mkey));

				if(LIB_memcmp(aesKpk, &pkey1[1], pkey1[0]) == 0)  // compared with ACC_DEK MKEY
				{
					result = apiFailed;	// not accepted
					goto EXIT;
				}
			}
		}

		// ================== KPK is compared with ACC_DEK Session Key ==================
		// get ACC_DEK SKEY bundle
		OS_SECM_GetData(ADDR_PED_ACC_DEK_AES_SKEY_01, PED_ACC_DEK_AES_MSKEY_SLOT_LEN, buffer);

		// verify ACC_DEK SKEY bundle
		if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, buffer, temp, mac16, mkey))	// mkey as the KBPK for ACC_DEK SKEY
		{
			// retrieve ACC_DEK SKEY
			if(X9143_DecryptKeyBundle_AES(mac16, temp, pkey2, mkey))	// pkey2=ACC_DEK SKEY, mkey as the KBPK for ACC_DEK SKEY
			{
				if(LIB_memcmp(aesKpk, &pkey2[1], pkey2[0]) == 0)	// compared with ACC_DEK SKEY
				{
					result = apiFailed;	// not accepted
					goto EXIT;
				}
			}
		}

COMPARE_WITH_128_FPE_MKSK:
        // ================== KPK is compared with FPE Master Key ==================
		// get FPE MKEY bundle
		PED_FPE_ReadMKeyIndex((UINT8 *)&MkeyIndex);
		OS_SECM_GetData(ADDR_PED_FPE_KEY_AES_MKEY_01 + (MkeyIndex * PED_FPE_KEY_AES_MSKEY_SLOT_LEN), PED_FPE_KEY_AES_MSKEY_SLOT_LEN, buffer);

		// verify FPE MKEY bundle
		if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, buffer, temp, mac16, (UINT8 *)0))
		{
			// retrieve FPE MKEY
			if(X9143_DecryptKeyBundle_AES(mac16, temp, pkey1, (UINT8 *)0))	// pkey1=FPE MKEY
			{
				memmove(mkey, pkey1, sizeof(mkey));

				if(LIB_memcmp(aesKpk, &pkey1[1], pkey1[0]) == 0)  // compared with FPE MKEY
				{
					result = apiFailed;	// not accepted
					goto EXIT;
				}
			}
		}

		// ================== KPK is compared with FPE Session Key ==================
		// get FPE SKEY bundle
		OS_SECM_GetData(ADDR_PED_FPE_KEY_AES_SKEY_01, PED_FPE_KEY_AES_MSKEY_SLOT_LEN, buffer);

		// verify FPE SKEY bundle
		if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, buffer, temp, mac16, mkey))	// mkey as the KBPK for FPE SKEY
		{
			// retrieve FPE SKEY
			if(X9143_DecryptKeyBundle_AES(mac16, temp, pkey2, mkey))	// pkey2=FPE SKEY, mkey as the KBPK for FPE SKEY
			{
				if(LIB_memcmp(aesKpk, &pkey2[1], pkey2[0]) == 0)	// compared with FPE SKEY
				{
					result = apiFailed;	// not accepted
					goto EXIT;
				}
			}
		}

SAVE_128_KPK:
		// the KCV is calculated by MACing an all-zero block using the CMAC algorithm as specified in ISO 9797-1 (see also NIST SP 800-38B)
		// KCV = leftmost 5 bytes of AES(16*0x00 XOR K1, KPK)
		X9143_CMAC_DeriveSubKey_AES(aesKpk, PED_128_AES_KEY_PROTECT_KEY_LEN, k1, k2);

		memset(data, 0x00, sizeof(data));
		for(i = 0 ; i < 16 ; i++)
			data[i] ^= k1[i];

		api_aes_encipher(data, kcv, aesKpk, 16);

        // Save KPK : LEN(1) + KPK(16) + KCV(5)
		memset(buf, 0x00, sizeof(buf));
		buf[0] = PED_128_AES_KEY_PROTECT_KEY_LEN;			// LEN
		memmove(&buf[1], aesKpk, PED_128_AES_KEY_PROTECT_KEY_LEN);	// KPK
		memmove(&buf[1 + PED_128_AES_KEY_PROTECT_KEY_LEN], kcv, 5);	// KCV

		OS_SECM_PutData(ADDR_PED_128_AES_KEY_PROTECT_KEY, PED_128_AES_KEY_PROTECT_KEY_SLOT_LEN, buf);
	}
    else if(type == AES_192)	// enter 192-bit KPK for AES key
	{
		LIB_LCD_Cls();
		LIB_LCD_Puts(0, 0, FONT1, sizeof(os_msg_SET_AES_KPK), (UINT8 *)os_msg_SET_AES_KPK);

SET_192_AES_KPK1:
		// --- KPK1 ---
		LIB_LCD_ClearRow(1, 8, FONT1);
		LIB_LCD_Puts(1, 0, FONT1, sizeof(msg_KPK1), (UINT8 *)msg_KPK1);
		LIB_LCD_Puts(2, 0, FONT1, sizeof(msg_ruler), (UINT8 *)msg_ruler);
		
#if 0
		//Test key component 1
		for(i = 0 ; i < 24 ; i++)
		{
			aesKpk1[i] = 0x98;
		}
#else
		// byte 1 ~ byte 8
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(60, 3, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					aesKpk1[i] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}

		// byte 9 ~ byte 16
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(60, 4, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					aesKpk1[i + 8] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}

        // byte 17 ~ byte 24
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(60, 5, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					aesKpk1[i + 16] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}
#endif

		// enforce KPK1 != 0
        if(LIB_memcmpc(aesKpk1, 0x00, PED_192_AES_KEY_PROTECT_KEY_LEN) == 0)
		{
			result = apiFailed;	// not accepted
			goto EXIT;
		}

		// the KCV is calculated by MACing an all-zero block using the CMAC algorithm as specified in ISO 9797-1 (see also NIST SP 800-38B)
		// KCV1 = leftmost 5 bytes of AES(16*0x00 XOR K1, KPK1)
		LIB_LCD_Puts(7, 0, FONT1, sizeof(msg_KCV1), (UINT8 *)msg_KCV1);

		X9143_CMAC_DeriveSubKey_AES(aesKpk1, PED_192_AES_KEY_PROTECT_KEY_LEN, k1, k2);

		memset(data, 0x00, sizeof(data));
		for(i = 0 ; i < 16 ; i++)
			data[i] ^= k1[i];

		api_aes_encipher(data, kcv, aesKpk1, 24);
		
		LIB_DispHexByte(7, 5, kcv[0]);
		LIB_DispHexByte(7, 8, kcv[1]);
		LIB_DispHexByte(7, 11, kcv[2]);
        LIB_DispHexByte(7, 14, kcv[3]);
		LIB_DispHexByte(7, 17, kcv[4]);

		if(!LIB_WaitKeyYesNo(8, 0))
			goto SET_192_AES_KPK1;

SET_192_AES_KPK2:
		// --- KPK2 ---
		LIB_LCD_ClearRow(1, 8, FONT1);
		LIB_LCD_Puts(1, 0, FONT1, sizeof(msg_KPK2), (UINT8 *)msg_KPK2);
		LIB_LCD_Puts(2, 0, FONT1, sizeof(msg_ruler), (UINT8 *)msg_ruler);
		
#if 0
		//Test key component 2
		for(i = 0 ; i < 24 ; i++)
		{
			aesKpk2[i] = 0x87;
		}
#else
		// byte 1 ~ byte 8
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(60, 3, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					aesKpk2[i] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}

		// byte 9 ~ byte 16
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(60, 4, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					aesKpk2[i + 8] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}

        // byte 17 ~ byte 24
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(60, 5, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					aesKpk2[i + 16] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}
#endif

		// enforce KPK2 != 0
        if(LIB_memcmpc(aesKpk2, 0x00, PED_192_AES_KEY_PROTECT_KEY_LEN) == 0)
		{
			result = apiFailed;	// not accepted
			goto EXIT;
		}

		// the KCV is calculated by MACing an all-zero block using the CMAC algorithm as specified in ISO 9797-1 (see also NIST SP 800-38B)
		// KCV2 = leftmost 5 bytes of AES(16*0x00 XOR K1, KPK2)
		LIB_LCD_Puts(7, 0, FONT1, sizeof(msg_KCV2), (UINT8 *)msg_KCV2);

		X9143_CMAC_DeriveSubKey_AES(aesKpk2, PED_192_AES_KEY_PROTECT_KEY_LEN, k1, k2);

		memset(data, 0x00, sizeof(data));
		for(i = 0 ; i < 16 ; i++)
			data[i] ^= k1[i];

		api_aes_encipher(data, kcv, aesKpk2, 24);
		
		LIB_DispHexByte(7, 5, kcv[0]);
		LIB_DispHexByte(7, 8, kcv[1]);
		LIB_DispHexByte(7, 11, kcv[2]);
        LIB_DispHexByte(7, 14, kcv[3]);
		LIB_DispHexByte(7, 17, kcv[4]);

		if(!LIB_WaitKeyYesNo(8, 0))
			goto SET_192_AES_KPK2;

		// final KPK = KPK1 xor KPK2
		for(i = 0 ; i < PED_192_AES_KEY_PROTECT_KEY_LEN ; i++)
			aesKpk[i] = aesKpk1[i] ^ aesKpk2[i];

COMPARE_WITH_192_PEK_MKSK:
		// ================== KPK is compared with PEK Master Key ==================
		// get PEK MKEY bundle
		PED_PEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
		OS_SECM_GetData(ADDR_PED_PEK_AES_MKEY_01 + (MkeyIndex * PED_PEK_AES_MSKEY_SLOT_LEN), PED_PEK_AES_MSKEY_SLOT_LEN, buffer);

		// verify PEK MKEY bundle
        if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, buffer, temp, mac16, (UINT8 *)0))
		{
			// retrieve PEK MKEY
            if(X9143_DecryptKeyBundle_AES(mac16, temp, pkey1, (UINT8 *)0))	// pkey1=PEK MKEY
			{
                if(pkey1[0] != 24)  // no need to compare with PEK
                    goto COMPARE_WITH_192_ACCDEK_MKSK;
                
				memmove(mkey, pkey1, sizeof(mkey));

				if(LIB_memcmp(aesKpk, &pkey1[1], pkey1[0]) == 0)  // compared with PEK MKEY
				{
					result = apiFailed;	// not accepted
					goto EXIT;
				}
			}
		}

		// ================== KPK is compared with PEK Session Key ==================
		for(i = 0 ; i < MAX_PEK_SKEY_CNT ; i++)
		{
            // get PEK SKEY bundle
			OS_SECM_GetData(ADDR_PED_PEK_AES_SKEY_01 + (i * PED_PEK_AES_MSKEY_SLOT_LEN), PED_PEK_AES_MSKEY_SLOT_LEN, buffer);
			
            // verify PEK SKEY bundle
            if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, buffer, temp, mac16, mkey))	// mkey as the KBPK for PEK SKEY
			{
                // retrieve PEK SKEY
				if(X9143_DecryptKeyBundle_AES(mac16, temp, pkey2, mkey))	// pkey2=SKEY, mkey as the KBPK for PEK SKEY
				{
					if(LIB_memcmp(aesKpk, &pkey2[1], pkey2[0]) == 0) // compared with PEK SKEY
					{
						result = apiFailed;	// not accepted
						goto EXIT;
					}
				}
			}
		}

COMPARE_WITH_192_ACCDEK_MKSK:
		// ================== KPK is compared with ACC_DEK Master Key ==================
		// get ACC_DEK MKEY bundle
		PED_AccDEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
		OS_SECM_GetData(ADDR_PED_ACC_DEK_AES_MKEY_01 + (MkeyIndex * PED_ACC_DEK_AES_MSKEY_SLOT_LEN), PED_ACC_DEK_AES_MSKEY_SLOT_LEN, buffer);

		// verify ACC_DEK MKEY bundle
		if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, buffer, temp, mac16, (UINT8 *)0))
		{
			// retrieve ACC_DEK MKEY
			if(X9143_DecryptKeyBundle_AES(mac16, temp, pkey1, (UINT8 *)0))	// pkey1=ACC_DEK MKEY
			{
                if(pkey1[0] != 24)  // no need to compare with ACC_DEK
                    goto SAVE_192_KPK;

				memmove(mkey, pkey1, sizeof(mkey));

				if(LIB_memcmp(aesKpk, &pkey1[1], pkey1[0]) == 0)  // compared with ACC_DEK MKEY
				{
					result = apiFailed;	// not accepted
					goto EXIT;
				}
			}
		}

		// ================== KPK is compared with ACC_DEK Session Key ==================
		// get ACC_DEK SKEY bundle
		OS_SECM_GetData(ADDR_PED_ACC_DEK_AES_SKEY_01, PED_ACC_DEK_AES_MSKEY_SLOT_LEN, buffer);

		// verify ACC_DEK SKEY bundle
		if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, buffer, temp, mac16, mkey))	// mkey as the KBPK for ACC_DEK SKEY
		{
			// retrieve ACC_DEK SKEY
			if(X9143_DecryptKeyBundle_AES(mac16, temp, pkey2, mkey))	// pkey2=ACC_DEK SKEY, mkey as the KBPK for ACC_DEK SKEY
			{
				if(LIB_memcmp(aesKpk, &pkey2[1], pkey2[0]) == 0)	// compared with ACC_DEK SKEY
				{
					result = apiFailed;	// not accepted
					goto EXIT;
				}
			}
		}

SAVE_192_KPK:
		// the KCV is calculated by MACing an all-zero block using the CMAC algorithm as specified in ISO 9797-1 (see also NIST SP 800-38B)
		// KCV = leftmost 5 bytes of AES(16*0x00 XOR K1, KPK)
		X9143_CMAC_DeriveSubKey_AES(aesKpk, PED_192_AES_KEY_PROTECT_KEY_LEN, k1, k2);

		memset(data, 0x00, sizeof(data));
		for(i = 0 ; i < 16 ; i++)
			data[i] ^= k1[i];

		api_aes_encipher(data, kcv, aesKpk, 24);

		// Save KPK : LEN(1) + KPK(24) + KCV(5)
		memset(buf, 0x00, sizeof(buf));
		buf[0] = PED_192_AES_KEY_PROTECT_KEY_LEN;			// LEN
		memmove(&buf[1], aesKpk, PED_192_AES_KEY_PROTECT_KEY_LEN);	// KPK
		memmove(&buf[1 + PED_192_AES_KEY_PROTECT_KEY_LEN], kcv, 5);	// KCV

		OS_SECM_PutData(ADDR_PED_192_AES_KEY_PROTECT_KEY, PED_192_AES_KEY_PROTECT_KEY_SLOT_LEN, buf);
	}
    else if(type == AES_256)	// enter 256-bit KPK for AES key
	{
		LIB_LCD_Cls();
		LIB_LCD_Puts(0, 0, FONT1, sizeof(os_msg_SET_AES_KPK), (UINT8 *)os_msg_SET_AES_KPK);

SET_256_AES_KPK1:
		// --- KPK1 ---
		LIB_LCD_ClearRow(1, 9, FONT1);
		LIB_LCD_Puts(1, 0, FONT1, sizeof(msg_KPK1), (UINT8 *)msg_KPK1);
		LIB_LCD_Puts(2, 0, FONT1, sizeof(msg_ruler), (UINT8 *)msg_ruler);
		
#if 0
		//Test key component 1
		for(i = 0 ; i < 32 ; i++)
		{
			aesKpk1[i] = 0x98;
		}
#else
		// byte 1 ~ byte 8
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(120, 3, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					aesKpk1[i] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}

		// byte 9 ~ byte 16
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(120, 4, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					aesKpk1[i + 8] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}

        // byte 17 ~ byte 24
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(120, 5, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					aesKpk1[i + 16] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}

        // byte 25 ~ byte 32
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(120, 6, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					aesKpk1[i + 24] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}
#endif

		// enforce KPK1 != 0
        if(LIB_memcmpc(aesKpk1, 0x00, PED_256_AES_KEY_PROTECT_KEY_LEN) == 0)
		{
			result = apiFailed;	// not accepted
			goto EXIT;
		}

		// the KCV is calculated by MACing an all-zero block using the CMAC algorithm as specified in ISO 9797-1 (see also NIST SP 800-38B)
		// KCV1 = leftmost 5 bytes of AES(16*0x00 XOR K1, KPK1)
		LIB_LCD_Puts(8, 0, FONT1, sizeof(msg_KCV1), (UINT8 *)msg_KCV1);

		X9143_CMAC_DeriveSubKey_AES(aesKpk1, PED_256_AES_KEY_PROTECT_KEY_LEN, k1, k2);

		memset(data, 0x00, sizeof(data));
		for(i = 0 ; i < 16 ; i++)
			data[i] ^= k1[i];

		api_aes_encipher(data, kcv, aesKpk1, 32);
		
		LIB_DispHexByte(8, 5, kcv[0]);
		LIB_DispHexByte(8, 8, kcv[1]);
		LIB_DispHexByte(8, 11, kcv[2]);
        LIB_DispHexByte(8, 14, kcv[3]);
		LIB_DispHexByte(8, 17, kcv[4]);

		if(!LIB_WaitKeyYesNo(9, 0))
			goto SET_256_AES_KPK1;

SET_256_AES_KPK2:
		// --- KPK2 ---
		LIB_LCD_ClearRow(1, 9, FONT1);
		LIB_LCD_Puts(1, 0, FONT1, sizeof(msg_KPK2), (UINT8 *)msg_KPK2);
		LIB_LCD_Puts(2, 0, FONT1, sizeof(msg_ruler), (UINT8 *)msg_ruler);
		
#if 0
		//Test key component 2
		for(i = 0 ; i < 32 ; i++)
		{
			aesKpk2[i] = 0x87;
		}
#else
		// byte 1 ~ byte 8
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(120, 3, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					aesKpk2[i] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}

		// byte 9 ~ byte 16
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(120, 4, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					aesKpk2[i + 8] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}

        // byte 17 ~ byte 24
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(120, 5, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					aesKpk2[i + 16] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}

        // byte 25 ~ byte 32
		while(1)
		{
			memset(buf, 0x00, sizeof(buf));
			if(LIB_GetAlphaNumDigits2(120, 6, 0, FONT1, 16, buf) == TRUE)
			{
				if(buf[0] != 16)
					continue;

				for(i = 0 ; i < 8 ; i++)
					aesKpk2[i + 24] = LIB_ascw2hexb(&buf[1 + i * 2]);	// convert to hex format

				break;
			}
			else
			{
				result = apiFailed;
				goto EXIT;
			}
		}
#endif

		// enforce KPK2 != 0
        if(LIB_memcmpc(aesKpk2, 0x00, PED_256_AES_KEY_PROTECT_KEY_LEN) == 0)
		{
			result = apiFailed;	// not accepted
			goto EXIT;
		}

		// the KCV is calculated by MACing an all-zero block using the CMAC algorithm as specified in ISO 9797-1 (see also NIST SP 800-38B)
		// KCV2 = leftmost 5 bytes of AES(16*0x00 XOR K1, KPK2)
		LIB_LCD_Puts(8, 0, FONT1, sizeof(msg_KCV2), (UINT8 *)msg_KCV2);

		X9143_CMAC_DeriveSubKey_AES(aesKpk2, PED_256_AES_KEY_PROTECT_KEY_LEN, k1, k2);

		memset(data, 0x00, sizeof(data));
		for(i = 0 ; i < 16 ; i++)
			data[i] ^= k1[i];

		api_aes_encipher(data, kcv, aesKpk2, 32);
		
		LIB_DispHexByte(8, 5, kcv[0]);
		LIB_DispHexByte(8, 8, kcv[1]);
		LIB_DispHexByte(8, 11, kcv[2]);
        LIB_DispHexByte(8, 14, kcv[3]);
		LIB_DispHexByte(8, 17, kcv[4]);

		if(!LIB_WaitKeyYesNo(9, 0))
			goto SET_256_AES_KPK2;

		// final KPK = KPK1 xor KPK2
		for(i = 0 ; i < PED_256_AES_KEY_PROTECT_KEY_LEN ; i++)
			aesKpk[i] = aesKpk1[i] ^ aesKpk2[i];

COMPARE_WITH_256_PEK_MKSK:
		// ================== KPK is compared with PEK Master Key ==================
		// get PEK MKEY bundle
		PED_PEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
		OS_SECM_GetData(ADDR_PED_PEK_AES_MKEY_01 + (MkeyIndex * PED_PEK_AES_MSKEY_SLOT_LEN), PED_PEK_AES_MSKEY_SLOT_LEN, buffer);

		// verify PEK MKEY bundle
        if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, buffer, temp, mac16, (UINT8 *)0))
		{
			// retrieve PEK MKEY
            if(X9143_DecryptKeyBundle_AES(mac16, temp, pkey1, (UINT8 *)0))	// pkey1=PEK MKEY
			{
                if(pkey1[0] != 32)  // no need to compare with PEK
                    goto COMPARE_WITH_256_ACCDEK_MKSK;
                
				memmove(mkey, pkey1, sizeof(mkey));

				if(LIB_memcmp(aesKpk, &pkey1[1], pkey1[0]) == 0)  // compared with PEK MKEY
				{
					result = apiFailed;	// not accepted
					goto EXIT;
				}
			}
		}

		// ================== KPK is compared with PEK Session Key ==================
		for(i = 0 ; i < MAX_PEK_SKEY_CNT ; i++)
		{
            // get PEK SKEY bundle
			OS_SECM_GetData(ADDR_PED_PEK_AES_SKEY_01 + (i * PED_PEK_AES_MSKEY_SLOT_LEN), PED_PEK_AES_MSKEY_SLOT_LEN, buffer);
			
            // verify PEK SKEY bundle
            if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, buffer, temp, mac16, mkey))	// mkey as the KBPK for PEK SKEY
			{
                // retrieve PEK SKEY
				if(X9143_DecryptKeyBundle_AES(mac16, temp, pkey2, mkey))	// pkey2=SKEY, mkey as the KBPK for PEK SKEY
				{
					if(LIB_memcmp(aesKpk, &pkey2[1], pkey2[0]) == 0) // compared with PEK SKEY
					{
						result = apiFailed;	// not accepted
						goto EXIT;
					}
				}
			}
		}

COMPARE_WITH_256_ACCDEK_MKSK:
		// ================== KPK is compared with ACC_DEK Master Key ==================
		// get ACC_DEK MKEY bundle
		PED_AccDEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
		OS_SECM_GetData(ADDR_PED_ACC_DEK_AES_MKEY_01 + (MkeyIndex * PED_ACC_DEK_AES_MSKEY_SLOT_LEN), PED_ACC_DEK_AES_MSKEY_SLOT_LEN, buffer);

		// verify ACC_DEK MKEY bundle
		if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, buffer, temp, mac16, (UINT8 *)0))
		{
			// retrieve ACC_DEK MKEY
			if(X9143_DecryptKeyBundle_AES(mac16, temp, pkey1, (UINT8 *)0))	// pkey1=ACC_DEK MKEY
			{
                if(pkey1[0] != 32)  // no need to compare with ACC_DEK
                    goto SAVE_256_KPK;

				memmove(mkey, pkey1, sizeof(mkey));

				if(LIB_memcmp(aesKpk, &pkey1[1], pkey1[0]) == 0)  // compared with ACC_DEK MKEY
				{
					result = apiFailed;	// not accepted
					goto EXIT;
				}
			}
		}

		// ================== KPK is compared with ACC_DEK Session Key ==================
		// get ACC_DEK SKEY bundle
		OS_SECM_GetData(ADDR_PED_ACC_DEK_AES_SKEY_01, PED_ACC_DEK_AES_MSKEY_SLOT_LEN, buffer);

		// verify ACC_DEK SKEY bundle
		if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, buffer, temp, mac16, mkey))	// mkey as the KBPK for ACC_DEK SKEY
		{
			// retrieve ACC_DEK SKEY
			if(X9143_DecryptKeyBundle_AES(mac16, temp, pkey2, mkey))	// pkey2=ACC_DEK SKEY, mkey as the KBPK for ACC_DEK SKEY
			{
				if(LIB_memcmp(aesKpk, &pkey2[1], pkey2[0]) == 0)	// compared with ACC_DEK SKEY
				{
					result = apiFailed;	// not accepted
					goto EXIT;
				}
			}
		}

SAVE_256_KPK:
		// the KCV is calculated by MACing an all-zero block using the CMAC algorithm as specified in ISO 9797-1 (see also NIST SP 800-38B)
		// KCV = leftmost 5 bytes of AES(16*0x00 XOR K1, KPK)
		X9143_CMAC_DeriveSubKey_AES(aesKpk, PED_256_AES_KEY_PROTECT_KEY_LEN, k1, k2);

		memset(data, 0x00, sizeof(data));
		for(i = 0 ; i < 16 ; i++)
			data[i] ^= k1[i];

		api_aes_encipher(data, kcv, aesKpk, 32);

		// Save KPK : LEN(1) + KPK(32) + KCV(5)
		memset(buf, 0x00, sizeof(buf));
		buf[0] = PED_256_AES_KEY_PROTECT_KEY_LEN;			// LEN
		memmove(&buf[1], aesKpk, PED_256_AES_KEY_PROTECT_KEY_LEN);	// KPK
		memmove(&buf[1 + PED_256_AES_KEY_PROTECT_KEY_LEN], kcv, 5);	// KCV

		OS_SECM_PutData(ADDR_PED_256_AES_KEY_PROTECT_KEY, PED_256_AES_KEY_PROTECT_KEY_SLOT_LEN, buf);
	}

    // KPK setup ok
	PED_SetKPKStatus(type, 1);

EXIT:
	// clear sensitive data
	memset(buf, 0x00, sizeof(buf));
	memset(aesKpk1, 0x00, sizeof(aesKpk1));
	memset(aesKpk2, 0x00, sizeof(aesKpk2));
	memset(aesKpk, 0x00, sizeof(aesKpk));
    memset(k1, 0x00, sizeof(k1));
	memset(k2, 0x00, sizeof(k2));
	memset(kcv, 0x00, sizeof(kcv));
	memset(data, 0x00, sizeof(data));
    memset(temp, 0x00, sizeof(temp));
    memset(buffer, 0x00, sizeof(buffer));
    memset(pkey1, 0x00, sizeof(pkey1));
    memset(pkey2, 0x00, sizeof(pkey2));
    memset(mkey, 0x00, sizeof(mkey));
    memset(mac16, 0x00, sizeof(mac16));

	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: ADMIN - Load Initial Key Encryption Key (IKEK).
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK  - OK
//           apiFailed - ERROR
// ---------------------------------------------------------------------------
#if 0
UINT8	PED_ADMIN_LoadIKEK(void)
{
#if 0
	UINT8	buffer[32];
	UINT8	tid[9];
	UINT32	len;


	LIB_LCD_Cls();
	LIB_LCD_Puts(0, 0, FONT0, sizeof(os_msg_LOAD_IKEK), (UINT8 *)os_msg_LOAD_IKEK);

	// Request TID(8)
	LIB_LCD_Puts(1, 0, FONT0, sizeof(os_msg_TERM_ID), (UINT8 *)os_msg_TERM_ID);

	// read current TID
	OS_SECM_GetData(ADDR_PED_TERM_ID, PED_TERM_ID_LEN, &tid[1]);
	tid[0] = PED_TERM_ID_LEN;

	if((tid[1] < '0') || (tid[1] > '9') || (tid[2] < '0') || (tid[2] > '9') ||
		(tid[3] < '0') || (tid[3] > '9') || (tid[4] < '0') || (tid[4] > '9') ||
		(tid[5] < '0') || (tid[5] > '9') || (tid[6] < '0') || (tid[6] > '9') ||
		(tid[7] < '0') || (tid[7] > '9') || (tid[8] < '0') || (tid[8] > '9'))
	{
		// set default TID to TSN if illegal tid values
		api_sys_info(SID_TerminalSerialNumber, buffer);
		memmove(&tid[1], &buffer[4], 8);
	}

	// show TID
	LIB_LCD_Puts(1, 13, FONT0, PED_TERM_ID_LEN, &tid[1]);

	if(LIB_GetNumKey(0, NUM_TYPE_DIGIT + NUM_TYPE_LEADING_ZERO, '_', FONT0, 2, 8, buffer) == FALSE)
		return(apiOK); // aborted

	if(!((buffer[0] == 1) && (buffer[1] == '0')))
	{
		if(buffer[0] != 8)
		{
			memset(&tid[1], 0x30, 8);
			len = PED_TERM_ID_LEN - buffer[0] + 1;
			memmove(&tid[len], &buffer[1], buffer[0]);
		}
		else
			memmove(&tid[1], &buffer[1], buffer[0]);

		// save new TID
		OS_SECM_PutData(ADDR_PED_TERM_ID, PED_TERM_ID_LEN, &tid[1]);
	}

	// erase IKEK
	PED_EraseIKEK();
	PED_SetIkekStatus(0);

	// --- Generate dynamic KEK ---  
	LIB_LCD_Cls();
	LIB_LCD_PutMsg(1, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

	if(PED_LoadInitialKeyEncryptionKey(&tid[1]) == apiFailed)	// Download Initial Key Encryption Key (IKEK)
	{
		LIB_LCD_PutMsg(1, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);
		LIB_WaitTimeAndKey(300);

		return apiFailed;
	}

	//Load IKEK ok
	PED_SetIkekStatus(1);
	LIB_LCD_PutMsg(1, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_OK), (UINT8 *)os_msg_LOAD_KEY_OK);
	LIB_WaitTimeAndKey(300);

	return apiOK;
#endif
	UINT8	buffer[32];
	UINT8	tid[9];
	UINT32	len;


	// read current TID
	OS_SECM_GetData(ADDR_PED_TERM_ID, PED_TERM_ID_LEN, &tid[1]);
	tid[0] = PED_TERM_ID_LEN;

	if((tid[1] < '0') || (tid[1] > '9') || (tid[2] < '0') || (tid[2] > '9') ||
		(tid[3] < '0') || (tid[3] > '9') || (tid[4] < '0') || (tid[4] > '9') ||
		(tid[5] < '0') || (tid[5] > '9') || (tid[6] < '0') || (tid[6] > '9') ||
		(tid[7] < '0') || (tid[7] > '9') || (tid[8] < '0') || (tid[8] > '9'))
	{
		// set default TID to zero if illegal tid values
		memset(&tid[1], '0', 8);
	}

	// erase IKEK
	PED_EraseIKEK();
	PED_SetIkekStatus(0);

	// --- Generate IKEK ---
	if(PED_LoadInitialKeyEncryptionKey(&tid[1]) == apiFailed)	// Download Initial Key Encryption Key (IKEK)
		return apiFailed;

	//Load IKEK ok
	PED_SetIkekStatus(1);

	return  apiOK;
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: ADMIN - Load Master/Session keys.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK  - OK
//           apiFailed - ERROR
// ---------------------------------------------------------------------------
UINT8	PED_ADMIN_LoadMasterSessionKey(void)
{
    UINT8   result = apiOK;
	UINT8	buffer[32];
	UINT8	tid[9];
	UINT32	len;


	LIB_LCD_Cls();
	LIB_LCD_Puts(0, 0, FONT1, sizeof(os_msg_LOAD_MSKEY), (UINT8 *)os_msg_LOAD_MSKEY);

	// Request TID(8)
	LIB_LCD_Puts(1, 0, FONT1, sizeof(os_msg_TERM_ID), (UINT8 *)os_msg_TERM_ID);

	// read current TID
	OS_SECM_GetData(ADDR_PED_TERM_ID, PED_TERM_ID_LEN, &tid[1]);
	tid[0] = PED_TERM_ID_LEN;

	if((tid[1] < '0') || (tid[1] > '9') || (tid[2] < '0') || (tid[2] > '9') ||
	   (tid[3] < '0') || (tid[3] > '9') || (tid[4] < '0') || (tid[4] > '9') ||
	   (tid[5] < '0') || (tid[5] > '9') || (tid[6] < '0') || (tid[6] > '9') ||
	   (tid[7] < '0') || (tid[7] > '9') || (tid[8] < '0') || (tid[8] > '9'))
	{
		// set default TID to TSN if illegal tid values
		api_sys_info(SID_TerminalSerialNumber, buffer);
		memmove(&tid[1], &buffer[4], 8);
	}

	// show TID
	LIB_LCD_Puts(1, 13, FONT1, PED_TERM_ID_LEN, &tid[1]);

	if(LIB_GetNumKey(0, NUM_TYPE_DIGIT + NUM_TYPE_LEADING_ZERO, '_', FONT1, 2, 8, buffer) == FALSE)
    {
        result = apiOK; // aborted
        goto EXIT;
    }

	if(!((buffer[0] == 1) && (buffer[1] == '0')))
	{
		if(buffer[0] != 8)
		{
			memset(&tid[1], 0x30, 8);
			len = PED_TERM_ID_LEN - buffer[0] + 1;
			memmove(&tid[len], &buffer[1], buffer[0]);
		}
		else
			memmove(&tid[1], &buffer[1], buffer[0]);

		// save new TID
		OS_SECM_PutData(ADDR_PED_TERM_ID, PED_TERM_ID_LEN, &tid[1]);
	}

	LIB_LCD_ClearRow(1, 2, FONT1);
	LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

	if(PED_VerifyKPKStatus(1) == FALSE)
	{
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);
		LIB_WaitTimeAndKey(300);
		result = apiFailed;
        goto EXIT;
	}

	// reset flag
	PED_SetKPKStatus(1, 0);

	// init communication port
	if(LIB_OpenAUX(COM1, auxDLL, COM_9600) == FALSE)
	{
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);
		LIB_WaitTimeAndKey(300);
		result = apiFailed; // device error
        goto EXIT;
	}
    
	// --- Generate Master/Session keys ---
	if(PED_LoadMasterSessionKey(&tid[1]) == apiFailed)
	{
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);

		// erase all keys
		PED_EraseSensitiveKeys();

		LIB_CloseAUX();

		LIB_WaitTimeAndKey(300);

		result = apiFailed;
        goto EXIT;
	}
	else
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_OK), (UINT8 *)os_msg_LOAD_KEY_OK);

	LIB_CloseAUX();

	LIB_WaitTimeAndKey(300);

EXIT:
    // clear sensitive data
	memset(buffer, 0x00, sizeof(buffer));
    memset(tid, 0x00, sizeof(tid));

	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: ADMIN - Load Master/Session keys for PIN Encryption Key (PEK).
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK  - OK
//           apiFailed - ERROR
// ---------------------------------------------------------------------------
UINT8	PED_ADMIN_LoadPEKMasterSessionKey(void)
{
    UINT8   result = apiOK;
	UINT8	buffer[32];
	UINT8	tid[9];
	UINT32	len;


	LIB_LCD_Cls();
	LIB_LCD_Puts(0, 0, FONT1, sizeof(os_msg_LOAD_MSKEY), (UINT8 *)os_msg_LOAD_MSKEY);

	// Request TID(8)
	LIB_LCD_Puts(1, 0, FONT1, sizeof(os_msg_TERM_ID), (UINT8 *)os_msg_TERM_ID);

	// read current TID
	OS_SECM_GetData(ADDR_PED_TERM_ID, PED_TERM_ID_LEN, &tid[1]);
	tid[0] = PED_TERM_ID_LEN;

	if((tid[1] < '0') || (tid[1] > '9') || (tid[2] < '0') || (tid[2] > '9') ||
	   (tid[3] < '0') || (tid[3] > '9') || (tid[4] < '0') || (tid[4] > '9') ||
	   (tid[5] < '0') || (tid[5] > '9') || (tid[6] < '0') || (tid[6] > '9') ||
	   (tid[7] < '0') || (tid[7] > '9') || (tid[8] < '0') || (tid[8] > '9'))
	{
		// set default TID to TSN if illegal tid values
		api_sys_info(SID_TerminalSerialNumber, buffer);
		memmove(&tid[1], &buffer[4], 8);
	}

	// show TID
	LIB_LCD_Puts(1, 13, FONT1, PED_TERM_ID_LEN, &tid[1]);

	if(LIB_GetNumKey(0, NUM_TYPE_DIGIT + NUM_TYPE_LEADING_ZERO, '_', FONT1, 2, 8, buffer) == FALSE)
    {
        result = apiOK; // aborted
        goto EXIT;
    }

	if(!((buffer[0] == 1) && (buffer[1] == '0')))
	{
		if(buffer[0] != 8)
		{
			memset(&tid[1], 0x30, 8);
			len = PED_TERM_ID_LEN - buffer[0] + 1;
			memmove(&tid[len], &buffer[1], buffer[0]);
		}
		else
			memmove(&tid[1], &buffer[1], buffer[0]);

		// save new TID
		OS_SECM_PutData(ADDR_PED_TERM_ID, PED_TERM_ID_LEN, &tid[1]);
	}

	LIB_LCD_ClearRow(1, 2, FONT1);
	LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

	// if(PED_VerifyKPKStatus(1) == FALSE)
	// {
	// 	LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);
	// 	LIB_WaitTimeAndKey(300);
	// 	return apiFailed;
	// }

	// reset flag
	// PED_SetKPKStatus(1, 0);

	// init communication port
	if(LIB_OpenAUX(COM1, auxDLL, COM_9600) == FALSE)
	{
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);
		LIB_WaitTimeAndKey(300);
		result = apiFailed; // device error
        goto EXIT;
	}
    
	// --- Generate Master/Session keys ---
	if(PED_LoadPEKMasterSessionKey(&tid[1]) == apiFailed)
	{
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);

		// erase all keys
		PED_EraseSensitiveKeys();

		LIB_CloseAUX();

		LIB_WaitTimeAndKey(300);

		result = apiFailed;
        goto EXIT;
	}
	else
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_OK), (UINT8 *)os_msg_LOAD_KEY_OK);

	LIB_CloseAUX();

	LIB_WaitTimeAndKey(300);

EXIT:
    // clear sensitive data
	memset(buffer, 0x00, sizeof(buffer));
    memset(tid, 0x00, sizeof(tid));

	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: ADMIN - Load DUKPT keys.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK  - OK
//           apiFailed - ERROR
// ---------------------------------------------------------------------------
UINT8	PED_ADMIN_LoadDukptKey(void)
{
    UINT8   result = apiOK;
	UINT8	buffer[32];
	UINT8	tid[9];
	UINT32	len;


	LIB_LCD_Cls();
	LIB_LCD_Puts(0, 0, FONT1, sizeof(os_msg_LOAD_DUKPT), (UINT8 *)os_msg_LOAD_DUKPT);

	// Request TID(8)
	LIB_LCD_Puts(1, 0, FONT1, sizeof(os_msg_TERM_ID), (UINT8 *)os_msg_TERM_ID);

	// read current TID
	OS_SECM_GetData(ADDR_PED_TERM_ID, PED_TERM_ID_LEN, &tid[1]);
	tid[0] = PED_TERM_ID_LEN;

	if((tid[1] < '0') || (tid[1] > '9') || (tid[2] < '0') || (tid[2] > '9') ||
	   (tid[3] < '0') || (tid[3] > '9') || (tid[4] < '0') || (tid[4] > '9') ||
	   (tid[5] < '0') || (tid[5] > '9') || (tid[6] < '0') || (tid[6] > '9') ||
	   (tid[7] < '0') || (tid[7] > '9') || (tid[8] < '0') || (tid[8] > '9'))
	{
		// set default TID to TSN if illegal tid values
		api_sys_info(SID_TerminalSerialNumber, buffer);
		memmove(&tid[1], &buffer[4], 8);
	}

	// show TID
	LIB_LCD_Puts(1, 13, FONT1, PED_TERM_ID_LEN, &tid[1]);

	if(LIB_GetNumKey(0, NUM_TYPE_DIGIT + NUM_TYPE_LEADING_ZERO, '_', FONT1, 2, 8, buffer) == FALSE)
    {
        result = apiOK; // aborted
        goto EXIT;
    }

	if(!((buffer[0] == 1) && (buffer[1] == '0')))
	{
		if(buffer[0] != 8)
		{
			memset(&tid[1], 0x30, 8);
			len = PED_TERM_ID_LEN - buffer[0] + 1;
			memmove(&tid[len], &buffer[1], buffer[0]);
		}
		else
			memmove(&tid[1], &buffer[1], buffer[0]);

		// save new TID
		OS_SECM_PutData(ADDR_PED_TERM_ID, PED_TERM_ID_LEN, &tid[1]);
	}

	LIB_LCD_ClearRow(1, 2, FONT1);
	LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

	if(PED_VerifyKPKStatus(1) == FALSE)
	{
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);
		LIB_WaitTimeAndKey(300);
		result = apiFailed;
        goto EXIT;
	}

	// reset flag
	PED_SetKPKStatus(1, 0);

	// init communication port
	if(LIB_OpenAUX(COM1, auxDLL, COM_9600) == FALSE)
	{
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);
		LIB_WaitTimeAndKey(300);
		result = apiFailed; // device error
        goto EXIT;
	}

	// --- Generate DUKPT Initial key ---
	if(PED_LoadDukptInitKey(&tid[1]) == apiFailed)
	{
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);

		// erase all keys
		PED_EraseSensitiveKeys();

		LIB_CloseAUX();

		LIB_WaitTimeAndKey(300);

		result = apiFailed;
        goto EXIT;
	}
	else
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_OK), (UINT8 *)os_msg_LOAD_KEY_OK);

	LIB_CloseAUX();

	LIB_WaitTimeAndKey(300);

EXIT:
    // clear sensitive data
	memset(buffer, 0x00, sizeof(buffer));
    memset(tid, 0x00, sizeof(tid));

	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: ADMIN - Load AES DUKPT keys.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK  - OK
//           apiFailed - ERROR
// ---------------------------------------------------------------------------
UINT8	PED_ADMIN_LoadAesDukptKey(void)
{
    UINT8   result = apiOK;
	UINT8	buffer[32];
	UINT8	tid[9];
	UINT32	len;


	LIB_LCD_Cls();
	LIB_LCD_Puts(0, 0, FONT1, sizeof(os_msg_LOAD_DUKPT), (UINT8 *)os_msg_LOAD_DUKPT);

	// Request TID(8)
	LIB_LCD_Puts(1, 0, FONT1, sizeof(os_msg_TERM_ID), (UINT8 *)os_msg_TERM_ID);

	// read current TID
	OS_SECM_GetData(ADDR_PED_TERM_ID, PED_TERM_ID_LEN, &tid[1]);
	tid[0] = PED_TERM_ID_LEN;

	if((tid[1] < '0') || (tid[1] > '9') || (tid[2] < '0') || (tid[2] > '9') ||
	   (tid[3] < '0') || (tid[3] > '9') || (tid[4] < '0') || (tid[4] > '9') ||
	   (tid[5] < '0') || (tid[5] > '9') || (tid[6] < '0') || (tid[6] > '9') ||
	   (tid[7] < '0') || (tid[7] > '9') || (tid[8] < '0') || (tid[8] > '9'))
	{
		// set default TID to TSN if illegal tid values
		api_sys_info(SID_TerminalSerialNumber, buffer);
		memmove(&tid[1], &buffer[4], 8);
	}

	// show TID
	LIB_LCD_Puts(1, 13, FONT1, PED_TERM_ID_LEN, &tid[1]);

	if(LIB_GetNumKey(0, NUM_TYPE_DIGIT + NUM_TYPE_LEADING_ZERO, '_', FONT1, 2, 8, buffer) == FALSE)
    {
        result = apiOK; // aborted
        goto EXIT;
    }

	if(!((buffer[0] == 1) && (buffer[1] == '0')))
	{
		if(buffer[0] != 8)
		{
			memset(&tid[1], 0x30, 8);
			len = PED_TERM_ID_LEN - buffer[0] + 1;
			memmove(&tid[len], &buffer[1], buffer[0]);
		}
		else
			memmove(&tid[1], &buffer[1], buffer[0]);

		// save new TID
		OS_SECM_PutData(ADDR_PED_TERM_ID, PED_TERM_ID_LEN, &tid[1]);
	}

	LIB_LCD_ClearRow(1, 2, FONT1);
	LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

	// if(PED_VerifyKPKStatus(1) == FALSE)
	// {
	// 	LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);
	// 	LIB_WaitTimeAndKey(300);
	// 	return apiFailed;
	// }

	// reset flag
	// PED_SetKPKStatus(1, 0);

	// init communication port
	if(LIB_OpenAUX(COM1, auxDLL, COM_9600) == FALSE)
	{
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);
		LIB_WaitTimeAndKey(300);
		result = apiFailed; // device error
        goto EXIT;
	}

	// --- Generate DUKPT Initial key ---
	if(PED_LoadAesDukptInitKey(&tid[1]) == apiFailed)
	{
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);

		// erase all keys
		PED_EraseSensitiveKeys();

		LIB_CloseAUX();

		LIB_WaitTimeAndKey(300);

		result = apiFailed;
        goto EXIT;
	}
	else
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_OK), (UINT8 *)os_msg_LOAD_KEY_OK);

	LIB_CloseAUX();

	LIB_WaitTimeAndKey(300);

EXIT:
    // clear sensitive data
	memset(buffer, 0x00, sizeof(buffer));
    memset(tid, 0x00, sizeof(tid));

	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: ADMIN - Load ISO4_KEY for ISO9564-1:2017 format 4.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK  - OK
//           apiFailed - ERROR
// ---------------------------------------------------------------------------
UINT8	PED_ADMIN_LoadISO4KEY(void)
{
    UINT8   result = apiOK;
	UINT8	buffer[32];
	UINT8	tid[9];
	UINT32	len;


	LIB_LCD_Cls();
	LIB_LCD_Puts(0, 0, FONT1, sizeof(os_msg_LOAD_ISO4KEY), (UINT8 *)os_msg_LOAD_ISO4KEY);

	// Request TID(8)
	LIB_LCD_Puts(1, 0, FONT1, sizeof(os_msg_TERM_ID), (UINT8 *)os_msg_TERM_ID);

	// read current TID
	OS_SECM_GetData(ADDR_PED_TERM_ID, PED_TERM_ID_LEN, &tid[1]);
	tid[0] = PED_TERM_ID_LEN;

	if((tid[1] < '0') || (tid[1] > '9') || (tid[2] < '0') || (tid[2] > '9') ||
	   (tid[3] < '0') || (tid[3] > '9') || (tid[4] < '0') || (tid[4] > '9') ||
	   (tid[5] < '0') || (tid[5] > '9') || (tid[6] < '0') || (tid[6] > '9') ||
	   (tid[7] < '0') || (tid[7] > '9') || (tid[8] < '0') || (tid[8] > '9'))
	{
		// set default TID to TSN if illegal tid values
		api_sys_info(SID_TerminalSerialNumber, buffer);
		memmove(&tid[1], &buffer[4], 8);
	}

	// show TID
	LIB_LCD_Puts(1, 13, FONT1, PED_TERM_ID_LEN, &tid[1]);

	if(LIB_GetNumKey(0, NUM_TYPE_DIGIT + NUM_TYPE_LEADING_ZERO, '_', FONT1, 2, 8, buffer) == FALSE)
    {
        result = apiOK; // aborted
        goto EXIT;
    }

	if(!((buffer[0] == 1) && (buffer[1] == '0')))
	{
		if(buffer[0] != 8)
		{
			memset(&tid[1], 0x30, 8);
			len = PED_TERM_ID_LEN - buffer[0] + 1;
			memmove(&tid[len], &buffer[1], buffer[0]);
		}
		else
			memmove(&tid[1], &buffer[1], buffer[0]);

		// save new TID
		OS_SECM_PutData(ADDR_PED_TERM_ID, PED_TERM_ID_LEN, &tid[1]);
	}

	LIB_LCD_ClearRow(1, 2, FONT1);
	LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

	if(PED_VerifyKPKStatus(5) == FALSE)
	{
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);
		LIB_WaitTimeAndKey(300);
		result = apiFailed;
        goto EXIT;
	}

	// reset flag
	PED_SetKPKStatus(5, 0);

	// init communication port
	if(LIB_OpenAUX(COM1, auxDLL, COM_9600) == FALSE)
	{
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);
		LIB_WaitTimeAndKey(300);
		result = apiFailed; // device error
        goto EXIT;
	}

	// --- Generate ISO4_KEY ---
	//if(PED_LoadISO4MasterSessionKey(&tid[1]) == apiFailed)
	if(PED_LoadISO4KEY(&tid[1]) == apiFailed)
	{
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);

		// erase all keys
		PED_EraseSensitiveKeys();

		LIB_CloseAUX();

		LIB_WaitTimeAndKey(300);

		result = apiFailed;
        goto EXIT;
	}
	else
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_OK), (UINT8 *)os_msg_LOAD_KEY_OK);

	LIB_CloseAUX();

	LIB_WaitTimeAndKey(300);

EXIT:
    // clear sensitive data
	memset(buffer, 0x00, sizeof(buffer));
    memset(tid, 0x00, sizeof(tid));

	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: ADMIN - Load Account Data Encryption Key (ACC_DEK).
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK  - OK
//           apiFailed - ERROR
// ---------------------------------------------------------------------------
UINT8	PED_ADMIN_LoadAccDEK(void)
{
    UINT8   result = apiOK;
	UINT8	buffer[32];
	UINT8	tid[9];
	UINT32	len;


	LIB_LCD_Cls();
	LIB_LCD_Puts(0, 0, FONT1, sizeof(os_msg_LOAD_ACCDEK), (UINT8 *)os_msg_LOAD_ACCDEK);

	// Request TID(8)
	LIB_LCD_Puts(1, 0, FONT1, sizeof(os_msg_TERM_ID), (UINT8 *)os_msg_TERM_ID);

	// read current TID
	OS_SECM_GetData(ADDR_PED_TERM_ID, PED_TERM_ID_LEN, &tid[1]);
	tid[0] = PED_TERM_ID_LEN;

	if((tid[1] < '0') || (tid[1] > '9') || (tid[2] < '0') || (tid[2] > '9') ||
	   (tid[3] < '0') || (tid[3] > '9') || (tid[4] < '0') || (tid[4] > '9') ||
	   (tid[5] < '0') || (tid[5] > '9') || (tid[6] < '0') || (tid[6] > '9') ||
	   (tid[7] < '0') || (tid[7] > '9') || (tid[8] < '0') || (tid[8] > '9'))
	{
		// set default TID to TSN if illegal tid values
		api_sys_info(SID_TerminalSerialNumber, buffer);
		memmove(&tid[1], &buffer[4], 8);
	}

	// show TID
	LIB_LCD_Puts(1, 13, FONT1, PED_TERM_ID_LEN, &tid[1]);

	if(LIB_GetNumKey(0, NUM_TYPE_DIGIT + NUM_TYPE_LEADING_ZERO, '_', FONT1, 2, 8, buffer) == FALSE)
    {
        result = apiOK; // aborted
        goto EXIT;
    }

	if(!((buffer[0] == 1) && (buffer[1] == '0')))
	{
		if(buffer[0] != 8)
		{
			memset(&tid[1], 0x30, 8);
			len = PED_TERM_ID_LEN - buffer[0] + 1;
			memmove(&tid[len], &buffer[1], buffer[0]);
		}
		else
			memmove(&tid[1], &buffer[1], buffer[0]);

		// save new TID
		OS_SECM_PutData(ADDR_PED_TERM_ID, PED_TERM_ID_LEN, &tid[1]);
	}

	LIB_LCD_ClearRow(1, 2, FONT1);
	LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

	if(PED_VerifyKPKStatus(1) == FALSE)
	{
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);
		LIB_WaitTimeAndKey(300);
		result = apiFailed;
        goto EXIT;
	}

	// reset flag
	PED_SetKPKStatus(1, 0);

	// init communication port
	if(LIB_OpenAUX(COM1, auxDLL, COM_9600) == FALSE)
	{
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);
		LIB_WaitTimeAndKey(300);
		result = apiFailed; // device error
        goto EXIT;
	}

	// --- Generate ACC_DEK ---
	if(PED_LoadAccDEKMasterSessionKey(&tid[1]) == apiFailed)
	{
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);

		// erase all keys
		PED_EraseSensitiveKeys();

		LIB_CloseAUX();

		LIB_WaitTimeAndKey(300);

		result = apiFailed;
        goto EXIT;
	}
	else
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_OK), (UINT8 *)os_msg_LOAD_KEY_OK);

	LIB_CloseAUX();

	LIB_WaitTimeAndKey(300);

EXIT:
    // clear sensitive data
	memset(buffer, 0x00, sizeof(buffer));
    memset(tid, 0x00, sizeof(tid));

	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: ADMIN - Load Account Data Encryption Key (ACC_DEK).
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK  - OK
//           apiFailed - ERROR
// ---------------------------------------------------------------------------
UINT8	PED_ADMIN_LoadAccDEKMasterSessionKey(void)
{
    UINT8   result = apiOK;
	UINT8	buffer[32];
	UINT8	tid[9];
	UINT32	len;


	LIB_LCD_Cls();
	LIB_LCD_Puts(0, 0, FONT1, sizeof(os_msg_LOAD_ACCDEK), (UINT8 *)os_msg_LOAD_ACCDEK);

	// Request TID(8)
	LIB_LCD_Puts(1, 0, FONT1, sizeof(os_msg_TERM_ID), (UINT8 *)os_msg_TERM_ID);

	// read current TID
	OS_SECM_GetData(ADDR_PED_TERM_ID, PED_TERM_ID_LEN, &tid[1]);
	tid[0] = PED_TERM_ID_LEN;

	if((tid[1] < '0') || (tid[1] > '9') || (tid[2] < '0') || (tid[2] > '9') ||
	   (tid[3] < '0') || (tid[3] > '9') || (tid[4] < '0') || (tid[4] > '9') ||
	   (tid[5] < '0') || (tid[5] > '9') || (tid[6] < '0') || (tid[6] > '9') ||
	   (tid[7] < '0') || (tid[7] > '9') || (tid[8] < '0') || (tid[8] > '9'))
	{
		// set default TID to TSN if illegal tid values
		api_sys_info(SID_TerminalSerialNumber, buffer);
		memmove(&tid[1], &buffer[4], 8);
	}

	// show TID
	LIB_LCD_Puts(1, 13, FONT1, PED_TERM_ID_LEN, &tid[1]);

	if(LIB_GetNumKey(0, NUM_TYPE_DIGIT + NUM_TYPE_LEADING_ZERO, '_', FONT1, 2, 8, buffer) == FALSE)
    {
        result = apiOK; // aborted
        goto EXIT;
    }

	if(!((buffer[0] == 1) && (buffer[1] == '0')))
	{
		if(buffer[0] != 8)
		{
			memset(&tid[1], 0x30, 8);
			len = PED_TERM_ID_LEN - buffer[0] + 1;
			memmove(&tid[len], &buffer[1], buffer[0]);
		}
		else
			memmove(&tid[1], &buffer[1], buffer[0]);

		// save new TID
		OS_SECM_PutData(ADDR_PED_TERM_ID, PED_TERM_ID_LEN, &tid[1]);
	}

	LIB_LCD_ClearRow(1, 2, FONT1);
	LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

	// if(PED_VerifyKPKStatus(1) == FALSE)
	// {
	// 	LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);
	// 	LIB_WaitTimeAndKey(300);
	// 	return apiFailed;
	// }

	// reset flag
	// PED_SetKPKStatus(1, 0);

	// init communication port
	if(LIB_OpenAUX(COM1, auxDLL, COM_9600) == FALSE)
	{
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);
		LIB_WaitTimeAndKey(300);
		result = apiFailed; // device error
        goto EXIT;
	}

	// --- Generate ACC_DEK ---
	if(PED_LoadAccDEKMasterSessionKey(&tid[1]) == apiFailed)
	{
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);

		// erase all keys
		PED_EraseSensitiveKeys();

		LIB_CloseAUX();

		LIB_WaitTimeAndKey(300);

		result = apiFailed;
        goto EXIT;
	}
	else
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_OK), (UINT8 *)os_msg_LOAD_KEY_OK);

	LIB_CloseAUX();

	LIB_WaitTimeAndKey(300);

EXIT:
    // clear sensitive data
	memset(buffer, 0x00, sizeof(buffer));
    memset(tid, 0x00, sizeof(tid));

	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: ADMIN - Load Format-Preserving Encryption Key (FPE Key).
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK  - OK
//           apiFailed - ERROR
// ---------------------------------------------------------------------------
UINT8	PED_ADMIN_LoadFPEKey(void)
{
    UINT8   result = apiOK;
	UINT8	buffer[32];
	UINT8	tid[9];
	UINT32	len;


	LIB_LCD_Cls();
	LIB_LCD_Puts(0, 0, FONT1, sizeof(os_msg_LOAD_FPEKEY), (UINT8 *)os_msg_LOAD_FPEKEY);

	// Request TID(8)
	LIB_LCD_Puts(1, 0, FONT1, sizeof(os_msg_TERM_ID), (UINT8 *)os_msg_TERM_ID);

	// read current TID
	OS_SECM_GetData(ADDR_PED_TERM_ID, PED_TERM_ID_LEN, &tid[1]);
	tid[0] = PED_TERM_ID_LEN;

	if((tid[1] < '0') || (tid[1] > '9') || (tid[2] < '0') || (tid[2] > '9') ||
	   (tid[3] < '0') || (tid[3] > '9') || (tid[4] < '0') || (tid[4] > '9') ||
	   (tid[5] < '0') || (tid[5] > '9') || (tid[6] < '0') || (tid[6] > '9') ||
	   (tid[7] < '0') || (tid[7] > '9') || (tid[8] < '0') || (tid[8] > '9'))
	{
		// set default TID to TSN if illegal tid values
		api_sys_info(SID_TerminalSerialNumber, buffer);
		memmove(&tid[1], &buffer[4], 8);
	}

	// show TID
	LIB_LCD_Puts(1, 13, FONT1, PED_TERM_ID_LEN, &tid[1]);

	if(LIB_GetNumKey(0, NUM_TYPE_DIGIT + NUM_TYPE_LEADING_ZERO, '_', FONT1, 2, 8, buffer) == FALSE)
    {
        result = apiOK; // aborted
        goto EXIT;
    }

	if(!((buffer[0] == 1) && (buffer[1] == '0')))
	{
		if(buffer[0] != 8)
		{
			memset(&tid[1], 0x30, 8);
			len = PED_TERM_ID_LEN - buffer[0] + 1;
			memmove(&tid[len], &buffer[1], buffer[0]);
		}
		else
			memmove(&tid[1], &buffer[1], buffer[0]);

		// save new TID
		OS_SECM_PutData(ADDR_PED_TERM_ID, PED_TERM_ID_LEN, &tid[1]);
	}

	LIB_LCD_ClearRow(1, 2, FONT1);
	LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

	if(PED_VerifyKPKStatus(5) == FALSE)
	{
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);
		LIB_WaitTimeAndKey(300);
		result = apiFailed;
        goto EXIT;
	}

	// reset flag
	PED_SetKPKStatus(5, 0);

	// init communication port
	if(LIB_OpenAUX(COM1, auxDLL, COM_9600) == FALSE)
	{
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);
		LIB_WaitTimeAndKey(300);
		result = apiFailed; // device error
        goto EXIT;
	}

	// --- Generate FPE Key ---
	//if(PED_LoadFPEMasterSessionKey(&tid[1]) == apiFailed)
	if(PED_LoadFPEKey(&tid[1]) == apiFailed)
	{
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);

		// erase all keys
		PED_EraseSensitiveKeys();

		LIB_CloseAUX();

		LIB_WaitTimeAndKey(300);

		result = apiFailed;
        goto EXIT;
	}
	else
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_OK), (UINT8 *)os_msg_LOAD_KEY_OK);

	LIB_CloseAUX();

	LIB_WaitTimeAndKey(300);

EXIT:
    // clear sensitive data
	memset(buffer, 0x00, sizeof(buffer));
    memset(tid, 0x00, sizeof(tid));

	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: ADMIN - Load Format-Preserving Encryption Key (FPE Key).
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK  - OK
//           apiFailed - ERROR
// ---------------------------------------------------------------------------
UINT8	PED_ADMIN_LoadFPEMasterSessionKey(void)
{
    UINT8   result = apiOK;
	UINT8	buffer[32];
	UINT8	tid[9];
	UINT32	len;


	LIB_LCD_Cls();
	LIB_LCD_Puts(0, 0, FONT1, sizeof(os_msg_LOAD_FPEKEY), (UINT8 *)os_msg_LOAD_FPEKEY);

	// Request TID(8)
	LIB_LCD_Puts(1, 0, FONT1, sizeof(os_msg_TERM_ID), (UINT8 *)os_msg_TERM_ID);

	// read current TID
	OS_SECM_GetData(ADDR_PED_TERM_ID, PED_TERM_ID_LEN, &tid[1]);
	tid[0] = PED_TERM_ID_LEN;

	if((tid[1] < '0') || (tid[1] > '9') || (tid[2] < '0') || (tid[2] > '9') ||
	   (tid[3] < '0') || (tid[3] > '9') || (tid[4] < '0') || (tid[4] > '9') ||
	   (tid[5] < '0') || (tid[5] > '9') || (tid[6] < '0') || (tid[6] > '9') ||
	   (tid[7] < '0') || (tid[7] > '9') || (tid[8] < '0') || (tid[8] > '9'))
	{
		// set default TID to TSN if illegal tid values
		api_sys_info(SID_TerminalSerialNumber, buffer);
		memmove(&tid[1], &buffer[4], 8);
	}

	// show TID
	LIB_LCD_Puts(1, 13, FONT1, PED_TERM_ID_LEN, &tid[1]);

	if(LIB_GetNumKey(0, NUM_TYPE_DIGIT + NUM_TYPE_LEADING_ZERO, '_', FONT1, 2, 8, buffer) == FALSE)
    {
        result = apiOK; // aborted
        goto EXIT;
    }

	if(!((buffer[0] == 1) && (buffer[1] == '0')))
	{
		if(buffer[0] != 8)
		{
			memset(&tid[1], 0x30, 8);
			len = PED_TERM_ID_LEN - buffer[0] + 1;
			memmove(&tid[len], &buffer[1], buffer[0]);
		}
		else
			memmove(&tid[1], &buffer[1], buffer[0]);

		// save new TID
		OS_SECM_PutData(ADDR_PED_TERM_ID, PED_TERM_ID_LEN, &tid[1]);
	}

	LIB_LCD_ClearRow(1, 2, FONT1);
	LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

	// if(PED_VerifyKPKStatus(5) == FALSE)
	// {
	// 	LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);
	// 	LIB_WaitTimeAndKey(300);
	// 	return apiFailed;
	// }

	// reset flag
	// PED_SetKPKStatus(5, 0);

	// init communication port
	if(LIB_OpenAUX(COM1, auxDLL, COM_9600) == FALSE)
	{
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);
		LIB_WaitTimeAndKey(300);
		result = apiFailed; // device error
        goto EXIT;
	}

	// --- Generate FPE Key ---
	if(PED_LoadFPEMasterSessionKey(&tid[1]) == apiFailed)
	{
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);

		// erase all keys
		PED_EraseSensitiveKeys();

		LIB_CloseAUX();

		LIB_WaitTimeAndKey(300);

		result = apiFailed;
        goto EXIT;
	}
	else
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_OK), (UINT8 *)os_msg_LOAD_KEY_OK);

	LIB_CloseAUX();

	LIB_WaitTimeAndKey(300);

EXIT:
    // clear sensitive data
	memset(buffer, 0x00, sizeof(buffer));
    memset(tid, 0x00, sizeof(tid));

	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: ADMIN - Load CA public key (CAPK).
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK  - OK
//           apiFailed - ERROR
// ---------------------------------------------------------------------------
UINT8	PED_ADMIN_LoadCAPK(void)
{
    UINT8   result = apiOK;
	UINT8	buffer[32];
	UINT8	tid[9];
	UINT32	len;


	LIB_LCD_Cls();
	LIB_LCD_Puts(0, 0, FONT1, sizeof(os_msg_LOAD_CAPK), (UINT8 *)os_msg_LOAD_CAPK);

	// Request TID(8)
	LIB_LCD_Puts(1, 0, FONT1, sizeof(os_msg_TERM_ID), (UINT8 *)os_msg_TERM_ID);

	// read current TID
	OS_SECM_GetData(ADDR_PED_TERM_ID, PED_TERM_ID_LEN, &tid[1]);
	tid[0] = PED_TERM_ID_LEN;

	if((tid[1] < '0') || (tid[1] > '9') || (tid[2] < '0') || (tid[2] > '9') ||
	   (tid[3] < '0') || (tid[3] > '9') || (tid[4] < '0') || (tid[4] > '9') ||
	   (tid[5] < '0') || (tid[5] > '9') || (tid[6] < '0') || (tid[6] > '9') ||
	   (tid[7] < '0') || (tid[7] > '9') || (tid[8] < '0') || (tid[8] > '9'))
	{
		// set default TID to TSN if illegal tid values
		api_sys_info(SID_TerminalSerialNumber, buffer);
		memmove(&tid[1], &buffer[4], 8);
	}

	// show TID
	LIB_LCD_Puts(1, 13, FONT1, PED_TERM_ID_LEN, &tid[1]);

	if(LIB_GetNumKey(0, NUM_TYPE_DIGIT + NUM_TYPE_LEADING_ZERO, '_', FONT1, 2, 8, buffer) == FALSE)
    {
        result = apiOK; // aborted
        goto EXIT;
    }

	if(!((buffer[0] == 1) && (buffer[1] == '0')))
	{
		if(buffer[0] != 8)
		{
			memset(&tid[1], 0x30, 8);
			len = PED_TERM_ID_LEN - buffer[0] + 1;
			memmove(&tid[len], &buffer[1], buffer[0]);
		}
		else
			memmove(&tid[1], &buffer[1], buffer[0]);

		// save new TID
		OS_SECM_PutData(ADDR_PED_TERM_ID, PED_TERM_ID_LEN, &tid[1]);
	}

	LIB_LCD_ClearRow(1, 2, FONT1);
	LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

	//if(PED_VerifyKPKStatus(5) == FALSE)
	//{
	//	LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);
	//	LIB_WaitTimeAndKey(300);
	//	return apiFailed;
	//}

	// reset flag
	//PED_SetKPKStatus(5, 0);

	// init communication port
	if(LIB_OpenAUX(COM1, auxDLL, COM_9600) == FALSE)
	{
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);
		LIB_WaitTimeAndKey(300);
		result = apiFailed; // device error
        goto EXIT;
	}

	// --- Generate CAPK ---
	if(PED_LoadCaPublicKey(&tid[1]) == apiFailed)
	{
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);

		// erase all keys
		PED_EraseSensitiveKeys();

		LIB_CloseAUX();

		LIB_WaitTimeAndKey(300);

		result = apiFailed;
        goto EXIT;
	}
	else
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_OK), (UINT8 *)os_msg_LOAD_KEY_OK);

	LIB_CloseAUX();

	LIB_WaitTimeAndKey(300);

EXIT:
    // clear sensitive data
	memset(buffer, 0x00, sizeof(buffer));
    memset(tid, 0x00, sizeof(tid));

	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: Reset secure memory.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
UINT8	PED_ADMIN_ResetSecureMemory(void)
{
	UINT8	result = FALSE;


	LIB_LCD_Cls();
	LIB_LCD_Puts(0, 0, FONT1, sizeof(os_msg_RESET_SECM), (UINT8 *)os_msg_RESET_SECM);

	if(LIB_WaitKeyMsgYesNoTO(3, COL_LEFTMOST, sizeof(os_msg_Q_RESET), (UINT8 *)os_msg_Q_RESET) == TRUE)
	{
		LIB_LCD_PutMsg(1, COL_MIDWAY, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

		// OS_SECM_ClearData(0, SEC_MEMORY_SIZE, 0x00);
        OS_SECM_ClearAllSecureFile();

		LIB_LCD_PutMsg(3, COL_RIGHTMOST, FONT1 + attrCLEARWRITE, sizeof(os_msg_OK), (UINT8 *)os_msg_OK);
		LIB_WaitTimeAndKey(100);

		return TRUE;
	};

	return FALSE;
}

// ---------------------------------------------------------------------------
// FUNCTION: Setup SRED status.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK     - OK
//           apiFailed - ERROR
// ---------------------------------------------------------------------------
UINT8	PED_ADMIN_SetupSRED(void)
{
	UINT8	key;


	LIB_LCD_Cls();
	LIB_LCD_Puts(0, 0, FONT1, sizeof(os_msg_SETUP_SRED), (UINT8 *)os_msg_SETUP_SRED);
	LIB_LCD_Puts(4, 0, FONT1, sizeof(os_msg_DISABLE), (UINT8 *)os_msg_DISABLE);
	LIB_LCD_Puts(5, 0, FONT1, sizeof(os_msg_ENABLE), (UINT8 *)os_msg_ENABLE);

	while(1)
	{
		key = LIB_WaitTimeAndKey(3000);
		if(key == '0')	// Disable SRED
		{
			if(OS_SECM_SetSredStatus(0) == TRUE)
				return apiOK;
			else
				return apiFailed;
		}
		else if(key == '1')	// Enable SRED
		{
			if(OS_SECM_SetSredStatus(1) == TRUE)
				return apiOK;
			else
				return apiFailed;
		}
		else if(key == 255)	// Timeout
			return apiFailed;
	}
}

// ---------------------------------------------------------------------------
// FUNCTION: ADMIN - Load Manufacturer's RSA public key (MFG_PUB_KEY) and
//                   EDC certificate (EDC_PUB_CERT).
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK  - OK
//           apiFailed - ERROR
//                 EDC                                            SCD
//           ---------------                                ---------------
//           CMD(1)+TSN(8)+TID(8)                    ->
//                                                   <-     CMD(1)+MFG_PUB_KEY(n)
//                                                   ... 
//           CMD(1)+TSN(8)+TID(8)                    ->
//                                                   <-     CMD(1)+MFG_PUB_KEY(n)
//           CMD(1)+TSN(8)+TID(8)                    ->
//                                                   <-     CMD(1)
//
//           CMD(1)+TSN(8)+TID(8)+EDC_PUB_KEY(n)     ->
//                                                   <-     CMD(1)
//                                                   ...
//           CMD(1)+TSN(8)+TID(8)+EDC_PUB_KEY(n)     ->
//                                                   <-     CMD(1)
//           CMD(1)+TSN(8)+TID(8)                    ->
//                                                   <-     CMD(1)
//
//           CMD(1)+TSN(8)+TID(8)                    ->
//                                                   <-     CMD(1)+EDC_PUB_CERT(n)
//                                                   ...
//           CMD(1)+TSN(8)+TID(8)                    ->
//                                                   <-     CMD(1)+EDC_PUB_CERT(n)
//           CMD(1)+TSN(8)+TID(8)                    ->
//                                                   <-     CMD(1)
// ---------------------------------------------------------------------------
UINT8	PED_ADMIN_DeviceAuthenticationPhase1(void)
{
    UINT8	buffer[32];
	UINT8	tid[9];
	UINT32	len;
    UINT8	temp[32];
	UINT8	buf1[257];
	UINT8	buf2[257];
	UINT8	nextblk;
	UINT8	result;
	UINT8	index;
    UINT32	iLen;
    UINT32	totalLen = 0;
    UINT32  offset;
    UINT8   maxSize = 238;  //255-1-16
    UINT8   transferCount;
    UINT8   lastSize;
    UINT8   *mfgPubKey;
    UINT32  edcPubKeySize = 0;
    UINT8   *edcPubKey;
    UINT8   *edcPubCert;
    FILE    *fptr;
    int     ret;


    LIB_LCD_Cls();
    LIB_LCD_Puts(0, 0, FONT0, sizeof(os_msg_DEV_AUTH_1), (UINT8 *)os_msg_DEV_AUTH_1);

    // Request TID(8)
	LIB_LCD_Puts(1, 0, FONT0, sizeof(os_msg_TERM_ID), (UINT8 *)os_msg_TERM_ID);

	// read current TID
	OS_SECM_GetData(ADDR_PED_TERM_ID, PED_TERM_ID_LEN, &tid[1]);
	tid[0] = PED_TERM_ID_LEN;

	if((tid[1] < '0') || (tid[1] > '9') || (tid[2] < '0') || (tid[2] > '9') ||
	   (tid[3] < '0') || (tid[3] > '9') || (tid[4] < '0') || (tid[4] > '9') ||
	   (tid[5] < '0') || (tid[5] > '9') || (tid[6] < '0') || (tid[6] > '9') ||
	   (tid[7] < '0') || (tid[7] > '9') || (tid[8] < '0') || (tid[8] > '9'))
	{
		// set default TID to TSN if illegal tid values
		api_sys_info(SID_TerminalSerialNumber, buffer);
		memmove(&tid[1], &buffer[4], 8);
	}

	// show TID
	LIB_LCD_Puts(1, 13, FONT0, PED_TERM_ID_LEN, &tid[1]);

	if(LIB_GetNumKey(0, NUM_TYPE_DIGIT + NUM_TYPE_LEADING_ZERO, '_', FONT0, 2, 8, buffer) == FALSE)
		return(apiOK); // aborted

	if(!((buffer[0] == 1) && (buffer[1] == '0')))
	{
		if(buffer[0] != 8)
		{
			memset(&tid[1], 0x30, 8);
			len = PED_TERM_ID_LEN - buffer[0] + 1;
			memmove(&tid[len], &buffer[1], buffer[0]);
		}
		else
			memmove(&tid[1], &buffer[1], buffer[0]);

		// save new TID
		OS_SECM_PutData(ADDR_PED_TERM_ID, PED_TERM_ID_LEN, &tid[1]);
	}

	LIB_LCD_ClearRow(1, 2, FONT0);
	LIB_LCD_PutMsg(2, COL_MIDWAY, FONT0 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

    // init communication port
	if(LIB_OpenAUX(COM1, auxDLL, COM_9600) == FALSE)
	{
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT0 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);
		LIB_WaitTimeAndKey(300);
		return apiFailed; // device error
	}

    //--- Load MFG_PUB_KEY ---
    //      EDC                             SCD
    //---------------                 ---------------
    //CMD(1)+TSN(8)+TID(8)     ->
    //                         <-     CMD(1)+MFG_PUB_KEY(n)
    //                         ... 
    //CMD(1)+TSN(8)+TID(8)     ->
    //                         <-     CMD(1)+MFG_PUB_KEY(n)
    //CMD(1)+TSN(8)+TID(8)     ->
    //                         <-     CMD(1)

    printf(">>>>>>Load MFG_PUB_KEY===============================================\n");    // ==== [Debug] ====

    nextblk = 0x00;
    result = apiOK;
    offset = 0;

    mfgPubKey = malloc(640 * sizeof(UINT8));
    if(mfgPubKey == NULL)
    {
        perror("unable to allocate required memory");
        result = apiFailed;
        goto EXIT;
    }

	while(result == apiOK)
	{
		// Terminal Serial Number (TSN)
        api_sys_info(SID_TerminalSerialNumber, temp);
        memmove(buf2, &temp[4], 8);

        // Terminal ID (TID)
        memmove(&buf2[8], &tid[1], 8);

        memmove(&buf1[3], buf2, 16);
        buf1[0] = 17;
        buf1[1] = 0;
        buf1[2] = KMC_DEVICE_AUTH_PHASE_1 | nextblk; // command

        result = apiFailed;

        // send CMD(1) + TSN(8) + TID(8) to HOST for download MFG_PUB_KEY
        if(LIB_TransmitAUX(buf1) == TRUE)
        {
            if(LIB_ReceiveAUX(buf2) == TRUE)
            {
                if(buf2[2] == buf1[2])  // check command code
                {
                    iLen = buf2[0] + buf2[1] * 256;

                    if(iLen > 1)
                    {
                        memmove(&mfgPubKey[offset], &buf2[3], iLen - 1); // without CMD(1)

                        offset += iLen - 1;
                        totalLen += iLen - 1;
                    }
                    else
                    {
                        if(iLen == 1)   // end of record?
                            result = apiOK;
                        
                        break;
                    }

                    nextblk = KMC_NEXT;
                    result = apiOK;
                }
            }
        }
    }   // while()

    if(result == apiFailed)
        goto EXIT;

    fptr = fopen("/home/root/MFG_PUB_KEY.pem", "wb+");
    if(fptr == NULL)
    {
        perror("open MFG_PUB_KEY.pem failed");
        result = apiFailed;
        goto EXIT;
    }

    ret = fwrite(mfgPubKey, totalLen, 1, fptr);
    if(!ret)
    {
        perror("fwrite fail");
        fclose(fptr);

        result = apiFailed;
        goto EXIT;
    }
    else
    {
        sync();
        fclose(fptr);
    }

    //--- Send EDC_PUB_KEY ---
    //      EDC                                            SCD
    //---------------                                ---------------
    //CMD(1)+TSN(8)+TID(8)+EDC_PUB_KEY(n)     ->
    //                                        <-     CMD(1)
    //                                        ...
    //CMD(1)+TSN(8)+TID(8)+EDC_PUB_KEY(n)     ->
    //                                        <-     CMD(1)
    //CMD(1)+TSN(8)+TID(8)                    ->
    //                                        <-     CMD(1)

    printf(">>>>>>Send EDC_PUB_KEY===============================================\n");    // ==== [Debug] ====

    fptr = fopen("/home/root/EDC_PUB_KEY.pem", "rb");
    if(fptr == NULL)
    {
        printf("open EDC_PUB_KEY.pem failed\n");
        result = apiFailed;
        goto EXIT;
    }

    fseek(fptr, 0, SEEK_END);
    edcPubKeySize = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);

    edcPubKey = malloc(edcPubKeySize);
    if(edcPubKey == NULL)
    {
        perror("unable to allocate required memory");
        result = apiFailed;
        goto EXIT;
    }

    fread(edcPubKey, edcPubKeySize, 1, fptr);

    transferCount = edcPubKeySize / maxSize;
    lastSize = edcPubKeySize % maxSize;
    if(lastSize > 0)
        transferCount++;
    
    offset = 0;

    while(result == apiOK)
    {
        // Terminal Serial Number (TSN)
        api_sys_info(SID_TerminalSerialNumber, temp);
        memmove(buf2, &temp[4], 8);

        // Terminal ID (TID)
        memmove(&buf2[8], &tid[1], 8);

        memmove(&buf1[3], buf2, 16);
        // buf1[0] = 17;
        buf1[1] = 0;
        buf1[2] = KMC_DEVICE_AUTH_PHASE_1 | nextblk; // command

        if(transferCount > 1)
        {
            // EDC_PUB_KEY
            memmove(&buf1[19], &edcPubKey[offset], maxSize);
            offset += maxSize;

            buf1[0] = 17 + maxSize;
        }
        else if(transferCount == 1)
        {
            if(lastSize > 0)
            {
                memmove(&buf1[19], &edcPubKey[offset], lastSize);
                offset += lastSize;

                buf1[0] = 17 + lastSize;
            }
            else
            {
                memmove(&buf1[19], &edcPubKey[offset], maxSize);
                offset += maxSize;

                buf1[0] = 17 + maxSize;
            }
        }
        else if(transferCount == 0)
        {
            // send CMD(1) + TSN(8) + TID(8) to SCD
            buf1[0] = 17;
        }

        result = apiFailed;
        
        // send CMD(1) + TSN(8) + TID(8) + EDC_PUB_KEY(n) to SCD
        if(LIB_TransmitAUX(buf1) == TRUE)
        {
            if(LIB_ReceiveAUX(buf2) == TRUE)
            {
                if(buf2[2] == buf1[2])  // check command code
                {
                    iLen = buf2[0] + buf2[1] * 256;

                    if(iLen == 1)
                        result = apiOK;

                    if(transferCount == 0)
                        break;
                    
                    transferCount--;
                }
            }
        }
    }   // while()

    fclose(fptr);
    free(edcPubKey);

    if(result == apiFailed)
        goto EXIT;

    //--- Load EDC_PUB_CERT ---
    //EDC_PUB_CERT = EDC_PUB_KEY || Sign(MFG_PRV_KEY, EDC_PUB_KEY)

    //      EDC                             SCD
    //---------------                 ---------------
    //CMD(1)+TSN(8)+TID(8)     ->
    //                         <-     CMD(1)+EDC_PUB_CERT(n)
    //                         ...
    //CMD(1)+TSN(8)+TID(8)     ->
    //                         <-     CMD(1)+EDC_PUB_CERT(n)
    //CMD(1)+TSN(8)+TID(8)     ->
    //                         <-     CMD(1)

    printf(">>>>>>Load EDC_PUB_CERT===============================================\n");    // ==== [Debug] ====

    offset = 0;
    totalLen = 0;

    edcPubCert = malloc(1024 * sizeof(UINT8));
    if(edcPubCert == NULL)
    {
        perror("unable to allocate required memory");
        result = apiFailed;
        goto EXIT;
    }

	while(result == apiOK)
    {
        // Terminal Serial Number (TSN)
        api_sys_info(SID_TerminalSerialNumber, temp);
        memmove(buf2, &temp[4], 8);

        // Terminal ID (TID)
        memmove(&buf2[8], &tid[1], 8);

        memmove(&buf1[3], buf2, 16);
        buf1[0] = 17;
        buf1[1] = 0;
        buf1[2] = KMC_DEVICE_AUTH_PHASE_1 | nextblk; // command

        result = apiFailed;

        // send CMD(1) + TSN(8) + TID(8) to SCD for download EDC_PUB_CERT
        if(LIB_TransmitAUX(buf1) == TRUE)
        {
            if(LIB_ReceiveAUX(buf2) == TRUE)
            {
                if(buf2[2] == buf1[2])  // check command code
                {
                    iLen = buf2[0] + buf2[1] * 256;

                    if(iLen > 1)
                    {
                        memmove(&edcPubCert[offset], &buf2[3], iLen - 1); // without CMD(1)

                        offset += iLen - 1;
                        totalLen += iLen - 1;
                    }
                    else
                    {
                        if(iLen == 1)   // end of record?
                            result = apiOK;
                        
                        break;
                    }

                    nextblk = KMC_NEXT;
                    result = apiOK;
                }
            }
        }
    }   // while()

    if(result == apiFailed)
        goto EXIT;

    fptr = fopen("/home/root/EDC_PUB_CERT", "wb+");
    if(fptr == NULL)
    {
        perror("open EDC_PUB_CERT failed");
        result = apiFailed;
        goto EXIT;
    }

    ret = fwrite(edcPubCert, totalLen, 1, fptr);
    if(!ret)
    {
        perror("fwrite fail");
        fclose(fptr);

        result = apiFailed;
        goto EXIT;
    }
    else
    {
        sync();
        fclose(fptr);
    }
    
EXIT:
    if(mfgPubKey)
        free(mfgPubKey);

    if(edcPubCert)
        free(edcPubCert);

    if(result == apiFailed)
        LIB_LCD_PutMsg(2, COL_MIDWAY, FONT0 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);
    else
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT0 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_OK), (UINT8 *)os_msg_LOAD_KEY_OK);

	LIB_CloseAUX();

	LIB_WaitTimeAndKey(300);

    // clear sensitive data
    memset(buffer, 0x00, sizeof(buffer));
    memset(tid, 0x00, sizeof(tid));
	memset(temp, 0x00, sizeof(temp));
	memset(buf1, 0x00, sizeof(buf1));
	memset(buf2, 0x00, sizeof(buf2));

    return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: ADMIN - Mutual authentication between EDC and SCD using secret
//                   information.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK  - OK
//           apiFailed - ERROR
//                 EDC                                                               SCD
//           ---------------                                                   ---------------
//           CMD(1)+TSN(8)+TID(8)+UID(16)+EDC_PUB_CERT(n)               ->
//                                                                      <-     CMD(1)
//                                                                      ...
//           CMD(1)+TSN(8)+TID(8)+EDC_PUB_CERT(n)                       ->
//                                                                      <-     CMD(1)
//           CMD(1)+TSN(8)+TID(8)                                       ->
//                                                                             Verify(MFG_PUB_KEY, EDC_PUB_CERT)
//                                                                      <-     CMD(1)+AUTH_RESULT(1)

//           CMD(1)+TSN(8)+TID(8)                                       ->
//                                                                      <-     CMD(1)+UID(16)+Sign(MFG_PRV_KEY, UID)(n)
//           CMD(1)+TSN(8)+TID(8)                                       ->
//                                                                      ...
//                                                                      <-     CMD(1)+Sign(MFG_PRV_KEY, UID)(n)
//           CMD(1)+TSN(8)+TID(8)                                       ->
//                                                                      <-     CMD(1)
//           Verify(MFG_PUB_KEY, Sign(MFG_PRV_KEY, UID))
//           Compare EDC's UID with the received UID
//           CMD(1)+TSN(8)+TID(8)+AUTH_RESULT(1)                        ->
//                                                                      <-     CMD(1)
//
//           CMD(1)+TSN(8)+TID(8)+UID(16)+Sign(EDC_PRV_KEY, UID)(n)     ->
//                                                                      <-     CMD(1)
//                                                                      ...
//           CMD(1)+TSN(8)+TID(8)+Sign(EDC_PRV_KEY, UID)(n)             ->
//                                                                      <-     CMD(1)
//           CMD(1)+TSN(8)+TID(8)                                       ->
//                                                                             Verify(EDC_PUB_KEY, Sign(EDC_PRV_KEY, UID))
//                                                                      <-     CMD(1)+AUTH_RESULT(1)
// ---------------------------------------------------------------------------
UINT8	PED_ADMIN_DeviceAuthenticationPhase2(void)
{
    UINT8	buffer[32];
	UINT8	tid[9];
	UINT32	len;
    UINT8	temp[32];
	UINT8	buf1[257];
	UINT8	buf2[257];
	UINT8	nextblk;
	UINT8	result;
	UINT8	index;
    UINT32	iLen;
    UINT32	totalLen = 0;
    UINT32  offset;
    UINT8   maxSize = 238;  //255-1-16
    UINT8   transferCount;
    UINT8   lastSize;
    UINT8   isFirst;
    UINT32  edcPubCertSize;
    UINT8   *edcPubCert;
    UINT8   message[16 + 256];  //MESSAGE = UID || Sign(MFG_PRV_KEY, UID)
    UINT8   message2[16 + 256];  //MESSAGE2 = UID || Sign(EDC_PRV_KEY, UID)
    UINT8   uid[16];
    UINT8   signature[256];
    UINT16  signatureLen;
    UINT8   isAuthenticated = FALSE;
    FILE    *fptr;
    UINT8   authStatus;
    struct  stat stbuf;
    

    LIB_LCD_Cls();
    LIB_LCD_Puts(0, 0, FONT0, sizeof(os_msg_DEV_AUTH_2), (UINT8 *)os_msg_DEV_AUTH_2);

    // Request TID(8)
	LIB_LCD_Puts(1, 0, FONT0, sizeof(os_msg_TERM_ID), (UINT8 *)os_msg_TERM_ID);

	// read current TID
	OS_SECM_GetData(ADDR_PED_TERM_ID, PED_TERM_ID_LEN, &tid[1]);
	tid[0] = PED_TERM_ID_LEN;

	if((tid[1] < '0') || (tid[1] > '9') || (tid[2] < '0') || (tid[2] > '9') ||
	   (tid[3] < '0') || (tid[3] > '9') || (tid[4] < '0') || (tid[4] > '9') ||
	   (tid[5] < '0') || (tid[5] > '9') || (tid[6] < '0') || (tid[6] > '9') ||
	   (tid[7] < '0') || (tid[7] > '9') || (tid[8] < '0') || (tid[8] > '9'))
	{
		// set default TID to TSN if illegal tid values
		api_sys_info(SID_TerminalSerialNumber, buffer);
		memmove(&tid[1], &buffer[4], 8);
	}

	// show TID
	LIB_LCD_Puts(1, 13, FONT0, PED_TERM_ID_LEN, &tid[1]);

	if(LIB_GetNumKey(0, NUM_TYPE_DIGIT + NUM_TYPE_LEADING_ZERO, '_', FONT0, 2, 8, buffer) == FALSE)
		return(apiOK); // aborted

	if(!((buffer[0] == 1) && (buffer[1] == '0')))
	{
		if(buffer[0] != 8)
		{
			memset(&tid[1], 0x30, 8);
			len = PED_TERM_ID_LEN - buffer[0] + 1;
			memmove(&tid[len], &buffer[1], buffer[0]);
		}
		else
			memmove(&tid[1], &buffer[1], buffer[0]);

		// save new TID
		OS_SECM_PutData(ADDR_PED_TERM_ID, PED_TERM_ID_LEN, &tid[1]);
	}

	LIB_LCD_ClearRow(1, 2, FONT0);
	LIB_LCD_PutMsg(2, COL_MIDWAY, FONT0 + attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING);

    // init communication port
	if(LIB_OpenAUX(COM1, auxDLL, COM_9600) == FALSE)
	{
		LIB_LCD_PutMsg(2, COL_MIDWAY, FONT0 + attrCLEARWRITE, sizeof(os_msg_LOAD_KEY_FAILED), (UINT8 *)os_msg_LOAD_KEY_FAILED);
		LIB_WaitTimeAndKey(300);
		return apiFailed; // device error
	}

    //--- Send UID || EDC_PUB_CERT ---
    //EDC_PUB_CERT = EDC_PUB_KEY || Sign(MFG_PRV_KEY, EDC_PUB_KEY)

    //      EDC                                                               SCD
    //---------------                                                   ---------------
    //CMD(1)+TSN(8)+TID(8)+UID(16)+EDC_PUB_CERT(n)               ->
    //                                                           <-     CMD(1)
    //                                                           ...
    //CMD(1)+TSN(8)+TID(8)+EDC_PUB_CERT(n)                       ->
    //                                                           <-     CMD(1)
    //CMD(1)+TSN(8)+TID(8)                                       ->
    //                                                                  Verify(MFG_PUB_KEY, EDC_PUB_CERT)
    //                                                           <-     CMD(1)+AUTH_RESULT(1)

    printf(">>>>>>Send UID || EDC_PUB_CERT===============================================\n");    // ==== [Debug] ====

    nextblk = 0x00;
    result = apiOK;
    offset = 0;
    isFirst = TRUE;

    fptr = fopen("/home/root/EDC_PUB_CERT", "rb");
    if(fptr == NULL)
    {
        printf("open EDC_PUB_CERT failed\n");
        result = apiFailed;
        goto EXIT;
    }

    fseek(fptr, 0, SEEK_END);
    edcPubCertSize = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);

    edcPubCert = malloc(edcPubCertSize);
    if(edcPubCert == NULL)
    {
        perror("unable to allocate required memory");
        result = apiFailed;
        goto EXIT;
    }

    fread(edcPubCert, edcPubCertSize, 1, fptr);

    transferCount = (16 + edcPubCertSize) / maxSize;
    lastSize = (16 + edcPubCertSize) % maxSize;
    if(lastSize > 0)
        transferCount++;
    
    while(result == apiOK)
    {
        // Terminal Serial Number (TSN)
        api_sys_info(SID_TerminalSerialNumber, temp);
        memmove(buf2, &temp[4], 8);

        // Terminal ID (TID)
        memmove(&buf2[8], &tid[1], 8);

        memmove(&buf1[3], buf2, 16);
        // buf1[0] = 17;
        buf1[1] = 0;
        buf1[2] = KMC_DEVICE_AUTH_PHASE_2 | nextblk; // command

        if(isFirst)
        {
            isFirst = FALSE;

            //UID(16)
            memmove(&buf1[19], &temp[1], temp[0]);
            //EDC_PUB_CERT(222)
            memmove(&buf1[19 + temp[0]], edcPubCert, maxSize - temp[0]);
            offset += maxSize - temp[0];
            
            buf1[0] = 17 + maxSize;
        }
        else
        {
            if(transferCount > 1)
            {
                //only EDC_PUB_CERT
                memmove(&buf1[19], &edcPubCert[offset], maxSize);
                offset += maxSize;

                buf1[0] = 17 + maxSize;
            }
            else if(transferCount == 1)
            {
                if(lastSize > 0)
                {
                    //only EDC_PUB_CERT
                    memmove(&buf1[19], &edcPubCert[offset], lastSize);
                    offset += lastSize;

                    buf1[0] = 17 + lastSize;
                }
                else
                {
                    //only EDC_PUB_CERT
                    memmove(&buf1[19], &edcPubCert[offset], maxSize);
                    offset += maxSize;

                    buf1[0] = 17 + maxSize;
                }
            }
            else if(transferCount == 0)
            {
                // send CMD(1) + TSN(8) + TID(8) to SCD
                buf1[0] = 17;
            }
        }

        result = apiFailed;

        // send CMD(1) + TSN(8) + TID(8) + UID(16) + EDC_PUB_CERT(n) to SCD
        if(LIB_TransmitAUX(buf1) == TRUE)
        {
            if(LIB_ReceiveAUX(buf2) == TRUE)
            {
                if(buf2[2] == buf1[2])  // check command code
                {
                    iLen = buf2[0] + buf2[1] * 256;

                    if(transferCount > 0)
                    {
                        //SCD response CMD(1)
                        if(iLen == 1)
                            result = apiOK;
                        
                        transferCount--;
                    }
                    else
                    {
                        //SCD response CMD(1)+AUTH_RESULT(1)
                        if(iLen == 2)
                        {
                            if(buf2[3] == 1)    //AUTH_RESULT
                            {
                                printf("Signature verification of EDC_PUB_KEY is successful\n");
                                result = apiOK;
                            }
                            else
                                printf("Signature verification of EDC_PUB_KEY failed\n");

                            break;
                        }
                    }

                    nextblk = KMC_NEXT;
                }
            }
        }
    }   // while()

    fclose(fptr);
    free(edcPubCert);

    if(result == apiFailed)
        goto EXIT;

    //--- Load MESSAGE ---
    //MESSAGE = UID || Sign(MFG_PRV_KEY, UID)

    //      EDC                                                    SCD
    //---------------                                        ---------------
    //CMD(1)+TSN(8)+TID(8)                            ->
    //                                                <-     CMD(1)+UID(16)+Sign(MFG_PRV_KEY, UID)(n)
    //CMD(1)+TSN(8)+TID(8)                            ->
    //                                                ...
    //                                                <-     CMD(1)+Sign(MFG_PRV_KEY, UID)(n)
    //CMD(1)+TSN(8)+TID(8)                            ->
    //                                                <-     CMD(1)
    //Verify(MFG_PUB_KEY, Sign(MFG_PRV_KEY, UID))
    //Compare EDC's UID with the received UID
    //CMD(1)+TSN(8)+TID(8)+AUTH_RESULT(1)             ->
    //                                                <-     CMD(1)

    printf(">>>>>>Load MESSAGE===============================================\n");    // ==== [Debug] ====

    offset = 0;
    totalLen = 0;

    while(result == apiOK)
    {
        // Terminal Serial Number (TSN)
        api_sys_info(SID_TerminalSerialNumber, temp);
        memmove(buf2, &temp[4], 8);

        // Terminal ID (TID)
        memmove(&buf2[8], &tid[1], 8);

        memmove(&buf1[3], buf2, 16);
        buf1[0] = 17;
        buf1[1] = 0;
        buf1[2] = KMC_DEVICE_AUTH_PHASE_2 | nextblk; // command

        result = apiFailed;

        // send CMD(1) + TSN(8) + TID(8) to SCD for download MESSAGE
        if(LIB_TransmitAUX(buf1) == TRUE)
        {
            if(LIB_ReceiveAUX(buf2) == TRUE)
            {
                if(buf2[2] == buf1[2])  // check command code
                {
                    iLen = buf2[0] + buf2[1] * 256;

                    if(iLen > 1)
                    {
                        memmove(&message[offset], &buf2[3], iLen - 1); // without CMD(1)

                        offset += iLen - 1;
                        totalLen += iLen - 1;
                    }
                    else
                    {
                        if(iLen == 1)   // end of record?
                        {
                            memmove(uid, message, 16);
                            memmove(signature, &message[16], 256);

                            fptr = fopen("/home/root/MFG_PUB_KEY.pem", "rb");
                            if(fptr == NULL)
                            {
                                printf("open MFG_PUB_KEY.pem failed\n");
                                result = apiFailed;
                                goto EXIT;
                            }

                            SecretInfo_Verify(fptr, uid, 16, signature, 256, &isAuthenticated);
                            if(isAuthenticated == 1)
                            {
                                printf("Signature verification of MESSAGE is successful\n");
                                
                                //compare EDC's UID with the received UID
                                api_sys_info(SID_TerminalSerialNumber, temp);
                                if(LIB_memcmp(&temp[1], uid, temp[0]) == 0)
                                {
                                    //send CMD(1) + TSN(8) + TID(8) + AUTH_RESULT(1) to SCD
                                    buf1[19] = 1;
                                    buf1[0] = 18;
                                }
                                else
                                {
                                    //send CMD(1) + TSN(8) + TID(8) + AUTH_RESULT(1) to SCD
                                    buf1[19] = 0;
                                    buf1[0] = 18;
                                }
                            }
                            else
                            {
                                printf("Signature verification of MESSAGE failed\n");

                                //send CMD(1) + TSN(8) + TID(8) + AUTH_RESULT(1) to SCD
                                buf1[19] = 0;
                                buf1[0] = 18;
                            }

                            if(LIB_TransmitAUX(buf1) == TRUE)
                            {
                                if(LIB_ReceiveAUX(buf2) == TRUE)
                                {
                                    if(buf2[2] == buf1[2])  // check command code
                                    {
                                        iLen = buf2[0] + buf2[1] * 256;
                                        if(iLen == 1)
                                        {
                                            if((isAuthenticated == 1) && (LIB_memcmp(&temp[1], uid, temp[0]) == 0))
                                            {
                                                result = apiOK;
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    nextblk = KMC_NEXT;
                    result = apiOK;
                }
            }
        }
    }   // while()

    fclose(fptr);

    if(result == apiFailed)
        goto EXIT;
    
    //--- Send MESSAGE2 ---
    //MESSAGE2 = UID || Sign(EDC_PRV_KEY, UID)

    //      EDC                                                               SCD
    //---------------                                                   ---------------
    //CMD(1)+TSN(8)+TID(8)+UID(16)+Sign(EDC_PRV_KEY, UID)(n)     ->
    //                                                           <-     CMD(1)
    //                                                           ...
    //CMD(1)+TSN(8)+TID(8)+Sign(EDC_PRV_KEY, UID)(n)             ->
    //                                                           <-     CMD(1)
    //CMD(1)+TSN(8)+TID(8)                                       ->
    //                                                                  Verify(EDC_PUB_KEY, Sign(EDC_PRV_KEY, UID))
    //                                                           <-     CMD(1)+AUTH_RESULT(1)

    printf(">>>>>>Send MESSAGE2===============================================\n");    // ==== [Debug] ====

    api_sys_info(SID_TerminalSerialNumber, temp);
    memmove(uid, &temp[1], temp[0]);

    result = SecretInfo_GetEdcPrvKey();
    if(result == apiFailed)
    {
        printf("read EDC private key from the secure file failed\n");
        goto EXIT;
    }

    fptr = fopen("/home/root/EDC_PRV_KEY.pem", "rb");
    if(fptr == NULL)
    {
        printf("open EDC_PRV_KEY.pem failed\n");
        result = apiFailed;
        goto EXIT;
    }

    SecretInfo_Sign(fptr, uid, 16, signature, &signatureLen);

    memmove(message2, uid, 16);
    memmove(&message2[16], signature, signatureLen);

    transferCount = sizeof(message2) / maxSize;
    lastSize = sizeof(message2) % maxSize;
    if(lastSize > 0)
        transferCount++;

    offset = 0;

    while(result == apiOK)
    {
        // Terminal Serial Number (TSN)
        api_sys_info(SID_TerminalSerialNumber, temp);
        memmove(buf2, &temp[4], 8);

        // Terminal ID (TID)
        memmove(&buf2[8], &tid[1], 8);

        memmove(&buf1[3], buf2, 16);
        // buf1[0] = 17;
        buf1[1] = 0;
        buf1[2] = KMC_DEVICE_AUTH_PHASE_2 | nextblk; // command

        if(transferCount > 1)
        {
            memmove(&buf1[19], &message2[offset], maxSize);
            offset += maxSize;

            buf1[0] = 17 + maxSize;
        }
        else if(transferCount == 1)
        {
            if(lastSize > 0)
            {
                memmove(&buf1[19], &message2[offset], lastSize);
                offset += lastSize;

                buf1[0] = 17 + lastSize;
            }
            else
            {
                memmove(&buf1[19], &message[offset], maxSize);
                offset += maxSize;

                buf1[0] = 17 + maxSize;
            }
        }
        else if(transferCount == 0)
        {
            // send CMD(1) + TSN(8) + TID(8) to SCD
            buf1[0] = 17;
        }

        result = apiFailed;

        // send CMD(1) + TSN(8) + TID(8) + MESSAGE2(n) to SCD
        if(LIB_TransmitAUX(buf1) == TRUE)
        {
            if(LIB_ReceiveAUX(buf2) == TRUE)
            {
                if(buf2[2] == buf1[2])  // check command code
                {
                    iLen = buf2[0] + buf2[1] * 256;

                    if(transferCount > 0)
                    {
                        //SCD response CMD(1)
                        if(iLen == 1)
                            result = apiOK;
                        
                        transferCount--;
                    }
                    else
                    {
                        //SCD response CMD(1)+AUTH_RESULT(1)
                        if(iLen == 2)
                        {
                            if(buf2[3] == 1)    //AUTH_RESULT
                            {
                                printf("Signature verification of MESSAGE2 is successful\n");
                                result = apiOK;
                            }
                            else
                                printf("Signature verification of MESSAGE 2 failed\n");
                        }
                        
                        break;
                    }

                    nextblk = KMC_NEXT;
                }
            }
        }
    }   // while()

    fclose(fptr);

EXIT:
    if(result == apiFailed)
    {
        authStatus = 0;
        
        LIB_LCD_PutMsg(2, COL_MIDWAY, FONT0 + attrCLEARWRITE, sizeof(os_msg_VERIFY_FAILED), (UINT8 *)os_msg_VERIFY_FAILED);
    }
    else
    {
        authStatus = 1;

        LIB_LCD_PutMsg(2, COL_MIDWAY, FONT0 + attrCLEARWRITE, sizeof(os_msg_VERIFY_OK), (UINT8 *)os_msg_VERIFY_OK);
    }

	LIB_CloseAUX();

	LIB_WaitTimeAndKey(300);

    SecretInfo_writeAuditTrail(authStatus);

    //if EDC_PRV_KEY.pem exist
    if(stat("/home/root/EDC_PRV_KEY.pem", &stbuf) == 0)
        SecretInfo_DeleteEdcPrvKey();

    // clear sensitive data
    memset(buffer, 0x00, sizeof(buffer));
    memset(tid, 0x00, sizeof(tid));
	memset(temp, 0x00, sizeof(temp));
	memset(buf1, 0x00, sizeof(buf1));
	memset(buf2, 0x00, sizeof(buf2));
    memset(message, 0x00, sizeof(message));
    memset(message2, 0x00, sizeof(message2));
    memset(uid, 0x00, sizeof(uid));
    memset(signature, 0x00, sizeof(signature));

    return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: Request and verify new password.     
// INPUT   : len - max. length of psw string. (6..12 digits)
// OUTPUT  : psw - the password to be verified. (L-V)
// RETURN  : TRUE
//           FALSE
//	         -1	(aborted)
// ---------------------------------------------------------------------------
UINT8	PED_RequestNewPassword(UINT8 len, UINT8 *psw)
{
    UINT    result = TRUE;
	UINT8	i;
	UINT8	buf1[PED_PSW_SLOT_LEN];
	UINT8	buf2[PED_PSW_SLOT_LEN];


	//    LIB_LCD_Cls();
	LIB_LCD_ClearRow(1, 7, FONT0);
	LIB_LCD_PutMsg(1, COL_LEFTMOST, FONT0 + attrCLEARWRITE, sizeof(os_msg_NEW_PASSWORD), (UINT8 *)os_msg_NEW_PASSWORD);

	if(LIB_GetNumKey(30, NUM_TYPE_STAR + NUM_TYPE_LEADING_ZERO + NUM_TYPE_LEFT_JUSTIFY, '_', FONT0, 2, len, buf1) == TRUE)
	{
		if(buf1[0] < MIN_PED_PSW_LEN)	// 2014-10-15
        {
            result = FALSE;
            goto EXIT;
        }

		//      if( buf1[0] != 0 )
		//        {
		//        if( buf1[0] != len )
		//          return( FALSE );
		//        }
	}
	else
    {
        result = -1;
        goto EXIT;
    }


	LIB_LCD_PutMsg(1, COL_LEFTMOST, FONT0 + attrCLEARWRITE, sizeof(os_msg_CONFIRM_PASSWORD), (UINT8 *)os_msg_CONFIRM_PASSWORD);

	if(LIB_GetNumKey(30, NUM_TYPE_LEADING_ZERO + NUM_TYPE_STAR + NUM_TYPE_LEFT_JUSTIFY, '_', FONT0, 2, len, buf2) == TRUE)
	{
		if(buf2[0] < MIN_PED_PSW_LEN)	// 2014-10-15
        {
            result = FALSE;
            goto EXIT;
        }

		//      if( buf2[0] != 0 )
		//        {
		//        if( buf2[0] != len )
		//          return( FALSE );
		//        }
	}

	for(i = 0; i < buf1[0] + 1; i++)
	{
		if(buf1[i] != buf2[i])
        {
            result = FALSE;
            goto EXIT;
        }
	}

	memmove(psw, buf1, buf1[0] + 1);

EXIT:
	// PATCH: 2009-04-07, clear sensitive data
	memset(buf1, 0x00, sizeof(buf1));
	memset(buf2, 0x00, sizeof(buf2));

	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: Change ADMIN password.     
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
#if	0
UINT8	PED_ADMIN_ChangePassword( void )
{
UINT8	status;
UINT8	buf[PED_PSW_SLOT_LEN];


	LIB_LCD_PutMsg( 0, COL_RIGHTMOST, FONT0+attrCLEARWRITE, sizeof(os_msg_ADMIN2), (UINT8 *)os_msg_ADMIN2 );
	
	status = PED_RequestNewPassword( PED_ADMIN_PSW_LEN, buf );
	if( status == 255 )
	  {
	  // PATCH: 2009-04-07, clear sensitive data
	  memset( buf, 0x00, sizeof(buf) );
	  
	  return( apiOK ); // abort
	  }
	  
	if( status == TRUE )
	  {
	  OS_SECM_PutData( ADDR_PED_ADMIN_PSW, buf[0]+1, buf );
	  
	  LIB_LCD_PutMsg( 3, COL_RIGHTMOST, FONT1+attrCLEARWRITE, sizeof(os_msg_OK), (UINT8 *)os_msg_OK );  
	  LIB_WaitTimeAndKey( 100 );

	  // PATCH: 2009-04-07, clear sensitive data
	  memset( buf, 0x00, sizeof(buf) );

	  return( apiOK );
	  }
	else
	  {
	  if( status == FALSE )
	    {
	    LIB_LCD_PutMsg( 3, COL_RIGHTMOST, FONT1+attrCLEARWRITE, sizeof(os_msg_ERROR), (UINT8 *)os_msg_ERROR );  
	    LIB_WaitTimeAndKey( 100 );
	    }

	  // PATCH: 2009-04-07, clear sensitive data
	  memset( buf, 0x00, sizeof(buf) );

	  return( apiFailed );
	  }
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Change ADMIN password.     
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
#if	0
UINT8	PED_USER_ChangePassword( void )
{
UINT8	status;
UINT8	buf[PED_PSW_SLOT_LEN];

	
	LIB_LCD_PutMsg( 0, COL_RIGHTMOST, FONT0+attrCLEARWRITE, sizeof(os_msg_USER2), (UINT8 *)os_msg_USER2 );

	status = PED_RequestNewPassword( PED_USER_PSW_LEN, buf );
	if( status == 255 )
	  {
	  // PATCH: 2009-04-07, clear sensitive data
	  memset( buf, 0x00, sizeof(buf) );
	  
	  return( apiOK ); // abort
	  }
	  
	if( status == TRUE )
	  {
	  OS_SECM_PutData( ADDR_PED_USER_PSW, buf[0]+1, buf );
	  
	  LIB_LCD_PutMsg( 3, COL_RIGHTMOST, FONT1+attrCLEARWRITE, sizeof(os_msg_OK), (UINT8 *)os_msg_OK );  
	  LIB_WaitTimeAndKey( 100 );

	  // PATCH: 2009-04-07, clear sensitive data
	  memset( buf, 0x00, sizeof(buf) );
  
	  return( apiOK );
	  }
	else
	  {
	  if( status == FALSE )
	    {
	    LIB_LCD_PutMsg( 3, COL_RIGHTMOST, FONT1+attrCLEARWRITE, sizeof(os_msg_ERROR), (UINT8 *)os_msg_ERROR );  
	    LIB_WaitTimeAndKey( 100 );
	    }

	  // PATCH: 2009-04-07, clear sensitive data
	  memset( buf, 0x00, sizeof(buf) );
	  	    
	  return( apiFailed );
	  }
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: to verify all existent DUKPT.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE	
//           FALSE
// ---------------------------------------------------------------------------
UINT32	PED_VerifyKeyStatus_DUKPT( void )
{
UINT32	i, j;
UINT8	lrc;
UINT8	buf[MAX_FUTURE_KEY_REG_CNT*MAX_PAYMENT_SCHEME_CNT];
UINT32	fDUKPT;


	// ================== DUKPT Key ==================
	fDUKPT = FALSE;
	for( i=0; i<MAX_FUTURE_KEY_REG_CNT; i++ )
	   {
	   OS_SECM_GetData( ADDR_FUTURE_KEY_REG+(i*FUTURE_KEY_SLOT_LEN), FUTURE_KEY_SLOT_LEN, buf );
	   	   
	   //  verify DUKPT key by checking LRC
	   if( LIB_memcmpc( buf, 0x00, FUTURE_KEY_SLOT_LEN ) != 0 )
	     {
	     lrc = 0;
	     for( j=0; j<FUTURE_KEY_LEN; j++ )
	        lrc ^= buf[j];
	     if( lrc == buf[j] )
	       fDUKPT = TRUE;
	     }	     
	   }
	   
    // clear sensitive data
	memset(buf, 0x00, sizeof(buf));

	return( fDUKPT );
}

// ---------------------------------------------------------------------------
// FUNCTION: to verify all existent AES DUKPT.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE	
//           FALSE
// ---------------------------------------------------------------------------
UINT32	PED_VerifyKeyStatus_AES_DUKPT( void )
{
    UINT32  i;
    UINT8   buf[MAX_KEYLENGTH];
    UINT8   keyType;
    UINT8   fDUKPT;
    

    // ================== DUKPT Key ==================
    fDUKPT = FALSE;

    for(i = 0 ; i < MAX_NUM_REG ; i++)
    {
        // OS_SECM_GetData(ADDR_INT_DERIVATION_KEY_REG + (i * MAX_KEYLENGTH), MAX_KEYLENGTH, buf);
        OS_SECM_GetIntDerivationKeyReg(i * MAX_KEYLENGTH, MAX_KEYLENGTH, buf);
        
        PED_AES_DUKPT_GetKeyType(&keyType);
        if(keyType == AES_128)
        {
            //  verify DUKPT key
            if(LIB_memcmpc(buf, 0x00, 16) != 0)
                fDUKPT = TRUE;
        }
        else if(keyType == AES_192)
        {
            //  verify DUKPT key
            if(LIB_memcmpc(buf, 0x00, 24) != 0)
                fDUKPT = TRUE;
        }
        else if(keyType == AES_256)
        {
            //  verify DUKPT key
            if(LIB_memcmpc(buf, 0x00, 32) != 0)
                fDUKPT = TRUE;
        }
    }

    // clear sensitive data
	memset(buf, 0x00, sizeof(buf));

    return fDUKPT;
}

// ---------------------------------------------------------------------------
// FUNCTION: get status of PED keys.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
//
//           PED KEY STATUS
//           DUKPT
//           -
//           MASTER/SESSION
//           0123456789ABCDEF
//           ----------------
//	     ----------------
//           FIXED
//           -                 [OK]
// ---------------------------------------------------------------------------
#if	0
UINT8	PED_USER_VerifyKeyStatus( void )
{
UINT32	i, j;
UINT8	lrc;
UINT8	str_dukpt[] = {"DUKPT"};
UINT8	str_ms[]    = {"MASTER/SESSION"};
UINT8	str_fixed[] = {"FIXED"};
UINT8	str_na[]    = {"<NA>"};
UINT8	str_rid[]   = {"RID:"};
UINT8	str_capk[]  = {"CAPK"};
UINT8	str_aes[]   = {"ISO4_AES"};
UINT8	str_slot10[]= {"0123456789"};
UINT8	str_slot21[]= {"012345678901234567890"};
UINT8	buf[MAX_FUTURE_KEY_REG_CNT*MAX_PAYMENT_SCHEME_CNT];
UINT8	temp[MAX_FUTURE_KEY_REG_CNT];

PED_KEY_BUNDLE	KeyBundle;
UINT8	keydata[KEY_DATA_LEN+1]; // L-V
UINT8	mac8[8];
UINT8	keypress;


	LIB_LCD_Cls();
	LIB_LCD_PutMsg( 0, COL_LEFTMOST, FONT0, sizeof(os_msg_PED_KEY_STATUS), (UINT8 *)os_msg_PED_KEY_STATUS );
	
	// ================== Fixed Key ==================
	LIB_LCD_Puts( 1, 0, FONT0+attrREVERSE, sizeof(str_fixed)-1, str_fixed );	
	LIB_LCD_Puts( 2, 0, FONT0, sizeof(str_slot10)-1, str_slot10 );
	memset( temp, '-', sizeof(str_slot10)-1 );
	
	for( i=0; i<MAX_FKEY_CNT; i++ )
	   {
	   // get FIXED key bundle
	   OS_SECM_GetData( ADDR_PED_FKEY_01+(i*PED_FKEY_SLOT_LEN), PED_FKEY_SLOT_LEN, (UINT8 *)&KeyBundle );
	   	   
	   // verify FIXED key bundle	   
	   if( TR31_VerifyKeyBundle( PED_FKEY_SLOT_LEN, (UINT8 *)&KeyBundle, keydata, mac8 ) )
	     temp[i] = '*';
	   }
	   
	LIB_LCD_Puts( 3, 0, FONT0, sizeof(str_slot10)-1, temp );	// show key status

//	if( LIB_WaitKey() == 'x' )
	keypress = LIB_WaitTimeAndKey(3000);		// 2014-11-04
	if( (keypress == 'x') || (keypress == 255) )	//
	  {
	  // PATCH: 2009-04-07, clear sensitive data
	  memset( (UINT8 *)&KeyBundle, 0x00, sizeof(KeyBundle) );
	  memset( (UINT8 *)&keydata, 0x00, sizeof(keydata) );
	  
	  return( apiOK );
	  }
	  
	LIB_LCD_ClearRow( 1, 7, FONT0 );
	
	
	// ================== Master/Session Key ==================
	LIB_LCD_Puts( 1, 0, FONT0+attrREVERSE, sizeof(str_ms)-1, str_ms );
	
	LIB_LCD_Puts( 2, 0, FONT0, sizeof(str_slot10)-1, str_slot10 );
	memset( temp, '-', sizeof(str_slot10)-1 );
	
	for( i=0; i<MAX_MKEY_CNT; i++ )
	   {
	   // get MASTER key bundle
	   OS_SECM_GetData( ADDR_PED_MKEY_01+(i*PED_MSKEY_SLOT_LEN), PED_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle );
	   
	   // verify MASTER key bundle
	    if( TR31_VerifyKeyBundle( PED_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle, keydata, mac8 ) )
	      temp[i] = '*';
	   }   
	LIB_LCD_Puts( 3, 0, FONT0, sizeof(str_slot10)-1, temp );	// show key status
	
	memset( temp, '-', sizeof(str_slot10)-1 );
	
	for( i=0; i<MAX_SKEY_CNT; i++ )
	   {
	   // get SESSION key bundle
	   OS_SECM_GetData( ADDR_PED_SKEY_01+(i*PED_MSKEY_SLOT_LEN), PED_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle );
	   
	   // verify SESSION key bundle
	    if( TR31_VerifyKeyBundle( PED_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle, keydata, mac8 ) )
	      temp[i] = '*';
	   }
	LIB_LCD_Puts( 4, 0, FONT0, sizeof(str_slot10)-1, temp );	// show key status
	
//	if( LIB_WaitKey() == 'x' )
	keypress = LIB_WaitTimeAndKey(3000);		// 2014-11-04
	if( (keypress == 'x') || (keypress == 255) )	//
	  {
	  // PATCH: 2009-04-07, clear sensitive data
	  memset( (UINT8 *)&KeyBundle, 0x00, sizeof(KeyBundle) );
	  memset( (UINT8 *)&keydata, 0x00, sizeof(keydata) );

	  return( apiOK );
	  }
	  
	LIB_LCD_ClearRow( 1, 7, FONT0 );
	
	
	// ================== DUKPT Key ==================
	LIB_LCD_Puts( 1, 0, FONT0+attrREVERSE, sizeof(str_dukpt)-1, str_dukpt );
	LIB_LCD_Puts( 2, 0, FONT0, sizeof(str_slot21)-1, str_slot21 );
	memset( temp, '-', sizeof(temp) );
	
	for( i=0; i<MAX_FUTURE_KEY_REG_CNT; i++ )
	   {
	   OS_SECM_GetData( ADDR_FUTURE_KEY_REG+(i*FUTURE_KEY_SLOT_LEN), FUTURE_KEY_SLOT_LEN, buf );
	   	   
	   //  verify DUKPT key by checking LRC
	   if( LIB_memcmpc( buf, 0x00, FUTURE_KEY_SLOT_LEN ) != 0 )
	     {
	     lrc = 0;
	     for( j=0; j<FUTURE_KEY_LEN; j++ )
	        lrc ^= buf[j];
	     if( lrc == buf[j] )
	       temp[i] = '*';
	     }	     
	   }
	   
	memset( buf, 0x00, FUTURE_KEY_SLOT_LEN );	// clear FUTURE key buffer  
	LIB_LCD_Puts( 3, 0, FONT0, sizeof(temp), temp );	// show key status
	
//	if( LIB_WaitKey() == 'x' )
	keypress = LIB_WaitTimeAndKey(3000);		// 2014-11-04
	if( (keypress == 'x') || (keypress == 255) )	//
	  {
	  // PATCH: 2009-04-07, clear sensitive data
	  memset( (UINT8 *)&KeyBundle, 0x00, sizeof(KeyBundle) );
	  memset( (UINT8 *)&keydata, 0x00, sizeof(keydata) );
	  memset( buf, 0x00, sizeof(buf) );
	  
	  return( apiOK );
	  }
	  
	LIB_LCD_ClearRow( 1, 7, FONT0 );


	// ================== ISO4 AES Key ==================
	LIB_LCD_Puts( 1, 0, FONT0+attrREVERSE, sizeof(str_aes)-1, str_aes );
	
	OS_SECM_GetData( ADDR_PED_ISO4_AES_KEY, PED_ISO4_AES_KEY_SLOT_LEN, buf );
	if( PED_VerifyISO4KEY( buf ) )
	  LIB_LCD_Putc( 3, 0, FONT0, '*' );
	else
	  LIB_LCD_Putc( 3, 0, FONT0, '-' );
	  
	keypress = LIB_WaitTimeAndKey(3000);		// 2014-11-04
	if( (keypress == 'x') || (keypress == 255) )	//
	  {
	  // clear sensitive data
	  memset( buf, 0x00, sizeof(buf) );
	  return( apiOK );
	  }
	  
	LIB_LCD_ClearRow( 1, 7, FONT0 );
	
	
	// ================== CAPK ==================
	LIB_LCD_PutMsg( 1, COL_LEFTMOST, FONT0+attrREVERSE, sizeof(str_capk)-1, str_capk );
	
	PED_CAPK_GetInfo( buf );
	
	if( buf[0] == 0xFF )
	  {
	  LIB_LCD_PutMsg( 2, COL_LEFTMOST, FONT0, sizeof(str_na)-1, str_na );
//	  LIB_WaitKey();
	  LIB_WaitTimeAndKey(3000);	// 2014-11-04
	  }
	else
	  {
	  LIB_LCD_PutMsg( 2, COL_LEFTMOST, FONT0, sizeof(str_rid)-1, str_rid );
	  for( i=0; i<MAX_PAYMENT_SCHEME_CNT; i++ )
	     {
	     if( buf[i*21] != 0xFF )
	       {
	       for( j=0; j<RID_LEN; j++ )
	          LIB_DispHexByte( 2, 2*j+4, buf[i*21+j] );
	          
//	       LIB_DumpHexData( 0, 4, buf[i*21+5], &buf[i*21+6] );
	       LIB_DumpHexDataTO( 0, 4, buf[i*21+5], &buf[i*21+6] );	// 2014-11-04
	       }
	     }
	  }

	// PATCH: 2009-04-07, clear sensitive data
	memset( (UINT8 *)&KeyBundle, 0x00, sizeof(KeyBundle) );
	memset( (UINT8 *)&keydata, 0x00, sizeof(keydata) );
	memset( buf, 0x00, sizeof(buf) );

	return( apiOK );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Change key operation mode.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
//
//           CHANGE KEY MODE:
//           > MASTER/SESSION
//             DUKPT
//		       ISO4
//             
// ---------------------------------------------------------------------------
UINT8	PED_ADMIN_ChangeKeyMode(void)
{
	UINT8	buffer[16];
	UINT8	result;
	UINT8	start;


	start = PED_ReadKeyMode();
	if(start > MAX_PED_KEY_MODE)
		start = 0;

	LIB_LCD_Cls();
	LIB_LCD_Puts(0, 0, FONT1, sizeof(os_msg_SELECT), (UINT8 *)os_msg_SELECT);

	buffer[0] = 1;	// starting row number
	buffer[1] = 5;	// max lcd row cnt
	buffer[2] = 4;	// list items
	buffer[3] = 18;	// item length
	buffer[4] = 0;	// offset of LEN field in item
	buffer[5] = FONT1;
	result = LIB_ListBox(start, &buffer[0], (UINT8 *)&os_list_KEY_MODE[0], 30); // wait for selection (T.O.=30sec)

	if(result != 0xff)
	{
		PED_WriteKeyMode(result);

		LIB_LCD_PutMsg(6, COL_RIGHTMOST, FONT1 + attrCLEARWRITE, sizeof(os_msg_OK), (UINT8 *)os_msg_OK);
		LIB_WaitTimeAndKey(100);
		return apiOK;
	}

    // clear sensitive data
	memset(buffer, 0x00, sizeof(buffer));

	return apiFailed;
}

// ---------------------------------------------------------------------------
// FUNCTION: Change key operation mode.
// INPUT   : modify - TRUE : allow to select master key index.
//                    FALSE: not allow to select master key index.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
//
//           CHANGE KEY MODE:
//           > FIXED
//             MASTER/SESSION
//		     > SELECT MASTER KEY (0..9)
//             DUKPT
// ---------------------------------------------------------------------------
#if	0
UINT8	PED_USER_ChangeKeyMode( UINT8 modify )
{
UINT8	buffer[16];
UINT8	result;
UINT8	start;	
	
	
	start = PED_ReadKeyMode();
	if( start > MAX_PED_KEY_MODE )
	  start = 0;
	
	LIB_LCD_Cls();
        LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_SELECT), (UINT8 *)os_msg_SELECT );
	
	buffer[0] = 1;	// starting row number
        buffer[1] = 5+1;	// max lcd row cnt
        buffer[2] = 4+1;	// list items
        buffer[3] = 18;	// item length
        buffer[4] = 0;	// offset of LEN field in item
        buffer[5] = FONT0;
	result = LIB_ListBox( start, &buffer[0], (UINT8 *)&os_list_KEY_MODE[0], 30 ); // wait for selection (T.O.=30sec)
	
	if( result != 0xff )
	  {
	  PED_WriteKeyMode( result );
	  
	  LIB_LCD_PutMsg( 3, COL_RIGHTMOST, FONT1+attrCLEARWRITE, sizeof(os_msg_OK), (UINT8 *)os_msg_OK );  
	  LIB_WaitTimeAndKey( 100 );
	  }
	
	return( apiOK );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Show terminal serial number.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
//
//	     TERMINAL S/N
//	     nnnnnnn
// ---------------------------------------------------------------------------
#if	0
UINT8	PED_USER_ShowTSN( void )
{
UINT8	temp[32];


	LIB_LCD_Cls();
	LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_TERM_SN), (UINT8 *)os_msg_TERM_SN );

	if( api_sys_info( SID_TerminalSerialNumber, temp ) == apiOK )
	  {
//	  LIB_LCD_Puts( 2, 0, FONT0, temp[0], &temp[1] );
	  LIB_LCD_Puts( 2, 0, FONT0, 8, &temp[4] );
//	  LIB_WaitKey();
	  LIB_WaitTimeAndKey( 3000 );	// 2014-10-30, wait max 30 seconds
	  }
	  
	return( apiOK );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Dual password control.
// INPUT   : mode -	0 = DSS
//			1 = KMS (ADMIN)
//			2 = SRED
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
UINT32	PED_DualPasswordControl( UINT32 mode )
{
//UINT32	i;
UINT32	first = FALSE;
UINT8	result;
UINT8	psw1[PED_PSW_SLOT_LEN];
UINT8	psw2[PED_PSW_SLOT_LEN];
UINT8	buf[PED_PSW_SLOT_LEN];
UINT8	pkey1[PED_PSW_SLOT_LEN];
UINT8	pkey2[PED_PSW_SLOT_LEN];
FILE    *fp;
// char    *command_rmsecure_region = "rm /home/root/secure_region";
char    *command_rmsecure_directory = "rm -r /home/root/secure_directory";


	if((mode != 0) && (mode != 1) && (mode != 2))
	  return( FALSE );

	// get two passwords in secure memory
	if( mode == 0 )
	  {
	  OS_SECM_GetData( ADDR_PED_DSS_PSW1, PED_PSW_SLOT_LEN, psw1 );
	  OS_SECM_GetData( ADDR_PED_DSS_PSW2, PED_PSW_SLOT_LEN, psw2 );
	  }
	else if(mode == 1)
	  {
	  OS_SECM_GetData( ADDR_PED_ADMIN_PSW1, PED_PSW_SLOT_LEN, psw1 );
	  OS_SECM_GetData( ADDR_PED_ADMIN_PSW2, PED_PSW_SLOT_LEN, psw2 );
	  }
	else
	{
		OS_SECM_GetData(ADDR_PED_SRED_PSW1, PED_PSW_SLOT_LEN, psw1);
		OS_SECM_GetData(ADDR_PED_SRED_PSW2, PED_PSW_SLOT_LEN, psw2);
	}
	
	if( (psw1[0] == 0) || (psw1[0] > MAX_PED_PSW_LEN) || (psw2[0] == 0) || (psw2[0] > MAX_PED_PSW_LEN) )
	  {
	  // first time use, setup default passwords "1111111" & "2222222"
	  first = TRUE;
	  
	  if( mode == 0 )	// 2014-10-15
	    {
	    psw1[0] = MIN_PED_PSW_LEN;
	    memset( &psw1[1], '1', MIN_PED_PSW_LEN );
	    OS_SECM_PutData( ADDR_PED_DSS_PSW1, MIN_PED_PSW_LEN+1, psw1 );
	    
	    psw2[0] = MIN_PED_PSW_LEN;
	    memset( &psw2[1], '2', MIN_PED_PSW_LEN );
	    OS_SECM_PutData( ADDR_PED_DSS_PSW2, MIN_PED_PSW_LEN+1, psw2 );
	    }
	  else if(mode == 1)
	    {
	    psw1[0] = MIN_PED_PSW_LEN;
	    memset( &psw1[1], '1', MIN_PED_PSW_LEN );
	    OS_SECM_PutData( ADDR_PED_ADMIN_PSW1, MIN_PED_PSW_LEN+1, psw1 );
	    
	    psw2[0] = MIN_PED_PSW_LEN;
	    memset( &psw2[1], '2', MIN_PED_PSW_LEN );
	    OS_SECM_PutData( ADDR_PED_ADMIN_PSW2, MIN_PED_PSW_LEN+1, psw2 );
	    }
	  else
	  {
		  psw1[0] = MIN_PED_PSW_LEN;
		  memset(&psw1[1], '1', MIN_PED_PSW_LEN);
		  OS_SECM_PutData(ADDR_PED_SRED_PSW1, MIN_PED_PSW_LEN + 1, psw1);
	    
		  psw2[0] = MIN_PED_PSW_LEN;
		  memset(&psw2[1], '2', MIN_PED_PSW_LEN);
		  OS_SECM_PutData(ADDR_PED_SRED_PSW2, MIN_PED_PSW_LEN + 1, psw2);
	  }
	  }

	// request the 1'st password
	result = LIB_EnterPassWordLen2( MAX_PED_PSW_CNT, 1, psw1, 0 );
	if( result == TRUE )
	  {
	  if( first )	// request to change it on first use
	    {
	    result = PED_RequestNewPassword( MAX_PED_PSW_LEN, buf );
	    if( (result == 255) || (result == FALSE) )
	      result = FALSE;
	    else
	      {
	      if( LIB_memcmp( psw1, buf, buf[0]+1 ) != 0 )	// different from the old one?
	        {
	        // 2014-10-30, enforce different password values
	        memmove( pkey1, buf, buf[0]+1 );
	        
	        if( mode == 0 )		// DSS1 != DSS2, ADMIN1, ADMIN2
	          {
	          // DSS1 != DSS2 ?
	          OS_SECM_GetData( ADDR_PED_DSS_PSW2, PED_PSW_SLOT_LEN, pkey2 );
	          if( LIB_memcmp( pkey1, pkey2, pkey1[0]+1 ) == 0 )
	            {
	            result = FALSE;
	            goto EXIT;
	            }

	          // DSS1 != ADMIN1 ?
	          OS_SECM_GetData( ADDR_PED_ADMIN_PSW1, PED_PSW_SLOT_LEN, pkey2 );
	          if( LIB_memcmp( pkey1, pkey2, pkey1[0]+1 ) == 0 )
	            {
	            result = FALSE;
	            goto EXIT;
	            }

	          // DSS1 != ADMIN2 ?
	          OS_SECM_GetData( ADDR_PED_ADMIN_PSW2, PED_PSW_SLOT_LEN, pkey2 );	          
	          if( LIB_memcmp( pkey1, pkey2, pkey1[0]+1 ) == 0 )
	            {
	            result = FALSE;
	            goto EXIT;
	            }
	          }
			else if(mode == 1)	// AMDIN1 != DSS1, DSS2, ADMIN2
	          {
	          // ADMIN1 != DSS1 ?
	          OS_SECM_GetData( ADDR_PED_DSS_PSW1, PED_PSW_SLOT_LEN, pkey2 );	          
	          if( LIB_memcmp( pkey1, pkey2, pkey1[0]+1 ) == 0 )
	            {
	            result = FALSE;
	            goto EXIT;
	            }

	          // ADMIN1 != DSS2 ?
	          OS_SECM_GetData( ADDR_PED_DSS_PSW2, PED_PSW_SLOT_LEN, pkey2 );
	          if( LIB_memcmp( pkey1, pkey2, pkey1[0]+1 ) == 0 )
	            {
	            result = FALSE;
	            goto EXIT;
	            }

	          // ADMIN1 != ADMIN2 ?
	          OS_SECM_GetData( ADDR_PED_ADMIN_PSW2, PED_PSW_SLOT_LEN, pkey2 );
	          if( LIB_memcmp( pkey1, pkey2, pkey1[0]+1 ) == 0 )
	            {
	            result = FALSE;
	            goto EXIT;
	            }
	          }
			else  //SRED1 != DSS1, DSS2, ADMIN1, ADMIN2, SRED2
			{
				//SRED1 != DSS1 ?
				OS_SECM_GetData(ADDR_PED_DSS_PSW1, PED_PSW_SLOT_LEN, pkey2);
				if(LIB_memcmp(pkey1, pkey2, pkey1[0] + 1) == 0)
				{
					result = FALSE;
					goto EXIT;
				}

				//SRED1 != DSS2 ?
				OS_SECM_GetData(ADDR_PED_DSS_PSW2, PED_PSW_SLOT_LEN, pkey2);
				if(LIB_memcmp(pkey1, pkey2, pkey1[0] + 1) == 0)
				{
					result = FALSE;
					goto EXIT;
				}

				//SRED1 != ADMIN1 ?
				OS_SECM_GetData(ADDR_PED_ADMIN_PSW1, PED_PSW_SLOT_LEN, pkey2);
				if(LIB_memcmp(pkey1, pkey2, pkey1[0] + 1) == 0)
				{
					result = FALSE;
					goto EXIT;
				}

				//SRED1 != ADMIN2 ?
				OS_SECM_GetData(ADDR_PED_ADMIN_PSW2, PED_PSW_SLOT_LEN, pkey2);
				if(LIB_memcmp(pkey1, pkey2, pkey1[0] + 1) == 0)
				{
					result = FALSE;
					goto EXIT;
				}

				//SRED1 != SRED2 ?
				OS_SECM_GetData(ADDR_PED_SRED_PSW2, PED_PSW_SLOT_LEN, pkey2);
				if(LIB_memcmp(pkey1, pkey2, pkey1[0] + 1) == 0)
				{
					result = FALSE;
					goto EXIT;
				}
			}

			if(mode == 0)
				OS_SECM_PutData(ADDR_PED_DSS_PSW1, buf[0] + 1, buf);		// save new psw1 for DSS
			else if(mode == 1)
				OS_SECM_PutData(ADDR_PED_ADMIN_PSW1, buf[0] + 1, buf);	// save new psw1 for KMS
			else
				OS_SECM_PutData(ADDR_PED_SRED_PSW1, buf[0] + 1, buf);	//save new psw1 for SRED
	        }
	      else
	        result = FALSE;
	      }
	    }
	  }
	else
    {
        //Added by Tammy, enter wrong password MAX_PED_PSW_CNT times
        if(result == FALSE)
        {
            // fp = popen(command_rmsecure_region, "r");
            fp = popen(command_rmsecure_directory, "r");
            if(!fp)
                perror("Failed to remove secure_directory");
            else
            {
                //wait 200 ms to complete the deletion of secure_region
                LIB_WaitTime(20);
                // init_secure_memory();
                init_secure_file();
            }
            
            pclose(fp);
        }

        result = FALSE;
    }

	if( result != TRUE )
        goto EXIT;
	  
	// change psw1?
//	result = PED_ChangePassword( mode, 0 );
//	if( result == FALSE )
//	  {
//	  first = FALSE;
//	  goto EXIT;
//	  }

	// request the 2'nd password
	result = LIB_EnterPassWordLen2( MAX_PED_PSW_CNT, 1, psw2, 1 );
	if( result == TRUE )
	  {
	  if( first )	// request to change it on first use
	    {
	    result = PED_RequestNewPassword( MAX_PED_PSW_LEN, buf );
	    if( (result == 255) || (result == FALSE) )
	      result = FALSE;
	    else
	      {
	      if( LIB_memcmp( psw2, buf, buf[0]+1 ) != 0 )	// different from the old one?
	        {
	        // 2014-10-30, enforce different password values
	        memmove( pkey1, buf, buf[0]+1 );
	        
	        if( mode == 0 )		// DSS2 != DSS1, ADMIN1, ADMIN2
	          {
	          // DSS2 != DSS1 ?
	          OS_SECM_GetData( ADDR_PED_DSS_PSW1, PED_PSW_SLOT_LEN, pkey2 );	          
	          if( LIB_memcmp( pkey1, pkey2, pkey1[0]+1 ) == 0 )
	            {
	            result = FALSE;
	            goto EXIT;
	            }

	          // DSS2 != ADMIN1 ?
	          OS_SECM_GetData( ADDR_PED_ADMIN_PSW1, PED_PSW_SLOT_LEN, pkey2 );	          
	          if( LIB_memcmp( pkey1, pkey2, pkey1[0]+1 ) == 0 )
	            {
	            result = FALSE;
	            goto EXIT;
	            }

	          // DSS2 != ADMIN2 ?
	          OS_SECM_GetData( ADDR_PED_ADMIN_PSW2, PED_PSW_SLOT_LEN, pkey2 );	          
	          if( LIB_memcmp( pkey1, pkey2, pkey1[0]+1 ) == 0 )
	            {
	            result = FALSE;
	            goto EXIT;
	            }
	          }
			else if(mode == 1)	// AMDIN2 != DSS1, DSS2, ADMIN1
	          {
	          // ADMIN2 != DSS1 ?
	          OS_SECM_GetData( ADDR_PED_DSS_PSW1, PED_PSW_SLOT_LEN, pkey2 );	          
	          if( LIB_memcmp( pkey1, pkey2, pkey1[0]+1 ) == 0 )
	            {
	            result = FALSE;
	            goto EXIT;
	            }

	          // ADMIN2 != DSS2 ?
	          OS_SECM_GetData( ADDR_PED_DSS_PSW2, PED_PSW_SLOT_LEN, pkey2 );	          
	          if( LIB_memcmp( pkey1, pkey2, pkey1[0]+1 ) == 0 )
	            {
	            result = FALSE;
	            goto EXIT;
	            }

	          // ADMIN2 != ADMIN1 ?
	          OS_SECM_GetData( ADDR_PED_ADMIN_PSW1, PED_PSW_SLOT_LEN, pkey2 );	          
	          if( LIB_memcmp( pkey1, pkey2, pkey1[0]+1 ) == 0 )
	            {
	            result = FALSE;
	            goto EXIT;
	            }
	          }
			else  //SRED2 != DSS1, DSS2, ADMIN1, ADMIN2, SRED1
			{
				//SRED2 != DSS1 ?
				OS_SECM_GetData(ADDR_PED_DSS_PSW1, PED_PSW_SLOT_LEN, pkey2);
				if(LIB_memcmp(pkey1, pkey2, pkey1[0] + 1) == 0)
				{
					result = FALSE;
					goto EXIT;
				}

				//SRED2 != DSS2 ?
				OS_SECM_GetData(ADDR_PED_DSS_PSW2, PED_PSW_SLOT_LEN, pkey2);
				if(LIB_memcmp(pkey1, pkey2, pkey1[0] + 1) == 0)
				{
					result = FALSE;
					goto EXIT;
				}

				//SRED2 != ADMIN1 ?
				OS_SECM_GetData(ADDR_PED_ADMIN_PSW1, PED_PSW_SLOT_LEN, pkey2);
				if(LIB_memcmp(pkey1, pkey2, pkey1[0] + 1) == 0)
				{
					result = FALSE;
					goto EXIT;
				}

				//SRED2 != ADMIN2 ?
				OS_SECM_GetData(ADDR_PED_ADMIN_PSW2, PED_PSW_SLOT_LEN, pkey2);
				if(LIB_memcmp(pkey1, pkey2, pkey1[0] + 1) == 0)
				{
					result = FALSE;
					goto EXIT;
				}

				//SRED2 != SRED1 ?
				OS_SECM_GetData(ADDR_PED_SRED_PSW1, PED_PSW_SLOT_LEN, pkey2);
				if(LIB_memcmp(pkey1, pkey2, pkey1[0] + 1) == 0)
				{
					result = FALSE;
					goto EXIT;
				}
			}
	          
			if(mode == 0)
				OS_SECM_PutData(ADDR_PED_DSS_PSW2, buf[0] + 1, buf);		// save new psw2 for DSS
			else if(mode == 1)
				OS_SECM_PutData(ADDR_PED_ADMIN_PSW2, buf[0] + 1, buf);	// save new psw2 for KMS
			else
				OS_SECM_PutData(ADDR_PED_SRED_PSW2, buf[0] + 1, buf);	//save new psw2 for SRED
	        }
	      else
	        result = FALSE;
	      }
	    }
	  }
	else
    {
        //Added by Tammy, enter wrong password MAX_PED_PSW_CNT times
        if(result == FALSE)
        {
            // fp = popen(command_rmsecure_region, "r");
            fp = popen(command_rmsecure_directory, "r");
            if(!fp)
                perror("Failed to remove secure_directory");
            else
            {
                //wait 200 ms to complete the deletion of secure_region
                LIB_WaitTime(20);
                // init_secure_memory();
                init_secure_file();
            }
            
            pclose(fp);
        }

        result = FALSE;
    }

	if( result != TRUE )
        goto EXIT;
	  
	// change psw2?
//	result = PED_ChangePassword( mode, 1 );
//	if( result == FALSE )
//	  {
//	  first = FALSE;
//	  goto EXIT;
//	  }

EXIT:
	// clear sensitive data
	memset( psw1, 0x00, sizeof(psw1) );
	memset( psw2, 0x00, sizeof(psw2) );
	memset( buf, 0x00, sizeof(buf) );
	memset( pkey1, 0x00, sizeof(pkey1) );
	memset( pkey2, 0x00, sizeof(pkey2) );
	
	if( !result && first )
	  {
	  if( mode == 0 )
	    {
	    OS_SECM_PutData( ADDR_PED_DSS_PSW1, PED_PSW_SLOT_LEN, psw1 );
	    OS_SECM_PutData( ADDR_PED_DSS_PSW2, PED_PSW_SLOT_LEN, psw2 );
	    }
	  else if(mode == 1)
	    {
	    OS_SECM_PutData( ADDR_PED_ADMIN_PSW1, PED_PSW_SLOT_LEN, psw1 );
	    OS_SECM_PutData( ADDR_PED_ADMIN_PSW2, PED_PSW_SLOT_LEN, psw2 );
	    }
	  else
	  {
		  OS_SECM_PutData(ADDR_PED_SRED_PSW1, PED_PSW_SLOT_LEN, psw1);
		  OS_SECM_PutData(ADDR_PED_SRED_PSW2, PED_PSW_SLOT_LEN, psw2);
	  }
	  }
	
	return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: Dual password control. (NEW)
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//	         FALSE
// ---------------------------------------------------------------------------
#if 0
UINT32	PED_DualPasswordControl()
{
	//UINT32	i;
	UINT32	first = FALSE;
	UINT8	result;
	UINT8	psw1[PED_PSW_SLOT_LEN];
	UINT8	psw2[PED_PSW_SLOT_LEN];
	UINT8	buf[PED_PSW_SLOT_LEN];
	UINT8	pkey1[PED_PSW_SLOT_LEN];
	UINT8	pkey2[PED_PSW_SLOT_LEN];
	

	// get two passwords in secure memory
	OS_SECM_GetData(ADDR_PED_ADMIN_PSW1, PED_PSW_SLOT_LEN, psw1);
	OS_SECM_GetData(ADDR_PED_ADMIN_PSW2, PED_PSW_SLOT_LEN, psw2);

	if( ((psw1[0] == 0) || (psw1[0] > MAX_PED_PSW_LEN) || (psw2[0] == 0) || (psw2[0] > MAX_PED_PSW_LEN)) ||
	    ((psw1[0] == 7) && (psw1[1] == '1') && (psw1[2] == '1') && (psw1[3] == '1') && (psw1[4] == '1') && (psw1[5] == '1') && (psw1[6] == '1') && (psw1[7] == '1')) ||
	    ((psw2[0] == 7) && (psw2[1] == '2') && (psw2[2] == '2') && (psw2[3] == '2') && (psw2[4] == '2') && (psw2[5] == '2') && (psw2[6] == '2') && (psw2[7] == '2'))  )
	{
		// first time use, setup default passwords "1111111" & "2222222"
		first = TRUE;

		psw1[0] = MIN_PED_PSW_LEN;
		memset(&psw1[1], '1', MIN_PED_PSW_LEN);
		OS_SECM_PutData(ADDR_PED_ADMIN_PSW1, MIN_PED_PSW_LEN + 1, psw1);

		psw2[0] = MIN_PED_PSW_LEN;
		memset(&psw2[1], '2', MIN_PED_PSW_LEN);
		OS_SECM_PutData(ADDR_PED_ADMIN_PSW2, MIN_PED_PSW_LEN + 1, psw2);
	}
	  
ENTITY1:
	// request the 1'st password
	result = LIB_EnterPassWordLen2(MAX_PED_PSW_CNT, 1, psw1, 0);
	if(result == TRUE)
	{
		if(first)	// request to change it on first use
		{
			result = PED_RequestNewPassword(MAX_PED_PSW_LEN, buf);
			if((result == 255) || (result == FALSE))
				result = FALSE;
			else
			{
				if(LIB_memcmp(psw1, buf, buf[0] + 1) != 0)	// different from the old one?
				{
					// enforce different password values
					memmove(pkey1, buf, buf[0] + 1);

					// ADMIN1 != ADMIN2 ?
					OS_SECM_GetData(ADDR_PED_ADMIN_PSW2, PED_PSW_SLOT_LEN, pkey2);
					if(LIB_memcmp(pkey1, pkey2, pkey1[0] + 1) == 0)
					{
						result = FALSE;
						goto EXIT;
					}
					else
						OS_SECM_PutData(ADDR_PED_ADMIN_PSW1, buf[0] + 1, buf);	// save new psw1 for KMS
				}
				else
					result = FALSE;
			}
		}
	}
	else
		result = FALSE;

	if(result != TRUE)
		goto EXIT;


ENTITY2:
	// request the 2'nd password
	result = LIB_EnterPassWordLen2(MAX_PED_PSW_CNT, 1, psw2, 1);
	if(result == TRUE)
	{
		if(first)	// request to change it on first use
		{
			result = PED_RequestNewPassword(MAX_PED_PSW_LEN, buf);
			if((result == 255) || (result == FALSE))
				result = FALSE;
			else
			{
				if(LIB_memcmp(psw2, buf, buf[0] + 1) != 0)	// different from the old one?
				{
					// enforce different password values
					memmove(pkey1, buf, buf[0] + 1);

					// ADMIN2 != ADMIN1 ?
					OS_SECM_GetData(ADDR_PED_ADMIN_PSW1, PED_PSW_SLOT_LEN, pkey2);
					if(LIB_memcmp(pkey1, pkey2, pkey1[0] + 1) == 0)
					{
						result = FALSE;
						goto EXIT;
					}
					else
						OS_SECM_PutData(ADDR_PED_ADMIN_PSW2, buf[0] + 1, buf);	// save new psw2 for KMS
				}
				else
					result = FALSE;
			}
		}
	}
	else
		result = FALSE;

EXIT:
	// clear sensitive data
	memset(psw1, 0x00, sizeof(psw1));
	memset(psw2, 0x00, sizeof(psw2));
	memset(buf, 0x00, sizeof(buf));
	memset(pkey1, 0x00, sizeof(pkey1));
	memset(pkey2, 0x00, sizeof(pkey2));

	if(!result && first)
	{
		OS_SECM_PutData(ADDR_PED_ADMIN_PSW1, PED_PSW_SLOT_LEN, psw1);
		OS_SECM_PutData(ADDR_PED_ADMIN_PSW2, PED_PSW_SLOT_LEN, psw2);
	}

	return(result);
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Administrator mode selection.
//	     > CHANGE PASSWORD		<- default = "000000"
//	       VERIFY KEY STATUS
//	       CHANGE KEY MODE
//			> DUKPT
//			  MASTER/SESSION
//				> SELECT MASTER KEY (0..9)
//			  FIXED
//	       SHOW TERM SN
//	       LOAD PED KEY
//			> FIXED
//			  MASTER/SESSION
//			  DUKPT
//			  CAPK
//	       DELETE PED KEY
//	       
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
#if	0
UINT8	PED_AdministratorMode( void )
{
UINT8	buffer[PED_PSW_SLOT_LEN];
UINT8	result;
UINT8	status=0;
UINT8	start;
	
	
	LIB_LCD_Cls();
	LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_ADMIN), (UINT8 *)os_msg_ADMIN );

	// 2014-07-28, dual control password
	PED_SetSensitiveServiceTime( TRUE );
	if( !PED_DualPasswordControl(1) )
	  {
	  PED_SetSensitiveServiceTime( FALSE );
	  return( apiFailed );
	  }
	
	start = 0;
	while(1)
	     {
	     LIB_LCD_Cls();
             LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_SELECT), (UINT8 *)os_msg_SELECT );
             LIB_LCD_PutMsg( 0, COL_RIGHTMOST, FONT0, sizeof(os_msg_ADMIN2), (UINT8 *)os_msg_ADMIN2 );
	
	     buffer[0] = 1;	// starting row number
	     buffer[1] = 12+1-1;	// max lcd row cnt
	     buffer[2] = 12-1;	// list items
             buffer[3] = 18;	// item length
             buffer[4] = 0;	// offset of LEN field in item
             buffer[5] = FONT0;
//	     result = LIB_ListBox( start, &buffer[0], (UINT8 *)&os_list_ADMIN_MODE[0], 30 );	// wait for selection (T.O.=30sec)
	     result = LIB_ListBox( start, &buffer[0], (UINT8 *)&os_list_ADMIN_MODE[0], 30+30 );	// wait for selection (T.O.=60sec for more items)
	     
	     switch( result )
         	 {
                 case 0xff: // aborted
                      
                      PED_SetSensitiveServiceTime( FALSE );
                      return( apiOK );

                 case 0x00: // VERIFY KEY STATUS
                 
                      status = PED_USER_VerifyKeyStatus();
                      break;
                      
                 case 0x01: // CHANGE KEY MODE
                 
		      status = PED_USER_ChangeKeyMode( TRUE );
                      break;
                      
                 case 0x02: // SHOW TERM SN
                 
                      status = PED_USER_ShowTSN();
                      break;
                      
                 case 0x03: // LOAD PED KEY
                      
                      status = PED_ADMIN_LoadPedKey();
					  PED_EraseIKEK();
                      break;
                      
                 case 0x04: // DELETE PED KEY
                 
                      status = PED_ADMIN_DeletePedKey();
                      break;
                      
		 case 0x05: // LOAD IKEK KEY

		      status = PED_ADMIN_LoadIKEK();
		      break;
		      
		 case 0x06: // LOAD ISO4 AES KEY

		      status = PED_ADMIN_LoadISO4KEY();	// 2019-01-28
		      break;
		      
		 case 0x07: // LOAD ACC DEK, 2019-03-08

		      if( SYS_FUNC_SetAccDEK() )
		        status = apiOK;
		      else
		      	status = apiFailed;
		      	
		      break;

		 case 0x08: // SETUP SRED (enabled or disabled SRED), 2019-03-08

		      if( SYS_FUNC_SetSRED() )
		      	status = apiOK;
		      else
		      	status = apiFailed;
		      	
		      PED_SetSensitiveServiceTime( FALSE );
		      
		      return( status );

		 case 0x09: // CLEAR SECM (clear all sensitive data), 2019-03-08

		      status = PED_ResetMode();
		      
		      PED_SetSensitiveServiceTime( FALSE );
		      
		      return( status );
		      
		 case 0x0A: // RESET SECM (erase all NVSRAM data), 2019-03-08

		      if( SYS_FUNC_ResetNVSRAM() )
		        status = apiOK;
		      else
		      	status = apiFailed;
		      
		      PED_SetSensitiveServiceTime( FALSE );
		      
		      return( status );
                 }

	     if( status == apiFailed )
	       {
	       PED_SetSensitiveServiceTime( FALSE );
	       return( apiFailed );
	       }

	     start = result;
	     } // while(1)

}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: User mode selection.
//	     > CHANGE PASSWORD		<- default = "000000"
//	       VERIFY KEY STATUS
//	       CHANGE KEY MODE
//	       SHOW TERM SN
//
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
#if	0
UINT8	PED_UserMode( void )
{
UINT8	buffer[PED_PSW_SLOT_LEN];
UINT8	result;
UINT8	status=0;
UINT8	start;

	
	LIB_LCD_Cls();
	LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_USER), (UINT8 *)os_msg_USER );
	
	OS_SECM_GetData( ADDR_PED_USER_PSW, PED_PSW_SLOT_LEN, buffer );	// L-V
	
	result = LIB_EnterPassWordLen( MAX_PED_PSW_CNT, 1, buffer );
	memset( buffer, 0x00, sizeof(buffer) );	// clear PIN buffer
	
	if( result == 255 )
	  return( apiOK ); // abort
	if( result != TRUE )
	  return( apiFailed );
	  
	start = 0;
	while(1)
	     {
	     LIB_LCD_Cls();
             LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_SELECT), (UINT8 *)os_msg_SELECT );
             LIB_LCD_PutMsg( 0, COL_RIGHTMOST, FONT0, sizeof(os_msg_USER2), (UINT8 *)os_msg_USER2 );
	
	     buffer[0] = 1;	// starting row number
             buffer[1] = 5;	// max lcd row cnt
             buffer[2] = 4-1;	// list items, 2014-10-15
             buffer[3] = 18;	// item length
             buffer[4] = 0;	// offset of LEN field in item
             buffer[5] = FONT0;
	     result = LIB_ListBox( start, &buffer[0], (UINT8 *)&os_list_USER_MODE[0], 30 ); // wait for selection (T.O.=30sec)
	     
	     switch( result )
         	 {
                 case 0xff: // aborted
                      
                      return( apiOK );

                 case 0x00: // CHANGE PASSWORD
                      
                      status = PED_USER_ChangePassword();
                      break;
                      
                 case 0x01: // VERIFY KEY STATUS
                      
//                    status = PED_USER_VerifyKeyStatus();
		      status = apiOK;	// not supported
                      break;
                      
                 case 0x02: // CHANGE KEY MODE

//		      status = PED_USER_ChangeKeyMode( FALSE );
		      status = apiOK;	// not supported
                      break;
#if	0	// 2014-10-15, remove this entry
                 case 0x03: // SHOW TERM SN
                 
                      status = PED_USER_ShowTSN();
                      break;
#endif
                 }

	     if( status == apiFailed )
	       return( apiFailed );
	           
	     start = result;
	     } // while(1)

}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Reset all PED secure data. (key, passsword...)
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UINT8	PED_ResetMode( void )
{	
#ifdef _LCD_ENABLED_
	LIB_LCD_Cls();
//	if( LIB_WaitKeyMsgYesNo( 3, COL_LEFTMOST, sizeof(os_msg_Q_RESET), (UINT8 *)os_msg_Q_RESET ) == TRUE )
	if( LIB_WaitKeyMsgYesNoTO( 3, COL_LEFTMOST, sizeof(os_msg_Q_RESET), (UINT8 *)os_msg_Q_RESET ) == TRUE )
	  {
	  LIB_LCD_PutMsg( 1, COL_MIDWAY, FONT1+attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING );

	  PED_EraseSensitiveData();
	  
	  LIB_LCD_PutMsg( 3, COL_RIGHTMOST, FONT1+attrCLEARWRITE, sizeof(os_msg_OK), (UINT8 *)os_msg_OK );  
	  LIB_WaitTimeAndKey( 100 );
	  
	  return( apiOK );
	  };
	  
	return( apiFailed );
#else
	PED_EraseSensitiveData();
	LIB_BUZ_OkBeep();
	return apiOK;
#endif

}
// ---------------------------------------------------------------------------
// FUNC  : save IMEK & IAEK keys to FLASH.
// INPUT : IMEK, IAEK - the key value. (L-V)
// OUTPUT: none.
// RETURN: apiOK
//         apiFailed
// ---------------------------------------------------------------------------
UCHAR	PED_PutWaveIEK( UCHAR *imek, UCHAR *iaek )
{
UCHAR	result;

	
	if( imek[0] != 0 )
	  {
	  result = OS_FLS_PutData( F_ADDR_CL_IMEK, CL_IMEK_LEN, &imek[1] );	// save (IMEK) to FLASH
	  if( !result )
	    return( apiFailed );
	  }
	
	if( iaek[0] != 0 )
	  {
	  result = OS_FLS_PutData( F_ADDR_CL_IAEK, CL_IAEK_LEN, &iaek[1] );	// save (IAEK) to FLASH
	  if( !result )
	    return( apiFailed );
	  }

	return( apiOK );
}
// ---------------------------------------------------------------------------
// FUNC  : get IMEK from FLASH.
// INPUT : none.
// OUTPUT: IMEK - the key value. (L-V)
// RETURN: apiOK
//         apiFailed
// ---------------------------------------------------------------------------
UCHAR	PED_GetWaveIMEK( UCHAR *imek )
{
UCHAR	result = apiFailed;
UCHAR	buf[CL_IMEK_LEN];


	OS_FLS_GetData( F_ADDR_CL_IMEK, CL_IMEK_LEN, buf );
	if( LIB_memcmpc( buf, 0xFF, CL_IMEK_LEN ) != 0 )
	  {
	  imek[0] = CL_IMEK_LEN;
	  memmove( &imek[1], buf, CL_IMEK_LEN );
	  result = apiOK;
	  }
	
	return( result );
}
// ---------------------------------------------------------------------------
// FUNC  : get IAEK from FLASH.
// INPUT : none.
// OUTPUT: IAEK - the key value. (L-V)
// RETURN: apiOK
//         apiFailed
// ---------------------------------------------------------------------------
UCHAR	PED_GetWaveIAEK( UCHAR *iaek )
{
UCHAR	result = apiFailed;
UCHAR	buf[CL_IAEK_LEN];


	OS_FLS_GetData( F_ADDR_CL_IAEK, CL_IAEK_LEN, buf );
	if( LIB_memcmpc( buf, 0xFF, CL_IAEK_LEN ) != 0 )
	  {
	  iaek[0] = CL_IAEK_LEN;
	  memmove( &iaek[1], buf, CL_IAEK_LEN );
	  result = apiOK;
	  }
	
	return( result );
}
// ---------------------------------------------------------------------------
// FUNCTION: Verify the KPK loaded status.
// INPUT   : type - 0x01 = KPK used for 128-bit TDES key.
//                  0x02 = KPK used for 192-bit TDES key.
//                  0x03 = KPK used for 128-bit AES key.
//                  0x04 = KPK used for 192-bit AES key.
//                  0x05 = KPK used for 256-bit AES key.
// OUTPUT  : none.
// RETURN  : TRUE  - KPK has been setup.
//           FALSE - KPK has not been setup.
// ---------------------------------------------------------------------------
UINT8	PED_VerifyKPKStatus(UINT8 type)
{
	UINT8	status = 0;


	if(type == TDES_128)
	{
		OS_SECM_GetData(ADDR_SYS_SETUP_128_TDES_KPK_FLAG, 1, &status);

		if(status == 0x01)
			return TRUE;
		else
			return FALSE;
	}
    else if(type == TDES_192)
	{
		OS_SECM_GetData(ADDR_SYS_SETUP_192_TDES_KPK_FLAG, 1, &status);

		if(status == 0x01)
			return TRUE;
		else
			return FALSE;
	}
    else if(type == AES_128)
	{
		OS_SECM_GetData(ADDR_SYS_SETUP_128_AES_KPK_FLAG, 1, &status);

		if(status == 0x01)
			return TRUE;
		else
			return FALSE;
	}
    else if(type == AES_192)
	{
		OS_SECM_GetData(ADDR_SYS_SETUP_192_AES_KPK_FLAG, 1, &status);

		if(status == 0x01)
			return TRUE;
		else
			return FALSE;
	}
	else if(type == AES_256)
	{
		OS_SECM_GetData(ADDR_SYS_SETUP_256_AES_KPK_FLAG, 1, &status);

		if(status == 0x01)
			return TRUE;
		else
			return FALSE;
	}

	return FALSE;
}

// ---------------------------------------------------------------------------
// FUNCTION: Set the KPK loaded status
// INPUT   : type - 0x01 = KPK used for 128-bit TDES key.
//                  0x02 = KPK used for 192-bit TDES key.
//                  0x03 = KPK used for 128-bit AES key.
//                  0x04 = KPK used for 192-bit AES key.
//                  0x05 = KPK used for 256-bit AES key.
//           mode - 0 = KPK has not been setup.
//				    1 = KPK has been setup.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_SetKPKStatus(UINT8 type, UINT8 mode)
{
	if(type == TDES_128)
		OS_SECM_PutData(ADDR_SYS_SETUP_128_TDES_KPK_FLAG, 1, &mode);
    else if(type == TDES_192)
		OS_SECM_PutData(ADDR_SYS_SETUP_192_TDES_KPK_FLAG, 1, &mode);
    else if(type == AES_128)
		OS_SECM_PutData(ADDR_SYS_SETUP_128_AES_KPK_FLAG, 1, &mode); 
    else if(type == AES_192)
		OS_SECM_PutData(ADDR_SYS_SETUP_192_AES_KPK_FLAG, 1, &mode); 
	else if(type == AES_256)
		OS_SECM_PutData(ADDR_SYS_SETUP_256_AES_KPK_FLAG, 1, &mode);
}

// ---------------------------------------------------------------------------
// FUNCTION: Verify the IKEK loaded status by checking "ADDR_SYS_LOAD_IKEK_FLAG".
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - load IKEK from Initial Key Loading System (IKLS)
//           FALSE - load IKEK from PED_InitIKEK()
// ---------------------------------------------------------------------------
UINT8	PED_VerifyIkekStatus(void)
{
	UINT8	status = 0;


	OS_SECM_GetData(ADDR_SYS_LOAD_IKEK_FLAG, 1, &status);

	if(status == 0x01)
		return TRUE;
	else
		return FALSE;
}

// ---------------------------------------------------------------------------
// FUNCTION: Set the IKEK loaded status
// INPUT   : mode - 0 = load IKEK from PED_InitIKEK()
//				    1 = load IKEK from Initial Key Loading System (IKLS)
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_SetIkekStatus(UINT8 mode)
{
	OS_SECM_PutData(ADDR_SYS_LOAD_IKEK_FLAG, 1, &mode);
}

// ---------------------------------------------------------------------------
// FUNCTION: Reset system.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
//void	PED_ResetSystem( void )
//{
//	// memory remap
//	BSP_WR32( PMU_CFG_REG,  BSP_RD32( PMU_CFG_REG ) & 0xFFFFFFFE );
//
//	// soft reset
//	BSP_WR32( PMU_RST_REG,  BSP_RD32( PMU_RST_REG ) | 0x00010000 );
//}

// ---------------------------------------------------------------------------
// FUNCTION: Select download method
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
/*UINT8	PED_SelectDownloadMethod(void)
{
	UINT8	msg_list_DOWNLOAD_MODE[] =
			{ 
			  06, 'R','E','M','O','T','E',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',  // 00: REMOTE
			  05, 'L','O','C','A','L',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '  // 01: LOCAL
			};
	UINT8	buffer[8];
	UINT8	result;
	UINT8	start;

	start = 0;
	while(1)
	{
		LIB_LCD_Cls();
		LIB_LCD_Puts(0, 0, FONT0, sizeof(os_msg_SELECT), (UINT8 *)os_msg_SELECT);

		buffer[0] = 1;	// starting row number
		buffer[1] = 3;	// max lcd row cnt
		buffer[2] = 2;	// list items
		buffer[3] = 18;	// item length
		buffer[4] = 0;	// offset of LEN field in item
		buffer[5] = FONT0;
		result = LIB_ListBox(start, &buffer[0], (UINT8 *)&msg_list_DOWNLOAD_MODE[0], 30); // wait for selection (T.O.=30sec)

		switch(result)
		{
			case 0xff: //Aborted

				return FALSE;

			case 0x00: //Remote key loading

				PED_loadKey_flag = PED_REMOTE_KEY_LOADING;
				return TRUE;

			case 0x01: //Local key loading

				PED_loadKey_flag = PED_LOCAL_KEY_LOADING;
				return TRUE;
		}

		start = result;
	}
}*/

// ---------------------------------------------------------------------------
// FUNCTION: Erase Secret Key (use for remote key loading).
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
/*void	PED_EraseSecretKey(void)
{
	OS_SECM_ClearData(ADDR_PED_SECRET_KEY, PED_SECRET_KEY_LEN, 0x00);
}*/

// ---------------------------------------------------------------------------
// FUNCTION: get status of PED keys.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
//
//           PED KEY STATUS
//           MASTER/SESSION
//           0123456789
//           ----------------
//	         ----------------
//           DUKPT
//           012345678901234567890
//           ---------------------
//           ISO4_KEY
//           -
//           ACC_DEK
//           -
//           FPE KEY
//           -
//           CAPK
//                             [OK]
// ---------------------------------------------------------------------------
/*
UINT8	PED_VerifyKeyStatus(void)
{
	UINT32	i, j;
	UINT8	lrc;
	UINT8	str_dukpt[] = {"DUKPT"};
	UINT8	str_ms[] = {"MASTER/SESSION"};
	UINT8	str_na[] = {"<NA>"};
	UINT8	str_rid[] = {"RID:"};
	UINT8	str_capk[] = {"CAPK"};
	UINT8	str_iso4[] = {"ISO4_KEY"};
	UINT8	str_accDek[] = {"ACC_DEK"};
	UINT8	str_fpe[] = {"FPE KEY"};
	UINT8	str_slot10[] = {"0123456789"};
	UINT8	str_slot21[] = {"012345678901234567890"};
	UINT8	buf[MAX_FUTURE_KEY_REG_CNT * MAX_PAYMENT_SCHEME_CNT];
	UINT8	temp[MAX_FUTURE_KEY_REG_CNT];

	PED_KEY_BUNDLE	KeyBundle;
	UINT8	keydata[KEY_DATA_LEN + 1]; // L-V
	UINT8	mac8[8];
	UINT8	mkey[PED_MSKEY_LEN + 1];
	UINT8	keypress;


	LIB_LCD_Cls();
	LIB_LCD_PutMsg(0, COL_LEFTMOST, FONT1, sizeof(os_msg_PED_KEY_STATUS), (UINT8 *)os_msg_PED_KEY_STATUS);

	// ================== Master/Session Key ==================
	LIB_LCD_Puts(1, 0, FONT1 + attrREVERSE, sizeof(str_ms) - 1, str_ms);

	LIB_LCD_Puts(2, 0, FONT1, sizeof(str_slot10) - 1, str_slot10);
	memset(temp, '-', sizeof(str_slot10) - 1);

	for(i = 0; i < MAX_MKEY_CNT; i++)
	{
		// get MASTER key bundle
		OS_SECM_GetData(ADDR_PED_MKEY_01 + (i * PED_MSKEY_SLOT_LEN), PED_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle);

		// verify MASTER key bundle
		if(TR31_VerifyKeyBundle(PED_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle, keydata, mac8, (UINT8 *)0))
		{
			temp[i] = '*';

			// retrieve MKEY
			TR31_DecryptKeyBundle(mac8, keydata, mkey, (UINT8 *)0);	// mkey=MKEY (as the KBPK for SKEY)
		}
	}
	LIB_LCD_Puts(3, 0, FONT1, sizeof(str_slot10) - 1, temp);	// show key status

	memset(temp, '-', sizeof(str_slot10) - 1);

	for(i = 0 ; i < MAX_SKEY_CNT ; i++)
	{
		// get SESSION key bundle
		OS_SECM_GetData(ADDR_PED_SKEY_01 + (i * PED_MSKEY_SLOT_LEN), PED_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle);

		// verify SESSION key bundle
		if(TR31_VerifyKeyBundle(PED_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle, keydata, mac8, mkey))
			temp[i] = '*';
	}
	LIB_LCD_Puts(4, 0, FONT1, sizeof(str_slot10) - 1, temp);	// show key status

	keypress = LIB_WaitTimeAndKey(3000);
	if((keypress == 'x') || (keypress == 255))
	{
		// clear sensitive data
		memset((UINT8 *)&KeyBundle, 0x00, sizeof(KeyBundle));
		memset((UINT8 *)&keydata, 0x00, sizeof(keydata));
		memset(mkey, 0x00, sizeof(mkey));

		return(apiOK);
	}

	LIB_LCD_ClearRow(1, 7, FONT1);


	// ================== DUKPT Key ==================
	LIB_LCD_Puts(1, 0, FONT1 + attrREVERSE, sizeof(str_dukpt) - 1, str_dukpt);
	LIB_LCD_Puts(2, 0, FONT1, sizeof(str_slot21) - 1, str_slot21);
	memset(temp, '-', sizeof(temp));

	for(i = 0 ; i < MAX_FUTURE_KEY_REG_CNT ; i++)
	{
		OS_SECM_GetData(ADDR_FUTURE_KEY_REG + (i * FUTURE_KEY_SLOT_LEN), FUTURE_KEY_SLOT_LEN, buf);

		//  verify DUKPT key by checking LRC
		if(LIB_memcmpc(buf, 0x00, FUTURE_KEY_SLOT_LEN) != 0)
		{
			lrc = 0;
			for(j = 0 ; j < FUTURE_KEY_LEN ; j++)
				lrc ^= buf[j];
			if(lrc == buf[j])
				temp[i] = '*';
		}
	}

	memset(buf, 0x00, FUTURE_KEY_SLOT_LEN);	// clear FUTURE key buffer  
	LIB_LCD_Puts(3, 0, FONT1, sizeof(temp), temp);	// show key status

	keypress = LIB_WaitTimeAndKey(3000);
	if((keypress == 'x') || (keypress == 255))
	{
		// clear sensitive data
		memset((UINT8 *)&KeyBundle, 0x00, sizeof(KeyBundle));
		memset((UINT8 *)&keydata, 0x00, sizeof(keydata));
		memset(buf, 0x00, sizeof(buf));

		return(apiOK);
	}

	LIB_LCD_ClearRow(1, 7, FONT1);


	// ================== ISO4_KEY ==================
	LIB_LCD_Puts(1, 0, FONT1 + attrREVERSE, sizeof(str_iso4) - 1, str_iso4);

	if(PED_VerifyISO4KEY())
		LIB_LCD_Putc(2, 0, FONT1, '*');
	else
		LIB_LCD_Putc(2, 0, FONT1, '-');

	keypress = LIB_WaitTimeAndKey(3000);
	if((keypress == 'x') || (keypress == 255))
	{
		// clear sensitive data
		memset(buf, 0x00, sizeof(buf));
		return(apiOK);
	}

	LIB_LCD_ClearRow(1, 7, FONT1);


	// ================== ACC_DEK ==================
	LIB_LCD_Puts(1, 0, FONT1 + attrREVERSE, sizeof(str_accDek) - 1, str_accDek);

	if(PED_VerifyAccDEK())
		LIB_LCD_Putc(2, 0, FONT1, '*');
	else
		LIB_LCD_Putc(2, 0, FONT1, '-');

	keypress = LIB_WaitTimeAndKey(3000);
	if((keypress == 'x') || (keypress == 255))
	{
		// clear sensitive data
		memset(buf, 0x00, sizeof(buf));
		return(apiOK);
	}

	LIB_LCD_ClearRow(1, 7, FONT1);


	// ================== FPE Key ==================
	LIB_LCD_Puts(1, 0, FONT1 + attrREVERSE, sizeof(str_fpe) - 1, str_fpe);

	if(PED_VerifyFPEKey())
		LIB_LCD_Putc(2, 0, FONT1, '*');
	else
		LIB_LCD_Putc(2, 0, FONT1, '-');

	keypress = LIB_WaitTimeAndKey(3000);
	if((keypress == 'x') || (keypress == 255))
	{
		// clear sensitive data
		memset(buf, 0x00, sizeof(buf));
		return(apiOK);
	}

	LIB_LCD_ClearRow(1, 7, FONT1);


	// ================== CAPK ==================
	LIB_LCD_PutMsg(1, COL_LEFTMOST, FONT1 + attrREVERSE, sizeof(str_capk) - 1, str_capk);

	PED_CAPK_GetInfo(buf);

	if(buf[0] == 0xFF)
	{
		LIB_LCD_PutMsg(2, COL_LEFTMOST, FONT1, sizeof(str_na) - 1, str_na);
		LIB_WaitTimeAndKey(3000);
	}
	else
	{
		LIB_LCD_PutMsg(2, COL_LEFTMOST, FONT1, sizeof(str_rid) - 1, str_rid);
		for(i = 0; i < MAX_PAYMENT_SCHEME_CNT; i++)
		{
			if(buf[i * 21] != 0xFF)
			{
				for(j = 0; j < RID_LEN; j++)
					LIB_DispHexByte(2, 2 * j + 4, buf[i * 21 + j]);

				LIB_DumpHexDataTO(0, 4, buf[i * 21 + 5], &buf[i * 21 + 6]);
			}
		}
	}

	// clear sensitive data
	memset((UINT8 *)&KeyBundle, 0x00, sizeof(KeyBundle));
	memset((UINT8 *)&keydata, 0x00, sizeof(keydata));
	memset(buf, 0x00, sizeof(buf));

	return(apiOK);
}
*/

// ---------------------------------------------------------------------------
// FUNCTION: get status of PED keys.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
//
//           PED KEY STATUS
//           MASTER/SESSION
//           0123456789
//           ----------------
//	         ----------------
//           DUKPT
//           012345678901234567890
//           ---------------------
//           ACC_DEK
//           -
//           FPE KEY
//           -
//           CAPK
//                             [OK]
// ---------------------------------------------------------------------------
UINT8	PED_VerifyKeyStatus(void)
{
    UINT8   keyType;
	UINT32	i, j;
	UINT8	lrc;
	UINT8	str_dukpt[] = {"AES DUKPT"};
	UINT8	str_na[] = {"<NA>"};
	UINT8	str_rid[] = {"RID:"};
	UINT8	str_capk[] = {"CAPK"};
    UINT8   str_pek[] = {"PEK"};
	UINT8	str_accDek[] = {"ACC_DEK"};
	UINT8	str_fpe[] = {"FPE KEY"};
	UINT8	str_slot10[] = {"0123456789"};
	UINT8	str_slot32[] = {"01234567890123456789012345678901"};
	UINT8	buf[MAX_FUTURE_KEY_REG_CNT * MAX_PAYMENT_SCHEME_CNT];
	UINT8	temp[MAX_NUM_REG];
    UINT8   buf2[MAX_KEYLENGTH];

    X9143_TDES_PED_KEY_BUNDLE   tdesKeyBundle;
    X9143_AES_PED_KEY_BUNDLE    aesKeyBundle;
    UINT8	keydata[X9143_AES_KEY_DATA_LEN + 1]; // L-V
	UINT8	mac8[8];
    UINT8   mac16[16];
    UINT8	mkey[32 + 1];
	UINT8	keypress;


	LIB_LCD_Cls();
	LIB_LCD_PutMsg(0, COL_LEFTMOST, FONT1, sizeof(os_msg_PED_KEY_STATUS), (UINT8 *)os_msg_PED_KEY_STATUS);

	// ================== Master/Session Key for PEK ==================
	LIB_LCD_Puts(1, 0, FONT1 + attrREVERSE, sizeof(str_pek) - 1, str_pek);

	LIB_LCD_Puts(2, 0, FONT1, sizeof(str_slot10) - 1, str_slot10);
	memset(temp, '-', sizeof(str_slot10) - 1);

	for(i = 0 ; i < MAX_PEK_MKEY_CNT ; i++)
	{
        // get PEK key type
        PED_PEK_GetKeyType(&keyType);

        // specify the KPK used to verify and decrypt key bundle
        PED_SetKPKStatus(keyType, 1);

        if((keyType == TDES_128) || (keyType == TDES_192))
        {
            // get PEK MKEY bundle
		    OS_SECM_GetData(ADDR_PED_PEK_TDES_MKEY_01 + (i * PED_PEK_TDES_MSKEY_SLOT_LEN), PED_PEK_TDES_MSKEY_SLOT_LEN, (UINT8 *)&tdesKeyBundle);

            // verify PEK MKEY bundle
            if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, (UINT8 *)&tdesKeyBundle, keydata, mac8, (UINT8 *)0))
            {
                temp[i] = '*';

                // retrieve PEK MKEY
                X9143_DecryptKeyBundle_TDES(mac8, keydata, mkey, (UINT8 *)0);	// mkey=PEK MKEY (as the KBPK for PEK SKEY)
            }
        }
        else if((keyType == AES_128) || (keyType == AES_192) || (keyType == AES_256))
        {
            // get PEK MKEY bundle
		    OS_SECM_GetData(ADDR_PED_PEK_AES_MKEY_01 + (i * PED_PEK_AES_MSKEY_SLOT_LEN), PED_PEK_AES_MSKEY_SLOT_LEN, (UINT8 *)&aesKeyBundle);

            // verify PEK MKEY bundle
            if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, (UINT8 *)&aesKeyBundle, keydata, mac16, (UINT8 *)0))
            {
                temp[i] = '*';

                // retrieve PEK MKEY
                X9143_DecryptKeyBundle_AES(mac16, keydata, mkey, (UINT8 *)0);	// mkey=PEK MKEY (as the KBPK for PEK SKEY)
            }
        }
	}
	LIB_LCD_Puts(3, 0, FONT1, sizeof(str_slot10) - 1, temp);	// show key status

	memset(temp, '-', sizeof(str_slot10) - 1);

	for(i = 0 ; i < MAX_PEK_SKEY_CNT ; i++)
	{
        if((keyType == TDES_128) || (keyType == TDES_192))
        {
            // get PEK SKEY bundle
            OS_SECM_GetData(ADDR_PED_PEK_TDES_SKEY_01 + (i * PED_PEK_TDES_MSKEY_SLOT_LEN), PED_PEK_TDES_MSKEY_SLOT_LEN, (UINT8 *)&tdesKeyBundle);

            // verify PEK SKEY bundle
            if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, (UINT8 *)&tdesKeyBundle, keydata, mac8, mkey))
                temp[i] = '*';
        }
        else if((keyType == AES_128) || (keyType == AES_192) || (keyType == AES_256))
        {
            // get PEK SKEY bundle
            OS_SECM_GetData(ADDR_PED_PEK_AES_SKEY_01 + (i * PED_PEK_AES_MSKEY_SLOT_LEN), PED_PEK_AES_MSKEY_SLOT_LEN, (UINT8 *)&aesKeyBundle);

            // verify PEK SKEY bundle
            if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, (UINT8 *)&aesKeyBundle, keydata, mac16, mkey))
                temp[i] = '*';
        }
	}

	LIB_LCD_Puts(4, 0, FONT1, sizeof(str_slot10) - 1, temp);	// show key status

    // reset flag
    PED_SetKPKStatus(keyType, 0);

	keypress = LIB_WaitTimeAndKey(3000);
	if((keypress == 'x') || (keypress == 255))
	{
        // reset flag
        PED_SetKPKStatus(keyType, 0);

		// clear sensitive data
		memset((UINT8 *)&tdesKeyBundle, 0x00, sizeof(tdesKeyBundle));
        memset((UINT8 *)&aesKeyBundle, 0x00, sizeof(aesKeyBundle));
		memset(mkey, 0x00, sizeof(mkey));

		return(apiOK);
	}

	LIB_LCD_ClearRow(1, 7, FONT1);


	// ================== AES DUKPT Key ==================
	LIB_LCD_Puts(1, 0, FONT1 + attrREVERSE, sizeof(str_dukpt) - 1, str_dukpt);
	LIB_LCD_Puts(2, 0, FONT1, 16, str_slot32);
	memset(temp, '-', sizeof(temp));
    
    for(i = 0 ; i < MAX_NUM_REG ; i++)
    {
        // OS_SECM_GetData(ADDR_INT_DERIVATION_KEY_REG + (i * MAX_KEYLENGTH), MAX_KEYLENGTH, buf2);
        OS_SECM_GetIntDerivationKeyReg(i * MAX_KEYLENGTH, MAX_KEYLENGTH, buf2);
        
        PED_AES_DUKPT_GetKeyType(&keyType);
        if(keyType == AES_128)
        {
            //  verify DUKPT key
            if(LIB_memcmpc(buf2, 0x00, 16) != 0)
                temp[i] = '*';
        }
        else if(keyType == AES_192)
        {
            //  verify DUKPT key
            if(LIB_memcmpc(buf2, 0x00, 24) != 0)
                temp[i] = '*';
        }
        else if(keyType == AES_256)
        {
            //  verify DUKPT key
            if(LIB_memcmpc(buf2, 0x00, 32) != 0)
                temp[i] = '*';
        }
    }

	memset(buf, 0x00, MAX_KEYLENGTH);	// clear FUTURE key buffer  
	
    LIB_LCD_Puts(3, 0, FONT1, 16, temp);	// show key status
    LIB_LCD_Puts(4, 0, FONT1, 16, &temp[16]);	// show key status

	keypress = LIB_WaitTimeAndKey(3000);
	if((keypress == 'x') || (keypress == 255))
	{
		// clear sensitive data
		memset(buf, 0x00, sizeof(buf));

		return(apiOK);
	}

	LIB_LCD_ClearRow(1, 7, FONT1);


	// ================== Master/Session Key for ACC_DEK ==================
	LIB_LCD_Puts(1, 0, FONT1 + attrREVERSE, sizeof(str_accDek) - 1, str_accDek);

	// get ACC_DEK key type
    PED_AccDEK_GetKeyType(&keyType);

    // specify the KPK used to verify and decrypt key bundle
    PED_SetKPKStatus(keyType, 1);

    if(keyType == TDES_192)
    {
        // get ACC_DEK MKEY bundle
	    OS_SECM_GetData(ADDR_PED_ACC_DEK_TDES_MKEY_01, PED_ACC_DEK_TDES_MSKEY_SLOT_LEN, (UINT8 *)&tdesKeyBundle);

        // verify ACC_DEK MKEY bundle
        if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, (UINT8 *)&tdesKeyBundle, keydata, mac8, (UINT8 *)0))
        {
            // retrieve ACC_DEK MKEY
            if(X9143_DecryptKeyBundle_TDES(mac8, keydata, mkey, (UINT8 *)0))	// mkey=ACC_DEK MKEY (as the KBPK for ACC_DEK SKEY)
            {
                // get ACC_DEK SKEY bundle
                OS_SECM_GetData(ADDR_PED_ACC_DEK_TDES_SKEY_01, PED_ACC_DEK_TDES_MSKEY_SLOT_LEN, (UINT8 *)&tdesKeyBundle);

                // verify ACC_DEK SKEY bundle
                if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, (UINT8 *)&tdesKeyBundle, keydata, mac8, mkey))
                    LIB_LCD_Putc(2, 0, FONT1, '*'); // show key status
                else
                    LIB_LCD_Putc(2, 0, FONT1, '-'); // show key status
            }
            else
                LIB_LCD_Putc(2, 0, FONT1, '-'); // show key status
        }
        else
            LIB_LCD_Putc(2, 0, FONT1, '-'); // show key status
    }
    else if((keyType == AES_128) || (keyType == AES_192) || (keyType == AES_256))
    {
        // get ACC_DEK MKEY bundle
	    OS_SECM_GetData(ADDR_PED_ACC_DEK_AES_MKEY_01, PED_ACC_DEK_AES_MSKEY_SLOT_LEN, (UINT8 *)&aesKeyBundle);

        // verify ACC_DEK MKEY bundle
        if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, (UINT8 *)&aesKeyBundle, keydata, mac16, (UINT8 *)0))
        {
            // retrieve ACC_DEK MKEY
            if(X9143_DecryptKeyBundle_AES(mac16, keydata, mkey, (UINT8 *)0))	// mkey=ACC_DEK MKEY (as the KBPK for ACC_DEK SKEY)
            {
                // get ACC_DEK SKEY bundle
                OS_SECM_GetData(ADDR_PED_ACC_DEK_AES_SKEY_01, PED_ACC_DEK_AES_MSKEY_SLOT_LEN, (UINT8 *)&aesKeyBundle);

                // verify ACC_DEK SKEY bundle
                if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, (UINT8 *)&aesKeyBundle, keydata, mac16, mkey))
                    LIB_LCD_Putc(2, 0, FONT1, '*'); // show key status
                else
                    LIB_LCD_Putc(2, 0, FONT1, '-'); // show key status
            }
            else
                LIB_LCD_Putc(2, 0, FONT1, '-'); // show key status
        }
        else
            LIB_LCD_Putc(2, 0, FONT1, '-'); // show key status
    }
    else
        LIB_LCD_Putc(2, 0, FONT1, '-'); // show key status

    // reset flag
    PED_SetKPKStatus(keyType, 0);

	keypress = LIB_WaitTimeAndKey(3000);
	if((keypress == 'x') || (keypress == 255))
	{
		// clear sensitive data
		memset((UINT8 *)&tdesKeyBundle, 0x00, sizeof(tdesKeyBundle));
        memset((UINT8 *)&aesKeyBundle, 0x00, sizeof(aesKeyBundle));
		memset(mkey, 0x00, sizeof(mkey));

		return(apiOK);
	}

	LIB_LCD_ClearRow(1, 7, FONT1);


	// ================== Master/Session Key for FPE Key ==================
	LIB_LCD_Puts(1, 0, FONT1 + attrREVERSE, sizeof(str_fpe) - 1, str_fpe);

    // get FPE key type
    PED_FPE_GetKeyType(&keyType);

    // specify the KPK used to verify and decrypt key bundle
    PED_SetKPKStatus(keyType, 1);

    // get FPE MKEY bundle
	OS_SECM_GetData(ADDR_PED_FPE_KEY_AES_MKEY_01, PED_FPE_KEY_AES_MSKEY_SLOT_LEN, (UINT8 *)&aesKeyBundle);

    // verify FPE MKEY bundle
    if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, (UINT8 *)&aesKeyBundle, keydata, mac16, (UINT8 *)0))
    {
        // retrieve FPE MKEY
        if(X9143_DecryptKeyBundle_AES(mac16, keydata, mkey, (UINT8 *)0))	// mkey=FPE MKEY (as the KBPK for FPE SKEY)
        {
            // get FPE SKEY bundle
            OS_SECM_GetData(ADDR_PED_FPE_KEY_AES_SKEY_01, PED_FPE_KEY_AES_MSKEY_SLOT_LEN, (UINT8 *)&aesKeyBundle);

            // verify FPE SKEY bundle
            if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, (UINT8 *)&aesKeyBundle, keydata, mac16, mkey))
                LIB_LCD_Putc(2, 0, FONT1, '*'); // show key status
            else
                LIB_LCD_Putc(2, 0, FONT1, '-'); // show key status
        }
        else
            LIB_LCD_Putc(2, 0, FONT1, '-'); // show key status
    }
    else
        LIB_LCD_Putc(2, 0, FONT1, '-'); // show key status

    // reset flag
    PED_SetKPKStatus(keyType, 0);

	keypress = LIB_WaitTimeAndKey(3000);
	if((keypress == 'x') || (keypress == 255))
	{
		// clear sensitive data
        memset((UINT8 *)&aesKeyBundle, 0x00, sizeof(aesKeyBundle));
		memset(mkey, 0x00, sizeof(mkey));

		return(apiOK);
	}

	LIB_LCD_ClearRow(1, 7, FONT1);


	// ================== CAPK ==================
	LIB_LCD_PutMsg(1, COL_LEFTMOST, FONT1 + attrREVERSE, sizeof(str_capk) - 1, str_capk);

	PED_CAPK_GetInfo(buf);

	if(buf[0] == 0xFF)
	{
		LIB_LCD_PutMsg(2, COL_LEFTMOST, FONT1, sizeof(str_na) - 1, str_na);
		LIB_WaitTimeAndKey(3000);
	}
	else
	{
		LIB_LCD_PutMsg(2, COL_LEFTMOST, FONT1, sizeof(str_rid) - 1, str_rid);
		for(i = 0; i < MAX_PAYMENT_SCHEME_CNT; i++)
		{
			if(buf[i * 21] != 0xFF)
			{
				for(j = 0; j < RID_LEN; j++)
					LIB_DispHexByte(2, 2 * j + 4, buf[i * 21 + j]);

				LIB_DumpHexDataTO(0, 4, buf[i * 21 + 5], &buf[i * 21 + 6]);
			}
		}
	}

	// clear sensitive data
	memset(buf, 0x00, sizeof(buf));
    memset(temp, 0x00, sizeof(temp));
    memset(buf2, 0x00, sizeof(buf2));
    memset(keydata, 0x00, sizeof(keydata));
    memset(mac8, 0x00, sizeof(mac8));
    memset(mac16, 0x00, sizeof(mac16));
    memset(mkey, 0x00, sizeof(mkey));

	return(apiOK);
}

// ---------------------------------------------------------------------------
// FUNCTION: Show terminal serial number.
// INPUT   : none.
// OUTPUT  : tsn - terminal serial number.
// RETURN  : apiOK
//           apiFailed
//
//	         TERMINAL S/N
//	         nnnnnnnn
// ---------------------------------------------------------------------------
UINT8	PED_ShowTSN(UCHAR *tsn)
{
	UINT8	temp[32];


	memset(tsn, 0x00, sizeof(tsn));

	if(api_sys_info(SID_TerminalSerialNumber, temp) == apiOK)
	{
		memmove(tsn, &temp[4], 8);
	}

    // clear sensitive data
	memset(temp, 0x00, sizeof(temp));

	return apiOK;
}

// ---------------------------------------------------------------------------
// FUNCTION: Write DHCP ENABLE flag.
// INPUT   : flag
//		     TRUE:	enable
//		     FALSE:	disable
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
#ifdef	_DHCP_ENABLED_
UINT32	PED_SetStateDHCP(UINT16 flag)
{
	OS_SECM_PutData(ADDR_DHCP_FLAG, DHCP_FLAG_LEN, (UCHAR *)&flag);
	return apiOK;
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Write DHCP ENABLE flag.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE	-- DHCP enabled.
//           FALSE	-- DHCP disabled.
// ---------------------------------------------------------------------------
#ifdef	_DHCP_ENABLED_
UINT32	PED_GetStateDHCP(void)
{
    UINT16	flag;


	OS_SECM_GetData(ADDR_DHCP_FLAG, DHCP_FLAG_LEN, (UCHAR *)&flag);

	if((flag == 0xFFFF) || (flag == 0x0000))
	  return FALSE;
	else
	  return TRUE;
}
#endif
