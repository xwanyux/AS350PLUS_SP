//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : i.MX6UL							                        **
//**  PRODUCT  : AS350-X6	                                                **
//**                                                                        **
//**  FILE     : OS_PED2.C                                                  **
//**  MODULE   : 			   				                                **
//**                                                                        **
//**  FUNCTION : OS::PED2 (PCI PED Entry for API)			                **
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

#include "PEDKconfig.h"

//#include "bsp_types.h"
//#include "bsp_cipher.h"
#include "POSAPI.h"
#include "OS_MSG.h"
#include "OS_LIB.h"
#include "OS_PED.h"
#include "OS_SECM.h"
#include "OS_CAPK.h"
#include "OS_PED.h"

#include "DEV_KBD.h"
#include "DEV_PED.h"

#include "LCDTFTAPI.h"
#include "ANS_TR31_2010.h"
#include "ANS_TR31_2018.h"
#include "ANS_X9143_2022.h"

#include "EMVAPI.h"

extern	UINT8	post_dhn_buz1;
extern	UINT8	post_dhn_buz2;

extern	UINT32	os_ped_PanDownCnt;
extern	UINT32	os_ped_PanUpCnt;
extern  UINT32  os_ped_PanLife;

extern	void	PED_InUse( UINT32 status );

//#define	PED_IDLE_TOUT			10*60*100	// inactivity time (no in use, idle) = 10min
#define	PED_IDLE_TOUT			(24*60*60*100)-1	// inactivity time (no in use, idle) = 24hr
#define	PED_PIN_LIFE_TOUT		15*100;		// effective PIN life cycle = 15sec
#define	PED_PIN_INTERVAL_TOUT		30*100;		// min PIN entry interval = 30sec
#define	PED_SENSITIVE_SERVICE_TOUT	15*60*100	// max sensitive service time = 15min

UINT8	os_ped_state = PED_STATE_END;
UINT8	os_ped_dhn_tim = 0;
UINT16	os_ped_tout = 0;
UINT16	os_ped_tick1;
UINT16	os_ped_tick2;
UINT32	os_ped_DownCnt = 0;
UINT32	os_ped_UpCnt = 0;
UINT32	os_ped_PinLife = 0;
UINT32	os_ped_IdleCnt = PED_IDLE_TOUT;

UINT32	os_ped_SensitiveServiceTime = PED_SENSITIVE_SERVICE_TOUT;
UINT32	os_ped_KeyPress = FALSE;

#ifdef	_BOOT_FROM_POST0_
UINT32	os_ped_activity = TRUE;
#else
UINT32	os_ped_activity = FALSE;
#endif

UINT8   *ped_pin_ptr = NULL;


// ---------------------------------------------------------------------------
// FUNCTION: To setup one master key at the specified key slot.
// INPUT   : index  - key slot index. (default 0)
//	         length - length of master key data. (MUST be 80)
//	         key    - master key data. (EMK(80):in ANSI TR-31 key bundle format)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UINT8	PED_SetMasterKey(UINT8 index, UINT8 length, UINT8 *key)
{
    UINT8	temp[X9143_TDES_KEY_DATA_LEN + 1];  // ==== [Debug] ====
	UINT8	buf[X9143_TDES_KEY_BUNDLE_LEN]; // ==== [Debug] ====
	//UINT8	temp[KEY_DATA_LEN + 1];
	//UINT8	buf[KEY_BUNDLE_LEN];
	UINT8	pkey1[PED_MSKEY_LEN + 1];
	UINT8	pkey2[PED_MSKEY_LEN + 1];
	UINT8	skey[PED_MSKEY_LEN + 1];
	UINT8	mkey[PED_MSKEY_LEN + 1];
	UINT32	i;
	UINT32	flag;
	UINT8	MkeyIndex;
	UINT8	result;
	UINT8	mac8[8];


	result = apiFailed;

    printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);    // ==== [Debug] ====

	//if((index < MAX_MKEY_CNT) && (length == KEY_BUNDLE_LEN))
    if((index < MAX_MKEY_CNT) && (length == X9143_TDES_KEY_BUNDLE_LEN))
	{
		//if(TR31_VerifyKeyBundle(length, key, temp, mac8, (UINT8 *)0 ))
        if(X9143_VerifyKeyBundle_TDES(length, key, temp, mac8, (UINT8 *)0 ))
		{
			//if(TR31_DecryptKeyBundle(mac8, temp, pkey1, (UINT8 *)0 ))	// pkey1=MKEY
            if(X9143_DecryptKeyBundle_TDES(mac8, temp, pkey1, (UINT8 *)0 ))	// pkey1=MKEY
			{
				// enforce the key value different from KPK and ACC_DEK MSKEY
				flag = TRUE;

				OS_SECM_GetData(ADDR_PED_TDES_KEY_PROTECT_KEY, PED_TDES_KEY_PROTECT_KEY_LEN + 1, pkey2);	// pkey2=KPK

				for(i = 0; i < pkey1[0]; i++)
				{
					pkey1[i + 1] &= 0xFE; // ignore parity bit of input KEY
					pkey2[i + 1] &= 0xFE; // ignore parity bit of KPK
				}

                // ==== [Debug] ====
                printf("pkey1 = ");
                for(int i = 0 ; i < pkey1[0] + 1 ; i++)
                    printf("%02x", pkey1[i]);
                printf("\n");
                printf("pkey2 = ");
                for (int i = 0 ; i < pkey1[0] + 1 ; i++)
                    printf("%02x", pkey2[i]);
                printf("\n");
                // ==== [Debug] ====

                if(LIB_memcmp(pkey1, pkey2, pkey1[0] + 1) == 0)	// compared with KPK
					flag = FALSE;
				else
				{
					// get ACC_DEK MKEY bundle
					PED_AccDEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
					OS_SECM_GetData(ADDR_PED_ACC_DEK_MKEY_01 + (MkeyIndex * PED_ACC_DEK_MSKEY_SLOT_LEN), PED_ACC_DEK_MSKEY_SLOT_LEN, buf);

					// verify ACC_DEK MKEY bundle
					//if(TR31_VerifyKeyBundle(KEY_BUNDLE_LEN, buf, temp, mac8, (UINT8 *)0 ))
                    if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, buf, temp, mac8, (UINT8 *)0 ))
					{
						// retrieve ACC_DEK MKEY
						//if(TR31_DecryptKeyBundle(mac8, temp, pkey2, (UINT8 *)0 ))	// pkey2=ACC_DEK MKEY
                        if(X9143_DecryptKeyBundle_TDES(mac8, temp, pkey2, (UINT8 *)0 ))	// pkey2=ACC_DEK MKEY
						{
							memmove(mkey, pkey2, sizeof(mkey));

							for(i = 0 ; i < pkey1[0] ; i++)
								pkey2[i + 1] &= 0xFE;	// ignore parity bit of ACC_DEK MKEY

							if(LIB_memcmp(&pkey1[1], &pkey2[1], pkey1[0]) == 0)	// compared with ACC_DEK MKEY
								flag = FALSE;
							else
							{
								// get ACC_DEK SKEY bundle
								OS_SECM_GetData(ADDR_PED_ACC_DEK_SKEY_01, PED_ACC_DEK_MSKEY_SLOT_LEN, buf);

								// verify ACC_DEK SKEY bundle
								//if(TR31_VerifyKeyBundle(KEY_BUNDLE_LEN, buf, temp, mac8, mkey ))
                                if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, buf, temp, mac8, mkey ))
								{
									// retrieve ACC_DEK SKEY
									//if(TR31_DecryptKeyBundle(mac8, temp, pkey2, mkey ))	// pkey2=ACC_DEK SKEY
                                    if(X9143_DecryptKeyBundle_TDES(mac8, temp, pkey2, mkey ))	// pkey2=ACC_DEK SKEY
									{
										memmove( skey, &pkey2[1], pkey2[0] );
										
										// retrieve ACC_DEK SKEY from ACC_DEK MKEY
//										PED_TripleDES2(&mkey[1], pkey2[0], &pkey2[1], skey);

										for(i = 0 ; i < pkey2[0] ; i++)
											skey[i] &= 0xFE;	 // ignore parity bit of SKEY

										if(LIB_memcmp(&pkey1[1], skey, pkey1[0]) == 0)	// compared with ACC_DEK SKEY
											flag = FALSE;
										else
											flag = TRUE;
									}
								}
							}
						}
					}
				}

				// save it
				if(flag)
				{
					OS_SECM_PutData(ADDR_PED_MKEY_01 + (index * PED_MSKEY_SLOT_LEN), PED_MSKEY_SLOT_LEN, key);
					PED_WriteMKeyIndex(index);

					result = apiOK;
				}
			}
		}
	}

	// clear sensitive data
	memset(temp, 0x00, sizeof(temp));
	memset(buf, 0x00, sizeof(buf));
	memset(pkey1, 0x00, sizeof(pkey1));
	memset(pkey2, 0x00, sizeof(pkey2));
	memset(skey, 0x00, sizeof(skey));
	memset(mkey, 0x00, sizeof(mkey));
	
	return(result);
}

// ---------------------------------------------------------------------------
// FUNCTION: To setup one master key at the specified key slot.
// INPUT   : index  - key slot index. (default 0)
//	         length - length of master key data. (MUST be 96 or 144)
//	         key    - master key data. (EMK(96):in ANSI X9.143 TDES key bundle format)
//                                     (EMK(144):in ANSI X9.143 AES key bundle format)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UINT8	PED_PEK_SetMasterKey(UINT8 index, UINT8 length, UINT8 *key)
{
    UINT8	temp[X9143_TDES_KEY_DATA_LEN + 1];
	UINT8	buf[X9143_TDES_KEY_BUNDLE_LEN];
    UINT8	tdes_temp[X9143_TDES_KEY_DATA_LEN + 1];
	UINT8	tdes_buf[X9143_TDES_KEY_BUNDLE_LEN];
    UINT8	aes_temp[X9143_AES_KEY_DATA_LEN + 1];
	UINT8	aes_buf[X9143_AES_KEY_BUNDLE_LEN];
	UINT8	pkey1[PED_PEK_MSKEY_LEN + 1];
	UINT8	pkey2[PED_PEK_MSKEY_LEN + 1];
	UINT8	skey[PED_PEK_MSKEY_LEN + 1];
	UINT8	mkey[PED_PEK_MSKEY_LEN + 1];
	UINT32	i;
	UINT32	flag;
	UINT8	MkeyIndex;
	UINT8	result;
	UINT8	mac8[8];
    UINT8   mac16[16];
    UINT8   keyType;


	result = apiFailed;

    if((index < MAX_PEK_MKEY_CNT) && (length == X9143_TDES_KEY_BUNDLE_LEN))
	{
        printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);    // ==== [Debug] ====

        if(X9143_VerifyKeyBundle_TDES(length, key, tdes_temp, mac8, (UINT8 *)0 ))
		{
            if(X9143_DecryptKeyBundle_TDES(mac8, tdes_temp, pkey1, (UINT8 *)0 ))	// pkey1=PEK MKEY
			{
				// enforce the key value different from KPK and ACC_DEK MSKEY
				flag = TRUE;

                if(pkey1[0] == PED_128_TDES_KEY_PROTECT_KEY_LEN)
				    OS_SECM_GetData(ADDR_PED_128_TDES_KEY_PROTECT_KEY, PED_128_TDES_KEY_PROTECT_KEY_LEN + 1, pkey2);	// pkey2=KPK
                else if(pkey1[0] == PED_192_TDES_KEY_PROTECT_KEY_LEN)
				    OS_SECM_GetData(ADDR_PED_192_TDES_KEY_PROTECT_KEY, PED_192_TDES_KEY_PROTECT_KEY_LEN + 1, pkey2);	// pkey2=KPK

				for(i = 0; i < pkey1[0]; i++)
				{
					pkey1[i + 1] &= 0xFE; // ignore parity bit of input KEY
					pkey2[i + 1] &= 0xFE; // ignore parity bit of KPK
				}

                // ==== [Debug] ====
                printf("pkey1 = ");
                for(int i = 0 ; i < pkey1[0] + 1 ; i++)
                    printf("%02x", pkey1[i]);
                printf("\n");
                printf("pkey2 = ");
                for (int i = 0 ; i < pkey1[0] + 1 ; i++)
                    printf("%02x", pkey2[i]);
                printf("\n");
                // ==== [Debug] ====

                if(LIB_memcmp(pkey1, pkey2, pkey1[0] + 1) == 0)	// compared with KPK
					flag = FALSE;
				else
				{
                    PED_AccDEK_GetKeyType(&keyType);

                    // enforce the key value different from ACC_DEK MSKEY
                    if((pkey1[0] == PED_192_TDES_KEY_PROTECT_KEY_LEN) && (keyType == TDES_192)) // ACC_DEK MSKEY is TDES-192 key
                    {
                        // get ACC_DEK MKEY bundle
					    PED_AccDEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
					    OS_SECM_GetData(ADDR_PED_ACC_DEK_TDES_MKEY_01 + (MkeyIndex * PED_ACC_DEK_TDES_MSKEY_SLOT_LEN), PED_ACC_DEK_TDES_MSKEY_SLOT_LEN, tdes_buf);

					    // verify ACC_DEK MKEY bundle
                        if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, tdes_buf, tdes_temp, mac8, (UINT8 *)0 ))
					    {
					    	// retrieve ACC_DEK MKEY
                            if(X9143_DecryptKeyBundle_TDES(mac8, tdes_temp, pkey2, (UINT8 *)0 ))	// pkey2=ACC_DEK MKEY
					    	{
					    		memmove(mkey, pkey2, sizeof(mkey));

					    		for(i = 0 ; i < pkey1[0] ; i++)
					    			pkey2[i + 1] &= 0xFE;	// ignore parity bit of ACC_DEK MKEY

					    		if(LIB_memcmp(&pkey1[1], &pkey2[1], pkey1[0]) == 0)	// compared with ACC_DEK MKEY
					    			flag = FALSE;
					    		else
					    		{
					    			// get ACC_DEK SKEY bundle
					    			OS_SECM_GetData(ADDR_PED_ACC_DEK_TDES_SKEY_01, PED_ACC_DEK_TDES_MSKEY_SLOT_LEN, tdes_buf);

					    			// verify ACC_DEK SKEY bundle
                                    if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, tdes_buf, tdes_temp, mac8, mkey))
					    			{
					    				// retrieve ACC_DEK SKEY
                                        if(X9143_DecryptKeyBundle_TDES(mac8, tdes_temp, pkey2, mkey))	// pkey2=ACC_DEK SKEY
					    				{
					    					memmove(skey, &pkey2[1], pkey2[0]);

					    					for(i = 0 ; i < pkey2[0] ; i++)
					    						skey[i] &= 0xFE;	 // ignore parity bit of ACC_DEK SKEY

					    					if(LIB_memcmp(&pkey1[1], skey, pkey1[0]) == 0)	// compared with ACC_DEK SKEY
					    						flag = FALSE;
					    					else
					    						flag = TRUE;
					    				}
					    			}
					    		}
					    	}
					    }
                    }
				}

				// save it
				if(flag)
				{
					OS_SECM_PutData(ADDR_PED_PEK_TDES_MKEY_01 + (index * PED_PEK_TDES_MSKEY_SLOT_LEN), PED_PEK_TDES_MSKEY_SLOT_LEN, key);
					PED_PEK_WriteMKeyIndex(index);

					result = apiOK;
				}
			}
		}
	}
    else if((index < MAX_PEK_MKEY_CNT) && (length == X9143_AES_KEY_BUNDLE_LEN))
    {
        printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);    // ==== [Debug] ====

        if(X9143_VerifyKeyBundle_AES(length, key, aes_temp, mac16, (UINT8 *)0 ))
		{
            if(X9143_DecryptKeyBundle_AES(mac16, aes_temp, pkey1, (UINT8 *)0 ))	// pkey1=PEK MKEY
			{
				// enforce the key value different from KPK, ACC_DEK MSKEY, and FPE MSKEY
				flag = TRUE;

                if(pkey1[0] == PED_128_AES_KEY_PROTECT_KEY_LEN)
				    OS_SECM_GetData(ADDR_PED_128_AES_KEY_PROTECT_KEY, PED_128_AES_KEY_PROTECT_KEY_LEN + 1, pkey2);	// pkey2=KPK
                else if(pkey1[0] == PED_192_AES_KEY_PROTECT_KEY_LEN)
				    OS_SECM_GetData(ADDR_PED_192_AES_KEY_PROTECT_KEY, PED_192_AES_KEY_PROTECT_KEY_LEN + 1, pkey2);	// pkey2=KPK
                else if(pkey1[0] == PED_256_AES_KEY_PROTECT_KEY_LEN)
				    OS_SECM_GetData(ADDR_PED_256_AES_KEY_PROTECT_KEY, PED_256_AES_KEY_PROTECT_KEY_LEN + 1, pkey2);	// pkey2=KPK


				for(i = 0; i < pkey1[0]; i++)
				{
					pkey1[i + 1] &= 0xFE; // ignore parity bit of input KEY
					pkey2[i + 1] &= 0xFE; // ignore parity bit of KPK
				}

                // ==== [Debug] ====
                printf("pkey1 = ");
                for(int i = 0 ; i < pkey1[0] + 1 ; i++)
                    printf("%02x", pkey1[i]);
                printf("\n");
                printf("pkey2 = ");
                for (int i = 0 ; i < pkey1[0] + 1 ; i++)
                    printf("%02x", pkey2[i]);
                printf("\n");
                // ==== [Debug] ====

                if(LIB_memcmp(pkey1, pkey2, pkey1[0] + 1) == 0)	// compared with KPK
					flag = FALSE;
				else
				{
                    PED_AccDEK_GetKeyType(&keyType);

                    // enforce the key value different from ACC_DEK MSKEY
                    if(((pkey1[0] == PED_128_AES_KEY_PROTECT_KEY_LEN) && (keyType == AES_128)) ||   // ACC_DEK MSKEY is AES-128 key
                       ((pkey1[0] == PED_192_AES_KEY_PROTECT_KEY_LEN) && (keyType == AES_192)) ||   // ACC_DEK MSKEY is AES-192 key
                       ((pkey1[0] == PED_256_AES_KEY_PROTECT_KEY_LEN) && (keyType == AES_256)))     // ACC_DEK MSKEY is AES-256 key
                    {
                        // get ACC_DEK MKEY bundle
					    PED_AccDEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
					    OS_SECM_GetData(ADDR_PED_ACC_DEK_AES_MKEY_01 + (MkeyIndex * PED_ACC_DEK_AES_MSKEY_SLOT_LEN), PED_ACC_DEK_AES_MSKEY_SLOT_LEN, aes_buf);

					    // verify ACC_DEK MKEY bundle
                        if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, aes_buf, aes_temp, mac16, (UINT8 *)0 ))
					    {
					    	// retrieve ACC_DEK MKEY
                            if(X9143_DecryptKeyBundle_AES(mac16, aes_temp, pkey2, (UINT8 *)0 ))	// pkey2=ACC_DEK MKEY
					    	{
					    		memmove(mkey, pkey2, sizeof(mkey));

					    		for(i = 0 ; i < pkey1[0] ; i++)
					    			pkey2[i + 1] &= 0xFE;	// ignore parity bit of ACC_DEK MKEY

					    		if(LIB_memcmp(&pkey1[1], &pkey2[1], pkey1[0]) == 0)	// compared with ACC_DEK MKEY
					    			flag = FALSE;
					    		else
					    		{
					    			// get ACC_DEK SKEY bundle
					    			OS_SECM_GetData(ADDR_PED_ACC_DEK_AES_SKEY_01, PED_ACC_DEK_AES_MSKEY_SLOT_LEN, aes_buf);

					    			// verify ACC_DEK SKEY bundle
                                    if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, aes_buf, aes_temp, mac16, mkey))
					    			{
					    				// retrieve ACC_DEK SKEY
                                        if(X9143_DecryptKeyBundle_AES(mac16, aes_temp, pkey2, mkey))	// pkey2=ACC_DEK SKEY
					    				{
					    					memmove(skey, &pkey2[1], pkey2[0]);

					    					for(i = 0 ; i < pkey2[0] ; i++)
					    						skey[i] &= 0xFE;	 // ignore parity bit of SKEY

					    					if(LIB_memcmp(&pkey1[1], skey, pkey1[0]) == 0)	// compared with ACC_DEK SKEY
					    						flag = FALSE;
					    					else
					    						flag = TRUE;
					    				}
					    			}
					    		}
					    	}
					    }
                    }

                    if(flag)
                    {
                        PED_FPE_GetKeyType(&keyType);

                        // enforce the key value different from FPE MSKEY
                        if((pkey1[0] == PED_128_AES_KEY_PROTECT_KEY_LEN) && (keyType == AES_128)) // FPE MSKEY is AES-128 key
                        {
                            // get FPE MKEY bundle
                            PED_FPE_ReadMKeyIndex((UINT8 *)&MkeyIndex);
                            OS_SECM_GetData(ADDR_PED_FPE_KEY_AES_MKEY_01 + (MkeyIndex * PED_FPE_KEY_AES_MSKEY_SLOT_LEN), PED_FPE_KEY_AES_MSKEY_SLOT_LEN, aes_buf);

                            // verify FPE MKEY bundle
				            if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, aes_buf, aes_temp, mac16, (UINT8 *)0 ))
				            {
				            	// retrieve FPE MKEY
				            	if(X9143_DecryptKeyBundle_AES(mac16, aes_temp, pkey2, (UINT8 *)0 ))	// pkey2=FPE MKEY
				            	{
				            		memmove(mkey, pkey2, sizeof(mkey));

				            		for(i = 0 ; i < pkey1[0] ; i++)
				            			pkey2[i + 1] &= 0xFE;	// ignore parity bit of FPE MKEY

				            		if(LIB_memcmp(&pkey1[1], &pkey2[1], pkey1[0]) == 0)	// compared with FPE MKEY
				            			flag = FALSE;
				            		else
				            		{
				            			// get FPE SKEY bundle
				            			OS_SECM_GetData(ADDR_PED_FPE_KEY_AES_SKEY_01, PED_FPE_KEY_AES_MSKEY_SLOT_LEN, aes_buf);

				            			// verify FPE SKEY bundle
				            			if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, aes_buf, aes_temp, mac16, mkey))
				            			{
				            				// retrieve FPE SKEY
				            				if(X9143_DecryptKeyBundle_AES(mac16, aes_temp, pkey2, mkey))	// pkey2=FPE SKEY
				            				{
                                                memmove(skey, &pkey2[1], pkey2[0]);
                                                
				            					for(i = 0 ; i < pkey2[0] ; i++)
				            						skey[i] &= 0xFE;	 // ignore parity bit of FPE SKEY

				            					if(LIB_memcmp(&pkey1[1], skey, pkey1[0]) == 0)	// compared with FPE SKEY
				            						flag = FALSE;
				            					else
				            						flag = TRUE;
				            				}
				            			}
				            		}
				            	}
				            }
                        }
                    }
				}

				// save it
				if(flag)
				{
					OS_SECM_PutData(ADDR_PED_PEK_AES_MKEY_01 + (index * PED_PEK_AES_MSKEY_SLOT_LEN), PED_PEK_AES_MSKEY_SLOT_LEN, key);
					PED_WriteMKeyIndex(index);

					result = apiOK;
				}
			}
		}
    }

	// clear sensitive data
	memset(temp, 0x00, sizeof(temp));
	memset(buf, 0x00, sizeof(buf));
    memset(tdes_temp, 0x00, sizeof(tdes_temp));
	memset(tdes_buf, 0x00, sizeof(tdes_buf));
    memset(aes_temp, 0x00, sizeof(aes_temp));
	memset(aes_buf, 0x00, sizeof(aes_buf));
	memset(pkey1, 0x00, sizeof(pkey1));
	memset(pkey2, 0x00, sizeof(pkey2));
	memset(skey, 0x00, sizeof(skey));
	memset(mkey, 0x00, sizeof(mkey));
	
	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: To select one master key from the specified key slot for the
