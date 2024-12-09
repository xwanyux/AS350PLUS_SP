//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : 							                                **
//**  PRODUCT  : AS350-X6							                        **
//**                                                                        **
//**  FILE     : ANS_TR31_2010.C					                        **
//**  MODULE   : 			   				                                **
//**                                                                        **
//**  FUNCTION : Key Bundle Management					                    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2022/05/24                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2008-2022 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
// CBC MAC Key Block Format	(X9 TR-31 2010)
// +--------+-----------------+------------+------------+---------+-------+	
// | Header | Header(optional)| Key Length |	Key	| Padding |  MAC  |
// |--------|-----------------|------------|------------|---------|-------|
// | 16-byte|                 | 2-byte     | 16-Byte    | 6-byte  | 8-byte|
// +--------+-----------------+------------+------------+---------+-------+
//                            |<---         encrypted         --->|
// |<---			MAC			      --->|
//
// Key Block Header (KBH) Format
// BYTE#	Field Name		Encoding	Encrypted
// -----	--------------------	--------	---------
// 0		Key Block Version ID	1AN		No ('B')
// 1-4		Key Block Length	4N		No (eg. "0080" in decimal after encoding)
// 5-6		Key Usage		2AN		No
// 7		Algorithm		1AN		No
// 8		Mode of Use		1AN		No
// 9-10		Key Version Number	2AN		No
// 11		Expotability		1AN		No
// 12-13	No. of Optional Blocks	2N		No
// 14-15	RFU			2N		No
// 16...	First Optional Block
//
// Key Block Length = KBH(16) + 2*[Key Length(2) + KEY(16) + PAD(6) + MAC(8)] = 80 bytes after encoding in hex-ASCII
//
// Key Block Protection Key:	The KEK from which the "Key Block Encryption Key" and "Key Block MAC Key" are derived.
//				It was previously exchanged between two communication parties. (KBPK)
// Key Block Encryption Key:	Used for enciphering the Key Block.
// Key Block MAC Key:		Used for calculation MAC over the Key Block.
// Key Block Binding Method:	A technique used to protect the secrecy and integrity of the Key Block.
//				Using "key derivation" according to X9 TR-31 2010.
//
// Key Loader <-> TRSM (Exchange KBPK)
// Key Block   -> TRSM, TRSM Shall:
// 1. Make sure the length of the key block matches the contents of bytes 1-4.
//    If not match, returns an error and stop processing the block.
// 2. Check "Key Usage".
// 3. Check "Alogrithm".
// 4. Check other header bytes depending on 2 and 3.
// 
//----------------------------------------------------------------------------
#include <stdlib.h>
//#include <stdio.h>
#include <string.h>

#include "PEDKconfig.h"

#include "POSAPI.h"

#include "OS_LIB.h"
#include "OS_SECM.h"
#include "OS_PED.h"

//#include "DEV_SRAM.h"
#include "DEV_PED.h"

#include "ANS_TR31_2010.h"


//#define	_TR31_DEBUG_		1	// decommend for DEBUG TEST

// ---------------------------------------------------------------------------
// FUNCTION: To generate Key Block Encryption Key,
//	     derived from Key Block Protection Key (Kb) according to ANSI TR-31.
// INPUT   : k1  - CMAC subkey.
// OUTPUT  : Kbe - Key Block Encryption Key. (16 bytes)
// RETURN  : TRUE	- success.
//           FALSE	- failure. (Key Block Protection Key is absent)
// ---------------------------------------------------------------------------
#if 0
UINT32	TR31_GenKeyBlockEncryptKey( UINT8 *k1, UINT8 *Kbe )
{
UINT32	i;
UINT32	status;
UINT8	Kb[PED_TDES_KEY_PROTECT_KEY_SLOT_LEN];
UINT8	kdid[8] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80 };	// key derivation input data
UINT8	temp[8];
//UINT8	odata[8];


	status = FALSE;
	
	// retrieve Key Block Protection Key
	OS_SECM_GetData(ADDR_PED_TDES_KEY_PROTECT_KEY, PED_TDES_KEY_PROTECT_KEY_SLOT_LEN, Kb );

#ifdef	_TR31_DEBUG_
	// predefine Kb for TEST ONLY
	Kb[0]  = PED_TDES_KEY_PROTECT_KEY_LEN;
	Kb[1]  = 0xDD;
	Kb[2]  = 0x75;
	Kb[3]  = 0x15;
	Kb[4]  = 0xF2;
	Kb[5]  = 0xBF;
	Kb[6]  = 0xC1;
	Kb[7]  = 0x7F;
	Kb[8]  = 0x85;
	Kb[9]  = 0xCE;
	Kb[10] = 0x48;
	Kb[11] = 0xF3;
	Kb[12] = 0xCA;
	Kb[13] = 0x25;
	Kb[14] = 0xCB;
	Kb[15] = 0x21;
	Kb[16] = 0xF6;
