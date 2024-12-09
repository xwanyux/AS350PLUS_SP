//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ECC	                                                    **
//**  PRODUCT  : AS350	                                                    **
//**                                                                        **
//**  FILE     : FLASHAPI.H						    **
//**  MODULE   : Declaration of NOR/NAND FLASH Module.		    	    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2017/03/31                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2017 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _FLASH_API_H_
#define _FLASH_API_H_


//----------------------------------------------------------------------------
#define	TYPE_NOR			0			// default flash type
#define	TYPE_NAND			1			//


//----------------------------------------------------------------------------
typedef	struct API_FLASH_S
{
	ULONG		Type;					// Flash type (0=NOR FLASH, 1=NAND FLASH)
	ULONG		StPage;					// start page no
	ULONG		EndPage;				// end   page no
	ULONG		StAddr;					// start address
	ULONG		EndAddr;				// end   address
	ULONG		Len;					// data length
}  __attribute__((packed)) API_FLASH;


typedef	struct API_FLASH_ADDR_S
{
	ULONG		Type;					// Flash type (0=NOR FLASH, 1=NAND FLASH)
	ULONG		StAddr;					// start address
	ULONG		EndAddr;				// end   address
} __attribute__((packed)) API_FLASH_ADDR;


//----------------------------------------------------------------------------
//	Function Prototypes for USB Device
//----------------------------------------------------------------------------
extern	ULONG	api_flash_TypeSelect( ULONG Type );
extern	ULONG	api_flash_PageSelect( UINT Page, API_FLASH_ADDR *pAddr );
extern	ULONG	api_flash_PageLink( API_FLASH pFls, UCHAR Action );

extern	ULONG	api_flash_PageRead( API_FLASH pFls, UCHAR *pData );
extern	ULONG	api_flash_PageWrite( API_FLASH pFls, UCHAR *pData );
extern	ULONG	api_flash_PageClear( API_FLASH pFls, UCHAR Pattern );


//----------------------------------------------------------------------------
#endif
