//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : 							    **
//**  PRODUCT  : AS330							    **
//**                                                                        **
//**  FILE     : DUKPT-3D.C (cf. ANS X9.24-2004 & ISO 9564-1:2002)          **
//**  MODULE   :                                                            **
//**                                                                        **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2018/07/05                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2005-2018 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include <string.h>

//#include "bsp_types.h"
//#include "bsp_cipher.h"
#include "POSAPI.h"
#include "OS_SECM.h"

//#include "EMVDC.h"
//#include "GDATAEX.h"
//#include "EMVAPI.h"
//#include "TOOLS.h"

//#define	UCHAR	UINT8
//#define	UINT	UINT16
//#define	ULONG	UINT32

// ---------------------------------------------------------------------------
// FUNCTION: Left shift the string N bits.
// INPUT   : old - the original string.
//           len - length of string.
//           cnt - number of bits to be shifted.
// OUTPUT  : new - the result string.
// REF     : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void  left_shift_str( UCHAR *new, UCHAR *old, UCHAR len, UCHAR cnt )
{
UCHAR i, j;

      memmove( new, old, len );

      for( i=0; i<cnt; i++ )
         {
         for( j=0; j<len; j++ )
            {
            new[j] <<= 1;  // leftshift N'th byte one bit

            if( (j+1) < len )
              {
              if( new[j+1] & 0x80 )  // shfit the (N-1)'th carry to the N'th lsb
                new[j] |= 0x01;
              }
            else
              break;
            }
         }
}

// ---------------------------------------------------------------------------
// FUNCTION: Right shift the string N bits.
// INPUT   : old - the original string.
//           len - length of string.
//           cnt - number of bits to be shifted.
// OUTPUT  : new - the result string.
// REF     : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void  right_shift_str( UCHAR *new, UCHAR *old, UCHAR len, UCHAR cnt )
{
UCHAR i, j;

      memmove( new, old, len );

      for( i=0; i<cnt; i++ )
         {
         for( j=0; j<len; j++ )
            {
            new[len-j-1] >>= 1;  // rightshift N'th byte one bit

            if( len-j-1 )
              {
              if( new[len-j-2] & 0x01 )  // shfit the (N-1)'th carry to the N'th msb
                new[len-j-1] |= 0x80;
              }
            else
              break;
            }
         }
}

// ---------------------------------------------------------------------------
// FUNCTION: Generate the initially loaded PED Key.
// INPUT   : ksn  - key serial number.
//           dkey - derivation key.
// OUTPUT  : ikey - the initial key, which will be loaded to terminal.
// RETURN  : none.
// NOTE    : this function is used by issuer only.
// ---------------------------------------------------------------------------
/*
void  DUKPT_GenInitKey( UCHAR *ksn, UCHAR *dkey, UCHAR *ikey )
{
UCHAR buf1[10];
UCHAR buf2[8];
UCHAR key[16];

      memmove( buf1, ksn, 10 );
      memmove( key, dkey, 16 );

      // set the 21 lsb of this 10-byte register to zero
      buf1[7] &= 0xE0;
      buf1[8] = 0x00;
      buf1[9] = 0x00;

      // take the 8 MSB of this 10-byte register, and ENC/DEC/ENC these
      // 8 bytes using the double-length derivation key, per the TECB mode
      api_des_encipher( buf1, buf2, &key[0] );
      api_des_decipher( buf1, buf2, &key[8] );
      api_des_encipher( buf1, buf2, &key[0] );

      memmove( ikey, buf2, 8 ); // left half of the initial key

      key[0] ^= 0xC0;
      key[1] ^= 0xC0;
      key[2] ^= 0xC0;
      key[3] ^= 0xC0;
      key[8] ^= 0xC0;
      key[9] ^= 0xC0;
      key[10] ^= 0xC0;
      key[11] ^= 0xC0;

      memmove( buf1, ksn, 10 );

      // set the 21 lsb of this 10-byte register to zero
      buf1[7] &= 0xE0;
      buf1[8] = 0x00;
      buf1[9] = 0x00;

      // take the 8 MSB of this 10-byte register, and ENC/DEC/ENC these
      // 8 bytes using the double-length derivation key, per the TECB mode
      api_des_encipher( buf1, buf2, &key[0] );
      api_des_decipher( buf1, buf2, &key[8] );
      api_des_encipher( buf1, buf2, &key[0] );

      memmove( &ikey[8], buf2, 8 ); // right half of the initial key
}
*/

// ---------------------------------------------------------------------------
// FUNCTION: Generate Message Authentication Code.
// INPUT   : icv  - initial chain value. (8-byte)
//                  let icv = 0's if using CBC mode.
//           len  - length of data.
//           data - data to be authenticated.
// OUTPUT  : mac  - the MAC. (8-byte)
//           icv  - next icv values.
// RETURN  : none.
// NOTE    : The "DUKPT_RequestPinEntry()" shall be called prior to calling 
//           this function, the MAC key is updated over there.
// ---------------------------------------------------------------------------
//void	DUKPT_GenMAC( UCHAR *icv, UINT len, UCHAR *data, UCHAR *mac )
//{
//UINT  i, j;
//UINT  cnt1;
//UCHAR ctext[8];
//UCHAR ptext[8];
//UCHAR left_bytes;
//UCHAR buf[8];
//UCHAR deskey[16];
//
//      
//      OS_SECM_GetData( ADDR_MAC_KEY_REG, FUTURE_KEY_LEN, deskey );	// retrieve MAC key reg as DESkey
//
//      cnt1 = len / 8;
//      if( cnt1 == 0 )
//        {
//        i = 0;
//        j = 0;
//        left_bytes = len;
//        goto mac2;
//        }
//
//      if( len % 8 )
//        left_bytes = len - cnt1*8;
//      else
//        left_bytes = 0;
//
//      for( i=0; i<cnt1; i++ )
//         {
//         // XOR( ICV, D1 )
//         for( j=0; j<8; j++ )
//            ptext[j] = icv[j] ^ data[i*8+j];
//
//         // DES( ptext, key(L) )
//         api_des_encipher( ptext, ctext, deskey );
//         memmove( icv, ctext, 8 );
//         }
//         
//mac2:
//      if( left_bytes )
//        {
//        memset( buf, 0x00, 8 );
//        memmove( buf, &data[i*8], left_bytes );
//
//      // XOR( ICV, D1 )
//        for( j=0; j<8; j++ )
//           ptext[j] = icv[j] ^ buf[j];
//
//        // DES( ptext, key(L) )
//        api_des_encipher( ptext, ctext, deskey );
//
//        // -DES( ctext, key(R) )
//        api_des_decipher( ptext, ctext, &deskey[8] );
//
//        // DES( ptext, key(L) )
//        api_des_encipher( ptext, ctext, deskey );
//        }
//
//      memmove( mac, ctext, 8 );
//}

