//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L                                                       **
//**  PRODUCT  : AS330/QS200                                                **
//**                                                                        **
//**  FILE     : DEV_AUX.H                                                  **
//**  MODULE   : Declaration of AUX Module.		                    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2007/09/17                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2007 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _DEV_AUX_H_
#define _DEV_AUX_H_

//----------------------------------------------------------------------------
#include "bsp_types.h"
#include "bsp_uart.h"
#include "POSAPI.h"

#define	AUX_DEFAULT_BUF_SIZE			512
//#define	AUX_MAX_BUF_SIZE			4*1024+128		// 2013-05-10
#define	AUX_MAX_BUF_SIZE			10*1024+128		// 2013-09-30, TEST ONLY
#define	MAX_AUX_ACKS				8

#define	AUX_MODE_BYPASS				0x00			// protocol: transparent
#define	AUX_MODE_SOH				0x01			// protocol: SOH hexLEN(2) hexMSG(N) LRC
#define	AUX_MODE_DLL				0x02			// protocol: SOH hexLEN(1) hexMSG(N) LRC
#define	AUX_MODE_STX				0x03			// protocol: STX txtMSG(N) ETX LRC
#define	AUX_MODE_STXB				0x04			// protocol: STX bcdLEN(2) hexMSG(N) ETX LRC
#define	AUX_MODE_VISAWAVE			0x05			// protocol: STX hexMSG(N) CRC(2) ETX
#define	AUX_MODE_BARCODE			0x06			// protocol: txtMSG(N) CR
#define	AUX_MODE_SVC				0x0A			// protocol: STX hexMSG(N) CRC(2) ETX
#define	AUX_MODE_DLESTX				0x0B			// protocol: DLE STX hexMSG(N) DLE ETX LRC
#define	AUX_MODE_STXLEN				0x0C			// protocol: STX LEN(2) TEXT CRCC(2)
#define	AUX_MODE_DLESTX2			0x0D			// protocol: DLE STX LEN(2) hexMSG(N) LRC DLE ETX (IPASS)

#define	AUX_STATE_IDLE				0x00
#define	AUX_STATE_LEN				0x01
#define AUX_STATE_LEN_L				0x01
#define	AUX_STATE_LEN_H				0x02
#define AUX_STATE_DATA				0x03
#define	AUX_STATE_LRC				0x04
#define	AUX_STATE_WAIT_ACK			0x05

#define	AUX_STATE_LEN_L2			0x06
#define	AUX_STATE_LEN_H2			0x07

#define	AUX_STATUS_FREE				0x00			// OK
#define	AUX_STATUS_ERROR			0x01			// timeout or device error
#define	AUX_STATUS_BUSY				0x02			// transmit in progress

#define	WAVE_STATE_IDLE				0x00			// state machine for VisaWAVE
#define	WAVE_STATE_SEQNO			0x51
#define	WAVE_STATE_TXI_H			0x52
#define	WAVE_STATE_TXI_L			0x53
#define	WAVE_STATE_RXI_H			0x54
#define	WAVE_STATE_RXI_L			0x55
#define	WAVE_STATE_INST				0x56
#define	WAVE_STATE_LEN_H			0x57
#define	WAVE_STATE_LEN_L			0x58
#define	WAVE_STATE_RESP				0x59
#define	WAVE_STATE_DATA				0x5A
#define	WAVE_STATE_CRC_H			0x5B
#define	WAVE_STATE_CRC_L			0x5C
#define	WAVE_STATE_ETX				0x5D

#define	SVC_STATE_IDLE				0x00			// state machine for SVC
#define	SVC_STATE_HEAD				0xA1
#define	SVC_STATE_LEN_H				0xA7
#define	SVC_STATE_LEN_L				0xA8
#define	SVC_STATE_DATA				0xAA
#define	SVC_STATE_CRC_H				0xAB
#define	SVC_STATE_CRC_L				0xAC
#define	SVC_STATE_ETX				0xAD

#define	DLE2_STATE_IDLE				0x00			// state machine for DLESTX2
#define	DLE2_STATE_STX				0xD1
#define	DLE2_STATE_LEN_H			0xD2
#define	DLE2_STATE_LEN_L			0xD3
#define	DLE2_STATE_DATA				0xD4
#define	DLE2_STATE_LRC 				0xD5
#define	DLE2_STATE_DLE				0xD6
#define	DLE2_STATE_ETX				0xD7

#define	WAVE_INST_POLL				0x07
#define	WAVE_INST_SET_OPTI			0x10
#define	WAVE_INST_SET_PARA			0x12

