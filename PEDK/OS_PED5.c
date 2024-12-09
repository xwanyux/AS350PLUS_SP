//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : i.MX6UL                                                    **
//**  PRODUCT  : AS350-X6                                                   **
//**                                                                        **
//**  FILE     : OS_PED5.C                                                  **
//**  MODULE   : 			   				                                **
//**                                                                        **
//**  FUNCTION : OS::PED5 (PCI PED Entry for CAPK)			                **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2022/05/24                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2022 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include "PEDKconfig.h"

#include "POSAPI.h"

#ifdef	_CAPK_IN_SRAM_
#include "DEV_SRAM.h"
#else
#include "OS_SECM.h"
#include "OS_CAPK.h"
#endif


// ---------------------------------------------------------------------------
// FUNCTION: verify CA public key by checking HASH value.
// INPUT   : capk - the key components of one CAPK.
//
//	     CAPK Structure in SRAM (fixed 300 bytes per key slot)
//		-------------------------	-------------
//		Key Components			Size in bytes
//		-------------------------	-------------
//		RID				5
//		INDEX				1
//		EXPONENT LENGTH			1
//		MODULUS LENGTH			2
//		HASH				20
//		EXPONENT			3
//		MODULUS				256
//		HASH ALGORITHM INDICATOR	1
//		PK ALGORITHM INDICATOR		1
//		RFU				10
//		-------------------------	-------------
//		where: HASH(20)=SHA1(RID+INDEX+MOD+EXP)
//
// OUTPUT  : none.
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
UINT32	PED_CAPK_VerifyKey( UINT8 *capk )
{
#ifdef	_CAPK_IN_SRAM_

UINT8	temp[CAPK_KEY_SLOT_LEN2];
UINT8	hash[20];
UINT8	explen;
UINT16	modlen;
UINT32	iLen;


	if( LIB_memcmpc( capk, 0x00, CAPK_KEY_SLOT_LEN2 ) == 0 )
	  return( FALSE ); // ignored if NULL key data
	  
	if( LIB_memcmpc( &capk[9], 0xFF, 20 ) == 0 )	// skip checking CAPK HASH if they are all 0xFF
	  return( TRUE );

	explen = capk[6];
	modlen = capk[7]*256 + capk[8];
                       
	memmove( temp, capk, 5 );			// RID(5)
	memmove( &temp[5], &capk[5], 1 );		// index(1)
	memmove( &temp[6], &capk[29+explen], modlen );	// modulus(x), 2011-11-10
	memmove( &temp[6+modlen], &capk[29], explen );	// exponent(y)
                       
	iLen = 5 + 1 + modlen + explen;
	api_sys_SHA1( iLen, temp, hash );	// generate digest of SHA1
//		LIB_DumpHexData( 0, 0, 20, &capk[9] );
//		LIB_DumpHexData( 0, 4, 20, hash );
	if( LIB_memcmp( hash, &capk[9], 20 ) == 0 )	// HASH value identical?
	  return( TRUE );
	else
	  return( FALSE );
#else

UINT8	temp[CAPK_KEY_SLOT_LEN];
UINT8	hash[20];
UINT8	explen;
UINT16	modlen;
UINT32	iLen;


	if( LIB_memcmpc( capk, 0x00, CAPK_KEY_SLOT_LEN2 ) == 0 )
	  return( FALSE ); // ignored if NULL key data
	  
	if( LIB_memcmpc( &capk[9], 0xFF, 20 ) == 0 )	// skip checking CAPK HASH if they are all 0xFF
	  return( TRUE );

	explen = capk[6];
	modlen = capk[7]*256 + capk[8];
                       
	memmove( temp, capk, 5 );			// RID(5)
	memmove( &temp[5], &capk[5], 1 );		// index(1)
	memmove( &temp[6], &capk[29+explen], modlen );	// modulus(x), 2011-11-10
	memmove( &temp[6+modlen], &capk[29], explen );	// exponent(y)
                       
	iLen = 5 + 1 + modlen + explen;
	api_sys_SHA1( iLen, temp, hash );	// generate digest of SHA1
//		LIB_DumpHexData( 0, 0, 20, &capk[9] );
//		LIB_DumpHexData( 0, 4, 20, hash );
	if( LIB_memcmp( hash, &capk[9], 20 ) == 0 )	// HASH value identical?
	  return( TRUE );
	else
	  return( FALSE );
#endif
}

