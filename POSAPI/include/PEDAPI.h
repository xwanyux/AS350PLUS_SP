//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : 							    **
//**  PRODUCT  : AS330							    **
//**                                                                        **
//**  FILE     : PEDAPI.H         	                                    **
//**  MODULE   : Declaration of the PED APIs.				    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2018/07/05                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2018 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _PEDAPI_H_
#define _PEDAPI_H_

//----------------------------------------------------------------------------
#include "POSAPI.h"

//----------------------------------------------------------------------------
#define	PED_KEY_MODE_NULL		0	// none
//#define	PED_KEY_MODE_FIXED		1	// fixed key
#define	PED_KEY_MODE_MS			1	// master/session key
#define	PED_KEY_MODE_DUKPT		2	// derived unique key per transaction
#define	PED_KEY_MODE_ISO4		3	// ISO4_KEY

#define	EPB_ISO0	0			// PIN block format ISO 9564 Format 0 (ANSI X9.8)
#define	EPB_ISO1	1			//				    1
#define	EPB_ISO2	2			//				    2
#define	EPB_ISO3	3			//				    3

#define	MAC_ISO16609	0x00			// MAC Algorithm 1 for ISO 16609 (TDEA)
#define	MAC_ALG1	0x01			// MAC Algorithm 1 for ISO 9797-1 (mode:low nibble)
#define	MAC_ALG2	0x02			//               2
#define	MAC_ALG3	0x03			//               3
#define	MAC_ALG4	0x04			//               4
#define	MAC_ALG5	0x05			//               5
#define	MAC_ALG6	0x06			//               6

#define	MAC_PAD1	0x10			// MAC Padding Method 1 for ISO 9797-1 (mode:high nibble)
#define	MAC_PAD2	0x20			//                    2
#define	MAC_PAD3	0x30			//                    3
#define	MAC_PAD4	0x40			//                    4

#define PED_DEV_INT             	0	// internal pinpad device
#define PED_DEV_EXT             	1	// external pinpad device

//typedef	struct API_GENMAC_S
//{
//	UCHAR			Mode;		// algorithm and padding method
//	UCHAR			Index;		// key slot index (not used for DUKPT)
//	UINT			Length;		// length of data to be processed
//	UCHAR			ICV[8];		// initial chain vector
//} __attribute__((packed)) API_GENMAC;


//----------------------------------------------------------------------------
//		Function Prototypes
//----------------------------------------------------------------------------
/*
** API: PED
*/

extern	UCHAR	api_ped_GetPin( UINT tout, UCHAR *amt );
extern  UCHAR	api_xped_GetPin( UINT tout, UCHAR *amt );
extern	UCHAR	api_ped_GetKeyMode( void );
extern	UCHAR	api_ped_GetKeyHeader_CAPK( UCHAR index, UCHAR *pkh );
extern	UCHAR	api_ped_SelectKey_CAPK( UCHAR pki, UCHAR *rid, UCHAR *pkh, UCHAR *index );

extern	UCHAR	api_ped_GenPinBlock_FXKEY( UCHAR mode, UCHAR index, UCHAR *pan, UCHAR *epb );
extern	UCHAR	api_ped_GenPinBlock_MSKEY( UCHAR mode, UCHAR index, UCHAR *pan, UCHAR *epb );
extern	UCHAR	api_ped_GenPinBlock_DUKPT( UCHAR mode, UCHAR *pan, UCHAR *epb, UCHAR *ksn );
extern	UCHAR	api_ped_GenPinBlock_AES_DUKPT( UINT mode, UCHAR *pan, UCHAR *epb, UCHAR *ksn );
extern  UCHAR	api_ped_GenPinBlock_AESKEY( UCHAR mode, UCHAR index, UCHAR *pan, UCHAR *epb );

extern	UCHAR	api_ped_GenMAC_FXKEY( UCHAR mode, UCHAR index, UCHAR *icv, UINT length, UCHAR *data, UCHAR *mac );
//extern	UCHAR	api_ped_GenMAC_MSKEY( UCHAR mode, UCHAR index, UCHAR *icv, UINT length, UCHAR *data, UCHAR *mac );
extern	UCHAR	api_ped_GenMAC_MSKEY( UINT mode, UCHAR index, UCHAR *icv, UINT length, UCHAR *data, UCHAR *mac );
extern	UCHAR	api_ped_GenMAC_DUKPT( UCHAR mode, UCHAR *icv, UINT length, UCHAR *data, UCHAR *mac, UCHAR *ksn );
extern  UCHAR	api_ped_GenMAC_AES_DUKPT( UINT mode, UCHAR *icv, UINT length, UCHAR *data, UCHAR *mac, UCHAR *ksn );

extern	void	api_ped_SetupPinPad( UCHAR *sbuf );
extern  UCHAR	api_ped_SetPinPadPort( UCHAR port );

//----------------------------------------------------------------------------
#endif
