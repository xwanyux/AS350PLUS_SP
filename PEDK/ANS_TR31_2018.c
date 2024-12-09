//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : 							                                **
//**  PRODUCT  : AS350-X6							                        **
//**                                                                        **
//**  FILE     : ANS_TR31_2018.C					                        **
//**  MODULE   : 			   				                                **
//**                                                                        **
//**  FUNCTION : Key Bundle Management for AES.				                **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2022/05/24                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2008-2022 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
// CBC MAC Key Block Format	(X9 TR-31 2018)
// +--------+-----------------+------------+------------+---------+-------+	
// | Header | Header(optional)| Key Length |	Key	| Padding |  MAC  |
// |--------|-----------------|------------|------------|---------|-------|
// | 16-byte|                 | 2-byte     | 16-Byte    | 14-byte |16-byte|
// +--------+-----------------+------------+------------+---------+-------+
//                            |<---         encrypted         --->|
// |<---			MAC			      --->|
//
// Key Block Header (KBH) Format
// BYTE#	Field Name		Encoding	Encrypted
// -----	--------------------	--------	---------
// 0		Key Block Version ID	1AN		No ('D') -- Key block protected using the AES Key Derivation Binding Method (see section 5.3.2.3)
// 1-4		Key Block Length	4N		No (eg. "0112" in decimal after encoding)
// 5-6		Key Usage		2AN		No
// 7		Algorithm		1AN		No ('A') -- AES
// 8		Mode of Use		1AN		No
// 9-10		Key Version Number	2AN		No
// 11		Expotability		1AN		No
// 12-13	No. of Optional Blocks	2N		No
// 14-15	RFU			2N		No
// 16...	First Optional Block
//
// Key Block Length = KBH(16) + 2*[Key Length(2) + KEY(16) + PAD(6+8) + MAC(8+8)] = 112 bytes after encoding in hex-ASCII
//
// Key Block Protection Key:	The KEK from which the "Key Block Encryption Key" and "Key Block MAC Key" are derived.
//				It was previously exchanged between two communication parties. (KBPK) => 32 bytes (256-bit)
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
#include "ANS_TR31_2018.h"


//#define	_TR31_DEBUG_		1	// decommend for DEBUG TEST

// ---------------------------------------------------------------------------
// FUNCTION: To generate Key Block Encryption Key,
//	     derived from Key Block Protection Key (Kb) according to ANSI TR-31.
// INPUT   : k2  - CMAC subkey. (16 bytes)
// OUTPUT  : Kbe - Key Block Encryption Key. (32 bytes)
// RETURN  : TRUE	- success.
//           FALSE	- failure. (Key Block Protection Key is absent)
// ---------------------------------------------------------------------------
UINT32	TR31_GenKeyBlockEncryptKey_AES( UINT8 *k2, UINT8 *Kbe )
{
UINT32	i;
UINT32	status;
UINT8	Kb[PED_AES_KEY_PROTECT_KEY_SLOT_LEN];
UINT8	kdid[16] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x04, 0x01, 0x00,
		     0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };	// key derivation input data
