//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : APDU_SAM.C                                                 **
//**  MODULE   : apdu_SAM_GET_PUBLIC_KEY()                                  **
//**             apdu_SAM_LOAD_EXTERNAL_PK()                                **
//**             apdu_SAM_RSA_RECOVER()                                     **
//**             apdu_SAM_HASH()                                            **
//**             apdu_SAM_CLEAN()                                           **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2003/01/11                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2003-2007 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include <string.h>

#include "POSAPI.h"
//#include <EMVDC.h>
#include "GDATAEX.h"
#include "EMVAPI.h"
//#include <XCONSTP3.H>

//#include <TOOLS.h>
//#include <UI.H>

// ---------------------------------------------------------------------------
// FUNCTION: load CA public keys for EMV L2 TEST.
//           (Create_Pub) -> Select_Pub -> Load_Pub
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// PREQ    : the FileIDs have been created (0x8001-0x8032) for the max key size;
//           exponent = 65537, modulus = 2048 bits (total 288 bytes).
// ---------------------------------------------------------------------------
UCHAR apdu_SAM_LOAD_EMVL2_CAPK( void )
{
UCHAR i;
UCHAR c_apdu[258];
UCHAR r_apdu[8];
UINT  iFID;
UINT  iLen;

//      iFID = KEY_FID_01; // initial key file id
//
//      for( i=0; i<MAX_KEY_SLOT_CNT; i++ )
//         {
//         // select public key (Select_Pub)
//         c_apdu[0] = 0x07;                 // length of APDU
//         c_apdu[1] = 0x00;                 //
//
//         c_apdu[2] = 0x90;                     // CLA
//         c_apdu[3] = 0xE3;                     // INS
//         c_apdu[4] = 0x00;                     // P1
//         c_apdu[5] = 0x00;                     // P2
//         c_apdu[6] = 0x02;                     // Lc
//         c_apdu[7] = (iFID & 0xff00) >> 8;     // Data=FID
//         c_apdu[8] = iFID & 0x00ff;            //
//         if( api_ifm_exchangeAPDU( g_dhn_sam, c_apdu, r_apdu ) != apiOK )
//           return( apiFailed );
//         iLen = r_apdu[1]*256 + r_apdu[0];
//         if( (iLen != 2) || (r_apdu[iLen] != 0x90) || (r_apdu[iLen+1] != 0x00) )
//           return( apiFailed );
//
////       UI_WaitKey();
//
//         // (Load_Pub)
//         // load Header (RID+INDEX+EXP_LEN+MOD_LEN+SHA1) & Exponent
//         c_apdu[0] = 5+29+CA_PublicKey[i][OFFSET_SAM_EXP_LEN];
//         c_apdu[1] = 0x00;                 // length of APDU
//
//         c_apdu[2] = 0x90;                     // CLA
//         c_apdu[3] = 0x30;                     // INS
//         c_apdu[4] = 0x00;                     // P1=offset HI
//         c_apdu[5] = 0x00;                     // P2=offset LO
//         c_apdu[6] = 29+CA_PublicKey[i][OFFSET_SAM_EXP_LEN];    // Lc = header size
//         memmove( &c_apdu[7], &CA_PublicKey[i][0], c_apdu[6] ); // Data
//
//         if( api_ifm_exchangeAPDU( g_dhn_sam, c_apdu, r_apdu ) != apiOK )
//           return( apiFailed );
//         iLen = r_apdu[1]*256 + r_apdu[0];
//         if( (iLen != 2) || (r_apdu[iLen] != 0x90) || (r_apdu[iLen+1] != 0x00) )
//           return( apiFailed );
//
////       UI_WaitKey();
//
//         // load Modulus
//         c_apdu[0] = 5+CA_PublicKey[i][OFFSET_SAM_MOD_LEN+1]; // 00 NN
//         c_apdu[1] = 0x00;                 // length of APDU
//
//         c_apdu[2] = 0x90;                     // CLA
//         c_apdu[3] = 0x30;                     // INS
//         c_apdu[4] = 0x00;                                   // P1=offset HI
//         c_apdu[5] = 29+CA_PublicKey[i][OFFSET_SAM_EXP_LEN]; // P2=offset LO
//         c_apdu[6] = CA_PublicKey[i][OFFSET_SAM_MOD_LEN+1]; // Lc
//         memmove( &c_apdu[7], &CA_PublicKey[i][OFFSET_SAM_MOD], c_apdu[6] );
//
//         if( api_ifm_exchangeAPDU( g_dhn_sam, c_apdu, r_apdu ) != apiOK )
//           return( apiFailed );
//         iLen = r_apdu[1]*256 + r_apdu[0];
//         if( (iLen != 2) || (r_apdu[iLen] != 0x90) || (r_apdu[iLen+1] != 0x00) )
//           return( apiFailed );
//
//         TL_DispHexWord(0,0,iFID);
////       UI_WaitKey();
//
//         iFID++;
//         }

      return( apiOK );

}