//	         succeeding operation.
// INPUT   : index  - key slot index.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UINT8	PED_SelectMasterKey( UINT8 index )
{
	if( index >= MAX_MKEY_CNT )
	  return( apiFailed );
	
	PED_WriteMKeyIndex( index );
	
	return( apiOK );
}

// ---------------------------------------------------------------------------
// FUNCTION: To setup one session key at the specified key slot.
// INPUT   : index  - key slot index.
//	         length - length of session key data. (MUST be 80)
//	         key    - session key data, encrypted by the selected master key.
//		              (ESK(80):in ANSI TR-31 key bundle format)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UINT8	PED_SetSessionKey(UINT8 index, UINT8 length, UINT8 *key)
{
    UINT8	temp[X9143_TDES_KEY_DATA_LEN + 1];  // ==== [Debug] ====
	UINT8	buf[X9143_TDES_KEY_BUNDLE_LEN]; // ==== [Debug] ====
	//UINT8	temp[KEY_DATA_LEN + 1];
	//UINT8	buf[KEY_BUNDLE_LEN];
	UINT8	pkey1[PED_MSKEY_LEN + 1];
	UINT8	pkey2[PED_MSKEY_LEN + 1];
	UINT8	skey[PED_MSKEY_LEN + 1];
	UINT8	mkey[PED_MSKEY_LEN + 1];
	UINT32	i, j;
	UINT32	flag;
	UINT8	MkeyIndex;
	UINT8	result;
	UINT8	mac8[8];

	
    printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);    // ==== [Debug] ====

	// get MKEY bundle
	PED_ReadMKeyIndex((UINT8 *)&MkeyIndex);
	OS_SECM_GetData(ADDR_PED_MKEY_01 + (MkeyIndex * PED_MSKEY_SLOT_LEN), PED_MSKEY_SLOT_LEN, buf);
	
	// verify MKEY bundle
	//if( TR31_VerifyKeyBundle(KEY_BUNDLE_LEN, buf, temp, mac8, (UINT8 *)0 ) )
    if( X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, buf, temp, mac8, (UINT8 *)0 ) )
	  {
	  // retrieve MKEY
	  //if( TR31_DecryptKeyBundle(mac8, temp, pkey2, (UINT8 *)0 ) )	// pkey2=MKEY (as the KBPK for SKEY)
      if( X9143_DecryptKeyBundle_TDES(mac8, temp, pkey2, (UINT8 *)0 ) )	// pkey2=MKEY (as the KBPK for SKEY)
	    memmove(mkey, pkey2, sizeof(mkey));
	  else
	    return( apiFailed );
	  }


	result = apiFailed;
	
	//if((index < MAX_SKEY_CNT) && (length == KEY_BUNDLE_LEN))
    if((index < MAX_SKEY_CNT) && (length == X9143_TDES_KEY_BUNDLE_LEN))
	{
		//if( TR31_VerifyKeyBundle(length, key, temp, mac8, mkey) )
        if( X9143_VerifyKeyBundle_TDES(length, key, temp, mac8, mkey) )
		{
			//if(TR31_DecryptKeyBundle(mac8, temp, pkey1, mkey))	// pkey1=SKEY
            if(X9143_DecryptKeyBundle_TDES(mac8, temp, pkey1, mkey))	// pkey1=SKEY
			{
				memmove( skey, &pkey1[1], pkey1[0] );

                // ==== [Debug] ====
                printf("pkey1 = ");
                for(int k = 0 ; k < pkey1[0] + 1 ; k++)
                    printf("%02x", pkey1[k]);
                printf("\n");
                // ==== [Debug] ====
				
				// enforce the key value different from MKEY and KPK
				flag = FALSE;

				// get MKEY bundle
//				PED_ReadMKeyIndex((UINT8 *)&MkeyIndex);
//				OS_SECM_GetData(ADDR_PED_MKEY_01 + (MkeyIndex * PED_MSKEY_SLOT_LEN), PED_MSKEY_SLOT_LEN, buf);

				// verify MKEY bundle
//				if(TR31_VerifyKeyBundle(KEY_BUNDLE_LEN, buf, temp, mac8))
//				{
					// retrieve MKEY
//					if(TR31_DecryptKeyBundle(mac8, temp, pkey2))	// pkey2=MKEY
//					{
//						memmove(mkey, pkey2, sizeof(mkey));

						// retrieve SKEY from MKEY
//						PED_TripleDES2(&pkey2[1], pkey1[0], &pkey1[1], skey);

						for(i = 0; i < pkey2[0]; i++)
						{
							skey[i] &= 0xFE;	 // ignore parity bit of SKEY
							pkey2[i + 1] &= 0xFE; // ignore parity bit of MKEY
						}

						if(LIB_memcmp(skey, &pkey2[1], pkey2[0]) == 0)  // compared with MKEY
							flag = FALSE;
						else
						{
							OS_SECM_GetData(ADDR_PED_TDES_KEY_PROTECT_KEY, PED_TDES_KEY_PROTECT_KEY_LEN + 1, pkey2);

							for(i = 0; i < pkey2[0]; i++)
								pkey2[i + 1] &= 0xFE; // ignore parity bit of KPK

							if(LIB_memcmp(skey, &pkey2[1], pkey2[0]) == 0) // compared with KPK
								flag = FALSE;
							else
							{
								flag = TRUE;
							}
						}
//					}
//				}

				if(flag)
				{
					// enforce the key value different from other session keys	      
					for(i = 0; i < MAX_SKEY_CNT; i++)
					{
						if(i != index)
						{
							OS_SECM_GetData(ADDR_PED_SKEY_01 + (i * PED_MSKEY_SLOT_LEN), PED_MSKEY_SLOT_LEN, buf);
							//if(TR31_VerifyKeyBundle(KEY_BUNDLE_LEN, buf, temp, mac8, mkey))
                            if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, buf, temp, mac8, mkey))
							{
								//if(TR31_DecryptKeyBundle(mac8, temp, pkey1, mkey))	// pkey1=SKEY
                                if(X9143_DecryptKeyBundle_TDES(mac8, temp, pkey1, mkey))	// pkey1=SKEY
								{
									memmove( pkey2, &pkey1[1], pkey1[0] );
									
									// retrieve other SKEY from MKEY
//									PED_TripleDES2(&mkey[1], pkey1[0], &pkey1[1], pkey2); // pkey2=other SKEY

									for(j = 0; j < pkey1[0]; j++)
										pkey2[j] &= 0xFE; // ignore parity bit of session key

									if(LIB_memcmp(skey, pkey2, pkey1[0]) == 0) // compared with other session key
									{
										flag = FALSE;
										break;
									}
								}
							}
						}
					} // for(i)
				}
			}

			if(flag)
			{
				// enforce the key value different from ACC_DEK MSKEY

				// get ACC_DEK MKEY bundle
				PED_AccDEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
				OS_SECM_GetData(ADDR_PED_ACC_DEK_MKEY_01 + (MkeyIndex * PED_ACC_DEK_MSKEY_SLOT_LEN), PED_ACC_DEK_MSKEY_SLOT_LEN, buf);

				// verify ACC_DEK MKEY bundle
				//if(TR31_VerifyKeyBundle(KEY_BUNDLE_LEN, buf, temp, mac8, (UINT8 *)0 ))
                if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, buf, temp, mac8, (UINT8 *)0 ))
				{
					// retrieve ACC_DEK MKEY
					//if(TR31_DecryptKeyBundle(mac8, temp, pkey2, (UINT8 *)0 ))	// pkey2=ACC_DEK MKEY (as the KBPK for ACC_DEK SKEY)
                    if(X9143_DecryptKeyBundle_TDES(mac8, temp, pkey2, (UINT8 *)0 ))	// pkey2=ACC_DEK MKEY (as the KBPK for ACC_DEK SKEY)
					{
						memmove(mkey, pkey2, sizeof(mkey));

						for(i = 0 ; i < pkey2[0] ; i++)
							pkey2[i + 1] &= 0xFE;	// ignore parity bit of ACC_DEK MKEY

						if(LIB_memcmp(skey, &pkey2[1], pkey2[0]) == 0)	// compared with ACC_DEK MKEY
							flag = FALSE;
						else
						{
							// get ACC_DEK SKEY bundle
							OS_SECM_GetData(ADDR_PED_ACC_DEK_SKEY_01, PED_ACC_DEK_MSKEY_SLOT_LEN, buf);

							// verify ACC_DEK SKEY bundle
							//if(TR31_VerifyKeyBundle(KEY_BUNDLE_LEN, buf, temp, mac8, mkey))
                            if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, buf, temp, mac8, mkey))
							{
								// retrieve ACC_DEK SKEY
								//if(TR31_DecryptKeyBundle(mac8, temp, pkey2, mkey))	// pkey2=ACC_DEK SKEY
                                if(X9143_DecryptKeyBundle_TDES(mac8, temp, pkey2, mkey))	// pkey2=ACC_DEK SKEY
								{
									memmove( pkey1, &pkey2[1], pkey2[0] );
									
									// retrieve ACC_DEK SKEY from ACC_DEK MKEY
//									PED_TripleDES2(&mkey[1], pkey2[0], &pkey2[1], pkey1);

									for(i = 0 ; i < pkey2[0] ; i++)
										pkey1[i] &= 0xFE;	 // ignore parity bit of ACC_DEK SKEY

									if(LIB_memcmp(skey, pkey1, pkey2[0]) == 0)	// compared with ACC_DEK SKEY
										flag = FALSE;
									else
										flag = TRUE;
								}
							}
						}
					}
				}
			}

			// save it
			if(flag)
			{
				OS_SECM_PutData(ADDR_PED_SKEY_01 + (index * PED_MSKEY_SLOT_LEN), PED_MSKEY_SLOT_LEN, key);
				result = apiOK;
			}
		}
	}

	// clear sensitive data
	memset(temp, 0x00, sizeof(temp));
	memset(buf, 0x00, sizeof(buf));
	memset(pkey1, 0x00, sizeof(pkey1));
	memset(pkey2, 0x00, sizeof(pkey2));
	memset(skey, 0x00, sizeof(skey));
	memset(mkey, 0x00, sizeof(mkey));
	
	return(result);
}

// ---------------------------------------------------------------------------
// FUNCTION: To setup one session key at the specified key slot.
// INPUT   : index  - key slot index.
//	         length - length of session key data. (MUST be 96 or 144)
//	         key    - session key data, encrypted by the selected master key.
//		              (ESK(96):in ANSI X9.143 TDES key bundle format)
//		              (ESK(144):in ANSI X9.143 AES key bundle format)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UINT8	PED_PEK_SetSessionKey(UINT8 index, UINT8 length, UINT8 *key)
{
    UINT8	temp[X9143_TDES_KEY_DATA_LEN + 1];
	UINT8	buf[X9143_TDES_KEY_BUNDLE_LEN];
	UINT8	tdes_temp[X9143_TDES_KEY_DATA_LEN + 1];
	UINT8	tdes_buf[X9143_TDES_KEY_BUNDLE_LEN];
    UINT8	aes_temp[X9143_AES_KEY_DATA_LEN + 1];
	UINT8	aes_buf[X9143_AES_KEY_BUNDLE_LEN];
	UINT8	pkey1[PED_PEK_MSKEY_LEN + 1];
	UINT8	pkey2[PED_PEK_MSKEY_LEN + 1];
	UINT8	skey[PED_PEK_MSKEY_LEN + 1];
	UINT8	mkey[PED_PEK_MSKEY_LEN + 1];
	UINT32	i, j;
	UINT32	flag;
	UINT8	MkeyIndex;
	UINT8	result;
	UINT8	mac8[8];
    UINT8   mac16[16];
    UINT8   keyType;


    result = apiFailed;
	
    if((index < MAX_PEK_SKEY_CNT) && (length == X9143_TDES_KEY_BUNDLE_LEN))
	{
        printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);    // ==== [Debug] ====

        // get PEK MKEY bundle
        PED_PEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
        OS_SECM_GetData(ADDR_PED_PEK_TDES_MKEY_01 + (MkeyIndex * PED_PEK_TDES_MSKEY_SLOT_LEN), PED_PEK_TDES_MSKEY_SLOT_LEN, tdes_buf);
        
        // verify PEK MKEY bundle
        if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, tdes_buf, tdes_temp, mac8, (UINT8 *)0))
        {
            // retrieve PEK MKEY
            if(X9143_DecryptKeyBundle_TDES(mac8, tdes_temp, pkey2, (UINT8 *)0)) // pkey2=PEK MKEY (as the KBPK for PEK SKEY)
                memmove(mkey, pkey2, sizeof(mkey));
            else
                return apiFailed;
        }

        if(X9143_VerifyKeyBundle_TDES(length, key, tdes_temp, mac8, mkey))
		{
            if(X9143_DecryptKeyBundle_TDES(mac8, tdes_temp, pkey1, mkey))	// pkey1=PEK SKEY
			{
				memmove(skey, &pkey1[1], pkey1[0]);

                // ==== [Debug] ====
                printf("pkey1 = ");
                for(int k = 0 ; k < pkey1[0] + 1 ; k++)
                    printf("%02x", pkey1[k]);
                printf("\n");
                // ==== [Debug] ====
				
				// enforce the key value different from PEK MKEY, KPK, and ACC_DEK MSKEY
				flag = FALSE;

				for(i = 0 ; i < pkey2[0] ; i++)
				{
					skey[i] &= 0xFE;	 // ignore parity bit of PEK SKEY
					pkey2[i + 1] &= 0xFE; // ignore parity bit of PEK MKEY
				}

				if(LIB_memcmp(skey, &pkey2[1], pkey2[0]) == 0)  // compared with PEK MKEY
					flag = FALSE;
				else
				{
                    if(pkey1[0] == PED_128_TDES_KEY_PROTECT_KEY_LEN)
                        OS_SECM_GetData(ADDR_PED_128_TDES_KEY_PROTECT_KEY, PED_128_TDES_KEY_PROTECT_KEY_LEN + 1, pkey2);	// pkey2=KPK
                    else if(pkey1[0] == PED_192_TDES_KEY_PROTECT_KEY_LEN)
                        OS_SECM_GetData(ADDR_PED_192_TDES_KEY_PROTECT_KEY, PED_192_TDES_KEY_PROTECT_KEY_LEN + 1, pkey2);	// pkey2=KPK

					for(i = 0 ; i < pkey2[0] ; i++)
						pkey2[i + 1] &= 0xFE; // ignore parity bit of KPK

					if(LIB_memcmp(skey, &pkey2[1], pkey2[0]) == 0) // compared with KPK
						flag = FALSE;
					else
					{
						flag = TRUE;
					}
				}

				if(flag)
				{
					// enforce the key value different from other session keys	      
					for(i = 0 ; i < MAX_PEK_SKEY_CNT ; i++)
					{
						if(i != index)
						{
							OS_SECM_GetData(ADDR_PED_PEK_TDES_SKEY_01 + (i * PED_PEK_TDES_MSKEY_SLOT_LEN), PED_PEK_TDES_MSKEY_SLOT_LEN, tdes_buf);
                            if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, tdes_buf, tdes_temp, mac8, mkey))
							{
                                if(X9143_DecryptKeyBundle_TDES(mac8, tdes_temp, pkey1, mkey))	// pkey1=PEK SKEY
								{
									memmove(pkey2, &pkey1[1], pkey1[0]);

									for(j = 0 ; j < pkey1[0] ; j++)
										pkey2[j] &= 0xFE; // ignore parity bit of session key

									if(LIB_memcmp(skey, pkey2, pkey1[0]) == 0) // compared with other session key
									{
										flag = FALSE;
										break;
									}
								}
							}
						}
					} // for(i)

                    if(flag)
			        {
                        PED_AccDEK_GetKeyType(&keyType);

                        // enforce the key value different from ACC_DEK MSKEY
                        if((pkey1[0] == PED_192_TDES_KEY_PROTECT_KEY_LEN) && (keyType == TDES_192))   // ACC_DEK MSKEY is TDES-192 key
                        {
                            // get ACC_DEK MKEY bundle
			        	    PED_AccDEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
			        	    OS_SECM_GetData(ADDR_PED_ACC_DEK_TDES_MKEY_01 + (MkeyIndex * PED_ACC_DEK_TDES_MSKEY_SLOT_LEN), PED_ACC_DEK_TDES_MSKEY_SLOT_LEN, tdes_buf);
            
			        	    // verify ACC_DEK MKEY bundle
                            if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, tdes_buf, tdes_temp, mac8, (UINT8 *)0 ))
			        	    {
			        	    	// retrieve ACC_DEK MKEY
                                if(X9143_DecryptKeyBundle_TDES(mac8, tdes_temp, pkey2, (UINT8 *)0 ))	// pkey2=ACC_DEK MKEY (as the KBPK for ACC_DEK SKEY)
			        	    	{
			        	    		memmove(mkey, pkey2, sizeof(mkey));
            
			        	    		for(i = 0 ; i < pkey2[0] ; i++)
			        	    			pkey2[i + 1] &= 0xFE;	// ignore parity bit of ACC_DEK MKEY
            
			        	    		if(LIB_memcmp(skey, &pkey2[1], pkey2[0]) == 0)	// compared with ACC_DEK MKEY
			        	    			flag = FALSE;
			        	    		else
			        	    		{
			        	    			// get ACC_DEK SKEY bundle
			        	    			OS_SECM_GetData(ADDR_PED_ACC_DEK_TDES_SKEY_01, PED_ACC_DEK_TDES_MSKEY_SLOT_LEN, tdes_buf);
            
			        	    			// verify ACC_DEK SKEY bundle
                                        if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, tdes_buf, tdes_temp, mac8, mkey))
			        	    			{
			        	    				// retrieve ACC_DEK SKEY
                                            if(X9143_DecryptKeyBundle_TDES(mac8, tdes_temp, pkey2, mkey))	// pkey2=ACC_DEK SKEY
			        	    				{
			        	    					memmove(pkey1, &pkey2[1], pkey2[0]);
            
			        	    					for(i = 0 ; i < pkey2[0] ; i++)
			        	    						pkey1[i] &= 0xFE;	 // ignore parity bit of ACC_DEK SKEY
            
			        	    					if(LIB_memcmp(skey, pkey1, pkey2[0]) == 0)	// compared with ACC_DEK SKEY
			        	    						flag = FALSE;
			        	    					else
			        	    						flag = TRUE;
			        	    				}
			        	    			}
			        	    		}
			        	    	}
			        	    }
                        }
			        }
				}
			}

			// save it
			if(flag)
			{
				OS_SECM_PutData(ADDR_PED_PEK_TDES_SKEY_01 + (index * PED_PEK_TDES_MSKEY_SLOT_LEN), PED_PEK_TDES_MSKEY_SLOT_LEN, key);
				result = apiOK;
			}
		}
	}
    else if((index < MAX_PEK_SKEY_CNT) && (length == X9143_AES_KEY_BUNDLE_LEN))
    {
        printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);    // ==== [Debug] ====

        // get PEK MKEY bundle
        PED_PEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
        OS_SECM_GetData(ADDR_PED_PEK_AES_MKEY_01 + (MkeyIndex * PED_PEK_AES_MSKEY_SLOT_LEN), PED_PEK_AES_MSKEY_SLOT_LEN, aes_buf);
        
        // verify PEK MKEY bundle
        if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, aes_buf, aes_temp, mac16, (UINT8 *)0))
        {
            // retrieve PEK MKEY
            if(X9143_DecryptKeyBundle_AES(mac16, aes_temp, pkey2, (UINT8 *)0)) // pkey2=PEK MKEY (as the KBPK for SKEY)
                memmove(mkey, pkey2, sizeof(mkey));
            else
                return apiFailed;
        }

        if(X9143_VerifyKeyBundle_AES(length, key, aes_temp, mac16, mkey))
		{
            if(X9143_DecryptKeyBundle_AES(mac16, aes_temp, pkey1, mkey))	// pkey1=PEK SKEY
			{
				memmove(skey, &pkey1[1], pkey1[0]);

                // ==== [Debug] ====
                printf("pkey1 = ");
                for(int k = 0 ; k < pkey1[0] + 1 ; k++)
                    printf("%02x", pkey1[k]);
                printf("\n");
                // ==== [Debug] ====
				
				// enforce the key value different from PEK MKEY, KPK, ACC_DEK MSKEY, and FPE MSKEY
				flag = FALSE;

				for(i = 0 ; i < pkey2[0] ; i++)
				{
					skey[i] &= 0xFE;	 // ignore parity bit of PEK SKEY
					pkey2[i + 1] &= 0xFE; // ignore parity bit of PEK MKEY
				}

				if(LIB_memcmp(skey, &pkey2[1], pkey2[0]) == 0)  // compared with PEK MKEY
					flag = FALSE;
				else
				{
                    if(pkey1[0] == PED_128_AES_KEY_PROTECT_KEY_LEN)
                        OS_SECM_GetData(ADDR_PED_128_AES_KEY_PROTECT_KEY, PED_128_AES_KEY_PROTECT_KEY_LEN + 1, pkey2);	// pkey2=KPK
                    else if(pkey1[0] == PED_192_AES_KEY_PROTECT_KEY_LEN)
                        OS_SECM_GetData(ADDR_PED_192_AES_KEY_PROTECT_KEY, PED_192_AES_KEY_PROTECT_KEY_LEN + 1, pkey2);	// pkey2=KPK
                    else if(pkey1[0] == PED_256_AES_KEY_PROTECT_KEY_LEN)
                        OS_SECM_GetData(ADDR_PED_256_AES_KEY_PROTECT_KEY, PED_256_AES_KEY_PROTECT_KEY_LEN + 1, pkey2);	// pkey2=KPK

					for(i = 0; i < pkey2[0]; i++)
						pkey2[i + 1] &= 0xFE; // ignore parity bit of KPK

					if(LIB_memcmp(skey, &pkey2[1], pkey2[0]) == 0) // compared with KPK
						flag = FALSE;
					else
					{
						flag = TRUE;
					}
				}

				if(flag)
				{
					// enforce the key value different from other session keys	      
					for(i = 0 ; i < MAX_PEK_SKEY_CNT ; i++)
					{
						if(i != index)
						{
							OS_SECM_GetData(ADDR_PED_PEK_AES_SKEY_01 + (i * PED_PEK_AES_MSKEY_SLOT_LEN), PED_PEK_AES_MSKEY_SLOT_LEN, aes_buf);
                            if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, aes_buf, aes_temp, mac16, mkey))
							{
                                if(X9143_DecryptKeyBundle_AES(mac16, aes_temp, pkey1, mkey))	// pkey1=PEK SKEY
								{
									memmove(pkey2, &pkey1[1], pkey1[0]);

									for(j = 0; j < pkey1[0]; j++)
										pkey2[j] &= 0xFE; // ignore parity bit of session key

									if(LIB_memcmp(skey, pkey2, pkey1[0]) == 0) // compared with other session key
									{
										flag = FALSE;
										break;
									}
								}
							}
						}
					} // for(i)

                    if(flag)
			        {
                        PED_AccDEK_GetKeyType(&keyType);

                        // enforce the key value different from ACC_DEK MSKEY
                        if(((pkey1[0] == PED_128_AES_KEY_PROTECT_KEY_LEN) && (keyType == AES_128)) ||   // ACC_DEK MSKEY is AES-128 key
                           ((pkey1[0] == PED_192_AES_KEY_PROTECT_KEY_LEN) && (keyType == AES_192)) ||   // ACC_DEK MSKEY is AES-192 key
                           ((pkey1[0] == PED_256_AES_KEY_PROTECT_KEY_LEN) && (keyType == AES_256)))     // ACC_DEK MSKEY is AES-256 key
                        {
                            // get ACC_DEK MKEY bundle
			        	    PED_AccDEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
			        	    OS_SECM_GetData(ADDR_PED_ACC_DEK_AES_MKEY_01 + (MkeyIndex * PED_ACC_DEK_AES_MSKEY_SLOT_LEN), PED_ACC_DEK_AES_MSKEY_SLOT_LEN, aes_buf);

			        	    // verify ACC_DEK MKEY bundle
                            if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, aes_buf, aes_temp, mac16, (UINT8 *)0 ))
			        	    {
			        	    	// retrieve ACC_DEK MKEY
                                if(X9143_DecryptKeyBundle_AES(mac16, aes_temp, pkey2, (UINT8 *)0 ))	// pkey2=ACC_DEK MKEY (as the KBPK for ACC_DEK SKEY)
			        	    	{
			        	    		memmove(mkey, pkey2, sizeof(mkey));

			        	    		for(i = 0 ; i < pkey2[0] ; i++)
			        	    			pkey2[i + 1] &= 0xFE;	// ignore parity bit of ACC_DEK MKEY

			        	    		if(LIB_memcmp(skey, &pkey2[1], pkey2[0]) == 0)	// compared with ACC_DEK MKEY
			        	    			flag = FALSE;
			        	    		else
			        	    		{
			        	    			// get ACC_DEK SKEY bundle
			        	    			OS_SECM_GetData(ADDR_PED_ACC_DEK_AES_SKEY_01, PED_ACC_DEK_AES_MSKEY_SLOT_LEN, aes_buf);

			        	    			// verify ACC_DEK SKEY bundle
                                        if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, aes_buf, aes_temp, mac16, mkey))
			        	    			{
			        	    				// retrieve ACC_DEK SKEY
                                            if(X9143_DecryptKeyBundle_AES(mac16, aes_temp, pkey2, mkey))	// pkey2=ACC_DEK SKEY
			        	    				{
			        	    					memmove(pkey1, &pkey2[1], pkey2[0]);

			        	    					for(i = 0 ; i < pkey2[0] ; i++)
			        	    						pkey1[i] &= 0xFE;	 // ignore parity bit of ACC_DEK SKEY

			        	    					if(LIB_memcmp(skey, pkey1, pkey2[0]) == 0)	// compared with ACC_DEK SKEY
			        	    						flag = FALSE;
			        	    					else
			        	    						flag = TRUE;
			        	    				}
			        	    			}
			        	    		}
			        	    	}
			        	    }
                        }

                        if(flag)
                        {
                            PED_FPE_GetKeyType(&keyType);

                            // enforce the key value different from FPE MSKEY
                            if((pkey1[0] == PED_128_AES_KEY_PROTECT_KEY_LEN) && (keyType == AES_128)) // FPE MSKEY is AES-128 key
                            {
                                // get FPE MKEY bundle
                                PED_FPE_ReadMKeyIndex((UINT8 *)&MkeyIndex);
                                OS_SECM_GetData(ADDR_PED_FPE_KEY_AES_MKEY_01 + (MkeyIndex * PED_FPE_KEY_AES_MSKEY_SLOT_LEN), PED_FPE_KEY_AES_MSKEY_SLOT_LEN, aes_buf);

                                // verify FPE MKEY bundle
			        	        if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, aes_buf, aes_temp, mac16, (UINT8 *)0 ))
			        	        {
			        	        	// retrieve FPE MKEY
			        	        	if(X9143_DecryptKeyBundle_AES(mac16, aes_temp, pkey2, (UINT8 *)0 ))	// pkey2=FPE MKEY
			        	        	{
			        	        		memmove(mkey, pkey2, sizeof(mkey));

			        	        		for(i = 0 ; i < pkey1[0] ; i++)
			        	        			pkey2[i + 1] &= 0xFE;	// ignore parity bit of FPE MKEY

			        	        		if(LIB_memcmp(skey, &pkey2[1], pkey1[0]) == 0)	// compared with FPE MKEY
			        	        			flag = FALSE;
			        	        		else
			        	        		{
			        	        			// get FPE SKEY bundle
			        	        			OS_SECM_GetData(ADDR_PED_FPE_KEY_AES_SKEY_01, PED_FPE_KEY_AES_MSKEY_SLOT_LEN, aes_buf);

			        	        			// verify FPE SKEY bundle
			        	        			if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, aes_buf, aes_temp, mac16, mkey))
			        	        			{
			        	        				// retrieve FPE SKEY
			        	        				if(X9143_DecryptKeyBundle_AES(mac16, aes_temp, pkey2, mkey))	// pkey2=FPE SKEY
			        	        				{
                                                    memmove(pkey1, &pkey2[1], pkey2[0]);

			        	        					for(i = 0 ; i < pkey2[0] ; i++)
			        	        						pkey1[i] &= 0xFE;	 // ignore parity bit of FPE SKEY

			        	        					if(LIB_memcmp(skey, pkey1, pkey2[0]) == 0)	// compared with FPE SKEY
			        	        						flag = FALSE;
			        	        					else
			        	        						flag = TRUE;
			        	        				}
			        	        			}
			        	        		}
			        	        	}
			        	        }
                            }
                        }
			        }
				}
			}

			// save it
			if(flag)
			{
				OS_SECM_PutData(ADDR_PED_PEK_AES_SKEY_01 + (index * PED_PEK_AES_MSKEY_SLOT_LEN), PED_PEK_AES_MSKEY_SLOT_LEN, key);
				result = apiOK;
			}
		}
    }

	// clear sensitive data
	memset(temp, 0x00, sizeof(temp));
	memset(buf, 0x00, sizeof(buf));
    memset(tdes_temp, 0x00, sizeof(tdes_temp));
	memset(tdes_buf, 0x00, sizeof(tdes_buf));
    memset(aes_temp, 0x00, sizeof(aes_temp));
	memset(aes_buf, 0x00, sizeof(aes_buf));
	memset(pkey1, 0x00, sizeof(pkey1));
	memset(pkey2, 0x00, sizeof(pkey2));
	memset(skey, 0x00, sizeof(skey));
	memset(mkey, 0x00, sizeof(mkey));
	
	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: To setup one master key at the specified key slot.