// ---------------------------------------------------------------------------
// FUNC  : To get the info of CAPK's in SAM.
// INPUT : none.
// OUTPUT: info - RID[5]           : 00~04
//                IndexNo0[1]      : 05
//                Index0[15]       : 06~20
//
//                RID[5]           : 21~25
//                IndexNo1[1]      : 26
//                Index1[15]       : 27~41
//
//                RID[5]           : 42~46
//                IndexNo1[1]      : 47
//                Index1[15]       : 48~62
//                ..............
//                0xFF
//
//                IndexNo   : Number of key index
//                Index     : Key index
//                sizeof(info) = 210 bytes. (21 bytes * 10 payment systems)
//				 one payment system max. 15 indexes.
//
// OUTPUT: none.
// RETURN: none.
// NOTE  : call this function after api_emvk_InitEMVKernel().
// ---------------------------------------------------------------------------
void	PED_CAPK_GetInfo( UINT8 *info )
{
#ifdef	_CAPK_IN_SRAM_

UINT8	i,j;
UINT8	cnt;
UINT8	buf[CAPK_KEY_SLOT_LEN2];
UINT32	iAddr;


	memset( info, 0x00, 21*MAX_PAYMENT_SCHEME_CNT );
	for( i=0; i<MAX_PAYMENT_SCHEME_CNT; i++ )
	   info[21*i] = 0xff;

	iAddr = 0;
	
      for( i=0; i<MAX_CAPK_CNT; i++ )
         {
         OS_SRAM_GetKeyData( iAddr, CAPK_KEY_SLOT_LEN2, buf );	// get CAPK
         
//       if( PED_CAPK_VerifyKey( buf ) )		// verify CAPK
//         {
           if( (LIB_memcmpc( buf, 0x00, RID_LEN ) != 0) && (LIB_memcmpc( buf, 0xFF, RID_LEN ) != 0) ) // RID=0's ? (last key slot)
             {
             // compare RID
             for( j=0; j<MAX_PAYMENT_SCHEME_CNT; j++ )
                {
                if( LIB_memcmp( buf, &info[21*j], RID_LEN ) == 0 )
                  {
                  cnt = info[21*j+5];
                  if( cnt < MAX_INDEX_CNT )
                    {
                    info[21*j+5] += 1;		// IndexNo+1
                    info[21*j+cnt+6] = buf[5];	// index
		    }
                  break;
                  }
                else
                  {
                  if( info[21*j] == 0xFF )
                    {
                    memmove( &info[21*j], buf, RID_LEN ); // setup new RID

                    cnt = info[21*j+5];
                    if( cnt < MAX_INDEX_CNT )
                      {
                      info[21*j+5] += 1;		// IndexNo+1
                      info[21*j+cnt+6] = buf[5];	// index
		      }
                    break;
                    }
                  }
                }   
             }
           else
             break;
//         }
         iAddr += CAPK_KEY_SLOT_LEN2;	// next key slot
         }
#else

UINT8	i,j;
UINT8	cnt;
UINT8	buf[CAPK_KEY_SLOT_LEN];
UINT32	iAddr;


	memset( info, 0x00, 21*MAX_PAYMENT_SCHEME_CNT );
	for( i=0; i<MAX_PAYMENT_SCHEME_CNT; i++ )
	   info[21*i] = 0xff;

	iAddr = 0;
	
      for( i=0; i<MAX_CAPK_CNT; i++ )
         {
         OS_SECM_GetKeyData( iAddr, CAPK_KEY_SLOT_LEN, buf );	// get CAPK
         
//       if( PED_CAPK_VerifyKey( buf ) )		// verify CAPK
//         {
           if( (LIB_memcmpc( buf, 0x00, RID_LEN ) != 0) && (LIB_memcmpc( buf, 0xFF, RID_LEN ) != 0) ) // RID=0's ? (last key slot)
             {
             // compare RID
             for( j=0; j<MAX_PAYMENT_SCHEME_CNT; j++ )
                {
                if( LIB_memcmp( buf, &info[21*j], RID_LEN ) == 0 )
                  {
                  cnt = info[21*j+5];
                  if( cnt < MAX_INDEX_CNT )
                    {
                    info[21*j+5] += 1;		// IndexNo+1
                    info[21*j+cnt+6] = buf[5];	// index
		    }
                  break;
                  }
                else
                  {
                  if( info[21*j] == 0xFF )
                    {
                    memmove( &info[21*j], buf, RID_LEN ); // setup new RID

                    cnt = info[21*j+5];
                    if( cnt < MAX_INDEX_CNT )
                      {
                      info[21*j+5] += 1;		// IndexNo+1
                      info[21*j+cnt+6] = buf[5];	// index
		      }
                    break;
                    }
                  }
                }   
             }
           else
             break;
//         }
         iAddr += CAPK_KEY_SLOT_LEN;	// next key slot
         }
#endif
}

