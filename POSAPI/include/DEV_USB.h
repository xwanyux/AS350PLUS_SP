//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : MAX32550 (32-bit Platform)				    **
//**  PRODUCT  : AS330_mPOS						    **
//**                                                                        **
//**  FILE     : DEV_BUZ.H                                                  **
//**  MODULE   : Declaration of USB Device Module.	                    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2018/06/05                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2018 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _DEV_USB_H_
#define _DEV_USB_H_

//----------------------------------------------------------------------------
/** Global includes */
#include <config.h>
#include <errors.h>
/** Other includes */
#include <cobra_defines.h>
#include <cobra_macros.h>
#include <cobra_functions.h>
#include <mml_cdc.h>

#include "POSAPI.h"
#include "USBAPI.h"


//----------------------------------------------------------------------------
#define	MAX_USB_PORTS				1

#define	USB_DID_DEVICE				0x00
#define	USB_DID_HOST				0x01

//----------------------------------------------------------------------------
#define	USB_MAX_BUF_SIZE			( 1 * 1024 )
#define	USB_MAX_RX_BUF_SIZE			( 5888 )	// 5888=0x1700

//----------------------------------------------------------------------------
typedef struct
{
	/** System frequency */
	unsigned int				system;
	/** AHB frequency */
	unsigned int				ahb;
	/** APB frequency */
	unsigned int				apb;
	/**  */
	volatile unsigned char			rx_complete;
	/**  */
	volatile unsigned char			tx_complete;
	/** Read data buffer */
//	unsigned char				read_buffer[USB_MAX_BUF_SIZE];
	unsigned char				*read_buffer;
	/** Relative offset on next free room for data to receive */
	unsigned int				ofst_read_free;
	/** Relative offset on last data to read */
	unsigned int				ofst_read_last;
	/** Write data buffer */
//	unsigned char				write_buffer[USB_MAX_BUF_SIZE];
	unsigned char				*write_buffer;
	/** Relative offset on next free room for data to write */
	unsigned int				ofst_write_free;
	/** Relative offset on last data to send */
	unsigned int				ofst_write_last;

} t_usb2uart_context;


typedef struct USB_DATA_S
{
	UINT32			Len;					// final length of data
	UINT8			Data[USB_MAX_BUF_SIZE];			// package payload
} USB_DATA;

typedef struct USB_RX_DATA_S
{
	UINT32			Len;					// final length of data
	UINT8			Data[USB_MAX_RX_BUF_SIZE];		// package payload
} USB_RX_DATA;


//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
extern	UINT32	OS_USB_Init( void );
extern	UINT32	OS_USB_Open( API_USB_PARA para );
extern	UINT32	OS_USB_Close( API_USB_PARA para );
extern	UINT32	OS_USB_RxReady( UINT8 *dbuf );
extern	UINT32	OS_USB_RxString( UINT8 *dbuf );
extern	UINT32	OS_USB_TxReady( void );
extern	UINT32	OS_USB_TxString( UINT8 *sbuf );

//----------------------------------------------------------------------------
#endif
