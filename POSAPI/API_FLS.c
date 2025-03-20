//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : 							    **
//**  PRODUCT  : AS350 PLUS	                                            **
//**                                                                        **
//**  FILE     : API_FLS.C 	                                            **
//**  MODULE   : api_fls_write()			                    **
//**		 api_fls_read()						    **
//**									    **
//**									    **
//**  FUNCTION : API::FLS (Special Flash functions for CTLS Kernel)	    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2025/01/10                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2025 SymLink Corporation. All rights reserved.	    **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include "POSAPI.h"
#include "OS_FLASH.h"

#include "FLSAPI.h"

extern	void	PED_TripleDES( UINT8 *key, UINT16 len, UINT8 *idata, UINT8 *odata );	// defined in PEDK.OS_PED1.c
extern	void	PED_TripleDES2( UINT8 *key, UINT16 len, UINT8 *idata, UINT8 *odata );	//
extern	void	OS_FLS_GetData( UINT32 addr, UINT32 len, UINT8 *data );
extern	UINT32	OS_FLS_PutData( UINT32 addr, UINT32 len, UINT8 *data );

//extern	ULONG	api_sram_read( ULONG offset, ULONG length, UCHAR *data );	// for AS350
//extern	ULONG	api_sram_write( ULONG offset, ULONG length, UCHAR *data );	//

#define	SRAM_BASE_PCD_EMV_KEY		0x00000000			// 0x00003000		//
#define	SRAM_BASE_PCD_EMV_KEY_SIZE	MAX_CL_CAPK_CNT*CL_CAPK_LEN	//0x5000		// 20KB


ULONG	os_FLS_RW_CAPK_FLAG = 0;			// 0=access CAPK in SRAM (in DRAM for AS350+), 1=access CAPK in FLASH (NA)
UCHAR	DRAM_ADDR_CL_CAPK[SRAM_BASE_PCD_EMV_KEY_SIZE];	// AS350P key storage for CTLS CAPKs



// ---------------------------------------------------------------------------
// FUNCTION: To read the contents of RAM from the specified address.
// INPUT   : offset - linear address offset.
//	     length - length of data to be read.
// OUTPUT  : data
//	     Buffer to store the data read.
// RETURN  : apiOK
//           apiFailed
// NOTE	   : AS350+ alternative to api_sram_read()
// ---------------------------------------------------------------------------
ULONG	FLS_sram_read( ULONG offset, ULONG length, UCHAR *data )
{
UCHAR	*pAddr = &DRAM_ADDR_CL_CAPK;


	pAddr += offset;
	
	memmove( data, pAddr, length );
	
	return( apiOK );
}

// ---------------------------------------------------------------------------
// FUNCTION: To write data into the RAM memory according to the given address.
// INPUT   : offset - linear address offset.
//	     length - length of data to be written.
//	     data
//	     Buffer to store the data to be written.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// NOTE	   : AS350+ alternative to api_sram_write()
// ---------------------------------------------------------------------------
ULONG	FLS_sram_write( ULONG offset, ULONG length, UCHAR *data )
{	  
UINT8	*pAddr = &DRAM_ADDR_CL_CAPK;


	pAddr += offset;
	
	memmove( pAddr, data, length );
	
	return( apiOK );
}

// ---------------------------------------------------------------------------
//inline	static	
void	FLS_ReadIEK( UCHAR *sbuf )
{
UINT8	buf[64];
UINT8	kcv[8];
UINT8	XCSN[16];
UINT8	eIEK[16];
UINT8	IEK[16];

	
	// XCSN(16) =	CSN[0], 0x01, CSN[1], 0x23, CSN[2], 0x45, CSN[3], 0x67
	//		CSN[4], 0x89, CSN[5], 0xAB, CSN[6], 0xCD, CSN[7], 0xEF
	api_sys_info( SID_TerminalSerialNumber, buf );
	XCSN[0]  = buf[4];	// CSN[0]
	XCSN[1]  = 0x01;
	XCSN[2]  = buf[5];	// CSN[1]
	XCSN[3]  = 0x23;
	XCSN[4]  = buf[6];	// CSN[2]
	XCSN[5]  = 0x45;
	XCSN[6]  = buf[7];	// CSN[3]
	XCSN[7]  = 0x67;
	XCSN[8]  = buf[8];	// CSN[4]
	XCSN[9]  = 0x89;
	XCSN[10] = buf[9];	// CSN[5]
	XCSN[11] = 0xAB;
	XCSN[12] = buf[10];	// CSN[6]
	XCSN[13] = 0xCD;
	XCSN[14] = buf[11];	// CSN[7]
	XCSN[15] = 0xEF;
	
	memmove( eIEK, sbuf, 16 );
	
	PED_TripleDES2( XCSN, 16, eIEK, IEK );
	memmove( sbuf, IEK, 16 );
}