// ---------------------------------------------------------------------------
// FUNCTION: To read the CAPK header by key slot index.
// INPUT   : index - key slot index. (00..nn)
// OUTPUT  : pkh   - CA public key header data as follows. (fixed 29 bytes)
//                   RID		5  bytes
//		     KEY INDEX		1  byte
//		     EXP LENGTH		1  byte
//		     MOD LENGTH		2  bytes (H-L)
//		     HASH		20 bytes
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UINT8	PED_CAPK_GetKeyHeader( UINT8 index, UINT8 *pkh )
{
#ifdef	_CAPK_IN_SRAM_

UINT32	iAddr;
UINT8	capk[CAPK_KEY_SLOT_LEN2];


	// read CAPK
	iAddr = index*CAPK_KEY_SLOT_LEN2;
	if( OS_SRAM_GetKeyData( iAddr, CAPK_KEY_SLOT_LEN2, capk ) != TRUE )
	  return( apiFailed );
	
	// verify CAPK
	if( PED_CAPK_VerifyKey( capk ) )
	  {
	  memmove( pkh, capk, CAPK_HEADER_LEN );
	  return( apiOK );
	  }
	else
	  return( apiFailed );
#else

UINT32	iAddr;
UINT8	capk[CAPK_KEY_SLOT_LEN];


	PED_InUse( TRUE );
	
	// read CAPK
	iAddr = index*CAPK_KEY_SLOT_LEN;
	OS_SECM_GetKeyData( iAddr, CAPK_KEY_SLOT_LEN, capk );

	// verify CAPK
	if( PED_CAPK_VerifyKey( capk ) )
	  {
	  memmove( pkh, capk, CAPK_HEADER_LEN );
	  
	  PED_InUse( FALSE );
	  return( apiOK );
	  }
	else
	  {
	  PED_InUse( FALSE );
	  return( apiFailed );
	  }
#endif
}

