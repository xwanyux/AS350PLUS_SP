//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : i.MX6UL							                        **
//**  PRODUCT  : AS350-X6							                        **
//**                                                                        **
//**  FILE     : OS_PED4.C						                            **
//**  MODULE   : 			   				                                **
//**                                                                        **
//**  FUNCTION : OS::PED4 (MAC code Generation)				                **
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
#include <string.h>

#include "POSAPI.h"
#include "OS_LIB.h"
#include "OS_SECM.h"
#include "OS_PED.h"
//#include "DEV_SRAM.h"
#include "DEV_PED.h"
#include "ANS_TR31_2010.h"


// ---------------------------------------------------------------------------
// FUNCTION: Data prefixing padding of ISO 9797-1.
// INPUT   : method - padding method. (MAC_PADx)
//           length - original data length.
// OUTPUT  : prefix - the prefixing padding data block. (8 bytes)
// RETURN  : apiOK     - prefixing padding is available.
//	     apiFailed - prefixing padding is not available.
// ---------------------------------------------------------------------------
UINT8	PED_PrefixPadding( UINT8 method, UINT16 length, UINT8 *prefix )
{
UINT8	buf[8];
UINT32	DataLen;

	
	if( method == MAC_PAD3 )
	  {
	  memset( buf, 0x00, 8 );
	  
	  DataLen = length * 8;	// in bits
	  buf[7] = DataLen & 0x000000FF;
	  buf[6] = (DataLen & 0x0000FF00) >> 8;
	  buf[5] = (DataLen & 0x00FF0000) >> 16;
	  buf[4] = (DataLen & 0xFF000000) >> 24;	  
	  memmove( prefix, buf, 8 );
	  
	  return( apiOK );
	  }
	else
	  return( apiFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: Data postfixing padding of ISO 9797-1.
// INPUT   : method    - padding method. (MAC_PADx)
//           leftbytes - remaining data byte number to be padded.
//           lastblock - the last data block to be padded.
// OUTPUT  : postfix - the postfixing padding data block. (8 bytes)
// RETURN  : apiOK     - prefixing padding is available.
//	     apiFailed - prefixing padding is not available.
// ---------------------------------------------------------------------------
UINT8	PED_PostfixPadding( UINT8 method, UINT16 leftbytes, UINT8 *lastblock, UINT8 *postfix )
{
UINT8	buf[8];
UINT8	result;

	
	result = apiOK;
	memset( buf, 0x00, 8 );
	
	switch( method )
	      {
	      case MAC_PAD1:	// default
	      case MAC_PAD3:
	           
	           if( leftbytes == 0 )
	             result = apiFailed;
	           else
	             {
	             // padded with '0' bits
	             memmove( buf, lastblock, leftbytes );
	             memmove( postfix, buf, 8 );
	             }
	           break;
	      
	      case MAC_PAD2:
	      
	           // padded with single '1' bit and few '0' bits
	           if( leftbytes == 0 )
	             {
	             buf[0] = 0x80;
	             memmove( postfix, buf, 8 );
	             }
	           else
	             {
	             memmove( buf, lastblock, leftbytes );
	             buf[leftbytes] = 0x80;
	             memmove( postfix, buf, 8 );
	             }
	           break;
	      }
	      
	return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: Data postfixing padding of ISO 9797-1. (method 4)
// INPUT   : keySize   - key size in bytes. (max 32 bytes)
//           leftbytes - remaining data byte number to be padded.
//           lastblock - the last data block to be padded.
// OUTPUT  : postfix - the postfixing padding data block. (max 32 bytes)
// RETURN  : apiOK     - prefixing padding is available.
//	     apiFailed - prefixing padding is not available.
// ---------------------------------------------------------------------------
UINT8	PED_PostfixPadding4( UINT8 keySize, UINT16 leftbytes, UINT8 *lastblock, UINT8 *postfix )
{
UINT8	buf[32];
UINT8	result;

	
	result = apiOK;
	memset( buf, 0x00, keySize );
	
	// padded with single '1' bit and few '0' bits ONLY when leftbytes available
	if( leftbytes )
	  {
	  memmove( buf, lastblock, leftbytes );
	  buf[leftbytes] = 0x80;
	  memmove( postfix, buf, keySize );
	  }
	else
	  result = apiFailed;
	             
	return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate MAC value by using ISO 16609 algorithm 1. (TDEA CBC MAC)
// INPUT   : pm     - padding method. (MAC_PADx)
//           deskey - DES or TDES key used to encrypt.
//	     icv    - initial chain value. (8-byte)
//           len    - length of data.
//           data   - data to be authenticated.
// OUTPUT  : mac    - the MAC value. (8-byte)
//           icv    - next icv values.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
void	PED_GenMAC_ISO16609( UINT8 pm, UINT8 *deskey, UINT8 *icv, UINT16 len, UINT8 *data, UINT8 *mac )
{
UINT16	i, j;
UINT16	cnt;
UINT16	left_bytes;
UINT8	ctext[8];
UINT8	ptext[8];
UINT8	buf[8];


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
             PED_des_encipher( ptext, ctext, deskey );
             
             // -DES( ctext, key(R) )
             PED_des_decipher( ptext, ctext, &deskey[8] );

             // DES( ptext, key(L) )
             PED_des_encipher( ptext, ctext, deskey );
             
             memmove( icv, ctext, 8 );
             }
          }

        if( PED_PostfixPadding( pm, left_bytes, &data[i*8], buf ) == apiOK )
          {          
          // XOR( ICV, Postfix )
          for( j=0; j<8; j++ )
             ptext[j] = icv[j] ^ buf[j];

          // DES( ptext, key(L) )
          PED_des_encipher( ptext, ctext, deskey );
          
          // -DES( ctext, key(R) )
          PED_des_decipher( ptext, ctext, &deskey[8] );

          // DES( ptext, key(L) )
          PED_des_encipher( ptext, ctext, deskey );
          }

        memmove( mac, ctext, 8 );
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate MAC value by using ISO 9797-1 algorithm 1. (ANSI X9.9)
// INPUT   : pm     - padding method. (MAC_PADx)
//           deskey - DES or TDES key used to encrypt.
//	     icv    - initial chain value. (8-byte)
//           len    - length of data.
//           data   - data to be authenticated.
// OUTPUT  : mac    - the MAC value. (8-byte)
//           icv    - next icv values.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
void	PED_GenMAC_ALG1( UINT8 pm, UINT8 *deskey, UINT8 *icv, UINT16 len, UINT8 *data, UINT8 *mac )
{
UINT16	i, j;
UINT16	cnt;
UINT16	left_bytes;
UINT8	ctext[8];
UINT8	ptext[8];
UINT8	buf[8];


        cnt = len / 8;
        left_bytes = len % 8;
        i = 0;
        
        if( PED_PrefixPadding( pm, len, buf ) == apiOK )
          {
          // XOR( ICV, Prefix )
          for( j=0; j<8; j++ )
             ptext[j] = icv[j] ^ buf[j];
             
          // DES( ptext, key(L) )
          PED_des_encipher( ptext, ctext, deskey );
          memmove( icv, ctext, 8 );
          }

        if( cnt )
          {
          for( i=0; i<cnt; i++ )
             {
             // XOR( ICV, D1 )
             for( j=0; j<8; j++ )
                ptext[j] = icv[j] ^ data[i*8+j];

             // DES( ptext, key(L) )
             PED_des_encipher( ptext, ctext, deskey );
             memmove( icv, ctext, 8 );
             }
          }

        if( PED_PostfixPadding( pm, left_bytes, &data[i*8], buf ) == apiOK )
          {          
          // XOR( ICV, Postfix )
          for( j=0; j<8; j++ )
             ptext[j] = icv[j] ^ buf[j];

          // DES( ptext, key(L) )
          PED_des_encipher( ptext, ctext, deskey );
          }

        memmove( mac, ctext, 8 );
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate MAC value by using ISO 9797-1 algorithm 3. (ANSI X9.19)
// INPUT   : pm     - padding method. (MAC_PADx)
//           deskey - DES or TDES key used to encrypt.
//	     icv    - initial chain value. (8-byte)
//           len    - length of data.
//           data   - data to be authenticated.
// OUTPUT  : mac    - the MAC value. (8-byte)
//           icv    - next icv values.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
void	PED_GenMAC_ALG3( UINT8 pm, UINT8 *deskey, UINT8 *icv, UINT16 len, UINT8 *data, UINT8 *mac )
{
UINT16	i, j;
UINT16	cnt;
UINT16	left_bytes;
UINT8	ctext[8];
UINT8	ptext[8];
UINT8	buf[8];


        cnt = len / 8;
        left_bytes = len % 8;
        i = 0;
        
        if( PED_PrefixPadding( pm, len, buf ) == apiOK )
          {
          // XOR( ICV, Prefix )
          for( j=0; j<8; j++ )
             ptext[j] = icv[j] ^ buf[j];
             
          // DES( ptext, key(L) )
          PED_des_encipher( ptext, ctext, deskey );
          memmove( icv, ctext, 8 );
          }

        if( cnt )
          {
          for( i=0; i<cnt; i++ )
             {
             // XOR( ICV, D1 )
             for( j=0; j<8; j++ )
                ptext[j] = icv[j] ^ data[i*8+j];

             // DES( ptext, key(L) )
             PED_des_encipher( ptext, ctext, deskey );
             memmove( icv, ctext, 8 );
             }
          }

        if( PED_PostfixPadding( pm, left_bytes, &data[i*8], buf ) == apiOK )
          {          
          // XOR( ICV, Postfix )
          for( j=0; j<8; j++ )
             ptext[j] = icv[j] ^ buf[j];

          // DES( ptext, key(L) )
          PED_des_encipher( ptext, ctext, deskey );
          }

        // -DES( ctext, key(R) )
        PED_des_decipher( ptext, ctext, &deskey[8] );

        // DES( ptext, key(L) )
        PED_des_encipher( ptext, ctext, deskey );

        memmove( mac, ctext, 8 );
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate MAC value by using Master/Session key algorithm.
// INPUT   : mode   - algorithm and padding method.
//		      MAC_ALGx + MAC_PADx
// 	     index  - session key index used to encrypt the PIN block.
//	     icv    - initial chain vector for CBC mode. (8-byte)
//           length - length of data. (may be zero if data string is empty)
//           data   - data to be authenticated.
// OUTPUT  : mac    - the MAC value. (8-byte)
//           icv    - next icv values.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
/*
UINT8	PED_MSKEY_GenMAC( UINT8 mode, UINT8 index, UINT8 *icv, UINT16 length, UINT8 *data, UINT8 *mac )
{
UINT8	skey[PED_MSKEY_LEN+1];	// L-V
UINT8	mkey[PED_MSKEY_LEN+1];	// L-V
PED_KEY_BUNDLE	KeyBundle;
UINT8	keydata[KEY_DATA_LEN+1]; // L-V
UINT8	eskey[PED_MSKEY_LEN+1];	// L-V
UINT8	MkeyIndex;
UINT8	result;
UINT8	mac8[8];

UINT8	temp[KEY_DATA_LEN + 1];
UINT8	buf[KEY_BUNDLE_LEN];


	PED_InUse( TRUE );
	
	result = apiFailed;
	
	// check SKEY index
	if( index < MAX_SKEY_CNT )
	  {
	  // get MKEY bundle
	  PED_ReadMKeyIndex((UINT8 *)&MkeyIndex);
	  OS_SECM_GetData(ADDR_PED_MKEY_01 + (MkeyIndex * PED_MSKEY_SLOT_LEN), PED_MSKEY_SLOT_LEN, buf);
	    
	  // verify MKEY bundle
	  if( TR31_VerifyKeyBundle(KEY_BUNDLE_LEN, buf, temp, mac8, (UINT8 *)0 ) )
	    {
	    // retrieve MKEY
	    if( !TR31_DecryptKeyBundle(mac8, temp, mkey, (UINT8 *)0 ) )	// mkey=MKEY (as the KBPK for SKEY)
	      {
	      result = apiFailed;
	      goto EXIT;
	      }
	    }

	  // get ESKEY bundle
	  OS_SECM_GetData( ADDR_PED_SKEY_01+(index*PED_MSKEY_SLOT_LEN), PED_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle );
	  
	  // verify ESKEY bundle
	  if( TR31_VerifyKeyBundle( PED_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle, keydata, mac8, mkey ) )
	    {
	    // check ESKEY key usage
	    if( (KeyBundle.Usage[0] == 'D') && (KeyBundle.Usage[1] == '0') ) // for DATA encryption?
	      {
	      // retrieve ESKEY key (SKEY encrypted by MKEY)
	      if( TR31_DecryptKeyBundle( mac8, keydata, eskey, mkey ) )
	        {
	        memmove( skey, &eskey[1], eskey[0] );
	        
	        // get MKEY bundle
		PED_ReadMKeyIndex( (UINT8 *)&MkeyIndex );
		OS_SECM_GetData( ADDR_PED_MKEY_01+(MkeyIndex*PED_MSKEY_SLOT_LEN), PED_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle );
		
		// verify MKEY bundle
		if( TR31_VerifyKeyBundle( PED_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle, keydata, mac8, (UINT8 *)0 ) )
		  {
		  // check MKEY key usage
		  if( (KeyBundle.Usage[0] == 'K') && (KeyBundle.Usage[1] == '0') ) // for KEY encryption?
		    {
		    // retrieve MKEY
//	            if( TR31_DecryptKeyBundle( mac8, keydata, mkey ) )
//	              {
//		      LIB_DumpHexData( 0, 0, 17, mkey );
		      
	              // retrieve SKEY from MKEY
//		      PED_TripleDES2( &mkey[1], eskey[0], &eskey[1], skey );
//	              LIB_DumpHexData( 0, 3, 17, skey );

		      result = apiOK;
//	              }
		    }
		  }
	        }
	      }
	    }

	  if( result == apiOK )
	    {
	    result = apiFailed;
	  
	    switch( mode & 0x0F )
	    	  {
	  	  case MAC_ISO16609:	// CBC-TDEA-MAC
	               
	               PED_GenMAC_ISO16609( mode & 0xF0, skey, icv, length, data, mac );	// PATCH: 2009-04-06
	               
	               result = apiOK;
	  	       break;
	  	       
	  	  case MAC_ALG1:	// CBC-MAC (ANSI X9.9)
	  	
	  	       PED_GenMAC_ALG1( mode & 0xF0, skey, icv, length, data, mac );		// PATCH: 2009-04-06
	  	     
	  	       result = apiOK;
	  	       break;
	  	     
	  	  case MAC_ALG2:	// CBC-MAC (NA)
	  	       break;
	  	     
	  	  case MAC_ALG3:	// CBC-MAC (ANSI X9.19)
	  	
	  	       PED_GenMAC_ALG3( mode & 0xF0, skey, icv, length, data, mac );		// PATCH: 2009-04-06
	  	     
	  	       result = apiOK;
	  	       break;
	  	     
	  	  case MAC_ALG4:	// (NA)
	  	       break;
	  	     
	  	  case MAC_ALG5:	// (NA)
	  	       break;
	  	     
	  	  case MAC_ALG6:	// (NA)
	  	       break;
	  	  }
	    }

EXIT:
	  // clear sensitive data
	  memset( mkey, 0x00, sizeof(mkey) );	// clear KEY data  
	  memset( skey, 0x00, sizeof(skey) );	// clear KEY data
	  memset( (UINT8 *)&KeyBundle, 0x00, sizeof(KeyBundle) );
	  memset( keydata, 0x00, sizeof(keydata) );
	  memset( eskey, 0x00, sizeof(eskey) );
	  
	  memset(temp, 0x00, sizeof(temp));
	  memset(buf, 0x00, sizeof(buf));
	  }
	
	PED_InUse( FALSE );
	
	return( result );
}
*/

// ---------------------------------------------------------------------------
// FUNCTION: To generate MAC value by using Master/Session key algorithm.
// INPUT   : mode   - algorithm and padding method.
//		              MAC_ALGx + MAC_PADx
// 	         index  - session key index used to encrypt the PIN block.
//	         icv    - initial chain vector for CBC mode. (8-byte)
//           length - length of data. (may be zero if data string is empty)
//           data   - data to be authenticated.
// OUTPUT  : mac    - the MAC value. (8-byte)
//           icv    - next icv values.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UINT8	PED_MSKEY_GenMAC( UINT16 mode, UINT8 index, UINT8 *icv, UINT16 length, UINT8 *data, UINT8 *mac )
{
    UINT8	pkey[PED_PEK_MSKEY_LEN + 1];    // L-V
    UINT8	skey[PED_PEK_MSKEY_LEN+1];	// L-V
    UINT8	mkey[PED_PEK_MSKEY_LEN+1];	// L-V
    X9143_TDES_PED_KEY_BUNDLE   tdesKeyBundle;
    X9143_AES_PED_KEY_BUNDLE    aesKeyBundle; 
    UINT8	keydata[X9143_AES_KEY_DATA_LEN + 1]; // L-V
    UINT8	MkeyIndex;
    UINT8	result;
    UINT8	mac8[8];
    UINT8   mac16[16];
    UINT8   keyType;


	PED_InUse(TRUE);
	
	result = apiOK;

    // check PEK SKEY index
    if(index < MAX_PEK_SKEY_CNT)
    {
        PED_PEK_GetKeyType(&keyType);
        PED_SetKPKStatus(keyType, 1);

        if((keyType == TDES_128) || (keyType == TDES_192))
        {
            // get PEK MKEY bundle
            PED_PEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
            OS_SECM_GetData(ADDR_PED_PEK_TDES_MKEY_01 + (MkeyIndex * PED_PEK_TDES_MSKEY_SLOT_LEN), PED_PEK_TDES_MSKEY_SLOT_LEN, (UINT8 *)&tdesKeyBundle);

            // verify PEK MKEY bundle
            if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, (UINT8 *)&tdesKeyBundle, keydata, mac8, (UINT8 *)0 ))
            {
                // check PEK MKEY key usage
                if((tdesKeyBundle.Usage[0] == 'K') && (tdesKeyBundle.Usage[1] == '0')) // for KEY encryption?
                {
                    // retrieve PEK MKEY
                    if(!X9143_DecryptKeyBundle_TDES(mac8, keydata, mkey, (UINT8 *)0 ))	// mkey=PEK MKEY (as the KBPK for SKEY)
                    {
                        // reset flag
                        PED_SetKPKStatus(keyType, 0);

                        result = apiFailed;
                        goto EXIT;
                    }
                }
            }

            if(result == apiOK)
            {
                result = apiFailed;

                // get PEK SKEY bundle
                OS_SECM_GetData(ADDR_PED_PEK_TDES_SKEY_01 + (index * PED_PEK_TDES_MSKEY_SLOT_LEN), PED_PEK_TDES_MSKEY_SLOT_LEN, (UINT8 *)&tdesKeyBundle);

                // verify PEK SKEY bundle
                if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, (UINT8 *)&tdesKeyBundle, keydata, mac8, mkey))
                {
                    // check PEK SKEY key usage
                    if((tdesKeyBundle.Usage[0] == 'D') && (tdesKeyBundle.Usage[1] == '0')) // for DATA encryption?
                    {
                        // retrieve PEK SKEY
                        if(X9143_DecryptKeyBundle_TDES(mac8, keydata, pkey, mkey)) // pkey=PEK SKEY
                        {
                            // ==== [Debug] ====
                            printf("pkey = ");
                            for(int i = 0 ; i < pkey[0] + 1 ; i++)
                                printf("%02x", pkey[i]);
                            printf("\n");
                            // ==== [Debug] ====

                            memmove(skey, &pkey[1], pkey[0]);

                            result = apiOK;
                        }
                    }
                }
            }
        }
        else if((keyType == AES_128) || (keyType == AES_192) || (keyType == AES_256))
        {
            // get PEK MKEY bundle
            PED_PEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
            OS_SECM_GetData(ADDR_PED_PEK_AES_MKEY_01 + (MkeyIndex * PED_PEK_AES_MSKEY_SLOT_LEN), PED_PEK_AES_MSKEY_SLOT_LEN, (UINT8 *)&aesKeyBundle);

            // verify PEK MKEY bundle
            if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, (UINT8 *)&aesKeyBundle, keydata, mac16, (UINT8 *)0 ))
            {
                // check MKEY key usage
                if((aesKeyBundle.Usage[0] == 'K') && (aesKeyBundle.Usage[1] == '0')) // for KEY encryption?
                {
                    // retrieve PEK MKEY
                    if(!X9143_DecryptKeyBundle_AES(mac16, keydata, mkey, (UINT8 *)0 ))	// mkey=PEK MKEY (as the KBPK for SKEY)
                    {
                        // reset flag
                        PED_SetKPKStatus(keyType, 0);

                        result = apiFailed;
                        goto EXIT;
                    }
                }
            }

            if(result == apiOK)
            {
                result = apiFailed;

                // get PEK SKEY bundle
                OS_SECM_GetData(ADDR_PED_PEK_AES_SKEY_01 + (index * PED_PEK_AES_MSKEY_SLOT_LEN), PED_PEK_AES_MSKEY_SLOT_LEN, (UINT8 *)&aesKeyBundle);

                // verify PEK SKEY bundle
                if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, (UINT8 *)&aesKeyBundle, keydata, mac16, mkey))
                {
                    // check PEK SKEY key usage
                    if((aesKeyBundle.Usage[0] == 'D') && (aesKeyBundle.Usage[1] == '0')) // for DATA encryption?
                    {
                        // retrieve PEK SKEY
                        if(X9143_DecryptKeyBundle_AES(mac16, keydata, pkey, mkey)) // pkey=PEK SKEY
                        {
                            // ==== [Debug] ====
                            printf("pkey = ");
                            for(int i = 0 ; i < pkey[0] + 1 ; i++)
                                printf("%02x", pkey[i]);
                            printf("\n");
                            // ==== [Debug] ====

                            memmove(skey, &pkey[1], pkey[0]);

                            result = apiOK;
                        }
                    }
                }
            }
        }

        // reset flag
        PED_SetKPKStatus(keyType, 0);

        if(result == apiOK)
        {
            result = apiFailed;
            
            switch(mode & 0x0F)
            {
                case MAC_ISO16609: // CBC-TDEA-MAC

                    if((keyType == AES_128) || (keyType == AES_192) || (keyType == AES_256))
                        result = apiFailed;
                    else
                    {
                        PED_GenMAC_ISO16609(mode & 0xF0, skey, icv, length, data, mac);
                        result = apiOK;
                    }
                    
                    break;

                case MAC_ALG1: // CBC-MAC (ANSI X9.9)

                    if((keyType == AES_128) || (keyType == AES_192) || (keyType == AES_256))
                        result = apiFailed;
                    else
                    {
                        PED_GenMAC_ALG1(mode & 0xF0, skey, icv, length, data, mac);
                        result = apiOK;
                    }

                    break;

                case MAC_ALG2: // CBC-MAC (NA)
                    break;

                case MAC_ALG3: // CBC-MAC (ANSI X9.19)

                    if((keyType == AES_128) || (keyType == AES_192) || (keyType == AES_256))
                        result = apiFailed;
                    else
                    {
                        PED_GenMAC_ALG3(mode & 0xF0, skey, icv, length, data, mac);
                        result = apiOK;
                    }
                    
                    break;

                case MAC_ALG4: // (NA)
                    break;

                case MAC_ALG5: // (ONLY for AES key, to be implemented as CMAC)

                    if((keyType == TDES_128) || (keyType == TDES_192))
                        result = apiFailed;
                    else
                    {
                        //variable AES_128 (3) is equal to variable _AES128_ (2)
                        //variable AES_192 (4) is equal to variable _AES192_ (3)
                        //variable AES_256 (5) is equal to variable _AES256_ (4)
                        mode = (keyType - 1) << 8 | mode;

                        PED_GenMAC_ALG5( mode, skey, icv, length, data, mac );
                        result = apiOK;
                    }

                    break;

                case MAC_ALG6: // (NA)
                    break;
            }
        }

