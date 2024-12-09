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

#ifndef _SRED_FUNC_H_
#define _SRED_FUNC_H_

//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
extern	UINT8	SRED_Func_UseDOA(UINT8 keyScheme, UINT8 index, UINT8 mode, UINT8 *dataIn, UINT32 inLen, UINT8 *dataOut, UINT32 *outLen);
extern	UINT8	SRED_Func_StorePAN(UINT8 *pan, UINT8 length, UINT8 keyScheme, UINT8 index);
extern	UINT8	SRED_Func_GetPAN(UINT8 mode, UINT8 *data, UINT8 *length);
extern	UINT8	SRED_Func_StoreDataElement(UINT8 *iptTag, UINT8 iptLen, UINT8 *iptData, UINT8 keyScheme, UINT8 index);
extern	UINT8	SRED_Func_GetDataElement(UINT8 mode, UINT8 *iptTag, UINT8 *optData, UINT8 *optLen);
extern	UINT8	SRED_Func_EncryptPAN(UINT8 *dataIn, UINT8 inLen, UINT8 *dataOut, UINT8 *outLen);
extern	UINT8	SRED_Func_DecryptPAN(UINT8 *dataIn, UINT8 inLen, UINT8 *dataOut, UINT8 *outLen);
extern  void    SRED_Func_AccDEK_GetCurrentKSN(UINT8 *ksn);
extern  void    SRED_Func_FPE_GetCurrentKSN(UINT8 *ksn);

//----------------------------------------------------------------------------
#endif // !_SRED_FUNC_H_