UINT8	temp[16];


	status = FALSE;
	
	// retrieve Key Block Protection Key for AES
	OS_SECM_GetData( ADDR_PED_AES_KEY_PROTECT_KEY, PED_AES_KEY_PROTECT_KEY_SLOT_LEN, Kb );

	if( Kb[0] == PED_AES_KEY_PROTECT_KEY_LEN )
	  {
	  for( i=0; i<16; i++ )
	     temp[i] = kdid[i]^k2[i];
	  
	  api_aes_encipher( temp, Kbe, &Kb[1], 32 );	 // first 16 bytes of the Key Block Encryption Key
	  
	  kdid[0] += 1;	// counter++

	  for( i=0; i<16; i++ )
	     temp[i] = kdid[i]^k2[i];
	  
	  api_aes_encipher( temp, &Kbe[16], &Kb[1], 32 ); // second 16 bytes of the Key Block Encryption Key
	  
	  status = TRUE;  
	  }
	
	// clear sensitive data
	memset( Kb, 0x00, sizeof(Kb) );
	  
	return( status );
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate Key Block MAC Key,
//	     derived from Key Block Protection Key (Kb) according to ANSI TR-31.
// INPUT   : k2  - CMAC subkey. (16 bytes)
// OUTPUT  : Kbm - Key Block MAC Key. (32 bytes)
// RETURN  : TRUE	- success.
//           FALSE	- failure. (Key Block Protection Key is absent)
// ---------------------------------------------------------------------------
UINT32	TR31_GenKeyBlockMacKey_AES( UINT8 *k2, UINT8 *Kbm )
{
UINT32	i;
UINT32	status;
UINT8	Kb[PED_AES_KEY_PROTECT_KEY_SLOT_LEN];
UINT8	kdid[16] = { 0x01, 0x00, 0x01, 0x00, 0x00, 0x04, 0x01, 0x00,
		     0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };	// key derivation input data
UINT8	temp[16];


	status = FALSE;
	
	// retrieve Key Block Protection Key for AES
	OS_SECM_GetData( ADDR_PED_AES_KEY_PROTECT_KEY, PED_AES_KEY_PROTECT_KEY_SLOT_LEN, Kb );

	if( Kb[0] == PED_AES_KEY_PROTECT_KEY_LEN )
	  {
	  for( i=0; i<16; i++ )
	     temp[i] = kdid[i]^k2[i];
	  
	  api_aes_encipher( temp, Kbm, &Kb[1], 32 );	 // first 16 bytes of the Key Block Encryption Key
	  
	  kdid[0] += 1;	// counter++

	  for( i=0; i<16; i++ )
	     temp[i] = kdid[i]^k2[i];
	  
	  api_aes_encipher( temp, &Kbm[16], &Kb[1], 32 ); // second 16 bytes of the Key Block Encryption Key
	  
	  status = TRUE;  
	  }
	
	// clear sensitive data
	memset( Kb, 0x00, sizeof(Kb) );
	  
	return( status );
}

