//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : i.MX6UL							                        **
//**  PRODUCT  : AS350-X6							                        **
//**                                                                        **
//**  FILE     : OS_PED.H                                                   **
//**  MODULE   : Declaration of PCI PED Module.		                        **
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
#ifndef _OS_PED_H_
#define _OS_PED_H_

//----------------------------------------------------------------------------
//#define	_USE_IRAM_PARA_		// optional for passing parameters by internal secure RAM,
					// if system STACK is located at external RAM

//----------------------------------------------------------------------------
//      SOH LEN(1) KMC(1) INDEX(1) KEY(n) LRC
//----------------------------------------------------------------------------
#define KMC_NEXT							0x80
#define KMC_DOWNLOAD_DUKPT_IPEK				0x00
#define KMC_DOWNLOAD_DUKPT_IKSN				0x01
#define KMC_DOWNLOAD_MASTER_KEY				0x02
#define KMC_DOWNLOAD_SESSION_KEY			0x03
#define KMC_DOWNLOAD_FIXED_KEY				0x04
#define KMC_DOWNLOAD_CAPK					0x05
#define KMC_DOWNLOAD_KEY_PROTECT_KEY		0x06
#define KMC_DOWNLOAD_DYNAMIC_KEK			0x07
#define KMC_DOWNLOAD_IKEK					0x08

#if 0
#define	KMC_DOWNLOAD_AES_KEK				0x0A	// 2019-01-30
#define	KMC_DOWNLOAD_AES_KEY				0x0B	// 2019-01-30
#endif

//#define KMC_DOWNLOAD_ACC_DEK_MASTER_KEY		0x09    //[Debug] remark ====
//#define KMC_DOWNLOAD_ACC_DEK_SESSION_KEY	0x0A    //[Debug] remark ====

#if 0
#define KMC_DOWNLOAD_ISO4_MASTER_KEY		0x0B
#define KMC_DOWNLOAD_ISO4_SESSION_KEY		0x0C

#define KMC_DOWNLOAD_FPE_MASTER_KEY			0x0D
#define KMC_DOWNLOAD_FPE_SESSION_KEY		0x0E
#endif

#define KMC_DOWNLOAD_ISO4_KEY				0x0B
#define KMC_DOWNLOAD_FPE_KEY				0x0C

#define KMC_DOWNLOAD_CAPK					0x00
#define KMC_DOWNLOAD_AES_DUKPT_IPEK			0x01
#define KMC_DOWNLOAD_AES_DUKPT_IKSN			0x02
#define KMC_DOWNLOAD_PEK_MASTER_KEY			0x03
#define KMC_DOWNLOAD_PEK_SESSION_KEY		0x04
#define KMC_DOWNLOAD_PEK_DUKPT_IPEK			0x05
#define KMC_DOWNLOAD_PEK_DUKPT_IKSN			0x06
#define KMC_DOWNLOAD_ACC_DEK_MASTER_KEY		0x07
#define KMC_DOWNLOAD_ACC_DEK_SESSION_KEY	0x08
#define KMC_DOWNLOAD_ACC_DEK_DUKPT_IPEK		0x09
#define KMC_DOWNLOAD_ACC_DEK_DUKPT_IKSN		0x0A
#define KMC_DOWNLOAD_FPE_MASTER_KEY			0x0B
#define KMC_DOWNLOAD_FPE_SESSION_KEY		0x0C
#define KMC_DOWNLOAD_FPE_DUKPT_IPEK			0x0D
#define KMC_DOWNLOAD_FPE_DUKPT_IKSN			0x0E

#define KMC_DEVICE_AUTH_PHASE_1             0x10
#define KMC_DEVICE_AUTH_PHASE_2             0x11


//----------------------------------------------------------------------------
//      PED Key Type
//----------------------------------------------------------------------------
#define TDES_128    0x01
#define TDES_192    0x02
#define AES_128     0x03
#define AES_192     0x04
#define AES_256     0x05

