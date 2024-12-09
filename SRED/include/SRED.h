//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : i.MX6UL                                                    **
//**  PRODUCT  : AS350-X6                                                   **
//**                                                                        **
//**  FILE     : SRED.H                                                     **
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
#ifndef _SRED_H_
#define _SRED_H_

//----------------------------------------------------------------------------
#define SRED_BUFFER_SIZE		256
#define PED_PAN_ENCRYPT_TOUT	30*100
#define PED_PAN_LIFE_TOUT       30*100

//----------------------------------------------------------------------------
//	RTC structure
//----------------------------------------------------------------------------
typedef struct SRED_RTC_S
{
	UINT8	year[2];
	UINT8	month[2];
	UINT8	day[2];
	UINT8	hour[2];
	UINT8	minute[2];
	UINT8	second[2];
}SRED_RTC;

//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
extern	void	SRED_3DesEcb_Encrypt(UINT8 *dataOut, UINT32 *outLen, UINT8 *dataIn, UINT32 inLen, UINT8 *key);
extern	void	SRED_3DesEcb_Decrypt(UINT8 *dataOut, UINT32	*outLen, UINT8 *dataIn, UINT32 inLen, UINT8 *key);
extern	void	SRED_3DesCbc_Encrypt(UINT8 *dataOut, UINT32 *outLen, UINT8 *dataIn, UINT32 inLen, UINT8 *key, UINT8 *iv);
extern	void	SRED_3DesCbc_Decrypt(UINT8 *dataOut, UINT32	*outLen, UINT8 *dataIn, UINT32 inLen, UINT8 *key, UINT8 *iv);
extern  void    SRED_AesEcb_Encrypt(UINT8 *dataOut, UINT32 *outLen, UINT8 *dataIn, UINT32 inLen, UINT8 *key);
extern  void    SRED_AesEcb_Decrypt(UINT8 *dataOut, UINT32 *outLen, UINT8 *dataIn, UINT32 inLen, UINT8 *key);
extern	UINT8	SRED_GetAccDek(UINT8 *accDek);
extern	UINT8	SRED_GetFPEKey(UINT8 *FPEKey);
//extern	UINT8	SRED_MutualAuthentication(void);
extern	UINT8	SRED_EncryptThenMAC(UINT8 pedKey, UINT8 keyIndex, UINT8 ap, UINT8 mode, UINT8 *s, UINT8 *dataIn, UINT32 inLen, UINT8 *dataOut, UINT32 *outLen);
extern	void	SRED_DataOriginAuthentication(void);
extern	UINT8	SRED_GenerateSurrogatePAN(UINT8 *pan, UINT8 panLen, UINT8 *surrogate);
extern	void	SRED_GetSalt(UINT8 *salt);
extern	void	SRED_ResetEncryptLimit(void);
extern	void	SRED_EraseSecureData(void);
extern	UINT8	SRED_FXKEY_GetMacKey(UINT8 index, UINT8	*macKey);
extern	UINT8	SRED_MSKEY_GetMacKey(UINT8 index, UINT8	*macKey, UINT8 *keyLen);
extern	UINT8	SRED_DUKPT_GetMacKey(UINT8 *ksn, UINT8 *macKey);
extern  UINT8	SRED_AES_DUKPT_GetMacKey(UINT8 *ksn, UINT8 *macKey, UINT8 *keyLen);
extern	UINT8	SRED_Get_TLVLengthOfT(UINT8 *iptDataOfT, UINT8 *optLenOfT);
extern	UINT8	SRED_SubSplit(UINT8 data);
extern	void	SRED_Split(UINT8 *des, UINT8 *src, char pair_len);
extern	UINT8	SRED_SubCompress(UINT8 data);
extern	void	SRED_Compress(UINT8 *des, UINT8 *src, UINT8 pair_len);
extern	UINT8	SRED_OutputPAN(void);
//extern	UINT8	SRED_EncryptPan(void);
extern	UINT8	SRED_ManualPanKeyEntry(UINT8 *pan, UINT8 *panLen);
extern  UINT8	SRED_AES_DUKPT_setWorkingKeyType(UINT8 mode);

//----------------------------------------------------------------------------
#endif