// ---------------------------------------------------------------------------
// FUNCTION: To select CAPK by the specified public key INDEX & RID.
// INPUT   : pki   - CA public key index.
//           rid   - registered application provider id. (fixed 5 bytes)
// OUTPUT  : pkh   - CA public key header data as follows. (fixed 29 bytes)
//                   RID		5  bytes
//		     KEY INDEX		1  byte
//		     EXP LENGTH		1  byte
//		     MOD LENGTH		2  bytes (H-L)
//		     HASH		20 bytes
//	     index - the selected key slot index.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UINT8	PED_CAPK_SelectKey( UINT8 pki, UINT8 *rid, UINT8 *pkh, UINT8 *index )
{
#ifdef	_CAPK_IN_SRAM_

UINT32	i;
UINT32	iAddr;
UINT32	status;
UINT8	capk[CAPK_KEY_SLOT_LEN2];
UINT8	pkm[258];
UINT8	pke[3];


	status = FALSE;
	iAddr = 0;
	for( i=0; i<MAX_CAPK_CNT; i++ )
	   {
	   if( OS_SRAM_GetKeyData( iAddr+(i*CAPK_KEY_SLOT_LEN2), CAPK_KEY_SLOT_LEN2, capk ) != TRUE )	// read CAPK components
	     return( apiFailed );
	   
	   // EOF?
	   if( (LIB_memcmpc( &capk[OFFSET_CAPK_RID], 0x00, RID_LEN ) == 0) || ( LIB_memcmpc( &capk[OFFSET_CAPK_RID], 0xFF, RID_LEN ) == 0) )
	     break;

	   // matching RID & INDEX
	   if( (LIB_memcmp( rid, &capk[OFFSET_CAPK_RID], 5 ) == 0) && (pki == capk[OFFSET_CAPK_PKI]) )
	     {
	     // verify key
	     status = PED_CAPK_VerifyKey( capk );
	     *index = i;
	     break;
	     }
	   }

	if( status )
	  {
	  memmove( pkh, capk, CAPK_HEADER_LEN ); // get header data
	  
	  pkm[0] = capk[OFFSET_CAPK_MOD_LEN+1];	// length of modulus
	  pkm[1] = capk[OFFSET_CAPK_MOD_LEN+0];	//
	
	  if( capk[OFFSET_CAPK_EXP_LEN] == 1 )	// PATCH: 2011-11-11
	    memmove( &pkm[2], &capk[OFFSET_CAPK_MOD1], pkm[0]+pkm[1]*256 );	// modulus (exp_len = 1)
	  else
	    memmove( &pkm[2], &capk[OFFSET_CAPK_MOD3], pkm[0]+pkm[1]*256 );	// modulus (exp_len = 3)
//	  memmove( &pkm[2], &capk[OFFSET_CAPK_MOD], pkm[0]+pkm[1]*256 );	// modulus
	  
	  memmove( pke, &capk[OFFSET_CAPK_EXP], 3 );				// exponent
	  
	  status = api_rsa_loadkey( pkm, pke );	// load the selected key
	  }
	else
	  status = apiFailed;	// PATCH: 2011-11-11

	return( status );
#else

UINT32	i;
UINT32	iAddr;
UINT32	status;
UINT8	capk[CAPK_KEY_SLOT_LEN];
UINT8	pkm[258];
UINT8	pke[3];


	PED_InUse( TRUE );
	
	status = FALSE;
	iAddr = 0;
	for( i=0; i<MAX_CAPK_CNT; i++ )
	   {
	   OS_SECM_GetKeyData( iAddr+(i*CAPK_KEY_SLOT_LEN), CAPK_KEY_SLOT_LEN, capk );	// read CAPK components
	   
	   // EOF?
	   if( (LIB_memcmpc( &capk[OFFSET_CAPK_RID], 0x00, RID_LEN ) == 0) || ( LIB_memcmpc( &capk[OFFSET_CAPK_RID], 0xFF, RID_LEN ) == 0) )
	     break;

	   // matching RID & INDEX
	   if( (LIB_memcmp( rid, &capk[OFFSET_CAPK_RID], 5 ) == 0) && (pki == capk[OFFSET_CAPK_PKI]) )
	     {
	     // verify key
	     status = PED_CAPK_VerifyKey( capk );
	     *index = i;
	     break;
	     }
	   }

	if( status )
	  {
	  memmove( pkh, capk, CAPK_HEADER_LEN ); // get header data
	  
	  pkm[0] = capk[OFFSET_CAPK_MOD_LEN+1];	// length of modulus
	  pkm[1] = capk[OFFSET_CAPK_MOD_LEN+0];	//
	
	  if( capk[OFFSET_CAPK_EXP_LEN] == 1 )	// PATCH: 2011-11-11
	    memmove( &pkm[2], &capk[OFFSET_CAPK_MOD1], pkm[0]+pkm[1]*256 );	// modulus (exp_len = 1)
	  else
	    memmove( &pkm[2], &capk[OFFSET_CAPK_MOD3], pkm[0]+pkm[1]*256 );	// modulus (exp_len = 3)
	  
	  memmove( pke, &capk[OFFSET_CAPK_EXP], 3 );				// exponent
	  
	  status = api_rsa_loadkey( pkm, pke );	// load the selected key
	  }
	else
	  status = apiFailed;	// PATCH: 2011-11-11

	PED_InUse( FALSE );
	
	return( status );
#endif
}