// ---------------------------------------------------------------------------
// FUNCTION: construct a PIN block.
// INPUT   : format   - PIN block format = PIN xor PAN
//                      plaintext PIN field
//                          C N P P P P P/F P/F ... F (16-nibble)
//                      PAN field
//                          0 0 0 0 A1 A2 ....... A12 (16-nibble)
//           pindata  - L-V, the PIN digits. (L=0 for PIN bypassed)
//           pandata  - V  , the PAN digits. (12-digit bytes)
// OUTPUT  : pinblock - the plaintext pin in ASCII.
// RETURN  : none.
// ---------------------------------------------------------------------------
void  DUKPT_GeneratePINblock( UCHAR *pindata, UCHAR *pandata, UCHAR *pinblock )
{
UINT  i, j;
UCHAR nibble_h;
UCHAR nibble_l;
UCHAR panblock[16];

      for( i=0; i<16; i++ )
         pinblock[i]=0x0f; // filler

      pinblock[0] = 0x00; // Control field
      pinblock[1] = *pindata++; // length of pin digits

      if( pinblock[1] != 0 ) // PIN available?
        {
        // convert ASCII[2] to BCD[2] for PIN
        for( i=0; i<pinblock[1]; i++ )
           pinblock[i+2] = (*pindata++) & 0x0f;
        }

      // convert ASCII[2] to BCD[2] for PAN
      panblock[0] = 0x00;
      panblock[1] = 0x00;
      panblock[2] = 0x00;
      panblock[3] = 0x00;
      for( i=4; i<16; i++ )
         panblock[i] = (*pandata++) & 0x0f;

      // generate PIN block by modulo-2 addition of PIN & PAN
      for( i=0; i<16; i++ )
         pinblock[i] ^= panblock[i];

      // convert BCD[2] to BCD[1]
      j = 0;
      for( i=0; i<8; i++ )
         {
         nibble_h = pinblock[j] << 4;
         j++;
         nibble_l = pinblock[j];
         j++;
         pinblock[i] = nibble_h | nibble_l;
         }
}

// ---------------------------------------------------------------------------
// FUNCTION: Triple DES for DUKPT.
// INPUT   : Key_Reg     - key register. (16-byte)
// OUTPUT  : Crypto_Reg1 - the crypto register 1. (8-byte)
// RETURN  : none.
// ---------------------------------------------------------------------------
void  DUKPT_TripleDEA( UCHAR *Key_Reg, UCHAR *Crypto_Reg1 )
{
UCHAR deskey[16];
UCHAR data[8];

      memmove( deskey, Key_Reg, 16 );
      memmove( data, Crypto_Reg1, 8 );

      // Crypto Register-1 DEA-encrypted using the left half of the Key Register
      // as the key goes to Crypto Register-1

      PED_des_encipher( data, data, &deskey[0] );

      // Crypto Register-1 DEA-decrypted using the right half of the Key Register
      // as the key goes to Crypto Register-1

      PED_des_decipher( data, data, &deskey[8] );

      // Crypto Register-1 DEA-encrypted using the left half of the Key Register
      // as the key goes to Crypto Register-1

      PED_des_encipher( data, data, &deskey[0] );
      memmove( Crypto_Reg1, data, 8 );
}

// ---------------------------------------------------------------------------
// FUNCTION: Special encryption function for DUKPT.
// INPUT   : Key_Reg     - key register. (16-byte)
//           Crypto_Reg2 - the crypto register 2. (8-byte)
// OUTPUT  : Crypto_Reg1 - the crypto register 1. (8-byte)
// RETURN  : none.
// ---------------------------------------------------------------------------
void  DUKPT_SpecialEncrypt( UCHAR *Key_Reg, UCHAR *Crypto_Reg1, UCHAR *Crypto_Reg2 )
{
UCHAR i;
UCHAR buf[8];

      // 1. the Crypto Register-1 XOR'ed with the right half of the key Register goes to the Crypto Register-2
      for( i=0; i<8; i++ )
         Crypto_Reg2[i] = Crypto_Reg1[i] ^ Key_Reg[i+8];

      // 2. the Crypto Register-2 DEA-encrypted using, as the key, the left half of the Key Register goes to the Crypto Register-2
      PED_des_encipher( Crypto_Reg2, buf, &Key_Reg[0] );
      memmove( Crypto_Reg2, buf, 8 );
      
      // 3. Crypto Register-2 XOR'ed with the right half of the Key Register goes to Crytpo Register-2
      for( i=0; i<8; i++ )
         Crypto_Reg2[i] ^= Key_Reg[i+8];

      // 4. XOR the Key Register with hexdecimal C0C0 C0C0 0000 0000 C0C0 C0C0 0000 0000
      Key_Reg[0] ^= 0xC0;
      Key_Reg[1] ^= 0xC0;
      Key_Reg[2] ^= 0xC0;
      Key_Reg[3] ^= 0xC0;
      Key_Reg[8] ^= 0xC0;
      Key_Reg[9] ^= 0xC0;
      Key_Reg[10] ^= 0xC0;
      Key_Reg[11] ^= 0xC0;

      // 5. the Crypto Register-1 XOR'ed with the right half of the key Register goes to the Crypto Register-1
      for( i=0; i<8; i++ )
         Crypto_Reg1[i] ^= Key_Reg[i+8];

      // 6. the Crypto Register-1 DEA-encrypted using, as the key, the left half of the Key Register goes to the Crypto Register-1
      PED_des_encipher( Crypto_Reg1, buf, &Key_Reg[0] );
      memmove( Crypto_Reg1, buf, 8 );

      // 7. the Crypto Register-1 XOR'ed with the right half of the Key Register goes to the Crypto Register-1
      for( i=0; i<8; i++ )
         Crypto_Reg1[i] ^= Key_Reg[i+8];
}