// INPUT   : index  - key slot index. (default 0)
//	         length - length of master key data. (MUST be 112)
//	         key    - master key data. (EMK(112):in ANSI TR-31 AES key bundle format)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
#if 0
UINT8	PED_ISO4_SetMasterKey(UINT8 index, UINT8 length, UINT8 *key)
{
	UINT8	temp[KEY_DATA_LEN2 + 1];
	UINT8	buf[KEY_BUNDLE_LEN2];
	UINT8	pkey1[PED_ISO4_KEY_MSKEY_LEN + 1];
	UINT8	pkey2[PED_ISO4_KEY_MSKEY_LEN + 1];
	UINT8	skey[PED_ISO4_KEY_MSKEY_LEN + 1];
	UINT8	mkey[PED_ISO4_KEY_MSKEY_LEN + 1];
	UINT32	i;
	UINT32	flag;
	UINT8	MkeyIndex;
	UINT8	result;
	UINT8	mac16[16];


	result = apiFailed;

	if((index < MAX_ISO4_KEY_MKEY_CNT) && (length == KEY_BUNDLE_LEN2))
	{
		if(TR31_VerifyKeyBundle_AES(length, key, temp, mac16))
		{
			if(TR31_DecryptKeyBundle_AES(mac16, temp, pkey1))	// pkey1=ISO4_KEY MKEY
			{
				// enforce the key value different from FPE MSKEY
				flag = TRUE;

				// get FPE MKEY bundle
				PED_FPE_ReadMKeyIndex((UINT8 *)&MkeyIndex);
				OS_SECM_GetData(ADDR_PED_FPE_KEY_MKEY_01 + (MkeyIndex * PED_FPE_KEY_MSKEY_SLOT_LEN), PED_FPE_KEY_MSKEY_SLOT_LEN, buf);

				// verify FPE MKEY bundle
				if(TR31_VerifyKeyBundle_AES(KEY_BUNDLE_LEN2, buf, temp, mac16))
				{
					// retrieve FPE MKEY
					if(TR31_DecryptKeyBundle_AES(mac16, temp, pkey2))	// pkey2=FPE MKEY
					{
						memmove(mkey, pkey2, sizeof(mkey));

						for(i = 0 ; i < pkey1[0] ; i++)
						{
							pkey1[i + 1] &= 0xFE;	// ignore parity bit of ISO4_KEY MKEY
							pkey2[i + 1] &= 0xFE;	// ignore parity bit of FPE MKEY
						}

						if(LIB_memcmp(&pkey1[1], &pkey2[1], pkey1[0]) == 0)	// compared with FPE MKEY
							flag = FALSE;
						else
						{
							// get FPE SKEY bundle
							OS_SECM_GetData(ADDR_PED_FPE_KEY_SKEY_01, PED_FPE_KEY_MSKEY_SLOT_LEN, buf);

							// verify FPE SKEY bundle
							if(TR31_VerifyKeyBundle_AES(KEY_BUNDLE_LEN2, buf, temp, mac16))
							{
								// retrieve FPE ESKEY
								if(TR31_DecryptKeyBundle_AES(mac16, temp, pkey2))	// pkey2=FPE ESKEY
								{
									// retrieve FPE SKEY from FPE MKEY
									api_aes_decipher(skey, &pkey2[1], &mkey[1], pkey2[0]);

									for(i = 0 ; i < pkey2[0] ; i++)
										skey[i] &= 0xFE;	 // ignore parity bit of FPE SKEY

									if(LIB_memcmp(&pkey1[1], skey, pkey1[0]) == 0)	// compared with FPE SKEY
										flag = FALSE;
									else
										flag = TRUE;
								}
							}
						}
					}
				}

				// save it
				if(flag)
				{
					OS_SECM_PutData(ADDR_PED_ISO4_KEY_MKEY_01 + (index * PED_ISO4_KEY_MSKEY_SLOT_LEN), PED_ISO4_KEY_MSKEY_SLOT_LEN, key);
					PED_ISO4_WriteMKeyIndex(index);

					result = apiOK;
				}
			}
		}
	}

	// clear sensitive data
	memset(temp, 0x00, sizeof(temp));
	memset(buf, 0x00, sizeof(buf));
	memset(pkey1, 0x00, sizeof(pkey1));
	memset(pkey2, 0x00, sizeof(pkey2));
	memset(skey, 0x00, sizeof(skey));
	memset(mkey, 0x00, sizeof(mkey));
	
	return(result);
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To setup one session key at the specified key slot.
// INPUT   : index  - key slot index.
//	         length - length of session key data. (MUST be 112)
//	         key    - session key data, encrypted by the selected master key.
//		              (ESK(112):in ANSI TR-31 AES key bundle format)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
#if 0
UINT8	PED_ISO4_SetSessionKey(UINT8 index, UINT8 length, UINT8 *key)
{
	UINT8	temp[KEY_DATA_LEN2 + 1];
	UINT8	buf[KEY_BUNDLE_LEN2];
	UINT8	pkey1[PED_ISO4_KEY_MSKEY_LEN + 1];
	UINT8	pkey2[PED_ISO4_KEY_MSKEY_LEN + 1];
	UINT8	skey[PED_ISO4_KEY_MSKEY_LEN + 1];
	UINT8	mkey[PED_ISO4_KEY_MSKEY_LEN + 1];
	UINT32	i;
	UINT32	flag;
	UINT8	MkeyIndex;
	UINT8	result;
	UINT8	mac16[16];


	result = apiFailed;

	if((index < MAX_ISO4_KEY_SKEY_CNT) && (length == KEY_BUNDLE_LEN2))
	{
		if(TR31_VerifyKeyBundle_AES(length, key, temp, mac16))
		{
			if(TR31_DecryptKeyBundle_AES(mac16, temp, pkey1))	// pkey1=ISO4_KEY ESKEY
			{
				// enforce the key value different from ISO4_KEY MKEY and FPE MSKEY
				flag = TRUE;

				// get ISO4_KEY MKEY bundle
				PED_ISO4_ReadMKeyIndex((UINT8 *)&MkeyIndex);
				OS_SECM_GetData(ADDR_PED_ISO4_KEY_MKEY_01 + (MkeyIndex * PED_ISO4_KEY_MSKEY_SLOT_LEN), PED_ISO4_KEY_MSKEY_SLOT_LEN, buf);

				// verify ISO4_KEY MKEY bundle
				if(TR31_VerifyKeyBundle_AES(KEY_BUNDLE_LEN2, buf, temp, mac16))
				{
					// retrieve ISO4_KEY MKEY
					if(TR31_DecryptKeyBundle_AES(mac16, temp, pkey2))	// pkey2=ISO4_KEY MKEY
					{
						memmove(mkey, pkey2, sizeof(mkey));

						// retrieve ISO4_KEY SKEY from ISO4_KEY MKEY
						api_aes_decipher(skey, &pkey1[1], &pkey2[1], pkey1[0]);

						for(i = 0 ; i < pkey2[0] ; i++)
						{
							skey[i] &= 0xFE;	 // ignore parity bit of ISO4_KEY SKEY
							pkey2[i + 1] &= 0xFE; // ignore parity bit of ISO4_KEY MKEY
						}

						if(LIB_memcmp(skey, &pkey2[1], pkey2[0]) == 0)  // compared with ISO4_KEY MKEY
							flag = FALSE;
						else
						{
							// get FPE MKEY bundle
							PED_FPE_ReadMKeyIndex((UINT8 *)&MkeyIndex);
							OS_SECM_GetData(ADDR_PED_FPE_KEY_MKEY_01 + (MkeyIndex * PED_FPE_KEY_MSKEY_SLOT_LEN), PED_FPE_KEY_MSKEY_SLOT_LEN, buf);

							// verify FPE MKEY bundle
							if(TR31_VerifyKeyBundle_AES(KEY_BUNDLE_LEN2, buf, temp, mac16))
							{
								// retrieve FPE MKEY
								if(TR31_DecryptKeyBundle_AES(mac16, temp, pkey2))	// pkey2=FPE MKEY
								{
									memmove(mkey, pkey2, sizeof(mkey));

									for(i = 0 ; i < pkey2[0] ; i++)
										pkey2[i + 1] &= 0xFE;	// ignore parity bit of FPE MKEY

									if(LIB_memcmp(skey, &pkey2[1], pkey2[0]) == 0)	// compared with FPE MKEY
										flag = FALSE;
									else
									{
										// get FPE SKEY bundle
										OS_SECM_GetData(ADDR_PED_FPE_KEY_SKEY_01, PED_FPE_KEY_MSKEY_SLOT_LEN, buf);

										// verify FPE SKEY bundle
										if(TR31_VerifyKeyBundle_AES(KEY_BUNDLE_LEN2, buf, temp, mac16))
										{
											// retrieve FPE ESKEY
											if(TR31_DecryptKeyBundle_AES(mac16, temp, pkey2))	// pkey2=FPE ESKEY
											{
												// retrieve FPE SKEY from FPE MKEY
												api_aes_decipher(pkey1, &pkey2[1], &mkey[1], pkey2[0]);

												for(i = 0 ; i < pkey2[0] ; i++)
													pkey1[i] &= 0xFE;	 // ignore parity bit of FPE SKEY

												if(LIB_memcmp(skey, pkey1, pkey2[0]) == 0)	// compared with FPE SKEY
													flag = FALSE;
												else
													flag = TRUE;
											}
										}
									}
								}
							}
						}
					}
				}

				// save it
				if(flag)
				{
					OS_SECM_PutData(ADDR_PED_ISO4_KEY_SKEY_01 + (index * PED_ISO4_KEY_MSKEY_SLOT_LEN), PED_ISO4_KEY_MSKEY_SLOT_LEN, key);
					result = apiOK;
				}
			}
		}
	}

	// clear sensitive data
	memset(temp, 0x00, sizeof(temp));
	memset(buf, 0x00, sizeof(buf));
	memset(pkey1, 0x00, sizeof(pkey1));
	memset(pkey2, 0x00, sizeof(pkey2));
	memset(skey, 0x00, sizeof(skey));
	memset(mkey, 0x00, sizeof(mkey));
	
	return(result);
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To setup ISO4_KEY at the specified key slot.
// INPUT   : length - length of session key data. (MUST be 112)
//	         key    - session key data.
//		              (ESK(112):in ANSI TR-31 AES key bundle format)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UINT8	PED_SetISO4KEY(UINT8 length, UINT8 *key)
{
    UINT8	temp[X9143_AES_KEY_DATA_LEN + 1];   // ==== [Debug] ====
	UINT8	buf[X9143_AES_KEY_BUNDLE_LEN];  // ==== [Debug] ====
	//UINT8	temp[KEY_DATA_LEN2 + 1];
	//UINT8	buf[KEY_BUNDLE_LEN2];
	UINT8	pkey1[PED_ISO4_KEY_LEN + 1];
	UINT8	pkey2[PED_ISO4_KEY_LEN + 1];
	UINT32	i;
	UINT32	flag;
	UINT8	result;
	UINT8	mac16[16];


	result = apiFailed;

    printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);    // ==== [Debug] ====

	//if(length == KEY_BUNDLE_LEN2)
    if(length == X9143_AES_KEY_BUNDLE_LEN)
	{
		//if(TR31_VerifyKeyBundle_AES(length, key, temp, mac16))
        if(X9143_VerifyKeyBundle_AES(length, key, temp, mac16, (UINT8 *)0))
		{
			//if(TR31_DecryptKeyBundle_AES(mac16, temp, pkey1))	// pkey1=ISO4_KEY
            if(X9143_DecryptKeyBundle_AES(mac16, temp, pkey1, (UINT8 *)0))	// pkey1=ISO4_KEY
			{
                // ==== [Debug] ====
                printf("pkey1 = ");
                for(int i = 0 ; i < pkey1[0] + 1 ; i++)
                    printf("%02x", pkey1[i]);
                printf("\n");
                // ==== [Debug] ====

				// enforce the key value different from FPE KEY
				flag = TRUE;

				for(i = 0 ; i < pkey1[0] ; i++)
					pkey1[i + 1] &= 0xFE;	 // ignore parity bit of ISO4_KEY

				// get FPE KEY bundle
				OS_SECM_GetData(ADDR_PED_FPE_KEY, PED_FPE_KEY_SLOT_LEN, buf);

				// verify FPE KEY bundle
				//if(TR31_VerifyKeyBundle_AES(KEY_BUNDLE_LEN2, buf, temp, mac16))
                if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, buf, temp, mac16, (UINT8 *)0))
				{
					// retrieve FPE KEY
					//if(TR31_DecryptKeyBundle_AES(mac16, temp, pkey2))	// pkey2=FPE KEY
                    if(X9143_DecryptKeyBundle_AES(mac16, temp, pkey2, (UINT8 *)0))	// pkey2=FPE KEY
					{
						for(i = 0 ; i < pkey2[0] ; i++)
							pkey2[i + 1] &= 0xFE;	 // ignore parity bit of FPE KEY

						if(LIB_memcmp(&pkey1[1], &pkey2[1], pkey2[0]) == 0)	// compared with FPE KEY
							flag = FALSE;
						else
							flag = TRUE;
					}
				}

				// save it
				if(flag)
				{
					OS_SECM_PutData(ADDR_PED_ISO4_KEY, PED_ISO4_KEY_SLOT_LEN, key);
					result = apiOK;
				}
			}
		}
	}

	// clear sensitive data
	memset(temp, 0x00, sizeof(temp));
	memset(buf, 0x00, sizeof(buf));
	memset(pkey1, 0x00, sizeof(pkey1));
	memset(pkey2, 0x00, sizeof(pkey2));
	
	return(result);
}

