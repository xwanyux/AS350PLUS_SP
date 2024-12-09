//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS330_mPOS						    **
//**                                                                        **
//**  FILE     : EMVAPK.H                                                   **
//**  MODULE   : Declaration of related EMV Application Kernel functions.   **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2018/08/07                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2018 SymLink Corporation. All rights reserved.	    **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _EMVAPK_H_
#define _EMVAPK_H_

#include "POSAPI.h"


//----------------------------------------------------------------------------
//      Application Kernel Function
//----------------------------------------------------------------------------
//      APK_SPSE
extern  UCHAR apk_SelectPSE( UCHAR *fci );
extern  UCHAR apk_SelectDDF( UCHAR *fci, UCHAR *ddfname );
extern  UCHAR apk_SelectADF( UCHAR *fci, UCHAR *dfname, UCHAR occurrence );

//      APK_FTAG
extern  UCHAR *apk_FindTag( UCHAR tag1, UCHAR tag2, UCHAR *data );
extern  UCHAR apk_FindTagDOL( UCHAR tag1, UCHAR tag2, UCHAR *dol );
extern  UCHAR apk_CheckConsTag( UCHAR tag );
extern  UCHAR apk_CheckWordTag( UCHAR tag );
extern  UCHAR apk_CheckTermTag( UCHAR tag );
extern  UCHAR apk_CheckIccTag( UCHAR tag );
extern  UCHAR apk_CheckIssuerTag( UCHAR tag );
extern  UCHAR apk_SetBERLEN( UCHAR orgLen, UCHAR berLen[] );
extern  UINT  apk_GetBERLEN( UCHAR *de, UCHAR *cnt );
extern  UCHAR *apk_GetBERTLV( UCHAR *reclen, UCHAR *ptrobj, UCHAR *tlv );
extern  UCHAR *apk_GetBERTLV2( UCHAR reclen[], UCHAR *ptrobj, UCHAR *tlv, UCHAR padlen  );
extern  UCHAR apk_ScanIDE( UCHAR tag1, UCHAR tag2, UCHAR *de );
extern  UCHAR apk_ParseTLV( UCHAR reclen[], UCHAR *ptrobj, UCHAR padlen );
extern  void  apk_CheckIsoPadding_Left( UCHAR *rec );
extern  UCHAR apk_CheckIsoPadding_Right( UCHAR *rec, UCHAR *padlen );
extern  UCHAR apk_ParseLenFCI( UCHAR *ptrfci, UCHAR *padlen );
extern  UCHAR apk_ParseLenPSEDIR( UCHAR *ptrdir );

//      APK_RREC
extern  UCHAR apk_ReadRecord( UCHAR sfi, UCHAR recnum, UCHAR *recdata );

//      APK_MAPP
extern  UCHAR apk_MatchingAPP( UCHAR *aid );
extern  UCHAR apk_GetTermAID( UCHAR index, UCHAR *aid );
extern  void  apk_EmptyCandidateList( void );
extern  UCHAR apk_AddCandidateList( UCHAR *ptrdata );
extern  UCHAR apk_GetCandidateList( UCHAR *ptrdata );
extern  UCHAR apk_CheckCandidateList( void );
extern  UCHAR apk_ArrangeCandidateList( void );
extern  UCHAR apk_RemoveCandidateList( UCHAR link );

//      APK_SRAM
extern  UCHAR apk_WriteRamDataTERM( ULONG address, ULONG length, UCHAR *data );
extern  UCHAR apk_WriteRamDataICC( ULONG address, ULONG length, UCHAR *data );
extern  UCHAR apk_WriteRamDataKEY( ULONG address, ULONG length, UCHAR *data );
extern  UCHAR apk_ClearRamDataTERM( ULONG address, ULONG length, UCHAR pattern );
extern  UCHAR apk_ClearRamDataICC( ULONG address, ULONG length, UCHAR pattern );
extern  UCHAR apk_ClearRamDataKEY( ULONG address, ULONG length, UCHAR pattern );
extern  UCHAR apk_ReadRamDataTERM( ULONG address, ULONG length, UCHAR *data );
extern  UCHAR apk_ReadRamDataICC( ULONG address, ULONG length, UCHAR *data );
extern  UCHAR apk_ReadRamDataKEY( ULONG address, ULONG length, UCHAR *data );
extern  UCHAR apk_PushRamData( UCHAR *top, UCHAR maxstk, UINT length, UCHAR *item );
extern  UCHAR apk_PopRamData( UCHAR *top, UINT len, UCHAR *item );