#endif

	if( Kb[0] == PED_TDES_KEY_PROTECT_KEY_LEN )
	  {
	  for( i=0; i<8; i++ )
	     temp[i] = kdid[i]^k1[i];
	     
	  PED_TripleDES( &Kb[1], 8, temp, Kbe );	//  first 8 bytes of the Key Block Encryption Key
	  
	  kdid[0] += 1;	// counter++

	  for( i=0; i<8; i++ )
	     temp[i] = kdid[i]^k1[i];
	     
	  PED_TripleDES( &Kb[1], 8, temp, &Kbe[8] );	//  second 8 bytes of the Key Block Encryption Key
	  
	  status = TRUE;  
	  }
	
	// clear sensitive data
	memset( Kb, 0x00, sizeof(Kb) );
	  
	return( status );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To generate Key Block Encryption Key,
//	         derived from Key Block Protection Key (Kb) according to ANSI TR-31.
// INPUT   : k1     - CMAC subkey.
//	         KpkSrc - source of the Key Block Protection Key. (L-V)
//			          using primitive KPK if NULLPTR.
// OUTPUT  : Kbe    - Key Block Encryption Key. (16 bytes)
// RETURN  : TRUE	- success.
//           FALSE	- failure. (Key Block Protection Key is absent)
// ---------------------------------------------------------------------------
UINT32	TR31_GenKeyBlockEncryptKey(UINT8 *k1, UINT8 *Kbe, UINT8 *KpkSrc)
{
	UINT32	i;
	UINT32	status;
	UINT8	Kb[PED_TDES_KEY_PROTECT_KEY_SLOT_LEN];
	UINT8	kdid[8] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80};	// key derivation input data
	UINT8	temp[8];
	//UINT8	odata[8];


	status = FALSE;

	// retrieve Key Block Protection Key
	if(KpkSrc == NULLPTR)
		OS_SECM_GetData(ADDR_PED_TDES_KEY_PROTECT_KEY, PED_TDES_KEY_PROTECT_KEY_SLOT_LEN, Kb);
	else
		memmove(Kb, KpkSrc, PED_TDES_KEY_PROTECT_KEY_LEN + 1);

#ifdef	_TR31_DEBUG_
	// predefine Kb for TEST ONLY
	Kb[0] = PED_TDES_KEY_PROTECT_KEY_LEN;
	Kb[1] = 0xDD;
	Kb[2] = 0x75;
	Kb[3] = 0x15;
	Kb[4] = 0xF2;
	Kb[5] = 0xBF;
	Kb[6] = 0xC1;
	Kb[7] = 0x7F;
	Kb[8] = 0x85;
	Kb[9] = 0xCE;
	Kb[10] = 0x48;
	Kb[11] = 0xF3;
	Kb[12] = 0xCA;
	Kb[13] = 0x25;
	Kb[14] = 0xCB;
	Kb[15] = 0x21;
	Kb[16] = 0xF6;