// ---------------------------------------------------------------------------
// FUNCTION: delete public key file from SAM.
//           1. Select_Root -> Delete_Pub (not supported)
//           2. Select_Pub -> Load_Pub (fill key slot with 0s)
// INPUT   : fid - the file id for the specified key.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// PREQ    : the FileIDs have been created (8001-8009) for the max key size;
//           exponent = 65537, modulus = 2048 bits (total 288 bytes).
// ---------------------------------------------------------------------------
UCHAR apdu_SAM_DEL_PUBLIC_KEY( UINT fid )
{
UCHAR c_apdu[7+256];
UCHAR r_apdu[8];
UINT  iLen;

//    // --- METHOD 1 ---
//
//    // Change DIR to root (Sel_Pub)
//    c_apdu[0] = 0x04;                 // length of APDU
//    c_apdu[1] = 0x00;                 //
//
//    c_apdu[2] = 0x90;                     // CLA
//    c_apdu[3] = 0xE3;                     // INS
//    c_apdu[4] = 0x03;                     // P1
//    c_apdu[5] = 0x00;                     // P2
//    if( api_ifm_exchangeAPDU( g_dhn_sam, c_apdu, r_apdu ) != apiOK )
//      return( apiFailed );
//    iLen = r_apdu[1]*256 + r_apdu[0];
//    if( (iLen != 2) || (r_apdu[iLen] != 0x90) || (r_apdu[iLen+1] != 0x00) )
//      return( apiFailed );
//
//    // Delete public key (Delete_Pub)
//    c_apdu[0] = 0x04;                 // length of APDU
//    c_apdu[1] = 0x00;                 //
//
//    c_apdu[2] = 0x90;                     // CLA
//    c_apdu[3] = 0xE5;                     // INS
//    c_apdu[4] = (fid & 0xff00) >> 8;      // P1=fid_H
//    c_apdu[5] = fid & 0x00ff;             // P2=fid_L
//    if( api_ifm_exchangeAPDU( g_dhn_sam, c_apdu, r_apdu ) != apiOK )
//      return( apiFailed );
//    iLen = r_apdu[1]*256 + r_apdu[0];
//    if( (iLen != 2) || (r_apdu[iLen] != 0x90) || (r_apdu[iLen+1] != 0x00) )
//      return( apiFailed );

      // ---------------------------------------------------------------------

      // --- METHOD 2 ---

      // select public key (Select_Pub)
//    c_apdu[0] = 0x07;                 // length of APDU
//    c_apdu[1] = 0x00;                 //
//
//    c_apdu[2] = 0x90;                     // CLA
//    c_apdu[3] = 0xE3;                     // INS
//    c_apdu[4] = 0x00;                     // P1
//    c_apdu[5] = 0x00;                     // P2
//    c_apdu[6] = 0x02;                     // Lc
//    c_apdu[7] = (fid & 0xff00) >> 8;      // Data=FID
//    c_apdu[8] = fid & 0x00ff;             //
//    if( api_ifm_exchangeAPDU( g_dhn_sam, c_apdu, r_apdu ) != apiOK )
//      return( apiFailed );
//    iLen = r_apdu[1]*256 + r_apdu[0];
//    if( (iLen != 2) || (r_apdu[iLen] != 0x90) || (r_apdu[iLen+1] != 0x00) )
//      return( apiFailed );

//    UI_WaitKey();

      // (Load_Pub)
      // load Header (RID+INDEX+EXP_LEN+MOD_LEN+SHA1) & Exponent
//    memset( c_apdu, 0x00, sizeof(c_apdu) );
//
//    c_apdu[0] = 5+29+3;               //
//    c_apdu[1] = 0x00;                 // length of APDU
//
//    c_apdu[2] = 0x90;                     // CLA
//    c_apdu[3] = 0x30;                     // INS
//    c_apdu[4] = 0x00;                     // P1=offset HI
//    c_apdu[5] = 0x00;                     // P2=offset LO
//    c_apdu[6] = 29+3;                     // Lc = header size
//
//    if( api_ifm_exchangeAPDU( g_dhn_sam, c_apdu, r_apdu ) != apiOK )
//      return( apiFailed );
//    iLen = r_apdu[1]*256 + r_apdu[0];
//    if( (iLen != 2) || (r_apdu[iLen] != 0x90) || (r_apdu[iLen+1] != 0x00) )
//      return( apiFailed );

//    UI_WaitKey();

      // load Modulus (1'st 128 bytes)
//    c_apdu[0] = 5+128;                // 00 NN
//    c_apdu[1] = 0x00;                 // length of APDU
//
//    c_apdu[2] = 0x90;                     // CLA
//    c_apdu[3] = 0x30;                     // INS
//    c_apdu[4] = 0x00;                     // P1=offset HI
//    c_apdu[5] = 29+3;                     // P2=offset LO
//    c_apdu[6] = 128;                      // Lc
//
//    if( api_ifm_exchangeAPDU( g_dhn_sam, c_apdu, r_apdu ) != apiOK )
//      return( apiFailed );
//    iLen = r_apdu[1]*256 + r_apdu[0];
//    if( (iLen != 2) || (r_apdu[iLen] != 0x90) || (r_apdu[iLen+1] != 0x00) )
//      return( apiFailed );

//    UI_WaitKey();

      // load Modulus (2'nd 128 bytes)
//    c_apdu[0] = 5+128;                // 00 NN
//    c_apdu[1] = 0x00;                 // length of APDU
//
//    c_apdu[2] = 0x90;                     // CLA
//    c_apdu[3] = 0x30;                     // INS
//    c_apdu[4] = 0x00;                     // P1=offset HI
//    c_apdu[5] = 29+3+128;                 // P2=offset LO
//    c_apdu[6] = 128;                      // Lc
//
//    if( api_ifm_exchangeAPDU( g_dhn_sam, c_apdu, r_apdu ) != apiOK )
//      return( apiFailed );
//    iLen = r_apdu[1]*256 + r_apdu[0];
//    if( (iLen != 2) || (r_apdu[iLen] != 0x90) || (r_apdu[iLen+1] != 0x00) )
//      return( apiFailed );

//    UI_WaitKey();

      return( apiOK );
}

