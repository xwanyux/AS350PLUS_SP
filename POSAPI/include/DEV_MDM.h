//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L                                                       **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : DEV_MDM.H                                                  **
//**  MODULE   : Declaration of MODEM Module.		                    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2009/05/05                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2009 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _DEV_MDM_H_
#define _DEV_MDM_H_

//----------------------------------------------------------------------------
#include "bsp_types.h"


//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
extern	UINT32	OS_MDM_Open( UINT8 *atcmd, API_HOST *sbuf );
extern	UINT32	OS_MDM_Close( UINT8 dhn, UINT16 delay );
extern	UINT32	OS_MDM_Status( UINT8 *dbuf );
extern	UINT32	OS_MDM_RxReady( UINT8 *dbuf );
extern	UINT32	OS_MDM_RxString( UINT8 *dbuf );
extern	UINT32	OS_MDM_TxReady( void );
extern	UINT32	OS_MDM_TxString( UINT8 *sbuf );

// ---------------------------------------------------------------------------
#endif
