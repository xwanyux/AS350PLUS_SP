//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : i.MX6UL                                                    **
//**  PRODUCT  : AS350-X6                                                   **
//**                                                                        **
//**  FILE     : FPE.H                                                      **
//**  MODULE   : 			   				                                **
//**                                                                        **
//**  FUNCTION : Declaration of FPE Module.                    		        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2022/05/24                                                 **
//**  EDITOR   : Tammy                                                      **
//**                                                                        **
//**  Copyright(C) 2022 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _FPE_H_
#define _FPE_H_

//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
extern	void	FPE_Func_Ceiling(double value, int *ceil);
extern	void	FPE_Func_Floor(double value, int *floor);
//extern	void	FPE_Func_Log(int x, double *result);
extern	void	FPE_Func_ByteString(int x, UINT16 s, UINT8 *byteArray);
extern	void	FPE_Func_NUM(UINT8 radix, UINT8 *dataIn, UINT16 length, int *number);
extern	void	FPE_Func_NUM2(UINT8 *dataIn, UINT16 length, UINT64 *number);
extern	void	FPE_Func_STR(UINT8 radix, UINT16 strLen, int dataIn, UINT8 *dataOut);
extern	void	FPE_Func_REV(UINT8 *dataIn, UINT16 length, UINT8 *dataOut);
extern	void	FPE_Func_PRF(UINT8 *K, UINT8 *X, UINT16 strLen, UINT8 *Y);
extern	void	FPE_Func_PRF2(UINT8 *K, UINT8 *X, UINT16 strLen, UINT8 *Y);
extern	void	FPE_Func_CIPH(UINT8 *K, UINT8 *X, UINT16 strLen, UINT8 *Y);
extern	void	FPE_FF1_Encrypt(UINT8 radix, UINT8 *K, UINT8 *T, UINT16 tLen, UINT8 *X, UINT16 xLen, UINT8 *Y);
extern	void	FPE_FF1_Decrypt(UINT8 radix, UINT8 *K, UINT8 *T, UINT16 tLen, UINT8 *X, UINT16 xLen, UINT8 *Y);

#endif // !_FPE_H_