#define	SOH					0x01
#define	STX					0x02
#define	ETX					0x03
#define	EOT					0x04
#define	ENQ					0x05
#define	ACK					0x06
#define	LF					0x0A
#define	FORMFEED				0x0C
#define	CR					0x0D
#define	WACK					0x10
#define	DLE					0x10
#define	NAK					0x15
#define	ESC					0x1B

#define BSP_MAX_UARTS		3+1		// COM3 for LTE

typedef struct AUX_DATA_S
{
	UINT32			Len;					// final length of data
	UINT8			Data[AUX_MAX_BUF_SIZE+16];		// package payload
} AUX_DATA;


//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
extern	void	OS_AUX_Init( void );
extern	void	OS_AUX_ResetStatus( BSP_UART *pUart );
extern	UINT32	OS_AUX_TaskCheckTob( BSP_UART *pUart );
extern	BSP_UART *AUX_GetDHN( UINT32 port );
extern	void	OS_AUX_FlowControl( void );
extern	void	OS_AUX_IsrCallBack( BSP_UART *pUart );
extern	UINT32	OS_AUX_SendPackage( BSP_UART *pUart );
extern	UINT32	OS_AUX_ResendPackage( BSP_UART *pUart );

// DLL
extern	UINT32	OS_AUX_DataLinkRx_DLL( BSP_UART *pUart );
extern	UINT32	OS_AUX_DataLinkTx_DLL( BSP_UART *pUart, UINT8 *pData );
extern	UINT32	OS_AUX_DataLinkAck_DLL( BSP_UART *pUart, UINT8 Code );
extern	void	OS_AUX_DataLinkWaitAck_DLL( BSP_UART *pUart );
extern	void	OS_AUX_DataLinkRxTOB_DLL( UINT32 port );
extern	UINT32	AUX_DLL_TxReady( BSP_UART *pUart );

// SOH
extern	UINT32	OS_AUX_DataLinkRx_SOH( BSP_UART *pUart );
extern	UINT32	OS_AUX_DataLinkTx_SOH( BSP_UART *pUart, UINT8 *pData );
extern	void	OS_AUX_DataLinkRxTOB_SOH( UINT32 port );

// BYPASS
extern	UINT32	OS_AUX_DataLinkTx_BYPASS( BSP_UART *pUart, UINT8 *pData );
extern	UINT32	OS_AUX_DataLinkRx_BYPASS( BSP_UART *pUart );
extern	UINT32	AUX_BYPASS_TxReady( BSP_UART *pUart );
extern	void	OS_AUX_DataLinkRxTOB_BYPASS( UINT32 port );

// STX
extern	void	OS_AUX_DataLinkWaitAck_STX( BSP_UART *pUart );
extern	void	OS_AUX_DataLinkRxTOB_STX( UINT32 port );
extern	UINT32	OS_AUX_DataLinkRx_STX( BSP_UART *pUart );
extern	UINT32	OS_AUX_DataLinkTx_STX( BSP_UART *pUart, UINT8 *pData );

//WAVE
extern	UINT32	OS_AUX_DataLinkRx_WAVE( BSP_UART *pUart );
extern	UINT32	OS_AUX_DataLinkTx_WAVE( BSP_UART *pUart, UINT8 *pData );
extern	void	OS_AUX_DataLinkRxTOB_WAVE( UINT32 port );
extern	UINT16	WAVE_GenerateCRC16( UINT16 crc, UINT16 ch );

//SVC
extern	UINT32	OS_AUX_DataLinkRx_SVC( BSP_UART *pUart );
extern	UINT32	OS_AUX_DataLinkTx_SVC( BSP_UART *pUart, UINT8 *pData );

//BARCODE
extern	UINT32	OS_AUX_DataLinkRx_BARCODE( BSP_UART *pUart );
extern	void	OS_AUX_DataLinkRxTOB_BARCODE( UINT32 port );

//DLESTX2
extern	UINT32	OS_AUX_DataLinkRx_DLESTX2( BSP_UART *pUart );
extern	UINT32	OS_AUX_DataLinkTx_DLESTX2( BSP_UART *pUart, UINT8 *pData );
extern	void	OS_AUX_DataLinkRxTOB_DLESTX2( UINT32 port );



extern	BSP_UART *OS_AUX_Open( UINT8 Port, API_AUX pAux );
extern	UINT32	OS_AUX_Close( BSP_UART *pUart );
extern	UINT32	OS_AUX_RxReady( BSP_UART *pUart, UINT8 *dbuf, UINT32 type );
extern	UINT32	OS_AUX_RxString( BSP_UART *pUart, UINT8 *dbuf );
extern	UINT32	OS_AUX_TxReady( BSP_UART *pUart );
extern	UINT32	OS_AUX_TxString( BSP_UART *pUart, UINT8 *sbuf );

//----------------------------------------------------------------------------
#endif
