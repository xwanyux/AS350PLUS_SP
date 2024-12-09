//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : i.MX6UL							                        **
//**  PRODUCT  : AS350-X6							                        **
//**                                                                        **
//**  FILE     : OS_PED3.C						                            **
//**  MODULE   : 			   				                                **
//**                                                                        **
//**  FUNCTION : OS::PED3 (PIN Block Generation)			                **
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

#include "PEDKconfig.h"

#include "POSAPI.h"
#include "OS_LIB.h"
#include "OS_SECM.h"
#include "OS_PED.h"
#include "PEDAPI.h"

//#include "DEV_SRAM.h"
#include "DEV_PED.h"
#include "ANS_TR31_2010.h"
#include "ANS_TR31_2018.h"
#include "SRED_Func.h"

extern  UINT8   *ped_pin_ptr;


// ---------------------------------------------------------------------------
// FUNCTION: Load 12 rightmost digits of PAN.
// INPUT   : ipan - PAN digits. (L-V, the full PAN with check digit)
// OUTPUT  : opan - fixed 12 PAN digits. (ASCII)
//                 RULE:
//                 the 12 rightmost digits of PAN excluding the check digit.
//                 if PAN excluding the check digit is less than 12 digits,
//                 the digits are right justified and padded to the left with
//                 zeros.
// RETURN  : TRUE  - OK.
//           FALSE - ERROR. (PAN is not available)
// ---------------------------------------------------------------------------
void	PED_LoadPAN12( UINT8 *ipan, UINT8 *opan )
{
UINT8	i;
UINT8	panlen;
UINT8	len;

	memset( opan, 0x30, 12 );	// preset to '0's
      
	panlen = ipan[0];
	if( panlen > 12 )
	  len = 12;
	else
	  len = panlen;
        
	for( i=0; i<len; i++ )
	   opan[11-i] = ipan[panlen-(1+i)];
}

// ---------------------------------------------------------------------------
// FUNCTION: construct a PIN block using ISO 9564 format 0 (= ANSI X9.8).
// INPUT   : format   - PIN block format = PIN xor PAN
//                      plaintext PIN field
//                          C N P P P P P/F P/F P/F P/F P/F P/F P/F P/F F F(16-nibble)
//                      PAN field
//                          0 0 0 0 A1 A2 ....... A12 (16-nibble)
//           pindata  - L-V, the PIN digits. (L=0 for PIN bypassed)
//           pandata  - L-V, the PAN digits.
// OUTPUT  : pinblock - the plaintext PIN block.
// RETURN  : none.
// NOTE    : The format PIN block shall be reversibly enciphered when transmitted.
// ---------------------------------------------------------------------------
void	PED_GenPinBlock_ISO0( UCHAR *pindata, UCHAR *pandata, UCHAR *pinblock )
{
UINT8	epinblock[16];
UINT16	i, j;
UINT8	nibble_h;
UINT8	nibble_l;
UINT8	panblock[16];
UINT8	pan[12];

	
	PED_LoadPAN12( pandata, pan );

	for( i=0; i<16; i++ )
	   epinblock[i]=0x0f; // filler

	epinblock[0] = 0x00; // Control field
	epinblock[1] = *pindata++; // length of pin digits

	if( epinblock[1] != 0 ) // PIN available?
	  {
	  // convert ASCII[2] to BCD[2] for PIN
	  for( i=0; i<epinblock[1]; i++ )
	     epinblock[i+2] = (*pindata++) & 0x0f;
	  }

	// convert ASCII[2] to BCD[2] for PAN
	panblock[0] = 0x00;
	panblock[1] = 0x00;
	panblock[2] = 0x00;
	panblock[3] = 0x00;
	for( i=4; i<16; i++ )
	   panblock[i] = pan[i-4] & 0x0f;

	// generate PIN block by modulo-2 addition of PIN & PAN
	for( i=0; i<16; i++ )
	   epinblock[i] ^= panblock[i];

	// convert BCD[2] to BCD[1]
	j = 0;
	for( i=0; i<8; i++ )
	   {
	   nibble_h = epinblock[j] << 4;
	   j++;
	   nibble_l = epinblock[j];
	   j++;
	   epinblock[i] = nibble_h | nibble_l;
	   }
	   
	memmove( pinblock, epinblock, 8 );

	// clear sensitive data
	memset( epinblock, 0x00, sizeof(epinblock) );
}

// ---------------------------------------------------------------------------
// FUNCTION: construct a PIN block using ISO 9564 format 1.
// INPUT   : format   - PIN block format = PIN + TRANSACTION data.
//                      plaintext PIN field
//                          C N P P P P P/F P/F P/F P/F P/F P/F P/F P/F F F(16-nibble)
//                      Transaction field
//                          0 0 0 0 ... T T T ... T [56-(N*4) bits]
//			PIN Block Format
//			    C N P P P P P/T P/T P/T P/T P/T P/T P/T P/T T T
//
//           pindata    - L-V, the PIN digits. (L=0 for PIN bypassed)
//           transdata  -   V, the TRANS data. (0000B..1111B)
//			       unique binary data, such as transaction sequence number,
//			       time stamp, random number or similar. (implicit)
// OUTPUT  : pinblock - the plaintext PIN block.
// RETURN  : none.
// NOTE    : 1. The format PIN block shall be reversibly enciphered when transmitted.
//           2. This format should be used in situation where the PAN is not available.
//           3. The transaction field should not be transmitted and is not required
//	        in order to translate the PIN block to another format since the PIN
//	        length is known.
// ---------------------------------------------------------------------------
void	PED_GenPinBlock_ISO1( UCHAR *pindata, UCHAR *pinblock )
{
UINT8	epinblock[16];
UINT16	i, j;
UINT8	nibble_h;
UINT8	nibble_l;
UINT8	transfield[8];
UINT8	transblock[16];
UINT8	*transdata;
UINT8	index;
UINT8	len;


	for( i=0; i<16; i++ )
	   epinblock[i]=0x00; // filler

	epinblock[0] = 0x01; // Control field
	epinblock[1] = *pindata++; // length of pin digits

	if( epinblock[1] != 0 ) // PIN available?
	  {
	  // convert ASCII[2] to BCD[2] for PIN
	  for( i=0; i<epinblock[1]; i++ )
	     epinblock[i+2] = (*pindata++) & 0x0f;
	  }

	// generate random number for TRANSACTION FIELD data
	api_sys_random( transfield );
	transdata = &transfield[0];

	// convert HEX[1] to HEX[2] for TRANSACTION FIELD data
	memset( transblock, 0x00, sizeof(transblock) );
	len = (56 - epinblock[1]*4) / 4;  // nibbles of TRANSACTION FIELD data
	index = 16 - len;
	for( i=index; i<16; i+=2 )
	   {
	   transblock[i+0] = ((*transdata) & 0xf0) >> 4;
	   if( (i+1) < 16 )	// 2022-02-22
	     transblock[i+1] = ((*transdata++) & 0x0f);
	   }
	   
	// generate PIN block by concatenation of PIN & PAN
	for( i=0; i<16; i++ )
	   epinblock[i] |= transblock[i];

	// convert BCD[2] to BCD[1]
	j = 0;
	for( i=0; i<8; i++ )
	   {
	   nibble_h = epinblock[j] << 4;
	   j++;
	   nibble_l = epinblock[j];
	   j++;
	   epinblock[i] = nibble_h | nibble_l;
	   }
	   
	memmove( pinblock, epinblock, 8 );
	
	// clear sensitive data
	memset( epinblock, 0x00, sizeof(epinblock) );
}

