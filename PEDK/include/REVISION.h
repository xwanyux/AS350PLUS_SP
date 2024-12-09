//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : MAX32550 (32-bit Platform)				    **
//**  PRODUCT  : AS330_mPOS						    **
//**                                                                        **
//**  FILE     : REVISION.H                                                 **
//**  MODULE   : Revision of product.			                    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2018/05/15                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2018 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _REVISION_H_
#define _REVISION_H_

#include "bsp_types.h"

//----------------------------------------------------------------------------
// Revision History
//
// Version No	Date		Remark				Editor
// ----------	----------	-----------------------------	--------------
// V1.00	2021/12/06	1'st SLA release for PCI project.
// V1.01	2022/02/10	(1) Add PED_ResetPinPad() at POST to prevent PIN entry frequency.
//				(2) Check both SECDIAG & FLASH log in OS_SEC_CheckTamperStatus() for power-off tamper event.
//				(3) clear sensitive buffer data in password entry functions.
//
// V1.3		2022/02/15	Exclude VDDLO in OS_SEC_CheckTamperStatus().
//
// V1.4		2022/02/22	Fix odd pin length bug in PED_GenPinBlock_ISO1()
// V1.5		2022/04/01	Fix issues found in the 1'st full evaluation.
// V1.6		2022/05/05	Fix issues found in the re-test.
// V1.7		2022/05/27	Add model name and hardware version to PED_CMD_GET_VERSION for DTR F7
//
//----------------------------------------------------------------------------
UINT8	const	BIOS_MODEL_NO[]	  =	{"AS330"};		// 10+6 bytes for POST1 at 1002_0000 (formal) - POST1
//UINT8	const	BIOS_MODEL_NO_0[] =	{"AS-330.0  "};		// 10+6 bytes for POST0 at 101C_0000 (mirror) - POST0
UINT8	const	BIOS_SUB_MODEL_ID[] =	{".EPP"};		// sub-model id @(0,6)
//UINT8	const	MEM_CONFIG_486[]  =	{"MCONF:466"};		// SRAM:4MB, FLASH:16MB, SDRAM:32MB
//UINT8	const	MEM_CONFIG_286[]  =	{"MCONF:266"};		// SRAM:2MB, FLASH:16MB, SDRAM:32MB

UINT8	const	BIOS_RELEASE_NO[] =	{"V1.7"};		// cf. revision history
UINT8	const	FW_VERSION_NO[]	=	{"1.7.0"};		// same as BIOS_RELEASE_NO

//UINT8	const	BIOS_RELEASE_DATE[] =	{"20220527170000"};	// YYYY-MM-DD mm:hh:ss

//----------------------------------------------------------------------------
#endif
