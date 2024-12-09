//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : 							    **
//**  PRODUCT  : AS330							    **
//**                                                                        **
//**  FILE     : DEV_PED.H                                                  **
//**  MODULE   : Declaration of PED Module. 		                    **
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
#ifndef _DEV_PED_H_
#define _DEV_PED_H_

//----------------------------------------------------------------------------
#include "POSAPI.h"
#include "bsp_types.h"


#define	EPB_ISO0	0			// PIN block format ISO 9564 Format 0 (ANSI X9.8)
#define	EPB_ISO1	1			//				    1
#define	EPB_ISO2	2			//				    2
#define	EPB_ISO3	3			//				    3
#define	EPB_ISO4	4			//				    4

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

#define PED_DEV_INT             	0	// internal pinpad device
#define PED_DEV_EXT             	1	// internal pinpad device

#define	DES_MODE_ECB			0x00	// electronic code book
#define	DES_MODE_CBC			0x01	// cipher bolck chaining
#define	DES_MODE_ENCRYPT		0x00	// to encrypt
#define	DES_MODE_DECRYPT		0x80	// to decrypt

// PED Response Code (external)
#define PED_RC_OK                       0x00    //
#define PED_RC_FAILED                   0x01    //

// PED Command (external)
//#define PED_CMD_SET_MKEY                '0'
//#define PED_CMD_SET_SKEY                '1'
//#define PED_CMD_RESET                   '2'
//#define PED_CMD_GET_PIN                 '3'
//#define PED_CMD_MSKEY_GEN_PIN_BLOCK     '4'
//#define PED_CMD_MSKEY_GEN_MAC           '5'
//#define PED_CMD_GET_STATUS              '6'
//#define PED_CMD_OPEN                    '7'
//#define PED_CMD_CLOSE                   '8'

// Internal PINPED State
#define PED_STATE_START                 0
#define PED_STATE_WAIT_KEY              1
#define PED_STATE_PIN_READY             2
#define PED_STATE_END                   3

//----------------------------------------------------------------------------
//	External Function Prototypes
//----------------------------------------------------------------------------
//	OS_PED
//extern	void	PED_WriteKeyMode( UINT8 mode );
extern	UINT8	PED_ReadKeyMode( void );

//	OS_PED2
//extern	UINT8	PED_SetMasterKey( UINT8 index, UINT8 length, UINT8 *key );
//extern	UINT8	PED_SelectMasterKey( UINT8 index );
//extern	UINT8	PED_SetSessionKey( UINT8 index, UINT8 length, UINT8 *key );
//extern	UINT8	PED_SetFixedKey( UINT8 index, UINT8 length, UINT8 *key );
//extern	UINT8	PED_SetDUKPT( UINT8 mode, UINT8 *eipek, UINT8 *iksn );
extern		UINT8	PED_GetPin( UINT16 tout, UINT8 *amt );
extern      void    PED_ClearPin(void);

//	OS_PED3
extern	UINT8	PED_GenPinBlock( UINT8 *pb );
extern	UINT8	PED_FXKEY_GenPinBlock( UINT8 mode, UINT8 index, UINT8 *pan, UINT8 *epb );
extern	UINT8	PED_MSKEY_GenPinBlock( UINT8 mode, UINT8 index, UINT8 *pan, UINT8 *epb );
extern	UINT8	PED_DUKPT_GenPinBlock( UINT8 mode, UINT8 *pan, UINT8 *epb, UINT8 *ksn );
extern	UINT8	PED_AESKEY_GenPinBlock( UINT8 mode, UINT8 index, UINT8 *pan, UINT8 *epb );

//	OS_PED3-1
extern  UINT8	PED_AES_DUKPT_GenPinBlock( UINT16 mode, UINT8 *pan, UINT8 *epb, UINT8 *ksn );

//	OS_PED4
extern	UINT8	PED_FXKEY_GenMAC( UINT8 mode, UINT8 index, UINT8 *icv, UINT16 length, UINT8 *data, UINT8 *mac );
extern	UINT8	PED_MSKEY_GenMAC( UINT16 mode, UINT8 index, UINT8 *icv, UINT16 length, UINT8 *data, UINT8 *mac );
extern	UINT8	PED_DUKPT_GenMAC( UINT8 mode, UINT8 *icv, UINT16 length, UINT8 *data, UINT8 *mac, UINT8 *ksn );
extern  UINT8	PED_AES_DUKPT_GenMAC( UINT16 mode, UINT8 *icv, UINT16 length, UINT8 *data, UINT8 *mac, UINT8 *ksn );

//	OS_PED4-1
extern  UINT8	PED_AES_DUKPT_GenMAC( UINT16 mode, UINT8 *icv, UINT16 length, UINT8 *data, UINT8 *mac, UINT8 *ksn );

//	OS_PED5
extern	UINT8	PED_CAPK_GetKeyHeader( UINT8 index, UINT8 *pkh );
extern	UINT8	PED_CAPK_SelectKey( UINT8 pki, UINT8 *rid, UINT8 *pkh, UINT8 *index );

//----------------------------------------------------------------------------
#endif