// ---------------------------------------------------------------------------
// FUNCTION: construct a PIN block using ISO 9564 format 2.
// INPUT   : format   - PIN block format = PIN xor PAN
//                      plaintext PIN block.
//                          C N P P P P P/F P/F P/F P/F P/F P/F P/F P/F F F(16-nibble)
//           pindata  - L-V, the PIN digits. (L=0 for PIN bypassed)
// OUTPUT  : pinblock - the plaintext PIN block.
// RETURN  : none.
// NOTE    : The format 2 PIN block shall only be used in ICC offline PIN verification.
// ---------------------------------------------------------------------------
void	PED_GenPinBlock_ISO2( UCHAR *pindata, UCHAR *pinblock )
{
UINT8	epinblock[16];
UINT16	i, j;
UINT8	nibble_h;
UINT8	nibble_l;


	for( i=0; i<16; i++ )
	   epinblock[i]=0x0f; // filler

	epinblock[0] = 0x02; // Control field
	epinblock[1] = *pindata++; // length of pin digits

	if( epinblock[1] != 0 ) // PIN available?
	  {
	  // convert ASCII[2] to BCD[2] for PIN
	  for( i=0; i<epinblock[1]; i++ )
	     epinblock[i+2] = (*pindata++) & 0x0f;
	  }

	// convert BCD[2] to BCD[1]
	j = 0;
	for( i=0; i<8; i++ )
	   {
	   nibble_h = epinblock[j] << 4;
	   j++;
	   nibble_l = epinblock[j];
	   j++;
	   epinblock[i] = nibble_h | nibble_l;
	   }
	   
	memmove( pinblock, epinblock, 8 );
	
	// clear sensitive data
	memset( epinblock, 0x00, sizeof(epinblock) );
}

// ---------------------------------------------------------------------------
// FUNCTION: construct a PIN block using ISO 9564 format 3.
// INPUT   : format   - PIN block format = PIN xor PAN
//                      plaintext PIN field
//                          C N P P P P P/F P/F P/F P/F P/F P/F P/F P/F F F(16-nibble)
//                      PAN field
//                          0 0 0 0 A1 A2 ....... A12 (16-nibble)
//           pindata  - L-V, the PIN digits. (L=0 for PIN bypassed)
//           pandata  - L-V, the PAN digits.
// OUTPUT  : pinblock - the plaintext PIN block.
//	     sn       - the selected sequence number (the filler, may be from 1010B..1111B) - RFU
// RETURN  : none.
// NOTE    : The format PIN block shall be reversibly enciphered when transmitted.
// ---------------------------------------------------------------------------
void	PED_GenPinBlock_ISO3( UCHAR *pindata, UCHAR *pandata, UCHAR *pinblock )
{
UINT8	epinblock[16];
UINT16	i, j;
UINT8	nibble_h;
UINT8	nibble_l;
UINT8	panblock[16];
UINT8	random[8];
UINT8	filler;
UINT8	pan[12];

	
	PED_LoadPAN12( pandata, pan );

//	do{
//	  // randomly generate a filler between 10 and 15
//	  api_sys_random( random );
//	  filler = random[0] & 0x0f;
//	  }while( filler < 10 );	// PATCH: 2009-04-08, fill digits from 10..15
//
//	for( i=0; i<16; i++ )
//	   epinblock[i]=filler; // filler

	// PATCH: 2009-04-29, each digit should be random
	i = 0;
	while(i<16)
	     {
	     // randomly generate a filler between 10 and 15
	     api_sys_random( random );
	     
	     filler = random[0] & 0x0f;	// 00..15
	     if( filler >= 10 )
	       epinblock[i++]=filler;	// random filler
	     }

	epinblock[0] = 0x03;	// PATCH: 2009-04-08, Control field=3
	epinblock[1] = *pindata++; // length of pin digits

	if( epinblock[1] != 0 ) // PIN available?
	  {
	  // convert ASCII[2] to BCD[2] for PIN
	  for( i=0; i<epinblock[1]; i++ )
	     epinblock[i+2] = (*pindata++) & 0x0f;
	  }

	// convert ASCII[2] to BCD[2] for PAN
	panblock[0] = 0x00;
	panblock[1] = 0x00;
	panblock[2] = 0x00;
	panblock[3] = 0x00;
	for( i=4; i<16; i++ )
	   panblock[i] = pan[i-4] & 0x0f;

	// generate PIN block by modulo-2 addition of PIN & PAN
	for( i=0; i<16; i++ )
	   epinblock[i] ^= panblock[i];

	// convert BCD[2] to BCD[1]
	j = 0;
	for( i=0; i<8; i++ )
	   {
	   nibble_h = epinblock[j] << 4;
	   j++;
	   nibble_l = epinblock[j];
	   j++;
	   epinblock[i] = nibble_h | nibble_l;
	   }
	   
	memmove( pinblock, epinblock, 8 );
	
	// clear sensitive data
	memset( epinblock, 0x00, sizeof(epinblock) );
}