// ---------------------------------------------------------------------------
// FUNCTION: general icc get response command.
// INPUT   : len  - the expected length of data to be retrieved.
// OUTPUT  : data - the response data retrieved from SAM.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR apdu_SAM_GET_RESPONSE( UCHAR len, UCHAR *data )
{
UCHAR c_apdu[7];
UINT  iLen;

      c_apdu[0] = 0x05;                 // length of APDU
      c_apdu[1] = 0x00;                 //

      c_apdu[2] = 0x00;                     // CLA
      c_apdu[3] = 0xC0;                     // INS
      c_apdu[4] = 0x00;                     // P1
      c_apdu[5] = 0x00;                     // P2
      c_apdu[6] = len;                      // Le

      if( api_ifm_exchangeAPDU( g_dhn_sam, c_apdu, data ) != apiOK )
        return( apiFailed );

      iLen = data[1]*256 + data[0];
      if( ((iLen - 2) != len) || (data[iLen] != 0x90) || (data[iLen+1] != 0x00) )
        return( apiFailed );
      else
        return( apiOK );
}

// ---------------------------------------------------------------------------
// FUNCTION: get RSA CA public key (n,e) from key slot of SAM.
// INPUT   : pki   - public key index. (1 byte)
//           rid   - registered application provider id. (5 bytes)
// OUTPUT  : pkm   - public key modulus.  (2L-V, V:max 256 bytes)
//           pke   - public key exponent. (integer, 3 bytes)
// REF     : g_key_fid, g_capk_cnt
// RETURN  : apiOK
//           apiFailed (not found or device error)
// ---------------------------------------------------------------------------
UCHAR apdu_SAM_GET_PUBLIC_KEY( UCHAR pki, UCHAR *rid, UCHAR *pkm, UCHAR *pke )
{
UCHAR i;
UCHAR c_apdu[16];
UCHAR r_apdu[258];
UINT  iLen;
UINT  iLen2;
UINT  iFID;


      // ---------------------------------------------------------------------
      // Method 2: The CA public keys are stored in terminal SRAM.
      // ---------------------------------------------------------------------
GET_PK2:

      iFID = KEY_FID_01; // initial key file id
      for( i=0; i<g_capk_cnt; i++ )
         {
         apk_ReadRamDataKEY( ADDR_CA_PK_01+CA_PK_LEN*i, CA_PK_HEADER_LEN, &r_apdu[2] );

         // matching RID & INDEX
         if( (TL_memcmp( rid, &r_apdu[2+OFFSET_SAM_RID], 5 ) == 0) && (pki == r_apdu[2+OFFSET_SAM_PKI]) )
           break;
         else
           iFID++;
         }

      if( iFID >= (KEY_FID_01 + g_capk_cnt) )
        return( apiFailed ); // public key not found

      pkm[0] = r_apdu[2+OFFSET_SAM_MOD_LEN+1]; // length of modulus
      pkm[1] = r_apdu[2+OFFSET_SAM_MOD_LEN+0]; //

      g_key_fid = iFID; // set file id for the selected key

      return( apiOK );

      // ---------------------------------------------------------------------
      // Method 3: The CA public keys are hardcoded in terminal ROM.
      // ---------------------------------------------------------------------
}

