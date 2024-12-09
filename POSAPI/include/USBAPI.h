//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : MAX32550 (32-bit Platform)				    **
//**  PRODUCT  : AS330_mPOS						    **
//**                                                                        **
//**  FILE     : USBAPI.H						    **
//**  MODULE   : Declaration of USB Module.		    		    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2018/06/04                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2018 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _USB_API_H_
#define _USB_API_H_


//----------------------------------------------------------------------------
//		USB Communication Port Number
//----------------------------------------------------------------------------
#define		USB_PORT_DEVICE			0x00	// device port (ECR)
#define		USB_PORT_HOST			0x01	// host port   (pen drive)
#define		USB_PORT_OTG			0x80	// OTG port

//----------------------------------------------------------------------------
//		USB Communication Device Class
//----------------------------------------------------------------------------
#define		USB_CDC_RS232			0x00
#define		USB_CDC_MASS_STORAGE		0x80

//----------------------------------------------------------------------------
//		USB Function ID
//----------------------------------------------------------------------------
#define		FID_USB_OPEN_DEVICE		0
#define		FID_USB_OPEN_HOST		1
#define		FID_USB_CLOSE_DEVICE		2
#define		FID_USB_CLOSE_HOST		3

//----------------------------------------------------------------------------
#pragma pack(1)
typedef	struct	API_USB_PARA_S			// USB parameters
{
	UINT		ID;			// function ID
	UCHAR		CDC;			// communication device class
	
	UCHAR		Mode;			// data link protocol
	UINT		Baud;			// baud rate
	UINT		Tob;			// inter-byte timeout for receiving character string
	UINT		Tor;			// timeout of waiting for response
	UCHAR		Resend;			// re-transmit limit
	UCHAR		Acks;			// no. of repetitive acknowledgement to the received message
						
	UINT		RxLen;			// length of data to be read
	UCHAR		RFU[2];			// RFU
} API_USB_PARA;		// __attribute__((packed)) API_USB_PARA;
#pragma pack()


//----------------------------------------------------------------------------
//	Function Prototypes for USB Device
//----------------------------------------------------------------------------
extern	UCHAR	api_usb_open( UCHAR port, API_USB_PARA para );
extern	UCHAR	api_usb_close( UCHAR dhn );
extern	UCHAR	api_usb_rxready( UCHAR dhn, UCHAR *dbuf );
extern	UCHAR	api_usb_rxstring( UCHAR dhn, UCHAR *dbuf );
extern	UCHAR	api_usb_txready( UCHAR dhn );
extern	UCHAR	api_usb_txstring( UCHAR dhn, UCHAR *sbuf );


//----------------------------------------------------------------------------
#endif
