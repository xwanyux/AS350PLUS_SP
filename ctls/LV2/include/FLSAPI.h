//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  :                                                            **
//**  PRODUCT  : AS210	                                                    **
//**                                                                        **
//**  FILE     : FLSAPI.H     	          	                            **
//**  MODULE   : Declaration of FLASH APIs. 		                    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2013/07/08                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2013 SymLink Corporation. All rights reserved.	    **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _FLSAPI_H_
#define _FLSAPI_H_


#define FLS_ADDRESS_IMEK				0
#define FLS_ADDRESS_IAEK				0

//
//		Define Even Number Address to Prevent api_fls_read Error
//
#define FLS_ADDRESS_IMEK_MDK				0		// 16	bytes
#define FLS_ADDRESS_MEK						16		// 16	bytes
#define FLS_ADDRESS_IAEK_MDK				32		// 16	bytes
#define FLS_ADDRESS_AEK						48		// 16	bytes
#define FLS_ADDRESS_Scheme					64		// 34	bytes
#define FLS_ADDRESS_Parameter				98		// 128	bytes
#define FLS_ADDRESS_EMV_Tag					226		// 256	bytes
#define FLS_ADDRESS_Message					482		// 1320	bytes
#define FLS_ADDRESS_CVM						1802	// 8	bytes
#define FLS_ADDRESS_APID					1810	// 166	bytes	Total APID number(1 byte) + Total Data Bytes(1 bytes)+ Data(APID(41 bytes))*4
#define FLS_ADDRESS_ExceptionFile			1976	// 112	bytes	Total File num + (L(1 byte) + V(10 bytes))*10
#define FLS_ADDRESS_BaudRate				2088	// 2	bytes
#define FLS_ADDRESS_VLP_Support				2090	// 2	bytes
#define FLS_ADDRESS_RevoList				2092	// 102	bytes	Total Revo List(1 bytes) + (L(1 byte) + V(10 bytes))*10
#define FLS_ADDRESS_DRLEnable				2194	// 2	bytes
#define FLS_ADDRESS_Private_Msg				2196	// 330	bytes
#define FLS_ADDRESS_Cash_Data				2526	// 20	bytes
#define FLS_ADDRESS_CashBack_Data			2546	// 20	bytes
#define FLS_ADDRESS_MSDOption				2566	// 4	bytes
#define FLS_ADDRESS_CONFIGURATION_DATA		2570	// 514	bytes
#define FLS_ADDRESS_MULTIAID_ENABLE			3084	// 2	bytes
#define FLS_ADDRESS_MULTIAID_DATA			3086	// 176	bytes
#define FLS_ADDRESS_NCCC_PARAMETER			3262	// 22	bytes
#define FLS_ADDRESS_MULTIAID_ENABLE_DPAS	3284	// 2	bytes
#define FLS_ADDRESS_MULTIAID_DATA_DPAS		3286	// 118	bytes


//		Flash Buffer Size
#define FLS_SIZE_CONFIGURATION_DATA		514		//Len(2) + Data(512)

#define FLS_SIZE_MULTIAID_DATA			(2+FLS_SIZE_MULTIAID_AID_SET*FLS_NUMBER_AID_SET_UPI)		//Len(2) + 3 Sets AID for UPI
#define FLS_SIZE_MULTIAID_DATA_DPAS		(2+FLS_SIZE_MULTIAID_AID_SET*FLS_NUMBER_AID_SET_DISCOVER)	//Len(2) + 2 Sets AID for Discover
#define FLS_SIZE_MULTIAID_AID_SET		58

#define FLS_NUMBER_AID_SET_UPI			3
#define FLS_NUMBER_AID_SET_DISCOVER		2

//----------------------------------------------------------------------------
//		FLASH READ/WRITE ID
//----------------------------------------------------------------------------
#define		FLSID_CAPK			0
#define		FLSID_PARA			1
#define		FLSID_IMEK			2
#define		FLSID_IAEK			3

#define		FLSType_SRAM		0
#define		FLSType_Flash		1


//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
extern	ULONG	api_fls_read( UCHAR id, ULONG addr, ULONG len, UCHAR *buf );
extern	ULONG	api_fls_write( UCHAR id, ULONG addr, ULONG len, UCHAR *buf );


//----------------------------------------------------------------------------
//20140105
/*
	
	ULONG	 api_fls_memtype( UCHAR id, UCHAR type )
	INPUT:	   

	id = key id
	CAPK, or PARA, IMEK, IAEK
	PARA, IMEK, IAEK�u�n���w��@�A�N�i�H�@�_�ͮġC

	Type = memory type
	0 = SRAM (internal or external SRAM)
	1 = FLASH (system default)

	OUTPUT: none
	RETURN: apiOK, apiFailed
*/
extern  ULONG	api_fls_memtype( UCHAR id, UCHAR type );

#endif