// ---------------------------------------------------------------------------
// FUNCTION: To setup one master key at the specified key slot.
// INPUT   : index  - key slot index. (default 0)
//	         length - length of master key data. (MUST be 80)
//	         key    - master key data. (EMK(80):in ANSI TR-31 key bundle format)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
/*
UINT8	PED_AccDEK_SetMasterKey(UINT8 index, UINT8 length, UINT8 *key)
{
    UINT8	temp[X9143_TDES_KEY_DATA_LEN + 1];  // ==== [Debug] ====
	UINT8	buf[X9143_TDES_KEY_BUNDLE_LEN]; // ==== [Debug] ====
	//UINT8	temp[KEY_DATA_LEN + 1];
	//UINT8	buf[KEY_BUNDLE_LEN];
	UINT8	pkey1[PED_ACC_DEK_MSKEY_LEN + 1];
	UINT8	pkey2[PED_ACC_DEK_MSKEY_LEN + 1];
	UINT8	skey[PED_ACC_DEK_MSKEY_LEN + 1];
	UINT8	mkey[PED_ACC_DEK_MSKEY_LEN + 1];
	UINT32	i, j;
	UINT32	flag;
	UINT8	MkeyIndex;
	UINT8	result;
	UINT8	mac8[8];


	result = apiFailed;

    printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);    // ==== [Debug] ====

	//if((index < MAX_ACC_DEK_MKEY_CNT) && (length == KEY_BUNDLE_LEN))
    if((index < MAX_ACC_DEK_MKEY_CNT) && (length == X9143_TDES_KEY_BUNDLE_LEN))
	{
		//if(TR31_VerifyKeyBundle(length, key, temp, mac8, (UINT8 *)0 ))
        if(X9143_VerifyKeyBundle_TDES(length, key, temp, mac8, (UINT8 *)0 ))
		{
			//if(TR31_DecryptKeyBundle(mac8, temp, pkey1, (UINT8 *)0 ))	// pkey1=ACC_DEK MKEY
            if(X9143_DecryptKeyBundle_TDES(mac8, temp, pkey1, (UINT8 *)0 ))	// pkey1=ACC_DEK MKEY
			{
                // ==== [Debug] ====
                printf("pkey1 = ");
                for(int i = 0 ; i < pkey1[0] + 1 ; i++)
                    printf("%02x", pkey1[i]);
                printf("\n");
                // ==== [Debug] ====

				// enforce the key value different from KPK and MSKEY
				flag = TRUE;

				OS_SECM_GetData(ADDR_PED_TDES_KEY_PROTECT_KEY, PED_TDES_KEY_PROTECT_KEY_LEN + 1, pkey2);	// pkey2=KPK

				for(i = 0; i < pkey1[0]; i++)
				{
					pkey1[i + 1] &= 0xFE; // ignore parity bit of input KEY
					pkey2[i + 1] &= 0xFE; // ignore parity bit of KPK
				}

				if(LIB_memcmp(pkey1, pkey2, pkey1[0] + 1) == 0)	// compared with KPK
					flag = FALSE;
				else
				{
					// get MKEY bundle
					PED_ReadMKeyIndex((UINT8 *)&MkeyIndex);
					OS_SECM_GetData(ADDR_PED_MKEY_01 + (MkeyIndex * PED_MSKEY_SLOT_LEN), PED_MSKEY_SLOT_LEN, buf);

					// verify MKEY bundle
					//if(TR31_VerifyKeyBundle(KEY_BUNDLE_LEN, buf, temp, mac8, (UINT8 *)0 ))
                    if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, buf, temp, mac8, (UINT8 *)0 ))
					{
						// retrieve MKEY
						//if(TR31_DecryptKeyBundle(mac8, temp, pkey2, (UINT8 *)0 ))	// pkey2=MKEY
                        if(X9143_DecryptKeyBundle_TDES(mac8, temp, pkey2, (UINT8 *)0 ))	// pkey2=MKEY
						{
							memmove(mkey, pkey2, sizeof(mkey));

							for(i = 0 ; i < pkey1[0] ; i++)
								pkey2[i + 1] &= 0xFE;	// ignore parity bit of MKEY

							if(LIB_memcmp(&pkey1[1], &pkey2[1], pkey1[0]) == 0)	// compared with MKEY
								flag = FALSE;
							else
							{
								for(i = 0; i < MAX_SKEY_CNT; i++)
								{
									OS_SECM_GetData(ADDR_PED_SKEY_01 + (i * PED_MSKEY_SLOT_LEN), PED_MSKEY_SLOT_LEN, buf);
									//if(TR31_VerifyKeyBundle(KEY_BUNDLE_LEN, buf, temp, mac8, mkey ))
                                    if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, buf, temp, mac8, mkey ))
									{
										//if(TR31_DecryptKeyBundle(mac8, temp, pkey2, mkey ))	// pkey2=SKEY
                                        if(X9143_DecryptKeyBundle_TDES(mac8, temp, pkey2, mkey ))	// pkey2=SKEY
										{
											memmove( skey, &pkey2[1], pkey2[0] );
											
											// retrieve SKEY from MKEY
//											PED_TripleDES2(&mkey[1], pkey2[0], &pkey2[1], skey, mkey );

											for(j = 0; j < pkey2[0]; j++)
												skey[j] &= 0xFE; // ignore parity bit of SKEY

											if(LIB_memcmp(&pkey1[1], skey, pkey1[0]) == 0) // compared with SKEY
											{
												flag = FALSE;
												break;
											}
										}
									}
								}
							}
						}
					}
				}

				// save it
				if(flag)
				{
					OS_SECM_PutData(ADDR_PED_ACC_DEK_MKEY_01 + (index * PED_ACC_DEK_MSKEY_SLOT_LEN), PED_ACC_DEK_MSKEY_SLOT_LEN, key);
					PED_AccDEK_WriteMKeyIndex(index);

					result = apiOK;
				}
			}
		}
	}

	// clear sensitive data
	memset(temp, 0x00, sizeof(temp));
	memset(buf, 0x00, sizeof(buf));
	memset(pkey1, 0x00, sizeof(pkey1));
	memset(pkey2, 0x00, sizeof(pkey2));
	memset(skey, 0x00, sizeof(skey));
	memset(mkey, 0x00, sizeof(mkey));
	
	return(result);
}
*/

// ---------------------------------------------------------------------------
// FUNCTION: To setup one master key at the specified key slot.
// INPUT   : index  - key slot index. (default 0)
//	         length - length of master key data. (MUST be 96 or 144)
//	         key    - master key data. (EMK(96):in ANSI X9.143 TDES key bundle format)
//                                     (EMK(144):in ANSI X9.143 AES key bundle format)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UINT8	PED_AccDEK_SetMasterKey(UINT8 index, UINT8 length, UINT8 *key)
{
    UINT8	temp[X9143_TDES_KEY_DATA_LEN + 1];
	UINT8	buf[X9143_TDES_KEY_BUNDLE_LEN];
	UINT8	tdes_temp[X9143_TDES_KEY_DATA_LEN + 1];
	UINT8	tdes_buf[X9143_TDES_KEY_BUNDLE_LEN];
    UINT8	aes_temp[X9143_AES_KEY_DATA_LEN + 1];
	UINT8	aes_buf[X9143_AES_KEY_BUNDLE_LEN];
	UINT8	pkey1[PED_ACC_DEK_MSKEY_LEN + 1];
	UINT8	pkey2[PED_ACC_DEK_MSKEY_LEN + 1];
	UINT8	skey[PED_ACC_DEK_MSKEY_LEN + 1];
	UINT8	mkey[PED_ACC_DEK_MSKEY_LEN + 1];
	UINT32	i, j;
	UINT32	flag;
	UINT8	MkeyIndex;
	UINT8	result;
	UINT8	mac8[8];
    UINT8	mac16[16];
    UINT8   keyType;


	result = apiFailed;

    if((index < MAX_ACC_DEK_MKEY_CNT) && (length == X9143_TDES_KEY_BUNDLE_LEN))
	{
        printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);    // ==== [Debug] ====

        if(X9143_VerifyKeyBundle_TDES(length, key, tdes_temp, mac8, (UINT8 *)0 ))
		{
            if(X9143_DecryptKeyBundle_TDES(mac8, tdes_temp, pkey1, (UINT8 *)0 ))	// pkey1=ACC_DEK MKEY
			{
                // ==== [Debug] ====
                printf("pkey1 = ");
                for(int i = 0 ; i < pkey1[0] + 1 ; i++)
                    printf("%02x", pkey1[i]);
                printf("\n");
                // ==== [Debug] ====

				// enforce the key value different from KPK and PEK MSKEY
				flag = TRUE;

				OS_SECM_GetData(ADDR_PED_192_TDES_KEY_PROTECT_KEY, PED_192_TDES_KEY_PROTECT_KEY_LEN + 1, pkey2);	// pkey2=KPK

				for(i = 0; i < pkey1[0]; i++)
				{
					pkey1[i + 1] &= 0xFE; // ignore parity bit of input KEY
					pkey2[i + 1] &= 0xFE; // ignore parity bit of KPK
				}

				if(LIB_memcmp(pkey1, pkey2, pkey1[0] + 1) == 0)	// compared with KPK
					flag = FALSE;
				else
				{
                    PED_PEK_GetKeyType(&keyType);

                    // enforce the key value different from PEK MSKEY
                    if((pkey1[0] == PED_192_TDES_KEY_PROTECT_KEY_LEN) && (keyType == TDES_192)) // PEK MSKEY is TDES-192 key
                    {
                        // get PEK MKEY bundle
					    PED_PEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
					    OS_SECM_GetData(ADDR_PED_PEK_TDES_MKEY_01 + (MkeyIndex * PED_PEK_TDES_MSKEY_SLOT_LEN), PED_PEK_TDES_MSKEY_SLOT_LEN, tdes_buf);

					    // verify PEK MKEY bundle
                        if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, tdes_buf, tdes_temp, mac8, (UINT8 *)0 ))
					    {
					    	// retrieve PEK MKEY
                            if(X9143_DecryptKeyBundle_TDES(mac8, tdes_temp, pkey2, (UINT8 *)0 ))	// pkey2=PEK MKEY
					    	{
					    		memmove(mkey, pkey2, sizeof(mkey));

					    		for(i = 0 ; i < pkey1[0] ; i++)
					    			pkey2[i + 1] &= 0xFE;	// ignore parity bit of PEK MKEY

					    		if(LIB_memcmp(&pkey1[1], &pkey2[1], pkey1[0]) == 0)	// compared with PEK MKEY
					    			flag = FALSE;
					    		else
					    		{
					    			for(i = 0 ; i < MAX_PEK_SKEY_CNT ; i++)
					    			{
                                        // get PEK SKEY bundle
					    				OS_SECM_GetData(ADDR_PED_PEK_TDES_SKEY_01 + (i * PED_PEK_TDES_MSKEY_SLOT_LEN), PED_PEK_TDES_MSKEY_SLOT_LEN, tdes_buf);

                                        // verify PEK SKEY bundle
                                        if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, tdes_buf, tdes_temp, mac8, mkey))
					    				{
					    					// retrieve PEK SKEY
                                            if(X9143_DecryptKeyBundle_TDES(mac8, tdes_temp, pkey2, mkey))	// pkey2=PEK SKEY
					    					{
					    						memmove(skey, &pkey2[1], pkey2[0]);

					    						for(j = 0 ; j < pkey2[0] ; j++)
					    							skey[j] &= 0xFE; // ignore parity bit of PEK SKEY

					    						if(LIB_memcmp(&pkey1[1], skey, pkey1[0]) == 0) // compared with PEK SKEY
					    						{
					    							flag = FALSE;
					    							break;
					    						}
					    					}
					    				}
					    			}
					    		}
					    	}
					    }
                    }
				}

				// save it
				if(flag)
				{
					OS_SECM_PutData(ADDR_PED_ACC_DEK_TDES_MKEY_01 + (index * PED_ACC_DEK_TDES_MSKEY_SLOT_LEN), PED_ACC_DEK_TDES_MSKEY_SLOT_LEN, key);
					PED_AccDEK_WriteMKeyIndex(index);

					result = apiOK;
				}
			}
		}
	}
    else if((index < MAX_ACC_DEK_MKEY_CNT) && (length == X9143_AES_KEY_BUNDLE_LEN))
    {
        printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);    // ==== [Debug] ====

        if(X9143_VerifyKeyBundle_AES(length, key, aes_temp, mac16, (UINT8 *)0 ))
		{
            if(X9143_DecryptKeyBundle_AES(mac16, aes_temp, pkey1, (UINT8 *)0 ))	// pkey1=ACC_DEK MKEY
			{
                // ==== [Debug] ====
                printf("pkey1 = ");
                for(int i = 0 ; i < pkey1[0] + 1 ; i++)
                    printf("%02x", pkey1[i]);
                printf("\n");
                // ==== [Debug] ====

				// enforce the key value different from KPK, PEK MSKEY, and FPE MSKEY
				flag = TRUE;

				if(pkey1[0] == PED_128_AES_KEY_PROTECT_KEY_LEN)
				    OS_SECM_GetData(ADDR_PED_128_AES_KEY_PROTECT_KEY, PED_128_AES_KEY_PROTECT_KEY_LEN + 1, pkey2);	// pkey2=KPK
                else if(pkey1[0] == PED_192_AES_KEY_PROTECT_KEY_LEN)
				    OS_SECM_GetData(ADDR_PED_192_AES_KEY_PROTECT_KEY, PED_192_AES_KEY_PROTECT_KEY_LEN + 1, pkey2);	// pkey2=KPK
                else if(pkey1[0] == PED_256_AES_KEY_PROTECT_KEY_LEN)
				    OS_SECM_GetData(ADDR_PED_256_AES_KEY_PROTECT_KEY, PED_256_AES_KEY_PROTECT_KEY_LEN + 1, pkey2);	// pkey2=KPK

				for(i = 0 ; i < pkey1[0] ; i++)
				{
					pkey1[i + 1] &= 0xFE; // ignore parity bit of input KEY
					pkey2[i + 1] &= 0xFE; // ignore parity bit of KPK
				}

				if(LIB_memcmp(pkey1, pkey2, pkey1[0] + 1) == 0)	// compared with KPK
					flag = FALSE;
				else
				{
                    PED_PEK_GetKeyType(&keyType);
                    
                    // enforce the key value different from PEK MSKEY
                    if(((pkey1[0] == PED_128_AES_KEY_PROTECT_KEY_LEN) && (keyType == AES_128)) ||   // PEK MSKEY is AES-128 key
                       ((pkey1[0] == PED_192_AES_KEY_PROTECT_KEY_LEN) && (keyType == AES_192)) ||   // PEK MSKEY is AES-192 key
                       ((pkey1[0] == PED_256_AES_KEY_PROTECT_KEY_LEN) && (keyType == AES_256)))     // PEK MSKEY is AES-256 key
                    {
                        // get PEK MKEY bundle
					    PED_PEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
					    OS_SECM_GetData(ADDR_PED_PEK_AES_MKEY_01 + (MkeyIndex * PED_PEK_AES_MSKEY_SLOT_LEN), PED_PEK_AES_MSKEY_SLOT_LEN, aes_buf);

					    // verify PEK MKEY bundle
                        if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, aes_buf, aes_temp, mac16, (UINT8 *)0 ))
					    {
					    	// retrieve PEK MKEY
                            if(X9143_DecryptKeyBundle_AES(mac16, aes_temp, pkey2, (UINT8 *)0 ))	// pkey2=PEK MKEY
					    	{
					    		memmove(mkey, pkey2, sizeof(mkey));

					    		for(i = 0 ; i < pkey1[0] ; i++)
					    			pkey2[i + 1] &= 0xFE;	// ignore parity bit of PEK MKEY

					    		if(LIB_memcmp(&pkey1[1], &pkey2[1], pkey1[0]) == 0)	// compared with PEK MKEY
					    			flag = FALSE;
					    		else
					    		{
					    			for(i = 0 ; i < MAX_PEK_SKEY_CNT ; i++)
					    			{
                                        // get PEK SKEY bundle
					    				OS_SECM_GetData(ADDR_PED_PEK_AES_SKEY_01 + (i * PED_PEK_AES_MSKEY_SLOT_LEN), PED_PEK_AES_MSKEY_SLOT_LEN, aes_buf);

                                        // verify PEK SKEY bundle
                                        if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, aes_buf, aes_temp, mac16, mkey))
					    				{
					    					// retrieve PEK SKEY
                                            if(X9143_DecryptKeyBundle_AES(mac16, aes_temp, pkey2, mkey))	// pkey2=PEK SKEY
					    					{
					    						memmove(skey, &pkey2[1], pkey2[0]);

					    						for(j = 0 ; j < pkey2[0] ; j++)
					    							skey[j] &= 0xFE; // ignore parity bit of PEK SKEY

					    						if(LIB_memcmp(&pkey1[1], skey, pkey1[0]) == 0) // compared with PEK SKEY
					    						{
					    							flag = FALSE;
					    							break;
					    						}
					    					}
					    				}
					    			}
					    		}
					    	}
					    }
                    }
					
                    if(flag)
                    {
                        PED_FPE_GetKeyType(&keyType);

                        // enforce the key value different from FPE MSKEY
                        if((pkey1[0] == PED_128_AES_KEY_PROTECT_KEY_LEN) && (keyType == AES_128)) // FPE MSKEY is AES-128 key
                        {
                            // get FPE MKEY bundle
                            PED_FPE_ReadMKeyIndex((UINT8 *)&MkeyIndex);
                            OS_SECM_GetData(ADDR_PED_FPE_KEY_AES_MKEY_01 + (MkeyIndex * PED_FPE_KEY_AES_MSKEY_SLOT_LEN), PED_FPE_KEY_AES_MSKEY_SLOT_LEN, aes_buf);

                            // verify FPE MKEY bundle
				            if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, aes_buf, aes_temp, mac16, (UINT8 *)0 ))
				            {
				            	// retrieve FPE MKEY
				            	if(X9143_DecryptKeyBundle_AES(mac16, aes_temp, pkey2, (UINT8 *)0 ))	// pkey2=FPE MKEY
				            	{
				            		memmove(mkey, pkey2, sizeof(mkey));

				            		for(i = 0 ; i < pkey1[0] ; i++)
				            			pkey2[i + 1] &= 0xFE;	// ignore parity bit of FPE MKEY

				            		if(LIB_memcmp(&pkey1[1], &pkey2[1], pkey1[0]) == 0)	// compared with FPE MKEY
				            			flag = FALSE;
				            		else
				            		{
				            			// get FPE SKEY bundle
				            			OS_SECM_GetData(ADDR_PED_FPE_KEY_AES_SKEY_01, PED_FPE_KEY_AES_MSKEY_SLOT_LEN, aes_buf);

				            			// verify FPE SKEY bundle
				            			if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, aes_buf, aes_temp, mac16, mkey))
				            			{
				            				// retrieve FPE SKEY
				            				if(X9143_DecryptKeyBundle_AES(mac16, aes_temp, pkey2, mkey))	// pkey2=FPE SKEY
				            				{
                                                memmove(skey, &pkey2[1], pkey2[0]);
                                                
				            					for(i = 0 ; i < pkey2[0] ; i++)
				            						skey[i] &= 0xFE;	 // ignore parity bit of FPE SKEY

				            					if(LIB_memcmp(&pkey1[1], skey, pkey1[0]) == 0)	// compared with FPE SKEY
				            						flag = FALSE;
				            					else
				            						flag = TRUE;
				            				}
				            			}
				            		}
				            	}
				            }
                        }
                    }
				}

				// save it
				if(flag)
				{
					OS_SECM_PutData(ADDR_PED_ACC_DEK_AES_MKEY_01 + (index * PED_ACC_DEK_AES_MSKEY_SLOT_LEN), PED_ACC_DEK_AES_MSKEY_SLOT_LEN, key);
					PED_AccDEK_WriteMKeyIndex(index);

					result = apiOK;
				}
			}
		}
    }

	// clear sensitive data
	memset(temp, 0x00, sizeof(temp));
	memset(buf, 0x00, sizeof(buf));
    memset(tdes_temp, 0x00, sizeof(tdes_temp));
	memset(tdes_buf, 0x00, sizeof(tdes_buf));
    memset(aes_temp, 0x00, sizeof(aes_temp));
	memset(aes_buf, 0x00, sizeof(aes_buf));
	memset(pkey1, 0x00, sizeof(pkey1));
	memset(pkey2, 0x00, sizeof(pkey2));
	memset(skey, 0x00, sizeof(skey));
	memset(mkey, 0x00, sizeof(mkey));
	
	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: To setup one session key at the specified key slot.