// ---------------------------------------------------------------------------
// FUNCTION: load public key (n,e) to SAM slot. (eg. issuer or icc PK)
// INPUT   : slot     - slot number to store the key. (=FID)
//           modulus  - public key modulus.  (2L-V, V:max 256 bytes)
//           exponent - public key exponent. (3 bytes, value=2, 3, or 65537)
//
//           1. load public key header + exp (29+(1 or 3) bytes)
//           2. load modulus (up to 256 bytes)
//
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR apdu_SAM_LOAD_EXTERNAL_PK( UINT slot, UCHAR *modulus, UCHAR *exponent )
{
UCHAR len;
UINT  iLen;
UCHAR c_apdu[258];
UCHAR r_apdu[8];

      // select public key (Select_Pub)
      c_apdu[0] = 0x07;                 // length of APDU
      c_apdu[1] = 0x00;                 //

      c_apdu[2] = 0x90;                     // CLA
      c_apdu[3] = 0xE3;                     // INS
      c_apdu[4] = 0x00;                     // P1
      c_apdu[5] = 0x00;                     // P2
      c_apdu[6] = 0x02;                     // Lc
      c_apdu[7] = (slot & 0xff00) >> 8;     // Data=FID
      c_apdu[8] = slot & 0x00ff;            //
      if( api_ifm_exchangeAPDU( g_dhn_sam, c_apdu, r_apdu ) != apiOK )
        return( apiFailed );
      iLen = r_apdu[1]*256 + r_apdu[0];

      if( (iLen != 2) || (r_apdu[iLen] != 0x90) || (r_apdu[iLen+1] != 0x00) )
        return( apiFailed );

//    UI_WaitKey();

      // (Load_Pub)
      // load Header (RID+INDEX+EXP_LEN+MOD_LEN+SHA1) & Exponent

      if( (exponent[0] == 0x01) && (exponent[1] == 0x00) && (exponent[2] == 0x01) )
        c_apdu[13] = 3;                 // exp length
      else
        c_apdu[13] = 1;
      len = c_apdu[13];

      c_apdu[0] = 5+29+len;             //
      c_apdu[1] = 0x00;                 // length of APDU

      c_apdu[2] = 0x90;                 // CLA
      c_apdu[3] = 0x30;                 // INS
      c_apdu[4] = 0x00;                 // P1=offset HI, 00=CA public key
//    c_apdu[4] = 0x80;                 // P1=offset HI, 80=working public key
      c_apdu[5] = 0x00;                 // P2=offset LO
      c_apdu[6] = 29+len;               // Lc = header size

      c_apdu[7]  = 0xF0;                // working key RID=F0 00 00 00 00
      c_apdu[8]  = 0x00;                //
      c_apdu[9]  = 0x00;                //
      c_apdu[10] = 0x00;                //
      c_apdu[11] = 0x00;                //

      c_apdu[12] = 0x01;                // index

      c_apdu[14] = modulus[1];          // modulus length
      c_apdu[15] = modulus[0];          //

      memset( &c_apdu[16], 0x00, 20 );  // SHA1 (RFU)
      if( (len == 1) && (exponent[0] == 0) && (exponent[1] == 0) && (exponent[2] != 0) )
        memmove( &c_apdu[16+20], &exponent[2], len ); // PATCH: 2003-09-05, exponent = "00 00 02" or "00 00 03"
      else                                            //                    reset exponent length=3 to 1
        memmove( &c_apdu[16+20], exponent, len ); // exponent (1 or 3 bytes)

      if( api_ifm_exchangeAPDU( g_dhn_sam, c_apdu, r_apdu ) != apiOK )
        return( apiFailed );
      iLen = r_apdu[1]*256 + r_apdu[0];
      if( (iLen != 2) || (r_apdu[iLen] != 0x90) || (r_apdu[iLen+1] != 0x00) )
        return( apiFailed );

//    UI_WaitKey();

      // load Modulus
      c_apdu[0] = 5+modulus[0];         // 00 NN
      c_apdu[1] = 0x00;                 // length of APDU

      c_apdu[2] = 0x90;                 // CLA
      c_apdu[3] = 0x30;                 // INS
      c_apdu[4] = 0x00;                 // P1=offset HI, 00=CA public key
//    c_apdu[4] = 0x80;                 // P1=offset HI, 80=working public key
      c_apdu[5] = 29+len;               // P2=offset LO
      c_apdu[6] = modulus[0];           // Lc
      memmove( &c_apdu[7], &modulus[2], c_apdu[6] );

      if( api_ifm_exchangeAPDU( g_dhn_sam, c_apdu, r_apdu ) != apiOK )
        return( apiFailed );
      iLen = r_apdu[1]*256 + r_apdu[0];
      if( (iLen != 2) || (r_apdu[iLen] != 0x90) || (r_apdu[iLen+1] != 0x00) )
        return( apiFailed );
      else
        return( apiOK );
}