//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
extern	void	PED_InitIKEK( void );
extern	void	PED_EraseIKEK( void );
extern	void	PED_EraseDKEK(void);
extern	void	PED_EraseKEK( void );
extern	void	PED_EraseKPK(UINT8 type);
extern  void	PED_TDES_EraseKPK(UINT8 type);
extern  void	PED_AES_EraseKPK(UINT8 type);
extern	void	PED_EraseMSK( void );
extern  void	PED_ErasePEK(void);
extern	void	PED_EraseDUKPT( void );
extern  void	PED_EraseAesDUKPT(void);
extern	void	PED_EraseFK( void );
extern	void	PED_EraseISO4MasterKey(void);
extern	void	PED_EraseISO4SessionKey(void);
extern	void	PED_EraseISO4KEY(void);
extern	void	PED_EraseAccDEKMasterKey(void);
extern	void	PED_EraseAccDEKSessionKey(void);
extern	void	PED_EraseAccDEK(void);
extern	void	PED_EraseFPEMasterKey(void);
extern	void	PED_EraseFPESessionKey(void);
extern	void	PED_EraseFPEKEY(void);
extern	void	PED_EraseAdminPSW( void );
extern	void	PED_EraseUserPSW( void );
extern	void	PED_EraseCAPK( void );


extern	UINT8	PED_GenerateKEK( UINT8 *tid );
extern	UINT8	PED_LoadInitialKeyEncryptionKey(UINT8 *tid);
extern	UINT8	PED_LoadDynamicKEK(UINT8 *tid);
extern	UINT8	PED_LoadDukptInitKey( UINT8 *tid );
extern  UINT8	PED_LoadAesDukptInitKey(UINT8 *tid);
extern	UINT8	PED_LoadMasterSessionKey( UINT8 *tid );
extern	UINT8	PED_LoadPEKMasterSessionKey( UINT8 *tid );
extern	UINT8	PED_LoadISO4MasterSessionKey(UINT8 *tid);
extern	UINT8	PED_LoadISO4KEY(UINT8 *tid);
extern	UINT8	PED_LoadAccDEKMasterSessionKey(UINT8 *tid);
extern	UINT8	PED_LoadFPEMasterSessionKey(UINT8 *tid);
extern	UINT8	PED_LoadFPEKey(UINT8 *tid);
//extern	UINT8	PED_LoadFixedKey( UINT8 *tid );
extern	UINT8	PED_LoadCaPublicKey( UINT8 *tid );

extern	UINT8	PED_AdministratorMode( void );
extern	UINT8	PED_UserMode( void );
extern	UINT8	PED_ResetMode( void );

extern	void	PED_SetSensitiveServiceTime( UINT32 flag );

extern	void	PED_InUse( UINT32 status );
extern	void	PED_WriteMKeyIndex(UINT8 index);
extern	void	PED_ReadMKeyIndex( UINT8 *index );
extern  void	PED_PEK_WriteMKeyIndex(UINT8 index);
extern  void	PED_PEK_ReadMKeyIndex(UINT8 *index);
extern	void	PED_ISO4_WriteMKeyIndex(UINT8 index);
extern	void	PED_ISO4_ReadMKeyIndex(UINT8 *index);
extern	void	PED_AccDEK_WriteMKeyIndex(UINT8 index);
extern	void	PED_AccDEK_ReadMKeyIndex(UINT8 *index);
extern	void	PED_FPE_WriteMKeyIndex(UINT8 index);
extern	void	PED_FPE_ReadMKeyIndex(UINT8 *index);

extern  void	PED_PEK_SetKeyType(UINT8 keyType);
extern  void	PED_PEK_GetKeyType(UINT8 *keyType);
extern  void	PED_AccDEK_SetKeyType(UINT8 keyType);
extern  void	PED_AccDEK_GetKeyType(UINT8 *keyType);
extern  void	PED_FPE_SetKeyType(UINT8 keyType);
extern  void	PED_FPE_GetKeyType(UINT8 *keyType);
extern  void	PED_AES_DUKPT_SetKeyType(UINT8 keyType);
extern  void	PED_AES_DUKPT_GetKeyType(UINT8 *keyType);

extern	UINT32	PED_des_encipher( UINT8 *pIn, UINT8 *pOut, UINT8 *pKey );
extern	UINT32	PED_des_decipher( UINT8 *pOut, UINT8 *pIn, UINT8 *pKey );
extern	void	PED_TripleDES( UINT8 *key, UINT8 len, UINT8 *idata, UINT8 *odata );
extern	void	PED_TripleDES2( UINT8 *key, UINT8 len, UINT8 *idata, UINT8 *odata );

extern	void	PED_CBC_TripleDES( UINT8 *key, UINT8 *icv, UINT8 len, UINT8 *idata, UINT8 *odata );
extern	void	PED_CBC_TripleDES2( UINT8 *key, UINT8 *icv, UINT8 len, UINT8 *idata, UINT8 *odata );