// INPUT   : index  - key slot index.
//	         length - length of session key data. (MUST be 80)
//	         key    - session key data, encrypted by the selected master key.
//		              (ESK(80):in ANSI TR-31 key bundle format)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
/*
UINT8	PED_AccDEK_SetSessionKey(UINT8 index, UINT8 length, UINT8 *key)
{
    UINT8	temp[X9143_TDES_KEY_DATA_LEN + 1];  // ==== [Debug] ====
	UINT8	buf[X9143_TDES_KEY_BUNDLE_LEN]; // ==== [Debug] ====
	//UINT8	temp[KEY_DATA_LEN + 1];
	//UINT8	buf[KEY_BUNDLE_LEN];
	UINT8	pkey1[PED_ACC_DEK_MSKEY_LEN + 1];
	UINT8	pkey2[PED_ACC_DEK_MSKEY_LEN + 1];
	UINT8	skey[PED_ACC_DEK_MSKEY_LEN + 1];
	UINT8	mkey[PED_ACC_DEK_MSKEY_LEN + 1];
	UINT32	i, j;
	UINT32	flag;
	UINT8	MkeyIndex;
	UINT8	result;
	UINT8	mac8[8];


    printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);    // ==== [Debug] ====

	// get ACC_DEK MKEY bundle
	PED_AccDEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
	OS_SECM_GetData(ADDR_PED_ACC_DEK_MKEY_01 + (MkeyIndex * PED_ACC_DEK_MSKEY_SLOT_LEN), PED_ACC_DEK_MSKEY_SLOT_LEN, buf);
	
	// verify ACC_DEK MKEY bundle
	//if( TR31_VerifyKeyBundle(KEY_BUNDLE_LEN, buf, temp, mac8, (UINT8 *)0 ) )
    if( X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, buf, temp, mac8, (UINT8 *)0 ) )
	  {
	  // retrieve ACC_DEK MKEY
	  //if( TR31_DecryptKeyBundle(mac8, temp, pkey2, (UINT8 *)0 ) )	// pkey2=ACC_DEK MKEY (as the KBPK for SKEY)
      if( X9143_DecryptKeyBundle_TDES(mac8, temp, pkey2, (UINT8 *)0 ) )	// pkey2=ACC_DEK MKEY (as the KBPK for SKEY)
	    memmove(mkey, pkey2, sizeof(mkey));
	  else
	    return( apiFailed );
	  }
	  
	  
	result = apiFailed;

	//if((index < MAX_ACC_DEK_SKEY_CNT) && (length == KEY_BUNDLE_LEN))
    if((index < MAX_ACC_DEK_SKEY_CNT) && (length == X9143_TDES_KEY_BUNDLE_LEN))
	{
		//if( TR31_VerifyKeyBundle(length, key, temp, mac8, mkey) )
        if( X9143_VerifyKeyBundle_TDES(length, key, temp, mac8, mkey) )
		{
			//if(TR31_DecryptKeyBundle(mac8, temp, pkey1, mkey))	// pkey1=ACC_DEK SKEY
            if(X9143_DecryptKeyBundle_TDES(mac8, temp, pkey1, mkey))	// pkey1=ACC_DEK SKEY
			{
				memmove( skey, &pkey1[1], pkey1[0] );

                // ==== [Debug] ====
                printf("pkey1 = ");
                for(int i = 0 ; i < pkey1[0] + 1 ; i++)
                    printf("%02x", pkey1[i]);
                printf("\n");
                // ==== [Debug] ====
				
				// enforce the key value different from ACC_DEK MKEY, KPK, and MSKEY
				flag = TRUE;

				// get ACC_DEK MKEY bundle
//				PED_AccDEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
//				OS_SECM_GetData(ADDR_PED_ACC_DEK_MKEY_01 + (MkeyIndex * PED_ACC_DEK_MSKEY_SLOT_LEN), PED_ACC_DEK_MSKEY_SLOT_LEN, buf);

				// verify ACC_DEK MKEY bundle
//				if(TR31_VerifyKeyBundle(KEY_BUNDLE_LEN, buf, temp, mac8))
//				{
					// retrieve ACC_DEK MKEY
//					if(TR31_DecryptKeyBundle(mac8, temp, pkey2))	// pkey2=ACC_DEK MKEY
//					{
//						memmove(mkey, pkey2, sizeof(mkey));

						// retrieve ACC_DEK SKEY from ACC_DEK MKEY
//						PED_TripleDES2(&pkey2[1], pkey1[0], &pkey1[1], skey);

						for(i = 0; i < pkey2[0]; i++)
						{
							skey[i] &= 0xFE;	 // ignore parity bit of ACC_DEK SKEY
							pkey2[i + 1] &= 0xFE; // ignore parity bit of ACC_DEK MKEY
						}

						if(LIB_memcmp(skey, &pkey2[1], pkey2[0]) == 0)  // compared with ACC_DEK MKEY
							flag = FALSE;
						else
						{
							OS_SECM_GetData(ADDR_PED_TDES_KEY_PROTECT_KEY, PED_TDES_KEY_PROTECT_KEY_LEN + 1, pkey2);	// pkey2=KPK

							for(i = 0; i < pkey2[0]; i++)
								pkey2[i + 1] &= 0xFE; // ignore parity bit of KPK

							if(LIB_memcmp(skey, &pkey2[1], pkey2[0]) == 0) // compared with KPK
								flag = FALSE;
							else
							{
								// get MKEY bundle
								PED_ReadMKeyIndex((UINT8 *)&MkeyIndex);
								OS_SECM_GetData(ADDR_PED_MKEY_01 + (MkeyIndex * PED_MSKEY_SLOT_LEN), PED_MSKEY_SLOT_LEN, buf);

								// verify MKEY bundle
								//if(TR31_VerifyKeyBundle(KEY_BUNDLE_LEN, buf, temp, mac8, (UINT8 *)0 ))
                                if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, buf, temp, mac8, (UINT8 *)0 ))
								{
									// retrieve MKEY
									//if(TR31_DecryptKeyBundle(mac8, temp, pkey2, (UINT8 *)0 ))	// pkey2=MKEY
                                    if(X9143_DecryptKeyBundle_TDES(mac8, temp, pkey2, (UINT8 *)0 ))	// pkey2=MKEY
									{
										memmove(mkey, pkey2, sizeof(mkey));

										for(i = 0 ; i < pkey2[0] ; i++)
											pkey2[i + 1] &= 0xFE;	// ignore parity bit of MKEY

										if(LIB_memcmp(skey, &pkey2[1], pkey2[0]) == 0)	// compared with MKEY
											flag = FALSE;
										else
										{
											for(i = 0; i < MAX_SKEY_CNT; i++)
											{
												OS_SECM_GetData(ADDR_PED_SKEY_01 + (i * PED_MSKEY_SLOT_LEN), PED_MSKEY_SLOT_LEN, buf);
												//if(TR31_VerifyKeyBundle(KEY_BUNDLE_LEN, buf, temp, mac8, mkey ))
                                                if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, buf, temp, mac8, mkey ))
												{
													//if(TR31_DecryptKeyBundle(mac8, temp, pkey2, mkey ))	// pkey2=ESKEY
                                                    if(X9143_DecryptKeyBundle_TDES(mac8, temp, pkey2, mkey ))	// pkey2=ESKEY
													{
														memmove( pkey1, &pkey2[1], pkey2[0] );
														
														// retrieve SKEY from MKEY
//														PED_TripleDES2(&mkey[1], pkey2[0], &pkey2[1], pkey1);

														for(j = 0; j < pkey2[0]; j++)
															pkey1[j] &= 0xFE; // ignore parity bit of SKEY

														if(LIB_memcmp(skey, pkey1, pkey2[0]) == 0) // compared with SKEY
														{
															flag = FALSE;
															break;
														}
													}
												}
											}
										}
									}
								}
							}
						}
//					}
//				}

				// save it
				if(flag)
				{
					OS_SECM_PutData(ADDR_PED_ACC_DEK_SKEY_01 + (index * PED_ACC_DEK_MSKEY_SLOT_LEN), PED_ACC_DEK_MSKEY_SLOT_LEN, key);
					result = apiOK;
				}
			}
		}
	}

	// clear sensitive data
	memset(temp, 0x00, sizeof(temp));
	memset(buf, 0x00, sizeof(buf));
	memset(pkey1, 0x00, sizeof(pkey1));
	memset(pkey2, 0x00, sizeof(pkey2));
	memset(skey, 0x00, sizeof(skey));
	memset(mkey, 0x00, sizeof(mkey));
	
	return(result);
}
*/

// ---------------------------------------------------------------------------
// FUNCTION: To setup one session key at the specified key slot.
// INPUT   : index  - key slot index.
//	         length - length of session key data. (MUST be 96 or 144)
//	         key    - session key data, encrypted by the selected master key.
//		              (ESK(96):in ANSI X9.143 TDES key bundle format)
//		              (ESK(144):in ANSI X9.143 AES key bundle format)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UINT8	PED_AccDEK_SetSessionKey(UINT8 index, UINT8 length, UINT8 *key)
{
    UINT8	temp[X9143_TDES_KEY_DATA_LEN + 1];
	UINT8	buf[X9143_TDES_KEY_BUNDLE_LEN];
	UINT8	tdes_temp[X9143_TDES_KEY_DATA_LEN + 1];
	UINT8	tdes_buf[X9143_TDES_KEY_BUNDLE_LEN];
    UINT8	aes_temp[X9143_AES_KEY_DATA_LEN + 1];
	UINT8	aes_buf[X9143_AES_KEY_BUNDLE_LEN];
	UINT8	pkey1[PED_ACC_DEK_MSKEY_LEN + 1];
	UINT8	pkey2[PED_ACC_DEK_MSKEY_LEN + 1];
	UINT8	skey[PED_ACC_DEK_MSKEY_LEN + 1];
	UINT8	mkey[PED_ACC_DEK_MSKEY_LEN + 1];
	UINT32	i, j;
	UINT32	flag;
	UINT8	MkeyIndex;
	UINT8	result;
	UINT8	mac8[8];
    UINT8   mac16[16];
    UINT8   keyType;

	
	result = apiFailed;

    if((index < MAX_ACC_DEK_SKEY_CNT) && (length == X9143_TDES_KEY_BUNDLE_LEN))
	{
        printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);    // ==== [Debug] ====

        // get ACC_DEK MKEY bundle
        PED_AccDEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
        OS_SECM_GetData(ADDR_PED_ACC_DEK_TDES_MKEY_01 + (MkeyIndex * PED_ACC_DEK_TDES_MSKEY_SLOT_LEN), PED_ACC_DEK_TDES_MSKEY_SLOT_LEN, tdes_buf);
        
        // verify ACC_DEK MKEY bundle
        if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, tdes_buf, tdes_temp, mac8, (UINT8 *)0 ))
        {
            // retrieve ACC_DEK MKEY
            if(X9143_DecryptKeyBundle_TDES(mac8, tdes_temp, pkey2, (UINT8 *)0 ))	// pkey2=ACC_DEK MKEY (as the KBPK for ACC_DEK SKEY)
                memmove(mkey, pkey2, sizeof(mkey));
            else
                return apiFailed;
        }

        if(X9143_VerifyKeyBundle_TDES(length, key, tdes_temp, mac8, mkey))
		{
            if(X9143_DecryptKeyBundle_TDES(mac8, tdes_temp, pkey1, mkey))	// pkey1=ACC_DEK SKEY
			{
				memmove(skey, &pkey1[1], pkey1[0]);

                // ==== [Debug] ====
                printf("pkey1 = ");
                for(int i = 0 ; i < pkey1[0] + 1 ; i++)
                    printf("%02x", pkey1[i]);
                printf("\n");
                // ==== [Debug] ====
				
				// enforce the key value different from ACC_DEK MKEY, KPK, and PEK MSKEY
				flag = TRUE;

				for(i = 0 ; i < pkey2[0] ; i++)
				{
					skey[i] &= 0xFE;	 // ignore parity bit of ACC_DEK SKEY
					pkey2[i + 1] &= 0xFE; // ignore parity bit of ACC_DEK MKEY
				}

				if(LIB_memcmp(skey, &pkey2[1], pkey2[0]) == 0)  // compared with ACC_DEK MKEY
					flag = FALSE;
				else
				{
                    OS_SECM_GetData(ADDR_PED_192_TDES_KEY_PROTECT_KEY, PED_192_TDES_KEY_PROTECT_KEY_LEN + 1, pkey2);	// pkey2=KPK

					for(i = 0 ; i < pkey2[0] ; i++)
						pkey2[i + 1] &= 0xFE; // ignore parity bit of KPK

					if(LIB_memcmp(skey, &pkey2[1], pkey2[0]) == 0) // compared with KPK
						flag = FALSE;
					else
					{
                        PED_PEK_GetKeyType(&keyType);

                        // enforce the key value different from PEK MSKEY
                        if((pkey1[0] == PED_192_TDES_KEY_PROTECT_KEY_LEN) && (keyType == TDES_192))   // PEK MSKEY is TDES-192 key
                        {
                            // get PEK MKEY bundle
						    PED_PEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
						    OS_SECM_GetData(ADDR_PED_PEK_TDES_MKEY_01 + (MkeyIndex * PED_PEK_TDES_MSKEY_SLOT_LEN), PED_PEK_TDES_MSKEY_SLOT_LEN, tdes_buf);

						    // verify PEK MKEY bundle
                            if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, tdes_buf, tdes_temp, mac8, (UINT8 *)0 ))
						    {
						    	// retrieve PEK MKEY
                                if(X9143_DecryptKeyBundle_TDES(mac8, tdes_temp, pkey2, (UINT8 *)0 ))	// pkey2=PEK MKEY
						    	{
						    		memmove(mkey, pkey2, sizeof(mkey));

						    		for(i = 0 ; i < pkey2[0] ; i++)
						    			pkey2[i + 1] &= 0xFE;	// ignore parity bit of PEK MKEY

						    		if(LIB_memcmp(skey, &pkey2[1], pkey2[0]) == 0)	// compared with PEK MKEY
						    			flag = FALSE;
						    		else
						    		{
						    			for(i = 0 ; i < MAX_PEK_SKEY_CNT ; i++)
						    			{
						    				OS_SECM_GetData(ADDR_PED_PEK_TDES_SKEY_01 + (i * PED_PEK_TDES_MSKEY_SLOT_LEN), PED_PEK_TDES_MSKEY_SLOT_LEN, tdes_buf);
    
                                            if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, tdes_buf, tdes_temp, mac8, mkey))
						    				{
                                                if(X9143_DecryptKeyBundle_TDES(mac8, tdes_temp, pkey2, mkey))	// pkey2=PEK SKEY
						    					{
						    						memmove(pkey1, &pkey2[1], pkey2[0]);
    
						    						for(j = 0 ; j < pkey2[0] ; j++)
						    							pkey1[j] &= 0xFE; // ignore parity bit of PEK SKEY

						    						if(LIB_memcmp(skey, pkey1, pkey2[0]) == 0) // compared with PEK SKEY
						    						{
						    							flag = FALSE;
						    							break;
						    						}
						    					}
						    				}
						    			}
						    		}
						    	}
						    }
                        }
					}
				}

				// save it
				if(flag)
				{
					OS_SECM_PutData(ADDR_PED_ACC_DEK_TDES_SKEY_01 + (index * PED_ACC_DEK_TDES_MSKEY_SLOT_LEN), PED_ACC_DEK_TDES_MSKEY_SLOT_LEN, key);
					result = apiOK;
				}
			}
		}
	}
    else if((index < MAX_ACC_DEK_SKEY_CNT) && (length == X9143_AES_KEY_BUNDLE_LEN))
	{
        printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);    // ==== [Debug] ====

        // get ACC_DEK MKEY bundle
        PED_AccDEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
        OS_SECM_GetData(ADDR_PED_ACC_DEK_AES_MKEY_01 + (MkeyIndex * PED_ACC_DEK_AES_MSKEY_SLOT_LEN), PED_ACC_DEK_AES_MSKEY_SLOT_LEN, aes_buf);
        
        // verify ACC_DEK MKEY bundle
        if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, aes_buf, aes_temp, mac16, (UINT8 *)0 ))
        {
            // retrieve ACC_DEK MKEY
            if(X9143_DecryptKeyBundle_AES(mac16, aes_temp, pkey2, (UINT8 *)0 ))	// pkey2=ACC_DEK MKEY (as the KBPK for ACC_DEK SKEY)
                memmove(mkey, pkey2, sizeof(mkey));
            else
                return apiFailed;
        }

        if(X9143_VerifyKeyBundle_AES(length, key, aes_temp, mac16, mkey))
		{
            if(X9143_DecryptKeyBundle_AES(mac16, aes_temp, pkey1, mkey))	// pkey1=ACC_DEK SKEY
			{
				memmove(skey, &pkey1[1], pkey1[0]);

                // ==== [Debug] ====
                printf("pkey1 = ");
                for(int i = 0 ; i < pkey1[0] + 1 ; i++)
                    printf("%02x", pkey1[i]);
                printf("\n");
                // ==== [Debug] ====
				
				// enforce the key value different from ACC_DEK MKEY, KPK, and FPE MSKEY
				flag = TRUE;

				for(i = 0 ; i < pkey2[0] ; i++)
				{
					skey[i] &= 0xFE;	 // ignore parity bit of ACC_DEK SKEY
					pkey2[i + 1] &= 0xFE; // ignore parity bit of ACC_DEK MKEY
				}

				if(LIB_memcmp(skey, &pkey2[1], pkey2[0]) == 0)  // compared with ACC_DEK MKEY
					flag = FALSE;
				else
				{
                    if(pkey1[0] == PED_128_AES_KEY_PROTECT_KEY_LEN)
                        OS_SECM_GetData(ADDR_PED_128_AES_KEY_PROTECT_KEY, PED_128_AES_KEY_PROTECT_KEY_LEN + 1, pkey2);	// pkey2=KPK
                    else if(pkey1[0] == PED_192_AES_KEY_PROTECT_KEY_LEN)
                        OS_SECM_GetData(ADDR_PED_192_AES_KEY_PROTECT_KEY, PED_192_AES_KEY_PROTECT_KEY_LEN + 1, pkey2);	// pkey2=KPK
                    else if(pkey1[0] == PED_256_AES_KEY_PROTECT_KEY_LEN)
                        OS_SECM_GetData(ADDR_PED_256_AES_KEY_PROTECT_KEY, PED_256_AES_KEY_PROTECT_KEY_LEN + 1, pkey2);	// pkey2=KPK
                    
					for(i = 0 ; i < pkey2[0] ; i++)
						pkey2[i + 1] &= 0xFE; // ignore parity bit of KPK

					if(LIB_memcmp(skey, &pkey2[1], pkey2[0]) == 0) // compared with KPK
						flag = FALSE;
					else
					{
                        PED_PEK_GetKeyType(&keyType);

                        // enforce the key value different from PEK MSKEY
                        if(((pkey1[0] == PED_128_AES_KEY_PROTECT_KEY_LEN) && (keyType == AES_128)) ||   // PEK MSKEY is AES-128 key
                           ((pkey1[0] == PED_192_AES_KEY_PROTECT_KEY_LEN) && (keyType == AES_192)) ||   // PEK MSKEY is AES-192 key
                           ((pkey1[0] == PED_256_AES_KEY_PROTECT_KEY_LEN) && (keyType == AES_256)))     // PEK MSKEY is AES-256 key
                        {
                            // get PEK MKEY bundle
						    PED_PEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
						    OS_SECM_GetData(ADDR_PED_PEK_AES_MKEY_01 + (MkeyIndex * PED_PEK_AES_MSKEY_SLOT_LEN), PED_PEK_AES_MSKEY_SLOT_LEN, aes_buf);

						    // verify PEK MKEY bundle
                            if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, aes_buf, aes_temp, mac16, (UINT8 *)0 ))
						    {
						    	// retrieve PEK MKEY
                                if(X9143_DecryptKeyBundle_AES(mac16, aes_temp, pkey2, (UINT8 *)0 ))	// pkey2=PEK MKEY
						    	{
						    		memmove(mkey, pkey2, sizeof(mkey));

						    		for(i = 0 ; i < pkey2[0] ; i++)
						    			pkey2[i + 1] &= 0xFE;	// ignore parity bit of PEK MKEY

						    		if(LIB_memcmp(skey, &pkey2[1], pkey2[0]) == 0)	// compared with PEK MKEY
						    			flag = FALSE;
						    		else
						    		{
						    			for(i = 0 ; i < MAX_PEK_SKEY_CNT ; i++)
						    			{
						    				OS_SECM_GetData(ADDR_PED_PEK_AES_SKEY_01 + (i * PED_PEK_AES_MSKEY_SLOT_LEN), PED_PEK_AES_MSKEY_SLOT_LEN, aes_buf);
    
                                            if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, aes_buf, aes_temp, mac16, mkey))
						    				{
                                                if(X9143_DecryptKeyBundle_AES(mac16, aes_temp, pkey2, mkey))	// pkey2=PEK SKEY
						    					{
						    						memmove(pkey1, &pkey2[1], pkey2[0]);
    
						    						for(j = 0 ; j < pkey2[0] ; j++)
						    							pkey1[j] &= 0xFE; // ignore parity bit of PEK SKEY

						    						if(LIB_memcmp(skey, pkey1, pkey2[0]) == 0) // compared with PEK SKEY
						    						{
						    							flag = FALSE;
						    							break;
						    						}
						    					}
						    				}
						    			}
						    		}
						    	}
						    }
                        }

						if(flag)
                        {
                            PED_FPE_GetKeyType(&keyType);

                            // enforce the key value different from FPE MSKEY
                            if((pkey1[0] == PED_128_AES_KEY_PROTECT_KEY_LEN) && (keyType == AES_128)) // FPE MSKEY is AES-128 key
                            {
                                // get FPE MKEY bundle
                                PED_FPE_ReadMKeyIndex((UINT8 *)&MkeyIndex);
                                OS_SECM_GetData(ADDR_PED_FPE_KEY_AES_MKEY_01 + (MkeyIndex * PED_FPE_KEY_AES_MSKEY_SLOT_LEN), PED_FPE_KEY_AES_MSKEY_SLOT_LEN, aes_buf);

                                // verify FPE MKEY bundle
			        	        if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, aes_buf, aes_temp, mac16, (UINT8 *)0 ))
			        	        {
			        	        	// retrieve FPE MKEY
			        	        	if(X9143_DecryptKeyBundle_AES(mac16, aes_temp, pkey2, (UINT8 *)0 ))	// pkey2=FPE MKEY
			        	        	{
			        	        		memmove(mkey, pkey2, sizeof(mkey));

			        	        		for(i = 0 ; i < pkey1[0] ; i++)
			        	        			pkey2[i + 1] &= 0xFE;	// ignore parity bit of FPE MKEY

			        	        		if(LIB_memcmp(skey, &pkey2[1], pkey1[0]) == 0)	// compared with FPE MKEY
			        	        			flag = FALSE;
			        	        		else
			        	        		{
			        	        			// get FPE SKEY bundle
			        	        			OS_SECM_GetData(ADDR_PED_FPE_KEY_AES_SKEY_01, PED_FPE_KEY_AES_MSKEY_SLOT_LEN, aes_buf);

			        	        			// verify FPE SKEY bundle
			        	        			if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, aes_buf, aes_temp, mac16, mkey))
			        	        			{
			        	        				// retrieve FPE SKEY
			        	        				if(X9143_DecryptKeyBundle_AES(mac16, aes_temp, pkey2, mkey))	// pkey2=FPE SKEY
			        	        				{
                                                    memmove(pkey1, &pkey2[1], pkey2[0]);

			        	        					for(i = 0 ; i < pkey2[0] ; i++)
			        	        						pkey1[i] &= 0xFE;	 // ignore parity bit of FPE SKEY

			        	        					if(LIB_memcmp(skey, pkey1, pkey2[0]) == 0)	// compared with FPE SKEY
			        	        						flag = FALSE;
			        	        					else
			        	        						flag = TRUE;
			        	        				}
			        	        			}
			        	        		}
			        	        	}
			        	        }
                            }
                        }
					}
				}

				// save it
				if(flag)
				{
					OS_SECM_PutData(ADDR_PED_ACC_DEK_AES_SKEY_01 + (index * PED_ACC_DEK_AES_MSKEY_SLOT_LEN), PED_ACC_DEK_AES_MSKEY_SLOT_LEN, key);
					result = apiOK;
				}
			}
		}
	}

	// clear sensitive data
	memset(temp, 0x00, sizeof(temp));
	memset(buf, 0x00, sizeof(buf));
    memset(tdes_temp, 0x00, sizeof(tdes_temp));
	memset(tdes_buf, 0x00, sizeof(tdes_buf));
    memset(aes_temp, 0x00, sizeof(aes_temp));
	memset(aes_buf, 0x00, sizeof(aes_buf));
	memset(pkey1, 0x00, sizeof(pkey1));
	memset(pkey2, 0x00, sizeof(pkey2));
	memset(skey, 0x00, sizeof(skey));
	memset(mkey, 0x00, sizeof(mkey));
	
	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: To setup one master key at the specified key slot.