// ---------------------------------------------------------------------------
// FUNCTION: recover the certificate data using the specified public key.
// INPUT   : slot - slot number of the selected public key (=FID)
//           pkc  - public key certificate. (2L-V, V:max 256 bytes)
// OUTPUT    pkc  - the recover data of the certificate. (2L-V, V:max 256 bytes)
// REF     :
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR apdu_SAM_RSA_RECOVER( UINT slot, UCHAR *pkc )
{
UINT  iLen;
UCHAR c_apdu[260];
UCHAR r_apdu[8];

      // select public key (Select_Pub)
      c_apdu[0] = 0x07;                 // length of APDU
      c_apdu[1] = 0x00;                 //

      c_apdu[2] = 0x90;                     // CLA
      c_apdu[3] = 0xE3;                     // INS
      c_apdu[4] = 0x00;                     // P1
      c_apdu[5] = 0x00;                     // P2
      c_apdu[6] = 0x02;                     // Lc
      c_apdu[7] = (slot & 0xff00) >> 8;     // Data=FID
      c_apdu[8] = slot & 0x00ff;            //
      if( api_ifm_exchangeAPDU( g_dhn_sam, c_apdu, r_apdu ) != apiOK )
        return( apiFailed );
      iLen = r_apdu[1]*256 + r_apdu[0];
      if( (iLen != 2) || (r_apdu[iLen] != 0x90) || (r_apdu[iLen+1] != 0x00) )
        return( apiFailed );

//    UI_WaitKey();

      // recover the certificate data (Ext_Recover)
      iLen = 6 + pkc[0];
      c_apdu[0] = iLen & 0x00ff;        // length of APDU
      c_apdu[1] = (iLen & 0xff00) >> 8; //

      c_apdu[2] = 0x90;                     // CLA
      c_apdu[3] = 0x36;                     // INS
      c_apdu[4] = 0x00;                     // P1
      c_apdu[5] = 0x00;                     // P2
      c_apdu[6] = pkc[0];                   // Lc
      memmove( &c_apdu[7], &pkc[2], pkc[0] ); // Data
      iLen = pkc[0];
      c_apdu[7+iLen] = 0x00;                // Le

      if( api_ifm_exchangeAPDU( g_dhn_sam, c_apdu, pkc ) != apiOK )
        return( apiFailed );
      iLen = pkc[1]*256 + pkc[0];
      if( (pkc[iLen] != 0x90) || (pkc[iLen+1] != 0x00) )
        return( apiFailed );

      iLen -= 2; // ignore "9000"
      pkc[0] = iLen & 0x00ff;
      pkc[1] = (iLen & 0xff00) >> 8;

      return( apiOK );
}