// ---------------------------------------------------------------------------
// FUNCTION: Erase all CA Public Keys.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_EraseCAPK( void )
{
#ifdef	_CAPK_IN_SRAM_

UINT32	i;

	
	for( i=0; i<MAX_CAPK_CNT; i++ )
	   {
	   OS_SRAM_ClearKeyData( i*CAPK_KEY_SLOT_LEN2, CAPK_KEY_SLOT_LEN2, 0x00 );
	   }
#else

UINT32	i;

	
	for( i=0; i<MAX_CAPK_CNT; i++ )
	   {
	   OS_SECM_ClearKeyData( i*CAPK_KEY_SLOT_LEN, CAPK_KEY_SLOT_LEN, 0x00 );
	   }
#endif
}

// ---------------------------------------------------------------------------
// FUNCTION: To setup one fixed key at the specified key slot.
// INPUT   : key   - CAPK components. (fixed 300 bytes)
//	     index - the selected key slot index.
//
//	     CAPK Structure in SRAM (fixed 300 bytes per key slot)
//		-------------------------	-------------
//		Key Components			Size in bytes
//		-------------------------	-------------
//		RID				5
//		INDEX				1
//		EXPONENT LENGTH			1
//		MODULUS LENGTH			2
//		HASH				20
//		EXPONENT			3
//		MODULUS				256
//		HASH ALGORITHM INDICATOR	1
//		PK ALGORITHM INDICATOR		1
//		RFU				10
//		-------------------------	-------------
//
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UINT8	PED_SetCAPK( UINT32 index, UINT8 *key )
{
#ifdef	_CAPK_IN_SRAM_

UINT8	result;
UINT8	buf1[32];
UINT8	buf2[32];


	result = apiFailed;
	
	if( index < MAX_CAPK_CNT )	// boundary check
	  {
	  if( PED_CAPK_VerifyKey( key ) )
	    {
//	    OS_SRAM_GetKeyData( index*CAPK_KEY_SLOT_LEN2, sizeof(buf1), buf1 );
//	    memset( buf2, 0x00, sizeof(buf1) );
//	    
//	    if( LIB_memcmp( buf1, buf2, sizeof(buf1) ) == 0 )	// empty key slot?
//	      {
	      if( OS_SRAM_PutKeyData( index*CAPK_KEY_SLOT_LEN2, CAPK_KEY_SLOT_LEN2, key ) )
	        result = apiOK;
//	      }
	    }
	  }
	  
	return( result );
#else

UINT8	result;
UINT8	buf1[32];
UINT8	buf2[32];


	result = apiFailed;
	
	if( index < MAX_CAPK_CNT )	// boundary check
	  {
	  if( PED_CAPK_VerifyKey( key ) )
	    {
	    OS_SECM_PutKeyData( index*CAPK_KEY_SLOT_LEN, CAPK_KEY_SLOT_LEN, key );
	    result = apiOK;
	    }
	  }
	  
	return( result );
#endif
}
