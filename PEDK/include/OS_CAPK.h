//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : 							                                **
//**  PRODUCT  : AS350-X6							                        **
//**                                                                        **
//**  FILE     : OS_CAPK.H						                            **
//**  MODULE   : Declaration of CAPK structure.		                        **
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
#ifndef _OS_CAPK_H_
#define _OS_CAPK_H_

//----------------------------------------------------------------------------
//#include "POSAPI.h"


//----------------------------------------------------------------------------
//	CAPK Key Slot Components Addressing Offset
//----------------------------------------------------------------------------
#define	CAPK_KEY_SLOT_LEN	300	// internal NVSRAM
#define	CAPK_KEY_SLOT_LEN2	300+4	// external SRAM
#define	CAPK_HEADER_LEN		29	// RID(5),INDEX(1),EXPLEN(1),MODLEN(2),HASH(20)
//#define	MAX_CAPK_CNT		17	// max 17 CAPK key slots
#define	MAX_CAPK_CNT		50	// max 50 CAPK key slots
#define	MAX_PAYMENT_SCHEME_CNT	6	// max 6 payment schemes
//#define	MAX_INDEX_CNT		4	// max 4 indexes per scheme
#define	MAX_INDEX_CNT		10	// max 10 indexes per scheme
#define	RID_LEN			5	//

#define OFFSET_CAPK_RID         0x0000                  // key file structure
#define OFFSET_CAPK_PKI         0x0005                  //
#define OFFSET_CAPK_EXP_LEN     0x0006                  //
#define OFFSET_CAPK_MOD_LEN     0x0007                  //
#define OFFSET_CAPK_SHA1        0x0009                  //
#define OFFSET_CAPK_EXP         0x001D                  //
#define OFFSET_CAPK_MOD         0x0020                  // for fixed length exponent=3 bytes
#define OFFSET_CAPK_MOD1        0x001E                  // for variable length exponent=2 or 3 (1 byte)
#define OFFSET_CAPK_MOD3        0x0020                  // for variable length exponent=2^16+1 (3 bytes)

#define	ADDR_CAPK_RID		0
#define	ADDR_CAPK_INDEX		ADDR_CAPK_RID+5
#define	ADDR_CAPK_EXP_LEN	ADDR_CAPK_INDEX+1
#define	ADDR_CAPK_MOD_LEN	ADDR_CAPK_EXP_LEN+1
#define	ADDR_CAPK_HASH		ADDR_CAPK_MOD_LEN+2
#define	ADDR_CAPK_EXP		ADDR_CAPK_HASH+20
#define	ADDR_CAPK_MOD		ADDR_CAPK_EXP+3
#define	ADDR_CAPK_HASH_AI	ADDR_CAPK_MOD+256
#define	ADDR_CAPK_PK_AI		ADDR_CAPK_HASH_AI+1
#define	ADDR_CAPK_RFU		ADDR_CAPK_PK_AI+1

//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
#endif