// ---------------------------------------------------------------------------
// FUNCTION: generate a hash value.
// INPUT   : algorithm - SHA1 (20-byte) or MD5 (16-byte).
//           length    - length of data to be digested.
//           data      - the specified data.
// OUTPUT    digest    - the digest (20 or 16 bytes).
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR apdu_SAM_HASH( UCHAR algorithm, UINT length, UCHAR *data, UCHAR *digest )
{
UINT  i;
UCHAR c_apdu[MAX_SAM_BUF_LEN+7];
UCHAR r_apdu[24];

      // --- DEBUG ONLY ---
//#ifdef  L2_SW_DEBUG
//
//UCHAR ISSUER_X_HASH[]={
//                   0xD8, 0x16, 0x00, 0xE8, 0xEE, 0x55, 0xA8, 0x3B, 0xD5, 0xCF,
//                   0x38, 0x44, 0x34, 0x30, 0x87, 0xC9, 0x39, 0x29, 0xD4, 0xA0
//                   };
//
//      for( i=0; i<20; i++ )
//         digest[i] = ISSUER_X_HASH[i];
//
//      return( apiOK );
//#endif
      // ------------------

      if( length <= MAX_SAM_BUF_LEN )
        {
        i = 0x05 + length;
        c_apdu[0] = i & 0x00ff;         // length of APDU
        c_apdu[1] = (i & 0xff00) >> 8;  //

        c_apdu[2] = 0x90;                       // CLA
        c_apdu[3] = 0xB2;                       // INS = SHA1
        c_apdu[4] = 0x00;                       // P1  = mode
        c_apdu[5] = 0x00;                       // P2
        c_apdu[6] = (UCHAR)length;              // Le
        memmove( &c_apdu[7], data, length );    // Data

        if( api_ifm_exchangeAPDU( g_dhn_sam, c_apdu, r_apdu ) != apiOK )
          return( apiFailed );
        if( (r_apdu[0] != 2) || (r_apdu[2] != 0x61) || (r_apdu[3] != 0x14) )
          return( apiFailed );
        }
      else
        {
        // send "Initial Block"
        i = 0x05 + MAX_SAM_BUF_LEN;
        c_apdu[0] = i & 0x00ff;         // length of APDU
        c_apdu[1] = (i & 0xff00) >> 8;  //
        c_apdu[2] = 0x90;                          // CLA
        c_apdu[3] = 0xB2;                          // INS = SHA1
        c_apdu[4] = 0x10;                          // P1  = mode
        c_apdu[5] = 0x01;                          // P2  = inital
        c_apdu[6] = MAX_SAM_BUF_LEN;               // Le
        memmove( &c_apdu[7], data, c_apdu[6] );    // Data

        if( api_ifm_exchangeAPDU( g_dhn_sam, c_apdu, r_apdu ) != apiOK )
          return( apiFailed );
        if( (r_apdu[0] != 2) || (r_apdu[2] != 0x90) || (r_apdu[3] != 0x00) )
          return( apiFailed );

//      UI_WaitKey();

        length -= MAX_SAM_BUF_LEN;
        data += MAX_SAM_BUF_LEN;

        while( length > MAX_SAM_BUF_LEN )
             {
             // send "Intermediate Blocks"
             i = 0x05 + MAX_SAM_BUF_LEN;
             c_apdu[0] = i & 0x00ff;         // length of APDU
             c_apdu[1] = (i & 0xff00) >> 8;  //
             c_apdu[2] = 0x90;                          // CLA
             c_apdu[3] = 0xB2;                          // INS = SHA1
             c_apdu[4] = 0x10;                          // P1  = mode
             c_apdu[5] = 0x40;                          // P2  = intermidate
             c_apdu[6] = MAX_SAM_BUF_LEN;               // Le
             memmove( &c_apdu[7], data, c_apdu[6] );    // Data

             if( api_ifm_exchangeAPDU( g_dhn_sam, c_apdu, r_apdu ) != apiOK )
               return( apiFailed );
             if( (r_apdu[0] != 2) || (r_apdu[2] != 0x90) || (r_apdu[3] != 0x00) )
               return( apiFailed );

             length -= MAX_SAM_BUF_LEN;
             data += MAX_SAM_BUF_LEN;

//           UI_WaitKey();
             }

        // send "Last Block" and generate HASH result
        i = 0x05 + length;
        c_apdu[0] = i & 0x00ff;         // length of APDU
        c_apdu[1] = (i & 0xff00) >> 8;  //
        c_apdu[2] = 0x90;                          // CLA
        c_apdu[3] = 0xB2;                          // INS = SHA1
        c_apdu[4] = 0x10;                          // P1  = mode
        c_apdu[5] = 0x7F;                          // P2  = last
        c_apdu[6] = (UCHAR)length;                 // Le
        memmove( &c_apdu[7], data, c_apdu[6] );    // Data

        if( api_ifm_exchangeAPDU( g_dhn_sam, c_apdu, r_apdu ) != apiOK )
          return( apiFailed );
        if( (r_apdu[0] != 2) || (r_apdu[2] != 0x61) || (r_apdu[3] != 0x14) )
          return( apiFailed );
        }

//    UI_WaitKey();

      // get final HASH result
      if( apdu_SAM_GET_RESPONSE( 20, r_apdu ) != apiOK )
        return( apiFailed );
      else
        {
        memmove( digest, &r_apdu[2], 20 );
        return( apiOK );
        }
}