// ---------------------------------------------------------------------------
// FUNCTION: Set to "one" that bit in the Shift Register.
// INPUT   : ksnr      - key register. (10-byte)
// OUTPUT  : Shift_Reg - the shift register. (8-byte)
// RETURN  : none.
// ---------------------------------------------------------------------------
UCHAR DUKPT_SetBit( UCHAR *ksnr, UCHAR *Shift_Reg )
{
UCHAR i;
UCHAR ec1[3];
UCHAR ec2[3];

      // set to "one" that bit in the Shift Register that corresponds to the
      // rightmost "one" bit in the Encryption Counter, making all other bits
      // in the Shift Register equal zero.
//    memmove( ec1, &KeySerialNum_Reg[7], 3 );
      memmove( ec1, ksnr+7, 3 );
      ec1[0] &= 0x1F; // 21 bits encryption counter

      if( (ec1[0] | ec1[1] | ec1[2]) == 0 )
        return( 0 ); // the Encryption Counter = 0

      if( ec1[2] & 0x01 )
        i = 1; // the rightmost bit position (from #1)
      else
        {
        for( i=2; i<=21; i++ )
           {
           right_shift_str( ec2, ec1, 3, 1 );
           memmove( ec1, ec2, 3 );
           if( ec1[2] & 0x01 )
             break;
           }
        }

      Shift_Reg[0] = 0x00;
      Shift_Reg[1] = 0x00;
      Shift_Reg[2] = 0x01;

      if( --i )
        {
        left_shift_str( ec1, Shift_Reg, 3, i );
        memmove( Shift_Reg, ec1, 3 );
        }

      return( 21-i ); // the bit "one" position in Shift Register (from MSB to LSB 21..1)
}

// ---------------------------------------------------------------------------
// FUNCTION: Load 12 rightmost digits of PAN.
// INPUT   : ipan - EMV cn10 PAN BCD format. (LL-V)
// OUTPUT  : opan - fixed 12 PAN digits. (ASCII)
//                 RULE:
//                 the 12 rightmost digits of PAN excluding the check digit.
//                 if PAN excluding the check digit is less than 12 digits,
//                 the digits are right justified and padded to the left with
//                 zeros.
// RETURN  : TRUE  - OK.
//           FALSE - ERROR. (PAN is not available)
// ---------------------------------------------------------------------------
//UCHAR PED_GetPAN12_EX( UCHAR *ipan, UCHAR *opan )
//{
//UCHAR apan[21];
//UCHAR len;
//
//      apk_ReadRamDataICC( ADDR_ICC_AP_PAN, 12, ipan );
//
//      LIB_bcd2asc( ipan[0], &ipan[2], apan );
//      len = apan[0];
//
//      if( apan[0] >= 13 )
//        memmove( opan, &apan[1+len-13], 12 );
//      else
//        {
//        memset( opan, 0x30, 12 ); // preset to '0'
//        len = 12 - apan[0] + 1;
//        memmove( &opan[len], &apan[1], apan[0] - 1 );
//        }
//}

// ---------------------------------------------------------------------------
// FUNCTION: Load 12 rightmost digits of PAN.
// INPUT   : ipan - PAN digits. (L-V) full PAN with check digit.
// OUTPUT  : opan - fixed 12 PAN digits. (ASCII)
//                 RULE:
//                 the 12 rightmost digits of PAN excluding the check digit.
//                 if PAN excluding the check digit is less than 12 digits,
//                 the digits are right justified and padded to the left with
//                 zeros.
// RETURN  : TRUE  - OK.
//           FALSE - ERROR. (PAN is not available)
// ---------------------------------------------------------------------------
//UCHAR PED_GetPAN12( UCHAR *ipan, UCHAR *opan )
//{
//UCHAR i;
//UCHAR panlen;
//UCHAR len;
//
//      memset( opan, 0x30, 12 );	// preset to '0's
//      
//      panlen = ipan[0];
//      if( panlen > 12 )
//        len = 12;
//      else
//        len = panlen;
//        
//      for( i=0; i<len; i++ )
//         opan[11-i] = ipan[panlen-(1+i)];
//}