//      APK_INIT
extern  UCHAR apk_InitApp( UCHAR *pdol, UCHAR *data );

//      APK_OFDA
extern  UCHAR apk_OFDA_BuildInputList( UCHAR *outbuf );
extern  UCHAR apk_RetrievePublicKeyCA( UCHAR pki, UCHAR *rid, UCHAR *pkm, UCHAR *pke );
extern  UCHAR apk_LoadExternalPK( UINT slot, UCHAR *pkm, UCHAR *pke );
extern  UCHAR apk_RecoverPKC( UCHAR *pkm, UCHAR *pke, UCHAR *pkc );
extern  UCHAR apk_RecoverIPKC( UCHAR *pkm, UCHAR *pke, UCHAR *pkc );
extern  UCHAR apk_HASH( UCHAR algorithm, UINT length, UCHAR *data, UCHAR *digest );
extern  UCHAR apk_GetChallengeTERM( UCHAR *random );
extern  UCHAR apk_GetChallengeICC( UCHAR *random );
extern  UCHAR apk_InternalAuthen( UCHAR *ddol, UCHAR *sdad );
extern  UCHAR apk_SelectSAM( void );
extern  UCHAR apk_CleanSAM( void );

//      APK_PRES
extern  UCHAR apk_ProcessRestrict_AVN( void );
extern  UCHAR apk_ProcessRestrict_AUC( void );
extern	UCHAR apk_ProcessRestrict_AED( void );

//      APK_CVML
extern  UCHAR apk_CVM_PlaintextPIN( UINT tout, UCHAR *msg );
extern  UCHAR apk_CVM_EncipheredPIN( UINT tout, UCHAR *msg );
extern  UCHAR apk_CVM_OnlineEncipheredPIN( UINT tout, UCHAR *msg, UCHAR *epb, UCHAR *ksn, UCHAR mod, UCHAR idx );

//      APK_TRMS
extern  UCHAR apk_RandomNumber( void );
extern  UCHAR apk_FloorLimitChecking( UCHAR *amt1, UCHAR *amt2 );
extern  UCHAR apk_RandomTransactionSelection( UCHAR *amt1, UCHAR *amt2 );
extern	UCHAR apk_VelocityChecking( void );

//      APK_GACC
extern  UCHAR apk_GenerateAC( UCHAR type, UCHAR *cdol, UCHAR *ac );
extern  UCHAR apk_ExternalAuthen( UCHAR *iad );
extern  UCHAR apk_IssuerScriptProcessing( UCHAR tag, UCHAR *ist );
extern  UCHAR apk_IssuerScriptProcessing2( UCHAR tag, UINT addr_ist );

//----------------------------------------------------------------------------
//      Application Kernel Definition
//----------------------------------------------------------------------------
#define apkOK                   0x00
#define apkFailed               0x01
#define apkReady                0x00
#define apkNotReady             0x02
#define apkIncorrect            0x03
#define apkUnknown              0x04
#define apkNotSupported         0x05
#define apkTRM1                 0x11
#define apkTRM2                 0x12
#define apkTRM3                 0x14
#define apkTRM5                 0x18                    // PBOC2.0, ATC data missing
#define apkTRM4                 0x50                    // org = 0x18
#define apkFallBack             0x80                    // JCB SPEC
#define apkDeviceError          0xfe
#define apkOutOfService         0xff

#define apkFileInvalidated      0x80
#define apkFuncNotSupported     0x81
#define apkFileNotFound         0x82
#define apkRecNotFound          0x83
#define apkDataInvalidated      0x84
#define apkCondNotSatisfied     0x85
#define apkFciMissing           0x86
#define apkMObjMissing 0x87     //20090302_Richard:CC.135.00, mandatory data objects are not present in response to generate AC

#define apkExactMatch           0x00
#define apkNoMatch              0x01
#define apkPartialMatch         0x02

//----------------------------------------------------------------------------
#endif
