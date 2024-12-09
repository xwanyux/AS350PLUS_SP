//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : 							    **
//**  PRODUCT  : AS350 PLUS						    **
//**                                                                        **
//**  FILE     : OS_PIN4-1.C						    **
//**  MODULE   : 			   				    **
//**                                                                        **
//**  FUNCTION : OS::PED4-1 (MAC code Generation by AES-DUKPT)		    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2023/04/11                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2023 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
// Generate MAC by using the following KEYs derived by DUKPT:
// (1) TDES KEY MAC is the same as defined in DUKPT-2004. (OS_PED4.c)
// (2) AES KEY MAC is only applied to MAC algorithm 5 & 6, as defined in ISO9797-1-2011.
// 	MAC Algorithm 5 is commonly known as OMAC1 or CMAC.	(to be implemented here)
// 	MAC Algorithm 6 is commonly known as LMAC.		(RFU)
//
//----------------------------------------------------------------------------
#include <string.h>

#if	1
#include "POSAPI.h"
#include "OS_LIB.h"
#include "OS_SECM.h"
#include "OS_PED.h"
#include "DEV_PED.h"
//#include "ANS_TR31_2010.h"
#else
#include "bsp_types.h"
#include "POSAPI.h"
#include "OS_LIB.h"
#include "OS_SECM.h"
#include "OS_PED.h"
#include "DEV_SRAM.h"
#include "DEV_PED.h"
#include "ANS_TR31.h"
#endif


extern	UCHAR	AES_DUKPT_RequestMacKey( UCHAR keyType, UCHAR *workingKey );	// defined in DUKPT-AES.c
extern	ULONG	Key_Length( ULONG keyType );					//
extern  void    AES_DUKPT_GetKSN(UCHAR *ksn);



// ---------------------------------------------------------------------------
// FUNCTION: To derive subkeys from ISO 9797-1 algorithm 5.
// INPUT   : srckey - source key. (AES key)
//		      AES:  16/24/32 bytes (AES-CMAC)
//		      TDEA: 16/24 bytes (N/A here, only for TDEA-CMAC)
//	     keysize - size of srckey.
// OUTPUT  : k1	- subkey K1. (fixed 16 bytes)
//	     k2 - subkey K2. (fixed 16 bytes)
// RETURN  : TRUE	- success.
//           FALSE	- failure. (Key Block Protection Key is absent)
// ---------------------------------------------------------------------------
UINT32	PED_CMAC_DeriveSubKey_AES( UINT8 keysize, UINT8 *srckey, UINT8 *k1, UINT8 *k2 )
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
	api_aes_encipher( idata, odata, srckey, keysize );	// odata = S(16)
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
// FUNCTION: To generate MAC value by using ISO 9797-1 algorithm 5.
//		MAC Algorithm 5 is commonly known as OMAC1 [19] or CMAC [14].
//		[14]: NIST Special Publication 800-38B: 2005
//		      The CMAC Mode for Authentication.
//		Key Derivation Method 2. (K1, K2 are derived from the MAC algorithm key K)
//		Final Iteration 3.
//			if the data string to be input to the MAC algorithm had a length (in bits) that is a
//			positive integer multiple of n then: (Hq is the last block of MAC algorithm)
//			Hq := eK (Dq �� Hq -1 �� K1),
//			else
//			Hq := eK (Dq �� Hq -1 �� K2).
//		Output Transformation 1. (G := Hq)
//
// INPUT   : mode   - bit0~7, padding method. (MAC_PADx)
//		      bit8~15,key type
//		      Padding method 4 shall ONLY be used with MAC Algorithm 5.
//	     keylen - size of aeskey; 16,24,32 bytes
//           aeskey - AES key used to encrypt.
//	     icv    - initial chain value. (16,24,32byte)
//           len    - length of data.
//           data   - data to be authenticated.
// OUTPUT  : mac    - the MAC value. (16-byte)
//           icv    - next icv values.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
void	PED_GenMAC_ALG5( UINT16 mode, UINT8 *aeskey, UINT8 *icv, UINT16 len, UINT8 *data, UINT8 *mac )
{
UINT16	i, j;
UINT16	cnt;

UINT16	left_bytes;
UINT8	ctext[32];
UINT8	ptext[32];
UINT8	buf[32];
UINT8	buf2[32];
UINT8	K1[16];
UINT8	K2[16];

UINT8	keySize;
UINT8	keyType;
UINT8	pm;


	keyType = (mode & 0xff00) >> 8;
	keySize = Key_Length( keyType )/8;
	pm	= mode & 0x00f0;

	// based on 16 bytes data block
	cnt = len / sizeof(K1);
        left_bytes = len % sizeof(K1);
        
        PED_CMAC_DeriveSubKey_AES( keySize, aeskey, K1, K2 );	// derive subkeys K1 and K2

	i = 0;
	
	if( (cnt == 1) && (left_bytes == 0) )	// exact 1 block
	  {
	  for( j=0; j<keySize; j++ )
	     ptext[j] = icv[j] ^ data[j];
	  
	  goto LAST;
	  }

        if( cnt )
          {
          if( left_bytes )
            cnt++;
            
          for( i=0; i<cnt-1; i++ )
             {
             // XOR( ICV, D1 )
             for( j=0; j<sizeof(K1); j++ )
                ptext[j] = icv[j] ^ data[i*sizeof(K1)+j];

	     api_aes_encipher( ptext, ctext, aeskey, keySize );

	     memmove( icv, ctext, sizeof(K1) );
             }
          }

PADDING:
	
	if( left_bytes )
	  {
          PED_PostfixPadding4( keySize, left_bytes, &data[i*sizeof(K1)], buf ); // PED_PostfixPadding() to be modified for unit block other than 8 bytes
          
          // XOR( ICV, Postfix )
	  for( j=0; j<sizeof(K1); j++ )
	     ptext[j] = icv[j] ^ buf[j];
          }
        else
          {
	  memmove( ptext, &data[i*sizeof(K1)], keySize );
	  for( j=0; j<sizeof(K1); j++ )
	     ptext[j] ^= icv[j];
          }

LAST:
        memset( buf, 0x00, sizeof(buf) );
        
	if( left_bytes == 0 )
	  memmove( buf, K1, sizeof(K1) );
	else
	  memmove( buf, K2, sizeof(K2) );
	  
	for( j=0; j<sizeof(K1); j++ )
	   ptext[j] ^= buf[j];

        api_aes_encipher( ptext, ctext, aeskey, keySize );
        
        memmove( mac, ctext, 16 );
        
        memset( K1, 0x00, sizeof(K1) );
        memset( K2, 0x00, sizeof(K2) );
}