// ---------------------------------------------------------------------------
// FUNCTION: Load initial keys to terminal memory for DUKPT. (PED function)
// INPUT   : id            - to identify load initial key or new one key.
//			     00 = load initial key
//			     FF = new key
//           CurKeyPtr     - current key pointer.
//           Shift_Reg     - shfit register. (3-byte)
//           InitPinEncKey - initial PIN encryption key. (16-byte)
//           KeySerialNum  - key serial number. (10-byte)
// OUTPUT  : none.
// RETURN  : TRUE  - OK.
//           FALSE - the PIN pad is now inoperative, having encrypted more than 1 million PINs.
// ---------------------------------------------------------------------------
UCHAR DUKPT_LoadInitialKey( UCHAR id, UINT CurKeyPtr, UCHAR *Shift_Reg, UCHAR *InitPinEncKey, UCHAR *KeySerialNum )
{
UCHAR i, j;
UCHAR KeySerialNum_Reg[KSN_REG_LEN];    // Key Serial Number Register (IKSN(59bits)+EC(21bits))
UCHAR Key_Reg[FUTURE_KEY_LEN];          // Key Register
UCHAR Crypto_Reg1[8];                   // Crypto Register 1
UCHAR Crypto_Reg2[8];                   // Crypto Register 2
UCHAR buf1[8];
UCHAR buf2[8];
UCHAR ec1[3];
UCHAR ec2[3];
UCHAR cc;
UINT  iCurrentKey_Ptr;
UCHAR len;


      // check special ID for PED_RequestPinEntry()
      // ID=0x00 -- load initial key
      //    0xFF -- new key
      if( id == 0xFF )
        {
        iCurrentKey_Ptr = CurKeyPtr;
	OS_SECM_GetData( ADDR_KSN_REG, KSN_REG_LEN, KeySerialNum_Reg );
        goto New_Key;
        }
       else
        {
        if( id != 00 )
          return( FALSE );
        }

      // clear MAC Key Register
      len = 0;
      OS_SECM_PutData( ADDR_MAC_KEY_REG, 1, (UCHAR *)&len );	// erase length of the MAC key reg

      // store the initial PIN encryption key into Future Key Register #21
      OS_SECM_PutData( ADDR_FUTURE_KEY_REG+(MAX_FUTURE_KEY_REG_CNT-1)*FUTURE_KEY_SLOT_LEN, FUTURE_KEY_LEN, InitPinEncKey );

      // generate and store the LRC on this Future Key Register
      cc = 0;
      for( j=0; j<FUTURE_KEY_LEN; j++ )
         cc ^= *InitPinEncKey++;

      OS_SECM_PutData( ADDR_FUTURE_KEY_REG+(MAX_FUTURE_KEY_REG_CNT-1)*FUTURE_KEY_SLOT_LEN+FUTURE_KEY_LEN, 1, (UCHAR *)&cc );

      // write the address of Future Key Register #21 into the Current Key Pointer
      iCurrentKey_Ptr = MAX_FUTURE_KEY_REG_CNT - 1; // from #0

      // store the Key Serial Number into the Key Serial Number Register
      memmove( KeySerialNum_Reg, KeySerialNum, 10 );

      // clear the Encryption Counter (the 21 rightmost bits of the KSN reg)
      KeySerialNum_Reg[7] &= 0xE0;
      KeySerialNum_Reg[8] = 0x00;
      KeySerialNum_Reg[9] = 0x00;

      OS_SECM_PutData( ADDR_KSN_REG, KSN_REG_LEN, KeySerialNum_Reg );

      // set bit #1 (the leftmost bit) of the Shift Register to "one",
      // setting all of the other bits to "zero".
      memset( Shift_Reg, 0x00, 3 );
      Shift_Reg[0] = 0x10;

      goto New_Key_3;

New_Key:

      // count the number of "one" bits in the 21-bit Encryption Counter.
      // if this number is less than 10, go to "New Key-1"
      memmove( ec1, &KeySerialNum_Reg[7], 3 );
      ec1[0] &= 0x1F;

      cc = 0;
      for( j=0; j<21; j++ )
         {
         if( ec1[2] & 0x01 )
           cc++;

         right_shift_str( ec2, ec1, 3, 1 );
         memmove( ec1, ec2, 3 );
         }

      if( cc < 10 )
        goto New_Key_1;

      // erase the key at ![Current Key Pointer]
      OS_SECM_ClearData( ADDR_FUTURE_KEY_REG+iCurrentKey_Ptr*FUTURE_KEY_SLOT_LEN, FUTURE_KEY_LEN, 0x00 );

      // set the LRC for ![Current Key Pointer] to an invalid value
      // (e.g., increment the LRC by one)
      OS_SECM_GetData( ADDR_FUTURE_KEY_REG+iCurrentKey_Ptr*FUTURE_KEY_SLOT_LEN+FUTURE_KEY_LEN, 1, (UCHAR *)&cc );     
      cc++;
      OS_SECM_PutData( ADDR_FUTURE_KEY_REG+iCurrentKey_Ptr*FUTURE_KEY_SLOT_LEN+FUTURE_KEY_LEN, 1, (UCHAR *)&cc );

      // add the Shift Register to the Encryption Counter
      // (this procedure skips those counter values that would have more than 10 "one" bits
      memmove( ec1, &KeySerialNum_Reg[7], 3 );
      cc = ec1[0] & 0xE0;

      if( (ec1[2] != 0) || (Shift_Reg[2] != 0) )
        {
        ec1[2] += Shift_Reg[2];
        if( ec1[2] == 0 )
          {
          ec1[1]++;
          if( ec1[1] == 0 )
            ec1[0]++;
          }
        }

      if( (ec1[1] != 0) || (Shift_Reg[1] != 0) )
        {
        ec1[1] += Shift_Reg[1];
        if( ec1[1] == 0 )
          ec1[0]++;
        }

      ec1[0] += Shift_Reg[0];
      ec1[0] &= 0x1F;
      ec1[0] |= cc;

      memmove( &KeySerialNum_Reg[7], ec1, 3 );
      OS_SECM_PutData( ADDR_KSN_REG+7, 3, ec1 );

      goto New_Key_2;

New_Key_1:

      // shfit the Shift Register right one bit
      right_shift_str( buf1, Shift_Reg, 3, 1 );
      memmove( Shift_Reg, buf1, 3 );

      // if the Shift Register now contains all zeros, go to "New Key-4"
      // else goto "New Key-3"
      if( (Shift_Reg[0] | Shift_Reg[1] | Shift_Reg[2]) == 0 )
        goto New_Key_4;

New_Key_3:

      // the Shift Register, right justified in 64 bits, padded to the left
      // with zeros, OR'ed with the 64 rightmost bits of the Key Serial Number Register,
      // is transferred into the Crypto Register-1.
      memset( buf1, 0x00, 8 );
      memmove( &buf1[5], Shift_Reg, 3 );

      for( i=0; i<8; i++ )
         Crypto_Reg1[i] = buf1[i] | KeySerialNum_Reg[2+i];

      // copy ![Current Key Pointer] into the Key Register
      OS_SECM_GetData( ADDR_FUTURE_KEY_REG+iCurrentKey_Ptr*FUTURE_KEY_SLOT_LEN, FUTURE_KEY_LEN, Key_Reg );
      
      // call the subroutine "Non-reversible Key Generation Process"
      DUKPT_SpecialEncrypt( Key_Reg, Crypto_Reg1, Crypto_Reg2 );

      // store the contents of the Crypto Register-1 into the left half of the Future Key Register
      // indicated by the position of the single "one" bit in the Shift Register
      left_shift_str( buf1, Shift_Reg, 3, 2 );  // throw away leftmost 2 bits

      for( i=0; i<21; i++ )
         {
         left_shift_str( buf2, buf1, 3, 1 );  // left shift 1 bits
         memmove( buf1, buf2, 3 );
         if( buf1[0] & 0x80 )  // until "one" is met
           break;
         }

      OS_SECM_PutData( ADDR_FUTURE_KEY_REG+i*FUTURE_KEY_SLOT_LEN+0, 8, Crypto_Reg1 );

      // store the contents of the Crypto Register-2 into the right half of the Future Key Register
      // indicated by the position of the single "one" bit in the Shift Register

      OS_SECM_PutData( ADDR_FUTURE_KEY_REG+i*FUTURE_KEY_SLOT_LEN+8, 8, Crypto_Reg2 );

      // generate and store the LRC on this Future Key Register
      cc = 0;
      for( j=0; j<8; j++ )
         cc ^= Crypto_Reg1[j];

      for( j=0; j<8; j++ )
         cc ^= Crypto_Reg2[j];

      OS_SECM_PutData( ADDR_FUTURE_KEY_REG+i*FUTURE_KEY_SLOT_LEN+FUTURE_KEY_LEN, 1, (UCHAR *)&cc );
      
      goto New_Key_1;

New_Key_4:

      if( id == 0x00 ) // load initial key?
        goto New_Key_5;

      // erase the key at ![Current Key Pointer]
      OS_SECM_ClearData( ADDR_FUTURE_KEY_REG+iCurrentKey_Ptr*FUTURE_KEY_SLOT_LEN, FUTURE_KEY_LEN, 0x00 );

      // set the LRC for ![Current Key Pointer] to an invalid value
      // (e.g., increment the LRC by one)
      OS_SECM_GetData( ADDR_FUTURE_KEY_REG+iCurrentKey_Ptr*FUTURE_KEY_SLOT_LEN+FUTURE_KEY_LEN, 1, (UCHAR *)&cc );
      cc++;
      OS_SECM_PutData( ADDR_FUTURE_KEY_REG+iCurrentKey_Ptr*FUTURE_KEY_SLOT_LEN+FUTURE_KEY_LEN, 1, (UCHAR *)&cc );

New_Key_5:

      // add one to the Encryption Counter
      cc = KeySerialNum_Reg[7] & 0xE0; // backup lsb 3-bit of IKSN
      KeySerialNum_Reg[9] += 1;
      if( KeySerialNum_Reg[9] == 0 )
        {
        KeySerialNum_Reg[8] += 1;
        if( KeySerialNum_Reg[8] == 0 )
          KeySerialNum_Reg[7] += 1;
        }
      KeySerialNum_Reg[7] &= 0x1F;
      KeySerialNum_Reg[7] |= cc;

      OS_SECM_PutData( ADDR_KSN_REG+7, 3, &KeySerialNum_Reg[7] );

New_Key_2:

      // if the Encryption Counter contains all zero, cease operation
      if( ((KeySerialNum_Reg[7] & 0x1F) | KeySerialNum_Reg[8] | KeySerialNum_Reg[9]) == 0 )
        return( FALSE ); // the PIN pad is now inoperative, having encrypted more than 1 million PINs
      else
        return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: Request PIN entry for DUKPT. (PED function)
// INPUT   : pinblock - the plaintext pinblock. (fixed 8 bytes)
// OUTPUT  : ksn      - format1: L-V(10)
//                      key serial number with leading hex "F's" suppressed. (max.10-byte)
//                      format2: V(10)
//                      fix 10-byte with leading hex "F's".
//           epb      - encryption PIN block. (8-byte)
// RETURN  : TRUE  - OK.
//           FALSE - the PIN pad is now inoperative, having encrypted more than 1 million PINs.
// NOTE    : This function shall be called prior to invoking "PED_DUKPT_GenMAC()" to generate MAC code.
// ---------------------------------------------------------------------------
UCHAR DUKPT_RequestPinEntry(  UCHAR *pinblock, UCHAR *ksn, UCHAR *epb )
{
UCHAR i, j;
UCHAR ec1[3];
UCHAR ec2[3];
UCHAR KeySerialNum_Reg[KSN_REG_LEN];    // Key Serial Number Register (IKSN(59bits)+EC(21bits))
UCHAR MAC_Key_Reg[FUTURE_KEY_LEN];      // MAC Key Register -- optional
UCHAR Key_Reg[FUTURE_KEY_LEN];          // Key Register
UCHAR Shift_Reg[3];                     // rightmost 21 bits are effective
UCHAR Crypto_Reg1[8];                   // Crypto Register 1
UCHAR buf[17];
UCHAR cc;
UINT  iCurrentKey_Ptr;
UCHAR len;


      // retrieve KSN value
      OS_SECM_GetData( ADDR_KSN_REG, KSN_REG_LEN, KeySerialNum_Reg );

      // transfer the primary account number as received in the externally
      // initiated command into the Account Number Register (external process)

      // activate the PIN pad keyboard and the Enter Key (external process)

      // if the PIN is not entered, send the encrypted PIN block response message
      // without the PIN-related data elements and go to "Exit" (external process)

      // if the PIN is entered, use the cardholder-entered PIN and the PAN
      // to generate the cleartext PIN block and store it in the Crypto Register-1
      memmove( Crypto_Reg1, pinblock, 8 );
      
Request_PIN_Entry_1:

      // Set Bit
      i = DUKPT_SetBit( KeySerialNum_Reg, Shift_Reg );
      
      // write into Current Key Pointer the address of that Future Key Register
      // indicated by the position of the "one" bit in the Shift Register
      iCurrentKey_Ptr = (UINT)(i - 1); // from #0

      // check the LRC on ![Current Key Pointer]
      // If this byte is correct (valid key), go to "Request PIN Entry 2"
      OS_SECM_GetData( ADDR_FUTURE_KEY_REG+iCurrentKey_Ptr*FUTURE_KEY_SLOT_LEN, FUTURE_KEY_SLOT_LEN, buf );

      cc = 0;
      for( j=0; j<FUTURE_KEY_LEN; j++ )
         cc ^= buf[j];

      if( cc == buf[j] ) // check LRC
        goto Request_PIN_Entry_2;
      else // LRC incorrect
        {
        // add the Shift Register to the Encryption Counter
        // (to skip over the invalid key)
        memmove( ec1, &KeySerialNum_Reg[7], 3 );
        cc = ec1[0] & 0xE0;

        if( (ec1[2] != 0) || (Shift_Reg[2] != 0) )
          {
          ec1[2] += Shift_Reg[2];
          if( ec1[2] == 0 )
            {
            ec1[1]++;
            if( ec1[1] == 0 )
              ec1[0]++;
            }
          }

        if( (ec1[1] != 0) || (Shift_Reg[1] != 0) )
          {
          ec1[1] += Shift_Reg[1];
          if( ec1[1] == 0 )
            ec1[0]++;
          }

        ec1[0] += Shift_Reg[0];
        ec1[0] &= 0x1F;
        ec1[0] |= cc;

        memmove( &KeySerialNum_Reg[7], ec1, 3 );
	OS_SECM_PutData( ADDR_KSN_REG+7, 3, ec1 );

        // if the Encryption Counter contains all zeros, cease operation.
        // (the PIN Entry Device is now inoperative, having encrypted more than 1 million PINs)
        if( ((KeySerialNum_Reg[7] & 0x1F) | KeySerialNum_Reg[8] | KeySerialNum_Reg[9]) == 0 )
          return( FALSE );

        goto Request_PIN_Entry_1;
        }

Request_PIN_Entry_2:

      // copy ![Current Key Pointer] into the Key Register
      OS_SECM_GetData( ADDR_FUTURE_KEY_REG+iCurrentKey_Ptr*FUTURE_KEY_SLOT_LEN, FUTURE_KEY_LEN, Key_Reg );
      
      // (Optional: perform this step if you need to generate a key that will
      // be used in a message authentication process; this step does not affect
      // the generation of the PIN encryption key) XOR the value in the Key Register
      // with hexdecimal "0000 0000 0000 FF00 0000 0000 0000 FF00" and save this
      // resultant key in the MAC key register
      memmove( MAC_Key_Reg, Key_Reg, FUTURE_KEY_LEN );
      MAC_Key_Reg[6] = Key_Reg[6] ^ 0xFF;
      MAC_Key_Reg[14] = Key_Reg[14] ^ 0xFF;
      
      len = FUTURE_KEY_LEN;
      OS_SECM_PutData( ADDR_MAC_KEY_REG, 1, (UCHAR *)&len );			// store length of the MAC key reg
      OS_SECM_PutData( ADDR_MAC_KEY_REG+1, FUTURE_KEY_LEN, MAC_Key_Reg );	// store value of the MAC key reg
      
      // exclusive-OR the Key Register with hexdecimal "0000 0000 0000 00FF 0000 0000 0000 00FF"
      // (this will produce a variant of the key)
      Key_Reg[7] ^= 0xFF;
      Key_Reg[15] ^= 0xFF;

      // call the subroutine "Triple-DEA Encrypt"
      DUKPT_TripleDEA( Key_Reg, Crypto_Reg1 );

      // format and transmit the encrypted PIN block response message, which includes:
      // -- the data in the Key Serial Number Register with leading hex "F's"
      //    suppressed (includes the 21-bit Encryption Counter)
      // -- the encrypted PIN block in the Crypto Register-1
      //
      // === TO BE IMPLEMENTED ===

      memmove( epb, Crypto_Reg1, 8 ); // encryption pin block
      memmove( ksn, KeySerialNum_Reg, 10 ); // FORMAT2: key serial number with leading "F's"

      return( DUKPT_LoadInitialKey( 0xFF, iCurrentKey_Ptr, Shift_Reg, ec1, ec2 ) ); // goto "New Key"
}

// ***************************************************************************
//	New Functions for DUKPT Application
// ***************************************************************************

// ---------------------------------------------------------------------------
// FUNCTION: Request a current DUKPT key for encryption.
// INPUT   : usage    - key usage for encryption (new spec 2009 or 2017 in C.5.2)
//			0=PIN
//			1=MAC BOTH
//			2=MAC RESPONSE
//			3=DATA BOTH
//			4=DATA RESPONSE
// OUTPUT  : ksn      - fixed 10-byte with leading hex "F's". (to be sent to acquirer)
//           pek      - current PEK. (fixed 16-byte)
// RETURN  : TRUE  - OK.
//           FALSE - the PIN pad is now inoperative, having encrypted more than 1 million PINs.
// ---------------------------------------------------------------------------
UCHAR	DUKPT_RequestPEK( UCHAR *pek, UCHAR *ksn, UCHAR usage )
{
UCHAR i, j;
UCHAR ec1[3];
UCHAR ec2[3];
UCHAR KeySerialNum_Reg[KSN_REG_LEN];    // Key Serial Number Register (IKSN(59bits)+EC(21bits))
UCHAR MAC_Key_Reg[FUTURE_KEY_LEN];      // MAC Key Register -- optional
UCHAR Key_Reg[FUTURE_KEY_LEN];          // Key Register
UCHAR Shift_Reg[3];                     // rightmost 21 bits are effective
UCHAR Crypto_Reg1[8];                   // Crypto Register 1
UCHAR buf[17];
UCHAR cc;
UINT  iCurrentKey_Ptr;
UCHAR len;

UCHAR DATA[16];
UCHAR KEY[16];


      // retrieve KSN value
      OS_SECM_GetData( ADDR_KSN_REG, KSN_REG_LEN, KeySerialNum_Reg );

      // transfer the primary account number as received in the externally
      // initiated command into the Account Number Register (external process)

      // activate the PIN pad keyboard and the Enter Key (external process)

      // if the PIN is not entered, send the encrypted PIN block response message
      // without the PIN-related data elements and go to "Exit" (external process)

      // if the PIN is entered, use the cardholder-entered PIN and the PAN
      // to generate the cleartext PIN block and store it in the Crypto Register-1
//    memmove( Crypto_Reg1, pinblock, 8 );
      
Request_PIN_Entry_1:

      // Set Bit
      i = DUKPT_SetBit( KeySerialNum_Reg, Shift_Reg );
      
      // write into Current Key Pointer the address of that Future Key Register
      // indicated by the position of the "one" bit in the Shift Register
      iCurrentKey_Ptr = (UINT)(i - 1); // from #0

      // check the LRC on ![Current Key Pointer]
      // If this byte is correct (valid key), go to "Request PIN Entry 2"
      OS_SECM_GetData( ADDR_FUTURE_KEY_REG+iCurrentKey_Ptr*FUTURE_KEY_SLOT_LEN, FUTURE_KEY_SLOT_LEN, buf );

      cc = 0;
      for( j=0; j<FUTURE_KEY_LEN; j++ )
         cc ^= buf[j];

      if( cc == buf[j] ) // check LRC
        goto Request_PIN_Entry_2;
      else // LRC incorrect
        {
        // add the Shift Register to the Encryption Counter
        // (to skip over the invalid key)
        memmove( ec1, &KeySerialNum_Reg[7], 3 );
        cc = ec1[0] & 0xE0;

        if( (ec1[2] != 0) || (Shift_Reg[2] != 0) )
          {
          ec1[2] += Shift_Reg[2];
          if( ec1[2] == 0 )
            {
            ec1[1]++;
            if( ec1[1] == 0 )
              ec1[0]++;
            }
          }

        if( (ec1[1] != 0) || (Shift_Reg[1] != 0) )
          {
          ec1[1] += Shift_Reg[1];
          if( ec1[1] == 0 )
            ec1[0]++;
          }

        ec1[0] += Shift_Reg[0];
        ec1[0] &= 0x1F;
        ec1[0] |= cc;

        memmove( &KeySerialNum_Reg[7], ec1, 3 );
	OS_SECM_PutData( ADDR_KSN_REG+7, 3, ec1 );

        // if the Encryption Counter contains all zeros, cease operation.
        // (the PIN Entry Device is now inoperative, having encrypted more than 1 million PINs)
        if( ((KeySerialNum_Reg[7] & 0x1F) | KeySerialNum_Reg[8] | KeySerialNum_Reg[9]) == 0 )
          return( FALSE );

        goto Request_PIN_Entry_1;
        }

Request_PIN_Entry_2:

      // copy ![Current Key Pointer] into the Key Register
      OS_SECM_GetData( ADDR_FUTURE_KEY_REG+iCurrentKey_Ptr*FUTURE_KEY_SLOT_LEN, FUTURE_KEY_LEN, Key_Reg );
      
      // (Optional: perform this step if you need to generate a key that will
      // be used in a message authentication process; this step does not affect
      // the generation of the PIN encryption key) XOR the value in the Key Register
      // with hexdecimal "0000 0000 0000 FF00 0000 0000 0000 FF00" and save this
      // resultant key in the MAC key register
#if	0
      memmove( MAC_Key_Reg, Key_Reg, FUTURE_KEY_LEN );
      MAC_Key_Reg[6] = Key_Reg[6] ^ 0xFF;
      MAC_Key_Reg[14] = Key_Reg[14] ^ 0xFF;
      
      len = FUTURE_KEY_LEN;
      OS_SECM_PutData( ADDR_MAC_KEY_REG, 1, (UCHAR *)&len );			// store length of the MAC key reg
      OS_SECM_PutData( ADDR_MAC_KEY_REG+1, FUTURE_KEY_LEN, MAC_Key_Reg );	// store value of the MAC key reg
      
      // exclusive-OR the Key Register with hexdecimal "0000 0000 0000 00FF 0000 0000 0000 00FF"
      // (this will produce a variant of the key)
      Key_Reg[7] ^= 0xFF;
      Key_Reg[15] ^= 0xFF;
#endif

      // NEW SPEC using VARIANT constants for the transaction keys (PIN, MAC, DATA)
      switch( usage )
            {
            case 0:	// PIN ENCRYPTION
            	 
            	 Key_Reg[7] ^= 0xFF;
            	 Key_Reg[15] ^= 0xFF;
            	 
            	 break;
            	 
            case 1:	// MAC BOTH
            	 
            	 Key_Reg[6] ^= 0xFF;
            	 Key_Reg[14] ^= 0xFF;
            	 
            	 break;
            	 
            case 2:	// MAC RESPONSE

            	 Key_Reg[4] ^= 0xFF;
            	 Key_Reg[12] ^= 0xFF;
            	 
            	 break;
            	 
            case 3:	// DATA BOTH
            	
            	 Key_Reg[5] ^= 0xFF;
            	 Key_Reg[13] ^= 0xFF;
            	 
            	 memmove( KEY, Key_Reg, 16 );
            	 
		 memmove( DATA, &KEY[0], 8 );
            	 DUKPT_TripleDEA( KEY, &DATA[0] );
            	 
            	 memmove( &DATA[8], &KEY[8], 8 );
            	 DUKPT_TripleDEA( KEY, &DATA[8] );
            	 
            	 memmove( Key_Reg, DATA, 16 );
            	 
            	 break;
            	 
            case 4:	// DATA RESPONSE
            	
            	 Key_Reg[3] ^= 0xFF;
            	 Key_Reg[11] ^= 0xFF;
            	 
            	 memmove( KEY, Key_Reg, 16 );
            	 
		 memmove( DATA, &KEY[0], 8 );
            	 DUKPT_TripleDEA( KEY, &DATA[0] );
            	 
            	 memmove( &DATA[8], &KEY[8], 8 );
            	 DUKPT_TripleDEA( KEY, &DATA[8] );
            	 
            	 memmove( Key_Reg, DATA, 16 );
            	 
            	 break;
            }


      // call the subroutine "Triple-DEA Encrypt"
//    DUKPT_TripleDEA( Key_Reg, Crypto_Reg1 );

      memmove( pek, Key_Reg, sizeof(Key_Reg) );	// current PEK for encryption (fixed 16 bytes)

      // format and transmit the encrypted PIN block response message, which includes:
      // -- the data in the Key Serial Number Register with leading hex "F's"
      //    suppressed (includes the 21-bit Encryption Counter)
      // -- the encrypted PIN block in the Crypto Register-1
      //
      // === TO BE IMPLEMENTED ===

//    memmove( epb, Crypto_Reg1, 8 ); // encryption pin block
      memmove( ksn, KeySerialNum_Reg, 10 ); // FORMAT2: key serial number with leading "F's"

      return( DUKPT_LoadInitialKey( 0xFF, iCurrentKey_Ptr, Shift_Reg, ec1, ec2 ) ); // goto "New Key"
}

// ---------------------------------------------------------------------------
// FUNCTION: Derive IPEK from BDK & IKSN by the method as described in the spec "A.6".
//
// Step (1) & (2)
//	IKSN = FF FF 98 76 54 32 10 E0 00 00
//	BDK  = 01 23 45 67 89 AB CD EF FE DC BA 98 76 54 32 10
// Step (3) & (4)
//	DATA = FF FF 98 76 54 32 10 E0
//	KEY  = 01 23 45 67 89 AB CD EF FE DC BA 98 76 54 32 10
//	Left half of the initial key = TDES( DATA, KEY ) = 6A C2 92 FA A1 31 5B 4D
// Step (5) & (6)
//	DATA = FF FF 98 76 54 32 10 E0
//	KEY  = [01 23 45 67 89 AB CD EF FE DC BA 98 76 54 32 10] XOR
//	       [C0 C0 C0 C0 00 00 00 00 C0 C0 C0 C0 00 00 00 00]
//	     =  C1 E3 85 A7 89 AB CD EF 3E 1C 7A 58 76 54 32 10
//	Right half of the initial key = TDES( DATA, KEY ) = 85 8A B3 A3 D7 D5 93 3A
//	
//	IPEK = 6A C2 92 FA A1 31 5B 4D 85 8A B3 A3 D7 D5 93 3A
//
// INPUT   : bdk      - base derivation key (BDK, fixed 16 bytes of TDES key)
//	     iksn     - initial key serial number (IKSN, fixed 10 bytes)
// OUTPUT  : ipek     - derived initial PIN entry key (IPEK, fixed 16 bytes of TDES key)
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
UCHAR	DUKPT_DeriveIPEK( UCHAR *bdk, UCHAR *iksn, UCHAR *ipek )
{
UCHAR	i;
UCHAR	data[16];
UCHAR	key[16];
UCHAR	variant[16] = {0xC0, 0xC0, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0x00 };


	memmove( key, bdk, 16 );
	memmove( data, iksn, 8 );
	DUKPT_TripleDEA( key, data );
	memmove( ipek, data, 8 );	// left half of IPEK
	
	memmove( key, bdk, 16 );
	for( i=0; i<16; i++ )
	   key[i] = key[i]^variant[i];
	memmove( data, iksn, 8 );
	DUKPT_TripleDEA( key, data );
	memmove( &ipek[8], data, 8 );	// right half of IPEK
	
	return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: Reload DUKPT system. (new future keys will be generated again)
// INPUT   : bdk      - base derivation key (BDK, fixed 16 bytes of TDES key),
//			effective only when mode = 0 or 2.
//	     iksn     - initial key serial number (IKSN, fixed 10 bytes)
//	     mode     - 0=BDK is clear text. (in "bdk")
//			1=BDK is one of the session keys. ("bdk" is no used)
//		        2=BDK is encrypted by a master key. (RFU)
//	     index    - key slot index of SK.
// OUTPUT  : none.
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
UCHAR	DUKPT_ReloadPEK( UCHAR mode, UCHAR index, UCHAR *bdk, UCHAR *iksn )
{
UCHAR	result = FALSE;
UCHAR	mkey[PED_MSKEY_SLOT_LEN2];
UCHAR	skey[PED_MSKEY_SLOT_LEN2];
UCHAR	Shift_Reg[3];
UCHAR	pbdk[16];
UCHAR	ipek[16];	// IPEK derived from BDK & IKSN


	switch( mode )
	      {
	      case 0:	// BDK is clear text
	      	
	      	   DUKPT_DeriveIPEK( bdk, iksn, ipek );
	      	   return( DUKPT_LoadInitialKey( 0, 0, Shift_Reg, ipek, iksn ) );
	      	   
	      case 1:	// BDK is one of the session keys

	      	   OS_SECM_GetData( ADDR_PED_SKEY_01+(index*PED_MSKEY_SLOT_LEN2), PED_MSKEY_SLOT_LEN2, skey );
	      	   if( skey[0] )
	      	     {
	      	     memmove( pbdk, &skey[1], PED_MSKEY_LEN );
	      	     DUKPT_DeriveIPEK( pbdk, iksn, ipek );
	      	     return( DUKPT_LoadInitialKey( 0, 0, Shift_Reg, ipek, iksn ) );
	      	     }
	      	     
	      	   break;
	      	   
	      case 2:	// BDK is encrypted by a master key (TDES)
	      	
	      	   OS_SECM_GetData( ADDR_PED_MKEY_01+(index*PED_MSKEY_SLOT_LEN2), PED_MSKEY_SLOT_LEN2, mkey );
	      	   if( mkey[0] )
	      	     {
	      	     PED_TripleDES2( &mkey[1], 16, bdk, pbdk );
	      	     memset( mkey, 0x00, sizeof(mkey) );
	      	     
	      	     DUKPT_DeriveIPEK( pbdk, iksn, ipek );
	      	     return( DUKPT_LoadInitialKey( 0, 0, Shift_Reg, ipek, iksn ) );
	      	     }
	      	     
	      	   break;
	      
	      }
	      
	return( result );
}

// ---------------------------------------------------------------------------
#if	0
void	TEST_DUKPT()
{
UCHAR	bdk[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};
UCHAR	iksn[10]= {0xFF, 0xFF, 0x98, 0x76, 0x54, 0x32, 0x10, 0xE0, 0x00, 0x00};
UCHAR	pek[16];
UCHAR	ksn[10];
UCHAR	usage = 3;	// DATA BOTH


	LIB_LCD_Puts( 0, 0, FONT0, sizeof("DUKPT"), (UINT8 *)"DUKPT" );
	LIB_DispHexByte( 1, 0, DUKPT_ReloadPEK( 0, 0, bdk, iksn ) );
	
	LIB_DispHexByte( 2, 0, DUKPT_RequestPEK( pek, ksn, usage ) );
	LIB_DumpHexData( 0, 3, 16, pek );
	LIB_DumpHexData( 0, 6, 10, ksn );
	
	for(;;);
	
}
#endif

// ---------------------------------------------------------------------------