// ---------------------------------------------------------------------------
// FUNCTION: construct a PIN block using ISO 9564-1:2017 format 4.
// INPUT   : format   - enciphered PIN block format:
//			BLK_A = AES(K,PIN)
//			BLK_B = BLK_A XOR PAN
//			EPB   = AES(K,BLK_B)
//
//                      PIN field (32-nibble)
//                          C   N   P   P   P   P   P/F P/F P/F P/F P/F P/F P/F P/F F   F
//			    R   R   R   R   R   R   R   R   R   R   R   R   R   R   R   R
//			    where:
//				C=0100
//				N=0100~1100
//				F=1010
//				R=0000~1111 (randomly)
//                      PAN field (32-nibble)
//                          M   A   A   A   A   A   A   A   A   A   A   A   A   A/0 A/0 A/0
//			    A/0 A/0 A/0 A/0 0   0   0   0   0   0   0   0   0   0   0   0
//			    where:
//				M=0000~0111 if PAN length >=12
//				 =0000 if PAN length <12
//           pindata  - L-V, the PIN digits. (L=0 for PIN bypassed)
//           pandata  - L-V, the PAN digits.
//	     key      - AES key. (128 bits)
// OUTPUT  : epb      - the enciphered PIN block. (128 bits)
// RETURN  : none.
// ---------------------------------------------------------------------------
/*
void	PED_GenEncPinBlock_ISO4( UCHAR *pindata, UCHAR *pandata, UCHAR *key, UCHAR *epb )
{
UINT16	i, j;
UINT8	pinblock[32];
UINT8	panblock[32];
UINT8	random[8];
UINT8	blk_a[16];
UINT8	blk_b[16];
UINT8	nibble_h;
UINT8	nibble_l;
UINT8	filler;
UINT8	len;
// #define _DEBUG_AES_ 1

	// PIN field
	filler = 10;	// F=1010
	for( i=0; i<16; i++ )
	   pinblock[i] = filler;
	   
	pinblock[0] = 0x04;	// Control field, C=0100
	pinblock[1] = *pindata++; // length of pin digits N

	if( pinblock[1] != 0 ) // PIN available?
	  {
	  // convert ASCII[2] to BCD[2] for PIN
	  for( i=0; i<pinblock[1]; i++ )
	     pinblock[i+2] = (*pindata++) & 0x0f;
	  }

	for( i=0; i<2; i++ )
	   {
	   api_sys_random( random );
	   
	   for( j=0; j<8; j++ )
	      {
	      filler = random[j] & 0x0f;	// R=0000..1111
	      pinblock[(i*8)+16+j] = filler;
	      }
	   }

//	LIB_DumpHexData( 0, 0, 32, pinblock );

	// convert BCD[2] to BCD[1]
	j = 0;
	for( i=0; i<16; i++ )
	   {
	   nibble_h = pinblock[j] << 4;
	   j++;
	   nibble_l = pinblock[j];
	   j++;
	   pinblock[i] = nibble_h | nibble_l;
	   }
	   
#ifdef	_DEBUG_AES_
	_CLS();
	
	_PRINTF("AES key:\r\n");
	for( i=0; i<16; i++ )
	   _PRINTF("%02X ", key[i] );
	_PRINTF("\r\n");	   
	
	_PRINTF("PIN block:\r\n");
	for( i=0; i<16; i++ )
	   _PRINTF("%02X ", pinblock[i] );
	_PRINTF("\r\n");
#endif
//	LIB_DumpHexData( 0, 5, 16, pinblock );

	// PAN field
	memset( panblock, 0x00, sizeof(panblock) );	// pad digits=0000
	
	len = pandata[0];
	if( len >= 12 )	// PAN data length >=12
	  {
	  panblock[0] = pandata[0]-12;	// M=PAN_LEN-12
	  memmove( &panblock[1], &pandata[1], len );
	  }
	else	// PAN data length <12
	  {
	  panblock[0] = 0;		// M=0, the digits are right justified and padded to the left with zeros
	  memmove( &panblock[1+12-len], &pandata[1], len );
	  }

	// convert ASCII[2] to BCD[2] for PAN
	for( i=0; i<19; i++ )
	   panblock[i+1] &= 0x0F;

//	LIB_DumpHexData( 0, 8, 32, panblock );

	// convert BCD[2] to BCD[1]
	j = 0;
	for( i=0; i<16; i++ )
	   {
	   nibble_h = panblock[j] << 4;
	   j++;
	   nibble_l = panblock[j];
	   j++;
	   panblock[i] = nibble_h | nibble_l;
	   }
	   
#ifdef	_DEBUG_AES_
	_PRINTF("PAN block:\r\n");
	for( i=0; i<16; i++ )
	   _PRINTF("%02X ", panblock[i] );
	_PRINTF("\r\n");
#endif
//	LIB_DumpHexData( 0, 13, 16, panblock );

	// generate BLOCK_A = AES(KEY,PINBLOCK)
	api_aes_encipher( pinblock, blk_a, key, 16 );
	
	// generate BLOCK_B by modulo-2 addition of BLOCK_A & PAN
	for( i=0; i<16; i++ )
	   blk_b[i] = blk_a[i] ^ panblock[i];
#ifdef	_DEBUG_AES_
	_PRINTF("blk_b:\r\n");
	for( i=0; i<16; i++ )
	   _PRINTF("%02X ", blk_b[i] );
	_PRINTF("\r\n");
#endif	   
	// generate enciphered EPB = AES(KEY,BLOCK_B)
	api_aes_encipher( blk_b, epb, key, 16 );

#ifdef	_DEBUG_AES_
//	LIB_WaitKey();
//	_CLS();
	_PRINTF("EPB:\r\n");
	for( i=0; i<16; i++ )
	   _PRINTF("%02X ", epb[i] );
	_PRINTF("\r\n");	
	
//	LIB_DumpHexData( 0, 4, 16, epb );
	
	// DEBUG ONLY (decription) ==================================
	api_aes_decipher( blk_b, epb, key, 16 ); // blk_b
//	LIB_DumpHexData( 0, 0, 16, blk_b );
	
	for( i=0; i<16; i++ )			 // blk_a
	   blk_a[i] = blk_b[i] ^ panblock[i];
//	LIB_DumpHexData( 0, 2, 16, blk_a );
	
	memset( pinblock, 0x00, sizeof(pinblock) );
	api_aes_decipher( pinblock, blk_a, key, 16 );
//	LIB_DumpHexData( 0, 4, 16, pinblock );	 // plaintext pinblock

	_PRINTF("Decrypted PIN Block:\r\n");
	for( i=0; i<16; i++ )
	   _PRINTF("%02X ", pinblock[i] );
	_PRINTF("\r\n");
	LIB_WaitKey();
	
	// ===========================================================
#endif

	// clear sensitive data
	memset( pinblock, 0x00, sizeof(pinblock) );
	memset( panblock, 0x00, sizeof(panblock) );
	memset( random, 0x00, sizeof(random) );
	memset( blk_a, 0x00, sizeof(blk_a) );
	memset( blk_b, 0x00, sizeof(blk_b) );
}
*/