// ---------------------------------------------------------------------------
//inline	static	
void	FLS_WriteIEK( UCHAR *sbuf, UCHAR *dbuf )
{
UINT8	buf[64];
UINT8	kcv[8];
UINT8	XCSN[16];
UINT8	eIEK[16];
UINT8	IEK[16];

	
	// XCSN(16) =	CSN[0], 0x01, CSN[1], 0x23, CSN[2], 0x45, CSN[3], 0x67
	//		CSN[4], 0x89, CSN[5], 0xAB, CSN[6], 0xCD, CSN[7], 0xEF
	api_sys_info( SID_TerminalSerialNumber, buf );
	XCSN[0]  = buf[4];	// CSN[0]
	XCSN[1]  = 0x01;
	XCSN[2]  = buf[5];	// CSN[1]
	XCSN[3]  = 0x23;
	XCSN[4]  = buf[6];	// CSN[2]
	XCSN[5]  = 0x45;
	XCSN[6]  = buf[7];	// CSN[3]
	XCSN[7]  = 0x67;
	XCSN[8]  = buf[8];	// CSN[4]
	XCSN[9]  = 0x89;
	XCSN[10] = buf[9];	// CSN[5]
	XCSN[11] = 0xAB;
	XCSN[12] = buf[10];	// CSN[6]
	XCSN[13] = 0xCD;
	XCSN[14] = buf[11];	// CSN[7]
	XCSN[15] = 0xEF;
	
	memmove( eIEK, sbuf, 16 );
	
	PED_TripleDES( XCSN, 16, eIEK, IEK );
	memmove( dbuf, IEK, 16 );
}

// ---------------------------------------------------------------------------
// FUNCTION: To read the contents of FLASH from the specified address.
// INPUT   : id   - 0 = CAPK
//		    1 = PARAMETERS
//		    2 = IMEK
//		    3 = IAEK
//	     addr - offset address of the backup flash area.
//	     len  - length of data to be read.
// OUTPUT  : buf  - buffer to store the data to be read.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
ULONG	api_fls_read( UCHAR id, ULONG addr, ULONG len, UCHAR *buf )
{
ULONG	pFlsMem;
ULONG	status = apiOK;
UCHAR	temp[16];

	
	switch( id )
	      {
	      case FLSID_CAPK:
	           pFlsMem = F_ADDR_CL_CAPK;
	           pFlsMem += addr;
	           if( (pFlsMem + len) > F_ADDR_CL_CAPK_END )
	             status = apiFailed;
	           else
	             {
	             if( os_FLS_RW_CAPK_FLAG == 0 )
	               pFlsMem = SRAM_BASE_PCD_EMV_KEY + addr;
	             }
	           break;
	      
	      case FLSID_PARA:
	           pFlsMem = F_ADDR_CL_PARA;
	           pFlsMem += addr;
	           if( (pFlsMem + len) > F_ADDR_CL_PARA_END )
	             status = apiFailed;
	           break;
	           
	      case FLSID_IMEK:
	           pFlsMem = F_ADDR_CL_IMEK;
	           pFlsMem += addr;
	           if( ((pFlsMem + len) > F_ADDR_IMEK_END) || (len != 16) )
	             status = apiFailed;
	           break;
	           
	      case FLSID_IAEK:
	           pFlsMem = F_ADDR_CL_IAEK;
	           pFlsMem += addr;
	           if( ((pFlsMem + len) > F_ADDR_IAEK_END) || (len != 16) )
	             status = apiFailed;
	           break;

	      case FLSID_DATA:	// 2015-09-24
	      	
	      	   addr <<= 1;	// word boundary
	           pFlsMem = F_ADDR_CL_DATA;
	           pFlsMem += addr;
	           if( (pFlsMem + len) > F_ADDR_CL_DATA_END )
	             status = apiFailed;

	           break;

	      default:
	           return( apiFailed );
	      }

	if( status == apiOK )
	  {
	  if( id == FLSID_DATA )
	    {
	    OS_FLS_GetData( pFlsMem, len, buf );		// read data from FLASH
	    return( apiOK );
	    }

	  if( (id == FLSID_CAPK) && (os_FLS_RW_CAPK_FLAG == 0) )
	    status = FLS_sram_read( pFlsMem, len, buf );	// read CAPK from SRAM
	  else
	    OS_FLS_GetData( pFlsMem, len, buf );		// read CAPK or other data from FLASH
	
	  if( (id == FLSID_IMEK) || (id == FLSID_IAEK) )
	    {	      
	    FLS_ReadIEK( buf );
	    }
	  }
	    
	return( status );
}