// ---------------------------------------------------------------------------
//  This function is used to replace "PED_DUKPT_GenMAC()" in OS_PED4.c
// ---------------------------------------------------------------------------
// FUNCTION: To generate MAC by using DUKPT algorithm.
// INPUT   : mode   - algorithm of MAC.
//		      bit0~7: (original)
//		      bit8~15: (new for key type)
//			0: _2TDEA_
//			1: _3TDEA_
//			2: _AES128_
//			3: _AES192_
//			4: _AES256_
//	     icv    - initial chain vector for CBC mode. (8-byte)
//	     length - length of data.
//	     data   - data to be processed.
// OUTPUT  : mac    - MAC. (8/16/24/32 bytes)
//	     ksn    - key serial number. (12 bytes)
// RETURN  : apiOK
//           apiFailed
//	     apiDeviceError	// PED inoperative due to over 1M use limit.
// NOTE    : the first parameter of the following PED API shall be modified.
//	     api_ped_GenMAC_DUKPT( UINT mode, UCHAR *icv, UINT length, UCHAR *data, UCHAR *mac, UCHAR *ksn );
// ---------------------------------------------------------------------------
UINT8	PED_AES_DUKPT_GenMAC( UINT16 mode, UINT8 *icv, UINT16 length, UINT8 *data, UINT8 *mac, UINT8 *ksn )
{
UINT8	mackey[32];
UINT8	keylen;
UINT8	result;
UINT16	keyType;
UINT16	keyUsage = 2;		// 2=_Message_Authentication_generation_

	
	PED_InUse( TRUE );
	
	result = apiFailed;
	
	keyType = (mode & 0xff00) >> 8;
	  
//	if( PED_VerifyKeyStatus_DUKPT() )
//	  {
	  memset( mackey, 0x00, sizeof(mackey) );
	  if( AES_DUKPT_RequestMacKey( keyType, mackey ) )	// NEW
	    {
            printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);    // ==== [Debug] ====

            // ==== [Debug] ====
            if((keyType == (TDES_128 - 1)) || (keyType == (AES_128 - 1)))
            {
                printf("key = ");
                for(int i = 0 ; i < 16 ; i++)
                    printf("%02x", mackey[i]);
                printf("\n");
            }
            else if((keyType == (TDES_192 - 1)) || (keyType == (AES_192 - 1)))
            {
                printf("key = ");
                for(int i = 0 ; i < 24 ; i++)
                    printf("%02x", mackey[i]);
                printf("\n");
            }
            else if(keyType == (AES_256 - 1))
            {
                printf("key = ");
                for(int i = 0 ; i < 32 ; i++)
                    printf("%02x", mackey[i]);
                printf("\n");
            }
            // ==== [Debug] ====

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

	    	     PED_GenMAC_ALG3( mode & 0xF0, mackey, icv, length, data, mac );
	    	     
	    	     result = apiOK;
	    	     break;
	    	     
	    	case MAC_ALG4:	// (NA)
	    	     break;
	    	     
	    	case MAC_ALG5:	// (ONLY for AES key, to be implemented as CMAC)
		       
	    	     PED_GenMAC_ALG5( mode, mackey, icv, length, data, mac );

	    	     result = apiOK;
	    	     break;
	    	     
	    	case MAC_ALG6:	// (NA)
	    	     break;
	    	}

            AES_DUKPT_GetKSN(ksn);
	    }
	  else
	    result = apiDeviceError;	// 2010-04-30

//	  }

	memset( mackey, 0x00, sizeof(mackey) );				// clear KEY data
	
	PED_InUse( FALSE );
	
	return( result );
}