// ---------------------------------------------------------------------------
// FUNCTION: construct a PIN block using ISO 9564-1:2017 format 4.
// INPUT   : format   - enciphered PIN block format:
//			BLK_A = AES(K,PIN)
//			BLK_B = BLK_A XOR PAN
//			EPB   = AES(K,BLK_B)
//
//                      PIN field (32-nibble)
//                          C   N   P   P   P   P   P/F P/F P/F P/F P/F P/F P/F P/F F   F
//			    R   R   R   R   R   R   R   R   R   R   R   R   R   R   R   R
//			    where:
//				C=0100
//				N=0100~1100
//				F=1010
//				R=0000~1111 (randomly)
//                      PAN field (32-nibble)
//                          M   A   A   A   A   A   A   A   A   A   A   A   A   A/0 A/0 A/0
//			    A/0 A/0 A/0 A/0 0   0   0   0   0   0   0   0   0   0   0   0
//			    where:
//				M=0000~0111 if PAN length >=12
//				 =0000 if PAN length <12
//           pindata  - L-V, the PIN digits. (L=0 for PIN bypassed)
//           pandata  - L-V, the PAN digits.
//	     key      - AES key. (128/192/256 bits)
// OUTPUT  : epb      - the enciphered PIN block. (128 bits)
// RETURN  : none.
// ---------------------------------------------------------------------------
void	PED_GenEncPinBlock_ISO4( UCHAR *pindata, UCHAR *pandata, UCHAR *key, UCHAR *epb )
{
    UINT16  i, j;
    UINT8   pinblock[32];
    UINT8   panblock[32];
    UINT8   random[8];
    UINT8   blk_a[16];
    UINT8   blk_b[16];
    UINT8   nibble_h;
    UINT8   nibble_l;
    UINT8   filler;
    UINT8   len;
    UINT8   keyType;
    UINT8   keyMode;

    keyMode = PED_ReadKeyMode();
    
    if(keyMode == PED_KEY_MODE_ISO4)
        PED_PEK_GetKeyType(&keyType);
    else if(keyMode == PED_KEY_MODE_DUKPT)
        PED_AES_DUKPT_GetKeyType(&keyType);

    // PIN field
    filler = 10; // F=1010
    for(i = 0 ; i < 16 ; i++)
       pinblock[i] = filler;

    pinblock[0] = 0x04;       // Control field, C=0100
    pinblock[1] = *pindata++; // length of pin digits N

    if(pinblock[1] != 0) // PIN available?
    {
       // convert ASCII[2] to BCD[2] for PIN
       for(i = 0 ; i < pinblock[1] ; i++)
         pinblock[i + 2] = (*pindata++) & 0x0f;
    }

    for(i = 0 ; i < 2 ; i++)
    {
       api_sys_random(random);

       for(j = 0 ; j < 8 ; j++)
       {
         filler = random[j] & 0x0f; // R=0000..1111
         pinblock[(i * 8) + 16 + j] = filler;
       }
    }

    // convert BCD[2] to BCD[1]
    j = 0;
    for(i = 0 ; i < 16 ; i++)
    {
       nibble_h = pinblock[j] << 4;
       j++;
       nibble_l = pinblock[j];
       j++;
       pinblock[i] = nibble_h | nibble_l;
    }

    // PAN field
    memset(panblock, 0x00, sizeof(panblock)); // pad digits=0000

    len = pandata[0];
    if(len >= 12) // PAN data length >=12
    {
       panblock[0] = pandata[0] - 12; // M=PAN_LEN-12
       memmove(&panblock[1], &pandata[1], len);
    }
    else // PAN data length <12
    {
       panblock[0] = 0; // M=0, the digits are right justified and padded to the left with zeros
       memmove(&panblock[1 + 12 - len], &pandata[1], len);
    }

    // convert ASCII[2] to BCD[2] for PAN
    for(i = 0 ; i < 19 ; i++)
       panblock[i + 1] &= 0x0F;

    // convert BCD[2] to BCD[1]
    j = 0;
    for(i = 0 ; i < 16 ; i++)
    {
       nibble_h = panblock[j] << 4;
       j++;
       nibble_l = panblock[j];
       j++;
       panblock[i] = nibble_h | nibble_l;
    }

    // ==== [Debug] ====
    printf("pinblock = ");
    for(int i = 0 ; i < 16 ; i++)
        printf("%02x", pinblock[i]);
    printf("\n");

    if(keyType == AES_128)
    {
        printf("key = ");
        for(int i = 0 ; i < 16 ; i++)
            printf("%02x", key[i]);
        printf("\n");
    }
    else if(keyType == AES_192)
    {
        printf("key = ");
        for(int i = 0 ; i < 24 ; i++)
            printf("%02x", key[i]);
        printf("\n");
    }
    else if(keyType == AES_256)
    {
        printf("key = ");
        for(int i = 0 ; i < 32 ; i++)
            printf("%02x", key[i]);
        printf("\n");
    }
    // ==== [Debug] ====

    // generate BLOCK_A = AES(KEY,PINBLOCK)
    if(keyType == AES_128)
        api_aes_encipher(pinblock, blk_a, key, 16);
    else if(keyType == AES_192)
        api_aes_encipher(pinblock, blk_a, key, 24);
    else if(keyType == AES_256)
        api_aes_encipher(pinblock, blk_a, key, 32);

    // generate BLOCK_B by modulo-2 addition of BLOCK_A & PAN
    for(i = 0 ; i < 16 ; i++)
       blk_b[i] = blk_a[i] ^ panblock[i];

    // generate enciphered EPB = AES(KEY,BLOCK_B)
    if(keyType == AES_128)
        api_aes_encipher(blk_b, epb, key, 16);
    else if(keyType == AES_192)
        api_aes_encipher(blk_b, epb, key, 24);
    else if(keyType == AES_256)
        api_aes_encipher(blk_b, epb, key, 32);

    // clear sensitive data
    memset(pinblock, 0x00, sizeof(pinblock));
    memset(panblock, 0x00, sizeof(panblock));
    memset(random, 0x00, sizeof(random));
    memset(blk_a, 0x00, sizeof(blk_a));
    memset(blk_b, 0x00, sizeof(blk_b));
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate plaintext PIN block by using ISO 9564 format 2.
// INPUT   : none.
// OUTPUT  : pb     - plaintext pin block (fixed 8 bytes).
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UINT8	PED_GenPinBlock( UINT8 *pb )
{
UINT8	pin[PED_PIN_SLOT_LEN];


	// restore PIN data
	// OS_SECM_GetData( ADDR_PED_PIN, PED_PIN_SLOT_LEN, pin ); // L-V
    memcpy(pin, ped_pin_ptr, PED_PIN_SLOT_LEN); // L-V
	
	// generate PIN block by using ISO 9564 format 2
	PED_GenPinBlock_ISO2( pin, pb );
	
	memset( pin, 0x00, sizeof(pin) );

	// OS_SECM_ClearData( ADDR_PED_PIN, PED_PIN_SLOT_LEN, 0 );	// clear PIN data
    if(ped_pin_ptr != NULL)
        PED_ClearPin();  // deallocate PIN data memory
	
	return( apiOK );
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate encrypted PIN block by using Master/Session key algorithm.
// INPUT   : mode   - algorithm of PIN block.
//		      0: ISO 9564 Format 0 (ANSI X9.8)
//		      1: ISO 9564 Format 1
//		      2: ISO 9564 Format 2
//		      3: ISO 9564 Format 3
// 	         index  - session key index used to encrypt the PIN block.
//	         pan    - full PAN digits or transaction field for ISO3. (format: L-V)
// OUTPUT  : epb    - encrypted pin block (fixed 8 bytes).
//	         sn     - randomly selected sequence number used only for ISO3. (1 byte) - RFU
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
#if 0
UINT8	PED_MSKEY_GenPinBlock( UINT8 mode, UINT8 index, UINT8 *pan, UINT8 *epb )
{
UINT8	pin[PED_PIN_SLOT_LEN];
UINT8	keydata[KEY_DATA_LEN+1]; // L-V
UINT8	pinblock[8];
UINT8	eskey[PED_MSKEY_LEN+1];	// L-V
UINT8	skey[PED_MSKEY_LEN+1];	// L-V
UINT8	mkey[PED_MSKEY_LEN+1];	// L-V
PED_KEY_BUNDLE	KeyBundle;
UINT8	MkeyIndex;
UINT8	result;
UINT8	mac8[8];

UINT8	temp[KEY_DATA_LEN + 1];
UINT8	buf[KEY_BUNDLE_LEN];
	

	memset( epb, 0x00, 8 );
	result = apiOK;

	// check SKEY index
	if( index < MAX_SKEY_CNT )
	  {
	  // restore PIN data
	  OS_SECM_GetData( ADDR_PED_PIN, PED_PIN_SLOT_LEN, pin ); // L-V
	  if( pin[0] == 0 )
	    return( apiFailed ); // PIN not available

	  PED_InUse( TRUE );
	
	  // generate plaintext PIN Block
	  switch( mode )
	        {
	        case EPB_ISO0:

				 if(pan[0] != 0)
					 PED_GenPinBlock_ISO0(pin, pan, pinblock);
	             else
	               result = apiFailed;
	               
	      	     break;
	      	   
	        case EPB_ISO1:
	      
	      	     PED_GenPinBlock_ISO1( pin, pinblock );
	             break;
	           
	        case EPB_ISO2:
	      
	      	     PED_GenPinBlock_ISO2( pin, pinblock );
	             break;
	           
	        case EPB_ISO3:
			
				 if(pan[0] != 0)
					 PED_GenPinBlock_ISO3(pin, pan, pinblock);
	      	     else
	      	       result = apiFailed;
	      	       
	             break;
	             
	        default:
	             result = apiFailed;
	             break;
	        }

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

	  // generate EPB (Encrypted PIN Block) by using SKEY
	  if( result == apiOK )
	    {
	     result = apiFailed;
	     
	    // get ESKEY bundle
	    OS_SECM_GetData( ADDR_PED_SKEY_01+(index*PED_MSKEY_SLOT_LEN), PED_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle );
	    
	    // verify ESKEY bundle
	    if( TR31_VerifyKeyBundle( PED_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle, keydata, mac8, mkey ) )
	      {
	      // check ESKEY key usage
	      if( (KeyBundle.Usage[0] == 'P') && (KeyBundle.Usage[1] == '0') ) // for PIN encryption?
	        {
	        // retrieve ESKEY key
	        if( TR31_DecryptKeyBundle( mac8, keydata, eskey, mkey ) )
	          {
//	          LIB_DumpHexData( 0, 0, 17, eskey );
		  
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
	              if( TR31_DecryptKeyBundle( mac8, keydata, mkey, (UINT8 *)0 ) )
	                {
//	                LIB_DumpHexData( 0, 0, 17, mkey );

		        // retrieve SKEY from MKEY
//		        PED_TripleDES2( &mkey[1], eskey[0], &eskey[1], skey );
//		        LIB_DumpHexData( 0, 3, 16, skey );
		      
		        // generate encrypted PIN block
	                PED_TripleDES( skey, 8, pinblock, epb );
	                result = apiOK;
		        }
		      }
		    }		  
	          }
		}
	      }
	    }
	  }
	else
	  result = apiFailed;

EXIT:
	
	memset( mkey, 0x00, sizeof(mkey) );				// clear KEY data
	memset( skey, 0x00, sizeof(skey) );				// clear KEY data
	memset( pin, 0x00, sizeof(pin) );				// clear PIN buffer
	memset( pinblock, 0x00, sizeof(pinblock) );  			// clear PIN block
	memset( keydata, 0x00, sizeof(keydata) );			// PATCH: 2009-04-07
	memset( eskey, 0x00, sizeof(eskey) );				//

	memset(temp, 0x00, sizeof(temp));
	memset(buf, 0x00, sizeof(buf));

	OS_SECM_ClearData( ADDR_PED_PIN, PED_PIN_SLOT_LEN, 0 );		// clear PIN data
	
	PED_InUse( FALSE );
	
	return( result );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To generate encrypted PIN block by using Master/Session key algorithm.
// INPUT   : mode   - algorithm of PIN block.
//		      0: ISO 9564 Format 0 (ANSI X9.8)
//		      1: ISO 9564 Format 1
//		      2: ISO 9564 Format 2
//		      3: ISO 9564 Format 3
// 	         index  - session key index used to encrypt the PIN block.
//	         pan    - full PAN digits or transaction field for ISO3. (format: L-V)
// OUTPUT  : epb    - encrypted pin block (fixed 8 bytes).
//	         sn     - randomly selected sequence number used only for ISO3. (1 byte) - RFU
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UINT8	PED_MSKEY_GenPinBlock( UINT8 mode, UINT8 index, UINT8 *pan, UINT8 *epb )
{
    UINT8   pin[PED_PIN_SLOT_LEN];
    UINT8   keydata[X9143_TDES_KEY_DATA_LEN + 1];    // L-V
    UINT8   pinblock[8];
    UINT8	pkey[PED_PEK_MSKEY_LEN + 1];    // L-V
    UINT8	skey[PED_PEK_MSKEY_LEN];
    UINT8	mkey[PED_PEK_MSKEY_LEN + 1];    // L-V
    X9143_TDES_PED_KEY_BUNDLE   KeyBundle;
    UINT8   MkeyIndex;
    UINT8   result;
    UINT8   mac8[8];
    UINT8   keyType;

    memset(epb, 0x00, 8);
    result = apiOK;

    // check PEK SKEY index
    if(index < MAX_PEK_SKEY_CNT)
    {
        // restore PIN data
        // OS_SECM_GetData(ADDR_PED_PIN, PED_PIN_SLOT_LEN, pin); // L-V
        memcpy(pin, ped_pin_ptr, PED_PIN_SLOT_LEN); // L-V
        if(pin[0] == 0)
            return apiFailed; // PIN not available

        PED_InUse(TRUE);

        // generate plaintext PIN Block
        switch(mode)
        {
            case EPB_ISO0:

                if(pan[0] != 0)
                    PED_GenPinBlock_ISO0(pin, pan, pinblock);
                else
                    result = apiFailed;

                break;

            case EPB_ISO1:

                PED_GenPinBlock_ISO1(pin, pinblock);
                break;

#if 0
            case EPB_ISO2:

                PED_GenPinBlock_ISO2(pin, pinblock);
                break;
#endif

            case EPB_ISO3:

                if(pan[0] != 0)
                    PED_GenPinBlock_ISO3(pin, pan, pinblock);
                else
                    result = apiFailed;

                break;

            default:
                result = apiFailed;
                break;
        }

        PED_PEK_GetKeyType(&keyType);
        PED_SetKPKStatus(keyType, 1);

        // get PEK MKEY bundle
        PED_PEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
        OS_SECM_GetData(ADDR_PED_PEK_TDES_MKEY_01 + (MkeyIndex * PED_PEK_TDES_MSKEY_SLOT_LEN), PED_PEK_TDES_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle);

        // verify PEK MKEY bundle
        if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, (UINT8 *)&KeyBundle, keydata, mac8, (UINT8 *)0 ))
        {
            // check MKEY key usage
            if((KeyBundle.Usage[0] == 'K') && (KeyBundle.Usage[1] == '0')) // for KEY encryption?
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
            OS_SECM_GetData(ADDR_PED_PEK_TDES_SKEY_01 + (index * PED_PEK_TDES_MSKEY_SLOT_LEN), PED_PEK_TDES_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle);
                                            
            // verify PEK SKEY bundle
            if(X9143_VerifyKeyBundle_TDES(X9143_TDES_KEY_BUNDLE_LEN, (UINT8 *)&KeyBundle, keydata, mac8, mkey))
            {
                // check PEK SKEY key usage
                if((KeyBundle.Usage[0] == 'P') && (KeyBundle.Usage[1] == '0')) // for PIN encryption?
                {
                    // retrieve PEK SKEY
                    if(X9143_DecryptKeyBundle_TDES(mac8, keydata, pkey, mkey)) // pkey=PEK SKEY
                    {
                        // ==== [Debug] ====
                        printf("pkey = ");
                        for(int i = 0 ; i < pkey[0] + 1 ; i++)
                            printf("%02x", pkey[i]);
                        printf("\n");
                        printf("pinblock = ");
                        for(int i = 0 ; i < 8 ; i++)
                            printf("%02x", pinblock[i]);
                        printf("\n");
                        // ==== [Debug] ====

                        memmove(skey, &pkey[1], pkey[0]);

                        // generate encrypted PIN block
                        if(keyType == TDES_128)
                            PED_TripleDES(skey, 8, pinblock, epb);
                        else
                            api_3des_encipher(pinblock, epb, skey);

                        result = apiOK;
                    }
                }
            }
        }

        // reset flag
        PED_SetKPKStatus(keyType, 0);
    }
    else
      result = apiFailed;