// INPUT   : index  - key slot index. (default 0)
//	         length - length of master key data. (MUST be 112)
//	         key    - master key data. (EMK(112):in ANSI TR-31 AES key bundle format)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
/*
UINT8	PED_FPE_SetMasterKey(UINT8 index, UINT8 length, UINT8 *key)
{
	UINT8	temp[KEY_DATA_LEN2 + 1];
	UINT8	buf[KEY_BUNDLE_LEN2];
	UINT8	pkey1[PED_FPE_KEY_MSKEY_LEN + 1];
	UINT8	pkey2[PED_FPE_KEY_MSKEY_LEN + 1];
	UINT8	skey[PED_FPE_KEY_MSKEY_LEN + 1];
	UINT8	mkey[PED_FPE_KEY_MSKEY_LEN + 1];
	UINT32	i;
	UINT32	flag;
	UINT8	MkeyIndex;
	UINT8	result;
	UINT8	mac16[16];


	result = apiFailed;

	if((index < MAX_FPE_KEY_MKEY_CNT) && (length == KEY_BUNDLE_LEN2))
	{
		if(TR31_VerifyKeyBundle_AES(length, key, temp, mac16))
		{
			if(TR31_DecryptKeyBundle_AES(mac16, temp, pkey1))	// pkey1=FPE MKEY
			{
				// enforce the key value different from ISO4_KEY MSKEY
				flag = TRUE;

				// get ISO4_KEY MKEY bundle
				PED_ISO4_ReadMKeyIndex((UINT8 *)&MkeyIndex);
				OS_SECM_GetData(ADDR_PED_ISO4_KEY_MKEY_01 + (MkeyIndex * PED_ISO4_KEY_MSKEY_SLOT_LEN), PED_ISO4_KEY_MSKEY_SLOT_LEN, buf);

				// verify ISO4_KEY MKEY bundle
				if(TR31_VerifyKeyBundle_AES(KEY_BUNDLE_LEN2, buf, temp, mac16))
				{
					// retrieve ISO4_KEY MKEY
					if(TR31_DecryptKeyBundle_AES(mac16, temp, pkey2))	// pkey2=ISO4_KEY MKEY
					{
						memmove(mkey, pkey2, sizeof(mkey));

						for(i = 0 ; i < pkey1[0] ; i++)
						{
							pkey1[i + 1] &= 0xFE;	// ignore parity bit of FPE MKEY
							pkey2[i + 1] &= 0xFE;	// ignore parity bit of ISO4_KEY MKEY
						}

						if(LIB_memcmp(&pkey1[1], &pkey2[1], pkey1[0]) == 0)	// compared with ISO4_KEY MKEY
							flag = FALSE;
						else
						{
							// get ISO4_KEY SKEY bundle
							OS_SECM_GetData(ADDR_PED_ISO4_KEY_SKEY_01, PED_ISO4_KEY_MSKEY_SLOT_LEN, buf);

							// verify ISO4_KEY SKEY bundle
							if(TR31_VerifyKeyBundle_AES(KEY_BUNDLE_LEN2, buf, temp, mac16))
							{
								// retrieve ISO4_KEY ESKEY
								if(TR31_DecryptKeyBundle_AES(mac16, temp, pkey2))	// pkey2=ISO4_KEY ESKEY
								{
									// retrieve ISO4_KEY SKEY from ISO4_KEY MKEY
									api_aes_decipher(skey, &pkey2[1], &mkey[1], pkey2[0]);

									for(i = 0 ; i < pkey2[0] ; i++)
										skey[i] &= 0xFE;	 // ignore parity bit of ISO4_KEY SKEY

									if(LIB_memcmp(&pkey1[1], skey, pkey1[0]) == 0)	// compared with ISO4_KEY SKEY
										flag = FALSE;
									else
										flag = TRUE;
								}
							}
						}
					}
				}

				// save it
				if(flag)
				{
					OS_SECM_PutData(ADDR_PED_FPE_KEY_MKEY_01 + (index * PED_FPE_KEY_MSKEY_SLOT_LEN), PED_FPE_KEY_MSKEY_SLOT_LEN, key);
					PED_FPE_WriteMKeyIndex(index);

					result = apiOK;
				}
			}
		}
	}

	// clear sensitive data
	memset(temp, 0x00, sizeof(temp));
	memset(buf, 0x00, sizeof(buf));
	memset(pkey1, 0x00, sizeof(pkey1));
	memset(pkey2, 0x00, sizeof(pkey2));
	memset(skey, 0x00, sizeof(skey));
	memset(mkey, 0x00, sizeof(mkey));
	
	return(result);
}
*/

// ---------------------------------------------------------------------------
// FUNCTION: To setup one master key at the specified key slot.
// INPUT   : index  - key slot index. (default 0)
//	         length - length of master key data. (MUST be 144)
//	         key    - master key data. (ESK(144):in ANSI X9.143 AES key bundle format)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UINT8	PED_FPE_SetMasterKey(UINT8 index, UINT8 length, UINT8 *key)
{
    UINT8	temp[X9143_AES_KEY_DATA_LEN + 1];
	UINT8	buf[X9143_AES_KEY_BUNDLE_LEN];
	UINT8	pkey1[PED_FPE_KEY_MSKEY_LEN + 1];
	UINT8	pkey2[PED_FPE_KEY_MSKEY_LEN + 1];
	UINT8	skey[PED_FPE_KEY_MSKEY_LEN + 1];
	UINT8	mkey[PED_FPE_KEY_MSKEY_LEN + 1];
	UINT32	i, j;
	UINT32	flag;
	UINT8	MkeyIndex;
	UINT8	result;
	UINT8	mac16[16];
    UINT8   keyType;


	result = apiFailed;

	if((index < MAX_FPE_KEY_MKEY_CNT) && (length == X9143_AES_KEY_BUNDLE_LEN))
	{
        printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);    // ==== [Debug] ====

		if(X9143_VerifyKeyBundle_AES(length, key, temp, mac16, (UINT8 *)0))
		{
			if(X9143_DecryptKeyBundle_AES(mac16, temp, pkey1, (UINT8 *)0))	// pkey1=FPE MKEY
			{
                // ==== [Debug] ====
                printf("pkey1 = ");
                for(int i = 0 ; i < pkey1[0] + 1 ; i++)
                    printf("%02x", pkey1[i]);
                printf("\n");
                // ==== [Debug] ====

				// enforce the key value different from KPK, PEK MSKEY, and ACC_DEK MSKEY
				flag = TRUE;

                OS_SECM_GetData(ADDR_PED_128_AES_KEY_PROTECT_KEY, PED_128_AES_KEY_PROTECT_KEY_LEN + 1, pkey2);	// pkey2=KPK

                for(i = 0 ; i < pkey1[0] ; i++)
				{
					pkey1[i + 1] &= 0xFE; // ignore parity bit of input KEY
					pkey2[i + 1] &= 0xFE; // ignore parity bit of KPK
				}

                if(LIB_memcmp(pkey1, pkey2, pkey1[0] + 1) == 0)	// compared with KPK
					flag = FALSE;
				else
                {
                    PED_PEK_GetKeyType(&keyType);
                    
                    // enforce the key value different from PEK MSKEY
                    if(keyType == AES_128)  // PEK MSKEY is AES-128 key
                    {
                        // get PEK MKEY bundle
					    PED_PEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
					    OS_SECM_GetData(ADDR_PED_PEK_AES_MKEY_01 + (MkeyIndex * PED_PEK_AES_MSKEY_SLOT_LEN), PED_PEK_AES_MSKEY_SLOT_LEN, buf);

					    // verify PEK MKEY bundle
                        if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, buf, temp, mac16, (UINT8 *)0 ))
					    {
					    	// retrieve PEK MKEY
                            if(X9143_DecryptKeyBundle_AES(mac16, temp, pkey2, (UINT8 *)0 ))	// pkey2=PEK MKEY
					    	{
					    		memmove(mkey, pkey2, sizeof(mkey));

					    		for(i = 0 ; i < pkey1[0] ; i++)
					    			pkey2[i + 1] &= 0xFE;	// ignore parity bit of PEK MKEY

					    		if(LIB_memcmp(&pkey1[1], &pkey2[1], pkey1[0]) == 0)	// compared with PEK MKEY
					    			flag = FALSE;
					    		else
					    		{
					    			for(i = 0 ; i < MAX_PEK_SKEY_CNT ; i++)
					    			{
                                        // get PEK SKEY bundle
					    				OS_SECM_GetData(ADDR_PED_PEK_AES_SKEY_01 + (i * PED_PEK_AES_MSKEY_SLOT_LEN), PED_PEK_AES_MSKEY_SLOT_LEN, buf);

                                        // verify PEK SKEY bundle
                                        if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, buf, temp, mac16, mkey))
					    				{
					    					// retrieve PEK SKEY
                                            if(X9143_DecryptKeyBundle_AES(mac16, temp, pkey2, mkey))	// pkey2=PEK SKEY
					    					{
					    						memmove(skey, &pkey2[1], pkey2[0]);

					    						for(j = 0 ; j < pkey2[0] ; j++)
					    							skey[j] &= 0xFE; // ignore parity bit of PEK SKEY

					    						if(LIB_memcmp(&pkey1[1], skey, pkey1[0]) == 0) // compared with PEK SKEY
					    						{
					    							flag = FALSE;
					    							break;
					    						}
					    					}
					    				}
					    			}
					    		}
					    	}
					    }
                    }

                    if(flag)
                    {
                        PED_AccDEK_GetKeyType(&keyType);

                        // enforce the key value different from ACC_DEK MSKEY
                        if(((pkey1[0] == PED_128_AES_KEY_PROTECT_KEY_LEN) && (keyType == AES_128)) ||   // ACC_DEK MSKEY is AES-128 key
                           ((pkey1[0] == PED_192_AES_KEY_PROTECT_KEY_LEN) && (keyType == AES_192)) ||   // ACC_DEK MSKEY is AES-192 key
                           ((pkey1[0] == PED_256_AES_KEY_PROTECT_KEY_LEN) && (keyType == AES_256)))     // ACC_DEK MSKEY is AES-256 key
                        {
                            // get ACC_DEK MKEY bundle
					        PED_AccDEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
					        OS_SECM_GetData(ADDR_PED_ACC_DEK_AES_MKEY_01 + (MkeyIndex * PED_ACC_DEK_AES_MSKEY_SLOT_LEN), PED_ACC_DEK_AES_MSKEY_SLOT_LEN, buf);

					        // verify ACC_DEK MKEY bundle
                            if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, buf, temp, mac16, (UINT8 *)0 ))
					        {
					        	// retrieve ACC_DEK MKEY
                                if(X9143_DecryptKeyBundle_AES(mac16, temp, pkey2, (UINT8 *)0 ))	// pkey2=ACC_DEK MKEY
					        	{
					        		memmove(mkey, pkey2, sizeof(mkey));

					        		for(i = 0 ; i < pkey1[0] ; i++)
					        			pkey2[i + 1] &= 0xFE;	// ignore parity bit of ACC_DEK MKEY

					        		if(LIB_memcmp(&pkey1[1], &pkey2[1], pkey1[0]) == 0)	// compared with ACC_DEK MKEY
					        			flag = FALSE;
					        		else
					        		{
					        			// get ACC_DEK SKEY bundle
					        			OS_SECM_GetData(ADDR_PED_ACC_DEK_AES_SKEY_01, PED_ACC_DEK_AES_MSKEY_SLOT_LEN, buf);

					        			// verify ACC_DEK SKEY bundle
                                        if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, buf, temp, mac16, mkey))
					        			{
					        				// retrieve ACC_DEK SKEY
                                            if(X9143_DecryptKeyBundle_AES(mac16, temp, pkey2, mkey))	// pkey2=ACC_DEK SKEY
					        				{
					        					memmove(skey, &pkey2[1], pkey2[0]);

					        					for(i = 0 ; i < pkey2[0] ; i++)
					        						skey[i] &= 0xFE;	 // ignore parity bit of SKEY

					        					if(LIB_memcmp(&pkey1[1], skey, pkey1[0]) == 0)	// compared with ACC_DEK SKEY
					        						flag = FALSE;
					        					else
					        						flag = TRUE;
					        				}
					        			}
					        		}
					        	}
					        }
                        }
                    }
                }

				// save it
				if(flag)
				{
					OS_SECM_PutData(ADDR_PED_FPE_KEY_AES_MKEY_01 + (index * PED_FPE_KEY_AES_MSKEY_SLOT_LEN), PED_FPE_KEY_AES_MSKEY_SLOT_LEN, key);
					PED_FPE_WriteMKeyIndex(index);

					result = apiOK;
				}
			}
		}
	}

	// clear sensitive data
	memset(temp, 0x00, sizeof(temp));
	memset(buf, 0x00, sizeof(buf));
	memset(pkey1, 0x00, sizeof(pkey1));
	memset(pkey2, 0x00, sizeof(pkey2));
	memset(skey, 0x00, sizeof(skey));
	memset(mkey, 0x00, sizeof(mkey));
	
	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: To setup one session key at the specified key slot.
// INPUT   : index  - key slot index.
//	         length - length of session key data. (MUST be 112)
//	         key    - session key data, encrypted by the selected master key.
//		              (ESK(112):in ANSI TR-31 AES key bundle format)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
/*
UINT8	PED_FPE_SetSessionKey(UINT8 index, UINT8 length, UINT8 *key)
{
	UINT8	temp[KEY_DATA_LEN2 + 1];
	UINT8	buf[KEY_BUNDLE_LEN2];
	UINT8	pkey1[PED_FPE_KEY_MSKEY_LEN + 1];
	UINT8	pkey2[PED_FPE_KEY_MSKEY_LEN + 1];
	UINT8	skey[PED_FPE_KEY_MSKEY_LEN + 1];
	UINT8	mkey[PED_FPE_KEY_MSKEY_LEN + 1];
	UINT32	i;
	UINT32	flag;
	UINT8	MkeyIndex;
	UINT8	result;
	UINT8	mac16[16];


	result = apiFailed;

	if((index < MAX_FPE_KEY_SKEY_CNT) && (length == KEY_BUNDLE_LEN2))
	{
		if(TR31_VerifyKeyBundle_AES(length, key, temp, mac16))
		{
			if(TR31_DecryptKeyBundle_AES(mac16, temp, pkey1))	// pkey1=FPE ESKEY
			{
				// enforce the key value different from FPE MKEY and ISO4_KEY MSKEY
				flag = TRUE;

				// get FPE MKEY bundle
				PED_FPE_ReadMKeyIndex((UINT8 *)&MkeyIndex);
				OS_SECM_GetData(ADDR_PED_FPE_KEY_MKEY_01 + (MkeyIndex * PED_FPE_KEY_MSKEY_SLOT_LEN), PED_FPE_KEY_MSKEY_SLOT_LEN, buf);

				// verify FPE MKEY bundle
				if(TR31_VerifyKeyBundle_AES(KEY_BUNDLE_LEN2, buf, temp, mac16))
				{
					// retrieve FPE MKEY
					if(TR31_DecryptKeyBundle_AES(mac16, temp, pkey2))	// pkey2=FPE MKEY
					{
						memmove(mkey, pkey2, sizeof(mkey));

						// retrieve FPE SKEY from FPE MKEY
						api_aes_decipher(skey, &pkey1[1], &pkey2[1], pkey1[0]);

						for(i = 0 ; i < pkey2[0] ; i++)
						{
							skey[i] &= 0xFE;	 // ignore parity bit of FPE SKEY
							pkey2[i + 1] &= 0xFE; // ignore parity bit of FPE MKEY
						}

						if(LIB_memcmp(skey, &pkey2[1], pkey2[0]) == 0)  // compared with FPE MKEY
							flag = FALSE;
						else
						{
							// get ISO4_KEY MKEY bundle
							PED_ISO4_ReadMKeyIndex((UINT8 *)&MkeyIndex);
							OS_SECM_GetData(ADDR_PED_ISO4_KEY_MKEY_01 + (MkeyIndex * PED_ISO4_KEY_MSKEY_SLOT_LEN), PED_ISO4_KEY_MSKEY_SLOT_LEN, buf);

							// verify ISO4_KEY MKEY bundle
							if(TR31_VerifyKeyBundle_AES(KEY_BUNDLE_LEN2, buf, temp, mac16))
							{
								// retrieve ISO4_KEY MKEY
								if(TR31_DecryptKeyBundle_AES(mac16, temp, pkey2))	// pkey2=ISO4_KEY MKEY
								{
									memmove(mkey, pkey2, sizeof(mkey));

									for(i = 0 ; i < pkey2[0] ; i++)
										pkey2[i + 1] &= 0xFE;	// ignore parity bit of ISO4_KEY MKEY

									if(LIB_memcmp(skey, &pkey2[1], pkey2[0]) == 0)	// compared with ISO4_KEY MKEY
										flag = FALSE;
									else
									{
										// get ISO4_KEY SKEY bundle
										OS_SECM_GetData(ADDR_PED_ISO4_KEY_SKEY_01, PED_ISO4_KEY_MSKEY_SLOT_LEN, buf);

										// verify ISO4_KEY SKEY bundle
										if(TR31_VerifyKeyBundle_AES(KEY_BUNDLE_LEN2, buf, temp, mac16))
										{
											// retrieve ISO4_KEY ESKEY
											if(TR31_DecryptKeyBundle_AES(mac16, temp, pkey2))	// pkey2=ISO4_KEY ESKEY
											{
												// retrieve ISO4_KEY SKEY from ISO4_KEY MKEY
												api_aes_decipher(pkey1, &pkey2[1], &mkey[1], pkey2[0]);

												for(i = 0 ; i < pkey2[0] ; i++)
													pkey1[i] &= 0xFE;	 // ignore parity bit of ISO4_KEY SKEY

												if(LIB_memcmp(skey, pkey1, pkey2[0]) == 0)	// compared with ISO4_KEY SKEY
													flag = FALSE;
												else
													flag = TRUE;
											}
										}
									}
								}
							}
						}
					}
				}

				// save it
				if(flag)
				{
					OS_SECM_PutData(ADDR_PED_FPE_KEY_SKEY_01 + (index * PED_FPE_KEY_MSKEY_SLOT_LEN), PED_FPE_KEY_MSKEY_SLOT_LEN, key);
					result = apiOK;
				}
			}
		}
	}

	// clear sensitive data
	memset(temp, 0x00, sizeof(temp));
	memset(buf, 0x00, sizeof(buf));
	memset(pkey1, 0x00, sizeof(pkey1));
	memset(pkey2, 0x00, sizeof(pkey2));
	memset(skey, 0x00, sizeof(skey));
	memset(mkey, 0x00, sizeof(mkey));
	
	return(result);
}
*/

// ---------------------------------------------------------------------------
// FUNCTION: To setup one session key at the specified key slot.
// INPUT   : index  - key slot index.
//	         length - length of session key data. (MUST be 144)
//	         key    - session key data, encrypted by the selected master key.
//		              (ESK(144):in ANSI X9.143 AES key bundle format)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UINT8	PED_FPE_SetSessionKey(UINT8 index, UINT8 length, UINT8 *key)
{
	UINT8	temp[X9143_AES_KEY_DATA_LEN + 1];
	UINT8	buf[X9143_AES_KEY_BUNDLE_LEN];
	UINT8	pkey1[PED_FPE_KEY_MSKEY_LEN + 1];
	UINT8	pkey2[PED_FPE_KEY_MSKEY_LEN + 1];
	UINT8	skey[PED_FPE_KEY_MSKEY_LEN + 1];
	UINT8	mkey[PED_FPE_KEY_MSKEY_LEN + 1];
	UINT32	i, j;
	UINT32	flag;
	UINT8	MkeyIndex;
	UINT8	result;
	UINT8	mac16[16];
    UINT8   keyType;


	result = apiFailed;

	if((index < MAX_FPE_KEY_SKEY_CNT) && (length == X9143_AES_KEY_BUNDLE_LEN))
	{
        printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);    // ==== [Debug] ====
        
        // get FPE MKEY bundle
		PED_FPE_ReadMKeyIndex((UINT8 *)&MkeyIndex);
		OS_SECM_GetData(ADDR_PED_FPE_KEY_AES_MKEY_01 + (MkeyIndex * PED_FPE_KEY_AES_MSKEY_SLOT_LEN), PED_FPE_KEY_AES_MSKEY_SLOT_LEN, buf);
        
        // verify FPE MKEY bundle
		if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, buf, temp, mac16, (UINT8 *)0))
        {
            // retrieve FPE MKEY
			if(X9143_DecryptKeyBundle_AES(mac16, temp, pkey2, (UINT8 *)0))	// pkey2=FPE MKEY (as the KBPK for FPE SKEY)
				memmove(mkey, pkey2, sizeof(mkey));
            else
                return apiFailed;
        }

		if(X9143_VerifyKeyBundle_AES(length, key, temp, mac16, mkey))
		{
			if(X9143_DecryptKeyBundle_AES(mac16, temp, pkey1, mkey))	// pkey1=FPE SKEY
			{
                memmove(skey, &pkey1[1], pkey1[0]);

                // ==== [Debug] ====
                printf("pkey1 = ");
                for(int i = 0 ; i < pkey1[0] + 1 ; i++)
                    printf("%02x", pkey1[i]);
                printf("\n");
                // ==== [Debug] ====

				// enforce the key value different from FPE MKEY, KPK, PEK MSKEY, and ACC_DEK MSKEY
				flag = TRUE;
                
                for(i = 0 ; i < pkey2[0]; i++)
				{
					skey[i] &= 0xFE;	 // ignore parity bit of FPE SKEY
					pkey2[i + 1] &= 0xFE; // ignore parity bit of FPE MKEY
				}

                if(LIB_memcmp(skey, &pkey2[1], pkey2[0]) == 0)  // compared with FPE MKEY
					flag = FALSE;
				else
				{
                    OS_SECM_GetData(ADDR_PED_128_AES_KEY_PROTECT_KEY, PED_128_AES_KEY_PROTECT_KEY_LEN + 1, pkey2);	// pkey2=KPK

                    for(i = 0 ; i < pkey2[0] ; i++)
						pkey2[i + 1] &= 0xFE; // ignore parity bit of KPK
                    
                    if(LIB_memcmp(skey, &pkey2[1], pkey2[0]) == 0) // compared with KPK
						flag = FALSE;
                    else
                    {
                        PED_PEK_GetKeyType(&keyType);

                        // enforce the key value different from PEK MSKEY
                        if(keyType == AES_128)  // PEK MSKEY is AES-128 key
                        {
                            // get PEK MKEY bundle
						    PED_PEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
						    OS_SECM_GetData(ADDR_PED_PEK_AES_MKEY_01 + (MkeyIndex * PED_PEK_AES_MSKEY_SLOT_LEN), PED_PEK_AES_MSKEY_SLOT_LEN, buf);

						    // verify PEK MKEY bundle
                            if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, buf, temp, mac16, (UINT8 *)0 ))
						    {
						    	// retrieve PEK MKEY
                                if(X9143_DecryptKeyBundle_AES(mac16, temp, pkey2, (UINT8 *)0 ))	// pkey2=PEK MKEY
						    	{
						    		memmove(mkey, pkey2, sizeof(mkey));

						    		for(i = 0 ; i < pkey2[0] ; i++)
						    			pkey2[i + 1] &= 0xFE;	// ignore parity bit of PEK MKEY

						    		if(LIB_memcmp(skey, &pkey2[1], pkey2[0]) == 0)	// compared with PEK MKEY
						    			flag = FALSE;
						    		else
						    		{
						    			for(i = 0 ; i < MAX_PEK_SKEY_CNT ; i++)
						    			{
						    				OS_SECM_GetData(ADDR_PED_PEK_AES_SKEY_01 + (i * PED_PEK_AES_MSKEY_SLOT_LEN), PED_PEK_AES_MSKEY_SLOT_LEN, buf);
    
                                            if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, buf, temp, mac16, mkey))
						    				{
                                                if(X9143_DecryptKeyBundle_AES(mac16, temp, pkey2, mkey))	// pkey2=PEK SKEY
						    					{
						    						memmove(pkey1, &pkey2[1], pkey2[0]);
    
						    						for(j = 0 ; j < pkey2[0] ; j++)
						    							pkey1[j] &= 0xFE; // ignore parity bit of PEK SKEY

						    						if(LIB_memcmp(skey, pkey1, pkey2[0]) == 0) // compared with PEK SKEY
						    						{
						    							flag = FALSE;
						    							break;
						    						}
						    					}
						    				}
						    			}
						    		}
						    	}
						    }
                        }

                        if(flag)
                        {
                            PED_AccDEK_GetKeyType(&keyType);

                            // enforce the key value different from ACC_DEK MSKEY
                            if(keyType == AES_128)  // ACC_DEK MSKEY is AES-128 key
                            {
                                // get ACC_DEK MKEY bundle
			        	        PED_AccDEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
			        	        OS_SECM_GetData(ADDR_PED_ACC_DEK_AES_MKEY_01 + (MkeyIndex * PED_ACC_DEK_AES_MSKEY_SLOT_LEN), PED_ACC_DEK_AES_MSKEY_SLOT_LEN, buf);

			        	        // verify ACC_DEK MKEY bundle
                                if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, buf, temp, mac16, (UINT8 *)0 ))
			        	        {
			        	        	// retrieve ACC_DEK MKEY
                                    if(X9143_DecryptKeyBundle_AES(mac16, temp, pkey2, (UINT8 *)0 ))	// pkey2=ACC_DEK MKEY (as the KBPK for ACC_DEK SKEY)
			        	        	{
			        	        		memmove(mkey, pkey2, sizeof(mkey));

			        	        		for(i = 0 ; i < pkey2[0] ; i++)
			        	        			pkey2[i + 1] &= 0xFE;	// ignore parity bit of ACC_DEK MKEY

			        	        		if(LIB_memcmp(skey, &pkey2[1], pkey2[0]) == 0)	// compared with ACC_DEK MKEY
			        	        			flag = FALSE;
			        	        		else
			        	        		{
			        	        			// get ACC_DEK SKEY bundle
			        	        			OS_SECM_GetData(ADDR_PED_ACC_DEK_AES_SKEY_01, PED_ACC_DEK_AES_MSKEY_SLOT_LEN, buf);

			        	        			// verify ACC_DEK SKEY bundle
                                            if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, buf, temp, mac16, mkey))
			        	        			{
			        	        				// retrieve ACC_DEK SKEY
                                                if(X9143_DecryptKeyBundle_AES(mac16, temp, pkey2, mkey))	// pkey2=ACC_DEK SKEY
			        	        				{
			        	        					memmove(pkey1, &pkey2[1], pkey2[0]);

			        	        					for(i = 0 ; i < pkey2[0] ; i++)
			        	        						pkey1[i] &= 0xFE;	 // ignore parity bit of ACC_DEK SKEY

			        	        					if(LIB_memcmp(skey, pkey1, pkey2[0]) == 0)	// compared with ACC_DEK SKEY
			        	        						flag = FALSE;
			        	        					else
			        	        						flag = TRUE;
			        	        				}
			        	        			}
			        	        		}
			        	        	}
			        	        }
                            }
                        }
                    }
                }

                // save it
				if(flag)
				{
					OS_SECM_PutData(ADDR_PED_FPE_KEY_AES_SKEY_01 + (index * PED_FPE_KEY_AES_MSKEY_SLOT_LEN), PED_FPE_KEY_AES_MSKEY_SLOT_LEN, key);
					result = apiOK;
				}
			}
		}
	}

	// clear sensitive data
	memset(temp, 0x00, sizeof(temp));
	memset(buf, 0x00, sizeof(buf));
	memset(pkey1, 0x00, sizeof(pkey1));
	memset(pkey2, 0x00, sizeof(pkey2));
	memset(skey, 0x00, sizeof(skey));
	memset(mkey, 0x00, sizeof(mkey));
	
	return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: To setup FPE Key at the specified key slot.