// ---------------------------------------------------------------------------
// FUNCTION: to generate an unpredictable number (8-byte).
// INPUT   : none.
// OUTPUT    random - the random number.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR apdu_SAM_GET_CHALLENGE( UCHAR *random )
{
UCHAR c_apdu[7];
UCHAR r_apdu[12];

      // --- DEBUG ONLY ---
#ifdef  L2_SW_DEBUG

UCHAR ran_nbr[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };

      memmove( random, ran_nbr, 8 );
      return( apiOK );

#endif
      // ------------------

      c_apdu[0] = 0x05;                 // length of APDU
      c_apdu[1] = 0x00;                 //

      c_apdu[2] = 0x90;                     // CLA
      c_apdu[3] = 0xB6;                     // INS
      c_apdu[4] = 0x00;                     // P1
      c_apdu[5] = 0x00;                     // P2
      c_apdu[6] = 0x08;                     // Le

      if( api_ifm_exchangeAPDU( g_dhn_sam, c_apdu, r_apdu ) != apiOK )
        return( apiFailed );
      if( (r_apdu[0] != 10) || (r_apdu[10] != 0x90) || (r_apdu[11] != 0x00) )
        return( apiFailed );

      memmove( random, &r_apdu[2], 8 );

      return( apiOK );
}

// ---------------------------------------------------------------------------
// FUNCTION: The SELECT command is used to select the ICC PSE, DDF, or ADF
//           corresponding to the submitted file name or AID.
// INPUT   : filename   - 1L-V
//           occurrence - 0x00=first or only occurrence.
//                        0x01=next occurrence.
// OUTPUT  : fci - 2L-V type, the file control information.
// REF     : g_dhn_sam
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR apdu_SAM_SELECT( UCHAR *filename, UCHAR occurrence, UCHAR *fci )
{
UCHAR i;
UCHAR c_apdu[24];

      c_apdu[0] = filename[0] + 6;      // length of APDU
      c_apdu[1] = 0x00;                 //
      c_apdu[2] = 0x00;                     // CLA
      c_apdu[3] = 0xa4;                     // INS
      c_apdu[4] = 0x04;                     // P1
      c_apdu[5] = (occurrence << 1) & 0x03; // P2
      c_apdu[6] = filename[0];              // Lc
      for(i=0; i<filename[0]; i++)          // Data=filename (5~16 bytes)
         c_apdu[7+i] = filename[1+i];       //
      c_apdu[7+i] = 0x00;                   // Le

      return( api_ifm_exchangeAPDU( g_dhn_sam, c_apdu, fci ) );
}

// ---------------------------------------------------------------------------
// FUNCTION: garbage collector to avoid error code 6611 (out of system memory).
//           this command is for OCS-SAM ONLY.
// INPUT   : none.
// OUTPUT  : response - 2L-V ("90 00")
// REF     : g_dhn_sam
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR apdu_SAM_CLEAN( UCHAR *response )
{
UCHAR c_apdu[8];

      c_apdu[0] = 0x04;                 // length of APDU
      c_apdu[1] = 0x00;                 //
      c_apdu[2] = 0x90;                     // CLA
      c_apdu[3] = 0x34;                     // INS
      c_apdu[4] = 0x03;                     // P1
      c_apdu[5] = 0x00;                     // P2
      return( api_ifm_exchangeAPDU( g_dhn_sam, c_apdu, response) );
}