EXIT:
        // clear sensitive data
        memset(pkey, 0x00, sizeof(pkey)); // clear KEY data
        memset(mkey, 0x00, sizeof(mkey)); // clear KEY data
        memset(skey, 0x00, sizeof(skey)); // clear KEY data
        memset((UINT8 *)&tdesKeyBundle, 0x00, sizeof(tdesKeyBundle));
        memset((UINT8 *)&aesKeyBundle, 0x00, sizeof(aesKeyBundle));
        memset(keydata, 0x00, sizeof(keydata));
    }

    PED_InUse(FALSE);

    return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate MAC by using DUKPT algorithm.
// INPUT   : mode   - algorithm of MAC.
//	     icv    - initial chain vector for CBC mode. (8-byte)
//	     length - length of data.
//	     data   - data to be processed.
// OUTPUT  : mac    - MAC. (8 bytes)
//	     ksn    - key serial number. (10 bytes)
// RETURN  : apiOK
//           apiFailed
//	     apiDeviceError	// PED inoperative due to over 1M use limit.
// NOTE    : The "DUKPT_RequestPinEntry()" shall be called prior to calling 
//           this function, the MAC key is updated over there.
// ---------------------------------------------------------------------------
UINT8	PED_DUKPT_GenMAC( UINT8 mode, UINT8 *icv, UINT16 length, UINT8 *data, UINT8 *mac, UINT8 *ksn )
{
UINT8	mackey[FUTURE_KEY_LEN];
UINT8	keylen;
UINT8	result;

	
	PED_InUse( TRUE );
	
	result = apiFailed;
	
//	OS_SECM_GetData( ADDR_MAC_KEY_REG, 1, (UCHAR *)&keylen );	// retrieve length of the MAC key reg
//	if( keylen )
	if( PED_VerifyKeyStatus_DUKPT() )			// PATCH: 2010-03-09
	  {
	  memset( mackey, 0x00, sizeof(mackey) );
	  if( DUKPT_RequestPinEntry(  mackey, ksn, mac ) )	// PATCH: 2010-03-08, retrieve current ksn and new a next future key
	    {							// the "mackey" & "mac" are null parameters
	    OS_SECM_GetData( ADDR_MAC_KEY_REG+1, FUTURE_KEY_LEN, mackey );	// retrieve real current MAC key
	    
//	    LIB_DumpHexData( 0, 0, FUTURE_KEY_LEN, mackey );
	    
//	    // retrieve MAC_KEY(16) as DESKEY
//	    OS_SECM_GetData( ADDR_MAC_KEY_REG+1, FUTURE_KEY_LEN, mackey );
//	    OS_SECM_GetData( ADDR_KSN_REG, KSN_REG_LEN, ksn );
	    
	    switch( mode & 0x0F )
	    	{
	    	case MAC_ISO16609:	// CBC-TDEA-MAC
	                 
	             PED_GenMAC_ISO16609( mode & 0xF0, mackey, icv, length, data, mac );
	                 
	             result = apiOK;
	    	     break;	
	    	
	    	case MAC_ALG1:	// CBC-MAC (ANSI X9.9)
	    	
	    	     PED_GenMAC_ALG1( mode & 0xF0, mackey, icv, length, data, mac );
	    	     
	    	     result = apiOK;
	    	     break;
	    	     
	    	case MAC_ALG2:	// CBC-MAC (NA)
	    	     break;
	    	     
	    	case MAC_ALG3:	// CBC-MAC (ANSI X9.19)
			
//			LIB_OpenKeyAll();
//			LIB_DispHexByte( 0, 0, mode );
//			LIB_DumpHexData( 0, 1, 16, mackey );
//			LIB_DumpHexData( 0, 1, 8, icv );
//			LIB_DumpHexData( 0, 1, length, data );

			
	    	     PED_GenMAC_ALG3( mode & 0xF0, mackey, icv, length, data, mac );
	    	     
//	    	     	LIB_DumpHexData( 0, 1, 8, mac );
	    	     
	    	     result = apiOK;
	    	     break;
	    	     
	    	case MAC_ALG4:	// (NA)
	    	     break;
	    	     
	    	case MAC_ALG5:	// (NA)
	    	     break;
	    	     
	    	case MAC_ALG6:	// (NA)
	    	     break;
	    	}
	    }
	  else
	    result = apiDeviceError;	// 2010-04-30
	    
	  memset( mackey, 0x00, sizeof(mackey) );				// clear KEY data	
	  OS_SECM_ClearData( ADDR_MAC_KEY_REG, FUTURE_KEY_SLOT_LEN, 0x00 );	// clear KEY data
	  }
	
	PED_InUse( FALSE );
	
	return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate MAC value by using Fixed key algorithm.
// INPUT   : mode   - algorithm and padding method.
//		      MAC_ALGx + MAC_PADx
// 	     index  - fixed key index used to encrypt the PIN block.
//	     icv    - initial chain vector for CBC mode. (8-byte)
//           length - length of data. (may be zero if data string is empty)
//           data   - data to be authenticated.
// OUTPUT  : mac    - the MAC value. (8-byte)
//           icv    - next icv values.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
#if	0
UINT8	PED_FXKEY_GenMAC( UINT8 mode, UINT8 index, UINT8 *icv, UINT16 length, UINT8 *data, UINT8 *mac )
{
#ifdef	_USE_IRAM_PARA_
// 2014-11-24, allocate sensitive data buffer on MCU internal SRAM
#define	FKEY_SIZE_GMAC		PED_FKEY_LEN+1
#define	FKEY_ADDR_GMAC		0x0000F000	// pointer to MCU internal SRAM

UINT8	*fkey = (UINT8 *)FKEY_ADDR_GMAC;
#else
UINT8	fkey[PED_FKEY_LEN+1];	// L-V
#endif

PED_KEY_BUNDLE	KeyBundle;
UINT8	keydata[KEY_DATA_LEN+1]; // L-V

UINT8	result;
UINT8	mac8[8];

	
	PED_InUse( TRUE );
	
	result = apiFailed;
	
	// check FKEY index
	if( index < MAX_FKEY_CNT )
	  {
	  // get FIXED key bundle
	  OS_SECM_GetData( ADDR_PED_FKEY_01+(index*PED_FKEY_SLOT_LEN), PED_FKEY_SLOT_LEN, (UINT8 *)&KeyBundle );
	  
	  // verify FIXED key bundle
	  if( TR31_VerifyKeyBundle( PED_FKEY_SLOT_LEN, (UINT8 *)&KeyBundle, keydata, mac8 ) )
	    {
	    // check key usage
	    if( (KeyBundle.Usage[0] == 'D') && (KeyBundle.Usage[1] == '0') ) // for DATA encryption?
	      {
	      // retrieve FIXED key
	      if( TR31_DecryptKeyBundle( mac8, keydata, fkey ) )
	        result = apiOK;
	      }
	    }
	  
	  if( result == apiOK )
	    {
	    result = apiFailed;
	    
	    switch( mode & 0x0F )
	          {
	          case MAC_ISO16609:	// CBC-TDEA-MAC
	               
	               PED_GenMAC_ISO16609( mode & 0xF0, &fkey[1], icv, length, data, mac );
	               
	               result = apiOK;
	  	       break;
	               
	  	  case MAC_ALG1:	// CBC-MAC (ANSI X9.9)
	  	
	  	       PED_GenMAC_ALG1( mode & 0xF0, &fkey[1], icv, length, data, mac );
	  	     
	  	       result = apiOK;
	  	       break;
	  	     
	  	  case MAC_ALG2:	// CBC-MAC (NA)
	  	       break;
	  	     
	  	  case MAC_ALG3:	// CBC-MAC (ANSI X9.19)
	  	
	  	       PED_GenMAC_ALG3( mode & 0xF0, &fkey[1], icv, length, data, mac );
	  	     
	  	       result = apiOK;
	  	       break;
	  	     
	  	  case MAC_ALG4:	// (NA)
	  	       break;
	  	     
	  	  case MAC_ALG5:	// (NA)
	  	       break;
	  	     
	  	  case MAC_ALG6:	// (NA)
	  	       break;
	  	  }
	    }

#ifdef	_USE_IRAM_PARA_	    
	  memset( fkey, 0x00, sizeof(UINT8)*FKEY_SIZE_GMAC );
#else
	  memset( fkey, 0x00, sizeof(fkey) );	// clear KEY data
#endif

	  // PATCH: 2009-04-07, clear sensitive data
	  memset( (UINT8 *)&KeyBundle, 0x00, sizeof(KeyBundle) );
	  memset( keydata, 0x00, sizeof(keydata) );
	  }
	
	PED_InUse( FALSE );
	
	return( result );
}
#endif