EXIT:

    memset(pkey, 0x00, sizeof(pkey));         // clear KEY data
    memset(mkey, 0x00, sizeof(mkey));         // clear KEY data
    memset(skey, 0x00, sizeof(skey));         // clear KEY data
    memset(pin, 0x00, sizeof(pin));           // clear PIN buffer
    memset(pinblock, 0x00, sizeof(pinblock)); // clear PIN block
    memset(keydata, 0x00, sizeof(keydata));   // clear KEY data

    // OS_SECM_ClearData(ADDR_PED_PIN, PED_PIN_SLOT_LEN, 0); // clear PIN data
    if(ped_pin_ptr != NULL)
        PED_ClearPin();  // deallocate PIN data memory

    PED_InUse(FALSE);

    return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate encrypted PIN block by using DUKPT key algorithm.
// INPUT   : mode   - algorithm of PIN block.
//		      0: ISO 9564 Format 0 (ANSI X9.8)
//		      1: ISO 9564 Format 1
//		      2: ISO 9564 Format 2
//		      3: ISO 9564 Format 3
//	         pan    - PAN digits. (L-V)
// OUTPUT  : epb    - encrypted pin block (fixed 8 bytes).
//	         ksn    - key serial number.  (fixed 10 bytes).
// RETURN  : apiOK
//           apiFailed
//	         apiDeviceError	// PED inoperative due to over 1M use limit.
// ---------------------------------------------------------------------------
UINT8	PED_DUKPT_GenPinBlock( UINT8 mode, UINT8 *pan, UINT8 *epb, UINT8 *ksn )
{
UINT8	pin[PED_PIN_SLOT_LEN];
UINT8	pinblock[8];
UINT8	result;


	memset( epb, 0x00, 8 );
	memset( ksn, 0x00, 10 );
	result = apiOK;
	
	// restore PIN data
	// OS_SECM_GetData( ADDR_PED_PIN, PED_PIN_SLOT_LEN, pin ); // L-V
    memcpy(pin, ped_pin_ptr, PED_PIN_SLOT_LEN); // L-V
	if( pin[0] == 0 )
	    return( apiFailed ); // PIN not available
	
	PED_InUse( TRUE );
	
	// generate plaintext PIN Block
	switch( mode )
	      {
	      case EPB_ISO0:
	      
			   if(pan[0] != 0)
				   PED_GenPinBlock_ISO0(pin, pan, pinblock);
	           else
	             result = apiFailed;
	             
	  	   break;
	      	   
	      case EPB_ISO1:
	      
	           PED_GenPinBlock_ISO1( pin, pinblock );
	           break;

#if 0	           
	      case EPB_ISO2:
	      
	           PED_GenPinBlock_ISO2( pin, pinblock );
	           break;
#endif

	      case EPB_ISO3:
	      
			   if(pan[0] != 0)
				   PED_GenPinBlock_ISO3(pin, pan, pinblock);
	      	   else
	      	     result = apiFailed;
	      	     
	           break;
	             
	      default:
	           result = apiFailed;
	           break;
	      }

	// generate EPB & KSN by using DUKPT key
	if( result == apiOK )
	  {
	  if( DUKPT_RequestPinEntry( pinblock, ksn, epb ) != TRUE )
//	    result = apiFailed;
	    result = apiDeviceError;	// 2010-04-30
	  }

	memset( pin, 0x00, sizeof(pin) );			// clear PIN buffer
	memset( pinblock, 0x00, sizeof(pinblock) );		// clear PIN block

	// OS_SECM_ClearData( ADDR_PED_PIN, PED_PIN_SLOT_LEN, 0 );	// clear PIN data
    if(ped_pin_ptr != NULL)
        PED_ClearPin();  // deallocate PIN data memory
	
	PED_InUse( FALSE );
	
	return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate encrypted PIN block by using Fixed key algorithm.
// INPUT   : mode   - algorithm of PIN block.
//		      0: ISO 9564 Format 0 (ANSI X9.8)
//		      1: ISO 9564 Format 1
//		      2: ISO 9564 Format 2
//		      3: ISO 9564 Format 3
// 	     index  - fixed key index.
//	     pan    - PAN. (format: L-V)
// OUTPUT  : epb    - encrypted pin block (8 bytes).
//	     sn     - randomly selected sequence number used only for ISO3. (1 byte) - RFU
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
#if	0
UINT8	PED_FXKEY_GenPinBlock( UINT8 mode, UINT8 index, UINT8 *pan, UINT8 *epb )
{
#ifdef	_USE_IRAM_PARA_
// 2014-12-12, allocate sensitive data buffer on MCU internal SRAM
#define	PIN_SIZE_FXGPB		PED_PIN_SLOT_LEN
#define	PIN_ADDR_FXGPB		0x0000F000	// pointer to MCU internal SRAM
#define	KEYDATA_SIZE_FXGPB	KEY_DATA_LEN+1
#define	KEYDATA_ADDR_FXGPB	PIN_ADDR_FXGPB+PIN_SIZE_FXGPB
#define	PINBLOCK_SIZE_FXGPB	8
#define	PINBLOCK_ADDR_FXGPB	KEYDATA_ADDR_FXGPB+KEYDATA_SIZE_FXGPB
#define	FKEY_SIZE_FXGPB		PED_FKEY_LEN+1
#define	FKEY_ADDR_FXGPB		PINBLOCK_ADDR_FXGPB+PINBLOCK_SIZE_FXGPB

UINT8	*pin = (UINT8 *)PIN_ADDR_FXGPB;
UINT8	*keydata = (UINT8 *)KEYDATA_ADDR_FXGPB;
UINT8	*pinblock = (UINT8 *)PINBLOCK_ADDR_FXGPB;
UINT8	*fkey = (UINT8 *)FKEY_ADDR_FXGPB;
#else
UINT8	pin[PED_PIN_SLOT_LEN];
UINT8	keydata[KEY_DATA_LEN+1]; // L-V
UINT8	pinblock[8];
UINT8	fkey[PED_FKEY_LEN+1]; // L-V
#endif

PED_KEY_BUNDLE	KeyBundle;

UINT8	result;
UINT8	mac8[8];


	memset( epb, 0x00, 8 );
	result = apiOK;

	// check FKEY index
	if( index < MAX_FKEY_CNT )
	  {
	  // restore PIN data
	  OS_SECM_GetData( ADDR_PED_PIN, PED_PIN_SLOT_LEN, pin ); // L-V
	  if( pin[0] == 0 )
	    return( apiFailed ); // PIN not available
	  
	  PED_InUse( TRUE );
	  
	  // generate plaintext PIN Block
	  switch( mode )
	        {
	        case EPB_ISO0:
	      
	      	     if( pan[0] != 0 )
	               PED_GenPinBlock_ISO0( pin, pan, pinblock );
	             else
	               result = apiFailed;
	               
	      	     break;
	      	   
	        case EPB_ISO1:
	      
	      	     PED_GenPinBlock_ISO1( pin, pinblock );
	             break;
	           
	        case EPB_ISO2:
	      
	      	     PED_GenPinBlock_ISO2( pin, pinblock );
	             break;
	           
	        case EPB_ISO3:
	      
	     	     if( pan[0] != 0 )
	      	       PED_GenPinBlock_ISO3( pin, pan, pinblock );
	      	     else
	      	       result = apiFailed;
	      	       
	             break;
	             
	        default:
	             result = apiFailed;
	             break;
	        }
	        
	  // generate EPB (Encrypted PIN Block) by using FKEY
	  if( result == apiOK )
	    {
	    result = apiFailed;
	    
	    // get FIXED key bundle
	    OS_SECM_GetData( ADDR_PED_FKEY_01+(index*PED_FKEY_SLOT_LEN), PED_FKEY_SLOT_LEN, (UINT8 *)&KeyBundle );
	    
	    // verify FIXED key bundle
	    if( TR31_VerifyKeyBundle( PED_FKEY_SLOT_LEN, (UINT8 *)&KeyBundle, keydata, mac8 ) )
	      {
	      // check key usage
	      if( (KeyBundle.Usage[0] == 'P') && (KeyBundle.Usage[1] == '0') ) // for PIN encryption?
	        {
	        // retrieve FIXED key
	        if( TR31_DecryptKeyBundle( mac8, keydata, fkey ) )
	          {
	          // generate encrypted PIN block
	          PED_TripleDES( &fkey[1], 8, pinblock, epb );  
	          result = apiOK;
	          }
		}
	      }
	    }
	  }
	else
	  result = apiFailed;

#ifdef	_USE_IRAM_PARA_  
	memset( fkey, 0x00, sizeof(UINT8)*FKEY_SIZE_FXGPB );
	memset( pin, 0x00, sizeof(UINT8)*PIN_SIZE_FXGPB );
	memset( pinblock, 0x00, sizeof(UINT8)*PINBLOCK_SIZE_FXGPB );
	memset( keydata, 0x00, sizeof(UINT8)*KEYDATA_SIZE_FXGPB );
#else
	memset( fkey, 0x00, sizeof(fkey) );				// clear KEY data	
	memset( pin, 0x00, sizeof(pin) );				// clear PIN buffer
	memset( pinblock, 0x00, sizeof(pinblock) );			// clear PIN block
	memset( keydata, 0x00, sizeof(keydata) );			// PATCH: 2009-04-07
#endif

	memset( (UINT8 *)&KeyBundle, 0x00, sizeof(KeyBundle) );		//
	OS_SECM_ClearData( ADDR_PED_PIN, PED_PIN_SLOT_LEN, 0 );		// clear PIN data
	
	PED_InUse( FALSE );
	
	return( result );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To generate encrypted PIN block by using AES key algorithm.
// INPUT   : mode   - algorithm of PIN block. (fixed 0x04)
//		      4: ISO 9564 Format 4
// 	         index  - key index. (fixed 0x00)
//	         pan    - PAN. (format: L-V)
// OUTPUT  : epb    - the enciphered PIN block. (16 bytes: 128 bits)
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
#if 0
UINT8	PED_AESKEY_GenPinBlock( UINT8 mode, UINT8 index, UINT8 *pan, UINT8 *epb )
{
UINT8	pin[PED_PIN_SLOT_LEN];
UINT8	pinblock[16];
UINT8	result = apiFailed;
PED_KEY_BUNDLE2	KeyBundle;
UINT8	keydata[KEY_DATA_LEN2 + 1]; // L-V
UINT8	eskey[PED_ISO4_KEY_MSKEY_LEN + 1];	// L-V
UINT8	skey[PED_ISO4_KEY_LEN];
UINT8	mkey[PED_ISO4_KEY_MSKEY_LEN + 1];	// L-V
UINT8	mac16[16];
UINT8	MkeyIndex;


	memset( epb, 0x00, 16 );

	// check format
	if( (mode != EPB_ISO4) || (index != 0) )
	  return( apiFailed );

	PED_InUse( TRUE );

	// get ISO4_KEY bundle
	OS_SECM_GetData(ADDR_PED_ISO4_KEY + (index * PED_ISO4_KEY_SLOT_LEN), PED_ISO4_KEY_SLOT_LEN, (UINT8 *)&KeyBundle);

	// verify ISO4_KEY bundle
	if(TR31_VerifyKeyBundle_AES(PED_ISO4_KEY_SLOT_LEN, (UINT8 *)&KeyBundle, keydata, mac16))
	{
		// check ISO4_KEY key usage
		if((KeyBundle.Usage[0] == 'P') && (KeyBundle.Usage[1] == '0')) // for PIN encryption?
		{
			// retrieve ISO4_KEY
			if(TR31_DecryptKeyBundle_AES(mac16, keydata, eskey))
			{
				memmove(skey, &eskey[1], eskey[0]);

				// restore PIN data
				OS_SECM_GetData(ADDR_PED_PIN, PED_PIN_SLOT_LEN, pin); // L-V
				if(pin[0] != 0)
				{
					// generate EPB (Encrypted PIN Block) by using ISO4_KEY
					PED_GenEncPinBlock_ISO4(pin, pan, skey, epb);
				}

				result = apiOK;
			}

			/*
			// retrieve ISO4_KEY ESKEY key (SKEY encrypted by MKEY)
			if(TR31_DecryptKeyBundle_AES(mac16, keydata, eskey))
			{
				// get ISO4_KEY MKEY bundle
				PED_ISO4_ReadMKeyIndex((UINT8 *)&MkeyIndex);
				OS_SECM_GetData(ADDR_PED_ISO4_KEY_MKEY_01 + (MkeyIndex * PED_ISO4_KEY_MSKEY_SLOT_LEN), PED_ISO4_KEY_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle);

				// verify ISO4_KEY MKEY bundle
				if(TR31_VerifyKeyBundle_AES(KEY_BUNDLE_LEN2, (UINT8 *)&KeyBundle, keydata, mac16))
				{
					// check ISO4_KEY MKEY key usage
					if((KeyBundle.Usage[0] == 'K') && (KeyBundle.Usage[1] == '0')) // for KEY encryption?
					{
						// retrieve ISO4_KEY MKEY
						if(TR31_DecryptKeyBundle_AES(mac16, keydata, mkey))	// mkey=ISO4_KEY MKEY
						{
							// retrieve ISO4_KEY SKEY from ISO4_KEY MKEY
							api_aes_decipher(skey, &eskey[1], &mkey[1], eskey[0]);

							// restore PIN data
							OS_SECM_GetData(ADDR_PED_PIN, PED_PIN_SLOT_LEN, pin); // L-V
							if(pin[0] != 0)
							{
								// generate EPB (Encrypted PIN Block) by using ISO4_KEY
								PED_GenEncPinBlock_ISO4(pin, pan, skey, epb);
							}
							
							result = apiOK;
						}
					}
				}
			}
			*/
		}
	}
	
	memset( pin, 0x00, sizeof(pin) );				// clear PIN buffer
	memset( pinblock, 0x00, sizeof(pinblock) );			// clear PIN block
	memset(keydata, 0x00, sizeof(keydata));
	memset(eskey, 0x00, sizeof(eskey));
	memset(skey, 0x00, sizeof(skey));
	memset(mkey, 0x00, sizeof(mkey));

	OS_SECM_ClearData( ADDR_PED_PIN, PED_PIN_SLOT_LEN, 0 );		// clear PIN data
	
	PED_InUse( FALSE );
	
	return( result );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To generate encrypted PIN block by using AES key algorithm.
// INPUT   : mode   - algorithm of PIN block. (fixed 0x04)
//		              4: ISO 9564 Format 4
// 	         index  - key index. (fixed 0x00)
//	         pan    - PAN. (format: L-V)
// OUTPUT  : epb    - the enciphered PIN block. (16 bytes: 128 bits)
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UINT8	PED_AESKEY_GenPinBlock( UINT8 mode, UINT8 index, UINT8 *pan, UINT8 *epb )
{
    UINT8   pin[PED_PIN_SLOT_LEN];
    UINT8   pinblock[16];
    UINT8   result = apiOK;
    X9143_AES_PED_KEY_BUNDLE   KeyBundle;
    UINT8   keydata[X9143_AES_KEY_DATA_LEN + 1];        // L-V
    UINT8	pkey[PED_PEK_MSKEY_LEN + 1];    // L-V
    UINT8   skey[PED_PEK_MSKEY_LEN];
    UINT8   mkey[PED_PEK_MSKEY_LEN + 1]; // L-V
    UINT8   mac16[16];
    UINT8   MkeyIndex;
    UINT8   keyType;

    memset(epb, 0x00, 16);

    // check format
    if((mode != EPB_ISO4) || (index != 0))
        return apiFailed;

    PED_InUse(TRUE);

    // check PEK SKEY index
    if(index < MAX_PEK_SKEY_CNT)
    {
        PED_PEK_GetKeyType(&keyType);
        PED_SetKPKStatus(keyType, 1);

        // get PEK MKEY bundle
        PED_PEK_ReadMKeyIndex((UINT8 *)&MkeyIndex);
        OS_SECM_GetData(ADDR_PED_PEK_AES_MKEY_01 + (MkeyIndex * PED_PEK_AES_MSKEY_SLOT_LEN), PED_PEK_AES_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle);

        // verify PEK MKEY bundle
        if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, (UINT8 *)&KeyBundle, keydata, mac16, (UINT8 *)0 ))
        {
            // check MKEY key usage
            if((KeyBundle.Usage[0] == 'K') && (KeyBundle.Usage[1] == '0')) // for KEY encryption?
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
            OS_SECM_GetData(ADDR_PED_PEK_AES_SKEY_01 + (index * PED_PEK_AES_MSKEY_SLOT_LEN), PED_PEK_AES_MSKEY_SLOT_LEN, (UINT8 *)&KeyBundle);
                                            
            // verify PEK SKEY bundle
            if(X9143_VerifyKeyBundle_AES(X9143_AES_KEY_BUNDLE_LEN, (UINT8 *)&KeyBundle, keydata, mac16, mkey))
            {
                // check PEK SKEY key usage
                if((KeyBundle.Usage[0] == 'P') && (KeyBundle.Usage[1] == '0')) // for PIN encryption?
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

                        // restore PIN data
                        // OS_SECM_GetData(ADDR_PED_PIN, PED_PIN_SLOT_LEN, pin); // L-V
                        memcpy(pin, ped_pin_ptr, PED_PIN_SLOT_LEN); // L-V
                        if(pin[0] != 0)
                        {
                            // generate EPB (Encrypted PIN Block) by using ISO4_KEY
                            PED_GenEncPinBlock_ISO4(pin, pan, skey, epb);
                        }

                        result = apiOK;
                    }
                }
            }
        }

        // reset flag
        PED_SetKPKStatus(keyType, 0);
    }
    else
      result = apiFailed;

EXIT:
    memset(pin, 0x00, sizeof(pin));           // clear PIN buffer
    memset(pinblock, 0x00, sizeof(pinblock)); // clear PIN block
    memset(keydata, 0x00, sizeof(keydata));
    memset(pkey, 0x00, sizeof(pkey));
    memset(skey, 0x00, sizeof(skey));
    memset(mkey, 0x00, sizeof(mkey));

    // OS_SECM_ClearData(ADDR_PED_PIN, PED_PIN_SLOT_LEN, 0); // clear PIN data
    if(ped_pin_ptr != NULL)
        PED_ClearPin();  // deallocate PIN data memory

    PED_InUse(FALSE);

    return result;
}
