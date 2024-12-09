//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : i.MX6UL                                				    **
//**  PRODUCT  : AS350-X6						                            **
//**                                                                        **
//**  FILE     : SRED_DBG_Function.H                                        **
//**  MODULE   : Declaration of the SRED debug APIs.	                    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2022/05/15                                                 **
//**  EDITOR   : Tammy                                                      **
//**                                                                        **
//**  Copyright(C) 2022 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _SRED_DBG_FUNCTION_H_
#define _SRED_DBG_FUNCTION_H_

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
extern	UINT8	SRED_DBG_Open_AUX(UINT8 port);
extern	void	SRED_DBG_Close_AUX(void);
extern	UINT8	SRED_DBG_Tx_AUX(UINT8 auxDHN, UINT8 *sndData);
extern	UINT8	SRED_DBG_SubSplit(UINT8 data);
extern	void	SRED_DBG_Split(UINT8 *des, UINT8 *src, char pair_len);
extern	void	SRED_DBG_Put_Data(UINT8 iptOption, UINT16 iptLen, UINT8 *iptHex);
extern	void	SRED_DBG_Put_Hex(UINT16 iptLen, UINT8 *iptHex);
extern	void	SRED_DBG_Put_String(UINT8 iptLen, UINT8 *iptString);
extern	void	SRED_DBG_Put_UCHAR(UINT8 iptUCHAR);
extern	void	SRED_DBG_Put_UINT(UINT16 iptUINT);
extern	void	SRED_DBG_Put_ULONG(UINT32 iptULONG);

//----------------------------------------------------------------------------
#endif