#endif

	if(Kb[0] == PED_TDES_KEY_PROTECT_KEY_LEN)
	{
		for(i = 0; i < 8; i++)
			temp[i] = kdid[i] ^ k1[i];

		PED_TripleDES(&Kb[1], 8, temp, Kbe);	//  first 8 bytes of the Key Block Encryption Key

		kdid[0] += 1;	// counter++

		for(i = 0; i < 8; i++)
			temp[i] = kdid[i] ^ k1[i];

		PED_TripleDES(&Kb[1], 8, temp, &Kbe[8]);	//  second 8 bytes of the Key Block Encryption Key

		status = TRUE;
	}

	// clear sensitive data
	memset(Kb, 0x00, sizeof(Kb));

	return(status);
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate Key Block MAC Key,
//	     derived from Key Block Protection Key (Kb) according to ANSI TR-31.
// INPUT   : k1  - CMAC subkey.
// OUTPUT  : Kbm - Key Block MAC Key.        (16 bytes)
// RETURN  : TRUE	- success.
//           FALSE	- failure. (Key Block Protection Key is absent)
// ---------------------------------------------------------------------------
#if 0
UINT32	TR31_GenKeyBlockMacKey( UINT8 *k1, UINT8 *Kbm )
{
UINT32	i;
UINT32	status;
UINT8	Kb[PED_TDES_KEY_PROTECT_KEY_SLOT_LEN];
UINT8	kdid[8] = { 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80 };	// key derivation input data
UINT8	temp[8];
//UINT8	odata[8];


	status = FALSE;
	
	// retrieve Key Block Protection Key
	OS_SECM_GetData(ADDR_PED_TDES_KEY_PROTECT_KEY, PED_TDES_KEY_PROTECT_KEY_SLOT_LEN, Kb );

#ifdef	_TR31_DEBUG_
	// predefine Kb for TEST ONLY
	Kb[0]  = PED_TDES_KEY_PROTECT_KEY_LEN;
	Kb[1]  = 0xDD;
	Kb[2]  = 0x75;
	Kb[3]  = 0x15;
	Kb[4]  = 0xF2;
	Kb[5]  = 0xBF;
	Kb[6]  = 0xC1;
	Kb[7]  = 0x7F;
	Kb[8]  = 0x85;
	Kb[9]  = 0xCE;
	Kb[10] = 0x48;
	Kb[11] = 0xF3;
	Kb[12] = 0xCA;
	Kb[13] = 0x25;
	Kb[14] = 0xCB;
	Kb[15] = 0x21;
	Kb[16] = 0xF6;
#endif

	if( Kb[0] == PED_TDES_KEY_PROTECT_KEY_LEN )
	  {
	  for( i=0; i<8; i++ )
	     temp[i] = kdid[i]^k1[i];
	     
	  PED_TripleDES( &Kb[1], 8, temp, Kbm );	//  first 8 bytes of the Key Block MAC Key
	  
	  kdid[0] += 1;	// counter++

	  for( i=0; i<8; i++ )
	     temp[i] = kdid[i]^k1[i];
	     
	  PED_TripleDES( &Kb[1], 8, temp, &Kbm[8] );	//  second 8 bytes of the Key Block MAC Key
	  
	  status = TRUE;  
	  }

	// clear sensitive data
	memset( Kb, 0x00, sizeof(Kb) );
	  
	return( status );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To generate Key Block MAC Key,
//	         derived from Key Block Protection Key (Kb) according to ANSI TR-31.
// INPUT   : k1     - CMAC subkey.
//	         KpkSrc - source of the Key Block Protection Key. (L-V)
//			          using primitive KPK if NULLPTR.
// OUTPUT  : Kbm    - Key Block MAC Key.        (16 bytes)
// RETURN  : TRUE	- success.
//           FALSE	- failure. (Key Block Protection Key is absent)
// ---------------------------------------------------------------------------
UINT32	TR31_GenKeyBlockMacKey(UINT8 *k1, UINT8 *Kbm, UINT8 *KpkSrc)
{
	UINT32	i;
	UINT32	status;
	UINT8	Kb[PED_TDES_KEY_PROTECT_KEY_SLOT_LEN];
	UINT8	kdid[8] = {0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80};	// key derivation input data
	UINT8	temp[8];
	//UINT8	odata[8];


	status = FALSE;

	// retrieve Key Block Protection Key
	if(KpkSrc == NULLPTR)
		OS_SECM_GetData(ADDR_PED_TDES_KEY_PROTECT_KEY, PED_TDES_KEY_PROTECT_KEY_SLOT_LEN, Kb);
	else
		memmove(Kb, KpkSrc, PED_TDES_KEY_PROTECT_KEY_LEN + 1);

#ifdef	_TR31_DEBUG_
	// predefine Kb for TEST ONLY
	Kb[0] = PED_TDES_KEY_PROTECT_KEY_LEN;
	Kb[1] = 0xDD;
	Kb[2] = 0x75;
	Kb[3] = 0x15;
	Kb[4] = 0xF2;
	Kb[5] = 0xBF;
	Kb[6] = 0xC1;
	Kb[7] = 0x7F;
	Kb[8] = 0x85;
	Kb[9] = 0xCE;
	Kb[10] = 0x48;
	Kb[11] = 0xF3;
	Kb[12] = 0xCA;
	Kb[13] = 0x25;
	Kb[14] = 0xCB;
	Kb[15] = 0x21;
	Kb[16] = 0xF6;
#endif

	if(Kb[0] == PED_TDES_KEY_PROTECT_KEY_LEN)
	{
		for(i = 0; i < 8; i++)
			temp[i] = kdid[i] ^ k1[i];

		PED_TripleDES(&Kb[1], 8, temp, Kbm);	//  first 8 bytes of the Key Block MAC Key

		kdid[0] += 1;	// counter++

		for(i = 0; i < 8; i++)
			temp[i] = kdid[i] ^ k1[i];

		PED_TripleDES(&Kb[1], 8, temp, &Kbm[8]);	//  second 8 bytes of the Key Block MAC Key

		status = TRUE;
	}

	// clear sensitive data
	memset(Kb, 0x00, sizeof(Kb));

	return(status);
}

// ---------------------------------------------------------------------------
// FUNCTION: To derive subkeys from key block protection key according to
//	     TR-31 2010.
// INPUT   : srckey - source key. (16 bytes)
// OUTPUT  : k1	- subkey K1. (8 bytes)
//	     k2 - subkey K2. (8 bytes)
// RETURN  : TRUE	- success.
//           FALSE	- failure. (Key Block Protection Key is absent)
// ---------------------------------------------------------------------------
UINT32	TR31_CMAC_DeriveSubKey( UINT8 *srckey, UINT8 *k1, UINT8 *k2 )
{
UINT32	i;
UINT32	status;
UINT8	idata[8];
UINT8	odata[8];
UINT8	key_s[8];
UINT8	key_k1[8];
//UINT8	key_k2[8];
UINT8	R64[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1B };
UINT8	carry = 0;
UINT8	data = 0;


	status = TRUE;
	
//	if( Kb[0] == PED_TDES_KEY_PROTECT_KEY_LEN )
//	  {
	  // derive S
	  memset( idata, 0x00, 8 );
	  PED_TripleDES( srckey, 8, idata, odata );		// odata = S
	  memmove( key_s, odata, 8 );
//	  LIB_DumpHexData( 0, 0, 8, odata );

	  // derive K1
	  carry = 0;
	  for( i=0; i<8; i++ )	// S << 1
	     {
	     data = key_s[7-i];
	     key_s[7-i] = (data << 1);
	     key_s[7-i] |= carry;// carry of the previous byte
	     
	     if( data & 0x80 )	// LSB for the next byte
	       carry = 1;
	     else
	       carry = 0;
	     }
	  
	  // if MSB of S = 1, then K1 = (S << 1) XOR R64
	  // else 		   K1 = (S << 1)
	  
	  if( odata[0] & 0x80 )
	    {
	    for( i=0; i<8; i++ )
	       key_s[i] ^= R64[i];
	    }
	  
	  memmove( k1, key_s, 8 );	// K1
	  
	  memmove( odata, key_s, 8 );
	  memmove( key_k1, key_s, 8 );
	  
	  // derive K2
	  carry = 0;
	  for( i=0; i<8; i++ )	// K1 << 1
	     {
	     data = key_k1[7-i];
	     key_k1[7-i] = (data << 1);
	     key_k1[7-i] |= carry;// carry of the previous byte
	     
	     if( data & 0x80 )	// LSB for the next byte
	       carry = 1;
	     else
	       carry = 0;
	     }
	  
	  // if MSB of K1 = 1, then K2 = (K1 << 1) XOR R64
	  // else 		    K2 = (K1 << 1)
	  
	  if( odata[0] & 0x80 )
	    {
	    for( i=0; i<8; i++ )
	       key_k1[i] ^= R64[i];
	    }
	  
	  memmove( k2, key_k1, 8 );	// K2
//	  }
	
//	memset( Kb, 0x00, sizeof(Kb) );	// clear sensitive data
	  
	return( status );
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate MAC value by using X9 TR-31 2010 CMAC algorithm.
// INPUT   : pm     - padding method. (MAC_PADx)
//           kbm    - Key Block MAC Key used to encrypt. (16-byte)
//	     km1    - Subkey of the kbm. (8-byte)
//	     icv    - initial chain value. (8-byte)
//           len    - length of data.
//           data   - data to be authenticated.
// OUTPUT  : mac    - the MAC value. (8-byte)
//           icv    - next icv values.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
void	TR31_GenMAC_CMAC( UINT8 pm, UINT8 *kbm, UINT8 *km1, UINT8 *icv, UINT16 len, UINT8 *data, UINT8 *mac )
{
UINT16	i, j, k;
UINT16	cnt;
UINT16	left_bytes;
UINT8	ctext[8];
UINT8	ptext[8];
//UINT8	buf[8];


//	LIB_DumpHexData( 0, 0, 16, kbm );
//	LIB_DumpHexData( 0, 3, 8, km1 );
//	LIB_DumpHexData( 0, 0, len, data );

        cnt = len / 8;
        left_bytes = len % 8;
        i = 0;

        if( cnt )
          {
          for( i=0; i<cnt; i++ )
             {
             // XOR( ICV, D1 )
             for( j=0; j<8; j++ )
                ptext[j] = icv[j] ^ data[i*8+j];

             // DES( ptext, key(L) )
             PED_des_encipher( ptext, ctext, kbm );
             
             // -DES( ctext, key(R) )
             PED_des_decipher( ptext, ctext, &kbm[8] );

             // DES( ptext, key(L) )
             PED_des_encipher( ptext, ctext, kbm );
             
             memmove( icv, ctext, 8 );
             
             if( i == (cnt-2) )
               {
               for( k=0; k<8; k++ )
	          icv[k] ^= km1[k];
               }
             
//           LIB_DumpHexData( 0, 0, 8, icv );
             }
          }

        memmove( mac, ctext, 8 );
}

// ---------------------------------------------------------------------------
// FUNCTION: To verify the encrypted key bundle format according to ANSI TR-31.
// INPUT   : KeyBundle - encrypted key block compliant to ANSI TR-31 ASCII format.
//           Length    - size of bytes of the key bundle.
// OUTPUT  : eKeyData  - encrypted key data in HEX format. (L-V)
//			 eKeyData(24) = CMAC((Len(2) + Key(16) + Padding(6)), Kbe)
//	     mac       - MAC(8)
// RETURN  : TRUE	- valid format
//           FALSE	- invalid format
// ---------------------------------------------------------------------------
#if	0
UINT32	TR31_VerifyKeyBundle( UINT16 Length, UINT8 *KeyBundle, UINT8 *eKeyData, UINT8 *mac )
{
UINT16	i;
UINT32	status;
UINT16	len;
UINT8	temp[64];
UINT8	Kbm[16];	// key block MAC key
UINT8	Kbe[16];	// key block ENC key
UINT8	key[64];
UINT8	icv[8];
//UINT8	mac[8];
UINT8	K1[8];
UINT8	K2[8];
UINT8	Kb[PED_TDES_KEY_PROTECT_KEY_SLOT_LEN];
PED_KEY_BUNDLE	keyblock;
PED_KEY_DATA	keydata;


	status = FALSE;

	memmove( (UINT8 *)&keyblock, KeyBundle, sizeof( keyblock ) );
	
	// Make sure the length of the key block matches the contents of bytes 1-4
	memmove( temp, keyblock.BlockLen, sizeof(keyblock.BlockLen) );
	temp[sizeof(keyblock.BlockLen)] = 0x00;
	len = atoi( temp );
	
	if( (Length <= 80) && (Length == len) && (keyblock.VersionID[0] == 'B') && 
	    (keyblock.OptionBlocks[0] == '0') && (keyblock.OptionBlocks[1] == '0') )
	  {
	  // convert ASCII to HEX for the key data (LEN KEY PADDING MAC)
	  for( i=0; i<(Length-16); i++ )
	     temp[i] = LIB_ascw2hexb( &keyblock.KeyLen[i*2] );
	  memmove( (UINT8 *)&keydata, temp, (Length-16)/2 );
	  
//	  LIB_DumpHexData( 0, 0, 32, temp );

	  // retrieve Key Block Protection Key
	  OS_SECM_GetData(ADDR_PED_TDES_KEY_PROTECT_KEY, PED_TDES_KEY_PROTECT_KEY_SLOT_LEN, Kb );
//	  LIB_DumpHexData( 0, 0, PED_TDES_KEY_PROTECT_KEY_SLOT_LEN, Kb );
	  if( Kb[0] != PED_TDES_KEY_PROTECT_KEY_LEN )
	    return( FALSE );
	  
#ifdef	_TR31_DEBUG_	
	// predefine Kb for TEST ONLY
	Kb[0]  = PED_TDES_KEY_PROTECT_KEY_LEN;
	Kb[1]  = 0xDD;
	Kb[2]  = 0x75;
	Kb[3]  = 0x15;
	Kb[4]  = 0xF2;
	Kb[5]  = 0xBF;
	Kb[6]  = 0xC1;
	Kb[7]  = 0x7F;
	Kb[8]  = 0x85;
	Kb[9]  = 0xCE;
	Kb[10] = 0x48;
	Kb[11] = 0xF3;
	Kb[12] = 0xCA;
	Kb[13] = 0x25;
	Kb[14] = 0xCB;
	Kb[15] = 0x21;
	Kb[16] = 0xF6;
#endif

	  // CMAC Subkey (K1 & K2) Derivation from the Key Block Protection Key (Kb)
	  TR31_CMAC_DeriveSubKey( &Kb[1], K1, K2 );
//	  LIB_DumpHexData( 0, 0, 8, K1 );
//	  LIB_DumpHexData( 0, 2, 8, K2 );
	  
	  // derive Key Block MAC Key (Kbm) from Key Block Protection Key (Kb)
	  if( TR31_GenKeyBlockMacKey( K1, Kbm ) )
	    {
//	    LIB_DumpHexData( 0, 0, 16, Kbm );
	
	    if( TR31_GenKeyBlockEncryptKey( K1, Kbe ) )
	    {
//	    LIB_DumpHexData( 0, 0, 16, Kbe );
	    
	    // decrypt the key bundle [KEY_LENGTH(2) + KEY(16) + PADDING(6)] with IV=MAC
	    memmove( temp, &keydata.KeyLen[0], ((Length-16)/2)-8 );
	    memmove( icv, keydata.MAC, 8 );
	    PED_CBC_TripleDES2( Kbe, icv, ((Length-16)/2)-8, temp, key );
//	    LIB_DumpHexData( 0, 0, 24, key );
	    
//	    temp[0] = ((Length-16)/2)-8;
//	    memmove( &temp[1], &keydata.KeyLen[0], ((Length-16)/2)-8 );
//	    if( !TR31_DecryptKeyBundle( &keydata.MAC[0], temp, key ) )
//	      return( FALSE );
	      
//	    LIB_DumpHexData( 0, 0, 17, key );
	    
	    // CMAC Subkey (KM1 & KM2) Derivation from the Key Block MAC Key (Kbm)
	    TR31_CMAC_DeriveSubKey( Kbm, K1, K2 );	// KM1 = K1
//	    LIB_DumpHexData( 0, 0, 8, K1 );
//	    LIB_DumpHexData( 0, 2, 8, K2 );
	    
	    // verify CBC MAC for the key bundle (KBH(16) + KEY_LENGTH(2) + KEY(16) + PADDING(6)
	    memmove( temp, (UINT8 *)&keyblock, 16 );		// KBH(16)
	    memmove( &temp[16], key, ((Length-16)/2)-8 );	// KEY_LENGTH(2) + KEY(16) + PADDING(6)
	    
//	    PED_GenMAC_ISO16609( MAC_PAD1, Kbm, icv, 16+sizeof(keydata)-4, temp, mac );
//	    LIB_DumpHexData( 0, 0, 16+sizeof(keydata)-8, temp );

	    memset( icv, 0x00, 8 );
	    TR31_GenMAC_CMAC( MAC_PAD1, Kbm, K1, icv, 16+sizeof(keydata)-8, temp, mac );
	    
//	    LIB_DumpHexData( 0, 0, 34, (UINT8 *)&keydata );
//	    LIB_DumpHexData( 0, 0, 8, mac );
//	    LIB_DumpHexData( 0, 2, 8, keydata.MAC );
	    }
	    
	    if( LIB_memcmp( keydata.MAC, mac, 8 ) == 0 ) // MAC OK?
	      {
	      eKeyData[0] = ((Length-16)/2)-8;
	      memmove( &eKeyData[1], &keydata.KeyLen[0], eKeyData[0] );
	      status = TRUE;
	      }
	    }
	  }

//	LIB_DumpHexData( 0, 2, eKeyData[0]+1, eKeyData );

	// clear sensitive data
	memset( temp, 0x00, sizeof(temp) );
	memset( Kbm, 0x00, sizeof(Kbm) );
	memset( Kbe, 0x00, sizeof(Kbe) );	// 2014-10-14
	memset( Kb, 0x00, sizeof(Kb) );
	memset( K1, 0x00, sizeof(K1) );
	memset( K2, 0x00, sizeof(K2) );
	memset( key, 0x00, sizeof(key) );
//	memset( mac, 0x00, sizeof(mac) );
	memset( (UINT8 *)&keyblock, 0x00, sizeof(keyblock) );
	memset( (UINT8 *)&keydata, 0x00, sizeof(keydata) );
	
	return( status );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To verify the encrypted key bundle format according to ANSI TR-31.
// INPUT   : KeyBundle - encrypted key block compliant to ANSI TR-31 ASCII format.
//           Length    - size of bytes of the key bundle.
//	         KpkSrc    - source of the Key Block Protection Key. (L-V)
//			             using primitive KPK if NULLPTR.
// OUTPUT  : eKeyData  - encrypted key data in HEX format. (L-V)
//			 eKeyData(24) = CMAC((Len(2) + Key(16) + Padding(6)), Kbe)
//	         mac       - MAC(8)
// RETURN  : TRUE	   - valid format
//           FALSE	   - invalid format
// ---------------------------------------------------------------------------
UINT32	TR31_VerifyKeyBundle( UINT16 Length, UINT8 *KeyBundle, UINT8 *eKeyData, UINT8 *mac, UINT8 *KpkSrc )
{
UINT16	i;
UINT32	status;
UINT16	len;
UINT8	temp[64];
UINT8	Kbm[16];	// key block MAC key
UINT8	Kbe[16];	// key block ENC key
UINT8	key[64];
UINT8	icv[8];
//UINT8	mac[8];
UINT8	K1[8];
UINT8	K2[8];
UINT8	Kb[PED_TDES_KEY_PROTECT_KEY_SLOT_LEN];
PED_KEY_BUNDLE	keyblock;
PED_KEY_DATA	keydata;


	status = FALSE;

	memmove( (UINT8 *)&keyblock, KeyBundle, sizeof( keyblock ) );
	
	// Make sure the length of the key block matches the contents of bytes 1-4
	memmove( temp, keyblock.BlockLen, sizeof(keyblock.BlockLen) );
	temp[sizeof(keyblock.BlockLen)] = 0x00;
	len = atoi( temp );
	
	if( (Length <= 80) && (Length == len) && (keyblock.VersionID[0] == 'B') && 
	    (keyblock.OptionBlocks[0] == '0') && (keyblock.OptionBlocks[1] == '0') )
	  {
	  // convert ASCII to HEX for the key data (LEN KEY PADDING MAC)
	  for( i=0; i<(Length-16); i++ )
	     temp[i] = LIB_ascw2hexb( &keyblock.KeyLen[i*2] );
	  memmove( (UINT8 *)&keydata, temp, (Length-16)/2 );
	  
//	  LIB_DumpHexData( 0, 0, 32, temp );

	  // retrieve Key Block Protection Key
	  if( KpkSrc == NULLPTR )
	    OS_SECM_GetData(ADDR_PED_TDES_KEY_PROTECT_KEY, PED_TDES_KEY_PROTECT_KEY_SLOT_LEN, Kb );
	  else
	    memmove( Kb, KpkSrc, PED_TDES_KEY_PROTECT_KEY_LEN+1 );
	    
//	  LIB_DumpHexData( 0, 0, PED_TDES_KEY_PROTECT_KEY_SLOT_LEN, Kb );
	  if( Kb[0] != PED_TDES_KEY_PROTECT_KEY_LEN )
	    return( FALSE );

	  // CMAC Subkey (K1 & K2) Derivation from the Key Block Protection Key (Kb)
	  TR31_CMAC_DeriveSubKey( &Kb[1], K1, K2 );
//	  LIB_DumpHexData( 0, 0, 8, K1 );
//	  LIB_DumpHexData( 0, 2, 8, K2 );
	  
	  // derive Key Block MAC Key (Kbm) from Key Block Protection Key (Kb)
	  if( TR31_GenKeyBlockMacKey( K1, Kbm, KpkSrc ) )
	    {
//	    LIB_DumpHexData( 0, 0, 16, Kbm );
	
	    if( TR31_GenKeyBlockEncryptKey( K1, Kbe, KpkSrc ) )
	    {
//	    LIB_DumpHexData( 0, 0, 16, Kbe );
	    
	    // decrypt the key bundle [KEY_LENGTH(2) + KEY(16) + PADDING(6)] with IV=MAC
	    memmove( temp, &keydata.KeyLen[0], ((Length-16)/2)-8 );
	    memmove( icv, keydata.MAC, 8 );
	    PED_CBC_TripleDES2( Kbe, icv, ((Length-16)/2)-8, temp, key );
//	    LIB_DumpHexData( 0, 0, 24, key );
	    
//	    temp[0] = ((Length-16)/2)-8;
//	    memmove( &temp[1], &keydata.KeyLen[0], ((Length-16)/2)-8 );
//	    if( !TR31_DecryptKeyBundle( &keydata.MAC[0], temp, key ) )
//	      return( FALSE );
	      
//	    LIB_DumpHexData( 0, 0, 17, key );
	    
	    // CMAC Subkey (KM1 & KM2) Derivation from the Key Block MAC Key (Kbm)
	    TR31_CMAC_DeriveSubKey( Kbm, K1, K2 );	// KM1 = K1
//	    LIB_DumpHexData( 0, 0, 8, K1 );
//	    LIB_DumpHexData( 0, 2, 8, K2 );
	    
	    // verify CBC MAC for the key bundle (KBH(16) + KEY_LENGTH(2) + KEY(16) + PADDING(6)
	    memmove( temp, (UINT8 *)&keyblock, 16 );		// KBH(16)
	    memmove( &temp[16], key, ((Length-16)/2)-8 );	// KEY_LENGTH(2) + KEY(16) + PADDING(6)
	    
//	    PED_GenMAC_ISO16609( MAC_PAD1, Kbm, icv, 16+sizeof(keydata)-4, temp, mac );
//	    LIB_DumpHexData( 0, 0, 16+sizeof(keydata)-8, temp );

	    memset( icv, 0x00, 8 );
	    TR31_GenMAC_CMAC( MAC_PAD1, Kbm, K1, icv, 16+sizeof(keydata)-8, temp, mac );
	    
//	    LIB_DumpHexData( 0, 0, 34, (UINT8 *)&keydata );
//	    LIB_DumpHexData( 0, 0, 8, mac );
//	    LIB_DumpHexData( 0, 2, 8, keydata.MAC );
	    }
	    
	    if( LIB_memcmp( keydata.MAC, mac, 8 ) == 0 ) // MAC OK?
	      {
	      eKeyData[0] = ((Length-16)/2)-8;
	      memmove( &eKeyData[1], &keydata.KeyLen[0], eKeyData[0] );
	      status = TRUE;
	      }
	    }
	  }

//	LIB_DumpHexData( 0, 2, eKeyData[0]+1, eKeyData );

	// clear sensitive data
	memset( temp, 0x00, sizeof(temp) );
	memset( Kbm, 0x00, sizeof(Kbm) );
	memset( Kbe, 0x00, sizeof(Kbe) );	// 2014-10-14
	memset( Kb, 0x00, sizeof(Kb) );
	memset( K1, 0x00, sizeof(K1) );
	memset( K2, 0x00, sizeof(K2) );
	memset( key, 0x00, sizeof(key) );
//	memset( mac, 0x00, sizeof(mac) );
	memset( (UINT8 *)&keyblock, 0x00, sizeof(keyblock) );
	memset( (UINT8 *)&keydata, 0x00, sizeof(keydata) );
	
	return( status );
}

// ---------------------------------------------------------------------------
// FUNCTION: To restore the plaintext Key Block Encryption Key from key bundle
//	     according to ANSI TR-31.
// INPUT   : eKeyData	- encrypted key data in HEX format. (L-V)
//			  eKeyData(24) = TDES((Len(2) + Key(16) + Padding(6)), Kbe)
//           icv	- initial chain vector. (8 bytes)
// OUTPUT  : key	- target plaintext key. (L-V)
// RETURN  : TRUE	- valid format
//           FALSE	- invalid format
// ---------------------------------------------------------------------------
#if	0
UINT32	TR31_DecryptKeyBundle( UINT8 *icv, UINT8 *eKeyData, UINT8 *key )
{
UINT32	status;
UINT8	Kbe[16];	// key block encryption key
UINT8	keydata[32];
UINT8	iv[8];
UINT8	K1[8];
UINT8	K2[8];
UINT8	Kb[PED_TDES_KEY_PROTECT_KEY_SLOT_LEN];
UINT16	len;


//	LIB_DumpHexData( 0, 0, 8, icv );
//	LIB_DumpHexData( 0, 2, 24+1, eKeyData );

	status = FALSE;

	// retrieve Key Block Protection Key
	OS_SECM_GetData(ADDR_PED_TDES_KEY_PROTECT_KEY, PED_TDES_KEY_PROTECT_KEY_SLOT_LEN, Kb );
	  
#ifdef	_TR31_DEBUG_	
	// predefine Kb for TEST ONLY
	Kb[0]  = PED_TDES_KEY_PROTECT_KEY_LEN;
	Kb[1]  = 0xDD;
	Kb[2]  = 0x75;
	Kb[3]  = 0x15;
	Kb[4]  = 0xF2;
	Kb[5]  = 0xBF;
	Kb[6]  = 0xC1;
	Kb[7]  = 0x7F;
	Kb[8]  = 0x85;
	Kb[9]  = 0xCE;
	Kb[10] = 0x48;
	Kb[11] = 0xF3;
	Kb[12] = 0xCA;
	Kb[13] = 0x25;
	Kb[14] = 0xCB;
	Kb[15] = 0x21;
	Kb[16] = 0xF6;
	
	eKeyData[0]  = 24;
	eKeyData[1]  = 0x94;
	eKeyData[2]  = 0xB4;
	eKeyData[3]  = 0x20;
	eKeyData[4]  = 0x07;
	eKeyData[5]  = 0x9C;
	eKeyData[6]  = 0xC8;
	eKeyData[7]  = 0x0B;
	eKeyData[8]  = 0xA3;
	
	eKeyData[9]  = 0x46;
	eKeyData[10] = 0x1F;
	eKeyData[11] = 0x86;
	eKeyData[12] = 0xFE;
	eKeyData[13] = 0x26;
	eKeyData[14] = 0xEF;
	eKeyData[15] = 0xC4;
	eKeyData[16] = 0xA3;

	eKeyData[17] = 0xB8;
	eKeyData[18] = 0xE4;
	eKeyData[19] = 0xFA;
	eKeyData[20] = 0x4C;
	eKeyData[21] = 0x5F;
	eKeyData[22] = 0x53;
	eKeyData[23] = 0x41;
	eKeyData[24] = 0x17;
	
	icv[0] = 0x6E;
	icv[1] = 0xED;
	icv[2] = 0x7B;
	icv[3] = 0x72;
	icv[4] = 0x7B;
	icv[5] = 0x8A;
	icv[6] = 0x24;
	icv[7] = 0x8E;
#endif

	if( Kb[0] != PED_TDES_KEY_PROTECT_KEY_LEN )
	  return( FALSE );
	  
	  // CMAC Subkey (K1 & K2) Derivation from the Key Block Protection Key (Kb)
	TR31_CMAC_DeriveSubKey( &Kb[1], K1, K2 );
//	LIB_DumpHexData( 0, 0, 8, K1 );
//	LIB_DumpHexData( 0, 2, 8, K2 );
//	LIB_DumpHexData( 0, 4, 8, icv );
//	LIB_DumpHexData( 0, 0, eKeyData[0], &eKeyData[1] );

	memmove( iv, icv, 8 );
	
	if( TR31_GenKeyBlockEncryptKey( K1, Kbe ) )
	  {
//	  LIB_DumpHexData( 0, 0, 16, Kbe );
	  PED_CBC_TripleDES2( Kbe, iv, eKeyData[0], &eKeyData[1], keydata );
//	  LIB_DumpHexData( 0, 0, 18, keydata );

	  len = ( keydata[0]*256 + keydata[1] )/8;	// key length in bytes
	  if( len == 16 )
	    {
	    key[0] = len;
	    memmove( &key[1], &keydata[2], len );
	    
	    status = TRUE;
	    }
	  }
	
	// clear sensitive data
	memset( Kbe, 0x00, sizeof(Kbe) );
	memset( keydata, 0x00, sizeof(keydata) );
	memset( Kb, 0x00, sizeof(Kb) );
	memset( K1, 0x00, sizeof(K1) );
	memset( K2, 0x00, sizeof(K2) );
	
	return( status );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To restore the plaintext Key Block Encryption Key from key bundle
//	         according to ANSI TR-31.
// INPUT   : eKeyData - encrypted key data in HEX format. (L-V)
//			 eKeyData(24) = TDES((Len(2) + Key(16) + Padding(6)), Kbe)
//           icv	  - initial chain vector. (8 bytes)
//	         KpkSrc   - source of the Key Block Protection Key. (L-V)
//			            using primitive KPK if NULLPTR.
// OUTPUT  : key	  - target plaintext key. (L-V)
// RETURN  : TRUE	  - valid format
//           FALSE	  - invalid format
// ---------------------------------------------------------------------------
UINT32	TR31_DecryptKeyBundle( UINT8 *icv, UINT8 *eKeyData, UINT8 *key, UINT8 *KpkSrc  )
{
UINT32	status;
UINT8	Kbe[16];	// key block encryption key
UINT8	keydata[32];
UINT8	iv[8];
UINT8	K1[8];
UINT8	K2[8];
UINT8	Kb[PED_TDES_KEY_PROTECT_KEY_SLOT_LEN];
UINT16	len;


//	LIB_DumpHexData( 0, 0, 8, icv );
//	LIB_DumpHexData( 0, 2, 24+1, eKeyData );

	status = FALSE;

	// retrieve Key Block Protection Key
	if( KpkSrc == NULLPTR )
	  OS_SECM_GetData(ADDR_PED_TDES_KEY_PROTECT_KEY, PED_TDES_KEY_PROTECT_KEY_SLOT_LEN, Kb );
	else
	  memmove( Kb, KpkSrc, PED_TDES_KEY_PROTECT_KEY_LEN+1 );

	if( Kb[0] != PED_TDES_KEY_PROTECT_KEY_LEN )
	  return( FALSE );
	  
	  // CMAC Subkey (K1 & K2) Derivation from the Key Block Protection Key (Kb)
	TR31_CMAC_DeriveSubKey( &Kb[1], K1, K2 );
//	LIB_DumpHexData( 0, 0, 8, K1 );
//	LIB_DumpHexData( 0, 2, 8, K2 );
//	LIB_DumpHexData( 0, 4, 8, icv );
//	LIB_DumpHexData( 0, 0, eKeyData[0], &eKeyData[1] );

	memmove( iv, icv, 8 );
	
	if( TR31_GenKeyBlockEncryptKey( K1, Kbe, KpkSrc ) )
	  {
//	  LIB_DumpHexData( 0, 0, 16, Kbe );
	  PED_CBC_TripleDES2( Kbe, iv, eKeyData[0], &eKeyData[1], keydata );
//	  LIB_DumpHexData( 0, 0, 18, keydata );

	  len = ( keydata[0]*256 + keydata[1] )/8;	// key length in bytes
	  if( len == 16 )
	    {
	    key[0] = len;
	    memmove( &key[1], &keydata[2], len );
	    
	    status = TRUE;
	    }
	  }
	
	// clear sensitive data
	memset( Kbe, 0x00, sizeof(Kbe) );
	memset( keydata, 0x00, sizeof(keydata) );
	memset( Kb, 0x00, sizeof(Kb) );
	memset( K1, 0x00, sizeof(K1) );
	memset( K2, 0x00, sizeof(K2) );
	
	return( status );
}
