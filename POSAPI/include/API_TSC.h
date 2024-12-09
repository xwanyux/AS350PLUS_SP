//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  :                                                            **
//**  PRODUCT  : AS350	                                                    **
//**                                                                        **
//**  FILE     : TSCAPI.H						    **
//**  MODULE   : Declaration of TSC Module.		    		    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2013/08/06                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2013 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _TSC_API_H_
#define _TSC_API_H_


//----------------------------------------------------------------------------
//		TSC Function ID
//----------------------------------------------------------------------------
#define		FID_TSC_OPEN			0
#define		FID_TSC_CLOSE			1
#define		FID_TSC_BUTTON			2
#define		FID_TSC_SIGNPAD			3
#define		FID_TSC_STATUS_BUTTON		4
#define		FID_TSC_STATUS_SIGNPAD		5
#define		FID_TSC_GET_SIGNATURE		6

//----------------------------------------------------------------------------
//		Signature Status
//----------------------------------------------------------------------------
#define		SIGN_STATUS_REVERSE		0x08

#define		SIGN_STATUS_PROCESSING		0x80	// in processing
#define		SIGN_STATUS_NOT_READY		0x00	// signature not available
#define		SIGN_STATUS_READY		0x01	// signature is available
#define		SIGN_STATUS_ROTATE		0x02	// request to rotate sign pad
#define		SIGN_STATUS_READY_REVERSE	SIGN_STATUS_READY + SIGN_STATUS_REVERSE	// signature is available in reverse direction
#define		SIGN_STATUS_CLEAR		0x03	// request to clear (retry) on sign pad
#define		SIGN_STATUS_TIMEOUT		0x04	// timeout
#define		SIGN_STATUS_EXIT		0x05	// exit pinpad function


//----------------------------------------------------------------------------
//		TSC 
//----------------------------------------------------------------------------
#define		DEV_OPEN		0x01
#define		DEV_CLOSE		0x00
//----------------------------------------------------------------------------
typedef	struct	API_TSC_PARA_S			// TSC parameters
{
	UINT		ID;			// function ID
	UINT		X;			// object X position
	UINT		Y;			// object Y position
	UINT		Width;			// object width
	UINT		Height;			// object height
	UINT		Timeout;		// in 10ms units
	UINT		RxLen;			// length of data to be read
	UCHAR		RFU[2];			// RFU
	int			fd;
} __attribute__((packed)) API_TSC_PARA;



//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
// extern	UCHAR	api_tsc_open( UCHAR deviceid );
// extern	UCHAR	api_tsc_close( UINT16 dhn );
// extern	UCHAR	api_tsc_status( UCHAR dhn, UCHAR *sbuf, UCHAR *status );
// extern	UCHAR	api_tsc_signpad( UCHAR dhn, UCHAR *sbuf, UCHAR *status, UCHAR *palette );
// extern	UCHAR	api_tsc_getsign( UCHAR dhn, UCHAR *sbuf, UCHAR *status, UCHAR *dbuf );

extern	UCHAR	api_tsc_open( UCHAR deviceid );
extern	UCHAR	api_tsc_close( UCHAR dhn );
extern	UCHAR	api_tsc_status( UCHAR dhn, API_TSC_PARA para, UCHAR *STATUS );
extern	UCHAR	api_tsc_signpad( UCHAR dhn, API_TSC_PARA para, UCHAR *status, UCHAR *palette );
extern	UCHAR	api_tsc_getsign( UCHAR dhn, API_TSC_PARA para, UCHAR *status, UCHAR *sign, ULONG *length );

//----------------------------------------------------------------------------
#endif