// INPUT   : length - length of session key data. (MUST be 112)
//	         key    - session key data, encrypted by the selected master key.
//		              (ESK(112):in ANSI TR-31 AES key bundle format)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UINT8	PED_SetFPEKey(UINT8 length, UINT8 *key)
{
    UINT8	temp[X9143_AES_KEY_DATA_LEN + 1];   // ==== [Debug] ====
	UINT8	buf[X9143_AES_KEY_BUNDLE_LEN];  // ==== [Debug] ====
	//UINT8	temp[KEY_DATA_LEN2 + 1];
	//UINT8	buf[KEY_BUNDLE_LEN2];
	UINT8	pkey1[PED_FPE_KEY_LEN + 1];
	UINT8	pkey2[PED_FPE_KEY_LEN + 1];
	UINT32	i;
	UINT32	flag;
	UINT8	result;
	UINT8	mac16[16];


	result = apiFailed;

    printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);    // ==== [Debug] ====

	//if(length == KEY_BUNDLE_LEN2)
    if(length == X9143_AES_KEY_BUNDLE_LEN)
	{
		//if(TR31_VerifyKeyBundle_AES(length, key, temp, mac16))
        if(X9143_VerifyKeyBundle_AES(length, key, temp, mac16, (UINT8 *)0))
		{
			//if(TR31_DecryptKeyBundle_AES(mac16, temp, pkey1))	// pkey1=FPE KEY
            if(X9143_DecryptKeyBundle_AES(mac16, temp, pkey1, (UINT8 *)0))	// pkey1=FPE KEY
			{
                // ==== [Debug] ====
                printf("pkey1 = ");
                for(int i = 0 ; i < pkey1[0] + 1 ; i++)
                    printf("%02x", pkey1[i]);
                printf("\n");
                // ==== [Debug] ====

				// enforce the key value different from ISO4_KEY
				flag = TRUE;

				for(i = 0 ; i < pkey1[0] ; i++)
					pkey1[i + 1] &= 0xFE;	 // ignore parity bit of FPE KEY

				// get ISO4_KEY bundle
				OS_SECM_GetData(ADDR_PED_ISO4_KEY, PED_ISO4_KEY_SLOT_LEN, buf);

				// verify ISO4_KEY bundle
				//if(TR31_VerifyKeyBundle_AES(KEY_BUNDLE_LEN2, buf, temp, mac16))
                if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, buf, temp, mac16, (UINT8 *)0))
				{
					// retrieve ISO4_KEY
					//if(TR31_DecryptKeyBundle_AES(mac16, temp, pkey2))	// pkey2=ISO4_KEY
                    if(X9143_DecryptKeyBundle_AES(mac16, temp, pkey2, (UINT8 *)0))	// pkey2=ISO4_KEY
					{
						for(i = 0 ; i < pkey2[0] ; i++)
							pkey2[i + 1] &= 0xFE;	 // ignore parity bit of ISO4_KEY

						if(LIB_memcmp(&pkey1[1], &pkey2[1], pkey2[0]) == 0)	// compared with ISO4_KEY
							flag = FALSE;
						else
							flag = TRUE;
					}
				}

				// save it
				if(flag)
				{
					OS_SECM_PutData(ADDR_PED_FPE_KEY, PED_FPE_KEY_SLOT_LEN, key);
					result = apiOK;
				}
			}
		}
	}

	// clear sensitive data
	memset(temp, 0x00, sizeof(temp));
	memset(buf, 0x00, sizeof(buf));
	memset(pkey1, 0x00, sizeof(pkey1));
	memset(pkey2, 0x00, sizeof(pkey2));
	
	return(result);
}

// ---------------------------------------------------------------------------
// FUNCTION: To setup one fixed key at the specified key slot.
// INPUT   : index  - key slot index. (default 0)
//	     length - length of fixed key data. (MUST be 72)
//	     key    - fixed key data. (EFK(72):in ANSI TR-31 key bundle format)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
#if	0
UINT8	PED_SetFixedKey( UINT8 index, UINT8 length, UINT8 *key )
{
#ifdef	_USE_IRAM_PARA_
// 2014-11-24, allocate sensitive data buffer on MCU internal SRAM
#define	TEMP_SIZE_SFXK		KEY_DATA_LEN+1
#define	TEMP_ADDR_SFXK		0x0000F200	// pointer to MCU internal SRAM
#define	BUF_SIZE_SFXK		KEY_BUNDLE_LEN
#define	BUF_ADDR_SFXK		TEMP_ADDR_SFXK+TEMP_SIZE_SFXK
#define	PKEY1_SIZE_SFXK		PED_FKEY_LEN+1
#define	PKEY1_ADDR_SFXK		BUF_ADDR_SFXK+BUF_SIZE_SFXK
#define	PKEY2_SIZE_SFXK		PED_FKEY_LEN+1
#define	PKEY2_ADDR_SFXK		PKEY1_ADDR_SFXK+PKEY1_SIZE_SFXK

#define	PKEY11_SIZE_SFXK	PED_FKEY_LEN+1
#define	PKEY11_ADDR_SFXK	PKEY2_ADDR_SFXK+PKEY2_SIZE_SFXK
#define	PKEY22_SIZE_SFXK	PED_FKEY_LEN+1
#define	PKEY22_ADDR_SFXK	PKEY11_ADDR_SFXK+PKEY11_SIZE_SFXK
#define	SKEY_SIZE_SFXK		PED_FKEY_LEN+1
#define	SKEY_ADDR_SFXK		PKEY22_ADDR_SFXK+PKEY22_SIZE_SFXK
#define	MKEY_SIZE_SFXK		PED_FKEY_LEN+1
#define	MKEY_ADDR_SFXK		SKEY_ADDR_SFXK+SKEY_SIZE_SFXK

UINT8	*temp = (UINT8 *)TEMP_ADDR_SFXK;
UINT8	*buf = (UINT8 *)BUF_ADDR_SFXK;
UINT8	*pkey1 = (UINT8 *)PKEY1_ADDR_SFXK;
UINT8	*pkey2 = (UINT8 *)PKEY2_ADDR_SFXK;

UINT8	*pkey11 = (UINT8 *)PKEY11_ADDR_SFXK;
UINT8	*pkey22 = (UINT8 *)PKEY22_ADDR_SFXK;
UINT8	*skey = (UINT8 *)SKEY_ADDR_SFXK;
UINT8	*mkey = (UINT8 *)MKEY_ADDR_SFXK;
#else
UINT8	temp[KEY_DATA_LEN+1];
UINT8	buf[KEY_BUNDLE_LEN];
UINT8	pkey1[PED_FKEY_LEN+1];
UINT8	pkey2[PED_FKEY_LEN+1];

UINT8	pkey11[PED_FKEY_LEN+1];
UINT8	pkey22[PED_FKEY_LEN+1];
UINT8	skey[PED_FKEY_LEN+1];
UINT8	mkey[PED_FKEY_LEN+1];
#endif

UINT32	i, j;
UINT32	flag;
UINT8	MkeyIndex;
UINT8	result;
UINT8	mac8[8];
	
	
	result = apiFailed;
	  
	if( (index < MAX_FKEY_CNT) && (length == KEY_BUNDLE_LEN) )
	  {
	  if( TR31_VerifyKeyBundle( length, key, temp, mac8 ) )
	    {
	    flag = FALSE;
	    
	    if( TR31_DecryptKeyBundle( mac8, temp, pkey1 ) )
	      {
	      // enforce the key value different from key encryption keys (KPK)
	      // PATCH: 2009-04-07
	      OS_SECM_GetData( ADDR_PED_TDES_KEY_PROTECT_KEY, PED_TDES_KEY_PROTECT_KEY_LEN+1, pkey2 );
	
	      for( i=0; i<pkey1[0]; i++ )
	         {
	         pkey1[i+1] &= 0xFE; // ignore parity bit of input KEY
	         pkey2[i+1] &= 0xFE; // ignore parity bit of KPK
	         }

	      if( LIB_memcmp( pkey1, pkey2, pkey1[0]+1 ) == 0 ) // compared with KPK
	        flag = FALSE;
	      else
	        {
	        // enforce the key value different from other fixed keys
	        flag = TRUE;
	      
	        for( i=0; i<MAX_FKEY_CNT; i++ )
	           {
	           if( i != index )
	             {
	             OS_SECM_GetData( ADDR_PED_FKEY_01+(i*PED_FKEY_SLOT_LEN), PED_FKEY_SLOT_LEN, buf );
	             if( TR31_VerifyKeyBundle( KEY_BUNDLE_LEN, buf, temp, mac8 ) )
	               {
	               if( TR31_DecryptKeyBundle( mac8, temp, pkey2 ) )
	                 {	                 
	                 for( j=0; j<pkey2[0]; j++ )
	                    pkey2[j+1] &= 0xFE; // ignore parity bit of fixed key
	                    	                    
	                 if( LIB_memcmp( pkey1, pkey2, pkey1[0]+1 ) == 0 ) // compared with other fixed key
	                   {
	                   flag = FALSE;
	                   break;
	                   }
	                 }
	               }
	             }
	           } // for(i)
	        }
	      } // if( TR31_DecryptKeyBundle()

	    // 2014-07-27, enforce the key value different from other session keys
	    if( flag )
	      {
	      PED_ReadMKeyIndex( (UINT8 *)&MkeyIndex );
	      OS_SECM_GetData( ADDR_PED_MKEY_01+(MkeyIndex*PED_MSKEY_SLOT_LEN), PED_MSKEY_SLOT_LEN, buf );

	      // verify MKEY bundle
	      if( TR31_VerifyKeyBundle( KEY_BUNDLE_LEN, buf, temp, mac8 ) )
	        {
	        // retrieve MKEY
	        if( TR31_DecryptKeyBundle( mac8, temp, pkey22 ) )
	          {
	          memmove( mkey, pkey22, sizeof(mkey) );
	          
	          for( i=0; i<MAX_SKEY_CNT; i++ )
	             {
	             OS_SECM_GetData( ADDR_PED_SKEY_01+(i*PED_MSKEY_SLOT_LEN), PED_MSKEY_SLOT_LEN, buf );
	             if( TR31_VerifyKeyBundle( KEY_BUNDLE_LEN, buf, temp, mac8 ) )
	               {
	               if( TR31_DecryptKeyBundle( mac8, temp, pkey11 ) )		// pkey11=ESKEY
	                 {
	                 // retrieve SKEY from MKEY
	          	 PED_TripleDES2( &mkey[1], pkey11[0], &pkey11[1], pkey22 );	// pkey22=SKEY

	                 for( j=0; j<pkey11[0]; j++ )
	                    pkey22[j] &= 0xFE; // ignore parity bit of session key
	                    
	                 if( LIB_memcmp( &pkey1[1], pkey22, pkey11[0] ) == 0 ) // compared with session key
	                   {
	                   flag = FALSE;
	                   break;
	                   }
	                 }
	               }
	             } // for(MAX_SKEY_CNT)
	          }
	        }
	      }

#if 0
	    // 2019-04-11, enforce the key value different from AES key
	    if( flag )
	      {
	      OS_SECM_GetData( ADDR_PED_ISO4_AES_KEY, PED_ISO4_AES_KEY_SLOT_LEN, buf );
	      if( PED_VerifyISO4KEY( buf ) )
	      	{
	      	for( j=0; j<PED_ISO4_AES_KEY_LEN; j++ )
	      	   buf[j+1] &= 0xFE; // ignore parity bit of AES key
	      	   
	      	if( LIB_memcmp( &pkey1[1], &buf[1], PED_ISO4_AES_KEY_LEN ) == 0 ) // compared with AES key
	      	  flag = FALSE;
	      	}
	      }
#endif

	    // save it
	    if( flag )
	      {
	      OS_SECM_PutData( ADDR_PED_FKEY_01+(index*PED_FKEY_SLOT_LEN), PED_FKEY_SLOT_LEN, key );
	      result = apiOK;
	      }
	    }
	  }
	
	// PATCH: 2009-04-07, clear sensitive data
#ifdef	_USE_IRAM_PARA_
	memset( temp, 0x00, sizeof(UINT8)*TEMP_SIZE_SFXK );
	memset( buf, 0x00, sizeof(UINT8)*BUF_SIZE_SFXK );
	memset( pkey1, 0x00, sizeof(UINT8)*PKEY1_SIZE_SFXK );
	memset( pkey2, 0x00, sizeof(UINT8)*PKEY2_SIZE_SFXK );
	memset( pkey11, 0x00, sizeof(UINT8)*PKEY11_SIZE_SFXK );
	memset( pkey22, 0x00, sizeof(UINT8)*PKEY22_SIZE_SFXK );
	memset( skey, 0x00, sizeof(UINT8)*SKEY_SIZE_SFXK );
	memset( mkey, 0x00, sizeof(UINT8)*MKEY_SIZE_SFXK );
#else
	memset( temp, 0x00, sizeof(temp) );	
	memset( buf, 0x00, sizeof(buf) );
	memset( pkey1, 0x00, sizeof(pkey1) );
	memset( pkey2, 0x00, sizeof(pkey2) );
	memset( pkey11, 0x00, sizeof(pkey11) );
	memset( pkey22, 0x00, sizeof(pkey22) );
	memset( skey, 0x00, sizeof(skey) );
	memset( mkey, 0x00, sizeof(mkey) );
#endif
	return( result );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To setup DUKPT key.
// INPUT   : mode  - 0=DES, 1=TDES (default).
// 	         eipek - encrypted initial PIN encryption key.
//		     (EIPEK(80):in ANSI TR-31 key bundle format)
//	         ksn   - key serial number. (MUST be 10 bytes)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// NOTE	   : The 21 future keys will be setup after successful execution.
// ---------------------------------------------------------------------------
UINT8	PED_SetDUKPT(UINT8 mode, UINT8 *eipek, UINT8 *iksn)
{
	UINT8	Shift_Reg[3];
    UINT8	temp[X9143_TDES_KEY_DATA_LEN + 1];	// L+V  // ==== [Debug] ====
	//UINT8	temp[KEY_DATA_LEN + 1];	// L+V
	UINT8	ipek[FUTURE_KEY_SLOT_LEN];
	UINT8	pkey[PED_TDES_KEY_PROTECT_KEY_LEN + 1];
	UINT32	i;
	UINT8	result;
	UINT8	mac8[8];


	result = apiFailed;

	//if(TR31_VerifyKeyBundle(KEY_BUNDLE_LEN, eipek, temp, mac8, (UINT8 *)0 ))	// verify key bundle
    if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, eipek, temp, mac8, (UINT8 *)0 ))	// verify key bundle
	{
		//if(TR31_DecryptKeyBundle(mac8, temp, ipek, (UINT8 *)0 ))		// retrieve IPEK
        if(X9143_DecryptKeyBundle_TDES(mac8, temp, ipek, (UINT8 *)0 ))		// retrieve IPEK
		{
			// enforce the key value different from key encryption keys (KPK)
			memmove(temp, ipek, PED_TDES_KEY_PROTECT_KEY_LEN + 1);	// backup original IPEK to TEMP
			OS_SECM_GetData(ADDR_PED_TDES_KEY_PROTECT_KEY, PED_TDES_KEY_PROTECT_KEY_LEN + 1, pkey);

			for(i = 0; i < ipek[0]; i++)
			{
				ipek[i + 1] &= 0xFE; // ignore parity bit of input KEY
				pkey[i + 1] &= 0xFE; // ignore parity bit of KPK
			}

            printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);    // ==== [Debug] ====
            // ==== [Debug] ====
            printf("ipek = ");
            for(int j = 0 ; j < ipek[0] + 1 ; j++)
                printf("%02x", ipek[j]);
            printf("\n");
            // ==== [Debug] ====

			if(LIB_memcmp(ipek, pkey, ipek[0] + 1) != 0) // compared with KPK
			{
				if(DUKPT_LoadInitialKey(0, 0, Shift_Reg, &temp[1], iksn)) // setup DUKPT keys
					result = apiOK;
			}
}
	  }

	// clear sensitive data
	memset(Shift_Reg, 0x00, sizeof(Shift_Reg));
	memset(temp, 0x00, sizeof(temp));
	memset(ipek, 0x00, sizeof(ipek));
	memset(pkey, 0x00, sizeof(pkey));
	
	return(result);
}

