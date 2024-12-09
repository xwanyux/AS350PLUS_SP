//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS350-X6						                            **
//**                                                                        **
//**  FILE     : L2API_SRED.C						                        **
//**  MODULE   : api_emv_ProcessingRestrictions()                           **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2022/05/24                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2022 SymLink Corporation. All rights reserved.	        **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include "POSAPI.h"
#include "EMVDC.h"
#include "GDATAEX.h"
#include "EMVAPI.h"
#include "EMVAPK.h"
#include "SRED_Func.h"

//extern	UCHAR	SRED_Func_StorePAN( UCHAR *pan, UCHAR length );
//extern	UCHAR	SRED_Func_GetPAN( UCHAR mode, UCHAR *data, UCHAR *length );
//extern	UINT32	SRED_VerifySredStatus(void);


// ---------------------------------------------------------------------------
// FUNCTION: save data element to PAGE SRAM. (PAN)
// INPUT   : source  - data source (only ICC)
//         : address - begin address to write.
//           length  - length of the data element.
//           data    - the data element to be saved.
// OUTPUT  : none.
// RETURN  : emvOK
//           emvFailed
// ---------------------------------------------------------------------------
UCHAR	api_emv_PutDataElement_SRED( UCHAR source, ULONG address, ULONG length, UCHAR *data )
{
UCHAR	KeyScheme;
UCHAR	Tag;


	if( source != DE_ICC )
	  return( emvFailed );
 
//#ifndef	_SRED_ENABLED_
//	apk_WriteRamDataICC( address, length, data );
//#else
//	SRED_Func_StorePAN( data, length );
//#endif

	apk_WriteRamDataICC( address, length, data );	//
	
//	SRED_Func_StorePAN( data, length );		// 2019-03-21

//	TL_DumpHexData( 0, 0, length, data );

	KeyScheme = api_ped_GetKeyMode();
	Tag = 0x5A;	// PAN
	SRED_Func_StoreDataElement( &Tag, length, data, KeyScheme, g_key_index );	// 2019-04-17

	return( emvOK );
}

// ---------------------------------------------------------------------------
// FUNCTION: get data element from PAGE SRAM. (PAN)
// INPUT   : source  - data source (only ICC).
//         : address - begin address to write.
//           length  - length of the data element.
// OUTPUT  : data    - the data element read.
// RETURN  : emvOK
//           emvFailed
// ---------------------------------------------------------------------------
UCHAR	api_emv_GetDataElement_SRED( UCHAR source, ULONG address, ULONG length, UCHAR *data )
{
ULONG	i;
ULONG	len;
ULONG	len2;
UCHAR	Tag;
UCHAR	buf[10];


	if( source != DE_ICC )
	  return( emvFailed );
	  
//#ifndef	_SRED_ENABLED_
//	apk_ReadRamDataICC( address, length, data );
//#else
//	SRED_Func_GetPAN( 2, data, &len );	// get encrypted PAN
//#endif

	if( !SRED_VerifySredStatus() )	// 2019-03-21, SRED enabled?
	  apk_ReadRamDataICC( address, length, data );
	else
	  {
//	  SRED_Func_GetPAN( 1, data, &len );	// get encrypted PAN
	  					// 0=plaintext PAN data
	  					// 1=only first six and last four digits are plaintext
	  					// 2=the whole PAN data is encrypted
	  Tag = 0x5A;	// PAN		
	  SRED_Func_GetDataElement( 1, &Tag, data, &len );
//	  TL_DumpHexData( 0, 0, len, data );

	  memset( buf, 0xff, sizeof(buf) );
	  len2 = len/2;
	  for( i=0; i<len2; i++ )
	     buf[i] = ((data[i*2] & 0x0F) << 4) | (data[i*2+1] & 0x0F);
	  
	  if( len & 1 )
	    {
	    buf[i] = ((data[i*2] & 0x0F) << 4) | 0x0F;
	    len2++;
	    }
	  
	  memmove( data, buf, sizeof(buf) );
	  }
}