// ---------------------------------------------------------------------------
// FUNCTION: To read the contents of FLASH from the specified address.
// INPUT   : id   - 0 = CAPK
//		    1 = PARAMETERS
//		    2 = IMEK
//		    3 = IAEK
//	     addr - offset address of the backup flash area.
//	     len  - length of data to be read.
// 	     buf  - buffer to store the data to be written.
//	     
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
ULONG	api_fls_write( UCHAR id, ULONG addr, ULONG len, UCHAR *buf )
{	
ULONG	pFlsMem;
ULONG	status = apiOK;
UCHAR	temp[16];

	  
	switch( id )
	      {
	      case FLSID_CAPK:
	           pFlsMem = F_ADDR_CL_CAPK;
	           pFlsMem += addr;
	           if( (pFlsMem + len) > F_ADDR_CL_CAPK_END )
	             status = apiFailed;
	           else
	             {
	             if( os_FLS_RW_CAPK_FLAG == 0 )
	               pFlsMem = SRAM_BASE_PCD_EMV_KEY + addr;
	             }
	           break;
	      
	      case FLSID_PARA:
	           pFlsMem = F_ADDR_CL_PARA;
	           pFlsMem += addr;
	           if( (pFlsMem + len) > F_ADDR_CL_PARA_END )
	             status = apiFailed;
	           break;
#if	1           
	      case FLSID_IMEK:
	           pFlsMem = F_ADDR_CL_IMEK;
	           pFlsMem += addr;
	           if( (pFlsMem + len) > F_ADDR_IMEK_END )
	             status = apiFailed;
	           break;
	           
	      case FLSID_IAEK:
	           pFlsMem = F_ADDR_CL_IAEK;
	           pFlsMem += addr;
	           if( (pFlsMem + len) > F_ADDR_IAEK_END )
	             status = apiFailed;
	           break;
#endif

	      case FLSID_DATA:	// 2015-07-20
	      	
	      	   addr <<= 1;	// word boundary
	           pFlsMem = F_ADDR_CL_DATA;
	           pFlsMem += addr;
	           if( (pFlsMem + len) > F_ADDR_CL_DATA_END )
	             status = apiFailed;

	           break;

	      default:
	           return( apiFailed );
	      }

	if( status == apiOK )
	  {
	  if( id == FLSID_DATA )
	    {
	    if( OS_FLS_PutData( pFlsMem, len, buf ) == apiOK )	// write data to FLASH
	      return( apiOK );
	    else
	      return( apiFailed );
	    }

	  if( (id == FLSID_IMEK) || (id == FLSID_IAEK) )
	    {
	    FLS_WriteIEK( buf, temp );
	    if( OS_FLS_PutData( pFlsMem, len, temp ) == apiOK )
	      return( apiOK );
	    }
	  else
	    {
	    if( (id == FLSID_CAPK) && (os_FLS_RW_CAPK_FLAG == 0) )
	      {
	      status = FLS_sram_write( pFlsMem, len, buf );	// write CAPK to SRAM
	      return( status );
	      }
	    else
	      {
	      if( OS_FLS_PutData( pFlsMem, len, buf ) == apiOK ) // write CAPK or other data to FLASH
	        return( apiOK );
	      }
	    }
	  }

	return( apiFailed );
}