// ---------------------------------------------------------------------------
// FUNCTION: To setup AES DUKPT key.
// INPUT   : eipek - encrypted initial PIN encryption key.
//		     (EIPEK(144):in ANSI X9.143 AES key bundle format)
//           eiksn - encrypted initial key serial number.
//		     (EIKSN(144):in ANSI X9.143 AES key bundle format)
//	         (ksn must be 12 bytes.)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// NOTE	   : The 32 future keys will be setup after successful execution.
// ---------------------------------------------------------------------------
// UINT8	PED_SetAESDUKPT(UINT8 *eipek, UINT8 *iksn)
UINT8	PED_SetAESDUKPT(UINT8 *eipek, UINT8 *eiksn)
{
	UINT8	Shift_Reg[3];
    UINT8	temp[X9143_AES_KEY_DATA_LEN + 1];	// L+V
    UINT8	ipek[MAX_KEYLENGTH + 1];
    UINT8   temp2[X9143_AES_KEY_DATA_LEN + 1];  // L+V
    UINT8   iksn[12 + 1];
	UINT8	pkey[PED_256_AES_KEY_PROTECT_KEY_LEN + 1];
	UINT32	i;
	UINT8	result;
	UINT8	mac16[16];


	result = apiFailed;

    if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, eipek, temp, mac16, (UINT8 *)0 ))	// verify key bundle
	{
        if(X9143_DecryptKeyBundle_AES(mac16, temp, ipek, (UINT8 *)0 ))		// retrieve IPEK
		{
			// enforce the key value different from key encryption keys (KPK)
            if(ipek[0] == PED_128_AES_KEY_PROTECT_KEY_LEN)
            {
                memmove(temp, ipek, PED_128_AES_KEY_PROTECT_KEY_LEN + 1);	// backup original IPEK to TEMP
                OS_SECM_GetData(ADDR_PED_128_AES_KEY_PROTECT_KEY, PED_128_AES_KEY_PROTECT_KEY_LEN + 1, pkey);	// pkey2=KPK
            }
            else if(ipek[0] == PED_192_AES_KEY_PROTECT_KEY_LEN)
            {
                memmove(temp, ipek, PED_192_AES_KEY_PROTECT_KEY_LEN + 1);	// backup original IPEK to TEMP
                OS_SECM_GetData(ADDR_PED_192_AES_KEY_PROTECT_KEY, PED_192_AES_KEY_PROTECT_KEY_LEN + 1, pkey);	// pkey2=KPK
            }
            else if(ipek[0] == PED_256_AES_KEY_PROTECT_KEY_LEN)
            {
                memmove(temp, ipek, PED_256_AES_KEY_PROTECT_KEY_LEN + 1);	// backup original IPEK to TEMP
                OS_SECM_GetData(ADDR_PED_256_AES_KEY_PROTECT_KEY, PED_256_AES_KEY_PROTECT_KEY_LEN + 1, pkey);	// pkey2=KPK
            }

			for(i = 0 ; i < ipek[0] ; i++)
			{
				ipek[i + 1] &= 0xFE; // ignore parity bit of input KEY
				pkey[i + 1] &= 0xFE; // ignore parity bit of KPK
			}

            printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);    // ==== [Debug] ====
            // ==== [Debug] ====
            printf("ipek = ");
            for(int j = 0 ; j < ipek[0] + 1 ; j++)
                printf("%02x", ipek[j]);
            printf("\n");
            // ==== [Debug] ====

			if(LIB_memcmp(ipek, pkey, ipek[0] + 1) != 0) // compared with KPK
			{
                // erase the current intermediate derivation key
                PED_EraseAesDUKPT();
                
                if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, eiksn, temp2, mac16, (UINT8 *)0 ))	// verify key bundle
                {
                    if(X9143_DecryptKeyBundle_AES(mac16, temp2, iksn, (UINT8 *)0 ))		// retrieve IKSN
                    {
                        // ==== [Debug] ====
                        printf("iksn = ");
                        for(int k = 0 ; k < iksn[0] + 1 ; k++)
                            printf("%02x", iksn[k]);
                        printf("\n");
                        // ==== [Debug] ====

                        if(ipek[0] == PED_128_AES_KEY_PROTECT_KEY_LEN)
                        {
                            AES_DUKPT_LoadInitialKey(&temp[1], 2, &iksn[1]);    // AES-128
                        }
                        else if(ipek[0] == PED_192_AES_KEY_PROTECT_KEY_LEN)
                        {
                            AES_DUKPT_LoadInitialKey(&temp[1], 3, &iksn[1]);    // AES-192
                        }
                        else if(ipek[0] == PED_256_AES_KEY_PROTECT_KEY_LEN)
                        {
                            AES_DUKPT_LoadInitialKey(&temp[1], 4, &iksn[1]);    // AES-256
                        }

                        result = apiOK;
                    }
                }
			}
        }
	}

	// clear sensitive data
	memset(Shift_Reg, 0x00, sizeof(Shift_Reg));
	memset(temp, 0x00, sizeof(temp));
	memset(ipek, 0x00, sizeof(ipek));
    memset(temp2, 0x00, sizeof(temp2));
	memset(iksn, 0x00, sizeof(iksn));
	memset(pkey, 0x00, sizeof(pkey));
	
	return(result);
}

// ---------------------------------------------------------------------------
// FUNCTION: To set timeout for PED keyin entry.
// INPUT   : tout  - timeout in seconds used to wait for PIN entry.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
void	PED_SetKeyInTimeOut( UINT8 tout )
{
	os_ped_tout = tout;
}

// ---------------------------------------------------------------------------
// FUNCTION: To enable PIN pad device.
// INPUT   : dev   - PIN pad device, 0 = internal, 1 = external.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// NOTE    : This function must be called once before requesting
//           any other PIN pad functions.
// ---------------------------------------------------------------------------
UINT8	PED_OpenPinPad( void )
{
	os_ped_tout = 0;
	
	if( os_ped_dhn_tim != 0 )
	  {
	  api_tim_close( os_ped_dhn_tim );
	  os_ped_dhn_tim = 0;
	  }
	  
	return( apiOK );
}

// ---------------------------------------------------------------------------
// FUNCTION: To set or reset sensitive service time. (max 15min)
// INPUT   : flag - TRUE  = set (enable)
//		    FALSE = reset (disable)
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_SetSensitiveServiceTime( UINT32 flag )
{
	os_ped_SensitiveServiceTime = PED_SENSITIVE_SERVICE_TOUT;
	os_ped_KeyPress = FALSE;
	
	PED_InUse( flag );
}

// ---------------------------------------------------------------------------
// FUNCTION: To reset PIN pad device.
// INPUT   : tout  - timeout in seconds used to wait for PIN entry.
//           type  - transaction type. (RFU)
//           amt   - amount to be confirmed on display. (external)
//                   format: LEN(1) + ASCII(n)
//                           LEN = 0: no confirmation.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// NOTE    : This function must be called before requesting PIN
//           entry (api_ped_GetPin()).
// ---------------------------------------------------------------------------
void	PED_ResetPinPad( void )
{
UINT8	pinlen;

	// PATCH: 2009-04-07
//	OS_SECM_GetData( ADDR_PED_PIN, 1, (UINT8 *)&pinlen );
//	if( pinlen )
//	  os_ped_DownCnt = PED_PIN_INTERVAL_TOUT;	// limit pin entry if the PIN data is available before power-on

	// PATCH: 2009-04-29
	os_ped_DownCnt = PED_PIN_INTERVAL_TOUT;		// always limit pin entry at each system reboot (the 1'st PIN entry allowed is 30s later)

	// OS_SECM_ClearData( ADDR_PED_PIN, PED_PIN_SLOT_LEN, 0 );
    if(ped_pin_ptr != NULL)
        PED_ClearPin();
	
	os_ped_state = PED_STATE_START;
	
	os_ped_tout = 0;
	
	PED_SetSensitiveServiceTime( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: To request PIN entry from PIN pad device.
// INPUT   : none.
// OUTPUT  : keyin - value of the key-in digit.
//                   For INTERNAL PIN pad.
//                   Only the following characters will be
//                   returned to AP.
//                   (1) '*' for '0' to '9' keys.
//                   (2) CLEAR, CANCEL, BACK-SPACE or ENTER key.
//                   (3) NULL if only ENTER key is pressed.
//                   (4) 0xFF if PIN length less than 4 or over 12 digits.
//                   For EXTERNAL PIN pad.
//                   Only CANCEL or ENTER key will be
//                   returned to AP.
// RETURN  : apiOK         (key-in digit available)
//           apiFailed     (timeout or invalid PIN length)
// NOTE    : The "api_ped_ResetPinPad()" shall be called prior to
//           invoking this function.
// REF     : os_ped_tout
//           os_ped_state
// ---------------------------------------------------------------------------
UINT8	PED_GetPinEntry( UINT8 *keyin )
{
//UINT8	kek[16];
UINT8	pin[PED_PIN_SLOT_LEN];
UINT8	temp[PED_PIN_SLOT_LEN];
UINT8	key;
UINT8	pinlen;


      // init default KEK
//    PED_GetIKEK( kek );

      switch( os_ped_state )
            {
            case PED_STATE_START:

                //  OS_SECM_ClearData( ADDR_PED_PIN, PED_PIN_SLOT_LEN, 0 );
                 memset(ped_pin_ptr, 0x00, PED_PIN_SLOT_LEN);
                 
                 if( os_ped_dhn_tim != 0 )
                   {
                   api_tim_close( os_ped_dhn_tim );
                   os_ped_dhn_tim = 0;
                   }

                 os_ped_dhn_tim = api_tim_open(100); // time tick = 1sec
                 if( os_ped_dhn_tim != apiOutOfService )
                   api_tim_gettick( os_ped_dhn_tim, (UCHAR *)&os_ped_tick1 );
                 else
                   {
                   api_tim_close( os_ped_dhn_tim );
                   os_ped_dhn_tim = 0;

                   os_ped_state = PED_STATE_END;
                   return( apiFailed );
                   }

                 // go throuth next state for waiting keyin

            case PED_STATE_WAIT_KEY:

                 // wait key & check time out
                 do{
                   api_tim_gettick( os_ped_dhn_tim, (UCHAR *)&os_ped_tick2 );
                   if( (os_ped_tick2 - os_ped_tick1) >= os_ped_tout )
                     {
                     api_tim_close( os_ped_dhn_tim );
                     os_ped_dhn_tim = 0;
                     
                    //  OS_SECM_ClearData( ADDR_PED_PIN, PED_PIN_SLOT_LEN, 0 );
                     memset(ped_pin_ptr, 0x00, PED_PIN_SLOT_LEN);

                     os_ped_state = PED_STATE_END;
                     return( apiFailed );
                     }
                   } while( LIB_GetKeyStatus() != apiReady );

                 // key stroked
//               key = LIB_WaitMuteKey();
		 key = LIB_WaitKey();

                 if( (key >= 0x30) && (key <= 0x39) ) // '0'..'9'
                   {
                //    OS_SECM_GetData( ADDR_PED_PIN, PED_PIN_SLOT_LEN, temp );
                   memcpy(temp, ped_pin_ptr, PED_PIN_SLOT_LEN);
                   memset( pin, 0x00, sizeof(pin) );

                   pinlen = temp[0];
                   if( pinlen < 12 )
                     {
                     if( os_ped_state == PED_STATE_WAIT_KEY )
                       {
                       // decrypt PIN by using default KEK (excluding PINLEN)
                       if( pinlen )
//                       PED_TripleDES2( kek, 16, (UCHAR *)&temp[1], &pin[1] );
		         memmove( &pin[1], &temp[1], PED_PIN_LEN );	// plaintext format
                       }

                     pinlen++;

                     temp[0] = pinlen;
                     pin[0] = pinlen;
                     pin[pinlen] = key;

                     // encrypt PIN by using default KEK (excluding PINLEN)
//                   PED_TripleDES( kek, 16, &pin[1], (UCHAR *)&temp[1] );
		     memmove( &temp[1], &pin[1], PED_PIN_LEN );
                    //  OS_SECM_PutData( ADDR_PED_PIN, PED_PIN_SLOT_LEN, temp );
                     memcpy(ped_pin_ptr, temp, PED_PIN_SLOT_LEN);

//                   os_ped_state = PED_STATE_WAIT_KEY;
                     *keyin = 0x2A; // asterisk
//                   return( apiOK );
                     }
                   else
                     {
                     // PIN length over 12 digits
//                   os_ped_state = PED_STATE_WAIT_KEY;
                     *keyin = 0xFF;
//                   return( apiOK );
                     }
                     
                   // PATCH: 2009-04-06, clear sensitive data
                   memset( temp, 0x00, sizeof(temp) );
                   memset( pin , 0x00, sizeof(pin) );
                   key = 0;
                   pinlen = 0;
                   
                   os_ped_state = PED_STATE_WAIT_KEY;
                   return( apiOK );
                   }

                 if( key == 'n' ) // CLEAR
                   {
                //    OS_SECM_ClearData( ADDR_PED_PIN, PED_PIN_SLOT_LEN, 0 );
                   memset(ped_pin_ptr, 0x00, PED_PIN_SLOT_LEN);

                   *keyin = key;
                   
                   // PATCH: 2009-04-06, clear sensitive data
                   memset( temp, 0x00, sizeof(temp) );
                   memset( pin , 0x00, sizeof(pin) );
                   key = 0;
                   pinlen = 0;
                                      
                   return( apiOK );
                   }

                 if( key == 'x' ) // CANCEL
                   {
                   api_tim_close( os_ped_dhn_tim );
                   os_ped_dhn_tim = 0;
                   
                //    OS_SECM_ClearData( ADDR_PED_PIN, PED_PIN_SLOT_LEN, 0 );
                   memset(ped_pin_ptr, 0x00, PED_PIN_SLOT_LEN);


                   *keyin = key;

                   // PATCH: 2009-04-06, clear sensitive data
                   memset( temp, 0x00, sizeof(temp) );
                   memset( pin , 0x00, sizeof(pin) );
                   key = 0;
                   pinlen = 0;
                   
                   return( apiOK );
                   }

                 if( key == '#' ) // BACK-SPACE
                   {
                //    OS_SECM_GetData( ADDR_PED_PIN, 1, temp );
                   temp[0] = *ped_pin_ptr;
                   if( temp[0] != 0 )
                     {
                     temp[0] -= 1;
                    //  OS_SECM_PutData( ADDR_PED_PIN, 1, temp );
                     *ped_pin_ptr = temp[0];
                     }
                   
                   *keyin = key;

                   // PATCH: 2009-04-06, clear sensitive data
                   memset( temp, 0x00, sizeof(temp) );
                   memset( pin , 0x00, sizeof(pin) );
                   key = 0;
                   pinlen = 0;

                   return( apiOK );
                   }

                 if( key == 'y' ) // ENTER
                   {
                //    OS_SECM_GetData( ADDR_PED_PIN, 1, temp );
                   temp[0] = *ped_pin_ptr;

                   if( temp[0] == 0 ) // bypass PIN
                     {
                     api_tim_close( os_ped_dhn_tim );
                     os_ped_dhn_tim = 0;
                     
                     os_ped_state = PED_STATE_END;
                     *keyin = 0x00;   // NULL
//                   return( apiOK ); // done
                     }
		   else
		     {
                     if( temp[0] < 4 )
                       {
                       // PIN length less than 4 digits
                       os_ped_state = PED_STATE_WAIT_KEY;
                       *keyin = 0xFF;
                       LIB_BUZ_Beep2();	// 2010-03-22
//                     return( apiOK );
                       }
                     else
                       {
                       api_tim_close( os_ped_dhn_tim );
                       os_ped_dhn_tim = 0;
                     
                       os_ped_state = PED_STATE_PIN_READY; // ready to generate PIN block
                       *keyin = key;
//                     return( apiOK ); // done
                       }
                     }
		   
		   // PATCH: 2009-04-06, clear sensitive data
                   memset( temp, 0x00, sizeof(temp) );
                   memset( pin , 0x00, sizeof(pin) );
                   key = 0;
                   pinlen = 0;
                   
		   return( apiOK );
                   }

            default: // PED_STATE_END or PED_STATE_PIN_READY

		 // PATCH: 2009-04-06, clear sensitive data
                 memset( temp, 0x00, sizeof(temp) );
                 memset( pin , 0x00, sizeof(pin) );
                 key = 0;
                 pinlen = 0;

                //  OS_SECM_ClearData( ADDR_PED_PIN, PED_PIN_SLOT_LEN, 0 );
                 memset(ped_pin_ptr, 0x00, PED_PIN_SLOT_LEN);
                 os_ped_state = PED_STATE_END;
                 return( apiFailed );
            }
}

// ---------------------------------------------------------------------------
// FUNCTION: PED task monitor, called by system timer every 10ms periodically.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_PED_Task( void )
{	
UINT32	data;


	//PAN encrypting interval
	if(os_ped_PanDownCnt)
	{
		os_ped_PanDownCnt--;
	}

	os_ped_PanUpCnt++;

    // PAN life cycle
    if(os_ped_PanLife)
    {
        os_ped_PanLife--;
        if(os_ped_PanLife == 0)
            api_sram_clear(SRAM_BASE_EMV_ICC + ADDR_ICC_AP_PAN, 12, 0x00);
    }

	// PIN interval
	if( os_ped_DownCnt )
	  os_ped_DownCnt--;
	  
	os_ped_UpCnt++;
	
	// PIN life cycle
	if( os_ped_PinLife )
	  {
	  os_ped_PinLife--;
	  if( os_ped_PinLife == 0 )
	    PED_ResetPinPad();
	  }
	  
	// PED activity checking
	if( os_ped_activity == FALSE )
	  {
	  if( os_ped_IdleCnt == 0 )
	    {
	    // system warm reset to non-sensitive mode
	    LIB_ResetSystem();
	    }
	  else
	    os_ped_IdleCnt--;	// down-counter for self test
	  }
	else
	  {
	  if( os_ped_IdleCnt != 0 )	// 2022-02-08
	    os_ped_IdleCnt--;	// down-counter for self test
	  }
	  
	// sensitive services time out checking
	if( os_ped_activity == TRUE )
	  {
	  if( os_ped_KeyPress )
	    {
	    if( os_ped_SensitiveServiceTime == 0 )
	      {
	      // system warm reset to non-sensitive mode
	      LIB_ResetSystem();
	      }
	    else
	      os_ped_SensitiveServiceTime--;
	    }
	  }
}

// ---------------------------------------------------------------------------
// FUNCTION: Setup activity status for PED.
// INPUT   : activity - status.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_InUse( UINT32 status )
{
    if((status == FALSE) && (os_ped_IdleCnt == 0))
    {
        os_ped_activity = status;
        for(;;);    // stop here and wait for reset
    }

    os_ped_activity = status;
}

// ---------------------------------------------------------------------------
// FUNCTION: To request PIN entry from PIN pad device.
// INPUT   : amt   - total amount to be confirmed on display. (external)
//                   format: LEN(1) + ASCII(n)
//	         tout  - PIN entry timeout in seconds.
//
//	         0123456789012345
//	         TOTAL $NNNNNNNNN
//	     
//	         ENTER PIN:
//                     ******
//
// OUTPUT  : none.
// RETURN  : apiOK       - PIN entered
//	         apiNotReady - PIN bypassed
//           apiFailed   - timeout or illegal entry
//	         apiOutOfService - aborted by cancel key.
//
// POLICY  : (1) Total entry timeout = 30 sec
//	         (2) Legal PIN length: 4..12 digits
//           (3) Min get PIN interval = 30 sec
// ---------------------------------------------------------------------------
UINT8	PED_GetPin( UINT16 tout, UINT8 *amt )
{
	//UINT8	const str_TOTAL[] =	{"TOTAL"};
	//UINT8	const str_ENTER_PIN[] =	{"ENTER PIN:"};
	UINT8	keyin;
	UINT8	status;
	UINT8	index;
	UINT8	pinbuf[16];
	UINT8	kat[KBD_KAT_SIZE];
	UINT32	flag;
	API_LCDTFT_PARA para;
	API_LCDTFT_PARA	para2;


	//	if( amt[0] > 10 )
	//	  return( apiFailed );

	flag = TRUE;		// 2010-04-18, not show amount if its length > 16 digits
	if(amt[0] > 16)	//
		flag = FALSE;		//

	  // make sure system timer is active  
	os_ped_UpCnt = 0;
	LIB_WaitTime(2);
	while(os_ped_UpCnt == 0);

	// check get PIN interval
	if(os_ped_DownCnt)
		return(apiFailed);

	PED_InUse(TRUE);

	PP_SetupPinPad(&para);	// get PIN pad settings for PIN entry function from APP layer
	memmove(&para2, &para, sizeof(API_LCDTFT_PARA));	// para for clear function
	para2.Col = 1;

	// enable PED sound
	pinbuf[0] = 1;
	pinbuf[1] = 5;
	pinbuf[2] = 5;
	post_dhn_buz1 = api_buz_open(pinbuf);	// sound for OK

	pinbuf[0] = 1;				// 2010-03-22
	pinbuf[1] = 10;
	pinbuf[2] = 10;
	post_dhn_buz2 = api_buz_open(pinbuf);	// sound for ERROR

	LIB_LCD_Cls();

	// show TOTAL $NNNNNNNN
//	if( amt[0] != 0 )
//	  {
//	  LIB_LCD_PutMsg( 0, COL_LEFTMOST, FONT1, sizeof(str_TOTAL)-1, (UINT8 *)str_TOTAL );
//	  LIB_LCD_PutMsg( 0, COL_RIGHTMOST, FONT1, amt[0], &amt[1] );
//	  }
//	
//	LIB_LCD_PutMsg( 2, COL_LEFTMOST, FONT1, sizeof(str_ENTER_PIN)-1, (UINT8 *)str_ENTER_PIN );

	PP_show_enter_pin(amt);

	// store current keypad status
	OS_KBD_GetKAT(kat);
	//	LIB_DumpHexData( 0, 0, 5, kat );

		// enable PED
	PED_ResetPinPad();
	LIB_OpenKeyNum(); 		// enable numeric keypad
	PED_SetKeyInTimeOut(tout);	// setup timeout = 30 sec
    PED_InUse(TRUE);

//	LIB_BUZ_Beep1();			// 2009-11-10
//	LIB_LCD_Putc( 3, 0, FONT1, '_' );	// show cursor

    ped_pin_ptr = calloc(PED_PIN_SLOT_LEN, sizeof(UINT8));
    if(ped_pin_ptr == NULL)
        return apiFailed;

	index = 0;
	flag = TRUE;
	// Get PIN Process
	while(flag)
	{
		status = PED_GetPinEntry((UINT8 *)&keyin);
		if(status == apiFailed)
        {
            if(ped_pin_ptr != NULL)
                PED_ClearPin();
			break;
        }

		switch(keyin)
		{
			case '*':	// digit 0..9

				pinbuf[index++] = keyin;
				//	                LIB_LCD_PutMsg( 3, COL_LEFTMOST, FONT1+attrCLEARWRITE, index, pinbuf );
				LIB_LCDTFT_PutStr(para, index, pinbuf);
				break;

			case 'n':	// CLEAR

				index = 0;
				//	                LIB_LCD_ClearRow( 3, 1, FONT1 );
				//	                LIB_LCD_Putc( 3, 0, FONT1, '_' );	// show cursor
				LIB_LCDTFT_ClearRow(para2);
				break;

			case '#':	// BACK-SPACE

				if(index)
				{
					index--;

					if(index == 0)
					{
						//	           	    LIB_LCD_ClearRow( 3, 1, FONT1 );
						//	           	    LIB_LCD_Putc( 3, 0, FONT1, '_' );	// show cursor
						LIB_LCDTFT_ClearRow(para2);
					}
					else
						//	           	    LIB_LCD_PutMsg( 3, COL_LEFTMOST, FONT1+attrCLEARWRITE, index, pinbuf );
						LIB_LCDTFT_PutStr(para, index, pinbuf);
				}
				break;

			case 'y':	// ENTER

                flag = FALSE;
				break;

			case 'x':	// CANCEL

				status = apiOutOfService;	// 2010-03-22, to terminate trans.
				flag = FALSE;
				break;

			case 0x00:	// bypass

				status = apiNotReady;
				flag = FALSE;
				break;

			case 0xFF:	// illegal length

				break;
		}
	}

	if(status == apiOK)
	{
		os_ped_DownCnt = PED_PIN_INTERVAL_TOUT;	// setup min get PIN interval = 30sec
		os_ped_PinLife = PED_PIN_LIFE_TOUT;		// setup effective PIN life cycle = 15sec
	}

	// restore original keypad status
	OS_KBD_SetKAT(kat);

	LIB_LCD_Cls();

	// disable PED sound
	BSP_Delay_n_ms(50);	// wait for end of current beep sound before task close
	api_buz_close(post_dhn_buz1);
	api_buz_close(post_dhn_buz2);

	PED_InUse(FALSE);

	return(status);
}

// ---------------------------------------------------------------------------
// FUNCTION: To clear PIN data and deallocate the memory.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void    PED_ClearPin(void)
{
    memset(ped_pin_ptr, 0x00, PED_PIN_SLOT_LEN);
    free(ped_pin_ptr);
}