// ---------------------------------------------------------------------------
// FUNCTION: To derive subkeys from key block protection key according to
//	     TR-31 2018.
// INPUT   : srckey - source key. (32 bytes)
// OUTPUT  : k1	- subkey K1. (16 bytes)
//	     k2 - subkey K2. (16 bytes)
// RETURN  : TRUE	- success.
//           FALSE	- failure. (Key Block Protection Key is absent)
// ---------------------------------------------------------------------------
UINT32	TR31_CMAC_DeriveSubKey_AES( UINT8 *srckey, UINT8 *k1, UINT8 *k2 )
{
UINT32	i;
UINT32	status;
UINT8	idata[16];
UINT8	odata[16];
UINT8	key_s[16];
UINT8	key_k1[16];
UINT8	R128[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87 };
UINT8	carry = 0;
UINT8	data = 0;


	status = TRUE;
	
	// derive S
	memset( idata, 0x00, 16 );
	api_aes_encipher( idata, odata, srckey, 32 );	// odata = S(16)
	memmove( key_s, odata, 16 );
//	LIB_DumpHexData( 0, 0, 16, odata );

	// derive K1
	carry = 0;
	for( i=0; i<16; i++ )	// S << 1
	   {
	   data = key_s[15-i];
	   key_s[15-i] = (data << 1);
	   key_s[15-i] |= carry;// carry of the previous byte
	   
	   if( data & 0x80 )	// LSB for the next byte
	     carry = 1;
	   else
	     carry = 0;
	   }
	
	// if MSB of S = 1, then K1 = (S << 1) XOR R128
	// else 		 K1 = (S << 1)

	if(odata[0] & 0x80)
	{
		for(i = 0 ; i < 16 ; i++)
			key_s[i] ^= R128[i];
	}
	
	memmove( k1, key_s, 16 );	// K1(16)
	
	memmove(odata, key_s, 16);
	memmove( key_k1, key_s, 16 );
	
	// derive K2
	carry = 0;
	for( i=0; i<16; i++ )	// K1 << 1
	   {
	   data = key_k1[15-i];
	   key_k1[15-i] = (data << 1);
	   key_k1[15-i] |= carry;// carry of the previous byte
	   
	   if( data & 0x80 )	// LSB for the next byte
	     carry = 1;
	   else
	     carry = 0;
	   }

	// if MSB of K1 = 1, then K2 = (K1 << 1) XOR R128
	// else 		  K2 = (K1 << 1)

	if(odata[0] & 0x80)
	{
		for(i = 0 ; i < 16 ; i++)
			key_k1[i] ^= R128[i];
	}
	
	memmove( k2, key_k1, 16 );	// K2(16)
	  
	return( status );
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate MAC value by using X9 TR-31 2018 CMAC algorithm.
// INPUT   : pm     - padding method. (MAC_PADx)
//           kbm    - Key Block MAC Key used to encrypt. (32-byte)
//	     km1    - Subkey of the kbm. (16-byte)
//	     icv    - initial chain value. (16-byte)
//           len    - length of data.
//           data   - data to be authenticated.
// OUTPUT  : mac    - the MAC value. (16-byte)
//           icv    - next icv values.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
void	TR31_GenMAC_CMAC_AES( UINT8 pm, UINT8 *kbm, UINT8 *km1, UINT8 *icv, UINT16 len, UINT8 *data, UINT8 *mac )
{
UINT16	i, j, k;
UINT16	cnt;
UINT16	left_bytes;
UINT8	ctext[16];
UINT8	ptext[16];



//	LIB_DumpHexData( 0, 0, 32, kbm );
//	LIB_DumpHexData( 0, 3, 16, km1 );
//	LIB_DumpHexData( 0, 0, len, data );

        cnt = len / 16;
        left_bytes = len % 16;
        i = 0;

        if( cnt )
          {
          for( i=0; i<cnt; i++ )
             {
             // XOR( ICV, D1 )
             for( j=0; j<16; j++ )
                ptext[j] = icv[j] ^ data[i*16+j];

	     // ECB-ENC (AES)
	     api_aes_encipher( ptext, ctext, kbm, 32 );
             
             memmove( icv, ctext, 16 );
             
             if( i == (cnt-2) )
               {
               for( k=0; k<16; k++ )
	          icv[k] ^= km1[k];
               }
             
//           LIB_DumpHexData( 0, 0, 16, icv );
             }
          }

        memmove( mac, ctext, 16 );
}

// ---------------------------------------------------------------------------
// FUNCTION: To verify the encrypted key bundle format according to ANSI TR-31.
// INPUT   : KeyBundle - encrypted key block compliant to ANSI TR-31 ASCII format.
//           Length    - size of bytes of the key bundle.
// OUTPUT  : eKeyData  - encrypted key data in HEX format. (L-V)
//			 eKeyData(32) = CMAC((Len(2) + Key(16) + Padding(14)), Kbe)
//	     mac       - MAC(16)
// RETURN  : TRUE	- valid format
//           FALSE	- invalid format
// ---------------------------------------------------------------------------
UINT32	TR31_VerifyKeyBundle_AES( UINT16 Length, UINT8 *KeyBundle, UINT8 *eKeyData, UINT8 *mac )
{
UINT16	i;
UINT32	status;
UINT16	len;
UINT8	temp[96];
UINT8	Kbm[32];	// key block MAC key
UINT8	Kbe[32];	// key block ENC key
UINT8	key[96];
UINT8	icv[16];
//UINT8	mac[16];
UINT8	K1[16];
UINT8	K2[16];
UINT8	Kb[PED_AES_KEY_PROTECT_KEY_SLOT_LEN];
PED_KEY_BUNDLE2	keyblock;
PED_KEY_DATA2	keydata;


	status = FALSE;

	memmove( (UINT8 *)&keyblock, KeyBundle, sizeof( keyblock ) );
	
	// Make sure the length of the key block matches the contents of bytes 1-4
	memmove( temp, keyblock.BlockLen, sizeof(keyblock.BlockLen) );
	temp[sizeof(keyblock.BlockLen)] = 0x00;
	len = atoi( temp );
	
	if( (Length <= 112) && (Length == len) && (keyblock.VersionID[0] == 'D') && 
	    (keyblock.OptionBlocks[0] == '0') && (keyblock.OptionBlocks[1] == '0') )
	  {
	  // convert ASCII to HEX for the key data (LEN KEY PADDING MAC)
	  for( i=0; i<(Length-16); i++ )
	     temp[i] = LIB_ascw2hexb( &keyblock.KeyLen[i*2] );
	  memmove( (UINT8 *)&keydata, temp, (Length-16)/2 );
	  
//	  LIB_DumpHexData( 0, 0, 48, temp );

#ifdef	_TR31_DEBUG_	
	// predefine Kb for TEST ONLY
	Kb[0]  = PED_AES_KEY_PROTECT_KEY_LEN;
	Kb[1]  = 0x88;
	Kb[2]  = 0xE1;
	Kb[3]  = 0xAB;
	Kb[4]  = 0x2A;
	Kb[5]  = 0x2E;
	Kb[6]  = 0x3D;
	Kb[7]  = 0xD3;
	Kb[8]  = 0x8C;
	
	Kb[9]  = 0x1F;
	Kb[10] = 0xA0;
	Kb[11] = 0x39;
	Kb[12] = 0xA5;
	Kb[13] = 0x36;
	Kb[14] = 0x50;
	Kb[15] = 0x0C;
	Kb[16] = 0xC8;

	Kb[17] = 0xA8;
	Kb[18] = 0x7A;
	Kb[19] = 0xB9;
	Kb[20] = 0xD6;
	Kb[21] = 0x2D;
	Kb[22] = 0xC9;
	Kb[23] = 0x2C;
	Kb[24] = 0x01;
	
	Kb[25] = 0x05;
	Kb[26] = 0x8F;
	Kb[27] = 0xA7;
	Kb[28] = 0x9F;
	Kb[29] = 0x44;
	Kb[30] = 0x65;
	Kb[31] = 0x7D;
	Kb[32] = 0xE6;
	
	OS_SECM_PutData( ADDR_PED_AES_KEY_PROTECT_KEY, PED_AES_KEY_PROTECT_KEY_SLOT_LEN, Kb );
#endif

	  // retrieve Key Block Protection Key
	  OS_SECM_GetData( ADDR_PED_AES_KEY_PROTECT_KEY, PED_AES_KEY_PROTECT_KEY_SLOT_LEN, Kb );
//	  LIB_DumpHexData( 0, 0, PED_KEY_PROTECT_KEY_SLOT_LEN, Kb );
	  if( Kb[0] != PED_AES_KEY_PROTECT_KEY_LEN )
	    return( FALSE );

	  // CMAC Subkey (K1 & K2) Derivation from the Key Block Protection Key (Kb)
	  TR31_CMAC_DeriveSubKey_AES( &Kb[1], K1, K2 );
//	  LIB_DumpHexData( 0, 0, 16, K1 );
//	  LIB_DumpHexData( 0, 2, 16, K2 );
	  
	  // derive Key Block MAC Key (Kbm) from Key Block Protection Key (Kb)
	  if( TR31_GenKeyBlockMacKey_AES( K2, Kbm ) )
	    {
//	    LIB_DumpHexData( 0, 0, 32, Kbm );
	
	    if( TR31_GenKeyBlockEncryptKey_AES( K2, Kbe ) )
	      {
//	      LIB_DumpHexData( 0, 0, 32, Kbe );
	      
	      // decrypt the key bundle [KEY_LENGTH(2) + KEY(16) + PADDING(14)] with IV=MAC
	      memmove( temp, &keydata.KeyLen[0], ((Length-16)/2)-16 );
	      memmove( icv, keydata.MAC, 16 );
	      PED_CBC_AES2( Kbe, Kb[0], icv, ((Length-16)/2)-16, temp, key );
//	      LIB_DumpHexData( 0, 0, 32, key );

#ifdef	_TR31_DEBUG_
	UINT32	i;
	_DEBUGPRINTF("\r\nKbm=\r\n");
	for( i=0; i<32; i++ )
	   _DEBUGPRINTF("%02X ", Kbm[i] );
	_DEBUGPRINTF("\r\n");
	
	_DEBUGPRINTF("\r\nKbe=\r\n");
	for( i=0; i<32; i++ )
	   _DEBUGPRINTF("%02X ", Kbe[i] );
	_DEBUGPRINTF("\r\n");

	_DEBUGPRINTF("\r\nkey=\r\n");
	for( i=0; i<32; i++ )
	   _DEBUGPRINTF("%02X ", key[i] );
	_DEBUGPRINTF("\r\n");
	for(;;);
#endif
	      
	      // CMAC Subkey (KM1 & KM2) Derivation from the Key Block MAC Key (Kbm)
	      TR31_CMAC_DeriveSubKey_AES( Kbm, K1, K2 );	// KM1 = K1
//	      LIB_DumpHexData( 0, 0, 16, K1 );
//	      LIB_DumpHexData( 0, 2, 16, K2 );

#ifdef	_TR31_DEBUG_
	UINT32	i;
	_DEBUGPRINTF("\r\nKM1=\r\n");
	for( i=0; i<16; i++ )
	   _DEBUGPRINTF("%02X ", K1[i] );
	_DEBUGPRINTF("\r\n");
	for(;;);
#endif

	      // verify CBC MAC for the key bundle (KBH(16) + KEY_LENGTH(2) + KEY(16) + PADDING(14)
	      memmove( temp, (UINT8 *)&keyblock, 16 );		// KBH(16)
	      memmove( &temp[16], key, ((Length-16)/2)-16 );	// KEY_LENGTH(2) + KEY(16) + PADDING(14)
              
	      memset( icv, 0x00, 16 );
	      TR31_GenMAC_CMAC_AES( MAC_PAD1, Kbm, K1, icv, 16+sizeof(keydata)-16, temp, mac );
	      
//	      LIB_DumpHexData( 0, 0, 34, (UINT8 *)&keydata );
//	      LIB_DumpHexData( 0, 0, 16, mac );
//	      LIB_DumpHexData( 0, 2, 16, keydata.MAC );

#ifdef	_TR31_DEBUG_
	UINT32	i;
	_DEBUGPRINTF("\r\nmac=\r\n");
	for( i=0; i<16; i++ )
	   _DEBUGPRINTF("%02X ", mac[i] );
	_DEBUGPRINTF("\r\n");

	_DEBUGPRINTF("\r\keydata_MAC=\r\n");
	for( i=0; i<16; i++ )
	   _DEBUGPRINTF("%02X ", keydata.MAC[i] );
	_DEBUGPRINTF("\r\n");
	for(;;);
#endif
	      }
	      
	    if( LIB_memcmp( keydata.MAC, mac, 16 ) == 0 ) // MAC OK?
	      {
	      eKeyData[0] = ((Length-16)/2)-16;
	      memmove( &eKeyData[1], &keydata.KeyLen[0], eKeyData[0] );
	      status = TRUE;
	      }
	    }
	  }

//	LIB_DumpHexData( 0, 2, eKeyData[0]+1, eKeyData );

	// clear sensitive data
	memset( temp, 0x00, sizeof(temp) );
	memset( Kbm, 0x00, sizeof(Kbm) );
	memset( Kbe, 0x00, sizeof(Kbe) );
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
//			  eKeyData(32) = TDES((Len(2) + Key(16) + Padding(14)), Kbe)
//           icv	- initial chain vector. (16 bytes)
// OUTPUT  : key	- target plaintext key. (L-V)
// RETURN  : TRUE	- valid format
//           FALSE	- invalid format
// ---------------------------------------------------------------------------
UINT32	TR31_DecryptKeyBundle_AES( UINT8 *icv, UINT8 *eKeyData, UINT8 *key )
{
UINT32	status;
UINT8	Kbe[32];	// key block encryption key
UINT8	keydata[32];
UINT8	iv[16];
UINT8	K1[16];
UINT8	K2[16];
UINT8	Kb[PED_AES_KEY_PROTECT_KEY_SLOT_LEN];
UINT16	len;


//	LIB_DumpHexData( 0, 0, 16, icv );
//	LIB_DumpHexData( 0, 2, 32+1, eKeyData );

	status = FALSE;

	// retrieve Key Block Protection Key
	OS_SECM_GetData( ADDR_PED_AES_KEY_PROTECT_KEY, PED_AES_KEY_PROTECT_KEY_SLOT_LEN, Kb );
	  
#ifdef	_TR31_DEBUG_	
	// predefine Kb for TEST ONLY
	Kb[0]  = PED_AES_KEY_PROTECT_KEY_LEN;
	Kb[1]  = 0x88;
	Kb[2]  = 0xE1;
	Kb[3]  = 0xAB;
	Kb[4]  = 0x2A;
	Kb[5]  = 0x2E;
	Kb[6]  = 0x3D;
	Kb[7]  = 0xD3;
	Kb[8]  = 0x8C;
	
	Kb[9]  = 0x1F;
	Kb[10] = 0xA0;
	Kb[11] = 0x39;
	Kb[12] = 0xA5;
	Kb[13] = 0x36;
	Kb[14] = 0x50;
	Kb[15] = 0x0C;
	Kb[16] = 0xC8;

	Kb[17] = 0xA8;
	Kb[18] = 0x7A;
	Kb[19] = 0xB9;
	Kb[20] = 0xD6;
	Kb[21] = 0x2D;
	Kb[22] = 0xC9;
	Kb[23] = 0x2C;
	Kb[24] = 0x01;
	
	Kb[25] = 0x05;
	Kb[26] = 0x8F;
	Kb[27] = 0xA7;
	Kb[28] = 0x9F;
	Kb[29] = 0x44;
	Kb[30] = 0x65;
	Kb[31] = 0x7D;
	Kb[32] = 0xE6;
	
	OS_SECM_PutData( ADDR_PED_AES_KEY_PROTECT_KEY, PED_AES_KEY_PROTECT_KEY_SLOT_LEN, Kb );
	
	eKeyData[0]  = 32;
	eKeyData[1]  = 0xB8;
	eKeyData[2]  = 0x26;
	eKeyData[3]  = 0x79;
	eKeyData[4]  = 0x11;
	eKeyData[5]  = 0x4F;
	eKeyData[6]  = 0x47;
	eKeyData[7]  = 0x0F;
	eKeyData[8]  = 0x54;
	
	eKeyData[9]  = 0x01;
	eKeyData[10] = 0x65;
	eKeyData[11] = 0xED;
	eKeyData[12] = 0xFB;
	eKeyData[13] = 0xF7;
	eKeyData[14] = 0xE2;
	eKeyData[15] = 0x50;
	eKeyData[16] = 0xFC;

	eKeyData[17] = 0xEA;
	eKeyData[18] = 0x43;
	eKeyData[19] = 0xF8;
	eKeyData[20] = 0x10;
	eKeyData[21] = 0xD2;
	eKeyData[22] = 0x15;
	eKeyData[23] = 0xF8;
	eKeyData[24] = 0xD2;
	
	eKeyData[25] = 0x07;
	eKeyData[26] = 0xE2;
	eKeyData[27] = 0xE4;
	eKeyData[28] = 0x17;
	eKeyData[29] = 0xC0;
	eKeyData[30] = 0x71;
	eKeyData[31] = 0x56;
	eKeyData[32] = 0xA2;
	
	icv[0] = 0x7E;
	icv[1] = 0x8E;
	icv[2] = 0x31;
	icv[3] = 0xDA;
	icv[4] = 0x05;
	icv[5] = 0xF7;
	icv[6] = 0x42;
	icv[7] = 0x55;
	
	icv[8] = 0x09;
	icv[9] = 0x59;
	icv[10]= 0x3D;
	icv[11]= 0x03;
	icv[12]= 0xA4;
	icv[13]= 0x57;
	icv[14]= 0xDC;
	icv[15]= 0x34;
#endif

	if( Kb[0] != PED_AES_KEY_PROTECT_KEY_LEN )
	  return( FALSE );
	  
	  // CMAC Subkey (K1 & K2) Derivation from the Key Block Protection Key (Kb)
	TR31_CMAC_DeriveSubKey_AES( &Kb[1], K1, K2 );
//	LIB_DumpHexData( 0, 0, 16, K1 );
//	LIB_DumpHexData( 0, 2, 16, K2 );
//	LIB_DumpHexData( 0, 4, 16, icv );
//	LIB_DumpHexData( 0, 0, eKeyData[0], &eKeyData[1] );

#ifdef	_TR31_DEBUG_
	UINT32	i;
	_DEBUGPRINTF("\r\nK1=\r\n");
	for( i=0; i<16; i++ )
	   _DEBUGPRINTF("%02X ", K1[i] );
	_DEBUGPRINTF("\r\n");

	_DEBUGPRINTF("\r\nK2=\r\n");
	for( i=0; i<16; i++ )
	   _DEBUGPRINTF("%02X ", K2[i] );
	_DEBUGPRINTF("\r\n");
	for(;;);
#endif

	memmove( iv, icv, 16 );
	
	if( TR31_GenKeyBlockEncryptKey_AES( K2, Kbe ) )
	  {
//	  LIB_DumpHexData( 0, 0, 32, Kbe );
	  PED_CBC_AES2( Kbe, Kb[0], iv, eKeyData[0], &eKeyData[1], keydata );
//	  LIB_DumpHexData( 0, 0, 18, keydata );

#ifdef	_TR31_DEBUG_
	UINT32	i;
	_DEBUGPRINTF("\r\nKbe=\r\n");
	for( i=0; i<32; i++ )
	   _DEBUGPRINTF("%02X ", Kbe[i] );
	_DEBUGPRINTF("\r\n");

	_DEBUGPRINTF("\r\keydata=\r\n");
	for( i=0; i<18; i++ )
	   _DEBUGPRINTF("%02X ", keydata[i] );
	_DEBUGPRINTF("\r\n");
	for(;;);
#endif

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

// ---------------------------------------------------------------------------
#ifdef	_TR31_DEBUG_
UINT32	TEST_TR31_2018( void )
{
UINT8	icv[16];
UINT8	eKeyData[64];
UINT8	key[64];
UINT8	mac[16];
UINT8	KeyBundle[] = {
	"D0112P0AE00E0000B82679114F470F540165EDFBF7E250FCEA43F810D215F8D207E2E417C07156A27E8E31DA05F7425509593D03A457DC34"
	};

UINT16	Length = KEY_BUNDLE_LEN2;


	LIB_OpenKeyAll();
	
	_DEBUGPRINTF("\r\nTEST_TR31_2018\n");
	
	LIB_WaitKey();
	
	TR31_VerifyKeyBundle_AES( Length, KeyBundle, eKeyData, mac );
	
//	TR31_DecryptKeyBundle_AES( icv, eKeyData, key );
	
	for(;;);
}
#endif