extern	void	PED_CBC_AES2(UINT8 *key, UINT8 keyLen, UINT8 *icv, UINT8 len, UINT8 *idata, UINT8 *odata);

extern	void	PED_CAPK_GetInfo( UINT8 *info );
extern	UINT8	PED_SetCAPK( UINT32 index, UINT8 *key );

//extern	UINT8	PED_SetFixedKey( UINT8 index, UINT8 length, UINT8 *key );
extern	UINT8	PED_SetSessionKey( UINT8 index, UINT8 length, UINT8 *key );
extern	UINT8	PED_SetMasterKey( UINT8 index, UINT8 length, UINT8 *key );
extern  UINT8	PED_PEK_SetMasterKey(UINT8 index, UINT8 length, UINT8 *key);
extern  UINT8	PED_PEK_SetSessionKey(UINT8 index, UINT8 length, UINT8 *key);
extern	UINT8	PED_SetDUKPT( UINT8 mode, UINT8 *eipek, UINT8 *iksn );
extern	UINT8	PED_SetAESDUKPT(UINT8 *eipek, UINT8 *iksn);
extern	UINT8	PED_ISO4_SetMasterKey(UINT8 index, UINT8 length, UINT8 *key);
extern	UINT8	PED_ISO4_SetSessionKey(UINT8 index, UINT8 length, UINT8 *key);
extern	UINT8	PED_SetISO4KEY(UINT8 length, UINT8 *key);
extern	UINT8	PED_AccDEK_SetMasterKey(UINT8 index, UINT8 length, UINT8 *key);
extern	UINT8	PED_AccDEK_SetSessionKey(UINT8 index, UINT8 length, UINT8 *key);
extern	UINT8	PED_FPE_SetMasterKey(UINT8 index, UINT8 length, UINT8 *key);
extern	UINT8	PED_FPE_SetSessionKey(UINT8 index, UINT8 length, UINT8 *key);
extern	UINT8	PED_SetFPEKey(UINT8 length, UINT8 *key);

extern	UCHAR	DUKPT_RequestPinEntry(UCHAR *pinblock, UCHAR *ksn, UCHAR *epb);

extern	UINT32	PED_DualPasswordControl();
extern  UINT32	PED_DualPasswordControl(UINT32 mode);

extern	UINT8	PED_VerifyKPKStatus(UINT8 type);
extern	void	PED_SetKPKStatus(UINT8 type, UINT8 mode);
extern	UINT8	PED_VerifyIkekStatus(void);
extern	void	PED_SetIkekStatus(UINT8 mode);
extern	UINT8	PED_VerifyISO4KEY(void);
extern	UINT32	PED_VerifyAccDEK(void);
extern	UINT8	PED_VerifyFPEKey(void);
extern	UINT8	PED_VerifyKeyStatus(void);
extern	UINT8	PED_ShowTSN(UCHAR *tsn);

extern	UINT8	PED_ADMIN_DeletePedKey(void);
extern	UINT8	PED_ADMIN_SetKeyBlockProtectionKey(UINT8 type);
extern  UINT8	PED_ADMIN_SetTDESKeyBlockProtectionKey(UINT8 type);
extern  UINT8	PED_ADMIN_SetAESKeyBlockProtectionKey(UINT8 type);
extern	UINT8	PED_ADMIN_LoadIKEK(void);
extern	UINT8	PED_ADMIN_LoadMasterSessionKey(void);
extern  UINT8	PED_ADMIN_LoadPEKMasterSessionKey(void);
extern	UINT8	PED_ADMIN_LoadDukptKey(void);
extern  UINT8	PED_ADMIN_LoadAesDukptKey(void);
extern	UINT8	PED_ADMIN_LoadISO4KEY(void);
extern	UINT8	PED_ADMIN_LoadAccDEK(void);
extern  UINT8	PED_ADMIN_LoadAccDEKMasterSessionKey(void);
extern	UINT8	PED_ADMIN_LoadFPEKey(void);
extern  UINT8	PED_ADMIN_LoadFPEMasterSessionKey(void);
extern	UINT8	PED_ADMIN_LoadCAPK(void);
extern	UINT8	PED_ADMIN_ChangeKeyMode(void);
extern	UINT8	PED_ADMIN_ResetSecureMemory(void);
extern	UINT8	PED_ADMIN_SetupSRED(void);
extern  UINT8	PED_ADMIN_DeviceAuthenticationPhase1(void);
extern  UINT8	PED_ADMIN_DeviceAuthenticationPhase2(void);

//----------------------------------------------------------------------------
#endif
